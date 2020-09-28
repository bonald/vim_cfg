
#include "proto.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_iproute_node_define.h"
#include "gen/tbl_nexthop_define.h"
#include "lib_netlink.h"
#include <linux/version.h>
#include <linux/netlink.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 32)
#include <linux/if_link.h>
#else
#define IFLA_RTA(r)  ((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ifinfomsg))))
#define IFLA_PAYLOAD(n) NLMSG_PAYLOAD(n,sizeof(struct ifinfomsg))

#define NLA_ALIGNTO           4
#define NLA_ALIGN(len)        (((len) + NLA_ALIGNTO - 1) & ~(NLA_ALIGNTO - 1))
#define NLA_HDRLEN            ((int) NLA_ALIGN(sizeof(struct nlattr)))

struct nlattr {
    __u16           nla_len;
    __u16           nla_type;
};

#define IFLA_MAX 100
#endif /* !LINUX_VERSION_CODE */

#define RTMGRP_LINK         0x0001
#define RTMGRP_NOTIFY       0x0002
#define RTMGRP_NEIGH        0x0004
#define RTMGRP_TC           0x0008
#define RTMGRP_IPV4_IFADDR  0x0010
#define RTMGRP_IPV4_MROUTE  0x0020
#define RTMGRP_IPV4_ROUTE   0x0040
#define RTMGRP_IPV4_RULE    0x0080
#define RTMGRP_IPV6_IFADDR  0x0100
#define RTMGRP_IPV6_MROUTE  0x0200
#define RTMGRP_IPV6_ROUTE   0x0400
#define RTMGRP_IPV6_IFINFO  0x0800

#define RTNL_FAMILY_IPMR    128
#define RTNL_FAMILY_IP6MR   129

#define IFLA_IFNAME         3

/*add by zhw for ipmc*/
#define 	PIM_VRF_ID   2
enum
{
    IFA_UNSPEC,
    IFA_ADDRESS,
    IFA_LOCAL,
    IFA_LABEL,
    IFA_BROADCAST,
    IFA_ANYCAST,
    IFA_CACHEINFO,
    IFA_MULTICAST,
    __IFA_MAX,
};
#define IFA_MAX (__IFA_MAX - 1)

struct ifaddrmsg
{
    uint8        ifa_family;
    uint8        ifa_prefixlen;  /* The prefix length        */
    uint8        ifa_flags;  /* Flags            */
    uint8        ifa_scope;  /* Address scope        */
    uint32       ifa_index;  /* Link index           */
};

/* ifa_flags */
#define IFA_F_SECONDARY     0x01
#define IFA_F_TEMPORARY     IFA_F_SECONDARY

#define IFA_F_NODAD     0x02
#define IFA_F_OPTIMISTIC    0x04
#define IFA_F_DADFAILED     0x08
#define IFA_F_HOMEADDRESS   0x10
#define IFA_F_DEPRECATED    0x20
#define IFA_F_TENTATIVE     0x40
#define IFA_F_PERMANENT     0x80


/* Routing message attributes */
enum rtattr_type_t
{
    RTA_UNSPEC,
    RTA_DST,/*1*/
    RTA_SRC,
    RTA_IIF,
    RTA_OIF,
    RTA_GATEWAY,/*5*/
    RTA_PRIORITY,/*6*/
    RTA_PREFSRC,
    RTA_METRICS,
    RTA_MULTIPATH,/*9*/
    RTA_PROTOINFO, /* no longer used */
    RTA_FLOW,
    RTA_CACHEINFO,
    RTA_SESSION, /* no longer used */
    RTA_MP_ALGO, /* no longer used */
/* SYSTEM MODIFIED by chenc for bug 52095, 2019-06-25 */
#ifdef _CENTEC_
    RTA_ROUTE_TYPE, /*static or ospf*/
#endif
/*chenc end*/
    RTA_TABLE,
    __RTA_MAX
};

#define RTA_MAX (__RTA_MAX - 1)

#define RTM_RTA(r)  ((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct rtmsg))))
#define RTM_PAYLOAD(n) NLMSG_PAYLOAD(n,sizeof(struct rtmsg))

/* rtm_type */

enum
{
    RTN_UNSPEC,
    RTN_UNICAST,        /* Gateway or direct route  */
    RTN_LOCAL,      /* Accept locally       */
    RTN_BROADCAST,      /* Accept locally as broadcast,
                   send as broadcast */
    RTN_ANYCAST,        /* Accept locally as broadcast,
                   but send as unicast */
    RTN_MULTICAST,      /* Multicast route      */
    RTN_BLACKHOLE,      /* Drop             */
    RTN_UNREACHABLE,    /* Destination is unreachable   */
    RTN_PROHIBIT,       /* Administratively prohibited  */
    RTN_THROW,      /* Not in this table        */
    RTN_NAT,        /* Translate this address   */
    RTN_XRESOLVE,       /* Use external resolver    */
    __RTN_MAX
};

#define RTN_MAX (__RTN_MAX - 1)

/* rtm_protocol */

#define RTPROT_UNSPEC   0
#define RTPROT_REDIRECT 1   /* Route installed by ICMP redirects; not used by current IPv4 */
#define RTPROT_KERNEL   2   /* Route installed by kernel        */
#define RTPROT_BOOT     3   /* Route installed during boot      */
#define RTPROT_STATIC   4   /* Route installed by administrator */

#define RTPROT_GATED	8	/* Apparently, GateD */
#define RTPROT_RA	9	/* RDISC/ND router advertisements */
#define RTPROT_MRT	10	/* Merit MRT */
#define RTPROT_ZEBRA	11	/* Zebra */
#define RTPROT_BIRD	12	/* BIRD */
#define RTPROT_DNROUTED	13	/* DECnet routing daemon */
#define RTPROT_XORP	14	/* XORP */
#define RTPROT_NTK	15	/* Netsukuku */
#define RTPROT_DHCP	16      /* DHCP client */
/* rtm_scope

   Really it is not scope, but sort of distance to the destination.
   NOWHERE are reserved for not existing destinations, HOST is our
   local addresses, LINK are destinations, located on directly attached
   link and UNIVERSE is everywhere in the Universe.

   Intermediate values are also possible f.e. interior routes
   could be assigned a value between UNIVERSE and LINK.
*/

enum rt_scope_t
{
    RT_SCOPE_UNIVERSE=0,
/* User defined values  */
    RT_SCOPE_SITE=200,
    RT_SCOPE_LINK=253,
    RT_SCOPE_HOST=254,
    RT_SCOPE_NOWHERE=255
};

/* rtm_flags */

#define RTM_F_NOTIFY        0x100   /* Notify user of route change  */
#define RTM_F_CLONED        0x200   /* This route is cloned     */
#define RTM_F_EQUALIZE      0x400   /* Multipath equalizer: NI  */
#define RTM_F_PREFIX        0x800   /* Prefix addresses     */

struct rtattr
{
    uint16   rta_len;
    uint16   rta_type;
};

struct ifinfomsg {
    unsigned char ifi_family;
    unsigned char __ifi_pad;
    unsigned short ifi_type; /* ARPHRD_* */
    int ifi_index;           /* Link index	*/
    unsigned ifi_flags;      /* IFF_* flags	*/
    unsigned ifi_change;     /* IFF_* change mask */
};

enum {
    NDA_UNSPEC,
    NDA_DST,
    NDA_LLADDR,
    NDA_CACHEINFO,
    NDA_PROBES,
    __NDA_MAX
};

#define NDA_MAX (__NDA_MAX - 1)

/*
 *  Neighbor Cache Entry Flags
 */

#define NTF_USE     0x01
#define NTF_PROXY   0x08    /* == ATF_PUBL */
#define NTF_ROUTER  0x80

/*
 *  Neighbor Cache Entry States.
 */
#define NUD_INCOMPLETE  0x01
#define NUD_REACHABLE   0x02
#define NUD_STALE   0x04
#define NUD_DELAY   0x08
#define NUD_PROBE   0x10
#define NUD_FAILED  0x20

/* Dummy states */
#define NUD_NOARP   0x40
#define NUD_PERMANENT   0x80
#define NUD_NONE    0x00

/* Macros to handle rtattributes */

#define RTA_ALIGNTO 4
#define RTA_ALIGN(len) ( ((len)+RTA_ALIGNTO-1) & ~(RTA_ALIGNTO-1) )
#define RTA_OK(rta,len) ((len) >= (int)sizeof(struct rtattr) && \
             (rta)->rta_len >= sizeof(struct rtattr) && \
             (rta)->rta_len <= (len))
#define RTA_NEXT(rta,attrlen)   ((attrlen) -= RTA_ALIGN((rta)->rta_len), \
                 (struct rtattr*)(((char*)(rta)) + RTA_ALIGN((rta)->rta_len)))
#define RTA_LENGTH(len) (RTA_ALIGN(sizeof(struct rtattr)) + (len))
#define RTA_SPACE(len)  RTA_ALIGN(RTA_LENGTH(len))
#define RTA_DATA(rta)   ((void*)(((char*)(rta)) + RTA_LENGTH(0)))
#define RTA_PAYLOAD(rta) ((int)((rta)->rta_len) - RTA_LENGTH(0))

#define IFA_RTA(r)  ((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ifaddrmsg))))
#define IFA_PAYLOAD(n) NLMSG_PAYLOAD(n,sizeof(struct ifaddrmsg))

#define NL_SOCK_NAME_LEN    20
#define RT_TABLE_MAIN       0

#define RTNH_ALIGNTO    4
#define RTNH_ALIGN(len) ( ((len)+RTNH_ALIGNTO-1) & ~(RTNH_ALIGNTO-1) )
#define RTNH_OK(rtnh,len) ((rtnh)->rtnh_len >= sizeof(struct rtnexthop) && \
               ((int)(rtnh)->rtnh_len) <= (len))
#define RTNH_NEXT(rtnh) ((struct rtnexthop*)(((char*)(rtnh)) + RTNH_ALIGN((rtnh)->rtnh_len)))
#define RTNH_LENGTH(len) (RTNH_ALIGN(sizeof(struct rtnexthop)) + (len))
#define RTNH_SPACE(len) RTNH_ALIGN(RTNH_LENGTH(len))
#define RTNH_DATA(rtnh)   ((struct rtattr*)(((char*)(rtnh)) + RTNH_LENGTH(0)))

/* Socket interface to kernel */
typedef struct
{
    int32 sock;
    int32 seq;
    struct sal_sockaddr_nl snl;
    char name[NL_SOCK_NAME_LEN];
    ctc_task_t *t_read;
} lib_nl_sock_t;

typedef struct
{
    RT_INTERFACE_FUNC       add_interface;
    RT_INTERFACE_FUNC       del_interface;
    RT_IPV4_ADDR_FUNC       ipv4_add_addr;
    RT_IPV4_ADDR_FUNC       ipv4_del_addr;
    RT_IPV4_ROUTE_FUNC      ipv4_add_route;
    RT_IPV4_ROUTE_FUNC      ipv4_del_route;
    RT_IPV4_MROUTE_FUNC     ipv4_add_mroute;
    RT_IPV4_MROUTE_FUNC     ipv4_del_mroute;
    RT_IPV4_RT_MULTI_FUNC   ipv4_add_route_multipath_check;
    RT_IPV4_RT_MULTI_FUNC   ipv4_del_route_multipath_check;
    RT_IPV4_RT_MULTI_FUNC   ipv4_add_route_multipath;
    RT_IPV4_RT_MULTI_FUNC   ipv4_del_route_multipath;
    RT_IPV4_NH_GROUP_FUNC   ipv4_add_nh_group;
    RT_IPV4_NH_GROUP_FUNC   ipv4_del_nh_group;
    RT_IPV4_NH_GROUP_FUNC   ipv4_get_nh_key;
    RT_IPV4_ADD_NEIGH_FUNC  ipv4_add_neigh;
    RT_IPV4_ADD_NEIGH_FUNC  ipv4_add_imcompete_neigh;
    RT_IPV4_DEL_NEIGH_FUNC  ipv4_del_neigh;
    RT_IPV4_GET_NEIGH_FUNC  ipv4_get_neigh;
    
    RT_IPV6_ADDR_FUNC       ipv6_add_addr;
    RT_IPV6_ADDR_FUNC       ipv6_del_addr;
    RT_IPV6_ROUTE_FUNC      ipv6_add_route;
    RT_IPV6_ROUTE_FUNC      ipv6_del_route;
    RT_IPV6_RT_MULTI_FUNC   ipv6_add_route_multipath_check;
    RT_IPV6_RT_MULTI_FUNC   ipv6_del_route_multipath_check;
    RT_IPV6_RT_MULTI_FUNC   ipv6_add_route_multipath;
    RT_IPV6_RT_MULTI_FUNC   ipv6_del_route_multipath;
    RT_IPV6_NH_GROUP_FUNC   ipv6_get_nh_key;
    RT_IPV6_GET_NEIGH_FUNC  ipv6_get_neigh;
} lib_nl_ops_t;

