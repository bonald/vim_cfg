#include <sal.h>
#include <ctc_sai_common.h>
#include <ctc_sai_common_attr.h>
#include <ctc_sai_stp.h>
#include <ctc_sai_sai.h>
#include <ctc_vector.h>
#include <ctc_api.h>
#include <ctc_sai_debug.h>

#define CTC_SAI_STP_DEFAULT_INSTANCE    0

static ctc_sai_attr_entry_info_t g_sai_attr_entries[] = {
    {
        .id     = SAI_STP_ATTR_VLAN_LIST,
        .type   = SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_ATTR_ID_END,
        .type   = 0,
    },
};

#define ________SAI_SAI_INNER_API_FUNC

sai_status_t
ctc_sai_stp_change_vlan(
    _In_ stbl_stp_t* p_sdb_stp,
    _In_ const sai_vlan_id_t vlan_id,
    _In_ uint32 is_add)
{
    stbl_stp_key_t key;
    stbl_stp_t *p_default_sdb_stp = NULL;

    key.stp_id = CTC_SAI_STP_DEFAULT_INSTANCE;
    p_default_sdb_stp = stbl_stp_get_stp(&key);
    if (NULL == p_default_sdb_stp)
    {
        return SAI_STATUS_FAILURE;
    }
           
    if (is_add)
    {
        ctc_sai_stp_add_vlan(p_sdb_stp, vlan_id);
        ctc_sai_stp_remove_vlan(p_default_sdb_stp, vlan_id);
    }
    else
    {
        ctc_sai_stp_remove_vlan(p_sdb_stp, vlan_id);
        ctc_sai_stp_add_vlan(p_default_sdb_stp, vlan_id);
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t
ctc_sai_stp_db_init()
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    p_sai_glb->stp_id_max_count = GLB_STP_INSTANCE_MAX;

    ctclib_opb_create(&p_sai_glb->stpid_opb, 0, p_sai_glb->stp_id_max_count, "stpid");
    return SAI_STATUS_SUCCESS;
}

static sai_status_t
ctc_sai_stp_remove_entry(
    _In_ stbl_stp_t *p_sdb_stp)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    sai_vlan_id_t vlan_id = 0;

    for (vlan_id = 1; vlan_id < CTC_SAI_VLAN_ID_MAX; vlan_id++)
    {
        if (CTC_BMP_ISSET(p_sdb_stp->vlan_bmp, vlan_id))
        {
            ctc_sai_stp_change_vlan(p_sdb_stp, vlan_id, FALSE);
        }
    }
    
    ctclib_opb_free_offset(&p_sai_glb->stpid_opb, p_sdb_stp->key.stp_id);
    stbl_stp_del_stp(&p_sdb_stp->key);

    return SAI_STATUS_SUCCESS;
}

stbl_stp_t*
ctc_sai_stp_get_by_oid(
    _In_  const sai_object_id_t stp_oid)
{
    stbl_stp_key_t key;
    key.stp_id = CTC_SAI_OBJECT_INDEX_GET(stp_oid);
    return stbl_stp_get_stp(&key);
}

