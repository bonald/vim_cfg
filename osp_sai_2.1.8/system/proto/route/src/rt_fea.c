
#include "proto.h"
#include "lib_tblinc.h"
#include "route.h"
#include "lib_netlink.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_nexthop_group_define.h"
#include "gen/tbl_nexthop_group.h"
#include "gen/tbl_arp_fib_define.h"
#include "gen/tbl_arp_fib.h"
#include "gen/tbl_sys_spec_define.h"
#include "gen/tbl_sys_spec.h"
#include "gen/tbl_chassis_define.h"
#include "gen/tbl_chassis.h"
#include "gen/tbl_ecmp_global_define.h"
#include "gen/tbl_ecmp_global.h"
#include "gen/tbl_rr_prefix_define.h"
#include "gen/tbl_rr_prefix.h"
#include "gen/tbl_mrt_define.h"
#include "gen/tbl_mrt.h"
#include "gen/tbl_iproute_node_add_fail_define.h"
#include "gen/tbl_iproute_node_add_fail.h"
#include "gen/tbl_iproute_node_add_fail_count_define.h"
#include "gen/tbl_iproute_node_add_fail_count.h"
#include "gen/ds_connected_define.h"
#include "gen/ds_connected.h"
#include "gen/tbl_nd_fib_define.h"
#include "gen/tbl_nd_fib.h"
#include "arp_api.h"
#include "arpratelimit_api.h"
#include "rt_fea.h"
#include "rt_if.h"
#include "sal.h"
#include "sal_types.h"
#include "lib_ioctl.h"
#include "lib_sysctl.h"
#include <linux/version.h>
#include "arpratelimit_api.h"
#include "nd_api.h"

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

char *
rt_fea_api_nexthop_type_desc(uint32 nexthop_type)
{
    switch(nexthop_type)
    {
    case RTN_UNSPEC:
        return "RTN_UNSPEC";
    case RTN_UNICAST:        /* Gateway or direct route  */
        return "RTN_UNICAST";
    case RTN_LOCAL:      /* Accept locally       */
        return "RTN_LOCAL";
    case RTN_BROADCAST:      /* Accept locally as broadcast,
                   send as broadcast */
        return "RTN_BROADCAST";
    case RTN_ANYCAST:        /* Accept locally as broadcast,
                   but send as unicast */
        return "RTN_ANYCAST";
    case RTN_MULTICAST:      /* Multicast route      */
        return "RTN_MULTICAST";
    case RTN_BLACKHOLE:      /* Drop             */
        return "RTN_BLACKHOLE";
    case RTN_UNREACHABLE:    /* Destination is unreachable   */
        return "RTN_UNREACHABLE";
    case RTN_PROHIBIT:       /* Administratively prohibited  */
        return "RTN_PROHIBIT";
    case RTN_THROW:      /* Not in this table        */
        return "RTN_THROW";
    case RTN_NAT:       /* Translate this address   */
        return "RTN_NAT";
    case RTN_XRESOLVE:       /* Use external resolver    */
        return "RTN_XRESOLVE";
    default:
        return "unknown nexthop type";
    }
}
#ifdef CONFIG_IPV4
//#define RTN_BLACKHOLE 6
//#define RTN_LOCAL 2
//#define RTN_UNICAST 1
#define ECMP_ENABLE         1
#define ECMP_DISABLE        0
#define MULTIPATH_CHECK     8
#define ROUTE_COUNT_MAX     1023
/* modified by wangjj for adding the count of system init, bug 40782, 2016-12-15 */
//#define ROUTE_INIT_IPUC_CTT 4
#define ROUTE_INIT_IPUC_CTT 0

static const char proc_ipv4_forwarding[] = "/proc/sys/net/ipv4/ip_forward";
static int32 route_count = 0;
int32 route_ecmp_cnt = 0;

static int32 route_count_v6 = 0;


int32
rt_fea_ipv4_update_nh_by_group(vrf_id_t vrf_id, addr_t *dest, nexthop_key_t *nhg,
                    uint32 nh_num, uint8 prefixlen, uint32 distance);
int32
rt_fea_ipv4_del_nhg(tbl_iproute_node_t  *pst_iproute, nexthop_key_t *nhg,uint32 nh_num, uint32 rr_en, uint32 is_ipv4);


int32
rt_neigh_counter_inc(vrf_id_t vrf_id)
{
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();

    p_rt_glb->current_host_routes++;
    tbl_route_global_set_route_global_field(p_rt_glb, TBL_ROUTE_GLOBAL_FLD_CURRENT_HOST_ROUTES);
    return PM_E_NONE;
}

/*IPv4 ecmp groups*/
int32
rt_ecmp_counter_inc(vrf_id_t vrf_id)
{
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();

    p_rt_glb->current_ecmp_routes++;
    tbl_route_global_set_route_global_field(p_rt_glb, TBL_ROUTE_GLOBAL_FLD_CURRENT_ECMP_ROUTES);
    return PM_E_NONE;
}

/*IPv4 ecmp groups*/
int32
rt_ecmp_groups_counter_inc(vrf_id_t vrf_id)
{
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();

    p_rt_glb->current_ecmp_groups++;
    tbl_route_global_set_route_global_field(p_rt_glb, TBL_ROUTE_GLOBAL_FLD_CURRENT_ECMP_GROUPS);
    return PM_E_NONE;
}

/*IPv4 static route*/
int32
rt_static_routes_inc(vrf_id_t vrf_id)
{
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();

    p_rt_glb->current_static_routes++;
    tbl_route_global_set_route_global_field(p_rt_glb, TBL_ROUTE_GLOBAL_FLD_CURRENT_STATIC_ROUTES);
    return PM_E_NONE;
}

/*Indirect IPv4 routes*/
int32
rt_route_counter_inc(vrf_id_t vrf_id)
{
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();

    p_rt_glb->current_remote_routes++;
    tbl_route_global_set_route_global_field(p_rt_glb, TBL_ROUTE_GLOBAL_FLD_CURRENT_REMOTE_ROUTES);
    return PM_E_NONE;
}

/*Indirect IPv4 routes regardless interface up down*/
int32
rt_indirect_ipv4_route_inc(vrf_id_t vrf_id, uint32 num)
{
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();

    if (p_rt_glb->current_indirect_ipv4_route_num + num > rt_fea_indirect_route_count_get_max())
    {
        logid_sys(LOG_ROUTE_3_STM_INC_DEC_ERR, "Indirect IPv4 routes", p_rt_glb->current_indirect_ipv4_route_num, "increase", num, rt_fea_indirect_route_count_get_max());
        return PM_E_ROUTE_INDIRECT_ROUTE_FULL;
    }
    else
    {
        p_rt_glb->current_indirect_ipv4_route_num += num;
        tbl_route_global_set_route_global_field(p_rt_glb, TBL_ROUTE_GLOBAL_FLD_CURRENT_INDIRECT_IPV4_ROUTE_NUM);
    }
    return PM_E_NONE;
}

/*IPv6 ecmp groups*/
int32
rt_ecmp_v6_counter_inc(vrf_id_t vrf_id)
{
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();

    p_rt_glb->current_ecmp_v6_routes++;
    tbl_route_global_set_route_global_field(p_rt_glb, TBL_ROUTE_GLOBAL_FLD_CURRENT_ECMP_V6_ROUTES);
    return PM_E_NONE;
}

/*IPv6 ecmp groups*/
int32
rt_ecmp_groups_v6_counter_inc(vrf_id_t vrf_id)
{
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();

    p_rt_glb->current_ecmp_v6_groups++;
    tbl_route_global_set_route_global_field(p_rt_glb, TBL_ROUTE_GLOBAL_FLD_CURRENT_ECMP_V6_GROUPS);
    return PM_E_NONE;
}

/*IPv6 static route*/
int32
rt_static_v6_routes_inc(vrf_id_t vrf_id)
{
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();

    p_rt_glb->current_static_v6_routes++;
    tbl_route_global_set_route_global_field(p_rt_glb, TBL_ROUTE_GLOBAL_FLD_CURRENT_STATIC_V6_ROUTES);
    return PM_E_NONE;
}

/*Indirect IPv6 routes*/
int32
rt_route_v6_counter_inc(vrf_id_t vrf_id)
{
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();

    p_rt_glb->current_remote_v6_routes++;
    tbl_route_global_set_route_global_field(p_rt_glb, TBL_ROUTE_GLOBAL_FLD_CURRENT_REMOTE_V6_ROUTES);
    return PM_E_NONE;
}

/*Indirect IPv6 routes regardless interface up down*/
int32
rt_indirect_ipv6_route_inc(vrf_id_t vrf_id, uint32 num)
{
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();

    p_rt_glb->current_indirect_ipv6_route_num += num;
    tbl_route_global_set_route_global_field(p_rt_glb, TBL_ROUTE_GLOBAL_FLD_CURRENT_INDIRECT_IPV6_ROUTE_NUM);
    return PM_E_NONE;
}

int32
rt_neigh_counter_dec(vrf_id_t vrf_id)
{
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();

    p_rt_glb->current_host_routes--;
    tbl_route_global_set_route_global_field(p_rt_glb, TBL_ROUTE_GLOBAL_FLD_CURRENT_HOST_ROUTES);
    return PM_E_NONE;
}

/*IPv4 ecmp groups*/
int32
rt_ecmp_counter_dec(vrf_id_t vrf_id)
{
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();

    p_rt_glb->current_ecmp_routes--;
    tbl_route_global_set_route_global_field(p_rt_glb, TBL_ROUTE_GLOBAL_FLD_CURRENT_ECMP_ROUTES);
    return PM_E_NONE;
}

/*IPv4 ecmp groups*/
int32
rt_ecmp_groups_counter_dec(vrf_id_t vrf_id)
{
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();

    p_rt_glb->current_ecmp_groups--;
    tbl_route_global_set_route_global_field(p_rt_glb, TBL_ROUTE_GLOBAL_FLD_CURRENT_ECMP_GROUPS);
    return PM_E_NONE;
}

/*IPv4 static route*/
int32
rt_static_routes_dec(vrf_id_t vrf_id)
{
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();

    p_rt_glb->current_static_routes--;
    tbl_route_global_set_route_global_field(p_rt_glb, TBL_ROUTE_GLOBAL_FLD_CURRENT_STATIC_ROUTES);
    return PM_E_NONE;
}

/*IPv6 static route*/
int32
rt_static_v6_routes_dec(vrf_id_t vrf_id)
{
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();

    p_rt_glb->current_static_v6_routes--;
    tbl_route_global_set_route_global_field(p_rt_glb, TBL_ROUTE_GLOBAL_FLD_CURRENT_STATIC_V6_ROUTES);
    return PM_E_NONE;
}

/*Indirect IPv4 routes*/
int32
rt_route_counter_dec(vrf_id_t vrf_id)
{
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();

    p_rt_glb->current_remote_routes--;
    tbl_route_global_set_route_global_field(p_rt_glb, TBL_ROUTE_GLOBAL_FLD_CURRENT_REMOTE_ROUTES);
    return PM_E_NONE;
}

/*Indirect IPv4 routes regardless interface up down*/
int32
rt_indirect_ipv4_route_dec(vrf_id_t vrf_id, uint32 num)
{
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();

    if (p_rt_glb->current_indirect_ipv4_route_num < num)
    {
        logid_sys(LOG_ROUTE_3_STM_INC_DEC_ERR, "Indirect IPv4 routes", p_rt_glb->current_indirect_ipv4_route_num, "decrease", num, rt_fea_indirect_route_count_get_max());
    }
    else
    {
        p_rt_glb->current_indirect_ipv4_route_num -= num;
        tbl_route_global_set_route_global_field(p_rt_glb, TBL_ROUTE_GLOBAL_FLD_CURRENT_INDIRECT_IPV4_ROUTE_NUM);
    }
    return PM_E_NONE;
}

/*IPv6 ecmp groups*/
int32
rt_ecmp_v6_counter_dec(vrf_id_t vrf_id)
{
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();

    //if (p_rt_glb->current_ecmp_v6_routes > 0)
    {
        p_rt_glb->current_ecmp_v6_routes--;
        tbl_route_global_set_route_global_field(p_rt_glb, TBL_ROUTE_GLOBAL_FLD_CURRENT_ECMP_V6_ROUTES);
    }
    return PM_E_NONE;
}

/*IPv6 ecmp groups*/
int32
rt_ecmp_groups_v6_counter_dec(vrf_id_t vrf_id)
{
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();

    p_rt_glb->current_ecmp_v6_groups--;
    tbl_route_global_set_route_global_field(p_rt_glb, TBL_ROUTE_GLOBAL_FLD_CURRENT_ECMP_V6_GROUPS);
    return PM_E_NONE;
}

/*Indirect IPv6 routes*/
int32
rt_route_v6_counter_dec(vrf_id_t vrf_id)
{
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();

    p_rt_glb->current_remote_v6_routes--;
    tbl_route_global_set_route_global_field(p_rt_glb, TBL_ROUTE_GLOBAL_FLD_CURRENT_REMOTE_V6_ROUTES);
    return PM_E_NONE;
}

/*Indirect IPv6 routes regardless interface up down*/
int32
rt_indirect_ipv6_route_dec(vrf_id_t vrf_id, uint32 num)
{
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();

    p_rt_glb->current_indirect_ipv6_route_num -= num;
    tbl_route_global_set_route_global_field(p_rt_glb, TBL_ROUTE_GLOBAL_FLD_CURRENT_INDIRECT_IPV6_ROUTE_NUM);
    return PM_E_NONE;
}

uint32
rt_indirect_ipv6_route_count_max_check(uint32 num)
{
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();
    uint32 is_over = FALSE;
    
    if (p_rt_glb->current_indirect_ipv6_route_num + num -1 >= rt_fea_indirect_route_v6_count_get_max())
    {
        is_over = TRUE;
    }

    return is_over;
}

int32 route_get_frr_ospf_route_num()
{
    int32 frr_ospf_route_num = 0;
    FILE *fp_console = NULL;
    char buf[256];

    rt_vty_exec_cmd("show ip route summary write");

    fp_console = sal_fopen("/tmp/ospf_route_num", "r");
    if (fp_console)
    {
        while (sal_fgets(buf, 128, fp_console))
        {    
            sal_sscanf(buf, "%d", &frr_ospf_route_num);
        }
        sal_fclose(fp_console);
        return frr_ospf_route_num;
    }
    else
    {
        return -1;
    }
}

uint32
rt_indirect_ipv4_route_count_max_check(uint32 incr_num)
{
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();
    tbl_iproute_node_add_fail_count_master_t   * p_iproute_node_add_fail_count_master    = tbl_iproute_node_add_fail_count_get_master();
    uint32 is_over = FALSE;
    uint32 total_num = 0;

    int32 frr_ospf_route_num = 0;
    frr_ospf_route_num = route_get_frr_ospf_route_num();
    if (frr_ospf_route_num != -1)
    {
        total_num += frr_ospf_route_num;
    }

    total_num += p_rt_glb->current_indirect_ipv4_route_num;

    /*modified by chenc for bug 52729, 2019-06-27*/
    int32 route_add_fail_count = p_iproute_node_add_fail_count_master->add_fail_hash->count;
    if (total_num >= route_add_fail_count)
    {
        total_num -= route_add_fail_count;
    }
    /*chenc end*/
    
    if (total_num + incr_num - 1 >= rt_fea_indirect_route_count_get_max())
    {
        is_over = TRUE;
    }

    return is_over;
}

uint32
rt_fea_static_route_count_get_max()
{
    tbl_sys_spec_t  *p_sys_spec = NULL;
    tbl_chassis_t   *p_chassis  = NULL;
    int32 type = 1000;

    p_chassis = tbl_chassis_get_chassis();
    if (NULL == p_chassis) {
        return 0;
    }

    type = p_chassis->cur_stm_mode;
    if (type < GLB_STM_MODE_MAX) 
    {
        p_sys_spec = tbl_sys_spec_get_sys_spec_by_type(type);
        if(NULL != p_sys_spec) 
        {
            return (p_sys_spec->static_routes );
        }
    }

    return 0;
}

uint32
rt_fea_indirect_route_count_get_max()
{
    tbl_sys_spec_t  *p_sys_spec = NULL;
    tbl_chassis_t   *p_chassis  = NULL;
    int32 type = 1000;

    p_chassis = tbl_chassis_get_chassis();
    if (NULL == p_chassis) 
    {
        return 0;
    }

    type = p_chassis->cur_stm_mode;
    if (type < GLB_STM_MODE_MAX) 
    {
        p_sys_spec = tbl_sys_spec_get_sys_spec_by_type(type);
        if(NULL != p_sys_spec) 
        {
            return (p_sys_spec->indirect_routes - ROUTE_INIT_IPUC_CTT);
        }
    }
    return 0;
}

uint32
rt_fea_static_route_count_max_check()
{
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();
    uint32 is_over = FALSE;
    
    if (p_rt_glb->current_remote_routes >= rt_fea_static_route_count_get_max())
    {
        is_over = TRUE;
        logid_sys(LOG_ROUTE_4_STATIC_ROUTE_OVERFLOW);
    }
    return is_over;
}

uint32
rt_fea_indirect_route_count_max_check()
{
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();
    uint32 is_over = FALSE;
    
    if (p_rt_glb->current_remote_routes >= rt_fea_indirect_route_count_get_max())
    {
        is_over = TRUE;
        logid_sys(LOG_ROUTE_4_FIB_OVERFLOW);
    }
    return is_over;
}

uint32
rt_fea_indirect_route_v6_count_get_max()
{
    tbl_sys_spec_t  *p_sys_spec = NULL;
    tbl_chassis_t   *p_chassis  = NULL;
    int32 type = 1000;

    p_chassis = tbl_chassis_get_chassis();
    if (NULL == p_chassis) 
    {
        return 0;
    }
    type = p_chassis->cur_stm_mode;
    if (type < GLB_STM_MODE_MAX) 
    {
        p_sys_spec = tbl_sys_spec_get_sys_spec_by_type(type);
        if(NULL != p_sys_spec) 
        {
            return (p_sys_spec->indirect_v6_routes - ROUTE_INIT_IPUC_CTT);
        }
    }
    return 0;
}

uint32
rt_fea_static_route_v6_count_get_max()
{
    tbl_sys_spec_t  *p_sys_spec = NULL;
    tbl_chassis_t   *p_chassis  = NULL;
    int32 type = 1000;

    p_chassis = tbl_chassis_get_chassis();
    if (NULL == p_chassis) 
    {
        return 0;
    }
    type = p_chassis->cur_stm_mode;
    if (type < GLB_STM_MODE_MAX) 
    {
        p_sys_spec = tbl_sys_spec_get_sys_spec_by_type(type);
        if(NULL != p_sys_spec) 
        {
            return (p_sys_spec->static_v6_routes);
        }
    }
    return 0;
}

uint32
rt_fea_indirect_route_v6_count_max_check()
{
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();
    uint32 is_over = FALSE;
    
    if (p_rt_glb->current_remote_v6_routes >= rt_fea_indirect_route_v6_count_get_max())
    {
        is_over = TRUE;
        logid_sys(LOG_ROUTE_6_FIB_OVERFLOW);
    }
    return is_over;
}

uint32
rt_fea_static_route_v6_count_max_check()
{
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();
    uint32 is_over = FALSE;
    
    if (p_rt_glb->current_remote_v6_routes >= rt_fea_static_route_v6_count_get_max())
    {
        is_over = TRUE;
        logid_sys(LOG_ROUTE_6_STATIC_ROUTE_OVERFLOW);
    }
    return is_over;
}


uint32
rt_static_route_count_max_check()
{
    uint32 is_over = 0;
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();
    #if 0
    tbl_static_rt_cnt_master_t *pst_master = NULL;

    pst_master = tbl_static_rt_cnt_get_master();
    if (NULL == pst_master)
    {
        return is_over;
    }
    
    /* For support all routes can be added by static route config */
    //if (pst_master->rt_cnt_hash->count >= RT_STATIC_MAX_CNT)
    if (pst_master->rt_cnt_hash->count >= rt_fea_route_count_get_max())
    {
        is_over = 1;
    }
    #else
    if (NULL == p_rt_glb)
    {
        return is_over;
    }

    if (p_rt_glb->current_static_routes >= rt_fea_static_route_count_get_max())
    {
        is_over = 1;
    }
    #endif

    return is_over;
}

uint32
rt_static_v6_route_count_max_check()
{
    uint32 is_over = 0;
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();
    #if 0
    tbl_static_rt_cnt_master_t *pst_master = NULL;

    pst_master = tbl_static_rt_cnt_get_master();
    if (NULL == pst_master)
    {
        return is_over;
    }
    
    /* For support all routes can be added by static route config */
    //if (pst_master->rt_cnt_hash->count >= RT_STATIC_MAX_CNT)
    if (pst_master->rt_cnt_hash->count >= rt_fea_route_count_get_max())
    {
        is_over = 1;
    }
    #else
    if (NULL == p_rt_glb)
    {
        return is_over;
    }

    if (p_rt_glb->current_static_v6_routes >= rt_fea_static_route_v6_count_get_max())
    {
        is_over = 1;
    }
    #endif

    return is_over;
}

uint32
rt_fea_v6_ecmp_member_count_get_max()
{
    tbl_sys_spec_t  *p_sys_spec = NULL;
    tbl_chassis_t   *p_chassis  = NULL;
    int32 type = 1000;

    p_chassis = tbl_chassis_get_chassis();
    if (NULL == p_chassis) {
        return 0;
    }

    type = p_chassis->cur_stm_mode;
    if (type < GLB_STM_MODE_MAX) {
        p_sys_spec = tbl_sys_spec_get_sys_spec_by_type(type);
        if(NULL != p_sys_spec) {
                return p_sys_spec->ecmp_v6_member_maximum;
        }
    }

    return 0;
}

uint32
rt_fea_v6_ecmp_count_get_max()
{
    tbl_sys_spec_t  *p_sys_spec = NULL;
    tbl_chassis_t   *p_chassis  = NULL;
    int32 type = 1000;

    p_chassis = tbl_chassis_get_chassis();
    if (NULL == p_chassis) {
        return 0;
    }

    type = p_chassis->cur_stm_mode;
    if (type < GLB_STM_MODE_MAX) {
        p_sys_spec = tbl_sys_spec_get_sys_spec_by_type(type);
        if(NULL != p_sys_spec) {
            #ifdef OFPRODUCT
                return p_sys_spec->ecmp_v6_routes -GLB_OPENFLOW_SELECT_GROUP_MAX;
            #else
                return p_sys_spec->ecmp_v6_routes;
            #endif
        }
    }

    return 0;
}

/* nhg count max check*/
uint32
rt_fea_v6_ecmp_count_max_check()
{
    uint32 is_over = FALSE;

    /*use tbl_nexthop_group*/
    #if 0
    tbl_nexthop_group_master_t    * master     = tbl_nexthop_group_get_master();

    if (master && master->nhg_hash->count >= rt_fea_v6_ecmp_count_get_max())
    {
        is_over = TRUE;
    }
    #else
    /*for ipv4 and ipv6 share tbl_nexthop_group, cannot use tbl_nexthop_group to check*/
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();
    
    if (p_rt_glb && p_rt_glb->current_ecmp_v6_routes >= rt_fea_v6_ecmp_count_get_max())
    {
        is_over = TRUE;
        //logid_sys(LOG_ROUTE_4_FIB_OVERFLOW);
    }
    #endif

    return is_over;
}

/* nhg count max check*/
uint32
rt_fea_v6_ecmp_groups_count_max_check()
{
    uint32 is_over = FALSE;

    /*use tbl_nexthop_group*/
    #if 0
    tbl_nexthop_group_master_t    * master     = tbl_nexthop_group_get_master();

    if (master && master->nhg_hash->count >= rt_fea_v6_ecmp_count_get_max())
    {
        is_over = TRUE;
    }
    #else
    /*for ipv4 and ipv6 share tbl_nexthop_group, cannot use tbl_nexthop_group to check*/
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();
    
    if (p_rt_glb && p_rt_glb->current_ecmp_v6_groups >= rt_fea_v6_ecmp_count_get_max())
    {
        is_over = TRUE;
        //logid_sys(LOG_ROUTE_4_FIB_OVERFLOW);
    }
    #endif

    return is_over;
}

uint32
rt_fea_ecmp_member_count_get_max()
{
    tbl_sys_spec_t  *p_sys_spec = NULL;
    tbl_chassis_t   *p_chassis  = NULL;
    int32 type = 1000;

    p_chassis = tbl_chassis_get_chassis();
    if (NULL == p_chassis) {
        return 0;
    }

    type = p_chassis->cur_stm_mode;
    if (type < GLB_STM_MODE_MAX) {
        p_sys_spec = tbl_sys_spec_get_sys_spec_by_type(type);
        if(NULL != p_sys_spec) {
                return p_sys_spec->ecmp_member_maximum;
        }
    }

    return 0;
}

uint32
rt_fea_ecmp_count_get_max()
{
    tbl_sys_spec_t  *p_sys_spec = NULL;
    tbl_chassis_t   *p_chassis  = NULL;
    int32 type = 1000;

    p_chassis = tbl_chassis_get_chassis();
    if (NULL == p_chassis) {
        return 0;
    }

    type = p_chassis->cur_stm_mode;
    if (type < GLB_STM_MODE_MAX) {
        p_sys_spec = tbl_sys_spec_get_sys_spec_by_type(type);
        if(NULL != p_sys_spec) {
            #ifdef OFPRODUCT
                return p_sys_spec->ecmp_routes -GLB_OPENFLOW_SELECT_GROUP_MAX;
            #else
                return p_sys_spec->ecmp_routes;
            #endif
        }
    }

    return 0;
}


uint32
rt_fea_ecmp_count_max_check()
{
    uint32 is_over = FALSE;

    /*use tbl_nexthop_group*/
    #if 0
    tbl_nexthop_group_master_t    * master     = tbl_nexthop_group_get_master();

    if (master && master->nhg_hash->count >= rt_fea_ecmp_count_get_max())
    {
        is_over = TRUE;
    }
    #else
    /*for ipv4 and ipv6 share tbl_nexthop_group, cannot use tbl_nexthop_group to check*/
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();
    
    if (p_rt_glb && p_rt_glb->current_ecmp_routes >= rt_fea_ecmp_count_get_max())
    {
        is_over = TRUE;
        //logid_sys(LOG_ROUTE_4_FIB_OVERFLOW);
    }
    #endif

    return is_over;
}

uint32
rt_fea_ecmp_groups_count_max_check()
{
    uint32 is_over = FALSE;

    /*use tbl_nexthop_group*/
    #if 0
    tbl_nexthop_group_master_t    * master     = tbl_nexthop_group_get_master();

    if (master && master->nhg_hash->count >= rt_fea_ecmp_count_get_max())
    {
        is_over = TRUE;
    }
    #else
    /*for ipv4 and ipv6 share tbl_nexthop_group, cannot use tbl_nexthop_group to check*/
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();
    
    if (p_rt_glb && p_rt_glb->current_ecmp_groups >= rt_fea_ecmp_count_get_max())
    {
        is_over = TRUE;
        //logid_sys(LOG_ROUTE_4_FIB_OVERFLOW);
    }
    #endif

    return is_over;
}

uint32
rt_fea_route_v6_count_min_check()
{
    uint32 is_min = 0;

    if (1 > route_count_v6)
    {
        is_min = 1;
    }

    return is_min;
}

uint32
rt_fea_get_route_v6_count_global()
{
    return route_count_v6;
}

uint32
rt_fea_route_v6_count_increase()
{
    return route_count_v6++;
}

uint32
rt_fea_route_v6_count_decrease()
{
    return route_count_v6--;
}

uint32
rt_fea_route_count_min_check()
{
    uint32 is_min = 0;

    if (1 > route_count)
    {
        is_min = 1;
    }

    return is_min;
}

uint32
rt_fea_get_route_count_global()
{
    return route_count;
}

uint32
rt_fea_route_count_increase()
{
    return route_count++;
}

uint32
rt_fea_route_count_decrease()
{
    return route_count--;
}