typedef struct
{
    lib_nl_sock_t *listen[GLB_MAX_VRFID];
    lib_nl_sock_t *cmd;
    lib_nl_sock_t *arp;
    lib_nl_ops_t  ops;
} lib_nl_master_t;

/* Socket interface to kernel */
struct nlsock
{
  int sock;
  int seq;
  struct sockaddr_nl snl;
  const char *name;
} netlink      = { -1, 0, {0}, "netlink-listen"},     /* kernel messages */
  netlink_cmd  = { -1, 0, {0}, "netlink-cmd"};        /* command channel */

uint32 g_netlink_dispatch_count = 0;
lib_nl_master_t *g_nl_master = NULL;

int32
lib_sock_set_buf(uint32 sock, uint32 send, int32 size)
{
    uint32 option = (send) ? SO_SNDBUF : SO_RCVBUF;
    return sal_setsockopt(sock, SOL_SOCKET, option, &size, sizeof (size));
}

int32
lib_sock_get_buf(uint32 sock, uint32 send, int32 *size)
{
    uint32 option = (send) ? SO_SNDBUF : SO_RCVBUF;
    socklen_t len = sizeof(int32);
    return sal_getsockopt(sock, SOL_SOCKET, option, size, &len);
}

static int32
_lib_netlink_parse(int32 (*dispatch)(struct sal_sockaddr_nl *, struct nlmsghdr *), lib_nl_sock_t *nl)
{
    int32 status = 0;
    int32 rc = PM_E_NONE;

    while (1)
    {
        char buf[4096];
        struct sal_iovec iov = {buf, sizeof buf};
        struct sal_sockaddr_nl snl;
        struct sal_msghdr msg = { (void*)&snl, sizeof snl, &iov, 1, NULL, 0, 0};
        struct nlmsghdr *h;

        status = sal_recvmsg(nl->sock, &msg, 0);
        if (status < 0)
        {
            if (EINTR == errno)
            {
                continue;
            }

            if (EWOULDBLOCK == errno || EAGAIN == errno)
            {
                break;
            }

            if (ESPIPE == errno)
            {
                log_sys(M_MOD_LIB, E_ERROR, "%s recvmsg overrun: %s", nl->name, sal_strerror(errno));
                break;
            }
        }

        if (snl.nl_pid != 0)
        {
            continue;
        }

        if (status == 0)
        {
            return PM_E_FAIL;
        }

        if (msg.msg_namelen != sizeof snl)
        {
            return PM_E_FAIL;
        }

        for (h = (struct nlmsghdr *) buf; NLMSG_OK (h, status); h = NLMSG_NEXT (h, status))
        {
            /* Finish of reading. */
            if (h->nlmsg_type == NLMSG_DONE)
            {
                return rc;
            }

            /* Error handling. */
            if (h->nlmsg_type == NLMSG_ERROR)
            {
                struct nlmsgerr *err = (struct nlmsgerr *) NLMSG_DATA (h);

                /* Sometimes the nlmsg_type is NLMSG_ERROR but the err->error
                 *                  is 0. This is a success. */
                if (err->error == 0)
                {
                    /* return if not a multipart message, otherwise continue */
                    if (!(h->nlmsg_flags & NLM_F_MULTI))
                    {
                        return PM_E_NONE;
                    }
                    continue;
                }

                if (h->nlmsg_len < NLMSG_LENGTH (sizeof (struct nlmsgerr)))
                {
                    return PM_E_FAIL;
                }

                return PM_E_FAIL;
            }

            rc = (*dispatch) (&snl, h);
            if (rc < 0)
            {
                log_sys(M_MOD_LIB, E_ERROR, "%s process netlink msg fail, rc is %d", nl->name, rc);
            }

            if (rc > 0)
            {
                return rc;
            }
        }

        /* After error care. */
        if (msg.msg_flags & MSG_TRUNC)
        {
            continue;
        }

        if (status)
        {
            return PM_E_FAIL;
        }
    }

    return rc;
}

static void
_lib_netlink_parse_rtattr(struct rtattr **tb, int max, struct rtattr *rta,
                      int len)
{
    while (RTA_OK (rta, len))
    {
      if (rta->rta_type <= max)
        tb[rta->rta_type] = rta;
      rta = RTA_NEXT (rta, len);
    }
}

vrf_id_t
lib_netlink_rtm_get_vrf_id(struct rtmsg *rtm)
{
#ifdef HAVE_MULTIPLE_FIB
    return (vrf_id_t)rtm->rtm_vrf;
#endif /* HAVE_MULTIPLE_FIB */
    return 0;
}

static int
netlink_link_change (struct sockaddr_nl *snl, struct nlmsghdr *h)
{
    int len;
    struct ifinfomsg *ifi;
    struct rtattr *tb[IFLA_MAX + 1];
    char *name;

    ifi = NLMSG_DATA (h);

    if (!(h->nlmsg_type == RTM_NEWLINK || h->nlmsg_type == RTM_DELLINK))
    {
        return 0;
    }

    len = h->nlmsg_len - NLMSG_LENGTH (sizeof (struct ifinfomsg));
    if (len < 0)
    {
        return -1;
    }

    /* Looking up interface name. */
    sal_memset (tb, 0, sizeof tb);
    _lib_netlink_parse_rtattr (tb, 12, IFLA_RTA (ifi), len);

    if (tb[IFLA_IFNAME] == NULL)
        return -1;

    name = (char *) RTA_DATA (tb[IFLA_IFNAME]);

    /* Add interface. */
    if (h->nlmsg_type == RTM_NEWLINK)
    {
        g_nl_master->ops.add_interface(name, ifi->ifi_index);
    }
    else
    {
        g_nl_master->ops.del_interface(name, ifi->ifi_index);
    }

    return 0;
}

int32
netlink_addr_process(struct sal_sockaddr_nl *snl, struct nlmsghdr *h)
{
    struct ifaddrmsg *ifa = NULL;
    struct rtattr *tb[IFA_MAX + 1];
    void *addr;
    void *broad;
    int32 len = 0;

    ifa = NLMSG_DATA(h);
    len = h->nlmsg_len - NLMSG_LENGTH(sizeof (struct ifaddrmsg));
    if (len < 0)
    {
        return PM_E_FAIL;
    }

    sal_memset(tb, 0, sizeof tb);
    _lib_netlink_parse_rtattr(tb, IFA_MAX, IFA_RTA(ifa), len);

    /* logic copied from iproute2/ip/ipaddress.c:print_addrinfo() */
    if (tb[IFA_LOCAL] == NULL)
        tb[IFA_LOCAL] = tb[IFA_ADDRESS];
    if (tb[IFA_ADDRESS] == NULL)
        tb[IFA_ADDRESS] = tb[IFA_LOCAL];

    /* local interface address */
    addr = (tb[IFA_LOCAL] ? RTA_DATA(tb[IFA_LOCAL]) : NULL);

    /* is there a peer address? */
    if (tb[IFA_ADDRESS] &&
      sal_memcmp(RTA_DATA(tb[IFA_ADDRESS]), RTA_DATA(tb[IFA_LOCAL]), RTA_PAYLOAD(tb[IFA_ADDRESS])))
    {
        broad = RTA_DATA(tb[IFA_ADDRESS]);
    }
    else
    {
        /* seeking a broadcast address */
        broad = (tb[IFA_BROADCAST] ? RTA_DATA(tb[IFA_BROADCAST]) : NULL);
    }

    if (AF_INET == ifa->ifa_family)
    {
        if (RTM_NEWADDR == h->nlmsg_type)
        {
            if (g_nl_master->ops.ipv4_add_addr)
            {
                g_nl_master->ops.ipv4_add_addr(GLB_DEF_VRFID, addr, ifa->ifa_prefixlen, ifa->ifa_index);
            }
        }
        else
        {
            if (g_nl_master->ops.ipv4_del_addr)
            {
                g_nl_master->ops.ipv4_del_addr(GLB_DEF_VRFID, addr, ifa->ifa_prefixlen, ifa->ifa_index);
            }
        }
    }
    else if (AF_INET6 == ifa->ifa_family)
    {
        if (RTM_NEWADDR == h->nlmsg_type)
        {
            if (g_nl_master->ops.ipv6_add_addr)
            {
                g_nl_master->ops.ipv6_add_addr(GLB_DEF_VRFID, addr, ifa->ifa_prefixlen, ifa->ifa_index);
            }
        }
        else
        {
            if (g_nl_master->ops.ipv6_del_addr)
            {
                g_nl_master->ops.ipv6_del_addr(GLB_DEF_VRFID, addr, ifa->ifa_prefixlen, ifa->ifa_index);
            }
        }
    }
    (void)broad;
    return PM_E_NONE;
}

int32
netlink_mroute_process(vrf_id_t vrf_id, struct sal_sockaddr_nl *snl, struct nlmsghdr *h)
{
    int32   rc  = PM_E_NONE;
    int32   len = 0;
    uint32  iif = 0;
    uint32  oif[256];
	uint32  oif_count = 0;
    void *  source = NULL;
    void *  group  = NULL;
	
	LIB_NL_LOG_ERROR("zhw test:netlink_mroute_process ");
    struct rtmsg *rtm;
    rtm = NLMSG_DATA(h);
    len = h->nlmsg_len - NLMSG_LENGTH(sizeof(struct rtmsg));

    struct rtattr *tb[RTA_MAX + 1];
    sal_memset(tb, 0, sizeof tb);
    _lib_netlink_parse_rtattr(tb, RTA_MAX, RTM_RTA(rtm), len);

    if (tb[RTA_IIF])
    {
        iif = *(int *)RTA_DATA(tb[RTA_IIF]);
    }

    if (tb[RTA_SRC])
    {
        source = RTA_DATA(tb[RTA_SRC]);
    }

    if (tb[RTA_DST])
    {
        group  = RTA_DATA(tb[RTA_DST]);
    }

    if (tb[RTA_MULTIPATH]) {
        struct rtnexthop *rtnh = (struct rtnexthop *)RTA_DATA(tb[RTA_MULTIPATH]);

        len = RTA_PAYLOAD(tb[RTA_MULTIPATH]);
        for (;;) {
            if (len < (int)sizeof(*rtnh) || rtnh->rtnh_len > len)
                break;

            if(rtnh->rtnh_ifindex == iif)
            {
                len -= NLMSG_ALIGN(rtnh->rtnh_len);
                rtnh = RTNH_NEXT(rtnh);
                continue;
            }

            oif[oif_count] = rtnh->rtnh_ifindex;
            oif_count++;

            len -= NLMSG_ALIGN(rtnh->rtnh_len);
            rtnh = RTNH_NEXT(rtnh);
        }
    }

    if (RTM_NEWROUTE == h->nlmsg_type)
    {
        if (g_nl_master->ops.ipv4_add_mroute)
        {
            g_nl_master->ops.ipv4_add_mroute(vrf_id, source, group, iif, oif, oif_count);
        }
    }
    else
    {
        if (g_nl_master->ops.ipv4_del_mroute)
        {
            g_nl_master->ops.ipv4_del_mroute(vrf_id, source, group, iif, oif, oif_count);
        }
    }

    return rc;
}

struct message {
	int key;
	char *str;
};

