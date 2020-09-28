#include <sai.h>
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
#include <ctc_sai_nat.h>

#ifdef TSINGMA
#ifndef OFPRODUCT
static ctc_sai_attr_entry_info_t g_sai_attr_entries[] = {
    {
        .id     = SAI_NAT_ATTR_INSIDE_INTERFACE,
        .type   = SAI_ATTR_FALG_CREATE | SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_NAT_ATTR_ENTRY_HIT,
        .type   = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_ATTR_ID_END,
        .type   = 0,
    },
};


/* internal -> external SNAT process rules*/
void
ctc_sai_nat_sa_translate(_In_ const sai_nat_forward_entry_t* p_nat_forward_entry, ctc_ipuc_nat_sa_param_t *p_nat_sa)
{

    p_nat_sa->ip_ver = CTC_IP_VER_4;
    p_nat_sa->vrf_id = p_nat_forward_entry->vrf_id;
    sal_memcpy(&p_nat_sa->ipsa.ipv4, &p_nat_forward_entry->inside_local_ip.addr.ip4, sizeof(sai_ip4_t));
    sal_memcpy(&p_nat_sa->new_ipsa.ipv4, &p_nat_forward_entry->inside_global_ip.addr.ip4, sizeof(sai_ip4_t));
    p_nat_sa->l4_src_port = p_nat_forward_entry->inside_local_port;
    p_nat_sa->new_l4_src_port = p_nat_forward_entry->inside_global_port;
    if (p_nat_forward_entry->flags & SAI_NAT_FWD_FLAG_USE_TCP_PORT)
    {
        p_nat_sa->flag |= CTC_IPUC_NAT_FLAG_USE_TCP_PORT;
    }

    return;
}

/* internal -> external SNAT process rules*/
sai_status_t
ctc_sai_nat_sa_op(_In_ const sai_nat_forward_entry_t* nat_forward_entry, int32 (*fn) (ctc_ipuc_nat_sa_param_t*))
{
    int32 ret = 0;
    ctc_ipuc_nat_sa_param_t ipuc_nat_sa;

    sal_memset(&ipuc_nat_sa, 0, sizeof(ctc_ipuc_nat_sa_param_t));

    ctc_sai_nat_sa_translate(nat_forward_entry, &ipuc_nat_sa);

    ret = fn(&ipuc_nat_sa);

    return ctc_sai_get_error_from_sdk_error(ret);
}

void
ctc_sai_nat_ipuc_translate(_In_ const sai_nat_forward_entry_t* nat_forward_entry, ctc_ipuc_param_t *p_ipuc)
{
    p_ipuc->ip_ver = CTC_IP_VER_4;
    p_ipuc->vrf_id = nat_forward_entry->vrf_id;
    sal_memcpy(&p_ipuc->ip.ipv4, &nat_forward_entry->inside_global_ip.addr.ip4, sizeof(sai_ip4_t));
    p_ipuc->masklen = 32;
    p_ipuc->l4_dst_port = nat_forward_entry->inside_global_port;
    if (nat_forward_entry->flags & SAI_NAT_FWD_FLAG_USE_TCP_PORT)
    {
        p_ipuc->is_tcp_port = 1;
    }

    return;
}


/* external -> internal DNAT process rules*/
sai_status_t
ctc_sai_nat_ipuc_op(_In_ const sai_nat_forward_entry_t* nat_forward_entry, uint32 nhid, int32 (*fn) (ctc_ipuc_param_t*))
{
    int32 ret = 0;
    ctc_ipuc_param_t ipuc_info;

    sal_memset(&ipuc_info, 0, sizeof(ctc_ipuc_param_t));
    ctc_sai_nat_ipuc_translate(nat_forward_entry, &ipuc_info);
    ipuc_info.nh_id = nhid;

    ret = fn(&ipuc_info);
    return ctc_sai_get_error_from_sdk_error(ret);
}


