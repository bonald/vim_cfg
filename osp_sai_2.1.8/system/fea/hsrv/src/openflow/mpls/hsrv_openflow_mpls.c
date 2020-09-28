/****************************************************************************************
 * hsrv_openflow_mpls.c   :  Hal server openflow mpls related function
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision     :       R0.01
 * Author       :       Zhang Dongya
 * Date         :       2015-11-26
 * Reason       :       First Create.
 ****************************************************************************************/

#include "hsrv_inc.h"

#include "hsrv_openflow.h"
#include "hsrv_openflow_mpls_priv.h"
#include "hsrv_openflow_mpls.h"
#include "hsrv_msg_openflow.h"

#define HSRV_MPLS_ETH_TYPE       0x8847
#define HSRV_MPLS_MCAST_ETH_TYPE 0x8848

static HSRV_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_ALLOC_RES_FUNC 
    hsrv_openflow_mpls_output_action_seq_res_func[GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_VALID_MAX];

static int32
_hsrv_openflow_mpls_alloc_res_for_seq_no_process_needed(glb_openflow_flow_action_t **cur_flow_action,
                                                                        hsrv_of_nh_info_t* p_member_nh_array,
                                                                        uint16 output_index)
{
    return HSRV_E_NONE;
}

/*GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_POP_ALL_MPLS_1*/
static int32
_hsrv_openflow_mpls_alloc_res_for_seq_pop_all_mpls_1(glb_openflow_flow_action_t **cur_flow_action,
                                                                   hsrv_of_nh_info_t* p_member_nh_array,
                                                                   uint16 mcast_member_index)
{
    const glb_openflow_flow_action_t *push_l2 = NULL;
    const glb_openflow_flow_action_t *pop_l2 = NULL;
    const glb_openflow_flow_action_t *pop_all_mpls = NULL;
    const glb_openflow_flow_action_t *set_dl_dst = NULL;
    const glb_openflow_flow_action_t *set_vlan_vid = NULL;
    const glb_openflow_flow_action_t *output = NULL;
    hsrv_ofnh_ipuc_param_t ipuc_param;
    uint16 output_gport;
    int32 ret;

    (void)pop_all_mpls;
    (void)pop_l2;
    (void)push_l2;
    /*pull all action out and allocate nexthop accordingly*/
    GLB_OPENFLOW_MPLS_DECODE_ACTION(*cur_flow_action, pop_l2, GLB_OPENFLOW_ACTION_TYPE_POP_L2);
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_ACTION(*cur_flow_action, pop_all_mpls, GLB_OPENFLOW_ACTION_TYPE_POP_ALL_MPLS);
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_PUSH_L2_OUTPUT_ACTION(*cur_flow_action,
                                                  push_l2,
                                                  set_dl_dst,
                                                  set_vlan_vid,
                                                  output);

    if(output->value.output.ifindex < GLB_BOND_IFINDEX_BASE)
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(output->value.output.ifindex, &output_gport));
    }
    else
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_agg_portid(output->value.output.ifindex,&output_gport));
    }

    sal_memset(&ipuc_param, 0, sizeof(ipuc_param));
    ipuc_param.gport = output_gport;
    sal_memcpy(&(ipuc_param.nexthop_mac), &(set_dl_dst->value.mac), sizeof(ipuc_param.nexthop_mac));

    if (set_vlan_vid)
    {
        ipuc_param.vlan_vid = set_vlan_vid->value.u16;
    }
    else
    {
        ipuc_param.vlan_vid  = GLB_MAX_VLAN_ID;    
    }
    ipuc_param.rewrite_ether_type = TRUE;

    ret = hsrv_openflow_nexthop_alloc_ipuc_nh(&ipuc_param, &p_member_nh_array[mcast_member_index]);
    if (ret)
    {
        goto alloc_error_out;
    }
    
    p_member_nh_array[mcast_member_index].nh_offset.redirect_to_ingress_port = FALSE;   
    p_member_nh_array[mcast_member_index].gport = output_gport;
    p_member_nh_array[mcast_member_index].ifindex = output->value.output.ifindex;
        
    return HSRV_E_NONE;
    
parse_error_out:
    HSRV_LOG_ERR("Failed to parse mpls action list seq pop_all_mpls_1.\n");
    return HSRV_E_OPENFLOW_MPLS_PARSE_ACTIONLIST_SEQ_FAILED;

alloc_error_out:
    HSRV_LOG_ERR("Failed to allocate mpls nexthop for mpls action list seq pop_all_mpls_1.\n");
    return HSRV_E_OPENFLOW_MPLS_ALLOC_RESOURCE_FAILED;    
}

/*GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_TAG_PACKET_IN_L2VPN_0*/
static int32
_hsrv_openflow_mpls_alloc_res_for_seq_tag_packet_in_l2vpn_0(glb_openflow_flow_action_t **cur_flow_action,
                                                                           hsrv_of_nh_info_t* p_member_nh_array,
                                                                           uint16 mcast_member_index)
{
    const glb_openflow_flow_action_t *push_l2 = NULL;
    const glb_openflow_flow_action_t *push_mpls = NULL;
    const glb_openflow_flow_action_t *set_mpls_tc = NULL;
    const glb_openflow_flow_action_t *set_mpls_ttl = NULL;
    const glb_openflow_flow_action_t *set_mpls_label = NULL;
    const glb_openflow_flow_action_t *set_dl_dst = NULL;
    const glb_openflow_flow_action_t *set_vlan_vid = NULL;
    const glb_openflow_flow_action_t *output = NULL;
    hsrv_ofnh_mpls_param_t mpls_param;
    uint16 output_gport;
    int32 ret;

    (void)push_l2;
    /*pull all action out and allocate nexthop accordingly*/
    GLB_OPENFLOW_MPLS_DECODE_PUSH_MPLS_ACTION(*cur_flow_action, 
                                 push_mpls, 
                                 set_mpls_tc, 
                                 set_mpls_ttl, 
                                 set_mpls_label);               
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_PUSH_L2_OUTPUT_ACTION(*cur_flow_action,
                                      push_l2,
                                      set_dl_dst,
                                      set_vlan_vid,
                                      output);    
    
    /* Currently we only care about label/dl_dlst/output/vlan vid, tc/ttl will use a default value. */
    if(output->value.output.ifindex < GLB_BOND_IFINDEX_BASE)
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(output->value.output.ifindex, &output_gport));
    }
    else
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_agg_portid(output->value.output.ifindex,&output_gport));
    }
    sal_memset(&mpls_param, 0, sizeof(mpls_param));
    mpls_param.gport = output_gport;
    mpls_param.type = GLB_OPENFLOW_MPLS_NEXTHOP_TYPE_PUSH_L2VPN;
    mpls_param.u.push_l2vpn.inner_svlan_tag_op = GLB_OPENFLOW_MPLS_NEXTHOP_TAG_OP_NONE;
/* modified by liwh for bug 46591 to support mpls tpoam, 2018-03-14 */
#if 0
    mpls_param.u.push_l2vpn.tunnel_label.label_flag = 0; /* no tunnel label. */
#endif
/* liwh end */

    sal_memcpy(&(mpls_param.u.push_l2vpn.nexthop_mac), &(set_dl_dst->value.mac), sizeof(mpls_param.u.push_l2vpn.nexthop_mac));
    if (set_vlan_vid)
    {
        mpls_param.u.push_l2vpn.outer_vlan_vid = set_vlan_vid->value.u16;
    }
    else
    {
        mpls_param.u.push_l2vpn.outer_vlan_vid = GLB_MAX_VLAN_ID;    
    }

    /* modified by liwh for bug 46591 to support mpls tpoam, 2018-03-14 */
    mpls_param.u.push_l2vpn.pw_label.label_flag = 0;
    mpls_param.u.push_l2vpn.tunnel_label.label_flag = GLB_OPENFLOW_MPLS_NH_LABEL_IS_VALID;
    if (HSRV_MPLS_MCAST_ETH_TYPE == push_mpls->value.u16)
    {
        mpls_param.u.push_l2vpn.tunnel_label.label_flag |= GLB_OPENFLOW_MPLS_NH_LABEL_IS_MCAST;
    }    
    mpls_param.u.push_l2vpn.tunnel_label.exp_type = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_MAP;
    if (set_mpls_tc)
    {
        mpls_param.u.push_l2vpn.tunnel_label.exp = set_mpls_tc->value.u8;
        mpls_param.u.push_l2vpn.tunnel_label.exp_type = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_ASSIGN;
    }
    else
    {    
        mpls_param.u.push_l2vpn.tunnel_label.exp = GLB_OPENFLOW_MPLS_DEFAULT_EXP;
    }
    if (set_mpls_ttl)
    {
        mpls_param.u.push_l2vpn.tunnel_label.ttl = set_mpls_ttl->value.u8;
    }
    else
    {
        mpls_param.u.push_l2vpn.tunnel_label.ttl = GLB_OPENFLOW_MPLS_DEFAULT_TTL;
    }
    mpls_param.u.push_l2vpn.tunnel_label.label = set_mpls_label->value.u32;
    /* liwh end */
  
/* modified by liwh for bug 46591 to support mpls tpoam, 2018-03-14 */
#if 0
    mpls_param.u.push_l2vpn.pw_label.label_flag = GLB_OPENFLOW_MPLS_NH_LABEL_IS_VALID;
    if (HSRV_MPLS_MCAST_ETH_TYPE == push_mpls->value.u16)
    {
        mpls_param.u.push_l2vpn.pw_label.label_flag |= GLB_OPENFLOW_MPLS_NH_LABEL_IS_MCAST;
    }
    /*added by hansf for bug46108*/
    mpls_param.u.push_l2vpn.pw_label.exp_type = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_MAP;
    if (set_mpls_tc)
    {
        mpls_param.u.push_l2vpn.pw_label.exp_type = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_ASSIGN;
        mpls_param.u.push_l2vpn.pw_label.exp = set_mpls_tc->value.u8;
    }
    else
    {    
        mpls_param.u.push_l2vpn.pw_label.exp = GLB_OPENFLOW_MPLS_DEFAULT_EXP;
    }
    if (set_mpls_ttl)
    {
        mpls_param.u.push_l2vpn.pw_label.ttl = set_mpls_ttl->value.u8;
    }
    else
    {
        mpls_param.u.push_l2vpn.pw_label.ttl = GLB_OPENFLOW_MPLS_DEFAULT_TTL;
    }
    mpls_param.u.push_l2vpn.pw_label.label = set_mpls_label->value.u32;
#endif
/* liwh end */

    ret = hsrv_openflow_nexthop_alloc_mpls_nh(&mpls_param, &p_member_nh_array[mcast_member_index]);
    if (ret == HSRV_E_OPENFLOW_MPLS_TUNNELID_RESOURCE_FULL) 
    {
        goto mpls_tunnel_id_exhausted;
    }
    else if (ret)
    {
        goto alloc_error_out;
    }
    p_member_nh_array[mcast_member_index].nh_offset.redirect_to_ingress_port = FALSE;   
    p_member_nh_array[mcast_member_index].gport = output_gport;
    p_member_nh_array[mcast_member_index].ifindex = output->value.output.ifindex;

    return HSRV_E_NONE;

parse_error_out:
    HSRV_LOG_ERR("Failed to parse mpls action list seq tag_packet_in_l2vpn_0.\n");
    return HSRV_E_OPENFLOW_MPLS_PARSE_ACTIONLIST_SEQ_FAILED;

alloc_error_out:
    HSRV_LOG_ERR("Failed to allocate mpls nexthop for mpls action list seq tag_packet_in_l2vpn_0.\n");
    return HSRV_E_OPENFLOW_MPLS_ALLOC_RESOURCE_FAILED;
    
mpls_tunnel_id_exhausted:
    /* Have warned in hsrv_openflow_nexthop_alloc_mpls_nh, no need to warn again. */
    return HSRV_E_OPENFLOW_MPLS_TUNNELID_RESOURCE_FULL;
}

