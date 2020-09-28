#include <sai.h>
#include "sal.h"
#include <ctc_api.h>
#include <ctc_sai_common.h>
#include <ctc_sai_common_attr.h>
#include <ctc_sai_notifier_call.h>
#include <ctc_sai_sai.h>
#include <ctc_sai_nexthop.h>
#include <ctc_sai_nexthop_group.h>
#include <ctc_sai_debug.h>
#include <ctc_sai_l2mc.h>
#include <ctc_sai_routerintf.h>
#include <ctc_sai_port.h>

#define CTC_SAI_NEXTHOP_GROUP_BLOCK_SIZE    64
#define CTC_SAI_NEXTHOP_GROUP_MAX           239

static ctc_sai_attr_entry_info_t g_sai_attr_entries[] = {
    {
        .id     = SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_COUNT,
        .type   = SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_NEXT_HOP_GROUP_ATTR_TYPE,
        .type   =  SAI_ATTR_FALG_READ | SAI_ATTR_FALG_WRITE,
    },
    {
        .id     = SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_LIST,
        .type   = SAI_ATTR_FALG_CREATE | SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_NEXT_HOP_GROUP_ATTR_ECMP_HSAH,
        .type   =  SAI_ATTR_FALG_READ | SAI_ATTR_FALG_WRITE,
    },
    {
        .id     = SAI_NEXT_HOP_GROUP_ATTR_ECMP_FAILOVER_EN,
        .type   =  SAI_ATTR_FALG_READ | SAI_ATTR_FALG_WRITE,
    },
    {
        .id     = SAI_ATTR_ID_END,
        .type   = 0,
    },
};

