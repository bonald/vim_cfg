/****************************************************************************
* $Id$
*  Centec neighbor information related MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Centecnetworks
* Date          : 2015-07-10
* Reason        : First Create.
****************************************************************************/

#include "hsrv_inc.h"
#include "hsrv_router.h"
#include "gen/tbl_iproute_node_define.h"
#include "gen/tbl_nexthop.h"
#include "gen/tbl_nexthop_define.h"
#include "gen/tbl_fea_nexthop_define.h"
#include "gen/tbl_fea_nexthop.h"
#include "gen/tbl_nexthop_group.h"
#include "gen/tbl_nexthop_group_define.h"

#define HRSV_ROUTE_ACT_DEL      0
#define HRSV_ROUTE_ACT_ADD      1
#define HRSV_ROUTE_ACT_UPD      2

#define CTC_IPV4_ADDR_LEN_IN_BIT        32          /**< IPv4 address length in bit */

#define IPV4_LEN_TO_MASK(mask, len)  \
    {                           \
        (mask) = (len) ? ~((1 << (CTC_IPV4_ADDR_LEN_IN_BIT - (len))) - 1) : 0; \
    }

static const uint8 maskbit[] =
{
    0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff
};

extern int32
hsrv_tunneldb_cdb_route_oper_add_function(tbl_iproute_node_t* p_rt_node);

extern int32
hsrv_tunneldb_cdb_route_oper_del_function(tbl_iproute_node_t* p_rt_node);

void
sal_masklen2ip6 (int32 masklen, sai_ip6_t* netmask)
{
  char *pnt;
  int bit;
  int offset;

  sal_memset (netmask, 0, sizeof (sai_ip6_t));
  pnt = (char *) netmask;

  offset = masklen / 8;
  bit = masklen % 8;

  while (offset--)
    *pnt++ = 0xff;

  if (bit)
    *pnt = maskbit[bit];
}

