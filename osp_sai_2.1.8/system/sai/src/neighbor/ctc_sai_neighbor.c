#include <sai.h>
#include <sal.h>
#include "ctc_api.h"
#include <ctc_sai_common.h>
#include <ctc_sai_common_attr.h>
#include <ctc_sai_notifier_call.h>
#include <ctc_sai_sai.h>
#include <ctc_sai_fdb.h>
#include <ctc_sai_neighbor.h>
#include <ctc_sai_routerintf.h>
#include <ctc_sai_hostif.h>
#include <ctc_sai_nexthop.h>
#include <ctc_sai_port.h>
#include <ctc_sai_debug.h>

#define NEIGHBOR_HASH_BLOCK_SIZE    64
#define ROUTED_FDB_AGE_ARP_NOTIFY   1

#define CTC_SAI_IPV6_PREFIX_IS_LINK_LOCAL(val) \
    ((val[0] == 0xFE) && (val[1] == 0x80))
    
SAI_NEI_CB g_hsrv_neighbor_fdb_cb = NULL;
sal_mutex_t *g_sai_neigh_mutex = NULL;

/* modified by liwh for bug 48486, 2018-08-14 */
SAI_NEI_ND_CB g_hsrv_neighbor_nd_cb = NULL;
/* liwh end */

static sai_status_t
__ctc_sai_check_action_fn(
	_In_  ctc_sai_attr_entry_list_t *pattr,
	_In_  void 						*data);

static ctc_sai_attr_entry_info_t g_sai_attr_entries[] = {
    {
        .id     = SAI_NEIGHBOR_ATTR_DST_MAC_ADDRESS,
        .type   = SAI_ATTR_FALG_CREATE | SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_NEIGHBOR_ATTR_PACKET_ACTION,
        .type   = SAI_ATTR_FLAG_DEFAULT | SAI_ATTR_FALG_READ | SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_CREATE,
        .default_value.s32 = SAI_PACKET_ACTION_FORWARD,
        .check_fn = {
            .func_fn = __ctc_sai_check_action_fn,
            .func_parameter = NULL,
        }
    },
    {
        .id     = SAI_NEIGHBOR_ATTR_L3IFTYPE,
        .type   = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_NEIGHBOR_ATTR_CATEGORY_ID,
        .type   = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_ATTR_ID_END,
        .type   = 0,
    },
};

#define ________SAI_SAI_INNER_API_FUNC

int32
sai_nei_register_fdb(SAI_NEI_CB fn)
{
    g_hsrv_neighbor_fdb_cb = fn;
    return SAI_STATUS_SUCCESS;
}

/* modified by liwh for bug 48486, 2018-08-14 */
int32
sai_nei_register_nd(SAI_NEI_ND_CB fn)
{
    g_hsrv_neighbor_nd_cb = fn;
    return SAI_STATUS_SUCCESS;
}
/* liwh end */

