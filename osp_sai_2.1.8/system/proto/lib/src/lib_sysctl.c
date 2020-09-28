
#include <sys/sysctl.h>
#include "proto.h"
#include "lib_sysctl.h"

int32
lib_sysctl_set_icmp_error_ratelimit(uint32 ratelimit)
{
#ifdef _KERNEL_VER_2_6_
#ifdef _NEW_VERSION_LINUX_
    uint32 kerrate = ratelimit;
#else
    uint32 kerrate = ratelimit * HZ / 1000;
#endif
    int32 name[] = {CTL_NET, NET_IPV4, NET_IPV4_ICMP_RATELIMIT};
    size_t size;

    if (-1 == sysctl(name, sizeof(name)/sizeof(int), 0, &size, &kerrate, sizeof(kerrate)))
    {
        return PM_E_FAIL;
    }
    
    return PM_E_NONE;
#else
    char sysctl_name[256];

    sal_snprintf(sysctl_name, sizeof(sysctl_name), "sysctl -w net.ipv4.icmp_ratelimit=%u > /dev/null",
        ratelimit);

    return system(sysctl_name);
#endif
}

int32
lib_sysctl_set_intf_ip_unreachable_en(vrf_id_t vrf_id, char *ifname, uint8 enable)
{
#ifdef _KERNEL_VER_2_6_
    char sysctl_name[256];
    uint32 set = enable ? TRUE : FALSE;

    sal_snprintf(sysctl_name, sizeof(sysctl_name), "chvrf %d sysctl -w net.ipv4.conf.%s.icmp_unreach_enable=%u > /dev/null",
        vrf_id, ifname, set);
    
    return system(sysctl_name);
#else
#if 1
    /* raw-kernel not support set IP unreachable */
    return PM_E_NONE;
#else
    char sysctl_name[256];
    uint32 set = enable ? TRUE : FALSE;

    if (GLB_DEF_VRFID == vrf_id)
    {
        sal_snprintf(sysctl_name, sizeof(sysctl_name), "sysctl -w net.ipv4.conf.%s.icmp_unreach_enable=%u > /dev/null",
            ifname, set);
    }
    else
    {
        sal_snprintf(sysctl_name, sizeof(sysctl_name), NETNS_CMD_PREFIX" sysctl -w net.ipv4.conf.%s.icmp_unreach_enable=%u > /dev/null",
            ifname, set);
    }
    
    return system(sysctl_name);
#endif
#endif
}

int32
lib_sysctl_set_intf_ip_redirects_en(vrf_id_t vrf_id, char *ifname, uint8 enable)
{
#ifdef _KERNEL_VER_2_6_
    char sysctl_name[256];
    uint32 set = enable ? TRUE : FALSE;
    
    sal_snprintf(sysctl_name, sizeof(sysctl_name), "chvrf %d sysctl -w net.ipv4.conf.%s.send_redirects=%u > /dev/null",
        vrf_id, ifname, set);
#else
    char sysctl_name[256];
    uint32 set = enable ? TRUE : FALSE;

    if (GLB_DEF_VRFID == vrf_id)
    {
        sal_snprintf(sysctl_name, sizeof(sysctl_name), "sysctl -w net.ipv4.conf.%s.send_redirects=%u > /dev/null",
            ifname, set);
    }
    else
    {
        sal_snprintf(sysctl_name, sizeof(sysctl_name), NETNS_CMD_PREFIX" sysctl -w net.ipv4.conf.%s.send_redirects=%u > /dev/null",
            ifname, set);
    }
#endif

    return system(sysctl_name);
}

