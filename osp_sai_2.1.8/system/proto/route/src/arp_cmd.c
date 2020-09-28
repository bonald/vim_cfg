
#include "proto.h"
#include "glb_ip_define.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_kernel_if_define.h"
#include "gen/tbl_kernel_if.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_arp_define.h"
#include "gen/tbl_arp.h"
#include "gen/tbl_arp_fib_define.h"
#include "gen/tbl_arp_fib.h"
#include "gen/tbl_route_global_define.h"
#include "gen/tbl_route_global.h"
#include "gen/tbl_vrrp_vmac_define.h"
#include "gen/tbl_vrrp_vmac.h"
#include "cdb_ds.h"
#include "lib_ioctl.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "arp_api.h"
#include "rt_fea.h"
#include "route.h"


#define ARP_IPV4_ADDR_LEN_IN_BIT        32          /**< IPv4 address length in bit */

#define ARP_IPV4_LEN_TO_MASK(mask, len)  \
    {                           \
        (mask) = (len) ? ~((1 << (ARP_IPV4_ADDR_LEN_IN_BIT - (len))) - 1) : 0; \
    }


/* This is the adapter for centec user ARP */
#define CENTEC_USER_ARP

static      uint32          ultotal = 0;
static      uint32          ulincmplt = 0;
static      uint32          ulstatic = 0;
static      uint32          uldynamic = 0;
static      uint32          ulrouter = 0;   

int32
_arp_user_flush_entry_one(tbl_arp_t *p_db_arp)
{
    int32 rc = PM_E_NONE;

    if (NULL == p_db_arp)
    {
        return PM_E_FAIL;
    }
    
    rc = arp_api_aging_time_init(p_db_arp);

    return rc;
}

int32
arp_user_flush_entry(void *data, tbl_iter_args_t *pargs)
{
    tbl_arp_t       *p_db_arp = data;
    int32           rc = PM_E_NONE;

    if (ARP_TYPE_STATIC == p_db_arp->type )
    {
        return PM_E_NONE;
    }
    
    if ((NULL == pargs->argv[0]) && (NULL == pargs->argv[1]))
    {
        rc = _arp_user_flush_entry_one(p_db_arp);
    }

    if ((NULL != pargs->argv[0]) && (NULL == pargs->argv[1]))
    {
        if (0 == sal_strcmp(p_db_arp->ifname, pargs->argv[0]))
        {
            rc = _arp_user_flush_entry_one(p_db_arp);
        }
    }

    if ((NULL == pargs->argv[0]) && (NULL != pargs->argv[1]))
    {
        if (0 == sal_memcmp(&p_db_arp->key.ip, pargs->argv[1], sizeof(addr_ipv4_t)))
        {
            rc = _arp_user_flush_entry_one(p_db_arp);
        }
    }

    return rc;
}

static int32
_arp_cmd_process_flush(char *ifname, addr_ipv4_t *destip)
{
    tbl_iter_args_t iter_args;

    sal_memset(&iter_args, 0, sizeof(tbl_iter_args_t));
    iter_args.argv[0] = ifname;
    iter_args.argv[1] = destip;

    tbl_arp_iterate(arp_user_flush_entry, &iter_args);
    return PM_E_NONE;
}

