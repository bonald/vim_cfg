/****************************************************************************************
 * hsrv_openflow.c   :  Hal server openflow related function
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision     :       R0.01
 * Author       :       Yan Xing'an
 * Date         :       2014-12-15
 * Reason       :       First Create.
 ****************************************************************************************/
#include "hsrv_inc.h"
#include "hsrv_openflow.h"
#include "hsrv_openflow_flow.h"
#include "hsrv_msg_openflow.h"
#include "hsrv_openflow_group_priv.h"
#include "hsrv_openflow_group.h"
#include "hsrv_openflow_port.h"
#include "hsrv_openflow_linkagg.h"
#include "hsrv_openflow_tpoam.h"


/*************************
 * Global and Declaration
 *************************/
extern hsrv_openflow_stats_master_t* g_pst_openflow_stats_master;

typedef struct 
    {
        hsrv_openflow_action_combo_t action_combo;
        glb_openflow_metadata_t metadata;
        uint32 mpls_output_action_seq_type;
        uint32 mpls_label;
        uint32 mpls_tc;
        uint32 mpls_ttl;
    }_hsrv_openflow_action_combo_t;

static int32
_hsrv_openflow_group_release_buckets(hsrv_openflow_group_t* p_group);

static int32
_hsrv_openflow_group_allocate_buckets(hsrv_openflow_group_t* p_group);

static int32
_hsrv_openflow_group_add_linkagg_group(hsrv_openflow_group_t* p_group);

static int32
_hsrv_openflow_group_add_buckets_to_linkagg_group(hsrv_openflow_group_t* p_group);

static int32
_hsrv_openflow_group_get_del_nexthop(hsrv_openflow_group_t* p_new_group, hsrv_openflow_group_t* p_old_group, uint32 *nhid_del_array, uint32 *nhid_del_count);

extern int32
_hsrv_openflow_tunnel_lookup_info(uint32 ifindex, hsrv_openflow_tunnel_info_t** pp_info);

#define _____INTERNAL_COMMON_APIs_____
/*************************************************************************************************
 * Name         : _hsrv_openflow_add_nested_group_member_one
 * Purpose      : Fea layer group add nested group member one
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_add_nested_group_member_one(hsrv_openflow_group_t* p_group, hsrv_openflow_group_t* p_nested_group)
{
    uint32 bucket_idx;
    uint32 member_idx;
    hsrv_ofnh_mcast_member_param_t member_param;
    hsrv_of_nh_info_t* p_member = NULL;
    hsrv_openflow_group_nexthop_info_t* p_nh_info = NULL;

    p_nh_info = &p_nested_group->u.dft.nh_info;
    if (GLB_OF_GROUP_TYPE_ALL == p_nested_group->group.group_type)
    {
        for (bucket_idx = 0; bucket_idx < p_nh_info->bucket_count; bucket_idx++)
        {
            for (member_idx = 0; member_idx < p_nh_info->buckets[bucket_idx].member_count; member_idx++)
            {
                p_member = &p_nh_info->buckets[bucket_idx].member_nh[member_idx];
                sal_memset(&member_param, 0, sizeof(hsrv_ofnh_mcast_member_param_t));
                member_param.group_nhid = p_group->u.dft.nh_info.main_nh.nh_offset.nhid;
                member_param.op_type = HSRV_OFNH_MCAST_OP_ADD_MEMBER;
                member_param.mem_gport = p_member->gport;
                member_param.mem_nhid = p_member->nh_offset.nhid;
                member_param.port_check_discard = p_member->nh_offset.port_check_discard;
                member_param.redirect_to_ingress_port = p_member->nh_offset.redirect_to_ingress_port;
                HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_add_mcast_member(&member_param));
            }
        }
    }
    else if (GLB_OF_GROUP_TYPE_FF == p_nested_group->group.group_type)
    {
        if (p_nested_group->group.choose_bucket_idx != UINT32_MAX)
        {
            for (member_idx = 0; member_idx < p_nh_info->buckets[p_nested_group->group.choose_bucket_idx].member_count; member_idx++)
            {
                p_member = &p_nh_info->buckets[p_nested_group->group.choose_bucket_idx].member_nh[member_idx];
                sal_memset(&member_param, 0, sizeof(hsrv_ofnh_mcast_member_param_t));
                member_param.group_nhid = p_group->u.dft.nh_info.main_nh.nh_offset.nhid;
                member_param.op_type = HSRV_OFNH_MCAST_OP_ADD_MEMBER;
                member_param.mem_gport = p_member->gport;
                member_param.mem_nhid = p_member->nh_offset.nhid;
                member_param.port_check_discard = p_member->nh_offset.port_check_discard;
                member_param.redirect_to_ingress_port = p_member->nh_offset.redirect_to_ingress_port;
                HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_add_mcast_member(&member_param));
            }
        }
    }
    return HSRV_E_NONE;
}
/*************************************************************************************************
 * Name         : _hsrv_openflow_add_nested_group_member
 * Purpose      : Fea layer group add nested group member
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_add_nested_group_member(hsrv_openflow_group_t* p_group)
{
#if 0
    uint32 group_id;
    uint32 group_idx;
    uint16 linkagg_port = 0;
    hsrv_ofnh_mcast_member_param_t member_param;
    hsrv_openflow_group_t* p_nested_group = NULL;
    hsrv_openflow_group_nexthop_info_t* p_nh_info = NULL;

    for (group_idx = 0; group_idx < p_group->group.nested_group_count; group_idx ++)
    {
        group_id = p_group->group.nested_group[group_idx];
        HSRV_IF_ERROR_RETURN(hsrv_openflow_groupdb_lookup_group(group_id, &p_nested_group));
        if (!p_nested_group)
        {
            continue;
        }

        p_nh_info = &p_nested_group->u.dft.nh_info;
        linkagg_port = CTC_MAP_TID_TO_GPORT(p_nh_info->linkagg_nh.tid);

        sal_memset(&member_param, 0, sizeof(hsrv_ofnh_mcast_member_param_t));
        member_param.group_nhid = p_group->u.dft.nh_info.main_nh.nh_offset.nhid;
        member_param.op_type = HSRV_OFNH_MCAST_OP_ADD_MEMBER;
        member_param.mem_gport = linkagg_port;
        member_param.mem_nhid = p_nh_info->linkagg_nh.nh_offset.nhid;
        member_param.port_check_discard = hsrv_openflow_group_get_drop_ingress();
        member_param.redirect_to_ingress_port = 0;
        HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_add_mcast_member(&member_param));
    }
#endif
    /*added by hansf for support group chain:all+all,all+ff*/
    uint32 group_id;
    uint32 group_idx;
    hsrv_openflow_group_t* p_nested_group = NULL;
    
    for (group_idx = 0; group_idx < p_group->group.nested_group_count; group_idx ++)
    {
        group_id = p_group->group.nested_group[group_idx];
        HSRV_IF_ERROR_RETURN(hsrv_openflow_groupdb_lookup_group(group_id, &p_nested_group));
        if (!p_nested_group)
        {
            return HSRV_E_ENTRY_NOT_EXIST;
        }
        HSRV_IF_ERROR_RETURN(_hsrv_openflow_add_nested_group_member_one(p_group, p_nested_group));
    }

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_del_nested_group_member_one
 * Purpose      : Fea layer group delete nested group member one
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_del_nested_group_member_one(hsrv_openflow_group_t* p_group, hsrv_openflow_group_t* p_nested_group)
{
    uint32 bucket_idx;
    uint32 member_idx;
    hsrv_openflow_group_nexthop_info_t* p_nh_info = NULL;

    p_nh_info = &p_nested_group->u.dft.nh_info;
    for (bucket_idx = 0; bucket_idx < p_nh_info->bucket_count; bucket_idx++)
    {
        if (GLB_OF_GROUP_TYPE_FF == p_nested_group->group.group_type)
        {
            /* check ff group first member */
            if (p_nested_group->group.choose_bucket_idx != UINT32_MAX &&
                p_nested_group->group.choose_bucket_idx != bucket_idx)
            {
                continue;
            }

            /* check ff group all bucket invalid */
            if (p_nested_group->group.choose_bucket_idx == UINT32_MAX)
            {
                continue;
            }
        }
 
        for (member_idx = 0; member_idx < p_nh_info->buckets[bucket_idx].member_count; member_idx++)
        {
            hsrv_of_nh_info_t *member_nh;
            hsrv_ofnh_mcast_member_param_t param;

            member_nh = &p_nh_info->buckets[bucket_idx].member_nh[member_idx];
            param.group_nhid = p_group->u.dft.nh_info.main_nh.nh_offset.nhid;
            param.op_type = HSRV_OFNH_MCAST_OP_DEL_MEMBER;
            param.mem_gport = member_nh->gport;
            param.mem_nhid = member_nh->nh_offset.nhid;
            param.port_check_discard = member_nh->nh_offset.port_check_discard;
            param.redirect_to_ingress_port = member_nh->nh_offset.redirect_to_ingress_port;
            HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_remove_mcast_member(&param));
        } 
    }
    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_del_nested_group_member
 * Purpose      : Fea layer group delete nested group member
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_del_nested_group_member(hsrv_openflow_group_t* p_group)
{
#if 0
    uint32 group_id;
    uint32 group_idx;
    uint16 linkagg_port = 0;
    hsrv_ofnh_mcast_member_param_t member_param;
    hsrv_openflow_group_t* p_nested_group = NULL;
    hsrv_openflow_group_nexthop_info_t* p_nh_info = NULL;

    for (group_idx = 0; group_idx < p_group->group.nested_group_count; group_idx ++)
    {
        group_id = p_group->group.nested_group[group_idx];
        HSRV_IF_ERROR_RETURN(hsrv_openflow_groupdb_lookup_group(group_id, &p_nested_group));
        if (!p_nested_group)
        {
            continue;
        }

        p_nh_info = &p_nested_group->u.dft.nh_info;
        linkagg_port = CTC_MAP_TID_TO_GPORT(p_nh_info->linkagg_nh.tid);

        sal_memset(&member_param, 0, sizeof(hsrv_ofnh_mcast_member_param_t));
        member_param.group_nhid = p_group->u.dft.nh_info.main_nh.nh_offset.nhid;
        member_param.op_type = HSRV_OFNH_MCAST_OP_DEL_MEMBER;
        member_param.mem_gport = linkagg_port;
        member_param.mem_nhid = p_nh_info->linkagg_nh.nh_offset.nhid;
        member_param.port_check_discard = hsrv_openflow_group_get_drop_ingress();
        member_param.redirect_to_ingress_port = 0;
        HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_remove_mcast_member(&member_param)); 
    }