sai_status_t
ctc_nexthop_group_id_sort_bob(_In_  sai_object_id_t *nhid_sort)
{
    sai_object_id_t     id_compare = 0;
    uint32              i = 0;
    uint32              k = 0;

    for(i = 0; i < CTC_SAI_DEFAULT_MAX_ECMP_NUM; i++)
    {
        for(k = CTC_SAI_DEFAULT_MAX_ECMP_NUM - 1; k > i; k--)
        {
            if (0 == nhid_sort[i] || 0 == nhid_sort[k])
            {
                continue;
            }

            if (nhid_sort[i] > nhid_sort[k])
            {
                id_compare = nhid_sort[k];
                nhid_sort[k] = nhid_sort[i];
                nhid_sort[i] = id_compare;
            }
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_nexthop_group_id_sort(_In_  sai_object_id_t *nhid_sort)
{
    sai_object_id_t         nhid_st[CTC_SAI_DEFAULT_MAX_ECMP_NUM];
    sai_object_id_t         id_compare = 0;
    uint32                  i = 0;
    uint32                  j = 0;
    uint32                  k = 0;

    sal_memset(nhid_st,  0, sizeof(nhid_st));

    for(i = 0; i < CTC_SAI_DEFAULT_MAX_ECMP_NUM; i++)
    {
        if (nhid_sort[i])
        {
            id_compare = nhid_sort[i];
        }
        else
        {
            continue;
        }

        for(k = i; k < CTC_SAI_DEFAULT_MAX_ECMP_NUM; k++)
        {
            if (id_compare > nhid_sort[k] && nhid_sort[k])
            {
                id_compare = nhid_sort[k];
                sal_memset(&nhid_sort[k],  0, sizeof(sai_object_id_t));
            }
        }

        sal_memset(&nhid_sort[i],  0, sizeof(sai_object_id_t));
        j ++;
        nhid_st[j] = id_compare;
    }

    sal_memcpy(nhid_sort, nhid_st,sizeof(nhid_st));

    return SAI_STATUS_SUCCESS;
}

sai_status_t
__nexthop_group_create_ecmp_group(
    _In_  stbl_nexthop_group_t          *p_sdb_nhgroup)
{
    ctclib_slistnode_t                  *listnode       = NULL;    
    stbl_nexthop_t                      *p_sdb_nh       = NULL;
    ctc_nh_ecmp_nh_param_t              nh_param;
    int32_t                             idx_node        = 0;
    int32_t                             sdk_ret         = 0;

    sal_memset(&nh_param, 0, sizeof(nh_param));

    CTCLIB_SLIST_LOOP(p_sdb_nhgroup->nh_list.obj_list, p_sdb_nh, listnode)
    {
        nh_param.nhid[idx_node] = CTC_SAI_OBJECT_INDEX_GET(p_sdb_nh->nh_oid);
        idx_node++;
    }

    nh_param.nh_num = idx_node;
    nh_param.type = p_sdb_nhgroup->ecmp_type;
    nh_param.failover_en = p_sdb_nhgroup->failover_en;

    sdk_ret = ctc_nh_add_ecmp(p_sdb_nhgroup->key.group_id, &nh_param);
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
__nexthop_group_update_ecmp_group(
    _In_  stbl_nexthop_group_t          *p_sdb_nhgroup)
{
    ctclib_slistnode_t                  *listnode       = NULL;    
    stbl_nexthop_t                      *p_sdb_nh       = NULL;
    ctc_nh_ecmp_nh_param_t              nh_param;
    int32_t                             idx_node        = 0;
    int32_t                             sdk_ret         = 0;

    /* 1. remove nhg */  
    sdk_ret = ctc_nh_remove_ecmp(p_sdb_nhgroup->key.group_id);

    /* 2. add nhg agagin */
    sal_memset(&nh_param, 0, sizeof(nh_param));
    CTCLIB_SLIST_LOOP(p_sdb_nhgroup->nh_list.obj_list, p_sdb_nh, listnode)
    {
        nh_param.nhid[idx_node] = CTC_SAI_OBJECT_INDEX_GET(p_sdb_nh->nh_oid);
        idx_node++;
    }
    nh_param.nh_num = idx_node;
    nh_param.type = p_sdb_nhgroup->ecmp_type;
    nh_param.failover_en = p_sdb_nhgroup->failover_en;

    sdk_ret = ctc_nh_add_ecmp(p_sdb_nhgroup->key.group_id, &nh_param);
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

stbl_nexthop_group_t*
ctc_nexthop_group_get_by_oid(
    _In_  const sai_object_id_t nhg_id)
{
    stbl_nexthop_group_key_t key;

    key.group_id = CTC_SAI_OBJECT_INDEX_GET(nhg_id);
    return stbl_nexthop_group_get_nexthop_group(&key);
}

void
ctc_nexthop_group_nhg_cnt_inc()
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    p_sai_glb->nh_group_curr_count++;
}

void
ctc_nexthop_group_nhg_cnt_dec()
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    p_sai_glb->nh_group_curr_count--;
}

sai_status_t
__nexthop_group_create_nexthoplist(
    _In_  stbl_nexthop_group_t          *p_sdb_nhgroup,
    _In_  const sai_object_list_t       *objlist)
{
    stbl_nexthop_t                      *p_sdb_nh    = NULL;
    sai_status_t    ret         = SAI_STATUS_SUCCESS;
    uint32_t        i           = 0;

    for (i = 0; i < objlist->count; i++)
    {
        p_sdb_nh = ctc_nexthop_get_by_oid(objlist->list[i]);
        if (NULL == p_sdb_nh)
        {
            log_sys(M_MOD_SAI, E_ERROR, "Not found nexthop group oid 0x%"PRIx64" 0x%"PRIx64"when creat nexthoplist", objlist->list[i], objlist->list[i] & 0xFFFFFFFF);
            return SAI_STATUS_ITEM_NOT_FOUND;
        }
        stbl_nexthop_group_set_nh_list(p_sdb_nhgroup, TRUE, p_sdb_nh);
    }

    if (!p_sdb_nhgroup->nh_list.obj_list->count)
    {
        ret = SAI_STATUS_INVALID_PARAMETER;
    }

    return ret;
}

sai_status_t
__nexthop_group_create_ipmc_member(
    _In_  stbl_nexthop_group_t          *p_sdb_nhgroup,
    _In_  const sai_object_list_t       *objlist)
{
    stbl_rif_t     *p_sdb_rif    = NULL;
    sai_status_t    ret         = SAI_STATUS_SUCCESS;
    uint32_t        i           = 0;

    for (i = 0; i < objlist->count; i++)
    {
        p_sdb_rif = ctc_routerintf_get_by_oid(objlist->list[i]);
        if (NULL == p_sdb_rif)
        {
            log_sys(M_MOD_SAI, E_ERROR, "Not found routerintf oid 0x%"PRIx64, objlist->list[i]);
            return SAI_STATUS_ITEM_NOT_FOUND;
        }
        stbl_nexthop_group_set_nh_list(p_sdb_nhgroup, TRUE, p_sdb_rif);
    }
#if 0
    if (!p_sdb_nhgroup->nh_list.obj_list->count)
    {
        ret = SAI_STATUS_INVALID_PARAMETER;
    }
#endif
    return ret;
}

sai_status_t
__nexthop_group_create_ipmc_group(
    _In_  stbl_nexthop_group_t          *p_sdb_nhgroup)
{
    ctclib_slistnode_t                  *listnode       = NULL;    
    stbl_rif_t                      	*p_sdb_rif    = NULL;
    ctc_mcast_nh_param_group_t 			nh_mcast_group = {0};
    int32_t                             sdk_ret         = 0;

	/* 1. alloc group resource */
    sal_memset(&nh_mcast_group, 0, sizeof(nh_mcast_group));

	/* 2. create mcast group */
	nh_mcast_group.mc_grp_id = p_sdb_nhgroup->ipmc_group_id;
	sdk_ret = ctc_nh_add_mcast(p_sdb_nhgroup->key.group_id, &nh_mcast_group);
	if (sdk_ret < 0)
    {
        return ctc_sai_get_error_from_sdk_error(sdk_ret);
    }

	/* 3. foreach add ipmc member */
    CTCLIB_SLIST_LOOP(p_sdb_nhgroup->nh_list.obj_list, p_sdb_rif, listnode)
    {
    	sal_memset(&nh_mcast_group, 0, sizeof(nh_mcast_group));
		nh_mcast_group.mc_grp_id = p_sdb_nhgroup->ipmc_group_id;
	    nh_mcast_group.opcode = CTC_NH_PARAM_MCAST_ADD_MEMBER;
	    nh_mcast_group.mem_info.member_type = CTC_NH_PARAM_MEM_IPMC_LOCAL;
		
    	switch(p_sdb_rif->type)
    	{
		case SAI_ROUTER_INTERFACE_TYPE_PORT:
			nh_mcast_group.mem_info.l3if_type = CTC_L3IF_TYPE_PHY_IF; 
			ctc_sai_port_objectid_to_gport(p_sdb_rif->port_oid, &nh_mcast_group.mem_info.destid);
			
			break;
		case SAI_ROUTER_INTERFACE_TYPE_VLAN:
			break;
		default:
			break;
		}

		sdk_ret = ctc_nh_update_mcast(p_sdb_nhgroup->key.group_id, &nh_mcast_group);
    }

    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}


sai_status_t
ctc_nexthop_group_add_entry(
    _In_  stbl_nexthop_group_t          *p_sdb_nhgroup,
    _In_  const sai_object_list_t       *objlist)
{
    sai_status_t                ret = SAI_STATUS_SUCCESS;

	if(SAI_NEXT_HOP_GROUP_L3MC != p_sdb_nhgroup->type)
	{
    	CTC_SAI_ERROR_GOTO(__nexthop_group_create_nexthoplist(p_sdb_nhgroup, objlist),ret,out);

	    CTC_SAI_ERROR_GOTO(__nexthop_group_create_ecmp_group(p_sdb_nhgroup),ret,out);
	}else{
		CTC_SAI_ERROR_GOTO(__nexthop_group_create_ipmc_member(p_sdb_nhgroup, objlist),ret,out);
		//CTC_SAI_ERROR_GOTO(__nexthop_group_create_ipmc_group(p_sdb_nhgroup),ret,out);
	}

    ctc_nexthop_group_nhg_cnt_inc();

out:
    return ret;
}

sai_status_t
_ctc_sai_nhgroup_add_nexthop(
    _In_  stbl_nexthop_group_t          *p_sdb_nhgroup,
    _In_  uint32_t                      next_hop_count,
    _In_  const sai_object_id_t         * nexthops)
{
    return SAI_STATUS_NOT_IMPLEMENTED;
}

sai_status_t
_ctc_sai_nhgroup_remove_nexthop(
    _In_  stbl_nexthop_group_t          *p_sdb_nhgroup,
    _In_  uint32_t                      next_hop_count,
    _In_  const sai_object_id_t         *nexthops)
{
    return SAI_STATUS_NOT_IMPLEMENTED;
}

sai_status_t
ctc_nexthop_group_db_init()
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    sai_status_t    ret         = SAI_STATUS_SUCCESS;

    p_sai_glb->nh_group_max_count = CTC_SAI_NEXTHOP_GROUP_MAX_ENTRY;

    CTC_SAI_ERROR_GOTO(
        ctclib_opb_create(&p_sai_glb->nexthop_group_id_opb, CTC_SAI_NH_GROUP_OFFSET, p_sai_glb->nh_group_max_count, "nexthop_group_id"),
        ret,out);
out:
    return ret;
}


sai_status_t
ctc_nhg_sai_attr_alloc_attr_entry_list(const ctc_sai_attr_entry_info_t const *attr_info_entries,
                                 const sai_attribute_t *attr_list,
                                 uint32_t attr_count,
                                 sai_object_id_t *oid_list,
                                 _Out_ ctc_sai_attr_entry_list_t **pattr_entry_list)
{
    uint32_t attr_info_count = 0;
    uint32_t attr_idx        = 0;
    ctc_sai_attr_entry_list_t *psai_attr_entry_list = NULL;

    CTC_SAI_PTR_VALID_CHECK(attr_info_entries);
    CTC_SAI_PTR_VALID_CHECK(pattr_entry_list);

    while(attr_info_entries[attr_info_count].id != SAI_ATTR_ID_END)
        attr_info_count++;

    if(0 == attr_info_count)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    psai_attr_entry_list = XCALLOC(MEM_FEA_TEMP_TODO, sizeof(ctc_sai_attr_entry_list_t) * attr_info_count);
    *pattr_entry_list = psai_attr_entry_list;

    if(!psai_attr_entry_list )
    {
        return SAI_STATUS_NO_MEMORY;
    }

    sal_memset(psai_attr_entry_list,0,sizeof(ctc_sai_attr_entry_list_t) * attr_info_count);

    for(attr_idx = 0; attr_list && attr_idx < attr_count; attr_idx++)
    {
        if(attr_list[attr_idx].id >= attr_info_count)
        {
            return SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + attr_idx;
        }
        if(attr_list[attr_idx].id == SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_LIST)
        {
            psai_attr_entry_list[attr_list[attr_idx].id].value.objlist.list = oid_list;
            psai_attr_entry_list[attr_list[attr_idx].id].value.objlist.count = attr_list[attr_idx].value.objlist.count;
            sal_memcpy(oid_list, attr_list[attr_idx].value.objlist.list, attr_list[attr_idx].value.objlist.count*(sizeof(sai_object_id_t)));
        }
        else
        {
            psai_attr_entry_list[attr_list[attr_idx].id].value = attr_list[attr_idx].value;
        }
        psai_attr_entry_list[attr_list[attr_idx].id].attr_index = attr_idx;
        psai_attr_entry_list[attr_list[attr_idx].id].valid = true;
    }

    attr_idx = 0;
    while(attr_info_entries[attr_idx].id != SAI_ATTR_ID_END)
    {
        if(SAI_ATTR_FLAG_DEFAULT == (SAI_ATTR_FLAG_DEFAULT &attr_info_entries[attr_idx].type))
        {
            if(false == psai_attr_entry_list[attr_idx].valid)
            {
                psai_attr_entry_list[attr_idx].value =
                    attr_info_entries[attr_idx].default_value;
                psai_attr_entry_list[attr_idx].valid = true;
            }
        }
        attr_idx++;
    }

    *pattr_entry_list = psai_attr_entry_list;

    return SAI_STATUS_SUCCESS;
}


#define ________SAI_SAI_INNER_DEBUG_FUNC
sai_status_t
ctc_sai_create_next_hop_group_debug_param(
    _Out_ sai_object_id_t* next_hop_group_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list)
{
    CTC_SAI_DEBUG("out:next_hop_group_id 0x%llx", (*next_hop_group_id));
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_remove_next_hop_group_debug_param(
    _In_ sai_object_id_t next_hop_group_id)
{
    CTC_SAI_DEBUG("in:next_hop_group_id 0x%llx", next_hop_group_id);
    return SAI_STATUS_SUCCESS;
}
sai_status_t
ctc_sai_add_next_hop_to_group_debug_param(
    _In_ sai_object_id_t next_hop_group_id,
    _In_ uint32_t next_hop_count,
    _In_ const sai_object_id_t* nexthops)
{
    CTC_SAI_DEBUG("in:next_hop_group_id 0x%llx next_hop_count %u", next_hop_group_id, next_hop_count);
    return SAI_STATUS_SUCCESS;
}
sai_status_t
ctc_sai_remove_next_hop_from_group_debug_param(
    _In_ sai_object_id_t next_hop_group_id,
    _In_ uint32_t next_hop_count,
    _In_ const sai_object_id_t* nexthops)
{
    CTC_SAI_DEBUG("in:next_hop_group_id 0x%llx next_hop_count %u", next_hop_group_id, next_hop_count);
    return SAI_STATUS_SUCCESS;
}
#define ________SAI_SAI_INNER_API_FUNC

/*
* Routine Description:
*    Create next hop group
*
* Arguments:
*    [out] next_hop_group_id - next hop group id
*    [in] attr_count - number of attributes
*    [in] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_create_next_hop_group(
    _Out_ sai_object_id_t* next_hop_group_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list)
{
    stbl_sai_glb_t                  *p_sai_glb        = stbl_sai_glb_get_sai_glb();
    sai_status_t                    ret               = SAI_STATUS_SUCCESS;
    ctc_sai_attr_entry_list_t       *pattr_entry_list = NULL;
    stbl_nexthop_group_t            nhgroup;
    stbl_nexthop_group_t            *p_sdb_nhgroup    = NULL;
    uint32_t index           = 0;
	uint32_t 							opf_index = 0;
    sai_object_id_t oid_list[CTC_SAI_NHG_ATTR_LIST_MAX];

    CTC_SAI_DEBUG_FUNC();

    CTC_SAI_PTR_VALID_CHECK(next_hop_group_id);
    CTC_SAI_PTR_VALID_CHECK(attr_list);

    sal_memset(oid_list, 0, sizeof(oid_list));
    CTC_SAI_ERROR_GOTO(ctc_nhg_sai_attr_alloc_attr_entry_list(g_sai_attr_entries,
                            attr_list,
                            attr_count,
                            oid_list,
                            &pattr_entry_list),ret,out);

    CTC_SAI_ERROR_GOTO(ctc_sai_attr_check_attr_entry_list(g_sai_attr_entries,
                            pattr_entry_list),ret,out);

    if (p_sai_glb->nh_group_curr_count >= p_sai_glb->nh_group_max_count)
    {
        return SAI_STATUS_TABLE_FULL;
    }

    sal_memset(&nhgroup, 0, sizeof(nhgroup));

    CTC_SAI_ERROR_GOTO(ctclib_opb_alloc_offset(&p_sai_glb->nexthop_group_id_opb, &index), ret, out);
    nhgroup.key.group_id = index;
    if (pattr_entry_list[SAI_NEXT_HOP_GROUP_ATTR_TYPE].valid)
    {
        nhgroup.type = pattr_entry_list[SAI_NEXT_HOP_GROUP_ATTR_TYPE].value.s32;
		if(SAI_NEXT_HOP_GROUP_L3MC == nhgroup.type)
		{
			CTC_SAI_ERROR_GOTO(ctc_sai_l2mc_alloc_offset(&opf_index), ret, out);
			nhgroup.ipmc_group_id = opf_index;
		}
    }

    if (pattr_entry_list[SAI_NEXT_HOP_GROUP_ATTR_ECMP_HSAH].valid)
    {
        nhgroup.ecmp_type = pattr_entry_list[SAI_NEXT_HOP_GROUP_ATTR_ECMP_HSAH].value.u32;
    }

    if (pattr_entry_list[SAI_NEXT_HOP_GROUP_ATTR_ECMP_FAILOVER_EN].valid)
    {
        nhgroup.failover_en = pattr_entry_list[SAI_NEXT_HOP_GROUP_ATTR_ECMP_FAILOVER_EN].value.booldata;
    }

    stbl_nexthop_group_add_nexthop_group(&nhgroup);
    p_sdb_nhgroup = stbl_nexthop_group_get_nexthop_group(&nhgroup.key);
    if (NULL == p_sdb_nhgroup)
    {
        ret = SAI_STATUS_NO_MEMORY;
        goto out;
    }

    CTC_SAI_ERROR_GOTO(
        ctc_nexthop_group_add_entry(p_sdb_nhgroup,
                &pattr_entry_list[SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_LIST].value.objlist),
                ret, out);

    *next_hop_group_id = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_NEXT_HOP_GROUP, p_sdb_nhgroup->key.group_id);
    ctc_sai_create_next_hop_group_debug_param(next_hop_group_id, attr_count, attr_list);

    /* modified by liwh for bug 52148, 2019-05-17 */
    if (pattr_entry_list)
    {
        ctc_sai_attr_free_attr_entry_list(pattr_entry_list);
    }
    /* liwh end */

    return ret;

out:
    if (pattr_entry_list)
    {
        ctc_sai_attr_free_attr_entry_list(pattr_entry_list);
    }

	if(nhgroup.ipmc_group_id)
	{
		ctc_sai_l2mc_free_offset(nhgroup.ipmc_group_id);
	}

    return ret;
}

/*
* Routine Description:
*    Remove next hop group
*
* Arguments:
*    [in] next_hop_group_id - next hop group id
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_remove_next_hop_group(
    _In_ sai_object_id_t next_hop_group_id)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    int32_t                         sdk_ret = 0;
    sai_status_t                    ret = SAI_STATUS_SUCCESS;
    stbl_nexthop_group_t            *p_sdb_nhgroup = NULL;
    stbl_nexthop_t                  *p_sdb_nh = NULL;
    ctclib_slistnode_t              *listnode = NULL;
    ctclib_slistnode_t              *nn = NULL;
    
    CTC_SAI_DEBUG_FUNC();
    ctc_sai_remove_next_hop_group_debug_param(next_hop_group_id);

    p_sdb_nhgroup = ctc_nexthop_group_get_by_oid(next_hop_group_id);
    if (NULL == p_sdb_nhgroup)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    sdk_ret = ctc_nh_remove_ecmp(p_sdb_nhgroup->key.group_id);
    ret = ctc_sai_get_error_from_sdk_error(sdk_ret);
    if (!ret)
    {
        ctc_nexthop_group_nhg_cnt_dec();
    }

    ctclib_opb_free_offset(&p_sai_glb->nexthop_group_id_opb, CTC_SAI_OBJECT_INDEX_GET(p_sdb_nhgroup->key.group_id));

    CTCLIB_SLIST_LOOP_DEL(p_sdb_nhgroup->nh_list.obj_list, p_sdb_nh, listnode, nn)
    {
        stbl_nexthop_group_set_nh_list(p_sdb_nhgroup, FALSE, p_sdb_nh);
    }

	if(SAI_NEXT_HOP_GROUP_L3MC == p_sdb_nhgroup->type)
	{
		CTC_SAI_ERROR_GOTO(ctc_sai_l2mc_free_offset(p_sdb_nhgroup->ipmc_group_id), ret, out);
	}

    stbl_nexthop_group_del_nexthop_group(&p_sdb_nhgroup->key);
out:
    return ret;
}

/*
* Routine Description:
*    Set Next Hop Group attribute
*
* Arguments:
*    [in] sai_object_id_t - next_hop_group_id
*    [in] attr - attribute
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_set_next_hop_group_attribute(
    _In_ sai_object_id_t next_hop_group_id,
    _In_ const sai_attribute_t *attr)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    stbl_nexthop_group_t      *p_sdb_nhgroup = NULL;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_PTR_VALID_CHECK(attr);

    p_sdb_nhgroup = ctc_nexthop_group_get_by_oid(next_hop_group_id);
    if (NULL == p_sdb_nhgroup)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    p_sdb_nhgroup->ecmp_type   = attr[0].value.u32;
    p_sdb_nhgroup->failover_en = attr[1].value.booldata;

    /* sdk can't update ecmp hash type only, we need remove nhg first, then add once again */
    ret = __nexthop_group_update_ecmp_group(p_sdb_nhgroup);

    return ret;
}


/*
* Routine Description:
*    Get Next Hop Group attribute
*
* Arguments:
*    [in] sai_object_id_t - next_hop_group_id
*    [in] attr_count - number of attributes
*    [inout] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_get_next_hop_group_attribute(
    _In_ sai_object_id_t next_hop_group_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    CTC_SAI_DEBUG_FUNC();

    return SAI_STATUS_SUCCESS;
}

/*
* Routine Description:
*    Add next hop to a group
*
* Arguments:
*    [in] next_hop_group_id - next hop group id
*    [in] next_hop_count - number of next hops
*    [in] nexthops - array of next hops
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_add_next_hop_to_group(
    _In_ sai_object_id_t next_hop_group_id,
    _In_ uint32_t next_hop_count,
    _In_ const sai_object_id_t* nexthops)
{
    sai_status_t                    ret = SAI_STATUS_SUCCESS;
    stbl_nexthop_group_t            *p_sdb_nhgroup = NULL;

    CTC_SAI_DEBUG_FUNC();
    ctc_sai_add_next_hop_to_group_debug_param(next_hop_group_id, next_hop_count, nexthops);

    p_sdb_nhgroup = ctc_nexthop_group_get_by_oid(next_hop_group_id);
    if (NULL == p_sdb_nhgroup)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    ret = _ctc_sai_nhgroup_add_nexthop(p_sdb_nhgroup, next_hop_count, nexthops);
    return ret;
}


/*
* Routine Description:
*    Remove next hop from a group
*
* Arguments:
*    [in] next_hop_group_id - next hop group id
*    [in] next_hop_count - number of next hops
*    [in] nexthops - array of next hops
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_remove_next_hop_from_group(
    _In_ sai_object_id_t next_hop_group_id,
    _In_ uint32_t next_hop_count,
    _In_ const sai_object_id_t* nexthops)
{
    sai_status_t                    ret = SAI_STATUS_SUCCESS;
    stbl_nexthop_group_t            *p_sdb_nhgroup = NULL;

    CTC_SAI_DEBUG_FUNC();
    ctc_sai_remove_next_hop_from_group_debug_param(next_hop_group_id, next_hop_count, nexthops);

    p_sdb_nhgroup = ctc_nexthop_group_get_by_oid(next_hop_group_id);
    if (NULL == p_sdb_nhgroup)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    ret = _ctc_sai_nhgroup_remove_nexthop(p_sdb_nhgroup, next_hop_count, nexthops);
    return ret;
}


#define ________SAI_SAI_INNER_API_FUNC
static sai_status_t
__init_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;

    CTC_SAI_ERROR_GOTO(ctc_nexthop_group_db_init(),ret,out);

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

#define ________SAI_SAI_API_FUNC
/* define sai 0.9.2 */
static sai_next_hop_group_api_t      g_sai_api_func = {
    .create_next_hop_group          = ctc_sai_create_next_hop_group,
    .remove_next_hop_group          = ctc_sai_remove_next_hop_group,
    .set_next_hop_group_attribute   = ctc_sai_set_next_hop_group_attribute,
    .get_next_hop_group_attribute   = ctc_sai_get_next_hop_group_attribute,
    .add_next_hop_to_group          = ctc_sai_add_next_hop_to_group,
    .remove_next_hop_from_group     = ctc_sai_remove_next_hop_from_group,
};

static ctc_sai_api_reg_info_t g_api_reg_info = {
        .id             = SAI_API_NEXT_HOP_GROUP,
        .init_func      = __init_mode_fn,
        .exit_func      = __exit_mode_fn,
        .api_method_table = &g_sai_api_func,
        .private_data     = NULL,
};

#define ________SAI_OUTER_FUNC
sai_status_t
ctc_sai_nexthop_group_init()
{
    api_reg_register_fn(&g_api_reg_info);

    return SAI_STATUS_SUCCESS;
}
