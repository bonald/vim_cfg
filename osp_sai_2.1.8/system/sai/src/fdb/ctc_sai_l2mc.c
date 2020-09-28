#include <sai.h>
#include <sal.h>
#include "ctc_api.h"
#include <ctc_sai_fdb.h>
#include <ctc_sai_common.h>
#include <ctc_sai_common_attr.h>
#include <ctc_sai_notifier_call.h>
#include <ctc_sai_sai.h>
#include <ctc_sai_port.h>
#include <ctc_sai_debug.h>

#undef  __MODULE__
#define __MODULE__ SAI_API_L2MC

sai_status_t
ctc_sai_l2mc_alloc_offset(uint32_t *popf_index)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    sai_status_t    ret         = SAI_STATUS_SUCCESS;
    uint32          opf_index   = 0;
    
    CTC_SAI_ERROR_GOTO_MAPSDKERR2SAI(
        ctclib_opb_alloc_offset(&p_sai_glb->l2mc_group_opb, &opf_index),ret,out);
    *popf_index = opf_index;
    
out:
    return ret;
}

sai_status_t
ctc_sai_l2mc_free_offset(uint32_t opf_index)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    sai_status_t    ret         = SAI_STATUS_SUCCESS;
    
    CTC_SAI_ERROR_GOTO_MAPSDKERR2SAI(
        ctclib_opb_free_offset(&p_sai_glb->l2mc_group_opb, opf_index),ret,out);
out:
    return ret;
}

int32
ctc_sai_mgroup_id_opf_alloc(uint32* p_index)
{
    return ctc_sai_l2mc_alloc_offset(p_index);
}

int32
ctc_sai_mgroup_id_opf_free(uint32 opf_index)
{
    return ctc_sai_l2mc_free_offset(opf_index);
}

stbl_l2mc_t*
ctc_sai_l2mc_db_get(const sai_l2mc_entry_t* p_l2mc_entry)
{
    stbl_l2mc_key_t key;

    sal_memset(&key, 0, sizeof(key));
    sal_memcpy(&key.mac_address, p_l2mc_entry->mac_address, MAC_ADDR_LEN);
    key.vlan_id = p_l2mc_entry->vlan_id;
    
    return stbl_l2mc_get_l2mc(&key);
}