#endif
    /*added by hansf for support group chain:all+all,all+ff*/
    uint32 group_id;
    uint32 group_idx;
    hsrv_openflow_group_t* p_nested_group = NULL;
        
    for (group_idx = 0; group_idx < p_group->group.nested_group_count; group_idx ++)
    {
        group_id = p_group->group.nested_group[group_idx];
        HSRV_IF_ERROR_RETURN(hsrv_openflow_groupdb_lookup_group(group_id, &p_nested_group));
        if (!p_nested_group)
        {
            return HSRV_E_ENTRY_NOT_EXIST;
        }
        _hsrv_openflow_del_nested_group_member_one(p_group, p_nested_group);
    }

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_group_set_first_live_bucket
 * Purpose      : Fea layer group set first live bucket
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_set_first_live_bucket(hsrv_openflow_group_t* p_group)
{
    uint32 n_buckets = 0;
    uint32 bucket_idx;
    bool has_link_up = FALSE;
    bool link_status = FALSE;
    uint32 watch_port_ifindex;

    n_buckets = p_group->group.n_buckets;
    for (bucket_idx = 0; bucket_idx < n_buckets; bucket_idx ++)
    {
        watch_port_ifindex = p_group->group.bucket_list[bucket_idx].watch_port;
        link_status = hsrv_openflow_port_get_link_status(watch_port_ifindex);
        if (link_status)
        {
            has_link_up = TRUE;
            p_group->group.choose_bucket_idx = bucket_idx;
            break;
        }
    }

    if (!has_link_up)
    {
        p_group->group.choose_bucket_idx = UINT32_MAX;
    }

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_group_decide_output_data_proto
 * Purpose      : Fea layer group decide output data proto
 * Input        : p_data: hsrv_openflow_map_output_data_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_decide_output_data_proto(hsrv_openflow_map_output_data_t* p_data)
{
    uint16 count;
    glb_openflow_flow_action_t* p_action;
    
    for (count = 0; count < p_data->p_actlist->action_count; count ++)
    {
        p_action = p_data->p_actlist->actlist + count;
        switch (p_action->type)
        {
            /* L3 IPV4. */
            case GLB_OPENFLOW_ACTION_TYPE_SET_IPV4_SRC:        
            case GLB_OPENFLOW_ACTION_TYPE_SET_IPV4_DST:
            case GLB_OPENFLOW_ACTION_TYPE_DEC_IP_TTL:
            case GLB_OPENFLOW_ACTION_TYPE_SET_IP_ECN:        
            case GLB_OPENFLOW_ACTION_TYPE_SET_IP_DSCP:
            case GLB_OPENFLOW_ACTION_TYPE_DEC_TTL:        
            case GLB_OPENFLOW_ACTION_TYPE_SET_IP_TTL:
            case GLB_OPENFLOW_ACTION_TYPE_SET_IP_PROTO:
                p_data->eth_type = ETH_TYPE_IP;
                break;

            /* L3 ARP. */
            case GLB_OPENFLOW_ACTION_TYPE_SET_ARP_OP:
            case GLB_OPENFLOW_ACTION_TYPE_SET_ARP_SPA:
            case GLB_OPENFLOW_ACTION_TYPE_SET_ARP_TPA:
            case GLB_OPENFLOW_ACTION_TYPE_SET_ARP_SHA:
            case GLB_OPENFLOW_ACTION_TYPE_SET_ARP_THA:
                p_data->eth_type = ETH_TYPE_ARP;
                break;

            /* Group should not support these actions since we cannot infer TCP/UDP 
               from it, and SDK does not support tp src/dst regardless of TCP/UDP. */            
            case GLB_OPENFLOW_ACTION_TYPE_SET_TP_SRC:
            case GLB_OPENFLOW_ACTION_TYPE_SET_TP_DST:
                break;

            /* L4 TCP. */
            case GLB_OPENFLOW_ACTION_TYPE_SET_TCP_SRC:
            case GLB_OPENFLOW_ACTION_TYPE_SET_TCP_DST:
                // TODO: only IPv4 is supported by us, so we have to set it to IPv4.
                // TODO: if later we support IPv6, we must find a way to detect wether to support
                // TODO: both IPv4/IPv6 editting.
                p_data->eth_type = ETH_TYPE_IP;
                p_data->ip_proto = IP_PROTO_TCP;
                break;

            /* L4 UDP. */
            case GLB_OPENFLOW_ACTION_TYPE_SET_UDP_SRC:
            case GLB_OPENFLOW_ACTION_TYPE_SET_UDP_DST:
                p_data->eth_type = ETH_TYPE_IP;
                p_data->ip_proto = IP_PROTO_UDP;
                break;

            /* L4 SCTP. */
            case GLB_OPENFLOW_ACTION_TYPE_SET_SCTP_SRC:
            case GLB_OPENFLOW_ACTION_TYPE_SET_SCTP_DST:
                // TODO: add SCTP support.
                break;

            /* L4 ICMP. */     
            case GLB_OPENFLOW_ACTION_TYPE_SET_ICMPV4_TYPE:
            case GLB_OPENFLOW_ACTION_TYPE_SET_ICMPV4_CODE:
                p_data->eth_type = ETH_TYPE_IP;
                p_data->ip_proto = IP_PROTO_ICMP;
                break;

            /* Ignore field that inrrelevant or not suported yet. */            
            default:
                break;
        }     
    }
    
    return HSRV_E_NONE;
}


#define _____INTERNAL_STATS_APIs_____
/*************************************************************************************************
 * Name         : _hsrv_openflow_group_alloc_stats_id
 * Purpose      : Hsrv layer group alloc group stats id
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_alloc_stats_id(hsrv_openflow_group_t* p_group)
{
    HSRV_OPENFLOW_PTR_CHECK(p_group);
    /* allocate group stats */
    if (p_group->group.stats_en)
    {
        p_group->u.dft.stats_id = ctclib_opf_alloc_offset(g_pst_openflow_stats_master->pst_openflow_stats_id_opf , 0, 1);
    }
    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_group_release_stats_id
 * Purpose      : Hsrv layer group release group stats id
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_release_stats_id(hsrv_openflow_group_t* p_group)
{
    HSRV_OPENFLOW_PTR_CHECK(p_group);
    /* free group stats */
    if (p_group->group.stats_en)
    {
        ctclib_opf_free_offset(g_pst_openflow_stats_master->pst_openflow_stats_id_opf , 0,1, p_group->u.dft.stats_id);
        p_group->u.dft.stats_id = 0;
    }
    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_group_clear_group_stats
 * Purpose      : Hsrv layer group clear stats
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_clear_group_stats(hsrv_openflow_group_t* p_group)
{
    hsrv_of_stats_t stats;

    HSRV_OPENFLOW_PTR_CHECK(p_group);

    stats.resource_type = HSRV_OPENFLOW_STATS_RESOURCE_TYPE_GROUP;
    stats.stats_id = p_group->u.dft.stats_id;
    stats.stats.resource_id = p_group->group.group_id;
    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_clear_stats(&stats));

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_group_add_stats
 * Purpose      : Hsrv layer group create stats
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_add_stats(hsrv_openflow_group_t* p_group)
{
    hsrv_of_stats_t stats;

    HSRV_OPENFLOW_PTR_CHECK(p_group);

    /* alloc stat id */
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_alloc_stats_id(p_group));
    
    sal_memset(&stats, 0, sizeof(hsrv_of_stats_t));
    if (p_group->group.stats_en)
    {
        stats.resource_type = HSRV_OPENFLOW_STATS_RESOURCE_TYPE_GROUP;
        if (p_group->u.dft.nh_info.use_ecmp)
        {
            stats.resource_alloc_type.sub_type.group_alloc_type = GLB_OF_GROUP_ALLOC_TYPE_ECMP;
        }
        else if (p_group->u.dft.nh_info.use_mcast)
        {
            stats.resource_alloc_type.sub_type.group_alloc_type = GLB_OF_GROUP_ALLOC_TYPE_MCAST;
        }
        stats.stats_id = p_group->u.dft.stats_id;
        stats.stats.resource_id = p_group->group.group_id;
        HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_create_stats(&stats));
    }

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_group_del_stats
 * Purpose      : Hsrv layer group delete stats
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_del_stats(hsrv_openflow_group_t* p_group)
{
    hsrv_of_stats_t stats;

    HSRV_OPENFLOW_PTR_CHECK(p_group);

    if (p_group->group.stats_en)
    {
        stats.resource_type = HSRV_OPENFLOW_STATS_RESOURCE_TYPE_GROUP;
        stats.stats_id = p_group->u.dft.stats_id;
        stats.stats.resource_id = p_group->group.group_id;
        HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_del_stats(&stats));
    }

    /* release stats id */
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_release_stats_id(p_group));

    return HSRV_E_NONE;
}




#define _____INTERNAL_MCAST_APIs_____
/*************************************************************************************************
 * Name         : _hsrv_openflow_group_add_mcast_group
 * Purpose      : Hsrv layer group alloc mcast group nhid
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_add_mcast_group(hsrv_openflow_group_t* p_group)
{
    hsrv_of_nh_offset_t group_nh;
    hsrv_ofnh_mcast_param_t mcast_param;
    hsrv_openflow_group_nexthop_info_t* p_nh_info = NULL;

    HSRV_OPENFLOW_PTR_CHECK(p_group);

    p_nh_info = &p_group->u.dft.nh_info;
    p_nh_info->use_mcast = TRUE;

    /* create stats */
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_add_stats(p_group));

    sal_memset(&group_nh, 0, sizeof(hsrv_of_nh_offset_t));
    sal_memset(&mcast_param, 0, sizeof(hsrv_ofnh_mcast_param_t));

    group_nh.stats_id = p_group->u.dft.stats_id;
    mcast_param.stats_en = p_group->group.stats_en;
    mcast_param.no_fwd = FALSE;
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_alloc_mcast_group(&mcast_param, &group_nh));

    p_nh_info->main_nh.nh_offset.nhid    = group_nh.nhid;
    p_nh_info->main_nh.nh_offset.offset  = group_nh.offset;
    p_nh_info->main_nh.nh_offset.nh_type = group_nh.nh_type;

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_group_add_mcast_group
 * Purpose      : Hsrv layer group dealloc mcast group nhid
 * Input        : p_group_nh: hsrv_of_nh_offset_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_del_mcast_group(hsrv_of_nh_offset_t* p_group_nh)
{
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_release_mcast_group(p_group_nh));

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_group_add_buckets_to_group
 * Purpose      : Fea layer group add buckets to mcast group
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_add_buckets_to_mcast_group(hsrv_openflow_group_t* p_group)
{
    uint32 bucket_idx;
    uint32 member_idx;
    uint32 n_buckets = 0;
    hsrv_openflow_group_nexthop_info_t* p_nh_info = NULL;

    p_nh_info = &p_group->u.dft.nh_info;
    n_buckets = p_group->group.n_buckets;

    for (bucket_idx = 0; bucket_idx < n_buckets; bucket_idx ++)
    {
        if (0 == p_group->group.bucket_list[bucket_idx].n_actions)
        {
            continue;
        }

        if (GLB_OF_GROUP_TYPE_FF == p_group->group.group_type)
        {
            /* check ff group first member */
            if (p_group->group.choose_bucket_idx != UINT32_MAX &&
                p_group->group.choose_bucket_idx != bucket_idx)
            {
                continue;
            }

            /* check ff group all bucket invalid */
            if (p_group->group.choose_bucket_idx == UINT32_MAX)
            {
                continue;
            }
        }
        
        for (member_idx = 0; member_idx < p_nh_info->buckets[bucket_idx].member_count; member_idx ++)
        {
            hsrv_ofnh_mcast_member_param_t member_param;
            hsrv_of_nh_info_t* p_member = &p_nh_info->buckets[bucket_idx].member_nh[member_idx];

            sal_memset(&member_param, 0, sizeof(hsrv_ofnh_mcast_member_param_t));
            member_param.group_nhid = p_group->u.dft.nh_info.main_nh.nh_offset.nhid;
            member_param.op_type = HSRV_OFNH_MCAST_OP_ADD_MEMBER;
            member_param.mem_gport = p_member->gport;
            member_param.mem_nhid = p_member->nh_offset.nhid;
            member_param.port_check_discard = p_member->nh_offset.port_check_discard;
            member_param.redirect_to_ingress_port = p_member->nh_offset.redirect_to_ingress_port;
            HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_add_mcast_member(&member_param));
        }
    }

    /* add nested group */
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_add_nested_group_member(p_group));

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_group_del_bucket_from_mcast_buckets
 * Purpose      : Fea layer group delele buckets to mcast group
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_del_bucket_from_mcast_buckets(hsrv_openflow_group_t* p_group)
{
    uint32 bucket_idx;
    uint32 n_buckets;
    uint32 member_idx;
    hsrv_openflow_group_nexthop_info_t* p_nh_info = NULL;;

    p_nh_info = &p_group->u.dft.nh_info;
    n_buckets = p_group->group.n_buckets;
    if (n_buckets == 0)
    {
        return HSRV_E_NONE;
    }

    for (bucket_idx = 0; bucket_idx < p_nh_info->bucket_count; bucket_idx++)
    {
        if (GLB_OF_GROUP_TYPE_FF == p_group->group.group_type)
        {
            /* check ff group first member */
            if (p_group->group.choose_bucket_idx != UINT32_MAX &&
                p_group->group.choose_bucket_idx != bucket_idx)
            {
                continue;
            }

            /* check ff group all bucket invalid */
            if (p_group->group.choose_bucket_idx == UINT32_MAX)
            {
                continue;
            }
        }
 
        for (member_idx = 0; member_idx < p_nh_info->buckets[bucket_idx].member_count; member_idx++)
        {
            hsrv_of_nh_info_t *member_nh;
            hsrv_ofnh_mcast_member_param_t param;

            member_nh = &p_nh_info->buckets[bucket_idx].member_nh[member_idx];
            param.group_nhid = p_group->u.dft.nh_info.main_nh.nh_offset.nhid;
            param.op_type = HSRV_OFNH_MCAST_OP_DEL_MEMBER;
            param.mem_gport = member_nh->gport;
            param.mem_nhid = member_nh->nh_offset.nhid;
            param.port_check_discard = member_nh->nh_offset.port_check_discard;
            param.redirect_to_ingress_port = member_nh->nh_offset.redirect_to_ingress_port;
            HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_remove_mcast_member(&param));
        } 
    }

    /* delete nested group */
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_del_nested_group_member(p_group));

    return HSRV_E_NONE;
}