int32
hsrv_route_api_get_nexthop_by_addr4(addr_ipv4_t *pst_dest, uint32 *mask_len, addr_ipv4_t *pst_nexthop)
{
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    
    tbl_iproute_node_t *p_db_rt_node = NULL;
    tbl_iproute_node_t rt_node;
    addr_ipv4_t dest_tmp;
    addr_ipv4_t gate_tmp;
    uint8 best_prefixlen = 0;
    int32 index = 32;
    tbl_route_if_t *p_db_rtif = NULL;
    nexthop_key_t        nh_key = {0};
    ds_connected_t *if_conn = NULL;
    ds_connected_t *match_conn = NULL;
    addr_ipv4_t dest_tmp_2;

    sal_memset(&rt_node, 0, sizeof(rt_node));
    rt_node.key.p.family = AF_INET;
    
    for(index = 32; index >= 0; index--)
    {
        sal_memcpy(&dest_tmp, pst_dest, sizeof(addr_ipv4_t));
        dest_tmp.s_addr = sal_htonl(dest_tmp.s_addr);
        apply_mask_ipv4_addr(&dest_tmp, index);
        
        rt_node.key.p.prefixlen = index;
        sal_memcpy(&rt_node.key.p.u.prefix4, &dest_tmp, sizeof(addr_ipv4_t));
        p_db_rt_node = tbl_iproute_node_get_iproute_node(&rt_node.key);
        if (p_db_rt_node)
        {
            break;
        }
    }

    if (NULL == p_db_rt_node)
    {
        return HSRV_E_FAIL;
    }
    
    /*get nexthop*/
    if (p_db_rt_node->multipath)
    {
        sal_memcpy(&nh_key, &p_db_rt_node->nh_group.nh[0], sizeof(nh_key));
    }
    else
    {
        sal_memcpy(&nh_key, &p_db_rt_node->nh_key, sizeof(nh_key));
    }

    p_db_rtif = tbl_route_if_get_route_if_by_name(nh_key.ifname);
    if (NULL == p_db_rtif)
    {
        return HSRV_E_FAIL;
    }

    sal_memset(&gate_tmp, 0, sizeof(addr_ipv4_t));
    /*/key=0#1.1.1.0#24#AF_INET#0.0.0.0#ifindex_v6=0#/distance=1/multipath=0/nh_key=local#0.0.0.0#eth-0-1/nh_group=nh_num=0*/
    /*/key=0#1.1.1.1#32#AF_INET#0.0.0.0#ifindex_v6=0#/distance=1/multipath=0/nh_key=local#0.0.0.0#eth-0-1/nh_group=nh_num=0*/
    if (nh_key.type == NH_TYPE_LOCAL)
    {
        sal_memcpy(&gate_tmp, &p_db_rt_node->key.p.u.prefix4, sizeof(addr_ipv4_t));
    }
    /*/key=0#2.0.0.0#24#AF_INET#0.0.0.0#ifindex_v6=0#/distance=20/multipath=0/nh_key=ipv4#1.1.1.2#eth-0-1/nh_group=nh_num=0*/
    else if (nh_key.type == NH_TYPE_IPV4)
    {
        sal_memcpy(&gate_tmp, &nh_key.u.ipv4, sizeof(addr_ipv4_t));
    }
    else
    {
        return HSRV_E_FAIL;
    }

    /*get output if address*/
    CTCLIB_SLIST_LOOP_DEL(p_db_rtif->ifc_ipv4.obj_list, if_conn, listnode, next)
    {
        if (if_conn == NULL)
        {
            continue;
        }

        /*dest is equal to route_node*/
        if (0 == sal_memcmp(&if_conn->key.address.u.prefix4, &gate_tmp, sizeof(addr_ipv4_t))
            && if_conn->key.address.prefixlen == IPV4_MAX_BITLEN)
        {
            /*get*/
            match_conn = if_conn;
            sal_memcpy(pst_nexthop, &match_conn->key.address.u.prefix4, sizeof(addr_ipv4_t));
            *mask_len = IPV4_MAX_BITLEN;

            return HSRV_E_NONE;
        }

        sal_memset(&dest_tmp_2, 0, sizeof(dest_tmp_2));
        sal_memcpy(&dest_tmp_2, &(if_conn->key.address.u.prefix4), sizeof(addr_ipv4_t));
        //gate_tmp.s_addr = sal_htonl(gate_tmp.s_addr);
        //dest_tmp_2.s_addr = sal_htonl(dest_tmp_2.s_addr);
        apply_mask_ipv4_addr(&gate_tmp, if_conn->key.address.prefixlen);
        apply_mask_ipv4_addr(&dest_tmp_2, if_conn->key.address.prefixlen);
        if ((0 == sal_memcmp(&dest_tmp_2, &gate_tmp, sizeof(addr_ipv4_t))) 
            && (if_conn->key.address.prefixlen > best_prefixlen))
        {
            /*get*/
            //best_prefixlen = if_conn->key.address.prefixlen;
            match_conn = if_conn;

            if (nh_key.type == NH_TYPE_LOCAL)
            {
                best_prefixlen = if_conn->key.address.prefixlen;
                sal_memcpy(pst_nexthop, &match_conn->key.address.u.prefix4, sizeof(addr_ipv4_t));
            }
            else if(nh_key.type == NH_TYPE_IPV4)
            {
                best_prefixlen = p_db_rt_node->key.p.prefixlen;
                sal_memcpy(pst_nexthop, &nh_key.u.ipv4, sizeof(addr_ipv4_t));
            }
            else
            {
                return HSRV_E_FAIL;
            }
            
            *mask_len = best_prefixlen;
        }
    }

    if (NULL == match_conn)
    {
        return HSRV_E_FAIL;
    }
    
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_route_create_rt_entry
 * Purpose      : create route entry
 * Input        : p_rt_node
 * Output       : N/A
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_route_create_rt_entry(tbl_iproute_node_t* p_rt_node)
{
    tbl_fea_nexthop_t   *fea_nh             = NULL;
    tbl_fea_global_t    *p_fea_global       = NULL;
    sai_route_api_t     *route_api          = NULL;
    tbl_rif_t           *p_rif              = NULL;
    sai_unicast_route_entry_t   route;
    sai_attribute_t attr[8];
    uint32          attr_count = 0;
    tbl_rif_t       rif;
    tbl_route_if_t *p_route_if = NULL;
    tbl_route_if_key_t route_if_key;

    tbl_nexthop_group_t* p_nhg = NULL; 
    tbl_nexthop_group_t nhg;

    /*1. debug out infor*/
    HSRV_IPUC_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_rt_node);

    HSRV_IPUC_DEBUG("ip route add, route %x/%d,  \n"
        "    nexthop is %x, ifname %s \n"
        "    is_multipath %d, number is %d \n"
        "        IP %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\n", 
        p_rt_node->key.p.u.prefix4.s_addr, p_rt_node->key.p.prefixlen,
        p_rt_node->nh_key.u.ipv4.s_addr, p_rt_node->nh_key.ifname,
        p_rt_node->multipath,p_rt_node->nh_group.nh_num, 
        p_rt_node->nh_group.nh[0].u.ipv4.s_addr, p_rt_node->nh_group.nh[1].u.ipv4.s_addr, 
        p_rt_node->nh_group.nh[2].u.ipv4.s_addr, p_rt_node->nh_group.nh[3].u.ipv4.s_addr, 
        p_rt_node->nh_group.nh[4].u.ipv4.s_addr, p_rt_node->nh_group.nh[5].u.ipv4.s_addr, 
        p_rt_node->nh_group.nh[6].u.ipv4.s_addr, p_rt_node->nh_group.nh[7].u.ipv4.s_addr, 
        p_rt_node->nh_group.nh[8].u.ipv4.s_addr, p_rt_node->nh_group.nh[9].u.ipv4.s_addr, 
        p_rt_node->nh_group.nh[10].u.ipv4.s_addr, p_rt_node->nh_group.nh[11].u.ipv4.s_addr, 
        p_rt_node->nh_group.nh[12].u.ipv4.s_addr, p_rt_node->nh_group.nh[13].u.ipv4.s_addr, 
        p_rt_node->nh_group.nh[14].u.ipv4.s_addr, p_rt_node->nh_group.nh[15].u.ipv4.s_addr
        );

    /*2. process tbl*/
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ROUTE,(void**)&route_api));

    /*3. create ipuc route entry or ecmp route entry */
    sal_memset(&route, 0, sizeof(route));
    sal_memset(attr, 0, sizeof(attr));
    sal_memset(&rif, 0, sizeof(rif));
    if (0 == p_rt_node->multipath)
    {
        /* BEGIN: modified by taocy for BUGXXXX. set rif_oid field to all route, for further NAT refresh l3iftype process.*/
        sal_memcpy(rif.key.name, p_rt_node->nh_key.ifname, sizeof(rif.key.name));
        p_rif = tbl_rif_get_rif(&rif.key);
        if (NULL != p_rif) {    
            attr[attr_count].id = SAI_ROUTE_ATTR_RIF_ID;
            attr[attr_count].value.oid = p_rif->rif_id;
            attr_count++;
        }
        /* END: modified by taocy for BUGXXXX*/

        if (NH_TYPE_LOCAL == p_rt_node->nh_key.type)
        {
            attr[attr_count].id = SAI_ROUTE_ATTR_PACKET_ACTION;
            attr[attr_count].value.s32 = SAI_PACKET_ACTION_TRAP;
            attr_count++;
        }

        else if (NH_TYPE_IPV4 == p_rt_node->nh_key.type || NH_TYPE_IPV6 == p_rt_node->nh_key.type)
        {
            attr[attr_count].id = SAI_ROUTE_ATTR_PACKET_ACTION;
            attr[attr_count].value.s32 = SAI_PACKET_ACTION_FORWARD;
            attr_count++;

            fea_nh = tbl_fea_nexthop_get_fea_nexthop(&p_rt_node->nh_key);
            if (NULL != fea_nh)
            {
                attr[attr_count].id = SAI_ROUTE_ATTR_NEXT_HOP_ID;
                attr[attr_count].value.oid = fea_nh->nhid;
                attr_count++;
            }
        }
        /*add by chenc for add new black hole route for bug 45589*/
        else if (NH_TYPE_BLACKHOLE == p_rt_node->nh_key.type)
        {
            attr[attr_count].id = SAI_ROUTE_ATTR_PACKET_ACTION;
            attr[attr_count].value.s32 = SAI_PACKET_ACTION_DROP;
            attr_count++;
        }
        else
        {
            HSRV_IF_ERROR_RETURN(PM_E_NOT_SUPPORT);
        }

        attr[attr_count].id = SAI_ROUTE_ATTR_TRAP_PRIORITY;
        attr[attr_count].value.u8 = 0;              // default to zero
        attr_count++;

        /* begin added by taocy for NAT, 20190603. just support IPv4. do not support multipath.*/
        sal_memcpy(route_if_key.name, p_rt_node->nh_key.ifname, IFNAME_SIZE);
        p_route_if = tbl_route_if_get_route_if(&route_if_key);
        if (p_route_if)
        {
            /* NAT inside or disable, set l3iftye == internal*/
            if (GLB_IF_NAT_OUTSIDE != p_route_if->nat_enable)
            {
                attr[attr_count].id = SAI_ROUTE_ATTR_L3_IF_TYPE;
                attr[attr_count].value.u8 = SAI_ROUTER_INTERFACE_L3_TYPE_INTERNAL;
                attr_count++;
            }
        
            if ((p_route_if->nat_category_id < 255) && (0 != p_route_if->nat_category_id))
            {
                attr[attr_count].id = SAI_ROUTE_ATTR_CATEGORY_ID;
                attr[attr_count].value.u8 = p_route_if->nat_category_id;
                attr_count++;
            }
        }
        /* end added by taocy for NAT, 20190603. just support IPv4*/
    }
    else if (1 == p_rt_node->multipath)
    {
        attr[attr_count].id = SAI_ROUTE_ATTR_PACKET_ACTION;
        attr[attr_count].value.s32 = SAI_PACKET_ACTION_FORWARD;
        attr_count++;

        sal_memcpy(&nhg.key, &p_rt_node->nh_group, sizeof(nhg.key));
        p_nhg = tbl_nexthop_group_get_nexthop_group(&nhg.key);
        if (NULL != p_nhg)
        {
            attr[attr_count].id = SAI_ROUTE_ATTR_NEXT_HOP_ID;
            attr[attr_count].value.oid = p_nhg->nhg_oid;
            attr_count++;
        }
        else
        {
            HSRV_IF_ERROR_RETURN(SAI_STATUS_ITEM_NOT_FOUND);
        }

        attr[attr_count].id = SAI_ROUTE_ATTR_TRAP_PRIORITY;
        attr[attr_count].value.u8 = 0;              // default to zero
        attr_count++;

    }
    else
    {
        return HSRV_E_NONE;
    }

    p_fea_global = tbl_fea_global_get_fea_global();
    HSRV_PTR_CHECK(p_fea_global);
    route.vr_id = p_fea_global->router_id;

    if (AF_INET == p_rt_node->key.p.family)
    {
        route.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
        route.destination.addr.ip4 = sal_htonl(p_rt_node->key.p.u.prefix4.s_addr);
        IPV4_LEN_TO_MASK(route.destination.mask.ip4, p_rt_node->key.p.prefixlen);
    }
    else if (AF_INET6 == p_rt_node->key.p.family)
    {
        route.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV6;
        //sal_memcpy(&route.destination.addr.ip6[0], &p_rt_node->key.p.u.prefix6.__u6_addr32[0], 4);
        //sal_memcpy(&route.destination.addr.ip6[1], &p_rt_node->key.p.u.prefix6[4], 4);
        //sal_memcpy(&route.destination.addr.ip6[2], &p_rt_node->key.p.u.prefix6[8], 4);
        //sal_memcpy(&route.destination.addr.ip6[3], &p_rt_node->key.p.u.prefix6[12], 4);
        
        sal_memcpy(&route.destination.addr.ip6, &p_rt_node->key.p.u.prefix6.s6_addr[0], sizeof(sai_ip6_t));
        
        //route.destination.addr.ip6[0] = htonl(route.destination.addr.ip6[0]);
        //route.destination.addr.ip6[1] = htonl(route.destination.addr.ip6[1]);
        //route.destination.addr.ip6[2] = htonl(route.destination.addr.ip6[2]);
        //route.destination.addr.ip6[3] = htonl(route.destination.addr.ip6[3]);
        
        sal_masklen2ip6(p_rt_node->key.p.prefixlen, &route.destination.mask.ip6);
    }

    HSRV_IF_ERROR_RETURN(route_api->create_route(&route, attr_count, attr));
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_route_remove_rt_entry
 * Purpose      : remove route entry
 * Input        : p_rt_node
 * Output       : N/A
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_route_remove_rt_entry(tbl_iproute_node_t* p_rt_node)
{
    tbl_fea_global_t* p_fea_global  = NULL;
    sai_route_api_t*  route_api     = NULL;
    sai_unicast_route_entry_t route;

    /*1. debug out infor*/
    HSRV_IPUC_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_rt_node);

    HSRV_IPUC_DEBUG("ip route del, route %x/%d,  \n"
        "    nexthop is %x, ifname %s \n"
        "    is_multipath %d, number is %d \n"
        "        IP %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\n",
        p_rt_node->key.p.u.prefix4.s_addr, p_rt_node->key.p.prefixlen,
        p_rt_node->nh_key.u.ipv4.s_addr, p_rt_node->nh_key.ifname,
        p_rt_node->multipath,p_rt_node->nh_group.nh_num, 
        p_rt_node->nh_group.nh[0].u.ipv4.s_addr, p_rt_node->nh_group.nh[1].u.ipv4.s_addr, 
        p_rt_node->nh_group.nh[2].u.ipv4.s_addr, p_rt_node->nh_group.nh[3].u.ipv4.s_addr, 
        p_rt_node->nh_group.nh[4].u.ipv4.s_addr, p_rt_node->nh_group.nh[5].u.ipv4.s_addr, 
        p_rt_node->nh_group.nh[6].u.ipv4.s_addr, p_rt_node->nh_group.nh[7].u.ipv4.s_addr, 
        p_rt_node->nh_group.nh[8].u.ipv4.s_addr, p_rt_node->nh_group.nh[9].u.ipv4.s_addr, 
        p_rt_node->nh_group.nh[10].u.ipv4.s_addr, p_rt_node->nh_group.nh[11].u.ipv4.s_addr, 
        p_rt_node->nh_group.nh[12].u.ipv4.s_addr, p_rt_node->nh_group.nh[13].u.ipv4.s_addr, 
        p_rt_node->nh_group.nh[14].u.ipv4.s_addr, p_rt_node->nh_group.nh[15].u.ipv4.s_addr
        );

    /*2. process tbl*/
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ROUTE,(void**)&route_api));

    /*3. remove the ipuc or ecmp route entry */
    sal_memset(&route, 0, sizeof(route));
    p_fea_global = tbl_fea_global_get_fea_global();
    HSRV_PTR_CHECK(p_fea_global);

    route.vr_id = p_fea_global->router_id;
    if (AF_INET == p_rt_node->key.p.family)
    {
        route.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
        route.destination.addr.ip4 = sal_htonl(p_rt_node->key.p.u.prefix4.s_addr);
        IPV4_LEN_TO_MASK(route.destination.mask.ip4, p_rt_node->key.p.prefixlen);
    }
    else if (AF_INET6 == p_rt_node->key.p.family)
    {
        route.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV6;
        //sal_memcpy(&route.destination.addr.ip6[0], &p_rt_node->key.p.u.prefix6.__u6_addr32[0], 4);
        //sal_memcpy(&route.destination.addr.ip6[1], &p_rt_node->key.p.u.prefix6[4], 4);
        //sal_memcpy(&route.destination.addr.ip6[2], &p_rt_node->key.p.u.prefix6[8], 4);
        //sal_memcpy(&route.destination.addr.ip6[3], &p_rt_node->key.p.u.prefix6[12], 4);
        
        sal_memcpy(&route.destination.addr.ip6, &p_rt_node->key.p.u.prefix6.s6_addr[0], sizeof(sai_ip6_t));
        
        //route.destination.addr.ip6[0] = htonl(route.destination.addr.ip6[0]);
        //route.destination.addr.ip6[1] = htonl(route.destination.addr.ip6[1]);
        //route.destination.addr.ip6[2] = htonl(route.destination.addr.ip6[2]);
        //route.destination.addr.ip6[3] = htonl(route.destination.addr.ip6[3]);
        
        sal_masklen2ip6(p_rt_node->key.p.prefixlen, &route.destination.mask.ip6);
    }

    HSRV_IF_ERROR_RETURN(route_api->remove_route(&route));
    return HSRV_E_NONE;
}