/*GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_TAG_PACKET_IN_L2VPN_1*/
static int32
_hsrv_openflow_mpls_alloc_res_for_seq_tag_packet_in_l2vpn_1(glb_openflow_flow_action_t **cur_flow_action,
                                                                           hsrv_of_nh_info_t* p_member_nh_array,
                                                                           uint16 mcast_member_index)
{
    const glb_openflow_flow_action_t *pop_vlan = NULL;
    const glb_openflow_flow_action_t *push_l2 = NULL;
    const glb_openflow_flow_action_t *push_mpls = NULL;
    const glb_openflow_flow_action_t *set_mpls_tc = NULL;
    const glb_openflow_flow_action_t *set_mpls_ttl = NULL;
    const glb_openflow_flow_action_t *set_mpls_label = NULL;
    const glb_openflow_flow_action_t *set_dl_dst = NULL;
    const glb_openflow_flow_action_t *set_vlan_vid = NULL;
    const glb_openflow_flow_action_t *output = NULL;
    hsrv_ofnh_mpls_param_t mpls_param;
    uint16 output_gport;
    int32 ret;

    (void)push_l2;
    (void)pop_vlan;
    /*pull all action out and allocate nexthop accordingly*/
    GLB_OPENFLOW_MPLS_DECODE_ACTION(*cur_flow_action, pop_vlan, GLB_OPENFLOW_ACTION_TYPE_STRIP_VLAN);
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_PUSH_MPLS_ACTION(*cur_flow_action, 
                                 push_mpls, 
                                 set_mpls_tc, 
                                 set_mpls_ttl, 
                                 set_mpls_label);               
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_PUSH_L2_OUTPUT_ACTION(*cur_flow_action,
                                      push_l2,
                                      set_dl_dst,
                                      set_vlan_vid,
                                      output);    
    
    /* Currently we only care about label/dl_dlst/output/vlan vid, tc/ttl will use a default value. */
    if(output->value.output.ifindex < GLB_BOND_IFINDEX_BASE)
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(output->value.output.ifindex, &output_gport));
    }
    else
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_agg_portid(output->value.output.ifindex,&output_gport));
    }
    sal_memset(&mpls_param, 0, sizeof(mpls_param));
    mpls_param.gport = output_gport;
    mpls_param.type = GLB_OPENFLOW_MPLS_NEXTHOP_TYPE_PUSH_L2VPN;
    mpls_param.u.push_l2vpn.inner_svlan_tag_op = GLB_OPENFLOW_MPLS_NEXTHOP_TAG_OP_STRIP;
    mpls_param.u.push_l2vpn.tunnel_label.label_flag = 0; /* no tunnel label. */
    sal_memcpy(&(mpls_param.u.push_l2vpn.nexthop_mac), &(set_dl_dst->value.mac), sizeof(mpls_param.u.push_l2vpn.nexthop_mac));
    if (set_vlan_vid)
    {
        mpls_param.u.push_l2vpn.outer_vlan_vid = set_vlan_vid->value.u16;
    }
    else
    {
        mpls_param.u.push_l2vpn.outer_vlan_vid = GLB_MAX_VLAN_ID;    
    }
    mpls_param.u.push_l2vpn.pw_label.label_flag = GLB_OPENFLOW_MPLS_NH_LABEL_IS_VALID;
    if (HSRV_MPLS_MCAST_ETH_TYPE == push_mpls->value.u16)
    {
        mpls_param.u.push_l2vpn.pw_label.label_flag |= GLB_OPENFLOW_MPLS_NH_LABEL_IS_MCAST;
    }    
    mpls_param.u.push_l2vpn.pw_label.exp_type = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_ASSIGN;
    if (set_mpls_tc)
    {
        mpls_param.u.push_l2vpn.pw_label.exp = set_mpls_tc->value.u8;
    }
    else
    {    
        mpls_param.u.push_l2vpn.pw_label.exp = GLB_OPENFLOW_MPLS_DEFAULT_EXP;
    }
    if (set_mpls_ttl)
    {
        mpls_param.u.push_l2vpn.pw_label.ttl = set_mpls_ttl->value.u8;
    }
    else
    {
        mpls_param.u.push_l2vpn.pw_label.ttl = GLB_OPENFLOW_MPLS_DEFAULT_TTL;
    }
    mpls_param.u.push_l2vpn.pw_label.label = set_mpls_label->value.u32;
    ret = hsrv_openflow_nexthop_alloc_mpls_nh(&mpls_param, &p_member_nh_array[mcast_member_index]);
    if (ret == HSRV_E_OPENFLOW_MPLS_TUNNELID_RESOURCE_FULL) 
    {
        goto mpls_tunnel_id_exhausted;
    }
    else if (ret)
    {
        goto alloc_error_out;
    }
    p_member_nh_array[mcast_member_index].nh_offset.redirect_to_ingress_port = FALSE;   
    p_member_nh_array[mcast_member_index].gport = output_gport;
    p_member_nh_array[mcast_member_index].ifindex = output->value.output.ifindex;

    return HSRV_E_NONE;

parse_error_out:
    HSRV_LOG_ERR("Failed to parse mpls action list seq tag_packet_in_l2vpn_1.\n");
    return HSRV_E_OPENFLOW_MPLS_PARSE_ACTIONLIST_SEQ_FAILED;

alloc_error_out:
    HSRV_LOG_ERR("Failed to allocate mpls nexthop for mpls action list seq tag_packet_in_l2vpn_1.\n");
    return HSRV_E_OPENFLOW_MPLS_ALLOC_RESOURCE_FAILED;
    
mpls_tunnel_id_exhausted:
    /* Have warned in hsrv_openflow_nexthop_alloc_mpls_nh, no need to warn again. */
    return HSRV_E_OPENFLOW_MPLS_TUNNELID_RESOURCE_FULL;
}


/*GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_NORMAL_PUSH_0*/
static int32
_hsrv_openflow_mpls_alloc_res_for_seq_normal_push_0(glb_openflow_flow_action_t **cur_flow_action,
                                                                   hsrv_of_nh_info_t* p_member_nh_array,
                                                                   uint16 mcast_member_index)
{
    const glb_openflow_flow_action_t *push_l2 = NULL;
    const glb_openflow_flow_action_t *pop_l2 = NULL;
    const glb_openflow_flow_action_t *push_mpls = NULL;
    const glb_openflow_flow_action_t *set_mpls_tc = NULL;
    const glb_openflow_flow_action_t *set_mpls_ttl = NULL;
    const glb_openflow_flow_action_t *set_mpls_label = NULL;
    const glb_openflow_flow_action_t *set_dl_dst = NULL;
    const glb_openflow_flow_action_t *set_vlan_vid = NULL;
    const glb_openflow_flow_action_t *output = NULL;
    hsrv_ofnh_mpls_param_t mpls_param;
    hsrv_ofnh_ipuc_param_t ipuc_param;    
    uint16 output_gport;
    int32 ret;

    (void)pop_l2;
    (void)push_l2;
    /*pull all action out and allocate nexthop accordingly*/
    GLB_OPENFLOW_MPLS_DECODE_ACTION(*cur_flow_action, pop_l2, GLB_OPENFLOW_ACTION_TYPE_POP_L2);
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_PUSH_MPLS_ACTION(*cur_flow_action, 
                                             push_mpls, 
                                             set_mpls_tc, 
                                             set_mpls_ttl, 
                                             set_mpls_label);               
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_PUSH_L2_OUTPUT_ACTION(*cur_flow_action,
                                                  push_l2,
                                                  set_dl_dst,
                                                  set_vlan_vid,
                                                  output);

    if(output->value.output.ifindex < GLB_BOND_IFINDEX_BASE)
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(output->value.output.ifindex, &output_gport));
    }
    else
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_agg_portid(output->value.output.ifindex,&output_gport));
    }
    /*added by hansf for bug 43683*/
   // if (GLB_OPF_MPLS_IPV4_IMPLICIT_NULL != set_mpls_label->value.u32)
    if (set_mpls_label->value.u32)
    {
        sal_memset(&mpls_param, 0, sizeof(mpls_param));
        mpls_param.gport = output_gport;
        mpls_param.type = GLB_OPENFLOW_MPLS_NEXTHOP_TYPE_PUSH_L3VPN;

        mpls_param.u.push_l3vpn.tunnel_label.label_flag = GLB_OPENFLOW_MPLS_NH_LABEL_IS_VALID;
        if (HSRV_MPLS_MCAST_ETH_TYPE == push_mpls->value.u16)
        {
            mpls_param.u.push_l3vpn.tunnel_label.label_flag |= GLB_OPENFLOW_MPLS_NH_LABEL_IS_MCAST;
        }    
        mpls_param.u.push_l3vpn.tunnel_label.label_flag |= GLB_OPENFLOW_MPLS_NH_LABEL_MAP_TTL;
        mpls_param.u.push_l3vpn.tunnel_label.exp_type  = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_MAP;
        sal_memcpy(&(mpls_param.u.push_l3vpn.nexthop_mac), &(set_dl_dst->value.mac), sizeof(mpls_param.u.push_l3vpn.nexthop_mac));
        if (set_vlan_vid)
        {
            mpls_param.u.push_l3vpn.outer_vlan_vid = set_vlan_vid->value.u16;
        }
        else
        {
            mpls_param.u.push_l3vpn.outer_vlan_vid = GLB_MAX_VLAN_ID;    
        }

        if (set_mpls_tc)
        {
            mpls_param.u.push_l3vpn.tunnel_label.exp = set_mpls_tc->value.u8;
            mpls_param.u.push_l3vpn.tunnel_label.exp_type  = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_ASSIGN;
        }

        if (set_mpls_ttl)
        {
            mpls_param.u.push_l3vpn.tunnel_label.ttl = set_mpls_ttl->value.u8;
            mpls_param.u.push_l3vpn.tunnel_label.label_flag &= ~GLB_OPENFLOW_MPLS_NH_LABEL_MAP_TTL;
        }

        mpls_param.u.push_l3vpn.tunnel_label.label = set_mpls_label->value.u32;
        
        ret = hsrv_openflow_nexthop_alloc_mpls_nh(&mpls_param, &p_member_nh_array[mcast_member_index]);
        if (ret == HSRV_E_OPENFLOW_MPLS_TUNNELID_RESOURCE_FULL) 
        {
            goto mpls_tunnel_id_exhausted;
        }
        else if (ret)
        {
            goto alloc_error_out;
        }
    }
    else
    {
        sal_memset(&ipuc_param, 0, sizeof(ipuc_param));
        ipuc_param.gport = output_gport;
        sal_memcpy(&(ipuc_param.nexthop_mac), &(set_dl_dst->value.mac), sizeof(ipuc_param.nexthop_mac));

        if (set_vlan_vid)
        {
            ipuc_param.vlan_vid = set_vlan_vid->value.u16;
        }
        else
        {
            ipuc_param.vlan_vid  = GLB_MAX_VLAN_ID;    
        }

        ret = hsrv_openflow_nexthop_alloc_ipuc_nh(&ipuc_param, &p_member_nh_array[mcast_member_index]);
        if (ret)
        {
            goto alloc_error_out;
        }
    }
    
    p_member_nh_array[mcast_member_index].nh_offset.redirect_to_ingress_port = FALSE;   
    p_member_nh_array[mcast_member_index].gport = output_gport;
    p_member_nh_array[mcast_member_index].ifindex = output->value.output.ifindex;
        
    return HSRV_E_NONE;
    
parse_error_out:
    HSRV_LOG_ERR("Failed to parse mpls action list seq normal_push_0.\n");
    return HSRV_E_OPENFLOW_MPLS_PARSE_ACTIONLIST_SEQ_FAILED;

alloc_error_out:
    HSRV_LOG_ERR("Failed to allocate mpls nexthop for mpls action list seq normal_push_0.\n");
    return HSRV_E_OPENFLOW_MPLS_ALLOC_RESOURCE_FAILED;
    
mpls_tunnel_id_exhausted:
    /* Have warned in hsrv_openflow_nexthop_alloc_mpls_nh, no need to warn again. */
    return HSRV_E_OPENFLOW_MPLS_TUNNELID_RESOURCE_FULL;
}

