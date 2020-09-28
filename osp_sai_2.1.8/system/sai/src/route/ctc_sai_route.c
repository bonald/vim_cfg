#include <sal.h>
#include <sai.h>
#include <ctc_sai_common.h>
#include <ctc_sai_common_attr.h>
#include <ctc_sai_sai.h>
#include "ctc_sai_route.h"
#include <ctc_sai_nexthop_group.h>
#include <ctc_vector.h>
#include <ctc_sai_nexthop.h>
#include <ctc_sai_nexthop_group.h>
#include <ctc_sai_notifier_call.h>
#include <ctc_api.h>
#include <ctc_sai_debug.h>

#define ROUTE_HASH_B	128

static ctc_sai_attr_entry_info_t g_sai_attr_entries[] = {
    {
        .id     = SAI_ROUTE_ATTR_PACKET_ACTION,
        .type   = SAI_ATTR_FALG_WRITE| SAI_ATTR_FALG_READ | SAI_ATTR_FLAG_DEFAULT,
        .default_value.s32 = SAI_PACKET_ACTION_FORWARD,
    },
    {
        .id     = SAI_ROUTE_ATTR_TRAP_PRIORITY,
        .type   = SAI_ATTR_FALG_WRITE| SAI_ATTR_FALG_READ | SAI_ATTR_FLAG_DEFAULT,
        .default_value.u8 = 0,
    },
    {
        .id     = SAI_ROUTE_ATTR_NEXT_HOP_ID,
        .type   = SAI_ATTR_FALG_WRITE| SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_ROUTE_ATTR_RIF_ID,
        .type   = SAI_ATTR_FALG_WRITE| SAI_ATTR_FALG_READ,
    },
    {/* added by taocy for NAT, 2019-06-03*/
        .id     = SAI_ROUTE_ATTR_L3_IF_TYPE,
        .type   = SAI_ATTR_FALG_WRITE| SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_ROUTE_ATTR_CATEGORY_ID,
        .type   = SAI_ATTR_FALG_WRITE| SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_ATTR_ID_END,
        .type   = 0,
    },
};

#define ________SAI_SAI_INNER_API_FUNC

static int32
ctc_sai_route_add_default_ipuc_entry()
{
    uint32 nhid;
    ctc_ipuc_param_t ipuc_info;
    int32 ret = SAI_STATUS_SUCCESS;

    nhid = NEIGHBOR_ACTION_TRAP_NHID;

    /*when overlay is enabled, we add vrf = 0 default route to tcam because we are in sdk default route tcam mode*/
    sal_memset(&ipuc_info, 0, sizeof(ctc_ipuc_param_t));
    ipuc_info.ip_ver = CTC_IP_VER_4;
    ipuc_info.vrf_id = 0;
    ipuc_info.nh_id = nhid;     /* modified by ychen in 2014-01-21 for bug 26942 */
//    ipuc_info.is_ecmp_nh = 0;
    ipuc_info.route_flag = CTC_IPUC_FLAG_RPF_CHECK;
    ret = ctc_ipuc_add(&ipuc_info);

    return ret;
}