#define _____INTERNAL_ECMP_APIS_____
/*************************************************************************************************
 * Name         : _hsrv_openflow_group_add_ecmp_group
 * Purpose      : Fea layer group alloc ecmp group nhid
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_add_ecmp_group(hsrv_openflow_group_t* p_group)
{
    hsrv_of_nh_offset_t group_nh;
    hsrv_ofnh_mcast_param_t mcast_param;
    hsrv_openflow_group_nexthop_info_t* p_nh_info = NULL;

    HSRV_OPENFLOW_PTR_CHECK(p_group);

    p_nh_info = &p_group->u.dft.nh_info;
    p_nh_info->use_ecmp = TRUE;

    /* create stats */
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_add_stats(p_group));

    sal_memset(&group_nh, 0, sizeof(hsrv_of_nh_offset_t));
    sal_memset(&mcast_param, 0, sizeof(hsrv_ofnh_mcast_param_t));

    group_nh.stats_id = p_group->u.dft.stats_id;
    mcast_param.stats_en = p_group->group.stats_en;
    mcast_param.no_fwd = FALSE;
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_alloc_ecmp_group(&mcast_param, &group_nh));

    p_nh_info->main_nh.nh_offset.nhid = group_nh.nhid;
    p_nh_info->main_nh.nh_offset.offset = group_nh.offset;
    p_nh_info->main_nh.nh_offset.nh_type = group_nh.nh_type;

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_group_del_ecmp_group
 * Purpose      : Hsrv layer group dealloc ecmp group nhid
 * Input        : p_group_nh: hsrv_of_nh_offset_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_del_ecmp_group(hsrv_of_nh_offset_t* p_group_nh)
{
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_release_ecmp_group(p_group_nh));

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_group_add_buckets_to_ecmp_group
 * Purpose      : Fea layer group add buckets to ecmp group
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_add_buckets_to_ecmp_group(hsrv_openflow_group_t* p_group)
{
    uint32 ifindex;
    uint32 bucket_idx;
    uint32 member_idx = 0;
    uint32 n_buckets = 0;
    uint32 nhid_array[GLB_BUCKET_NUM_PER_SELECT_GROUP] = {0};
    hsrv_openflow_group_t* group_in_db = NULL;
    bool first_add_ecmp = FALSE;
    hsrv_openflow_group_nexthop_info_t* p_nh_info = NULL;

    HSRV_OPENFLOW_PTR_CHECK(p_group);

    p_nh_info = &p_group->u.dft.nh_info;
    n_buckets = p_group->group.n_buckets;

    for (bucket_idx = 0; bucket_idx < n_buckets; bucket_idx ++)
    {
        if (0 == p_group->group.bucket_list[bucket_idx].n_actions)
        {
            continue;
        }

        if (p_nh_info->buckets[bucket_idx].member_nh[0].nh_offset.nhid)
        {
            /* Added by weizj for bug 36895 */
            ifindex = p_nh_info->buckets[bucket_idx].member_nh[0].ifindex;
            if (!hsrv_openflow_port_get_link_status(ifindex))
            {
                continue;
            }

            nhid_array[member_idx] = p_nh_info->buckets[bucket_idx].member_nh[0].nh_offset.nhid;
            member_idx ++;
        }
    }

    if (member_idx)
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_groupdb_lookup_group(p_group->group.group_id, &group_in_db));
        if (NULL == group_in_db)
        {
            first_add_ecmp = TRUE;
        }

        HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_add_ecmp_member(
                             p_nh_info->main_nh.nh_offset.nhid,
                             p_group->u.dft.stats_id,
                             member_idx,
                             nhid_array, first_add_ecmp));
    }

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_group_del_buckets_from_ecmp_group
 * Purpose      : Fea layer group delete buckets to ecmp group
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_del_buckets_from_ecmp_group(hsrv_openflow_group_t* p_group)
{
    uint32 ifindex;
    uint32 bucket_idx;
    uint32 n_buckets;
    uint32 member_idx = 0;
    uint32 nhid_array[GLB_BUCKET_NUM_PER_SELECT_GROUP] = {0};
    hsrv_openflow_group_nexthop_info_t* p_nh_info = NULL;

    HSRV_OPENFLOW_PTR_CHECK(p_group);

    p_nh_info = &p_group->u.dft.nh_info;
    n_buckets = p_group->group.n_buckets;

    for (bucket_idx = 0; bucket_idx < n_buckets; bucket_idx ++)
    {
        if (0 == p_group->group.bucket_list[bucket_idx].n_actions)
        {
            continue;
        }

        /* Added by weizj for bug 36895 */
        ifindex = p_nh_info->buckets[bucket_idx].member_nh[0].ifindex;
        if (!hsrv_openflow_port_get_link_status(ifindex))
        {
            continue;
        }

        nhid_array[member_idx] = p_nh_info->buckets[bucket_idx].member_nh[0].nh_offset.nhid;
        member_idx ++;
    }

    if (member_idx)
    {
        hsrv_openflow_nexthop_remove_ecmp_member(p_nh_info->main_nh.nh_offset.nhid,
                                                 member_idx, nhid_array);
    }

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_group_mod_buckets_to_ecmp_group
 * Purpose      : Fea layer group mod buckets to ecmp group
 * Input        : p_old_group: hsrv_openflow_group_t*, p_new_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_mod_buckets_to_ecmp_group(hsrv_openflow_group_t* p_old_group,
    hsrv_openflow_group_t* p_new_group)
{
    uint32 bucket_idx = 0;
    uint32 member_idx = 0;
    uint32 member_idx2 = 0;
    uint32 del_nh_count = 0;
    uint32 old_nh_count = 0;
    uint32 new_nh_count = 0;
    uint32 nhid_del_array[GLB_MAX_MEMBER_PER_MULTICAST_GROUP] = {0};
    uint32 nhid_old_array[GLB_MAX_MEMBER_PER_MULTICAST_GROUP] = {0};
    uint32 nhid_new_array[GLB_MAX_MEMBER_PER_MULTICAST_GROUP] = {0};
    uint32 nhid_add_array[GLB_MAX_MEMBER_PER_MULTICAST_GROUP] = {0};
    hsrv_openflow_group_nexthop_info_t* p_nh_info = NULL;
    uint32 group_nhid;
    uint32 stats_id;
    bool flag = FALSE;
    uint32 count = 0;
    uint32 ifindex = 0;
    uint32 ret;

    HSRV_OPENFLOW_PTR_CHECK( p_old_group);
    HSRV_OPENFLOW_PTR_CHECK( p_new_group);

    group_nhid = p_old_group->u.dft.nh_info.main_nh.nh_offset.nhid;
    stats_id = p_old_group->u.dft.stats_id;
    sal_memcpy(&p_new_group->u, &p_old_group->u, sizeof(hsrv_openflow_group_info_t));
    

    /*get del_nh_id in old group*/
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_get_del_nexthop(p_new_group, p_old_group, nhid_del_array, &del_nh_count));

    /*remove del member from mcast and del nexthop*/
    p_nh_info = &p_old_group->u.dft.nh_info;
    for (bucket_idx = 0; bucket_idx < p_nh_info->bucket_count; bucket_idx ++)
    {
        if (0 == p_old_group->group.bucket_list[bucket_idx].n_actions)
        {
            continue;
        }
        hsrv_of_nh_info_t *member_nh;
        member_nh = &p_nh_info->buckets[bucket_idx].member_nh[0];
        ifindex = p_nh_info->buckets[bucket_idx].member_nh[0].ifindex;
        for (member_idx = 0; member_idx < del_nh_count; member_idx++)
        {
            if (member_nh->nh_offset.nhid == nhid_del_array[member_idx] && nhid_del_array[member_idx])
            {
                if (!hsrv_openflow_port_get_link_status(ifindex))
                {
                    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_release_nh_info(member_nh));
                }
                else
                {
                    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_remove_ecmp_member(group_nhid,
                             1, &nhid_del_array[member_idx]));
                    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_release_nh_info(member_nh));
                }
                nhid_del_array[member_idx] = 0;
            }
        }
        if (member_nh->nh_offset.nhid)
        {
            nhid_old_array[old_nh_count++] = member_nh->nh_offset.nhid;
        }
    }
    
    /*alloc new group nexthop*/
    ret = _hsrv_openflow_group_allocate_buckets(p_new_group);
    if (ret)
    {
        _hsrv_openflow_group_release_buckets(p_new_group);
        return HSRV_E_INVALID_PARAM;
    }

    /*get new group nexthop id*/
    p_nh_info = &p_new_group->u.dft.nh_info;
    for (bucket_idx = 0; bucket_idx < p_nh_info->bucket_count; bucket_idx++)
    {
        if (0 == p_new_group->group.bucket_list[bucket_idx].n_actions)
        {
            continue;
        }
        ifindex = p_nh_info->buckets[bucket_idx].member_nh[0].ifindex;
        if (!hsrv_openflow_port_get_link_status(ifindex))
        {
            continue;
        }
        nhid_new_array[new_nh_count++] = p_nh_info->buckets[bucket_idx].member_nh[0].nh_offset.nhid;
    }

    /*get new add nexthop id*/
    count = 0;
    for (member_idx = 0; member_idx < new_nh_count; member_idx ++)
    {
        flag = FALSE;
        for (member_idx2 = 0; member_idx2 < old_nh_count; member_idx2 ++)
        {
            if (nhid_old_array[member_idx2] == nhid_new_array[member_idx])
            {
                nhid_old_array[member_idx2] = 0;
                flag = TRUE;
                break;
            }
        }
        if (!flag)
        {
            nhid_add_array[count++] = nhid_new_array[member_idx];
        }
    }
    
    /*add new group nexthop id to mcast member*/
    if (count)
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_add_ecmp_member(group_nhid, stats_id,
                             count, nhid_add_array, FALSE));
    }
    
    /*delete old group rested nexthop and remove member*/
    p_nh_info = &p_old_group->u.dft.nh_info;
    for (bucket_idx = 0; bucket_idx < p_nh_info->bucket_count; bucket_idx++)
    {
        if (0 == p_old_group->group.bucket_list[bucket_idx].n_actions)
        {
            continue;
        }
        hsrv_of_nh_info_t *member_nh;
        member_nh = &p_nh_info->buckets[bucket_idx].member_nh[0];
        ifindex = p_nh_info->buckets[bucket_idx].member_nh[0].ifindex;
        if (member_nh->nh_offset.nhid)
        {
            HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_release_nh_info(member_nh));
        }
    }

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_group_mod_buckets_ecmp_to_linkagg_group
 * Purpose      : Fea layer group mod buckets to ecmp group
 * Input        : p_old_group: hsrv_openflow_group_t*, p_new_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_mod_buckets_ecmp_to_linkagg_group(hsrv_openflow_group_t* p_old_group,
    hsrv_openflow_group_t* p_new_group)
{
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_mod_buckets_to_ecmp_group(p_old_group, p_new_group));
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_add_linkagg_group(p_new_group));
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_add_buckets_to_linkagg_group(p_new_group));

    return HSRV_E_NONE;
}

#define _____INTERNAL_LINKAGG_APIS_____
/*************************************************************************************************
 * Name         : _hsrv_openflow_alloc_linkagg_nhid
 * Purpose      : Hsrv layer group alloc linkagg group nhid
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_alloc_linkagg_nhid(hsrv_openflow_group_nexthop_info_t* p_nh_info)
{
    hsrv_openflow_action_combo_t action_combo;
    hsrv_ofnh_flex_param_t param;

    sal_memset(&action_combo, 0, sizeof(action_combo));
    param.p_action_combo = &action_combo;
    GLB_OFPACT_FLD_SET(action_combo.flag, OUTPUT);

    action_combo.output_gport = CTC_MAP_TID_TO_GPORT(p_nh_info->linkagg_nh.tid);
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_alloc_flex_nh(&param, &p_nh_info->linkagg_nh));

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_release_linkagg_nhid
 * Purpose      : Hsrv layer group release linkagg group nhid
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_release_linkagg_nhid(hsrv_openflow_group_nexthop_info_t* p_nh_info)
{
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_release_nh_info(&p_nh_info->linkagg_nh));

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_group_add_linkagg_group
 * Purpose      : Hsrv layer group create linkagg
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_add_linkagg_group(hsrv_openflow_group_t* p_group)
{
    hsrv_openflow_group_nexthop_info_t* p_nh_info = NULL;

    HSRV_OPENFLOW_PTR_CHECK(p_group);

    if (!p_group->group.linkagg_en)
    {
        HSRV_OPENFLOW_LOG_ERR("Add group: %d linkagg resource error", p_group->group.group_id);
        return HSRV_E_INVALID_PARAM;
    }

    /* linkagg enable */
    p_nh_info = &p_group->u.dft.nh_info;
    p_nh_info->use_linkagg = TRUE;

    /* alloc linkagg tid */
    HSRV_IF_ERROR_RETURN(hsrv_openflow_linkagg_offset_alloc(&p_nh_info->linkagg_nh.tid));
    /* create linkagg */
    HSRV_IF_ERROR_RETURN(hsrv_openflow_linkagg_create(p_nh_info->linkagg_nh.tid));
    /* alloc linkagg nhid */
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_alloc_linkagg_nhid(p_nh_info));

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_group_del_linkagg_group
 * Purpose      : Hsrv layer group delete linkagg
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_del_linkagg_group(hsrv_openflow_group_t* p_group)
{
    hsrv_openflow_group_nexthop_info_t* p_nh_info = NULL;

    HSRV_OPENFLOW_PTR_CHECK(p_group);

    if (!p_group->group.linkagg_en)
    {
        HSRV_OPENFLOW_LOG_ERR("Delete group: %d linkagg resource error", p_group->group.group_id);
        return HSRV_E_INVALID_PARAM;
    }

    /* linkagg disable */
    p_nh_info = &p_group->u.dft.nh_info;

    /* delete linkagg */
    HSRV_IF_ERROR_RETURN(hsrv_openflow_linkagg_delete(p_nh_info->linkagg_nh.tid));
    /* release linkagg tid */
    HSRV_IF_ERROR_RETURN(hsrv_openflow_linkagg_offset_dealloc(p_nh_info->linkagg_nh.tid));
    /* delete linkagg nhid */
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_release_linkagg_nhid(p_nh_info));

    p_nh_info->linkagg_nh.tid = 0;
    p_nh_info->use_linkagg = FALSE;

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_group_add_buckets_to_linkagg_group
 * Purpose      : Hsrv layer group add member to link agg
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_add_buckets_to_linkagg_group(hsrv_openflow_group_t* p_group)
{
    uint32 bucket_idx;
    uint32 n_buckets = 0;
    uint32 n_actions = 0;
    hsrv_openflow_group_nexthop_info_t* p_nh_info = NULL;

    HSRV_OPENFLOW_PTR_CHECK(p_group);

    if (!p_group->group.linkagg_en)
    {
        HSRV_OPENFLOW_LOG_ERR("Add group: %d buckets to linkagg error", p_group->group.group_id);
        return HSRV_E_INVALID_PARAM;
    }

    p_nh_info = &p_group->u.dft.nh_info;
    n_buckets = p_group->group.n_buckets;

    for (bucket_idx = 0; bucket_idx < n_buckets; bucket_idx ++)
    {
        n_actions = p_group->group.bucket_list[bucket_idx].n_actions;
        if (0 == n_actions)
        {
            continue;
        }

        /* select group bucket has one output only */
        if (p_nh_info->buckets[bucket_idx].member_count > 0)
        {
            hsrv_of_nh_info_t* p_member;

            p_member = p_nh_info->buckets[bucket_idx].member_nh;
            /* Added by weizj for bug 36895 */
            if (!hsrv_openflow_port_get_link_status(p_member->ifindex))
            {
                continue;
            }
            HSRV_IF_ERROR_RETURN(hsrv_openflow_linkagg_add_port(p_nh_info->linkagg_nh.tid, p_member->gport));
        }
    }
    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_group_del_buckets_from_linkagg_group
 * Purpose      : Hsrv layer group del member from linkagg group
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_del_buckets_from_linkagg_group(hsrv_openflow_group_t* p_group)
{
    uint32 bucket_idx;
    uint32 n_buckets = 0;
    uint32 n_actions = 0;
    hsrv_openflow_group_nexthop_info_t* p_nh_info = NULL;

    if (!p_group->group.linkagg_en)
    {
        HSRV_OPENFLOW_LOG_ERR("Delete group: %d linkagg member error", p_group->group.group_id);
        return HSRV_E_INVALID_PARAM;
    }
    
    p_nh_info = &p_group->u.dft.nh_info;
    n_buckets = p_group->group.n_buckets;

    for (bucket_idx = 0; bucket_idx < n_buckets; bucket_idx ++)
    {
        n_actions = p_group->group.bucket_list[bucket_idx].n_actions;
        if (0 == n_actions)
        {
            continue;
        }

        /* select group bucket has one output only */
        if (p_nh_info->buckets[bucket_idx].member_count > 0)
        {
            hsrv_of_nh_info_t* p_member;

            p_member = p_nh_info->buckets[bucket_idx].member_nh;
            /* Added by weizj for bug 36895 */
            if (!hsrv_openflow_port_get_link_status(p_member->ifindex))
            {
                continue;
            }
            HSRV_IF_ERROR_RETURN(hsrv_openflow_linkagg_del_port(p_nh_info->linkagg_nh.tid, p_member->gport));
        }
    }
    
    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_group_mod_buckets_to_linkagg_group
 * Purpose      : Hsrv layer group mod member to link agg
 * Input        : p_old_group: hsrv_openflow_group_t*, p_new_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_mod_buckets_to_linkagg_group(hsrv_openflow_group_t* p_old_group,
    hsrv_openflow_group_t* p_new_group)
{
    uint32 ret;
    uint32 bucket_idx;
    uint32 loop;
    uint32 tid;
    uint16 bucket_gport;
    uint32 n_buckets = 0;
    uint32 n_actions = 0;
    uint32 old_gport_pool[GLB_BUCKET_NUM_PER_SELECT_GROUP] = {0};
    uint32 new_gport_pool[GLB_BUCKET_NUM_PER_SELECT_GROUP] = {0};
    hsrv_openflow_group_nexthop_info_t* p_nh_info = NULL;

    HSRV_OPENFLOW_PTR_CHECK(p_old_group);
    HSRV_OPENFLOW_PTR_CHECK(p_new_group);

#if 1 /* fix bug add by wangqj at 2017/2/15: GPORT value of first port maybe equal 0 */
    sal_memset(old_gport_pool, (uint32)-1, GLB_BUCKET_NUM_PER_SELECT_GROUP); // 0xFFFFFFFF
    sal_memset(new_gport_pool, (uint32)-1, GLB_BUCKET_NUM_PER_SELECT_GROUP);
#endif

    tid = p_old_group->u.dft.nh_info.linkagg_nh.tid;

    sal_memcpy(&p_new_group->u, &p_old_group->u, sizeof(hsrv_openflow_group_info_t));
    ret = _hsrv_openflow_group_allocate_buckets(p_new_group);
    if (ret)
    {
        _hsrv_openflow_group_release_buckets(p_new_group);
        return HSRV_E_INVALID_PARAM;
    }

    /* handle old group */
    p_nh_info = &p_old_group->u.dft.nh_info;
    n_buckets = p_old_group->group.n_buckets;
    for (bucket_idx = 0; bucket_idx < n_buckets; bucket_idx ++)
    {
        n_actions = p_old_group->group.bucket_list[bucket_idx].n_actions;
        if (0 == n_actions)
        {
            continue;
        }

        /* select group bucket has one output only */
        if (p_nh_info->buckets[bucket_idx].member_count > 0)
        {
            hsrv_of_nh_info_t* p_member;

            p_member = p_nh_info->buckets[bucket_idx].member_nh;
            /* Added by weizj for bug 36895 */
            if (!hsrv_openflow_port_get_link_status(p_member->ifindex))
            {
                continue;
            }
            old_gport_pool[bucket_idx] = p_member->gport;
        }
    }

    /* handle new group */
    p_nh_info = &p_new_group->u.dft.nh_info;
    n_buckets = p_new_group->group.n_buckets;
    for (bucket_idx = 0; bucket_idx < n_buckets; bucket_idx ++)
    {
        n_actions = p_new_group->group.bucket_list[bucket_idx].n_actions;
        if (0 == n_actions)
        {
            continue;
        }

        /* select group bucket has one output only */
        if (p_nh_info->buckets[bucket_idx].member_count > 0)
        {
            hsrv_of_nh_info_t* p_member;

            p_member = p_nh_info->buckets[bucket_idx].member_nh;
            /* Added by weizj for bug 36895 */
            if (!hsrv_openflow_port_get_link_status(p_member->ifindex))
            {
                continue;
            }
            new_gport_pool[bucket_idx] = p_member->gport;
        }
    }

    /* add new member */
    for (bucket_idx = 0; bucket_idx < p_new_group->group.n_buckets; bucket_idx ++)
    {
        bool flag = TRUE;
        bucket_gport = new_gport_pool[bucket_idx];
        if ((uint32)-1 == bucket_gport)
        {
            continue;
        }

        for (loop = 0; loop < p_old_group->group.n_buckets; loop ++)
        {
            if ((uint32)-1 == old_gport_pool[loop])
            {
                continue;
            }

            if (bucket_gport == old_gport_pool[loop])
            {
                flag = FALSE;
            }
        }

        if (flag)
        {
            HSRV_IF_ERROR_RETURN(hsrv_openflow_linkagg_add_port(tid, bucket_gport));
        }
    }

    /* delete old member */
    for (bucket_idx = 0; bucket_idx < p_old_group->group.n_buckets; bucket_idx ++)
    {
        bool flag = TRUE;
        bucket_gport = old_gport_pool[bucket_idx];
        for (loop = 0; loop < p_new_group->group.n_buckets; loop ++)
        {
            if (bucket_gport == new_gport_pool[loop])
            {
                flag = FALSE;
            }
        }

        if (flag)
        {
            HSRV_IF_ERROR_RETURN(hsrv_openflow_linkagg_del_port(tid, bucket_gport));
        }
    }
    
    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_group_mod_buckets_linkagg_to_ecmp_group
 * Purpose      : Hsrv layer group mod member to link agg
 * Input        : p_old_group: hsrv_openflow_group_t*, p_new_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_mod_buckets_linkagg_to_ecmp_group(hsrv_openflow_group_t* p_old_group,
    hsrv_openflow_group_t* p_new_group)
{
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_del_buckets_from_linkagg_group(p_old_group));
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_del_linkagg_group(p_old_group));
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_mod_buckets_to_ecmp_group(p_old_group, p_new_group));
    
    return HSRV_E_NONE;
}