sai_status_t
ctc_sai_stp_add_vlan(
    _In_  stbl_stp_t*          p_sdb_stp,
    _In_  const sai_vlan_id_t  vlan_id)
{
    int32_t    sdk_ret     = 0;

    CTC_SAI_ERROR_GOTO(ctc_stp_set_vlan_stpid(vlan_id, p_sdb_stp->key.stp_id), sdk_ret,out);
    GLB_BMP_SET(p_sdb_stp->vlan_bmp, vlan_id);

out:
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_sai_stp_remove_vlan(
    _In_ stbl_stp_t*           p_sdb_stp,
    _In_  const sai_vlan_id_t  vlan_id)
{
    int32_t    sdk_ret     = 0;

    GLB_BMP_UNSET(p_sdb_stp->vlan_bmp, vlan_id);
    
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_sai_stp_set_port_state(
    _In_  stbl_stp_t*               p_sdb_stp,
    _In_  sai_object_id_t           port_id,
    _In_  sai_port_stp_port_state_t stp_port_state)
{
    int32_t     sdk_ret     = 0;
    uint8_t     stp_state   = 0;

    switch(stp_port_state)
    {
    case SAI_PORT_STP_STATE_LEARNING:
        stp_state = CTC_STP_LEARNING;
        break;
    case SAI_PORT_STP_STATE_FORWARDING:
        stp_state = CTC_STP_FORWARDING;
        break;
    case SAI_PORT_STP_STATE_BLOCKING:
        stp_state = CTC_STP_BLOCKING;
        break;
    }

    sdk_ret = ctc_stp_set_state(CTC_SAI_OBJECT_INDEX_GET(port_id), p_sdb_stp->key.stp_id, stp_state);
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_sai_stp_get_port_state(
    _In_  stbl_stp_t*                p_sdb_stp,
    _In_  sai_object_id_t            port_id,
    _Out_ sai_port_stp_port_state_t* stp_port_state)
{
    int32_t     sdk_ret     = 0;
    uint8_t     stp_state   = 0;

    CTC_SAI_ERROR_GOTO(
        ctc_stp_get_state(CTC_SAI_OBJECT_INDEX_GET(port_id), p_sdb_stp->key.stp_id, &stp_state), sdk_ret, out);

    switch(stp_state)
    {
    case CTC_STP_LEARNING:
        *stp_port_state = SAI_PORT_STP_STATE_LEARNING;
        break;
    case CTC_STP_FORWARDING:
        *stp_port_state = SAI_PORT_STP_STATE_FORWARDING;
        break;
    case CTC_STP_BLOCKING:
        *stp_port_state = SAI_PORT_STP_STATE_BLOCKING;
        break;
    }

out:
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

static sai_status_t
ctc_sai_stp_get_vlans(
    _In_    stbl_stp_t*          p_sdb_stp,
    _Inout_ sai_attribute_t*     attr)
{
    uint32_t    vlan_idx = 1;
    uint32_t    vlan_count_index = 0;

    for (vlan_idx = 1; vlan_idx < CTC_SAI_VLAN_ID_MAX; vlan_idx++)
    {
        if (GLB_BMP_ISSET(p_sdb_stp->vlan_bmp, vlan_idx))
        {
            if (vlan_count_index >= attr->value.vlanlist.vlan_count)
            {
                break;
            }
            attr->value.vlanlist.vlan_list[vlan_count_index] = vlan_idx;
            vlan_count_index++;
        }
    }

    attr->value.vlanlist.vlan_count = vlan_count_index;

    return SAI_STATUS_SUCCESS;
}

#define ________SAI_SAI_API_FUNC

/**
 * @brief Create stp instance.
 *
 * @param[out] inst_id stp instance id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Value of attributes
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t
ctc_sai_create_stp_inst(
    _Out_ sai_object_id_t *inst_id,
    _In_  uint32_t attr_count,
    _In_  const sai_attribute_t *attr_list)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    stbl_stp_t      stp;
    uint32_t        index;
    sai_status_t    ret = SAI_STATUS_SUCCESS;

    CTC_SAI_DEBUG_FUNC();
    
    if (0 != attr_count)
    {
        return SAI_STATUS_NOT_IMPLEMENTED;
    }
    
    CTC_SAI_ERROR_GOTO(ctclib_opb_alloc_offset(&p_sai_glb->stpid_opb, &index), ret, out);

    sal_memset(&stp, 0, sizeof(stp));
    stp.key.stp_id = index;
    stp.stp_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_STP_INSTANCE, index);
    stbl_stp_add_stp(&stp);
    
    *inst_id = stp.stp_oid;

out:
    return ret;
}

/**
 * @brief Remove stp instance.
 *
 * @param[in] inst_id stp instance id
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t
ctc_sai_remove_stp_inst(
    _In_ sai_object_id_t inst_id)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    stbl_stp_t      *p_sdb_stp = NULL;

    CTC_SAI_DEBUG_FUNC();

    p_sdb_stp = ctc_sai_stp_get_by_oid(inst_id);
    if (NULL == p_sdb_stp)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    ret = ctc_sai_stp_remove_entry(p_sdb_stp);
    return ret;
}

/**
 * @brief Update stp state of a port in specified stp instance.
 *
 * @param[in] inst_id stp instance id
 * @param[in] port_id port id
 * @param[in] stp_port_state stp state of the port
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t
ctc_sai_set_stp_port_state(
    _In_ sai_object_id_t inst_id,
    _In_ sai_object_id_t port_id,
    _In_ sai_port_stp_port_state_t stp_port_state)
{
    stbl_stp_t      *p_sdb_stp = NULL;

    CTC_SAI_DEBUG_FUNC();

    p_sdb_stp = ctc_sai_stp_get_by_oid(inst_id);
    if (NULL == p_sdb_stp)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    return ctc_sai_stp_set_port_state(p_sdb_stp, port_id, stp_port_state);
}

/**
 * @brief Retrieve stp state of a port in specified stp instance.
 *
 * @param[in] inst_id stp instance id
 * @param[in] port_id port id
 * @param[out] stp_port_state stp state of the port
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t
ctc_sai_get_stp_port_state(
    _In_  sai_object_id_t inst_id,
    _In_  sai_object_id_t port_id,
    _Out_ sai_port_stp_port_state_t  *stp_port_state)
{
    stbl_stp_t      *p_sdb_stp = NULL;
    
    CTC_SAI_DEBUG_FUNC();

    p_sdb_stp = ctc_sai_stp_get_by_oid(inst_id);
    if (!p_sdb_stp)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    return ctc_sai_stp_get_port_state(p_sdb_stp, port_id, stp_port_state);
}

/**
 * @brief Set the attribute of STP instance.
 *
 * @param[in] inst_id stp instance id
 * @param[in] attr attribute value
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t
ctc_sai_set_stp_attribute(
    _In_ sai_object_id_t inst_id,
    _In_ const sai_attribute_t *attr)
{
    CTC_SAI_DEBUG_FUNC();

    return SAI_STATUS_INVALID_ATTRIBUTE_0;
}

/**
 * @brief Get the attribute of STP instance.
 *
 * @param[in] inst_id stp instance id
 * @param[in] attr_count number of the attribute
 * @param[in] attr_list attribute value
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t
ctc_sai_get_stp_attribute(
    _In_ sai_object_id_t inst_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    sai_status_t            ret         = SAI_STATUS_SUCCESS;
    stbl_stp_t*             p_sdb_stp = NULL;
    sai_attribute_t*        attr        = NULL;
    uint32_t                attr_idx    = 0;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_PTR_VALID_CHECK(attr_list);

    p_sdb_stp = ctc_sai_stp_get_by_oid(inst_id);

    CTC_SAI_DEBUG_FUNC();

    if (!p_sdb_stp)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    for(attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = attr_list + attr_idx;
        ret  = ctc_sai_attr_check_read_attr(g_sai_attr_entries,attr);

        if(ret != SAI_STATUS_SUCCESS)
        {
            return ret + attr_idx;
        }

        switch(attr->id)
        {
        case SAI_STP_ATTR_VLAN_LIST:
            ret = ctc_sai_stp_get_vlans(p_sdb_stp, attr);
            break;
        }

        if(ret != SAI_STATUS_SUCCESS)
        {
            break;
        }
    }

    return ret;
}

#define ________SAI_INNER_FUNC
sai_status_t
__init_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;

    CTC_SAI_ERROR_GOTO(ctc_sai_stp_db_init(),ret,out);

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

static sai_stp_api_t      g_sai_api_func = {
    .create_stp        = ctc_sai_create_stp_inst,
    .remove_stp        = ctc_sai_remove_stp_inst,
    .set_stp_port_state     = ctc_sai_set_stp_port_state,
    .get_stp_port_state     = ctc_sai_get_stp_port_state,
    .set_stp_attribute      = ctc_sai_set_stp_attribute,
    .get_stp_attribute      = ctc_sai_get_stp_attribute,
};

static ctc_sai_api_reg_info_t g_api_reg_info = {
        .id  = SAI_API_STP,
        .init_func = __init_mode_fn,
        .exit_func = __exit_mode_fn,
        .api_method_table = &g_sai_api_func,
        .private_data     = NULL,
};

#define ________SAI_OUTER_FUNC
sai_status_t
ctc_sai_stp_init()
{
    api_reg_register_fn(&g_api_reg_info);

    return SAI_STATUS_SUCCESS;
}