static int32
ctc_sai_route_add_default_route()
{
    uint32 nhid;
    uint32 ipv6_en;
    ctc_ipuc_param_t ipuc_info;
    int32 ret = SAI_STATUS_SUCCESS;

    /*1. get values and dump*/

    /*2.set to SDK*/
    nhid = NEIGHBOR_ACTION_TRAP_NHID;
    ipv6_en = TRUE;
    CTC_SAI_DEBUG("  default route nhid=%d.", nhid);

    /*when overlay is enabled, we add vrf = 0 default route to tcam because we are in sdk default route tcam mode*/
    sal_memset(&ipuc_info, 0, sizeof(ctc_ipuc_param_t));
    ipuc_info.ip_ver = CTC_IP_VER_4;
    ipuc_info.vrf_id = 0;
    ipuc_info.nh_id = nhid;     /* modified by ychen in 2014-01-21 for bug 26942 */
//    ipuc_info.is_ecmp_nh = 0;
/* modified by chenc for bug 50246 */
//#ifndef OFPRODUCT
    ipuc_info.route_flag = CTC_IPUC_FLAG_RPF_CHECK;
//#endif
    ret = ctc_ipuc_add(&ipuc_info);

    /* Modified by wangjj for init l3 multicast default entry, 2016-08-03 */
    ctc_ipmc_add_default_entry(CTC_IP_VER_4, CTC_IPMC_DEFAULT_ACTION_TO_CPU);

    if (ipv6_en)
    {
        ctc_ipuc_param_t ipuc_info;
        ctc_ipmc_force_route_t mcv6_route;

        #if 0
        sal_memset(&ipuc_info, 0, sizeof(ctc_ipuc_param_t));
        ipuc_info.ip_ver = CTC_IP_VER_6;
        ipuc_info.vrf_id = 0;
        ipuc_info.nh_id = nhid;  /*modified by ychen in 2014-01-21 for bug 26942*/
        ipuc_info.is_ecmp_nh = 0;
        ipuc_info.route_flag = CTC_IPUC_FLAG_RPF_CHECK;
        ret = ctc_ipuc_add(&ipuc_info);
        #endif
        
        /*set martion address*/
        sal_memset(&ipuc_info, 0, sizeof(ctc_ipuc_param_t));
        ipuc_info.nh_id = CTC_NH_RESERVED_NHID_FOR_DROP;
 //       ipuc_info.is_ecmp_nh = 0;

        ipuc_info.ip_ver = CTC_IP_VER_6;
        /*a) drop ::/128*/
        ipuc_info.masklen = 128;
        ret = ctc_ipuc_add(&ipuc_info);

        /*b) drop ::1/128*/
        /*modified by ychen in 2012-05-29 for version 92006 when merge sdk code,
        use SYS_IP_ADDRESS_SORT instead. use ipv6[3] instead of ipv6[0]*/
        ipuc_info.ip.ipv6[3] = 1;
        ipuc_info.masklen = 128;
        ret = ctc_ipuc_add(&ipuc_info);

        /*c). set mcast address in order to send ndp, ospf packet to cpu via force bridge when packet
        is multicast
        #define ALLNODE                            "ff02::1"
        #define ALLROUTER                        "ff02::2"
        #define SOLICITED_PREFIX            "ff02::1:ff00:0"*/
        sal_memset(&mcv6_route, 0, sizeof(ctc_ipmc_force_route_t));
        mcv6_route.ip_version = CTC_IP_VER_6;
        mcv6_route.force_bridge_en = 1;
        //mcv6_route.force_ucast_en = 1;

        mcv6_route.ipaddr0_valid = 1;
        mcv6_route.ip_addr0.ipv6[0] = 0xFF020000;
        mcv6_route.ip_addr0.ipv6[1] = 0x00000000;
        mcv6_route.ip_addr0.ipv6[2] = 0x00000000;
        mcv6_route.ip_addr0.ipv6[3] = 0x00000000;
        mcv6_route.addr0_mask = 120;
        //ret = ctc_ipmc_set_mcast_force_route(&mcv6_route);

        mcv6_route.ipaddr1_valid = 1;
        mcv6_route.ip_addr1.ipv6[0] = 0xFF020000;
        mcv6_route.ip_addr1.ipv6[1] = 0x00000000;
        mcv6_route.ip_addr1.ipv6[2] = 0x00000001;
        mcv6_route.ip_addr1.ipv6[3] = 0xFF000000;
        mcv6_route.addr1_mask = 104;
        ret = ctc_ipmc_set_mcast_force_route(&mcv6_route);
    }

    (void)ret;
    return SAI_STATUS_SUCCESS;
}

static int32
ctc_sai_route_add_ipda_martion()
{
    ctc_ipuc_param_t ipuc_info;
    ctc_ipuc_global_property_t rt_ctl;
    int32 ret = CTC_E_NONE;

    /*1. init*/
    sal_memset(&ipuc_info, 0, sizeof(ctc_ipuc_param_t));
    ipuc_info.nh_id = NEIGHBOR_ACTION_DROP_NHID;
//    ipuc_info.is_ecmp_nh = 0;

    ipuc_info.ip_ver = CTC_IP_VER_4;
    /*2. for ipv4 default route*/
    /*a) drop 0.x.x.x/8 */
    ipuc_info.ip.ipv4 = 0;
    ipuc_info.masklen = 8;
    ret = ctc_ipuc_add(&ipuc_info);

    /*b) drop 127.x.x.x/8*/
    ipuc_info.ip.ipv4 = 0x7f000000;
    ipuc_info.masklen = 8;
    ret = ctc_ipuc_add(&ipuc_info);

    /*c) 255.255.255.255/32*/
    ipuc_info.ip.ipv4 = 0xffffffff;
    ipuc_info.masklen = 32;
    ret = ctc_ipuc_add(&ipuc_info);

    /*3. set martion address check enable*/
    sal_memset(&rt_ctl, 0, sizeof(ctc_ipuc_global_property_t));

    SAI_SET_FLAG(rt_ctl.valid_flag, CTC_IP_GLB_PROP_V4_MARTIAN_CHECK_EN);
    rt_ctl.v4_martian_check_en = 1;

    SAI_SET_FLAG(rt_ctl.valid_flag, CTC_IP_GLB_PROP_V6_MARTIAN_CHECK_EN);
    rt_ctl.v6_martian_check_en = 1;

    ret = ctc_ipuc_set_global_property(&rt_ctl);

    (void)ret;
    return SAI_STATUS_SUCCESS;
}