/*GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_NORMAL_SWAP_0*/
static int32
_hsrv_openflow_mpls_alloc_res_for_seq_normal_swap_0(glb_openflow_flow_action_t **cur_flow_action,
                                                                   hsrv_of_nh_info_t* p_member_nh_array,
                                                                   uint16 mcast_member_index)
{
    const glb_openflow_flow_action_t *push_l2 = NULL;
    const glb_openflow_flow_action_t *pop_l2 = NULL;
    const glb_openflow_flow_action_t *push_mpls = NULL;
    const glb_openflow_flow_action_t *pop_mpls = NULL;
    const glb_openflow_flow_action_t *set_mpls_tc = NULL;
    const glb_openflow_flow_action_t *set_mpls_ttl = NULL;
    const glb_openflow_flow_action_t *set_mpls_label = NULL;
    const glb_openflow_flow_action_t *set_dl_dst = NULL;
    const glb_openflow_flow_action_t *set_vlan_vid = NULL;
    const glb_openflow_flow_action_t *output = NULL;
    hsrv_ofnh_mpls_param_t mpls_param;
    uint16 output_gport;
    int32 ret;
    
    (void)push_l2;
    (void)pop_l2;
    (void)pop_mpls;
    /*pull all action out and allocate nexthop accordingly*/
    GLB_OPENFLOW_MPLS_DECODE_ACTION(*cur_flow_action, pop_l2, GLB_OPENFLOW_ACTION_TYPE_POP_L2);
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_ACTION(*cur_flow_action, pop_mpls, GLB_OPENFLOW_ACTION_TYPE_POP_MPLS);
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_PUSH_MPLS_ACTION(*cur_flow_action, 
                                             push_mpls, 
                                             set_mpls_tc, 
                                             set_mpls_ttl, 
                                             set_mpls_label);               
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_PUSH_L2_OUTPUT_ACTION(*cur_flow_action,
                                                  push_l2,
                                                  set_dl_dst,
                                                  set_vlan_vid,
                                                  output);

    if(output->value.output.ifindex < GLB_BOND_IFINDEX_BASE)
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(output->value.output.ifindex, &output_gport));
    }
    else
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_agg_portid(output->value.output.ifindex,&output_gport));
    }
    /*added by hansf for bug 43683*/
    //if (GLB_OPF_MPLS_IPV4_IMPLICIT_NULL != set_mpls_label->value.u32)
    if (set_mpls_label->value.u32)
    {
        sal_memset(&mpls_param, 0, sizeof(mpls_param));
        mpls_param.gport = output_gport;
        mpls_param.type = GLB_OPENFLOW_MPLS_NEXTHOP_TYPE_SWAP_MPLS;

        mpls_param.u.swap_mpls.tunnel_label.label_flag = GLB_OPENFLOW_MPLS_NH_LABEL_IS_VALID;
        if (HSRV_MPLS_MCAST_ETH_TYPE == push_mpls->value.u16)
        {
            mpls_param.u.swap_mpls.tunnel_label.label_flag |= GLB_OPENFLOW_MPLS_NH_LABEL_IS_MCAST;
        }    
        mpls_param.u.swap_mpls.tunnel_label.label_flag |= GLB_OPENFLOW_MPLS_NH_LABEL_MAP_TTL;
        mpls_param.u.swap_mpls.tunnel_label.exp_type  = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_MAP;
        sal_memcpy(&(mpls_param.u.swap_mpls.nexthop_mac), &(set_dl_dst->value.mac), sizeof(mpls_param.u.swap_mpls.nexthop_mac));
        if (set_vlan_vid)
        {
            mpls_param.u.swap_mpls.outer_vlan_vid = set_vlan_vid->value.u16;
        }
        else
        {
            mpls_param.u.swap_mpls.outer_vlan_vid = GLB_MAX_VLAN_ID;    
        }

        if (set_mpls_tc)
        {
            mpls_param.u.swap_mpls.tunnel_label.exp = set_mpls_tc->value.u8;
            mpls_param.u.swap_mpls.tunnel_label.exp_type  = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_ASSIGN;
        }

        /*the 1 here is used to calculate the new ttl of swap label, the result is (ttl_of_orignal_label - 1)*/
        mpls_param.u.swap_mpls.tunnel_label.ttl = 1;
        if (set_mpls_ttl)
        {
            mpls_param.u.swap_mpls.tunnel_label.ttl = set_mpls_ttl->value.u8;
            mpls_param.u.swap_mpls.tunnel_label.label_flag &= ~GLB_OPENFLOW_MPLS_NH_LABEL_MAP_TTL;
        }

        mpls_param.u.swap_mpls.tunnel_label.label = set_mpls_label->value.u32;
        
        ret = hsrv_openflow_nexthop_alloc_mpls_nh(&mpls_param, &p_member_nh_array[mcast_member_index]);
        if (ret == HSRV_E_OPENFLOW_MPLS_TUNNELID_RESOURCE_FULL) 
        {
            goto mpls_tunnel_id_exhausted;
        }
        else if (ret)
        {
            goto alloc_error_out;
        }
    }
    else
    {
        sal_memset(&mpls_param, 0, sizeof(mpls_param));
        mpls_param.gport = output_gport;
        mpls_param.type = GLB_OPENFLOW_MPLS_NEXTHOP_TYPE_POP_PHP;

        sal_memcpy(&(mpls_param.u.pop_php.nexthop_mac), &(set_dl_dst->value.mac), sizeof(mpls_param.u.pop_php.nexthop_mac));
        if (set_vlan_vid)
        {
            mpls_param.u.pop_php.outer_vlan_vid = set_vlan_vid->value.u16;
        }
        else
        {
            mpls_param.u.pop_php.outer_vlan_vid = GLB_MAX_VLAN_ID;    
        }
        
        ret = hsrv_openflow_nexthop_alloc_mpls_nh(&mpls_param, &p_member_nh_array[mcast_member_index]);
        if (ret)
        {
            goto alloc_error_out;
        }
    }

    p_member_nh_array[mcast_member_index].nh_offset.redirect_to_ingress_port = FALSE;   
    p_member_nh_array[mcast_member_index].gport = output_gport;
    p_member_nh_array[mcast_member_index].ifindex = output->value.output.ifindex;
        
    return HSRV_E_NONE;
    
parse_error_out:
    HSRV_LOG_ERR("Failed to parse mpls action list seq normal_swap_0.\n");
    return HSRV_E_OPENFLOW_MPLS_PARSE_ACTIONLIST_SEQ_FAILED;

alloc_error_out:
    HSRV_LOG_ERR("Failed to allocate mpls nexthop for mpls action list seq normal_swap_0.\n");
    return HSRV_E_OPENFLOW_MPLS_ALLOC_RESOURCE_FAILED;
    
mpls_tunnel_id_exhausted:
    /* Have warned in hsrv_openflow_nexthop_alloc_mpls_nh, no need to warn again. */
    return HSRV_E_OPENFLOW_MPLS_TUNNELID_RESOURCE_FULL;
}

/*GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_NORMAL_SWAP_1*/
static int32
_hsrv_openflow_mpls_alloc_res_for_seq_normal_swap_1(glb_openflow_flow_action_t **cur_flow_action,
                                                                   hsrv_of_nh_info_t* p_member_nh_array,
                                                                   uint16 mcast_member_index)
{
    const glb_openflow_flow_action_t *push_l2 = NULL;
    const glb_openflow_flow_action_t *pop_l2 = NULL;
    const glb_openflow_flow_action_t *push_mpls_1 = NULL;
    const glb_openflow_flow_action_t *pop_mpls = NULL;
    const glb_openflow_flow_action_t *set_mpls_tc_1 = NULL;
    const glb_openflow_flow_action_t *set_mpls_ttl_1 = NULL;
    const glb_openflow_flow_action_t *set_mpls_label_1 = NULL;
    const glb_openflow_flow_action_t *push_mpls_2 = NULL;
    const glb_openflow_flow_action_t *set_mpls_tc_2 = NULL;
    const glb_openflow_flow_action_t *set_mpls_ttl_2 = NULL;
    const glb_openflow_flow_action_t *set_mpls_label_2 = NULL;
    const glb_openflow_flow_action_t *set_dl_dst = NULL;
    const glb_openflow_flow_action_t *set_vlan_vid = NULL;
    const glb_openflow_flow_action_t *output = NULL;
    hsrv_ofnh_mpls_param_t mpls_param;
    uint16 output_gport;
    int32 ret;

    (void)pop_mpls;
    (void)pop_l2;
    (void)push_l2;
    /*pull all action out and allocate nexthop accordingly*/
    GLB_OPENFLOW_MPLS_DECODE_ACTION(*cur_flow_action, pop_l2, GLB_OPENFLOW_ACTION_TYPE_POP_L2);
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_ACTION(*cur_flow_action, pop_mpls, GLB_OPENFLOW_ACTION_TYPE_POP_MPLS);
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_PUSH_MPLS_ACTION(*cur_flow_action, 
                                             push_mpls_1, 
                                             set_mpls_tc_1, 
                                             set_mpls_ttl_1, 
                                             set_mpls_label_1);               
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_PUSH_MPLS_ACTION(*cur_flow_action, 
                                             push_mpls_2, 
                                             set_mpls_tc_2, 
                                             set_mpls_ttl_2, 
                                             set_mpls_label_2);               
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_PUSH_L2_OUTPUT_ACTION(*cur_flow_action,
                                                  push_l2,
                                                  set_dl_dst,
                                                  set_vlan_vid,
                                                  output);

    if(output->value.output.ifindex < GLB_BOND_IFINDEX_BASE)
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(output->value.output.ifindex, &output_gport));
    }
    else
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_agg_portid(output->value.output.ifindex,&output_gport));
    }
    /*added by hansf for bug 43683*/
    if (GLB_OPF_MPLS_IPV4_IMPLICIT_NULL == set_mpls_label_2->value.u32)
    {
        sal_memset(&mpls_param, 0, sizeof(mpls_param));
        mpls_param.gport = output_gport;
        mpls_param.type = GLB_OPENFLOW_MPLS_NEXTHOP_TYPE_SWAP_MPLS;

        mpls_param.u.swap_mpls.tunnel_label.label_flag = GLB_OPENFLOW_MPLS_NH_LABEL_IS_VALID;
        if (HSRV_MPLS_MCAST_ETH_TYPE == push_mpls_1->value.u16)
        {
            mpls_param.u.swap_mpls.tunnel_label.label_flag |= GLB_OPENFLOW_MPLS_NH_LABEL_IS_MCAST;
        }    
        mpls_param.u.swap_mpls.tunnel_label.label_flag |= GLB_OPENFLOW_MPLS_NH_LABEL_MAP_TTL;
        mpls_param.u.swap_mpls.tunnel_label.exp_type  = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_MAP;
        sal_memcpy(&(mpls_param.u.swap_mpls.nexthop_mac), &(set_dl_dst->value.mac), sizeof(mpls_param.u.swap_mpls.nexthop_mac));
        if (set_vlan_vid)
        {
            mpls_param.u.swap_mpls.outer_vlan_vid = set_vlan_vid->value.u16;
        }
        else
        {
            mpls_param.u.swap_mpls.outer_vlan_vid = GLB_MAX_VLAN_ID;    
        }

        if (set_mpls_tc_1)
        {
            mpls_param.u.swap_mpls.tunnel_label.exp = set_mpls_tc_1->value.u8;
            mpls_param.u.swap_mpls.tunnel_label.exp_type  = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_ASSIGN;
        }

        /*the 1 here is used to calculate the new ttl of swap label, the result is (ttl_of_orignal_label - 1)*/
        mpls_param.u.swap_mpls.tunnel_label.ttl = 1;
        if (set_mpls_ttl_1)
        {
            mpls_param.u.swap_mpls.tunnel_label.ttl = set_mpls_ttl_1->value.u8;
            mpls_param.u.swap_mpls.tunnel_label.label_flag &= ~GLB_OPENFLOW_MPLS_NH_LABEL_MAP_TTL;
        }

        mpls_param.u.swap_mpls.tunnel_label.label = set_mpls_label_1->value.u32;
        
        ret = hsrv_openflow_nexthop_alloc_mpls_nh(&mpls_param, &p_member_nh_array[mcast_member_index]);
        if (ret == HSRV_E_OPENFLOW_MPLS_TUNNELID_RESOURCE_FULL) 
        {
            goto mpls_tunnel_id_exhausted;
        }
        else if (ret)
        {
            goto alloc_error_out;
        }
    }
    else
    {
        sal_memset(&mpls_param, 0, sizeof(mpls_param));
        mpls_param.gport = output_gport;
        mpls_param.type = GLB_OPENFLOW_MPLS_NEXTHOP_TYPE_SWAP_MPLS;

        mpls_param.u.swap_mpls.tunnel_label.label_flag = GLB_OPENFLOW_MPLS_NH_LABEL_IS_VALID;
        if (HSRV_MPLS_MCAST_ETH_TYPE == push_mpls_2->value.u16)
        {
            mpls_param.u.swap_mpls.tunnel_label.label_flag |= GLB_OPENFLOW_MPLS_NH_LABEL_IS_MCAST;
        }    
        //mpls_param.u.swap_mpls.tunnel_label.label_flag |= GLB_OPENFLOW_MPLS_NH_LABEL_MAP_TTL;
        //mpls_param.u.swap_mpls.tunnel_label.exp_type  = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_PACKET;
        sal_memcpy(&(mpls_param.u.swap_mpls.nexthop_mac), &(set_dl_dst->value.mac), sizeof(mpls_param.u.swap_mpls.nexthop_mac));
        if (set_vlan_vid)
        {
            mpls_param.u.swap_mpls.outer_vlan_vid = set_vlan_vid->value.u16;
        }
        else
        {
            mpls_param.u.swap_mpls.outer_vlan_vid = GLB_MAX_VLAN_ID;    
        }

        if (set_mpls_tc_2)
        {
            mpls_param.u.swap_mpls.tunnel_label.exp = set_mpls_tc_2->value.u8;
            mpls_param.u.swap_mpls.tunnel_label.exp_type  = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_ASSIGN;
        }
        else
        {
            mpls_param.u.swap_mpls.tunnel_label.exp = GLB_OPENFLOW_MPLS_DEFAULT_EXP;
            mpls_param.u.swap_mpls.tunnel_label.exp_type  = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_MAP;
        }

        /*the 1 here is used to calculate the new ttl of swap label, the result is (ttl_of_orignal_label - 1)*/
        //mpls_param.u.swap_mpls.tunnel_label.ttl = 1;
        if (set_mpls_ttl_2)
        {
            mpls_param.u.swap_mpls.tunnel_label.ttl = set_mpls_ttl_2->value.u8;
            //mpls_param.u.swap_mpls.tunnel_label.label_flag &= ~GLB_OPENFLOW_MPLS_NH_LABEL_MAP_TTL;
        }
        else
        {
            mpls_param.u.swap_mpls.tunnel_label.ttl = GLB_OPENFLOW_MPLS_DEFAULT_TTL;
            //mpls_param.u.swap_mpls.tunnel_label.label_flag &= ~GLB_OPENFLOW_MPLS_NH_LABEL_MAP_TTL;
        }

        mpls_param.u.swap_mpls.tunnel_label.label = set_mpls_label_2->value.u32;


        mpls_param.u.swap_mpls.pw_label.label_flag = GLB_OPENFLOW_MPLS_NH_LABEL_IS_VALID;
        if (HSRV_MPLS_MCAST_ETH_TYPE == push_mpls_1->value.u16)
        {
            mpls_param.u.swap_mpls.pw_label.label_flag |= GLB_OPENFLOW_MPLS_NH_LABEL_IS_MCAST;
        }    
        mpls_param.u.swap_mpls.pw_label.label_flag |= GLB_OPENFLOW_MPLS_NH_LABEL_MAP_TTL;
        mpls_param.u.swap_mpls.pw_label.exp_type  = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_MAP;

        if (set_mpls_tc_1)
        {
            mpls_param.u.swap_mpls.pw_label.exp = set_mpls_tc_1->value.u8;
            mpls_param.u.swap_mpls.pw_label.exp_type  = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_ASSIGN;
        }

        /*the 1 here is used to calculate the new ttl of swap label, the result is (ttl_of_orignal_label - 1)*/
        mpls_param.u.swap_mpls.pw_label.ttl = 1;
        if (set_mpls_ttl_1)
        {
            mpls_param.u.swap_mpls.pw_label.ttl = set_mpls_ttl_1->value.u8;
            mpls_param.u.swap_mpls.pw_label.label_flag &= ~GLB_OPENFLOW_MPLS_NH_LABEL_MAP_TTL;
        }

        mpls_param.u.swap_mpls.pw_label.label = set_mpls_label_1->value.u32;
        
        ret = hsrv_openflow_nexthop_alloc_mpls_nh(&mpls_param, &p_member_nh_array[mcast_member_index]);
        if (ret == HSRV_E_OPENFLOW_MPLS_TUNNELID_RESOURCE_FULL) 
        {
            goto mpls_tunnel_id_exhausted;
        }
        else if (ret)
        {
            goto alloc_error_out;
        }
    }

    p_member_nh_array[mcast_member_index].nh_offset.redirect_to_ingress_port = FALSE;   
    p_member_nh_array[mcast_member_index].gport = output_gport;
    p_member_nh_array[mcast_member_index].ifindex = output->value.output.ifindex;
        
    return HSRV_E_NONE;
    