int32
_rt_fea_ipv4_forwarding_set(uint32 enable)
{
    FILE *fp;

    fp = sal_fopen(proc_ipv4_forwarding, "w");
    if (fp == NULL)
    {
        return PM_E_FAIL;
    }

    if (enable == 0)
    {
        sal_fprintf(fp, "0\n");
    }
    else
    {
        sal_fprintf(fp, "1\n");
    }

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
rt_fea_ipv4_forwarding_set(uint32 enable)
{
    return _rt_fea_ipv4_forwarding_set(enable);
}

int32
rt_fea_del_route_by_ifname(void *obj, tbl_iter_args_t *pargs)
{
    char                ifname[IFNAME_SIZE];
    tbl_iproute_node_t  *pst_iproute_node = obj;
    int                 ret = PM_E_NONE;
    uint32              i    = 0;
    addr_ipv4_t         gate_addr;
    uint32              ecmp_del    = 0;
    vrf_id_t            vrf_id;
    addr_ipv4_t         dest;
    addr_ipv4_t         gate;
    uint8               prefixlen   = 0;
    uint32              ifindex     = 0;
    uint32              distance    = 0;
    char                buf[CMD_BUF_64];
    tbl_route_if_t      *p_db_rtif = pargs->argv[0];

    if (NULL == pst_iproute_node || NULL == p_db_rtif)
    {
        return PM_E_FAIL;
    }

    sal_memset(&gate_addr, 0, sizeof(gate_addr));
    sal_memset(ifname, 0, sizeof(ifname));
    sal_memcpy(ifname, p_db_rtif->key.name, sizeof(ifname));
    sal_memset(buf, 0, sizeof(buf));

    ifindex = p_db_rtif->kernel_ifindex;

    if (0 == pst_iproute_node->multipath && NH_TYPE_IPV4 == pst_iproute_node->nh_key.type)
    {
        if (0 == sal_memcmp(pst_iproute_node->nh_key.ifname, ifname, sizeof(ifname)))
        {
            vrf_id = pst_iproute_node->key.vrf_id;
            dest.s_addr = pst_iproute_node->key.p.u.prefix4.s_addr;
            gate.s_addr  = pst_iproute_node->nh_key.u.ipv4.s_addr;
            prefixlen = pst_iproute_node->key.p.prefixlen;

            ret = rt_fea_ipv4_del_route_nl(vrf_id, &dest, &gate, prefixlen, ifindex, distance, RTN_UNICAST, ZEBRA_ROUTE_MAX);
        }

        return PM_E_NONE;
    }
    /* modified by wangjj for bug 42871, when nhg related to same interface, 2017-02-18 */
    else if (1 == pst_iproute_node->multipath)
    {
        for (i = 0; i < pst_iproute_node->nh_group.nh_num; i++)
        {
            if (sal_memcmp(pst_iproute_node->nh_group.nh[i].ifname, ifname, sizeof(ifname)))
            {
                ecmp_del = 1;
                break;
            }
        }

        if (ecmp_del == 0)
        {
            vrf_id = pst_iproute_node->key.vrf_id;
            dest.s_addr = pst_iproute_node->key.p.u.prefix4.s_addr;
            gate.s_addr  = pst_iproute_node->nh_key.u.ipv4.s_addr;
            prefixlen = pst_iproute_node->key.p.prefixlen;
            
            ret = rt_fea_ipv4_del_route_multi_nl(vrf_id, &dest, NULL,
                                pst_iproute_node->nh_group.nh_num, prefixlen, distance, ZEBRA_ROUTE_MAX);
        }
    }

    return ret;
}

int32
rt_fea_static_route_count(void *obj, tbl_iter_args_t *pargs)
{
    tbl_iproute_node_t *pst_iproute_node = obj;
    uint32 count = *(uint32 *) pargs->argv[0];

    if (NH_TYPE_IPV4 != pst_iproute_node->nh_key.type)
    {
        return PM_E_NONE;
    }
    else
    {
        count ++;
        pargs->argv[0] = &count;
    }

    /* rt_type check for next */

    return PM_E_NONE;
}

int32
rt_fea_arp_hw_syn(vrf_id_t vrf_id, addr_ipv4_t *dest, uint8 is_static, uint32 op_flag)
{
    tbl_arp_fib_t   *pst_arp_fib = NULL;
    tbl_arp_fib_t   arp_fib;
    int32           rc = PM_E_NONE;

    sal_memset(&arp_fib, 0, sizeof(arp_fib));
    sal_memcpy(&arp_fib.key.ip, dest, sizeof(addr_ipv4_t));
    pst_arp_fib = tbl_arp_fib_get_arp_fib(&arp_fib.key);
    if (ARP_SYN_ADD == op_flag)
    {
        if (NULL != pst_arp_fib && is_static == (ARP_TYPE_STATIC == pst_arp_fib->type))
        {
            rc = tbl_arp_fib_del_arp_fib(&pst_arp_fib->key);
        }

        sal_memcpy(arp_fib.ifname, pst_arp_fib->ifname, IFNAME_SIZE);
        sal_memcpy(arp_fib.mac_addr, pst_arp_fib->mac_addr, 6);
        arp_fib.type = pst_arp_fib->type;

        rc = tbl_arp_fib_add_arp_fib(&arp_fib);
    }

    if (ARP_SYN_DEL == op_flag)
    {
        if (NULL == pst_arp_fib)
        {
            return PM_E_NOT_EXIST;
        }

        if (is_static == (ARP_TYPE_STATIC == pst_arp_fib->type))
        {
            rc = tbl_arp_fib_del_arp_fib(&pst_arp_fib->key);
        }
    }

    return rc;
}

static int32
rt_fea_arp_fib_del_by_check_loop(tbl_arp_fib_t *pst_arp_fib, tbl_iter_args_t *pargs)
{
    tbl_kernel_if_t *pst_kernel_if = NULL;
    int32           rc = PM_E_NONE;

    if (NULL == pargs->argv[0]  || NULL == pst_arp_fib)
    {
        return PM_E_NONE;
    }

    pst_kernel_if = pargs->argv[0];
    if (0 == sal_memcmp(pst_arp_fib->ifname, pst_kernel_if->name, sizeof(pst_arp_fib->ifname)))
    {
        rc = tbl_arp_fib_del_arp_fib(&pst_arp_fib->key);
    }

    return rc;
}

int32
rt_fea_arp_add_static_by_loop(tbl_arp_t *pst_arp, tbl_iter_args_t *pargs)
{
    tbl_interface_t *p_db_if = NULL;
    ds_connected_t  *ifc = NULL;
    uint32          add_not_up = 0;
    int32           rc = PM_E_NONE;

    if (NULL == pargs->argv[0] || NULL == pargs->argv[1] || NULL == pargs->argv[2] || NULL == pst_arp)
    {
        return PM_E_FAIL;
    }

    ifc = pargs->argv[0];
    add_not_up = *((uint32 *)pargs->argv[1]);
    p_db_if = pargs->argv[2];
    if (NULL == ifc)
    {
        return PM_E_FAIL;
    }

    if (arp_api_rt_ifc_match(&pst_arp->key.ip, ifc)
            && ifc->key.address.u.prefix4.s_addr != pst_arp->key.ip.s_addr
            && ifc->destination.u.prefix4.s_addr != pst_arp->key.ip.s_addr
            && 0 != sal_strncmp(pst_arp->ifname, p_db_if->key.name, sizeof(pst_arp->ifname)))
    {
        if (add_not_up && ARP_TYPE_STATIC == pst_arp->type && !pst_arp->is_router)
        {
            rc = arp_db_synch_hw_os(pst_arp, ARP_SYN_DEL);

            sal_memcpy(pst_arp->ifname, p_db_if->key.name, sizeof(pst_arp->ifname));
            pst_arp->ifindex = p_db_if->ifindex;
            rc = tbl_arp_set_arp_field(pst_arp, TBL_ARP_FLD_IFNAME);
            rc = tbl_arp_set_arp_field(pst_arp, TBL_ARP_FLD_IFINDEX);
            rc = arp_db_synch_hw_os(pst_arp, ARP_SYN_ADD);
        }
    }

    return rc;
}

int32
rt_fea_arp_del_static_by_loop(tbl_arp_t *pst_arp, tbl_iter_args_t *pargs)
{
    tbl_kernel_if_t *pst_kernel_if = NULL;
    int32           rc = PM_E_NONE;

    if (NULL == pargs->argv[0]  || NULL == pst_arp)
    {
        return PM_E_NONE;
    }

    pst_kernel_if = pargs->argv[0];
    if (0 == sal_strncmp(pst_arp->ifname, pst_kernel_if->name, sizeof(pst_arp->ifname)))
    {
        if (ARP_TYPE_STATIC == pst_arp->type && pst_arp->is_router)
        {
            rc = arp_db_entry_remove(pst_arp);
        }
        else if (ARP_TYPE_STATIC == pst_arp->type)
        {
            sal_memset(pst_arp->ifname, 0, sizeof(pst_arp->ifname));
            tbl_arp_set_arp_field(pst_arp, TBL_ARP_FLD_IFNAME);
            rc = arp_db_synch_hw_os(pst_arp, ARP_SYN_DEL);
        }
    }

    return rc;
}

static int32
rt_fea_arp_del_dynamic_by_loop(tbl_arp_t *pst_arp, tbl_iter_args_t *pargs)
{
    tbl_kernel_if_t *pst_kernel_if = NULL;
    tbl_arp_t       arp_syn;
    int32           rc = PM_E_NONE;

    sal_memset(&arp_syn, 0, sizeof(tbl_arp_t));
    if (NULL == pargs->argv[0]  || NULL == pst_arp)
    {
        return PM_E_NONE;
    }

    pst_kernel_if = pargs->argv[0];
    if (0 == sal_strncmp(pst_arp->ifname, pst_kernel_if->name, sizeof(pst_arp->ifname)))
    {
        if (ARP_TYPE_DYNAMIC == pst_arp->type)
        {
            sal_memcpy(&arp_syn, pst_arp, sizeof(tbl_arp_t));
            rc = rt_fea_arp_hw_syn(arp_syn.vrf_id, &arp_syn.key.ip,
                            (ARP_TYPE_STATIC == arp_syn.type), ARP_SYN_DEL);

            //rt_api_add_static_route_to_frr_when_del_arp(pst_arp);
            rc = tbl_arp_del_arp(&pst_arp->key);
        }
    }

    return rc;
}

#define rt_fea_ipv6_update_nh_by_group rt_fea_ipv4_update_nh_by_group
#define rt_fea_ipv6_del_nhg rt_fea_ipv4_del_nhg
#define rt_fea_ipv6_add_nhg rt_fea_ipv4_add_nhg

int32
rt_fea_ipv6_add_connect_route_by_link(vrf_id_t vrf_id, addr_ipv6_t *dest, uint8 prefixlen, char *ifname)
{
    tbl_iproute_node_t  *pst_iproute_node = NULL;
    tbl_iproute_node_t  iproute_node;
    uint32 over_check = 0;
    int32 rc = PM_E_NONE;

    if (NULL == dest)
    {
        return PM_E_FAIL;
    }

    /* 1. get route  */
    sal_memset(&iproute_node, 0, sizeof(iproute_node));
    iproute_node.key.vrf_id = vrf_id;
    iproute_node.key.p.family = AF_INET6;
    iproute_node.key.p.prefixlen = prefixlen;
    iproute_node.distance = 1;
    sal_memcpy (&iproute_node.key.p.u.prefix6, dest, sizeof(addr_ipv6_t));
    sal_memcpy (iproute_node.nh_key.ifname, ifname, sizeof(iproute_node.nh_key.ifname));
    iproute_node.nh_key.type = NH_TYPE_LOCAL;
    pst_iproute_node = tbl_iproute_node_get_iproute_node(&iproute_node.key);
    if ((NULL != pst_iproute_node)
        /* modified by liwh for bug 45943, 2017-12-22 */
        && (NH_TYPE_LOCAL == pst_iproute_node->nh_key.type))
        /* liwh end */
    {
        return PM_E_EXIST;
    }

    /* modified by liwh for bug 45943, 2017-12-22 */
    if (pst_iproute_node)
    {
        /*del old nexthop*/
        addr_t dest_all = {0};
        dest_all.family = AF_INET6;
        sal_memcpy(&dest_all.u.prefix6, dest, sizeof(addr_ipv6_t));
        rt_fea_ipv6_update_nh_by_group(vrf_id, &dest_all, &pst_iproute_node->nh_key, 0, prefixlen, pst_iproute_node->distance);
        
        /*del nexthop group if need*/
        if(pst_iproute_node->multipath)
        {
            PM_E_RETURN(rt_fea_ipv6_del_nhg(pst_iproute_node, NULL, pst_iproute_node->nh_group.nh_num, 0, FALSE));
        }
            
        rc = tbl_iproute_node_del_iproute_node(&pst_iproute_node->key);
        rt_fea_route_v6_count_decrease();
        rt_route_v6_counter_dec(DEFAULT_VRF_ID);
    }
    /* liwh end */

    /* 2. add route */
    over_check = rt_fea_indirect_route_v6_count_max_check();
    if (over_check)
    {
        return PM_E_NONE;
    }

    rc = tbl_iproute_node_add_iproute_node(&iproute_node);
    if (!rc)
    {
        rt_fea_route_v6_count_increase();
        rt_route_v6_counter_inc(DEFAULT_VRF_ID);
    }

    return rc;
}

int32
rt_fea_ipv6_del_connect_route_by_link(vrf_id_t vrf_id, addr_ipv6_t *dest, uint8 prefixlen, char *ifname)
{
    tbl_iproute_node_t  *pst_iproute_node   = NULL;
    tbl_iproute_node_t iproute_node;
    int32 rc = PM_E_NONE;

    if (NULL == dest)
    {
        return PM_E_FAIL;
    }

    /* 1. del route */
    if (rt_fea_route_v6_count_min_check())
    {
        DEBUG_ROUTE_IPV6("The route v6 entry num is less than minimum!");
        return PM_E_NONE;
    }

    sal_memset(&iproute_node, 0, sizeof(iproute_node));
    iproute_node.key.vrf_id = vrf_id;
    iproute_node.key.p.family = AF_INET6;
    iproute_node.key.p.prefixlen = prefixlen;
    sal_memcpy (&iproute_node.key.p.u.prefix6, dest, sizeof(addr_ipv6_t));
    pst_iproute_node = tbl_iproute_node_get_iproute_node(&iproute_node.key);
    if (NULL != pst_iproute_node)
    {
        if (NH_TYPE_LOCAL!= pst_iproute_node->nh_key.type 
            || sal_memcmp(pst_iproute_node->nh_key.ifname, ifname, sizeof(pst_iproute_node->nh_key.ifname)))
        {
            return PM_E_NONE;
        }

        rc = tbl_iproute_node_del_iproute_node(&pst_iproute_node->key);
        rt_fea_route_v6_count_decrease();
        rt_route_v6_counter_dec(DEFAULT_VRF_ID);
    }
    else
    {
        return PM_E_NONE;
    }

    return rc;
}

void
rt_fea_clear_nexthop_group_ifname(nexthop_ecmp_routed_t *nexthop_ecmp_routed)
{
    int32 index = 0;
    for(index = 0; index < nexthop_ecmp_routed->nh_num; index++)
    {
        sal_memset(nexthop_ecmp_routed->nh[index].ifname, 0x00, sizeof(nexthop_ecmp_routed->nh[index].ifname));
    }
}

int32
rt_fea_ipv4_add_connect_route_by_link(vrf_id_t vrf_id, addr_ipv4_t *dest, uint8 prefixlen, char *ifname)
{
    tbl_iproute_node_t  *pst_iproute_node = NULL;
    tbl_iproute_node_t  iproute_node;
    uint32 over_check = 0;
    int32 rc = PM_E_NONE;

    if (NULL == dest)
    {
        return PM_E_FAIL;
    }

    /* 1. get route  */
    sal_memset(&iproute_node, 0, sizeof(iproute_node));
    iproute_node.key.vrf_id = vrf_id;
    iproute_node.key.p.family = AF_INET;
    iproute_node.key.p.prefixlen = prefixlen;
    iproute_node.distance = 1;
    sal_memcpy (&iproute_node.key.p.u.prefix4, dest, sizeof(addr_ipv4_t));
    sal_memcpy (iproute_node.nh_key.ifname, ifname, sizeof(iproute_node.nh_key.ifname));
    iproute_node.nh_key.type = NH_TYPE_LOCAL;
    pst_iproute_node = tbl_iproute_node_get_iproute_node(&iproute_node.key);
    if ((NULL != pst_iproute_node)
        /* modified by liwh for bug 45943, 2017-12-22 */
        && (NH_TYPE_LOCAL == pst_iproute_node->nh_key.type))
        /* liwh end */
    {
        return PM_E_EXIST;
    }

    /* modified by liwh for bug 45943, 2017-12-22 */
    if (pst_iproute_node)
    {
        /*del old nexthop*/
        addr_t dest_all = {0};
        dest_all.family = AF_INET;
        sal_memcpy(&dest_all.u.prefix4, dest, sizeof(addr_ipv4_t));
        rt_fea_ipv4_update_nh_by_group(vrf_id, &dest_all, &pst_iproute_node->nh_key, 0, prefixlen, pst_iproute_node->distance);
        
        /*del nexthop group if need*/
        if(pst_iproute_node->multipath)
        {
            PM_E_RETURN(rt_fea_ipv4_del_nhg(pst_iproute_node, NULL, pst_iproute_node->nh_group.nh_num, 0, TRUE));
            if (ZEBRA_ROUTE_OSPF == pst_iproute_node->route_type)
            {
                nexthop_ecmp_routed_t nexthop_ecmp_routed = {0};
                sal_memset(&nexthop_ecmp_routed, 0x00, sizeof(nexthop_ecmp_routed));

                nexthop_ecmp_routed.nh_num = pst_iproute_node->nh_group.nh_num;
                sal_memcpy(nexthop_ecmp_routed.nh, pst_iproute_node->nh_group.nh, pst_iproute_node->nh_group.nh_num*sizeof(nexthop_key_t));
                rt_fea_clear_nexthop_group_ifname(&nexthop_ecmp_routed);
                route_cmd_ipv4_del_nhg(&nexthop_ecmp_routed, TRUE);
            }
        }
            
        rc = tbl_iproute_node_del_iproute_node(&pst_iproute_node->key);
        rt_fea_route_count_decrease();
        rt_route_counter_dec(DEFAULT_VRF_ID);
    }
    /* liwh end */

    /* 2. add route */
    over_check = rt_fea_indirect_route_count_max_check();
    if (over_check)
    {
        return PM_E_NONE;
    }

    rc = tbl_iproute_node_add_iproute_node(&iproute_node);
    if (!rc)
    {
        rt_fea_route_count_increase();
        rt_route_counter_inc(DEFAULT_VRF_ID);
    }

    return rc;
}

int32
rt_fea_ipv4_del_connect_route_by_link(vrf_id_t vrf_id, addr_ipv4_t *dest, uint8 prefixlen, char *ifname)
{
    tbl_iproute_node_t  *pst_iproute_node   = NULL;
    tbl_iproute_node_t iproute_node;
    int32 rc = PM_E_NONE;

    if (NULL == dest)
    {
        return PM_E_FAIL;
    }

    /* 1. del route */
    if (rt_fea_route_count_min_check())
    {
        DEBUG_ROUTE_IPV4("The route entry num is less than minimum!");
        return PM_E_NONE;
    }

    sal_memset(&iproute_node, 0, sizeof(iproute_node));
    iproute_node.key.vrf_id = vrf_id;
    iproute_node.key.p.family = AF_INET;
    iproute_node.key.p.prefixlen = prefixlen;
    sal_memcpy (&iproute_node.key.p.u.prefix4, dest, sizeof(addr_ipv4_t));
    pst_iproute_node = tbl_iproute_node_get_iproute_node(&iproute_node.key);
    if (NULL != pst_iproute_node)
    {
        if (NH_TYPE_LOCAL!= pst_iproute_node->nh_key.type 
            || sal_memcmp(pst_iproute_node->nh_key.ifname, ifname, sizeof(pst_iproute_node->nh_key.ifname)))
        {
            return PM_E_NONE;
        }

        rc = tbl_iproute_node_del_iproute_node(&pst_iproute_node->key);
        rt_fea_route_count_decrease();
        rt_route_counter_dec(DEFAULT_VRF_ID);
    }
    else
    {
        return PM_E_NONE;
    }

    return rc;
}

int32
rt_fea_del_arp_dynamic(tbl_kernel_if_t *p_db_kernel_if, char *ifname)
{
    tbl_iter_args_t     args;

    sal_memset(&args, 0, sizeof(args));
    args.argv[0] = p_db_kernel_if;
    tbl_arp_iterate((TBL_ITER_CB_FUNC)rt_fea_arp_del_dynamic_by_loop, &args);

    return PM_E_NONE;
}

int32
rt_fea_del_arp_static(tbl_kernel_if_t *p_db_kernel_if, char *ifname)
{
    tbl_iter_args_t     args;

    sal_memset(&args, 0, sizeof(args));
    args.argv[0] = p_db_kernel_if;
    tbl_arp_iterate((TBL_ITER_CB_FUNC)rt_fea_arp_del_static_by_loop, &args);

    return PM_E_NONE;
}

int32
rt_fea_del_arp_related(tbl_kernel_if_t *p_db_kernel_if, char *ifname)
{
    int32 rc = PM_E_NONE;

    rc = rt_fea_del_arp_static(p_db_kernel_if, ifname);
    rc = rt_fea_del_arp_dynamic(p_db_kernel_if, ifname);

    return rc;
}

int32
rt_fea_del_arp_fib(tbl_kernel_if_t *p_db_kernel_if, char *ifname)
{
    tbl_interface_t     *pst_interface = NULL;
    tbl_interface_key_t p_if_key ;
    tbl_iter_args_t     args;

    sal_memset(&p_if_key, 0, sizeof(p_if_key));
    sal_strcpy(p_if_key.name, ifname);

    pst_interface = tbl_interface_get_interface(&p_if_key);
    if (NULL != pst_interface)
    {
        sal_memset(&args, 0, sizeof(args));
        args.argv[0] = p_db_kernel_if;

        tbl_arp_fib_iterate((TBL_ITER_CB_FUNC)rt_fea_arp_fib_del_by_check_loop, &args);
    }

    return PM_E_NONE;
}

int32
rt_fea_ipv4_add_neighbor(vrf_id_t vrf_id, addr_ipv4_t *dest, uint8 *macaddr,
                uint32 ifindex, uint8 is_static, char *if_name)
{
    tbl_nexthop_t       *pst_nexthop    = NULL;    
    tbl_nexthop_t       *p_nexthop = NULL;
    tbl_arp_fib_t       *p_arp_fib = NULL;
    tbl_route_if_t      *p_rt_if = NULL;
    tbl_arp_fib_t       arp_fib;
    tbl_route_if_key_t  rtif_key;
    int32               rc = PM_E_NONE;
    char buf1[CMD_BUF_128] = {0}; 

    /* 1. get route_if by intf name */
    sal_memset(&rtif_key, 0, sizeof(rtif_key));
    sal_memcpy(rtif_key.name, if_name, sizeof(rtif_key.name));
    p_rt_if = tbl_route_if_get_route_if(&rtif_key);
    if (NULL == p_rt_if)
    {
        return PM_E_NOT_EXIST;
    }

    sal_memset(&arp_fib, 0, sizeof(arp_fib));
    sal_memcpy(&arp_fib.key.ip, dest, sizeof(addr_ipv4_t));
    sal_memcpy(arp_fib.ifname, rtif_key.name, IFNAME_SIZE);
    sal_memcpy(arp_fib.mac_addr, macaddr, 6);

    if (is_static)
    {
        arp_fib.type = ARP_TYPE_STATIC;
    }
    else
    {
        arp_fib.type = ARP_TYPE_DYNAMIC;
    }

    /* 2. get arp */
    p_arp_fib = tbl_arp_fib_get_arp_fib(&arp_fib.key);
    if (NULL != p_arp_fib && ARP_TYPE_DYNAMIC == p_arp_fib->type)
    {
        p_nexthop = rt_fea_nexthop_ipv4_get(rtif_key.name, dest);
        if (NULL == p_nexthop)
        {
            /*add by chenc for bug 45556*/
            log_sys(M_MOD_ROUTE, E_ERROR, "%s %d name:%s dest:%s, error\n", __func__, __LINE__, rtif_key.name, cdb_addr_ipv4_val2str(buf1, CMD_BUF_128, dest));
            return PM_E_FAIL;
            /*chenc end*/
            p_nexthop = rt_fea_nexthop_ipv4_add(rtif_key.name, dest);
            if (NULL == p_nexthop)
            {
                return PM_E_FAIL;
            }
        }
        /*add by chenc for bug 45556*/
        /*else
        {
            p_nexthop = rt_fea_nexthop_ipv4_add(rtif_key.name, dest);
        }*/
        /*chenc end*/

        if (arp_fib.type != p_arp_fib->type
                    || 0 != sal_memcmp(p_arp_fib->mac_addr, macaddr, sizeof(mac_addr_t)))
        {
            rc = tbl_arp_fib_del_arp_fib(&p_arp_fib->key);
            rc = tbl_arp_fib_add_arp_fib(&arp_fib);
        }
    }
    else if (NULL == p_arp_fib)
    {
        /* 3. add nexthop */
        pst_nexthop = rt_fea_nexthop_ipv4_add(arp_fib.ifname, dest);
        if (NULL == pst_nexthop)
        {
            return PM_E_FAIL;
        }

        rc = tbl_arp_fib_add_arp_fib(&arp_fib);
    }

    return rc;
}

int32
rt_fea_ipv4_del_neighbor(vrf_id_t vrf_id, addr_ipv4_t *dest, uint8 is_static)
{
    tbl_arp_fib_t   *pst_arp_fib    = NULL;
    tbl_nexthop_t   *pst_nexthop    = NULL;
    tbl_arp_fib_t   arp_fib;
    char            ifname[IFNAME_SIZE];
    int32           rc = PM_E_NONE;

    sal_memset(&arp_fib, 0, sizeof(arp_fib));
    sal_memset(ifname, 0, sizeof(ifname));

    sal_memcpy(&arp_fib.key.ip, dest, sizeof(addr_ipv4_t));
    pst_arp_fib = tbl_arp_fib_get_arp_fib(&arp_fib.key);
    if (NULL == pst_arp_fib)
    {
        return PM_E_NOT_EXIST;
    }

    sal_strcpy(ifname, pst_arp_fib->ifname);
    if (NULL != pst_arp_fib)
    {
        rc = tbl_arp_fib_del_arp_fib(&pst_arp_fib->key);
        pst_nexthop = rt_fea_nexthop_ipv4_get(ifname, dest);
        if (NULL != pst_nexthop)
        {
             rc = rt_fea_nexthop_ipv4_del(ifname, dest);
        }
    }

    return rc;
}

/* deleted arp and nexthop relation by liwh for bug 45444, 2017-10-26 */
#if 0
int32
rt_fea_arp_ipv4_add_neighbor(vrf_id_t vrf_id, addr_ipv4_t *dest, uint8 *macaddr, uint32 ifindex, uint8 is_static)
{
    tbl_nexthop_t   *pst_nexthop    = NULL;
    tbl_nexthop_t   *p_nexthop = NULL;
    tbl_kernel_if_t *p_db_kernel_if = NULL;
    tbl_interface_t *pst_interface = NULL;
    tbl_kernel_if_t kernel_if;
    tbl_arp_t       *p_arp = NULL;
    tbl_arp_t       arp;
    tbl_arp_t       arp_syn;
    int32           rc = PM_E_NONE;

    /* 1. get ifname by kernel ifindex */
    sal_memset(&kernel_if, 0, sizeof(kernel_if));
    sal_memset(&arp_syn, 0, sizeof(tbl_arp_t));

    pst_interface = tbl_interface_get_interface_by_ifindex(ifindex);
    if (NULL == pst_interface)
    {
        return PM_E_NONE;
    }

    p_db_kernel_if = tbl_kernel_if_get_by_name(pst_interface->key.name);
    if (NULL == p_db_kernel_if)
    {
        return PM_E_NONE;
    }

    sal_memset(&arp, 0, sizeof(tbl_arp_t));
    sal_memcpy(&arp.key.ip, dest, sizeof(addr_ipv4_t));
    sal_memcpy(arp.ifname, p_db_kernel_if->name, IFNAME_SIZE);
    sal_memcpy(arp.mac_addr, macaddr, 6);

    if (is_static)
    {
        arp.type = ARP_TYPE_STATIC;
    }
    else
    {
        arp.type = ARP_TYPE_DYNAMIC;
    }

    /* 2. get arp */
    p_arp = tbl_arp_get_arp(&arp.key);
    if (NULL != p_arp)
    {

        p_nexthop = rt_fea_nexthop_ipv4_get(p_db_kernel_if->name, dest);
        if (NULL == p_nexthop)
        {
            p_nexthop = rt_fea_nexthop_ipv4_add(p_db_kernel_if->name, dest);
            if (NULL == p_nexthop)
            {
                return PM_E_FAIL;
            }
        }
        else
        {
            p_nexthop = rt_fea_nexthop_ipv4_add(p_db_kernel_if->name, dest);
        }

        if (arp.type != p_arp->type || 0 != sal_memcmp(p_arp->mac_addr, macaddr, sizeof(mac_addr_t)))
        {

            sal_memcpy(&arp_syn, p_arp, sizeof(tbl_arp_t));
            rc = rt_fea_arp_hw_syn(arp_syn.vrf_id, &arp_syn.key.ip, (ARP_TYPE_STATIC == arp_syn.type), ARP_SYN_DEL);
        }

        rc = rt_fea_arp_hw_syn(p_arp->vrf_id, &p_arp->key.ip, (ARP_TYPE_STATIC == p_arp->type), ARP_SYN_ADD);
    }
    else
    {
        /* 3. add nexthop */
        pst_nexthop = rt_fea_nexthop_ipv4_add(arp.ifname, dest);
        if (NULL == pst_nexthop)
        {
            return PM_E_FAIL;
        }

        rc = rt_fea_arp_hw_syn(p_arp->vrf_id, &p_arp->key.ip, (ARP_TYPE_STATIC == p_arp->type), ARP_SYN_ADD);
    }

    return rc;
}

int32
rt_fea_arp_ipv4_del_neighbor(vrf_id_t vrf_id, addr_ipv4_t *dest, uint8 is_static)
{
    tbl_nexthop_t   *pst_nexthop    = NULL;
    tbl_arp_t       *p_arp          = NULL;
    tbl_arp_t       arp;
    tbl_arp_t       arp_syn;
    char            ifname[IFNAME_SIZE];
    int32           rc   = PM_E_NONE;

    sal_memset(&arp, 0, sizeof(tbl_arp_t));
    sal_memset(ifname, 0, sizeof(ifname));

    sal_memcpy(&arp.key.ip, dest, sizeof(addr_ipv4_t));
    p_arp = tbl_arp_get_arp(&arp.key);
    if (NULL == p_arp)
    {
        return PM_E_NONE;
    }

    sal_strcpy(ifname, p_arp->ifname);
    pst_nexthop = rt_fea_nexthop_ipv4_get(ifname, dest);
    if (NULL != pst_nexthop)
    {
        rc = rt_fea_nexthop_ipv4_del(ifname, dest);
    }

    sal_memset(&arp_syn, 0, sizeof(tbl_arp_t));
    sal_memcpy(&arp_syn, p_arp, sizeof(tbl_arp_t));
    rc = rt_fea_arp_hw_syn(arp_syn.vrf_id, &arp_syn.key.ip, (ARP_TYPE_STATIC == arp_syn.type), ARP_SYN_DEL);

    return rc;
}
#endif
/* liwh end */

/*set if up*/
int32
rt_fea_update_if_up_by_addr(tbl_interface_t *p_db_if)
{
    int32           rc = PM_E_NONE;
    int32           is_up = 0;

    is_up = tbl_interface_is_up(p_db_if);
    rc = lib_ioctl_if_set_up(p_db_if->key.name, is_up);

    return rc;
}

int32
rt_fea_update_if_running_by_newlink(tbl_interface_t *p_db_if, uint32 ifindex)
{
    int32           rc = PM_E_NONE;

    if (GLB_IF_TYPE_VLAN_IF == p_db_if->hw_type || GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type
        || GLB_IF_TYPE_PORT_IF == p_db_if->hw_type)
    {
        /*modified by chenc for remove patch function, 2019-05-11*/
        #ifndef USE_FPM_IPUC
        uint32  is_running = tbl_interface_is_running(p_db_if);
        rc = lib_netlink_link_set(ifindex, is_running);
        #endif
    }

    return rc;
}

int32
rt_fea_add_newlink(char *ifname, uint32 ifindex)
{
    tbl_kernel_if_t *p_db_kernel_if = NULL;
    tbl_kernel_if_t kernel_if;
    tbl_interface_t *p_db_if = NULL;
    int32           rc = PM_E_NONE;
    tbl_route_if_t  *p_db_rtif  = NULL;

    /* modified by liwh for bug 48318, 2018-08-01 */
    p_db_kernel_if = tbl_kernel_if_get_by_name(ifname);
    if (p_db_kernel_if)
    {
        if (p_db_kernel_if->key.ifindex != ifindex)
        {
            tbl_kernel_if_del_kernel_if(&p_db_kernel_if->key);
        }
    }
    /* liwh end */
        
    sal_memset(&kernel_if, 0, sizeof(kernel_if));
    kernel_if.key.ifindex = ifindex;
    p_db_kernel_if = tbl_kernel_if_get_kernel_if(&kernel_if.key);
    if (NULL == p_db_kernel_if)
    {
        sal_strcpy(kernel_if.name, ifname);
        tbl_kernel_if_add_kernel_if(&kernel_if);

        p_db_if = tbl_interface_get_interface_by_name(ifname);
        if (NULL == p_db_if)
        {
            return PM_E_FAIL;
        }

        p_db_rtif = tbl_route_if_get_route_if((tbl_route_if_key_t*)(&p_db_if->key));
        if (NULL != p_db_rtif)
        {
            p_db_rtif->kernel_ifindex = ifindex;
            tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_KERNEL_IFINDEX);
        }

        rc = rt_fea_update_if_running_by_newlink(p_db_if, ifindex);
        
        uint32 is_up_running = lib_ioctl_if_get_flags_and_check(ifname);
        rt_if_running_update(ifname, is_up_running);
        return rc;
    }
    else
    {
        p_db_if = tbl_interface_get_interface_by_name(ifname);
        if (NULL == p_db_if)
        {
            return PM_E_FAIL;
        }

        p_db_rtif = tbl_route_if_get_route_if((tbl_route_if_key_t*)(&p_db_if->key));
        if (NULL != p_db_rtif && p_db_rtif->kernel_ifindex != ifindex)
        {
            p_db_rtif->kernel_ifindex = ifindex;
            tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_KERNEL_IFINDEX);
        }

        /* modified by wangjj for fix bug 43648, 2017-04-14 */
        #ifdef _CENTEC_
        uint32 is_up_running = lib_ioctl_if_get_flags_and_check(p_db_kernel_if->name);
        rt_if_running_update(ifname, is_up_running);
        // rt_if_ipv4_del_related_route_by_link(p_db_if, p_db_rtif, is_up_running);
        #endif
    }

    /* Check if the interface is create or state update */
    if (0 == p_db_kernel_if->ip_addr.s_addr || (32 < p_db_kernel_if->masklen && 0 > p_db_kernel_if->masklen))
    {
        return PM_E_NONE;
    }

    return PM_E_NONE;
}

int32
rt_fea_del_newlink(char *ifname, uint32 ifindex)
{
    tbl_kernel_if_t *p_db_kernel_if = NULL;
    tbl_kernel_if_t kernel_if;

    sal_memset(&kernel_if, 0, sizeof(kernel_if));
    kernel_if.key.ifindex = ifindex;
    p_db_kernel_if = tbl_kernel_if_get_kernel_if(&kernel_if.key);
    if (NULL == p_db_kernel_if)
    {
        return PM_E_NONE;
    }

    //rt_if_ipv4_del_related_route_by_link(p_db_kernel_if, FALSE, 0);
    tbl_kernel_if_del_kernel_if(&kernel_if.key);

    return PM_E_NONE;
}