#define _____INTERNAL_APIs_____
/*************************************************************************************************
 * Name         : _hsrv_openflow_group_allocate_buckets
 * Purpose      : Fea layer group alloc group buckets
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_allocate_buckets(hsrv_openflow_group_t* p_group)
{
    uint32 buckets_size;
    uint32 bucket_idx;
    uint32 n_actions;
    hsrv_openflow_map_output_data_t output_data;
    hsrv_openflow_group_nexthop_info_t* p_nh_info = NULL;

    HSRV_OPENFLOW_PTR_CHECK(p_group);

    p_nh_info = &p_group->u.dft.nh_info;
    p_nh_info->bucket_count = p_group->group.n_buckets;

    buckets_size = p_nh_info->bucket_count * sizeof(hsrv_of_bucket_info_t);
    p_nh_info->buckets = HSRV_OPENFLOW_GROUP_MALLOC(buckets_size);
    if (NULL == p_nh_info->buckets)
    {
        return HSRV_E_NO_MEMORY;
    }
    sal_memset(p_nh_info->buckets, 0, buckets_size);

    for (bucket_idx = 0; bucket_idx < p_group->group.n_buckets; bucket_idx ++)
    {
        glb_openflow_bucket_t* p_bucket = &p_group->group.bucket_list[bucket_idx];
        n_actions = p_bucket->n_actions;
        if (0 == n_actions)
        {
            continue;
        }


        if ((p_bucket->mpls_output_action_seq_type != GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_VALID_MAX) &&
            (p_bucket->mpls_output_action_seq_type != GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_POP_ALL_MPLS_0))
        {
            hsrv_openflow_mpls_map_group_output_action(p_bucket,
                                  p_nh_info->buckets[bucket_idx].member_nh,
                                 &p_nh_info->buckets[bucket_idx].member_count);
        }
        else
        {
            sal_memset(&output_data, 0, sizeof(hsrv_openflow_map_output_data_t));
            output_data.p_actlist = HSRV_OPENFLOW_GROUP_MALLOC(sizeof(glb_openflow_flow_action_list_t));
            output_data.p_actlist->action_count = n_actions;
            output_data.p_actlist->actlist = p_bucket->actlist;
            output_data.is_group = 1; /* Added by weizj for bug 35135 */

            _hsrv_openflow_group_decide_output_data_proto(&output_data);
            hsrv_openflow_map_output_action_to_mcast_members(&output_data,
                                  p_nh_info->buckets[bucket_idx].member_nh,
                                 &p_nh_info->buckets[bucket_idx].member_count);
            
            HSRV_OPENFLOW_GROUP_FREE(output_data.p_actlist);
        }
    }

    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_map_group_action(glb_openflow_flow_action_t *p_action, _hsrv_openflow_action_combo_t *p_action_combo)
{

    switch(p_action->type)
    {
        case GLB_OPENFLOW_ACTION_TYPE_WRITE_METADATA:
            p_action_combo->metadata = p_action->value.m;
            break;
        case GLB_OPENFLOW_ACTION_TYPE_SET_MPLS_LABEL:
            p_action_combo->mpls_label = p_action->value.u32;
            break;
        case GLB_OPENFLOW_ACTION_TYPE_SET_MPLS_TC:
            p_action_combo->mpls_tc = p_action->value.u8;
            break;
        case GLB_OPENFLOW_ACTION_TYPE_SET_MPLS_TTL:
            p_action_combo->mpls_ttl = p_action->value.u8;
            break;
        default:
            HSRV_IF_ERROR_RETURN(hsrv_openflow_map_combo_action(p_action, &(p_action_combo->action_combo)));
            break;
    }
    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_group_get_del_nexthop(hsrv_openflow_group_t* p_new_group, hsrv_openflow_group_t* p_old_group, uint32 *nhid_del_array, uint32 *nhid_del_count)
{
    uint32 bucket_idx;
    uint32 member_idx;
    uint32 count = 0;
    uint32 nhid_count = 0;
    uint32 count_new = 0;
    uint32 count_old = 0;
    uint32 bucket_nh_num = 0;
    uint32 old_index = 0;
    uint32 new_index = 0;
    _hsrv_openflow_action_combo_t action_combo_old[GLB_MAX_MEMBER_PER_MULTICAST_GROUP];
    _hsrv_openflow_action_combo_t action_combo_new[GLB_MAX_MEMBER_PER_MULTICAST_GROUP];
    _hsrv_openflow_action_combo_t combo_action;
    glb_openflow_flow_action_t  *p_action = NULL;
    uint32 nhid_old[GLB_MAX_MEMBER_PER_MULTICAST_GROUP];
    bool is_truncate = FALSE;
    bool flag = FALSE;
    bool output_to_controller = FALSE;
    hsrv_openflow_tunnel_info_t* p_tnl_info = NULL;

    HSRV_OPENFLOW_PTR_CHECK(p_new_group);
    HSRV_OPENFLOW_PTR_CHECK(p_old_group);

    sal_memset(action_combo_new, 0, sizeof(action_combo_new));
    sal_memset(action_combo_old, 0, sizeof(action_combo_old));


    /*get old group nhid_id and corresponding action combo */
    for (bucket_idx = 0; bucket_idx < p_old_group->group.n_buckets; bucket_idx ++)
    {
        glb_openflow_bucket_t* p_bucket = &p_old_group->group.bucket_list[bucket_idx];
        sal_memset(&combo_action, 0, sizeof(_hsrv_openflow_action_combo_t));
        count = 0;
        bucket_nh_num = p_old_group->u.dft.nh_info.buckets[bucket_idx].member_count;
        combo_action.mpls_output_action_seq_type = p_bucket->mpls_output_action_seq_type;
        for (member_idx = 0; member_idx < p_bucket->n_actions; member_idx ++)
        {
            p_action =p_bucket->actlist + member_idx;
            
            HSRV_IF_ERROR_RETURN(_hsrv_openflow_map_group_action(p_action, &combo_action));
            if (GLB_OPENFLOW_ACTION_TYPE_SET_TRUNCATE == p_action->type)
            {
                is_truncate = TRUE;
            }
            
            if (GLB_OPENFLOW_ACTION_TYPE_OUTPUT != p_action->type)
            {
                continue;
            }

            if (is_truncate)
            {
                is_truncate = FALSE;
                continue;
            }
            if (GLB_OFPP_CONTROLLER == p_action->value.output.ofport)
            {
                output_to_controller = TRUE;
            }
            if (bucket_nh_num <= count)
            {
                break;
            }
            if (output_to_controller)
            {
                nhid_old[count_old] = p_old_group->u.dft.nh_info.buckets[bucket_idx].member_nh[bucket_nh_num-1].nh_offset.nhid;
            }
            else
            {
                nhid_old[count_old] = p_old_group->u.dft.nh_info.buckets[bucket_idx].member_nh[count].nh_offset.nhid;
            }

            /* BUG53998 mod_group with tunnel output bucket, system crash */
            if (GLB_OF_PORT_TYPE_TUNNEL == p_action->value.output.type)
            {
                p_tnl_info = NULL;
                _hsrv_openflow_tunnel_lookup_info(p_action->value.output.ifindex, &p_tnl_info);
                if (!p_tnl_info)
                {
                    HSRV_OPENFLOW_LOG_ERR("Not find tunnel info:%d", p_action->value.output.ifindex);
                    return HSRV_E_ENTRY_NOT_EXIST;
                }

                HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(p_tnl_info->tunnel_info.bond_port_ifindex, &combo_action.action_combo.output_gport));
            }
            else
            {
                HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(p_action->value.output.ifindex, &combo_action.action_combo.output_gport));
            }
            
            sal_memcpy(&action_combo_old[count_old], &combo_action, sizeof(_hsrv_openflow_action_combo_t));
            count ++;
            count_old ++;
           
        }
    }

    /*get new group action combo */
    for (bucket_idx = 0; bucket_idx < p_new_group->group.n_buckets; bucket_idx ++)
    {
        glb_openflow_bucket_t* p_bucket = &p_new_group->group.bucket_list[bucket_idx];
        sal_memset(&combo_action, 0, sizeof(_hsrv_openflow_action_combo_t));
        combo_action.mpls_output_action_seq_type = p_bucket->mpls_output_action_seq_type;
        for (member_idx = 0; member_idx < p_bucket->n_actions; member_idx ++)
        {
            p_action =p_bucket->actlist + member_idx;
            
            HSRV_IF_ERROR_RETURN(_hsrv_openflow_map_group_action(p_action, &combo_action));
            if (GLB_OPENFLOW_ACTION_TYPE_SET_TRUNCATE == p_action->type)
            {
                is_truncate = TRUE;
            }
            
            if (GLB_OPENFLOW_ACTION_TYPE_OUTPUT != p_action->type)
            {
                continue;
            }

            if (is_truncate)
            {
                is_truncate = FALSE;
                continue;
            }

            /* BUG53998 mod_group with tunnel output bucket, system crash */
            if (GLB_OF_PORT_TYPE_TUNNEL == p_action->value.output.type)
            {
                p_tnl_info = NULL;
                _hsrv_openflow_tunnel_lookup_info(p_action->value.output.ifindex, &p_tnl_info);
                if (!p_tnl_info)
                {
                    HSRV_OPENFLOW_LOG_ERR("Not find tunnel info:%d", p_action->value.output.ifindex);
                    return HSRV_E_ENTRY_NOT_EXIST;
                }

                HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(p_tnl_info->tunnel_info.bond_port_ifindex, &combo_action.action_combo.output_gport));
            }
            else
            {
                HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(p_action->value.output.ifindex, &combo_action.action_combo.output_gport));
            }

            sal_memcpy(&action_combo_new[count_new], &combo_action, sizeof(_hsrv_openflow_action_combo_t));
            count_new ++;
        }
    }

    nhid_count = 0;
    /*compare old action unit with new action unit to get the del-nexthop-id in old group */
    for (old_index = 0; old_index < count_old; old_index ++)
    {
        flag = FALSE;
        for (new_index = 0; new_index < count_new; new_index ++)
        {
            if (!sal_memcmp(&action_combo_new[new_index], &action_combo_old[old_index], sizeof(_hsrv_openflow_action_combo_t)))
            {
                sal_memset(&action_combo_new[new_index], 0, sizeof(_hsrv_openflow_action_combo_t));
                flag = TRUE;
                break;
            }
        }
        if (!flag)
        {
            nhid_del_array[nhid_count ++] = nhid_old[old_index];
        }
    }
    *nhid_del_count = nhid_count;
    
    return HSRV_E_NONE;
}