int32
hsrv_route_get_rt_entry(tbl_iproute_node_t* p_rt_node)
{
    tbl_fea_global_t* p_fea_global          = NULL;
    sai_route_api_t*  route_api             = NULL;
    sai_unicast_route_entry_t   route;
    sai_attribute_t             attr[1];
    uint32                      attr_count = 0;
    int32 ret = 0;

    /*1. debug out infor*/
    HSRV_IPUC_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_rt_node);

    /*2. process tbl*/
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ROUTE,(void**)&route_api));

    /*3. get route entry attribute */
    sal_memset(&route, 0, sizeof(route));
    p_fea_global = tbl_fea_global_get_fea_global();
    HSRV_PTR_CHECK(p_fea_global);

    route.vr_id = p_fea_global->router_id;
    if (AF_INET == p_rt_node->key.p.family)
    {
        route.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
        route.destination.addr.ip4 = sal_htonl(p_rt_node->key.p.u.prefix4.s_addr);
        IPV4_LEN_TO_MASK(route.destination.mask.ip4, p_rt_node->key.p.prefixlen);
    }
    else if (AF_INET6 == p_rt_node->key.p.family)
    {
        route.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV6;
        //sal_memcpy(&route.destination.addr.ip6[0], &p_rt_node->key.p.u.prefix6.__u6_addr32[0], 4);
        //sal_memcpy(&route.destination.addr.ip6[1], &p_rt_node->key.p.u.prefix6[4], 4);
        //sal_memcpy(&route.destination.addr.ip6[2], &p_rt_node->key.p.u.prefix6[8], 4);
        //sal_memcpy(&route.destination.addr.ip6[3], &p_rt_node->key.p.u.prefix6[12], 4);
        
        sal_memcpy(&route.destination.addr.ip6, &p_rt_node->key.p.u.prefix6.s6_addr[0], sizeof(sai_ip6_t));
        
        //route.destination.addr.ip6[0] = htonl(route.destination.addr.ip6[0]);
        //route.destination.addr.ip6[1] = htonl(route.destination.addr.ip6[1]);
        //route.destination.addr.ip6[2] = htonl(route.destination.addr.ip6[2]);
        //route.destination.addr.ip6[3] = htonl(route.destination.addr.ip6[3]);
        
        sal_masklen2ip6(p_rt_node->key.p.prefixlen, &route.destination.mask.ip6);
    }

    ret = route_api->get_route_attribute(&route, attr_count, attr);

    return ret;
}

