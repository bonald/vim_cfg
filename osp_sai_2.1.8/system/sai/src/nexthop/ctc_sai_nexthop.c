#include <sai.h>
#include <sal.h>
#include "ctc_api.h"
#include <ctc_sai_common.h>
#include <ctc_sai_common_attr.h>
#include <ctc_sai_notifier_call.h>
#include <ctc_sai_sai.h>
#include <ctc_sai_port.h>
#include <ctc_sai_routerintf.h>
#include <ctc_sai_nexthop.h>
#include <ctc_sai_neighbor.h>
#include <ctc_sai_debug.h>

#define CTC_SAI_NEXTHOP_BLOCK_SIZE	64

static ctc_sai_attr_entry_info_t g_sai_attr_entries[] = {
    {
        .id     = SAI_NEXT_HOP_ATTR_TYPE,
        .type   = SAI_ATTR_FALG_READ | SAI_ATTR_FALG_WRITE,
    },
    {
        .id     = SAI_NEXT_HOP_ATTR_IP,
        .type   = SAI_ATTR_FALG_CREATE | SAI_ATTR_FALG_READ,
    },
	{
        .id     = SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID,
        .type   = SAI_ATTR_FALG_CREATE | SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_ATTR_ID_END,
        .type   = 0,
    },
};

sai_status_t
ctc_sai_nexthop_alloc_offset(uint32_t *p_index)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    sai_status_t    ret         = SAI_STATUS_SUCCESS;
    uint32          index   = 0;
    
    CTC_SAI_ERROR_GOTO_MAPSDKERR2SAI(
        ctclib_opb_alloc_offset(&p_sai_glb->nexthop_id_opb, &index),ret,out);

    *p_index = index;
    
out:
    return ret;
}

sai_status_t
ctc_sai_nexthop_free_offset(uint32_t index)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    sai_status_t    ret         = SAI_STATUS_SUCCESS;
    
    CTC_SAI_ERROR_GOTO_MAPSDKERR2SAI(
        ctclib_opb_free_offset(&p_sai_glb->nexthop_id_opb, index),ret,out);
out:
    return ret;
}

void
ctc_nexthop_release(
	_In_  stbl_nexthop_t *p_sdb_nh)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    if(NULL == p_sdb_nh)
    {
        return;
    }

    ctclib_opb_free_offset(&p_sai_glb->nexthop_id_opb, CTC_SAI_OBJECT_INDEX_GET(p_sdb_nh->nh_oid));
    stbl_nexthop_del_array(CTC_SAI_OBJECT_INDEX_GET(p_sdb_nh->nh_oid));
    stbl_nexthop_del_nexthop(&p_sdb_nh->key);

    return;
}

stbl_nexthop_t*
ctc_nexthop_get_by_oid(
	_In_  const sai_object_id_t nh_oid)
{
    stbl_nexthop_master_t* p_master = stbl_nexthop_get_master();
    stbl_nexthop_t  *p_sdb_nexthop = NULL;
    uint32 nh_id = 0;

    nh_id = CTC_SAI_OBJECT_INDEX_GET(nh_oid);
    if (nh_id > GLB_MAX_CTC_NEXTHOP_ID)
    {
        return NULL;
    }
    p_sdb_nexthop = p_master->array[nh_id];
    return p_sdb_nexthop;
}

sai_status_t
ctc_nexthop_add_entry(
    _In_  stbl_nexthop_t          *p_sdb_nh,
    _In_  stbl_rif_t              *p_sdb_rif)
{
    ctc_ip_nh_param_t ipuc_nexthop;

    sal_memset(&ipuc_nexthop, 0, sizeof(ipuc_nexthop));
    ipuc_nexthop.dsnh_offset  = 0;
    ipuc_nexthop.oif.gport = 65535;

    SAI_SET_FLAG(ipuc_nexthop.flag, CTC_IP_NH_FLAG_UNROV);

    if (SAI_ROUTER_INTERFACE_TYPE_VLAN == p_sdb_rif->type)
    {
        ipuc_nexthop.oif.vid = p_sdb_rif->vlan_id;
        #if defined(GOLDENGATE) || defined(GREATBELT)
        ipuc_nexthop.oif.oif_type = CTC_NH_OIF_TYPE_VLAN_PORT;
        #endif
    }
    else
    {
        #if defined(GOLDENGATE) || defined(GREATBELT)
        ipuc_nexthop.oif.oif_type = CTC_NH_OIF_TYPE_ROUTED_PORT;
        #endif
        ctc_sai_port_objectid_to_gport(p_sdb_rif->port_oid, &ipuc_nexthop.oif.gport);
    }

    return ctc_nh_add_ipuc(CTC_SAI_OBJECT_INDEX_GET(p_sdb_nh->nh_oid), &ipuc_nexthop);
}