tbl_nexthop_t *
rt_fea_nexthop_ifname_add(char *ifname)
{
    tbl_nexthop_t *pst_nexthop = NULL;
    tbl_nexthop_t nexthop;
    int ret = PM_E_NONE;

    sal_memset(&nexthop, 0, sizeof(nexthop));
    nexthop.key.type = NH_TYPE_IFINDEX;
    sal_strcpy(nexthop.key.ifname, ifname);

    pst_nexthop = tbl_nexthop_get_nexthop(&nexthop.key);
    if(NULL == pst_nexthop)
    {
        nexthop.refcnt = 1;
        ret = tbl_nexthop_add_nexthop(&nexthop);
        if(PM_E_NONE == ret)
        {
            return tbl_nexthop_get_nexthop(&nexthop.key);
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        pst_nexthop->refcnt++;
        tbl_nexthop_set_nexthop_field_sync(pst_nexthop, TBL_NEXTHOP_FLD_REFCNT, FALSE);
    }

    return pst_nexthop;
}

int32
rt_fea_nexthop_ifname_del(char *ifname)
{
    tbl_nexthop_t   *pst_nexthop = NULL;
    tbl_nexthop_t   nexthop;
    int             ret          = PM_E_NONE;

    sal_memset(&nexthop, 0, sizeof(nexthop));
    nexthop.key.type = NH_TYPE_IFINDEX;
    sal_strcpy(nexthop.key.ifname, ifname);

    pst_nexthop = tbl_nexthop_get_nexthop(&nexthop.key);
    if(NULL == pst_nexthop)
    {
        return PM_E_NOT_EXIST;
    }

    pst_nexthop->refcnt--;

    if (0 == pst_nexthop->refcnt)
    {
        ret = tbl_nexthop_del_nexthop(&nexthop.key);
    }
    else
    {
        ret = tbl_nexthop_set_nexthop_field_sync(pst_nexthop, TBL_NEXTHOP_FLD_REFCNT, FALSE);
    }

    return ret;
}

#ifdef USE_FPM_IPUC
tbl_nexthop_t *
rt_fea_nexthop_ipv4_get_for_use_fpm_ipuc(nexthop_key_t *nh_key)
{
    tbl_nexthop_t nexthop;

    sal_memset(&nexthop, 0x00, sizeof(nexthop));
    nexthop.key.type = nh_key->type;
    sal_strcpy(nexthop.key.ifname, nh_key->ifname);
    sal_memcpy(&nexthop.key.u.ipv4, &nh_key->u.ipv4, sizeof(addr_ipv4_t));

    return tbl_nexthop_get_nexthop(&nexthop.key);
}

tbl_nexthop_group_t *
rt_fea_nexthop_group_ipv4_get_for_use_fpm_ipuc(tbl_iproute_node_t *p_iproute_node)
{
    tbl_nexthop_group_t nhgroup;

    sal_memset(&nhgroup, 0, sizeof(nhgroup));

    nhgroup.key.rr_en = p_iproute_node->nh_group.rr_en;
    nhgroup.key.nh_num = p_iproute_node->nh_group.nh_num;
    sal_memcpy(nhgroup.key.nh, &p_iproute_node->nh_group.nh, nhgroup.key.nh_num*sizeof(nexthop_key_t));

    return tbl_nexthop_group_get_nexthop_group(&nhgroup.key);
}
#endif

tbl_nexthop_t *
rt_fea_nexthop_ipv4_get(char *ifname, addr_ipv4_t *ipv4)
{
    tbl_nexthop_t nexthop;

    sal_memset(&nexthop, 0, sizeof(nexthop));
    nexthop.key.type = NH_TYPE_IPV4;
    sal_strcpy(nexthop.key.ifname, ifname);
    sal_memcpy(&nexthop.key.u.ipv4, ipv4, sizeof(addr_ipv4_t));

    return tbl_nexthop_get_nexthop(&nexthop.key);
}

tbl_nexthop_t *
rt_fea_nexthop_ipv4_add(char *ifname, addr_ipv4_t *ipv4)
{
    tbl_nexthop_t   *pst_nexthop = NULL;
    tbl_nexthop_t   nexthop;
    int             ret = PM_E_NONE;

    sal_memset(&nexthop, 0, sizeof(nexthop));
    nexthop.key.type = NH_TYPE_IPV4;
    sal_memcpy(nexthop.key.ifname, ifname, sizeof(nexthop.key.ifname));
    sal_memcpy(&nexthop.key.u.ipv4, ipv4, sizeof(addr_ipv4_t));

    pst_nexthop = tbl_nexthop_get_nexthop(&nexthop.key);
    if(NULL == pst_nexthop)
    {
        nexthop.refcnt = 1;
        ret = tbl_nexthop_add_nexthop(&nexthop);
        if(PM_E_NONE == ret)
        {
            return tbl_nexthop_get_nexthop(&nexthop.key);
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        pst_nexthop->refcnt++;
        tbl_nexthop_set_nexthop_field_sync(pst_nexthop, TBL_NEXTHOP_FLD_REFCNT, FALSE);
    }

    return pst_nexthop;
}

#ifdef USE_FPM_IPUC
int32
rt_fea_nexthop_ipv4_del_for_use_fpm_ipuc(tbl_nexthop_t *pst_nexthop)
{
    int             ret = PM_E_NONE;

    if(NULL == pst_nexthop)
    {
        return PM_E_NOT_EXIST;
    }

    pst_nexthop->refcnt--;

    if (0 == pst_nexthop->refcnt)
    {
        ret = tbl_nexthop_del_nexthop(&pst_nexthop->key);
    }
    else
    {
        ret = tbl_nexthop_set_nexthop_field_sync(pst_nexthop, TBL_NEXTHOP_FLD_REFCNT, FALSE);
    }

    return ret;
}
#endif

int32
rt_fea_nexthop_ipv4_del(char *ifname, addr_ipv4_t *ipv4)
{
    tbl_nexthop_t   *pst_nexthop = NULL;
    tbl_nexthop_t   nexthop;
    int             ret = PM_E_NONE;

    sal_memset(&nexthop, 0, sizeof(nexthop));
    nexthop.key.type = NH_TYPE_IPV4;
    sal_memcpy(nexthop.key.ifname, ifname, sizeof(nexthop.key.ifname));
    sal_memcpy(&nexthop.key.u.ipv4, ipv4, sizeof(addr_ipv4_t));

    pst_nexthop = tbl_nexthop_get_nexthop(&nexthop.key);
    if(NULL == pst_nexthop)
    {
        return PM_E_NOT_EXIST;
    }

    pst_nexthop->refcnt--;

    if (0 == pst_nexthop->refcnt)
    {
        ret = tbl_nexthop_del_nexthop(&nexthop.key);
    }
    else
    {
        ret = tbl_nexthop_set_nexthop_field_sync(pst_nexthop, TBL_NEXTHOP_FLD_REFCNT, FALSE);
    }

    return ret;
}

#ifdef USE_FPM_IPUC
tbl_nexthop_t *
rt_fea_nexthop_ipv6_get_for_use_fpm_ipuc(nexthop_key_t *nh_key)
{
    tbl_nexthop_t nexthop;

    sal_memset(&nexthop, 0x00, sizeof(nexthop));
    nexthop.key.type = nh_key->type;
    sal_strcpy(nexthop.key.ifname, nh_key->ifname);
    sal_memcpy(&nexthop.key.u.ipv6, &nh_key->u.ipv6, sizeof(addr_ipv6_t));

    return tbl_nexthop_get_nexthop(&nexthop.key);
}
#endif

tbl_nexthop_t *
rt_fea_nexthop_ipv6_get(char *ifname, addr_ipv6_t *ipv6)
{
    tbl_nexthop_t nexthop;

    sal_memset(&nexthop, 0, sizeof(nexthop));
    nexthop.key.type = NH_TYPE_IPV6;
    sal_strcpy(nexthop.key.ifname, ifname);
    sal_memcpy(&nexthop.key.u.ipv6, ipv6, sizeof(addr_ipv6_t));

    return tbl_nexthop_get_nexthop(&nexthop.key);
}

tbl_nexthop_t *
rt_fea_nexthop_ipv6_add(char *ifname, addr_ipv6_t *ipv6)
{
    tbl_nexthop_t   *pst_nexthop = NULL;
    tbl_nexthop_t   nexthop;
    int                   ret = PM_E_NONE;

    sal_memset(&nexthop, 0, sizeof(nexthop));
    nexthop.key.type = NH_TYPE_IPV6;
    sal_memcpy(nexthop.key.ifname, ifname, sizeof(nexthop.key.ifname));
    sal_memcpy(&nexthop.key.u.ipv6, ipv6, sizeof(addr_ipv6_t));

    pst_nexthop = tbl_nexthop_get_nexthop(&nexthop.key);
    if(NULL == pst_nexthop)
    {
        nexthop.refcnt = 1;
        ret = tbl_nexthop_add_nexthop(&nexthop);
        if(PM_E_NONE == ret)
        {
            return tbl_nexthop_get_nexthop(&nexthop.key);
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        pst_nexthop->refcnt++;
        tbl_nexthop_set_nexthop_field_sync(pst_nexthop, TBL_NEXTHOP_FLD_REFCNT, FALSE);
    }

    return pst_nexthop;
}

#ifdef USE_FPM_IPUC
int32
rt_fea_nexthop_ipv6_del_for_use_fpm_ipuc(tbl_nexthop_t *pst_nexthop)
{
    int             ret = PM_E_NONE;
    
    if(NULL == pst_nexthop)
    {
        return PM_E_NOT_EXIST;
    }

    pst_nexthop->refcnt--;

    if (0 == pst_nexthop->refcnt)
    {
        ret = tbl_nexthop_del_nexthop(&pst_nexthop->key);
    }
    else
    {
        ret = tbl_nexthop_set_nexthop_field_sync(pst_nexthop, TBL_NEXTHOP_FLD_REFCNT, FALSE);
    }

    return ret;
}
#endif

int32
rt_fea_nexthop_ipv6_del(char *ifname, addr_ipv6_t *ipv6)
{
    tbl_nexthop_t   *pst_nexthop = NULL;
    tbl_nexthop_t   nexthop;
    int             ret = PM_E_NONE;

    sal_memset(&nexthop, 0, sizeof(nexthop));
    nexthop.key.type = NH_TYPE_IPV6;
    sal_memcpy(nexthop.key.ifname, ifname, sizeof(nexthop.key.ifname));
    sal_memcpy(&nexthop.key.u.ipv6, ipv6, sizeof(addr_ipv6_t));

    pst_nexthop = tbl_nexthop_get_nexthop(&nexthop.key);
    if(NULL == pst_nexthop)
    {
        return PM_E_NOT_EXIST;
    }

    pst_nexthop->refcnt--;

    if (0 == pst_nexthop->refcnt)
    {
        ret = tbl_nexthop_del_nexthop(&nexthop.key);
    }
    else
    {
        ret = tbl_nexthop_set_nexthop_field_sync(pst_nexthop, TBL_NEXTHOP_FLD_REFCNT, FALSE);
    }

    return ret;
}

int32
rt_fea_set_arp_enable(uint32 ifindex)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 8, 13)
    tbl_route_if_key_t key;
    tbl_interface_t *p_db_if = NULL;
    tbl_route_if_t* p_db_rif = NULL;

    p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
    if (NULL == p_db_if)
    {
        return PM_E_NONE;
    }

    sal_memset(&key, 0, sizeof(key));
    sal_strcpy(key.name, p_db_if->key.name);
    p_db_rif = tbl_route_if_get_route_if(&key);
    if (NULL == p_db_rif)
    {
        return PM_E_NONE;
    }

    lib_sysctl_set_intf_enable_arp(p_db_if->key.name, TRUE);
    lib_sysctl_set_intf_arp_retrans_time(p_db_if->key.name, p_db_rif->arp_retrans_time);
    lib_sysctl_set_intf_arp_timeout(p_db_if->key.name, p_db_rif->arp_timeout);
#endif

    return PM_E_NONE;
}

int32
rt_fea_ipv6_add_addr(vrf_id_t vrf_id, ds_connected_v6_t *p_db_conn, uint32 ifindex)
{
    int32       rc = PM_E_NONE;

    if (GLB_IF_TYPE_NULL == tbl_interface_get_type_by_ifindex(ifindex))
    {
        /* filter manage if configurations */
        return PM_E_NONE;
    }

    p_db_conn->installed = TRUE;

    return rc;
}

int32
rt_fea_ipv6_del_addr(vrf_id_t vrf_id, ds_connected_v6_t *p_db_conn, uint32 ifindex)
{
    int32       rc = PM_E_NONE;

    if (GLB_IF_TYPE_NULL == tbl_interface_get_type_by_ifindex(ifindex))
    {
        /* filter manage if configurations */
        return PM_E_NONE;
    }

    if (!p_db_conn->installed)
    {
        return PM_E_NONE;
    }

    return rc;
}

int32
rt_fea_ipv4_add_addr(vrf_id_t vrf_id, ds_connected_t *p_db_conn, uint32 ifindex)
{
    int32       rc = PM_E_NONE;

    if (GLB_IF_TYPE_NULL == tbl_interface_get_type_by_ifindex(ifindex))
    {
        /* filter manage if configurations */
        return PM_E_NONE;
    }

    p_db_conn->installed = TRUE;

    return rc;
}

int32
rt_fea_ipv4_del_addr(vrf_id_t vrf_id, ds_connected_t *p_db_conn, uint32 ifindex)
{
    int32       rc = PM_E_NONE;

    if (GLB_IF_TYPE_NULL == tbl_interface_get_type_by_ifindex(ifindex))
    {
        /* filter manage if configurations */
        return PM_E_NONE;
    }

    if (!p_db_conn->installed)
    {
        return PM_E_NONE;
    }

    return rc;
}

int32
rt_fea_ipv6_add_addr_nl(vrf_id_t vrf_id, addr_ipv6_t *addr, uint8 prefixlen, uint32 ifindex)
{
    char        buf1[CMD_BUF_256] = {0};

    tbl_kernel_if_t         *p_db_kernel_if = NULL;
    tbl_interface_t         *p_db_if = NULL;
    tbl_kernel_if_key_t     kernel_if_key;
    uint32                  is_up_if = 0;
    uint32                  is_running = 0;
    int32                   rc = PM_E_NONE;
    int32 index = 0;
    
    if (NULL == addr)
    {
        DEBUG_ROUTE_IPV6("addr is NULL, return");
        return PM_E_FAIL;
    }

    DEBUG_ROUTE_IPV6("addr(%s), prefixlen(%u),  ifindex(%u)\n",
            cdb_addr_ipv6_val2str(buf1, CMD_BUF_256, addr), prefixlen, ifindex);

    /* filter manage if configurations */
    if (GLB_IF_TYPE_NULL == tbl_interface_get_type_by_ifindex(ifindex))
    {
        return PM_E_FAIL;
    }

    /* 1. get ifname by kernel ifindex */
    sal_memset(&kernel_if_key, 0, sizeof(kernel_if_key));
    kernel_if_key.ifindex = ifindex;
    p_db_kernel_if = tbl_kernel_if_get_kernel_if(&kernel_if_key);

    /* 2. sync the tbl_kernel_if field to cdb */
    if (NULL != p_db_kernel_if)
    {
        for (index = 0; index < RT_IF_ADDRESS_V6_MAX; index++)
        {
            if (IN6_IS_ADDR_UNSPECIFIED(&p_db_kernel_if->ip_addr_prefix_length_v6.ip_addr_v6[index]))
            {
                break;
            }
        }

        if (index == RT_IF_ADDRESS_V6_MAX)
        {
            DEBUG_ROUTE_IPV6("kernel if addr is too many");
            return PM_E_FAIL;
        }

        sal_memcpy (&p_db_kernel_if->ip_addr_prefix_length_v6.ip_addr_v6[index], addr, sizeof(addr_ipv6_t));

        if (prefixlen
        /* support vrrp modified by liwh for bug 45215, 2017-09-27 */
        && (IPV6_MAX_BITLEN != prefixlen))
        /* liwh end */
        {
            p_db_kernel_if->ip_addr_prefix_length_v6.masklen_v6[index] = prefixlen;
        }

        rc = tbl_kernel_if_set_kernel_if_field(p_db_kernel_if, TBL_KERNEL_IF_FLD_IP_ADDR_PREFIX_LENGTH_V6);
    }
    else
    {
        return PM_E_NONE;
    }

    is_up_if = lib_ioctl_if_get_flags_and_check(p_db_kernel_if->name);
    p_db_if = tbl_interface_get_interface_by_name(p_db_kernel_if->name);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    is_up_if = tbl_interface_is_up(p_db_if);
    is_running = tbl_interface_is_running(p_db_if);

    // TODO: cc ndp
    //rt_fea_set_arp_enable(p_db_if->ifindex);
    if (!is_up_if)
    {
        rc = rt_fea_update_if_up_by_addr(p_db_if);
    }
    else if (!is_running)
    {
        rc = lib_netlink_link_set(ifindex, is_running);
    }
        
    return rc;
}

int32
rt_fea_ipv6_del_addr_nl(vrf_id_t vrf_id, addr_ipv6_t *addr, uint8 prefixlen, uint32 ifindex)
{
    char        buf1[CMD_BUF_256] = {0};

    tbl_kernel_if_t     *p_db_kernel_if = NULL;
    tbl_kernel_if_key_t kernel_if_key;
    int32               rc = PM_E_NONE;
    int32 index = 0;
    
    if (NULL == addr)
    {
        DEBUG_ROUTE_IPV6("addr is NULL, return");
        return PM_E_FAIL;
    }

    DEBUG_ROUTE_IPV6("addr(%s), prefixlen(%u),  ifindex(%u)\n",
            cdb_addr_ipv6_val2str(buf1, CMD_BUF_256, addr), prefixlen, ifindex);

    /* filter manage if configurations */
    if (GLB_IF_TYPE_NULL == tbl_interface_get_type_by_ifindex(ifindex))
    {
        return PM_E_FAIL;
    }

    /* 1. get ifname by kernel ifindex */
    sal_memset(&kernel_if_key, 0, sizeof(kernel_if_key));
    kernel_if_key.ifindex = ifindex;
    p_db_kernel_if = tbl_kernel_if_get_kernel_if(&kernel_if_key);

    /* 2. sync the tbl_kernel_if field to cdb */
    if (NULL != p_db_kernel_if)
    {
        for (index = 0; index < RT_IF_ADDRESS_V6_MAX; index++)
        {
            if (0 == sal_memcmp (&p_db_kernel_if->ip_addr_prefix_length_v6.ip_addr_v6[index], addr, sizeof(addr_ipv6_t)))
            {
                sal_memset (&p_db_kernel_if->ip_addr_prefix_length_v6.ip_addr_v6[index], 0, sizeof(addr_ipv6_t));

                if (p_db_kernel_if->ip_addr_prefix_length_v6.masklen_v6[index] == prefixlen)
                {
                    p_db_kernel_if->ip_addr_prefix_length_v6.masklen_v6[index] = 0;
                }
                else
                {
                    DEBUG_ROUTE_IPV6("error prefix length is not same index %u, prefix %u %u", 
                        index, p_db_kernel_if->ip_addr_prefix_length_v6.masklen_v6[index], prefixlen);
                }
                break;
            }
        }

        if (index < RT_IF_ADDRESS_V6_MAX)
        {
            rc = tbl_kernel_if_set_kernel_if_field(p_db_kernel_if, TBL_KERNEL_IF_FLD_IP_ADDR_PREFIX_LENGTH_V6);
        }
    }
    else
    {
        return PM_E_NONE;
    }
        
    return rc;
}

int32
rt_fea_ipv4_add_addr_nl(vrf_id_t vrf_id, addr_ipv4_t *addr, uint8 prefixlen, uint32 ifindex)
{
    tbl_kernel_if_t         *p_db_kernel_if = NULL;
    tbl_interface_t         *p_db_if = NULL;
    tbl_kernel_if_key_t     kernel_if_key;
    uint32                  is_up_if = 0;
    uint32                  is_running = 0;
    int32                   rc = PM_E_NONE;

    DEBUG_ROUTE_IPV4();
    
    if (NULL == addr)
    {
        return PM_E_FAIL;
    }

    char        buf[CMD_BUF_256] = {0};
    DEBUG_ROUTE_IPV4("addr %s, prefixlen %u, ifindex %u",
        sal_inet_ntop(AF_INET, addr, buf, sizeof(buf)),
        prefixlen,
        ifindex);

    /* filter manage if configurations */
    if (GLB_IF_TYPE_NULL == tbl_interface_get_type_by_ifindex(ifindex))
    {
        return PM_E_FAIL;
    }

    /* 1. get ifname by kernel ifindex */
    sal_memset(&kernel_if_key, 0, sizeof(kernel_if_key));
    kernel_if_key.ifindex = ifindex;
    p_db_kernel_if = tbl_kernel_if_get_kernel_if(&kernel_if_key);

    /* 2. sync the tbl_kernel_if field to cdb */
    if (NULL != p_db_kernel_if)
    {
        sal_memcpy (&p_db_kernel_if->ip_addr, addr, sizeof(addr_ipv4_t));
        rc = tbl_kernel_if_set_kernel_if_field(p_db_kernel_if, TBL_KERNEL_IF_FLD_IP_ADDR);
    }
    else
    {
        return PM_E_NONE;
    }

    if (prefixlen
        /* support vrrp modified by liwh for bug 45215, 2017-09-27 */
        && (IPV4_MAX_BITLEN != prefixlen))
        /* liwh end */
    {
        p_db_kernel_if->masklen = prefixlen;
        rc = tbl_kernel_if_set_kernel_if_field(p_db_kernel_if, TBL_KERNEL_IF_FLD_MASKLEN);
    }

    is_up_if = lib_ioctl_if_get_flags_and_check(p_db_kernel_if->name);
    p_db_if = tbl_interface_get_interface_by_name(p_db_kernel_if->name);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    is_up_if = tbl_interface_is_up(p_db_if);
    is_running = tbl_interface_is_running(p_db_if);

    rt_fea_set_arp_enable(p_db_if->ifindex);
    if (!is_up_if)
    {
        rc = rt_fea_update_if_up_by_addr(p_db_if);
    }
    else if (!is_running)
    {
        rc = lib_netlink_link_set(ifindex, is_running);
    }

    return rc;
}

int32
rt_fea_ipv4_del_addr_nl(vrf_id_t vrf_id, addr_ipv4_t *addr, uint8 prefixlen, uint32 ifindex)
{
    tbl_kernel_if_t     *p_db_kernel_if = NULL;
    tbl_kernel_if_key_t kernel_if_key;
    int                 field_id;
    int32               rc = PM_E_NONE;
    int32               ret = PM_E_NONE;

    DEBUG_ROUTE_IPV4();
    
    if (NULL == addr)
    {
        return PM_E_FAIL;
    }

    char        buf[CMD_BUF_256] = {0};
    DEBUG_ROUTE_IPV4("addr %s, prefixlen %u, ifindex %u",
        sal_inet_ntop(AF_INET, addr, buf, sizeof(buf)),
        prefixlen,
        ifindex);

    /* filter manage if configurations */
    if (GLB_IF_TYPE_NULL == tbl_interface_get_type_by_ifindex(ifindex))
    {
        return PM_E_FAIL;
    }

    /* 1. get ifname by kernel ifindex */
    sal_memset(&kernel_if_key, 0, sizeof(kernel_if_key));
    kernel_if_key.ifindex = ifindex;
    p_db_kernel_if = tbl_kernel_if_get_kernel_if(&kernel_if_key);

    /* 2. sync the tbl_kernel_if field to cdb */
    if (NULL != p_db_kernel_if)
    {
        ret  = sal_memcmp (&p_db_kernel_if->ip_addr, addr, sizeof(addr_ipv4_t));
        if (!ret)
        {
            sal_memset (&p_db_kernel_if->ip_addr, 0, sizeof(addr_ipv4_t));
        }

        field_id = TBL_KERNEL_IF_FLD_IP_ADDR;
        rc = tbl_kernel_if_set_kernel_if_field(p_db_kernel_if, field_id);
    }
    else
    {
        return PM_E_NONE;
    }

    if (p_db_kernel_if->masklen == prefixlen)
    {
        p_db_kernel_if->masklen = 0;
        field_id = TBL_KERNEL_IF_FLD_MASKLEN;
        rc = tbl_kernel_if_set_kernel_if_field(p_db_kernel_if, field_id);
    }

    return rc;
}

uint32
rt_fea_route_match_rtif_addr(addr_ipv4_t addr_ip, uint32 mask_len)
{
    tbl_route_if_master_t   *p_master       = tbl_route_if_get_master();
    ctclib_slistnode_t      *listnode       = NULL;
    ctclib_slistnode_t      *listnode_sub   = NULL;
    ds_connected_t          *p_db_conn      = NULL;
    tbl_route_if_t          *p_db_rtif      = NULL;
    uint32                  is_match        = 0;
    uint32                  mask            = 0;
    addr_ipv4_t             ip_addr_cp;

    if (NULL == p_master)
    {
        return is_match;
    }

    sal_memset(&ip_addr_cp, 0, sizeof(ip_addr_cp));
    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_rtif, listnode)
    {
        if (NULL != p_db_rtif)
        {
#if 0
            p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&p_db_rtif->key);
            if (NULL != p_db_if)
            {
                is_running = tbl_interface_is_running(p_db_if);
            }
#endif
            /* modified by wangjj for fix bug 43648, need to ignore running change, 2017-04-15 */

            CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, p_db_conn, listnode_sub)
            {
                if (p_db_conn->key.address.prefixlen == mask_len)
                {
                    sal_memcpy(&ip_addr_cp, &p_db_conn->key.address.u.prefix4, sizeof(addr_ipv4_t));
                    IPV4_LEN_TO_MASK(mask, mask_len);
                    ip_addr_cp.s_addr = sal_htonl(ip_addr_cp.s_addr);
                    ip_addr_cp.s_addr = ip_addr_cp.s_addr & mask;
                    ip_addr_cp.s_addr = sal_htonl(ip_addr_cp.s_addr);
                    if (ip_addr_cp.s_addr == addr_ip.s_addr)
                    {
                        is_match = 1;
                    }
                }
            }
        }
    }

    return is_match;
}

/*delete route for rtif_match type route*/
int32
rt_fea_rtif_match_route_del(tbl_iproute_node_t  *pst_iproute_node, uint32 distance)
{
    int32               con_ret = PM_E_NONE;
    vrf_id_t            con_vrf_id;
    addr_ipv4_t         con_dest;
    uint8               con_prefixlen   = 0;

    if (NULL != pst_iproute_node)
    {
        if (ECMP_ENABLE == pst_iproute_node->multipath)
        {
            con_vrf_id = pst_iproute_node->key.vrf_id;
            con_dest.s_addr = pst_iproute_node->key.p.u.prefix4.s_addr;
            con_prefixlen = pst_iproute_node->key.p.prefixlen;

            con_ret = rt_fea_ipv4_del_route_multi_nl(con_vrf_id, &con_dest, NULL,
                                pst_iproute_node->nh_group.nh_num, con_prefixlen, distance, ZEBRA_ROUTE_MAX);
        }
        else
        {
            con_ret = tbl_iproute_node_del_iproute_node(&pst_iproute_node->key);
        }
    }
    return con_ret;
    
}

int32
rt_fea_ipv6_get_nh_key(vrf_id_t vrf_id, addr_ipv6_t *dest, addr_ipv6_t *gate, uint8 prefixlen,
                  uint32 ifindex, uint32 multipath, nexthop_key_t *nh_key)
{
    char        buf1[CMD_BUF_256] = {0};
    char        buf2[CMD_BUF_256] = {0};

    if (NULL != dest && NULL != gate)
    {
        DEBUG_ROUTE_IPV6("dest(%s), gate(%s), prefixlen(%u),  ifindex(%u), multipath(%u)", 
            cdb_addr_ipv6_val2str(buf1, CMD_BUF_256, dest), cdb_addr_ipv6_val2str(buf2, CMD_BUF_256, gate), 
            prefixlen, ifindex, multipath);
    }
    else if (NULL != dest &&  NULL == gate)
    {
        DEBUG_ROUTE_IPV6("dest(%s), prefixlen(%u),  ifindex(%u), multipath(%u)", 
            cdb_addr_ipv6_val2str(buf1, CMD_BUF_256, dest), 
            prefixlen, ifindex, multipath);
    }
    else if (NULL == dest &&  NULL != gate)
    {
        DEBUG_ROUTE_IPV6("gate(%s), prefixlen(%u),  ifindex(%u), multipath(%u)", 
            cdb_addr_ipv6_val2str(buf2, CMD_BUF_256, gate), 
            prefixlen, ifindex, multipath);
    }
    else
    {
        DEBUG_ROUTE_IPV6("dest and gate are NULL");
    }

    tbl_kernel_if_t     *p_db_kernel_if   = NULL;
    tbl_kernel_if_t     kernel_if;

    /*1, filter manage if configurations */
    if (GLB_IF_TYPE_NULL == tbl_interface_get_type_by_ifindex(ifindex))
    {
        //log_sys(M_MOD_ROUTE, E_DEBUG,"rt_fea_ipv4_get_nh_key ifindex can not find interface ifindex %d\n", ifindex);
        return PM_E_FAIL;
    }

    /* 2. get ifname by kernel ifindex */
    kernel_if.key.ifindex = ifindex;
    p_db_kernel_if = tbl_kernel_if_get_kernel_if(&kernel_if.key);
    if (NULL == p_db_kernel_if)
    {
        //log_sys(M_MOD_ROUTE, E_DEBUG,"rt_fea_ipv4_get_nh_key ifindex can not find kernel interface %d\n", ifindex);
        return PM_E_FAIL;
    }

    /* 3. add nexthop group member and copy the nexthop key info */
    if (NULL != gate)
    {
        nh_key->type = NH_TYPE_IPV6;
        sal_memcpy(nh_key->ifname, p_db_kernel_if->name, sizeof(nh_key->ifname));
        sal_memcpy(&nh_key->u.ipv6, gate, sizeof(addr_ipv6_t));
    }
    else
    {
        //log_sys(M_MOD_ROUTE, E_ERROR,"rt_fea_ipv4_get_nh_key gate is null ifindex %d\n", ifindex);
    }
        
    return PM_E_NONE;
}

/*
we use this function to build nhg structure.
*/
int32
rt_fea_ipv4_get_nh_key(vrf_id_t vrf_id, addr_ipv4_t *dest, addr_ipv4_t *gate, uint8 prefixlen,
                  uint32 ifindex, uint32 multipath, nexthop_key_t *nh_key)
{
    tbl_kernel_if_t     *p_db_kernel_if   = NULL;
    tbl_kernel_if_t     kernel_if;


    /*1, filter manage if configurations */
    if (GLB_IF_TYPE_NULL == tbl_interface_get_type_by_ifindex(ifindex))
    {
        //log_sys(M_MOD_ROUTE, E_DEBUG,"rt_fea_ipv4_get_nh_key ifindex can not find interface ifindex %d\n", ifindex);
        return PM_E_FAIL;
    }

    /* 2. get ifname by kernel ifindex */
    kernel_if.key.ifindex = ifindex;
    p_db_kernel_if = tbl_kernel_if_get_kernel_if(&kernel_if.key);
    if (NULL == p_db_kernel_if)
    {
        //log_sys(M_MOD_ROUTE, E_DEBUG,"rt_fea_ipv4_get_nh_key ifindex can not find kernel interface %d\n", ifindex);
        return PM_E_FAIL;
    }

    /* 3. add nexthop group member and copy the nexthop key info */
    if (NULL != gate)
    {
        nh_key->type = NH_TYPE_IPV4;
        sal_memcpy(nh_key->ifname, p_db_kernel_if->name, sizeof(nh_key->ifname));
        sal_memcpy(&nh_key->u.ipv4, gate, sizeof(addr_ipv4_t));
    }
    else
    {
        //log_sys(M_MOD_ROUTE, E_ERROR,"rt_fea_ipv4_get_nh_key gate is null ifindex %d\n", ifindex);
    }
    

    return PM_E_NONE;
}


/*add nexthop and cause arp for this nexthop*/
int32
rt_fea_ipv6_add_nh(vrf_id_t vrf_id, addr_ipv6_t *gate, char* ifname)
{
    tbl_nexthop_t       *pst_nexthop      = NULL;
    
    /* 1. add nexthop group member and copy the nexthop key info */
    if (NULL != gate && NULL != ifname)
    {
        pst_nexthop = rt_fea_nexthop_ipv6_add(ifname, gate);
        if (NULL == pst_nexthop)
        {
            return PM_E_FAIL;
        }

        /* support nd modified by liwh for bug 47547, 2018-07-25 */
        nd_neigh_route_add(pst_nexthop);
        /* liwh end */
    }

    return PM_E_NONE;
}

int32
rt_fea_ipv6_del_nh(vrf_id_t vrf_id, addr_ipv6_t *gate, char* ifname)
{
    tbl_nexthop_t       *pst_nexthop      = NULL;
    int32 rc;

    /* del nexthop group member and copy the nexthop key info */
    pst_nexthop = rt_fea_nexthop_ipv6_get(ifname, gate);
    if (NULL != pst_nexthop && NULL != ifname )
    {
        /* support nd modified by liwh for bug 47547, 2018-07-25 */
        nd_neigh_route_del(pst_nexthop);
        /* liwh end */
        
        PM_E_RETURN(rt_fea_nexthop_ipv6_del(ifname, gate));
    }

    return PM_E_NONE;
}

int32
rt_fea_ipv4_add_nh(vrf_id_t vrf_id, addr_ipv4_t *gate, char* ifname)
{
    tbl_nexthop_t       *pst_nexthop      = NULL;
    
    /* 1. add nexthop group member and copy the nexthop key info */
    if (NULL != gate && NULL != ifname)
    {
        pst_nexthop = rt_fea_nexthop_ipv4_add(ifname, gate);
        if (NULL == pst_nexthop)
        {
            return PM_E_FAIL;
        }
        arp_api_route_add(pst_nexthop);
    }

    return PM_E_NONE;
}

/*delete nexthop and cause arp del for this nexthop*/
int32
rt_fea_ipv4_del_nh(vrf_id_t vrf_id, addr_ipv4_t *gate, char* ifname)
{
    tbl_nexthop_t       *pst_nexthop      = NULL;
    int32 rc;

    /* del nexthop group member and copy the nexthop key info */
    pst_nexthop = rt_fea_nexthop_ipv4_get(ifname, gate);
    if (NULL != pst_nexthop && NULL != ifname )
    {
        arp_api_route_del(pst_nexthop);
        PM_E_RETURN(rt_fea_nexthop_ipv4_del(ifname, gate));
    }

    return PM_E_NONE;
}