int32
hsrv_route_update_ecmp_check_process(tbl_iproute_node_t* p_rt_node, uint32 act_add)
{
    int32 ret = PM_E_NONE;

    if (HRSV_ROUTE_ACT_ADD == act_add)
    {
        HSRV_IF_ERROR_RETURN(hsrv_route_create_rt_entry(p_rt_node));
    }
    else if (HRSV_ROUTE_ACT_DEL == act_add)
    {
        HSRV_IF_ERROR_RETURN(hsrv_route_remove_rt_entry(p_rt_node));
    } 
    else
    {
        return PM_E_NOT_SUPPORT;
    }

    return ret;
}

int32
hsrv_route_update_nexthop(tbl_iproute_node_t* p_rt_node)
{
    tbl_fea_global_t* p_fea_global          = NULL;
    sai_route_api_t*  route_api             = NULL;
    sai_unicast_route_entry_t   route;
    tbl_fea_nexthop_t * p_fea_nexthop;
    sai_attribute_t             attr[1];
    tbl_iproute_node_t *p_db_rt_node = NULL;

    sal_memset(attr, 0, sizeof(attr));
    
    p_db_rt_node = tbl_iproute_node_get_iproute_node(&p_rt_node->key);

    /*1. debug out infor*/
    HSRV_IPUC_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_rt_node);

    HSRV_IPUC_DEBUG("ip route update nexthop, route %x/%d,  \n"
        "    nexthop is %x, ifname %s \n"
        "    is_multipath %d, number is %d \n"
        "         %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x", 
        p_rt_node->key.p.u.prefix4.s_addr, p_rt_node->key.p.prefixlen,
        p_rt_node->nh_key.u.ipv4.s_addr, p_rt_node->nh_key.ifname,
        p_rt_node->multipath,p_rt_node->nh_group.nh_num, 
        p_rt_node->nh_group.nh[0].u.ipv4.s_addr, p_rt_node->nh_group.nh[1].u.ipv4.s_addr, 
        p_rt_node->nh_group.nh[2].u.ipv4.s_addr, p_rt_node->nh_group.nh[3].u.ipv4.s_addr, 
        p_rt_node->nh_group.nh[4].u.ipv4.s_addr, p_rt_node->nh_group.nh[5].u.ipv4.s_addr, 
        p_rt_node->nh_group.nh[6].u.ipv4.s_addr, p_rt_node->nh_group.nh[7].u.ipv4.s_addr, 
        p_rt_node->nh_group.nh[8].u.ipv4.s_addr, p_rt_node->nh_group.nh[9].u.ipv4.s_addr, 
        p_rt_node->nh_group.nh[10].u.ipv4.s_addr, p_rt_node->nh_group.nh[11].u.ipv4.s_addr, 
        p_rt_node->nh_group.nh[12].u.ipv4.s_addr, p_rt_node->nh_group.nh[13].u.ipv4.s_addr, 
        p_rt_node->nh_group.nh[14].u.ipv4.s_addr, p_rt_node->nh_group.nh[15].u.ipv4.s_addr);
    
    /*2. process tbl*/
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ROUTE,(void**)&route_api));

    /*3. get route entry attribute */
    sal_memset(&route, 0, sizeof(route));
    p_fea_global = tbl_fea_global_get_fea_global();
    HSRV_PTR_CHECK(p_fea_global);

    route.vr_id = p_fea_global->router_id;
    if (AF_INET == p_rt_node->key.p.family)
    {
        route.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
        route.destination.addr.ip4 = sal_htonl(p_rt_node->key.p.u.prefix4.s_addr);
        IPV4_LEN_TO_MASK(route.destination.mask.ip4, p_rt_node->key.p.prefixlen);
    }
    else if (AF_INET6 == p_rt_node->key.p.family)
    {
        route.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV6;
        //sal_memcpy(&route.destination.addr.ip6[0], &p_rt_node->key.p.u.prefix6.__u6_addr32[0], 4);
        //sal_memcpy(&route.destination.addr.ip6[1], &p_rt_node->key.p.u.prefix6[4], 4);
        //sal_memcpy(&route.destination.addr.ip6[2], &p_rt_node->key.p.u.prefix6[8], 4);
        //sal_memcpy(&route.destination.addr.ip6[3], &p_rt_node->key.p.u.prefix6[12], 4);
        
        sal_memcpy(&route.destination.addr.ip6, &p_rt_node->key.p.u.prefix6.s6_addr[0], sizeof(sai_ip6_t));
        
        //route.destination.addr.ip6[0] = htonl(route.destination.addr.ip6[0]);
        //route.destination.addr.ip6[1] = htonl(route.destination.addr.ip6[1]);
        //route.destination.addr.ip6[2] = htonl(route.destination.addr.ip6[2]);
        //route.destination.addr.ip6[3] = htonl(route.destination.addr.ip6[3]);
        
        sal_masklen2ip6(p_rt_node->key.p.prefixlen, &route.destination.mask.ip6);
    }

    /*upto cpu or remote route*/
    if(p_rt_node->nh_key.type == NH_TYPE_LOCAL)
    {
        attr[0].id = SAI_ROUTE_ATTR_PACKET_ACTION;
        attr[0].value.s32 = SAI_PACKET_ACTION_TRAP;
    }
    /*add by chenc for delete static route, then black hole route working for bug 45589*/
    else if (p_rt_node->nh_key.type == NH_TYPE_BLACKHOLE)
    {
        attr[0].id = SAI_ROUTE_ATTR_PACKET_ACTION;
        attr[0].value.s32 = SAI_PACKET_ACTION_DROP;
    }
    else
    {
        p_fea_nexthop = tbl_fea_nexthop_get_fea_nexthop(&p_rt_node->nh_key);
        HSRV_PTR_CHECK(p_fea_nexthop);
            
        /*add by chenc for static route replace black hole route working for bug 45589*/
        if (p_db_rt_node->nh_key.type == NH_TYPE_BLACKHOLE)
        {
            attr[0].id = SAI_ROUTE_ATTR_PACKET_ACTION;
            attr[0].value.s32 = SAI_PACKET_ACTION_FORWARD;

            HSRV_IF_ERROR_RETURN(route_api->set_route_attribute(&route, attr, p_fea_nexthop->nhid));
            return HSRV_E_NONE;
        }
        
        attr[0].id = SAI_ROUTE_ATTR_NEXT_HOP_ID;
        attr[0].value.oid = p_fea_nexthop->nhid;
    }
    
    HSRV_IF_ERROR_RETURN(route_api->set_route_attribute(&route, attr, 0));

    return PM_E_NONE;
}


