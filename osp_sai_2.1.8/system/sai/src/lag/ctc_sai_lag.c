#include <sai.h>
#include <ctc_port.h>
#include <sal.h>
#include <ctc_sai_common.h>
#include <ctc_sai_common_attr.h>
#include <ctc_sai_sai.h>
#include <ctc_sai_lag.h>
#include <ctc_sai_port.h>
#include <ctc_sai_acl.h>
#include <ctc_api.h>
#include <ctc_sai_debug.h>
#include <ctc_sai_nexthop.h>

static ctc_sai_attr_entry_info_t g_sai_attr_entries[] = {
    {
        .id     = SAI_LAG_ATTR_PORT_LIST,
        .type   = SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_ATTR_ID_END,
        .type   = 0,
    },
};

extern uint32 g_hash_value_global_agg_hash;

#define ________SAI_SAI_INNER_API_FUNC

sai_status_t
ctc_sai_lag_db_init()
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    
    p_sai_glb->lag_max_count = CTC_SAI_MAX_LINKAGG_GROUP_NUM;
    ctclib_opb_create(&p_sai_glb->lag_opb, 0, p_sai_glb->lag_max_count, "lag");
    
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_lag_create(
	_Out_ sai_object_id_t* lag_id,
	   _In_ uint32_t attr_count,
        _In_ sai_attribute_t *attr_list)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    uint32_t                        opf_index;
    int32_t                         sdk_ret     = 0;
    ctc_linkagg_group_t             linkagg_grp;

    if (attr_count < 1 || attr_list == NULL)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    
    sal_memset(&linkagg_grp,0,sizeof(ctc_linkagg_group_t));

    if (SAI_LAG_LOAD_BALANCE_DLB == attr_list->value.u32)
    {
        /*modified by yejl for bug 54178, 2019-10-30*/
        ctclib_opb_alloc_offset_position_reverse(&p_sai_glb->lag_opb, CTC_SAI_MAX_LINKAGG_DLB_GROUP_NUM-1, &opf_index); 
        linkagg_grp.linkagg_mode = CTC_LINKAGG_MODE_DLB;
    }
    else if (SAI_LAG_LOAD_BALANCE_RR == attr_list->value.u32)
    {
        ctclib_opb_alloc_offset_position_reverse(&p_sai_glb->lag_opb, CTC_SAI_MAX_LINKAGG_RR_GROUP_NUM-1, &opf_index);    
        linkagg_grp.linkagg_mode = CTC_LINKAGG_MODE_RR;
    }
    else if (SAI_LAG_LOAD_BALANCE_STATIC_FAILOVER == attr_list->value.u32)
    {
        ctclib_opb_alloc_offset_last_reverse(&p_sai_glb->lag_opb, &opf_index);    
        linkagg_grp.linkagg_mode =CTC_LINKAGG_MODE_STATIC; /* CTC_LINKAGG_MODE_STATIC_FAILOVER; modified by chenyq for bug 42560, chip bug*/
    }
#ifdef USW
    else if (SAI_LAG_LOAD_BALANCE_RESILIENT == attr_list->value.u32)
    {
        ctclib_opb_alloc_offset_last_reverse(&p_sai_glb->lag_opb, &opf_index);    
        linkagg_grp.linkagg_mode = CTC_LINKAGG_MODE_RESILIENT;    
    }
#endif
    else
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
     
    linkagg_grp.tid          = opf_index;
    *lag_id = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_LAG, opf_index);
    
    ctc_sai_acl_lag_install(CTC_INGRESS, *lag_id);
    ctc_sai_acl_lag_install(CTC_EGRESS, *lag_id);

    sdk_ret = ctc_linkagg_create(&linkagg_grp);
    if(CTC_E_INVALID_TID == sdk_ret)
    {
        return SAI_STATUS_TABLE_FULL;
    }
#ifdef TSINGMA
    if(linkagg_grp.linkagg_mode != CTC_LINKAGG_MODE_RR)
    {
       sdk_ret = ctc_linkagg_set_property(linkagg_grp.tid, CTC_LINKAGG_PROP_LB_HASH_OFFSET, g_hash_value_global_agg_hash);
       if (sdk_ret)
       {
           return sdk_ret;
       }
    }