/*************************************************************************************************
 * Name         : _hsrv_openflow_group_release_buckets
 * Purpose      : Fea layer group dealloc group buckets
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_release_buckets(hsrv_openflow_group_t* p_group)
{
    uint32 bucket_idx;
    uint32 member_idx;
    uint32 n_buckets;
    uint32 n_members;
    hsrv_openflow_group_nexthop_info_t* p_nh_info = NULL;

    p_nh_info = &p_group->u.dft.nh_info;
    n_buckets = p_nh_info->bucket_count;

    for (bucket_idx = 0; bucket_idx < n_buckets; bucket_idx ++)
    {
        n_members = p_nh_info->buckets[bucket_idx].member_count;
        for (member_idx = 0; member_idx < n_members; member_idx ++)
        {
            hsrv_of_nh_info_t *member_nh;

            member_nh = &p_nh_info->buckets[bucket_idx].member_nh[member_idx];
            hsrv_openflow_nexthop_release_nh_info(member_nh);
        }

        p_nh_info->buckets[bucket_idx].member_count = 0; 
        sal_memset(p_nh_info->buckets[bucket_idx].member_nh, 0,
                   sizeof(hsrv_of_nh_info_t) * GLB_ACTION_NUM_PER_BUCKET);
    }

    if (p_nh_info->buckets)
    {
        HSRV_OPENFLOW_GROUP_FREE(p_nh_info->buckets);
    }
                
    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_group_add_buckets_to_group
 * Purpose      : Fea layer group add buckets to group
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_add_buckets_to_group(hsrv_openflow_group_t* p_group)
{
    if (p_group->u.dft.nh_info.use_mcast)
    {
        HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_add_buckets_to_mcast_group(p_group));
    }

    if (p_group->u.dft.nh_info.use_ecmp)
    {
        HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_add_buckets_to_ecmp_group(p_group));
    }

    if (p_group->u.dft.nh_info.use_linkagg)
    {
        HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_add_buckets_to_linkagg_group(p_group));
    }

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_group_del_buckets_from_group
 * Purpose      : Fea layer group dealloc group buckets
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_del_buckets_from_group(hsrv_openflow_group_t* p_group)
{
    if (p_group->u.dft.nh_info.use_mcast)
    {
        HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_del_bucket_from_mcast_buckets(p_group));
    }

    if (p_group->u.dft.nh_info.use_ecmp)
    {
        HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_del_buckets_from_ecmp_group(p_group));
    }

    if (p_group->u.dft.nh_info.use_linkagg)
    {
        HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_del_buckets_from_linkagg_group(p_group));
    }

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_group_del_buckets_from_referred_group
 * Purpose      : Fea layer group remove group buckets
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_del_buckets_from_referred_group(hsrv_openflow_group_t* p_group)
{
    uint32 refer_group_id = 0;
    uint32 group_idx = 0;
    hsrv_openflow_group_t* p_refer_group = NULL;

    for (group_idx = 0; group_idx < p_group->group.referred_group_count; group_idx++)
    {
        refer_group_id  = p_group->group.referred_group[group_idx];
        HSRV_IF_ERROR_RETURN(hsrv_openflow_groupdb_lookup_group(refer_group_id, &p_refer_group));
        if (!p_refer_group)
        {
            continue;
        }
        HSRV_IF_ERROR_RETURN(_hsrv_openflow_del_nested_group_member_one(p_refer_group, p_group));
    }
    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_group_add_buckets_to_referred_group
 * Purpose      : Fea layer add buckets to group
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_add_buckets_to_referred_group(hsrv_openflow_group_t* p_group)
{
    uint32 refer_group_id = 0;
    uint32 group_idx = 0;
    hsrv_openflow_group_t* p_refer_group = NULL;

    for (group_idx = 0; group_idx < p_group->group.referred_group_count; group_idx++)
    {
        refer_group_id  = p_group->group.referred_group[group_idx];
        HSRV_IF_ERROR_RETURN(hsrv_openflow_groupdb_lookup_group(refer_group_id, &p_refer_group));
        if (!p_refer_group)
        {
            continue;
        }
        HSRV_IF_ERROR_RETURN(_hsrv_openflow_add_nested_group_member_one(p_refer_group, p_group));
    }
    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_group_add_buckets
 * Purpose      : Hsrv layer group add buckets
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_add_buckets(hsrv_openflow_group_t* p_group)
{
    HSRV_OPENFLOW_PTR_CHECK(p_group);

    switch (p_group->group.group_type)
    {
        case GLB_OF_GROUP_TYPE_FF:
            HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_set_first_live_bucket(p_group));
        case GLB_OF_GROUP_TYPE_ALL:
        case GLB_OF_GROUP_TYPE_SELECT:
        case GLB_OF_GROUP_TYPE_INDIRECT:
            HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_allocate_buckets(p_group));
            HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_add_buckets_to_group(p_group));
            break;
        default:
            break;
    }

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_group_del_buckets
 * Purpose      : Hsrv layer group release group buckets
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_del_buckets(hsrv_openflow_group_t* p_group)
{
    HSRV_OPENFLOW_PTR_CHECK(p_group);

    switch (p_group->group.group_type)
    {
        case GLB_OF_GROUP_TYPE_ALL:
        case GLB_OF_GROUP_TYPE_SELECT:
        case GLB_OF_GROUP_TYPE_INDIRECT:
        case GLB_OF_GROUP_TYPE_FF:
            HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_del_buckets_from_group(p_group));
            HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_release_buckets(p_group));
            break;
        default:
            break;
    }

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_group_add_group
 * Purpose      : Fea layer group alloc group nhid
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_add_group(hsrv_openflow_group_t* p_group)
{
    HSRV_OPENFLOW_PTR_CHECK(p_group);

    switch (p_group->group.group_type)
    {
        case GLB_OF_GROUP_TYPE_ALL:
        case GLB_OF_GROUP_TYPE_INDIRECT:
        case GLB_OF_GROUP_TYPE_FF:
            HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_add_mcast_group(p_group));
            break;
        case GLB_OF_GROUP_TYPE_SELECT:
            HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_add_ecmp_group(p_group));
#if 0 /* disable nesting group */
            if (p_group->group.linkagg_en)
            {
                HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_add_linkagg_group(p_group));
            }