int32
hsrv_route_update_nexthop_group(tbl_iproute_node_t* p_rt_node)
{
    tbl_fea_global_t* p_fea_global          = NULL;
    sai_route_api_t*  route_api             = NULL;
    sai_unicast_route_entry_t   route;
    tbl_nexthop_group_t * p_nhg;
    sai_attribute_t             attr[1];
    tbl_iproute_node_t *p_db_rt_node = NULL;
    
    p_db_rt_node = tbl_iproute_node_get_iproute_node(&p_rt_node->key);

    /*1. debug out infor*/
    HSRV_IPUC_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_rt_node);

    HSRV_IPUC_DEBUG("ip route update nexthopgroup, route %x/%d,  \n"
        "    nexthop is %x, ifname %s \n"
        "    is_multipath %d, number is %d \n"
        "        IP %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\n",
        p_rt_node->key.p.u.prefix4.s_addr, p_rt_node->key.p.prefixlen,
        p_rt_node->nh_key.u.ipv4.s_addr, p_rt_node->nh_key.ifname,
        p_rt_node->multipath,p_rt_node->nh_group.nh_num, 
        p_rt_node->nh_group.nh[0].u.ipv4.s_addr, p_rt_node->nh_group.nh[1].u.ipv4.s_addr, 
        p_rt_node->nh_group.nh[2].u.ipv4.s_addr, p_rt_node->nh_group.nh[3].u.ipv4.s_addr, 
        p_rt_node->nh_group.nh[4].u.ipv4.s_addr, p_rt_node->nh_group.nh[5].u.ipv4.s_addr, 
        p_rt_node->nh_group.nh[6].u.ipv4.s_addr, p_rt_node->nh_group.nh[7].u.ipv4.s_addr, 
        p_rt_node->nh_group.nh[8].u.ipv4.s_addr, p_rt_node->nh_group.nh[9].u.ipv4.s_addr, 
        p_rt_node->nh_group.nh[10].u.ipv4.s_addr, p_rt_node->nh_group.nh[11].u.ipv4.s_addr, 
        p_rt_node->nh_group.nh[12].u.ipv4.s_addr, p_rt_node->nh_group.nh[13].u.ipv4.s_addr, 
        p_rt_node->nh_group.nh[14].u.ipv4.s_addr, p_rt_node->nh_group.nh[15].u.ipv4.s_addr
        );

    if(p_rt_node->nh_group.nh_num == 0)
    {
        /*Do nothing*/
        return PM_E_NONE;
    }

    /*2. process tbl*/
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ROUTE,(void**)&route_api));

    /*3. get route entry attribute */
    sal_memset(&route, 0, sizeof(route));
    p_fea_global = tbl_fea_global_get_fea_global();
    HSRV_PTR_CHECK(p_fea_global);

    route.vr_id = p_fea_global->router_id;
    if (AF_INET == p_rt_node->key.p.family)
    {
        route.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
        route.destination.addr.ip4 = sal_htonl(p_rt_node->key.p.u.prefix4.s_addr);
        IPV4_LEN_TO_MASK(route.destination.mask.ip4, p_rt_node->key.p.prefixlen);
    }
    else if (AF_INET6 == p_rt_node->key.p.family)
    {
        route.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV6;
        //sal_memcpy(&route.destination.addr.ip6[0], &p_rt_node->key.p.u.prefix6.__u6_addr32[0], 4);
        //sal_memcpy(&route.destination.addr.ip6[1], &p_rt_node->key.p.u.prefix6[4], 4);
        //sal_memcpy(&route.destination.addr.ip6[2], &p_rt_node->key.p.u.prefix6[8], 4);
        //sal_memcpy(&route.destination.addr.ip6[3], &p_rt_node->key.p.u.prefix6[12], 4);
        
        sal_memcpy(&route.destination.addr.ip6, &p_rt_node->key.p.u.prefix6.s6_addr[0], sizeof(sai_ip6_t));
        
        //route.destination.addr.ip6[0] = htonl(route.destination.addr.ip6[0]);
        //route.destination.addr.ip6[1] = htonl(route.destination.addr.ip6[1]);
        //route.destination.addr.ip6[2] = htonl(route.destination.addr.ip6[2]);
        //route.destination.addr.ip6[3] = htonl(route.destination.addr.ip6[3]);
        
        sal_masklen2ip6(p_rt_node->key.p.prefixlen, &route.destination.mask.ip6);
    }

    p_nhg = tbl_nexthop_group_get_nexthop_group((tbl_nexthop_group_key_t*)(&p_rt_node->nh_group));
    HSRV_PTR_CHECK(p_nhg);
        
    /*add by chenc for ecmp route replace black hole route working for bug 45589*/
    if (p_db_rt_node->nh_key.type == NH_TYPE_BLACKHOLE)
    {
        attr[0].id = SAI_ROUTE_ATTR_PACKET_ACTION;
        attr[0].value.s32 = SAI_PACKET_ACTION_FORWARD;

        HSRV_IF_ERROR_RETURN(route_api->set_route_attribute(&route, attr, p_nhg->nhg_oid));
        return HSRV_E_NONE;
    }
    
    attr[0].id = SAI_ROUTE_ATTR_NEXT_HOP_ID;
    attr[0].value.oid = p_nhg->nhg_oid;

    //log_sys(M_MOD_HSRV, E_ERROR, "update_nexthop_group oid %"PRIu64, p_nhg->nhg_oid);
        
    HSRV_IF_ERROR_RETURN(route_api->set_route_attribute(&route, attr, 0));

    return PM_E_NONE;
}