void
rt_fea_ipv4_nhg_check(nexthop_key_t *nhg, uint32 nh_num, uint32 *new_nh_num)
{
    int32 i = 0;
    int32 j = 0;
    int32 index = 0;
    uint32 nh_num_tmp = 0;
    nexthop_key_t nexthop_zero;
    int32 need_check = FALSE;

    sal_memset(&nexthop_zero, 0x00, sizeof(nexthop_key_t));
    index++;
    for(i = 0; i < nh_num; i++)
    {
        for(j = i + 1; j < nh_num; j++)
        {
            if (0 != sal_memcmp(&nhg[j], &nexthop_zero, sizeof(nexthop_key_t)) 
                && 0 == sal_memcmp(&nhg[i], &nhg[j], sizeof(nexthop_key_t)))
            {
                sal_memset(&nhg[j], 0x00, sizeof(nexthop_key_t));
                need_check = TRUE;
            }
        }
    }


    if (need_check)
    {
        for (i = 0; i < nh_num; i++)
        {
            if (0 == sal_memcmp(&nhg[i], &nexthop_zero, sizeof(nexthop_key_t)))
            {
                for(j = i + 1; j < nh_num; j++)
                {
                    if (0 != sal_memcmp(&nhg[j], &nexthop_zero, sizeof(nexthop_key_t)))
                    {
                        sal_memcpy(&nhg[i], &nhg[j], sizeof(nexthop_key_t));
                        nh_num_tmp++;
                        sal_memset(&nhg[j], 0x00, sizeof(nexthop_key_t));
                        break;
                    }
                }
            }
            else
            {
                nh_num_tmp++;
            }
        }
    }
    *new_nh_num = nh_num_tmp;

}

int32
rt_fea_ipv4_ecmp_hash_type_is_rr(uint32 ecmp_hash)
{
    return ((ecmp_hash == GLB_ECMP_ROUND_ROBIN_LOAD_BALANCE) ? 1 : 0);
}

int32
rt_fea_ipv4_add_nhg(tbl_iproute_node_t  *pst_iproute, nexthop_key_t *nhg,uint32 nh_num, uint32 ecmp_hash, bool max_check, uint32 is_ipv4)
{
    tbl_nexthop_group_t nhgroup;
    tbl_nexthop_group_t* p_nhg;
    int32 rc;

    sal_memset(&nhgroup, 0, sizeof(nhgroup));

    nhgroup.key.rr_en = rt_fea_ipv4_ecmp_hash_type_is_rr(ecmp_hash);
    nhgroup.key.nh_num = nh_num;
    sal_memcpy(nhgroup.key.nh, nhg, nh_num*sizeof(nexthop_key_t));
    p_nhg = tbl_nexthop_group_get_nexthop_group(&nhgroup.key);
    
    
    if(NULL == p_nhg)
    {
        if (is_ipv4)
        {
            if (rt_fea_ecmp_count_max_check())
            {
                if (TRUE == max_check)
                {
                    log_sys(M_MOD_ROUTE, E_ERROR, "The ecmp entry num is more than maximum %u !", rt_fea_ecmp_count_get_max());
                    return PM_E_EXCEED_ECMP_MAX_NUM;
                }
            }

            rt_ecmp_counter_inc(DEFAULT_VRF_ID);
        }
        else
        {
            if (rt_fea_v6_ecmp_count_max_check())
            {
                if (TRUE == max_check)
                {
                    log_sys(M_MOD_ROUTE, E_ERROR, "The ecmp entry num is more than maximum %u !", rt_fea_v6_ecmp_count_get_max());
                    return PM_E_EXCEED_ECMP_MAX_NUM;
                }
            }

            rt_ecmp_v6_counter_inc(DEFAULT_VRF_ID);
        }
        
        nhgroup.ref_cnt = 1;
        nhgroup.ecmp_hash = ecmp_hash;
        
        
        PM_E_RETURN(tbl_nexthop_group_add_nexthop_group(&nhgroup));
    }
    else
    {
        p_nhg->ref_cnt ++;
    }
    
    return PM_E_NONE;

}

int32
rt_fea_ipv4_del_nhg(tbl_iproute_node_t  *pst_iproute, nexthop_key_t *nhg,uint32 nh_num, uint32 rr_en, uint32 is_ipv4)
{
    tbl_nexthop_group_t nhgroup;
    tbl_nexthop_group_t* p_nhg;
    int32 rc;

    sal_memset(&nhgroup, 0, sizeof(nhgroup));

    if(NULL == nhg && pst_iproute)
    {
        nhgroup.key.rr_en = pst_iproute->nh_group.rr_en;
        nhgroup.key.nh_num = pst_iproute->nh_group.nh_num;
        sal_memcpy(nhgroup.key.nh, pst_iproute->nh_group.nh, pst_iproute->nh_group.nh_num*sizeof(nexthop_key_t));
    }
    else
    {
        nhgroup.key.rr_en = rr_en;
        nhgroup.key.nh_num = nh_num;
        sal_memcpy(nhgroup.key.nh, nhg, nh_num*sizeof(nexthop_key_t));
    }
    p_nhg = tbl_nexthop_group_get_nexthop_group(&nhgroup.key);
    
    if(NULL == p_nhg)
    {
        nexthop_key_t nh_group[IP_ECMP_MAX_PATH];
        uint32 new_nh_num = 0;

        sal_memset(nh_group, 0x00, (IP_ECMP_MAX_PATH * sizeof(nexthop_key_t)));
        rt_fea_ipv4_nhg_check(nhgroup.key.nh, nhgroup.key.nh_num, &new_nh_num);
        if (nhgroup.key.nh_num == new_nh_num)
        {
            return PM_E_NOT_EXIST;
        }
        else
        {
            //sal_memset(&nhgroup, 0, sizeof(nhgroup));
            nhgroup.key.nh_num = new_nh_num;
            //sal_memcpy(nhgroup.key.nh, nh_group, new_nh_num*sizeof(nexthop_key_t));
        }

        p_nhg = tbl_nexthop_group_get_nexthop_group(&nhgroup.key);
        if (NULL == p_nhg)
        {
            return PM_E_NOT_EXIST;
        }
    }

    /*2, delete nhg, release resource*/
    if(p_nhg->ref_cnt > 1)
    {
        p_nhg->ref_cnt --;
    }
    else if(p_nhg->ref_cnt == 1)
    {
        /*delete nexthop group*/
        if (is_ipv4)
        {
            rt_ecmp_counter_dec(DEFAULT_VRF_ID);
        }
        else
        {
            rt_ecmp_v6_counter_dec(DEFAULT_VRF_ID);
        }
        
        PM_E_RETURN(tbl_nexthop_group_del_nexthop_group(&nhgroup.key));
    }
    else
    {
        return PM_E_FAIL;
    }
    
    return PM_E_NONE;

}

int32
rt_fea_ipv6_add_nh_by_group(vrf_id_t vrf_id, addr_ipv6_t *dest, nexthop_key_t *nhg,
                    uint32 nh_num, uint8 prefixlen, uint32 distance)
{

    int i;

    for(i=0; i<nh_num; i++)
    {
        rt_fea_ipv6_add_nh(vrf_id, &nhg[i].u.ipv6, nhg[i].ifname);
    }
    

    return PM_E_NONE;
}

/*
we use this function to add nexthop one by one by ecmp grup add funciton
*/
int32
rt_fea_ipv4_add_nh_by_group(vrf_id_t vrf_id, addr_ipv4_t *dest, nexthop_key_t *nhg,
                    uint32 nh_num, uint8 prefixlen, uint32 distance)
{

    int i;

    for(i=0; i<nh_num; i++)
    {
        rt_fea_ipv4_add_nh(vrf_id, &nhg[i].u.ipv4, nhg[i].ifname);
    }
    

    return PM_E_NONE;
}

int32
rt_fea_ipv6_del_nh_by_group(tbl_iproute_node_t  *pst_iproute, vrf_id_t vrf_id, addr_ipv6_t *dest, nexthop_key_t *nhg,
                    uint32 nh_num, uint8 prefixlen, uint32 distance)
{
    int i;

    if(nhg)
    {
        for(i=0; i<nh_num; i++)
        {
            rt_fea_ipv6_del_nh(vrf_id, &nhg[i].u.ipv6, nhg[i].ifname);
        }
    }
    else if(pst_iproute)
    {
        for(i=0; i<nh_num; i++)
        {
            rt_fea_ipv6_del_nh(vrf_id, &pst_iproute->nh_group.nh[i].u.ipv6, pst_iproute->nh_group.nh[i].ifname);
        }        
    }

    return PM_E_NONE;
}

int32
rt_fea_ipv4_del_nh_by_group(tbl_iproute_node_t  *pst_iproute, vrf_id_t vrf_id, addr_ipv4_t *dest, nexthop_key_t *nhg,
                    uint32 nh_num, uint8 prefixlen, uint32 distance)
{
    int i;

    if(nhg)
    {
        for(i=0; i<nh_num; i++)
        {
            rt_fea_ipv4_del_nh(vrf_id, &nhg[i].u.ipv4, nhg[i].ifname);
        }
    }
    else if(pst_iproute)
    {
        for(i=0; i<nh_num; i++)
        {
            rt_fea_ipv4_del_nh(vrf_id, &pst_iproute->nh_group.nh[i].u.ipv4, pst_iproute->nh_group.nh[i].ifname);
        }        
    }

    return PM_E_NONE;
}

/*
we use this function to udpate nexhtop when ecmp update
*/
int32
//rt_fea_ipv4_update_nh_by_group(vrf_id_t vrf_id, addr_ipv4_t *dest, nexthop_key_t *nhg,
rt_fea_ipv4_update_nh_by_group(vrf_id_t vrf_id, addr_t *dest, nexthop_key_t *nhg,
                    uint32 nh_num, uint8 prefixlen, uint32 distance)
{
    tbl_iproute_node_t  iproute_node;
    tbl_iproute_node_t  *p_rn = NULL;
    int i,j;
    int8 found = FALSE;

    sal_memset(&iproute_node, 0, sizeof(iproute_node));
    iproute_node.key.vrf_id = vrf_id;

    /*ipv4 may not set family*/
    iproute_node.key.p.family = (dest->family == AF_INET6) ? AF_INET6 : AF_INET;
    iproute_node.key.p.prefixlen = prefixlen;
    if (AF_INET == dest->family)
    {
        sal_memcpy (&iproute_node.key.p.u.prefix4, &dest->u.prefix4, sizeof(addr_ipv4_t));
    }
    else
    {
        sal_memcpy (&iproute_node.key.p.u.prefix6, &dest->u.prefix6, sizeof(addr_ipv6_t));
    }
    p_rn = tbl_iproute_node_get_iproute_node(&iproute_node.key);

    if(NULL == p_rn)
    {
        return PM_E_NOT_EXIST;
    }
    /*add new nexthop*/
    for(i=0; i<nh_num; i++)
    {
        if(p_rn->multipath)
        {
            for(j=0; j<p_rn->nh_group.nh_num; j++)
            {
                if(!sal_memcmp(&nhg[i], &p_rn->nh_group.nh[j], sizeof(nexthop_key_t)))
                {
                    found = TRUE;
                    break;
                }
            }
        }
        else
        {

            if(!sal_memcmp(&nhg[i], &p_rn->nh_key, sizeof(nexthop_key_t)))
            {
                found = TRUE;
            }
        }
        
        if(!found)
        {
            /*add nexthop for this member*/
            if (AF_INET == dest->family)
            {
                rt_fea_ipv4_add_nh(vrf_id, &nhg[i].u.ipv4, nhg[i].ifname);
            }
            else if (AF_INET6 == dest->family)
            {
                rt_fea_ipv6_add_nh(vrf_id, &nhg[i].u.ipv6, nhg[i].ifname);
            }
        } 
        found = FALSE;
    }

    /*del old nexthop*/
    if(p_rn->multipath)
    {
        for(i=0; i<p_rn->nh_group.nh_num; i++)
        {
            for(j=0; j<nh_num; j++)
            {
                if(!sal_memcmp(&p_rn->nh_group.nh[i], &nhg[j], sizeof(nexthop_key_t)))
                {
                    found = TRUE;
                    break;
                }
            }
            if(!found)
            {
                /*del nexthop for this member*/
                if (AF_INET == dest->family)
                {
                    rt_fea_ipv4_del_nh(vrf_id, &p_rn->nh_group.nh[i].u.ipv4, p_rn->nh_group.nh[i].ifname);
                }
                else if (AF_INET6 == dest->family)
                {
                    rt_fea_ipv6_del_nh(vrf_id, &p_rn->nh_group.nh[i].u.ipv6, p_rn->nh_group.nh[i].ifname);
                }
            }
            found = FALSE;
        }
    }
    else
    {

        for(j=0; j<nh_num; j++)
        {
            if(!sal_memcmp(&p_rn->nh_key, &nhg[j], sizeof(nexthop_key_t)))
            {
                found = TRUE;
                break;
            }
        }
        if(!found)
        {
            /*del nexthop for this member*/
            if (AF_INET == dest->family)
            {
                rt_fea_ipv4_del_nh(vrf_id, &p_rn->nh_key.u.ipv4, p_rn->nh_key.ifname);
            }
            else if (AF_INET6 == dest->family)
            {
                rt_fea_ipv6_del_nh(vrf_id, &p_rn->nh_key.u.ipv6, p_rn->nh_key.ifname);
            }
        }
        found = FALSE;
     
    }        
    
    return PM_E_NONE;
}

int32
rt_fea_ipv6_gate_check(addr_ipv6_t *gate)
{
    tbl_route_if_master_t *p_master = tbl_route_if_get_master();
    uint32          is_running = 0;
    addr_ipv6_t tmp1;
    addr_ipv6_t tmp2;
    ctclib_slistnode_t  *listnode = NULL;
    ds_connected_v6_t      *p_tmp_conn = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_interface_t *p_db_if = NULL;
    tbl_route_if_t *p_db_route_if = NULL;
    char        buf1[CMD_BUF_256] = {0};
    
    if (NULL == p_master)
    {
        return PM_E_NONE;
    }
    
    CTCLIB_SLIST_LOOP_DEL(p_master->if_list, p_db_route_if, listnode, next)
    {
        p_db_if = tbl_interface_get_interface_by_name(p_db_route_if->key.name);
        if (NULL == p_db_if)
        {
            continue;
        }
        
        is_running = tbl_interface_is_running(p_db_if);
        if (FALSE == is_running)
        {
            continue;
        }
        
        CTCLIB_SLIST_LOOP(p_db_route_if->ifc_ipv6.obj_list, p_tmp_conn, listnode)
        {
            sal_memset(&tmp1, 0 , sizeof (tmp1));
            sal_memset(&tmp2, 0 , sizeof (tmp2));
            sal_memcpy(&tmp1, &p_tmp_conn->key.address.u.prefix6, sizeof (addr_ipv6_t));
            sal_memcpy(&tmp2, gate, sizeof (addr_ipv6_t));
            apply_mask_ipv6_addr(&tmp1, p_tmp_conn->key.address.prefixlen);
            apply_mask_ipv6_addr(&tmp2, p_tmp_conn->key.address.prefixlen);
            if (0 == sal_memcmp(&tmp2, &tmp1, sizeof (tmp1)))
            {
                return PM_E_NONE;
            }
        }
    }
    
    DEBUG_ROUTE_IPV6("Gateway %s check failed, interface not running or prefix not match\n",                     
        cdb_addr_ipv6_val2str(buf1, sizeof (buf1), gate));
        
    return PM_E_FAIL;
}

int32
rt_fea_ipv4_gate_check(addr_ipv4_t *gate)
{
    tbl_route_if_master_t *p_master = tbl_route_if_get_master();
    uint32          is_running = 0;
    addr_ipv4_t tmp1 = {0};
    addr_ipv4_t tmp2 = {0};
    addr_ipv4_t mask_addr = {0};
    ctclib_slistnode_t  *listnode = NULL;
    ds_connected_t      *p_tmp_conn = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_interface_t *p_db_if = NULL;
    tbl_route_if_t *p_db_route_if = NULL;
    char        buf1[CMD_BUF_256] = {0};
    
    if (NULL == p_master)
    {
        return PM_E_NONE;
    }
    
    CTCLIB_SLIST_LOOP_DEL(p_master->if_list, p_db_route_if, listnode, next)
    {
        p_db_if = tbl_interface_get_interface_by_name(p_db_route_if->key.name);
        if (NULL == p_db_if)
        {
            continue;
        }
        
        is_running = tbl_interface_is_running(p_db_if);
        if (FALSE == is_running)
        {
            continue;
        }
        
        CTCLIB_SLIST_LOOP(p_db_route_if->ifc_ipv4.obj_list, p_tmp_conn, listnode)
        {
            //addr_mask_len2ip_v4(prefixlen, &mask_addr);
            addr_mask_len2ip_v4(p_tmp_conn->key.address.prefixlen, &mask_addr);
            tmp1.s_addr = p_tmp_conn->key.address.u.prefix4.s_addr & mask_addr.s_addr;
            tmp2.s_addr = gate->s_addr & mask_addr.s_addr;
            if (tmp2.s_addr == tmp1.s_addr)
            {
                return PM_E_NONE;
            }
        }
    }
    
    DEBUG_ROUTE_IPV4("Gateway %s check failed, interface not running or prefix not match",                     
        cdb_addr_ipv4_val2str(buf1, sizeof (buf1), gate));
        
    return PM_E_FAIL;
}

int32
rt_fea_ipv4_add_to_iproute_node_add_fail_count
    (vrf_id_t vrf_id, addr_ipv4_t *dest, uint8 prefixlen, addr_ipv4_t *gate, nexthop_key_t *nhg, uint32 add_nh_num)
{
    tbl_iproute_node_add_fail_count_t  *pst_iproute_node_add_fail_count = NULL;
    tbl_iproute_node_add_fail_count_t  iproute_node_add_fail_count;
    nexthop_key_t       nh_key;
//    int32 index = 0;

    if (NULL == dest )
    {
        DEBUG_ROUTE_IPV4("dest is NULL, return");
        return PM_E_FAIL;
    }
    
    addr_t dest_all = {0};
    dest_all.family = AF_INET;
    sal_memcpy(&dest_all.u.prefix4, dest, sizeof(addr_ipv4_t));
            
    sal_memset(&iproute_node_add_fail_count, 0, sizeof(iproute_node_add_fail_count));
    sal_memset(&nh_key, 0, sizeof(nh_key));

    /* 1. get route  */
    iproute_node_add_fail_count.key.vrf_id = vrf_id;
    iproute_node_add_fail_count.key.p.family = AF_INET;
    iproute_node_add_fail_count.key.p.prefixlen = prefixlen;
    sal_memcpy (&iproute_node_add_fail_count.key.p.u.prefix4, dest, sizeof(addr_ipv4_t));

#if 0
    if (NULL != gate)
    {
        sal_memcpy (&iproute_node_add_fail.key.nh_addr.ipv4, gate, sizeof(addr_ipv4_t));
    
        pst_iproute_node_add_fail = tbl_iproute_node_add_fail_get_iproute_node_add_fail(&iproute_node_add_fail.key);
        if (NULL == pst_iproute_node_add_fail)
        {
            tbl_iproute_node_add_fail_add_iproute_node_add_fail(&iproute_node_add_fail);
        }
    }
    else if (NULL == gate && NULL != nhg)
    {
        for(index = 0; index < add_nh_num; index++)
        {
            sal_memset(&iproute_node_add_fail.key.nh_addr.ipv4, 0, sizeof(iproute_node_add_fail.key.nh_addr.ipv4));
            sal_memcpy(&iproute_node_add_fail.key.nh_addr.ipv4, &nhg[index].u.ipv4, sizeof(addr_ipv4_t));
    
            pst_iproute_node_add_fail = tbl_iproute_node_add_fail_get_iproute_node_add_fail(&iproute_node_add_fail.key);
            if (NULL == pst_iproute_node_add_fail)
            {
                tbl_iproute_node_add_fail_add_iproute_node_add_fail(&iproute_node_add_fail);
            }
        }
    }
#endif

    pst_iproute_node_add_fail_count = tbl_iproute_node_add_fail_count_get_iproute_node_add_fail_count(&iproute_node_add_fail_count.key);
    if (NULL == pst_iproute_node_add_fail_count)
    {
        tbl_iproute_node_add_fail_count_add_iproute_node_add_fail_count(&iproute_node_add_fail_count);
    }

    return PM_E_NONE;
}

int32
rt_fea_ipv4_del_from_iproute_node_add_fail_count
    (vrf_id_t vrf_id, addr_ipv4_t *dest, uint8 prefixlen, addr_ipv4_t *gate, nexthop_key_t *nhg, uint32 add_nh_num)
{
    tbl_iproute_node_add_fail_count_t  *pst_iproute_node_add_fail_count = NULL;
    tbl_iproute_node_add_fail_count_t  iproute_node_add_fail_count;
    nexthop_key_t       nh_key;
    //int32 index = 0;

    if (NULL == dest )
    {
        DEBUG_ROUTE_IPV4("dest is NULL, return");
        return PM_E_FAIL;
    }
    
    addr_t dest_all = {0};
    dest_all.family = AF_INET;
    sal_memcpy(&dest_all.u.prefix4, dest, sizeof(addr_ipv4_t));
            
    sal_memset(&iproute_node_add_fail_count, 0, sizeof(iproute_node_add_fail_count));
    sal_memset(&nh_key, 0, sizeof(nh_key));

    /* 1. get route  */
    iproute_node_add_fail_count.key.vrf_id = vrf_id;
    iproute_node_add_fail_count.key.p.family = AF_INET;
    iproute_node_add_fail_count.key.p.prefixlen = prefixlen;
    sal_memcpy (&iproute_node_add_fail_count.key.p.u.prefix4, dest, sizeof(addr_ipv4_t));

#if 0
    if (NULL != gate)
    {
        sal_memcpy (&iproute_node_add_fail_count.key.nh_addr.ipv4, gate, sizeof(addr_ipv4_t));
    
        pst_iproute_node_add_fail_count = tbl_iproute_node_add_fail_count_get_iproute_node_add_fail_count(&iproute_node_add_fail_count.key);
        if (NULL != pst_iproute_node_add_fail_count)
        {
            tbl_iproute_node_add_fail_count_del_iproute_node_add_fail_count(&iproute_node_add_fail_count.key);
        }
    }
    else if (NULL == gate && NULL != nhg)
    {
        for(index = 0; index < add_nh_num; index++)
        {
            sal_memset(&iproute_node_add_fail_count.key.nh_addr.ipv4, 0, sizeof(iproute_node_add_fail_count.key.nh_addr.ipv4));
            sal_memcpy(&iproute_node_add_fail_count.key.nh_addr.ipv4, &nhg[index].u.ipv4, sizeof(addr_ipv4_t));
    
            pst_iproute_node_add_fail_count = tbl_iproute_node_add_fail_count_get_iproute_node_add_fail_count(&iproute_node_add_fail_count.key);
            if (NULL != pst_iproute_node_add_fail_count)
            {
                tbl_iproute_node_add_fail_count_del_iproute_node_add_fail_count(&iproute_node_add_fail_count.key);
            }
        }
    }
#endif

    pst_iproute_node_add_fail_count = tbl_iproute_node_add_fail_count_get_iproute_node_add_fail_count(&iproute_node_add_fail_count.key);
    if (NULL != pst_iproute_node_add_fail_count)
    {
        tbl_iproute_node_add_fail_count_del_iproute_node_add_fail_count(&iproute_node_add_fail_count.key);
    }

    return PM_E_NONE;
}

int32
rt_fea_ipv4_add_to_iproute_node_add_fail
    (vrf_id_t vrf_id, addr_ipv4_t *dest, uint8 prefixlen, addr_ipv4_t *gate, nexthop_key_t *nhg, uint32 add_nh_num)
{
    tbl_iproute_node_add_fail_t  *pst_iproute_node_add_fail = NULL;
    tbl_iproute_node_add_fail_t  iproute_node_add_fail;
    nexthop_key_t       nh_key;
    int32 index = 0;

    if (NULL == dest )
    {
        DEBUG_ROUTE_IPV4("dest is NULL, return");
        return PM_E_FAIL;
    }
    
    addr_t dest_all = {0};
    dest_all.family = AF_INET;
    sal_memcpy(&dest_all.u.prefix4, dest, sizeof(addr_ipv4_t));
            
    sal_memset(&iproute_node_add_fail, 0, sizeof(iproute_node_add_fail));
    sal_memset(&nh_key, 0, sizeof(nh_key));

    /* 1. get route  */
    iproute_node_add_fail.key.vrf_id = vrf_id;
    iproute_node_add_fail.key.p.family = AF_INET;
    iproute_node_add_fail.key.p.prefixlen = prefixlen;
    sal_memcpy (&iproute_node_add_fail.key.p.u.prefix4, dest, sizeof(addr_ipv4_t));
    if (NULL != gate)
    {
        sal_memcpy (&iproute_node_add_fail.key.nh_addr.ipv4, gate, sizeof(addr_ipv4_t));
    
        pst_iproute_node_add_fail = tbl_iproute_node_add_fail_get_iproute_node_add_fail(&iproute_node_add_fail.key);
        if (NULL == pst_iproute_node_add_fail)
        {
            tbl_iproute_node_add_fail_add_iproute_node_add_fail(&iproute_node_add_fail);
        }
    }
    else if (NULL == gate && NULL != nhg)
    {
        for(index = 0; index < add_nh_num; index++)
        {
            sal_memset(&iproute_node_add_fail.key.nh_addr.ipv4, 0, sizeof(iproute_node_add_fail.key.nh_addr.ipv4));
            sal_memcpy(&iproute_node_add_fail.key.nh_addr.ipv4, &nhg[index].u.ipv4, sizeof(addr_ipv4_t));
    
            pst_iproute_node_add_fail = tbl_iproute_node_add_fail_get_iproute_node_add_fail(&iproute_node_add_fail.key);
            if (NULL == pst_iproute_node_add_fail)
            {
                tbl_iproute_node_add_fail_add_iproute_node_add_fail(&iproute_node_add_fail);
            }
        }
    }

    return PM_E_NONE;
}

int32
rt_fea_ipv4_del_from_iproute_node_add_fail
    (vrf_id_t vrf_id, addr_ipv4_t *dest, uint8 prefixlen, addr_ipv4_t *gate, nexthop_key_t *nhg, uint32 add_nh_num)
{
    tbl_iproute_node_add_fail_t  *pst_iproute_node_add_fail = NULL;
    tbl_iproute_node_add_fail_t  iproute_node_add_fail;
    nexthop_key_t       nh_key;
    int32 index = 0;

    if (NULL == dest )
    {
        DEBUG_ROUTE_IPV4("dest is NULL, return");
        return PM_E_FAIL;
    }
    
    addr_t dest_all = {0};
    dest_all.family = AF_INET;
    sal_memcpy(&dest_all.u.prefix4, dest, sizeof(addr_ipv4_t));
            
    sal_memset(&iproute_node_add_fail, 0, sizeof(iproute_node_add_fail));
    sal_memset(&nh_key, 0, sizeof(nh_key));

    /* 1. get route  */
    iproute_node_add_fail.key.vrf_id = vrf_id;
    iproute_node_add_fail.key.p.family = AF_INET;
    iproute_node_add_fail.key.p.prefixlen = prefixlen;
    sal_memcpy (&iproute_node_add_fail.key.p.u.prefix4, dest, sizeof(addr_ipv4_t));
    if (NULL != gate)
    {
        sal_memcpy (&iproute_node_add_fail.key.nh_addr.ipv4, gate, sizeof(addr_ipv4_t));
    
        pst_iproute_node_add_fail = tbl_iproute_node_add_fail_get_iproute_node_add_fail(&iproute_node_add_fail.key);
        if (NULL != pst_iproute_node_add_fail)
        {
            tbl_iproute_node_add_fail_del_iproute_node_add_fail(&iproute_node_add_fail.key);
        }
    }
    else if (NULL == gate && NULL != nhg)
    {
        for(index = 0; index < add_nh_num; index++)
        {
            sal_memset(&iproute_node_add_fail.key.nh_addr.ipv4, 0, sizeof(iproute_node_add_fail.key.nh_addr.ipv4));
            sal_memcpy(&iproute_node_add_fail.key.nh_addr.ipv4, &nhg[index].u.ipv4, sizeof(addr_ipv4_t));
    
            pst_iproute_node_add_fail = tbl_iproute_node_add_fail_get_iproute_node_add_fail(&iproute_node_add_fail.key);
            if (NULL != pst_iproute_node_add_fail)
            {
                tbl_iproute_node_add_fail_del_iproute_node_add_fail(&iproute_node_add_fail.key);
            }
        }
    }
    /*when use fpm, del route nl gate maybe null*/
    /*here delete all iproute_node_add_fail whose dest is same with input dest*/
    #ifdef USE_FPM_IPUC
    else
    {
        ctclib_slistnode_t  *listnode = NULL;
        ctclib_slistnode_t *next = NULL;
        tbl_iproute_node_add_fail_t *p_db_iproute_node_add_fail = NULL;
        tbl_iproute_node_add_fail_master_t *p_master = tbl_iproute_node_add_fail_get_master();
        
        if (NULL == p_master)
        {
            return PM_E_NONE;
        }
        
        CTCLIB_SLIST_LOOP_DEL(p_master->add_fail_list, p_db_iproute_node_add_fail, listnode, next)
        {
            if (p_db_iproute_node_add_fail->key.p.family == iproute_node_add_fail.key.p.family
                && p_db_iproute_node_add_fail->key.p.prefixlen == iproute_node_add_fail.key.p.prefixlen
                && p_db_iproute_node_add_fail->key.vrf_id == iproute_node_add_fail.key.vrf_id
                && (0 == sal_memcmp(&p_db_iproute_node_add_fail->key.p.u.prefix4, &iproute_node_add_fail.key.p.u.prefix4, sizeof(addr_ipv4_t))))
            {
                tbl_iproute_node_add_fail_del_iproute_node_add_fail(&p_db_iproute_node_add_fail->key);
            }
        }
    }
    #endif

    return PM_E_NONE;
}

int32
rt_fea_ipv6_add_to_iproute_node_add_fail
    (vrf_id_t vrf_id, addr_ipv6_t *dest, uint8 prefixlen, addr_ipv6_t *gate, nexthop_key_t *nhg, uint32 add_nh_num)
{
    tbl_iproute_node_add_fail_t  *pst_iproute_node_add_fail = NULL;
    tbl_iproute_node_add_fail_t  iproute_node_add_fail;
    nexthop_key_t       nh_key;
    int32 index = 0;

    if (NULL == dest )
    {
        DEBUG_ROUTE_IPV6("dest is NULL, return");
        return PM_E_FAIL;
    }
    
    addr_t dest_all = {0};
    dest_all.family = AF_INET6;
    sal_memcpy(&dest_all.u.prefix6, dest, sizeof(addr_ipv6_t));
            
    sal_memset(&iproute_node_add_fail, 0, sizeof(iproute_node_add_fail));
    sal_memset(&nh_key, 0, sizeof(nh_key));

    /* 1. get route  */
    iproute_node_add_fail.key.vrf_id = vrf_id;
    iproute_node_add_fail.key.p.family = AF_INET6;
    iproute_node_add_fail.key.p.prefixlen = prefixlen;
    sal_memcpy (&iproute_node_add_fail.key.p.u.prefix6, dest, sizeof(addr_ipv6_t));

    if (NULL != gate)
    {
        sal_memcpy (&iproute_node_add_fail.key.nh_addr.ipv6, gate, sizeof(addr_ipv6_t));
        
        pst_iproute_node_add_fail = tbl_iproute_node_add_fail_get_iproute_node_add_fail(&iproute_node_add_fail.key);
        if (NULL == pst_iproute_node_add_fail)
        {
            tbl_iproute_node_add_fail_add_iproute_node_add_fail(&iproute_node_add_fail);
        }
    }
    else if (NULL == gate && NULL != nhg)
    {
        for(index = 0; index < add_nh_num; index++)
        {
            sal_memset(&iproute_node_add_fail.key.nh_addr.ipv6, 0, sizeof(iproute_node_add_fail.key.nh_addr.ipv6));
            sal_memcpy(&iproute_node_add_fail.key.nh_addr.ipv6, &nhg[index].u.ipv6, sizeof(addr_ipv6_t));
    
            pst_iproute_node_add_fail = tbl_iproute_node_add_fail_get_iproute_node_add_fail(&iproute_node_add_fail.key);
            if (NULL == pst_iproute_node_add_fail)
            {
                tbl_iproute_node_add_fail_add_iproute_node_add_fail(&iproute_node_add_fail);
            }
        }
    }

    return PM_E_NONE;
}

