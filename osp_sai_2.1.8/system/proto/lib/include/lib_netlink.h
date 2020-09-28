
#ifndef __LIB_NETLINK_H__
#define __LIB_NETLINK_H__

typedef enum
{
    RTM_BASE        = 15,
    RTM_NEWLINK     = 16,
    RTM_DELLINK,
    RTM_GETLINK,
    RTM_SETLINK,

    RTM_NEWADDR     = 20,
    RTM_DELADDR,
    RTM_GETADDR,

    RTM_NEWROUTE    = 24,
    RTM_DELROUTE,
    RTM_GETROUTE,

    RTM_NEWNEIGH    = 28,
    RTM_DELNEIGH,
    RTM_GETNEIGH,

    RTM_NEWVRF      = 88,
    RTM_DELVRF,
    RTM_GETVRF,
} rtm_type_t;

struct rtnexthop
{
    uint16  rtnh_len;
    uint8   rtnh_flags;
    uint8   rtnh_hops;
    int32   rtnh_ifindex;
};

struct rtmsg
{
    uint8       rtm_family;
    uint8       rtm_dst_len;
    uint8       rtm_src_len;
    uint8       rtm_tos;

    uint8       rtm_table;  /* Routing table id */
    uint8       rtm_protocol;   /* Routing protocol; see below  */
    uint8       rtm_scope;  /* See below */
    uint8       rtm_type;   /* See below    */

    uint32        rtm_flags;
};

struct ndmsg {
    uint8        ndm_family;
    uint8        ndm_pad1;
    uint16       ndm_pad2;
    uint32       ndm_ifindex;
    uint16       ndm_state;
    uint8        ndm_flags;
    uint8        ndm_type;
};

#define NL_PKT_BUF_SIZE 8192

#define MAX_PATH_NUM        64

