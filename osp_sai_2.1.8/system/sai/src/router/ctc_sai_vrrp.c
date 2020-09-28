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
#include <ctc_sai_vrrp.h>

static ctc_sai_attr_entry_info_t g_sai_attr_entries[] = {
    {
        .id     = SAI_VRRP_ATTR_VMAC_ADDRESS,
        .type   = SAI_ATTR_FALG_CREATE | SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_VRRP_ATTR_IF_MAC_ADDRESS,
        .type   = SAI_ATTR_FALG_CREATE | SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_ATTR_ID_END,
        .type   = 0,
    },
};

stbl_vrrp_vip_t*
ctc_vrrp_vip_get_by_key(
    _In_  const sai_vrrp_vip_entry_t* pkey)
{
    stbl_vrrp_vip_key_t key;

    key = pkey->ip_address;
    
    return stbl_vrrp_vip_get_vrrp_vip(&key);
}

/*
* Routine Description:
*    Create vrrp vip entry
*
* Arguments:
*    [in] vrrp_vip_entry - vrrp vip entry
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
ctc_sai_create_vrrp_vip_entry(
    _In_ const sai_vrrp_vip_entry_t* vrrp_vip_entry,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    stbl_vrrp_vip_master_t *p_master = stbl_vrrp_vip_get_master();
    stbl_vrrp_vip_t             vip;
    stbl_vrrp_vip_t             *p_sdb_vip = NULL;
    ctc_sai_attr_entry_list_t   *pattr_entry_list   = NULL;
    sai_status_t                ret = SAI_STATUS_SUCCESS;

    CTC_SAI_ERROR_GOTO(ctc_sai_attr_alloc_attr_entry_list(g_sai_attr_entries,
                            attr_list,
                            attr_count,
                            &pattr_entry_list),ret,out);

    CTC_SAI_ERROR_GOTO(ctc_sai_attr_check_attr_entry_list(g_sai_attr_entries,
                            pattr_entry_list),ret,out);

    /* 1. check vip exist */
    p_sdb_vip = ctc_vrrp_vip_get_by_key(vrrp_vip_entry);
    if (NULL != p_sdb_vip)
    {
        ret = SAI_STATUS_ITEM_ALREADY_EXISTS;
        goto out;
    }

    if (p_master->vip_hash->count >= p_sai_glb->vrrp_max_count)
    {
        ret = SAI_STATUS_TABLE_FULL;
        goto out;
    }
    
    /* 2. set vip parameter */
    sal_memset(&vip, 0, sizeof(vip));
    sal_memcpy(&vip.key, &vrrp_vip_entry->ip_address, sizeof(sai_ip_address_t));
    sal_memcpy(vip.vmac, pattr_entry_list[SAI_VRRP_ATTR_VMAC_ADDRESS].value.mac, sizeof(mac_addr_t));
    sal_memcpy(vip.if_mac, pattr_entry_list[SAI_VRRP_ATTR_IF_MAC_ADDRESS].value.mac, sizeof(mac_addr_t));

    stbl_vrrp_vip_add_vrrp_vip(&vip);
out:
    if(pattr_entry_list)
    {
        ctc_sai_attr_free_attr_entry_list(pattr_entry_list);
    }

    return ret;    
}

/*
* Routine Description:
*    Remove vrrp vip entry
*
* Arguments:
*    [in] vrrp_vip_entry - vrrp vip entry
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*
* Note: IP address expected in Network Byte Order.
*/
sai_status_t
ctc_sai_remove_vrrp_vip_entry(
    _In_ const sai_vrrp_vip_entry_t* vrrp_vip_entry)
{
    stbl_vrrp_vip_t             *p_sdb_vip = NULL;

    CTC_SAI_DEBUG_FUNC();

    CTC_SAI_PTR_VALID_CHECK(vrrp_vip_entry);

    p_sdb_vip = ctc_vrrp_vip_get_by_key(vrrp_vip_entry);
    if (NULL == p_sdb_vip)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    stbl_vrrp_vip_del_vrrp_vip(&p_sdb_vip->key);
    
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
ctc_vrrp_init()
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    sai_status_t ret        = SAI_STATUS_SUCCESS;

    p_sai_glb->vrrp_max_count = CTC_SAI_VRRP_MAX;

    return CTC_SAI_NOTIFY_DONE;

    return ret;
}

static sai_status_t
__init_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;

    CTC_SAI_ERROR_GOTO(ctc_vrrp_init(),ret,out);

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
static sai_vrrp_api_t      g_sai_api_func = {
    .create_vrrp_vip_entry      = ctc_sai_create_vrrp_vip_entry,
    .remove_vrrp_vip_entry      = ctc_sai_remove_vrrp_vip_entry,
};

static ctc_sai_api_reg_info_t g_api_vrrp_reg_info = {
        .id         = SAI_API_VRRP,
        .init_func  = __init_mode_fn,
        .exit_func  = __exit_mode_fn,
        .api_method_table = &g_sai_api_func,
        .private_data     = NULL,
};

#define ________SAI_OUTER_FUNC
sai_status_t
ctc_sai_vrrp_init()
{
    api_reg_register_fn(&g_api_vrrp_reg_info);

    return SAI_STATUS_SUCCESS;
}