int32
rt_fea_ipv6_del_from_iproute_node_add_fail
    (vrf_id_t vrf_id, addr_ipv6_t *dest, uint8 prefixlen, addr_ipv6_t *gate, nexthop_key_t *nhg, uint32 add_nh_num)
{
    tbl_iproute_node_add_fail_t  *pst_iproute_node_add_fail = NULL;
    tbl_iproute_node_add_fail_t  iproute_node_add_fail;
    nexthop_key_t       nh_key;
    int32 index = 0;

    if (NULL == dest )
    {
        DEBUG_ROUTE_IPV6("dest is NULL, return");
        return PM_E_FAIL;
    }
    
    addr_t dest_all = {0};
    dest_all.family = AF_INET6;
    sal_memcpy(&dest_all.u.prefix6, dest, sizeof(addr_ipv6_t));
            
    sal_memset(&iproute_node_add_fail, 0, sizeof(iproute_node_add_fail));
    sal_memset(&nh_key, 0, sizeof(nh_key));

    /* 1. get route  */
    iproute_node_add_fail.key.vrf_id = vrf_id;
    iproute_node_add_fail.key.p.family = AF_INET6;
    iproute_node_add_fail.key.p.prefixlen = prefixlen;
    sal_memcpy (&iproute_node_add_fail.key.p.u.prefix6, dest, sizeof(addr_ipv6_t));

    if (NULL != gate)
    {
        sal_memcpy (&iproute_node_add_fail.key.nh_addr.ipv6, gate, sizeof(addr_ipv6_t));
        
        pst_iproute_node_add_fail = tbl_iproute_node_add_fail_get_iproute_node_add_fail(&iproute_node_add_fail.key);
        if (NULL != pst_iproute_node_add_fail)
        {
            tbl_iproute_node_add_fail_del_iproute_node_add_fail(&iproute_node_add_fail.key);
        }
    }
    else if (NULL == gate && NULL != nhg)
    {
        for(index = 0; index < add_nh_num; index++)
        {
            sal_memset(&iproute_node_add_fail.key.nh_addr.ipv6, 0, sizeof(iproute_node_add_fail.key.nh_addr.ipv6));
            sal_memcpy(&iproute_node_add_fail.key.nh_addr.ipv6, &nhg[index].u.ipv6, sizeof(addr_ipv6_t));
    
            pst_iproute_node_add_fail = tbl_iproute_node_add_fail_get_iproute_node_add_fail(&iproute_node_add_fail.key);
            if (NULL != pst_iproute_node_add_fail)
            {
                tbl_iproute_node_add_fail_del_iproute_node_add_fail(&iproute_node_add_fail.key);
            }
        }
    }

    return PM_E_NONE;
}

int32
rt_fea_ipv6_add_route_nl(vrf_id_t vrf_id, addr_ipv6_t *dest, addr_ipv6_t *gate, uint8 prefixlen,
                  uint32 ifindex, uint32 distance, uint32 nexthop_type)
{
    char        buf1[CMD_BUF_256] = {0};
    char        buf2[CMD_BUF_256] = {0};

    /*gate can be null*/
    if (NULL != dest && NULL != gate)
    {
        DEBUG_ROUTE_IPV6("dest(%s), gate(%s), prefixlen(%u),  ifindex(%u), distance(%u), nexthop_type(%s)", 
            cdb_addr_ipv6_val2str(buf1, CMD_BUF_256, dest), cdb_addr_ipv6_val2str(buf2, CMD_BUF_256, gate), 
            prefixlen, ifindex, distance, rt_fea_api_nexthop_type_desc(nexthop_type));
    }
    else if (NULL != dest &&  NULL == gate)
    {
        DEBUG_ROUTE_IPV6("dest(%s), prefixlen(%u), ifindex(%u), distance(%u), nexthop_type(%s)", 
            cdb_addr_ipv6_val2str(buf1, CMD_BUF_256, dest), 
            prefixlen, ifindex, distance, rt_fea_api_nexthop_type_desc(nexthop_type));
    }
    else if (NULL == dest &&  NULL != gate)
    {
        DEBUG_ROUTE_IPV6("gate(%s), prefixlen(%u), ifindex(%u), distance(%u), nexthop_type(%s)", 
            cdb_addr_ipv6_val2str(buf2, CMD_BUF_256, gate), 
            prefixlen, ifindex, distance, rt_fea_api_nexthop_type_desc(nexthop_type));
    }
    else
    {
        DEBUG_ROUTE_IPV6("dest and gate are NULL");
    }

    if (IN6_IS_ADDR_LINKLOCAL(dest))
    {
        DEBUG_ROUTE_IPV6("Linklocal address %s, not commit to fea, return", cdb_addr_ipv6_val2str(buf1, sizeof(buf1), dest));
        return PM_E_NONE;
    }

    if (IN6_IS_ADDR_MULTICAST(dest))
    {
        DEBUG_ROUTE_IPV6("MULTICAST address %s, not commit to fea, return", cdb_addr_ipv6_val2str(buf1, sizeof(buf1), dest));
        return PM_E_NONE;
    }

    tbl_kernel_if_t     *p_db_kernel_if   = NULL;
    tbl_interface_t     *p_db_if = NULL;
    tbl_nexthop_t       *pst_nexthop      = NULL;
    tbl_iproute_node_t  *pst_iproute_node = NULL;
    tbl_iproute_node_t  iproute_node;
    tbl_kernel_if_t     kernel_if;
    uint32              over_check = 0;
    nexthop_key_t       nh_key;
    int32               rc = PM_E_NONE;
#ifndef USE_FPM_IPUC
    uint32 nh_num = 0;
    uint32  ecmp_hash = 0;

    nexthop_key_t nhg[IP_ECMP_MAX_PATH];
#endif

    if (NULL == dest )
    {
        DEBUG_ROUTE_IPV6("dest is NULL, return");
        return PM_E_FAIL;
    }
    
    addr_t dest_all = {0};
    dest_all.family = AF_INET6;
    sal_memcpy(&dest_all.u.prefix6, dest, sizeof(addr_ipv6_t));
            
    sal_memset(&iproute_node, 0, sizeof(iproute_node));
    sal_memset(&nh_key, 0, sizeof(nh_key));

    /* 1. get route  */
    iproute_node.key.vrf_id = vrf_id;
    iproute_node.key.p.family = AF_INET6;
    iproute_node.key.p.prefixlen = prefixlen;
    sal_memcpy (&iproute_node.key.p.u.prefix6, dest, sizeof(addr_ipv6_t));
    pst_iproute_node = tbl_iproute_node_get_iproute_node(&iproute_node.key);

    /*for bug 48636: static route destnation address is same with connect address, compose ecmp*/
    /*current is local route, distance is 0, then add a static route, distance is 1, so static route is not allowed*/
    if (pst_iproute_node && 0 == pst_iproute_node->multipath 
        && NH_TYPE_LOCAL == pst_iproute_node->nh_key.type 
        && RTN_UNICAST == nexthop_type)
    {
        DEBUG_ROUTE_IPV6("current route is local, new is unicast, not add, return");
        rt_fea_ipv6_add_to_iproute_node_add_fail(vrf_id, dest, prefixlen, gate, NULL, 0);
        return PM_E_NONE;
    }

    /* filter manage if configurations */
    if (GLB_IF_TYPE_NULL == tbl_interface_get_type_by_ifindex(ifindex))
    {
        /*NULL0 not have interface*/
        if (RTN_BLACKHOLE != nexthop_type)
        {
            DEBUG_ROUTE_IPV6("nexthop type is not blackhole, return fail");
            return PM_E_FAIL;
        }
    }
    
    if (RTN_BLACKHOLE != nexthop_type)
    {
        /* 2. get ifname by kernel ifindex */
        kernel_if.key.ifindex = ifindex;
        p_db_kernel_if = tbl_kernel_if_get_kernel_if(&kernel_if.key);
        if (NULL == p_db_kernel_if)
        {
            DEBUG_ROUTE_IPV6("kernel if %u does not exist, return fail", ifindex);
            return PM_E_FAIL;
        }
        else
        {
            p_db_if = tbl_interface_get_interface_by_name(p_db_kernel_if->name);
            //if (NULL != p_db_if && !tbl_interface_is_running(p_db_if))
            if (NULL == p_db_if /*&& !lib_ioctl_if_get_flags_and_check(p_db_kernel_if->name)*/)
            {
                DEBUG_ROUTE_IPV6("interface %s does not exist, return", p_db_kernel_if->name);
                return PM_E_NONE;
            }

            /* modified by liwh for bug 44633, 2017-07-26 */
            if (!lib_ioctl_if_get_flags_and_check(p_db_kernel_if->name))
            {
                return PM_E_NONE;   
            }
            /* liwh end */
        }
    }

    //log_sys(M_MOD_ROUTE, E_ERROR, "dest = %x, ifname = %s\n", *dest,  p_db_kernel_if->name);
    
    if (NULL != gate)
    {
        rc = rt_fea_ipv6_gate_check(gate);
        if (PM_E_NONE != rc)
        {
            rt_fea_ipv6_add_to_iproute_node_add_fail(vrf_id, dest, prefixlen, gate, NULL, 0);
            return PM_E_NONE;
        }
    }
    
    if (NULL != pst_iproute_node)
    {
        /*route update from ecmp to normal*/
#ifdef USE_FPM_IPUC
        if(NULL != gate && pst_iproute_node->multipath)
#else
        if (0)
#endif
        {
            DEBUG_ROUTE_IPV6("add ipv6 route: ecmp to normal");
            PM_E_RETURN(rt_fea_ipv6_get_nh_key(vrf_id, dest, gate, prefixlen, ifindex, distance, &nh_key));
            /*update ecmp group members*/
            rt_fea_ipv6_update_nh_by_group(vrf_id, &dest_all, &nh_key, 1, prefixlen, distance);
    
            /*update route to nh, update nexthop group last member to nexthop*/
            sal_memcpy(&pst_iproute_node->nh_key, &nh_key, sizeof(nexthop_key_t));
            PM_E_RETURN(tbl_iproute_node_set_iproute_node_field(pst_iproute_node, TBL_IPROUTE_NODE_FLD_NH_KEY));
            
            PM_E_RETURN(rt_fea_ipv6_del_nhg(pst_iproute_node, NULL, pst_iproute_node->nh_group.nh_num, 0, FALSE));
            pst_iproute_node->nh_group.nh_num = 0;
            sal_memset(pst_iproute_node->nh_group.nh, 0, sizeof(pst_iproute_node->nh_group.nh));
            //PM_E_RETURN(tbl_iproute_node_set_iproute_node_field(pst_iproute_node, TBL_IPROUTE_NODE_FLD_NH_GROUP));
    
            pst_iproute_node->multipath = 0;
            PM_E_RETURN(tbl_iproute_node_set_iproute_node_field(pst_iproute_node, TBL_IPROUTE_NODE_FLD_MULTIPATH));

            return PM_E_NONE;

        }
#ifndef USE_FPM_IPUC
        /*normal to ecmp 2*/
        else if(NULL != gate && 0 == pst_iproute_node->multipath)
        {
            DEBUG_ROUTE_IPV6("add ipv6 route: normal to ecmp 2");

            if (rt_fea_v6_ecmp_count_max_check())
            {
                DEBUG_ROUTE_IPV6("The ecmp entry num is more than maximum when change ipv6 route to ecmp!");
                log_sys(M_MOD_ROUTE, E_ERROR, "The ecmp entry num is more than maximum %u when change ipv6 route to ecmp!", 
                    rt_fea_v6_ecmp_count_get_max());

                rt_fea_ipv6_add_to_iproute_node_add_fail(vrf_id, dest, prefixlen, gate, NULL, 0);
                return PM_E_EXCEED_ECMP_MAX_NUM;
            }
        
            PM_E_RETURN(rt_fea_ipv6_get_nh_key(vrf_id, dest, gate, prefixlen, ifindex, distance, &nh_key));

            /*update ecmp group members*/
            nh_num = 2;
            sal_memset(nhg, 0, sizeof(nhg));
            sal_memcpy(&nhg, &pst_iproute_node->nh_key, sizeof(nexthop_key_t));
            sal_memcpy(&nhg[1], &nh_key, sizeof(nexthop_key_t));
            rt_fea_ipv6_update_nh_by_group(vrf_id, &dest_all, nhg, nh_num, prefixlen, distance);
    
            /*update route to nh, update nexthop group last member to nexthop*/
            //sal_memcpy(&pst_iproute_node->nh_key, &nh_key, sizeof(nexthop_key_t));
            //PM_E_RETURN(tbl_iproute_node_set_iproute_node_field(pst_iproute_node, TBL_IPROUTE_NODE_FLD_NH_KEY));

            //ecmp_hash = rt_fea_ipv4_get_ecmp_hash_type(vrf_id, dest, prefixlen, nh_num);
            PM_E_RETURN(rt_fea_ipv6_add_nhg(pst_iproute_node, nhg, nh_num, ecmp_hash, TRUE, FALSE));

            pst_iproute_node->nh_group.nh_num = nh_num;
            sal_memset(pst_iproute_node->nh_group.nh, 0, sizeof(pst_iproute_node->nh_group.nh));
            sal_memcpy(pst_iproute_node->nh_group.nh, nhg, sizeof(pst_iproute_node->nh_group.nh));
            PM_E_RETURN(tbl_iproute_node_set_iproute_node_field(pst_iproute_node, TBL_IPROUTE_NODE_FLD_NH_GROUP));
    
            pst_iproute_node->multipath = 1;
            PM_E_RETURN(tbl_iproute_node_set_iproute_node_field(pst_iproute_node, TBL_IPROUTE_NODE_FLD_MULTIPATH));

            return PM_E_NONE;
        }
        /*ecmp to ecmp*/
        else if(NULL != gate && 1 == pst_iproute_node->multipath)
        {
            DEBUG_ROUTE_IPV6("add ipv6 route: ecmp %u to ecmp %u", pst_iproute_node->nh_group.nh_num, pst_iproute_node->nh_group.nh_num + 1);
            PM_E_RETURN(rt_fea_ipv6_get_nh_key(vrf_id, dest, gate, prefixlen, ifindex, distance, &nh_key));

            nh_num = rt_fea_v6_ecmp_member_count_get_max();
            if (nh_num < pst_iproute_node->nh_group.nh_num + 1)
            {
                DEBUG_ROUTE_IPV6("add ipv6 route: ecmp %u to ecmp %u, too many member", pst_iproute_node->nh_group.nh_num, pst_iproute_node->nh_group.nh_num + 1);
                    PM_E_RETURN(rt_fea_ipv6_get_nh_key(vrf_id, dest, gate, prefixlen, ifindex, distance, &nh_key));
                rt_fea_ipv6_add_to_iproute_node_add_fail(vrf_id, dest, prefixlen, gate, NULL, 0);
                return PM_E_FAIL;
            }
    
            /*update ecmp group members*/
            nh_num = pst_iproute_node->nh_group.nh_num + 1;
            sal_memset(nhg, 0, sizeof(nhg));
            sal_memcpy(&nhg, &pst_iproute_node->nh_group.nh, sizeof(pst_iproute_node->nh_group.nh));
            sal_memcpy(&nhg[nh_num - 1], &nh_key, sizeof(nexthop_key_t));
            rt_fea_ipv6_update_nh_by_group(vrf_id, &dest_all, nhg, nh_num, prefixlen, distance);

            nexthop_ecmp_t nh_group;
            sal_memset(&nh_group, 0, sizeof(nh_group));
            /* update nhg, add new  nhg*/
            sal_memcpy(&nh_group, &pst_iproute_node->nh_group, sizeof(pst_iproute_node->nh_group));
        
            /*update route to nh, update nexthop group last member to nexthop*/
            //sal_memcpy(&pst_iproute_node->nh_key, &nh_key, sizeof(nexthop_key_t));
            //PM_E_RETURN(tbl_iproute_node_set_iproute_node_field(pst_iproute_node, TBL_IPROUTE_NODE_FLD_NH_KEY));

            //ecmp_hash = rt_fea_ipv4_get_ecmp_hash_type(vrf_id, dest, prefixlen, nh_num);
            PM_E_RETURN(rt_fea_ipv6_add_nhg(pst_iproute_node, nhg, nh_num, ecmp_hash, FALSE, FALSE));

            rt_fea_ipv6_del_from_iproute_node_add_fail(vrf_id, dest, prefixlen, NULL, nhg, nh_num);

            pst_iproute_node->nh_group.nh_num = nh_num;
            sal_memset(pst_iproute_node->nh_group.nh, 0, sizeof(pst_iproute_node->nh_group.nh));
            sal_memcpy(pst_iproute_node->nh_group.nh, nhg, sizeof(pst_iproute_node->nh_group.nh));
            PM_E_RETURN(tbl_iproute_node_set_iproute_node_field(pst_iproute_node, TBL_IPROUTE_NODE_FLD_NH_GROUP));
    
           /*remove old nhg*/
            PM_E_RETURN(rt_fea_ipv6_del_nhg(pst_iproute_node, nh_group.nh, nh_group.nh_num, 0, FALSE));
            
            rt_fea_ipv6_del_from_iproute_node_add_fail(vrf_id, dest, prefixlen, NULL, nh_group.nh, nh_group.nh_num);
        
            return PM_E_NONE;
        }
#endif
        /*route update from linklocal or black hole to normal*/
        else if (NULL != gate && (pst_iproute_node->nh_key.type == NH_TYPE_LOCAL || pst_iproute_node->nh_key.type == NH_TYPE_BLACKHOLE))
        {
            DEBUG_ROUTE_IPV6("add ipv6 route: linklocal or black hole to normal");
            PM_E_RETURN(rt_fea_ipv6_get_nh_key(vrf_id, dest, gate, prefixlen, ifindex, distance, &nh_key));
            /*update ecmp group members*/
            rt_fea_ipv6_update_nh_by_group(vrf_id, &dest_all, &nh_key, 1, prefixlen, distance);

            rt_fea_ipv6_del_from_iproute_node_add_fail(vrf_id, dest, prefixlen, gate, NULL, 0);

            /*update route to nh, update nexthop group last member to nexthop*/
            sal_memcpy(&pst_iproute_node->nh_key, &nh_key, sizeof(nexthop_key_t));
            PM_E_RETURN(tbl_iproute_node_set_iproute_node_field(pst_iproute_node, TBL_IPROUTE_NODE_FLD_NH_KEY));
            
            return PM_E_NONE;
        }
        /*route update from normal to normal*/
        else if (NULL != gate && 0 != sal_memcmp(&pst_iproute_node->nh_key.u.ipv6, gate, sizeof(addr_ipv6_t)))
        {
            DEBUG_ROUTE_IPV6("add ipv6 route: normal to normal");
            PM_E_RETURN(rt_fea_ipv6_get_nh_key(vrf_id, dest, gate, prefixlen, ifindex, distance, &nh_key));
            /*update ecmp group members*/
            rt_fea_ipv6_update_nh_by_group(vrf_id, &dest_all, &nh_key, 1, prefixlen, distance);

            rt_fea_ipv6_del_from_iproute_node_add_fail(vrf_id, dest, prefixlen, &pst_iproute_node->nh_key.u.ipv6, NULL, 0);

            rt_fea_ipv6_del_from_iproute_node_add_fail(vrf_id, dest, prefixlen, gate, NULL, 0);
    
            /*update route to nh, update nexthop group last member to nexthop*/
            sal_memcpy(&pst_iproute_node->nh_key, &nh_key, sizeof(nexthop_key_t));
            PM_E_RETURN(tbl_iproute_node_set_iproute_node_field(pst_iproute_node, TBL_IPROUTE_NODE_FLD_NH_KEY));
            
            return PM_E_NONE;
        }
        /*route update to linklocal or black hole*/
        else if(NULL == gate) 
        {
            DEBUG_ROUTE_IPV6("add ipv6 route: update to linklocal or black hole");
            /*update old nexthop*/
            rt_fea_ipv6_update_nh_by_group(vrf_id, &dest_all, &nh_key, 0, prefixlen, distance);
            
            sal_memset(&pst_iproute_node->nh_key, 0, sizeof(nexthop_key_t));
            /*add by chenc for route update to black hole route for bug 45589*/
            if (nexthop_type == RTN_BLACKHOLE)
            {
                pst_iproute_node->nh_key.type = NH_TYPE_BLACKHOLE;
            }
            else
            {
                pst_iproute_node->nh_key.type = NH_TYPE_LOCAL;
                sal_memcpy(pst_iproute_node->nh_key.ifname, p_db_kernel_if->name, IFNAME_SIZE);
            }
            
            PM_E_RETURN(tbl_iproute_node_set_iproute_node_field(pst_iproute_node, TBL_IPROUTE_NODE_FLD_NH_KEY));

            /*del nexthop group if need*/
            if(pst_iproute_node->multipath)
            {
                PM_E_RETURN(rt_fea_ipv6_del_nhg(pst_iproute_node, NULL, pst_iproute_node->nh_group.nh_num, 0, FALSE));
                pst_iproute_node->nh_group.nh_num = 0;
                sal_memset(pst_iproute_node->nh_group.nh, 0, sizeof(pst_iproute_node->nh_group.nh));
                //PM_E_RETURN(tbl_iproute_node_set_iproute_node_field(pst_iproute_node, TBL_IPROUTE_NODE_FLD_NH_GROUP));
                pst_iproute_node->multipath = 0;
                PM_E_RETURN(tbl_iproute_node_set_iproute_node_field(pst_iproute_node, TBL_IPROUTE_NODE_FLD_MULTIPATH));
            }
            
            return PM_E_NONE;
        }
        else
        {
            DEBUG_ROUTE_IPV6("add ipv6 route: already exist");
            return PM_E_EXIST;
        }
    }
    else
    {
        DEBUG_ROUTE_IPV6("add ipv6 route: new add");
        /* 3. add route */
        over_check = rt_fea_indirect_route_v6_count_max_check();
        if (over_check)
        {
            rt_fea_ipv6_add_to_iproute_node_add_fail(vrf_id, dest, prefixlen, gate, NULL, 0);
            DEBUG_ROUTE_IPV6("v6 route max check failed, return");
            return PM_E_NONE;
        }
    
        sal_memset(&iproute_node, 0, sizeof(iproute_node));
        if (NULL != gate)
        {
            pst_nexthop = rt_fea_nexthop_ipv6_add(p_db_kernel_if->name, gate);
            if (NULL == pst_nexthop)
            {
                rt_fea_ipv6_add_to_iproute_node_add_fail(vrf_id, dest, prefixlen, gate, NULL, 0);
                DEBUG_ROUTE_IPV6("nexthop %s %s add failed, return failed", p_db_kernel_if->name, cdb_addr_ipv6_val2str(buf1, sizeof(buf1), gate));
                return PM_E_FAIL;
            }
    
            sal_memcpy(&iproute_node.nh_key, &pst_nexthop->key, sizeof(iproute_node.nh_key));

            /* support nd modified by liwh for bug 47547, 2018-07-25 */
            nd_neigh_route_add(pst_nexthop);
            /* liwh end */
        }
        /*add by chenc for new add a black hole route for bug 45589*/
        else if (RTN_BLACKHOLE == nexthop_type)
        {
            iproute_node.nh_key.type = NH_TYPE_BLACKHOLE;
        }
        else
        {
            iproute_node.nh_key.type = NH_TYPE_LOCAL;
            sal_memcpy(iproute_node.nh_key.ifname, p_db_kernel_if->name, IFNAME_SIZE);
        }
    
        iproute_node.key.vrf_id = vrf_id;
        iproute_node.key.p.family = AF_INET6;
        iproute_node.key.p.prefixlen = prefixlen;
        sal_memcpy (&iproute_node.key.p.u.prefix6, dest, sizeof(addr_ipv6_t));
        iproute_node.distance = distance;
        rc = tbl_iproute_node_add_iproute_node(&iproute_node);
        if (!rc)
        {
            rt_fea_route_v6_count_increase();
            rt_route_v6_counter_inc(DEFAULT_VRF_ID);
        }
        else
        {
            DEBUG_ROUTE_IPV6("iproute node %s %s add failed, return failed", p_db_kernel_if->name, cdb_addr_ipv6_val2str(buf1, sizeof(buf1), dest));
        }
    
        return rc;
    }
    
    return PM_E_NONE;
}

int32
rt_fea_ipv6_del_route_nl(vrf_id_t vrf_id, addr_ipv6_t *dest, addr_ipv6_t *gate, uint8 prefixlen,
                  uint32 ifindex, uint32 distance, uint32 nexthop_type)
{
    char        buf1[CMD_BUF_256] = {0};
    char        buf2[CMD_BUF_256] = {0};

    if (NULL != dest && NULL != gate)
    {
        DEBUG_ROUTE_IPV6("dest(%s), gate(%s), prefixlen(%u),  ifindex(%u), distance(%u), nexthop_type(%s)", 
            cdb_addr_ipv6_val2str(buf1, CMD_BUF_256, dest), cdb_addr_ipv6_val2str(buf2, CMD_BUF_256, gate), 
            prefixlen, ifindex, distance, rt_fea_api_nexthop_type_desc(nexthop_type));
    }
    else if (NULL != dest &&  NULL == gate)
    {
        DEBUG_ROUTE_IPV6("dest(%s), prefixlen(%u), ifindex(%u), distance(%u), nexthop_type(%s)", 
            cdb_addr_ipv6_val2str(buf1, CMD_BUF_256, dest), 
            prefixlen, ifindex, distance, rt_fea_api_nexthop_type_desc(nexthop_type));
    }
    else if (NULL == dest &&  NULL != gate)
    {
        DEBUG_ROUTE_IPV6("gate(%s), prefixlen(%u),  ifindex(%u), distance(%u), nexthop_type(%s)", 
            cdb_addr_ipv6_val2str(buf2, CMD_BUF_256, gate), 
            prefixlen, ifindex, distance, rt_fea_api_nexthop_type_desc(nexthop_type));
    }
    else
    {
        DEBUG_ROUTE_IPV6("dest and gate are NULL");
    }

    tbl_iproute_node_t  *pst_iproute_node   = NULL;
    tbl_nexthop_t       *pst_nexthop = NULL;
    tbl_iproute_node_t  iproute_node;
    tbl_iproute_node_t  iproute_node_cp;
    int32               rc = PM_E_NONE;

    if (NULL == dest)
    {
        DEBUG_ROUTE_IPV6("dest is NULL, return");
        return PM_E_FAIL;
    }

#ifdef USE_FPM_IPUC
    /*fpm ipuc: delete route nexthop type/ifindex and gate are not specified 
    referance to: 
    static int netlink_route_info_fill()
        if (cmd == RTM_DELROUTE && !rib)
            return 1;
    */
    if (!(RTN_UNSPEC == nexthop_type && 0 == ifindex && NULL == gate))
#endif
    {
        /* filter manage if configurations */
        if (GLB_IF_TYPE_NULL == tbl_interface_get_type_by_ifindex(ifindex))
        {
            if (RTN_BLACKHOLE != nexthop_type)
            {
                DEBUG_ROUTE_IPV6("nexthop type is not blackhole, return fail");
                return PM_E_FAIL;
            }
        }
    }

    addr_t dest_all = {0};
    dest_all.family = AF_INET6;
    sal_memcpy(&dest_all.u.prefix6, dest, sizeof(addr_ipv6_t));

    /* 1. del route */
    sal_memset(&iproute_node, 0, sizeof(iproute_node));
    sal_memset(&iproute_node_cp, 0, sizeof(iproute_node_cp));

    iproute_node.key.vrf_id = vrf_id;
    iproute_node.key.p.family = AF_INET6;
    iproute_node.key.p.prefixlen = prefixlen;
    sal_memcpy (&iproute_node.key.p.u.prefix6, dest, sizeof(addr_ipv6_t));
    pst_iproute_node = tbl_iproute_node_get_iproute_node(&iproute_node.key);

    //rt_fea_ipv6_del_from_iproute_node_add_fail(vrf_id, dest, prefixlen, gate, NULL, 0);
    
    if (NULL == pst_iproute_node)
    {
        rt_fea_ipv6_del_from_iproute_node_add_fail(vrf_id, dest, prefixlen, gate, NULL, 0);
        DEBUG_ROUTE_IPV6("pst_iproute_node is NULL, return");
        return PM_E_NONE;
    }
    
    /* modified by liwh for bug 45983, 2017-12-28 */
    if (pst_iproute_node)
    {
        if ((NULL != gate) && (0 != sal_memcmp(&pst_iproute_node->nh_key.u.ipv6, gate, sizeof(addr_ipv6_t))))
        {
            DEBUG_ROUTE_IPV6("gate %s, pst_iproute_node->nh_key.u.ipv6 %s errorrrrrr\n", 
                cdb_addr_ipv6_val2str(buf1, sizeof(buf1), gate), cdb_addr_ipv6_val2str(buf2, sizeof(buf2), &pst_iproute_node->nh_key.u.ipv6));

            return PM_E_NONE;
        }
    }
    /* liwh end */

    /* modified by liwh for bug 45943, 2017-12-21 */
    if (pst_iproute_node 
        && (RTN_LOCAL != nexthop_type)
        && (NH_TYPE_LOCAL == pst_iproute_node->nh_key.type))
    {
        DEBUG_ROUTE_IPV6("dest %s, ifname %s is local ip address, return\n", 
            cdb_addr_ipv6_val2str(buf1, sizeof(buf1), dest), 
            pst_iproute_node->nh_key.ifname);
        return PM_E_NONE;
    }
    /* liwh end */

    if (NULL != pst_iproute_node)
    {
        sal_memcpy(&iproute_node_cp, pst_iproute_node, sizeof(iproute_node_cp));

        /*delete ecmp*/
        if (pst_iproute_node->multipath)
        {
            DEBUG_ROUTE_IPV6("del ipv6 route: delete ecmp");

            rt_fea_route_v6_count_decrease();
            rt_route_v6_counter_dec(DEFAULT_VRF_ID);
            /*dulplicate with rt_fea_ipv6_del_nhg->rt_ecmp_v6_counter_dec*/
            //rt_ecmp_v6_counter_dec(DEFAULT_VRF_ID);

            rt_fea_ipv6_del_nh_by_group(&iproute_node_cp, vrf_id, NULL, NULL, iproute_node_cp.nh_group.nh_num, 0, 0);

            tbl_nexthop_group_t nhgroup;
            sal_memset(&nhgroup, 0, sizeof(nhgroup));

            nhgroup.key.nh_num = pst_iproute_node->nh_group.nh_num;
            sal_memcpy(nhgroup.key.nh, pst_iproute_node->nh_group.nh, pst_iproute_node->nh_group.nh_num*sizeof(nexthop_key_t));

            //rt_fea_ipv6_del_from_iproute_node_add_fail(vrf_id, dest, prefixlen, NULL, pst_iproute_node->nh_group.nh, nhgroup.key.nh_num);

            tbl_iproute_node_del_iproute_node(&iproute_node.key);
        
            //PM_E_RETURN(rt_fea_ipv6_del_nhg(pst_iproute_node, NULL, nh_num, FALSE));
            /*pst_iproute_node has been delete, so pst_iproute_node is invalid*/
            PM_E_RETURN(rt_fea_ipv6_del_nhg(NULL, nhgroup.key.nh, nhgroup.key.nh_num, 0, FALSE));
            return PM_E_NONE;
        }
        else if (0 == pst_iproute_node->multipath)
        {
            DEBUG_ROUTE_IPV6("del ipv6 route: del last one");
            
            //rt_fea_ipv6_del_from_iproute_node_add_fail(vrf_id, dest, prefixlen, gate, NULL, 0);
#ifdef USE_FPM_IPUC
        pst_nexthop = rt_fea_nexthop_ipv6_get_for_use_fpm_ipuc(&pst_iproute_node->nh_key);
#endif
            rc = tbl_iproute_node_del_iproute_node(&iproute_node.key);
            rt_fea_route_v6_count_decrease();
            rt_route_v6_counter_dec(DEFAULT_VRF_ID);
        }
    }

    /*black hole not have nexthop, no need to delete nexthop*/
    if (NH_TYPE_BLACKHOLE == iproute_node_cp.nh_key.type)
    {
        DEBUG_ROUTE_IPV6("nexthop type is blackhole, return");
        return PM_E_NONE;
    }

    /* shouldn't deleted nexthop when iproute_node is local by liwh for bug 45444, 2017-10-26 */
    if (NH_TYPE_LOCAL == iproute_node_cp.nh_key.type)
    {
        DEBUG_ROUTE_IPV6("nexthop type is local, return");
        return PM_E_NONE;
    }
    /* liwh end */

#ifdef USE_FPM_IPUC
    if (NULL != pst_nexthop)
    {
        nd_neigh_route_del(pst_nexthop);
        rt_fea_nexthop_ipv6_del_for_use_fpm_ipuc(pst_nexthop);
    }
#else
    tbl_kernel_if_t     *p_db_kernel_if     = NULL;
    tbl_kernel_if_t     kernel_if;
    
    /* 2. get ifname by kernel ifindex */
    kernel_if.key.ifindex = ifindex;
    p_db_kernel_if = tbl_kernel_if_get_kernel_if(&kernel_if.key);
    if (NULL == p_db_kernel_if)
    {
        DEBUG_ROUTE_IPV6("kernel if %u does not exist, return fail", ifindex);
        return PM_E_FAIL;
    }

    /* 3. try to remove nexthop */
    if (NULL != gate)
    {
        pst_nexthop = rt_fea_nexthop_ipv6_get(p_db_kernel_if->name, gate);
        if (NULL != pst_nexthop)
        {
            /* support nd modified by liwh for bug 47547, 2018-07-25 */
            nd_neigh_route_del(pst_nexthop);
            /* liwh end */
            rt_fea_nexthop_ipv6_del(p_db_kernel_if->name, gate);
        }
    }
#endif

    return rc;
}