struct message rtproto_str [] =
{
  {RTPROT_REDIRECT, "redirect"},
  {RTPROT_KERNEL,   "kernel"},
  {RTPROT_BOOT,     "boot"},
  {RTPROT_STATIC,   "static"},
  {RTPROT_GATED,    "GateD"},
  {RTPROT_RA,       "router advertisement"},
  {RTPROT_MRT,      "MRT"},
  {RTPROT_ZEBRA,    "Zebra"},
#ifdef RTPROT_BIRD
  {RTPROT_BIRD,     "BIRD"},
#endif /* RTPROT_BIRD */
  {0,               NULL}
};

char *
lookup (struct message *mes, int32 key)
{
  struct message *pnt;

  for (pnt = mes; pnt->key != 0; pnt++)
    {
      if (pnt->key == key)
        {
          return pnt->str;
        }
    }
  return "N/A";
}

int32
netlink_route_process_ipv6(vrf_id_t vrf_id, struct sal_sockaddr_nl *snl, struct nlmsghdr *h, 
    struct rtmsg *rtm, void *dest, void *gate, int32 ifindex, uint32 distance)
{
    int32 rc = 0;
    int32 index = 0;
    int32 len = 0;
    uint32 num_nexthop = 0;
    nexthop_key_t nh_group[IP_ECMP_MAX_PATH];
    nexthop_key_t nh_key = {0};
    struct rtattr *tb[RTA_MAX + 1];
    
#ifdef USE_FPM_IPUC
    len = h->nlmsg_len - NLMSG_LENGTH(sizeof (struct rtmsg));
    if (len < 0)
    {
        return PM_E_FAIL;
    }
#endif

    sal_memset(tb, 0, sizeof tb);
    _lib_netlink_parse_rtattr(tb, RTA_MAX, RTM_RTA(rtm), len);

    struct rtnexthop *rtnh =
            (struct rtnexthop *) RTA_DATA (tb[RTA_MULTIPATH]);


    if (RTM_NEWROUTE == h->nlmsg_type)
    {
        if (g_nl_master->ops.ipv6_add_route && !tb[RTA_MULTIPATH])
        {
            g_nl_master->ops.ipv6_add_route(vrf_id, dest, gate, rtm->rtm_dst_len, ifindex, distance, rtm->rtm_type);
        }
    }
    else
    {
        if (g_nl_master->ops.ipv6_del_route && !tb[RTA_MULTIPATH])
        {
            g_nl_master->ops.ipv6_del_route(vrf_id, dest, gate, rtm->rtm_dst_len, ifindex, distance, rtm->rtm_type);
        }
    }

    if (!tb[RTA_MULTIPATH])
    {
        return PM_E_NONE;
    }

    len = RTA_PAYLOAD (tb[RTA_MULTIPATH]);
    sal_memset(nh_group, 0, sizeof(nh_group));
    sal_memset(&nh_key, 0, sizeof(nh_key));
    if (RTM_NEWROUTE == h->nlmsg_type)
    {
        for (;;)
        {
            if (len < (int) sizeof (*rtnh) || rtnh->rtnh_len > len)
            break;

            num_nexthop ++;
            index = rtnh->rtnh_ifindex;
            gate = 0;
            if (rtnh->rtnh_len > sizeof (*rtnh))
            {
                sal_memset (tb, 0, sizeof (tb));
                _lib_netlink_parse_rtattr (tb, RTA_MAX, RTNH_DATA (rtnh), rtnh->rtnh_len - sizeof (*rtnh));
                if (tb[RTA_GATEWAY])
                    gate = RTA_DATA (tb[RTA_GATEWAY]);
            }
            
            if (gate)
            {

                if (g_nl_master->ops.ipv6_get_nh_key)
                {
                    rc = g_nl_master->ops.ipv6_get_nh_key(vrf_id, dest, gate, rtm->rtm_dst_len,
                                            index, num_nexthop, &nh_key);
                }
                if(rc)
                {
                    num_nexthop --; /*back to privious one*/
                }
                else
                {
                    sal_memcpy(&nh_group[num_nexthop - 1], &nh_key, sizeof(nh_key));
                }
            }
            else
            {
                //log_sys(M_MOD_LIB, E_INFORMATIONAL,"route process func(gate false)");
            }

            len -= NLMSG_ALIGN(rtnh->rtnh_len);
            rtnh = RTNH_NEXT(rtnh);
        }

        if (g_nl_master->ops.ipv6_add_route_multipath)
        {
            if(num_nexthop)
            {
                g_nl_master->ops.ipv6_add_route_multipath(vrf_id, dest, nh_group, num_nexthop,
                                    rtm->rtm_dst_len, distance);
            }
        }
    }
    else if (RTM_DELROUTE == h->nlmsg_type)
    {
        if (g_nl_master->ops.ipv6_del_route_multipath_check)
        {
            rc = g_nl_master->ops.ipv6_del_route_multipath_check(vrf_id, dest, nh_group, num_nexthop,
                                    rtm->rtm_dst_len, distance);
            if (rc) {
                return PM_E_NONE;
            }
        }
        
        for (;;)
        {
            if (len < (int) sizeof (*rtnh) || rtnh->rtnh_len > len)
            break;

            num_nexthop ++;
            index = rtnh->rtnh_ifindex;
            gate = 0;
            if (rtnh->rtnh_len > sizeof (*rtnh))
            {
                sal_memset (tb, 0, sizeof (tb));
                _lib_netlink_parse_rtattr (tb, RTA_MAX, RTNH_DATA (rtnh), rtnh->rtnh_len - sizeof (*rtnh));
                if (tb[RTA_GATEWAY])
                    gate = RTA_DATA (tb[RTA_GATEWAY]);
            }

            if (gate)
            {
                if (g_nl_master->ops.ipv6_get_nh_key)
                {
                    g_nl_master->ops.ipv6_get_nh_key(vrf_id, dest, gate, rtm->rtm_dst_len,
                                            index, num_nexthop, &nh_key);
                }
                sal_memcpy(&nh_group[num_nexthop - 1], &nh_key, sizeof(nh_key));
            }
            else
            {
                sal_printf("route process fuc(gate false)");
            }

            len -= NLMSG_ALIGN(rtnh->rtnh_len);
            rtnh = RTNH_NEXT(rtnh);
        }

        if (g_nl_master->ops.ipv6_del_route_multipath)
        {
            g_nl_master->ops.ipv6_del_route_multipath(vrf_id, dest, nh_group, num_nexthop,
                                    rtm->rtm_dst_len, distance);
        }
    }
    
    return PM_E_NONE;
}

int32
netlink_route_process(struct sal_sockaddr_nl *snl, struct nlmsghdr *h)
{
    struct rtmsg *rtm = NULL;
    struct rtattr *tb[RTA_MAX + 1];
    int32 len = 0;
    int32 ifindex = 0;
    uint32 metric = 0;
    uint32 route_type = 0;
    uint32 distance = 0;
    void *dest = NULL;
    void *gate = NULL;
    nexthop_key_t nh_group[IP_ECMP_MAX_PATH];
    nexthop_key_t nh_key;
    uint8 anyaddr[16] = {0};
    vrf_id_t vrf_id = 0;
    uint32 num_nexthop = 0;
    int32 rc = 0;

    rtm = NLMSG_DATA(h);
    len = h->nlmsg_len - NLMSG_LENGTH(sizeof (struct rtmsg));
    if (len < 0)
    {
        return PM_E_FAIL;
    }

    MY_PRINTF("RTM_%sROUTE %s %s proto %s len %d", RTM_NEWROUTE == h->nlmsg_type ? "NEW" : "DEL",
	       rtm->rtm_family == AF_INET ? "ipv4" : "ipv6",
	       rtm->rtm_type == RTN_UNICAST ? "unicast" : "multicast",
	       lookup (rtproto_str, rtm->rtm_protocol), len);

    if (RTN_BROADCAST == rtm->rtm_type || RTN_ANYCAST == rtm->rtm_type )
    {
        return PM_E_NONE;
    }

    /* Flag check. */
    if (rtm->rtm_flags & RTM_F_CLONED)
    {
        return PM_E_NONE;
    }

    vrf_id = lib_netlink_rtm_get_vrf_id(rtm);

    sal_memset(tb, 0, sizeof tb);
    _lib_netlink_parse_rtattr(tb, RTA_MAX, RTM_RTA(rtm), len);

    if (tb[RTA_OIF])
    {
        ifindex = *(int*)RTA_DATA(tb[RTA_OIF]);
    }

    if (tb[RTA_DST])
    {
        dest = RTA_DATA (tb[RTA_DST]);
    }
    else
    {
        dest = anyaddr;
    }

    /* Distance.  */
    if (tb[RTA_PRIORITY])
    {
        distance = *(uint32*)RTA_DATA(tb[RTA_PRIORITY]);
    }

    /* Metric.  */
    if (tb[RTA_METRICS])
    {
        metric = *(uint32*)RTA_DATA(tb[RTA_METRICS]);
    }

    /* route type.  */
    if (tb[RTA_ROUTE_TYPE])
    {
        route_type = *(uint32*)RTA_DATA(tb[RTA_ROUTE_TYPE]);
    }

    if (tb[RTA_GATEWAY])
    {
        gate = RTA_DATA(tb[RTA_GATEWAY]);
    }

    if (AF_INET6 == rtm->rtm_family)
    {
        #ifdef USE_FPM_IPUC
        rtm = NLMSG_DATA(h);
        #endif
        return netlink_route_process_ipv6(vrf_id, snl, h, rtm, dest, gate, ifindex, distance);
    }
    else if (AF_INET == rtm->rtm_family)
    {
        int32 index = 0;

        struct rtnexthop *rtnh =
                (struct rtnexthop *) RTA_DATA (tb[RTA_MULTIPATH]);


        if (RTM_NEWROUTE == h->nlmsg_type)
        {
            if (g_nl_master->ops.ipv4_add_route && !tb[RTA_MULTIPATH])
            {
                g_nl_master->ops.ipv4_add_route(vrf_id, dest, gate, rtm->rtm_dst_len, ifindex, distance, rtm->rtm_type, route_type);
            }
        }
        else
        {
            if (g_nl_master->ops.ipv4_del_route && !tb[RTA_MULTIPATH])
            {
                g_nl_master->ops.ipv4_del_route(vrf_id, dest, gate, rtm->rtm_dst_len, ifindex, distance, rtm->rtm_type, route_type);
            }
        }

        if (!tb[RTA_MULTIPATH])
        {
            return PM_E_NONE;
        }

        len = RTA_PAYLOAD (tb[RTA_MULTIPATH]);
        sal_memset(nh_group, 0, sizeof(nh_group));
        sal_memset(&nh_key, 0, sizeof(nh_key));
        if (RTM_NEWROUTE == h->nlmsg_type)
        {
            for (;;)
            {
                if (len < (int) sizeof (*rtnh) || rtnh->rtnh_len > len)
                break;

                num_nexthop ++;
                index = rtnh->rtnh_ifindex;
                gate = 0;
                if (rtnh->rtnh_len > sizeof (*rtnh))
                {
                    sal_memset (tb, 0, sizeof (tb));
                    _lib_netlink_parse_rtattr (tb, RTA_MAX, RTNH_DATA (rtnh), rtnh->rtnh_len - sizeof (*rtnh));
                    if (tb[RTA_GATEWAY])
                        gate = RTA_DATA (tb[RTA_GATEWAY]);
                }
                
                if (gate)
                {

                    if (g_nl_master->ops.ipv4_get_nh_key)
                    {
                        rc = g_nl_master->ops.ipv4_get_nh_key(vrf_id, dest, gate, rtm->rtm_dst_len,
                                                index, num_nexthop, &nh_key);
                    }
                    if(rc)
                    {
                        num_nexthop --; /*back to privious one*/
                    }
                    else
                    {
                        sal_memcpy(&nh_group[num_nexthop - 1], &nh_key, sizeof(nh_key));
                    }
                }
                else
                {
                    //log_sys(M_MOD_LIB, E_INFORMATIONAL,"route process func(gate false)");
                }

                len -= NLMSG_ALIGN(rtnh->rtnh_len);
                rtnh = RTNH_NEXT(rtnh);
            }

            if (g_nl_master->ops.ipv4_add_route_multipath)
            {
                if(num_nexthop)
                {
                    g_nl_master->ops.ipv4_add_route_multipath(vrf_id, dest, nh_group, num_nexthop,
                                        rtm->rtm_dst_len, distance, route_type);
                }
            }
        }
        else if (RTM_DELROUTE == h->nlmsg_type)
        {
            if (g_nl_master->ops.ipv4_del_route_multipath_check)
            {
                rc = g_nl_master->ops.ipv4_del_route_multipath_check(vrf_id, dest, nh_group, num_nexthop,
                                        rtm->rtm_dst_len, distance, route_type);
                if (rc) {
                    return PM_E_NONE;
                }
            }
            
            for (;;)
            {
                if (len < (int) sizeof (*rtnh) || rtnh->rtnh_len > len)
                break;

                num_nexthop ++;
                index = rtnh->rtnh_ifindex;
                gate = 0;
                if (rtnh->rtnh_len > sizeof (*rtnh))
                {
                    sal_memset (tb, 0, sizeof (tb));
                    _lib_netlink_parse_rtattr (tb, RTA_MAX, RTNH_DATA (rtnh), rtnh->rtnh_len - sizeof (*rtnh));
                    if (tb[RTA_GATEWAY])
                        gate = RTA_DATA (tb[RTA_GATEWAY]);
                }

                if (gate)
                {
                    if (g_nl_master->ops.ipv4_get_nh_key)
                    {
                        g_nl_master->ops.ipv4_get_nh_key(vrf_id, dest, gate, rtm->rtm_dst_len,
                                                index, num_nexthop, &nh_key);
                    }
                    sal_memcpy(&nh_group[num_nexthop - 1], &nh_key, sizeof(nh_key));
                }
                else
                    sal_printf("route process fuc(gate false)");

                len -= NLMSG_ALIGN(rtnh->rtnh_len);
                rtnh = RTNH_NEXT(rtnh);
            }

            if (g_nl_master->ops.ipv4_del_route_multipath)
            {
                g_nl_master->ops.ipv4_del_route_multipath(vrf_id, dest, nh_group, num_nexthop,
                                        rtm->rtm_dst_len, distance, route_type);
            }
        }
    }
    else if (RTNL_FAMILY_IPMR == rtm->rtm_family)
    {
        return netlink_mroute_process(vrf_id, snl, h);
    }

    (void)metric;
    return PM_E_NONE;
}