#endif
            break;
        default:
            break;
    }

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_group_del_group
 * Purpose      : Hsrv layer group alloc group nhid
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_del_group(hsrv_openflow_group_t* p_group)
{
    hsrv_openflow_group_nexthop_info_t* p_nh_info = NULL;
    hsrv_of_nh_offset_t* p_group_nh = NULL;

    HSRV_OPENFLOW_PTR_CHECK(p_group);

    p_nh_info = &p_group->u.dft.nh_info;
    p_group_nh = &p_nh_info->main_nh.nh_offset;

    /* delete stats */
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_del_stats(p_group));

    if (p_nh_info->use_mcast)
    {
        HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_del_mcast_group(p_group_nh));
    }

    if (p_nh_info->use_ecmp)
    {
        HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_del_ecmp_group(p_group_nh));
    }

    if (p_nh_info->use_linkagg)
    {
        HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_del_linkagg_group(p_group));
    }

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_group_alloc_group
 * Purpose      : Fea layer group alloc group info
 * Input        : pp_group: hsrv_openflow_group_t*, p_fei_group: glb_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_alloc_group(hsrv_openflow_group_t* pp_group, glb_openflow_group_t *p_fei_group)
{
    uint32 buckets_size = 0;
    uint32 actions_size = 0;
    uint32 bucket_idx = 0;
    glb_openflow_group_t*  p_group = NULL;
    glb_openflow_bucket_t* p_bucket = NULL;

    HSRV_OPENFLOW_PTR_CHECK(pp_group);
    HSRV_OPENFLOW_PTR_CHECK(p_fei_group);

    p_group = &pp_group->group;
    sal_memcpy(p_group, p_fei_group, sizeof(glb_openflow_group_t));

    /* alloc glb_openflow_bucket_t */
    buckets_size = p_fei_group->n_buckets * sizeof(glb_openflow_bucket_t);
    p_group->bucket_list = HSRV_OPENFLOW_GROUP_MALLOC(buckets_size);
    HSRV_OPENFLOW_MEM_CHECK(p_group->bucket_list);
    sal_memset(p_group->bucket_list, 0, buckets_size);

    for (bucket_idx = 0; bucket_idx < p_fei_group->n_buckets; bucket_idx ++)
    {
        p_bucket = &p_fei_group->bucket_list[bucket_idx];
        sal_memcpy(&p_group->bucket_list[bucket_idx], p_bucket, sizeof(glb_openflow_bucket_t));

        if (!p_bucket->n_actions)
        {
            continue;
        }

        /* alloc glb_openflow_flow_action_t */
        actions_size = p_bucket->n_actions * sizeof(glb_openflow_flow_action_t);
        p_group->bucket_list[bucket_idx].actlist = HSRV_OPENFLOW_GROUP_MALLOC(actions_size);
        HSRV_OPENFLOW_MEM_CHECK(p_group->bucket_list[bucket_idx].actlist);
        sal_memset(p_group->bucket_list[bucket_idx].actlist, 0, actions_size);

        sal_memcpy(p_group->bucket_list[bucket_idx].actlist, p_bucket->actlist, actions_size);
    }
    
    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_group_release_group
 * Purpose      : Fea layer group dealloc group info
 * Input        : pp_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_release_group(hsrv_openflow_group_t* p_group)
{
    uint32 bucket_idx = 0;
    glb_openflow_bucket_t* p_bucket = NULL;

    if (!p_group)
    {
        return HSRV_E_NONE;
    }

    /* release glb_openflow_flow_action_t */
    for (bucket_idx = 0; bucket_idx < p_group->group.n_buckets; bucket_idx ++)
    {
        p_bucket = &p_group->group.bucket_list[bucket_idx];
        if (p_bucket->actlist)
        {
            HSRV_OPENFLOW_GROUP_FREE(p_bucket->actlist);
        }
        p_bucket->actlist = NULL;
    }

    /* release glb_openflow_bucket_t */
    if (p_group->group.bucket_list)
    {
        HSRV_OPENFLOW_GROUP_FREE(p_group->group.bucket_list);
        p_group->group.bucket_list = NULL;
    }

    /* release hsrv_openflow_group_t */
    HSRV_OPENFLOW_GROUP_FREE(p_group);
    p_group = NULL;

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_group_mod_all_group
 * Purpose      : Hsrv layer group mod all group
 * Input        : p_old_group: hsrv_openflow_group_t*, p_new_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_mod_all_group(hsrv_openflow_group_t* p_old_group, hsrv_openflow_group_t* p_new_group)
{
    uint32 ref_group_id = 0;
    uint32 group_idx = 0;
    uint32 nested_group_new_idx = 0;
    uint32 nested_group_old_idx = 0;
    hsrv_openflow_group_t *p_ref_group = NULL;
    hsrv_openflow_group_t *p_nested_group = NULL;
    uint32 bucket_idx = 0;
    uint32 member_idx = 0;
    uint32 member_idx2 = 0;
    uint32 del_nh_count = 0;
    uint32 old_nh_count = 0;
    uint32 new_nh_count = 0;
    uint32 nhid_del_array[GLB_MAX_MEMBER_PER_MULTICAST_GROUP] = {0};
    uint32 nhid_old_array[GLB_MAX_MEMBER_PER_MULTICAST_GROUP] = {0};
    uint32 nhid_new_array[GLB_MAX_MEMBER_PER_MULTICAST_GROUP] = {0};
    uint32 nhid_add_array[GLB_MAX_MEMBER_PER_MULTICAST_GROUP] = {0};
    hsrv_openflow_group_nexthop_info_t* p_nh_info = NULL;
    uint32 group_nhid;
    bool flag = FALSE;
    uint32 count = 0;
    uint32 index = 0;
    uint32 ret;

    HSRV_OPENFLOW_PTR_CHECK( p_old_group);
    HSRV_OPENFLOW_PTR_CHECK( p_new_group);

    group_nhid = p_old_group->u.dft.nh_info.main_nh.nh_offset.nhid;
    sal_memcpy(&p_new_group->u, &p_old_group->u, sizeof(hsrv_openflow_group_info_t));
    

    /*get del_nh_id  in old group*/
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_get_del_nexthop(p_new_group, p_old_group, nhid_del_array, &del_nh_count));

    /*remove del member from mcast and del nexthop*/
    p_nh_info = &p_old_group->u.dft.nh_info;
    for (bucket_idx = 0; bucket_idx < p_nh_info->bucket_count; bucket_idx++)
    {
        for (member_idx = 0; member_idx < p_nh_info->buckets[bucket_idx].member_count; member_idx++)
        {
            hsrv_of_nh_info_t *member_nh;
            hsrv_ofnh_mcast_member_param_t param;
            sal_memset(&param, 0 ,sizeof(hsrv_ofnh_mcast_member_param_t));
            member_nh = &p_nh_info->buckets[bucket_idx].member_nh[member_idx];
            for (index = 0; index < del_nh_count; index++)
            {
                if (nhid_del_array[index] == member_nh->nh_offset.nhid && nhid_del_array[index])
                {
                    nhid_del_array[index] = 0;
                    param.op_type = HSRV_OFNH_MCAST_OP_DEL_MEMBER;
                    param.mem_gport = member_nh->gport;
                    param.mem_nhid = member_nh->nh_offset.nhid;
                    param.port_check_discard = member_nh->nh_offset.port_check_discard;
                    param.redirect_to_ingress_port = member_nh->nh_offset.redirect_to_ingress_port;
                    /*remove del_nh_id from referred old group,for group chain modified*/
                    for (group_idx = 0; group_idx < p_new_group->group.referred_group_count; group_idx ++)
                    {
                        ref_group_id = p_new_group->group.referred_group[group_idx];
                        HSRV_IF_ERROR_RETURN(hsrv_openflow_groupdb_lookup_group(ref_group_id, &p_ref_group));
                        if (!p_ref_group)
                        {
                            continue;
                        }
                        param.group_nhid = p_ref_group->u.dft.nh_info.main_nh.nh_offset.nhid;
                        HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_remove_mcast_member(&param));
                    }
                    param.group_nhid = group_nhid;
                    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_remove_mcast_member(&param));
                    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_release_nh_info(member_nh));
                }
            }
            if (member_nh->nh_offset.nhid)
            {
                nhid_old_array[old_nh_count++] = member_nh->nh_offset.nhid;
            }
        } 
    }
    
    /*alloc new group nexthop*/
    ret = _hsrv_openflow_group_allocate_buckets(p_new_group);
    if (ret)
    {
        _hsrv_openflow_group_release_buckets(p_new_group);
        return HSRV_E_INVALID_PARAM;
    }

    /*get new group nexthop id*/
    p_nh_info = &p_new_group->u.dft.nh_info;
    for (bucket_idx = 0; bucket_idx < p_nh_info->bucket_count; bucket_idx++)
    {
        for (member_idx = 0; member_idx < p_nh_info->buckets[bucket_idx].member_count; member_idx++)
        {
            nhid_new_array[new_nh_count] = p_nh_info->buckets[bucket_idx].member_nh[member_idx].nh_offset.nhid;
            new_nh_count ++;
        } 
    }

    /*get new add nexthop id*/
    count = 0;
    for (member_idx = 0; member_idx < new_nh_count; member_idx ++)
    {
        flag = FALSE;
        for (member_idx2 = 0; member_idx2 < old_nh_count; member_idx2 ++)
        {
            if (nhid_old_array[member_idx2] == nhid_new_array[member_idx])
            {
                nhid_old_array[member_idx2] = 0;
                flag = TRUE;
                break;
            }
        }
        if (!flag)
        {
            nhid_add_array[count++] = nhid_new_array[member_idx];
        }
    }

    /*add new nexthop id to mcast member*/
    p_nh_info = &p_new_group->u.dft.nh_info;
    if (count)
    {
        for (bucket_idx = 0; bucket_idx < p_nh_info->bucket_count; bucket_idx++)
        {
            for (member_idx = 0; member_idx < p_nh_info->buckets[bucket_idx].member_count; member_idx++)
            {
                hsrv_of_nh_info_t *member_nh;
                hsrv_ofnh_mcast_member_param_t param;
                sal_memset(&param, 0 ,sizeof(hsrv_ofnh_mcast_member_param_t));
                member_nh = &p_nh_info->buckets[bucket_idx].member_nh[member_idx];
                for (index = 0; index < count; index ++)
                {
                    if (member_nh->nh_offset.nhid == nhid_add_array[index] && nhid_add_array[index])
                    {
                        nhid_add_array[index] = 0;
                        param.op_type = HSRV_OFNH_MCAST_OP_ADD_MEMBER;
                        param.mem_gport = member_nh->gport;
                        param.mem_nhid = member_nh->nh_offset.nhid;
                        param.port_check_discard = member_nh->nh_offset.port_check_discard;
                        param.redirect_to_ingress_port = member_nh->nh_offset.redirect_to_ingress_port;
                        /*add new nexthop to referred old group,for group chain modified*/
                        for (group_idx = 0; group_idx < p_new_group->group.referred_group_count; group_idx ++)
                        {
                            ref_group_id = p_new_group->group.referred_group[group_idx];
                            HSRV_IF_ERROR_RETURN(hsrv_openflow_groupdb_lookup_group(ref_group_id, &p_ref_group));
                            if (!p_ref_group)
                            {
                                continue;
                            }
                            param.group_nhid = p_ref_group->u.dft.nh_info.main_nh.nh_offset.nhid;
                            HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_add_mcast_member(&param));
                        }
                        param.group_nhid = group_nhid;
                        HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_add_mcast_member(&param));
                    }
                }
            } 
        }
    }
    
    /*delete old group rested nexthop*/
    p_nh_info = &p_old_group->u.dft.nh_info;
    for (bucket_idx = 0; bucket_idx < p_nh_info->bucket_count; bucket_idx++)
    {
        for (member_idx = 0; member_idx < p_nh_info->buckets[bucket_idx].member_count; member_idx++)
        {
            hsrv_of_nh_info_t *member_nh;
            member_nh = &p_nh_info->buckets[bucket_idx].member_nh[member_idx];
            if (member_nh->nh_offset.nhid)
            {
                HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_release_nh_info(member_nh));
            }
        } 
    }

    /*for group mod:get del nested group*/
    for (nested_group_old_idx = 0; nested_group_old_idx < p_old_group->group.nested_group_count; nested_group_old_idx++)
    {
        flag = FALSE;
        for (nested_group_new_idx = 0; nested_group_new_idx < p_new_group->group.nested_group_count; nested_group_new_idx++)
        {
            if (p_new_group->group.nested_group[nested_group_new_idx] == p_old_group->group.nested_group[nested_group_old_idx])
            {
                flag = TRUE;
            }
        }
        if (!flag)
        {
            HSRV_IF_ERROR_RETURN(hsrv_openflow_groupdb_lookup_group(p_old_group->group.nested_group[nested_group_old_idx], &p_nested_group));
            if (!p_nested_group)
            {
                return HSRV_E_ENTRY_NOT_EXIST;
            }
            HSRV_IF_ERROR_RETURN(_hsrv_openflow_del_nested_group_member_one(p_old_group, p_nested_group));
        }
    }

    /*for group mod:get add new nested group*/
    for (nested_group_new_idx = 0; nested_group_new_idx < p_new_group->group.nested_group_count; nested_group_new_idx++)
    {
        flag = FALSE;
        for (nested_group_old_idx = 0; nested_group_old_idx < p_old_group->group.nested_group_count; nested_group_old_idx++)
        {
            if (p_new_group->group.nested_group[nested_group_new_idx] == p_old_group->group.nested_group[nested_group_old_idx])
            {
                flag = TRUE;
            }
        }
        if (!flag)
        {
            HSRV_IF_ERROR_RETURN(hsrv_openflow_groupdb_lookup_group(p_new_group->group.nested_group[nested_group_new_idx], &p_nested_group));
            if (!p_nested_group)
            {
                return HSRV_E_ENTRY_NOT_EXIST;
            }
           HSRV_IF_ERROR_RETURN(_hsrv_openflow_add_nested_group_member_one(p_new_group, p_nested_group));
        }
    }

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_group_mod_select_group
 * Purpose      : Hsrv layer group mod select group
 * Input        : p_old_group: hsrv_openflow_group_t*, p_new_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_mod_select_group(hsrv_openflow_group_t* p_old_group, hsrv_openflow_group_t* p_new_group)
{
    uint16 type_mode = 0;

    HSRV_OPENFLOW_PTR_CHECK( p_old_group);
    HSRV_OPENFLOW_PTR_CHECK( p_new_group);

    if (p_old_group->group.linkagg_en && p_new_group->group.linkagg_en)
    {
        type_mode = 1; /* linkagg -> linkagg */
    }
    else if ((!p_old_group->group.linkagg_en) && p_new_group->group.linkagg_en)
    {
        type_mode = 2; /* ecmp -> linkagg */
    }
    else if (p_old_group->group.linkagg_en && (!p_new_group->group.linkagg_en))
    {
        type_mode = 3; /* linkagg -> ecmp */
    }
    else if ((!p_old_group->group.linkagg_en) && (!p_new_group->group.linkagg_en))
    {
        type_mode = 4; /* ecmp -> ecmp */
    }

    switch (type_mode)
    {
        case 1:
            HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_mod_buckets_to_linkagg_group(p_old_group, p_new_group));
            /* Added by weizj for bug 36896 */
            HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_mod_buckets_to_ecmp_group(p_old_group, p_new_group));
            break;
        case 2:
            HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_mod_buckets_ecmp_to_linkagg_group(p_old_group, p_new_group));
            break;
        case 3:
            HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_mod_buckets_linkagg_to_ecmp_group(p_old_group, p_new_group));
            break;
        case 4:
            HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_mod_buckets_to_ecmp_group(p_old_group, p_new_group));
            break;
        default:
            break;
    }

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_group_mod_indirect_group
 * Purpose      : Hsrv layer group mod indirect group
 * Input        : p_old_group: hsrv_openflow_group_t*, p_new_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_mod_indirect_group(hsrv_openflow_group_t* p_old_group, hsrv_openflow_group_t* p_new_group)
{
    uint32 ret;

    HSRV_OPENFLOW_PTR_CHECK( p_old_group);
    HSRV_OPENFLOW_PTR_CHECK( p_new_group);

    /* delete old indirect group member */
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_del_buckets_from_group(p_old_group));

    /* delete old group bucket nhid */
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_release_buckets(p_old_group));

    /* alloc new group first bucket */
    sal_memcpy(&p_new_group->u, &p_old_group->u, sizeof(hsrv_openflow_group_info_t));
    ret = _hsrv_openflow_group_allocate_buckets(p_new_group);
    if (ret)
    {
        _hsrv_openflow_group_release_buckets(p_new_group);
        return HSRV_E_INVALID_PARAM;
    }

    /* add new indirect group member */
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_add_buckets_to_group(p_new_group));

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_group_mod_ff_group
 * Purpose      : Hsrv layer group mod ff group
 * Input        : p_old_group: hsrv_openflow_group_t*, p_new_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_mod_ff_group(hsrv_openflow_group_t* p_old_group, hsrv_openflow_group_t* p_new_group)
{
    uint32 ret;

    HSRV_OPENFLOW_PTR_CHECK( p_old_group);
    HSRV_OPENFLOW_PTR_CHECK( p_new_group);

    /* set new group first bucket */
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_set_first_live_bucket(p_new_group));

    /* delete old ff group member */
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_del_buckets_from_group(p_old_group));

    /*added by hansf for ff group chain mod: delete old ff group member from referred group*/
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_del_buckets_from_referred_group(p_old_group));
    
    /* delete old group bucket nhid */
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_release_buckets(p_old_group));

    /* alloc new group first bucket */
    sal_memcpy(&p_new_group->u, &p_old_group->u, sizeof(hsrv_openflow_group_info_t));
    ret = _hsrv_openflow_group_allocate_buckets(p_new_group);
    if (ret)
    {
        _hsrv_openflow_group_release_buckets(p_new_group);
        return HSRV_E_INVALID_PARAM;
    }

    /* add new ff group member */
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_add_buckets_to_group(p_new_group));

    /*added by hansf for ff group chain mod: add new ff group member to referred group*/
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_add_buckets_to_referred_group(p_new_group));

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_group_mod_group
 * Purpose      : Fea layer group update group nhid
 * Input        : p_old_group: hsrv_openflow_group_t*, p_new_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_group_mod_group(hsrv_openflow_group_t* p_old_group, hsrv_openflow_group_t* p_new_group)
{
    uint32 group_id;

    HSRV_OPENFLOW_PTR_CHECK(p_old_group);
    HSRV_OPENFLOW_PTR_CHECK(p_new_group);

    group_id = p_old_group->group.group_id;
    if (p_old_group->group.group_type != p_new_group->group.group_type)
    {
        HSRV_OPENFLOW_LOG_ERR("Fail to update group: %d, group type is different", group_id);
        return HSRV_E_INVALID_PARAM;
    }

    switch (p_old_group->group.group_type)
    {
        case GLB_OF_GROUP_TYPE_ALL:
            HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_mod_all_group(p_old_group, p_new_group));
            break;
        case GLB_OF_GROUP_TYPE_SELECT:
            HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_mod_select_group(p_old_group, p_new_group));
            break;
        case GLB_OF_GROUP_TYPE_INDIRECT:
            HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_mod_indirect_group(p_old_group, p_new_group));
            break;
        case GLB_OF_GROUP_TYPE_FF:
            HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_mod_ff_group(p_old_group, p_new_group));
            break;
        default:
            break;
    }
    
    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_group_update_select_group_liveness
 * Purpose      : Fea layer group update select group liveness
 * Input        : p_group: hsrv_openflow_group_t*, ifindex: uint32, link_enable: bool
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
/* Added by weizj for bug 36895 */
static int32
_hsrv_openflow_group_update_select_group_liveness(hsrv_openflow_group_t* p_group,
    uint32 ifindex, bool link_enable)
{
    uint32 bucket_idx;
    uint32 n_buckets;
    uint16 gport;
    uint32 nhid_array[GLB_BUCKET_NUM_PER_SELECT_GROUP] = {0};
    hsrv_openflow_group_nexthop_info_t* p_nh_info = NULL;

    p_nh_info = &p_group->u.dft.nh_info;
    n_buckets = p_group->group.n_buckets;

    /* update select group member */
    for (bucket_idx = 0; bucket_idx < n_buckets; bucket_idx ++)
    {
        if (0 == p_group->group.bucket_list[bucket_idx].n_actions)
        {
            continue;
        }

        if (ifindex == p_nh_info->buckets[bucket_idx].member_nh[0].ifindex)
        {
            nhid_array[0] = p_nh_info->buckets[bucket_idx].member_nh[0].nh_offset.nhid;
            gport = p_nh_info->buckets[bucket_idx].member_nh[0].gport;
            /* update ecmp member */
            if (link_enable)
            {
                HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_add_ecmp_member(
                             p_nh_info->main_nh.nh_offset.nhid,
                             p_group->u.dft.stats_id,
                             1,
                             nhid_array, FALSE)); 
            }
            else
            {
                HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_remove_ecmp_member(
                             p_nh_info->main_nh.nh_offset.nhid,
                             1,
                             nhid_array));
            }

            /* update linkagg member */
            if (p_group->group.linkagg_en)
            {
                if (link_enable)
                {
                    HSRV_IF_ERROR_RETURN(hsrv_openflow_linkagg_add_port(
                        p_nh_info->linkagg_nh.tid, gport));
                }
                else
                {
                    HSRV_IF_ERROR_RETURN(hsrv_openflow_linkagg_del_port(
                        p_nh_info->linkagg_nh.tid, gport));
                }
            }
        }
    }  
    
    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_group_update_ff_group_liveness
 * Purpose      : Fea layer group update ff group liveness
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
/* Added by weizj for bug 36895 */
int32
_hsrv_openflow_group_update_ff_group_liveness(hsrv_openflow_group_t* p_group)
{
    HSRV_PTR_CHECK(p_group);

    /* delete old member */
    _hsrv_openflow_group_del_buckets_from_group(p_group);
    
    /*added by hansf for ff group chain mod: delete old ff group member from referred group*/
    _hsrv_openflow_group_del_buckets_from_referred_group(p_group);
    
    /* set new first bucket */
    _hsrv_openflow_group_set_first_live_bucket(p_group);
    /* add new member */
    _hsrv_openflow_group_add_buckets_to_group(p_group);

    /*added by hansf for ff group chain mod: add new ff group member to referred group*/
    _hsrv_openflow_group_add_buckets_to_referred_group(p_group);

    return HSRV_E_NONE;
}

#define _____HAL_APIs_____