int32
rt_fea_ipv4_add_route_nl(vrf_id_t vrf_id, addr_ipv4_t *dest, addr_ipv4_t *gate, uint8 prefixlen,
                  uint32 ifindex, uint32 distance, uint32 nexthop_type, uint32 route_type)
{
    tbl_kernel_if_t     *p_db_kernel_if   = NULL;
    tbl_interface_t     *p_db_if = NULL;
    tbl_nexthop_t       *pst_nexthop      = NULL;
    tbl_iproute_node_t  *pst_iproute_node = NULL;
    tbl_iproute_node_t  iproute_node;
    tbl_kernel_if_t     kernel_if;
    uint32              over_check = 0;
    nexthop_key_t       nh_key;
    int32               rc = PM_E_NONE;
    char        buf1[CMD_BUF_256] = {0};
    char        buf2[CMD_BUF_256] = {0};

    //log_sys(M_MOD_ROUTE, E_ERROR, "rt_fea_ipv4_add_route_nl \n");
    
    DEBUG_ROUTE_IPV4();
    /*gate can be null*/
    if (NULL != dest && NULL != gate)
    {
        DEBUG_ROUTE_IPV4("dest(%s), gate(%s), prefixlen(%u),  ifindex(%u), distance(%u), nexthop_type(%s), route_type(%d)\n", 
            cdb_addr_ipv4_val2str(buf1, CMD_BUF_256, dest), cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, gate), 
            prefixlen, ifindex, distance, rt_fea_api_nexthop_type_desc(nexthop_type), route_type);
    }
    else if (NULL != dest &&  NULL == gate)
    {
        DEBUG_ROUTE_IPV4("dest(%s), prefixlen(%u),  ifindex(%u), distance(%u), nexthop_type(%s), route_type(%d)\n", 
            cdb_addr_ipv4_val2str(buf1, CMD_BUF_256, dest), 
            prefixlen, ifindex, distance, rt_fea_api_nexthop_type_desc(nexthop_type), route_type);
    }
    else if (NULL == dest &&  NULL != gate)
    {
        DEBUG_ROUTE_IPV4("gate(%s), prefixlen(%u),  ifindex(%u), distance(%u), nexthop_type(%s), route_type(%d)\n", 
            cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, gate), 
            prefixlen, ifindex, distance, rt_fea_api_nexthop_type_desc(nexthop_type), route_type);
    }

    if (NULL == dest )
    {
        return PM_E_FAIL;
    }
    addr_t dest_all = {0};
    dest_all.family = AF_INET;
    sal_memcpy(&dest_all.u.prefix4, dest, sizeof(addr_ipv4_t));
            
    sal_memset(&iproute_node, 0, sizeof(iproute_node));
    sal_memset(&nh_key, 0, sizeof(nh_key));

    /* 1. get route  */
    iproute_node.key.vrf_id = vrf_id;
    iproute_node.key.p.family = AF_INET;
    iproute_node.key.p.prefixlen = prefixlen;
    sal_memcpy (&iproute_node.key.p.u.prefix4, dest, sizeof(addr_ipv4_t));
    pst_iproute_node = tbl_iproute_node_get_iproute_node(&iproute_node.key);

    /* filter manage if configurations */
    if (GLB_IF_TYPE_NULL == tbl_interface_get_type_by_ifindex(ifindex))
    {
        /*NULL0 not have interface*/
        if (RTN_BLACKHOLE != nexthop_type)
        {
            return PM_E_FAIL;
        }
    }
    
    if (RTN_BLACKHOLE != nexthop_type)
    {
        /* 2. get ifname by kernel ifindex */
        kernel_if.key.ifindex = ifindex;
        p_db_kernel_if = tbl_kernel_if_get_kernel_if(&kernel_if.key);
        if (NULL == p_db_kernel_if)
        {
            return PM_E_FAIL;
        }
        else
        {
            p_db_if = tbl_interface_get_interface_by_name(p_db_kernel_if->name);
            //if (NULL != p_db_if && !tbl_interface_is_running(p_db_if))
            if (NULL == p_db_if /*&& !lib_ioctl_if_get_flags_and_check(p_db_kernel_if->name)*/)
            {
                return PM_E_NONE;
            }

            /* modified by liwh for bug 44633, 2017-07-26 */
            if (!lib_ioctl_if_get_flags_and_check(p_db_kernel_if->name))
            {
                return PM_E_NONE;   
            }
            /* liwh end */
        }
    }

    //log_sys(M_MOD_ROUTE, E_ERROR, "dest = %x, ifname = %s\n", *dest,  p_db_kernel_if->name);
    
    if (NULL != gate)
    {
        rc = rt_fea_ipv4_gate_check(gate);
        if (PM_E_NONE != rc)
        {
            rt_fea_ipv4_add_to_iproute_node_add_fail(vrf_id, dest, prefixlen, gate, NULL, 0);
            rt_fea_ipv4_add_to_iproute_node_add_fail_count(vrf_id, dest, prefixlen, gate, NULL, 0);
            return PM_E_NONE;
        }
    }
    
    if (NULL != pst_iproute_node)
    {
        /*route update from ecmp to normal*/
        if(NULL != gate && pst_iproute_node->multipath)
        {
            PM_E_RETURN(rt_fea_ipv4_get_nh_key(vrf_id, dest, gate, prefixlen, ifindex, distance, &nh_key));
            /*update ecmp group members*/
            rt_fea_ipv4_update_nh_by_group(vrf_id, &dest_all, &nh_key, 1, prefixlen, distance);
    
            /*update route to nh, update nexthop group last member to nexthop*/
            sal_memcpy(&pst_iproute_node->nh_key, &nh_key, sizeof(nexthop_key_t));
            PM_E_RETURN(tbl_iproute_node_set_iproute_node_field(pst_iproute_node, TBL_IPROUTE_NODE_FLD_NH_KEY));

            rt_fea_ipv4_del_from_iproute_node_add_fail(vrf_id, dest, prefixlen, gate, NULL, 0);
            rt_fea_ipv4_del_from_iproute_node_add_fail_count(vrf_id, dest, prefixlen, gate, NULL, 0);

            rt_fea_ipv4_del_from_iproute_node_add_fail(vrf_id, dest, prefixlen, NULL, pst_iproute_node->nh_group.nh, pst_iproute_node->nh_group.nh_num);
            rt_fea_ipv4_del_from_iproute_node_add_fail_count(vrf_id, dest, prefixlen, NULL, pst_iproute_node->nh_group.nh, pst_iproute_node->nh_group.nh_num);
            
            PM_E_RETURN(rt_fea_ipv4_del_nhg(pst_iproute_node, NULL, pst_iproute_node->nh_group.nh_num, 0, TRUE));
            if (ZEBRA_ROUTE_OSPF == route_type)
            {
                nexthop_ecmp_routed_t nexthop_ecmp_routed = {0};
                sal_memset(&nexthop_ecmp_routed, 0x00, sizeof(nexthop_ecmp_routed));

                nexthop_ecmp_routed.nh_num = pst_iproute_node->nh_group.nh_num;
                sal_memcpy(nexthop_ecmp_routed.nh, pst_iproute_node->nh_group.nh, pst_iproute_node->nh_group.nh_num*sizeof(nexthop_key_t));
                rt_fea_clear_nexthop_group_ifname(&nexthop_ecmp_routed);
                route_cmd_ipv4_del_nhg(&nexthop_ecmp_routed, TRUE);
            }
            
            pst_iproute_node->nh_group.rr_en = 0;
            pst_iproute_node->nh_group.nh_num = 0;
            sal_memset(pst_iproute_node->nh_group.nh, 0, sizeof(pst_iproute_node->nh_group.nh));
            //PM_E_RETURN(tbl_iproute_node_set_iproute_node_field(pst_iproute_node, TBL_IPROUTE_NODE_FLD_NH_GROUP));
    
            pst_iproute_node->multipath = 0;
            PM_E_RETURN(tbl_iproute_node_set_iproute_node_field(pst_iproute_node, TBL_IPROUTE_NODE_FLD_MULTIPATH));

            return PM_E_NONE;
        }
        /*route update from linklocal or black hole to normal*/
        else if (NULL != gate && (pst_iproute_node->nh_key.type == NH_TYPE_LOCAL || pst_iproute_node->nh_key.type == NH_TYPE_BLACKHOLE))
        {
            PM_E_RETURN(rt_fea_ipv4_get_nh_key(vrf_id, dest, gate, prefixlen, ifindex, distance, &nh_key));
            /*update ecmp group members*/
            rt_fea_ipv4_update_nh_by_group(vrf_id, &dest_all, &nh_key, 1, prefixlen, distance);

            /*update route to nh, update nexthop group last member to nexthop*/
            sal_memcpy(&pst_iproute_node->nh_key, &nh_key, sizeof(nexthop_key_t));
            PM_E_RETURN(tbl_iproute_node_set_iproute_node_field(pst_iproute_node, TBL_IPROUTE_NODE_FLD_NH_KEY));
            
            return PM_E_NONE;
        }/*route update from normal to normal*/
        else if (NULL != gate && 0 != sal_memcmp(&pst_iproute_node->nh_key.u.ipv4, gate, sizeof(addr_ipv4_t)))
        {
            PM_E_RETURN(rt_fea_ipv4_get_nh_key(vrf_id, dest, gate, prefixlen, ifindex, distance, &nh_key));
            /*update ecmp group members*/
            rt_fea_ipv4_update_nh_by_group(vrf_id, &dest_all, &nh_key, 1, prefixlen, distance);

            rt_fea_ipv4_del_from_iproute_node_add_fail(vrf_id, dest, prefixlen, gate, NULL, 0);
            rt_fea_ipv4_del_from_iproute_node_add_fail_count(vrf_id, dest, prefixlen, gate, NULL, 0);

            rt_fea_ipv4_del_from_iproute_node_add_fail(vrf_id, dest, prefixlen, &pst_iproute_node->nh_key.u.ipv4, NULL, 0);
            rt_fea_ipv4_del_from_iproute_node_add_fail_count(vrf_id, dest, prefixlen, &pst_iproute_node->nh_key.u.ipv4, NULL, 0);
    
            /*update route to nh, update nexthop group last member to nexthop*/
            sal_memcpy(&pst_iproute_node->nh_key, &nh_key, sizeof(nexthop_key_t));
            PM_E_RETURN(tbl_iproute_node_set_iproute_node_field(pst_iproute_node, TBL_IPROUTE_NODE_FLD_NH_KEY));
            
            return PM_E_NONE;
        }
        else if(NULL == gate) /*route update to linklocal or black hole*/
        {
            /*update old nexthop*/
            rt_fea_ipv4_update_nh_by_group(vrf_id, &dest_all, &nh_key, 0, prefixlen, distance);

            rt_fea_ipv4_del_from_iproute_node_add_fail(vrf_id, dest, prefixlen, &pst_iproute_node->nh_key.u.ipv4, NULL, 0);
            rt_fea_ipv4_del_from_iproute_node_add_fail_count(vrf_id, dest, prefixlen, &pst_iproute_node->nh_key.u.ipv4, NULL, 0);
            
            sal_memset(&pst_iproute_node->nh_key, 0, sizeof(nexthop_key_t));
            /*add by chenc for route update to black hole route for bug 45589*/
            if (nexthop_type == RTN_BLACKHOLE)
            {
                pst_iproute_node->nh_key.type = NH_TYPE_BLACKHOLE;
            }
            else
            {
                pst_iproute_node->nh_key.type = NH_TYPE_LOCAL;
                sal_memcpy(pst_iproute_node->nh_key.ifname, p_db_kernel_if->name, IFNAME_SIZE);
            }
            
            PM_E_RETURN(tbl_iproute_node_set_iproute_node_field(pst_iproute_node, TBL_IPROUTE_NODE_FLD_NH_KEY));

            /*del nexthop group if need*/
            if(pst_iproute_node->multipath)
            {
                rt_fea_ipv4_del_from_iproute_node_add_fail(vrf_id, dest, prefixlen, NULL, pst_iproute_node->nh_group.nh, pst_iproute_node->nh_group.nh_num);
                rt_fea_ipv4_del_from_iproute_node_add_fail_count(vrf_id, dest, prefixlen, NULL, pst_iproute_node->nh_group.nh, pst_iproute_node->nh_group.nh_num);
                
                PM_E_RETURN(rt_fea_ipv4_del_nhg(pst_iproute_node, NULL, pst_iproute_node->nh_group.nh_num, 0, TRUE));
                if (ZEBRA_ROUTE_OSPF == route_type)
                {
                    nexthop_ecmp_routed_t nexthop_ecmp_routed = {0};
                    sal_memset(&nexthop_ecmp_routed, 0x00, sizeof(nexthop_ecmp_routed));

                    nexthop_ecmp_routed.nh_num = pst_iproute_node->nh_group.nh_num;
                    sal_memcpy(nexthop_ecmp_routed.nh, pst_iproute_node->nh_group.nh, pst_iproute_node->nh_group.nh_num*sizeof(nexthop_key_t));
                    rt_fea_clear_nexthop_group_ifname(&nexthop_ecmp_routed);
                    route_cmd_ipv4_del_nhg(&nexthop_ecmp_routed, TRUE);
                }
                
                pst_iproute_node->nh_group.rr_en = 0;
                pst_iproute_node->nh_group.nh_num = 0;
                sal_memset(pst_iproute_node->nh_group.nh, 0, sizeof(pst_iproute_node->nh_group.nh));
                //PM_E_RETURN(tbl_iproute_node_set_iproute_node_field(pst_iproute_node, TBL_IPROUTE_NODE_FLD_NH_GROUP));
                pst_iproute_node->multipath = 0;
                PM_E_RETURN(tbl_iproute_node_set_iproute_node_field(pst_iproute_node, TBL_IPROUTE_NODE_FLD_MULTIPATH));
            }
            
            return PM_E_NONE;
        }
        else
        {
            return PM_E_EXIST;
        }
    }
    else
    {
        /* 3. add route */
        over_check = rt_fea_indirect_route_count_max_check() ;
        if (over_check)
        {
            rt_fea_ipv4_add_to_iproute_node_add_fail(vrf_id, dest, prefixlen, gate, NULL, 0);
            rt_fea_ipv4_add_to_iproute_node_add_fail_count(vrf_id, dest, prefixlen, gate, NULL, 0);
            return PM_E_NONE;
        }
    
        sal_memset(&iproute_node, 0, sizeof(iproute_node));
        if (NULL != gate)
        {
            pst_nexthop = rt_fea_nexthop_ipv4_add(p_db_kernel_if->name, gate);
            if (NULL == pst_nexthop)
            {
                rt_fea_ipv4_add_to_iproute_node_add_fail(vrf_id, dest, prefixlen, gate, NULL, 0);
                rt_fea_ipv4_add_to_iproute_node_add_fail_count(vrf_id, dest, prefixlen, gate, NULL, 0);
                return PM_E_FAIL;
            }
    
            sal_memcpy(&iproute_node.nh_key, &pst_nexthop->key, sizeof(iproute_node.nh_key));
            rc = arp_api_route_add(pst_nexthop);
        }
        /*add by chenc for new add a black hole route for bug 45589*/
        else if (RTN_BLACKHOLE == nexthop_type)
        {
            iproute_node.nh_key.type = NH_TYPE_BLACKHOLE;
        }
        else
        {
            iproute_node.nh_key.type = NH_TYPE_LOCAL;
            sal_memcpy(iproute_node.nh_key.ifname, p_db_kernel_if->name, IFNAME_SIZE);
        }

        /*add by chenc for bug 50767, 2019-01-10*/
        if (RTN_BLACKHOLE != nexthop_type)
        {
            /*add by chenc for bug 49594, 2019-01-03*/
            tbl_route_if_t *p_db_route_if = NULL;
            p_db_route_if = tbl_route_if_get_route_if_by_name(p_db_kernel_if->name);

            if (RTN_UNICAST == nexthop_type && NULL == gate && NULL == p_db_route_if)
            {
                return PM_E_NONE;
            }
            /*bug 49594 end*/
        }
        /*bug 50767 end*/
    
        iproute_node.key.vrf_id = vrf_id;
        iproute_node.key.p.family = AF_INET;
        iproute_node.key.p.prefixlen = prefixlen;
        sal_memcpy (&iproute_node.key.p.u.prefix4, dest, sizeof(addr_ipv4_t));
        iproute_node.distance = distance;
        iproute_node.route_type = route_type;
        rc = tbl_iproute_node_add_iproute_node(&iproute_node);
    
        if (!rc)
        {
            rt_fea_route_count_increase();
            rt_route_counter_inc(DEFAULT_VRF_ID);
        }
    
        return rc;

    }
}

int32
rt_fea_ipv4_del_route_nl(vrf_id_t vrf_id, addr_ipv4_t *dest, addr_ipv4_t *gate, uint8 prefixlen,
                  uint32 ifindex, uint32 distance, uint32 nexthop_type, uint32 route_type)
{
    tbl_iproute_node_t  *pst_iproute_node   = NULL;
    tbl_nexthop_t       *pst_nexthop = NULL;
    tbl_iproute_node_t  iproute_node;
    tbl_iproute_node_t  iproute_node_cp;
    int32               rc = PM_E_NONE;
    tbl_nexthop_group_t       *pst_nexthop_group = NULL;
    
    char        buf1[CMD_BUF_256] = {0};
    char        buf2[CMD_BUF_256] = {0};

   // log_sys(M_MOD_ROUTE, E_ERROR, "rt_fea_ipv4_del_route_nl \n");
    DEBUG_ROUTE_IPV4();
    if (NULL != dest && NULL != gate)
    {
        DEBUG_ROUTE_IPV4("dest(%s), gate(%s), prefixlen(%u),  ifindex(%u), distance(%u), nexthop_type(%s), route_type(%d)\n", 
            cdb_addr_ipv4_val2str(buf1, CMD_BUF_256, dest), cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, gate), 
            prefixlen, ifindex, distance, rt_fea_api_nexthop_type_desc(nexthop_type), route_type);
    }
    else if (NULL != dest &&  NULL == gate)
    {
        DEBUG_ROUTE_IPV4("dest(%s), prefixlen(%u), ifindex(%u), distance(%u), nexthop_type(%s), route_type(%d)\n", 
            cdb_addr_ipv4_val2str(buf1, CMD_BUF_256, dest), 
            prefixlen, ifindex, distance, rt_fea_api_nexthop_type_desc(nexthop_type), route_type);
    }
    if (NULL == dest)
    {
        return PM_E_FAIL;
    }

#ifdef USE_FPM_IPUC
    /*fpm ipuc: delete route nexthop type/ifindex and gate are not specified 
    referance to: 
    static int netlink_route_info_fill()
        if (cmd == RTM_DELROUTE && !rib)
            return 1;
    */
    if (!(RTN_UNSPEC == nexthop_type && 0 == ifindex && NULL == gate))
#endif
    {
        /* filter manage if configurations */
        if (GLB_IF_TYPE_NULL == tbl_interface_get_type_by_ifindex(ifindex))
        {
            if (RTN_BLACKHOLE != nexthop_type)
            {
                return PM_E_FAIL;
            }
        }
    }

    //rt_fea_ipv4_del_from_iproute_node_add_fail(vrf_id, dest, prefixlen, gate, NULL, 0);

    /* 1. del route */
    sal_memset(&iproute_node, 0, sizeof(iproute_node));
    sal_memset(&iproute_node_cp, 0, sizeof(iproute_node_cp));

    iproute_node.key.vrf_id = vrf_id;
    iproute_node.key.p.family = AF_INET;
    iproute_node.key.p.prefixlen = prefixlen;
    sal_memcpy (&iproute_node.key.p.u.prefix4, dest, sizeof(addr_ipv4_t));
    pst_iproute_node = tbl_iproute_node_get_iproute_node(&iproute_node.key);

    /*add failed before, so here is NULL*/
    if (NULL == pst_iproute_node)
    {
        rt_fea_ipv4_del_from_iproute_node_add_fail(vrf_id, dest, prefixlen, gate, NULL, 0);
        rt_fea_ipv4_del_from_iproute_node_add_fail_count(vrf_id, dest, prefixlen, gate, NULL, 0);
    }
    
    /* modified by liwh for bug 45983, 2017-12-28 */
    if (pst_iproute_node)
    {
        if ((NULL != gate) && (0 != sal_memcmp(&pst_iproute_node->nh_key.u.ipv4, gate, sizeof(addr_ipv4_t))))
        {
            /* change log to debug, modified by liwh for bug 46190, 2018-01-19 */
            DEBUG_ROUTE_IPV4("gate %x, pst_iproute_node->nh_key.u.ipv4.s_addr %x\n", 
                gate->s_addr, pst_iproute_node->nh_key.u.ipv4.s_addr);

            return PM_E_NONE;
        }
    }
    /* liwh end */

    /* modified by liwh for bug 45943, 2017-12-21 */
    if (pst_iproute_node 
        && (RTN_LOCAL != nexthop_type)
        && (NH_TYPE_LOCAL == pst_iproute_node->nh_key.type))
    {
        /* change log to debug, modified by liwh for bug 46190, 2018-01-19 */
        DEBUG_ROUTE_IPV4("dest %x, ifname %s is local ip address\n", *dest, 
            pst_iproute_node->nh_key.ifname);
        return PM_E_NONE;
    }
    /* liwh end */

    if (NULL != pst_iproute_node)
    {
        sal_memcpy(&iproute_node_cp, pst_iproute_node, sizeof(iproute_node_cp));

        //log_sys(M_MOD_ROUTE, E_ERROR, "dest %x, ifname %s \n", *dest, pst_iproute_node->nh_key.ifname);

#ifdef USE_FPM_IPUC
        pst_nexthop = rt_fea_nexthop_ipv4_get_for_use_fpm_ipuc(&pst_iproute_node->nh_key);
        if (NULL == pst_nexthop || pst_iproute_node->multipath)
        {
            pst_nexthop_group = rt_fea_nexthop_group_ipv4_get_for_use_fpm_ipuc(pst_iproute_node);
        }
#endif
        rc = tbl_iproute_node_del_iproute_node(&iproute_node.key);
        rt_fea_route_count_decrease();
        rt_route_counter_dec(DEFAULT_VRF_ID);
    }
    else
    {
        return PM_E_NONE;
    }

    /*black hole not have nexthop, no need to delete nexthop*/
    if (NH_TYPE_BLACKHOLE == iproute_node_cp.nh_key.type)
    {
        return PM_E_NONE;
    }

    /* shouldn't deleted nexthop when iproute_node is local by liwh for bug 45444, 2017-10-26 */
    if (NH_TYPE_LOCAL == iproute_node_cp.nh_key.type)
    {         
        return PM_E_NONE;
    }
    /* liwh end */

#ifdef USE_FPM_IPUC
    if (pst_nexthop_group)
    {
        int32 i = 0;
        for (i = 0; i < pst_nexthop_group->key.nh_num; i++)
        {
            rc = arp_api_route_del_nh_arp_by_nexthop_key(&pst_nexthop_group->key.nh[i]);
        }
        rt_fea_ipv4_del_nh_by_group(NULL, vrf_id, NULL, &pst_nexthop_group->key.nh[0], pst_nexthop_group->key.nh_num, 0, 0);

        if (ZEBRA_ROUTE_OSPF == route_type || (0 == route_type && ZEBRA_ROUTE_OSPF == iproute_node_cp.route_type))
        {
            nexthop_ecmp_routed_t nexthop_ecmp_routed = {0};
            sal_memset(&nexthop_ecmp_routed, 0x00, sizeof(nexthop_ecmp_routed));

            nexthop_ecmp_routed.nh_num = pst_nexthop_group->key.nh_num;
            sal_memcpy(nexthop_ecmp_routed.nh, pst_nexthop_group->key.nh, pst_nexthop_group->key.nh_num*sizeof(nexthop_key_t));
            rt_fea_clear_nexthop_group_ifname(&nexthop_ecmp_routed);
            route_cmd_ipv4_del_nhg(&nexthop_ecmp_routed, TRUE);
        }
        rt_fea_ipv4_del_nhg(NULL, &pst_nexthop_group->key.nh[0], pst_nexthop_group->key.nh_num, pst_nexthop_group->key.rr_en, TRUE);
    }
    else
    {
        if (NULL != pst_nexthop)
        {
            rc = arp_api_route_del(pst_nexthop);
            rt_fea_nexthop_ipv4_del_for_use_fpm_ipuc(pst_nexthop);
        }
    }
#else
    /* 2. get ifname by kernel ifindex */
    tbl_kernel_if_t     *p_db_kernel_if     = NULL;
    tbl_kernel_if_t     kernel_if;
    
    kernel_if.key.ifindex = ifindex;
    p_db_kernel_if = tbl_kernel_if_get_kernel_if(&kernel_if.key);
    if (NULL == p_db_kernel_if)
    {
        return PM_E_FAIL;
    }

    /* 3. try to remove nexthop */
    if (NULL != gate)
    {
        pst_nexthop = rt_fea_nexthop_ipv4_get(p_db_kernel_if->name, gate);
        if (NULL != pst_nexthop)
        {
            rc = arp_api_route_del(pst_nexthop);
            rt_fea_nexthop_ipv4_del(p_db_kernel_if->name, gate);
        }
    }
#endif

    return rc;
}
#if 0
int32
rt_fea_ipv4_add_mroute_nl(vrf_id_t vrf_id, addr_ipv4_t *source, addr_ipv4_t *group,
                                       uint32 iif, uint32 *oif, uint32 oif_count)
{
    int32               rc = PM_E_NONE;
    tbl_kernel_if_t    *p_db_kernel_if   = NULL;
    tbl_kernel_if_key_t kernel_if_key;
    tbl_interface_t    *p_db_if = NULL;
    tbl_mrt_t          *pst_mrt = NULL;
    tbl_mrt_t           mrt;
    uint32              rpf_ifindex;
    uint32              oif_ifindex;
    uint32              index = 0;
    port_vlan_bmp_t     oif_index_bmp;

    sal_memset(&mrt, 0, sizeof(mrt));
	log_sys(M_MOD_IGMP, E_ALERT, "zhw test:RT FEA ad mroute, vrfid %d, source %x, group %x\n", vrf_id, *source, *group);
    /* get mroute  */
    mrt.key.vrf_id = DEFAULT_VRF_ID;
    sal_memcpy (&mrt.key.source, source, sizeof(addr_ipv4_t));
    sal_memcpy (&mrt.key.group, group, sizeof(addr_ipv4_t));
    pst_mrt = tbl_mrt_get_mrt(&mrt.key);

    /* get rpf_ifindex */
    kernel_if_key.ifindex = iif;
    p_db_kernel_if = tbl_kernel_if_get_kernel_if(&kernel_if_key);
    if (NULL == p_db_kernel_if)
    {
        return PM_E_FAIL;
    }
    p_db_if = tbl_interface_get_interface_by_name(p_db_kernel_if->name);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }
    else
    {
        rpf_ifindex = tbl_interface_get_ifindex_by_name(p_db_kernel_if->name);
    }

    /* get oif_index_bmp */
    GLB_BMP_INIT(oif_index_bmp);
    for (index = 0; index < oif_count; ++index)
    {
        kernel_if_key.ifindex = oif[index];
        p_db_kernel_if = tbl_kernel_if_get_kernel_if(&kernel_if_key);
        if (NULL == p_db_kernel_if)
        {
            return PM_E_FAIL;
        }
        p_db_if = tbl_interface_get_interface_by_name(p_db_kernel_if->name);
        if (NULL == p_db_if)
        {
            return PM_E_FAIL;
        }
        else
        {
            oif_ifindex = tbl_interface_get_ifindex_by_name(p_db_kernel_if->name);
            GLB_BMP_SET(oif_index_bmp, oif_ifindex);
        }
    }

    /* set to CDB */
    if (NULL != pst_mrt)
    {
    	log_sys(M_MOD_IGMP, E_ALERT, "zhw test: tbl_mrt_set_mrt_field \n");
        if (rpf_ifindex != pst_mrt->rpf_ifindex)
        {
            pst_mrt->rpf_ifindex = rpf_ifindex;
            PM_E_RETURN(tbl_mrt_set_mrt_field(pst_mrt, TBL_MRT_FLD_RPF_IFINDEX));
        }

        if (sal_memcmp(oif_index_bmp, pst_mrt->oif_index_bmp, sizeof(port_vlan_bmp_t)))
        {
            sal_memcpy(pst_mrt->oif_index_bmp, oif_index_bmp, sizeof(port_vlan_bmp_t));
            PM_E_RETURN(tbl_mrt_set_mrt_field(pst_mrt, TBL_MRT_FLD_OIF_INDEX_BMP));
        }
    }
    else
    {
        mrt.rpf_ifindex = rpf_ifindex;
        GLB_BMP_INIT(mrt.oif_index_bmp);
        sal_memcpy(mrt.oif_index_bmp, oif_index_bmp, sizeof(port_vlan_bmp_t));
        if (tbl_mrt_get_master()->mrt_hash->count >= GLB_L3MC_MAX_ROUTE_ENTRY_NUM)
        {
            DEBUG_ROUTE_IPV4("The L3 Multicast route entry num is more than maximum !");
            return PM_E_NONE;
        }
		log_sys(M_MOD_IGMP, E_ALERT, "zhw test: tbl_mrt_add_mrt\n");
        PM_E_RETURN(tbl_mrt_add_mrt(&mrt));
    }

    return PM_E_NONE;
}

int32
rt_fea_ipv4_del_mroute_nl(vrf_id_t vrf_id, addr_ipv4_t *source, addr_ipv4_t *group,
                                      uint32 iif, uint32 *oif, uint32 oif_count)
{
    int32               rc = PM_E_NONE;
    tbl_mrt_t          *pst_mrt = NULL;
    tbl_mrt_t           mrt;

    sal_memset(&mrt, 0, sizeof(mrt));
	log_sys(M_MOD_IGMP, E_ALERT, "zhw test: RT FEA del mroute, vrfid %d, source %x, group %x\n", vrf_id, *source, *group);
    /* get mroute  */
    mrt.key.vrf_id = vrf_id;
    sal_memcpy (&mrt.key.source, source, sizeof(addr_ipv4_t));
    sal_memcpy (&mrt.key.group, group, sizeof(addr_ipv4_t));
    pst_mrt = tbl_mrt_get_mrt(&mrt.key);
    if (NULL == pst_mrt)
    {
        return PM_E_NONE;
    }

    PM_E_RETURN(tbl_mrt_del_mrt(&mrt.key));
    return PM_E_NONE;
}
#endif