int32
netlink_neigh_add_process(struct sal_sockaddr_nl *snl, struct nlmsghdr *h)
{
#if 0
    struct ndmsg *ndm = NULL;
    struct rtattr *tb[RTN_MAX + 1];
    int32 len = 0;
    void *dst = NULL;
    void *lladdr = NULL;
    uint32 *probes = NULL;

    ndm = NLMSG_DATA(h);
    len = h->nlmsg_len - NLMSG_LENGTH(sizeof (struct ndmsg));
    if (len < 0)
    {
        return PM_E_FAIL;
    }

    /* for close the kernel arp */
    return PM_E_NONE;

    sal_memset(tb, 0, sizeof tb);
    _lib_netlink_parse_rtattr(tb, RTN_MAX, RTM_RTA(ndm), len);

    if (tb[NDA_DST])
    {
        dst = RTA_DATA(tb[NDA_DST]);
    }
    if (tb[NDA_LLADDR])
    {
        lladdr = RTA_DATA(tb[NDA_LLADDR]);
    }
    if (tb[NDA_PROBES])
    {
        probes = RTA_DATA(tb[NDA_PROBES]);
    }

    if (AF_INET == ndm->ndm_family)
    {
        if (GLB_FLAG_ISSET(ndm->ndm_state, NUD_INCOMPLETE))
        {
            if (g_nl_master->ops.ipv4_add_neigh)
            {
                g_nl_master->ops.ipv4_add_imcompete_neigh(GLB_DEF_VRFID, dst, NULL, ndm->ndm_ifindex, FALSE);
            }
        }
        else if (GLB_FLAG_ISSET(ndm->ndm_state, NUD_PERMANENT))
        {
            if (g_nl_master->ops.ipv4_add_neigh)
            {
                g_nl_master->ops.ipv4_add_neigh(GLB_DEF_VRFID, dst, lladdr, ndm->ndm_ifindex, TRUE);
            }
        }
        else if (GLB_FLAG_ISSET(ndm->ndm_state, NUD_REACHABLE))
        {
            if (g_nl_master->ops.ipv4_add_neigh)
            {
                g_nl_master->ops.ipv4_add_neigh(GLB_DEF_VRFID, dst, lladdr, ndm->ndm_ifindex, FALSE);
            }
        }
        else if (GLB_FLAG_ISSET(ndm->ndm_state, NUD_STALE))
        {
            if (g_nl_master->ops.ipv4_add_neigh)
            {
                g_nl_master->ops.ipv4_add_neigh(GLB_DEF_VRFID, dst, lladdr, ndm->ndm_ifindex, FALSE);
            }
        }
        else if (GLB_FLAG_ISSET(ndm->ndm_state, NUD_FAILED))
        {
            if (g_nl_master->ops.ipv4_add_neigh)
            {
                g_nl_master->ops.ipv4_add_imcompete_neigh(GLB_DEF_VRFID, dst, NULL, ndm->ndm_ifindex, FALSE);
            }
        }
    }
#endif
    return PM_E_NONE;
}

int32
netlink_neigh_del_process(struct sal_sockaddr_nl *snl, struct nlmsghdr *h)
{
#if 0
    struct ndmsg *ndm = NULL;
    struct rtattr *tb[RTN_MAX + 1];
    int32 len = 0;
    void *dst = NULL;
    void *lladdr = NULL;
    uint32 *probes = NULL;

    ndm = NLMSG_DATA(h);
    len = h->nlmsg_len - NLMSG_LENGTH(sizeof (struct ndmsg));
    if (len < 0)
    {
        return PM_E_FAIL;
    }

    /* for close the kernel arp */
    return PM_E_NONE;

    sal_memset(tb, 0, sizeof tb);
    _lib_netlink_parse_rtattr(tb, RTN_MAX, RTM_RTA(ndm), len);

    if (tb[NDA_DST])
    {
        dst = RTA_DATA(tb[NDA_DST]);
    }
    if (tb[NDA_LLADDR])
    {
        lladdr = RTA_DATA(tb[NDA_LLADDR]);
    }
    if (tb[NDA_PROBES])
    {
        probes = RTA_DATA(tb[NDA_PROBES]);
    }

    if (AF_INET == ndm->ndm_family)
    {
        if (GLB_FLAG_ISSET(ndm->ndm_state, NUD_PERMANENT))
        {
            if (g_nl_master->ops.ipv4_del_neigh)
            {
                g_nl_master->ops.ipv4_del_neigh(GLB_DEF_VRFID, dst, FALSE);
            }
        }
        else
        {
            if (g_nl_master->ops.ipv4_del_neigh)
            {
                g_nl_master->ops.ipv4_del_neigh(GLB_DEF_VRFID, dst, TRUE);
            }
        }
    }
#endif
    return PM_E_NONE;
}

int32
netlink_neigh_get_process(struct sal_sockaddr_nl *snl, struct nlmsghdr *h)
{
    struct ndmsg *ndm = NULL;
    struct rtattr *tb[RTN_MAX + 1];
    int32 len = 0;
    void *dst = NULL;
    void *lladdr = NULL;
    uint32 *probes = NULL;
    //char buf[64];

    ndm = NLMSG_DATA(h);
    len = h->nlmsg_len - NLMSG_LENGTH(sizeof (struct ndmsg));
    if (len < 0)
    {
        return PM_E_FAIL;
    }

    sal_memset(tb, 0, sizeof tb);
    _lib_netlink_parse_rtattr(tb, RTN_MAX, RTM_RTA(ndm), len);

    if (tb[NDA_DST])
    {
        dst = RTA_DATA(tb[NDA_DST]);
    }
    if (tb[NDA_LLADDR])
    {
        lladdr = RTA_DATA(tb[NDA_LLADDR]);
    }
    if (tb[NDA_PROBES])
    {
        probes = RTA_DATA(tb[NDA_PROBES]);
    }

    if (AF_INET == ndm->ndm_family)
    {
        //log_sys(M_MOD_NETLINK, E_ERROR, "netlink_neigh_get_process destip %s, ndm_state 0x%02x",
          //  sal_inet_ntop(AF_INET, dst, buf, sizeof(buf)), ndm->ndm_state);

        if (!(ndm->ndm_state & (NUD_PROBE|NUD_INCOMPLETE)))
        {
            return PM_E_NONE;
        }

        /* trigger arp miss */
        if (g_nl_master->ops.ipv4_get_neigh)
        {
            g_nl_master->ops.ipv4_get_neigh(dst, ndm->ndm_ifindex);
        }
    }
    /* support nd modified by liwh for bug 47547, 2018-07-04 */
    else if (AF_INET6 == ndm->ndm_family)
    {
        if (g_nl_master->ops.ipv6_get_neigh)
        {
            g_nl_master->ops.ipv6_get_neigh(dst, ndm->ndm_ifindex);
        }
    }
    /* liwh end */

    (void)lladdr;
    (void)probes;
    return PM_E_NONE;
}

char * netlink_msg_type_desc(uint32 type)
{
    switch (type)
    {
    case RTM_NEWLINK:
        return "RTM_NEWLINK";
    case RTM_DELLINK:
        return "RTM_DELLINK";
    case RTM_NEWADDR:
        return "RTM_NEWADDR";
    case RTM_DELADDR:
        return "RTM_DELADDR";
    case RTM_NEWROUTE:
        return "RTM_NEWROUTE";
    case RTM_DELROUTE:
        return "RTM_DELROUTE";
    case RTM_NEWNEIGH:
        return "RTM_NEWNEIGH";
    case RTM_GETNEIGH:
        return "RTM_GETNEIGH";
    case RTM_DELNEIGH:
        return "RTM_DELNEIGH";
    default:
        return "unknown netlink msg type";
    }
}
static int32
_lib_netlink_dispatch(struct sal_sockaddr_nl *snl, struct nlmsghdr *h)
{
    int32 rc = PM_E_NONE;

    g_netlink_dispatch_count++;
    LIB_NL_LOG("receive netlink msg %u, count %u", h->nlmsg_type, g_netlink_dispatch_count);
    MY_PRINTF("netlink rx %s", netlink_msg_type_desc(h->nlmsg_type));
    switch (h->nlmsg_type)
    {
    case RTM_NEWLINK:
    case RTM_DELLINK:
        return netlink_link_change(snl, h);
        break;
    case RTM_NEWADDR:
        return netlink_addr_process(snl, h);
        break;
    case RTM_DELADDR:
        return netlink_addr_process(snl, h);
        break;
    case RTM_NEWROUTE:
#ifndef USE_FPM_IPUC
        return netlink_route_process(snl, h);
#endif
        break;
    case RTM_DELROUTE:
#ifndef USE_FPM_IPUC
        return netlink_route_process(snl, h);
#endif
        break;
    case RTM_NEWNEIGH:
        rc = netlink_neigh_add_process(snl, h);
        break;
    case RTM_GETNEIGH:
        rc =  netlink_neigh_get_process(snl, h);
        break;
    case RTM_DELNEIGH:
        rc = netlink_neigh_del_process(snl, h);
        break;
    default:
        LIB_NL_LOG_ERROR("receive unhandled netlink msg %u\n", h->nlmsg_type);
        break;
    }
    (void)rc;
    return PM_E_NONE;
}

static void
_lib_netlink_read(void* p_data)
{
    lib_nl_sock_t *nl = (lib_nl_sock_t*)p_data;
    int32 rc = PM_E_NONE;

    rc = _lib_netlink_parse(_lib_netlink_dispatch, nl);
    (void)rc;
    return;
}