parse_error_out:
    HSRV_LOG_ERR("Failed to parse mpls action list seq normal_swap_0.\n");
    return HSRV_E_OPENFLOW_MPLS_PARSE_ACTIONLIST_SEQ_FAILED;

alloc_error_out:
    HSRV_LOG_ERR("Failed to allocate mpls nexthop for mpls action list seq normal_swap_0.\n");
    return HSRV_E_OPENFLOW_MPLS_ALLOC_RESOURCE_FAILED;
    
mpls_tunnel_id_exhausted:
    /* Have warned in hsrv_openflow_nexthop_alloc_mpls_nh, no need to warn again. */
    return HSRV_E_OPENFLOW_MPLS_TUNNELID_RESOURCE_FULL;
}

/*GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_NORMAL_POP_0*/
static int32
_hsrv_openflow_mpls_alloc_res_for_seq_normal_pop_0(glb_openflow_flow_action_t **cur_flow_action,
                                                                   hsrv_of_nh_info_t* p_member_nh_array,
                                                                   uint16 mcast_member_index)
{
    const glb_openflow_flow_action_t *pop_mpls = NULL;
    const glb_openflow_flow_action_t *output = NULL;
    const glb_openflow_flow_action_t *set_vlan_vid = NULL;
    const glb_openflow_flow_action_t *set_dl_dst = NULL;
    const glb_openflow_flow_action_t *pop_l2 = NULL;
    const glb_openflow_flow_action_t *push_l2 = NULL;
    
    hsrv_ofnh_mpls_param_t mpls_param;
    uint16 output_gport;
    int32 ret;
    
    (void)pop_mpls;
    (void)pop_l2;
    (void)push_l2;
    /*pull all action out and allocate nexthop accordingly*/
    GLB_OPENFLOW_MPLS_DECODE_ACTION(*cur_flow_action, pop_l2, GLB_OPENFLOW_ACTION_TYPE_POP_L2);
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_ACTION(*cur_flow_action, pop_mpls, GLB_OPENFLOW_ACTION_TYPE_POP_MPLS);
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_PUSH_L2_OUTPUT_ACTION(*cur_flow_action,
                                          push_l2,
                                          set_dl_dst,
                                          set_vlan_vid,
                                          output);

    if(output->value.output.ifindex < GLB_BOND_IFINDEX_BASE)
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(output->value.output.ifindex, &output_gport));
    }
    else
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_agg_portid(output->value.output.ifindex,&output_gport));
    }
    sal_memset(&mpls_param, 0, sizeof(mpls_param));
    mpls_param.gport = output_gport;
    mpls_param.type = GLB_OPENFLOW_MPLS_NEXTHOP_TYPE_POP_PHP;

    sal_memcpy(&(mpls_param.u.pop_php.nexthop_mac), &(set_dl_dst->value.mac), sizeof(mpls_param.u.pop_php.nexthop_mac));
    if (set_vlan_vid)
    {
        mpls_param.u.pop_php.outer_vlan_vid = set_vlan_vid->value.u16;
    }
    else
    {
        mpls_param.u.pop_php.outer_vlan_vid = GLB_MAX_VLAN_ID;    
    }
    
    ret = hsrv_openflow_nexthop_alloc_mpls_nh(&mpls_param, &p_member_nh_array[mcast_member_index]);
    if (ret)
    {
        goto alloc_error_out;
    }

    p_member_nh_array[mcast_member_index].nh_offset.redirect_to_ingress_port = FALSE;   
    p_member_nh_array[mcast_member_index].gport = output_gport;
    p_member_nh_array[mcast_member_index].ifindex = output->value.output.ifindex;
        
    return HSRV_E_NONE;
    
parse_error_out:
    HSRV_LOG_ERR("Failed to parse mpls action list seq normal_pop_0.\n");
    return HSRV_E_OPENFLOW_MPLS_PARSE_ACTIONLIST_SEQ_FAILED;

alloc_error_out:
    HSRV_LOG_ERR("Failed to allocate mpls nexthop for mpls action list seq normal_pop_0.\n");
    return HSRV_E_OPENFLOW_MPLS_ALLOC_RESOURCE_FAILED;
}

/*GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_NORMAL_POP_0*/
static int32
_hsrv_openflow_mpls_alloc_res_for_seq_normal_pop_1(glb_openflow_flow_action_t **cur_flow_action,
                                                                   hsrv_of_nh_info_t* p_member_nh_array,
                                                                   uint16 mcast_member_index)
{
    const glb_openflow_flow_action_t *pop_mpls = NULL;
    const glb_openflow_flow_action_t *output = NULL;
    const glb_openflow_flow_action_t *pop_l2 = NULL;
    uint32 nh_id;

    (void)pop_mpls;
    (void)pop_l2;
    (void)output;
    //hsrv_nh_get_default_drop(&nh_id);
    nh_id = HSRV_OF_NH_RESV_NHID_FOR_DROP;
    /*pull all action out and allocate nexthop accordingly*/
    GLB_OPENFLOW_MPLS_DECODE_ACTION(*cur_flow_action, pop_l2, GLB_OPENFLOW_ACTION_TYPE_POP_L2);
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_ACTION(*cur_flow_action, pop_mpls, GLB_OPENFLOW_ACTION_TYPE_POP_MPLS);
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_ACTION(*cur_flow_action, output, GLB_OPENFLOW_ACTION_TYPE_OUTPUT);

    p_member_nh_array[mcast_member_index].nh_offset.nh_type = HSRV_OF_NH_TYPE_MAX;
    p_member_nh_array[mcast_member_index].nh_offset.nhid = nh_id;
    p_member_nh_array[mcast_member_index].nh_offset.offset = 0;

    return HSRV_E_NONE;
    
parse_error_out:
    HSRV_LOG_ERR("Failed to parse mpls action list seq normal_pop_1.\n");
    return HSRV_E_OPENFLOW_MPLS_PARSE_ACTIONLIST_SEQ_FAILED;

}