/***************************************************************************************************
 * Name         : ctc_sai_nat_add_nh
 * Purpose      : add next hop
 * Input        : uint32 rif_id: inside route if id
                  const sai_nat_forward_entry_t *nat_forward_entry
 * Output       : uint32 *p_nhid
 * Return       : SAI_STATUS_XXXX
 * Note         : N/A
***************************************************************************************************/
sai_status_t
ctc_sai_nat_add_nh(uint32 *p_nhid, uint32 rif_id, const sai_nat_forward_entry_t *nat_forward_entry)
{
    sai_object_id_t rif_oid = 0;
    ctc_ip_tunnel_nh_param_t nh_param;
    stbl_rif_t          *p_sdb_rif = NULL;
    stbl_neighbor_t     *p_sdb_nb = NULL;
    stbl_neighbor_key_t  sdb_nb_key;
    int32 ret = 0;

    ret = ctc_sai_nexthop_alloc_offset(p_nhid);
    if (ret)
    {
        CTC_SAI_DEBUG("Alloc nh offset err(%d)", ret);
        return SAI_STATUS_TABLE_FULL;
    }

    rif_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_id);
    p_sdb_rif = ctc_routerintf_get_by_oid(rif_oid);
    if (!p_sdb_rif)
    {
        CTC_SAI_DEBUG("Failed to get router interface by oid:%lld.", rif_oid);
        ctc_sai_nexthop_free_offset(*p_nhid);
        return SAI_STATUS_FAILURE;
    }

    sal_memset(&nh_param, 0, sizeof(nh_param));
    if (SAI_ROUTER_INTERFACE_TYPE_VLAN == p_sdb_rif->type)
    {
        nh_param.oif.vid = p_sdb_rif->vlan_id;
    }
    else
    {
        ctc_sai_port_objectid_to_gport(p_sdb_rif->port_oid, &nh_param.oif.gport);
    }

    sal_memset(&sdb_nb_key, 0, sizeof(sdb_nb_key));
    sdb_nb_key.rif_id = rif_oid; /* sdb nb use rif_oid as key, not rif_id*/
    sal_memcpy(&sdb_nb_key.ip_address.addr.ip4, &nat_forward_entry->inside_local_ip.addr.ip4, sizeof(sai_ip4_t));
    p_sdb_nb = stbl_neighbor_get_neighbor(&sdb_nb_key);
    if (p_sdb_nb)
    {
        sal_memcpy(&nh_param.mac, p_sdb_nb->mac, sizeof(sai_mac_t));
    }

    nh_param.tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_IPV4_NAT;
    nh_param.tunnel_info.flag |= CTC_IP_NH_TUNNEL_FLAG_REROUTE_WITH_TUNNEL_HDR;
    sal_memcpy(&nh_param.tunnel_info.ip_da.ipv4, &nat_forward_entry->inside_local_ip.addr.ip4, sizeof(sai_ip4_t));
    nh_param.tunnel_info.l4_dst_port = nat_forward_entry->inside_local_port; 

    ret = ctc_nh_add_ip_tunnel(*p_nhid, &nh_param);
    return ctc_sai_get_error_from_sdk_error(ret);
}



/***************************************************************************************************
 * Name         : ctc_sai_nat_del_nh
 * Purpose      : delete nh by nhid
 * Input        : uint32 nhid
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
sai_status_t
ctc_sai_nat_del_nh(uint32 nhid)
{
    ctc_sai_nexthop_free_offset(nhid);
    ctc_nh_remove_ip_tunnel(nhid);

    return SAI_STATUS_SUCCESS;
}

/**
 * Routine Description:
 *    @brief Create nat forward entry
 *
 * Arguments:
 *    @param[in] nat_forward_entry - nat forward entry
 *    @param[in] attr_count - number of attributes
 *    @param[in] attrs - array of attributes
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 *
 * Note: IP address expected in Network Byte Order.
 */