static int32
_lib_netlink_socket(lib_nl_sock_t *nl, uint32 groups, uint32 non_block)
{
    struct sal_sockaddr_nl snl;
    socklen_t namelen;
    int32 sock = -1;
    int32 rc = PM_E_NONE;

    sock = sal_socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (sock < 0)
    {
        return PM_E_FAIL;
    }

    if (non_block)
    {
        sal_sock_set_nonblocking(sock, TRUE);
    }

    sal_memset(&snl, 0, sizeof snl);
    snl.nl_family = AF_NETLINK;
    snl.nl_groups = groups;

    /* Bind the socket to the netlink structure for anything. */
    rc = sal_bind(sock, (struct sal_sockaddr*)&snl, sizeof snl);
    if (rc < 0)
    {
        return PM_E_FAIL;
    }

    namelen = sizeof snl;
    rc = sal_getsockname(sock, (struct sal_sockaddr*)&snl, &namelen);
    if (rc < 0 || namelen != sizeof snl)
    {
        return PM_E_FAIL;
    }

    nl->snl = snl;
    nl->sock = sock;
    return rc;
}

/* Utility function  comes from iproute2.
   Authors: Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru> */
static int32
_lib_netlink_addattr_l(struct nlmsghdr *n, int32 maxlen, int32 type, void *data, int32 alen)
{
    int32 len = 0;
    struct rtattr *rta = NULL;

    len = RTA_LENGTH (alen);

    if (NLMSG_ALIGN (n->nlmsg_len) + len > maxlen)
    {
        return PM_E_FAIL;
    }

    rta = (struct rtattr *)(((char *) n) + NLMSG_ALIGN(n->nlmsg_len));
    rta->rta_type = type;
    rta->rta_len = len;
    sal_memcpy(RTA_DATA (rta), data, alen);
    n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + len;

    return PM_E_NONE;
}

static int32
_lib_netlink_addattr32(struct nlmsghdr *n, int maxlen, int type, int data)
{
    int len;
    struct rtattr *rta;

    len = RTA_LENGTH (4);

    if (NLMSG_ALIGN (n->nlmsg_len) + len > maxlen)
    {
        return PM_E_FAIL;
    }

    rta = (struct rtattr *) (((char *) n) + NLMSG_ALIGN(n->nlmsg_len));
    rta->rta_type = type;
    rta->rta_len = len;
    sal_memcpy(RTA_DATA (rta), &data, 4);
    n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + len;

    return PM_E_NONE;
}

int32
_lib_netlink_ack_getlink(struct nlmsghdr *nhmsg)
{
    struct ifinfomsg *ifinfo = NULL;

    ifinfo = (void *)NLMSG_DATA(nhmsg);

    return ifinfo->ifi_index;
}

static int32
_lib_netlink_talk_filter(struct sal_sockaddr_nl *nl, struct nlmsghdr *h)
{
    switch (h->nlmsg_type)
    {
    case RTM_NEWLINK:
        return _lib_netlink_ack_getlink(h);
    default:
        //LIB_NL_LOG_ERROR("Receive unhandled netlink msg %u for netlink talk\n", h->nlmsg_type);
        break;
    }

    return PM_E_NONE;
}

/* sendmsg() to netlink socket then recvmsg(). */
int32
_lib_netlink_talk(struct nlmsghdr *n, lib_nl_sock_t *nl)
{
    int32 status = 0;
    struct sockaddr_nl snl;
    struct iovec iov = { (void *) n, n->nlmsg_len };
    struct msghdr msg = { (void *) &snl, sizeof snl, &iov, 1, NULL, 0, 0 };
    int32 save_errno = 0;

    sal_memset (&snl, 0, sizeof snl);
    snl.nl_family = AF_NETLINK;

    n->nlmsg_seq = ++nl->seq;

    /* Request an acknowledgement by setting NLM_F_ACK */
    n->nlmsg_flags |= NLM_F_ACK;

#ifdef TODO
    if (IS_ZEBRA_DEBUG_KERNEL)
        LIB_NL_LOG("_lib_netlink_talk: %s type %s(%u), seq=%u", nl->name,
               lookup (nlmsg_str, n->nlmsg_type), n->nlmsg_type,
               n->nlmsg_seq);
#endif

    /* Send message to netlink interface. */
#ifdef TODO
    if (zserv_privs.change (ZPRIVS_RAISE))
    {
        LIB_NL_LOG_ERROR("Can't raise privileges");
    }
#endif
    status = sal_sendmsg(nl->sock, &msg, 0);
    save_errno = errno;
#ifdef TODO
    if (zserv_privs.change (ZPRIVS_LOWER))
    {
        LIB_NL_LOG_ERROR("Can't lower privileges");
    }
#endif
    if (status < 0)
    {
        LIB_NL_LOG_ERROR("_lib_netlink_talk sendmsg() error: %s", sal_strerror(save_errno));
        return -1;
    }

    /*
    * Get reply from netlink socket.
    * The reply should either be an acknowlegement or an error.
    */
    return _lib_netlink_parse(_lib_netlink_talk_filter, nl);
}

int32
netlink_rtmsg_fib_set(struct rtmsg *rtm, vrf_id_t vrf_id)
{
#ifdef HAVE_MULTIPLE_FIB
    rtm->rtm_vrf = vrf_id;
    rtm->rtm_table = RT_TABLE_MAIN;
#endif /* HAVE_MULTIPLE_FIB */

    return 0;
}

int32
kernel_del_local(prefix_t *p)
{
    MY_PRINTF("%x/%d", p->u.prefix4.s_addr, p->prefixlen);
    int bytelen;
    lib_nl_sock_t *nl = NULL;
    int ret;
    //uint32 brd;
    uint32 any;
    addr_ipv4_t netmask;

    struct
    {
        struct nlmsghdr n;
        struct rtmsg r;
        char buf[64];
    } req;

    memset (&req, 0, sizeof req);

    bytelen = (p->family == AF_INET ? 4 : 16);

    req.n.nlmsg_len = NLMSG_LENGTH (sizeof (struct rtmsg));
    req.n.nlmsg_flags = NLM_F_REQUEST;
    req.n.nlmsg_type = RTM_DELROUTE;
    req.r.rtm_family = p->family;
    req.r.rtm_type = RTN_LOCAL;
    //req.r.rtm_table = RT_TABLE_LOCAL;
    req.r.rtm_table = RT_TABLE_MAIN;
    req.r.rtm_scope = RT_SCOPE_NOWHERE;
    req.r.rtm_dst_len = IPV4_MAX_PREFIXLEN;

    if(p->family == AF_INET)
    {
        sal_masklen2ip (p->prefixlen, &netmask);
        //brd = p->u.prefix4.s_addr|~(netmask.s_addr);
        any = p->u.prefix4.s_addr & (netmask.s_addr);
    }

    if (p->family == AF_INET)
    {
        nl = g_nl_master->cmd;
    }
    else
    {
        return PM_E_NONE;
    }

    /* connected route */
    req.r.rtm_type = RTN_UNICAST;
    req.r.rtm_scope = RT_SCOPE_LINK;
    req.r.rtm_table = RT_TABLE_MAIN;
    req.r.rtm_dst_len = p->prefixlen;
    _lib_netlink_addattr_l (&req.n, sizeof req, RTA_PREFSRC, &p->u.prefix, bytelen);
    _lib_netlink_addattr_l (&req.n, sizeof req, RTA_DST, &any, bytelen);

    ret = _lib_netlink_talk (&req.n, nl);

    return ret;
}

int32
kernel_add_local(prefix_t *p, uint32 kernel_ifindex)
{
    MY_PRINTF("%x/%d", p->u.prefix4.s_addr, p->prefixlen);
    int bytelen;
    lib_nl_sock_t *nl;
    int ret;
    //uint32 brd;
    uint32 any;
    addr_ipv4_t netmask;

    struct
    {
        struct nlmsghdr n;
        struct rtmsg r;
        char buf[64];
    } req;

    memset (&req, 0, sizeof req);

    bytelen = (p->family == AF_INET ? 4 : 16);

    req.n.nlmsg_len = NLMSG_LENGTH (sizeof (struct rtmsg));
    req.n.nlmsg_flags = NLM_F_REQUEST|NLM_F_CREATE|NLM_F_EXCL;
    req.n.nlmsg_type = RTM_NEWROUTE;
    req.r.rtm_family = p->family;
    req.r.rtm_type = RTN_LOCAL;
    //req.r.rtm_table = RT_TABLE_LOCAL;
    req.r.rtm_table = RT_TABLE_MAIN;
    req.r.rtm_dst_len = IPV4_MAX_PREFIXLEN;

    _lib_netlink_addattr32(&req.n, sizeof req, RTA_OIF, kernel_ifindex);

    if (p->family == AF_INET)
    {
        nl = g_nl_master->cmd;
    }    
    else
    {
        return PM_E_NONE;
    }

    if(p->family == AF_INET)
    {
        sal_masklen2ip (p->prefixlen, &netmask);
        //brd = p->u.prefix4.s_addr|~(netmask.s_addr);
        any = p->u.prefix4.s_addr & (netmask.s_addr);
    }

    req.r.rtm_protocol = RTPROT_KERNEL;
    req.r.rtm_scope = RT_SCOPE_HOST;

    //_lib_netlink_addattr_l (&req.n, sizeof req, RTA_DST, &p->u.prefix, bytelen);
    //ret = netlink_talk (&req.n, nl, 1);

    /* re-create connected route */
    req.r.rtm_type = RTN_UNICAST;
    req.r.rtm_scope = RT_SCOPE_LINK;
    req.r.rtm_table = RT_TABLE_MAIN;
    req.r.rtm_dst_len = p->prefixlen;
    _lib_netlink_addattr_l (&req.n, sizeof req, RTA_PREFSRC, &p->u.prefix, bytelen);
    _lib_netlink_addattr_l (&req.n, sizeof req, RTA_DST, &any, bytelen);

    ret = _lib_netlink_talk (&req.n, nl);

    return ret;
}

static int32
_lib_netlink_interface(int cmd, int family, char *ifname, uint32 *ifindex)
{
    struct nlattr *pt_nlattr = NULL;
    char *pt_ifname = NULL;
    uint8 *pt_buf = NULL;
    int ret = 0;

    struct netlink_link_msg {
        struct nlmsghdr n;
        struct ifinfomsg ifinfo;
        uint8 buffer[NL_PKT_BUF_SIZE];
    } req;

    sal_memset(&req, 0, sizeof(struct netlink_link_msg));
    req.n.nlmsg_len = 0;
    req.n.nlmsg_type = RTM_GETLINK;
    req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    req.n.nlmsg_seq = 0;
    req.n.nlmsg_pid = 0;
    req.ifinfo.ifi_family = 0;
    req.ifinfo.ifi_type   = 0;
    req.ifinfo.ifi_index  = 0;
    req.ifinfo.ifi_flags  = 0;
    req.ifinfo.ifi_change = 0;
    req.n.nlmsg_len = sizeof(struct nlmsghdr) + sizeof(struct ifinfomsg);

    pt_buf = req.buffer;
    pt_nlattr = (void *)pt_buf;
    pt_nlattr->nla_type = IFLA_IFNAME;
    pt_nlattr->nla_len  = NLA_ALIGN(16 + sizeof(struct nlattr));
    pt_ifname = (void*)(pt_buf + NLA_ALIGN(sizeof(struct nlattr)));
    sal_strcpy(pt_ifname, ifname);
    pt_buf = pt_buf + NLA_ALIGN(16 + sizeof(struct nlattr));
    req.n.nlmsg_len += NLA_ALIGN(16 + sizeof(struct nlattr));
    *ifindex = _lib_netlink_talk(&req.n, g_nl_master->cmd);
    return ret;
}

