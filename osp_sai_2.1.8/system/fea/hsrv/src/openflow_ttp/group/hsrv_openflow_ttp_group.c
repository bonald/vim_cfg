
#include "hsrv_inc.h"
#include "hsrv_openflow.h"
#include "hsrv_openflow_ttp_group.h"
#include "hsrv_openflow_ttp_group_db.h"
#include "ctc_ttp_drv_nexthop.h"
#include "ctc_ttp_drv_stats.h"



/*************************
 * Global and Declaration
 *************************/
static int32
_hsrv_openflow_group_add_bucket_entry(glb_openflow_bucket_t* actlist, uint32 bucket_index, uint32 group_id);


#define _____EXTERNAL_APIs_____

hsrv_openflow_ttp_group_t*
hsrv_openflow_group_get_group_info(uint32 group_id)
{
    hsrv_openflow_ttp_group_t *p_group = NULL;
    hsrv_openflow_groupdb_lookup_group(group_id, &p_group);
    return p_group;
}


#define _____INTERNAL_APIs_____

static int32
hsrv_openflow_group_get_group_first_bucket(uint32 group_id, hsrv_openflow_ttp_group_bucket_entry_t** pp_group_bucket)
{
    hsrv_openflow_ttp_group_bucket_entry_t *p_db_bucket = NULL;
    hsrv_bucket_index_t *p_bucket_index = NULL;
    hsrv_openflow_ttp_group_t* p_db_group = NULL;
    ctclib_list_node_t *p_node = NULL;

    hsrv_openflow_groupdb_lookup_group(group_id ,&p_db_group);
    if (p_db_group)
    {
        p_node = ctclib_list_head(&p_db_group->bucket_index_list);
        if (p_node)
        {
            p_bucket_index = ctclib_container_of(p_node, hsrv_bucket_index_t, node);
            hsrv_openflow_groupdb_lookup_group_bucket(p_bucket_index->bucket_index, &p_db_bucket);
            *pp_group_bucket = p_db_bucket;
        }
    }

    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_group_add_mcast_group(hsrv_openflow_ttp_group_t* p_group)
{
    uint32 bucket_index = 0;
    uint32 mc_group_id = 0;
    uint32 nhid = 0;
    hsrv_openflow_ttp_group_bucket_entry_t *p_db_bucket = NULL;
    hsrv_openflow_ttp_group_t* p_db_group = NULL;
    hsrv_bucket_index_t *p_bucket_index = NULL;
    ctclib_list_node_t *p_node = NULL, *p_next = NULL;
    ctclib_list_t *p_list = &p_group->bucket_index_list;

    HSRV_OPENFLOW_MEM_CHECK(p_group);

    /*TODO if fail, where release resouce?*/
    /*alloc mcast group id*/
    HSRV_IF_ERROR_RETURN(ctc_ttp_drv_nexthop_alloc_mc_groupid(&mc_group_id));
    p_group->mc_group_id = mc_group_id;

    /*alloc mcast group nh*/
    HSRV_IF_ERROR_RETURN(ctc_ttp_drv_nexthop_alloc_nhid(&nhid));
    p_group->nh_id = nhid;
    
    /*add asic mcast group*/
    HSRV_IF_ERROR_RETURN(ctc_ttp_drv_nexthop_add_mcast_nh(mc_group_id, nhid, p_group->stats_id));

    /*collect mcast group member nexthop*/
    ctclib_list_for_each_safe(p_node, p_next, p_list)
    {
        p_bucket_index = ctclib_container_of(p_node, hsrv_bucket_index_t, node);
        bucket_index = p_bucket_index->bucket_index;

        hsrv_openflow_groupdb_lookup_group_bucket(bucket_index, &p_db_bucket);
        if (p_db_bucket)
        {
            hsrv_openflow_groupdb_lookup_group(p_db_bucket->reference_group_id ,&p_db_group);
            if (p_db_group)
            {
                /*add asic mcast group member*/
                HSRV_IF_ERROR_RETURN(ctc_ttp_drv_nexthop_add_mcast_member(p_group->nh_id, p_db_group->nh_id));
            }
        }
    }
    
    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_group_del_mcast_group(hsrv_openflow_ttp_group_t* p_group)
{
    uint32 bucket_index = 0;
    hsrv_openflow_ttp_group_bucket_entry_t *p_db_bucket = NULL;
    hsrv_openflow_ttp_group_t* p_db_group = NULL;
    hsrv_bucket_index_t *p_bucket_index = NULL;
    ctclib_list_node_t *p_node = NULL, *p_next = NULL;
    ctclib_list_t *p_list = &p_group->bucket_index_list;
    
    HSRV_OPENFLOW_MEM_CHECK(p_group);

    /*remove mcast group member*/
    ctclib_list_for_each_safe(p_node, p_next, p_list)
    {
        p_bucket_index = ctclib_container_of(p_node, hsrv_bucket_index_t, node);
        bucket_index = p_bucket_index->bucket_index;

        hsrv_openflow_groupdb_lookup_group_bucket(bucket_index, &p_db_bucket);
        if (p_db_bucket)
        {
            hsrv_openflow_groupdb_lookup_group(p_db_bucket->reference_group_id ,&p_db_group);
            if (p_db_group)
            {
                HSRV_IF_ERROR_RETURN(ctc_ttp_drv_nexthop_del_mcast_member(p_group->nh_id, p_db_group->nh_id));
            }
        }
    }

    /*remove mcast group*/
    HSRV_IF_ERROR_RETURN(ctc_ttp_drv_nexthop_del_mcast_nh(p_group->nh_id));
    
    /*release group id*/
    HSRV_IF_ERROR_RETURN(ctc_ttp_drv_nexthop_release_mc_groupid(p_group->mc_group_id));

    /*release nexthop id*/
    HSRV_IF_ERROR_RETURN(ctc_ttp_drv_nexthop_release_nhid(p_group->nh_id));

    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_group_add_ecmp_group(hsrv_openflow_ttp_group_t* p_group)
{
    uint32 bucket_index = 0;
    uint32 nhid = 0;
    uint32 ecmp_member[GLB_OPENFLOW_TTP_MAX_BUCKET_PER_ECMP_GROUP] = {0};
    uint32 count = 0;
    hsrv_openflow_ttp_group_bucket_entry_t *p_db_bucket = NULL;
    hsrv_openflow_ttp_group_t* p_db_group = NULL;
    hsrv_bucket_index_t *p_bucket_index = NULL;
    ctclib_list_node_t *p_node = NULL, *p_next = NULL;
    ctclib_list_t *p_list = &p_group->bucket_index_list;

    HSRV_OPENFLOW_MEM_CHECK(p_group);

    /*alloc ecmp nh*/
    HSRV_IF_ERROR_RETURN(ctc_ttp_drv_nexthop_alloc_nhid(&nhid));
    p_group->nh_id = nhid;
    
    /*add asic ecmp nh*/
    HSRV_IF_ERROR_RETURN(ctc_ttp_drv_nexthop_add_ecmp_nh(nhid, p_group->stats_id));

    /*collect ecmp member nexthop*/
    ctclib_list_for_each_safe(p_node, p_next, p_list)
    {
        p_bucket_index = ctclib_container_of(p_node, hsrv_bucket_index_t, node);
        bucket_index = p_bucket_index->bucket_index;

        hsrv_openflow_groupdb_lookup_group_bucket(bucket_index, &p_db_bucket);
        if (p_db_bucket)
        {
            hsrv_openflow_groupdb_lookup_group(p_db_bucket->reference_group_id ,&p_db_group);
            if (p_db_group)
            {
                ecmp_member[count] = p_db_group->nh_id;
                count ++;
            }
        }
    }

    /*add asic mcast group member*/
    HSRV_IF_ERROR_RETURN(ctc_ttp_drv_nexthop_add_ecmp_member(p_group->nh_id, ecmp_member, count));
    
    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_group_del_ecmp_group(hsrv_openflow_ttp_group_t* p_group)
{
    uint32 bucket_index = 0;
    uint32 ecmp_member[GLB_OPENFLOW_TTP_MAX_BUCKET_PER_ECMP_GROUP] = {0};
    uint32 count = 0;
    hsrv_openflow_ttp_group_bucket_entry_t *p_db_bucket = NULL;
    hsrv_openflow_ttp_group_t* p_db_group = NULL;
    hsrv_bucket_index_t *p_bucket_index = NULL;
    ctclib_list_node_t *p_node = NULL, *p_next = NULL;
    ctclib_list_t *p_list = &p_group->bucket_index_list;
    
    HSRV_OPENFLOW_MEM_CHECK(p_group);

    /*remove ecmp group member*/
    ctclib_list_for_each_safe(p_node, p_next, p_list)
    {
        p_bucket_index = ctclib_container_of(p_node, hsrv_bucket_index_t, node);
        bucket_index = p_bucket_index->bucket_index;

        hsrv_openflow_groupdb_lookup_group_bucket(bucket_index, &p_db_bucket);
        if (p_db_bucket)
        {
            hsrv_openflow_groupdb_lookup_group(p_db_bucket->reference_group_id ,&p_db_group);
            if (p_db_group)
            {
                ecmp_member[count] = p_db_group->nh_id;
                count ++;
            }
        }
    }

    /*remove ecmp group member*/
    HSRV_IF_ERROR_RETURN(ctc_ttp_drv_nexthop_del_ecmp_member(p_group->nh_id, ecmp_member, count));

    /*remove ecmp group nexthop*/
    HSRV_IF_ERROR_RETURN(ctc_ttp_drv_nexthop_del_ecmp_nh(p_group->nh_id));

    /*release nexthop id*/
    HSRV_IF_ERROR_RETURN(ctc_ttp_drv_nexthop_release_nhid(p_group->nh_id));

    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_group_add_l3_unicast_group(hsrv_openflow_ttp_group_t* p_group)
{
    uint32 bucket_index = 0;
    uint32 nhid = 0;
    hsrv_openflow_ttp_group_bucket_entry_t *p_l2_bucket = NULL, *p_l3_bucket = NULL;
    hsrv_bucket_index_t *p_bucket_index = NULL;
    ctclib_list_node_t *p_node = NULL, *p_next = NULL;
    ctclib_list_t *p_list = &p_group->bucket_index_list;

    HSRV_OPENFLOW_MEM_CHECK(p_group);

    /*alloc nhid*/
    HSRV_IF_ERROR_RETURN(ctc_ttp_drv_nexthop_alloc_nhid(&nhid));
    p_group->nh_id = nhid;

    //ipuc nexthop not support stats

    /*add l3 unicast nh*/
    ctclib_list_for_each_safe(p_node, p_next, p_list)
    {
        p_bucket_index = ctclib_container_of(p_node, hsrv_bucket_index_t, node);
        bucket_index = p_bucket_index->bucket_index;

        hsrv_openflow_groupdb_lookup_group_bucket(bucket_index, &p_l3_bucket);
        if (p_l3_bucket)
        {
            hsrv_openflow_group_get_group_first_bucket(p_l3_bucket->reference_group_id ,&p_l2_bucket);
            if (p_l2_bucket)
            {
                /*add asic nh*/
                HSRV_IF_ERROR_RETURN(ctc_ttp_drv_nexthop_add_ipuc_nh(p_l3_bucket, p_l2_bucket, p_group->nh_id));
            }
        }
    }
    
    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_group_del_l3_unicast_group(hsrv_openflow_ttp_group_t* p_group)
{
    HSRV_OPENFLOW_MEM_CHECK(p_group);

    /*remove l3 unicast nexthop*/
    HSRV_IF_ERROR_RETURN(ctc_ttp_drv_nexthop_del_ipuc_nh(p_group->nh_id));

    /*release nexthop id*/
    HSRV_IF_ERROR_RETURN(ctc_ttp_drv_nexthop_release_nhid(p_group->nh_id));
    
    return HSRV_E_NONE;
}


static int32
_hsrv_openflow_group_add_l2_interface_group(hsrv_openflow_ttp_group_t* p_group)
{
    uint32 bucket_index = 0;
    uint32 nhid = 0;
    hsrv_openflow_ttp_group_bucket_entry_t *p_db_bucket = NULL;
    hsrv_bucket_index_t *p_bucket_index = NULL;
    ctclib_list_node_t *p_node = NULL, *p_next = NULL;
    ctclib_list_t *p_list = &p_group->bucket_index_list;

    HSRV_OPENFLOW_MEM_CHECK(p_group);

    /*alloc nhid*/
    HSRV_IF_ERROR_RETURN(ctc_ttp_drv_nexthop_alloc_nhid(&nhid));
    p_group->nh_id = nhid;

    /*add l2 interface nh*/
    ctclib_list_for_each_safe(p_node, p_next, p_list)
    {
        p_bucket_index = ctclib_container_of(p_node, hsrv_bucket_index_t, node);
        bucket_index = p_bucket_index->bucket_index;

        hsrv_openflow_groupdb_lookup_group_bucket(bucket_index, &p_db_bucket);
        if (p_db_bucket)
        {
            /*add asic nh*/
            HSRV_IF_ERROR_RETURN(ctc_ttp_drv_nexthop_add_vlan_edit_nh(p_db_bucket, p_group->nh_id, p_group->stats_id));
        }
    }
    
    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_group_del_l2_interface_group(hsrv_openflow_ttp_group_t* p_group)
{
    HSRV_OPENFLOW_MEM_CHECK(p_group);

    /*remove l2 interface nexthop*/
    HSRV_IF_ERROR_RETURN(ctc_ttp_drv_nexthop_del_vlan_edit_nh(p_group->nh_id));

    /*release nexthop id*/
    HSRV_IF_ERROR_RETURN(ctc_ttp_drv_nexthop_release_nhid(p_group->nh_id));

    //TODO dec reference count
    
    return HSRV_E_NONE;
}


static int32
_hsrv_openflow_group_add_group(hsrv_openflow_ttp_group_t* p_group)
{
    HSRV_OPENFLOW_PTR_CHECK(p_group);

    switch (GLB_OPENFLOW_TTP_GROUP_TYPE(p_group->group_id))
    {
        case GLB_OPENFLOW_TTP_GROUP_TYPE_L2_FLOOD:
            HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_add_mcast_group(p_group));
            break;
        case GLB_OPENFLOW_TTP_GROUP_TYPE_L3_ECMP:
            HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_add_ecmp_group(p_group));
            break;
        case GLB_OPENFLOW_TTP_GROUP_TYPE_L3_UNICAST:
            HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_add_l3_unicast_group(p_group));
            break;
        case GLB_OPENFLOW_TTP_GROUP_TYPE_L2_INTERFACE:
            HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_add_l2_interface_group(p_group));
            break;
        default:
            break;
    }
    
    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_group_del_group(hsrv_openflow_ttp_group_t* p_group)
{
    HSRV_OPENFLOW_PTR_CHECK(p_group);

    switch (GLB_OPENFLOW_TTP_GROUP_TYPE(p_group->group_id))
    {
        case GLB_OPENFLOW_TTP_GROUP_TYPE_L2_FLOOD:
            HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_del_mcast_group(p_group));
            break;
        case GLB_OPENFLOW_TTP_GROUP_TYPE_L3_ECMP:
            HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_del_ecmp_group(p_group));
            break;
        case GLB_OPENFLOW_TTP_GROUP_TYPE_L3_UNICAST:
            HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_del_l3_unicast_group(p_group));
            break;
        case GLB_OPENFLOW_TTP_GROUP_TYPE_L2_INTERFACE:
            HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_del_l2_interface_group(p_group));
            break;
        default:
            break;
    }
    
    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_group_add_bucket_entry(glb_openflow_bucket_t* p_bucket, uint32 bucket_index, uint32 group_id)
{
    uint32 action_num = 0;
    uint32 set_group_id = 0;
    uint32 is_set_group_id = 0;
    uint32 ttl_dec = 0;
    uint32 ifindex = 0;
    uint16 set_vlan = 0;
    uint32 push_vlan = 0;
    uint32 strip_vlan = 0;
    uint8 dst_mac[MAC_ADDR_LEN];
    uint8 src_mac[MAC_ADDR_LEN];
    hsrv_openflow_ttp_port_info_t *p_port = NULL; 
    
    hsrv_openflow_ttp_group_bucket_entry_t  *p_hsrv_bucket = NULL;
    glb_openflow_flow_action_t *p_action = NULL;
    hsrv_openflow_ttp_group_t  *p_group = NULL;

    sal_memset(dst_mac, 0, sizeof(dst_mac));
    sal_memset(src_mac, 0, sizeof(src_mac));
    
    for(action_num = 0; action_num < p_bucket->n_actions; action_num ++)
    {
        p_action = p_bucket->actlist + action_num;
        switch(p_action->type)
        {
            case GLB_OPENFLOW_ACTION_TYPE_OUTPUT:
                ifindex = p_action->value.output.ifindex;
                break;

            case GLB_OPENFLOW_ACTION_TYPE_GROUP:
                set_group_id = p_action->value.u32;
                hsrv_openflow_groupdb_lookup_group(set_group_id, &p_group);
                if (!p_group)
                {
                    return HSRV_E_ENTRY_NOT_EXIST;
                }
                is_set_group_id = TRUE;
                break;

            case GLB_OPENFLOW_ACTION_TYPE_DEC_TTL:
                ttl_dec = TRUE;
                break;

            case GLB_OPENFLOW_ACTION_TYPE_PUSH_VLAN:
                push_vlan = TRUE;
                break;

            case GLB_OPENFLOW_ACTION_TYPE_SET_VLAN_VID:
                set_vlan = p_action->value.u16;
                break;

            case GLB_OPENFLOW_ACTION_TYPE_STRIP_VLAN:
                strip_vlan = TRUE;
                break;

            case GLB_OPENFLOW_ACTION_TYPE_SET_ETH_SRC:
                sal_memcpy(src_mac, p_action->value.mac, sizeof(p_action->value.mac));
                break;

            case GLB_OPENFLOW_ACTION_TYPE_SET_ETH_DST:
                sal_memcpy(dst_mac, p_action->value.mac, sizeof(p_action->value.mac));
                break;

            default:
                break;
        }

    }
    p_hsrv_bucket = HSRV_OPENFLOW_GROUP_MALLOC(sizeof(hsrv_openflow_ttp_group_bucket_entry_t));
    sal_memset(p_hsrv_bucket, 0 , sizeof(hsrv_openflow_ttp_group_bucket_entry_t));
    HSRV_OPENFLOW_MEM_CHECK(p_hsrv_bucket);
    p_hsrv_bucket->group_id = group_id;
    p_hsrv_bucket->bucket_index = bucket_index;
    if (GLB_OPENFLOW_TTP_GROUP_TYPE_L2_INTERFACE == GLB_OPENFLOW_TTP_GROUP_TYPE(group_id))
    {
        p_port = hsrv_openflow_port_get_port_info_by_ifindex(ifindex);
        if (!p_port)
        {
            return HSRV_E_PORT_NOT_EXIST;
        }
        p_hsrv_bucket->bucket_data.l2_interface.output_port = p_port->gport;
        
        if (push_vlan)
        {
            p_hsrv_bucket->bucket_data.l2_interface.push_vlan_tag = 1;
            p_hsrv_bucket->bucket_data.l2_interface.vlan_id = set_vlan;
        }
        else if(set_vlan) //TODO vlan0?
        {
            p_hsrv_bucket->bucket_data.l2_interface.vlan_id = set_vlan;
        }
        else if (strip_vlan)
        {
            p_hsrv_bucket->bucket_data.l2_interface.pop_vlan_tag = 1;
        }
        
    }
    else if (GLB_OPENFLOW_TTP_GROUP_TYPE_L3_UNICAST == GLB_OPENFLOW_TTP_GROUP_TYPE(group_id))
    {
        //TODO think group_id ==0 and route_mac,or port mac
        sal_memcpy(p_hsrv_bucket->bucket_data.l3_unicast.src_mac, src_mac, sizeof(src_mac));
        sal_memcpy(p_hsrv_bucket->bucket_data.l3_unicast.dst_mac, dst_mac, sizeof(dst_mac));
        p_hsrv_bucket->bucket_data.l3_unicast.l2_interface_group_id = set_group_id;
        p_hsrv_bucket->reference_group_id = set_group_id;
    }
    else if (GLB_OPENFLOW_TTP_GROUP_TYPE_L2_FLOOD == GLB_OPENFLOW_TTP_GROUP_TYPE(group_id))
    {
        p_hsrv_bucket->reference_group_id = set_group_id;
    }
    else if (GLB_OPENFLOW_TTP_GROUP_TYPE_L3_ECMP == GLB_OPENFLOW_TTP_GROUP_TYPE(group_id))
    {
        p_hsrv_bucket->reference_group_id = set_group_id;
    }
    
    hsrv_openflow_groupdb_add_group_bucket(p_hsrv_bucket);
    
    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_group_alloc_group(hsrv_openflow_ttp_group_t* pp_group, glb_openflow_group_t* p_fei_group)
{
    uint32 bucket_idx = 0;
    uint32 bucket_id = 0;
    glb_openflow_bucket_t* p_bucket = NULL;
    hsrv_bucket_index_t *p_bucket_index = NULL;


    HSRV_OPENFLOW_PTR_CHECK(pp_group);
    HSRV_OPENFLOW_PTR_CHECK(p_fei_group);

    pp_group->group_id = p_fei_group->group_id;
    pp_group->bucket_count = p_fei_group->n_buckets;
    ctclib_list_init(&pp_group->bucket_index_list);

    for (bucket_idx = 0; bucket_idx < p_fei_group->n_buckets; bucket_idx ++)
    {
        p_bucket = &p_fei_group->bucket_list[bucket_idx];

        if (!p_bucket->n_actions)
        {
            continue;
        }
        HSRV_IF_ERROR_RETURN(hsrv_openflow_groupdb_alloc_bucket_index(&bucket_id));
        HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_add_bucket_entry(p_bucket, bucket_id, p_fei_group->group_id));
        p_bucket_index = HSRV_OPENFLOW_GROUP_MALLOC(sizeof(hsrv_bucket_index_t));
        HSRV_OPENFLOW_MEM_CHECK(p_bucket_index);
        p_bucket_index->bucket_index = bucket_id;
        ctclib_list_insert_tail(&pp_group->bucket_index_list, &p_bucket_index->node);
    }

    /* l3 unicast group not support stats */
    if (GLB_OPENFLOW_TTP_GROUP_TYPE_L3_UNICAST != GLB_OPENFLOW_TTP_GROUP_TYPE(pp_group->group_id))
    {
        pp_group->stats_supported = 1;
    }
    
    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_group_release_group(hsrv_openflow_ttp_group_t* p_group)
{
    uint32 bucket_index = 0;
    hsrv_openflow_ttp_group_bucket_entry_t *p_bucket = NULL;
    hsrv_bucket_index_t *p_bucket_index = NULL;
    ctclib_list_node_t *p_node = NULL, *p_next = NULL;
    ctclib_list_t *p_list = &p_group->bucket_index_list;

    HSRV_OPENFLOW_MEM_CHECK(p_group);
    
    ctclib_list_for_each_safe(p_node, p_next, p_list)
    {
        p_bucket_index = ctclib_container_of(p_node, hsrv_bucket_index_t, node);
        bucket_index = p_bucket_index->bucket_index;
        ctclib_list_delete(p_list, p_node);
        HSRV_OPENFLOW_GROUP_FREE(p_bucket_index);

        /*release bucket entry*/
        hsrv_openflow_groupdb_lookup_group_bucket(bucket_index, &p_bucket);
        if (p_bucket)
        {
            hsrv_openflow_groupdb_del_group_bucket(bucket_index);
            hsrv_openflow_groupdb_release_bucket_index(bucket_index);
            
            HSRV_OPENFLOW_GROUP_FREE(p_bucket);
        }
    }

    /* release hsrv_openflow_ttp_group_t */
    HSRV_OPENFLOW_GROUP_FREE(p_group);
    p_group = NULL;

    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_group_create_stats(hsrv_openflow_ttp_group_t* p_group)
{
    uint32 stats_id = 0;
    /*alloc stats*/
    if (p_group->stats_supported)
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_add_stats(GLB_STATS_TYPE_OPENFLOW_GROUP, p_group->group_id, p_group->group_id, &stats_id));
        p_group->stats_id = stats_id;
    }
    
    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_group_delete_stats(hsrv_openflow_ttp_group_t* p_group)
{
    if (p_group->stats_supported)
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_del_stats(p_group->stats_id));
        p_group->stats_id = 0;
    }

    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_group_clear_stats(hsrv_openflow_ttp_group_t* p_group)
{
    if (p_group->stats_supported)
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_clear_stats(p_group->stats_id));
    }

    return HSRV_E_NONE;
}