static int32
ctc_sai_route_multicast_pdu_init(void)
{
    ctc_ipmc_force_route_t mcv4_route;
    int32 ret;

    /*for ipv4 pdu packets*/
    sal_memset(&mcv4_route, 0, sizeof(ctc_ipmc_force_route_t));
    mcv4_route.ip_version = CTC_IP_VER_4;
    mcv4_route.force_bridge_en = 1;    
    
   /*modified by jiangz for bug16559, 2011-10-08, for merge sdk*/  
    mcv4_route.ipaddr0_valid = 1;    
    mcv4_route.ip_addr0.ipv4 = 0xE0000000;
    mcv4_route.addr0_mask = 24;
    ret = ctc_ipmc_set_mcast_force_route(&mcv4_route);

   /*modified by jiangz for bug16559, 2011-10-08, for merge sdk, for invalid the second entry*/   
    mcv4_route.ipaddr1_valid = 1;
    mcv4_route.ip_addr1.ipv4 = 0x0;
    mcv4_route.addr1_mask = 32;
    ret = ctc_ipmc_set_mcast_force_route(&mcv4_route);

    (void)ret;
    return SAI_STATUS_SUCCESS;    
}

sai_status_t
ctc_sai_route_db_init()
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    
    p_sai_glb->route_max_count = CTC_SAI_ROUTE_MAX_ENTRY;
        
    /* add default route for ipda martion address */
    ctc_sai_route_add_ipda_martion();
    /* add default route for syn with system before */
    ctc_sai_route_add_default_route();
    /* add ipda range to force multicast pdu packet to cpu*/
    ctc_sai_route_multicast_pdu_init();

    return SAI_STATUS_SUCCESS;
}