static int32
_lib_netlink_neigh(int32 cmd, uint32 ifindex, int32 family, void *ip_addr, uint8 *ll_addr, uint32 lla_len)
{
    int32 bytelen = 0;

    struct {
        struct nlmsghdr     n;
        struct ndmsg        ndm;
        char            buf[256];
    } req;

    sal_memset(&req, 0, sizeof req);
    bytelen = (family == AF_INET ? 4 : 16);

    req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ndmsg));
    req.n.nlmsg_type = cmd;
    req.ndm.ndm_family = family;
    req.ndm.ndm_state = NUD_PERMANENT;
    req.ndm.ndm_ifindex = ifindex;

    if (RTM_NEWNEIGH == cmd)
    {
        req.n.nlmsg_flags = NLM_F_CREATE | NLM_F_REPLACE | NLM_F_REQUEST;
        req.ndm.ndm_type = RTN_UNICAST;
    }
    else
    {
        req.n.nlmsg_flags = NLM_F_REQUEST;
        req.ndm.ndm_type = RTN_UNSPEC;
    }

    _lib_netlink_addattr_l(&req.n, sizeof(req), NDA_DST, ip_addr, bytelen);
    if (RTM_NEWNEIGH == cmd)
    {
        _lib_netlink_addattr_l(&req.n, sizeof(req), NDA_LLADDR, ll_addr, lla_len);
    }

    /* Talk to netlink socket. */
    return _lib_netlink_talk(&req.n, g_nl_master->arp);
}

int32
lib_netlink_init_rt_arp()
{
    lib_nl_sock_t *nl = NULL;
    uint32 groups = 0;
    uint32 value = 0;
    int32 buf_size = 0;
    int32 rc = PM_E_NONE;

    groups = RTMGRP_LINK;
//    groups = RTMGRP_LINK | RTMGRP_IPV4_ROUTE | RTMGRP_IPV4_MROUTE | RTMGRP_IPV4_IFADDR | RTMGRP_NEIGH;
//#ifdef HAVE_IPV6
//    groups |= RTMGRP_IPV6_ROUTE | RTMGRP_IPV6_IFADDR;
//#endif /* HAVE_IPV6 */

    if (g_nl_master->arp != NULL)
    {
        return PM_E_NONE;
    }

    nl = XCALLOC(MEM_TEMP, sizeof(lib_nl_sock_t));
    if (NULL == nl)
    {
        return PM_E_NO_MEMORY;
    }

    /* Create netlink socket */
    rc = _lib_netlink_socket(nl, groups, 1);
    if (rc < 0)
    {
        XFREE(MEM_TEMP, nl);
        return rc;
    }

    sal_setsockopt(nl->sock, SOL_SOCKET, SO_VRF, &value, sizeof(value));

    buf_size = 0;
    lib_sock_get_buf(nl->sock, FALSE, &buf_size);
    buf_size *= 20;
    lib_sock_set_buf(nl->sock, FALSE, buf_size);

    nl->t_read = NULL;
    sal_snprintf(nl->name, NL_SOCK_NAME_LEN, "netlink-arp");

    g_nl_master->arp = nl;
    return PM_E_NONE;
}

int32
lib_netlink_init_rt_cmd()
{
    lib_nl_sock_t *nl = NULL;
    uint32 groups = 0;
    uint32 value = 0;
    int32 buf_size = 0;
    int32 rc = PM_E_NONE;

    //groups = RTMGRP_LINK;
    //groups = RTMGRP_LINK | RTMGRP_IPV4_ROUTE | RTMGRP_IPV4_MROUTE | RTMGRP_IPV4_IFADDR | RTMGRP_NEIGH;
    groups = RTMGRP_LINK | RTMGRP_IPV4_MROUTE;
//#ifdef HAVE_IPV6
//    groups |= RTMGRP_IPV6_ROUTE | RTMGRP_IPV6_IFADDR;
//#endif /* HAVE_IPV6 */
	LIB_NL_LOG_ERROR("zhw test: lib_netlink_init_rt_cmd start");	
    if (g_nl_master->cmd != NULL)
    {
    	LIB_NL_LOG_ERROR("zhw test: g_nl_master->cmd != NULL");	
        return PM_E_NONE;
    }

    nl = XCALLOC(MEM_TEMP, sizeof(lib_nl_sock_t));
    if (NULL == nl)
    {
        return PM_E_NO_MEMORY;
    }

    /* Create netlink socket */
    rc = _lib_netlink_socket(nl, groups, 1);
    if (rc < 0)
    {
        XFREE(MEM_TEMP, nl);
        return rc;
    }

    sal_setsockopt(nl->sock, SOL_SOCKET, SO_VRF, &value, sizeof(value));

    buf_size = 0;
    lib_sock_get_buf(nl->sock, FALSE, &buf_size);
    buf_size *= 20;
    lib_sock_set_buf(nl->sock, FALSE, buf_size);

    nl->t_read = NULL;
    sal_snprintf(nl->name, NL_SOCK_NAME_LEN, "netlink-cmd");

    g_nl_master->cmd = nl;
    return PM_E_NONE;
}

int32
lib_netlink_init_rt_listen(vrf_id_t vrf_id)
{
    lib_nl_sock_t *nl = NULL;
    uint32 groups = 0;
    uint32 value = 0;
    int32 buf_size = 0;
    int32 rc = PM_E_NONE;

    groups = RTMGRP_LINK | RTMGRP_IPV4_ROUTE | RTMGRP_IPV4_MROUTE | RTMGRP_IPV4_IFADDR | RTMGRP_NEIGH;
#ifdef HAVE_IPV6
    groups |= RTMGRP_IPV6_ROUTE | RTMGRP_IPV6_IFADDR;
#endif /* HAVE_IPV6 */

    if (vrf_id >= GLB_MAX_VRFID)
    {
        return PM_E_FAIL;
    }

    if (g_nl_master->listen[vrf_id] != NULL)
    {
        return PM_E_NONE;
    }

    nl = XCALLOC(MEM_TEMP, sizeof(lib_nl_sock_t));
    if (NULL == nl)
    {
        return PM_E_NO_MEMORY;
    }

    /* Create netlink socket */
    rc = _lib_netlink_socket(nl, groups, 1);
    if (rc < 0)
    {
        XFREE(MEM_TEMP, nl);
        return rc;
    }

    value = vrf_id;
    sal_setsockopt(nl->sock, SOL_SOCKET, SO_VRF, &value, sizeof(value));

    buf_size = 0;
    lib_sock_get_buf(nl->sock, FALSE, &buf_size);
    buf_size *= 20;
    lib_sock_set_buf(nl->sock, FALSE, buf_size);

    nl->t_read = ctc_task_add_read(TASK_PRI_NORMAL, _lib_netlink_read, nl, nl->sock);
    sal_snprintf(nl->name, NL_SOCK_NAME_LEN, "netlink-listen-%u", vrf_id);

    g_nl_master->listen[vrf_id] = nl;
    return PM_E_NONE;
}

int
addattr_l (struct nlmsghdr *n, size_t maxlen, int type, void *data, int alen)
{
  size_t len;
  struct rtattr *rta;

  len = RTA_LENGTH (alen);

  if (NLMSG_ALIGN (n->nlmsg_len) + len > maxlen)
    return -1;

  rta = (struct rtattr *) (((char *) n) + NLMSG_ALIGN (n->nlmsg_len));
  rta->rta_type = type;
  rta->rta_len = len;
  memcpy (RTA_DATA (rta), data, alen);
  n->nlmsg_len = NLMSG_ALIGN (n->nlmsg_len) + len;

  return 0;
}

int
addattr32 (struct nlmsghdr *n, size_t maxlen, int type, int data)
{
  size_t len;
  struct rtattr *rta;

  len = RTA_LENGTH (4);

  if (NLMSG_ALIGN (n->nlmsg_len) + len > maxlen)
    return -1;

  rta = (struct rtattr *) (((char *) n) + NLMSG_ALIGN (n->nlmsg_len));
  rta->rta_type = type;
  rta->rta_len = len;
  memcpy (RTA_DATA (rta), &data, 4);
  n->nlmsg_len = NLMSG_ALIGN (n->nlmsg_len) + len;

  return 0;
}

struct prefix
{
  u_char family;
  u_char prefixlen;
  union
  {
    u_char prefix;
    struct in_addr prefix4;
#ifdef HAVE_IPV6
    struct in6_addr prefix6;
#endif /* HAVE_IPV6 */
    struct
    {
      struct in_addr id;
      struct in_addr adv_router;
    } lp;
    u_char val[8];
    uintptr_t ptr;
  } u __attribute__ ((aligned (8)));
};

int
host_address_kernel_add_local (addr_ipv4_t *p, uint32_t ifindex, uint8 prefixlen)
{
    int bytelen;
    struct prefix p_if;

    struct
    {
        struct nlmsghdr n;
        struct rtmsg r;
        char buf[NL_PKT_BUF_SIZE];
    } req;

    memset (&req, 0, sizeof req);
    bytelen = 4;

    req.n.nlmsg_len = NLMSG_LENGTH (sizeof (struct rtmsg));
    req.n.nlmsg_flags = NLM_F_CREATE | NLM_F_REQUEST | NLM_F_REPLACE;
    req.n.nlmsg_type = RTM_NEWROUTE;
    req.r.rtm_family = AF_INET;
    req.r.rtm_table = 0;
    req.r.rtm_dst_len = prefixlen;
    req.r.rtm_type = RTN_UNICAST;
    req.r.rtm_scope = RT_SCOPE_HOST;
    req.r.rtm_protocol = RTPROT_UNSPEC;
    //req.r.rtm_protocol = RTPROT_STATIC;

    p_if.u.prefix4.s_addr = p->s_addr;
    addattr32 (&req.n, sizeof req, RTA_OIF, ifindex);
    addattr_l (&req.n, sizeof req, RTA_DST, &p_if.u.prefix, bytelen);

    /* Talk to netlink socket. */
    return _lib_netlink_talk(&req.n, g_nl_master->cmd);
}

int
host_address_kernel_del_local (addr_ipv4_t *p, uint32_t ifindex, uint8 prefixlen)
{
    int bytelen;
    struct prefix p_if;

    struct
    {
        struct nlmsghdr n;
        struct rtmsg r;
        char buf[NL_PKT_BUF_SIZE];
    } req;

    sal_memset (&req, 0, sizeof req);
    bytelen = 4;

    req.n.nlmsg_len = NLMSG_LENGTH (sizeof (struct rtmsg));
    //req.n.nlmsg_flags = NLM_F_REQUEST;
    req.n.nlmsg_type = RTM_DELROUTE;
    req.r.rtm_family = AF_INET;
    req.r.rtm_table = 0;
    req.r.rtm_dst_len = prefixlen;
    req.r.rtm_type = RTN_UNICAST;
    req.r.rtm_scope = RT_SCOPE_HOST;
    req.r.rtm_protocol = RTPROT_UNSPEC;

    p_if.u.prefix4.s_addr = p->s_addr;
    addattr32 (&req.n, sizeof req, RTA_OIF, ifindex);
    addattr_l (&req.n, sizeof req, RTA_DST, &p_if.u.prefix, bytelen);

    /* Talk to netlink socket. */
    return _lib_netlink_talk (&req.n, g_nl_master->cmd);
}

#include "lib_ioctl.h"
#include "gen/tbl_kernel_if_define.h"
#include "gen/tbl_kernel_if.h"

int32
lib_netlink_link_set(uint32_t ifindex, uint32 is_up)
{
    uint8  oper;
    struct {
        struct nlmsghdr     n;
        struct ifinfomsg    ifm;
        char            buf[256];
    } req;

    sal_memset(&req, 0, sizeof(req));

    req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
    req.n.nlmsg_flags = NLM_F_REQUEST;
    req.n.nlmsg_type = RTM_SETLINK;
    req.ifm.ifi_index = ifindex;
    req.ifm.ifi_family = AF_UNSPEC;
    req.ifm.ifi_flags = 0;

    tbl_kernel_if_t *p_db_kernel_if = NULL;
    tbl_kernel_if_t kernel_if;
    uint32          ifi_flags = 0;

    sal_memset(&kernel_if, 0, sizeof(kernel_if));
    kernel_if.key.ifindex = ifindex;
    p_db_kernel_if = tbl_kernel_if_get_kernel_if(&kernel_if.key);
    if (NULL != p_db_kernel_if)
    {
        lib_ioctl_if_set_up_running(p_db_kernel_if->name, is_up, &ifi_flags);
        req.ifm.ifi_flags = ifi_flags;

/* added by liwh for bug 54457, 2019-11-21 */
#ifdef TSINGMA
        if (is_up)
        {
            lib_ioctl_if_set_running(p_db_kernel_if->name, is_up);
            usleep(200000);
        }
#endif
/* ended by liwh */
    }

    req.ifm.ifi_change = 0;
    oper = (is_up) ? 6 : 5;                 /* IF_OPER_DORMANT (5), IF_OPER_UP (6) IF_OPER_LOWERLAYERDOWN (3) */
    addattr_l(&req.n, sizeof(req), 16, &oper, sizeof(oper)); /* IFLA_OPERSTATE (16)*/

    /* Talk to netlink socket. */
    return _lib_netlink_talk(&req.n, g_nl_master->cmd);
}