#define _____HAL_APIs_____


int32
fei_api_openflow_add_group(glb_openflow_group_t *p_fei_group)
{
    int32 ret = HSRV_E_NONE;
    hsrv_openflow_ttp_group_t* p_group = NULL;

    HSRV_IF_ERROR_RETURN(hsrv_openflow_groupdb_lookup_group(p_fei_group->group_id, &p_group));
    if (p_group)
    {
        return HSRV_E_NONE;
    }

    /* 1. alloc group info */
    p_group = HSRV_OPENFLOW_GROUP_MALLOC(sizeof(hsrv_openflow_ttp_group_t));
    HSRV_OPENFLOW_MEM_CHECK(p_group);
    sal_memset(p_group, 0, sizeof(hsrv_openflow_ttp_group_t));
    ret = _hsrv_openflow_group_alloc_group(p_group, p_fei_group);
    if (ret)
    {
        _hsrv_openflow_group_release_group(p_group);
        return ret;
    }

    /* 2.alloc stats */
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_create_stats(p_group));

    /* 3. alloc mcast/ecmp/redirect nhid */
    ret = _hsrv_openflow_group_add_group(p_group);
    if (ret)
    {
        HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_delete_stats(p_group));
        HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_del_group(p_group));
        HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_release_group(p_group));
        return ret;
    }
    
    /* 4. add to groupdb */
    hsrv_openflow_groupdb_add_group(p_group);

    return HSRV_E_NONE;
}