void
__route_entry_to_ipuc_param(stbl_route_t *p_sdb_route, ctc_ipuc_param_t* pipuc_info)
{
    uint32_t    mask_len    = 0;
    stbl_nexthop_group_t *p_sdb_nhgroup = NULL;
    
    switch (p_sdb_route->action)
    {
    case SAI_PACKET_ACTION_DROP:
        pipuc_info->nh_id = NEIGHBOR_ACTION_DROP_NHID;
        break;

    case SAI_PACKET_ACTION_FORWARD:
        pipuc_info->nh_id = CTC_SAI_OBJECT_INDEX_GET(p_sdb_route->nh_oid);
        CTC_SET_FLAG(pipuc_info->route_flag, CTC_IPUC_FLAG_TTL_CHECK);
        CTC_SET_FLAG(pipuc_info->route_flag, CTC_IPUC_FLAG_ICMP_CHECK);
        CTC_SET_FLAG(pipuc_info->route_flag, CTC_IPUC_FLAG_RPF_CHECK);
        break;

    case SAI_PACKET_ACTION_TRAP:
        pipuc_info->nh_id = NEIGHBOR_ACTION_TRAP_NHID;
        pipuc_info->l3_inf = (uint16)CTC_SAI_OBJECT_INDEX_GET(p_sdb_route->rif_oid);
        CTC_SET_FLAG(pipuc_info->route_flag, CTC_IPUC_FLAG_TTL_CHECK);
        CTC_SET_FLAG(pipuc_info->route_flag, CTC_IPUC_FLAG_CONNECT);
        CTC_SET_FLAG(pipuc_info->route_flag, CTC_IPUC_FLAG_RPF_CHECK);

        break;

    default:
        break;
    }
    
    if (SAI_IP_ADDR_FAMILY_IPV4 == p_sdb_route->key.destination.addr_family)
    {
        pipuc_info->ip_ver = CTC_IP_VER_4;
        sal_memcpy(&pipuc_info->ip.ipv4,
            &p_sdb_route->key.destination.addr.ip4,
            sizeof(sai_ip4_t));

        IPV4_MASK_TO_LEN(p_sdb_route->key.destination.mask.ip4,mask_len);
        pipuc_info->masklen = mask_len;
        if (SAI_PACKET_ACTION_FORWARD != p_sdb_route->action && CTC_IPV4_ADDR_LEN_IN_BIT == pipuc_info->masklen)
        {
            SAI_SET_FLAG(pipuc_info->route_flag, CTC_IPUC_FLAG_CPU);
            pipuc_info->nh_id = NEIGHBOR_ACTION_DROP_NHID;
            CTC_UNSET_FLAG(pipuc_info->route_flag, CTC_IPUC_FLAG_CONNECT);
            CTC_UNSET_FLAG(pipuc_info->route_flag, CTC_IPUC_FLAG_RPF_CHECK);
        }

        /* begin added by taocy for NAT, 20190603. just support IPv4*/
#ifdef TSINGMA
#ifndef OFPRODUCT
        if (SAI_ROUTER_INTERFACE_L3_TYPE_INTERNAL == p_sdb_route->l3if_type)
        {
            SAI_SET_FLAG(pipuc_info->route_flag, CTC_IPUC_FLAG_CANCEL_NAT);
        }

        if (p_sdb_route->category_id < 255)
        {
            pipuc_info->cid = p_sdb_route->category_id;
        }
#endif
#endif
        /* end added by taocy for NAT, 20190603*/
    }
    else
    {
        ipv6_addr_t ipv6 = {0};
        
        pipuc_info->ip_ver = CTC_IP_VER_6;
        sal_memcpy(pipuc_info->ip.ipv6,
            p_sdb_route->key.destination.addr.ip6,
            sizeof(sai_ip6_t));

        /*for ipv6 order switch*/
        pipuc_info->ip.ipv6[0] = htonl(pipuc_info->ip.ipv6[0]);
        pipuc_info->ip.ipv6[1] = htonl(pipuc_info->ip.ipv6[1]);
        pipuc_info->ip.ipv6[2] = htonl(pipuc_info->ip.ipv6[2]);
        pipuc_info->ip.ipv6[3] = htonl(pipuc_info->ip.ipv6[3]);

        /*for mask to len*/
        sal_memcpy(&ipv6,
            p_sdb_route->key.destination.mask.ip6,
            sizeof(sai_ip6_t));
            
        IPV6_MASK_TO_LEN(ipv6,mask_len);
        pipuc_info->masklen = mask_len;

        /*ipv6 address 128 route should be drop*/
        if (SAI_PACKET_ACTION_FORWARD != p_sdb_route->action && CTC_IPV6_ADDR_LEN_IN_BIT == pipuc_info->masklen)
        {
            SAI_SET_FLAG(pipuc_info->route_flag, CTC_IPUC_FLAG_CPU);
            pipuc_info->nh_id = NEIGHBOR_ACTION_DROP_NHID;
            CTC_UNSET_FLAG(pipuc_info->route_flag, CTC_IPUC_FLAG_CONNECT);
            CTC_UNSET_FLAG(pipuc_info->route_flag, CTC_IPUC_FLAG_RPF_CHECK);

            /* support nd modified by liwh for bug 47547, 2018-07-17 */
            SAI_SET_FLAG(pipuc_info->route_flag, CTC_IPUC_FLAG_PROTOCOL_ENTRY);
            SAI_SET_FLAG(pipuc_info->route_flag, CTC_IPUC_FLAG_SELF_ADDRESS);

            pipuc_info->l3_inf = 0;
            /* liwh end */
        }
    }

    pipuc_info->vrf_id = (uint16)CTC_SAI_OBJECT_INDEX_GET(p_sdb_route->key.vr_id);

    if ((SAI_OBJECT_TYPE_NEXT_HOP_GROUP == CTC_SAI_OBJECT_TYPE_GET(p_sdb_route->nh_oid)) 
            && SAI_PACKET_ACTION_FORWARD == p_sdb_route->action)
    {
        CTC_UNSET_FLAG(pipuc_info->route_flag, CTC_IPUC_FLAG_ICMP_CHECK);
        p_sdb_nhgroup = ctc_nexthop_group_get_by_oid(p_sdb_route->nh_oid);

        /* modified by liwh for bug 47038, 2018-04-27 */
        if ((p_sdb_nhgroup) && (p_sdb_nhgroup->nh_list.obj_list))
        /* liwh end */
        {
            if (CTC_SAI_IP_MAX_RPF_IF < p_sdb_nhgroup->nh_list.obj_list->count)
            {
                CTC_UNSET_FLAG(pipuc_info->route_flag, CTC_IPUC_FLAG_RPF_CHECK);
            }
            else if (CTC_SAI_IP_MAX_RPF_IF == p_sdb_nhgroup->nh_list.obj_list->count)
            {
                CTC_SAI_DEBUG("URPF will be disabled when nh_cnt > 16");
            }
        }
        /* modified by liwh for bug 47038, 2018-04-27 */
        else
        {
            if (!p_sdb_nhgroup)
            {       
                CTC_SAI_DEBUG("__route_entry_to_ipuc_param p_sdb_nhgroup is null\n");
            }
            else if (!p_sdb_nhgroup->nh_list.obj_list)
            {
                CTC_SAI_DEBUG("__route_entry_to_ipuc_param p_sdb_nhgroup->nh_list.obj_list is null\n");
            }
        }
        /* liwh end */
    }    
}

