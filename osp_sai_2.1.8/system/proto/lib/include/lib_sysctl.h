
#ifndef __LIB_SYSCTL_H__
#define __LIB_SYSCTL_H__

int32
lib_sysctl_set_icmp_error_ratelimit(uint32 ratelimit);

int32
lib_sysctl_set_intf_ip_unreachable_en(vrf_id_t vrf_id, char *ifname, uint8 enable);

int32
lib_sysctl_set_intf_ip_redirects_en(vrf_id_t vrf_id, char *ifname, uint8 enable);

int32
lib_sysctl_set_intf_arp_miss_by_name(char *ifname, uint8 enable);

int32
lib_sysctl_set_intf_enable_arp(char *ifname, uint8 enable);

int32
lib_sysctl_set_nei_retrans(vrf_id_t vrf_id, char *ifname, uint8 timelen);

int32
lib_sysctl_set_arp_max_number(uint32 max_number);

int32
lib_sysctl_set_intf_arp_retrans_time(char *ifname, uint32 retrans_time);

int32
lib_sysctl_set_intf_arp_timeout(char *ifname, uint32 timeout);

int32
lib_sysctl_nd_set_default_hop_limit(uint32 hop_limit);

int32
lib_sysctl_nd_set_if_hop_limit(char *ifname, uint32 hop_limit);

int32
lib_sysctl_nd_set_if_dad_attempts(char *ifname, uint32 dad_attempts);

int32
lib_sysctl_nd_set_if_ns_interval(char *ifname, uint32 ns_interval);

int32
lib_sysctl_nd_set_if_reachable_time(char *ifname, uint32 reachable_time);

#endif /* !__LIB_SYSCTL_H__ */