int32
hsrv_route_rt_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id,
      void *p_tbl, void *p_ds)
{
    tbl_iproute_node_t* p_rt_node = NULL;
    tbl_iproute_node_t *p_db_rt_node = NULL;

    HSRV_IPUC_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_tbl);
    p_rt_node = (tbl_iproute_node_t*)p_tbl;
    

    p_db_rt_node = tbl_iproute_node_get_iproute_node(&p_rt_node->key);

    switch (oper)
    {
    case CDB_OPER_ADD:
        HSRV_IF_ERROR_RETURN(hsrv_route_update_ecmp_check_process(p_rt_node, HRSV_ROUTE_ACT_ADD));
        break;

    case CDB_OPER_DEL:
        HSRV_PTR_CHECK(p_db_rt_node);
        HSRV_IF_ERROR_RETURN(hsrv_route_update_ecmp_check_process(p_db_rt_node, HRSV_ROUTE_ACT_DEL));
        break;

    case CDB_OPER_SET:
        HSRV_PTR_CHECK(p_db_rt_node);
        switch (field_id)
        {
        case TBL_IPROUTE_NODE_FLD_NH_GROUP:
            HSRV_IF_ERROR_RETURN(hsrv_route_update_nexthop_group(p_rt_node));
            break;

        case TBL_IPROUTE_NODE_FLD_NH_KEY:
            /*it means the route nexthop is updated from multimember to one member*/
            HSRV_IF_ERROR_RETURN(hsrv_route_update_nexthop(p_rt_node));
            break;
            
        default:
            return PM_E_INVALID_PARAM;
        }
        break;

    default:
        return HSRV_E_INVALID_PARAM;
    }

    return PM_E_NONE;
}