sai_status_t
ctc_nexthop_get_type(
	_In_  stbl_nexthop_t 	        *p_sdb_nh,
	_Inout_ sai_attribute_t			*attr)
{
    attr->value.s32 = p_sdb_nh->nh_type;
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_nexthop_get_ip(
	_In_  stbl_nexthop_t 	        *p_sdb_nh,
	_Inout_  sai_attribute_t		*attr)
{
    sal_memcpy(&attr->value.ipaddr, &p_sdb_nh->key.ip_address, sizeof(sai_ip_address_t));
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_nexthop_get_ifid(
	_In_  stbl_nexthop_t 	        *p_sdb_nh,
	_Inout_  sai_attribute_t		*attr)
{
    attr->value.oid = p_sdb_nh->key.rif_id;
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_nexthop_db_init()
{
    stbl_sai_glb_t               *p_sai_glb = stbl_sai_glb_get_sai_glb();
	sai_status_t                 ret = SAI_STATUS_SUCCESS;

    p_sai_glb->nh_max_count = GLB_MAX_CTC_NEXTHOP_ID;
    CTC_SAI_ERROR_GOTO(ctclib_opb_create(&p_sai_glb->nexthop_id_opb, 3000, p_sai_glb->nh_max_count, "nexthop_id"), ret, out);

out:
    return ret;

	goto out;
}

#define ________SAI_SAI_DEBUG_FUNC
sai_status_t
ctc_sai_create_next_hop_debug_param(
    _Out_ sai_object_id_t* next_hop_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list)
{
    CTC_SAI_DEBUG("in:attr_count %u", attr_count);
    CTC_SAI_DEBUG("out:next_hop_id 0x%llx", (*next_hop_id));

    return SAI_STATUS_SUCCESS;
}
sai_status_t
ctc_sai_remove_next_hop_debug_param(
    _In_ sai_object_id_t next_hop_id)
{
    CTC_SAI_DEBUG("in:next_hop_id %u", next_hop_id);

    return SAI_STATUS_SUCCESS;
}
sai_status_t
ctc_sai_get_next_hop_attribute_debug_param(
    _In_ sai_object_id_t next_hop_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    sai_status_t     ret         = SAI_STATUS_SUCCESS;
    uint32_t         attr_idx    = 0;
    sai_attribute_t* attr        = NULL;

    CTC_SAI_DEBUG("in:next_hop_id 0x%llx attr_count %u", next_hop_id, attr_count);
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
        case SAI_NEXT_HOP_ATTR_TYPE:
            CTC_SAI_DEBUG("out:SAI_NEXT_HOP_ATTR_TYPE %d", attr->value.s32);
            break;
        case SAI_NEXT_HOP_ATTR_IP:
            CTC_SAI_DEBUG("out:SAI_NEXT_HOP_ATTR_IP");
            break;
        case SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID:
            CTC_SAI_DEBUG("out:SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID 0x%llx", attr->value.oid);
            break;
        }

        if(ret != SAI_STATUS_SUCCESS)
        {
            break;
        }
    }

    return SAI_STATUS_SUCCESS;
}

#define ________SAI_SAI_API_FUNC
/*
* Routine Description:
*    Create next hop
*
* Arguments:
*    [out] next_hop_id - next hop id
*    [in] attr_count - number of attributes
*    [in] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*
* Note: IP address expected in Network Byte Order.
*/
sai_status_t
ctc_sai_create_next_hop(
    _Out_ sai_object_id_t* next_hop_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    sai_status_t                ret = SAI_STATUS_SUCCESS;
    ctc_sai_attr_entry_list_t   *pattr_entry_list   = NULL;
    stbl_rif_t                  *p_sdb_rif          = NULL;
    stbl_neighbor_t             *p_sdb_nb           = NULL;
    stbl_neighbor_key_t         nb_key;
    stbl_nexthop_t              sdb_nh;
    stbl_nexthop_t              *p_sdb_nh           = NULL;
    uint32                      index;

    CTC_SAI_DEBUG_FUNC();

    CTC_SAI_PTR_VALID_CHECK(next_hop_id);
    CTC_SAI_PTR_VALID_CHECK(attr_list);

    CTC_SAI_ERROR_GOTO(ctc_sai_attr_alloc_attr_entry_list(g_sai_attr_entries,
                            attr_list,
                            attr_count,
                            &pattr_entry_list), ret, out);

    CTC_SAI_ERROR_GOTO(ctc_sai_attr_check_attr_entry_list(g_sai_attr_entries,
                            pattr_entry_list), ret, out);

    sal_memset(&nb_key, 0, sizeof(nb_key));
    sal_memset(&sdb_nh, 0, sizeof(sdb_nh));

    sdb_nh.key.rif_id = pattr_entry_list[SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID].value.oid;
    sal_memcpy(&sdb_nh.key.ip_address, &pattr_entry_list[SAI_NEXT_HOP_ATTR_IP].value.ipaddr,sizeof(sai_ip_address_t));
    CTC_SAI_ERROR_GOTO(ctclib_opb_alloc_offset(&p_sai_glb->nexthop_id_opb, &index), ret, out);
    sdb_nh.nh_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_NEXT_HOP, index);
    /* 2. update nexthop para */
    if(pattr_entry_list[SAI_NEXT_HOP_ATTR_TYPE].valid)
    {
        sdb_nh.nh_type = pattr_entry_list[SAI_NEXT_HOP_ATTR_TYPE].value.s32;
    }

    p_sdb_rif = ctc_routerintf_get_by_oid_no_ref(sdb_nh.key.rif_id);
    /* 3. update nexthop para */
    if (NULL == p_sdb_rif)
    {
        log_sys(M_MOD_SAI, E_ERROR, "Not found nexthop oid %"PRIu64 " %"PRIu64 " when creat next hop", sdb_nh.key.rif_id, sdb_nh.key.rif_id & 0xFFFFFFFF);
        ret = SAI_STATUS_ITEM_NOT_FOUND;
        goto out;
    }
    
    CTC_SAI_ERROR_GOTO(ctc_nexthop_add_entry(&sdb_nh, p_sdb_rif), ret, out);
    CTC_SAI_ERROR_GOTO(stbl_nexthop_add_nexthop(&sdb_nh), ret, out);
    p_sdb_nh = stbl_nexthop_get_nexthop(&sdb_nh.key);
    if (NULL == p_sdb_nh)
    {
        ret = SAI_STATUS_NO_MEMORY;
        goto out;
    }
    stbl_nexthop_add_array(p_sdb_nh);

    /* 4. return nexthop id*/
    *next_hop_id = sdb_nh.nh_oid;

    sal_memcpy(&nb_key.ip_address, &sdb_nh.key.ip_address, sizeof(sdb_nh.key.ip_address));
    nb_key.rif_id = sdb_nh.key.rif_id;
    p_sdb_nb = stbl_neighbor_get_neighbor(&nb_key);
    if (NULL != p_sdb_nb)
    {
        p_sdb_nb->nh_oid = sdb_nh.nh_oid;
        ctc_neighbor_update_entry(p_sdb_nb, SAI_NEIGHBOR_CREATE);
    }

    ctc_sai_create_next_hop_debug_param(next_hop_id, attr_count, attr_list);

out:
    if (NULL != pattr_entry_list)
    {
        ctc_sai_attr_free_attr_entry_list(pattr_entry_list);
    }
    return ret;
}