sai_status_t
neighbor_update_nexthop(
    _In_  stbl_neighbor_t           *p_sdb_nb,
    _In_  sai_object_id_t           portid,
    _In_  uint32_t                  update_type)
{
    stbl_rif_t                  *p_sdb_rif = NULL;
    ctc_ip_nh_param_t            nh_param;
    ctc_ipuc_param_t             ipuc_info;
    uint32                       nh_id    = 0;
    int32                        sdk_ret  = 0;
    uint32                      update_act = 0;
    sal_memset(&nh_param, 0, sizeof(nh_param));

    update_act = update_type;
    if (SAI_NEIGHBOR_UPDATE_PORT == update_type)
    {
        if (portid == CTC_LPORT_CPU)
        {
            update_act = SAI_NEIGHBOR_REMOVE;
        }
        else
        {
            update_act = SAI_NEIGHBOR_CREATE;
        }
    }

    ctc_sai_port_objectid_to_gport(portid, &nh_param.oif.gport);

    /* 1. get router interface information */
    p_sdb_rif = ctc_routerintf_get_by_oid(p_sdb_nb->key.rif_id);
    if (NULL != p_sdb_rif)
    {
        if (SAI_ROUTER_INTERFACE_TYPE_VLAN == p_sdb_rif->type)
        {
            nh_param.oif.vid        = p_sdb_rif->vlan_id;
            #if defined(GOLDENGATE) || defined(GREATBELT)
            nh_param.oif.oif_type   = CTC_NH_OIF_TYPE_VLAN_PORT;
            #endif
            if (portid == CTC_LPORT_CPU && SAI_NEIGHBOR_CREATE == update_type)
            {
                return 0;
            }
        }
        else
        {
            #if defined(GOLDENGATE) || defined(GREATBELT)
            nh_param.oif.oif_type   = CTC_NH_OIF_TYPE_ROUTED_PORT;
            #endif
            ctc_sai_port_objectid_to_gport(p_sdb_rif->port_oid, &nh_param.oif.gport);
        }
    }
    /* remove neighbor, maybe router interface has been removed */
    else
    {
        #if defined(GOLDENGATE) || defined(GREATBELT)
        nh_param.oif.oif_type   = CTC_NH_OIF_TYPE_ROUTED_PORT;
        #endif
    }

    /* 2. set port id */
    p_sdb_nb->port_oid = portid;

    /* 3. set parameter */
    nh_id = CTC_SAI_OBJECT_INDEX_GET(p_sdb_nb->nh_oid);

    nh_param.dsnh_offset    = 0;
    //nh_param.upd_type       = CTC_NH_UPD_FWD_ATTR;
    sal_memcpy(nh_param.mac, p_sdb_nb->mac, sizeof(sai_mac_t));
    sal_memset(&ipuc_info,0,sizeof(ctc_ipuc_param_t));
    
    if(SAI_IP_ADDR_FAMILY_IPV4 == p_sdb_nb->key.ip_address.addr_family)
    {
        ipuc_info.ip.ipv4 = p_sdb_nb->key.ip_address.addr.ip4;
        ipuc_info.masklen = 32;
        ipuc_info.ip_ver  = CTC_IP_VER_4;
    }
    else
    {
        //sal_memcpy(ipuc_info.ip.ipv6, p_sdb_nb->key.ip_address.addr.ip6, sizeof(sai_ip6_t));
    
        sal_memcpy(&ipuc_info.ip.ipv6[0], &p_sdb_nb->key.ip_address.addr.ip6[0], 4);
        sal_memcpy(&ipuc_info.ip.ipv6[1], &p_sdb_nb->key.ip_address.addr.ip6[4], 4);
        sal_memcpy(&ipuc_info.ip.ipv6[2], &p_sdb_nb->key.ip_address.addr.ip6[8], 4);
        sal_memcpy(&ipuc_info.ip.ipv6[3], &p_sdb_nb->key.ip_address.addr.ip6[12], 4);
    
        ipuc_info.ip.ipv6[0] = htonl(ipuc_info.ip.ipv6[0]);
        ipuc_info.ip.ipv6[1] = htonl(ipuc_info.ip.ipv6[1]);
        ipuc_info.ip.ipv6[2] = htonl(ipuc_info.ip.ipv6[2]);
        ipuc_info.ip.ipv6[3] = htonl(ipuc_info.ip.ipv6[3]);
        
        ipuc_info.masklen = 128;
        ipuc_info.ip_ver  = CTC_IP_VER_6;
    }
    
    ipuc_info.nh_id = nh_id;
    ipuc_info.route_flag = CTC_IPUC_FLAG_NEIGHBOR;

    /* begin added by taocy for NAT, 20190603. just support IPv4*/
#ifdef TSINGMA
#ifndef OFPRODUCT
    if (p_sdb_rif)
    {
        if (SAI_ROUTER_INTERFACE_NAT_OUTSIDE != p_sdb_rif->nat_enable)
        {
            SAI_SET_FLAG(ipuc_info.route_flag, CTC_IPUC_FLAG_CANCEL_NAT);
        }
    
        ipuc_info.cid = p_sdb_rif->nat_category_id;
    }
#endif
#endif
    /* end added by taocy for NAT, 20190603*/

    /* 4. create neigbor ,one router should be created */
    switch (update_act)
    {
        case SAI_NEIGHBOR_CREATE:
        {
            nh_param.upd_type       = CTC_NH_UPD_UNRSV_TO_FWD;
            sdk_ret = ctc_nh_update_ipuc(nh_id, &nh_param);
            if (0 == sdk_ret)
            {
                CTC_SET_FLAG(ipuc_info.route_flag, CTC_IPUC_FLAG_TTL_CHECK);
                CTC_SET_FLAG(ipuc_info.route_flag, CTC_IPUC_FLAG_ICMP_CHECK);

                if ((SAI_IP_ADDR_FAMILY_IPV4 == p_sdb_nb->key.ip_address.addr_family)
                    || (!CTC_SAI_IPV6_PREFIX_IS_LINK_LOCAL(p_sdb_nb->key.ip_address.addr.ip6)))
                {
                    sdk_ret = ctc_ipuc_add(&ipuc_info);
                }
            }
            break;
        }
        /* 5. remove neigbor ,one router should be removed */
        case SAI_NEIGHBOR_REMOVE:
        {
            nh_param.upd_type       = CTC_NH_UPD_FWD_TO_UNRSV;

            if ((SAI_IP_ADDR_FAMILY_IPV4 == p_sdb_nb->key.ip_address.addr_family)
                || (!CTC_SAI_IPV6_PREFIX_IS_LINK_LOCAL(p_sdb_nb->key.ip_address.addr.ip6)))
            {
                sdk_ret = ctc_ipuc_remove(&ipuc_info);
            }

            sdk_ret = ctc_nh_update_ipuc(nh_id, &nh_param);
            break;
        }
        case SAI_NEIGHBOR_UPDATE_PORT:
        {
            sdk_ret = ctc_nh_update_ipuc(nh_id, &nh_param);
            break;
        }
        case SAI_NEIGHBOR_UPDATE_L3IFTYPE:
        case SAI_NEIGHBOR_UPDATE_CATEGORY_ID:
        {
            /* begin: added by taocy for bug 53016. same as CREATION, set TTL & ICMP CHECK flag.*/
            CTC_SET_FLAG(ipuc_info.route_flag, CTC_IPUC_FLAG_TTL_CHECK);
            CTC_SET_FLAG(ipuc_info.route_flag, CTC_IPUC_FLAG_ICMP_CHECK);
            /* end: added by taocy for bug 53016*/
            sdk_ret = ctc_ipuc_remove(&ipuc_info);
            sdk_ret = ctc_ipuc_add(&ipuc_info);
            break;
        }
        default:
        {
            return SAI_STATUS_FAILURE;
        }
    }

    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

void
ctc_neighbor_release(
	_In_  stbl_neighbor_t *p_sdb_nb)
{
    sai_object_id_t dma_port = CTC_LPORT_CPU;
    sai_status_t        ret     = SAI_STATUS_SUCCESS;

    CTC_SAI_ERROR_GOTO(
        neighbor_update_nexthop(p_sdb_nb, dma_port, SAI_NEIGHBOR_REMOVE), ret,out);

    sal_mutex_lock(g_sai_neigh_mutex);
    stbl_neighbor_del_neighbor(&p_sdb_nb->key);
    sal_mutex_unlock(g_sai_neigh_mutex);
out:
    return;
}

sai_status_t
ctc_neighbor_update_entry(
    _In_  stbl_neighbor_t *p_sdb_nb,
    _In_  uint32_t         update_type)
{
    sai_status_t        ret     = SAI_STATUS_SUCCESS;
    sai_fdb_entry_t     key;
    stbl_rif_t         *p_sdb_rif = NULL;
    ctc_sai_fdb_entry_t fdb_entry;
    sai_object_id_t port_id = 0;

    sal_memset(&fdb_entry,0,sizeof(ctc_sai_fdb_entry_t));
    sal_memset(&key,0,sizeof(sai_fdb_entry_t));

    p_sdb_rif = ctc_routerintf_get_by_oid(p_sdb_nb->key.rif_id);
    if (NULL == p_sdb_rif)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if(SAI_ROUTER_INTERFACE_TYPE_VLAN == p_sdb_rif->type)
    {
        if (SAI_STATUS_SUCCESS != ctc_fdb_get_portid(p_sdb_nb->mac, p_sdb_rif->vlan_id, &port_id))
        {
            port_id = CTC_LPORT_CPU;
        }
    }
    else
    {
        port_id = p_sdb_rif->port_oid;
    }

    CTC_SAI_ERROR_GOTO(
        neighbor_update_nexthop(p_sdb_nb, port_id, update_type), ret, out);

out:
    return ret;
}

static int32_t
_ctc_neighbor_remove_all_fn(void *obj, stbl_iter_args_t *pargs)
{
    stbl_neighbor_t *p_sdb_nb  = (stbl_neighbor_t*)obj;

    ctc_neighbor_release(p_sdb_nb);
    return 0;
}

sai_status_t
ctc_neighbor_remove_all()
{
    stbl_iter_args_t args;
    stbl_neighbor_iterate(_ctc_neighbor_remove_all_fn, &args);

    return 0;
}

sai_status_t
ctc_neighbor_set_mac(
	_In_  stbl_neighbor_t           *p_sdb_nb,
	_In_  const sai_attribute_t 	*attr)
{
    sai_status_t    ret = SAI_STATUS_NOT_SUPPORTED;

    if(SAI_STATUS_SUCCESS == ret)
    {
        sal_memcpy(&p_sdb_nb->mac,&attr->value.mac,sizeof(sai_mac_t));
    }

    return ret;
}

sai_status_t
ctc_neighbor_set_l3iftype(
    _In_  stbl_neighbor_t           *p_sdb_nb,
    _In_  const sai_attribute_t 	*attr)
{
    sai_status_t    ret = SAI_STATUS_NOT_SUPPORTED;

    ret = ctc_neighbor_update_entry(p_sdb_nb, SAI_NEIGHBOR_UPDATE_L3IFTYPE);
    if(SAI_STATUS_SUCCESS == ret)
    {
        // TODO: taocy save l3iftype or not????????   p_sdb_nb->
    }

    return ret;
}

sai_status_t
ctc_neighbor_set_category_id(
    _In_  stbl_neighbor_t           *p_sdb_nb,
    _In_  const sai_attribute_t 	*attr)
{
    sai_status_t    ret = SAI_STATUS_NOT_SUPPORTED;

    ret = ctc_neighbor_update_entry(p_sdb_nb, SAI_NEIGHBOR_UPDATE_CATEGORY_ID);
    if (SAI_STATUS_SUCCESS == ret)
    {
        // TODO: taocy save l3iftype or not????????   p_sdb_nb->
    }

    return ret;
}

int32
ctc_fdb_update_neighbor(void *obj, stbl_iter_args_t *pargs)
{
    stbl_neighbor_t         *p_sdb_nb = (stbl_neighbor_t*)obj;
    ctc_sai_fdb_entry_t     *pst_fdb = (ctc_sai_fdb_entry_t*)pargs->argv[0];
    stbl_rif_t              *p_sdb_rif = NULL;
    sai_object_id_t         port_id = 0;
    uint32_t                action = *(uint32_t*)pargs->argv[1];

    p_sdb_rif = ctc_routerintf_get_by_oid(p_sdb_nb->key.rif_id);
    if ((NULL == p_sdb_rif)
        || (SAI_ROUTER_INTERFACE_TYPE_VLAN != p_sdb_rif->type)
        || (p_sdb_rif->vlan_id != pst_fdb->fdb_key.vlan_id)
        || (0 != sal_memcmp(p_sdb_nb->mac, pst_fdb->fdb_key.mac_address, sizeof(mac_addr_t))))
    {
        return 0;
    }

    switch(action)
    {
    case SAI_FDB_EVENT_LEARNED:
        port_id = pst_fdb->port;
        break;

    case SAI_FDB_EVENT_AGED:
    case SAI_FDB_EVENT_FLUSHED:
        port_id = CTC_LPORT_CPU;
        if (NULL != g_hsrv_neighbor_fdb_cb)
        {
            (*g_hsrv_neighbor_fdb_cb)(&p_sdb_nb->key);
        }

        break;
    }

    if (p_sdb_nb->port_oid != port_id)
    {
        neighbor_update_nexthop(p_sdb_nb, port_id, SAI_NEIGHBOR_UPDATE_PORT);
    }

    return 0;
}

/* modified by liwh for bug 48486, 2018-08-14 */
int32
ctc_nd_update_neighbor(uint8 *p_daddr, char *ifname)
{
    if (NULL != g_hsrv_neighbor_nd_cb)
    {
        (*g_hsrv_neighbor_nd_cb)(p_daddr, ifname);
    }
    
    return 0;
}
/* liwh end */

int32_t
ctc_fdb_notifier_callback(
    _In_  struct ctc_sai_notifier_block *nb,
    _In_  uint32_t action,
    _In_  void *data)
{
    ctc_sai_fdb_entry_t*        pst_fdb = data;
    stbl_rif_t                  *p_sdb_rif = NULL;
    stbl_iter_args_t            args;

    sal_memset(&args, 0, sizeof(args));
    p_sdb_rif = ctc_routerintf_get_by_vlan(pst_fdb->fdb_key.vlan_id);
    if (NULL == p_sdb_rif)
    {
        return CTC_SAI_NOTIFY_OK;
    }

    args.argv[0] = pst_fdb;
    args.argv[1] = &action;

    sal_mutex_lock(g_sai_neigh_mutex);
    stbl_neighbor_iterate(ctc_fdb_update_neighbor, &args);
    sal_mutex_unlock(g_sai_neigh_mutex);

    return CTC_SAI_NOTIFY_OK;
}

static struct ctc_sai_notifier_block nb_fdb_notifier_cb = {
    .notifier_call = ctc_fdb_notifier_callback,
};

sai_status_t
ctc_neighbor_init()
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();

    p_sai_glb->nh_max_count = CTC_SAI_NEIGHBOR_MAX;
    ctc_sai_fdbevnet_notifier_register(&nb_fdb_notifier_cb);
    return CTC_SAI_NOTIFY_DONE;
}