int32
lib_netlink_register_add_newlink(RT_INTERFACE_FUNC fn)
{
    g_nl_master->ops.add_interface = fn;
    return PM_E_NONE;
}

int32
lib_netlink_register_del_newlink(RT_INTERFACE_FUNC fn)
{
    g_nl_master->ops.del_interface = fn;
    return PM_E_NONE;
}

int32
lib_netlink_register_ipv4_add_addr(RT_IPV4_ADDR_FUNC fn)
{
    g_nl_master->ops.ipv4_add_addr = fn;
    return PM_E_NONE;
}

int32
lib_netlink_register_ipv4_del_addr(RT_IPV4_ADDR_FUNC fn)
{
    g_nl_master->ops.ipv4_del_addr = fn;
    return PM_E_NONE;
}

int32
lib_netlink_register_ipv4_add_route(RT_IPV4_ROUTE_FUNC fn)
{
    g_nl_master->ops.ipv4_add_route = fn;
    return PM_E_NONE;
}

int32
lib_netlink_register_ipv4_del_route(RT_IPV4_ROUTE_FUNC fn)
{
    g_nl_master->ops.ipv4_del_route = fn;
    return PM_E_NONE;
}

int32
lib_netlink_register_ipv4_add_mroute(RT_IPV4_MROUTE_FUNC fn)
{
    g_nl_master->ops.ipv4_add_mroute = fn;
    return PM_E_NONE;
}

int32
lib_netlink_register_ipv4_del_mroute(RT_IPV4_MROUTE_FUNC fn)
{
    g_nl_master->ops.ipv4_del_mroute = fn;
    return PM_E_NONE;
}

int32
lib_netlink_register_ipv4_add_route_multipath_check(RT_IPV4_RT_MULTI_FUNC fn)
{
    g_nl_master->ops.ipv4_add_route_multipath_check = fn;
    return PM_E_NONE;
}

int32
lib_netlink_register_ipv4_del_route_multipath_check(RT_IPV4_RT_MULTI_FUNC fn)
{
    g_nl_master->ops.ipv4_del_route_multipath_check = fn;
    return PM_E_NONE;
}

int32
lib_netlink_register_ipv4_add_route_multipath(RT_IPV4_RT_MULTI_FUNC fn)
{
    g_nl_master->ops.ipv4_add_route_multipath = fn;
    return PM_E_NONE;
}

int32
lib_netlink_register_ipv4_del_route_multipath(RT_IPV4_RT_MULTI_FUNC fn)
{
    g_nl_master->ops.ipv4_del_route_multipath = fn;
    return PM_E_NONE;
}

int32
lib_netlink_register_ipv4_add_nh_group(RT_IPV4_NH_GROUP_FUNC fn)
{
    g_nl_master->ops.ipv4_add_nh_group = fn;
    return PM_E_NONE;
}

int32
lib_netlink_register_ipv4_get_nh_key(RT_IPV4_NH_GROUP_FUNC fn)
{
    g_nl_master->ops.ipv4_get_nh_key = fn;
    return PM_E_NONE;
}

int32
lib_netlink_register_ipv4_del_nh_group(RT_IPV4_NH_GROUP_FUNC fn)
{
    g_nl_master->ops.ipv4_del_nh_group = fn;
    return PM_E_NONE;
}

int32
lib_netlink_register_ipv4_add_neigh(RT_IPV4_ADD_NEIGH_FUNC fn)
{
    g_nl_master->ops.ipv4_add_neigh = fn;
    return PM_E_NONE;
}

int32
lib_netlink_register_ipv4_add_imcomplete_neigh(RT_IPV4_ADD_NEIGH_FUNC fn)
{
    g_nl_master->ops.ipv4_add_imcompete_neigh = fn;
    return PM_E_NONE;
}

int32
lib_netlink_register_ipv4_del_neigh(RT_IPV4_DEL_NEIGH_FUNC fn)
{
    g_nl_master->ops.ipv4_del_neigh = fn;
    return PM_E_NONE;
}

int32
lib_netlink_register_ipv4_get_neigh(RT_IPV4_GET_NEIGH_FUNC fn)
{
    g_nl_master->ops.ipv4_get_neigh = fn;
    return PM_E_NONE;
}

int32
lib_netlink_register_ipv6_get_neigh(RT_IPV6_GET_NEIGH_FUNC fn)
{
    g_nl_master->ops.ipv6_get_neigh = fn;
    return PM_E_NONE;
}

int32
lib_netlink_register_ipv6_add_addr(RT_IPV6_ADDR_FUNC fn)
{
    g_nl_master->ops.ipv6_add_addr = fn;
    return PM_E_NONE;
}

int32
lib_netlink_register_ipv6_del_addr(RT_IPV6_ADDR_FUNC fn)
{
    g_nl_master->ops.ipv6_del_addr = fn;
    return PM_E_NONE;
}

int32
lib_netlink_register_ipv6_add_route(RT_IPV6_ROUTE_FUNC fn)
{
    g_nl_master->ops.ipv6_add_route = fn;
    return PM_E_NONE;
}

int32
lib_netlink_register_ipv6_del_route(RT_IPV6_ROUTE_FUNC fn)
{
    g_nl_master->ops.ipv6_del_route = fn;
    return PM_E_NONE;
}

int32
lib_netlink_register_ipv6_add_route_multipath_check(RT_IPV6_RT_MULTI_FUNC fn)
{
    g_nl_master->ops.ipv6_add_route_multipath_check = fn;
    return PM_E_NONE;
}

int32
lib_netlink_register_ipv6_del_route_multipath_check(RT_IPV6_RT_MULTI_FUNC fn)
{
    g_nl_master->ops.ipv6_del_route_multipath_check = fn;
    return PM_E_NONE;
}

int32
lib_netlink_register_ipv6_add_route_multipath(RT_IPV6_RT_MULTI_FUNC fn)
{
    g_nl_master->ops.ipv6_add_route_multipath = fn;
    return PM_E_NONE;
}

int32
lib_netlink_register_ipv6_del_route_multipath(RT_IPV6_RT_MULTI_FUNC fn)
{
    g_nl_master->ops.ipv6_del_route_multipath = fn;
    return PM_E_NONE;
}

int32
lib_netlink_register_ipv6_get_nh_key(RT_IPV6_NH_GROUP_FUNC fn)
{
    g_nl_master->ops.ipv6_get_nh_key = fn;
    return PM_E_NONE;
}

int32
kernel_interface_get_ifindex(char *ifname, uint32 *ifindex)
{
    _lib_netlink_interface(RTM_GETLINK, AF_UNSPEC, ifname, ifindex);
    return PM_E_NONE;
}

int32
kernel_neigh_entry_add(addr_ipv4_t *ip_addr, uint8 *mac_addr, uint32 ifindex, uint8 is_proxy_arp)
{
    return _lib_netlink_neigh(RTM_NEWNEIGH, ifindex, AF_INET, ip_addr, mac_addr, MAC_ADDR_LEN);
}

int32
kernel_neigh_entry_del(addr_ipv4_t *ip_addr, uint8 *mac_addr, uint32 ifindex)
{
    return _lib_netlink_neigh(RTM_DELNEIGH, ifindex, AF_INET, ip_addr, mac_addr, MAC_ADDR_LEN);
}

/* support nd modified by liwh for bug 47547, 2018-06-28 */
int32
kernel_ipv6_neigh_entry_add(addr_ipv6_t *ip_addr, uint8 *mac_addr, uint32 ifindex)
{
    return _lib_netlink_neigh(RTM_NEWNEIGH, ifindex, AF_INET6, ip_addr, mac_addr, MAC_ADDR_LEN);
}

int32
kernel_ipv6_neigh_entry_del(addr_ipv6_t *ip_addr, uint8 *mac_addr, uint32 ifindex)
{
    return _lib_netlink_neigh(RTM_DELNEIGH, ifindex, AF_INET6, ip_addr, mac_addr, MAC_ADDR_LEN);
}
/* liwh end */

int32
lib_netlink_init()
{
    int32 rc = PM_E_NONE;

    g_nl_master = XCALLOC(MEM_PM_LIB_MASTER, sizeof(lib_nl_master_t));
    if (NULL == g_nl_master)
    {
        return PM_E_NO_MEMORY;
    }

    PM_E_RETURN(lib_netlink_init_rt_arp());
    /* Initialize cmd sock */
    PM_E_RETURN(lib_netlink_init_rt_cmd());

    /* Initialize listen sock for default fib */
    PM_E_RETURN(lib_netlink_init_rt_listen(GLB_DEF_VRFID));

    return PM_E_NONE;
}

void
_lib_netlink_read_pimsm()
{
    lib_nl_sock_t *nl = g_nl_master->listen[PIM_VRF_ID];
    int32 rc = PM_E_NONE;

    rc = _lib_netlink_parse(_lib_netlink_dispatch, nl);
	if(rc){
        LIB_NL_LOG_ERROR("_lib_netlink_read exit out");		
	}
    return;
}

int32 lib_netlink_init_pimsm()
{
	lib_nl_sock_t *nl = NULL;
	uint32 groups = 0;
	uint32 value = 0;
	int32 buf_size = 0;
	int32 rc = PM_E_NONE;

	vrf_id_t vrf_id = PIM_VRF_ID;

	groups = RTMGRP_LINK | RTMGRP_IPV4_ROUTE | RTMGRP_IPV4_IFADDR ;

	if (vrf_id >= GLB_MAX_VRFID)
	{
		return PM_E_FAIL;
	}
	
    g_nl_master = (lib_nl_master_t*)malloc(sizeof(lib_nl_master_t));
    if (NULL == g_nl_master)
    {
        return PM_E_NO_MEMORY;
    }
	
	/*if(g_nl_master->listen[vrf_id] != NULL)
	{
		g_nl_master->listen[vrf_id] == NULL;
	}
	
	if (g_nl_master->listen[vrf_id] != NULL)
	{
		LIB_NL_LOG_ERROR("zhw test:g_nl_master->listen[vrf_id] != NULL");	  
		return PM_E_FAIL;
	}*/

	nl = (lib_nl_sock_t*)malloc( sizeof(lib_nl_sock_t));
	if (NULL == nl)
	{
		return PM_E_NO_MEMORY;
	}

	/* Create netlink socket */
	rc = _lib_netlink_socket(nl, groups, 1);
	if (rc < 0)
	{
		free( nl);
		return rc;
	}

	value = vrf_id;
	sal_setsockopt(nl->sock, SOL_SOCKET, SO_VRF, &value, sizeof(value));

	buf_size = 0;
	lib_sock_get_buf(nl->sock, FALSE, &buf_size);
	buf_size *= 20;
	lib_sock_set_buf(nl->sock, FALSE, buf_size);

	sal_snprintf(nl->name, NL_SOCK_NAME_LEN, "netlink-listen-%u", vrf_id);

	g_nl_master->listen[vrf_id] = nl;

	return PM_E_NONE;

}

#ifdef CTC_LIB_NETLINK_UNUSE

int32
kernel_route_add_ipv4(vrf_id_t vrf_id, prefix_t *p)
{
    if (ROUTE_TYPE_CONNECT == rib->type)
    {
        return PM_E_NONE;
    }

    return _lib_netlink_route(RTM_NEWROUTE, AF_INET, NLM_F_CREATE|NLM_F_EXCL, vrf_id, p);
}