/*
* Routine Description:
*    Remove next hop
*
* Arguments:
*    [in] next_hop_id - next hop id
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_remove_next_hop(
    _In_ sai_object_id_t next_hop_id)
{
    sai_status_t             ret = SAI_STATUS_SUCCESS;
    stbl_nexthop_t          *p_sdb_nh = NULL;

    CTC_SAI_DEBUG_FUNC();
    ctc_sai_remove_next_hop_debug_param(next_hop_id);

    p_sdb_nh = ctc_nexthop_get_by_oid(next_hop_id);
    if (NULL == p_sdb_nh)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    ctc_nh_remove_ipuc(CTC_SAI_OBJECT_INDEX_GET(p_sdb_nh->nh_oid));
    ctc_nexthop_release(p_sdb_nh);
    return ret;
}

/*
* Routine Description:
*    Set Next Hop attribute
*
* Arguments:
*    [in] next_hop_id - next hop id
*    [in] attr - attribute
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_set_next_hop_attribute(
    _In_ sai_object_id_t next_hop_id,
    _In_ const sai_attribute_t *attr)
{
    CTC_SAI_DEBUG_FUNC();

    CTC_SAI_PTR_VALID_CHECK(attr);

	return SAI_STATUS_INVALID_ATTRIBUTE_0;
}

/*
* Routine Description:
*    Get Next Hop attribute
*
* Arguments:
*    [in] next_hop_id - next hop id
*    [in] attr_count - number of attributes
*    [inout] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_get_next_hop_attribute(
    _In_ sai_object_id_t next_hop_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    sai_status_t                ret         = SAI_STATUS_SUCCESS;
    sai_attribute_t*            attr        = NULL;
    uint32_t                    attr_idx    = 0;
    stbl_nexthop_t              *p_sdb_nh   = NULL;

    CTC_SAI_DEBUG_FUNC();

    CTC_SAI_PTR_VALID_CHECK(attr_list);

    p_sdb_nh = ctc_nexthop_get_by_oid(next_hop_id);
    if(NULL == p_sdb_nh)
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
        case SAI_NEXT_HOP_ATTR_TYPE:
            ret = ctc_nexthop_get_type(p_sdb_nh, attr);
            break;
        case SAI_NEXT_HOP_ATTR_IP:
            ret = ctc_nexthop_get_ip(p_sdb_nh, attr);
            break;
        case SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID:
            ret = ctc_nexthop_get_ifid(p_sdb_nh, attr);
            break;
        }

        if(ret != SAI_STATUS_SUCCESS)
        {
            break;
        }
    }
    ctc_sai_get_next_hop_attribute_debug_param(next_hop_id, attr_count, attr_list);
    return ret;
}

int32
stbl_nexthop_add_array(stbl_nexthop_t *p_nexthop)
{
    stbl_nexthop_master_t *p_master = stbl_nexthop_get_master();
    uint32 nh_id = CTC_SAI_OBJECT_INDEX_GET(p_nexthop->nh_oid);
    
    if (nh_id > GLB_MAX_CTC_NEXTHOP_ID)
    {
        return -1;
    }

    p_master->array[nh_id] = p_nexthop;
    return 0;
}

int32
stbl_nexthop_del_array(uint32 nh_id)
{
    stbl_nexthop_master_t *p_master = stbl_nexthop_get_master();
    if (nh_id > GLB_MAX_CTC_NEXTHOP_ID)
    {
        return -1;
    }
    p_master->array[nh_id] = NULL;
    
    return 0;
}

#define ________SAI_SAI_INNER_API_FUNC
static sai_status_t
__init_mode_fn(
    _In_  ctc_sai_api_reg_info_t    *preg,
    _In_  void                      *private_data)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;

    CTC_SAI_ERROR_GOTO(ctc_nexthop_db_init(),ret,out);

    preg->init_status =  INITIALIZED;

out:
    return ret;
}

static sai_status_t
__exit_mode_fn(
    _In_  ctc_sai_api_reg_info_t    *preg,
    _In_  void                      *private_data)
{
    preg->init_status = UNINITIALIZED;
    return SAI_STATUS_SUCCESS;
}

#define ________SAI_SAI_API_FUNC

static sai_next_hop_api_t      g_sai_api_func = {
        .create_next_hop            = ctc_sai_create_next_hop,
        .remove_next_hop            = ctc_sai_remove_next_hop,
        .set_next_hop_attribute     = ctc_sai_set_next_hop_attribute,
        .get_next_hop_attribute     = ctc_sai_get_next_hop_attribute,
};

static ctc_sai_api_reg_info_t g_api_reg_info = {
        .id  = SAI_API_NEXT_HOP,
        .init_func = __init_mode_fn,
        .exit_func = __exit_mode_fn,
        .api_method_table = &g_sai_api_func,
        .private_data     = NULL,
};

#define ________SAI_OUTER_FUNC
sai_status_t
ctc_sai_nexthop_init()
{
    api_reg_register_fn(&g_api_reg_info);

    return SAI_STATUS_SUCCESS;
}