int32 
rt_fea_ipv4_add_mroute_nl(uint32 vrf_id, addr_ipv4_t *source, addr_ipv4_t *group, uint32 iif, 
                      uint32 *oif, uint32 oif_count)
{
	tbl_kernel_if_t *p_k_if = NULL;
	tbl_kernel_if_key_t k_if_key;
	tbl_interface_t * p_if = NULL;
	tbl_mrt_t * pst_mrt = NULL;
	tbl_mrt_t mrt;
	uint32 rpf_ifindex;
	uint32 oif_ifindex;
	uint32 i = 0;
	port_vlan_bmp_t oif_index_bmp;
	int32 no_oif = TRUE;

	log_sys(M_MOD_IGMP, E_ALERT, "RT FEA ad mroute, vrfid %d, source %x, group %x\n", vrf_id, *source, *group);

	/*get para*/
	k_if_key.ifindex = iif;
	p_k_if = tbl_kernel_if_get_kernel_if(&k_if_key);
	if(NULL == p_k_if)
	{
	    log_sys(M_MOD_IGMP, E_WARNING, "RT FEA add mroute, rpf kernel if not exist!\n");
		return PM_E_NOT_EXIST;
	}
	p_if = tbl_interface_get_interface_by_name(p_k_if->name);
	if(NULL == p_if)
	{
	    log_sys(M_MOD_IGMP, E_WARNING, "RT FEA add mroute, rpf tbl if not exist!\n");
		return PM_E_NOT_EXIST;
	}
	log_sys(M_MOD_IGMP, E_ALERT, "RT FEA add mroute, rpf %s\n", p_k_if->name);
	rpf_ifindex = p_if->ifindex;
	
	GLB_BMP_INIT(oif_index_bmp);
	for(i=0; i<oif_count; i++)
	{
    	k_if_key.ifindex = oif[i];
    	p_k_if = tbl_kernel_if_get_kernel_if(&k_if_key);
    	if(NULL == p_k_if)
    	{
    	    log_sys(M_MOD_IGMP, E_WARNING, "RT FEA add mroute, oif loop %d, kernel if not exist!\n", i);
    		continue;
    	}
    	p_if = tbl_interface_get_interface_by_name(p_k_if->name);
    	if(NULL == p_if)
    	{
    	    log_sys(M_MOD_IGMP, E_WARNING, "RT FEA add mroute, oif loop %d, tbl if not exist!\n", i);
    		continue;
    	} 
		oif_ifindex= p_if->ifindex;
		GLB_BMP_SET(oif_index_bmp, oif_ifindex);
		no_oif = FALSE;

		log_sys(M_MOD_IGMP, E_ALERT, "RT FEA add mroute, oif %s\n", p_k_if->name);
	}
	if(no_oif)
	{
	    log_sys(M_MOD_IGMP, E_ALERT, "RT FEA add mroute, therer are no valid oif !\n");
		return PM_E_INVALID_IFNAME;
	}
	
    /*check mrt in db*/
	sal_memset(&mrt, 0, sizeof(mrt));
	mrt.key.vrf_id = vrf_id;
	sal_memcpy(&mrt.key.source, source, sizeof(addr_ipv4_t));
	sal_memcpy(&mrt.key.group, group, sizeof(addr_ipv4_t));
	pst_mrt = tbl_mrt_get_mrt(&mrt.key);
	if(NULL == pst_mrt)
	{
	    /*create db and set to cdb*/
		if(tbl_mrt_get_master()->mrt_hash->count >= GLB_IPV4_MROUTE_MAX_ENTRY_NUM)
		{
    	    log_sys(M_MOD_IGMP, E_WARNING, "RT FEA ad mroute entry exceed 512!\n");
    		return PM_E_NONE;
		}
		mrt.rpf_ifindex = rpf_ifindex;
		GLB_BMP_INIT(mrt.oif_index_bmp);
		sal_memcpy(mrt.oif_index_bmp, oif_index_bmp, sizeof(port_vlan_bmp_t));
		tbl_mrt_add_mrt(&mrt);
	}
	else
	{   /*update rpf ort oifbmp*/
	    if(rpf_ifindex != pst_mrt->rpf_ifindex)
	    {
	        pst_mrt->rpf_ifindex = rpf_ifindex;
			tbl_mrt_set_mrt_field(pst_mrt, TBL_MRT_FLD_RPF_IFINDEX);
	   	}

		if(sal_memcmp(oif_index_bmp, pst_mrt->oif_index_bmp, sizeof(port_vlan_bmp_t)))
		{
            sal_memcpy(pst_mrt->oif_index_bmp, oif_index_bmp, sizeof(port_vlan_bmp_t));
			tbl_mrt_set_mrt_field(pst_mrt, TBL_MRT_FLD_OIF_INDEX_BMP);
		}
	}

    return PM_E_NONE;
}



int32 
rt_fea_ipv4_del_mroute_nl(uint32 vrf_id, addr_ipv4_t *source, addr_ipv4_t *group, uint32 iif, 
                      uint32 *oif, uint32 oif_count)
{
    int32 rc = PM_E_NONE;
	tbl_mrt_t *pst_mrt = NULL;
	tbl_mrt_t mrt;

	log_sys(M_MOD_IGMP, E_ALERT, "RT FEA del mroute, vrfid %d, source %x, group %x\n", vrf_id, *source, *group);

	sal_memset(&mrt, 0, sizeof(mrt));
    mrt.key.vrf_id = vrf_id;
	sal_memcpy(&mrt.key.source, source, sizeof(addr_ipv4_t));
	sal_memcpy(&mrt.key.group, group, sizeof(addr_ipv4_t));
	pst_mrt = tbl_mrt_get_mrt(&mrt.key);
	if(NULL == pst_mrt)
	{
	    log_sys(M_MOD_IGMP, E_WARNING, "RT FEA del mroute, mrt not exist!\n");
		return PM_E_NONE;
	}

	PM_E_RETURN(tbl_mrt_del_mrt(&mrt.key));

    return rc;
}


int32
rt_fea_ipv4_get_ecmp_hash_type(vrf_id_t vrf_id, addr_ipv4_t *dest, uint8 prefixlen, uint32 nh_num)
{
    uint32  ecmp_hash = 0;
    uint32  rrprefix_index  = 0;
    tbl_ecmp_global_t   *p_db_ecmp_glb = NULL;
    tbl_rr_prefix_t     *p_db_rrprefix = NULL;
    tbl_rr_prefix_master_t  *p_master = NULL;

    addr_ipv4_t prefix_cp;
    uint32      mask = 0;

    if (nh_num <= 1)
    {
        return ecmp_hash;
    }

    p_db_ecmp_glb = tbl_ecmp_global_get_ecmp_global();
    if (!p_db_ecmp_glb)
    {
        return ecmp_hash;
    }

    ecmp_hash = p_db_ecmp_glb->load_balance_mode;

    p_master = tbl_rr_prefix_get_master();
    if (!p_master)
    {
        return ecmp_hash;
    }
    
    for (rrprefix_index = 0; rrprefix_index < GLB_MAX_ECMP_ROUND_ROBIN; rrprefix_index++)
    {
        p_db_rrprefix = p_master->rr_prefix_array[rrprefix_index];
        if (!p_db_rrprefix)
        {
            continue;
        }

        sal_memset(&prefix_cp, 0, sizeof(addr_ipv4_t));
        IPV4_LEN_TO_MASK(mask, prefixlen);
        sal_memcpy(&prefix_cp, &(p_db_rrprefix->rr_prefix.u.prefix4), sizeof(addr_ipv4_t));
        prefix_cp.s_addr = sal_htonl(prefix_cp.s_addr);
        prefix_cp.s_addr = prefix_cp.s_addr & mask;
        prefix_cp.s_addr = sal_htonl(prefix_cp.s_addr);

        if (prefixlen == p_db_rrprefix->rr_prefix.prefixlen 
            && !sal_memcmp(dest, &prefix_cp, sizeof(addr_ipv4_t)))
        {
            ecmp_hash = GLB_ECMP_ROUND_ROBIN_LOAD_BALANCE;
            break;
        }
    }

    return ecmp_hash;
}

int32
rt_fea_ipv6_add_new_multi_nh_route(vrf_id_t vrf_id, addr_ipv6_t *dest, nexthop_key_t *nhg,
                    uint32 nh_num, uint8 prefixlen, uint32 distance)
{
    tbl_iproute_node_t  *pst_iproute_node = NULL;
    tbl_iproute_node_t  iproute_node;
    uint32              over_check  = 0;
    uint32              ecmp_hash = 0;
    int32               rc = PM_E_NONE;

    /* 1. get route  */
    sal_memset(&iproute_node, 0, sizeof(iproute_node));

    iproute_node.key.vrf_id = vrf_id;
    iproute_node.key.p.family = AF_INET6;
    iproute_node.key.p.prefixlen = prefixlen;
    sal_memcpy (&iproute_node.key.p.u.prefix6, dest, sizeof(addr_ipv6_t));
    pst_iproute_node = tbl_iproute_node_get_iproute_node(&iproute_node.key);

    /*2.1 add new route*/
    if(NULL == pst_iproute_node)
    {
        over_check = rt_fea_indirect_route_v6_count_max_check();
        if (over_check)
        {
            return PM_E_NONE;
        }

        sal_memset(&iproute_node, 0, sizeof(iproute_node));
        iproute_node.key.vrf_id = vrf_id;
        iproute_node.key.p.family = AF_INET6;
        iproute_node.key.p.prefixlen = prefixlen;
        sal_memcpy (&iproute_node.key.p.u.prefix6, dest, sizeof(addr_ipv6_t));
        iproute_node.distance = distance;
        iproute_node.multipath = ECMP_ENABLE;
    

        iproute_node.nh_group.nh_num = nh_num;
        sal_memcpy(iproute_node.nh_group.nh, nhg, sizeof(nexthop_key_t)*nh_num);
        #if 0
        log_sys(M_MOD_ROUTE, E_ERROR, "%s: dest %x\n"
            "multipath number is %d"
            "    nexthop group %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x \n", 
            __func__, 
            iproute_node.key.p.u.prefix4.s_addr, multipath,
            iproute_node.nh_group.nh[0].u.ipv4.s_addr,
            iproute_node.nh_group.nh[1].u.ipv4.s_addr,
            iproute_node.nh_group.nh[2].u.ipv4.s_addr,
            iproute_node.nh_group.nh[3].u.ipv4.s_addr,
            iproute_node.nh_group.nh[4].u.ipv4.s_addr,
            iproute_node.nh_group.nh[5].u.ipv4.s_addr,
            iproute_node.nh_group.nh[6].u.ipv4.s_addr,
            iproute_node.nh_group.nh[7].u.ipv4.s_addr,
            iproute_node.nh_group.nh[8].u.ipv4.s_addr,
            iproute_node.nh_group.nh[9].u.ipv4.s_addr,
            iproute_node.nh_group.nh[10].u.ipv4.s_addr,
            iproute_node.nh_group.nh[11].u.ipv4.s_addr,
            iproute_node.nh_group.nh[12].u.ipv4.s_addr,
            iproute_node.nh_group.nh[13].u.ipv4.s_addr,
            iproute_node.nh_group.nh[14].u.ipv4.s_addr,
            iproute_node.nh_group.nh[15].u.ipv4.s_addr
            );
        #endif
        //ecmp_hash = rt_fea_ipv4_get_ecmp_hash_type(vrf_id, dest, prefixlen, nh_num);
        PM_E_RETURN(rt_fea_ipv6_add_nhg(NULL, nhg, nh_num, ecmp_hash, TRUE, FALSE));
        rc += tbl_iproute_node_add_iproute_node(&iproute_node);
        if (!rc && !over_check)
        {
            rt_fea_route_v6_count_increase();
            rt_route_v6_counter_inc(DEFAULT_VRF_ID);
            //rt_ecmp_v6_counter_inc(DEFAULT_VRF_ID);
        }
        else
        {
            DEBUG_ROUTE_IPV6("add ip route node failed");
        }
    }

    return rc;

}

int32
rt_fea_ipv4_add_new_multi_nh_route(vrf_id_t vrf_id, addr_ipv4_t *dest, nexthop_key_t *nhg,
                    uint32 nh_num, uint8 prefixlen, uint32 distance, uint32 route_type)
{
    tbl_iproute_node_t  *pst_iproute_node = NULL;
    tbl_iproute_node_t  iproute_node;
    uint32              over_check  = 0;
    uint32              ecmp_hash = 0;
    int32               rc = PM_E_NONE;

    /* 1. get route  */
    sal_memset(&iproute_node, 0, sizeof(iproute_node));

    iproute_node.key.vrf_id = vrf_id;
    iproute_node.key.p.family = AF_INET;
    iproute_node.key.p.prefixlen = prefixlen;
    sal_memcpy (&iproute_node.key.p.u.prefix4, dest, sizeof(addr_ipv4_t));
    pst_iproute_node = tbl_iproute_node_get_iproute_node(&iproute_node.key);

    /*2.1 add new route*/
    if(NULL == pst_iproute_node)
    {
        over_check = rt_fea_indirect_route_count_max_check();
        if (over_check)
        {
            rt_fea_ipv4_add_to_iproute_node_add_fail(vrf_id, dest, prefixlen, NULL, nhg, nh_num);
            rt_fea_ipv4_add_to_iproute_node_add_fail_count(vrf_id, dest, prefixlen, NULL, nhg, nh_num);
            return PM_E_NONE;
        }

        sal_memset(&iproute_node, 0, sizeof(iproute_node));
        iproute_node.key.vrf_id = vrf_id;
        iproute_node.key.p.family = AF_INET;
        iproute_node.key.p.prefixlen = prefixlen;
        sal_memcpy (&iproute_node.key.p.u.prefix4, dest, sizeof(addr_ipv4_t));
        iproute_node.distance = distance;
        iproute_node.multipath = ECMP_ENABLE;

        ecmp_hash = rt_fea_ipv4_get_ecmp_hash_type(vrf_id, dest, prefixlen, nh_num);
        iproute_node.nh_group.rr_en = rt_fea_ipv4_ecmp_hash_type_is_rr(ecmp_hash);
        iproute_node.nh_group.nh_num = nh_num;
        sal_memcpy(iproute_node.nh_group.nh, nhg, sizeof(nexthop_key_t)*nh_num);
        #if 0
        log_sys(M_MOD_ROUTE, E_ERROR, "%s: dest %x\n"
            "multipath number is %d"
            "    nexthop group %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x \n", 
            __func__, 
            iproute_node.key.p.u.prefix4.s_addr, multipath,
            iproute_node.nh_group.nh[0].u.ipv4.s_addr,
            iproute_node.nh_group.nh[1].u.ipv4.s_addr,
            iproute_node.nh_group.nh[2].u.ipv4.s_addr,
            iproute_node.nh_group.nh[3].u.ipv4.s_addr,
            iproute_node.nh_group.nh[4].u.ipv4.s_addr,
            iproute_node.nh_group.nh[5].u.ipv4.s_addr,
            iproute_node.nh_group.nh[6].u.ipv4.s_addr,
            iproute_node.nh_group.nh[7].u.ipv4.s_addr,
            iproute_node.nh_group.nh[8].u.ipv4.s_addr,
            iproute_node.nh_group.nh[9].u.ipv4.s_addr,
            iproute_node.nh_group.nh[10].u.ipv4.s_addr,
            iproute_node.nh_group.nh[11].u.ipv4.s_addr,
            iproute_node.nh_group.nh[12].u.ipv4.s_addr,
            iproute_node.nh_group.nh[13].u.ipv4.s_addr,
            iproute_node.nh_group.nh[14].u.ipv4.s_addr,
            iproute_node.nh_group.nh[15].u.ipv4.s_addr
            );
        #endif
        PM_E_RETURN(rt_fea_ipv4_add_nhg(NULL, nhg, nh_num, ecmp_hash, TRUE, TRUE));
        if (ZEBRA_ROUTE_OSPF == route_type)
        {
            nexthop_ecmp_routed_t nexthop_ecmp_routed = {0};
            sal_memset(&nexthop_ecmp_routed, 0x00, sizeof(nexthop_ecmp_routed));

            nexthop_ecmp_routed.nh_num = nh_num;
            sal_memcpy(nexthop_ecmp_routed.nh, nhg, nh_num*sizeof(nexthop_key_t));
            rt_fea_clear_nexthop_group_ifname(&nexthop_ecmp_routed);
            route_cmd_ipv4_add_nhg(&nexthop_ecmp_routed, TRUE, TRUE);
        }
        
        iproute_node.route_type = route_type;
        rc += tbl_iproute_node_add_iproute_node(&iproute_node);
        if (!rc && !over_check)
        {
            rt_fea_route_count_increase();
            rt_route_counter_inc(DEFAULT_VRF_ID);
            //rt_ecmp_counter_inc(DEFAULT_VRF_ID);
        }
        
    }

    return rc;

}

int32
rt_fea_ipv6_add_route_multi_nl_check(vrf_id_t vrf_id, addr_ipv6_t *dest, nexthop_key_t *nhg,
                    uint32 multipath, uint8 prefixlen, uint32 distance)
{
    char        buf1[CMD_BUF_256] = {0};

    tbl_iproute_node_t  *pst_iproute_node = NULL;
    tbl_iproute_node_t  iproute_node;
    uint32              over_check  = 0;
    
    if (NULL == dest)
    {
        DEBUG_ROUTE_IPV6("dest is NULL, return");
        return PM_E_FAIL;
    }

    DEBUG_ROUTE_IPV6("dest(%s), multipath(%u), prefixlen(%u), distance(%u)\n", 
            cdb_addr_ipv6_val2str(buf1, CMD_BUF_256, dest), multipath, prefixlen, distance);

    /* 1. get route  */
    sal_memset(&iproute_node, 0, sizeof(iproute_node));

    iproute_node.key.vrf_id = vrf_id;
    iproute_node.key.p.family = AF_INET6;
    iproute_node.key.p.prefixlen = prefixlen;
    sal_memcpy(&iproute_node.key.p.u.prefix6, dest, sizeof(addr_ipv6_t));
    pst_iproute_node = tbl_iproute_node_get_iproute_node(&iproute_node.key);
    if (NULL != pst_iproute_node)
    {
        //return PM_E_EXIST;
        return PM_E_FAIL;
    }

    /* 3. add route */
    over_check = rt_fea_indirect_route_v6_count_max_check();
    if (over_check)
    {
        return PM_E_FAIL;
    }

    return PM_E_NONE;
}

int32
rt_fea_ipv6_del_route_multi_nl_check(vrf_id_t vrf_id, addr_ipv6_t *dest, nexthop_key_t *nhg,
                    uint32 multipath, uint8 prefixlen, uint32 distance)
{
    char        buf1[CMD_BUF_256] = {0};

    tbl_iproute_node_t  *pst_iproute_node = NULL;
    tbl_iproute_node_t  iproute_node;

    if (NULL == dest)
    {
        DEBUG_ROUTE_IPV6("dest is NULL, return");
        return PM_E_FAIL;
    }

    DEBUG_ROUTE_IPV6("dest(%s), multipath(%u), prefixlen(%u), distance(%u)\n", 
        cdb_addr_ipv6_val2str(buf1, CMD_BUF_256, dest), multipath, prefixlen, distance);

    /* 1. del route */
    sal_memset(&iproute_node, 0, sizeof(iproute_node));
    
    iproute_node.key.vrf_id = vrf_id;
    iproute_node.key.p.family = AF_INET6;
    iproute_node.key.p.prefixlen = prefixlen;
    sal_memcpy (&iproute_node.key.p.u.prefix6, dest, sizeof(addr_ipv6_t));
    pst_iproute_node = tbl_iproute_node_get_iproute_node(&iproute_node.key);

    if (NULL != pst_iproute_node)
    {
        if (ECMP_ENABLE != pst_iproute_node->multipath)
        {
            return PM_E_FAIL;
        }
    }
    else
    {
        return PM_E_FAIL;
    }
        
    return PM_E_NONE;
}

int32
rt_fea_ipv4_add_route_multi_nl_check(vrf_id_t vrf_id, addr_ipv4_t *dest, nexthop_key_t *nhg,
                    uint32 multipath, uint8 prefixlen, uint32 distance, uint32 route_type)
{
    tbl_iproute_node_t  *pst_iproute_node = NULL;
    tbl_iproute_node_t  iproute_node;
    uint32              over_check  = 0;
    DEBUG_ROUTE_IPV4();
    if (NULL == dest)
    {
        return PM_E_FAIL;
    }

    /* 1. get route  */
    sal_memset(&iproute_node, 0, sizeof(iproute_node));

    iproute_node.key.vrf_id = vrf_id;
    iproute_node.key.p.family = AF_INET;
    iproute_node.key.p.prefixlen = prefixlen;
    sal_memcpy(&iproute_node.key.p.u.prefix4, dest, sizeof(addr_ipv4_t));
    pst_iproute_node = tbl_iproute_node_get_iproute_node(&iproute_node.key);
    if (NULL != pst_iproute_node)
    {
        //return PM_E_EXIST;
        return PM_E_FAIL;
    }

    /* 3. add route */
    over_check = rt_fea_indirect_route_count_max_check();
    if (over_check)
    {
        return PM_E_FAIL;
    }

    return PM_E_NONE;

}

int32
rt_fea_ipv4_del_route_multi_nl_check(vrf_id_t vrf_id, addr_ipv4_t *dest, nexthop_key_t *nhg,
                    uint32 multipath, uint8 prefixlen, uint32 distance, uint32 route_type)
{
    tbl_iproute_node_t  *pst_iproute_node = NULL;
    tbl_iproute_node_t  iproute_node;
    DEBUG_ROUTE_IPV4();
    if (NULL == dest)
    {
        return PM_E_FAIL;
    }

    /* 1. del route */
    sal_memset(&iproute_node, 0, sizeof(iproute_node));
    
    iproute_node.key.vrf_id = vrf_id;
    iproute_node.key.p.family = AF_INET;
    iproute_node.key.p.prefixlen = prefixlen;
    sal_memcpy (&iproute_node.key.p.u.prefix4, dest, sizeof(addr_ipv4_t));
    pst_iproute_node = tbl_iproute_node_get_iproute_node(&iproute_node.key);

    if (NULL != pst_iproute_node)
    {
        if (ECMP_ENABLE != pst_iproute_node->multipath)
        {
            return PM_E_FAIL;
        }
    }
    else
    {
        return PM_E_FAIL;
    }

    return PM_E_NONE;
}


int32
rt_fea_ipv6_add_route_multi_nl(vrf_id_t vrf_id, addr_ipv6_t *dest, nexthop_key_t *nhg,
                    uint32 add_nh_num, uint8 prefixlen, uint32 distance)
{
    uint32 index = 0;
    char        buf1[CMD_BUF_256] = {0};

    tbl_iproute_node_t  *pst_iproute_node = NULL;
    tbl_iproute_node_t  iproute_node;
    nexthop_ecmp_t nh_group;
    int32 rc = PM_E_NONE;
    /* GB only support 8 ecmp nexthops, modified by liwh for bug 45595, 2017-11-17 */
    uint32  nh_num = 0;
    /* liwh end */
    uint32  ecmp_hash = 0;

    if (NULL == dest)
    {
        DEBUG_ROUTE_IPV6("dest is NULL, return");
        return PM_E_FAIL;
    }

    DEBUG_ROUTE_IPV6("dest(%s), add_nh_num(%u), prefixlen(%u), distance(%u)\n", 
        cdb_addr_ipv6_val2str(buf1, CMD_BUF_256, dest), add_nh_num, prefixlen, distance);

    for(index = 0; index < add_nh_num; index++)
    {
        DEBUG_ROUTE_IPV6("nexthop%2u:%s %s\n", index, 
            cdb_addr_ipv6_val2str(buf1, CMD_BUF_256, &nhg[index].u.ipv6), rt_fea_api_nexthop_type_desc(nhg[index].type));
    }

    addr_t dest_all = {0};
    dest_all.family = AF_INET6;
    sal_memcpy(&dest_all.u.prefix6, dest, sizeof(addr_ipv6_t));

    /* GB only support 8 ecmp nexthops, modified by liwh for bug 45595, 2017-11-17 */
    nh_num = rt_fea_v6_ecmp_member_count_get_max();
    if (nh_num > add_nh_num)
    {
        nh_num =  add_nh_num;   
    }
    /* liwh end */
        
    sal_memset(&iproute_node, 0, sizeof(iproute_node));

    /* 1. get route  */
    iproute_node.key.vrf_id = vrf_id;
    iproute_node.key.p.family = AF_INET6;
    iproute_node.key.p.prefixlen = prefixlen;
    sal_memcpy (&iproute_node.key.p.u.prefix6, dest, sizeof(addr_ipv6_t));
    pst_iproute_node = tbl_iproute_node_get_iproute_node(&iproute_node.key);

    /*2.1 add new route*/
    if(NULL == pst_iproute_node)
    {
        /*fix bug 44963, arp table redundant*/
        if (rt_fea_v6_ecmp_count_max_check())
        {
            DEBUG_ROUTE_IPV6("The ecmp entry num is more than maximum when add new ipv6 route!");
            log_sys(M_MOD_ROUTE, E_ERROR, "The ecmp entry num is more than maximum %u when add new ipv6 route!", 
                rt_fea_v6_ecmp_count_get_max());

            rt_fea_ipv6_add_to_iproute_node_add_fail(vrf_id, dest, prefixlen, NULL, nhg, add_nh_num);
            
            return PM_E_EXCEED_ECMP_MAX_NUM;
        }
        
        rc = rt_fea_ipv6_add_nh_by_group(vrf_id, dest, nhg, nh_num, prefixlen, distance);
        
        rc += rt_fea_ipv6_add_new_multi_nh_route(vrf_id, dest, nhg, nh_num, prefixlen, distance);
        return rc;
    }
    
    /*2.2 update nhgroup, add member to nhg*/
    /*2.2.1 case one member to many member*/
    if(!pst_iproute_node->multipath && nh_num>1)
    {
        /*fix bug 45072, arp table redundant*/
        if (rt_fea_v6_ecmp_count_max_check())
        {
            DEBUG_ROUTE_IPV6("The ecmp entry num is more than maximum when change ipv6 route to ecmp!");
            log_sys(M_MOD_ROUTE, E_ERROR, "The ecmp entry num is more than maximum %u when change ipv6 route to ecmp!", 
                rt_fea_v6_ecmp_count_get_max());
                
            rt_fea_ipv6_add_to_iproute_node_add_fail(vrf_id, dest, prefixlen, NULL, nhg, add_nh_num);
            
            return PM_E_EXCEED_ECMP_MAX_NUM;
        }

        /*update ecmp group members*/
        rt_fea_ipv6_update_nh_by_group(vrf_id, &dest_all, nhg, nh_num, prefixlen, distance);

        
        /*create nhg and update route to nhg*/
        #if 0
        log_sys(M_MOD_ROUTE, E_ERROR, "rt_fea_ipv4_add_new_multi_nh_route1: dest %x\n"
            "multipath number is %d"
            "    nexthop group %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x \n", 
            iproute_node.key.p.u.prefix4.s_addr, multipath,
            pst_iproute_node->nh_group.nh[0].u.ipv4.s_addr,
            pst_iproute_node->nh_group.nh[1].u.ipv4.s_addr,
            pst_iproute_node->nh_group.nh[2].u.ipv4.s_addr,
            pst_iproute_node->nh_group.nh[3].u.ipv4.s_addr,
            pst_iproute_node->nh_group.nh[4].u.ipv4.s_addr,
            pst_iproute_node->nh_group.nh[5].u.ipv4.s_addr,
            pst_iproute_node->nh_group.nh[6].u.ipv4.s_addr,
            pst_iproute_node->nh_group.nh[7].u.ipv4.s_addr,
            pst_iproute_node->nh_group.nh[8].u.ipv4.s_addr,
            pst_iproute_node->nh_group.nh[9].u.ipv4.s_addr,
            pst_iproute_node->nh_group.nh[10].u.ipv4.s_addr,
            pst_iproute_node->nh_group.nh[11].u.ipv4.s_addr,
            pst_iproute_node->nh_group.nh[12].u.ipv4.s_addr,
            pst_iproute_node->nh_group.nh[13].u.ipv4.s_addr,
            pst_iproute_node->nh_group.nh[14].u.ipv4.s_addr,
            pst_iproute_node->nh_group.nh[15].u.ipv4.s_addr
            );
        #endif
        // TODO:cc
        //ecmp_hash = rt_fea_ipv4_get_ecmp_hash_type(vrf_id, dest, prefixlen, nh_num);
        PM_E_RETURN(rt_fea_ipv6_add_nhg(pst_iproute_node, nhg, nh_num, ecmp_hash, TRUE, FALSE));
        pst_iproute_node->nh_group.nh_num = nh_num;
        sal_memcpy(pst_iproute_node->nh_group.nh, nhg, nh_num*sizeof(nexthop_key_t));
        
        pst_iproute_node->multipath = 1;
        //pst_iproute_node->rt_type = GLB_IPUC_REMOTE;
        PM_E_RETURN(tbl_iproute_node_set_iproute_node_field(pst_iproute_node, TBL_IPROUTE_NODE_FLD_MULTIPATH));
        PM_E_RETURN(tbl_iproute_node_set_iproute_node_field(pst_iproute_node, TBL_IPROUTE_NODE_FLD_NH_GROUP));
        
    }

    /*2.3 update nhgroup, del member to nhg*/
    /*2.3.1 case many member to one member*/
    else if(pst_iproute_node->multipath && nh_num==1)
    {
        /*update ecmp group members*/
        rt_fea_ipv6_update_nh_by_group(vrf_id, &dest_all, nhg, nh_num, prefixlen, distance);

        /*update route to nh, update nexthop group last member to nexthop*/
        sal_memcpy(&pst_iproute_node->nh_key, nhg, sizeof(nexthop_key_t));
        PM_E_RETURN(tbl_iproute_node_set_iproute_node_field(pst_iproute_node, TBL_IPROUTE_NODE_FLD_NH_KEY));
        PM_E_RETURN(rt_fea_ipv6_del_nhg(pst_iproute_node, NULL, nh_num, 0, FALSE));

        pst_iproute_node->nh_group.nh_num = 0;
        sal_memset(pst_iproute_node->nh_group.nh, 0, sizeof(pst_iproute_node->nh_group.nh));
        PM_E_RETURN(tbl_iproute_node_set_iproute_node_field(pst_iproute_node, TBL_IPROUTE_NODE_FLD_NH_GROUP));

        pst_iproute_node->multipath = 0;
        PM_E_RETURN(tbl_iproute_node_set_iproute_node_field(pst_iproute_node, TBL_IPROUTE_NODE_FLD_MULTIPATH));
    }
    /*2.4 other case*/
    else if(pst_iproute_node->multipath && nh_num>1)
    {
        /*update ecmp group members*/
        rt_fea_ipv6_update_nh_by_group(vrf_id, &dest_all, nhg, nh_num, prefixlen, distance);

        /* update nhg, add new  nhg*/
        sal_memcpy(&nh_group, &pst_iproute_node->nh_group, sizeof(pst_iproute_node->nh_group));

        /*create nhg and update route to nhg*/
        #if 0
        log_sys(M_MOD_ROUTE, E_ERROR, "rt_fea_ipv4_add_new_multi_nh_route1: dest %x\n"
            "multipath number is %d"
            "    nexthop group %x %x %x %x %x %x %x %x\n", 
            iproute_node.key.p.u.prefix4.s_addr, multipath,
            nhg[0].u.ipv4.s_addr,
            nhg[1].u.ipv4.s_addr,
            nhg[2].u.ipv4.s_addr,
            nhg[3].u.ipv4.s_addr,
            nhg[4].u.ipv4.s_addr,
            nhg[5].u.ipv4.s_addr,
            nhg[6].u.ipv4.s_addr,
            nhg[7].u.ipv4.s_addr
            );
        #endif
        // TODO: cc
        //ecmp_hash = rt_fea_ipv4_get_ecmp_hash_type(vrf_id, dest, prefixlen, nh_num);
        PM_E_RETURN(rt_fea_ipv6_add_nhg(pst_iproute_node, nhg, nh_num, ecmp_hash, FALSE, FALSE));
        

        sal_memset(&pst_iproute_node->nh_group, 0, sizeof(pst_iproute_node->nh_group));
        pst_iproute_node->nh_group.nh_num = nh_num;
        sal_memcpy(pst_iproute_node->nh_group.nh, nhg, nh_num*sizeof(nexthop_key_t));
        PM_E_RETURN(tbl_iproute_node_set_iproute_node_field(pst_iproute_node, TBL_IPROUTE_NODE_FLD_NH_GROUP));

        /*remove old nhg*/
        PM_E_RETURN(rt_fea_ipv6_del_nhg(pst_iproute_node, nh_group.nh, nh_group.nh_num, 0, FALSE));
        
    }
    else
    {
        DEBUG_ROUTE_IPV6("Can't process ecmp case v6 route is %s!", 
            cdb_addr_ipv6_val2str(buf1, CMD_BUF_256, &pst_iproute_node->key.p.u.prefix6));
        return PM_E_FAIL;
    }
        
    return PM_E_NONE;
}

int32
rt_fea_ipv6_del_route_multi_nl(vrf_id_t vrf_id, addr_ipv6_t *dest, nexthop_key_t *nhg,
                    uint32 add_nh_num, uint8 prefixlen, uint32 distance)
{
    uint32 index = 0;
    char        buf1[CMD_BUF_256] = {0};

    tbl_iproute_node_t  *pst_iproute_node = NULL;
    tbl_iproute_node_t  iproute_node;
    tbl_iproute_node_t  iproute_node_cp;
    //int32               rc = PM_E_NONE;

    if (NULL == dest)
    {
        DEBUG_ROUTE_IPV6("dest is NULL, return");
        return PM_E_FAIL;
    }

    DEBUG_ROUTE_IPV6("dest(%s), add_nh_num(%u), prefixlen(%u), distance(%u)\n", 
        cdb_addr_ipv6_val2str(buf1, CMD_BUF_256, dest), add_nh_num, prefixlen, distance);

    for(index = 0; index < add_nh_num; index++)
    {
        DEBUG_ROUTE_IPV6("nexthop%2u:%s %s\n", index, 
            cdb_addr_ipv6_val2str(buf1, CMD_BUF_256, &nhg[index].u.ipv6), rt_fea_api_nexthop_type_desc(nhg[index].type));
    }

    /* 1. del route */
    sal_memset(&iproute_node, 0, sizeof(iproute_node));

    iproute_node.key.vrf_id = vrf_id;
    iproute_node.key.p.family = AF_INET6;
    iproute_node.key.p.prefixlen = prefixlen;
    sal_memcpy (&iproute_node.key.p.u.prefix6, dest, sizeof(addr_ipv6_t));
    pst_iproute_node = tbl_iproute_node_get_iproute_node(&iproute_node.key);

    if (pst_iproute_node && NULL == nhg)
    {
        rt_fea_ipv6_del_from_iproute_node_add_fail(vrf_id, dest, prefixlen, NULL, pst_iproute_node->nh_group.nh, add_nh_num);
    }
    else
    {
        rt_fea_ipv6_del_from_iproute_node_add_fail(vrf_id, dest, prefixlen, NULL, nhg, add_nh_num);
    }
    
    if (NULL != pst_iproute_node)
    {
        //log_sys(M_MOD_ROUTE, E_ERROR, "rt_fea_ipv4_del_route_multi_nl_1 \n");
        if (ECMP_ENABLE != pst_iproute_node->multipath)
        {
            DEBUG_ROUTE_IPV6("ipv6 route node is not multipath, return");
            return PM_E_FAIL;
        }

        rt_fea_route_v6_count_decrease();
        rt_route_v6_counter_dec(DEFAULT_VRF_ID);
        //rt_ecmp_v6_counter_dec(DEFAULT_VRF_ID);

        sal_memset(&iproute_node_cp, 0, sizeof(iproute_node_cp));
        sal_memcpy (&iproute_node_cp, pst_iproute_node, sizeof(iproute_node_cp));

        rt_fea_ipv6_del_nh_by_group(&iproute_node_cp, vrf_id, dest, nhg, add_nh_num, prefixlen, distance);
        
        tbl_iproute_node_del_iproute_node(&iproute_node.key);

        rt_fea_ipv6_del_nhg(&iproute_node_cp, nhg, add_nh_num, 0, FALSE);

         //log_sys(M_MOD_ROUTE, E_ERROR, "rt_fea_ipv4_del_route_multi_nl_2, dest %x, ifname %s \n", *dest, pst_iproute_node->nh_key.ifname);

    }
    else
    {
        DEBUG_ROUTE_IPV6("ipv6 route node is NULL, return");
        return PM_E_NONE;
    }
        
    return PM_E_NONE;
}