/*GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_L2VPN_INGRESS_0*/
static int32
_hsrv_openflow_mpls_alloc_res_for_seq_l2vpn_ingress_0(glb_openflow_flow_action_t **cur_flow_action,
                                                                   hsrv_of_nh_info_t* p_member_nh_array,
                                                                   uint16 mcast_member_index)
{
    const glb_openflow_flow_action_t *pop_vlan = NULL;
    const glb_openflow_flow_action_t *push_mpls_1 = NULL;
    const glb_openflow_flow_action_t  *set_mpls_tc_1 = NULL;
    const glb_openflow_flow_action_t  *set_mpls_ttl_1 = NULL;
    const glb_openflow_flow_action_t *set_mpls_label_1 = NULL;
    const glb_openflow_flow_action_t *push_mpls_2 = NULL;
    const glb_openflow_flow_action_t  *set_mpls_tc_2 = NULL;
    const glb_openflow_flow_action_t  *set_mpls_ttl_2 = NULL;
    const glb_openflow_flow_action_t *set_mpls_label_2 = NULL;
    const glb_openflow_flow_action_t *output = NULL;
    const glb_openflow_flow_action_t *set_dl_dst = NULL;
    const glb_openflow_flow_action_t *set_vlan_vid = NULL;
    const glb_openflow_flow_action_t *push_l2 = NULL;
    
    hsrv_ofnh_mpls_param_t mpls_param;
    uint16 output_gport;
    int32 ret;

    (void)push_l2;
    (void)pop_vlan;
    /*pull all action out and allocate nexthop accordingly*/
    GLB_OPENFLOW_MPLS_DECODE_ACTION(*cur_flow_action, pop_vlan, GLB_OPENFLOW_ACTION_TYPE_STRIP_VLAN);
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_PUSH_MPLS_ACTION(*cur_flow_action, push_mpls_1, set_mpls_tc_1, set_mpls_ttl_1, set_mpls_label_1);               
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_PUSH_MPLS_ACTION(*cur_flow_action, push_mpls_2, set_mpls_tc_2, set_mpls_ttl_2, set_mpls_label_2);               
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_PUSH_L2_OUTPUT_ACTION(*cur_flow_action,
                                                  push_l2,
                                                  set_dl_dst,
                                                  set_vlan_vid,
                                                  output);
    
    if(output->value.output.ifindex < GLB_BOND_IFINDEX_BASE)
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(output->value.output.ifindex, &output_gport));
    }
    else
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_agg_portid(output->value.output.ifindex,&output_gport));
    }
    sal_memset(&mpls_param, 0, sizeof(mpls_param));
    mpls_param.gport = output_gport;
    mpls_param.type = GLB_OPENFLOW_MPLS_NEXTHOP_TYPE_PUSH_L2VPN;
    mpls_param.u.push_l2vpn.inner_svlan_tag_op = GLB_OPENFLOW_MPLS_NEXTHOP_TAG_OP_STRIP;
    
    mpls_param.u.push_l2vpn.tunnel_label.label_flag = 0; /* no tunnel label. */
    /*added by hansf for bug 43683*/
    //if (GLB_OPF_MPLS_IPV4_IMPLICIT_NULL != set_mpls_label_2->value.u32)
    if (set_mpls_label_2->value.u32)
    {
        mpls_param.u.push_l2vpn.tunnel_label.label_flag = GLB_OPENFLOW_MPLS_NH_LABEL_IS_VALID;
        if (HSRV_MPLS_MCAST_ETH_TYPE == push_mpls_2->value.u16)
        {
            mpls_param.u.push_l2vpn.tunnel_label.label_flag |= GLB_OPENFLOW_MPLS_NH_LABEL_IS_MCAST;
        }    
        mpls_param.u.push_l2vpn.tunnel_label.exp_type = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_MAP;
        if (set_mpls_tc_2)
        {
            mpls_param.u.push_l2vpn.tunnel_label.exp = set_mpls_tc_2->value.u8;
            mpls_param.u.push_l2vpn.tunnel_label.exp_type = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_ASSIGN;
        }
        else
        {    
            mpls_param.u.push_l2vpn.tunnel_label.exp = GLB_OPENFLOW_MPLS_DEFAULT_EXP;
        }
        if (set_mpls_ttl_2)
        {
            mpls_param.u.push_l2vpn.tunnel_label.ttl = set_mpls_ttl_2->value.u8;
        }
        else
        {
            mpls_param.u.push_l2vpn.tunnel_label.ttl = GLB_OPENFLOW_MPLS_DEFAULT_TTL;
        }
        mpls_param.u.push_l2vpn.tunnel_label.label = set_mpls_label_2->value.u32;
    }
    
    sal_memcpy(&(mpls_param.u.push_l2vpn.nexthop_mac), &(set_dl_dst->value.mac), sizeof(mpls_param.u.push_l2vpn.nexthop_mac));
    if (set_vlan_vid)
    {
        mpls_param.u.push_l2vpn.outer_vlan_vid = set_vlan_vid->value.u16;
    }
    else
    {
        mpls_param.u.push_l2vpn.outer_vlan_vid = GLB_MAX_VLAN_ID;    
    }
    
    mpls_param.u.push_l2vpn.pw_label.label_flag = GLB_OPENFLOW_MPLS_NH_LABEL_IS_VALID;
    if (HSRV_MPLS_MCAST_ETH_TYPE == push_mpls_1->value.u16)
    {
        mpls_param.u.push_l2vpn.pw_label.label_flag |= GLB_OPENFLOW_MPLS_NH_LABEL_IS_MCAST;
    }    
    mpls_param.u.push_l2vpn.pw_label.exp_type = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_MAP;
    if (set_mpls_tc_1)
    {
        mpls_param.u.push_l2vpn.pw_label.exp = set_mpls_tc_1->value.u8;
        mpls_param.u.push_l2vpn.pw_label.exp_type = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_ASSIGN;
    }
    else
    {    
        mpls_param.u.push_l2vpn.pw_label.exp = GLB_OPENFLOW_MPLS_DEFAULT_EXP;
    }
    if (set_mpls_ttl_1)
    {
        mpls_param.u.push_l2vpn.pw_label.ttl = set_mpls_ttl_1->value.u8;
    }
    else
    {
        mpls_param.u.push_l2vpn.pw_label.ttl = GLB_OPENFLOW_MPLS_DEFAULT_TTL;
    }
    mpls_param.u.push_l2vpn.pw_label.label = set_mpls_label_1->value.u32;
    ret = hsrv_openflow_nexthop_alloc_mpls_nh(&mpls_param, &p_member_nh_array[mcast_member_index]);
    if (ret == HSRV_E_OPENFLOW_MPLS_TUNNELID_RESOURCE_FULL) 
    {
        goto mpls_tunnel_id_exhausted;
    }
    else if (ret)
    {
        goto alloc_error_out;
    }

    p_member_nh_array[mcast_member_index].nh_offset.redirect_to_ingress_port = FALSE;   
    p_member_nh_array[mcast_member_index].gport = output_gport;
    p_member_nh_array[mcast_member_index].ifindex = output->value.output.ifindex;
        
    return HSRV_E_NONE;
    
parse_error_out:
    HSRV_LOG_ERR("Failed to parse mpls action list seq l2vpn_ingress_0.\n");
    return HSRV_E_OPENFLOW_MPLS_PARSE_ACTIONLIST_SEQ_FAILED;

alloc_error_out:
    HSRV_LOG_ERR("Failed to allocate mpls nexthop for mpls action list seq l2vpn_ingress_0.\n");
    return HSRV_E_OPENFLOW_MPLS_ALLOC_RESOURCE_FAILED;
    
mpls_tunnel_id_exhausted:
    /* Have warned in hsrv_openflow_nexthop_alloc_mpls_nh, no need to warn again. */
    return HSRV_E_OPENFLOW_MPLS_TUNNELID_RESOURCE_FULL;
}

/*GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_L2VPN_INGRESS_1*/
static int32
_hsrv_openflow_mpls_alloc_res_for_seq_l2vpn_ingress_1(glb_openflow_flow_action_t **cur_flow_action,
                                                                   hsrv_of_nh_info_t* p_member_nh_array,
                                                                   uint16 mcast_member_index)
{
    const glb_openflow_flow_action_t *push_vlan = NULL;
    const glb_openflow_flow_action_t *set_vlan_vid_payload = NULL;
    const glb_openflow_flow_action_t *push_mpls_1 = NULL;
    const glb_openflow_flow_action_t *set_mpls_tc_1 = NULL;
    const glb_openflow_flow_action_t *set_mpls_ttl_1 = NULL;
    const glb_openflow_flow_action_t *set_mpls_label_1 = NULL;
    const glb_openflow_flow_action_t *push_mpls_2 = NULL;
    const glb_openflow_flow_action_t *set_mpls_tc_2 = NULL;
    const glb_openflow_flow_action_t *set_mpls_ttl_2 = NULL;
    const glb_openflow_flow_action_t *set_mpls_label_2 = NULL;
    const glb_openflow_flow_action_t *output = NULL;
    const glb_openflow_flow_action_t *set_dl_dst = NULL;
    const glb_openflow_flow_action_t *set_vlan_vid = NULL;
    const glb_openflow_flow_action_t *push_l2 = NULL;
    
    hsrv_ofnh_mpls_param_t mpls_param;
    uint16 output_gport;
    int32 ret;

    (void)push_l2;
    (void)push_vlan;
    /*pull all action out and allocate nexthop accordingly*/
    GLB_OPENFLOW_MPLS_DECODE_ACTION(*cur_flow_action, push_vlan, GLB_OPENFLOW_ACTION_TYPE_PUSH_VLAN);
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_ACTION(*cur_flow_action, set_vlan_vid_payload, GLB_OPENFLOW_ACTION_TYPE_SET_VLAN_VID);
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_PUSH_MPLS_ACTION(*cur_flow_action, push_mpls_1, set_mpls_tc_1, set_mpls_ttl_1, set_mpls_label_1);               
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_PUSH_MPLS_ACTION(*cur_flow_action, push_mpls_2, set_mpls_tc_2, set_mpls_ttl_2, set_mpls_label_2);               
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_PUSH_L2_OUTPUT_ACTION(*cur_flow_action,
                                                  push_l2,
                                                  set_dl_dst,
                                                  set_vlan_vid,
                                                  output);
    
    if(output->value.output.ifindex < GLB_BOND_IFINDEX_BASE)
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(output->value.output.ifindex, &output_gport));
    }
    else
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_agg_portid(output->value.output.ifindex,&output_gport));
    }
    sal_memset(&mpls_param, 0, sizeof(mpls_param));
    mpls_param.gport = output_gport;
    mpls_param.type = GLB_OPENFLOW_MPLS_NEXTHOP_TYPE_PUSH_L2VPN;
    mpls_param.u.push_l2vpn.inner_svlan_tag_op = GLB_OPENFLOW_MPLS_NEXTHOP_TAG_OP_INSERT;
    mpls_param.u.push_l2vpn.inner_svlan_id = set_vlan_vid_payload->value.u16;
    
    mpls_param.u.push_l2vpn.tunnel_label.label_flag = 0; /* no tunnel label. */
    /*added by hansf for bug 43683*/
    //if (GLB_OPF_MPLS_IPV4_IMPLICIT_NULL != set_mpls_label_2->value.u32)
    if (set_mpls_label_2->value.u32)
    {
        mpls_param.u.push_l2vpn.tunnel_label.label_flag = GLB_OPENFLOW_MPLS_NH_LABEL_IS_VALID;
        if (HSRV_MPLS_MCAST_ETH_TYPE == push_mpls_2->value.u16)
        {
            mpls_param.u.push_l2vpn.tunnel_label.label_flag |= GLB_OPENFLOW_MPLS_NH_LABEL_IS_MCAST;
        }    
        mpls_param.u.push_l2vpn.tunnel_label.exp_type = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_MAP;
        if (set_mpls_tc_2)
        {
            mpls_param.u.push_l2vpn.tunnel_label.exp = set_mpls_tc_2->value.u8;
            mpls_param.u.push_l2vpn.tunnel_label.exp_type = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_ASSIGN;
        }
        else
        {    
            mpls_param.u.push_l2vpn.tunnel_label.exp = GLB_OPENFLOW_MPLS_DEFAULT_EXP;
        }
        if (set_mpls_ttl_2)
        {
            mpls_param.u.push_l2vpn.tunnel_label.ttl = set_mpls_ttl_2->value.u8;
        }
        else
        {
            mpls_param.u.push_l2vpn.tunnel_label.ttl = GLB_OPENFLOW_MPLS_DEFAULT_TTL;
        }
        mpls_param.u.push_l2vpn.tunnel_label.label = set_mpls_label_2->value.u32;
    }
    
    sal_memcpy(&(mpls_param.u.push_l2vpn.nexthop_mac), &(set_dl_dst->value.mac), sizeof(mpls_param.u.push_l2vpn.nexthop_mac));
    if (set_vlan_vid)
    {
        mpls_param.u.push_l2vpn.outer_vlan_vid = set_vlan_vid->value.u16;
    }
    else
    {
        mpls_param.u.push_l2vpn.outer_vlan_vid = GLB_MAX_VLAN_ID;    
    }
    
    mpls_param.u.push_l2vpn.pw_label.label_flag = GLB_OPENFLOW_MPLS_NH_LABEL_IS_VALID;
    if (HSRV_MPLS_MCAST_ETH_TYPE == push_mpls_1->value.u16)
    {
        mpls_param.u.push_l2vpn.pw_label.label_flag |= GLB_OPENFLOW_MPLS_NH_LABEL_IS_MCAST;
    }    
    mpls_param.u.push_l2vpn.pw_label.exp_type = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_MAP;
    if (set_mpls_tc_1)
    {
        mpls_param.u.push_l2vpn.pw_label.exp = set_mpls_tc_1->value.u8;
        mpls_param.u.push_l2vpn.pw_label.exp_type = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_ASSIGN;
    }
    else
    {    
        mpls_param.u.push_l2vpn.pw_label.exp = GLB_OPENFLOW_MPLS_DEFAULT_EXP;
    }
    if (set_mpls_ttl_1)
    {
        mpls_param.u.push_l2vpn.pw_label.ttl = set_mpls_ttl_1->value.u8;
    }
    else
    {
        mpls_param.u.push_l2vpn.pw_label.ttl = GLB_OPENFLOW_MPLS_DEFAULT_TTL;
    }
    mpls_param.u.push_l2vpn.pw_label.label = set_mpls_label_1->value.u32;
    
    ret = hsrv_openflow_nexthop_alloc_mpls_nh(&mpls_param, &p_member_nh_array[mcast_member_index]);
    if (ret == HSRV_E_OPENFLOW_MPLS_TUNNELID_RESOURCE_FULL) 
    {
        goto mpls_tunnel_id_exhausted;
    }
    else if (ret)
    {
        goto alloc_error_out;
    }

    p_member_nh_array[mcast_member_index].nh_offset.redirect_to_ingress_port = FALSE;   
    p_member_nh_array[mcast_member_index].gport = output_gport;
    p_member_nh_array[mcast_member_index].ifindex = output->value.output.ifindex;
        
    return HSRV_E_NONE;
    