int32
kernel_route_del_ipv4(vrf_id_t vrf_id, prefix_t *p)
{
    if (ROUTE_TYPE_CONNECT == rib->type)
    {
        return PM_E_NONE;
    }

    return _lib_netlink_route(RTM_DELROUTE, AF_INET, 0, vrf_id, p);
}

int32
kernel_route_update_ipv4(vrf_id_t vrf_id, prefix_t *p)
{
    int32 rc = PM_E_NONE;

    if (ROUTE_TYPE_CONNECT == select->type)
    {
        return PM_E_NONE;
    }

    rc = _lib_netlink_route(RTM_NEWROUTE, AF_INET, NLM_F_REPLACE, vrf_id, p);
    if (rc < 0)
    {
        rc = kernel_route_add_ipv4(vrf_id, p);
    }

    return rc;
}

static int32
netlink_set_running(struct rtattr *rta, int maxlen, int type, void *data, int alen)
{
    int32 len;
    struct rtattr *subrta;

    len = RTA_LENGTH(alen);

    if (RTA_ALIGN(rta->rta_len) + len > maxlen)
    {
        return PM_E_FAIL;
    }

    subrta = (struct rtattr*) (((char*)rta) + RTA_ALIGN(rta->rta_len));
    subrta->rta_type = type;
    subrta->rta_len = len;
    sal_memcpy(RTA_DATA(subrta), data, alen);
    rta->rta_len = NLMSG_ALIGN(rta->rta_len) + len;

    return PM_E_NONE;
}

int
pal_if_set_oper_state (struct interface *ifp, bool_t enable)
{
    int ret;
    u8  oper;
    struct {
        struct nlmsghdr     n;
        struct ifinfomsg    ifm;
        char            buf[256];
    } req;

    if (!ifp || !(ifp->vrf))
        return -1;

    memset (&req, 0, sizeof req);

    req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
    req.n.nlmsg_flags = NLM_F_REQUEST;
    req.n.nlmsg_type = 19; /* RTM_SETLINK (19) */
    req.ifm.ifi_index = ifp->ifindex;
    req.ifm.ifi_family = AF_UNSPEC;
    req.ifm.ifi_flags = 0;
    req.ifm.ifi_change = 0;

    oper = (enable) ? 6 : 5; /* IF_OPER_DORMANT (5), IF_OPER_UP (6) */
    addattr_l (&req.n, sizeof(req), 16, &oper, sizeof(oper)); /* IFLA_OPERSTATE (16)*/
#ifdef _NEW_VERSION_LINUX_
    /* Talk to netlink socket. */
    if(0 != ifp->vrf->fib_id)
        pal_sock_set_bindtofib (netlink_cmd.sock, ifp->vrf->fib_id);
#endif
    ret = netlink_talk (&req.n, netlink_listen[ifp->vrf->fib_id], 1);
    if (ret < 0)
        return -1;

    return 0;
}


static int32
_lib_netlink_rta_addattr_l(struct rtattr *rta, int maxlen, int type, void *data, int alen)
{
    int32 len;
    struct rtattr *subrta;

    len = RTA_LENGTH(alen);

    if (RTA_ALIGN(rta->rta_len) + len > maxlen)
    {
        return PM_E_FAIL;
    }

    subrta = (struct rtattr*) (((char*)rta) + RTA_ALIGN(rta->rta_len));
    subrta->rta_type = type;
    subrta->rta_len = len;
    sal_memcpy(RTA_DATA(subrta), data, alen);
    rta->rta_len = NLMSG_ALIGN(rta->rta_len) + len;

    return PM_E_NONE;
}

/* Routing table change via netlink interface. */
static int32
_lib_netlink_route(int32 cmd, int32 family, uint16 flags, vrf_id_t vrf_id, prefix_t *p)
{
    lib_nl_sock_t *nl = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_nexthop_t *nexthop = NULL;
    int bytelen;
    int nexthop_num = 0;
    int discard = FALSE;
    int ret;

    struct
    {
        struct nlmsghdr n;
        struct rtmsg r;
        char buf[1024];
    } req;

    sal_memset(&req, 0, sizeof(req));

    bytelen = (family == AF_INET ? 4 : 16);

    req.n.nlmsg_len = NLMSG_LENGTH (sizeof(struct rtmsg));
    req.n.nlmsg_flags = NLM_F_REQUEST | flags;
    req.n.nlmsg_type = cmd;
    req.r.rtm_family = family;
    req.r.rtm_table = RT_TABLE_MAIN;
    req.r.rtm_dst_len = p->prefixlen;
    req.r.rtm_protocol = RTPROT_STATIC;
    req.r.rtm_scope = RT_SCOPE_UNIVERSE;
    if (rib->flags & RIB_FLAG_BLACKHOLE)
    {
        discard = TRUE;
    }
    else
    {
        discard = FALSE;
    }
    /* Set rtm FIB and Table ID. */
    netlink_rtmsg_fib_set(&req.r, vrf_id);
    req.r.rtm_dst_len = p->prefixlen;

    if (cmd == RTM_NEWROUTE)
    {
        if (discard)
        {
            req.r.rtm_type = RTN_BLACKHOLE;
        }
        else
        {
            req.r.rtm_type = RTN_UNICAST;
        }

        /* Reset the FIB flag first.  */
        CTCLIB_SLIST_LOOP(rib->nh.obj_list, nexthop, listnode)
        {
            GLB_UNSET_FLAG(nexthop->flags, NEXTHOP_FLAG_OS);
        }
    }
    else
    {
    }

    _lib_netlink_addattr_l(&req.n, sizeof req, RTA_DST, &p->u.prefix, bytelen);

#ifdef HAVE_UPDATE_FIB_METRIC
    /* Metric. */
    _lib_netlink_addattr32(&req.n, sizeof req, RTA_PRIORITY, rib->metric);
#endif /* HAVE_UPDATE_FIB_METRIC */

    if (discard)
    {
        if (cmd == RTM_NEWROUTE)
        {
            CTCLIB_SLIST_LOOP(rib->nh.obj_list, nexthop, listnode)
            {
                GLB_SET_FLAG(nexthop->flags, NEXTHOP_FLAG_OS);
            }
        }
        goto skip;
    }

    /* Multipath case. */
    if (rib->nh_active_num == 1)
    {
        CTCLIB_SLIST_LOOP(rib->nh.obj_list, nexthop, listnode)
        {
            if (GLB_FLAG_ISSET(nexthop->flags, NEXTHOP_FLAG_ACTIVE))
            {
                if (GLB_FLAG_ISSET(nexthop->flags, NEXTHOP_FLAG_RECURSIVE))
                {
                    if (nexthop->rgate.type == NH_TYPE_IPV4)
                    {
                        _lib_netlink_addattr_l(&req.n, sizeof req, RTA_GATEWAY, &nexthop->rgate.u.ipv4, bytelen);
                    }
#ifdef HAVE_IPV6
                    if (nexthop->rgate.type == NH_TYPE_IPV6)
                    {
                        _lib_netlink_addattr_l(&req.n, sizeof req, RTA_GATEWAY, &nexthop->rgate.u.ipv6, bytelen);
                    }
#endif /* HAVE_IPV6 */
                    if (nexthop->rgate.type == NH_TYPE_IFINDEX)
                    {
                        _lib_netlink_addattr32(&req.n, sizeof req, RTA_OIF, nexthop->rifindex);
                    }
                }
                else
                {
                    if (nexthop->key.type == NH_TYPE_IPV4)
                    {
                        _lib_netlink_addattr_l(&req.n, sizeof req, RTA_GATEWAY, &nexthop->key.u.ipv4, bytelen);
                    }
#ifdef HAVE_IPV6
                    if (nexthop->key.type == NH_TYPE_IPV6)
                    {
                        _lib_netlink_addattr_l(&req.n, sizeof req, RTA_GATEWAY, &nexthop->key.u.ipv6, bytelen);
                    }
#endif /* HAVE_IPV6 */
                    if (nexthop->key.type == NH_TYPE_IFINDEX)
                    {
                        _lib_netlink_addattr32(&req.n, sizeof req, RTA_OIF, nexthop->ifindex);
                    }
                }

                if (cmd == RTM_NEWROUTE)
                {
                    GLB_SET_FLAG(nexthop->flags, NEXTHOP_FLAG_OS);
                }
                else
                {
                    GLB_UNSET_FLAG(nexthop->flags, NEXTHOP_FLAG_OS);
                }
                nexthop_num++;
                break;
            }
        }
    }
    else
    {
        char buf[1024];
        struct rtattr *rta = (void *) buf;
        struct rtnexthop *rtnh;

        rta->rta_type = RTA_MULTIPATH;
        rta->rta_len = RTA_LENGTH(0);
        rtnh = RTA_DATA(rta);

        nexthop_num = 0;

        CTCLIB_SLIST_LOOP(rib->nh.obj_list, nexthop, listnode)
        {
            if ((cmd == RTM_NEWROUTE && GLB_FLAG_ISSET(nexthop->flags, NEXTHOP_FLAG_ACTIVE))
             || (cmd == RTM_DELROUTE && GLB_FLAG_ISSET(nexthop->flags, NEXTHOP_FLAG_OS)))
            {
                nexthop_num++;

                rtnh->rtnh_len = sizeof (*rtnh);
                rtnh->rtnh_flags = 0;
                rtnh->rtnh_hops = 0;
                rta->rta_len += rtnh->rtnh_len;

                if (GLB_FLAG_ISSET(nexthop->flags, NEXTHOP_FLAG_RECURSIVE))
                {
                    if (nexthop->rgate.type == NH_TYPE_IPV4)
                    {
                        _lib_netlink_rta_addattr_l(rta, 4096, RTA_GATEWAY, &nexthop->rgate.u.ipv4, bytelen);
                        rtnh->rtnh_len += sizeof (struct rtattr) + 4;
                    }
#ifdef HAVE_IPV6
                    if (nexthop->rgate.type == NH_TYPE_IPV6)
                    {
                        _lib_netlink_rta_addattr_l(rta, 4096, RTA_GATEWAY, &nexthop->rgate.u.ipv6, bytelen);
                    }
#endif /* HAVE_IPV6 */
                    /* ifindex */
                    if (nexthop->rgate.type == NH_TYPE_IFINDEX)
                    {
                        rtnh->rtnh_ifindex = nexthop->rifindex;
                    }
                    else
                    {
                        rtnh->rtnh_ifindex = 0;
                    }
                }
                else
                {
                    if (nexthop->key.type == NH_TYPE_IPV4)
                    {
                        _lib_netlink_rta_addattr_l(rta, 4096, RTA_GATEWAY, &nexthop->key.u.ipv4, bytelen);
                        rtnh->rtnh_len += sizeof (struct rtattr) + 4;
                    }
#ifdef HAVE_IPV6
                    if (nexthop->key.type == NH_TYPE_IPV6)
                    {
                        _lib_netlink_rta_addattr_l(rta, 4096, RTA_GATEWAY, &nexthop->key.u.ipv6, bytelen);
                    }
#endif /* HAVE_IPV6 */
                    /* ifindex */
                    if (nexthop->key.type == NH_TYPE_IFINDEX)
                    {
                        rtnh->rtnh_ifindex = nexthop->ifindex;
                    }
                    else
                    {
                        rtnh->rtnh_ifindex = 0;
                    }
                }
                rtnh = RTNH_NEXT(rtnh);

                if (cmd == RTM_NEWROUTE)
                {
                    GLB_SET_FLAG(nexthop->flags, NEXTHOP_FLAG_OS);
                }
                else
                {
                    GLB_UNSET_FLAG(nexthop->flags, NEXTHOP_FLAG_OS);
                }
            }
        }

        if (rta->rta_len > RTA_LENGTH (0))
        {
            _lib_netlink_addattr_l(&req.n, 1024, RTA_MULTIPATH, RTA_DATA(rta), RTA_PAYLOAD(rta));
        }
    }

skip:

    if (vrf_id)
    {
        nl = g_nl_master->listen[vrf_id];
    }
    else
    {
        nl = g_nl_master->cmd;
    }
    ret = _lib_netlink_talk(&req.n, nl);

    return ret;
}

#endif