sai_status_t
__route_update_ipuc_to_sdk(stbl_route_t *p_sdb_route)
{
    ctc_ipuc_param_t  ipuc_info;
    int32_t     ret = 0;

    sal_memset(&ipuc_info, 0, sizeof(ipuc_info));
    __route_entry_to_ipuc_param(p_sdb_route, &ipuc_info);

    ret = ctc_ipuc_add(&ipuc_info);
    if (CTC_E_IPUC_INVALID_ROUTE_FLAG == ret)
    {
        if (CTC_FLAG_ISSET(ipuc_info.route_flag, CTC_IPUC_FLAG_RPF_CHECK))
        {
            CTC_UNSET_FLAG(ipuc_info.route_flag, CTC_IPUC_FLAG_RPF_CHECK);
            ret = ctc_ipuc_add(&ipuc_info);
        }
        /* check if the entry is same with neighbor */
    }
    /* modified by wangjj for fix bug 43512, when host entry full, 2017-03-28 */
    if (CTC_E_NO_RESOURCE == ret)
    {
        CTC_SAI_DEBUG("Resource full, add route failed");
        ret = CTC_E_NONE;
    }
#ifdef GREATBELT /*Greatbelt sdk 3.3 not support neighbor with ipuc/32, case will cause CTC_E_ENTRY_EXIST, temp solution*/
    if( ret != CTC_E_NONE)
    {
        ret = CTC_E_NONE;
    }
#endif

    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
__route_remove_ipuc_to_sdk(stbl_route_t *p_sdb_route)
{
    int32_t     ret = 0;
    ctc_ipuc_param_t  ipuc_info;

    sal_memset(&ipuc_info, 0, sizeof(ipuc_info));

    __route_entry_to_ipuc_param(p_sdb_route, &ipuc_info);

    ret = ctc_ipuc_remove(&ipuc_info);
#ifdef GREATBELT /*Greatbelt sdk 3.3 not support neighbor with ipuc/32, case will cause CTC_E_ENTRY_EXIST, temp solution*/
    if( ret != CTC_E_NONE)
    {
        ret = CTC_E_NONE;
    }
#endif
    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
_route_check_nexthop_exist(sai_object_id_t nexthop_id)
{
    stbl_nexthop_t                         *p_sdb_nh        = NULL;
    stbl_nexthop_group_t                   *p_sdb_nhgroup   = NULL;

    if (SAI_OBJECT_TYPE_NEXT_HOP == CTC_SAI_OBJECT_TYPE_GET(nexthop_id))
    {
        p_sdb_nh = ctc_nexthop_get_by_oid(nexthop_id);
        if(NULL == p_sdb_nh)
        {
            return SAI_STATUS_ITEM_NOT_FOUND;
        }
    }
    else if (SAI_OBJECT_TYPE_NEXT_HOP_GROUP == CTC_SAI_OBJECT_TYPE_GET(nexthop_id))
    {
        p_sdb_nhgroup = ctc_nexthop_group_get_by_oid(nexthop_id);
        if (NULL == p_sdb_nhgroup)
        {
            return SAI_STATUS_ITEM_NOT_FOUND;
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_route_db_remove_entry(stbl_route_t *p_sdb_route)
{
    sai_status_t ret = SAI_STATUS_SUCCESS;
    ctc_ipuc_param_t  ipuc_info;
    sai_unicast_route_entry_t default_entry;
    
    sal_memset(&ipuc_info, 0, sizeof(ipuc_info));
    sal_memset(&default_entry, 0, sizeof(default_entry));
    
    default_entry.vr_id = p_sdb_route->key.vr_id;
    __route_entry_to_ipuc_param(p_sdb_route, &ipuc_info);

    /*for nd priority is higher than route by chenc, 2019-03-19*/
    sai_ip6_t ip6_all_f;
    sal_memset(&ip6_all_f, 0xffff, sizeof(sai_ip6_t));
    uint32 get_ipv6_neighbor = FALSE;
    
    if (p_sdb_route->key.destination.addr_family == SAI_IP_ADDR_FAMILY_IPV6 
        && 0 == sal_memcmp(&p_sdb_route->key.destination.mask.ip6, &ip6_all_f, sizeof(sai_ip6_t)))
    {
        stbl_neighbor_master_t *p_master = stbl_neighbor_get_master();
        ctclib_slistnode_t *listnode = NULL;
        ctclib_slistnode_t *next = NULL;
        stbl_neighbor_t *p_db_neigh = NULL;

        if (NULL == p_master)
        {
            return SDB_E_NONE;
        }

        CTCLIB_SLIST_LOOP_DEL(p_master->neigh_list, p_db_neigh, listnode, next)
        {
            if (p_db_neigh->key.ip_address.addr_family == SAI_IP_ADDR_FAMILY_IPV6)
            {
                if (0 == sal_memcmp(&p_db_neigh->key.ip_address.addr.ip6, &p_sdb_route->key.destination.addr.ip6, sizeof(sai_ip6_t)))
                {
                    get_ipv6_neighbor = TRUE;
                    break;
                }
            }
        }
    }

    if (!get_ipv6_neighbor)
    {
        ret = ctc_ipuc_remove(&ipuc_info);
    }
    /*ended by chenc*/
//#if GREATBELT /*SDK3.3 don't support neighbor&ipuc, this case will cause CTC_E_ENTRY_NOT_EXIST*/
    if(ret != CTC_E_NONE)
    {
        ret = CTC_E_NONE;
    }
//#endif
    if(ret != 0)
    {
        return ctc_sai_get_error_from_sdk_error(ret);
    }

    if (0 == sal_memcmp(&default_entry, &p_sdb_route->key, sizeof(default_entry)))
    {
        ctc_sai_route_add_default_ipuc_entry();
    }

    stbl_route_del_route(&p_sdb_route->key);

    return SAI_STATUS_SUCCESS;
}

#define ________SAI_SAI_DEBUG_FUNC
sai_status_t
ctc_sai_set_route_attribute_debug_param(
    _In_ const sai_unicast_route_entry_t* unicast_route_entry,
    _In_ const sai_attribute_t *attr)
{
    CTC_SAI_DEBUG("in:vr_id %u", unicast_route_entry->vr_id);
    switch(attr->id)
    {
    case SAI_ROUTE_ATTR_PACKET_ACTION:
        CTC_SAI_DEBUG("in:SAI_ROUTE_ATTR_PACKET_ACTION %d", attr->value.s32);
        break;

    case SAI_ROUTE_ATTR_NEXT_HOP_ID:
        CTC_SAI_DEBUG("in:SAI_ROUTE_ATTR_NEXT_HOP_ID 0x%llx", attr->value.oid);
        break;
    }
    return SAI_STATUS_SUCCESS;
}
sai_status_t
ctc_sai_get_route_attribute_debug_param(
    _In_ const sai_unicast_route_entry_t* unicast_route_entry,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    uint32_t                    attr_idx    = 0;
    const sai_attribute_t *attr = NULL;

    CTC_SAI_DEBUG("in:vr_id 0x%llx attr_count %u", unicast_route_entry->vr_id, attr_count);
    for (attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = attr_list + attr_idx;
        switch (attr->id)
        {
        case SAI_ROUTE_ATTR_PACKET_ACTION:
            CTC_SAI_DEBUG("in:SAI_ROUTE_ATTR_PACKET_ACTION %d", attr->value.s32);
            break;
        case SAI_ROUTE_ATTR_TRAP_PRIORITY:
            CTC_SAI_DEBUG("in:SAI_ROUTE_ATTR_PACKET_ACTION %u", attr->value.u8);
            break;
        case SAI_ROUTE_ATTR_NEXT_HOP_ID:
            CTC_SAI_DEBUG("in:SAI_ROUTE_ATTR_PACKET_ACTION 0x%llx", attr->value.oid);
            break;
        }
    }

    return SAI_STATUS_SUCCESS;
}
#define ________SAI_SAI_API_FUNC
/*
* Routine Description:
*    Create Route
*
* Arguments:
*    [in] unicast_route_entry - route entry
*    [in] attr_count - number of attributes
*    [in] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*
* Note: IP prefix/mask expected in Network Byte Order.
*
*/
sai_status_t
ctc_sai_create_route(
    _In_ const sai_unicast_route_entry_t* unicast_route_entry,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list)
{
    sai_status_t                 ret = SAI_STATUS_SUCCESS;
    ctc_sai_attr_entry_list_t    *pattr_entry_list = NULL;
    stbl_route_t                 *p_sdb_route = NULL;
    stbl_vr_t                    *p_sdb_vr = NULL;
    stbl_route_t                 sdb_route;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_PTR_VALID_CHECK(attr_list);
    CTC_SAI_PTR_VALID_CHECK(unicast_route_entry);

    ret = ctc_sai_attr_alloc_attr_entry_list(g_sai_attr_entries,
                                   attr_list,
                                   attr_count,
                                   &pattr_entry_list);

    if(SAI_STATUS_SUCCESS != ret)
    {
        return ret;
    }
    sal_memset(&sdb_route, 0, sizeof(sdb_route));
    sal_memcpy(&sdb_route.key, unicast_route_entry, sizeof(sai_unicast_route_entry_t));
    
    /* 1. check route exist */
    p_sdb_route = stbl_route_get_route(&sdb_route.key);
    if(NULL != p_sdb_route)
    {
        ret = SAI_STATUS_ITEM_ALREADY_EXISTS;
        goto out;
    }

    p_sdb_vr = ctc_vr_get_by_oid(unicast_route_entry->vr_id);
    if(NULL == p_sdb_vr)
    {
        ret = SAI_STATUS_INVALID_OBJECT_ID;
        goto out;
    }

    /* 3. add parameter */
    if (pattr_entry_list[SAI_ROUTE_ATTR_NEXT_HOP_ID].valid)
    {
        ret = _route_check_nexthop_exist(pattr_entry_list[SAI_ROUTE_ATTR_NEXT_HOP_ID].value.oid);

        if(SAI_STATUS_SUCCESS != ret)
        {
            ret = SAI_STATUS_SUCCESS;
        }

        sdb_route.nh_oid = pattr_entry_list[SAI_ROUTE_ATTR_NEXT_HOP_ID].value.oid;
    }

    if(pattr_entry_list[SAI_ROUTE_ATTR_TRAP_PRIORITY].valid)
    {
        sdb_route.priority = pattr_entry_list[SAI_ROUTE_ATTR_TRAP_PRIORITY].value.u8;
    }

    if(pattr_entry_list[SAI_ROUTE_ATTR_PACKET_ACTION].valid)
    {
        sdb_route.action = pattr_entry_list[SAI_ROUTE_ATTR_PACKET_ACTION].value.s32;
    }
    else
    {
        sdb_route.action = SAI_PACKET_ACTION_FORWARD;
    }

    if (pattr_entry_list[SAI_ROUTE_ATTR_RIF_ID].valid)
    {
        sdb_route.rif_oid = pattr_entry_list[SAI_ROUTE_ATTR_RIF_ID].value.oid;
    }

    /* begin added by taocy for NAT, 20190603. support IPv4 only*/
    if (pattr_entry_list[SAI_ROUTE_ATTR_L3_IF_TYPE].valid)
    {
        sdb_route.l3if_type = pattr_entry_list[SAI_ROUTE_ATTR_L3_IF_TYPE].value.u8;
    }

    if (pattr_entry_list[SAI_ROUTE_ATTR_CATEGORY_ID].valid)
    {
        sdb_route.category_id = pattr_entry_list[SAI_ROUTE_ATTR_CATEGORY_ID].value.u8;
    }
    /* end added by taocy for NAT, 20190603. just support IPv4*/

    ret = __route_update_ipuc_to_sdk(&sdb_route);
    if (SAI_STATUS_SUCCESS != ret)
    {        
        return ret;
    }
    stbl_route_add_route(&sdb_route);
    
out:
    if(pattr_entry_list)
    {
        ctc_sai_attr_free_attr_entry_list(pattr_entry_list);
    }

    return ret;
}

/*
* Routine Description:
*    Remove Route
*
* Arguments:
*    [in] unicast_route_entry - route entry
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*
* Note: IP prefix/mask expected in Network Byte Order.
*/
sai_status_t
ctc_sai_remove_route(
    _In_ const sai_unicast_route_entry_t* unicast_route_entry)
{
    stbl_route_key_t             rt_key;
    stbl_route_t                 *p_sdb_route = NULL;
    sai_status_t                 ret = SAI_STATUS_SUCCESS;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_PTR_VALID_CHECK(unicast_route_entry);

    sal_memset(&rt_key, 0, sizeof(rt_key));
    sal_memcpy(&rt_key, unicast_route_entry, sizeof(sai_unicast_route_entry_t));
    
    /* 1. check route exist */
    p_sdb_route = stbl_route_get_route(&rt_key);    
    if(NULL == p_sdb_route)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    /* 2. remove route */
    ret = ctc_sai_route_db_remove_entry(p_sdb_route);
    if(SAI_STATUS_SUCCESS != ret)
    {
        return ret;
    }

    return SAI_STATUS_SUCCESS;
}

/*
* Routine Description:
*    Set route attribute value
*
* Arguments:
*    [in] unicast_route_entry - route entry
*    [in] attr - attribute
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_set_route_attribute(
    _In_ const sai_unicast_route_entry_t* unicast_route_entry,
    _In_ const sai_attribute_t *attr,
    _In_ sai_object_id_t nh_oid)
{
    stbl_route_key_t             rt_key;
    stbl_route_t                 *p_sdb_route = NULL;
    sai_status_t                 ret = SAI_STATUS_SUCCESS;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_PTR_VALID_CHECK(attr);
    CTC_SAI_PTR_VALID_CHECK(unicast_route_entry);

    ret = ctc_sai_attr_check_write_attr(g_sai_attr_entries,attr);
    if(ret != SAI_STATUS_SUCCESS)
    {
        log_sys(M_MOD_SAI, E_ERROR, "set_route_attribute failed for ctc_sai_attr_check_write_attr %u", ret);
        
        return ret;
    }

    sal_memset(&rt_key, 0, sizeof(rt_key));
    sal_memcpy(&rt_key, unicast_route_entry, sizeof(sai_unicast_route_entry_t));
    
    p_sdb_route = stbl_route_get_route(&rt_key);
    if (NULL == p_sdb_route)
    {
        log_sys(M_MOD_SAI, E_ERROR, "set_route_attribute failed for p_sdb_route NULL ");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    /*for bug 46502 by chenc*/
    if (SAI_ROUTE_ATTR_NEXT_HOP_ID != attr->id)
    {
        ret = __route_remove_ipuc_to_sdk(p_sdb_route);
    }
    
    ctc_sai_set_route_attribute_debug_param(unicast_route_entry, attr);
    switch(attr->id)
    {
    case SAI_ROUTE_ATTR_PACKET_ACTION:
        if (p_sdb_route->action != attr->value.s32)
        {
            p_sdb_route->action = attr->value.s32;
        }

        /*Modified by chenc for bug 51012*/
        if (p_sdb_route->action == SAI_PACKET_ACTION_FORWARD && nh_oid != 0)
        {
            p_sdb_route->nh_oid = nh_oid;
        }
        break;
        /*chenc end*/

    case SAI_ROUTE_ATTR_NEXT_HOP_ID:
        if (p_sdb_route->nh_oid != attr->value.oid)
        {
            p_sdb_route->nh_oid = attr->value.oid;
        }
        break;
    /* begin added by taocy for NAT, 2019-06-03*/
    case SAI_ROUTE_ATTR_L3_IF_TYPE:
        if (p_sdb_route->l3if_type != attr->value.u8)
        {
            p_sdb_route->l3if_type = attr->value.u8;
        }
        break;
    case SAI_ROUTE_ATTR_CATEGORY_ID:
        if (p_sdb_route->category_id != attr->value.u8)
        {
            p_sdb_route->category_id = attr->value.u8;
        }
        break;
    /* end added by taocy for NAT, 2019-06-03*/
    }

    ret = __route_update_ipuc_to_sdk(p_sdb_route);
    if (ret != 0)
    {
        log_sys(M_MOD_SAI, E_ERROR, "set_route_attribute failed for __route_update_ipuc_to_sdk %u", ret);
    }
    
    return ret;
}

/*
* Routine Description:
*    Get route attribute value
*
* Arguments:
*    [in] unicast_route_entry - route entry
*    [in] attr_count - number of attributes
*    [inout] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_get_route_attribute(
    _In_ const sai_unicast_route_entry_t* unicast_route_entry,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    stbl_route_key_t            rt_key;
    stbl_route_t                *p_sdb_route = NULL;
    sai_status_t                ret         = SAI_STATUS_SUCCESS;
    sai_attribute_t*            attr        = NULL;
    uint32_t                    attr_idx    = 0;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_PTR_VALID_CHECK(attr_list);
    CTC_SAI_PTR_VALID_CHECK(unicast_route_entry);

    sal_memset(&rt_key, 0, sizeof(rt_key));
    sal_memcpy(&rt_key, unicast_route_entry, sizeof(sai_unicast_route_entry_t));
    
    p_sdb_route = stbl_route_get_route(&rt_key);
    if (NULL == p_sdb_route)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    for(attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = attr_list + attr_idx;
        ret = ctc_sai_attr_check_read_attr(g_sai_attr_entries,attr);

        if(ret != SAI_STATUS_SUCCESS)
        {
            return ret + attr_idx;
        }

        switch(attr->id)
        {
        case SAI_ROUTE_ATTR_PACKET_ACTION:
            attr->value.s32 = p_sdb_route->action;
            break;
        case SAI_ROUTE_ATTR_TRAP_PRIORITY:
            attr->value.u8 = p_sdb_route->priority;
            break;
        case SAI_ROUTE_ATTR_NEXT_HOP_ID:
            attr->value.oid = p_sdb_route->nh_oid;
            break;
        }

        if(ret != SAI_STATUS_SUCCESS)
        {
            break;
        }
    }
    ctc_sai_get_route_attribute_debug_param(unicast_route_entry, attr_count, attr_list);
    return ret;
}

#define ________SAI_INNER_FUNC

static sai_status_t __init_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;

    ret = ctc_sai_route_db_init();
    if(SAI_STATUS_SUCCESS != ret)
    {
        return ret;
    }
    preg->init_status =  INITIALIZED;

    return ret;
}

static sai_status_t __exit_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    preg->init_status = UNINITIALIZED;
    return SAI_STATUS_SUCCESS;
}


static sai_route_api_t      g_sai_api_func = {
    .create_route           = ctc_sai_create_route,
    .remove_route           = ctc_sai_remove_route,
    .set_route_attribute    = ctc_sai_set_route_attribute,
    .get_route_attribute    = ctc_sai_get_route_attribute,
};

static ctc_sai_api_reg_info_t g_api_reg_info = {
        .id  = SAI_API_ROUTE,
        .init_func = __init_mode_fn,
        .exit_func = __exit_mode_fn,
        .api_method_table = &g_sai_api_func,
        .private_data     = NULL,
};

#define ________SAI_OUTER_FUNC
sai_status_t ctc_sai_route_init()
{
    api_reg_register_fn(&g_api_reg_info);

    return SAI_STATUS_SUCCESS;
}