int32
lib_sysctl_set_intf_arp_miss_by_name(char *ifname, uint8 enable)
{
    char sysctl_name[256];
    int32 app_solicit = enable? 1 : 0;
    int32 uc_solicit = enable? 0 : 3;
    int32 mc_solicit = enable? 0 : 3;
    /* Modified by zhangdy @20150415. 
       Since there are 2 ns, and we need do routing in mgmt namespace, there might be packets
       which received on vgmgmt interface and routing to mgmt interface, if we turn forwarding
       attribute to false, the packet will be dropped, and port forwarding will no work. */
    int32 forwarding    = TRUE;
    int32 notify        = FALSE;
    int32 rc = PM_E_NONE;

    /*1. appsolict*/
    sal_snprintf(sysctl_name, sizeof(sysctl_name), NETNS_CMD_PREFIX" sysctl -w net.ipv4.neigh.%s.app_solicit=%d > /dev/null",
        ifname, app_solicit);
    rc = system(sysctl_name);
    if (rc)
    {
        log_sys(M_MOD_LIB, E_ERROR, "set app_solicit failed, error string=%s", sal_strerror(errno));
        return rc;
    }

    /*2. ucast solict*/
    sal_snprintf(sysctl_name, sizeof(sysctl_name), NETNS_CMD_PREFIX" sysctl -w net.ipv4.neigh.%s.ucast_solicit=%d > /dev/null",
        ifname, uc_solicit);
    rc = system(sysctl_name);
    if (rc)
    {
        log_sys(M_MOD_LIB, E_ERROR, "set ucast_solicit failed, error string=%s", sal_strerror(errno));
         return rc;
    }

    /*3. mcast solict*/
    sal_snprintf(sysctl_name, sizeof(sysctl_name), NETNS_CMD_PREFIX" sysctl -w net.ipv4.neigh.%s.mcast_solicit=%d > /dev/null",
        ifname, mc_solicit);
    rc = system(sysctl_name);
    if (rc)
    {
        log_sys(M_MOD_LIB, E_ERROR, "set mcast_solicit failed, error string=%s", sal_strerror(errno));
         return rc;
    }

    /*4. forwarding*/
    sal_snprintf(sysctl_name, sizeof(sysctl_name), NETNS_CMD_PREFIX" sysctl -w net.ipv4.conf.%s.forwarding=%d > /dev/null",
        ifname, forwarding);
    rc = system(sysctl_name);
    if (rc)
    {
        log_sys(M_MOD_LIB, E_ERROR, "set forwarding failed, error string=%s", sal_strerror(errno));
         return rc;
    }

    /*5. disable kernel gratuitous arp at v3.8 */
    sal_snprintf(sysctl_name, sizeof(sysctl_name), NETNS_CMD_PREFIX" sysctl -w net.ipv4.conf.%s.arp_notify=%d > /dev/null",
        ifname, notify);
    rc = system(sysctl_name);
    if (rc)
    {
        log_sys(M_MOD_LIB, E_ERROR, "set arp_notify failed, error string=%s", sal_strerror(errno));
         return rc;
    }

    return PM_E_NONE;
}

int32
lib_sysctl_set_intf_enable_arp(char *ifname, uint8 enable)
{
    char sysctl_name[256];
    int32 app_solicit = enable? 1 : 0;
    int32 uc_solicit = 0;
    int32 mc_solicit = 0;
    int32 arp_notify = enable? 1 : 0;
    int32 arp_accept = enable? 1 : 0;

    int32 forwarding = TRUE;
    int32 rc = PM_E_NONE;

    /* kernel ARP setting 

        int32 app_solicit = enable? 0 : 1;
        int32 uc_solicit = enable? 3 : 0;
        int32 mc_solicit = enable? 3 : 0;
        int32 arp_notify = enable? 1 : 0;
        int32 arp_accept = enable? 1 : 0;
    */

    sal_memset(sysctl_name, 0, sizeof(sysctl_name));
    
    /*1. appsolict*/
    sal_snprintf(sysctl_name, sizeof(sysctl_name), "sysctl -w net.ipv4.neigh.%s.app_solicit=%d > /dev/null",
        ifname, app_solicit);
    rc = system(sysctl_name);
    if (rc)
    {
        log_sys(M_MOD_LIB, E_ERROR, "set app_solicit failed, error string=%s", sal_strerror(errno));
        return rc;
    }

    /*2. ucast solict*/
    sal_snprintf(sysctl_name, sizeof(sysctl_name), "sysctl -w net.ipv4.neigh.%s.ucast_solicit=%d > /dev/null",
        ifname, uc_solicit);
    rc = system(sysctl_name);
    if (rc)
    {
        log_sys(M_MOD_LIB, E_ERROR, "set ucast_solicit failed, error string=%s", sal_strerror(errno));
         return rc;
    }

    /*3. mcast solict*/
    sal_snprintf(sysctl_name, sizeof(sysctl_name), "sysctl -w net.ipv4.neigh.%s.mcast_solicit=%d > /dev/null",
        ifname, mc_solicit);
    rc = system(sysctl_name);
    if (rc)
    {
        log_sys(M_MOD_LIB, E_ERROR, "set mcast_solicit failed, error string=%s", sal_strerror(errno));
         return rc;
    }

    /*4. forwarding*/
    sal_snprintf(sysctl_name, sizeof(sysctl_name), "sysctl -w net.ipv4.conf.%s.forwarding=%d > /dev/null",
        ifname, forwarding);
    rc = system(sysctl_name);
    if (rc)
    {
        log_sys(M_MOD_LIB, E_ERROR, "set forwarding failed, error string=%s", sal_strerror(errno));
         return rc;
    }

    /*5. arp notify */
    /* Disable kernel gratuitous arp at v3.8 */
    arp_notify = 0;
    sal_snprintf(sysctl_name, sizeof(sysctl_name), "sysctl -w net.ipv4.conf.%s.arp_notify=%d > /dev/null",
        ifname, arp_notify);
    rc = system(sysctl_name);
    if (rc)
    {
        log_sys(M_MOD_LIB, E_ERROR, "set forwarding failed, error string=%s", sal_strerror(errno));
         return rc;
    }

    /*5. arp arp_accept */
    sal_snprintf(sysctl_name, sizeof(sysctl_name), "sysctl -w net.ipv4.conf.%s.arp_accept=%d > /dev/null",
        ifname, arp_accept);
    rc = system(sysctl_name);
    if (rc)
    {
        log_sys(M_MOD_LIB, E_ERROR, "set forwarding failed, error string=%s", sal_strerror(errno));
         return rc;
    }

    return PM_E_NONE;
}

