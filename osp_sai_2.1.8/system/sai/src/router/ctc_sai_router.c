#include <sai.h>
#include <sal.h>
#include "ctc_api.h"
#include <ctc_sai_common.h>
#include <ctc_sai_common_attr.h>
#include <ctc_sai_notifier_call.h>
#include <ctc_sai_router.h>
#include <ctc_sai_sai.h>
#include "ctc_sai_routerintf.h"
#include <ctc_sai_debug.h>

#define CTC_SAI_ROUTER_BLOCK_SIZE   16

static ctc_sai_attr_entry_info_t g_sai_attr_entries[] = {
    {
        .id     = SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V4_STATE,
        .type   = SAI_ATTR_FALG_WRITE| SAI_ATTR_FALG_READ | SAI_ATTR_FLAG_DEFAULT,
        .default_value.booldata = false,
    },
    {
        .id     = SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V6_STATE,
        .type   = SAI_ATTR_FALG_WRITE| SAI_ATTR_FALG_READ | SAI_ATTR_FLAG_DEFAULT,
        .default_value.booldata = false,
    },
    {
        .id     = SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS,
        .type   = SAI_ATTR_FALG_WRITE| SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_VIRTUAL_ROUTER_ATTR_VIOLATION_TTL1_ACTION,
        .type   = SAI_ATTR_FALG_WRITE| SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_VIRTUAL_ROUTER_ATTR_VIOLATION_IP_OPTIONS,
        .type   = SAI_ATTR_FALG_WRITE| SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_ATTR_ID_END,
        .type   = 0,
    },
};

#define ________SAI_SAI_INNER_API_FUNC
sai_status_t
ctc_vr_init()
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    sai_status_t ret        = SAI_STATUS_SUCCESS;

    p_sai_glb->vrf_max_count = CTC_MAX_VRFID_ID;
    if (0 != ctclib_opb_create(&p_sai_glb->vrf_id_opb, 0, p_sai_glb->vrf_max_count, "vrf_id"))
    {
        return SAI_STATUS_NO_MEMORY;
    }

    return ret;
}


sai_status_t
ctc_vr_alloc(
	_Out_ stbl_vr_t** pp_sdb_vr)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    stbl_vr_t       vr;
    stbl_vr_t 	    *p_sdb_vr = NULL;
    uint32_t        index = 0;
    sai_status_t    ret = SAI_STATUS_SUCCESS;

    sal_memset(&vr, 0, sizeof(vr));
    ctclib_opb_alloc_offset(&p_sai_glb->vrf_id_opb, &index);
    vr.key.vrf_id = index;
    vr.vr_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_VIRTUAL_ROUTER,index);
    stbl_vr_add_vr(&vr);

    p_sdb_vr = stbl_vr_get_vr(&vr.key);
    *pp_sdb_vr = p_sdb_vr;

    return ret;
}

void
ctc_vr_release(
	_In_  stbl_vr_t *p_sdb_vr)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    if (NULL == p_sdb_vr)
    {
        return;
    }
    ctclib_opb_free_offset(&p_sai_glb->vrf_id_opb, p_sdb_vr->key.vrf_id);
    stbl_vr_del_vr(&p_sdb_vr->key);

    return;
}

stbl_vr_t*
ctc_vr_get_by_oid_no_ref(
    _In_  const sai_object_id_t vr_oid)
{
    stbl_vr_key_t key;

    key.vrf_id = CTC_SAI_OBJECT_INDEX_GET(vr_oid);

    return stbl_vr_get_vr(&key);
}

stbl_vr_t*
ctc_vr_get_by_oid(
	_In_  const sai_object_id_t vr_oid)
{
    return ctc_vr_get_by_oid_no_ref(vr_oid);
}

sai_status_t
ctc_vr_add_router_intf(
	_In_  sai_object_id_t vr_oid,
	_In_  stbl_rif_t *p_sdb_rif)
{
    stbl_vr_key_t key;
    stbl_vr_t *p_sdb_vr = NULL;

    key.vrf_id = CTC_SAI_OBJECT_INDEX_GET(vr_oid);
    p_sdb_vr = stbl_vr_get_vr(&key);
    if (NULL == p_sdb_vr)
    {
        return -1;
    }
    return stbl_vr_set_rif_list(p_sdb_vr, TRUE, p_sdb_rif);
}

sai_status_t
ctc_vr_remove_router_intf(
	_In_  sai_object_id_t vr_oid,
	_In_  stbl_rif_t *p_sdb_rif)
{
    stbl_vr_key_t key;
    stbl_vr_t *p_sdb_vr = NULL;
    
    key.vrf_id = CTC_SAI_OBJECT_INDEX_GET(vr_oid);
    p_sdb_vr = stbl_vr_get_vr(&key);
    if (NULL == p_sdb_vr)
    {
        return -1;
    }
    return stbl_vr_set_rif_list(p_sdb_vr, FALSE, p_sdb_rif);
}