int32
fei_api_openflow_del_group(fei_openflow_del_group_req_t* p_fei_group)
{

    hsrv_openflow_ttp_group_t* p_group = NULL;

    HSRV_IF_ERROR_RETURN(hsrv_openflow_groupdb_lookup_group(p_fei_group->group_id, &p_group));
    if (NULL == p_group)
    {
        HSRV_OPENFLOW_LOG_ERR("Fail to delete group %u", p_fei_group->group_id);
        return HSRV_E_NONE;
    }
    
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_del_group(p_group));

    HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_delete_stats(p_group));

    HSRV_IF_ERROR_RETURN(hsrv_openflow_groupdb_del_group(p_group->group_id));
    
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_release_group(p_group));

    return HSRV_E_NONE;
}

int32
fei_api_openflow_clear_group_stats(fei_openflow_clear_stats_req_t *pst_req)
{
    hsrv_openflow_ttp_group_t* p_group = NULL;

    HSRV_IF_ERROR_RETURN(hsrv_openflow_groupdb_lookup_group(pst_req->id, &p_group));
    if (NULL == p_group)
    {
        return HSRV_E_NONE;
    }

    HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_clear_stats(p_group));

    return HSRV_E_NONE;
}


#define _____GROUP_DEBUG_____

#if 0
static void
hsrv_openflow_show_group_brief__(ctclib_hash_backet_t* phb, void *user_data)
{
    uint32 member_num = 0;
    uint32 member_idx;

    hsrv_openflow_group_t* p_group = (hsrv_openflow_group_t*)phb->data;
    hsrv_openflow_group_debug_data_t* p_data = user_data;

    for (member_idx = 0; member_idx < p_group->u.dft.nh_info.bucket_count; member_idx ++)
    {
        member_num += p_group->u.dft.nh_info.buckets[member_idx].member_count;
    }
                                                       
    sal_fprintf(p_data->fp, "%8u %8s %9u %9u %6u %8u\n",
            p_group->group.group_id,
            HSRV_OF_GROUP_TYPE_STR(p_group->group.group_type),
            p_group->group.n_buckets,
            p_group->u.dft.nh_info.main_nh.nh_offset.nhid,
            member_num,
            p_group->u.dft.stats_id);
}