int32
lib_sysctl_set_intf_arp_retrans_time(char *ifname, uint32 retrans_time)
{
    char sysctl_name[256];
    int rc = PM_E_NONE;

    sal_snprintf(sysctl_name, sizeof(sysctl_name), "sysctl -w net.ipv4.neigh.%s.retrans_time_ms=%d > /dev/null",
        ifname, retrans_time*1000);
    rc = system(sysctl_name);
    if (rc)
    {
        log_sys(M_MOD_LIB, E_ERROR, "set retrans_time failed, error string=%s", sal_strerror(errno));
        return rc;
    }

    return PM_E_NONE;
}

int32
lib_sysctl_set_intf_arp_timeout(char *ifname, uint32 timeout)
{
    char sysctl_name[256];
    int rc = PM_E_NONE;

    sal_snprintf(sysctl_name, sizeof(sysctl_name), "sysctl -w net.ipv4.neigh.%s.gc_stale_time=%d > /dev/null",
        ifname, timeout);
    rc = system(sysctl_name);
    if (rc)
    {
        log_sys(M_MOD_LIB, E_ERROR, "set retrans_time failed, error string=%s", sal_strerror(errno));
        return rc;
    }

    return PM_E_NONE;
}

int32
lib_sysctl_set_arp_max_number(uint32 max_number)
{
    char sysctl_name[256];
    int rc = PM_E_NONE;

    sal_snprintf(sysctl_name, sizeof(sysctl_name), "sysctl -w net.ipv4.neigh.default.gc_thresh3=%d > /dev/null",
        max_number);
    rc = system(sysctl_name);
    if (rc)
    {
        log_sys(M_MOD_LIB, E_ERROR, "set forwarding failed, error string=%s", sal_strerror(errno));
         return rc;
    }

    return PM_E_NONE;
}

int32
lib_sysctl_set_nei_retrans(vrf_id_t vrf_id, char *ifname, uint8 timelen)
{    
    uint32 num;
/* The CTC_NET_NEIGH_RETRANS_TIME_FACTOR must refer to the Linux HZ */    
    char sysctl_name[256];
    int32 rc = PM_E_NONE;

/*new kernel use USER_HZ, see proc_dointvec_userhz_jiffies()
->do_proc_dointvec_userhz_jiffies_conv()->clock_t_to_jiffies*/
#define CTC_NET_NEIGH_RETRANS_TIME_FACTOR 1000

    if (0 == timelen)
    {
        num = CTC_NET_NEIGH_RETRANS_TIME_FACTOR;
    }
    else
    {
        num = timelen*CTC_NET_NEIGH_RETRANS_TIME_FACTOR*3;
    }

    if (GLB_DEF_VRFID == vrf_id)
    {
        sal_snprintf(sysctl_name, sizeof(sysctl_name), "sysctl -w net.ipv4.neigh.%s.retrans_time_ms=%d > /dev/null",
            ifname, num);
    }
    else
    {
        sal_snprintf(sysctl_name, sizeof(sysctl_name), NETNS_CMD_PREFIX" sysctl -w net.ipv4.neigh.%s.retrans_time_ms=%d > /dev/null",
            ifname, num);
    }

    rc = system(sysctl_name);
    if (rc)
    {
        log_sys(M_MOD_LIB, E_ERROR, "set retrans_time_ms failed, error string=%s", sal_strerror(errno));
        return rc;
    }
    
    return PM_E_NONE;
}