#define ________SAI_SAI_DEBUG_FUNC
sai_status_t
ctc_sai_set_neighbor_attribute_debug_param(
    _In_ const sai_neighbor_entry_t* neighbor_entry,
    _In_ const sai_attribute_t *attr)
{
    switch(attr->id)
    {
    case SAI_NEIGHBOR_ATTR_DST_MAC_ADDRESS:
        CTC_SAI_DEBUG("in:SAI_NEIGHBOR_ATTR_DST_MAC_ADDRESS %x:%x:%x:%x:%x:%x",
            attr->value.mac[5], attr->value.mac[4], attr->value.mac[3],
            attr->value.mac[2], attr->value.mac[1], attr->value.mac[0]);
        break;

    case SAI_NEIGHBOR_ATTR_PACKET_ACTION:
        CTC_SAI_DEBUG("in:SAI_NEIGHBOR_ATTR_PACKET_ACTION %d", attr->value.s32);
        break;

    case SAI_NEIGHBOR_ATTR_CATEGORY_ID:
        CTC_SAI_DEBUG("in:SAI_NEIGHBOR_ATTR_CATEGORY_ID, %d", attr->value.u8);
        break;

    case SAI_NEIGHBOR_ATTR_L3IFTYPE:
        CTC_SAI_DEBUG("in:SAI_NEIGHBOR_ATTR_L3IFTYPE, %d", attr->value.u8);
        break;
    }

    return SAI_STATUS_SUCCESS;
}
sai_status_t
ctc_sai_get_neighbor_attribute_debug_param(
    _In_ const sai_neighbor_entry_t* neighbor_entry,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    sai_status_t     ret         = SAI_STATUS_SUCCESS;
    uint32_t         attr_idx    = 0;
    sai_attribute_t* attr        = NULL;

    for(attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = attr_list + attr_idx;
        ret = ctc_sai_attr_check_read_attr(g_sai_attr_entries,attr);

        if(ret != SAI_STATUS_SUCCESS)
        {
            return ret + attr_idx;
        }

        switch (attr->id)
        {
            case SAI_NEXT_HOP_ATTR_TYPE:
                CTC_SAI_DEBUG("out:SAI_NEXT_HOP_ATTR_TYPE %d", attr->value.s32);
                break;
            case SAI_NEXT_HOP_ATTR_IP:
                CTC_SAI_DEBUG("out:SAI_NEXT_HOP_ATTR_IP");
                break;
            case SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID:
                CTC_SAI_DEBUG("out:SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID 0x%llx", attr->value.oid);
                break;
            default:
                continue;
        }
    }

    return SAI_STATUS_SUCCESS;
}
#define ________SAI_SAI_API_FUNC
/*
* Routine Description:
*    Create neighbor entry
*
* Arguments:
*    [in] neighbor_entry - neighbor entry
*    [in] attr_count - number of attributes
*    [in] attrs - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*
* Note: IP address expected in Network Byte Order.
*/
sai_status_t
ctc_sai_create_neighbor_entry(
    _In_ const sai_neighbor_entry_t* neighbor_entry,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    stbl_neighbor_master_t *p_master = stbl_neighbor_get_master();
    ctc_sai_attr_entry_list_t   *pattr_entry_list   = NULL;
    stbl_rif_t                  *p_sdb_rif          = NULL;
    stbl_nexthop_t              *p_sdb_nh           = NULL;
    stbl_nexthop_key_t          nh_key;
    sai_status_t ret = SAI_STATUS_SUCCESS;
    stbl_neighbor_t     sdb_nb;
    stbl_neighbor_t     *p_sdb_nb 	= NULL;

    CTC_SAI_DEBUG_FUNC();

    CTC_SAI_PTR_VALID_CHECK(neighbor_entry);
    CTC_SAI_PTR_VALID_CHECK(attr_list);

    CTC_SAI_ERROR_GOTO(ctc_sai_attr_alloc_attr_entry_list(g_sai_attr_entries,
                            attr_list,
                            attr_count,
                            &pattr_entry_list),ret,out);

    CTC_SAI_ERROR_GOTO(ctc_sai_attr_check_attr_entry_list(g_sai_attr_entries,
                            pattr_entry_list),ret,out);

    /* 1. check neighor exist */
    sal_memset(&nh_key, 0, sizeof(nh_key));
    sal_memset(&sdb_nb, 0, sizeof(sdb_nb));
    sdb_nb.key.rif_id = neighbor_entry->rif_id;
    sdb_nb.key.ip_address = neighbor_entry->ip_address;
    
    p_sdb_nb = stbl_neighbor_get_neighbor(&sdb_nb.key);
    if (NULL != p_sdb_nb)
    {
        ret = SAI_STATUS_ITEM_ALREADY_EXISTS;
        goto out;
    }

    if (p_master->neigh_hash->count >= p_sai_glb->nh_max_count)
    {
        ret = SAI_STATUS_TABLE_FULL;
        goto out;
    }

    sal_mutex_lock(g_sai_neigh_mutex);
    stbl_neighbor_add_neighbor(&sdb_nb);
    p_sdb_nb = stbl_neighbor_get_neighbor(&sdb_nb.key);
    sal_mutex_unlock(g_sai_neigh_mutex);
    if (NULL == p_sdb_nb)
    {
        ret = SAI_STATUS_NO_MEMORY;
        goto out;
    }

    p_sdb_rif = ctc_routerintf_get_by_oid(neighbor_entry->rif_id);
    if(NULL == p_sdb_rif)
    {
        ret = SAI_STATUS_INVALID_OBJECT_ID;
        goto out1;
    }
    
    sal_memcpy(p_sdb_nb->mac,
        pattr_entry_list[SAI_NEIGHBOR_ATTR_DST_MAC_ADDRESS].value.mac,
        sizeof(mac_addr_t));

    p_sdb_nb->action = pattr_entry_list[SAI_NEIGHBOR_ATTR_PACKET_ACTION].value.s32;


    sal_memset(&nh_key, 0, sizeof(nh_key));
    nh_key.rif_id = p_sdb_nb->key.rif_id;
    sal_memcpy(&nh_key.ip_address, &p_sdb_nb->key.ip_address, sizeof(sai_ip_address_t));
    p_sdb_nh = stbl_nexthop_get_nexthop(&nh_key);
    if (NULL == p_sdb_nh)
    {
        p_sdb_nb->nh_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_NEXT_HOP,CTC_NH_RESERVED_NHID_FOR_DROP);
    }
    else
    {
        p_sdb_nb->nh_oid = p_sdb_nh->nh_oid;
    }

    /* 4. add neigbor */
    if(NULL != p_sdb_nh)
    {
        ctc_neighbor_update_entry(p_sdb_nb, SAI_NEIGHBOR_CREATE);
        if(SAI_STATUS_SUCCESS != ret)
        {
            ret = SAI_STATUS_FAILURE;
            goto out1;
        }
    }