int32
hsrv_route_rt_after_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id,
      void *p_tbl, void *p_ds)
{
    #ifndef OFPRODUCT
    return 0;
    #endif
    tbl_iproute_node_t* p_rt_node = NULL;
    tbl_iproute_node_t *p_db_rt_node = NULL;
    tbl_iproute_node_t db_rt_node_del; 

    HSRV_IPUC_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_tbl);
    p_rt_node = (tbl_iproute_node_t*)p_tbl;
    
    p_db_rt_node = tbl_iproute_node_get_iproute_node(&p_rt_node->key);
    if(CDB_OPER_ADD == oper)
    {
        hsrv_tunneldb_cdb_route_oper_add_function(p_db_rt_node);
    }
    else if(CDB_OPER_SET == oper)
    {
        if(field_id == TBL_IPROUTE_NODE_FLD_MULTIPATH)
        {
            if(p_db_rt_node->multipath == 0)
            {
                hsrv_tunneldb_cdb_route_oper_add_function(p_db_rt_node);
            }
        }
        
        if(field_id == TBL_IPROUTE_NODE_FLD_NH_GROUP)
        {
            if(p_db_rt_node->multipath != 0)
            {
                hsrv_tunneldb_cdb_route_oper_add_function(p_db_rt_node);      
            }
        }
        if(field_id == TBL_IPROUTE_NODE_FLD_NH_KEY)
        {
            /* modified by liwh for bug 49019, 2019-03-21 */
            if (0 == p_db_rt_node->multipath)
            /* liwh end */
            {
                hsrv_tunneldb_cdb_route_oper_add_function(p_db_rt_node);
            }
        }
    }
    else if(CDB_OPER_DEL == oper)
    {
        sal_memset(&db_rt_node_del, 0x0, sizeof(tbl_iproute_node_t));
        sal_memcpy(&db_rt_node_del.key, &p_rt_node->key, sizeof(tbl_iproute_node_key_t));
        hsrv_tunneldb_cdb_route_oper_del_function(&db_rt_node_del);
    }

    return PM_E_NONE;
}