sai_status_t
ctc_sai_l2mc_db_add(const sai_l2mc_entry_t* l2mc_entry, uint32 group_id)
{
    stbl_l2mc_t l2mc;

    /* 1. lookup entry exist */
    if (ctc_sai_l2mc_db_get(l2mc_entry))
    {
        return SAI_STATUS_ITEM_ALREADY_EXISTS;
    }

    sal_memset(&l2mc, 0, sizeof(l2mc));

    /* 3. evaluate db entry */
    sal_memcpy(&l2mc.key, l2mc_entry, sizeof(sai_l2mc_entry_t));
    l2mc.group_id = group_id;

    /* 4. add to db */
    stbl_l2mc_add_l2mc(&l2mc);

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_create_l2mc_entry(
    _In_ const sai_l2mc_entry_t* l2mc_entry,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    ctc_l2_mcast_addr_t l2_addr;
    uint32 opf_index = 0;
    int32           sdk_ret = SAI_STATUS_SUCCESS;
    sai_status_t    ret = SAI_STATUS_SUCCESS;
 
    sal_memset(&l2_addr, 0, sizeof(l2_addr));

    sal_memcpy(l2_addr.mac, l2mc_entry->mac_address, sizeof(mac_addr_t));
    l2_addr.fid = l2mc_entry->vlan_id;

    ret = ctclib_opb_alloc_offset(&p_sai_glb->l2mc_group_opb, &opf_index);
    if (ret < 0)
    {
        return SAI_STATUS_INSUFFICIENT_RESOURCES;
    }
    l2_addr.l2mc_grp_id = opf_index;

    ret = ctc_sai_l2mc_db_add(l2mc_entry, opf_index);
    if (ret < 0)
    {
        return ret;
    }

    sdk_ret = ctc_l2mcast_add_addr(&l2_addr);
    if (sdk_ret < 0)
    {
        return ctc_sai_get_error_from_sdk_error(sdk_ret);
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_create_l2mc_entry_with_flag(
    _In_ const sai_l2mc_entry_t* l2mc_entry,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list,
    _In_ uint32_t pkt_flag)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    ctc_l2_mcast_addr_t l2_addr;
    uint32 opf_index = 0;
    int32           sdk_ret = SAI_STATUS_SUCCESS;
    sai_status_t    ret = SAI_STATUS_SUCCESS;
 
    sal_memset(&l2_addr, 0, sizeof(l2_addr));

    sal_memcpy(l2_addr.mac, l2mc_entry->mac_address, sizeof(mac_addr_t));
    l2_addr.fid = l2mc_entry->vlan_id;

    ret = ctclib_opb_alloc_offset(&p_sai_glb->l2mc_group_opb, &opf_index);
    if (ret < 0)
    {
        return SAI_STATUS_INSUFFICIENT_RESOURCES;
    }
    l2_addr.l2mc_grp_id = opf_index;
    l2_addr.flag = pkt_flag;
    ret = ctc_sai_l2mc_db_add(l2mc_entry, opf_index);
    if (ret < 0)
    {
        return ret;
    }

    sdk_ret = ctc_l2mcast_add_addr(&l2_addr);
    if (sdk_ret < 0)
    {
        return ctc_sai_get_error_from_sdk_error(sdk_ret);
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_remove_l2mc_entry(
    _In_ const sai_l2mc_entry_t* l2mc_entry)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    stbl_l2mc_t     *p_sdb_l2mc = NULL;
    ctc_l2_mcast_addr_t l2_addr;
    int32           sdk_ret = SAI_STATUS_SUCCESS;
    sai_status_t    ret = SAI_STATUS_SUCCESS;
 
    sal_memset(&l2_addr, 0, sizeof(l2_addr));
    sal_memcpy(l2_addr.mac, l2mc_entry->mac_address, sizeof(mac_addr_t));
    l2_addr.fid = l2mc_entry->vlan_id;

    /* 1. lookup entry exist */
    p_sdb_l2mc = ctc_sai_l2mc_db_get(l2mc_entry);
    if (NULL == p_sdb_l2mc)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    
    ret = ctclib_opb_free_offset(&p_sai_glb->l2mc_group_opb, p_sdb_l2mc->group_id);
    if (ret < 0)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    stbl_l2mc_del_l2mc(&p_sdb_l2mc->key);

    sdk_ret = ctc_l2mcast_remove_addr(&l2_addr);
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_sai_add_ports_to_l2mc(
    _In_ const sai_l2mc_entry_t* l2mc_entry,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list)
{
    ctc_l2_mcast_addr_t l2_addr;
    sai_object_id_t port_oid = 0;
    int32           sdk_ret = SAI_STATUS_SUCCESS;
    sai_status_t    ret = SAI_STATUS_SUCCESS;

    if (0 == attr_count || NULL == attr_list)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    if (SAI_L2MC_ATTR_PORT_LIST != attr_list[0].id)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    if (0 == attr_list[0].value.objlist.count || NULL == attr_list[0].value.objlist.list)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    
    sal_memset(&l2_addr, 0, sizeof(l2_addr));

    port_oid = attr_list[0].value.objlist.list[0];
    sal_memcpy(l2_addr.mac, l2mc_entry->mac_address, sizeof(mac_addr_t));
    l2_addr.fid = l2mc_entry->vlan_id;
    ret = ctc_sai_port_objectid_to_gport(port_oid, &l2_addr.member.mem_port);
    if (ret < 0)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    sdk_ret = ctc_l2mcast_add_member(&l2_addr);
    if (sdk_ret < 0)
    {
        return ctc_sai_get_error_from_sdk_error(sdk_ret);
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_remove_ports_from_l2mc(
    _In_ const sai_l2mc_entry_t* l2mc_entry,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list)
{
    ctc_l2_mcast_addr_t l2_addr;
    sai_object_id_t port_oid = 0;
    int32           sdk_ret = SAI_STATUS_SUCCESS;
    sai_status_t    ret = SAI_STATUS_SUCCESS;

    if (0 == attr_count || NULL == attr_list)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    if (SAI_L2MC_ATTR_PORT_LIST != attr_list[0].id)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    if (0 == attr_list[0].value.objlist.count || NULL == attr_list[0].value.objlist.list)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    
    sal_memset(&l2_addr, 0, sizeof(l2_addr));

    port_oid = attr_list[0].value.objlist.list[0];
    sal_memcpy(l2_addr.mac, l2mc_entry->mac_address, sizeof(mac_addr_t));
    l2_addr.fid = l2mc_entry->vlan_id;
    ret = ctc_sai_port_objectid_to_gport(port_oid, &l2_addr.member.mem_port);
    if (ret < 0)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    sdk_ret = ctc_l2mcast_remove_member(&l2_addr);
    if (sdk_ret < 0)
    {
        return ctc_sai_get_error_from_sdk_error(sdk_ret);
    }

    return SAI_STATUS_SUCCESS;
}

#define ________SAI_L2MC_INNER_FUNC
static sai_status_t
__init_mode_fn(
    _In_  ctc_sai_api_reg_info_t    *preg,
    _In_  void                      *private_data)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;

    preg->init_status =  INITIALIZED;
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

/* define sai 0.9.2 */
static sai_l2mc_api_t      g_l2mc_api_func = {
    .create_l2mc_entry          = ctc_sai_create_l2mc_entry,
    .remove_l2mc_entry          = ctc_sai_remove_l2mc_entry,
    .add_ports_to_l2mc          = ctc_sai_add_ports_to_l2mc,
    .remove_ports_from_l2mc     = ctc_sai_remove_ports_from_l2mc,
    .set_l2mc_entry_attribute   = NULL,
    .get_l2mc_entry_attribute   = NULL,
    .create_l2mc_entry_with_flag = ctc_sai_create_l2mc_entry_with_flag,
};

static ctc_sai_api_reg_info_t g_l2mc_api_reg_info = {
        .id         = SAI_API_L2MC,
        .init_func  = __init_mode_fn,
        .exit_func  = __exit_mode_fn,
        .api_method_table = &g_l2mc_api_func,
        .private_data     = NULL,
};

#define ________SAI_L2MC_OUTER_FUNC

sai_status_t
ctc_sai_l2mc_init()
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    api_reg_register_fn(&g_l2mc_api_reg_info);

#ifdef OFPRODUCT
    if (0 != ctclib_opb_create(&p_sai_glb->l2mc_group_opb, 4096, 4504, "l2mc_id"))
#else
    if (0 != ctclib_opb_create(&p_sai_glb->l2mc_group_opb, 4096, 2048, "l2mc_id"))
#endif
    {
        return SAI_STATUS_NO_MEMORY;
    }

    return SAI_STATUS_SUCCESS;
}