out:
    if(pattr_entry_list)
    {
        ctc_sai_attr_free_attr_entry_list(pattr_entry_list);
    }

    return ret;

out1:
    if (p_sdb_nb)
    {
        ctc_neighbor_release(p_sdb_nb);
    }
    goto out;
}

/*
* Routine Description:
*    Remove neighbor entry
*
* Arguments:
*    [in] neighbor_entry - neighbor entry
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*
* Note: IP address expected in Network Byte Order.
*/
sai_status_t
ctc_sai_remove_neighbor_entry(
    _In_ const sai_neighbor_entry_t* neighbor_entry)
{
    stbl_neighbor_key_t nb_key;
    stbl_neighbor_t *p_sdb_nb = NULL;

    CTC_SAI_DEBUG_FUNC();

    sal_memset(&nb_key, 0, sizeof(nb_key));
    nb_key.rif_id = neighbor_entry->rif_id;
    nb_key.ip_address = neighbor_entry->ip_address;
    
    p_sdb_nb = stbl_neighbor_get_neighbor(&nb_key);
    if (NULL == p_sdb_nb)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    ctc_neighbor_release(p_sdb_nb);
    return SAI_STATUS_SUCCESS;
}

/*
* Routine Description:
*    Set neighbor attribute value
*
* Arguments:
*    [in] neighbor_entry - neighbor entry
*    [in] attr - attribute
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_set_neighbor_attribute(
    _In_ const sai_neighbor_entry_t* neighbor_entry,
    _In_ const sai_attribute_t *attr)
{
    stbl_neighbor_key_t nb_key;
    stbl_neighbor_t     *p_sdb_nb = NULL;
    sai_status_t        ret = SAI_STATUS_SUCCESS;

    CTC_SAI_DEBUG_FUNC();

    sal_memset(&nb_key, 0, sizeof(nb_key));
    nb_key.rif_id = neighbor_entry->rif_id;
    nb_key.ip_address = neighbor_entry->ip_address;
    
    p_sdb_nb = stbl_neighbor_get_neighbor(&nb_key);
    if (NULL == p_sdb_nb)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    
    ret = ctc_sai_attr_check_write_attr(g_sai_attr_entries, attr);
    if(ret != SAI_STATUS_SUCCESS)
    {
        return ret;
    }
    
    ctc_sai_set_neighbor_attribute_debug_param(neighbor_entry, attr);
    switch(attr->id)
    {
    case SAI_NEIGHBOR_ATTR_DST_MAC_ADDRESS:
        ret = ctc_neighbor_set_mac(p_sdb_nb, attr);
        break;

    case SAI_NEIGHBOR_ATTR_L3IFTYPE:
        ret = ctc_neighbor_set_l3iftype(p_sdb_nb, attr);
        break;
    case SAI_NEIGHBOR_ATTR_CATEGORY_ID:
        ret = ctc_neighbor_set_category_id(p_sdb_nb, attr);
        break;

    case SAI_NEIGHBOR_ATTR_PACKET_ACTION:
        ret = SAI_STATUS_NOT_SUPPORTED;
        break;

    default:
        ret = SAI_STATUS_NOT_SUPPORTED;
        break;
    }

    return ret;
}

/*
* Routine Description:
*    Get neighbor attribute value
*
* Arguments:
*    [in] neighbor_entry - neighbor entry
*    [in] attr_count - number of attributes
*    [inout] attrs - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_get_neighbor_attribute(
    _In_ const sai_neighbor_entry_t* neighbor_entry,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    stbl_neighbor_key_t nb_key;
    stbl_neighbor_t     *p_sdb_nb = NULL;
    sai_attribute_t     *attr = NULL;
    uint32_t            attr_idx = 0;
    sai_status_t        ret = SAI_STATUS_SUCCESS;

    CTC_SAI_DEBUG_FUNC();

    sal_memset(&nb_key, 0, sizeof(nb_key));
    nb_key.rif_id = neighbor_entry->rif_id;
    nb_key.ip_address = neighbor_entry->ip_address;
    
    p_sdb_nb = stbl_neighbor_get_neighbor(&nb_key);
    if (NULL == p_sdb_nb)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    
    ret = ctc_sai_attr_check_write_attr(g_sai_attr_entries,attr_list);
    if(ret != SAI_STATUS_SUCCESS)
    {
        return ret;
    }
    
    for(attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = attr_list + attr_idx;
        ret = ctc_sai_attr_check_read_attr(g_sai_attr_entries,attr);

        if(ret != SAI_STATUS_SUCCESS)
        {
            return ret + attr_idx;
        }

        switch(attr_list->id)
        {
        case SAI_NEIGHBOR_ATTR_DST_MAC_ADDRESS:
            sal_memcpy(attr_list->value.mac, p_sdb_nb->mac, sizeof(sai_mac_t));
            break;
        }

        if(ret != SAI_STATUS_SUCCESS)
        {
            break;
        }
    }
    ctc_sai_get_neighbor_attribute_debug_param(neighbor_entry, attr_count, attr_list);
    return ret;
}

/*
* Routine Description:
*    Remove all neighbor entries
*
* Arguments:
*    None
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_remove_all_neighbor_entries(void)
{
    CTC_SAI_DEBUG_FUNC();

    ctc_neighbor_remove_all();
    return SAI_STATUS_SUCCESS;
}

#define ________SAI_INNER_FUNC
static sai_status_t
__ctc_sai_check_action_fn(
    _In_  ctc_sai_attr_entry_list_t *pattr,
    _In_  void                      *data)
{
    if (SAI_PACKET_ACTION_DROP == pattr->value.s32
        || SAI_PACKET_ACTION_FORWARD  == pattr->value.s32)
    {
        return SAI_STATUS_SUCCESS;
    }

    return SAI_STATUS_ATTR_NOT_SUPPORTED_0 + pattr->attr_index;
}

static sai_status_t
__init_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;

    CTC_SAI_ERROR_GOTO(ctc_neighbor_init(),ret,out);
    sal_mutex_create(&g_sai_neigh_mutex);
    preg->init_status =  INITIALIZED;

out:
    return ret;
}

static sai_status_t
__exit_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    preg->init_status = UNINITIALIZED;
    return SAI_STATUS_SUCCESS;
}

/* define sai 0.9.2 */
static sai_neighbor_api_t      g_sai_api_func = {
    .create_neighbor_entry      = ctc_sai_create_neighbor_entry,
    .remove_neighbor_entry      = ctc_sai_remove_neighbor_entry,
    .set_neighbor_attribute     = ctc_sai_set_neighbor_attribute,
    .get_neighbor_attribute     = ctc_sai_get_neighbor_attribute,
    .remove_all_neighbor_entries= ctc_sai_remove_all_neighbor_entries,
};

static ctc_sai_api_reg_info_t g_api_reg_info = {
        .id         = SAI_API_NEIGHBOR,
        .init_func  = __init_mode_fn,
        .exit_func  = __exit_mode_fn,
        .api_method_table = &g_sai_api_func,
        .private_data     = NULL,
};

#define ________SAI_OUTER_FUNC
sai_status_t
ctc_sai_neighbor_init()
{
    api_reg_register_fn(&g_api_reg_info);

    return SAI_STATUS_SUCCESS;
}