int32
lib_sysctl_nd_set_default_hop_limit(uint32 hop_limit)
{
    /* modified by liwh for bug 48697, 2018-09-05 */
    char sysctl_name[256];
    //int rc = PM_E_NONE;

    sal_snprintf(sysctl_name, sizeof(sysctl_name), "sysctl -w net.ipv6.conf.default.hop_limit=%d > /dev/null",
        hop_limit);
    system(sysctl_name);

    
    /*if (rc && errno)
    {
        log_sys(M_MOD_LIB, E_ERROR, "set default hop limit failed, error string=%s", sal_strerror(errno));
        return rc;
    }*/
    /* liwh end */

    return PM_E_NONE;
}

int32
lib_sysctl_nd_set_if_hop_limit(char *ifname, uint32 hop_limit)
{
    /* modified by liwh for bug 48697, 2018-09-05 */
    char sysctl_name[256];
    //int rc = PM_E_NONE;
    char *cp_ifname = NULL;
    cp_ifname = ifname;
    char *div = NULL;
    div = sal_strstr(cp_ifname, "/");
    if (NULL != div)
    {
        div[0] = '_';
    }
    if (0 == sal_memcmp(ifname, "agg", 3))
    {
        return PM_E_NONE;    
    }

    sal_snprintf(sysctl_name, sizeof(sysctl_name), "sysctl -w net.ipv6.conf.%s.hop_limit=%d > /dev/null",
        ifname, hop_limit);
    system(sysctl_name);

    /*if (rc && errno)
    {
        log_sys(M_MOD_LIB, E_ERROR, "set interface hop limit failed, error string=%s", sal_strerror(errno));
        return rc;
    }*/

    return PM_E_NONE;
}

int32
lib_sysctl_nd_set_if_dad_attempts(char *ifname, uint32 dad_attempts)
{
    /* modified by liwh for bug 48697, 2018-09-05 */
    /*char sysctl_name[256];
    int rc = PM_E_NONE;

    if (0 == sal_memcmp(ifname, "agg", 3))
    {
        return PM_E_NONE;    
    }

    sal_snprintf(sysctl_name, sizeof(sysctl_name), "sysctl -w net.ipv6.conf.%s.dad_transmits=%d > /dev/null",
        ifname, dad_attempts);
    rc = system(sysctl_name);

    if (rc && errno)
    {
        log_sys(M_MOD_LIB, E_ERROR, "set interface dad attempts failed, error string=%s", sal_strerror(errno));
        return rc;
    }*/
    /* liwh end */

    return PM_E_NONE;
}

int32
lib_sysctl_nd_set_if_ns_interval(char *ifname, uint32 ns_interval)
{
    /* modified by liwh for bug 48697, 2018-09-05 */
    /*char sysctl_name[256];
    int rc = PM_E_NONE;

    if (0 == sal_memcmp(ifname, "agg", 3))
    {
        return PM_E_NONE;    
    }

    sal_snprintf(sysctl_name, sizeof(sysctl_name), "sysctl -w net.ipv6.neigh.%s.retrans_time_ms=%d > /dev/null",
        ifname, ns_interval);
    rc = system(sysctl_name);

    if (rc && errno)
    {
        log_sys(M_MOD_LIB, E_ERROR, "set interface ns interval failed, error string=%s", sal_strerror(errno));
        return rc;
    }*/
    /* liwh end */

    return PM_E_NONE;
}

int32
lib_sysctl_nd_set_if_reachable_time(char *ifname, uint32 reachable_time)
{
    /* modified by liwh for bug 48697, 2018-09-05 */
    /*char sysctl_name[256];
    int rc = PM_E_NONE;

    if (0 == sal_memcmp(ifname, "agg", 3))
    {
        return PM_E_NONE;    
    }

    sal_snprintf(sysctl_name, sizeof(sysctl_name), "sysctl -w net.ipv6.neigh.%s.base_reachable_time_ms=%d > /dev/null",
        ifname, reachable_time);
    rc = system(sysctl_name);

    if (rc && errno)
    {
        log_sys(M_MOD_LIB, E_ERROR, "set interface reachable time failed, error string=%s", sal_strerror(errno));
        return rc;
    }*/
    /* liwh end */

    return PM_E_NONE;
}