#endif
    ctc_sai_port_entry_db_alloc(*lag_id);
        
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_sai_lag_remove(
	_In_  sai_object_id_t lag_id)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    uint32_t                        opf_index;
    int32_t                         sdk_ret     = 0;
    ctc_linkagg_group_t             linkagg_grp;

    sal_memset(&linkagg_grp,0,sizeof(ctc_linkagg_group_t));

    ctc_sai_port_entry_db_free(lag_id);

    opf_index = CTC_SAI_OBJECT_INDEX_GET(lag_id);

    ctclib_opb_free_offset(&p_sai_glb->lag_opb, opf_index);

    ctc_linkagg_destroy(opf_index);

    ctc_sai_clear_port_all_stats(lag_id);

    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_sai_lag_creat_lag_member(
    _Out_ sai_object_id_t* lag_member_id,
    _In_ uint32_t attr_count,
    _In_ sai_attribute_t *attr_list)
{
    stbl_lag_memport_t  memport;
    stbl_lag_memport_t* p_sdb_memport = NULL;
    uint32          attr_idx = 0; 
    uint8           tid  = 0;
    uint32          gport = 0;
    sai_object_id_t lag_oid = 0;
    sai_object_id_t port_oid = 0;
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    sai_attribute_t*  attr = NULL;
    bool port_enable = false;
    stbl_port_t *p_sdb_port = NULL;
#ifdef GREATBELT
    uint32 vid;
    ctc_l2dflt_addr_t l2dflt_addr;

    sal_memset(&l2dflt_addr, 0, sizeof(l2dflt_addr));
    sal_memset(&memport, 0, sizeof(memport));
#endif
    for(attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = attr_list + attr_idx;
        switch(attr->id)
        {
        case SAI_LAG_MEMBER_ATTR_LAG_ID:
            lag_oid = attr->value.oid;
            tid = CTC_SAI_OBJECT_INDEX_GET(attr->value.oid); 
            break;

        case SAI_LAG_MEMBER_ATTR_PORT_ID:
            port_oid = attr->value.oid;
            gport = CTC_SAI_OBJECT_INDEX_GET(attr->value.oid);
            *lag_member_id = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_LAG_MEMBER, gport);     
            break;

        default: 
            break;
        }
    }

#ifdef GREATBELT
    /*For GB, once port is bind to lag, it should removed from vlan default entry*/
    p_sdb_port = ctc_sai_port_get_port(port_oid);
    if(NULL != p_sdb_port)
    {
        CTC_BMP_ITER_BEGIN(p_sdb_port->vlan_bmp, vid)
        {
            l2dflt_addr.fid = vid;
            l2dflt_addr.l2mc_grp_id = vid;
            l2dflt_addr.member.mem_port = gport;
            ctc_l2_remove_port_from_default_entry(&l2dflt_addr);
            
        }
        CTC_BMP_ITER_END(p_sdb_port->vlan_bmp, vid);
    }
    
#endif
    /* Modified by kcao 2015-03-25 for bug 37743, SAI does not satisfy LAG member operation
     * should uses ctc_sai_port_set_port_flag() flags
     * SAI_PORT_FLAG_AGGREGATED - add member port to LAG, should disable TX/RX
     * SAI_PORT_FLAG_AGG_OPRED  - member port change to operated, should enable TX/RX
     */
#if 0
    ret = ctc_linkagg_add_port(tid, gport);
    ret = ctc_port_get_mac_en(gport, &port_enable);
    if (ret < 0)
    {
         return ctc_sai_get_error_from_sdk_error(ret);
    }
    if (!port_enable)
    {
         ret = ctc_linkagg_remove_port(tid, gport);
         if (ret < 0)
         {
            return ctc_sai_get_error_from_sdk_error(ret);
         }
    }
#else
    (void)tid;
    (void)port_enable;
#endif
    memport.key.gport = gport;
    p_sdb_memport = stbl_lag_memport_get_lag_memport(&memport.key);
    if (NULL == p_sdb_memport)
    {
        stbl_lag_memport_add_lag_memport(&memport);
        p_sdb_memport = stbl_lag_memport_get_lag_memport(&memport.key);
        if (NULL == p_sdb_memport)
        {
            return SAI_STATUS_NO_MEMORY;
        }
    }
    p_sdb_memport->tid = tid;
    p_sdb_memport->port_oid = port_oid;

    ctc_sai_port_bind_lag_id(port_oid, lag_oid);
    ctc_sai_acl_lag_member_enable(lag_oid, port_oid);

    p_sdb_port = ctc_sai_port_entry_get_by_portid(port_oid);
    if (p_sdb_port)
    {
        p_sdb_port->lag_id = lag_oid;
    }
    
    return ret;
}