parse_error_out:
    HSRV_LOG_ERR("Failed to parse mpls action list seq l2vpn_ingress_1.\n");
    return HSRV_E_OPENFLOW_MPLS_PARSE_ACTIONLIST_SEQ_FAILED;

alloc_error_out:
    HSRV_LOG_ERR("Failed to allocate mpls nexthop for mpls action list seq l2vpn_ingress_1.\n");
    return HSRV_E_OPENFLOW_MPLS_ALLOC_RESOURCE_FAILED;
    
mpls_tunnel_id_exhausted:
    /* Have warned in hsrv_openflow_nexthop_alloc_mpls_nh, no need to warn again. */
    return HSRV_E_OPENFLOW_MPLS_TUNNELID_RESOURCE_FULL;    
}

/*GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_L2VPN_INGRESS_2*/
static int32
_hsrv_openflow_mpls_alloc_res_for_seq_l2vpn_ingress_2(glb_openflow_flow_action_t **cur_flow_action,
                                                                   hsrv_of_nh_info_t* p_member_nh_array,
                                                                   uint16 mcast_member_index)
{
    const glb_openflow_flow_action_t *set_vlan_vid_payload = NULL;
    const glb_openflow_flow_action_t *push_mpls_1 = NULL;
    const glb_openflow_flow_action_t *set_mpls_tc_1 = NULL;
    const glb_openflow_flow_action_t *set_mpls_ttl_1 = NULL;
    const glb_openflow_flow_action_t *set_mpls_label_1 = NULL;
    const glb_openflow_flow_action_t *push_mpls_2 = NULL;
    const glb_openflow_flow_action_t *set_mpls_tc_2 = NULL;
    const glb_openflow_flow_action_t *set_mpls_ttl_2 = NULL;
    const glb_openflow_flow_action_t *set_mpls_label_2 = NULL;
    const glb_openflow_flow_action_t *output = NULL;
    const glb_openflow_flow_action_t *set_dl_dst = NULL;
    const glb_openflow_flow_action_t *set_vlan_vid = NULL;
    const glb_openflow_flow_action_t *push_l2 = NULL;
    
    hsrv_ofnh_mpls_param_t mpls_param;
    uint16 output_gport;
    int32 ret;

    (void)push_l2;
    /*pull all action out and allocate nexthop accordingly*/
    GLB_OPENFLOW_MPLS_DECODE_ACTION(*cur_flow_action, set_vlan_vid_payload, GLB_OPENFLOW_ACTION_TYPE_SET_VLAN_VID);
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_PUSH_MPLS_ACTION(*cur_flow_action, push_mpls_1, set_mpls_tc_1, set_mpls_ttl_1, set_mpls_label_1);               
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_PUSH_MPLS_ACTION(*cur_flow_action, push_mpls_2, set_mpls_tc_2, set_mpls_ttl_2, set_mpls_label_2);               
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_PUSH_L2_OUTPUT_ACTION(*cur_flow_action,
                                                  push_l2,
                                                  set_dl_dst,
                                                  set_vlan_vid,
                                                  output);
    
    if(output->value.output.ifindex < GLB_BOND_IFINDEX_BASE)
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(output->value.output.ifindex, &output_gport));
    }
    else
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_agg_portid(output->value.output.ifindex,&output_gport));
    }
    sal_memset(&mpls_param, 0, sizeof(mpls_param));
    mpls_param.gport = output_gport;
    mpls_param.type = GLB_OPENFLOW_MPLS_NEXTHOP_TYPE_PUSH_L2VPN;
    mpls_param.u.push_l2vpn.inner_svlan_tag_op = GLB_OPENFLOW_MPLS_NEXTHOP_TAG_OP_REPLACE;
    mpls_param.u.push_l2vpn.inner_svlan_id = set_vlan_vid_payload->value.u16;
    
    mpls_param.u.push_l2vpn.tunnel_label.label_flag = 0; /* no tunnel label. */
    /*added by hansf for bug 43683*/
    //if (GLB_OPF_MPLS_IPV4_IMPLICIT_NULL != set_mpls_label_2->value.u32)
    if (set_mpls_label_2->value.u32)
    {
        mpls_param.u.push_l2vpn.tunnel_label.label_flag = GLB_OPENFLOW_MPLS_NH_LABEL_IS_VALID;
        if (HSRV_MPLS_MCAST_ETH_TYPE == push_mpls_2->value.u16)
        {
            mpls_param.u.push_l2vpn.tunnel_label.label_flag |= GLB_OPENFLOW_MPLS_NH_LABEL_IS_MCAST;
        }    
        mpls_param.u.push_l2vpn.tunnel_label.exp_type = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_MAP;
        if (set_mpls_tc_2)
        {
            mpls_param.u.push_l2vpn.tunnel_label.exp = set_mpls_tc_2->value.u8;
            mpls_param.u.push_l2vpn.tunnel_label.exp_type = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_ASSIGN;
        }
        else
        {    
            mpls_param.u.push_l2vpn.tunnel_label.exp = GLB_OPENFLOW_MPLS_DEFAULT_EXP;
        }
        if (set_mpls_ttl_2)
        {
            mpls_param.u.push_l2vpn.tunnel_label.ttl = set_mpls_ttl_2->value.u8;
        }
        else
        {
            mpls_param.u.push_l2vpn.tunnel_label.ttl = GLB_OPENFLOW_MPLS_DEFAULT_TTL;
        }
        mpls_param.u.push_l2vpn.tunnel_label.label = set_mpls_label_2->value.u32;
    }
    
    sal_memcpy(&(mpls_param.u.push_l2vpn.nexthop_mac), &(set_dl_dst->value.mac), sizeof(mpls_param.u.push_l2vpn.nexthop_mac));
    if (set_vlan_vid)
    {
        mpls_param.u.push_l2vpn.outer_vlan_vid = set_vlan_vid->value.u16;
    }
    else
    {
        mpls_param.u.push_l2vpn.outer_vlan_vid = GLB_MAX_VLAN_ID;    
    }
    
    mpls_param.u.push_l2vpn.pw_label.label_flag = GLB_OPENFLOW_MPLS_NH_LABEL_IS_VALID;
    if (HSRV_MPLS_MCAST_ETH_TYPE == push_mpls_1->value.u16)
    {
        mpls_param.u.push_l2vpn.pw_label.label_flag |= GLB_OPENFLOW_MPLS_NH_LABEL_IS_MCAST;
    }      
    mpls_param.u.push_l2vpn.pw_label.exp_type = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_MAP;
    if (set_mpls_tc_1)
    {
        mpls_param.u.push_l2vpn.pw_label.exp = set_mpls_tc_1->value.u8;
        mpls_param.u.push_l2vpn.pw_label.exp_type = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_ASSIGN;
    }
    else
    {    
        mpls_param.u.push_l2vpn.pw_label.exp = GLB_OPENFLOW_MPLS_DEFAULT_EXP;
    }
    if (set_mpls_ttl_1)
    {
        mpls_param.u.push_l2vpn.pw_label.ttl = set_mpls_ttl_1->value.u8;
    }
    else
    {
        mpls_param.u.push_l2vpn.pw_label.ttl = GLB_OPENFLOW_MPLS_DEFAULT_TTL;
    }
    mpls_param.u.push_l2vpn.pw_label.label = set_mpls_label_1->value.u32;
    
    ret = hsrv_openflow_nexthop_alloc_mpls_nh(&mpls_param, &p_member_nh_array[mcast_member_index]);
    if (ret == HSRV_E_OPENFLOW_MPLS_TUNNELID_RESOURCE_FULL) 
    {
        goto mpls_tunnel_id_exhausted;
    }
    else if (ret)
    {
        goto alloc_error_out;
    }

    p_member_nh_array[mcast_member_index].nh_offset.redirect_to_ingress_port = FALSE;   
    p_member_nh_array[mcast_member_index].gport = output_gport;
    p_member_nh_array[mcast_member_index].ifindex = output->value.output.ifindex;
        
    return HSRV_E_NONE;
    
parse_error_out:
    HSRV_LOG_ERR("Failed to parse mpls action list seq l2vpn_ingress_2.\n");
    return HSRV_E_OPENFLOW_MPLS_PARSE_ACTIONLIST_SEQ_FAILED;

alloc_error_out:
    HSRV_LOG_ERR("Failed to allocate mpls nexthop for mpls action list seq l2vpn_ingress_2.\n");
    return HSRV_E_OPENFLOW_MPLS_ALLOC_RESOURCE_FAILED;
    
mpls_tunnel_id_exhausted:
    /* Have warned in hsrv_openflow_nexthop_alloc_mpls_nh, no need to warn again. */
    return HSRV_E_OPENFLOW_MPLS_TUNNELID_RESOURCE_FULL;    
}

/*GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_L2VPN_INGRESS_3*/
static int32
_hsrv_openflow_mpls_alloc_res_for_seq_l2vpn_ingress_3(glb_openflow_flow_action_t **cur_flow_action,
                                                                   hsrv_of_nh_info_t* p_member_nh_array,
                                                                   uint16 mcast_member_index)
{
    const glb_openflow_flow_action_t *push_mpls_1 = NULL;
    const glb_openflow_flow_action_t *set_mpls_tc_1 = NULL;
    const glb_openflow_flow_action_t *set_mpls_ttl_1 = NULL;
    const glb_openflow_flow_action_t *set_mpls_label_1 = NULL;
    const glb_openflow_flow_action_t *push_mpls_2 = NULL;
    const glb_openflow_flow_action_t *set_mpls_tc_2 = NULL;
    const glb_openflow_flow_action_t *set_mpls_ttl_2 = NULL;
    const glb_openflow_flow_action_t *set_mpls_label_2 = NULL;
    const glb_openflow_flow_action_t *output = NULL;
    const glb_openflow_flow_action_t *set_dl_dst = NULL;
    const glb_openflow_flow_action_t *set_vlan_vid = NULL;
    const glb_openflow_flow_action_t *push_l2 = NULL;
       
    hsrv_ofnh_mpls_param_t mpls_param;
    uint16 output_gport;
    int32 ret;

    (void)push_l2;
    /*pull all action out and allocate nexthop accordingly*/
    GLB_OPENFLOW_MPLS_DECODE_PUSH_MPLS_ACTION(*cur_flow_action, push_mpls_1, set_mpls_tc_1, set_mpls_ttl_1, set_mpls_label_1);               
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_PUSH_MPLS_ACTION(*cur_flow_action, push_mpls_2, set_mpls_tc_2, set_mpls_ttl_2, set_mpls_label_2);               
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_PUSH_L2_OUTPUT_ACTION(*cur_flow_action,
                                                  push_l2,
                                                  set_dl_dst,
                                                  set_vlan_vid,
                                                  output);
    if(output->value.output.ifindex < GLB_BOND_IFINDEX_BASE)
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(output->value.output.ifindex, &output_gport));
    }
    else
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_agg_portid(output->value.output.ifindex,&output_gport));
    }

    sal_memset(&mpls_param, 0, sizeof(mpls_param));
    mpls_param.gport = output_gport;
    mpls_param.type = GLB_OPENFLOW_MPLS_NEXTHOP_TYPE_PUSH_L2VPN;
    mpls_param.u.push_l2vpn.inner_svlan_tag_op = GLB_OPENFLOW_MPLS_NEXTHOP_TAG_OP_KEEP_UNCHANGE;
    
    mpls_param.u.push_l2vpn.tunnel_label.label_flag = 0; /* no tunnel label. */
    /*added by hansf for bug 43683*/
    //if (GLB_OPF_MPLS_IPV4_IMPLICIT_NULL != set_mpls_label_2->value.u32)
    if (set_mpls_label_2->value.u32)
    {
        mpls_param.u.push_l2vpn.tunnel_label.label_flag = GLB_OPENFLOW_MPLS_NH_LABEL_IS_VALID;
        if (HSRV_MPLS_MCAST_ETH_TYPE == push_mpls_2->value.u16)
        {
            mpls_param.u.push_l2vpn.tunnel_label.label_flag |= GLB_OPENFLOW_MPLS_NH_LABEL_IS_MCAST;
        }    
        mpls_param.u.push_l2vpn.tunnel_label.exp_type = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_MAP;
        if (set_mpls_tc_2)
        {
            mpls_param.u.push_l2vpn.tunnel_label.exp = set_mpls_tc_2->value.u8;
            mpls_param.u.push_l2vpn.tunnel_label.exp_type = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_ASSIGN;
        }
        else
        {    
            mpls_param.u.push_l2vpn.tunnel_label.exp = GLB_OPENFLOW_MPLS_DEFAULT_EXP;
        }
        if (set_mpls_ttl_2)
        {
            mpls_param.u.push_l2vpn.tunnel_label.ttl = set_mpls_ttl_2->value.u8;
        }
        else
        {
            mpls_param.u.push_l2vpn.tunnel_label.ttl = GLB_OPENFLOW_MPLS_DEFAULT_TTL;
        }
        mpls_param.u.push_l2vpn.tunnel_label.label = set_mpls_label_2->value.u32;
    }
    
    sal_memcpy(&(mpls_param.u.push_l2vpn.nexthop_mac), &(set_dl_dst->value.mac), sizeof(mpls_param.u.push_l2vpn.nexthop_mac));
    if (set_vlan_vid)
    {
        mpls_param.u.push_l2vpn.outer_vlan_vid = set_vlan_vid->value.u16;
    }
    else
    {
        mpls_param.u.push_l2vpn.outer_vlan_vid = GLB_MAX_VLAN_ID;    
    }
    
    mpls_param.u.push_l2vpn.pw_label.label_flag = GLB_OPENFLOW_MPLS_NH_LABEL_IS_VALID;
    if (HSRV_MPLS_MCAST_ETH_TYPE == push_mpls_1->value.u16)
    {
        mpls_param.u.push_l2vpn.pw_label.label_flag |= GLB_OPENFLOW_MPLS_NH_LABEL_IS_MCAST;
    }    
    mpls_param.u.push_l2vpn.pw_label.exp_type = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_MAP;
    if (set_mpls_tc_1)
    {
        mpls_param.u.push_l2vpn.pw_label.exp = set_mpls_tc_1->value.u8;
        mpls_param.u.push_l2vpn.pw_label.exp_type = GLB_OPENFLOW_MPLS_NH_EXP_SELECT_ASSIGN;
    }
    else
    {    
        mpls_param.u.push_l2vpn.pw_label.exp = GLB_OPENFLOW_MPLS_DEFAULT_EXP;
    }
    if (set_mpls_ttl_1)
    {
        mpls_param.u.push_l2vpn.pw_label.ttl = set_mpls_ttl_1->value.u8;
    }
    else
    {
        mpls_param.u.push_l2vpn.pw_label.ttl = GLB_OPENFLOW_MPLS_DEFAULT_TTL;
    }
    mpls_param.u.push_l2vpn.pw_label.label = set_mpls_label_1->value.u32;
    
    ret = hsrv_openflow_nexthop_alloc_mpls_nh(&mpls_param, &p_member_nh_array[mcast_member_index]);
    if (ret == HSRV_E_OPENFLOW_MPLS_TUNNELID_RESOURCE_FULL) 
    {
        goto mpls_tunnel_id_exhausted;
    }
    else if (ret)
    {
        goto alloc_error_out;
    }

    p_member_nh_array[mcast_member_index].nh_offset.redirect_to_ingress_port = FALSE;   
    p_member_nh_array[mcast_member_index].gport = output_gport;
    p_member_nh_array[mcast_member_index].ifindex = output->value.output.ifindex;
        
    return HSRV_E_NONE;
    