#define LIB_NL_LOG(fmt, args...) \
    log_sys(M_MOD_NETLINK, E_ERROR, fmt, ##args);

#define LIB_NL_LOG_ERROR(fmt, args...) \
    log_sys(M_MOD_NETLINK, E_ERROR, fmt, ##args);

typedef int32 (*RT_INTERFACE_FUNC)(char *ifname, uint32 ifindex);

typedef int32 (*RT_IPV4_ADDR_FUNC)(vrf_id_t vrf_id, addr_ipv4_t *addr, uint8 prefixlen, uint32 ifindex);

typedef int32 (*RT_IPV4_ROUTE_FUNC)(vrf_id_t vrf_id, addr_ipv4_t *dest, addr_ipv4_t *gate, uint8 prefixlen, 
                                    uint32 ifindex, uint32 distance, uint32 nexthop_type, uint32 route_type);

typedef int32 (*RT_IPV4_MROUTE_FUNC)(vrf_id_t vrf_id, addr_ipv4_t *source, addr_ipv4_t *group,
                                    uint32 iif, uint32 *oif, uint32 oif_count);

typedef int32 (*RT_IPV4_RT_MULTI_FUNC)(vrf_id_t vrf_id, addr_ipv4_t *dest, nexthop_key_t *nhg, 
                    uint32 multipath, uint8 prefixlen, uint32 distance, uint32 route_type);

typedef int32 (*RT_IPV4_NH_GROUP_FUNC)(vrf_id_t vrf_id, addr_ipv4_t *dest, addr_ipv4_t *gate, uint8 prefixlen, 
                  uint32 ifindex, uint32 multipath, nexthop_key_t *nh_key);

typedef int32 (*RT_IPV4_ADD_NEIGH_FUNC)(vrf_id_t vrf_id, addr_ipv4_t *dest, uint8 *macaddr, uint32 ifindex, uint8 is_static, char *if_name);

typedef int32 (*RT_IPV4_DEL_NEIGH_FUNC)(vrf_id_t vrf_id, addr_ipv4_t *dest, uint8 is_static);

typedef int32 (*RT_IPV4_GET_NEIGH_FUNC)(addr_ipv4_t *dest, uint32 kernel_ifindex);

typedef int32 (*RT_IPV6_ADDR_FUNC)(vrf_id_t vrf_id, addr_ipv6_t *addr, uint8 prefixlen, uint32 ifindex);

typedef int32 (*RT_IPV6_ROUTE_FUNC)(vrf_id_t vrf_id, addr_ipv6_t *dest, addr_ipv6_t *gate, uint8 prefixlen, 
                                    uint32 ifindex, uint32 distance, uint32 nexthop_type);

typedef int32 (*RT_IPV6_RT_MULTI_FUNC)(vrf_id_t vrf_id, addr_ipv6_t *dest, nexthop_key_t *nhg, 
                    uint32 multipath, uint8 prefixlen, uint32 distance);

typedef int32 (*RT_IPV6_NH_GROUP_FUNC)(vrf_id_t vrf_id, addr_ipv6_t *dest, addr_ipv6_t *gate, uint8 prefixlen, 
                  uint32 ifindex, uint32 multipath, nexthop_key_t *nh_key);

typedef int32 (*RT_IPV6_GET_NEIGH_FUNC)(addr_ipv6_t *dest, uint32 kernel_ifindex);

int
host_address_kernel_add_local (addr_ipv4_t *p, uint32_t ifindex, uint8 prefixlen);

int
host_address_kernel_del_local (addr_ipv4_t *p, uint32_t ifindex, uint8 prefixlen);

int32
lib_netlink_register_add_newlink(RT_INTERFACE_FUNC fn);

int32
lib_netlink_register_del_newlink(RT_INTERFACE_FUNC fn);

int32
lib_netlink_register_ipv4_add_addr(RT_IPV4_ADDR_FUNC fn);

int32
lib_netlink_register_ipv4_del_addr(RT_IPV4_ADDR_FUNC fn);

int32
lib_netlink_register_ipv4_add_route(RT_IPV4_ROUTE_FUNC fn);

int32
lib_netlink_register_ipv4_del_route(RT_IPV4_ROUTE_FUNC fn);

int32
lib_netlink_register_ipv4_add_mroute(RT_IPV4_MROUTE_FUNC fn);

int32
lib_netlink_register_ipv4_del_mroute(RT_IPV4_MROUTE_FUNC fn);

int32
lib_netlink_register_ipv4_add_route_multipath_check(RT_IPV4_RT_MULTI_FUNC fn);

int32
lib_netlink_register_ipv4_del_route_multipath_check(RT_IPV4_RT_MULTI_FUNC fn);

int32
lib_netlink_register_ipv4_add_route_multipath(RT_IPV4_RT_MULTI_FUNC fn);

int32
lib_netlink_register_ipv4_del_route_multipath(RT_IPV4_RT_MULTI_FUNC fn);

int32
lib_netlink_register_ipv4_update_route_multipath(RT_IPV4_RT_MULTI_FUNC fn);

int32
lib_netlink_register_ipv4_add_nh_group(RT_IPV4_NH_GROUP_FUNC fn);

int32
lib_netlink_register_ipv4_get_nh_key(RT_IPV4_NH_GROUP_FUNC fn);

int32
lib_netlink_register_ipv4_del_nh_group(RT_IPV4_NH_GROUP_FUNC fn);

int32
lib_netlink_register_ipv4_add_neigh(RT_IPV4_ADD_NEIGH_FUNC fn);

int32
lib_netlink_register_ipv4_add_imcomplete_neigh(RT_IPV4_ADD_NEIGH_FUNC fn);

int32
lib_netlink_register_ipv4_del_neigh(RT_IPV4_DEL_NEIGH_FUNC fn);

int32
lib_netlink_register_ipv4_get_neigh(RT_IPV4_GET_NEIGH_FUNC fn);

int32
lib_netlink_register_ipv6_get_neigh(RT_IPV6_GET_NEIGH_FUNC fn);

int32
lib_netlink_register_ipv6_add_addr(RT_IPV6_ADDR_FUNC fn);

int32
lib_netlink_register_ipv6_del_addr(RT_IPV6_ADDR_FUNC fn);

int32
lib_netlink_register_ipv6_add_route(RT_IPV6_ROUTE_FUNC fn);

int32
lib_netlink_register_ipv6_del_route(RT_IPV6_ROUTE_FUNC fn);

int32
lib_netlink_register_ipv6_add_route_multipath_check(RT_IPV6_RT_MULTI_FUNC fn);

int32
lib_netlink_register_ipv6_del_route_multipath_check(RT_IPV6_RT_MULTI_FUNC fn);

int32
lib_netlink_register_ipv6_add_route_multipath(RT_IPV6_RT_MULTI_FUNC fn);

int32
lib_netlink_register_ipv6_del_route_multipath(RT_IPV6_RT_MULTI_FUNC fn);

int32
lib_netlink_register_ipv6_get_nh_key(RT_IPV6_NH_GROUP_FUNC fn);

int32
lib_netlink_link_set(uint32_t ifindex, uint32 is_up);

int32
kernel_del_local(prefix_t *p);

int32
kernel_add_local(prefix_t *p, uint32 kernel_ifindex);

int32
kernel_interface_get_ifindex(char *ifname, uint32 *ifindex);

int32
kernel_neigh_entry_add(addr_ipv4_t *ip_addr, uint8 *mac_addr, uint32 ifindex, uint8 is_proxy_arp);

int32
kernel_neigh_entry_del(addr_ipv4_t *ip_addr, uint8 *mac_addr, uint32 ifindex);

/* support nd modified by liwh for bug 47547, 2018-06-28 */
int32
kernel_ipv6_neigh_entry_add(addr_ipv6_t *ip_addr, uint8 *mac_addr, uint32 ifindex);

int32
kernel_ipv6_neigh_entry_del(addr_ipv6_t *ip_addr, uint8 *mac_addr, uint32 ifindex);
/* liwh end */

char*
netlink_msg_type_desc(uint32 type);

int32
netlink_route_process(struct sal_sockaddr_nl *snl, struct nlmsghdr *h);

int32
lib_netlink_init_rt_cmd();

int32
lib_netlink_init();

int32 lib_netlink_init_pimsm();
void _lib_netlink_read_pimsm();
#endif /* !__LIB_NETLINK_H__ */