sai_status_t
ctc_sai_create_nat_forward_entry(
    _In_ const sai_nat_forward_entry_t* nat_forward_entry,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list)
{
    //stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    sai_status_t ret = SAI_STATUS_SUCCESS;
    ctc_sai_attr_entry_list_t   *pattr_entry_list   = NULL;
    stbl_nat_forward_t *p_db_forward = NULL;
    stbl_nat_forward_t st_nat_forward;
    uint32 nhid = 0;

    CTC_SAI_DEBUG_FUNC();

    CTC_SAI_ERROR_GOTO(ctc_sai_attr_alloc_attr_entry_list(g_sai_attr_entries,
                            attr_list,
                            attr_count,
                            &pattr_entry_list),ret,out);

    CTC_SAI_ERROR_GOTO(ctc_sai_attr_check_attr_entry_list(g_sai_attr_entries,
                            pattr_entry_list),ret,out);

    if ((nat_forward_entry->inside_local_ip.addr_family != SAI_IP_ADDR_FAMILY_IPV4)
     || (nat_forward_entry->inside_global_ip.addr_family != SAI_IP_ADDR_FAMILY_IPV4))
    {
        /* IPv6 not supported*/
        ret = SAI_STATUS_NOT_SUPPORTED;
        goto out_end;
    }

    p_db_forward = stbl_nat_forward_get_nat_forward((stbl_nat_forward_key_t *)nat_forward_entry);
    if (p_db_forward)
    {
        char str[256];

        p_db_forward->use_cnt++;
        CTC_SAI_DEBUG("add existed entry, para:%s, cnt:%u", sdb_nat_forward_val2str(str, 256, nat_forward_entry), p_db_forward->use_cnt);
        return SAI_STATUS_SUCCESS;
    }

    /* 1. add nat next hop l3Edit */
    ret = ctc_sai_nat_add_nh(&nhid, pattr_entry_list[SAI_NAT_ATTR_INSIDE_INTERFACE].value.oid, nat_forward_entry);
    if (ret)
    {
        char str[256];
        CTC_SAI_DEBUG("add nat nh ret err(%d), para:%s", ret, sdb_nat_forward_val2str(str, 256, nat_forward_entry));
        goto out_end;
    }

    /* 2. Internel -> external, set NAT SA parameter */
    ret = ctc_sai_nat_sa_op(nat_forward_entry, ctc_ipuc_add_nat_sa);
    if (ret)
    {
        char str[256];
        CTC_SAI_DEBUG("add nat sa ret err(%d), para:%s", ret, sdb_nat_forward_val2str(str, 256, nat_forward_entry));
        goto out1;
    }

    /* 3. external -> Internal, add ipuc*/
    ret = ctc_sai_nat_ipuc_op(nat_forward_entry, nhid, ctc_ipuc_add);
    if (ret)
    {
        char str[256];
        CTC_SAI_DEBUG("add nat ipuc ret err(%d), nhid:%u, para:%s", ret, nhid, sdb_nat_forward_val2str(str, 256, nat_forward_entry));
        goto out;
    }

    /* 4. save to db*/
    sal_memset(&st_nat_forward, 0, sizeof(stbl_nat_forward_t));
    sal_memcpy(&st_nat_forward.key, nat_forward_entry, sizeof(stbl_nat_forward_key_t));
    st_nat_forward.use_cnt = 1;
    st_nat_forward.nhid = nhid;
    st_nat_forward.iif_oid = pattr_entry_list[SAI_NAT_ATTR_INSIDE_INTERFACE].value.oid; 
    stbl_nat_forward_add_nat_forward(&st_nat_forward);
    goto out_end;

out:
    ctc_sai_nat_sa_op(nat_forward_entry, ctc_ipuc_remove_nat_sa);
out1:
    ctc_sai_nat_del_nh(nhid);
out_end:
    if (pattr_entry_list)
    {
        ctc_sai_attr_free_attr_entry_list(pattr_entry_list);
    }
    return ret;
}

/**
 * Routine Description:
 *    @brief Remove nat forward entry
 *
 * Arguments:
 *    @param[in] nat_forward_entry - nat forward entry
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 *
 * Note: IP address expected in Network Byte Order.
 */