parse_error_out:
    HSRV_LOG_ERR("Failed to parse mpls action list seq l2vpn_ingress_3.\n");
    return HSRV_E_OPENFLOW_MPLS_PARSE_ACTIONLIST_SEQ_FAILED;

alloc_error_out:
    HSRV_LOG_ERR("Failed to allocate mpls nexthop for mpls action list seq l2vpn_ingress_3.\n");
    return HSRV_E_OPENFLOW_MPLS_ALLOC_RESOURCE_FAILED;
    
mpls_tunnel_id_exhausted:
    /* Have warned in hsrv_openflow_nexthop_alloc_mpls_nh, no need to warn again. */
    return HSRV_E_OPENFLOW_MPLS_TUNNELID_RESOURCE_FULL;  

}

/*GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_L2VPN_EGRESS_0*/
static int32
_hsrv_openflow_mpls_alloc_res_for_seq_l2vpn_egress_0(glb_openflow_flow_action_t **cur_flow_action,
                                                                   hsrv_of_nh_info_t* p_member_nh_array,
                                                                   uint16 mcast_member_index)
{
    const glb_openflow_flow_action_t *pop_l2 = NULL;
    const glb_openflow_flow_action_t *pop_mpls = NULL;
    const glb_openflow_flow_action_t *pop_vlan_payload = NULL;
    const glb_openflow_flow_action_t *output = NULL;

    hsrv_ofnh_xlate_param_t xlate_param;
    uint16 output_gport;
    int32 ret;    

    (void)pop_vlan_payload;
    (void)pop_mpls;
    (void)pop_l2;
    /*pull all action out and allocate nexthop accordingly*/
    GLB_OPENFLOW_MPLS_DECODE_ACTION(*cur_flow_action, pop_l2, GLB_OPENFLOW_ACTION_TYPE_POP_L2);
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_ACTION(*cur_flow_action, pop_mpls, GLB_OPENFLOW_ACTION_TYPE_POP_MPLS);
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_ACTION(*cur_flow_action, pop_vlan_payload, GLB_OPENFLOW_ACTION_TYPE_STRIP_VLAN);
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_ACTION(*cur_flow_action, output, GLB_OPENFLOW_ACTION_TYPE_OUTPUT);

    if(output->value.output.ifindex < GLB_BOND_IFINDEX_BASE)
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(output->value.output.ifindex, &output_gport));
    }
    else
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_agg_portid(output->value.output.ifindex,&output_gport));
    }
    sal_memset(&xlate_param, 0, sizeof(xlate_param));
    xlate_param.gport = output_gport;
    xlate_param.inner_svlan_tag_op = GLB_OPENFLOW_MPLS_NEXTHOP_TAG_OP_STRIP;

    ret = hsrv_openflow_nexthop_alloc_xlate_nh(&xlate_param, &p_member_nh_array[mcast_member_index]);
    if (ret)
    {
        goto alloc_error_out;
    }

    p_member_nh_array[mcast_member_index].nh_offset.redirect_to_ingress_port = FALSE;   
    p_member_nh_array[mcast_member_index].gport = output_gport;
    p_member_nh_array[mcast_member_index].ifindex = output->value.output.ifindex;
        
    return HSRV_E_NONE;
    
parse_error_out:
    HSRV_LOG_ERR("Failed to parse mpls action list seq l2vpn_egress_0.\n");
    return HSRV_E_OPENFLOW_MPLS_PARSE_ACTIONLIST_SEQ_FAILED;

alloc_error_out:
    HSRV_LOG_ERR("Failed to allocate mpls nexthop for mpls action list seq l2vpn_egress_0.\n");
    return HSRV_E_OPENFLOW_MPLS_ALLOC_RESOURCE_FAILED;
}

/*GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_L2VPN_EGRESS_1*/
static int32
_hsrv_openflow_mpls_alloc_res_for_seq_l2vpn_egress_1(glb_openflow_flow_action_t **cur_flow_action,
                                                                   hsrv_of_nh_info_t* p_member_nh_array,
                                                                   uint16 mcast_member_index)
{
    const glb_openflow_flow_action_t *pop_l2 = NULL;
    const glb_openflow_flow_action_t *pop_mpls = NULL;
    const glb_openflow_flow_action_t *push_vlan_payload = NULL;
    const glb_openflow_flow_action_t *set_vlan_vid_payload = NULL;
    const glb_openflow_flow_action_t *output = NULL;
    
    hsrv_ofnh_xlate_param_t xlate_param;
    uint16 output_gport;
    int32 ret;    

    (void)push_vlan_payload;
    (void)pop_mpls;
    (void)pop_l2;
    /*pull all action out and allocate nexthop accordingly*/
    GLB_OPENFLOW_MPLS_DECODE_ACTION(*cur_flow_action, pop_l2, GLB_OPENFLOW_ACTION_TYPE_POP_L2);
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_ACTION(*cur_flow_action, pop_mpls, GLB_OPENFLOW_ACTION_TYPE_POP_MPLS);
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_ACTION(*cur_flow_action, push_vlan_payload, GLB_OPENFLOW_ACTION_TYPE_PUSH_VLAN);
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_ACTION(*cur_flow_action, set_vlan_vid_payload, GLB_OPENFLOW_ACTION_TYPE_SET_VLAN_VID);
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_ACTION(*cur_flow_action, output, GLB_OPENFLOW_ACTION_TYPE_OUTPUT);

    if(output->value.output.ifindex < GLB_BOND_IFINDEX_BASE)
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(output->value.output.ifindex, &output_gport));
    }
    else
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_agg_portid(output->value.output.ifindex,&output_gport));
    }
    sal_memset(&xlate_param, 0, sizeof(xlate_param));
    xlate_param.gport = output_gport;
    xlate_param.inner_svlan_tag_op = GLB_OPENFLOW_MPLS_NEXTHOP_TAG_OP_INSERT;
    xlate_param.inner_svlan_id = set_vlan_vid_payload->value.u16;

    ret = hsrv_openflow_nexthop_alloc_xlate_nh(&xlate_param, &p_member_nh_array[mcast_member_index]);
    if (ret)
    {
        goto alloc_error_out;
    }

    p_member_nh_array[mcast_member_index].nh_offset.redirect_to_ingress_port = FALSE;   
    p_member_nh_array[mcast_member_index].gport = output_gport;
    p_member_nh_array[mcast_member_index].ifindex = output->value.output.ifindex;
        
    return HSRV_E_NONE;
    
parse_error_out:
    HSRV_LOG_ERR("Failed to parse mpls action list seq l2vpn_egress_0.\n");
    return HSRV_E_OPENFLOW_MPLS_PARSE_ACTIONLIST_SEQ_FAILED;

alloc_error_out:
    HSRV_LOG_ERR("Failed to allocate mpls nexthop for mpls action list seq l2vpn_egress_0.\n");
    return HSRV_E_OPENFLOW_MPLS_ALLOC_RESOURCE_FAILED;
}

/*GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_L2VPN_EGRESS_2*/
static int32
_hsrv_openflow_mpls_alloc_res_for_seq_l2vpn_egress_2(glb_openflow_flow_action_t **cur_flow_action,
                                                                   hsrv_of_nh_info_t* p_member_nh_array,
                                                                   uint16 mcast_member_index)
{
    const glb_openflow_flow_action_t *pop_l2 = NULL;
    const glb_openflow_flow_action_t *pop_mpls = NULL;
    const glb_openflow_flow_action_t *set_vlan_vid_payload = NULL;
    const glb_openflow_flow_action_t *output = NULL;
    
    hsrv_ofnh_xlate_param_t xlate_param;
    uint16 output_gport;
    int32 ret;    

    (void)pop_mpls;
    (void)pop_l2;
    /*pull all action out and allocate nexthop accordingly*/
    GLB_OPENFLOW_MPLS_DECODE_ACTION(*cur_flow_action, pop_l2, GLB_OPENFLOW_ACTION_TYPE_POP_L2);
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_ACTION(*cur_flow_action, pop_mpls, GLB_OPENFLOW_ACTION_TYPE_POP_MPLS);
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_ACTION(*cur_flow_action, set_vlan_vid_payload, GLB_OPENFLOW_ACTION_TYPE_SET_VLAN_VID);
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_ACTION(*cur_flow_action, output, GLB_OPENFLOW_ACTION_TYPE_OUTPUT);

    if(output->value.output.ifindex < GLB_BOND_IFINDEX_BASE)
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(output->value.output.ifindex, &output_gport));
    }
    else
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_agg_portid(output->value.output.ifindex,&output_gport));
    }
    sal_memset(&xlate_param, 0, sizeof(xlate_param));
    xlate_param.gport = output_gport;
    xlate_param.inner_svlan_tag_op = GLB_OPENFLOW_MPLS_NEXTHOP_TAG_OP_REPLACE;
    xlate_param.inner_svlan_id = set_vlan_vid_payload->value.u16;

    ret = hsrv_openflow_nexthop_alloc_xlate_nh(&xlate_param, &p_member_nh_array[mcast_member_index]);
    if (ret)
    {
        goto alloc_error_out;
    }

    p_member_nh_array[mcast_member_index].nh_offset.redirect_to_ingress_port = FALSE;   
    p_member_nh_array[mcast_member_index].gport = output_gport;
    p_member_nh_array[mcast_member_index].ifindex = output->value.output.ifindex;
        
    return HSRV_E_NONE;
    