int32
hsrv_openflow_show_group_brief(FILE *fp)
{
    hsrv_openflow_group_master_t* p_master = g_pst_openflow_group_master;
    hsrv_openflow_group_debug_data_t data;

    sal_memset(&data, 0, sizeof(hsrv_openflow_group_debug_data_t));
    sal_fprintf(fp, "-------------HSRV OPENFLOW GROUP BRIEF---------------------\n");
    sal_fprintf(fp, "Group hash count : %d\n", p_master->p_group_hash->count);
    sal_fprintf(fp, "Drop_pkt_to_ingress_port : %s\n",
                    p_master->drop_pkt_to_ingress_port ? "enable" : "disable");
    sal_fprintf(fp, "\n");
    sal_fprintf(fp, "%8s   %4s   %9s %9s %6s %8s\n",
                    "group_id", "type", "n_buckets", "main_nhid", "member", "stats_id");
    sal_fprintf(fp, "-------- -------- --------- --------- ------ --------\n");

    data.fp = fp;
    ctclib_hash_iterate(HSRV_OPENFLOW_GROUP_HASH, hsrv_openflow_show_group_brief__, &data);

    return HSRV_E_NONE;
}

int32
hsrv_openflow_show_group(FILE *fp, uint32 group_id)
{
    uint32 bucket_idx = 0;
    uint32 member_idx = 0;
    uint32 n_buckets = 0;
    uint32 n_members = 0;
    hsrv_openflow_group_t* p_group;
    glb_openflow_flow_action_list_t actlist;

    HSRV_IF_ERROR_RETURN(hsrv_openflow_groupdb_lookup_group(group_id, &p_group));
    if (NULL == p_group)
    {
        sal_fprintf(fp, "group_id %u not exist in db.\n", group_id);
        return HSRV_E_NONE;
    }

    n_buckets =  p_group->group.n_buckets;
    sal_fprintf(fp, "----------HSRV OPENFLOW GROUP--------\n");
    sal_fprintf(fp, "group_id      : %u\n", group_id);
    sal_fprintf(fp, "group_type    : %s\n", HSRV_OF_GROUP_TYPE_STR(p_group->group.group_type));
    sal_fprintf(fp, "n_buckets     : %u\n", n_buckets);
    sal_fprintf(fp, "stats_id      : %u\n", p_group->u.dft.stats_id);
    sal_fprintf(fp, "use_mcast     : %u\n", p_group->u.dft.nh_info.use_mcast);
    sal_fprintf(fp, "use_ecmp      : %u\n", p_group->u.dft.nh_info.use_ecmp);
    sal_fprintf(fp, "use_linkagg   : %u\n", p_group->u.dft.nh_info.use_linkagg);
    if (GLB_OF_GROUP_TYPE_SELECT == p_group->group.group_type)
    {
        sal_fprintf(fp, "tid           : %u\n", p_group->u.dft.nh_info.linkagg_nh.tid);
    }
    if (GLB_OF_GROUP_TYPE_FF == p_group->group.group_type)
    {
        bucket_idx = p_group->group.choose_bucket_idx;
        if (bucket_idx == UINT32_MAX)
        {
            sal_fprintf(fp, "live bucket idx : UINT32_MAX ");
            sal_fprintf(fp, "watch port : UINT32_MAX\n");
        }
        else
        {
            sal_fprintf(fp, "live bucket idx : %u ", bucket_idx + 1);
            sal_fprintf(fp, "watch port : %u\n", p_group->group.bucket_list[bucket_idx].watch_port);
        }
    }
    sal_fprintf(fp, "nested_group  :");
    for (member_idx = 0; member_idx < p_group->group.nested_group_count; member_idx ++)
    {
        sal_fprintf(fp, " %d,", p_group->group.nested_group[member_idx]);
    }
    sal_fprintf(fp, "\n");

    sal_fprintf(fp, "-------Main Nexthop-------\n");
    sal_fprintf(fp, "nh_type : %s\n", HSRV_OF_NH_TYPE_STR(p_group->u.dft.nh_info.main_nh.nh_offset.nh_type));
    sal_fprintf(fp, "nhid    : %u\n", p_group->u.dft.nh_info.main_nh.nh_offset.nhid);
    sal_fprintf(fp, "offset  : %u\n", p_group->u.dft.nh_info.main_nh.nh_offset.offset);
    sal_fprintf(fp, "---------Buckets----------\n");
    for (bucket_idx = 0; bucket_idx < n_buckets; bucket_idx++)
    {
        sal_fprintf(fp, "bucket idx : %u\n", bucket_idx + 1);
        actlist.action_count = p_group->group.bucket_list[bucket_idx].n_actions;
        actlist.actlist = p_group->group.bucket_list[bucket_idx].actlist;

        n_members = p_group->u.dft.nh_info.buckets[bucket_idx].member_count;
        sal_fprintf(fp, "nh member count : %u\n", n_members);
        sal_fprintf(fp, "member nhid :");
        for (member_idx = 0; member_idx < n_members; member_idx ++)
        {
            hsrv_of_nh_offset_t *nh_offset;
            nh_offset = &p_group->u.dft.nh_info.buckets[bucket_idx].member_nh[member_idx].nh_offset;
            sal_fprintf(fp, " %u,", nh_offset->nhid);
        }
        sal_fprintf(fp, "\n");
        sal_fprintf(fp, "actions :");
        hsrv_openflow_show_flow_actlist_actions__(fp, &actlist, 2);
        sal_fprintf(fp, "\n\n");
    }
    sal_fprintf(fp, "-------------------------------------\n");
    return HSRV_E_NONE;
}
#endif

#define _____GROUP_INIT_____

int32
hsrv_openflow_group_start(void)
{
    HSRV_IF_ERROR_RETURN(hsrv_openflow_groupdb_start());

    return HSRV_E_NONE;
}