sai_status_t
ctc_sai_lag_remove_lag_member(
    _In_ sai_object_id_t  lag_member_id
    )
{
    stbl_lag_memport_key_t  memport_key;
    stbl_lag_memport_t*     p_sdb_memport = NULL;
    uint32 gport = 0;
    uint8  tid = 0;
    sai_status_t ret = SAI_STATUS_SUCCESS;
    sai_object_id_t port_oid = 0;
    stbl_port_t *p_sdb_port = NULL;
#ifdef GREATBELT
    uint32 vid;
    ctc_l2dflt_addr_t l2dflt_addr;
    sai_object_id_t port_id = 0;

    sal_memset(&l2dflt_addr, 0, sizeof(l2dflt_addr));
#endif

    if (SAI_OBJECT_TYPE_LAG_MEMBER != CTC_SAI_OBJECT_TYPE_GET(lag_member_id))
    {
       return SAI_STATUS_INVALID_PARAMETER;
    }
    
    gport = CTC_SAI_OBJECT_INDEX_GET(lag_member_id);
    p_sdb_port = ctc_sai_port_entry_get_by_gport(gport);
    if (p_sdb_port)
    {
        p_sdb_port->lag_id = 0;
    }
    
    memport_key.gport = gport;
    p_sdb_memport = stbl_lag_memport_get_lag_memport(&memport_key);
    if (NULL == p_sdb_memport)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    
    tid = p_sdb_memport->tid;
    port_oid = p_sdb_memport->port_oid;

    ctc_sai_acl_lag_member_disable(port_oid, CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_LAG, tid));
    ctc_sai_port_bind_lag_id(port_oid, 0);

    stbl_lag_memport_del_lag_memport(&memport_key);
    
    /* Modified by kcao 2015-03-25 for bug 37743, SAI does not satisfy LAG member operation
     * should uses ctc_sai_port_set_port_flag() flags
     * SAI_PORT_FLAG_AGGREGATED - add member port to LAG, should disable TX/RX
     * SAI_PORT_FLAG_AGG_OPRED  - member port change to operated, should enable TX/RX
     */
#if 0     
    ret = ctc_linkagg_remove_port(tid, gport);
#endif
#ifdef GREATBELT
    /*For GB, once port is unbind to lag, it should add to vlan default entry*/
    port_id = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_PORT, gport);
    p_sdb_port = ctc_sai_port_get_port(port_id);
    if (NULL != p_sdb_port)
    {
        CTC_BMP_ITER_BEGIN(p_sdb_port->vlan_bmp, vid)
        {
            l2dflt_addr.fid = vid;
            l2dflt_addr.l2mc_grp_id = vid;
            l2dflt_addr.member.mem_port = gport;
            ctc_l2_add_port_to_default_entry(&l2dflt_addr);
            
        }
        CTC_BMP_ITER_END(p_sdb_port->vlan_bmp, vid);
    }
    