/* modified by liwh for bug 46591 to support mpls tpoam, 2018-03-28 */
/*************************************************************************************************
 * Name         : _hsrv_openflow_add_lsp_aps_group
 * Purpose      : Fea layer add lsp aps group
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_add_lsp_aps_group(hsrv_openflow_group_t* p_group)
{
    uint32                                      bucket_idx = 0;
    glb_openflow_bucket_t*             p_bucket = NULL;
    glb_openflow_tpoam_bucket_t*   p_tpoam_bucket = NULL;
    uint32                                      buckets_size = 0;
    uint16                                      gport = 0;
     
    HSRV_OPENFLOW_PTR_CHECK(p_group);
    
    buckets_size = p_group->group.n_buckets * sizeof(glb_openflow_tpoam_bucket_t);
    p_group->bucket_list = HSRV_OPENFLOW_GROUP_MALLOC(buckets_size);
    HSRV_OPENFLOW_MEM_CHECK(p_group->bucket_list);
    sal_memset(p_group->bucket_list, 0, buckets_size);
    
    for (bucket_idx = 0; bucket_idx < p_group->group.n_buckets; bucket_idx ++)
    {
        /* get group bucket and tpoam bucket */
        p_bucket = &p_group->group.bucket_list[bucket_idx];
        p_tpoam_bucket = &p_group->bucket_list[bucket_idx];

        p_tpoam_bucket->label0 = p_bucket->label0;
        p_tpoam_bucket->label1 = p_bucket->label1;
        p_tpoam_bucket->vid = p_bucket->vid;
        sal_memcpy(&p_tpoam_bucket->nexthop_mac, &p_bucket->nexthop_mac, sizeof(mac_addr_t));
        p_tpoam_bucket->ofport = p_bucket->ofport;
        p_tpoam_bucket->lsp_aps_group_id = p_bucket->lsp_aps_group_id;
        p_tpoam_bucket->ifindex = p_bucket->ifindex;

        if(p_tpoam_bucket->ifindex < GLB_BOND_IFINDEX_BASE)
        {
            HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(p_tpoam_bucket->ifindex, &gport));
        }
        else
        {
            HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_agg_portid(p_tpoam_bucket->ifindex, &gport));
        }

        p_tpoam_bucket->gport = gport;
    }

    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_create_lsp_aps_tbl(&p_group->aps_group.lsp_aps_nh_hash));

    if ((2== p_group->group.n_buckets) && p_group->bucket_list)
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_add_lsp_aps_tunnel(&p_group->bucket_list[0], &p_group->bucket_list[1],
            &p_group->aps_group.aps_group_offset, &p_group->aps_group.tunnel_id));
        /*modified by yejl for bug 52155, 2019-05-22*/
        HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_alloc_lsp_aps_nh(
            p_group->aps_group.tunnel_id, 0, 
            &p_group->aps_group.aps_nhid, &p_group->aps_group.aps_offset,
            &p_group->aps_group.mcast_nhid, &p_group->aps_group.mcast_offset,
            p_group->aps_group.lsp_aps_nh_hash));
        /*ended by yejl*/
    }

    p_group->aps_group.fast_aps_en = TRUE;
    
    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_del_lsp_aps_group
 * Purpose      : Fea layer delete lsp group
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_del_lsp_aps_group(hsrv_openflow_group_t* p_group)
{
    uint32                                       bucket_idx = 0;
    glb_openflow_tpoam_bucket_t*    p_tpoam_bucket = NULL;
    
    HSRV_OPENFLOW_PTR_CHECK(p_group);

    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_release_lsp_aps_nh(p_group->aps_group.lsp_aps_nh_hash, 0));
    
    if (p_group->aps_group.lsp_aps_nh_hash)
    {
        ctclib_hash_free (p_group->aps_group.lsp_aps_nh_hash);
        p_group->aps_group.lsp_aps_nh_hash = NULL;
    }

    hsrv_openflow_delete_lsp_aps_tunnel(p_group->aps_group.aps_group_offset, p_group->aps_group.tunnel_id);
    
    for (bucket_idx = 0; bucket_idx < p_group->group.n_buckets; bucket_idx ++)
    {
        /* get tpoam bucket */
        p_tpoam_bucket = &p_group->bucket_list[bucket_idx];

        HSRV_OPENFLOW_PTR_CHECK(p_tpoam_bucket);

        hsrv_openflow_nexthop_release_mpls_nh((hsrv_of_nh_info_t*)p_tpoam_bucket->p_nh_info);

        HSRV_OPENFLOW_GROUP_FREE(p_tpoam_bucket->p_nh_info);
    }

    HSRV_OPENFLOW_GROUP_FREE(p_group->bucket_list);

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_add_pw_aps_group
 * Purpose      : Fea layer add pw aps group
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_add_pw_aps_group(hsrv_openflow_group_t* p_group)
{
    uint32                                      bucket_idx = 0;
    glb_openflow_bucket_t*             p_bucket = NULL;
    glb_openflow_tpoam_bucket_t*   p_tpoam_bucket = NULL;
    uint32                                      buckets_size = 0; 
    uint16                                      gport = 0;
    uint32                                      tunnel_id = 0;
   
    HSRV_OPENFLOW_PTR_CHECK(p_group);

    buckets_size = p_group->group.n_buckets * sizeof(glb_openflow_tpoam_bucket_t);
    p_group->bucket_list = HSRV_OPENFLOW_GROUP_MALLOC(buckets_size);
    HSRV_OPENFLOW_MEM_CHECK(p_group->bucket_list);
    sal_memset(p_group->bucket_list, 0, buckets_size);
    
    for (bucket_idx = 0; bucket_idx < p_group->group.n_buckets; bucket_idx ++)
    {
        /* get group bucket and tpoam bucket */
        p_bucket = &p_group->group.bucket_list[bucket_idx];
        p_tpoam_bucket = &p_group->bucket_list[bucket_idx];
/* modified by yangl for bug 53718 to support mpls tpoam, 2019-10-24 */
        p_tpoam_bucket->label0 = p_bucket->label0;
        p_tpoam_bucket->mpls_tc0 = p_bucket->mpls_tc0;
        p_tpoam_bucket->mpls_tc0_is_set= p_bucket->mpls_tc0_is_set;
        p_tpoam_bucket->mpls_ttl0= p_bucket->mpls_ttl0;
        
        p_tpoam_bucket->label1 = p_bucket->label1;
        p_tpoam_bucket->label1 = p_bucket->label1;
        p_tpoam_bucket->mpls_tc1 = p_bucket->mpls_tc1;
        p_tpoam_bucket->mpls_tc1_is_set= p_bucket->mpls_tc1_is_set;
        p_tpoam_bucket->mpls_ttl1= p_bucket->mpls_ttl1;
/*yangl end*/       
        p_tpoam_bucket->vid = p_bucket->vid;
        sal_memcpy(&p_tpoam_bucket->nexthop_mac, &p_bucket->nexthop_mac, sizeof(mac_addr_t));
        p_tpoam_bucket->ofport = p_bucket->ofport;
        p_tpoam_bucket->lsp_aps_group_id = p_bucket->lsp_aps_group_id;
        p_tpoam_bucket->ifindex = p_bucket->ifindex;

        if(p_tpoam_bucket->ifindex < GLB_BOND_IFINDEX_BASE)
        {
            HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(p_tpoam_bucket->ifindex, &gport));
        }
        else
        {
            HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_agg_portid(p_tpoam_bucket->ifindex, &gport));
        }
        p_tpoam_bucket->gport = gport;

        HSRV_IF_ERROR_RETURN(hsrv_openflow_add_pw_tunnel( p_tpoam_bucket, &tunnel_id));
  
        /* working lsp */
        if (0 == bucket_idx)
        {
            p_group->aps_group.tunnel_id = tunnel_id;
            p_group->aps_group.pw_label = p_tpoam_bucket->label0;
            p_group->aps_group.gport = p_tpoam_bucket->gport;
        }
        else
        {
            p_group->aps_group.ptunnel_id = tunnel_id;
            p_group->aps_group.ppw_label = p_tpoam_bucket->label0;
            p_group->aps_group.pgport = p_tpoam_bucket->gport;
        }
    }

    p_group->aps_group.fast_aps_en = TRUE;

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_del_pw_aps_group
 * Purpose      : Fea layer delete pw aps group
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_del_pw_aps_group(hsrv_openflow_group_t* p_group)
{
    uint32                                       bucket_idx = 0;
    glb_openflow_tpoam_bucket_t*    p_tpoam_bucket = NULL;
    
    HSRV_OPENFLOW_PTR_CHECK(p_group);

    for (bucket_idx = 0; bucket_idx < p_group->group.n_buckets; bucket_idx ++)
    {
        /* get tpoam bucket */
        p_tpoam_bucket = &p_group->bucket_list[bucket_idx];

        HSRV_OPENFLOW_PTR_CHECK(p_tpoam_bucket);

        hsrv_openflow_nexthop_release_mpls_nh((hsrv_of_nh_info_t*)p_tpoam_bucket->p_nh_info);

        HSRV_OPENFLOW_GROUP_FREE(p_tpoam_bucket->p_nh_info);
    }

    HSRV_OPENFLOW_GROUP_FREE(p_group->bucket_list);

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_add_pw_aps_lsp_aps_group
 * Purpose      : Fea layer add pw&lsp aps group
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_add_pw_aps_lsp_aps_group(hsrv_openflow_group_t* p_group)
{
    uint32                                      bucket_idx = 0;
    glb_openflow_bucket_t*             p_bucket = NULL;
    uint32                                      buckets_size = 0;
    Hsrv2AgtMsg_t                          hagt_msg;
    hsrv_openflow_group_t*             p_lsp1_group = NULL;
    hsrv_openflow_group_t*             p_lsp2_group = NULL;
   
    sal_memset(&hagt_msg, 0, sizeof(Hsrv2AgtMsg_t));
    
    HSRV_OPENFLOW_PTR_CHECK(p_group);

    buckets_size = p_group->group.n_buckets * sizeof(glb_openflow_tpoam_bucket_t);
    p_group->bucket_list = HSRV_OPENFLOW_GROUP_MALLOC(buckets_size);
    HSRV_OPENFLOW_MEM_CHECK(p_group->bucket_list);
    sal_memset(p_group->bucket_list, 0, buckets_size);
    
    for (bucket_idx = 0; bucket_idx < p_group->group.n_buckets; bucket_idx ++)
    {
        /* get group bucket and tpoam bucket */
        p_bucket = &p_group->group.bucket_list[bucket_idx];

        /* working lsp */
        if (0 == bucket_idx)
        {
            hsrv_openflow_group_lookup_group(p_bucket->lsp_aps_group_id, &p_lsp1_group);
            if (p_lsp1_group)
            {
                /*modified by yejl for bug 52155, 2019-05-22*/    
                HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_alloc_lsp_aps_nh(
                    p_lsp1_group->aps_group.tunnel_id, p_bucket->label0, 
                    &p_lsp1_group->aps_group.aps_nhid, &p_lsp1_group->aps_group.aps_offset,
                    &p_lsp1_group->aps_group.mcast_nhid, &p_lsp1_group->aps_group.mcast_offset,
                    p_lsp1_group->aps_group.lsp_aps_nh_hash));
                /*ended by yejl*/
                
                p_group->aps_group.tunnel_id = p_lsp1_group->aps_group.tunnel_id;
                p_group->aps_group.pw_label = p_bucket->label0;
                p_group->aps_group.lsp_aps_group_id = p_lsp1_group->aps_group.aps_group_offset; /*modified by yejl for bug 47825, 2019-06-25*/
                p_group->aps_group.bucket_lsp_aps_group_id = p_bucket->lsp_aps_group_id;
                p_group->aps_group.gport = GLB_INVALID_GPORTID;
            }
        }
        else
        {
            hsrv_openflow_group_lookup_group(p_bucket->lsp_aps_group_id, &p_lsp2_group);
            if (p_lsp2_group)
            {
                /*modified by yejl for bug 52155, 2019-05-22*/
                HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_alloc_lsp_aps_nh(
                    p_lsp2_group->aps_group.tunnel_id, p_bucket->label0, 
                    &p_lsp2_group->aps_group.aps_nhid, &p_lsp2_group->aps_group.aps_offset,
                    &p_lsp2_group->aps_group.mcast_nhid, &p_lsp2_group->aps_group.mcast_offset,
                    p_lsp2_group->aps_group.lsp_aps_nh_hash));
                /*ended by yejl*/
                
                p_group->aps_group.ptunnel_id = p_lsp2_group->aps_group.tunnel_id;
                p_group->aps_group.ppw_label = p_bucket->label0;
                p_group->aps_group.plsp_aps_group_id = p_lsp2_group->aps_group.aps_group_offset; /*modified by yejl for bug 47825, 2019-06-25*/
                p_group->aps_group.bucket_plsp_aps_group_id = p_bucket->lsp_aps_group_id;
                p_group->aps_group.pgport = GLB_INVALID_GPORTID;
            }
        }
    }

    p_group->aps_group.fast_aps_en = TRUE;

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_del_pw_aps_lsp_aps_group
 * Purpose      : Fea layer delete pw&lsp aps group
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_del_pw_aps_lsp_aps_group(hsrv_openflow_group_t* p_group)
{
    uint32                                       bucket_idx = 0;
    glb_openflow_bucket_t*               p_bucket = NULL;
    hsrv_openflow_group_t*               p_lsp_group = NULL;
    
    HSRV_OPENFLOW_PTR_CHECK(p_group);

    for (bucket_idx = 0; bucket_idx < p_group->group.n_buckets; bucket_idx ++)
    {
        /* get group bucket and tpoam bucket */
        p_bucket = &p_group->group.bucket_list[bucket_idx];

        HSRV_OPENFLOW_PTR_CHECK(p_bucket);

        hsrv_openflow_group_lookup_group(p_bucket->lsp_aps_group_id, &p_lsp_group);
        if (p_lsp_group)
        {
            HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_release_lsp_aps_nh(p_lsp_group->aps_group.lsp_aps_nh_hash, p_bucket->label0));
        }
    }

    HSRV_OPENFLOW_GROUP_FREE(p_group->bucket_list);

    return HSRV_E_NONE;
}

/* liwh end */