parse_error_out:
    HSRV_LOG_ERR("Failed to parse mpls action list seq l2vpn_egress_0.\n");
    return HSRV_E_OPENFLOW_MPLS_PARSE_ACTIONLIST_SEQ_FAILED;

alloc_error_out:
    HSRV_LOG_ERR("Failed to allocate mpls nexthop for mpls action list seq l2vpn_egress_0.\n");
    return HSRV_E_OPENFLOW_MPLS_ALLOC_RESOURCE_FAILED;
}

/*GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_L2VPN_EGRESS_3*/
static int32
_hsrv_openflow_mpls_alloc_res_for_seq_l2vpn_egress_3(glb_openflow_flow_action_t **cur_flow_action,
                                                                   hsrv_of_nh_info_t* p_member_nh_array,
                                                                   uint16 mcast_member_index)
{
    const glb_openflow_flow_action_t *pop_l2 = NULL;
    const glb_openflow_flow_action_t *pop_mpls = NULL;
    const glb_openflow_flow_action_t *output = NULL;
    
    hsrv_ofnh_xlate_param_t xlate_param;
    uint16 output_gport;
    int32 ret;    

    (void)pop_mpls;
    (void)pop_l2;
    /*pull all action out and allocate nexthop accordingly*/
    GLB_OPENFLOW_MPLS_DECODE_ACTION(*cur_flow_action, pop_l2, GLB_OPENFLOW_ACTION_TYPE_POP_L2);
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_ACTION(*cur_flow_action, pop_mpls, GLB_OPENFLOW_ACTION_TYPE_POP_MPLS);
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(*cur_flow_action, *cur_flow_action);
    GLB_OPENFLOW_MPLS_DECODE_ACTION(*cur_flow_action, output, GLB_OPENFLOW_ACTION_TYPE_OUTPUT);

    if(output->value.output.ifindex < GLB_BOND_IFINDEX_BASE)
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(output->value.output.ifindex, &output_gport));
    }
    else
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_agg_portid(output->value.output.ifindex,&output_gport));
    }
    sal_memset(&xlate_param, 0, sizeof(xlate_param));
    xlate_param.gport = output_gport;
    xlate_param.inner_svlan_tag_op = GLB_OPENFLOW_MPLS_NEXTHOP_TAG_OP_KEEP_UNCHANGE;

    ret = hsrv_openflow_nexthop_alloc_xlate_nh(&xlate_param, &p_member_nh_array[mcast_member_index]);
    if (ret)
    {
        goto alloc_error_out;
    }

    p_member_nh_array[mcast_member_index].nh_offset.redirect_to_ingress_port = FALSE;   
    p_member_nh_array[mcast_member_index].gport = output_gport;
    p_member_nh_array[mcast_member_index].ifindex = output->value.output.ifindex;
        
    return HSRV_E_NONE;
    
parse_error_out:
    HSRV_LOG_ERR("Failed to parse mpls action list seq l2vpn_egress_0.\n");
    return HSRV_E_OPENFLOW_MPLS_PARSE_ACTIONLIST_SEQ_FAILED;

alloc_error_out:
    HSRV_LOG_ERR("Failed to allocate mpls nexthop for mpls action list seq l2vpn_egress_0.\n");
    return HSRV_E_OPENFLOW_MPLS_ALLOC_RESOURCE_FAILED;
}

int32
hsrv_openflow_mpls_map_group_output_action(glb_openflow_bucket_t* p_bucket,
          hsrv_of_nh_info_t* p_member_nh_array,
          uint32* p_member_cnt)
{
    glb_openflow_flow_action_t *cur_flow_action = NULL;
    uint32 i;
    uint32 mpls_output_action_seq_no = GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_VALID_MAX;
    int32 ret = HSRV_E_OPENFLOW_MPLS_ALLOC_RESOURCE_FAILED;

    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_FIRST(p_bucket->actlist, cur_flow_action);
    for (i = 0; i < p_bucket->mpls_output_count; i++)
    {    
        mpls_output_action_seq_no = p_bucket->mpls_output_action_seq_type;
        if (mpls_output_action_seq_no < GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_VALID_MAX)
        {
            HSRV_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_ALLOC_RES_FUNC func_ptr = NULL;
            func_ptr = hsrv_openflow_mpls_output_action_seq_res_func[mpls_output_action_seq_no];
            if (NULL != func_ptr)
            {
                ret = (func_ptr)(&cur_flow_action, p_member_nh_array, *p_member_cnt);
            }
            else
            {
                HSRV_LOG_ERR("HSRV OPENFLOW MPLS module is not initialized properly.\n");
                goto error_out;
            }
            
            if (!ret)
            {
                p_member_nh_array[*p_member_cnt].nh_offset.port_check_discard = hsrv_openflow_group_get_drop_ingress();   
                /* If this member alloc failed, continue to try nexthop member. */
                (*p_member_cnt)++;
            }

            /*skip the OFPAT_OUTPUT action to prepare for next action seq processing*/
            if ((i + 1) < p_bucket->mpls_output_count)
            {
                GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(cur_flow_action, cur_flow_action);
            }
        }
        else
        {
            /*can't reach here, parse ERROR*/
            goto error_out;
        }
    }

    return HSRV_E_NONE;

error_out:
    return HSRV_E_OPENFLOW_MPLS_ALLOC_RESOURCE_FAILED;
}

int32
hsrv_openflow_mpls_map_flow_output_action(hsrv_openflow_flow_t *flow)
{
    glb_openflow_flow_action_list_t *flow_action_list = &flow->flow.actlist;
    glb_openflow_flow_action_t *cur_flow_action = NULL;
    uint32 i, cur_action_index = 0;
    uint32 mpls_output_action_seq_no = GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_VALID_MAX;
    int32 ret = HSRV_E_OPENFLOW_MPLS_ALLOC_RESOURCE_FAILED;
    hsrv_of_nh_offset_t group_nh;
    hsrv_ofnh_mcast_param_t mcast_param;
    hsrv_openflow_nexthop_info_t *nh_info = &flow->u.dft.var.nh_info;
    uint16 effective_member_count;
    uint32 should_skip = 0;
        
    /* Get first action check whether it's meter, if meter move cur action pointer to next action. */
    for(cur_action_index = 0; cur_action_index < flow_action_list->action_count; cur_action_index ++)
    {
        cur_flow_action = &((flow_action_list->actlist)[cur_action_index]);
        if(cur_flow_action->type == GLB_OPENFLOW_ACTION_TYPE_INST_METER)
        {
            should_skip = 1;
        }
        if(cur_flow_action->type == GLB_OPENFLOW_ACTION_TYPE_SET_TRUNCATE)
        {
            cur_action_index += 1;
            should_skip = 1;
        }
        if(cur_flow_action->type == GLB_OPENFLOW_ACTION_TYPE_SET_QUEUE)
        {
            should_skip = 1;
        }
        if(0 == should_skip)
        {
            break;
        }
        should_skip = 0;
    }

    effective_member_count = 0;
    for (i = 0; i < flow->flow.u.dft.mpls_output_count; i++)
    {    
        mpls_output_action_seq_no = flow->flow.u.dft.mpls_output[i];
        if (mpls_output_action_seq_no < GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_VALID_MAX)
        {
            HSRV_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_ALLOC_RES_FUNC func_ptr = NULL;
            func_ptr = hsrv_openflow_mpls_output_action_seq_res_func[mpls_output_action_seq_no];
            /* bug53825 add NULL ptr check cur_flow_action */
            if (NULL != func_ptr && NULL != cur_flow_action)
            {
                ret = (func_ptr)(&cur_flow_action, nh_info->member_nh, effective_member_count);
            }
            else
            {
                HSRV_LOG_ERR("HSRV OPENFLOW MPLS module is not initialized properly.\n");
                goto error_out;
            }
            
            if (!ret)
            {
                nh_info->member_nh[effective_member_count].nh_offset.port_check_discard = hsrv_openflow_flowdb_get_drop_ingress();   
                /* If this member alloc failed, continue to try nexthop member. */
                effective_member_count++;
            }

            /*skip the OFPAT_OUTPUT action to prepare for next action seq processing*/
            if ((i + 1) < flow->flow.u.dft.mpls_output_count)
            {
                GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(cur_flow_action, cur_flow_action);
            }
        }
        else
        {
            /*can't reach here, parse ERROR*/
            goto error_out;
        }
    }

    /*for normal MPLS operation,we do not allow multiple output, thus we do not need allocate
      multicast group*/
    if (GLB_OPENFLOW_MPLS_ACTION_SEQ_IS_NORMAL_MPLS(flow->flow.u.dft.mpls_output[0]))
    {
        nh_info->main_nh.nh_offset.nhid    = nh_info->member_nh[0].nh_offset.nhid;
        nh_info->main_nh.nh_offset.offset  = nh_info->member_nh[0].nh_offset.offset;
        nh_info->main_nh.nh_offset.nh_type = nh_info->member_nh[0].nh_offset.nh_type;
        nh_info->main_nh.share_info = nh_info->member_nh[0].share_info;
        nh_info->use_mcast = FALSE;
    }
    else 
    {
        /* Always use mcast group to keep simple. */
        memset(&group_nh, 0, sizeof(hsrv_of_nh_offset_t));
        mcast_param.stats_en = FALSE;
        mcast_param.no_fwd = FALSE;
        HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_alloc_mcast_group(&mcast_param, &group_nh));
        nh_info->main_nh.nh_offset.nhid    = group_nh.nhid;
        nh_info->main_nh.nh_offset.offset  = group_nh.offset;
        nh_info->main_nh.nh_offset.nh_type = group_nh.nh_type;
        nh_info->use_mcast = TRUE;
        nh_info->member_count = effective_member_count;
        for (i = 0; i < nh_info->member_count; i ++)
        {
            hsrv_ofnh_mcast_member_param_t member_param;
            hsrv_of_nh_info_t* member;

            member = &nh_info->member_nh[i];
            member_param.group_nhid = group_nh.nhid;
            member_param.op_type = HSRV_OFNH_MCAST_OP_ADD_MEMBER;
            member_param.mem_gport = member->gport;
            member_param.mem_nhid = member->nh_offset.nhid;
            member_param.port_check_discard = member->nh_offset.port_check_discard;
            member_param.redirect_to_ingress_port = member->nh_offset.redirect_to_ingress_port;
            
            HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_add_mcast_member(&member_param));
        }
    }
    
    return HSRV_E_NONE;

error_out:
    return HSRV_E_OPENFLOW_MPLS_ALLOC_RESOURCE_FAILED;
}

static int32
hsrv_openflow_mpls_register_action_seq_callback(void)
{
#define REG(type, func)\
    hsrv_openflow_mpls_output_action_seq_res_func[GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_ ## type] = _hsrv_openflow_mpls_alloc_res_for_seq_ ## func;

    /* for cubro mpls. */
    REG(POP_ALL_MPLS_0, no_process_needed);
    REG(POP_ALL_MPLS_1, pop_all_mpls_1);
    REG(TAG_PACKET_IN_L2VPN_0, tag_packet_in_l2vpn_0);
    REG(TAG_PACKET_IN_L2VPN_1, tag_packet_in_l2vpn_1);
    
    REG(NORMAL_PUSH_0, normal_push_0);
    REG(NORMAL_SWAP_0, normal_swap_0);
    REG(NORMAL_SWAP_1, normal_swap_1);
    REG(NORMAL_POP_0,  normal_pop_0);
    REG(NORMAL_POP_1,  normal_pop_1);
    
    REG(L2VPN_INGRESS_0, l2vpn_ingress_0);
    REG(L2VPN_INGRESS_1, l2vpn_ingress_1);
    REG(L2VPN_INGRESS_2, l2vpn_ingress_2);
    REG(L2VPN_INGRESS_3, l2vpn_ingress_3);

    REG(L2VPN_EGRESS_0, l2vpn_egress_0);
    REG(L2VPN_EGRESS_1, l2vpn_egress_1);
    REG(L2VPN_EGRESS_2, l2vpn_egress_2);
    REG(L2VPN_EGRESS_3, l2vpn_egress_3);

#undef REG

    return HSRV_E_NONE;
}

int32
hsrv_openflow_mpls_init(hsrv_system_spec_info_t *p_system_info)
{
   HSRV_IF_ERROR_RETURN(hsrv_openflow_mpls_register_action_seq_callback());
    
    return HSRV_E_NONE;
}