sai_status_t
ctc_vr_set_v4_admin(
	_In_  stbl_vr_t             *p_sdb_vr,
	_In_  const sai_attribute_t *attr)
{
    stbl_rif_t 	                *p_sdb_rif 	= NULL;
    ctclib_slistnode_t *        listnode = NULL;
    sai_attribute_t             tmp_attr;

    p_sdb_vr->v4_admin = attr->value.booldata;

	sal_memset(&tmp_attr,0,sizeof(sai_attribute_t));

    CTCLIB_SLIST_LOOP(p_sdb_vr->rif_list.obj_list, p_sdb_rif, listnode)
    {
        tmp_attr.value.booldata = p_sdb_vr->v4_admin;
        ctc_routerintf_set_v4_admin(p_sdb_rif,&tmp_attr);
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_vr_set_v6_admin(
	_In_  stbl_vr_t             *p_sdb_vr,
	_In_  const sai_attribute_t *attr)
{
    stbl_rif_t 	                *p_sdb_rif 	= NULL;
    ctclib_slistnode_t *        listnode = NULL;
    sai_attribute_t             tmp_attr;

    p_sdb_vr->v6_admin = attr->value.booldata;

	sal_memset(&tmp_attr, 0, sizeof(tmp_attr));

    CTCLIB_SLIST_LOOP(p_sdb_vr->rif_list.obj_list, p_sdb_rif, listnode)
    {
        tmp_attr.value.booldata = p_sdb_vr->v6_admin;
        ctc_routerintf_set_v6_admin(p_sdb_rif, &tmp_attr);
    }
    return SAI_STATUS_SUCCESS;
}

#define ________SAI_SAI_DEBUG_FUNC

sai_status_t
ctc_sai_set_virtual_router_attribute_debug_param(
    _In_ sai_object_id_t vr_id,
    _In_ const sai_attribute_t *attr)
{
    CTC_SAI_DEBUG("in:vr_id %u", vr_id);
    switch(attr->id)
    {
    case SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V4_STATE:
        CTC_SAI_DEBUG("in:SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V4_STATE");
        break;
    case SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V6_STATE:
        CTC_SAI_DEBUG("in:SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V6_STATE");
        break;
    case SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS:
    case SAI_VIRTUAL_ROUTER_ATTR_VIOLATION_TTL1_ACTION:
    case SAI_VIRTUAL_ROUTER_ATTR_VIOLATION_IP_OPTIONS:
        CTC_SAI_DEBUG("in:NOT SUPPORT");
        break;
    }
    return SAI_STATUS_SUCCESS;
}
sai_status_t
ctc_sai_get_virtual_router_attribute_debug_param(
    _In_ sai_object_id_t vr_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    uint32_t                    attr_idx    = 0;
    const sai_attribute_t *attr = NULL;

    CTC_SAI_DEBUG("in:vr_id 0x%llx attr_count %u", vr_id, attr_count);
    for (attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = attr_list + attr_idx;
        switch (attr->id)
        {
        case SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V4_STATE:
            CTC_SAI_DEBUG("in:SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V4_STATE");
            break;
        case SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V6_STATE:
            CTC_SAI_DEBUG("in:SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V6_STATE %u", attr->value.booldata);
            break;
        case SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS:
        case SAI_VIRTUAL_ROUTER_ATTR_VIOLATION_TTL1_ACTION:
        case SAI_VIRTUAL_ROUTER_ATTR_VIOLATION_IP_OPTIONS:
            CTC_SAI_DEBUG("in:NOT SUPPORT");
            break;
        }
    }

    return SAI_STATUS_SUCCESS;
}
#define ________SAI_SAI_API_FUNC
/*
* Routine Description:
*    Create virtual router
*
* Arguments:
*    [out] vr_id - virtual router id
*    [in] attr_count - number of attributes
*    [in] attr_list - array of attributes
*
* Return Values:
*  - SAI_STATUS_SUCCESS on success
*  - SAI_STATUS_ADDR_NOT_FOUND if neither SAI_SWITCH_ATTR_SRC_MAC_ADDRESS nor
*    SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS is set.
*/
sai_status_t
ctc_sai_create_virtual_router(
    _Out_ sai_object_id_t *vr_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list)
{
    sai_status_t                 ret 		= SAI_STATUS_SUCCESS;
    ctc_sai_attr_entry_list_t    *pattr_entry_list 	= NULL;
    stbl_vr_t                    *p_sdb_vr          = NULL;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_PTR_VALID_CHECK(vr_id);

    CTC_SAI_ERROR_GOTO(ctc_sai_attr_alloc_attr_entry_list(g_sai_attr_entries,
                            attr_list,
                            attr_count,
                            &pattr_entry_list),ret,out);

    CTC_SAI_ERROR_GOTO(ctc_sai_attr_check_attr_entry_list(g_sai_attr_entries,
                            pattr_entry_list),ret,out);

    CTC_SAI_ERROR_GOTO(ctc_vr_alloc(&p_sdb_vr),ret,out);

    *vr_id = p_sdb_vr->vr_oid;
    
    if(pattr_entry_list[SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS].valid)
    {
        sal_memcpy(p_sdb_vr->smac,
            pattr_entry_list[SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS].value.mac,
            sizeof(mac_addr_t));
    }

    if(pattr_entry_list[SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V4_STATE].valid)
    {
        p_sdb_vr->v4_admin =
            pattr_entry_list[SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V4_STATE].value.booldata;
    }

    if(pattr_entry_list[SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V6_STATE].valid)
    {
        p_sdb_vr->v6_admin =
            pattr_entry_list[SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V6_STATE].value.booldata;
    }

out:
    if(pattr_entry_list)
    {
        ctc_sai_attr_free_attr_entry_list(pattr_entry_list);
    }
    return ret;
}

/*
* Routine Description:
*    Remove virtual router
*
* Arguments:
*    [in] vr_id - virtual router id
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_remove_virtual_router(
    _In_ sai_object_id_t vr_id)
{
    sai_status_t         ret 		= SAI_STATUS_SUCCESS;
    stbl_vr_t            *p_sdb_vr  = NULL;

    CTC_SAI_DEBUG_FUNC();
    
    p_sdb_vr = ctc_vr_get_by_oid_no_ref(vr_id);
    if(NULL == p_sdb_vr)
    {
        ret = SAI_STATUS_ITEM_NOT_FOUND;
        goto out;
    }

    ctc_vr_release(p_sdb_vr);
out:
    return ret;
}

/*
* Routine Description:
*    Set virtual router attribute Value
*
* Arguments:
*    [in] vr_id - virtual router id
*    [in] attr - attribute
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_set_virtual_router_attribute(
    _In_ sai_object_id_t vr_id,
    _In_ const sai_attribute_t *attr)
{
    sai_status_t                ret 		= SAI_STATUS_SUCCESS;
    stbl_vr_t                  *p_sdb_vr 	= NULL;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_PTR_VALID_CHECK(attr);

    CTC_SAI_ERROR_GOTO(ctc_sai_attr_check_write_attr(g_sai_attr_entries,attr),ret,out);

    p_sdb_vr = ctc_vr_get_by_oid(vr_id);
    if(NULL == p_sdb_vr)
    {
        ret = SAI_STATUS_ITEM_NOT_FOUND;
        goto out;
    }

    ctc_sai_set_virtual_router_attribute_debug_param(vr_id, attr);
    switch(attr->id)
    {
    case SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V4_STATE:
        ret = ctc_vr_set_v4_admin(p_sdb_vr, attr);
        break;
    case SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V6_STATE:
        ret = ctc_vr_set_v6_admin(p_sdb_vr, attr);
        break;
    case SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS:
        ret = SAI_STATUS_NOT_SUPPORTED;
        break;
    case SAI_VIRTUAL_ROUTER_ATTR_VIOLATION_TTL1_ACTION:
        ret = SAI_STATUS_NOT_SUPPORTED;
        break;
    case SAI_VIRTUAL_ROUTER_ATTR_VIOLATION_IP_OPTIONS:
        ret = SAI_STATUS_NOT_SUPPORTED;
        break;
    }

    ctc_vr_release(p_sdb_vr);

out:
    return ret;
}

/*
* Routine Description:
*    Get virtual router attribute Value
*
* Arguments:
*    [in] vr_id - virtual router id
*    [in] attr_count - number of attributes
*    [in] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_get_virtual_router_attribute(
    _In_ sai_object_id_t vr_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{ 
    return SAI_STATUS_NOT_SUPPORTED;
}

#define ________SAI_INNER_FUNC
static sai_status_t
__init_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;

    CTC_SAI_ERROR_GOTO(ctc_vr_init(),ret,out);

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
static sai_virtual_router_api_t      g_sai_api_func = {
    .create_virtual_router              = ctc_sai_create_virtual_router,
    .remove_virtual_router              = ctc_sai_remove_virtual_router,
    .set_virtual_router_attribute       = ctc_sai_set_virtual_router_attribute,
    .get_virtual_router_attribute       = ctc_sai_get_virtual_router_attribute,
};

static ctc_sai_api_reg_info_t g_api_reg_info = {
        .id         = SAI_API_VIRTUAL_ROUTER,
        .init_func  = __init_mode_fn,
        .exit_func  = __exit_mode_fn,
        .api_method_table = &g_sai_api_func,
        .private_data     = NULL,
};

#define ________SAI_OUTER_FUNC
sai_status_t
ctc_sai_vr_init()
{
    api_reg_register_fn(&g_api_reg_info);

    return SAI_STATUS_SUCCESS;
}