/*************************************************************************************************
 * Name         : fei_api_openflow_add_group
 * Purpose      : Hsrv layer group add group info
 * Input        : p_fei_group: glb_openflow_group_t *
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
fei_api_openflow_add_group(glb_openflow_group_t *p_fei_group)
{
    int32 ret = HSRV_E_NONE;
    hsrv_openflow_group_t* p_group = NULL;

    HSRV_IF_ERROR_RETURN(hsrv_openflow_groupdb_lookup_group(p_fei_group->group_id, &p_group));
    if (p_group)
    {
        HSRV_IF_ERROR_RETURN(fei_api_openflow_mod_group(p_group, p_fei_group));
        return HSRV_E_NONE;
    }

    /* 1. alloc group info */
    p_group = HSRV_OPENFLOW_GROUP_MALLOC(sizeof(hsrv_openflow_group_t));
    HSRV_OPENFLOW_MEM_CHECK(p_group);
    sal_memset(p_group, 0, sizeof(hsrv_openflow_group_t));
    ret = _hsrv_openflow_group_alloc_group(p_group, p_fei_group);
    if (ret)
    {
        _hsrv_openflow_group_release_group(p_group);
        return ret;
    }

    /* 2. alloc mcast/ecmp/linkagg nhid */
    ret = _hsrv_openflow_group_add_group(p_group);
    if (ret)
    {
        _hsrv_openflow_group_del_group(p_group);
        _hsrv_openflow_group_release_group(p_group);
        return ret;
    }
    
    /* 3. add members to mcast */
    ret = _hsrv_openflow_group_add_buckets(p_group);
    if (ret)
    {
        _hsrv_openflow_group_del_buckets(p_group);
        _hsrv_openflow_group_del_group(p_group);
        _hsrv_openflow_group_release_group(p_group);
        return ret;
    }

    /* modified by liwh for bug 46591 to support mpls tpoam, 2018-03-26 */
    if (GLB_OF_GROUP_TYPE_LSP_APS == p_group->group.group_type)
    {
        ret = _hsrv_openflow_add_lsp_aps_group(p_group);
        if (ret)
        {
            _hsrv_openflow_group_del_buckets(p_group);
            _hsrv_openflow_group_del_group(p_group);
            _hsrv_openflow_group_release_group(p_group);
            return ret;
        }
    }
    else if (GLB_OF_GROUP_TYPE_PW_APS == p_group->group.group_type)
    {
        if (0 == p_group->group.nested_group_count)
        {
            ret = _hsrv_openflow_add_pw_aps_group(p_group);
        }
        else
        {
            ret = _hsrv_openflow_add_pw_aps_lsp_aps_group(p_group);    
        }
        
        if (ret)
        {
            _hsrv_openflow_group_del_buckets(p_group);
            _hsrv_openflow_group_del_group(p_group);
            _hsrv_openflow_group_release_group(p_group);
            return ret;
        }
    }
    /* liwh end */

    /* 4. add to groupdb */
    hsrv_openflow_groupdb_add_group(p_group);

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : fei_api_openflow_del_group
 * Purpose      : Hsrv layer group delete group info
 * Input        : pst_req: fei_openflow_del_group_req_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
fei_api_openflow_del_group(fei_openflow_del_group_req_t* p_fei_group)
{
    hsrv_openflow_group_t* p_group = NULL;

    HSRV_IF_ERROR_RETURN(hsrv_openflow_groupdb_lookup_group(p_fei_group->group_id, &p_group));
    if (NULL == p_group)
    {
        HSRV_OPENFLOW_LOG_ERR("Fail to delete group %u", p_fei_group->group_id);
        return HSRV_E_NONE;
    }

    /* modified by liwh for bug 46591 to support mpls tpoam, 2018-03-26 */
    if (GLB_OF_GROUP_TYPE_LSP_APS == p_group->group.group_type)
    {
        HSRV_IF_ERROR_RETURN(_hsrv_openflow_del_lsp_aps_group(p_group));
    }
    else if (GLB_OF_GROUP_TYPE_PW_APS == p_group->group.group_type)
    {
        if (0 == p_group->group.nested_group_count)
        {
            HSRV_IF_ERROR_RETURN(_hsrv_openflow_del_pw_aps_group(p_group));
        }
        else
        {
            HSRV_IF_ERROR_RETURN(_hsrv_openflow_del_pw_aps_lsp_aps_group(p_group));
        }
    }
    /* liwh end */

    /* 1. release members from mcast */
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_del_buckets(p_group));

    /* 2. release mcast/ecmp/linkagg nhid */
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_del_group(p_group));

    /* 3. delete to groupdb */
    HSRV_IF_ERROR_RETURN(hsrv_openflow_groupdb_del_group(p_group));
    
    /* 4. release group info */
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_release_group(p_group));

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : fei_api_openflow_mod_group
 * Purpose      : Hsrv layer group update group info
 * Input        : pst_req: fei_openflow_del_group_req_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
fei_api_openflow_mod_group(hsrv_openflow_group_t* p_old_group, glb_openflow_group_t *p_fei_group)
{
    uint32 ret = HSRV_E_NONE;
    hsrv_openflow_group_t* p_new_group = NULL;
    hsrv_openflow_group_nexthop_info_t* p_nh_info = NULL;

    /*added by hansf for group chain: decide if need to mod group or just update group reference info*/
    if (p_fei_group->update_ff_group_en && GLB_OF_GROUP_TYPE_FF == p_fei_group->group_type)
    {
        p_old_group->group.referred_group_count = p_fei_group->referred_group_count;
        sal_memcpy(p_old_group->group.referred_group, p_fei_group->referred_group, sizeof(p_fei_group->referred_group));
        return ret;
    }

    /* 1. alloc group info */
    p_new_group = HSRV_OPENFLOW_GROUP_MALLOC(sizeof(hsrv_openflow_group_t));
    HSRV_OPENFLOW_MEM_CHECK(p_new_group);
    sal_memset(p_new_group, 0, sizeof(hsrv_openflow_group_t));
    ret = _hsrv_openflow_group_alloc_group(p_new_group, p_fei_group);
    if (ret)
    {
        _hsrv_openflow_group_release_group(p_new_group);
        return ret;
    }

    /* 2. update group member */
    ret = _hsrv_openflow_group_mod_group(p_old_group, p_new_group);
    if (ret)
    {
        _hsrv_openflow_group_release_buckets(p_new_group);
        _hsrv_openflow_group_release_group(p_new_group);
        return ret;
    }

    /* 3. delete old group resource */
    p_nh_info = &p_old_group->u.dft.nh_info;
    if (p_nh_info->buckets)
    {
        HSRV_OPENFLOW_GROUP_FREE(p_nh_info->buckets);
    }

    HSRV_IF_ERROR_RETURN(hsrv_openflow_groupdb_del_group(p_old_group));
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_release_group(p_old_group));

    /* add new group resource */
    HSRV_IF_ERROR_RETURN(hsrv_openflow_groupdb_add_group(p_new_group));
    /* TODO FIXME: if any error, need rollback? */

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : fei_api_openflow_clear_group_stats
 * Purpose      : Hsrv layer group clear group stats
 * Input        : pst_req: fei_openflow_clear_stats_req_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
fei_api_openflow_clear_group_stats(fei_openflow_clear_stats_req_t *pst_req)
{
    hsrv_openflow_group_t* p_group = NULL;

    HSRV_IF_ERROR_RETURN(hsrv_openflow_groupdb_lookup_group(pst_req->id, &p_group));
    if (NULL == p_group)
    {
        return HSRV_E_NONE;
    }

    HSRV_IF_ERROR_RETURN(_hsrv_openflow_group_clear_group_stats(p_group));

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : fei_api_openflow_set_group_drop_ingress
 * Purpose      : Hsrv layer group set group drop ingress
 * Input        : pst_req: fei_openflow_set_drop_ingress*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
/* Added by weizj for bug 35135 */
int32 
fei_api_openflow_set_group_drop_ingress(fei_openflow_set_drop_ingress *pst_req)
{
    hsrv_openflow_groupdb_set_drop_ingress(pst_req->is_drop);
    
    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : hsrv_openflow_group_lookup_group
 * Purpose      : Fea layer group lookup group
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
hsrv_openflow_group_lookup_group(uint32 group_id, hsrv_openflow_group_t** pp_group)
{
    HSRV_IF_ERROR_RETURN(hsrv_openflow_groupdb_lookup_group(group_id, pp_group));
    
    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : hsrv_openflow_group_get_drop_ingress
 * Purpose      : Hsrv layer group get group drop ingress
 * Input        : N/A
 * Output       : N/A
 * Return       : bool
**************************************************************************************************/
/* Added by weizj for bug 35135 */
bool
hsrv_openflow_group_get_drop_ingress(void)
{
    return hsrv_openflow_groupdb_get_drop_ingress();
}

/*************************************************************************************************
 * Name         : hsrv_openflow_group_get_drop_ingress
 * Purpose      : Hsrv layer group get group drop ingress
 * Input        : N/A
 * Output       : N/A
 * Return       : bool
**************************************************************************************************/
int32
hsrv_openflow_group_update_group_liveness(uint32 ifindex, bool link_enable)
{
    uint32 group_idx;
    uint32 group_id;
    hsrv_openflow_group_watch_port_lookup_t data;
    hsrv_openflow_group_t* p_group = NULL;

    sal_memset(&data, 0, sizeof(hsrv_openflow_group_watch_port_lookup_t));
    data.ifindex = ifindex;
    data.link_up = link_enable;
    HSRV_IF_ERROR_RETURN(hsrv_openflow_groupdb_loop_watch_port(&data));

    for (group_idx = 0; group_idx < data.group_count; group_idx ++)
    {
        group_id = data.groups[group_idx];
        HSRV_IF_ERROR_RETURN(hsrv_openflow_groupdb_lookup_group(group_id, &p_group));
        if (!p_group)
        {
            return HSRV_E_NONE;
        }

        /* Modifed by weizj for bug 36895 */
        if (GLB_OF_GROUP_TYPE_FF == p_group->group.group_type)
        {
            /* handle ff group */
            _hsrv_openflow_group_update_ff_group_liveness(p_group);
        }
        else if(GLB_OF_GROUP_TYPE_SELECT == p_group->group.group_type)
        {
            /* handle select group */
            _hsrv_openflow_group_update_select_group_liveness(p_group, ifindex, link_enable);
        }
    }

    return HSRV_E_NONE;
}

#define _____GROUP_DEBUG_____
/*************************************************************************************************
 * Name         : hsrv_openflow_show_group_brief__
 * Purpose      : Hsrv layer group show group brief
 * Input        : fp: FILE *
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
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

/*************************************************************************************************
 * Name         : hsrv_openflow_show_group_brief
 * Purpose      : Hsrv layer group show group brief
 * Input        : fp: FILE *
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
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

/*************************************************************************************************
 * Name         : hsrv_openflow_show_group
 * Purpose      : Hsrv layer group show group
 * Input        : pst_req: fei_openflow_set_drop_ingress*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
hsrv_openflow_show_group(FILE *fp, uint32 group_id)
{
    uint32 bucket_idx = 0;
    uint32 member_idx = 0;
    uint32 n_buckets = 0;
    uint32 n_members = 0;
    hsrv_openflow_group_t* p_group;
    glb_openflow_flow_action_list_t actlist;
    ctclib_hash_backet_t*                      hb;
    ctclib_hash_backet_t*                      next;
    uint32                                             i = 0;
    hsrv_openflow_lsp_aps_nh_t*            p_nh = NULL;

    HSRV_IF_ERROR_RETURN(hsrv_openflow_groupdb_lookup_group(group_id, &p_group));
    if (NULL == p_group)
    {
        sal_fprintf(fp, "group_id %u not exist in db.\n", group_id);
        return HSRV_E_NONE;
    }

    n_buckets =  p_group->group.n_buckets;
    sal_fprintf(fp, "----------HSRV OPENFLOW GROUP--------\n");

    /* modified by liwh for bug 46591 to support mpls tpoam, 2018-04-26 */
    if ((GLB_OF_GROUP_TYPE_LSP_APS == p_group->group.group_type)
        ||(GLB_OF_GROUP_TYPE_PW_APS == p_group->group.group_type))
    {
        sal_fprintf(fp, "group_id              : %u\n", group_id);
        sal_fprintf(fp, "group_type            : %s\n", HSRV_OF_GROUP_TYPE_STR(p_group->group.group_type));
        sal_fprintf(fp, "aps_group_offset      : %u\n", p_group->aps_group.aps_group_offset);

        sal_fprintf(fp, "fast_aps_en           : %u\n", p_group->aps_group.fast_aps_en);
        sal_fprintf(fp, "tunnel_id             : %u\n", p_group->aps_group.tunnel_id);
        sal_fprintf(fp, "ptunnel_id            : %u\n", p_group->aps_group.ptunnel_id);

        sal_fprintf(fp, "pw_label              : %u\n", p_group->aps_group.pw_label);
        sal_fprintf(fp, "ppw_label             : %u\n", p_group->aps_group.ppw_label);
        sal_fprintf(fp, "gport                 : %u\n", p_group->aps_group.gport);
        sal_fprintf(fp, "pgport                : %u\n", p_group->aps_group.pgport);

        sal_fprintf(fp, "aps_nhid              : %u\n", p_group->aps_group.aps_nhid);
        sal_fprintf(fp, "aps_offset            : %u\n", p_group->aps_group.aps_offset);

        sal_fprintf(fp, "lsp_aps_group_id      : %u\n", p_group->aps_group.bucket_lsp_aps_group_id);
        sal_fprintf(fp, "plsp_aps_group_id     : %u\n", p_group->aps_group.bucket_plsp_aps_group_id);

        if (p_group->aps_group.lsp_aps_nh_hash && p_group->aps_group.lsp_aps_nh_hash->count)
        {
            for (i = 0; i < p_group->aps_group.lsp_aps_nh_hash->size; i++)
            {
                for (hb = p_group->aps_group.lsp_aps_nh_hash->index[i]; hb; hb = next)
                {
                    next = hb->next;
                    p_nh = (hsrv_openflow_lsp_aps_nh_t *)hb->data;
        
                    if (p_nh)
                    {
                        sal_fprintf(fp, "\npw_label              : %u\n", p_nh->pw_label);
                        sal_fprintf(fp, "nhid                  : %u\n", p_nh->nhid);
                        sal_fprintf(fp, "count                 : %u\n", p_nh->count);
                    }
                }
            }
        }
    
        return HSRV_E_NONE;
    }
    /* liwh end */
    
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

#define _____GROUP_INIT_____
/*************************************************************************************************
 * Name         : hsrv_openflow_group_start
 * Purpose      : Fea layer group start
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
hsrv_openflow_group_start(void)
{
    HSRV_IF_ERROR_RETURN(hsrv_openflow_groupdb_start());

    return HSRV_E_NONE;
}