int32
arp_cmd_process_flusharp(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if    = NULL;
    addr_ipv4_t     destip;
    uint32          i   = 0;
    int32           rc  = PM_E_NONE;

    CFG_DBG_FUNC();
    
    switch (para->oper)
    {
    case CDB_OPER_SET:
        if (0 == argc)
        {
            rc = _arp_cmd_process_flush(NULL, NULL);
        }
        else if (2 == argc)
        {
            if (0 == sal_strcmp(argv[i], "interface"))
            {
                p_db_if = tbl_interface_get_interface_by_name(argv[i+1]);
                if (NULL == p_db_if)
                {
                    CFG_INVALID_PARAM_RET("Interface does not exist!");
                }

                rc = _arp_cmd_process_flush(argv[i+1], NULL);
            }

            if (0 == sal_strcmp(argv[i], "ip"))
            {
                CFG_PARSER_IP_ADDR(argv[i+1], &destip, para);
                rc = _arp_cmd_process_flush(NULL, &destip);
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

    return rc;
}

int32
arp_cmd_mac_addr_str2val(uint8 *mac, const char *str, cfg_cmd_para_t *para)
{
    char    *mac_str    = NULL;
    char    buf[64];
    int32   rc = PM_E_NONE;

    sal_memset(buf, 0, sizeof(buf));
    sal_strncpy(buf, str, 64);
    mac_str = buf;

    rc = cdb_arp_mac_address_is_valid(mac_str);
    if (rc < 0)
    {
        CFG_INVALID_PARAM_RET("Mac address is invalid!");
    }
    
    if (3 != sal_sscanf(mac_str, "%4hx.%4hx.%4hx", (uint16*)&mac[0], (uint16*)&mac[2], (uint16*)&mac[4]))
    {
        CFG_INVALID_PARAM_RET("Mac address is invalid!");
    }
    
    *(uint16*)&mac[0] = sal_hton16(*(uint16*)&mac[0]);
    *(uint16*)&mac[2] = sal_hton16(*(uint16*)&mac[2]);
    *(uint16*)&mac[4] = sal_hton16(*(uint16*)&mac[4]);
    
    return PM_E_NONE;
}

int32
arp_cmd_ipv4_get_ifname_by_addr(void *obj, tbl_iter_args_t *pargs)
{
    addr_ipv4_t         *pst_addr       = pargs->argv[0];
    tbl_kernel_if_t     *pst_kernel_if  = obj;
    addr_ipv4_t         addr_cp1;
    addr_ipv4_t         addr_cp2;
    uint32              mask = 0;

    sal_memset(&addr_cp1, 0, sizeof(addr_ipv4_t));
    sal_memset(&addr_cp2, 0, sizeof(addr_ipv4_t));
    sal_memcpy(&addr_cp2 , pst_addr, sizeof(addr_ipv4_t));
    sal_memcpy(&addr_cp1, &pst_kernel_if->ip_addr, sizeof(addr_ipv4_t));

    addr_cp1.s_addr = sal_htonl(addr_cp1.s_addr);
    addr_cp2.s_addr = sal_htonl(addr_cp2.s_addr);
    ARP_IPV4_LEN_TO_MASK(mask, pst_kernel_if->masklen);
    addr_cp1.s_addr = addr_cp1.s_addr & mask;
    addr_cp2.s_addr = addr_cp2.s_addr & mask;
    if (0 != addr_cp2.s_addr && 0 == sal_memcmp(&addr_cp1, &addr_cp2, sizeof(addr_ipv4_t)))
    {
        pargs->argv[1] = pst_kernel_if->name;
    }

    return PM_E_NONE;    
}


int32
arp_cmd_ipv4_get_rt_ifname_by_addr(void *obj, tbl_iter_args_t *pargs)
{
    addr_ipv4_t         *pst_addr       = pargs->argv[0];
    tbl_route_if_t      *pst_rt_if      = obj;
    addr_ipv4_t         addr_cp1;
    addr_ipv4_t         addr_cp2;
    uint32              mask = 0;
    ctclib_slistnode_t  *listnode  = NULL;
    ds_connected_t      *p_db_conn = NULL;

    sal_memset(&addr_cp1, 0, sizeof(addr_ipv4_t));
    sal_memset(&addr_cp2, 0, sizeof(addr_ipv4_t));

    CTCLIB_SLIST_LOOP(pst_rt_if->ifc_ipv4.obj_list, p_db_conn, listnode)
    {
        sal_memcpy(&addr_cp1, &p_db_conn->key.address.u.prefix4, sizeof(addr_ipv4_t));
        sal_memcpy(&addr_cp2 , pst_addr, sizeof(addr_ipv4_t));
        addr_cp1.s_addr = sal_htonl(addr_cp1.s_addr);
        addr_cp2.s_addr = sal_htonl(addr_cp2.s_addr);
        if (0 != addr_cp2.s_addr && 0 == sal_memcmp(&addr_cp1, &addr_cp2, sizeof(addr_ipv4_t)))
        {
            pargs->argv[3] = pst_rt_if;            
        }
        
        ARP_IPV4_LEN_TO_MASK(mask, p_db_conn->key.address.prefixlen);
        addr_cp1.s_addr = addr_cp1.s_addr & mask;
        addr_cp2.s_addr = addr_cp2.s_addr & mask;
        if (0 != addr_cp2.s_addr && 0 == sal_memcmp(&addr_cp1, &addr_cp2, sizeof(addr_ipv4_t)))
        {
            pargs->argv[1] = pst_rt_if->key.name;
            pargs->argv[2] = pst_rt_if;            
            return PM_E_NONE;            
        }
    }

    return PM_E_NONE;    
}


int32
arp_cmd_ipv4_check_no_arp(addr_ipv4_t *dest, cfg_cmd_para_t *para)
{
    tbl_arp_t   *pst_arp    = NULL;
    tbl_arp_t   arp;

    sal_memset(&arp, 0, sizeof(arp));
    sal_memcpy(&arp.key.ip, dest, sizeof(addr_ipv4_t));

    /* get arp info */
    pst_arp = tbl_arp_get_arp(&arp.key);
    if (NULL == pst_arp || (NULL != pst_arp && ARP_TYPE_STATIC != pst_arp->type))
    {
        CFG_INVALID_PARAM_RET("%% Bad ARP command - No such static ARP entry!");
    }

    return PM_E_NONE;
}

static int32
_arp_user_cmd_ipv4_addr_check(addr_ipv4_t *ipaddr, char *err_str, uint32 len)
{
    uint32 ntoh_s_addr = 0;

    ntoh_s_addr = sal_ntoh32(ipaddr->s_addr);

    /* Check IP address */
    if (IN_EXPERIMENTAL(ntoh_s_addr) 
        || IN_CLASSD(ntoh_s_addr)
        || IN_LOOPBACK(ntoh_s_addr)
        || IPV4_NET0(ntoh_s_addr))
    {
        sal_snprintf(err_str, len, "Invalid IPv4 address");
        return PM_E_INVALID_PARAM;
    }

    return PM_E_NONE;
}

static int32
_arp_user_cmd_mac_addr_check(uint8 *macaddr, char *err_str, uint32 len)
{
    tbl_sys_global_t    *p_glb = tbl_sys_global_get_sys_global();
    uint32              mac1 = 0;
    uint16              mac2 = 0;
    
    mac1 = *(uint32*)&macaddr[0];
    mac2 = *(uint16*)&macaddr[4];
    if ((mac1 == 0 && mac2 == 0)
         || (mac1 == 0xFFFFFFFF && mac2 == 0xFFFF)
         || GLB_IS_MCAST_MAC(macaddr))
    {
        sal_snprintf(err_str, len, "Invalid MAC address");
        return PM_E_INVALID_PARAM;
    }

    if (0 == sal_memcmp(macaddr, p_glb->route_mac, MAC_ADDR_LEN))
    {
        sal_snprintf(err_str, len, "Cannot set route MAC address");
        return PM_E_INVALID_PARAM;
    }

    return PM_E_NONE;
}

static int32
_arp_user_cmd_param_check(tbl_arp_t *p_arp, uint32 check_mac, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    rc = _arp_user_cmd_ipv4_addr_check(&p_arp->key.ip, para->p_rs->desc, MAX_CMD_RET_LEN);
    if (rc < 0)
    {
        para->p_rs->rc = rc;
        return rc;
    }

    if (check_mac)
    {
        rc = _arp_user_cmd_mac_addr_check(p_arp->mac_addr, para->p_rs->desc, MAX_CMD_RET_LEN);
        if (rc < 0)
        {
            para->p_rs->rc = rc;
            return rc;
        }
    }
    
    return PM_E_NONE;
}

int32
arp_user_cmd_arp_addr_check(addr_ipv4_t *dest, uint32 *is_if_addr)
{
    tbl_interface_t *p_db_if = NULL;
    tbl_route_if_t  *p_rt_if = NULL;
    tbl_arp_t       *p_arp = NULL;
    tbl_arp_key_t   arp_key;

    sal_memset(&arp_key, 0 ,sizeof(arp_key));

    sal_memcpy(&arp_key, dest, sizeof(addr_ipv4_t));
    p_arp = tbl_arp_get_arp(&arp_key);
    if (NULL != p_arp)
    {
        p_db_if = tbl_interface_get_interface_by_name(p_arp->ifname);
        if (NULL == p_db_if)
        {
            return PM_E_FAIL;
        }

        p_rt_if = arp_api_check_addr_with_rt_if_exist(p_db_if, dest);
        if (NULL != p_rt_if && 0 == sal_strncmp(p_arp->ifname, p_rt_if->key.name, sizeof(p_arp->ifname)))
        {
            *is_if_addr = TRUE;
        }
    }

    return PM_E_NONE;
}

int32
arp_user_cmd_ipv4_add_arp_fib(vrf_id_t vrf_id, addr_ipv4_t *dest, uint8 *macaddr, char *name, cfg_cmd_para_t *para)
{
    tbl_arp_fib_t       *p_arp_fib = NULL;
    tbl_interface_t     *p_db_if = NULL;
    tbl_nexthop_t       *pst_nexthop = NULL;
    tbl_route_if_t      *p_rt_if = NULL;
    tbl_route_if_key_t  rtif_key;
    tbl_arp_fib_t       arp_fib;
    char                arp_ifname[IFNAME_SIZE];
    int32               rc = PM_E_NONE;
    tbl_arp_t *p_db_arp = NULL;
    tbl_arp_t           arp;

    sal_memset(&arp, 0, sizeof(tbl_arp_t));
    sal_memset(&arp_ifname, 0, sizeof(arp_ifname));
    sal_memset(&arp_fib, 0, sizeof(arp_fib));
    sal_memcpy(&arp_fib.key.ip, dest, sizeof(addr_ipv4_t));
    sal_memcpy(arp_fib.mac_addr, macaddr, sizeof(arp_fib.mac_addr));
    arp_fib.type = ARP_TYPE_STATIC;
    
    if (0 != sal_strcmp(name, arp_ifname))
    {
        sal_strcpy(arp_fib.ifname, name);
        p_db_if = tbl_interface_get_interface_by_name(name);

        sal_memset(&rtif_key, 0, sizeof(rtif_key));
        sal_memcpy(rtif_key.name, name, sizeof(rtif_key.name));
        p_rt_if = tbl_route_if_get_route_if(&rtif_key);
        if (NULL != p_db_if && NULL != p_rt_if)
        {
            if (0 == sal_memcmp(macaddr, p_db_if->mac_addr, sizeof(p_db_if->mac_addr)))
            {            
                CFG_CONFLICT_RET("Static ARP entry can not set with route-mac address");
            }
        }
    }
    else
    {
        return rc;
    }
    
    /* get arp fib info */
    p_arp_fib = tbl_arp_fib_get_arp_fib(&arp_fib.key);
    if (NULL != p_arp_fib)
    {
        pst_nexthop = rt_fea_nexthop_ipv4_get(name, dest);
        if (NULL == pst_nexthop)
        {
            pst_nexthop = rt_fea_nexthop_ipv4_add(name, dest);
            if (NULL == pst_nexthop)
            {
                return PM_E_NOT_EXIST;
            }
        }

        if (arp_fib.type != p_arp_fib->type 
                    || 0 != sal_memcmp(p_arp_fib->mac_addr, macaddr, sizeof(mac_addr_t)))
        {
            rc = tbl_arp_fib_del_arp_fib(&p_arp_fib->key);
            rc = tbl_arp_fib_add_arp_fib(&arp_fib);
            if (PM_E_NONE == rc)
            {
                sal_memcpy(&arp.key.ip, dest, sizeof(addr_ipv4_t));
                p_db_arp = tbl_arp_get_arp(&arp.key);
                if (p_db_arp)
                {
                    GLB_SET_FLAG(p_db_arp->flags, ARP_SYN_HW);
                }
            }
        }
    }
    else
    {
        /* 3. add nexthop */
        pst_nexthop = rt_fea_nexthop_ipv4_add(name, dest);
        if (NULL == pst_nexthop)
        {
            return PM_E_NOT_EXIST;
        }

        rc = tbl_arp_fib_add_arp_fib(&arp_fib);
        if (PM_E_NONE == rc)
        {
            sal_memcpy(&arp.key.ip, dest, sizeof(addr_ipv4_t));
            p_db_arp = tbl_arp_get_arp(&arp.key);
            if (p_db_arp)
            {
                GLB_SET_FLAG(p_db_arp->flags, ARP_SYN_HW);
            }
        }
        
    }

    return rc;
}

int32
arp_user_cmd_ipv4_del_arp_fib(vrf_id_t vrf_id, addr_ipv4_t *dest, cfg_cmd_para_t *para)
{
    tbl_arp_fib_t   *p_arp_fib = NULL;
    tbl_nexthop_t   *pst_nexthop = NULL;
    tbl_arp_fib_t   arp_fib;
    char            arp_ifname[IFNAME_SIZE];
    int32           rc = PM_E_NONE;

    sal_memset(&arp_fib, 0, sizeof(arp_fib));
    sal_memcpy(&arp_fib.key.ip, dest, sizeof(addr_ipv4_t));
    sal_memset(&arp_ifname, 0, sizeof(arp_ifname));

    /* get arp fib info */
    p_arp_fib = tbl_arp_fib_get_arp_fib(&arp_fib.key);
    if (NULL != p_arp_fib)
    {
        sal_memcpy(arp_ifname, p_arp_fib->ifname, sizeof(arp_ifname));
        rc = tbl_arp_fib_del_arp_fib(&p_arp_fib->key);
        pst_nexthop = rt_fea_nexthop_ipv4_get(arp_ifname, dest);
        if (NULL != pst_nexthop)
        {
            rc = rt_fea_nexthop_ipv4_del(arp_ifname, dest);
        }
    }

    return rc;
}

int32
arp_user_cmd_ipv4_add_arp(vrf_id_t vrf_id, addr_ipv4_t *dest, uint8 *macaddr, char *name, cfg_cmd_para_t *para)
{
    tbl_arp_t           *p_arp          = NULL;
    tbl_interface_t     *p_db_if        = NULL;
    tbl_route_if_t      *p_db_rtif = NULL;
    ctclib_slistnode_t  *listnode = NULL;
    ds_connected_t      *p_tmp_conn = NULL;
    tbl_route_if_key_t  rtif_key;
    tbl_arp_t           arp;
    char                arp_ifname[IFNAME_SIZE];
    int32               rc = PM_E_NONE;

    sal_memset(&rtif_key, 0, sizeof(rtif_key));
    sal_memcpy(rtif_key.name, name, sizeof(rtif_key.name));
    
    sal_memset(&arp_ifname, 0, sizeof(arp_ifname));
    sal_memset(&arp, 0, sizeof(tbl_arp_t));
    sal_memcpy(&arp.key.ip, dest, sizeof(addr_ipv4_t));
    sal_memcpy(arp.mac_addr, macaddr, sizeof(arp.mac_addr));
    
    p_db_rtif = tbl_route_if_get_route_if(&rtif_key);
    if (NULL != p_db_rtif)
    {
        CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, p_tmp_conn, listnode)
        {
            if (p_tmp_conn->key.address.u.prefix4.s_addr == dest->s_addr)
            {
                arp.is_router = FALSE;
            }
        }
    }

    /* 1. config check */
    PM_E_RETURN(_arp_user_cmd_param_check(&arp, TRUE, para));
    if (0 != sal_strcmp(name, arp_ifname))    
    {
        sal_strcpy(arp.ifname, name);
        p_db_if = tbl_interface_get_interface_by_name(name);
        if (NULL == p_db_if || NULL == p_db_rtif)
        {
            return PM_E_FAIL;
        }
        
        if (0 == sal_memcmp(macaddr, p_db_if->mac_addr, sizeof(p_db_if->mac_addr)))
        {            
            CFG_CONFLICT_RET("Static ARP entry can not set with route-mac address");
        }
    }
        
    /* 2. check duplicate config */
    p_arp = tbl_arp_get_arp(&arp.key);

    /* 3. call API */    
    if (NULL != p_arp)
    {
       rc = arp_api_update_static_arp(p_arp, &arp, p_db_if);
    }
    else
    {
        rc = arp_api_add_static_arp(&arp, p_db_if);
    }

    if (rc < 0)
    {
        if (PM_E_EXCEED_MAX_NUM == rc)
        {
            CFG_CONFLICT_RET("Exceeding maximum ARP entries limit");
        }
        
        CFG_CONFLICT_RET("Add ARP failed, error is %d", rc);
    }
    
    return rc;
}

int32
arp_user_cmd_ipv4_del_arp(vrf_id_t vrf_id, addr_ipv4_t *dest, cfg_cmd_para_t *para)
{
    tbl_route_if_t  *p_db_rtif = NULL;
    tbl_interface_t *p_db_if = NULL;
    ds_connected_t  *p_db_conn = NULL;
    tbl_arp_t       *p_arp = NULL;
    tbl_arp_t       arp;
    int32           rc = PM_E_NONE;
    
    sal_memset(&arp, 0, sizeof(tbl_arp_t));
    sal_memcpy(&arp.key.ip, dest, sizeof(addr_ipv4_t));

    /* 1. config check */
    PM_E_RETURN(_arp_user_cmd_param_check(&arp, FALSE, para));
    
    /* get arp user info */
    p_arp = tbl_arp_get_arp(&arp.key);
    if (NULL == p_arp)
    {
        return PM_E_NOT_EXIST;
    }

    if (GLB_INVALID_IFINDEX == p_arp->ifindex)
    {
        p_db_rtif = arp_api_check_addr_with_rt_if_exist_advance(dest);
        if (NULL != p_db_rtif)
        {
            p_db_if = tbl_interface_get_interface_by_name(p_db_rtif->key.name);
            p_db_conn = rt_if_lookup_ifc_ipv4(p_db_if, &p_arp->key.ip);
            /* modified by wangjj for filter the loopack interface bug 41901, 2016-11-30 */
            if (NULL != p_db_if && !INTERFACE_IS_LOOPBACK(p_db_if) && NULL != p_db_conn)
            {
                p_arp->ifindex = p_db_if->ifindex;
                p_arp->is_router = TRUE;
                sal_strcpy(p_arp->ifname, p_db_if->key.name);
                sal_memcpy(p_arp->mac_addr, p_db_if->mac_addr, sizeof(p_db_if->mac_addr));

                tbl_arp_set_arp_field(p_arp, TBL_ARP_FLD_IFINDEX);
                tbl_arp_set_arp_field(p_arp, TBL_ARP_FLD_IS_ROUTER);
                tbl_arp_set_arp_field(p_arp, TBL_ARP_FLD_IFNAME);
                tbl_arp_set_arp_field(p_arp, TBL_ARP_FLD_MAC_ADDR);

                /* the count of arp entry should dec when changing from static to router arp */
                rt_neigh_counter_dec(GLB_DEF_VRFID);

                return PM_E_NONE;
            }
        }
    }

    if (TRUE == p_arp->is_router)
    {
        CFG_CONFLICT_RET("Cannot unset ARP of interface address");
    }

    if (ARP_TYPE_STATIC != p_arp->type)
    {
        CFG_CONFLICT_RET("No such static ARP entry");
    }
        
    /* 3. call API */
    rc = arp_api_del_static_arp(p_arp);

    return rc;
}

static int32
_arp_user_cmd_get_arp(char *ifname, cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE            *fp = NULL;
    tbl_iter_args_t args;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    args.argv[2] = ifname;
    
    p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_arp_fib_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
arp_user_cmd_process_data_trans(char **argv, int32 argc, cfg_cmd_para_t *para,
                    vrf_id_t *vrf_id, addr_ipv4_t *dest, uint8 *macaddr)
{
    char                *mac_str = NULL;    
    addr_ipv4_t         ip_prefix;
    mac_addr_t          mac;    
    int32               retv;
    char                buf[64];
    int32               rc = PM_E_NONE;

    vrf_id = DEFAULT_VRF_ID;
    sal_memset(&ip_prefix, 0, sizeof(addr_ipv4_t));
    sal_memset(buf, 0, sizeof(buf));
    sal_memset(mac, 0, sizeof(mac));
    
    if (2 == argc)
    {
        retv = sal_inet_pton(AF_INET, argv[0], buf);   
        if (!retv)                                       
        {                                                
            CFG_INVALID_PARAM_RET("Invalid IPv4 address");
        }

        if (0 == sal_strncmp(argv[1], "ffff.ffff.ffff", sizeof("ffff.ffff.ffff")))
        {
            CFG_INVALID_PARAM_RET("Invalid MAC address");
        }
        
        sal_memcpy(&ip_prefix, buf, sizeof(addr_ipv4_t));
        sal_memcpy(dest, buf, sizeof(addr_ipv4_t));

        mac_str = argv[1];
        rc = arp_cmd_mac_addr_str2val(mac, mac_str, para);

        sal_memcpy(macaddr, mac, sizeof(mac));
    }
    else if (1 == argc)
    {
        retv = sal_inet_pton(AF_INET, argv[0], buf);   
        if (!retv)                                       
        {                                                
            CFG_INVALID_PARAM_RET("Invalid IPv4 address");
        }
        
        sal_memcpy(&ip_prefix, buf, sizeof(addr_ipv4_t));
        sal_memcpy(dest, buf, sizeof(addr_ipv4_t));
        rc =  arp_cmd_ipv4_check_no_arp(&ip_prefix, para);
    }
    else 
    {
        CFG_INVALID_PARAM_RET("The parameter number %u is not invalid", argc);
    }

    return rc;
}

int32
arp_cmd_process_arp_fib(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_sys_global_t    *p_sys_glb = tbl_sys_global_get_sys_global();
    cdb_node_t      *p_node = NULL;
    addr_ipv4_t     dest;
    mac_addr_t      mac_addr;    
    char            arp_ifname[IFNAME_SIZE];
    addr_ipv4_t     dest_check;    
    tbl_iter_args_t args;
    vrf_id_t        vrf_id;
    field_parse_t   field;
    uint32          is_if_addr = 0;
    int32           rc = PM_E_NONE;
    
    CFG_DBG_FUNC();
    p_node = cdb_get_tbl(TBL_ARP_FIB);

    if (NULL == p_sys_glb)
    {
        return PM_E_FAIL;
    }
    
    sal_memset(&dest_check, 0, sizeof(dest_check));
    sal_memset(&dest, 0, sizeof(dest));    
    sal_memset(mac_addr, 0, sizeof(mac_addr));
    args.argv[1] = NULL;
        
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        rc = arp_user_cmd_process_data_trans(argv, argc, para, &vrf_id, &dest, mac_addr);
        if (PM_E_NONE != rc)
        {
            return rc;
        }

        if (0 == sal_memcmp(mac_addr, p_sys_glb->route_mac, sizeof(mac_addr)))
        {
            CFG_INVALID_PARAM_RET("Static ARP entry can not set with route-mac address");
        }

        /* modified by chenc for bug 51035, 2019-02-18 */
        tbl_vrrp_vmac_master_t *p_vrrp_vmac_master = tbl_vrrp_vmac_get_master();
        ctclib_slistnode_t *listnode = NULL;
        ctclib_slistnode_t *next = NULL;
        tbl_vrrp_vmac_t *p_db_vrrp_vmac = NULL;

        if (NULL == p_vrrp_vmac_master)
        {
            CFG_INVALID_PARAM_RET("VRRP vmac master does not exist");
        }

        CTCLIB_SLIST_LOOP_DEL(p_vrrp_vmac_master->vrrp_vmac_list, p_db_vrrp_vmac, listnode, next)
        {
            if (0 == sal_memcmp(mac_addr, p_db_vrrp_vmac->key.mac, sizeof(mac_addr)))
            {
                CFG_INVALID_PARAM_RET("Static ARP entry can not set with VRRP VMAC");
            }
        }
        /*chenc end*/
        
        sal_memcpy(&dest_check, &dest, sizeof(dest_check));
        dest_check.s_addr = sal_htonl(dest_check.s_addr);
        if (IPV4_NET127(dest_check.s_addr))
        {
            CFG_INVALID_PARAM_RET("Invalid IPv4 address");
            //CFG_INVALID_PARAM_RET("Please check cli IP %s is invalid!", argv[0]);
        }

        args.argv[0] = &dest;
        args.argv[3] = NULL;
        sal_memset(arp_ifname, 0, sizeof(arp_ifname));
        //tbl_kernel_if_iterate(arp_cmd_ipv4_get_ifname_by_addr, &args);
        tbl_route_if_iterate(arp_cmd_ipv4_get_rt_ifname_by_addr, &args);

        /* check the interface state from kernel intf to tbl interface */
        if (NULL != args.argv[1])
        {
            sal_memcpy(arp_ifname, args.argv[1], sizeof(arp_ifname));
        }

        if (NULL != args.argv[3])
        {
            is_if_addr = TRUE;
        }
        
        //arp_user_cmd_arp_addr_check(&dest, &is_if_addr);
        rc = arp_user_cmd_ipv4_add_arp(vrf_id, &dest, mac_addr, arp_ifname, para);
        if (PM_E_NONE == rc && !is_if_addr)
        {
            rc = arp_user_cmd_ipv4_add_arp_fib(vrf_id, &dest, mac_addr, arp_ifname, para);
        }
        
        break;

    case CDB_OPER_DEL:
        rc = arp_user_cmd_process_data_trans(argv, argc, para, &vrf_id, &dest, mac_addr);
        if (PM_E_NONE != rc)
        {
            CFG_INVALID_PARAM_RET("Bad ARP command - No such static ARP entry!");
        }

        args.argv[0] = &dest;
        sal_memset(arp_ifname, 0, sizeof(arp_ifname));
        args.argv[3] = NULL;
        tbl_route_if_iterate(arp_cmd_ipv4_get_rt_ifname_by_addr, &args);
        if (NULL != args.argv[1])
        {
            sal_memcpy(arp_ifname, args.argv[1], sizeof(arp_ifname));
        }

        if (NULL != args.argv[3])
        {
            is_if_addr = TRUE;
        }
        
        //arp_user_cmd_arp_addr_check(&dest, &is_if_addr);
        rc = arp_user_cmd_ipv4_del_arp(vrf_id, &dest, para);
        if (PM_E_NONE == rc && !is_if_addr)
        {
            rc = arp_user_cmd_ipv4_del_arp_fib(vrf_id, &dest, para);
        }

        break;

    case CDB_OPER_SET:
        break;

    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _arp_user_cmd_get_arp(argv[0], para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

int32
arp_cmd_show_arp_summary_count(void *obj, tbl_iter_args_t *pargs)
{
    tbl_arp_t       *p_arp = obj;
    
    if (NULL == p_arp)
    {
        return PM_E_FAIL;
    }
    
    if (AS_INCOMPLETE == p_arp->status)
    {
        ulincmplt ++;
    }

    if (p_arp->is_router)
    {
        ulrouter ++;
    } 
    else if (ARP_TYPE_STATIC == p_arp->type)
    {
        ulstatic ++;
    }
    else if (ARP_TYPE_DYNAMIC == p_arp->type)
    {
        uldynamic ++;
    }

    ultotal ++;

    return PM_E_NONE;
}

int32
arp_cmd_show_arp_one(void *obj, tbl_iter_args_t *pargs)
{
    FILE                *fp = (FILE *)pargs->argv[0];
    tbl_arp_t           *p_arp = obj;
    tbl_route_if_t      *pst_route_if = NULL;
    tbl_route_if_key_t  rtif_key;
    char        ifname_ext[IFNAME_EXT_SIZE];
    char        buf[CMD_BUF_256];
    char        buf1[CMD_BUF_256];    
    int32       age_min = 0;
    int32       ageing_timeout = 0;
    int32       ageing_delay = 0;
    int32       rc  = PM_E_NONE;

    sal_memset(&rtif_key, 0, sizeof(rtif_key));
    sal_memset(ifname_ext, 0, sizeof(ifname_ext));
    sal_memset(buf, 0, sizeof(buf));
    sal_memset(buf1, 0, sizeof(buf1));
    
    ageing_delay = p_arp->aging_delay;
    sal_strcpy(rtif_key.name, p_arp->ifname);
    pst_route_if = tbl_route_if_get_route_if(&rtif_key);
    if (NULL != pst_route_if)
    {
        ageing_timeout = pst_route_if->arp_timeout;
        if (ageing_timeout < 0)
        {
            return PM_E_NONE;
        }
    }
    
    IFNAME_ETH2SHORT(p_arp->ifname, ifname_ext);
    if (ARP_TYPE_STATIC == p_arp->type)
    {
        sal_fprintf(fp, "%-10s %-16s %-10s %-16s %-10s\n", 
            "Internet",
            cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &p_arp->key.ip), 
            "-",
            cdb_mac_addr_val2str(buf1, CMD_BUF_256, p_arp->mac_addr),
            ifname_ext);
    }
    else if (ARP_TYPE_DYNAMIC == p_arp->type)
    {
        switch (p_arp->status)
        {
        case AS_REFRESH:

            break;

        case AS_REACHABLE:
            age_min = ((ageing_timeout-ageing_delay) >= 0) ? (ageing_timeout-ageing_delay)/60 : 0;

            sal_fprintf(fp, "%-10s %-16s %-10d %-16s %-10s\n", 
                "Internet",
                cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &p_arp->key.ip), 
                age_min,
                cdb_mac_addr_val2str(buf1, CMD_BUF_256, p_arp->mac_addr),
                ifname_ext);

            break;

        case AS_INCOMPLETE:
            sal_fprintf(fp, "%-10s %-16s %-10d %-16s %-10s\n", 
                "Internet",
                cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &p_arp->key.ip), 
                0,
                "Incomplete",
                ifname_ext);
            break;

        default:
            break;
        }
    }

    return rc;
}

int32
arp_cmd_show_arp_interface(void *obj, tbl_iter_args_t *pargs)
{
    tbl_arp_t       *p_arp = obj;
    char            *ifname = pargs->argv[1];
    int32           rc  = PM_E_NONE;

    if (0 == sal_strncmp(p_arp->ifname, ifname, sizeof(p_arp->ifname)))
    {
        rc = arp_cmd_show_arp_one(obj, pargs);
    }

    return rc;
}

int32
arp_cmd_show_arp_entry(char *if_name, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    FILE            *fp = NULL;
    tbl_iter_args_t args;    
    int32           rc  = PM_E_NONE;

    if (if_name)
    {
        p_db_if = tbl_interface_get_interface_by_name(if_name);
        if (NULL == p_db_if)
        {
            CFG_CONFLICT_RET("Interface %s does not exist", if_name);
        }
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    
    sal_fprintf(fp, "%-10s %-16s %-10s %-16s %-10s\n", "Protocol", "Address", "Age (min)", "Hardware Addr", "Interface");
    sal_fprintf(fp, "----------+----------------+----------+----------------+----------\n");

    args.argv[0] = fp;
    args.argv[1] = if_name;

    if (NULL == if_name)
    {
        rc = tbl_arp_iterate(arp_cmd_show_arp_one, &args);
    }
    else
    {
        rc = tbl_arp_iterate(arp_cmd_show_arp_interface, &args);
    }

    sal_fclose(fp);

    return rc;
}

int32
arp_cmd_show_arp_summary(cfg_cmd_para_t *para)
{
    tbl_route_global_t  *p_rt_glb = tbl_route_global_get_route_global();
    FILE                *fp = NULL;
    tbl_iter_args_t     args;
    int32           rc  = PM_E_NONE;

    ultotal     = 0;
    ulincmplt   = 0;
    ulstatic    = 0;
    uldynamic   = 0;
    ulrouter    = 0;   

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");    
    rc = tbl_arp_iterate(arp_cmd_show_arp_summary_count, &args);
    
    sal_fprintf(fp, " Gratuitous ARP learning is %s\n",(p_rt_glb->gratuitous_arp_learn_en ? "enabled" : "disabled"));
    sal_fprintf(fp, " %d IP ARP entries, with %d of them incomplete\n", ultotal, ulincmplt);
    sal_fprintf(fp, " (Static:%d,  Dynamic:%d, Interface:%d)\n", ulstatic, uldynamic, ulrouter);
    sal_fprintf(fp, " ARP Pkt Received number is: %d\n", p_rt_glb->arp_pkt_rx_count);
    sal_fprintf(fp, " ARP Pkt Send number is: %d\n", p_rt_glb->arp_pkt_tx_count);
    sal_fprintf(fp, " ARP Pkt Discard number is: %d\n", p_rt_glb->arp_pkt_discard_count);    
    
    sal_fclose(fp);

    return rc;
}

int32
arp_cmd_process_arp_show(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char        ifname[IFNAME_SIZE];
    int32       rc  = PM_E_NONE;

    CFG_DBG_FUNC();
    sal_memset(ifname, 0, sizeof(ifname));
    
    switch (para->oper)
    {
    case CDB_OPER_GET:
        if (0 == argc)
        {
            rc = arp_cmd_show_arp_entry(NULL, para);
        }
        else if (2 == argc)
        {
            if (0 == sal_strcmp(argv[0], "interface"))
            {
                sal_strcpy(ifname, argv[1]);
                rc = arp_cmd_show_arp_entry(ifname, para);
            }
        }
        else if (1 == argc)
        {
            if (0 == sal_strcmp(argv[0], "summary"))
            {
                rc = arp_cmd_show_arp_summary(para);
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

    return rc;
}

static int32
_arp_cmd_get_arp(char *ifname, cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE            *fp = NULL;
    tbl_iter_args_t args;

    sal_memset(&args, 0 ,sizeof(args));
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    args.argv[0] = p_field;
    args.argv[1] = fp;
    args.argv[2] = ifname;
    
    p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_arp_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
arp_cmd_process_arp(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t      *p_node = NULL;
    field_parse_t   field;
    uint8           mac_address[6];
    char            sys_cmd[128];
    int32           rc = PM_E_NONE;

    CFG_DBG_FUNC();

    sal_memset(sys_cmd, 0, sizeof(sys_cmd));
    sal_memset(mac_address, 0, sizeof(mac_address));
    p_node = cdb_get_tbl(TBL_ARP);

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        break;

    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _arp_cmd_get_arp(argv[0], para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