int32
rt_fea_ipv4_add_route_multi_nl(vrf_id_t vrf_id, addr_ipv4_t *dest, nexthop_key_t *nhg,
                    uint32 add_nh_num, uint8 prefixlen, uint32 distance, uint32 route_type)
{
    tbl_iproute_node_t  *pst_iproute_node = NULL;
    tbl_iproute_node_t  iproute_node;
    nexthop_ecmp_t nh_group;
    int32 rc = PM_E_NONE;
    /* GB only support 8 ecmp nexthops, modified by liwh for bug 45595, 2017-11-17 */
    uint32  nh_num = 0;
    /* liwh end */
    uint32  ecmp_hash = 0;
    char        buf1[CMD_BUF_256] = {0};
    char        buf2[CMD_BUF_256] = {0};
    int32 index = 0;
    
    DEBUG_ROUTE_IPV4();
    
    if (NULL != dest && NULL != nhg)
    {
        DEBUG_ROUTE_IPV4("dest(%s), prefixlen(%u), distance(%u), add_nh_num(%u), route_type(%d)\n", 
            cdb_addr_ipv4_val2str(buf1, CMD_BUF_256, dest), 
            prefixlen, distance, add_nh_num, route_type);

        for(index = 0; index < add_nh_num; index++)
        {
            DEBUG_ROUTE_IPV4("nhg(%u) %s\n", index, 
                cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, &nhg[index].u.ipv4));
        }
    }
    else if (NULL != dest &&  NULL == nhg)
    {
        DEBUG_ROUTE_IPV4("dest(%s), prefixlen(%u), distance(%u), add_nh_num(%u), route_type(%d)\n", 
            cdb_addr_ipv4_val2str(buf1, CMD_BUF_256, dest), 
            prefixlen, distance, add_nh_num, route_type);
    }
    else if (NULL == dest &&  NULL != nhg)
    {
        DEBUG_ROUTE_IPV4("prefixlen(%u), distance(%u), add_nh_num(%u), route_type(%d)\n", 
            prefixlen, distance, add_nh_num, route_type);

        for(index = 0; index < add_nh_num; index++)
        {
            DEBUG_ROUTE_IPV4("nhg(%u) %s\n", index, 
                cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, &nhg[index].u.ipv4));
        }
    }
    if (NULL == dest)
    {
        return PM_E_FAIL;
    }

    addr_t dest_all = {0};
    dest_all.family = AF_INET;
    sal_memcpy(&dest_all.u.prefix4, dest, sizeof(addr_ipv4_t));

    /* GB only support 8 ecmp nexthops, modified by liwh for bug 45595, 2017-11-17 */
    nh_num = rt_fea_ecmp_member_count_get_max();
    if (nh_num > add_nh_num)
    {
        nh_num =  add_nh_num;   
    }
    /* liwh end */
        
    sal_memset(&iproute_node, 0, sizeof(iproute_node));

    /* 1. get route  */
    iproute_node.key.vrf_id = vrf_id;
    iproute_node.key.p.family = AF_INET;
    iproute_node.key.p.prefixlen = prefixlen;
    sal_memcpy (&iproute_node.key.p.u.prefix4, dest, sizeof(addr_ipv4_t));
    pst_iproute_node = tbl_iproute_node_get_iproute_node(&iproute_node.key);

    /*2.1 add new route*/
    if(NULL == pst_iproute_node)
    {
        /*fix bug 44963, arp table redundant*/
        if (rt_fea_ecmp_count_max_check())
        {
            log_sys(M_MOD_ROUTE, E_ERROR, "The ecmp entry num is more than maximum when add new ip route!");

            rt_fea_ipv4_add_to_iproute_node_add_fail(vrf_id, dest, prefixlen, NULL, nhg, add_nh_num);
            rt_fea_ipv4_add_to_iproute_node_add_fail_count(vrf_id, dest, prefixlen, NULL, nhg, add_nh_num);

            return PM_E_EXCEED_ECMP_MAX_NUM;
        }
        
        rc = rt_fea_ipv4_add_nh_by_group(vrf_id, dest, nhg, nh_num, prefixlen, distance);

        rc += rt_fea_ipv4_add_new_multi_nh_route(vrf_id, dest, nhg, nh_num, prefixlen, distance, route_type);
        return rc;
    }
    
    /*2.2 update nhgroup, add member to nhg*/
    /*2.2.1 case one member to many member*/
    if(!pst_iproute_node->multipath && nh_num>1)
    {
        /*fix bug 45072, arp table redundant*/
        if (rt_fea_ecmp_count_max_check())
        {
            tbl_nexthop_group_t nhgroup;
            sal_memset(&nhgroup, 0, sizeof(nhgroup));

            ecmp_hash = rt_fea_ipv4_get_ecmp_hash_type(vrf_id, dest, prefixlen, nh_num);
            nhgroup.key.rr_en = rt_fea_ipv4_ecmp_hash_type_is_rr(ecmp_hash);
            nhgroup.key.nh_num = add_nh_num;
            sal_memcpy(nhgroup.key.nh, nhg, add_nh_num*sizeof(nexthop_key_t));
    
            if (tbl_nexthop_group_get_nexthop_group(&nhgroup.key))
            {
            }
            else
            {
                log_sys(M_MOD_ROUTE, E_ERROR, "The ecmp entry num is more than maximum when change ip route to ecmp!");
                
                rt_fea_ipv4_add_to_iproute_node_add_fail(vrf_id, dest, prefixlen, NULL, nhg, add_nh_num);
                rt_fea_ipv4_add_to_iproute_node_add_fail_count(vrf_id, dest, prefixlen, NULL, nhg, add_nh_num);
                
                return PM_E_EXCEED_ECMP_MAX_NUM;
            }
        }

        /*update ecmp group members*/
        rt_fea_ipv4_update_nh_by_group(vrf_id, &dest_all, nhg, nh_num, prefixlen, distance);

        
        /*create nhg and update route to nhg*/
        #if 0
        log_sys(M_MOD_ROUTE, E_ERROR, "rt_fea_ipv4_add_new_multi_nh_route1: dest %x\n"
            "multipath number is %d"
            "    nexthop group %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x \n", 
            iproute_node.key.p.u.prefix4.s_addr, multipath,
            pst_iproute_node->nh_group.nh[0].u.ipv4.s_addr,
            pst_iproute_node->nh_group.nh[1].u.ipv4.s_addr,
            pst_iproute_node->nh_group.nh[2].u.ipv4.s_addr,
            pst_iproute_node->nh_group.nh[3].u.ipv4.s_addr,
            pst_iproute_node->nh_group.nh[4].u.ipv4.s_addr,
            pst_iproute_node->nh_group.nh[5].u.ipv4.s_addr,
            pst_iproute_node->nh_group.nh[6].u.ipv4.s_addr,
            pst_iproute_node->nh_group.nh[7].u.ipv4.s_addr,
            pst_iproute_node->nh_group.nh[8].u.ipv4.s_addr,
            pst_iproute_node->nh_group.nh[9].u.ipv4.s_addr,
            pst_iproute_node->nh_group.nh[10].u.ipv4.s_addr,
            pst_iproute_node->nh_group.nh[11].u.ipv4.s_addr,
            pst_iproute_node->nh_group.nh[12].u.ipv4.s_addr,
            pst_iproute_node->nh_group.nh[13].u.ipv4.s_addr,
            pst_iproute_node->nh_group.nh[14].u.ipv4.s_addr,
            pst_iproute_node->nh_group.nh[15].u.ipv4.s_addr
            );
        #endif

        rt_fea_ipv4_del_from_iproute_node_add_fail(vrf_id, dest, prefixlen, &pst_iproute_node->nh_key.u.ipv4, NULL, 0);
        rt_fea_ipv4_del_from_iproute_node_add_fail_count(vrf_id, dest, prefixlen, &pst_iproute_node->nh_key.u.ipv4, NULL, 0);

        rt_fea_ipv4_del_from_iproute_node_add_fail(vrf_id, dest, prefixlen, NULL, nhg, nh_num);
        rt_fea_ipv4_del_from_iproute_node_add_fail_count(vrf_id, dest, prefixlen, NULL, nhg, nh_num);

        ecmp_hash = rt_fea_ipv4_get_ecmp_hash_type(vrf_id, dest, prefixlen, nh_num);
        PM_E_RETURN(rt_fea_ipv4_add_nhg(pst_iproute_node, nhg, nh_num, ecmp_hash, TRUE, TRUE));
        if (ZEBRA_ROUTE_OSPF == route_type)
        {
            nexthop_ecmp_routed_t nexthop_ecmp_routed = {0};
            sal_memset(&nexthop_ecmp_routed, 0x00, sizeof(nexthop_ecmp_routed));

            nexthop_ecmp_routed.nh_num = nh_num;
            sal_memcpy(nexthop_ecmp_routed.nh, nhg, nh_num*sizeof(nexthop_key_t));
            rt_fea_clear_nexthop_group_ifname(&nexthop_ecmp_routed);
            route_cmd_ipv4_add_nhg(&nexthop_ecmp_routed, TRUE, TRUE);
        }
        
        pst_iproute_node->nh_group.rr_en = rt_fea_ipv4_ecmp_hash_type_is_rr(ecmp_hash);
        pst_iproute_node->nh_group.nh_num = nh_num;
        sal_memcpy(pst_iproute_node->nh_group.nh, nhg, nh_num*sizeof(nexthop_key_t));
        
        pst_iproute_node->multipath = 1;
        //pst_iproute_node->rt_type = GLB_IPUC_REMOTE;
        PM_E_RETURN(tbl_iproute_node_set_iproute_node_field(pst_iproute_node, TBL_IPROUTE_NODE_FLD_MULTIPATH));
        PM_E_RETURN(tbl_iproute_node_set_iproute_node_field(pst_iproute_node, TBL_IPROUTE_NODE_FLD_NH_GROUP));
        
    }

    /*2.3 update nhgroup, del member to nhg*/
    /*2.3.1 case many member to one member*/
    else if(pst_iproute_node->multipath && nh_num==1)
    {
        /*update ecmp group members*/
        rt_fea_ipv4_update_nh_by_group(vrf_id, &dest_all, nhg, nh_num, prefixlen, distance);

        rt_fea_ipv4_del_from_iproute_node_add_fail(vrf_id, dest, prefixlen, &nhg->u.ipv4, NULL, 0);
        rt_fea_ipv4_del_from_iproute_node_add_fail_count(vrf_id, dest, prefixlen, &nhg->u.ipv4, NULL, 0);

        rt_fea_ipv4_del_from_iproute_node_add_fail(vrf_id, dest, prefixlen, NULL, pst_iproute_node->nh_group.nh, pst_iproute_node->nh_group.nh_num);
        rt_fea_ipv4_del_from_iproute_node_add_fail_count(vrf_id, dest, prefixlen, NULL, pst_iproute_node->nh_group.nh, pst_iproute_node->nh_group.nh_num);

        /*update route to nh, update nexthop group last member to nexthop*/
        sal_memcpy(&pst_iproute_node->nh_key, nhg, sizeof(nexthop_key_t));
        PM_E_RETURN(tbl_iproute_node_set_iproute_node_field(pst_iproute_node, TBL_IPROUTE_NODE_FLD_NH_KEY));
        PM_E_RETURN(rt_fea_ipv4_del_nhg(pst_iproute_node, NULL, nh_num, 0, TRUE));
        if (ZEBRA_ROUTE_OSPF == route_type)
        {
            nexthop_ecmp_routed_t nexthop_ecmp_routed = {0};
            sal_memset(&nexthop_ecmp_routed, 0x00, sizeof(nexthop_ecmp_routed));

            nexthop_ecmp_routed.nh_num = pst_iproute_node->nh_group.nh_num;
            sal_memcpy(nexthop_ecmp_routed.nh, pst_iproute_node->nh_group.nh, pst_iproute_node->nh_group.nh_num*sizeof(nexthop_key_t));
            rt_fea_clear_nexthop_group_ifname(&nexthop_ecmp_routed);
            route_cmd_ipv4_del_nhg(&nexthop_ecmp_routed, TRUE);
        }

        pst_iproute_node->nh_group.rr_en = 0;
        pst_iproute_node->nh_group.nh_num = 0;
        sal_memset(pst_iproute_node->nh_group.nh, 0, sizeof(pst_iproute_node->nh_group.nh));
        PM_E_RETURN(tbl_iproute_node_set_iproute_node_field(pst_iproute_node, TBL_IPROUTE_NODE_FLD_NH_GROUP));

        pst_iproute_node->multipath = 0;
        PM_E_RETURN(tbl_iproute_node_set_iproute_node_field(pst_iproute_node, TBL_IPROUTE_NODE_FLD_MULTIPATH));
    }
    /*2.4 other case*/
    else if(pst_iproute_node->multipath && nh_num>1)
    {
        /*update ecmp group members*/
        rt_fea_ipv4_update_nh_by_group(vrf_id, &dest_all, nhg, nh_num, prefixlen, distance);

        /* update nhg, add new  nhg*/
        sal_memcpy(&nh_group, &pst_iproute_node->nh_group, sizeof(pst_iproute_node->nh_group));

        /*create nhg and update route to nhg*/
        #if 0
        log_sys(M_MOD_ROUTE, E_ERROR, "rt_fea_ipv4_add_new_multi_nh_route1: dest %x\n"
            "multipath number is %d"
            "    nexthop group %x %x %x %x %x %x %x %x\n", 
            iproute_node.key.p.u.prefix4.s_addr, multipath,
            nhg[0].u.ipv4.s_addr,
            nhg[1].u.ipv4.s_addr,
            nhg[2].u.ipv4.s_addr,
            nhg[3].u.ipv4.s_addr,
            nhg[4].u.ipv4.s_addr,
            nhg[5].u.ipv4.s_addr,
            nhg[6].u.ipv4.s_addr,
            nhg[7].u.ipv4.s_addr
            );
        #endif

        rt_fea_ipv4_del_from_iproute_node_add_fail(vrf_id, dest, prefixlen, NULL, nhg, nh_num);
        rt_fea_ipv4_del_from_iproute_node_add_fail_count(vrf_id, dest, prefixlen, NULL, nhg, nh_num);

        rt_fea_ipv4_del_from_iproute_node_add_fail(vrf_id, dest, prefixlen, NULL, pst_iproute_node->nh_group.nh, pst_iproute_node->nh_group.nh_num);
        rt_fea_ipv4_del_from_iproute_node_add_fail_count(vrf_id, dest, prefixlen, NULL, pst_iproute_node->nh_group.nh, pst_iproute_node->nh_group.nh_num);

        ecmp_hash = rt_fea_ipv4_get_ecmp_hash_type(vrf_id, dest, prefixlen, nh_num);
        PM_E_RETURN(rt_fea_ipv4_add_nhg(pst_iproute_node, nhg, nh_num, ecmp_hash, FALSE, TRUE));
        if (ZEBRA_ROUTE_OSPF == route_type)
        {
            nexthop_ecmp_routed_t nexthop_ecmp_routed = {0};
            sal_memset(&nexthop_ecmp_routed, 0x00, sizeof(nexthop_ecmp_routed));

            nexthop_ecmp_routed.nh_num = nh_num;
            sal_memcpy(nexthop_ecmp_routed.nh, nhg, nh_num*sizeof(nexthop_key_t));
            rt_fea_clear_nexthop_group_ifname(&nexthop_ecmp_routed);
            route_cmd_ipv4_add_nhg(&nexthop_ecmp_routed, TRUE, TRUE);
        }

        sal_memset(&pst_iproute_node->nh_group, 0, sizeof(pst_iproute_node->nh_group));
        pst_iproute_node->nh_group.rr_en = rt_fea_ipv4_ecmp_hash_type_is_rr(ecmp_hash);
        pst_iproute_node->nh_group.nh_num = nh_num;
        sal_memcpy(pst_iproute_node->nh_group.nh, nhg, nh_num*sizeof(nexthop_key_t));
        PM_E_RETURN(tbl_iproute_node_set_iproute_node_field(pst_iproute_node, TBL_IPROUTE_NODE_FLD_NH_GROUP));

        /*remove old nhg*/
        PM_E_RETURN(rt_fea_ipv4_del_nhg(pst_iproute_node, nh_group.nh, nh_group.nh_num, nh_group.rr_en, TRUE));
        if (ZEBRA_ROUTE_OSPF == route_type)
        {
            nexthop_ecmp_routed_t nexthop_ecmp_routed = {0};
            sal_memset(&nexthop_ecmp_routed, 0x00, sizeof(nexthop_ecmp_routed));

            nexthop_ecmp_routed.nh_num = nh_group.nh_num;
            sal_memcpy(nexthop_ecmp_routed.nh, nh_group.nh, nh_group.nh_num*sizeof(nexthop_key_t));
            rt_fea_clear_nexthop_group_ifname(&nexthop_ecmp_routed);
            route_cmd_ipv4_del_nhg(&nexthop_ecmp_routed, TRUE);
        }
        
    }
    else
    {
        //log_sys(M_MOD_ROUTE, E_INFORMATIONAL, "can't process ecmp case route is %x!", pst_iproute_node->key.p.u.prefix4);
        return PM_E_FAIL;
    }
    
    return rc;

}

int32
rt_fea_ipv4_del_route_multi_nl(vrf_id_t vrf_id, addr_ipv4_t *dest, nexthop_key_t *nhg,
                    uint32 nh_num, uint8 prefixlen, uint32 distance, uint32 route_type)
{
    tbl_iproute_node_t  *pst_iproute_node = NULL;
    tbl_iproute_node_t  iproute_node;
    tbl_iproute_node_t  iproute_node_cp;
    int32               rc = PM_E_NONE;
    char        buf1[CMD_BUF_256] = {0};
    char        buf2[CMD_BUF_256] = {0};
    int32 index = 0;

    DEBUG_ROUTE_IPV4();
    
    if (NULL != dest && NULL != nhg)
    {
        DEBUG_ROUTE_IPV4("dest(%s), prefixlen(%u), distance(%u), nh_num(%u), route_type(%d)\n", 
            cdb_addr_ipv4_val2str(buf1, CMD_BUF_256, dest), 
            prefixlen, distance, nh_num, route_type);

        for(index = 0; index < nh_num; index++)
        {
            DEBUG_ROUTE_IPV4("nhg(%u) %s\n", index, 
                cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, &nhg[index].u.ipv4));
        }
    }
    else if (NULL != dest &&  NULL == nhg)
    {
        DEBUG_ROUTE_IPV4("dest(%s), prefixlen(%u), distance(%u), nh_num(%u), route_type(%d)\n", 
            cdb_addr_ipv4_val2str(buf1, CMD_BUF_256, dest), 
            prefixlen, distance, nh_num, route_type);
    }
    else if (NULL == dest &&  NULL != nhg)
    {
        DEBUG_ROUTE_IPV4("prefixlen(%u), distance(%u), nh_num(%u), route_type(%d)\n", 
            prefixlen, distance, nh_num, route_type);

        for(index = 0; index < nh_num; index++)
        {
            DEBUG_ROUTE_IPV4("nhg(%u) %s\n", index, 
                cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, &nhg[index].u.ipv4));
        }
    }
    
    if (NULL == dest)
    {
        return PM_E_FAIL;
    }

    /* 1. del route */
    sal_memset(&iproute_node, 0, sizeof(iproute_node));

    iproute_node.key.vrf_id = vrf_id;
    iproute_node.key.p.family = AF_INET;
    iproute_node.key.p.prefixlen = prefixlen;
    sal_memcpy (&iproute_node.key.p.u.prefix4, dest, sizeof(addr_ipv4_t));
    pst_iproute_node = tbl_iproute_node_get_iproute_node(&iproute_node.key);

    rt_fea_ipv4_del_from_iproute_node_add_fail(vrf_id, dest, prefixlen, NULL, nhg, nh_num);
    rt_fea_ipv4_del_from_iproute_node_add_fail_count(vrf_id, dest, prefixlen, NULL, nhg, nh_num);
    
    if (NULL != pst_iproute_node)
    {
        //log_sys(M_MOD_ROUTE, E_ERROR, "rt_fea_ipv4_del_route_multi_nl_1 \n");
        if (ECMP_ENABLE != pst_iproute_node->multipath)
        {
            return PM_E_FAIL;
        }

        rt_fea_route_count_decrease();
        rt_route_counter_dec(DEFAULT_VRF_ID);
        //rt_ecmp_counter_dec(DEFAULT_VRF_ID);

        sal_memset(&iproute_node_cp, 0, sizeof(iproute_node_cp));
        sal_memcpy (&iproute_node_cp, pst_iproute_node, sizeof(iproute_node_cp));

        rt_fea_ipv4_del_nh_by_group(&iproute_node_cp, vrf_id, dest, nhg, nh_num, prefixlen, distance);
        
        rc = tbl_iproute_node_del_iproute_node(&iproute_node.key);

        rt_fea_ipv4_del_nhg(&iproute_node_cp, nhg, nh_num, iproute_node_cp.nh_group.rr_en, TRUE);
        if (ZEBRA_ROUTE_OSPF == route_type || (0 == route_type && ZEBRA_ROUTE_OSPF == iproute_node_cp.route_type))
        {
            nexthop_ecmp_routed_t nexthop_ecmp_routed = {0};
            sal_memset(&nexthop_ecmp_routed, 0x00, sizeof(nexthop_ecmp_routed));

            nexthop_ecmp_routed.nh_num = nh_num;
            sal_memcpy(nexthop_ecmp_routed.nh, nhg, nh_num*sizeof(nexthop_key_t));
            rt_fea_clear_nexthop_group_ifname(&nexthop_ecmp_routed);
            route_cmd_ipv4_del_nhg(&nexthop_ecmp_routed, TRUE);
        }

         //log_sys(M_MOD_ROUTE, E_ERROR, "rt_fea_ipv4_del_route_multi_nl_2, dest %x, ifname %s \n", *dest, pst_iproute_node->nh_key.ifname);

    }
    else
    {
        return PM_E_NONE;
    }
    
    return rc;
}

int32
rt_fea_static_max_cfg_check(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    FILE *fp                                = NULL;
    tbl_route_global_t  *pst_rt_gl          = NULL;
    tbl_iproute_node_t  *pst_iproute_node   = NULL;
    tbl_iproute_node_key_t  rn_key;
    addr_ipv4_t         ip_addr;
    addr_ipv4_t         ip_addr_gw;
    tbl_iter_args_t     args;
    uint32              cnt      = 0;
    uint32              mask     = 0;
    uint32              mask_len = 0;
    uint32              buf = 0;
    uint32              buf2 = 0;

    CFG_DBG_FUNC();
    sal_memset(&args, 0, sizeof(args));
    sal_memset(&ip_addr, 0, sizeof(addr_ipv4_t));
    sal_memset(&ip_addr_gw, 0, sizeof(addr_ipv4_t));

    switch (para->oper)
    {
    case CDB_OPER_SET:
        if (0 == argc)
        {
            CFG_INVALID_PARAM_RET("The parameter number is not invalid!");
        }
        else if (1 == argc)
        {
            args.argv[0] = &cnt;
            tbl_iproute_node_iterate(rt_fea_static_route_count, &args);

            if (0 == sal_strcmp(argv[0], "num_check"))
            {
                cnt = *(uint32 *)args.argv[0];
                if (0 > cnt)
                {
                    CFG_INVALID_PARAM_RET("Counts of static route is not invalid!");
                }

                pst_rt_gl = tbl_route_global_get_route_global();
                if (NULL == pst_rt_gl)
                {
                    return PM_E_NOT_EXIST;
                }
                if (0 == pst_rt_gl->max_static)
                {
                    if (RT_STATIC_MAX_NUM <= cnt)
                    {
                        fp = sal_fopen("/tmp/rt_static_max_check", "w+");
                        sal_fprintf(fp, "Please check the count of static route entry , and max_static num in use is default 1024\n");
                        sal_fclose(fp);
                    }
                }
                else
                {
                    if (pst_rt_gl->max_static <= cnt)
                    {
                        fp = sal_fopen("/tmp/rt_static_max_check", "w+");
                        sal_fprintf(fp, "Please check the count of static route entry, and max_static num in use is define from tbl_route_global which is %d\n",
                                                pst_rt_gl->max_static);
                        sal_fclose(fp);
                    }
                }
            }
        }
        else if (5 == argc)
        {
            if (0 == sal_strcmp(argv[0], "num_check") && 0 == sal_strcmp(argv[1], "nexthop_group"))
            {
                buf2 = rt_cli_get_ipv4_prefix(argv[4]);

                ip_addr_gw.s_addr = buf2;
                if(IPV4_NET127(ip_addr_gw.s_addr)
                    || IPV4_CHECK2(ip_addr_gw.s_addr)
                    || IPV4_NET0(ip_addr_gw.s_addr))
                {
                    fp = sal_fopen("/tmp/rt_static_max_check", "a+");
                    sal_fprintf(fp, "Please check the configuation of static route gateway!\n");
                    sal_fclose(fp);
                    return PM_E_NONE;
                }

                mask_len = sal_atoi(argv[3]);
                if (0 == ip_addr.s_addr && 0 == mask_len)
                {
                    goto out_num_ck;
                }

                buf = rt_cli_get_ipv4_prefix(argv[2]);

                ip_addr.s_addr = buf;
                if(IN_MULTICAST(ip_addr.s_addr)
                    || IPV4_NET127(ip_addr.s_addr)
                    || IPV4_CHECK1(ip_addr.s_addr)
                    || IPV4_CHECK2(ip_addr.s_addr))
                {
                    fp = sal_fopen("/tmp/rt_static_max_check", "a+");
                    sal_fprintf(fp, "Please check the configuation of static route prefix!\n");
                    sal_fclose(fp);
                    return PM_E_NONE;
                }

                IPV4_LEN_TO_MASK(mask, (32 - mask_len));
                mask = ~ mask;
                ip_addr.s_addr = ip_addr.s_addr & mask;

                if (mask == ip_addr.s_addr && mask)
                {
                    fp = sal_fopen("/tmp/rt_static_max_check", "a+");
                    sal_fprintf(fp, "Please check the configuation of static route entry prefix!\n");
                    sal_fclose(fp);
                    return PM_E_NONE;
                }

out_num_ck:
                sal_memset(&rn_key, 0, sizeof(rn_key));
                rn_key.vrf_id = 0;
                rn_key.p.family = AF_INET;
                rn_key.p.prefixlen = mask_len;
                sal_memcpy (&rn_key.p.u.prefix4, &ip_addr, sizeof(addr_ipv4_t));
                pst_iproute_node = tbl_iproute_node_get_iproute_node(&rn_key);
                if (NULL == pst_iproute_node)
                {
                    return PM_E_NOT_EXIST;
                }

                if ((MULTIPATH_CHECK - 1) < pst_iproute_node->nh_group.nh_num)
                {
                    fp = sal_fopen("/tmp/rt_static_max_check", "a+");
                    sal_fprintf(fp, "Please check the nexthop num of static route entry cannot add more than %d!\n",
                                            pst_iproute_node->nh_group.nh_num);
                    sal_fclose(fp);
                    return PM_E_NONE;
                }
            }
        }
        else
        {
            CFG_INVALID_PARAM_RET("The parameter number %u is not invalid", argc);
        }

        break;

    default:
        CFG_INVALID_PARAM_RET("Only support update action");
        break;
    }

    return PM_E_NONE;
}

int32
rt_nhg_cmd_read_session_all(cfg_cmd_para_t *para)
{
    field_parse_t *p_field  = NULL;
    FILE          *fp       = NULL;    
    field_parse_t field;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        return PM_E_NO_MEMORY;
    }

    sal_memset(&args, 0 ,sizeof(args));
    p_field = &field;
    sal_memset(p_field, FLD_ID_ALL, sizeof(field_parse_t));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_nexthop_group_iterate((TBL_ITER_CB_FUNC)tbl_nexthop_group_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
rt_nhg_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    CFG_DBG_FUNC();

    switch (para->oper)
    {
    case CDB_OPER_GET:
        rt_nhg_cmd_read_session_all(para);
        break;

    default:
        break;
    }

    return PM_E_NONE;
}

int32
rt_fea_init(route_master_t *master)
{
    rt_fea_ipv4_forwarding_set(TRUE);
    lib_netlink_init();

    lib_netlink_register_add_newlink(rt_fea_add_newlink);
    lib_netlink_register_del_newlink(rt_fea_del_newlink);

    lib_netlink_register_ipv4_add_neigh(rt_fea_ipv4_add_neighbor);
    lib_netlink_register_ipv4_del_neigh(rt_fea_ipv4_del_neighbor);
    lib_netlink_register_ipv4_get_neigh(arp_api_miss_trigger);

    cdb_register_cfg_act_cb(ACT_FLUSH_ARP, arp_cmd_process_flusharp);
    cdb_register_cfg_tbl_cb(TBL_NEXTHOP_GROUP, rt_nhg_cmd_process);

    lib_netlink_register_ipv4_add_route(rt_fea_ipv4_add_route_nl);
    lib_netlink_register_ipv4_del_route(rt_fea_ipv4_del_route_nl);
    lib_netlink_register_ipv4_add_addr(rt_fea_ipv4_add_addr_nl);
    lib_netlink_register_ipv4_del_addr(rt_fea_ipv4_del_addr_nl);

    lib_netlink_register_ipv4_add_mroute(rt_fea_ipv4_add_mroute_nl);
    lib_netlink_register_ipv4_del_mroute(rt_fea_ipv4_del_mroute_nl);

    lib_netlink_register_ipv4_add_route_multipath_check(rt_fea_ipv4_add_route_multi_nl_check);
    lib_netlink_register_ipv4_del_route_multipath_check(rt_fea_ipv4_del_route_multi_nl_check);
    lib_netlink_register_ipv4_add_route_multipath(rt_fea_ipv4_add_route_multi_nl);
    lib_netlink_register_ipv4_del_route_multipath(rt_fea_ipv4_del_route_multi_nl);

    //lib_netlink_register_ipv4_add_nh_group(rt_fea_ipv4_add_nh_group_nl);
    //lib_netlink_register_ipv4_del_nh_group(rt_fea_ipv4_del_nh_group_nl);

    lib_netlink_register_ipv4_get_nh_key(rt_fea_ipv4_get_nh_key);


    lib_netlink_register_ipv6_add_route(rt_fea_ipv6_add_route_nl);
    lib_netlink_register_ipv6_del_route(rt_fea_ipv6_del_route_nl);
    lib_netlink_register_ipv6_add_addr(rt_fea_ipv6_add_addr_nl);
    lib_netlink_register_ipv6_del_addr(rt_fea_ipv6_del_addr_nl);

    lib_netlink_register_ipv6_add_route_multipath_check(rt_fea_ipv6_add_route_multi_nl_check);
    lib_netlink_register_ipv6_del_route_multipath_check(rt_fea_ipv6_del_route_multi_nl_check);
    lib_netlink_register_ipv6_add_route_multipath(rt_fea_ipv6_add_route_multi_nl);
    lib_netlink_register_ipv6_del_route_multipath(rt_fea_ipv6_del_route_multi_nl);

    lib_netlink_register_ipv6_get_nh_key(rt_fea_ipv6_get_nh_key);

    lib_netlink_register_ipv6_get_neigh(nd_api_miss_trigger);
    
    return PM_E_NONE;
}

#endif /* CONFIG_IPV4 */