sai_status_t
ctc_sai_remove_nat_forward_entry(
    _In_ const sai_nat_forward_entry_t* nat_forward_entry)
{
    char str[256];
    stbl_nat_forward_t * p_sdb_nat_forward = NULL;

    CTC_SAI_DEBUG_FUNC();
    if ((nat_forward_entry->inside_local_ip.addr_family != SAI_IP_ADDR_FAMILY_IPV4)
     || (nat_forward_entry->inside_global_ip.addr_family != SAI_IP_ADDR_FAMILY_IPV4))
    {
        /* IPv6 not supported*/
        return SAI_STATUS_NOT_SUPPORTED;
    }

    p_sdb_nat_forward = stbl_nat_forward_get_nat_forward((stbl_nat_forward_key_t *)nat_forward_entry);
    if (!p_sdb_nat_forward)
    {
        CTC_SAI_DEBUG("remove entry does not exist para:%s", sdb_nat_forward_val2str(str, 256, nat_forward_entry));
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    CTC_SAI_DEBUG("remove entry :%s, cnt:%u", sdb_nat_forward_val2str(str, 256, nat_forward_entry), p_sdb_nat_forward->use_cnt);
    if (p_sdb_nat_forward->use_cnt > 0)
    {
        p_sdb_nat_forward->use_cnt--;
    }

    if (p_sdb_nat_forward->use_cnt != 0)
    {
        return SAI_STATUS_SUCCESS;
    }

    /* 1. delete nh*/
    ctc_sai_nat_del_nh(p_sdb_nat_forward->nhid);

    /* 2. Internel -> external, delete NAT SA */
    ctc_sai_nat_sa_op(nat_forward_entry, ctc_ipuc_remove_nat_sa);

    /* 3. external -> Internal, delete ipuc*/
    ctc_sai_nat_ipuc_op(nat_forward_entry, 0, ctc_ipuc_remove);

    /* 4. deleted from DB*/
    stbl_nat_forward_del_nat_forward((stbl_nat_forward_key_t *)nat_forward_entry);
    return SAI_STATUS_SUCCESS;
}


/**
 * Routine Description:
 *    @brief set nat forward entry attribute
 *
 * Arguments:
 *    @param[in] nat_forward_entry - nat forward entry
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 *
 * Note: IP address expected in Network Byte Order.
 */
sai_status_t
ctc_sai_set_nat_forward_attribute(
    _In_ const sai_nat_forward_entry_t* nat_forward_entry,
    _In_ const sai_attribute_t *attr)
{
    stbl_nat_forward_t * p_sdb_nat_forward = NULL;
    ctc_ipuc_nat_sa_param_t ipuc_nat_sa;
    ctc_ipuc_param_t        ipuc_param;
    sai_status_t ret = SAI_STATUS_SUCCESS;

    p_sdb_nat_forward = stbl_nat_forward_get_nat_forward((stbl_nat_forward_key_t *)nat_forward_entry);
    if (NULL == p_sdb_nat_forward)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    ret = ctc_sai_attr_check_read_attr(g_sai_attr_entries, attr);
    if(ret != SAI_STATUS_SUCCESS)
    {
        return ret + attr->id;
    }

    sal_memset(&ipuc_nat_sa, 0, sizeof(ctc_ipuc_nat_sa_param_t));
    ctc_sai_nat_sa_translate(nat_forward_entry, &ipuc_nat_sa);

    sal_memset(&ipuc_param, 0, sizeof(ctc_ipuc_param_t));
    ctc_sai_nat_ipuc_translate(nat_forward_entry, &ipuc_param);
    switch(attr->id)
    {
        case SAI_NAT_ATTR_ENTRY_HIT:
            ret = ctc_ipuc_set_natsa_entry_hit(&ipuc_nat_sa, attr->value.u8);
            if (ret)
            {
                /* bug exist in sdk UML , this func will always return err.*/
                char str[256];
                CTC_SAI_DEBUG("Set natsa entry hit ret err(%d), para:%s", ret, sdb_nat_forward_val2str(str, 256, nat_forward_entry));
                return ctc_sai_get_error_from_sdk_error(ret);
            }

            ret = ctc_ipuc_set_entry_hit(&ipuc_param, attr->value.u8);
            if (ret)
            {
                CTC_SAI_DEBUG("Set nat ipuc entry hit ret err(%d)", ret);
                return ctc_sai_get_error_from_sdk_error(ret);
            }
            break;
    }

    return SAI_STATUS_SUCCESS;
}

/**
 * Routine Description:
 *    @brief get nat forward entry attribute
 *
 * Arguments:
 *    @param[in] nat_forward_entry - nat forward entry
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 *
 * Note: IP address expected in Network Byte Order.
 */
sai_status_t
ctc_sai_get_nat_forward_attribute(
    _In_ const sai_nat_forward_entry_t* nat_forward_entry,
    _In_ sai_attribute_t *attr)
{
    stbl_nat_forward_t * p_sdb_nat_forward = NULL;
    ctc_ipuc_nat_sa_param_t ipuc_nat_sa;
    ctc_ipuc_param_t        ipuc_param;
    sai_status_t ret = SAI_STATUS_SUCCESS;

    p_sdb_nat_forward = stbl_nat_forward_get_nat_forward((stbl_nat_forward_key_t *)nat_forward_entry);
    if (NULL == p_sdb_nat_forward)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    ret = ctc_sai_attr_check_read_attr(g_sai_attr_entries, attr);
    if(ret != SAI_STATUS_SUCCESS)
    {
        return ret + attr->id;
    }

    sal_memset(&ipuc_nat_sa, 0, sizeof(ctc_ipuc_nat_sa_param_t));
    ctc_sai_nat_sa_translate(nat_forward_entry, &ipuc_nat_sa);

    sal_memset(&ipuc_param, 0, sizeof(ctc_ipuc_param_t));
    ctc_sai_nat_ipuc_translate(nat_forward_entry, &ipuc_param);
    switch(attr->id)
    {
        case SAI_NAT_ATTR_ENTRY_HIT:
        {
            uint8 nat_sa_hit = FALSE;
            uint8 ipuc_hit = FALSE;
            char str[256];

            attr->value.u8 = FALSE;
            ret = ctc_ipuc_get_natsa_entry_hit(&ipuc_nat_sa, &nat_sa_hit);
            if (ret)
            {
                CTC_SAI_DEBUG("Get natsa entry hit ret err(%d) para:%s", ret, sdb_nat_forward_val2str(str, 256, nat_forward_entry));
                return ctc_sai_get_error_from_sdk_error(ret);
            }

            ret = ctc_ipuc_get_entry_hit(&ipuc_param, &ipuc_hit);
            if (ret)
            {
                CTC_SAI_DEBUG("Get ipuc entry hit ret err(%d) para:%s", ret, sdb_nat_forward_val2str(str, 256, nat_forward_entry));
                return ctc_sai_get_error_from_sdk_error(ret);
            }

            if (nat_sa_hit || ipuc_hit)
            {
                attr->value.u8 = TRUE;
            }
            break;
        }
    }

    return SAI_STATUS_SUCCESS;
}


sai_status_t
ctc_sai_nat_enable_global_acl()
{
#if 0 // TODO: acl modify. bug exist, after init, packets can not forward to CPU.
    ctc_acl_property_t acl_prop;

    sal_memset(&acl_prop, 0 ,sizeof(acl_prop));
    acl_prop.acl_en = 1;
    acl_prop.direction = CTC_INGRESS;
    acl_prop.acl_priority = 3; // TODO: taocy tmp use. refer to ctcos hsrv_qos_service_init & stm_alloc_tsingma_set_acl_default
    acl_prop.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_FORWARD;
    ctc_global_ctl_set(CTC_GLOBAL_ACL_LKUP_PROPERTY, (void *)&acl_prop);

    ctc_acl_league_t leg;
    sal_memset(&leg, 0 , sizeof(leg));
    leg.dir = GLB_INGRESS;
    leg.acl_priority = 0;
    leg.lkup_level_bitmap = 7;
    ret = ctc_acl_set_league_mode(&leg);	

    ctc_global_acl_property_t acl_glb_prop;
    sal_memset(&acl_glb_prop, 0, sizeof(ctc_global_acl_property_t));
    acl_glb_prop.dir = CTC_INGRESS;
    ctc_global_ctl_get(CTC_GLOBAL_ACL_PROPERTY, &acl_glb_prop);
    sal_memset(acl_glb_prop.l2l3_ext_key_use_l2l3_key, 0, sizeof(uint8)*MAX_CTC_PARSER_L3_TYPE);
    ctc_global_ctl_set(CTC_GLOBAL_ACL_PROPERTY, &acl_glb_prop);
#endif
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
ctc_nat_init()
{
    // TODO: init module by device capacity.
    ctc_sai_nat_enable_global_acl();
    return 0;
}

static sai_status_t
__init_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;

    CTC_SAI_ERROR_GOTO(ctc_nat_init(),ret,out);

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

static sai_nat_api_t      g_sai_api_func = {
    .create_nat_forward_entry  = ctc_sai_create_nat_forward_entry,
    .remove_nat_forward_entry  = ctc_sai_remove_nat_forward_entry,
    .set_nat_forward_attribute  = ctc_sai_set_nat_forward_attribute,
    .get_nat_forward_attribute  = ctc_sai_get_nat_forward_attribute,
};

static ctc_sai_api_reg_info_t g_api_nat_reg_info = {
        .id         = SAI_API_NAT,
        .init_func  = __init_mode_fn,
        .exit_func  = __exit_mode_fn,
        .api_method_table = &g_sai_api_func,
        .private_data     = NULL,
};
#endif
#endif


#define ________SAI_OUTER_FUNC
sai_status_t
ctc_sai_nat_init()
{
#ifdef TSINGMA
#ifndef OFPRODUCT
    api_reg_register_fn(&g_api_nat_reg_info);
#endif
#endif
    return SAI_STATUS_SUCCESS;
}