#endif
    if (ret < 0)
    {
        return ctc_sai_get_error_from_sdk_error(ret);
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_lag_get_ports(
    _In_    sai_object_id_t      lag_id,
    _Inout_ sai_attribute_t*     attr)
{
    int32_t     sdk_ret             = 0;
    uint32_t    port_count_index    = 0;
    uint32_t    pport[255];
#ifdef USW
    uint16_t     port_cnt            = 0;
#else
    uint8_t     port_cnt            = 0;
#endif

    CTC_SAI_ERROR_GOTO(ctc_linkagg_get_member_ports(CTC_SAI_OBJECT_INDEX_GET(lag_id),
            pport,&port_cnt),sdk_ret,out);

    for(port_count_index = 0; port_count_index < port_cnt; port_count_index++)
    {
        if(port_count_index >= attr->value.objlist.count)
        {
            break;
        }
        attr->value.objlist.list[port_count_index] =
            CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_PORT,pport[port_count_index]);
    }

    attr->value.objlist.count = port_count_index;

out:
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

int32_t
ctc_sai_lag_alloc_timestamp (sai_object_id_t port_id, uint32_t *ptimestamp_nhid)
{
    stbl_port_key_t         key;
    stbl_port_t             *p_sdb_port = NULL;
    ctc_misc_nh_param_t     nh_param;
    uint32_t                nexthopid    = 0;
    sai_status_t            ret 		 = SAI_STATUS_SUCCESS;
    
    sal_memset(&nh_param, 0, sizeof(ctc_misc_nh_param_t));

    key.port_oid = port_id;
    p_sdb_port = stbl_port_get_port(&key);
    if (!p_sdb_port)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    nh_param.type = CTC_MISC_NH_TYPE_OVER_L2_WITH_TS;
    sal_memcpy(nh_param.misc_param.over_l2edit.mac_da,
        ctc_sai_get_sys_info()->timestamp.dmac, sizeof(sai_mac_t));
    sal_memcpy(nh_param.misc_param.over_l2edit.mac_sa,
        ctc_sai_get_sys_info()->timestamp.smac, sizeof(sai_mac_t));
    nh_param.misc_param.over_l2edit.ether_type = ctc_sai_get_sys_info()->timestamp.type;
    
    if (p_sdb_port->timestamp_count == 0) {
        nh_param.gport = (0x1f00 | CTC_SAI_OBJECT_INDEX_GET(port_id));
        CTC_SAI_ERROR_GOTO(ctc_sai_nexthop_alloc_offset(&nexthopid),ret,err);
        CTC_SAI_ERROR_GOTO(ctc_nh_add_misc(nexthopid, &nh_param),ret,err1);

        p_sdb_port->timestamp_nexthopid = nexthopid;

        CTC_SAI_DEBUG("%s%d:port_oid = 0x%llx,timestamp_nhid = %d\n",
            __FUNCTION__,__LINE__,port_id, p_sdb_port->timestamp_nexthopid);
    }

    p_sdb_port->timestamp_count++;
    *ptimestamp_nhid = p_sdb_port->timestamp_nexthopid;

    CTC_SAI_DEBUG("%s%d:port_oid = 0x%llx,timestamp_count = %d\n",
            __FUNCTION__,__LINE__,port_id, p_sdb_port->timestamp_count);
    
    return SAI_STATUS_SUCCESS;
err1:
    ctc_sai_nexthop_free_offset(nexthopid);
err:
    CTC_SAI_DEBUG("%s%d:port_oid = 0x%llx\n",__FUNCTION__,__LINE__,port_id);
    return ret;
}

int32_t
ctc_sai_lag_free_timestamp (sai_object_id_t port_id)
{
    stbl_port_key_t         key;
    stbl_port_t             *p_sdb_port = NULL;

    key.port_oid = port_id;
    p_sdb_port = stbl_port_get_port(&key);
    if (!p_sdb_port)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    
    p_sdb_port->timestamp_count--;

    CTC_SAI_DEBUG("%s%d:port_oid = 0x%llx,timestamp_count = %d\n",
            __FUNCTION__,__LINE__,port_id, p_sdb_port->timestamp_count);
    
    if (p_sdb_port->timestamp_count <= 0)
    {
        CTC_SAI_DEBUG("%s%d:port_oid = 0x%llx,timestamp_nexthopid = %d\n",
            __FUNCTION__,__LINE__,port_id, p_sdb_port->timestamp_nexthopid);
        
        ctc_nh_remove_misc(p_sdb_port->timestamp_nexthopid);
        ctc_sai_nexthop_free_offset(p_sdb_port->timestamp_nexthopid);
    }
    
    return SAI_STATUS_SUCCESS;
}

#define ________SAI_SAI_DEBUG_FUNC
sai_status_t
ctc_sai_remove_lag_debug_param(
    _In_ sai_object_id_t  lag_id)
{
    CTC_SAI_DEBUG("in:lag_id 0x%llx", lag_id);
    return SAI_STATUS_SUCCESS;
}
sai_status_t
ctc_sai_get_lag_attribute_debug_param(
    _In_ sai_object_id_t lag_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    const sai_attribute_t *attr = NULL;
    uint32_t          attr_idx = 0; 

    for (attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = attr_list + attr_idx;
        ret = ctc_sai_attr_check_read_attr(g_sai_attr_entries,attr);

        if(ret != SAI_STATUS_SUCCESS)
        {
            return SAI_STATUS_SUCCESS;
        }

        switch(attr->id)
        {
            case SAI_LAG_ATTR_PORT_LIST:
                CTC_SAI_DEBUG("in:SAI_LAG_ATTR_PORT_LIST lag_id 0x%llx", lag_id);
                break;
            default: 
                break;
        }

        if(ret != SAI_STATUS_SUCCESS)
        {
            break;
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_create_lag_member_debug_param(
    _Out_ sai_object_id_t* lag_member_id,
    _In_ uint32_t attr_count,
    _In_ sai_attribute_t *attr_list)
{
    const sai_attribute_t *attr = NULL;
    uint32_t          attr_idx = 0; 

    CTC_SAI_DEBUG("out:lag_member_id 0x%llx", (*lag_member_id));
    for (attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = attr_list + attr_idx;
        switch (attr->id)
        {
        case SAI_LAG_MEMBER_ATTR_LAG_ID:
            CTC_SAI_DEBUG("out:SAI_LAG_MEMBER_ATTR_LAG_ID 0x%llx", attr->value.oid);
            break;
        case SAI_LAG_MEMBER_ATTR_PORT_ID:
            CTC_SAI_DEBUG("out:SAI_HOSTIF_ATTR_RIF_OR_PORT_ID 0x%llx", attr->value.oid);
            break;
         default:
            break;
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t ctc_sai_get_lag_member_attribute_debug_param(
    _In_ sai_object_id_t lag_member_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list
    )
{
    const sai_attribute_t *attr = NULL;
    uint32_t          attr_idx = 0; 

    CTC_SAI_DEBUG("in:lag_member_id 0x%llx, attr_count", lag_member_id, attr_count);
    for (attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = attr_list + attr_idx;
        switch (attr->id)
        {
        case SAI_LAG_MEMBER_ATTR_LAG_ID:
            CTC_SAI_DEBUG("out:SAI_LAG_MEMBER_ATTR_LAG_ID 0x%llx", attr->value.oid);
            break;
        case SAI_LAG_MEMBER_ATTR_PORT_ID:
            CTC_SAI_DEBUG("out:SAI_LAG_MEMBER_ATTR_PORT_ID 0x%llx", attr->value.oid);
            break;
         default:
            break;
        }
    }

    return SAI_STATUS_SUCCESS;
}
#define ________SAI_SAI_API_FUNC

/*
    \brief Create LAG
    \param[out] lag_id LAG id
    \param[in] attr_count number of attributes
    \param[in] attr_list array of attributes
    \return Success: SAI_STATUS_SUCCESS
            Failure: Failure status code on error
*/
sai_status_t
ctc_sai_create_lag(
    _Out_ sai_object_id_t* lag_id,
    _In_ uint32_t attr_count,
    _In_ sai_attribute_t *attr_list)
{
    CTC_SAI_DEBUG_FUNC();

    CTC_SAI_PTR_VALID_CHECK(lag_id);
    return ctc_sai_lag_create(lag_id, attr_count, attr_list);
}

/*
    \brief Remove LAG
    \param[in] lag_id LAG id
    \return Success: SAI_STATUS_SUCCESS
            Failure: Failure status code on error
*/
sai_status_t
ctc_sai_remove_lag(
    _In_ sai_object_id_t  lag_id)
{
    CTC_SAI_DEBUG_FUNC();
    ctc_sai_remove_lag_debug_param(lag_id);

    return ctc_sai_lag_remove(lag_id);
}


/*
    \brief Creat lag member id.
    \param[out] lag member id
    \param[in] attr_count number of attributes
    \param[in] attr_list array of attributes
    \return Success: SAI_STATUS_SUCCESS
            Failure: Failure status code on error
*/
sai_status_t
ctc_sai_create_lag_member(
    _Out_ sai_object_id_t* lag_member_id,
    _In_ uint32_t attr_count,
    _In_ sai_attribute_t *attr_list)
{
    sai_status_t ret = SAI_STATUS_SUCCESS;
    
    CTC_SAI_DEBUG_FUNC();

    CTC_SAI_PTR_VALID_CHECK(attr_list);
    CTC_SAI_PTR_VALID_CHECK(lag_member_id);
       
    ret = ctc_sai_lag_creat_lag_member(lag_member_id, attr_count, attr_list);
    if (SAI_STATUS_SUCCESS != ret)
    {
        return ret;
    }

    ctc_sai_create_lag_member_debug_param(lag_member_id, attr_count, attr_list);
    return ret;
}

/*
    \brief Remove lag member id
    \param[in] lag member id
    \return Success: SAI_STATUS_SUCCESS
            Failure: Failure status code on error          
*/
sai_status_t
ctc_sai_remove_lag_member(
    _In_ sai_object_id_t  lag_member_id
    )
{
    CTC_SAI_DEBUG_FUNC();

     return ctc_sai_lag_remove_lag_member(lag_member_id);
}


/*
    \brief Set LAG Attribute
    \param[in] lag_id LAG id
    \param[in] attr Structure containing ID and value to be set
    \return Success: SAI_STATUS_SUCCESS
            Failure: Failure status code on error
*/
sai_status_t
ctc_sai_set_lag_attribute(
    _In_ sai_object_id_t  lag_id,
    _In_ const sai_attribute_t *attr)
{
    CTC_SAI_DEBUG_FUNC();

    return SAI_STATUS_SUCCESS;
}

/*
    \brief Get LAG Attribute
    \param[in] lag_id LAG id
    \param[in] attr_count Number of attributes to be get
    \param[in,out] attr_list List of structures containing ID and value to be get
    \return Success: SAI_STATUS_SUCCESS
            Failure: Failure status code on error
*/

sai_status_t
ctc_sai_get_lag_attribute(
    _In_ sai_object_id_t lag_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    sai_attribute_t* attr        = NULL;
    uint32_t        attr_idx    = 0;

    CTC_SAI_DEBUG_FUNC();
    ctc_sai_get_lag_attribute_debug_param(lag_id, attr_count, attr_list);
    CTC_SAI_PTR_VALID_CHECK(attr_list);

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
            case SAI_LAG_ATTR_PORT_LIST:
                ret = ctc_sai_lag_get_ports(lag_id,attr);
                break;
                
            default: 
            break;
        }

        if(ret != SAI_STATUS_SUCCESS)
        {
            break;
        }
    }

    return SAI_STATUS_SUCCESS;
}

/*
    \brief Set LAG Member Attribute
    \param[in] lag_member_id LAG Member id
    \param[in] attr Structure containing ID and value to be set
    \return Success: SAI_STATUS_SUCCESS
            Failure: Failure status code on error
*/
sai_status_t ctc_sai_set_lag_member_attribute(
    _In_ sai_object_id_t  lag_member_id,
    _In_ const sai_attribute_t *attr
    )
{
    CTC_SAI_DEBUG_FUNC();

    return SAI_STATUS_SUCCESS;
}

/*
    \brief Get LAG Member Attribute
    \param[in] lag_member_id LAG Member id
    \param[in] attr_count Number of attributes to be get
    \param[in,out] attr_list List of structures containing ID and value to be get
    \return Success: SAI_STATUS_SUCCESS
            Failure: Failure status code on error
*/

sai_status_t ctc_sai_get_lag_member_attribute(
    _In_ sai_object_id_t lag_member_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list
    )
{
    CTC_SAI_DEBUG_FUNC();
    
    return SAI_STATUS_SUCCESS;
}

#define ________SAI_INNER_FUNC
static sai_status_t
__init_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;

    ret = ctc_sai_lag_db_init();
    if(SAI_STATUS_SUCCESS != ret)
    {
        return ret;
    }

    preg->init_status =  INITIALIZED;

    return ret;
}

static sai_status_t
__exit_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    preg->init_status = UNINITIALIZED;
    return SAI_STATUS_SUCCESS;
}

/* define sai 0.9.2 */
static sai_lag_api_t      g_sai_api_func = {
    .create_lag           = ctc_sai_create_lag,
    .remove_lag           = ctc_sai_remove_lag,
    .set_lag_attribute    = ctc_sai_set_lag_attribute,
    .get_lag_attribute    = ctc_sai_get_lag_attribute,
    .create_lag_member     = ctc_sai_create_lag_member,
    .remove_lag_member     = ctc_sai_remove_lag_member,
    .set_lag_member_attribute  = ctc_sai_set_lag_member_attribute,
    .get_lag_member_attribute  = ctc_sai_get_lag_member_attribute,
};

static ctc_sai_api_reg_info_t g_api_reg_info = {
        .id  		= SAI_API_LAG,
        .init_func 	= __init_mode_fn,
        .exit_func 	= __exit_mode_fn,
        .api_method_table = &g_sai_api_func,
        .private_data     = NULL,
};

#define ________SAI_OUTER_FUNC
sai_status_t
ctc_sai_lag_init()
{
    api_reg_register_fn(&g_api_reg_info);

    return SAI_STATUS_SUCCESS;
}

