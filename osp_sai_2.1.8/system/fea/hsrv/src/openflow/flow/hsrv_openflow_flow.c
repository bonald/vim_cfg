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
#include "hsrv_openflow_flow_priv.h"
#include "hsrv_openflow_group.h"
#include "hsrv_openflow_mpls.h"
#include "hsrv_msg_openflow.h"
#include "hsrv_openflow_tpoam.h"
#include "hsrv_tunneldb_cdb.h"

extern hsrv_openflow_stats_master_t* g_pst_openflow_stats_master;

extern int32
hsrv_openflow_map_out_action_include_dynamic_tunnel_port(hsrv_openflow_map_output_data_t* p_data, uint32* p_output_num, uint32* p_is_include);

extern bool
hsrv_openflow_flow_action_include_dynamic_tunnel_port(hsrv_openflow_flow_t* p_flow);

struct hsrv_openflow_debug_data_s
{
    FILE* fp;
    int32 i;
};
typedef struct hsrv_openflow_debug_data_s hsrv_openflow_flow_debug_data_t;

struct hsrv_openflow_debug_data1_s
{
    uint32 actlist_ptr;
    bool exist;
};
typedef struct hsrv_openflow_debug_data1_s hsrv_openflow_flow_debug_data1_t;

struct hsrv_openflow_all_mcast_port_data_s
{
    hsrv_openflow_action_combo_t* p_action;
    
    hsrv_of_nh_info_t* p_member_nh_array;
    uint32* p_member_cnt;
};
typedef struct hsrv_openflow_all_mcast_port_data_s hsrv_openflow_all_mcast_port_data_t;

hsrv_openflow_flow_master_t *g_pst_openflow_flow_master = NULL;

static int32
hsrv_openflow_reset_flow_info(hsrv_openflow_flow_info_t* p_info);

#define _____INTERNAL_APIs_____

#define _____FLOW_ID_____

static int32
hsrv_openflow_alloc_id(hsrv_openflow_flow_t* p_flow)
{
    p_flow->u.dft.cnst.entry_id = p_flow->flow.flow_id;

    switch(p_flow->flow.key.in_port.type)
    {
    case GLB_OF_PORT_TYPE_NONE:
        p_flow->u.dft.cnst.label_id = HSRV_OPENFLOW_GLOBAL_LABEL_BASE;
        break;

    case GLB_OF_PORT_TYPE_PHYSICAL:
        HSRV_IF_ERROR_RETURN(hsrv_openflow_port_get_label(
            p_flow->flow.key.in_port.ifindex, &p_flow->u.dft.cnst.label_id));
        break;
    
    case GLB_OF_PORT_TYPE_TUNNEL:
        /* Added by weizj for tunnel */
        p_flow->u.dft.cnst.label_id = HSRV_OPENFLOW_GLOBAL_LABEL_BASE;
        HSRV_IF_ERROR_RETURN(hsrv_openflow_tunnel_add_decap_flow(&p_flow->flow));
        /* End by weizj for tunnel */
        break;
    
    case GLB_OF_PORT_TYPE_BOND:
        HSRV_IF_ERROR_RETURN(hsrv_openflow_port_get_label(
            p_flow->flow.key.in_port.ifindex, &p_flow->u.dft.cnst.label_id));
        break;
    case GLB_OF_PORT_TYPE_AGG:
        HSRV_IF_ERROR_RETURN(hsrv_openflow_port_get_label(
            p_flow->flow.key.in_port.ifindex, &p_flow->u.dft.cnst.label_id));
        break;
    case GLB_OF_PORT_TYPE_OF_RSVD:
        /* Added by weizj for inband */
        #ifdef V580_OPENFLOW
        HSRV_IF_ERROR_RETURN(hsrv_openflow_inband_get_iloop_port_label(
            p_flow->flow.key.in_port.ofport, &p_flow->u.dft.cnst.label_id));
        #endif
        break;
    default:
        break;
    }

    /* allocate flow stats */
    if (p_flow->flow.stats_en)
    {
        p_flow->u.dft.cnst.stats_id = ctclib_opf_alloc_offset(g_pst_openflow_stats_master->pst_openflow_stats_id_opf , 0, 1);
    }

    return HSRV_E_NONE;
}

static int32
hsrv_openflow_release_id(hsrv_openflow_flow_t* p_flow)
{
    /* free flow stats */
    if (p_flow->flow.stats_en)
    {
        ctclib_opf_free_offset(g_pst_openflow_stats_master->pst_openflow_stats_id_opf , 0,1, p_flow->u.dft.cnst.stats_id);
    }

    /* Added by weizj for tunnel */
    HSRV_IF_ERROR_RETURN(hsrv_openflow_tunnel_del_decap_flow(&p_flow->flow));

    return HSRV_E_NONE;
}

#define _____FLOW_INFO_____

static int32
hsrv_openflow_alloc_flow_info(glb_openflow_flow_t* p_msg_flow, hsrv_openflow_flow_t** pp_flow)
{
    hsrv_openflow_flow_t* p_flow;
    
    p_flow = HSRV_OPENFLOW_FLOW_MALLOC(sizeof(hsrv_openflow_flow_t));
    HSRV_OPENFLOW_MEM_CHECK(p_flow);

    sal_memcpy(&p_flow->flow, p_msg_flow, sizeof(glb_openflow_flow_t));
    sal_memset(&p_flow->u, 0, sizeof(hsrv_openflow_flow_info_t));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_reset_flow_info(&p_flow->u));

    *pp_flow = p_flow;
    
    return HSRV_E_NONE;
}

static int32
hsrv_openflow_lookup_flow_info(glb_openflow_flow_t* p_msg_flow, hsrv_openflow_flow_t** pp_flow)
{
    hsrv_openflow_flow_t* p_flow;
    glb_openflow_flow_key_t key;
    
    HSRV_IF_ERROR_RETURN(hsrv_openflow_flowdb_lookup_flow(p_msg_flow->flow_id, &p_flow));
    if (NULL == p_flow)
    {
        return HSRV_E_INVALID_PARAM;
    }

    /* flow action may be changed, so we need release old actions */
    if (p_flow->flow.actlist.actlist)
    {
        HSRV_OPENFLOW_ACT_FREE(p_flow->flow.actlist.actlist);
    }

    /*tunnel should use hsrv flow key, some para created in hsrv*/
    sal_memcpy(&key, &(p_flow->flow.key), sizeof(glb_openflow_flow_key_t));
    sal_memcpy(&p_flow->flow, p_msg_flow, sizeof(glb_openflow_flow_t));
    sal_memcpy(&p_flow->flow.key, &key, sizeof(glb_openflow_flow_key_t));
    *pp_flow = p_flow;

    return HSRV_E_NONE;
}

static int32
hsrv_openflow_release_flow_info(hsrv_openflow_flow_t* p_flow)
{
    if (p_flow->flow.actlist.actlist)
    {
        HSRV_OPENFLOW_ACT_FREE(p_flow->flow.actlist.actlist);
    }
    HSRV_OPENFLOW_FLOW_FREE(p_flow);
    
    return HSRV_E_NONE;
}

static int32
hsrv_openflow_reset_flow_info(hsrv_openflow_flow_info_t* p_info)
{
    sal_memset(&p_info->dft.var, 0, sizeof(hsrv_openflow_dft_variable_info_t));
    p_info->dft.var.group_info.group_id = OFP_GROUP_INVALID_GROUP_ID;
    p_info->dft.var.meter_info.meter_id = OFP_METER_INVALID_METER_ID;
    p_info->dft.var.queue_info.queue_id = OFP_QUEUE_INVALID_QUEUE_ID;

    p_info->dft.var.old_truncate_len = 0;
    p_info->dft.var.truncate_len = 0;
    p_info->dft.var.truncate_output_port = 0;
    p_info->dft.var.strip_len = 0;
    return HSRV_E_NONE;
}


static int32
hsrv_openflow_move_flow_info(hsrv_openflow_flow_info_t* p_from, hsrv_openflow_flow_info_t* p_to)
{
    memcpy(p_to, p_from, sizeof(hsrv_openflow_flow_info_t));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_reset_flow_info(p_from));

    return HSRV_E_NONE;
}

#define _____FLOW_ACTION_____

static int32
hsrv_openflow_extract_action(hsrv_openflow_flow_t* p_flow)
{
    int32 count = 0;
    glb_openflow_flow_action_t* p_action;
    glb_openflow_flow_action_t* tmp_action;
    uint32 ifindex;
    uint16 vlanid = 0;
    uint16 vlan_cos = 0;
    uint32 group_vlan_type = 0;
    
    for (count = 0; count < p_flow->flow.actlist.action_count; count ++)
    {
        p_action = p_flow->flow.actlist.actlist + count;

        if (GLB_OPENFLOW_ACTION_TYPE_OUTPUT == p_action->type)
        {
            p_flow->u.dft.var.output_count++;
            if (GLB_OFPP_NORMAL == p_action->value.output.ofport)
            {
                p_flow->u.dft.var.normal = TRUE;
            }
        }
        else if (GLB_OPENFLOW_ACTION_TYPE_INST_METER == p_action->type)
        {
            p_flow->u.dft.var.meter_info.meter_id = p_action->value.u32;
        }
        /*added by hansf for support set vlan action before group*/
        else if (GLB_OPENFLOW_ACTION_TYPE_SET_VLAN_VID == p_action->type)
        {
            vlanid = p_action->value.u16;
            group_vlan_type |= GLB_GROUP_VLAN_SET;
        }
        else if (GLB_OPENFLOW_ACTION_TYPE_PUSH_VLAN == p_action->type)
        {
            group_vlan_type |= GLB_GROUP_VLAN_PUSH;
        }
        else if (GLB_OPENFLOW_ACTION_TYPE_STRIP_VLAN == p_action->type)
        {
            group_vlan_type |= GLB_GROUP_VLAN_POP;
        }
        else if (GLB_OPENFLOW_ACTION_TYPE_SET_VLAN_PCP == p_action->type)
        {
            vlan_cos = p_action->value.u8;
            group_vlan_type |= GLB_GROUP_VLAN_SET_PCP;
        }
        /*end hansf*/
        else if (GLB_OPENFLOW_ACTION_TYPE_GROUP == p_action->type)
        {
            p_flow->u.dft.var.group_info.group_id = p_action->value.u32;
            /*added by hansf for support set vlan action before group*/
            p_flow->u.dft.var.group_vlanid = vlanid;
            p_flow->u.dft.var.group_vlan_cos = vlan_cos;
            p_flow->u.dft.var.group_vlan_type = group_vlan_type;
        }
        else if (GLB_OPENFLOW_ACTION_TYPE_SET_QUEUE == p_action->type)
        {
            p_flow->u.dft.var.queue_info.queue_id = p_action->value.u32;
        }
        else if (GLB_OPENFLOW_ACTION_TYPE_SET_TRUNCATE == p_action->type)
        {
            if ((count + 1) < p_flow->flow.actlist.action_count)
            {
                /* truncate action should be followed by output directly */
                tmp_action = p_flow->flow.actlist.actlist + count + 1;
                if (GLB_OPENFLOW_ACTION_TYPE_OUTPUT == tmp_action->type)
                {
                    ifindex = tmp_action->value.output.ifindex;
                    if (!hsrv_ifdb_get_portid(ifindex, &p_flow->u.dft.var.truncate_output_port))
                    {
                        p_flow->u.dft.var.truncate_len = p_action->value.u32;
                        count++;
                    }
                }
            }
        }
        else if (GLB_OPENFLOW_ACTION_TYPE_SET_MPLS_LABEL == p_action->type)
        {
             p_flow->u.dft.var.pw_label = p_action->value.u32;
        }
        else if (GLB_OPENFLOW_ACTION_TYPE_SET_STRIP_HEADER == p_action->type)
        {
             p_flow->u.dft.var.strip_len = p_action->value.u32;
        }
    }
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_map_combo_action(glb_openflow_flow_action_t *p_action, hsrv_openflow_action_combo_t *p_action_combo)
{
    switch (p_action->type)
    {
    case GLB_OPENFLOW_ACTION_TYPE_OUTPUT:
        GLB_OFPACT_FLD_SET(p_action_combo->flag, OUTPUT);
        break;
    
    case GLB_OPENFLOW_ACTION_TYPE_SET_VLAN_VID:
        p_action_combo->vlan_id = p_action->value.u16;
        /* mod by hansf for support pop_vlan+set_vlan */
        if (GLB_OFPACT_FLD_ISSET(p_action_combo->flag, STRIP_SVLAN))
        {
            GLB_OFPACT_FLD_SET(p_action_combo->flag, CVLAN_VID);
            p_action_combo->cvlan_id = p_action->value.u16;
        }
        else
        {
            GLB_OFPACT_FLD_SET(p_action_combo->flag, SVLAN_VID);
        }
        break;
    
    case GLB_OPENFLOW_ACTION_TYPE_SET_VLAN_PCP:
        p_action_combo->vlan_pcp = p_action->value.u8;
        /* mod by hansf for support pop_vlan+set_vlan */
        if (GLB_OFPACT_FLD_ISSET(p_action_combo->flag, STRIP_SVLAN))
        {
            GLB_OFPACT_FLD_SET(p_action_combo->flag, CVLAN_COS);
            p_action_combo->cvlan_pcp = p_action->value.u8;
        }
        else
        {
            GLB_OFPACT_FLD_SET(p_action_combo->flag, SVLAN_COS);
        }
        break;
    
    case GLB_OPENFLOW_ACTION_TYPE_STRIP_VLAN:
        /* Take "mod_vlan_vid,strip_vlan" as "strip_vlan" */
        if (GLB_OFPACT_FLD_ISSET(p_action_combo->flag, STRIP_SVLAN))
        {
            GLB_OFPACT_FLD_UNSET(p_action_combo->flag, CVLAN_VID);
            GLB_OFPACT_FLD_UNSET(p_action_combo->flag, CVLAN_COS);
            GLB_OFPACT_FLD_SET(p_action_combo->flag, STRIP_CVLAN);
        }
        else
        {
            GLB_OFPACT_FLD_UNSET(p_action_combo->flag, SVLAN_VID);
            GLB_OFPACT_FLD_UNSET(p_action_combo->flag, SVLAN_COS);
            GLB_OFPACT_FLD_SET(p_action_combo->flag, STRIP_SVLAN);
        }
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_ETH_SRC:
        memcpy(p_action_combo->mac_sa, p_action->value.mac, sizeof(mac_addr_t));
        GLB_OFPACT_FLD_SET(p_action_combo->flag, MACSA);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_ETH_DST:
        memcpy(p_action_combo->mac_da, p_action->value.mac, sizeof(mac_addr_t));
        GLB_OFPACT_FLD_SET(p_action_combo->flag, MACDA);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_TUNNEL:
        p_action_combo->tunnel_id = p_action->value.u32;
        GLB_OFPACT_FLD_SET(p_action_combo->flag, TUNNEL_ID);
        break;
    
    case GLB_OPENFLOW_ACTION_TYPE_PUSH_VLAN:
        if (GLB_OFPACT_FLD_ISSET(p_action_combo->flag, PUSH_SVLAN))
        {
            /* Take the first pushed vlan as customer vlan tag */
            GLB_OFPACT_FLD_SET(p_action_combo->flag, PUSH_CVLAN);
            if (GLB_OFPACT_FLD_ISSET(p_action_combo->flag, SVLAN_VID))
            {
                GLB_OFPACT_FLD_UNSET(p_action_combo->flag, SVLAN_VID);
                GLB_OFPACT_FLD_SET(p_action_combo->flag, CVLAN_VID);
                p_action_combo->cvlan_id  = p_action_combo->vlan_id;
                p_action_combo->ctag_tpid = p_action_combo->stag_tpid;
                p_action_combo->stag_tpid = p_action->value.u16;
                p_action_combo->vlan_id = 0;
            }
            if (GLB_OFPACT_FLD_ISSET(p_action_combo->flag, SVLAN_COS))
            {
                GLB_OFPACT_FLD_UNSET(p_action_combo->flag, SVLAN_COS);
                GLB_OFPACT_FLD_SET(p_action_combo->flag, CVLAN_COS);
                p_action_combo->cvlan_pcp = p_action_combo->vlan_pcp;
                p_action_combo->vlan_pcp = 0;
            }
        }
        else
        {
            GLB_OFPACT_FLD_SET(p_action_combo->flag, PUSH_SVLAN);
            p_action_combo->stag_tpid = p_action->value.u16;
        }
        break;
    
    case GLB_OPENFLOW_ACTION_TYPE_INST_METER:
        p_action_combo->meter_id = p_action->value.u32;
        GLB_OFPACT_FLD_SET(p_action_combo->flag, METER);
        break;
    
    case GLB_OPENFLOW_ACTION_TYPE_SET_QUEUE:
        p_action_combo->queue_id = p_action->value.u32;
        GLB_OFPACT_FLD_SET(p_action_combo->flag, QUEUE_ID);
        break;
    
    case GLB_OPENFLOW_ACTION_TYPE_SET_IPV4_SRC:
        p_action_combo->l3.ipv4.ipsa = p_action->value.u32;
        GLB_OFPACT_FLD_SET(p_action_combo->flag, IPV4_);
        GLB_OFPACT_FLD_SET(p_action_combo->flag, IPSA);
        break;
    
    case GLB_OPENFLOW_ACTION_TYPE_SET_IPV4_DST:
        p_action_combo->l3.ipv4.ipda = p_action->value.u32;
        GLB_OFPACT_FLD_SET(p_action_combo->flag, IPV4_);
        GLB_OFPACT_FLD_SET(p_action_combo->flag, IPDA);
        break;

    case GLB_OPENFLOW_ACTION_TYPE_SET_TP_SRC:
        p_action_combo->l4.l4port.src_port = p_action->value.u16;
        GLB_OFPACT_FLD_SET(p_action_combo->flag, SPORT);
        break;
    
    case GLB_OPENFLOW_ACTION_TYPE_SET_TP_DST:
        p_action_combo->l4.l4port.dst_port = p_action->value.u16;
        GLB_OFPACT_FLD_SET(p_action_combo->flag, DPORT);
        break;

    case GLB_OPENFLOW_ACTION_TYPE_SET_TCP_SRC:
        p_action_combo->l4.l4port.src_port = p_action->value.u16;
        GLB_OFPACT_FLD_SET(p_action_combo->flag, TCP_);
        GLB_OFPACT_FLD_SET(p_action_combo->flag, SPORT);
        break;
    
    case GLB_OPENFLOW_ACTION_TYPE_SET_TCP_DST:
        p_action_combo->l4.l4port.dst_port = p_action->value.u16;
        GLB_OFPACT_FLD_SET(p_action_combo->flag, TCP_);
        GLB_OFPACT_FLD_SET(p_action_combo->flag, DPORT);
        break;

    case GLB_OPENFLOW_ACTION_TYPE_SET_UDP_SRC:
        p_action_combo->l4.l4port.src_port = p_action->value.u16;
        GLB_OFPACT_FLD_SET(p_action_combo->flag, UDP_);
        GLB_OFPACT_FLD_SET(p_action_combo->flag, SPORT);
        break;
    
    case GLB_OPENFLOW_ACTION_TYPE_SET_UDP_DST:
        p_action_combo->l4.l4port.dst_port = p_action->value.u16;
        GLB_OFPACT_FLD_SET(p_action_combo->flag, UDP_);
        GLB_OFPACT_FLD_SET(p_action_combo->flag, DPORT);
        break;
    
    case GLB_OPENFLOW_ACTION_TYPE_SET_SCTP_SRC:
        p_action_combo->l4.l4port.src_port = p_action->value.u16;
        GLB_OFPACT_FLD_SET(p_action_combo->flag, SCTP_);
        GLB_OFPACT_FLD_SET(p_action_combo->flag, SPORT);
        break;
    
    case GLB_OPENFLOW_ACTION_TYPE_SET_SCTP_DST:
        p_action_combo->l4.l4port.dst_port = p_action->value.u16;
        GLB_OFPACT_FLD_SET(p_action_combo->flag, SCTP_);
        GLB_OFPACT_FLD_SET(p_action_combo->flag, DPORT);
        break;
    
    case GLB_OPENFLOW_ACTION_TYPE_DEC_IP_TTL:
        GLB_OFPACT_FLD_SET(p_action_combo->flag, DEC_IP_TTL);
        /* Added by weizj for tunnel bug 33429 */
        if (GLB_OFPACT_FLD_ISSET(p_action_combo->flag, IP_TTL_))
        {
            GLB_OFPACT_FLD_UNSET(p_action_combo->flag, IP_TTL_);
        }
        break;
    
    case GLB_OPENFLOW_ACTION_TYPE_SET_IP_ECN:
        p_action_combo->l3.ip.ecn = p_action->value.u8;
        GLB_OFPACT_FLD_SET(p_action_combo->flag, IP_ECN);
        break;
    
    case GLB_OPENFLOW_ACTION_TYPE_SET_IP_DSCP:
        p_action_combo->l3.ip.dscp = p_action->value.u8;
        GLB_OFPACT_FLD_SET(p_action_combo->flag, IP_DSCP);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_DEC_TTL:
        GLB_OFPACT_FLD_SET(p_action_combo->flag, DEC_TTL);
        /* Added by weizj for tunnel bug 33429 */
        if (GLB_OFPACT_FLD_ISSET(p_action_combo->flag, IP_TTL_))
        {
            GLB_OFPACT_FLD_UNSET(p_action_combo->flag, IP_TTL_);
        }
        break;
    
    case GLB_OPENFLOW_ACTION_TYPE_SET_IP_TTL:
        p_action_combo->l3.ip.ttl = p_action->value.u8;
        GLB_OFPACT_FLD_SET(p_action_combo->flag, IP_TTL_);
        /* Added by weizj for tunnel bug 33429 */
        if (GLB_OFPACT_FLD_ISSET(p_action_combo->flag, DEC_IP_TTL))
        {
            GLB_OFPACT_FLD_UNSET(p_action_combo->flag, DEC_IP_TTL);
        }
        if (GLB_OFPACT_FLD_ISSET(p_action_combo->flag, DEC_TTL))
        {
            GLB_OFPACT_FLD_UNSET(p_action_combo->flag, DEC_TTL);
        }
        break;
    
    case GLB_OPENFLOW_ACTION_TYPE_SET_IP_PROTO:
        p_action_combo->l3.ip.proto = p_action->value.u8;
        GLB_OFPACT_FLD_SET(p_action_combo->flag, IP_PROTO);
        break;
    
    case GLB_OPENFLOW_ACTION_TYPE_DEC_MPLS_TTL:
        GLB_OFPACT_FLD_SET(p_action_combo->flag, MPLS_);
        GLB_OFPACT_FLD_SET(p_action_combo->flag, DEC_TTL);
        break;
    
    case GLB_OPENFLOW_ACTION_TYPE_SET_IPV6_SRC:
        sal_memcpy(&p_action_combo->l3.ipv6.ipsa, &p_action->value.ipv6, sizeof(ipv6_addr_t));
        GLB_OFPACT_FLD_SET(p_action_combo->flag, IPV6_);
        GLB_OFPACT_FLD_SET(p_action_combo->flag, IPSA);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_IPV6_DST:
        sal_memcpy(&p_action_combo->l3.ipv6.ipda, &p_action->value.ipv6, sizeof(ipv6_addr_t));
        GLB_OFPACT_FLD_SET(p_action_combo->flag, IPV6_);
        GLB_OFPACT_FLD_SET(p_action_combo->flag, IPDA);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_IPV6_LABEL:
        p_action_combo->l3.ipv6.label = p_action->value.u32;
        GLB_OFPACT_FLD_SET(p_action_combo->flag, IPV6_);
        GLB_OFPACT_FLD_SET(p_action_combo->flag, IPV6_LBL);
        break;

    case GLB_OPENFLOW_ACTION_TYPE_SET_ARP_OP:
        p_action_combo->l3.arp.arp_op = p_action->value.u16;
        GLB_OFPACT_FLD_SET(p_action_combo->flag, ARP_OP);
        break;
    
    case GLB_OPENFLOW_ACTION_TYPE_SET_ARP_SPA:
        p_action_combo->l3.arp.arp_spa = p_action->value.u32;
        GLB_OFPACT_FLD_SET(p_action_combo->flag, ARP_SPA);
        break;
    
    case GLB_OPENFLOW_ACTION_TYPE_SET_ARP_TPA:
        p_action_combo->l3.arp.arp_tpa = p_action->value.u32;
        GLB_OFPACT_FLD_SET(p_action_combo->flag, ARP_TPA);
        break;
    
    case GLB_OPENFLOW_ACTION_TYPE_SET_ARP_SHA:
        memcpy(p_action_combo->l3.arp.arp_sha, p_action->value.mac, sizeof(mac_addr_t));
        GLB_OFPACT_FLD_SET(p_action_combo->flag, ARP_SHA);
        break;
    
    case GLB_OPENFLOW_ACTION_TYPE_SET_ARP_THA:
        memcpy(p_action_combo->l3.arp.arp_tha, p_action->value.mac, sizeof(mac_addr_t));
        GLB_OFPACT_FLD_SET(p_action_combo->flag, ARP_THA);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_ETH_TYPE:
        p_action_combo->eth_type = p_action->value.u16;
        GLB_OFPACT_FLD_SET(p_action_combo->flag, ETH_TYPE);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_ICMPV4_TYPE:
        p_action_combo->l4.icmp.type = p_action->value.u8;
        GLB_OFPACT_FLD_SET(p_action_combo->flag, IPV4_);
        GLB_OFPACT_FLD_SET(p_action_combo->flag, ICMP_TYPE);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_ICMPV4_CODE:
        p_action_combo->l4.icmp.code = p_action->value.u8;
        GLB_OFPACT_FLD_SET(p_action_combo->flag, IPV4_);
        GLB_OFPACT_FLD_SET(p_action_combo->flag, ICMP_CODE);
        break;

    case GLB_OPENFLOW_ACTION_TYPE_SET_ICMPV6_TYPE:
        p_action_combo->l4.icmp.type = p_action->value.u8;
        GLB_OFPACT_FLD_SET(p_action_combo->flag, IPV6_);
        GLB_OFPACT_FLD_SET(p_action_combo->flag, ICMP_TYPE);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_ICMPV6_CODE:
        p_action_combo->l4.icmp.code = p_action->value.u8;
        GLB_OFPACT_FLD_SET(p_action_combo->flag, IPV6_);
        GLB_OFPACT_FLD_SET(p_action_combo->flag, ICMP_CODE);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_WRITE_METADATA:
    case GLB_OPENFLOW_ACTION_TYPE_PUSH_L2:
    case GLB_OPENFLOW_ACTION_TYPE_POP_L2:
    case GLB_OPENFLOW_ACTION_TYPE_PUSH_MPLS:
    case GLB_OPENFLOW_ACTION_TYPE_POP_MPLS:
    case GLB_OPENFLOW_ACTION_TYPE_SET_MPLS_LABEL:
    case GLB_OPENFLOW_ACTION_TYPE_SET_MPLS_TC:
    case GLB_OPENFLOW_ACTION_TYPE_SET_MPLS_TTL:
    case GLB_OPENFLOW_ACTION_TYPE_GROUP:
    case GLB_OPENFLOW_ACTION_TYPE_UNSUPPORTED:
    default:
        break;
    }    
    
    return HSRV_E_NONE;
}
/* Modifed by weizj for bug 35135 */
static int32 
hsrv_openflow_add_phy_and_agg_port_to_mcast(uint32 ifindex, void* pv_arg)
{
    hsrv_openflow_all_mcast_port_data_t* p_data = pv_arg;
    hsrv_openflow_action_combo_t* p_action_combo = p_data->p_action;
    hsrv_of_nh_info_t* p_member_nh_array = p_data->p_member_nh_array;
    uint32* p_member_cnt = p_data->p_member_cnt;
    hsrv_ofnh_flex_param_t param;
    uint16 gport;

    if ( ! hsrv_openflow_port_is_enabled(ifindex)
        || ! hsrv_openflow_port_is_add_to_bridge(ifindex)
        || hsrv_openflow_port_is_slave(ifindex))
    {
        return HSRV_E_NONE;
    }
    HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_agg_portid(ifindex, &gport));
    GLB_OFPACT_FLD_SET(p_action_combo->flag, OUTPUT);
    p_action_combo->output_gport = gport;
    
    param.p_action_combo = p_action_combo;
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_alloc_flex_nh(&param, 
        &p_member_nh_array[*p_member_cnt]));
    
    p_member_nh_array[*p_member_cnt].nh_offset.port_check_discard = TRUE;
    p_member_nh_array[*p_member_cnt].nh_offset.redirect_to_ingress_port = FALSE;
    p_member_nh_array[*p_member_cnt].gport = gport;
    p_member_nh_array[*p_member_cnt].to_all = TRUE;
    *p_member_cnt = *p_member_cnt + 1;
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_map_all_to_mcast_members(hsrv_openflow_action_combo_t* p_action, 
                                          hsrv_of_nh_info_t* p_member_nh_array,
                                          uint32* p_member_cnt)
{
    hsrv_openflow_all_mcast_port_data_t data;

    sal_memset(&data, 0, sizeof(hsrv_openflow_all_mcast_port_data_t));
    data.p_action = p_action;
    data.p_member_nh_array = p_member_nh_array;
    data.p_member_cnt = p_member_cnt;
    /* physical port */
    hsrv_ifdb_do_listloop(GLB_IF_TYPE_PORT_IF, hsrv_openflow_add_phy_and_agg_port_to_mcast, &data);

    return HSRV_E_NONE;    
}

static int32
hsrv_openflow_preproccess_combo_action(hsrv_openflow_map_output_data_t* p_data, hsrv_openflow_action_combo_t* p_action_combo)
{
    if (ETH_TYPE_IP == p_data->eth_type)
    {
        GLB_OFPACT_FLD_SET(p_action_combo->flag, IPV4_);
    }
    else if (ETH_TYPE_ARP == p_data->eth_type)
    {
        GLB_OFPACT_FLD_SET(p_action_combo->flag, ARP_);
    }
    else if (ETH_TYPE_IPV6 == p_data->eth_type)
    {
        GLB_OFPACT_FLD_SET(p_action_combo->flag, IPV6_);
    }
    if (IP_PROTO_TCP == p_data->ip_proto)
    {
        GLB_OFPACT_FLD_SET(p_action_combo->flag, TCP_);
    }
    else if (IP_PROTO_UDP == p_data->ip_proto)
    {
        GLB_OFPACT_FLD_SET(p_action_combo->flag, UDP_);
    }
    
    return HSRV_E_NONE;    
}

int32
hsrv_openflow_map_output_action_to_mcast_members(hsrv_openflow_map_output_data_t* p_data,
          hsrv_of_nh_info_t* p_member_nh_array,
          uint32* p_member_cnt)
{
    glb_openflow_flow_action_t* p_action;
    hsrv_openflow_action_combo_t action_combo;
    hsrv_ofnh_cpu_flex_param_t param;
    bool output_to_controller = FALSE;
    bool output_to_local = FALSE;
    bool is_in_port = FALSE;
    bool port_check_discard = FALSE;
    uint32 ifindex;
    uint32 ofport;
    int32 count;
    bool is_truncate = FALSE;
    
    memset(&action_combo, 0, sizeof(action_combo));
    action_combo.flow_tunnel_type = p_data->flow_tunnel_type;
    action_combo.payload_packet_type = p_data->payload_packet_type;
    HSRV_IF_ERROR_RETURN(hsrv_openflow_preproccess_combo_action(p_data, &action_combo));
    
    for (count = 0; count < p_data->p_actlist->action_count; count ++)
    {
        p_action = p_data->p_actlist->actlist + count;
        
        HSRV_IF_ERROR_RETURN(hsrv_openflow_map_combo_action(p_action, &action_combo));

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
        
        ifindex = p_action->value.output.ifindex;
        ofport = p_action->value.output.ofport;
        is_in_port = FALSE;
        
        if (GLB_OF_PORT_TYPE_PHYSICAL == p_action->value.output.type)
        {
            
            hsrv_ofnh_flex_param_t param;
            
            param.p_action_combo = &action_combo;
            if (p_action->value.output.tid)
            {
                action_combo.output_linkagg_gport = CTC_MAP_TID_TO_GPORT(p_action->value.output.tid);
            }
            HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(ifindex, &action_combo.output_gport));
            HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_alloc_flex_nh(&param, &p_member_nh_array[*p_member_cnt]));
            if(!check_feature_support(CONFIG_FEATURE_HYBRID_FORCE_BACK))
            {
                if(p_data->in_port == ofport)
                {
                    is_in_port = TRUE;
                }
            }
        }
        else if (GLB_OF_PORT_TYPE_TUNNEL == p_action->value.output.type)
        {
            /* Added by weizj for tunnel */
            hsrv_ofnh_tunnel_param_t param;

            sal_memset(&param, 0, sizeof(hsrv_ofnh_tunnel_param_t));
            param.p_action_combo = &action_combo;
            param.tnl.tunnel_ifindex = ifindex;
            HSRV_IF_ERROR_RETURN(hsrv_openflow_tunnel_alloc_tunnel_nh(&param, &p_member_nh_array[*p_member_cnt]));
            /* End by weizj for tunnel */
        }
        else if (GLB_OF_PORT_TYPE_BOND == p_action->value.output.type)
        {
            hsrv_ofnh_flex_param_t param;
            
            param.p_action_combo = &action_combo;
            HSRV_IF_ERROR_RETURN(hsrv_openflow_bond_get_gport_by_ifindex(ifindex, &action_combo.output_gport));
            HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_alloc_flex_nh(&param, &p_member_nh_array[*p_member_cnt]));
        }
        else if (GLB_OF_PORT_TYPE_AGG == p_action->value.output.type)
        {
            hsrv_ofnh_flex_param_t param;
            
            param.p_action_combo = &action_combo;
            HSRV_IF_ERROR_RETURN(hsrv_openflow_bond_get_gport_by_ifindex(ifindex, &action_combo.output_gport));
            HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_alloc_flex_nh(&param, &p_member_nh_array[*p_member_cnt]));
        }
        else if (GLB_OFPP_CONTROLLER == ofport)
        {
            output_to_controller = TRUE;
            continue;
        }
        else if (GLB_OFPP_LOCAL == ofport)
        {
            output_to_local = TRUE;
            continue;
        }
        else if(GLB_OFPP_IN_PORT == ofport)
        {
            if(check_feature_support(CONFIG_FEATURE_HYBRID_FORCE_BACK))
            {
                is_in_port = TRUE;

                hsrv_ofnh_flex_param_t param;
                
                param.p_action_combo = &action_combo;
                /* Use OF spec IN_PORT to differentiate nexthop, this gport will not collide
                   with any physical/aggregation/cpu port because its bigger value.
                   In hagt we will translate it to cpu port to make SDK happy, however the
                   cpu port is actually not used.
                   . */
                action_combo.output_gport = GLB_OFPP_IN_PORT;
                HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_alloc_flex_nh(&param, &p_member_nh_array[*p_member_cnt]));
            }
            else
            {
                is_in_port = TRUE;
                hsrv_ofnh_flex_param_t param;
                
                param.p_action_combo = &action_combo;
                hsrv_ifdb_get_portid(p_data->ifindex, &action_combo.output_gport);
                HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_alloc_flex_nh(&param, &p_member_nh_array[*p_member_cnt]));
            }
        }
        else if(GLB_OFPP_ALL == ofport)
        {
            HSRV_IF_ERROR_RETURN(hsrv_openflow_map_all_to_mcast_members(
                &action_combo,
                p_member_nh_array,
                p_member_cnt));
            continue;
        }
        /* Added by weizj for tunnel bug 33724
         * For output to physcial port, if user set drop_pkt_to_ingress_port
         * to true, we should enable the check in the ASIC.
         * Modifed by weizj for bug 35135 */
        if (p_data->is_group)
        {
            if (hsrv_openflow_group_get_drop_ingress())
            {
                port_check_discard = TRUE;
            }
        }
        else
        {
            if (hsrv_openflow_flowdb_get_drop_ingress())
            {
                port_check_discard = TRUE;
            }
        }

        p_member_nh_array[*p_member_cnt].nh_offset.port_check_discard = port_check_discard;
        p_member_nh_array[*p_member_cnt].nh_offset.redirect_to_ingress_port = is_in_port;
        p_member_nh_array[*p_member_cnt].gport = action_combo.output_gport;
        p_member_nh_array[*p_member_cnt].tid = p_action->value.output.tid;
        /* Added by weizj for bug 36895 */
        p_member_nh_array[*p_member_cnt].ifindex = ifindex;

        (*p_member_cnt) ++;
    }
    
    param.flow_id = p_data->flow_id;
    param.need_dsfwd = FALSE;
    param.table_id = 0;
    
    if (output_to_controller)
    {
        HSRV_IF_ERROR_RETURN(
            hsrv_openflow_nexthop_alloc_cpu_flex_nh(&param, 
                &p_member_nh_array[*p_member_cnt]));

        (*p_member_cnt) ++;
    }
    
    if (output_to_local)
    {
        /* Added by weizj for inband */
        param.is_local = TRUE;

        HSRV_IF_ERROR_RETURN(
            hsrv_openflow_nexthop_alloc_cpu_flex_nh(&param, 
                &p_member_nh_array[*p_member_cnt]));
        
        (*p_member_cnt) ++;
    }
    
    return HSRV_E_NONE;
}

static int32
hsrv_openflow_map_output_action(hsrv_openflow_flow_t* p_flow)
{
    hsrv_openflow_nexthop_info_t* p_nh_info = NULL;
    hsrv_of_nh_offset_t group_nh;
    hsrv_openflow_map_output_data_t output_data;
    hsrv_ofnh_mcast_param_t mcast_param;
    uint32 member_idx = 0;

    uint32 output_num = 0;
    uint32 is_include = 0;

    p_nh_info = &p_flow->u.dft.var.nh_info;
    if (0 == p_flow->u.dft.var.output_count)
    {
        p_nh_info->main_nh.nh_offset.nhid = HSRV_OF_NH_RESV_NHID_FOR_DROP;
        p_nh_info->main_nh.nh_offset.offset = 0;
        p_nh_info->main_nh.nh_offset.nh_type = HSRV_OF_NH_TYPE_NH_DROP;
        p_nh_info->use_mcast       = FALSE;
        return HSRV_E_NONE;
    }

    /* for cubro mpls. */
    if ((p_flow->flow.u.dft.mpls_output_action_seq_type != GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_VALID_MAX) &&
        (p_flow->flow.u.dft.mpls_output_action_seq_type != GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_POP_ALL_MPLS_0)/* pop all mpls treated as normal flow action*/)
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_mpls_map_flow_output_action(p_flow));
        return HSRV_E_NONE;
    }
    sal_memset(&output_data, 0, sizeof(hsrv_openflow_map_output_data_t));
    output_data.flow_id = p_flow->flow.flow_id;
    output_data.in_port = p_flow->flow.key.in_port.ofport;
    output_data.ifindex= p_flow->flow.key.in_port.ifindex;
    output_data.p_actlist = &p_flow->flow.actlist;
    /* Added by weizj for bug 35135 */
    output_data.is_group = 0;
    output_data.flow_tunnel_type = p_flow->flow.flow_tnl_type;
    output_data.payload_packet_type = p_flow->flow.payload_packet_type;
    if (p_flow->flow.key.mac.flags.ipv4_packet)
    {
        output_data.eth_type = ETH_TYPE_IP;
    }
    else if(GLB_OF_FLOW_RULE_TYPE_IPV6 == p_flow->flow.key.rule_type)   /* added by hansf for support ipv6 action */
    {
        output_data.eth_type = ETH_TYPE_IPV6;
    }
    else if (p_flow->flow.key.mac.flags.arp_packet)
    {
        output_data.eth_type = ETH_TYPE_ARP;
    }
    else if (p_flow->flow.key.mac.flags.eth_type)
    {
        output_data.eth_type = p_flow->flow.key.mac.eth_type;
    }
    if (p_flow->flow.key.rule.ipv4_rule.flags.l4_protocol)
    {
        output_data.ip_proto = p_flow->flow.key.rule.ipv4_rule.l4_protocol;
    }

    hsrv_openflow_map_out_action_include_dynamic_tunnel_port(&output_data, &output_num, &is_include);
    if(is_include > 0)
    {
        p_nh_info->use_dynamic_tunnel = TRUE;
        if(output_num > 1)
        {
            HSRV_IF_ERROR_RETURN(hsrv_tunneldb_cdb_oper_function(HSRV_TUNNEL_CDB_FLOW_ADD_MULTI_DYNAMIC_TUNNEL, p_flow));
        }
        else
        {
            HSRV_IF_ERROR_RETURN(hsrv_tunneldb_cdb_oper_function(HSRV_TUNNEL_CDB_FLOW_ADD_SINGLE_DYNAMIC_TUNNEL, p_flow));
        }

        return HSRV_E_NONE;
    }

    memset(&group_nh, 0, sizeof(hsrv_of_nh_offset_t));
    mcast_param.stats_en = FALSE;
    mcast_param.no_fwd = FALSE;
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_alloc_mcast_group(&mcast_param, &group_nh));
    
    p_nh_info->main_nh.nh_offset.nhid    = group_nh.nhid;
    p_nh_info->main_nh.nh_offset.offset  = group_nh.offset;
    p_nh_info->main_nh.nh_offset.nh_type = group_nh.nh_type;
    p_nh_info->use_mcast      = TRUE;
    

    
    HSRV_IF_ERROR_RETURN(hsrv_openflow_map_output_action_to_mcast_members(
                            &output_data,
                            p_nh_info->member_nh,
                            &p_nh_info->member_count));
    
    for (member_idx = 0; member_idx < p_nh_info->member_count; member_idx ++)
    {
        hsrv_ofnh_mcast_member_param_t member_param;
        hsrv_of_nh_info_t* p_member;

        p_member = &p_nh_info->member_nh[member_idx];
        member_param.group_nhid = group_nh.nhid;
        member_param.op_type = HSRV_OFNH_MCAST_OP_ADD_MEMBER;
        member_param.mem_gport = p_member->gport;
        member_param.mem_nhid = p_member->nh_offset.nhid;
        member_param.port_check_discard = p_member->nh_offset.port_check_discard;
        member_param.redirect_to_ingress_port = p_member->nh_offset.redirect_to_ingress_port;
        
        HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_add_mcast_member(&member_param));
    }

    return HSRV_E_NONE;
}

static int32
hsrv_openflow_unmap_output_action(hsrv_openflow_flow_info_t* p_flow_info)
{
    if (p_flow_info->dft.var.actlist)
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_release_nexthop_info(&p_flow_info->dft.var.actlist->nh_info));
    }
    else
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_release_nexthop_info(&p_flow_info->dft.var.nh_info));
    }
    return HSRV_E_NONE;
}

static int32
hsrv_openflow_map_action(hsrv_openflow_flow_t* p_flow)
{
    hsrv_openflow_nexthop_info_t* p_nh_info = NULL;
    hsrv_actlist_info_t* p_actlist = NULL;
    bool need_extract = TRUE;
    /* modified by liwh for bug 46591 to support mpls tpoam, 2018-04-04 */
    hsrv_openflow_pw_aps_nh_info_t    nh_info;
    /* liwh end */
    
    HSRV_IF_ERROR_RETURN(hsrv_openflow_extract_action(p_flow));

    p_nh_info = &p_flow->u.dft.var.nh_info;
    if (0 == p_flow->flow.actlist.action_count)
    {
        p_nh_info->main_nh.nh_offset.nhid = HSRV_OF_NH_RESV_NHID_FOR_DROP;
        p_nh_info->main_nh.nh_offset.offset = 0;
        p_nh_info->main_nh.nh_offset.nh_type = HSRV_OF_NH_TYPE_NH_DROP;
        p_nh_info->use_mcast       = FALSE;
        return HSRV_E_NONE;
    }
    
    if(hsrv_openflow_flow_action_include_dynamic_tunnel_port(p_flow))
    {
        need_extract = TRUE;
    }
    else
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_flowdb_lookup_flow_actlist(p_flow, &p_actlist));
        if (p_actlist)
        {
            need_extract = FALSE;
        }
    }
    
    if (OFP_GROUP_INVALID_GROUP_ID != p_flow->u.dft.var.group_info.group_id)
    {
        hsrv_openflow_group_t* p_group;
        need_extract = FALSE;
        hsrv_openflow_group_lookup_group(p_flow->u.dft.var.group_info.group_id, &p_group);

        if (p_group)
        {
            /* modified by liwh for bug 46591 to support mpls tpoam, 2018-04-04 */
            if (GLB_OF_GROUP_TYPE_LSP_APS == p_group->group.group_type)
            {
                p_flow->u.dft.var.is_aps_flow = TRUE;
                /*modified by yejl for bug 52155, 2019-05-22*/
                HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_alloc_lsp_aps_nh(
                    p_group->aps_group.tunnel_id, p_flow->u.dft.var.pw_label, 
                    &p_group->aps_group.aps_nhid, &p_group->aps_group.aps_offset,
                    &p_group->aps_group.mcast_nhid, &p_group->aps_group.mcast_offset,
                    p_group->aps_group.lsp_aps_nh_hash));

                #ifdef USW
                p_flow->u.dft.var.group_info.group_nhid = p_group->aps_group.mcast_nhid;
                #else
                p_flow->u.dft.var.group_info.group_nhid = p_group->aps_group.aps_nhid;
                #endif
                /*ended by yejl*/
            }
            else if (GLB_OF_GROUP_TYPE_PW_APS == p_group->group.group_type)
            {
                p_flow->u.dft.var.is_aps_flow = TRUE;

                sal_memset(&nh_info, 0, sizeof(nh_info));

                if (0 == p_group->aps_group.aps_nhid)
                {
                    HSRV_IF_ERROR_RETURN(hsrv_aps_group_offset_alloc(&p_group->aps_group.aps_group_offset));
                    nh_info.aps_id = p_group->aps_group.aps_group_offset;
                    nh_info.tunnel_id = p_group->aps_group.tunnel_id;
                    nh_info.ptunnel_id = p_group->aps_group.ptunnel_id;
                    nh_info.gport = p_group->aps_group.gport;
                    nh_info.pgport = p_group->aps_group.pgport;
                    nh_info.pw_label = p_group->aps_group.pw_label;
                    nh_info.ppw_label = p_group->aps_group.ppw_label;
                    nh_info.lsp_aps_group_id = p_group->aps_group.lsp_aps_group_id;
                    nh_info.plsp_aps_group_id = p_group->aps_group.plsp_aps_group_id;
                    /* modified by yangl for bug 53718 to support mpls tpoam, 2019-10-24 */
                    nh_info.mpls_tc0 = p_group->group.bucket_list[0].mpls_tc0;
                    nh_info.mpls_tc0_is_set = p_group->group.bucket_list[0].mpls_tc0_is_set;
                    nh_info.mpls_ttl0 = p_group->group.bucket_list[0].mpls_ttl0;

                    nh_info.mpls_tc1 = p_group->group.bucket_list[1].mpls_tc0;
                    nh_info.mpls_tc1_is_set = p_group->group.bucket_list[1].mpls_tc0_is_set;
                    nh_info.mpls_ttl1 = p_group->group.bucket_list[1].mpls_ttl0;
                    /*yangl end*/
                    /*modified by yejl for bug 52155, 2019-05-22*/
                    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_alloc_pw_aps_nh(
                        &nh_info, &p_group->aps_group.aps_nhid, &p_group->aps_group.aps_offset, 
                        &p_group->aps_group.mcast_nhid, &p_group->aps_group.mcast_offset));

                    #ifdef USW
                    p_flow->u.dft.var.group_info.group_nhid = p_group->aps_group.mcast_nhid;
                    #else
                    p_flow->u.dft.var.group_info.group_nhid = p_group->aps_group.aps_nhid;
                    #endif
                    /*ended by yejl*/

                     p_group->aps_group.aps_nhid_count++;
                }
                else
                {
                     p_group->aps_group.aps_nhid_count++;
                }
            }
            else
            /* liwh end */
            {
                p_flow->u.dft.var.group_info.group_nhid = p_group->u.dft.nh_info.main_nh.nh_offset.nhid;
                p_flow->u.dft.var.group_info.is_bound = TRUE;
            }
        }
    }

    if (p_flow->u.dft.var.normal)
    {
        need_extract = FALSE;
    }
    
    if (need_extract)
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_map_output_action(p_flow));
    }
    
    if (p_actlist)
    {
        p_actlist->ref_cnt ++;
    }
    else if (need_extract)
    {
        p_actlist = HSRV_OPENFLOW_HSRVACT_MALLOC(sizeof(hsrv_actlist_info_t));
        sal_memset(p_actlist, 0, sizeof(hsrv_actlist_info_t));
        p_actlist->ref_cnt = 1;
        p_actlist->actlist.action_count = p_flow->flow.actlist.action_count;
        p_actlist->actlist.actlist = HSRV_OPENFLOW_HSRVACT_MALLOC(p_flow->flow.actlist.action_count *
                                            sizeof(glb_openflow_flow_action_t));
        sal_memcpy(p_actlist->actlist.actlist,
               p_flow->flow.actlist.actlist,
               p_flow->flow.actlist.action_count *
               sizeof(glb_openflow_flow_action_t));
        sal_memcpy(&p_actlist->mdata.in_port, &p_flow->flow.key.in_port, sizeof(glb_openflow_inport_t));
        p_actlist->mdata.table_id = p_flow->flow.table;
        p_actlist->mdata.eth_type = p_flow->flow.key.mac.eth_type;
        if (p_flow->flow.key.mac.flags.ipv4_packet)
        {
            p_actlist->mdata.eth_type = ETH_TYPE_IP;
            p_actlist->mdata.ip_proto = p_flow->flow.key.rule.ipv4_rule.l4_protocol;
        }
        else if (p_flow->flow.key.mac.flags.arp_packet)
        {
            p_actlist->mdata.eth_type = ETH_TYPE_ARP;
        }
        else
        {
            p_actlist->mdata.eth_type = p_flow->flow.key.mac.eth_type;
        }
        
        sal_memcpy(&p_actlist->nh_info, &p_flow->u.dft.var.nh_info, sizeof(hsrv_openflow_nexthop_info_t));
        
        HSRV_IF_ERROR_RETURN(hsrv_openflow_flowdb_add_flow_actlist(p_actlist));
    }
    p_flow->u.dft.var.actlist = p_actlist;
    
    return HSRV_E_NONE;
}

static int32
hsrv_openflow_unmap_action(hsrv_openflow_flow_info_t* p_flow_info)
{
    hsrv_actlist_info_t* p_actlist;

    p_actlist = p_flow_info->dft.var.actlist;
    if (p_actlist)
    {
        if (0 == p_actlist->ref_cnt)
        {
            HSRV_OPENFLOW_LOG_ERR("Openflow actlist reference count error");
            return HSRV_E_NONE;
        }
        p_actlist->ref_cnt --;
        if (0 == p_actlist->ref_cnt)
        {
            hsrv_openflow_unmap_output_action(p_flow_info);
            hsrv_openflow_flowdb_del_flow_actlist(p_actlist);
            HSRV_OPENFLOW_HSRVACT_FREE(p_actlist->actlist.actlist);
            HSRV_OPENFLOW_HSRVACT_FREE(p_actlist);
        }
        p_flow_info->dft.var.actlist = NULL;
    }
    
    return HSRV_E_NONE;
}

static int32
hsrv_openflow_create_stats(hsrv_openflow_flow_t* p_flow_info)
{
    hsrv_of_stats_t stats;

    if (p_flow_info->flow.stats_en)
    {
        memset(&stats, 0, sizeof(hsrv_of_stats_t));
        stats.resource_type = HSRV_OPENFLOW_STATS_RESOURCE_TYPE_FLOW;
        stats.stats_id = p_flow_info->u.dft.cnst.stats_id;
        stats.stats.resource_id = p_flow_info->flow.flow_id;
        if ((GLB_OF_FLOW_RULE_TYPE_MPLS == p_flow_info->flow.key.rule_type)
            && !p_flow_info->flow.key.rule.mpls_rule.is_acl_based)
        {
            if (GLB_OPENFLOW_MPLS_ACTION_SEQ_IS_MPLS_EGRESS_RULE(p_flow_info->flow.u.dft.mpls_output_action_seq_type))
            {
                stats.is_mpls_key = TRUE;
                if (GLB_OPENFLOW_MPLS_ACTION_SEQ_IS_L2VPN_EGRESS(p_flow_info->flow.u.dft.mpls_output_action_seq_type))
                {
                    stats.is_mpls_vc = TRUE;
                }
                else
                {
                    stats.is_mpls_vc = FALSE;
                }
            }
            else
            {
                stats.is_mpls_key = FALSE;
            }
        }
        else
        {
            stats.is_mpls_key = FALSE;
        }
        HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_create_stats(&stats));
    }
    return HSRV_E_NONE;
}

static int32
hsrv_openflow_del_stats(hsrv_openflow_flow_t* p_flow_info)
{
    hsrv_of_stats_t stats;

    if (p_flow_info->flow.stats_en)
    {
        stats.resource_type = HSRV_OPENFLOW_STATS_RESOURCE_TYPE_FLOW;
        stats.stats_id = p_flow_info->u.dft.cnst.stats_id;
        stats.stats.resource_id = p_flow_info->flow.flow_id;
        HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_del_stats(&stats));
    }
    return HSRV_E_NONE;
}

#define _____MESSAGE_MAP_____

static int32
hsrv_openflow_map_entry_id(hsrv_openflow_flow_t* p_flow, HalMsgOpenFlowEntryId_t* p_entry_id)
{
    p_entry_id->table = p_flow->flow.table;
    p_entry_id->entryId = p_flow->u.dft.cnst.entry_id;
    p_entry_id->priority = p_flow->flow.priority;
    p_entry_id->label = p_flow->u.dft.cnst.label_id;

    return HSRV_E_NONE;
}

#if 0
/* Added by weizj for udf */
static int32
hsrv_openflow_map_udf(glb_openflow_udf_parser_t* p_parser, HalMsgOpenFlowSetUdfReq_t* p_req)
{
    p_req->isDel = p_parser->is_del;

    if (GLB_OPENFLOW_UDF_TYPE_L3 == p_parser->udf_type)
    {
        GLB_SET_FLAG(p_req->flags, GLB_OPENFLOW_UDF_TYPE_L3);
    }
    if (GLB_OPENFLOW_UDF_TYPE_L4 == p_parser->udf_type)
    {
        GLB_SET_FLAG(p_req->flags, GLB_OPENFLOW_UDF_TYPE_L4);
    }
    if (p_parser->ether_type)
    {
        GLB_SET_FLAG(p_req->flags, GLB_OPENFLOW_UDF_ETHTYPE);
        p_req->etherType = p_parser->ether_type;
    }
    if (p_parser->protocol)
    {
        GLB_SET_FLAG(p_req->flags, GLB_OPENFLOW_UDF_L3_PROTOCOL);
        p_req->protocol = p_parser->protocol;
    }
    if (p_parser->ip_version)
    {
        GLB_SET_FLAG(p_req->flags, GLB_OPENFLOW_UDF_IP_VERSION);
        p_req->ipVersion = p_parser->ip_version;
    }
    if (p_parser->l4_src)
    {
        GLB_SET_FLAG(p_req->flags, GLB_OPENFLOW_UDF_L4_SRC);
        p_req->l4Src = p_parser->l4_src;
    }
    if (p_parser->l4_dst)
    {
        GLB_SET_FLAG(p_req->flags, GLB_OPENFLOW_UDF_L4_DST);
        p_req->l4Dst = p_parser->l4_dst;
    }
    if (p_parser->length0)
    {
        GLB_SET_FLAG(p_req->flags, GLB_OPENFLOW_UDF_OFFSET0);
        p_req->offset0 = p_parser->offset0;
        p_req->length0 = p_parser->length0;
    }
    if (p_parser->length1)
    {
        GLB_SET_FLAG(p_req->flags, GLB_OPENFLOW_UDF_OFFSET1);
        p_req->offset1 = p_parser->offset1;
        p_req->length1 = p_parser->length1;
    }
    if (p_parser->length2)
    {
        GLB_SET_FLAG(p_req->flags, GLB_OPENFLOW_UDF_OFFSET2);
        p_req->offset2 = p_parser->offset2;
        p_req->length2 = p_parser->length2;
    }
    if (p_parser->length3)
    {
        GLB_SET_FLAG(p_req->flags, GLB_OPENFLOW_UDF_OFFSET3);
        p_req->offset3 = p_parser->offset3;
        p_req->length3 = p_parser->length3;
    }

    return HSRV_E_NONE;
}
#endif

static int32
hsrv_openflow_map_flow_mac_key(glb_openflow_mac_key_t* p_mac, HalMsgOpenFlowMacRule_t* p_mac_rule)
{
    if (p_mac->flags.mac_sa)
    {
        GLB_SET_FLAG(p_mac_rule->flag, GLB_OPENFLOW_MAC_RULE_FLAG_MACSA);
        p_mac_rule->macSa.buf = p_mac->mac_sa;
        p_mac_rule->macSa.size = sizeof(mac_addr_t);
        p_mac_rule->macSaMask.buf = p_mac->mac_sa_mask;
        p_mac_rule->macSaMask.size = sizeof(mac_addr_t);
    }
    if (p_mac->flags.mac_da)
    {
        GLB_SET_FLAG(p_mac_rule->flag, GLB_OPENFLOW_MAC_RULE_FLAG_MACDA);
        p_mac_rule->macDa.buf = p_mac->mac_da;
        p_mac_rule->macDa.size = sizeof(mac_addr_t);
        p_mac_rule->macDaMask.buf = p_mac->mac_da_mask;
        p_mac_rule->macDaMask.size = sizeof(mac_addr_t);
    }
    if (p_mac->flags.stag_valid)
    {
        GLB_SET_FLAG(p_mac_rule->flag, GLB_OPENFLOW_MAC_RULE_FLAG_STAG_VALID);
        p_mac_rule->sTagValid = p_mac->stag_valid;
    }
    if (p_mac->flags.svlan)
    {
        GLB_SET_FLAG(p_mac_rule->flag, GLB_OPENFLOW_MAC_RULE_FLAG_SVLAN_ID);
        p_mac_rule->sVlanId = p_mac->svlan;
        p_mac_rule->sVlanIdMask = p_mac->svlan_mask;
    }
    if (p_mac->flags.stag_cos)
    {
        GLB_SET_FLAG(p_mac_rule->flag, GLB_OPENFLOW_MAC_RULE_FLAG_STAG_COS);
        p_mac_rule->sTagCos = p_mac->stag_cos;
        p_mac_rule->sTagCosMask = p_mac->stag_cos_mask;
    }
    if (p_mac->flags.arp_packet)
    {
        GLB_SET_FLAG(p_mac_rule->flag, GLB_OPENFLOW_MAC_RULE_FLAG_ARP_PKT);
        p_mac_rule->arpPkt = p_mac->arp_packet;
    }
    if (p_mac->flags.ipv4_packet)
    {
        GLB_SET_FLAG(p_mac_rule->flag, GLB_OPENFLOW_MAC_RULE_FLAG_IPV4_PKT);
        p_mac_rule->ipv4Pkt = p_mac->ipv4_packet;
    }
    if (p_mac->flags.mpls_packet)
    {
        GLB_SET_FLAG(p_mac_rule->flag, GLB_OPENFLOW_MAC_RULE_FLAG_MPLS_PKT);
        p_mac_rule->mplsPkt = p_mac->mpls_packet;
    }
    if (p_mac->flags.eth_type)
    {
        GLB_SET_FLAG(p_mac_rule->flag, GLB_OPENFLOW_MAC_RULE_FLAG_ETHTYPE);
        p_mac_rule->ethType = p_mac->eth_type;
        p_mac_rule->ethTypeMask = p_mac->eth_type_mask;
    }
    if (p_mac->flags.ctag_valid)
    {
        GLB_SET_FLAG(p_mac_rule->flag, GLB_OPENFLOW_MAC_RULE_FLAG_CTAG_VALID);
        p_mac_rule->cTagValid = p_mac->ctag_valid;
    }
    if (p_mac->flags.cvlan)
    {
        GLB_SET_FLAG(p_mac_rule->flag, GLB_OPENFLOW_MAC_RULE_FLAG_CVLAN_ID);
        p_mac_rule->cVlanId = p_mac->cvlan;
        p_mac_rule->cVlanIdMask = p_mac->cvlan_mask;
    }
    if (p_mac->flags.ctag_cos)
    {
        GLB_SET_FLAG(p_mac_rule->flag, GLB_OPENFLOW_MAC_RULE_FLAG_CTAG_COS);
        p_mac_rule->cTagCos = p_mac->ctag_cos;
        p_mac_rule->cTagCosMask = p_mac->ctag_cos_mask;
    }
    return HSRV_E_NONE;
}

static int32
hsrv_openflow_map_flow_ipv4_key(glb_openflow_ipv4_key_t* p_ipv4, HalMsgOpenFlowIpv4Rule_t* p_ipv4_rule)
{
    if (p_ipv4->flags.ip_sa)
    {
        GLB_SET_FLAG(p_ipv4_rule->flag, GLB_OPENFLOW_IPV4_RULE_FLAG_IPSA);
        p_ipv4_rule->ipSa = p_ipv4->ip_sa;
        p_ipv4_rule->ipSaMask = p_ipv4->ip_sa_mask;
    }
    if (p_ipv4->flags.ip_da)
    {
        GLB_SET_FLAG(p_ipv4_rule->flag, GLB_OPENFLOW_IPV4_RULE_FLAG_IPDA);
        p_ipv4_rule->ipDa = p_ipv4->ip_da;
        p_ipv4_rule->ipDaMask = p_ipv4->ip_da_mask;
    }
    if (p_ipv4->flags.l4_protocol)
    {
        GLB_SET_FLAG(p_ipv4_rule->flag, GLB_OPENFLOW_IPV4_RULE_FLAG_L4_PROTO);
        p_ipv4_rule->l4Protocol = p_ipv4->l4_protocol;
    }
    if (p_ipv4->flags.l4_source_port)
    {
        GLB_SET_FLAG(p_ipv4_rule->flag, GLB_OPENFLOW_IPV4_RULE_FLAG_L4_SRC_PORT);
        p_ipv4_rule->l4SrcPort.port = p_ipv4->l4_source_port.port;
        p_ipv4_rule->l4SrcPort.portMask = p_ipv4->l4_source_port.port_mask;
    }
    if (p_ipv4->flags.l4_dest_port)
    {
        GLB_SET_FLAG(p_ipv4_rule->flag, GLB_OPENFLOW_IPV4_RULE_FLAG_L4_DST_PORT);
        p_ipv4_rule->l4DestPort.port = p_ipv4->l4_dest_port.port;
        p_ipv4_rule->l4DestPort.portMask = p_ipv4->l4_dest_port.port_mask;
    }
    if (p_ipv4->flags.tcp_flags)
    {
        GLB_SET_FLAG(p_ipv4_rule->flag, GLB_OPENFLOW_IPV4_RULE_FLAG_TCP_FLAG);
        p_ipv4_rule->tcpFlags = p_ipv4->tcp_flags;
    }
    if (p_ipv4->flags.icmp_type)
    {
        GLB_SET_FLAG(p_ipv4_rule->flag, GLB_OPENFLOW_IPV4_RULE_FLAG_ICMP_TYPE);
        p_ipv4_rule->icmpType = p_ipv4->icmp_type;
    }
    if (p_ipv4->flags.icmp_code)
    {
        GLB_SET_FLAG(p_ipv4_rule->flag, GLB_OPENFLOW_IPV4_RULE_FLAG_ICMP_CODE);
        p_ipv4_rule->icmpCode = p_ipv4->icmp_code;
    }
    if (p_ipv4->flags.dscp)
    {
        GLB_SET_FLAG(p_ipv4_rule->flag, GLB_OPENFLOW_IPV4_RULE_FLAG_DSCP);
        p_ipv4_rule->dscp = p_ipv4->dscp;
        p_ipv4_rule->dscpMask = p_ipv4->dscp_mask;
    }
    if (p_ipv4->flags.ecn)
    {
        GLB_SET_FLAG(p_ipv4_rule->flag, GLB_OPENFLOW_IPV4_RULE_FLAG_ECN);
        p_ipv4_rule->ecn = p_ipv4->ecn;
        p_ipv4_rule->ecnMask = p_ipv4->ecn_mask;
    }
    if (p_ipv4->flags.ip_frag)
    {
        GLB_SET_FLAG(p_ipv4_rule->flag, GLB_OPENFLOW_IPV4_RULE_FLAG_IP_FRAG);
        p_ipv4_rule->ipFrag = p_ipv4->ip_frag;
        p_ipv4_rule->ipFragMask = p_ipv4->ip_frag_mask;
    }
    return HSRV_E_NONE;
}

static int32
hsrv_openflow_map_flow_ipv6_key(glb_openflow_ipv6_key_t* p_ipv6, HalMsgOpenFlowIpv6Rule_t* p_ipv6_rule)
{
    if(p_ipv6->flags.ip_sa)
    {
        GLB_SET_FLAG(p_ipv6_rule->flag, GLB_OPENFLOW_IPV6_RULE_FLAG_IPSA);
        p_ipv6_rule->sprefix.buf = (uint8*)p_ipv6->ip_sa;
        p_ipv6_rule->sprefix.size = sizeof(ipv6_addr_t);

        p_ipv6_rule->sprefixMask.buf = (uint8*)p_ipv6->ip_sa_mask;
        p_ipv6_rule->sprefixMask.size = sizeof(ipv6_addr_t);
    }
    if(p_ipv6->flags.ip_da)
    {
        GLB_SET_FLAG(p_ipv6_rule->flag, GLB_OPENFLOW_IPV6_RULE_FLAG_IPDA);
        p_ipv6_rule->dprefix.buf = (uint8*)p_ipv6->ip_da;
        p_ipv6_rule->dprefix.size = sizeof(ipv6_addr_t);

        p_ipv6_rule->dprefixMask.buf = (uint8*)p_ipv6->ip_da_mask;
        p_ipv6_rule->dprefixMask.size = sizeof(ipv6_addr_t);
    }
    if(p_ipv6->flags.l4_protocol)
    {
        GLB_SET_FLAG(p_ipv6_rule->flag, GLB_OPENFLOW_IPV6_RULE_FLAG_L4_PROTO);
        p_ipv6_rule->l4Protocol = p_ipv6->l4_protocol;
    }
    if(p_ipv6->flags.l4_source_port)
    {
        GLB_SET_FLAG(p_ipv6_rule->flag, GLB_OPENFLOW_IPV6_RULE_FLAG_L4_SRC_PORT);
        p_ipv6_rule->l4SrcPort.port = p_ipv6->l4_source_port.port;
        p_ipv6_rule->l4SrcPort.portMask = p_ipv6->l4_source_port.port_mask;
    }
    if(p_ipv6->flags.l4_dest_port)
    {
        GLB_SET_FLAG(p_ipv6_rule->flag, GLB_OPENFLOW_IPV6_RULE_FLAG_L4_DST_PORT);
        p_ipv6_rule->l4DestPort.port = p_ipv6->l4_dest_port.port;
        p_ipv6_rule->l4DestPort.portMask = p_ipv6->l4_dest_port.port_mask;
    }
    if (p_ipv6->flags.icmp_type)
    {
        GLB_SET_FLAG(p_ipv6_rule->flag, GLB_OPENFLOW_IPV6_RULE_FLAG_ICMP_TYPE);
        p_ipv6_rule->icmpType = p_ipv6->icmp_type;
    }
    if (p_ipv6->flags.icmp_code)
    {
        GLB_SET_FLAG(p_ipv6_rule->flag, GLB_OPENFLOW_IPV6_RULE_FLAG_ICMP_CODE);
        p_ipv6_rule->icmpCode = p_ipv6->icmp_code;
    }
    if (p_ipv6->flags.dscp)
    {
        GLB_SET_FLAG(p_ipv6_rule->flag, GLB_OPENFLOW_IPV6_RULE_FLAG_DSCP);
        p_ipv6_rule->dscp = p_ipv6->dscp;
        p_ipv6_rule->dscpMask = p_ipv6->dscp_mask;
    }
    if (p_ipv6->flags.ecn)
    {
        GLB_SET_FLAG(p_ipv6_rule->flag, GLB_OPENFLOW_IPV6_RULE_FLAG_ECN);
        p_ipv6_rule->ecn = p_ipv6->ecn;
        p_ipv6_rule->ecnMask = p_ipv6->ecn_mask;
    }
    if (p_ipv6->flags.flow_label)
    {
        GLB_SET_FLAG(p_ipv6_rule->flag, GLB_OPENFLOW_IPV6_RULE_FLAG_FLOW_LABEL);
        p_ipv6_rule->flowLabel= p_ipv6->flow_label;
        p_ipv6_rule->flowLabelMask = p_ipv6->flow_label_mask;
    }
    if (p_ipv6->flags.ip_frag)
    {
        GLB_SET_FLAG(p_ipv6_rule->flag, GLB_OPENFLOW_IPV6_RULE_FLAG_IP_FRAG);
        p_ipv6_rule->ipFrag = p_ipv6->ip_frag;
        p_ipv6_rule->ipFragMask = p_ipv6->ip_frag_mask;
    }
    return HSRV_E_NONE;
}

static int32
hsrv_openflow_map_flow_arp_key(glb_openflow_arp_key_t* p_arp, HalMsgOpenFlowArpRule_t* p_arp_rule)
{
    if (p_arp->flags.sender_ip)
    {
        GLB_SET_FLAG(p_arp_rule->flag, GLB_OPENFLOW_ARP_RULE_FLAG_SENDER_IP);
        p_arp_rule->senderIp = p_arp->sender_ip;
        p_arp_rule->senderIpMask = p_arp->sender_ip_mask;
    }
    if (p_arp->flags.target_ip)
    {
        GLB_SET_FLAG(p_arp_rule->flag, GLB_OPENFLOW_ARP_RULE_FLAG_TARGET_IP);
        p_arp_rule->targetIp = p_arp->target_ip;
        p_arp_rule->targetIpMask= p_arp->target_ip_mask;
    }
    if (p_arp->flags.arp_op)
    {
        GLB_SET_FLAG(p_arp_rule->flag, GLB_OPENFLOW_ARP_RULE_FLAG_ARP_OPCODE);
        p_arp_rule->arpOpCode = p_arp->arp_op;
        p_arp_rule->arpOpCodeMask = p_arp->arp_op;
    }
    return HSRV_E_NONE;
}

/* Added by weizj for tunnel */
static int32
hsrv_openflow_map_flow_tnl_key(glb_openflow_tnl_key_t* p_tnl, HalMsgOpenFlowTnlRule_t* p_tnl_rule)
{
    if (p_tnl->metadata)
    {
        GLB_SET_FLAG(p_tnl_rule->flags, GLB_OPENFLOW_TNL_RULE_FLAG_METADATA);
        p_tnl_rule->metadata = p_tnl->metadata;
    }
    else if(p_tnl->tun_id)
    {
        GLB_SET_FLAG(p_tnl_rule->flags, GLB_OPENFLOW_TNL_RULE_FLAG_TUN_ID);
        p_tnl_rule->tunId = p_tnl->tun_id;
        /* support tunId mask */
        p_tnl_rule->tunId_mask = p_tnl->tun_id_mask;
    }

    return HSRV_E_NONE;
}
/* End by weizj for tunnel */

/* Added by weizj for udf */
static int32
hsrv_openflow_map_flow_udf_key(glb_openflow_udf_key_t* p_udf, HalMsgOpenFlowUdfRule_t* p_udf_rule)
{
    p_udf_rule->enable = p_udf->udf_enable;
    p_udf_rule->udfId = p_udf->udf_id;
    p_udf_rule->udf0 = p_udf->udf0;
    p_udf_rule->udfMask0 = p_udf->udf0_mask;
    p_udf_rule->udf1 = p_udf->udf1;
    p_udf_rule->udfMask1 = p_udf->udf1_mask;
    p_udf_rule->udf2 = p_udf->udf2;
    p_udf_rule->udfMask2 = p_udf->udf2_mask;
    p_udf_rule->udf3 = p_udf->udf3;
    p_udf_rule->udfMask3 = p_udf->udf3_mask;

    return HSRV_E_NONE;
}
/* End by weizj for udf */

static int32
hsrv_openflow_map_flow_mpls_key(glb_openflow_mpls_key_t* p_mpls, HalMsgOpenFlowMplsRule_t* p_mpls_rule)
{
    /* modified by yaom for openflow MPLS L2VPN 20160214 */
    if (p_mpls->is_acl_based)
    {
        /* 0 value means the field is not used, so no mask is needed like ipv4... */
        p_mpls_rule->mplsLabelNum = p_mpls->u.acl.mpls_label_num;
        
        p_mpls_rule->mplsLabel0 = p_mpls->u.acl.mpls_label0;
        p_mpls_rule->mplsLabel0Mask = p_mpls->u.acl.mpls_label0_mask;
            
        p_mpls_rule->mplsLabel1 = p_mpls->u.acl.mpls_label1;
        p_mpls_rule->mplsLabel1Mask = p_mpls->u.acl.mpls_label1_mask;

        p_mpls_rule->mplsLabel2 = p_mpls->u.acl.mpls_label2;
        p_mpls_rule->mplsLabel2Mask = p_mpls->u.acl.mpls_label2_mask;
    }
    else
    {
        p_mpls_rule->mplsLabelNum = 1; /* might support multiple labels in the future */
        p_mpls_rule->mplsLabel0 = p_mpls->u.ilm.mpls_label;
    }
    
    return HSRV_E_NONE;
}

static int32
hsrv_openflow_map_flow_key(glb_openflow_flow_key_t* p_key, HalMsgOpenFlowRule_t* p_rule)
{
    HSRV_IF_ERROR_RETURN(hsrv_openflow_map_flow_mac_key(
        &p_key->mac, &p_rule->macRule));

    /* Added by weizj for tunnel bug 33692 */
    HSRV_IF_ERROR_RETURN(hsrv_openflow_map_flow_tnl_key(
        &p_key->tnl, &p_rule->tnlRule));

    /* Added by weizj for udf */
    HSRV_IF_ERROR_RETURN(hsrv_openflow_map_flow_udf_key(
        &p_key->udf, &p_rule->udfRule));
    
    switch(p_key->rule_type)
    {
    case GLB_OF_FLOW_RULE_TYPE_ARP:
        p_rule->unionRule.present = unionRule_PR_arpRule;
        HSRV_IF_ERROR_RETURN(hsrv_openflow_map_flow_arp_key(
            &p_key->rule.arp_rule, &p_rule->unionRule.arpRule));
        break;
    
    case GLB_OF_FLOW_RULE_TYPE_IPV4:
        p_rule->unionRule.present = unionRule_PR_ipv4Rule;
        HSRV_IF_ERROR_RETURN(hsrv_openflow_map_flow_ipv4_key(
            &p_key->rule.ipv4_rule, &p_rule->unionRule.ipv4Rule));
        break;
    
    case GLB_OF_FLOW_RULE_TYPE_IPV6:
        p_rule->unionRule.present = unionRule_PR_ipv6Rule;
        HSRV_IF_ERROR_RETURN(hsrv_openflow_map_flow_ipv6_key(
            &p_key->rule.ipv6_rule, &p_rule->unionRule.ipv6Rule));
        break;
    
    case GLB_OF_FLOW_RULE_TYPE_MPLS:
        p_rule->unionRule.present = unionRule_PR_mplsRule;
        HSRV_IF_ERROR_RETURN(hsrv_openflow_map_flow_mpls_key(
            &p_key->rule.mpls_rule, &p_rule->unionRule.mplsRule));
        p_rule->unionRule.present = unionRule_PR_mplsRule;
        break;

    case GLB_OF_FLOW_RULE_TYPE_MPLS_MCAST:
        p_rule->unionRule.present = unionRule_PR_mplsMcastRule;
        HSRV_IF_ERROR_RETURN(hsrv_openflow_map_flow_mpls_key(
            &p_key->rule.mpls_rule, &p_rule->unionRule.mplsRule));
        p_rule->unionRule.present = unionRule_PR_mplsMcastRule;
        break;

    default:
        break;
    }
    
    return HSRV_E_NONE;
}

static int32
hsrv_openflow_map_flow_action(hsrv_openflow_flow_t* p_flow, HalMsgOpenFlowAction_t* p_action)
{
    p_action->stripPacket = p_flow->flow.flow_tnl_type ? 1 : 0;
    if (p_flow->u.dft.var.normal)
    {
        p_action->denyRouting = FALSE;
        p_action->denyBridging = FALSE;
        p_action->denyLearning = FALSE;
    }
    else
    {
        p_action->denyRouting = TRUE;
        p_action->denyBridging = TRUE;
        p_action->denyLearning = TRUE;

        if (p_flow->u.dft.var.group_info.group_id != OFP_GROUP_INVALID_GROUP_ID)
        {
            p_action->nexthopId = p_flow->u.dft.var.group_info.group_nhid;
        }
        else if (p_flow->u.dft.var.actlist)
        {
            p_action->nexthopId = p_flow->u.dft.var.actlist->nh_info.main_nh.nh_offset.nhid;
            /* for fdb default to controller entry, need mcast group id */
            p_action->nexthopOffset = p_flow->u.dft.var.actlist->nh_info.main_nh.nh_offset.offset;
        }
        else
        {
            p_action->nexthopId = p_flow->u.dft.var.nh_info.main_nh.nh_offset.nhid;
            p_action->nexthopOffset = p_flow->u.dft.var.nh_info.main_nh.nh_offset.offset;
        }
    }
    
    if (p_flow->u.dft.var.meter_info.meter_id != OFP_METER_INVALID_METER_ID)
    {
        /* Modifed by weizj for meter */
        p_action->policerId = p_flow->u.dft.var.meter_info.meter_id;
    }
    
    if (p_flow->u.dft.var.queue_info.queue_id != OFP_QUEUE_INVALID_QUEUE_ID)
    {
        p_action->priorityValid = 1;
        p_action->priority = p_flow->u.dft.var.queue_info.queue_id * 8;
        p_action->color = GLB_QOS_COLOR_GREEN;
    }
    
    if (p_flow->flow.stats_en)
    {
        p_action->statsEn = TRUE;
        p_action->statsId = p_flow->u.dft.cnst.stats_id;
    }

    if (GLB_OPENFLOW_MPLS_ACTION_SEQ_IS_POP_ALL_MPLS(p_flow->flow.u.dft.mpls_output_action_seq_type))
    {
        p_action->popAllMpls = TRUE;
    }

    /* modified by yaom for openflow MPLS L2VPN 20160214 */
    if (GLB_OPENFLOW_MPLS_ACTION_SEQ_IS_MPLS_EGRESS_RULE(p_flow->flow.u.dft.mpls_output_action_seq_type))
    {
        p_action->isDsMplsKey = TRUE;
        p_action->mplsActionType = p_flow->flow.u.dft.mpls_output_action_seq_type;
    }
    else
    {
        p_action->isDsMplsKey = FALSE;
    }

    if ((((GLB_OF_FLOW_RULE_TYPE_MPLS == p_flow->flow.key.rule_type)
         || (GLB_OF_FLOW_RULE_TYPE_MPLS_MCAST == p_flow->flow.key.rule_type))
         && (!p_flow->flow.key.rule.mpls_rule.is_acl_based)) || (GLB_OF_PORT_TYPE_TUNNEL == p_flow->flow.key.in_port.type))
    {
        p_action->resetPayloadOffset = FALSE;
    }
    else
    {
        p_action->resetPayloadOffset = TRUE;
    }

    if (p_flow->u.dft.var.old_truncate_len)
    {
        p_action->oldLogSessionId = GLB_OPENFLOW_TRUNCATE_MIRROR_SESSION_ID;
    }
    
    if (p_flow->u.dft.var.truncate_len)
    {
        p_action->logSessionId = GLB_OPENFLOW_TRUNCATE_MIRROR_SESSION_ID;
        p_action->logDestPort = p_flow->u.dft.var.truncate_output_port;
        p_action->truncateLen = p_flow->u.dft.var.truncate_len;
    }

    if (p_flow->u.dft.var.strip_len)
    {
        p_action->stripLen = p_flow->u.dft.var.strip_len;
    }

    /*added by hansf for set vlan action before group action*/
    p_action->groupVlanId = p_flow->u.dft.var.group_vlanid;
    p_action->groupVlanCos = p_flow->u.dft.var.group_vlan_cos;
    p_action->groupVlanType = p_flow->u.dft.var.group_vlan_type;
    return HSRV_E_NONE;
}

#define _____MESSAGE_API_____

static int32
hsrv_openflow_msg_add_flow(hsrv_openflow_flow_t* p_flow)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowAddFlowReq_t* p_req;
    
    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddFlow;
    
    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);
    
    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddFlow;
    HSRV_IF_ERROR_RETURN(hsrv_openflow_map_entry_id(p_flow, &p_req->entryId));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_map_flow_key(&p_flow->flow.key, &p_req->rule));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_map_flow_action(p_flow, &p_req->action));

    p_req->entryId.groupId = _hsrv_openflow_get_group_id_by_ifindex_class_id(p_flow->flow.key.in_port.ifindex, p_req->entryId.label, 0);
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));
    
    return HSRV_E_NONE;
}

static int32
hsrv_openflow_msg_update_flow(hsrv_openflow_flow_t* p_flow)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowUpdateFlowReq_t* p_req;
    
    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowUpdateFlow;
    
    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);
    
    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowUpdateFlow;
    HSRV_IF_ERROR_RETURN(hsrv_openflow_map_entry_id(p_flow, &p_req->entryId));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_map_flow_action(p_flow, &p_req->action));
    
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));
    
    return HSRV_E_NONE;
    
}

static int32
hsrv_openflow_msg_del_flow(hsrv_openflow_flow_t* p_flow)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowDelFlowReq_t* p_req;
    
    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowDelFlow;
    
    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);
    
    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelFlow;
    HSRV_IF_ERROR_RETURN(hsrv_openflow_map_entry_id(p_flow, &p_req->entryId));
    /* need to send mpls info for removing ilm */
    if ((GLB_OF_FLOW_RULE_TYPE_MPLS == p_flow->flow.key.rule_type)
        && !p_flow->flow.key.rule.mpls_rule.is_acl_based 
        && GLB_OPENFLOW_MPLS_ACTION_SEQ_IS_MPLS_EGRESS_RULE(p_flow->flow.u.dft.mpls_output_action_seq_type))
    {
        p_req->isDsMplsKey = TRUE;
        p_req->mplsLabel = p_flow->flow.key.rule.mpls_rule.u.ilm.mpls_label;
    }

    if (p_flow->u.dft.var.truncate_len)
    {
        p_req->logSessionId = GLB_OPENFLOW_TRUNCATE_MIRROR_SESSION_ID;
    }
    
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_msg_op_label(hsrv_openflow_label_t* p_label)
{
    return HSRV_E_NONE;
}

int32
hsrv_openflow_msg_set_udf_enable(uint8 enable)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowSetUdfReq_t* p_req;
    
    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowSetUdf;
    
    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);
    
    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowSetUdf;
    p_req->enable = enable;
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    return HSRV_E_NONE;
}

/* Added by weizj for udf */
int32
hsrv_openflow_msg_set_udf_parser(glb_openflow_udf_parser_t* p_parser)
{
    #if 0
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowSetUdfReq_t* p_req;
    
    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowSetUdf;
    
    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);
    
    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowSetUdf;
    HSRV_IF_ERROR_RETURN(hsrv_openflow_map_udf(p_parser, p_req));
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));
    #endif

    return HSRV_E_NONE;
}

/* modified by liwh for bug 46591 to support mpls tpoam, 2018-03-28 */
static int32
hsrv_openflow_del_aps_nexthop(hsrv_openflow_flow_t* p_flow)
{
    hsrv_openflow_group_t* p_group = NULL;
    
    if (OFP_GROUP_INVALID_GROUP_ID != p_flow->u.dft.var.group_info.group_id)
    {    
        hsrv_openflow_group_lookup_group(p_flow->u.dft.var.group_info.group_id, &p_group);

        /* group is lsp or pw aps group*/
        if (p_group)
        {
            if  (GLB_OF_GROUP_TYPE_LSP_APS == p_group->group.group_type)
            {
                hsrv_openflow_nexthop_release_lsp_aps_nh(p_group->aps_group.lsp_aps_nh_hash, p_flow->u.dft.var.pw_label);
            }
            else if  (GLB_OF_GROUP_TYPE_PW_APS == p_group->group.group_type)
            {
                p_group->aps_group.aps_nhid_count--;

                if (0 == p_group->aps_group.aps_nhid_count)
                {
                    /*modified by yejl for bug 52155, 2019-05-22*/
                    hsrv_openflow_nexthop_release_pw_aps_nh(
                        p_group->aps_group.aps_group_offset, p_group->aps_group.aps_nhid, p_group->aps_group.aps_offset, 
                        p_group->aps_group.mcast_nhid, p_group->aps_group.mcast_offset);
                    hsrv_aps_group_offset_release(p_group->aps_group.aps_group_offset);

                    p_group->aps_group.aps_nhid = 0;
                    p_group->aps_group.aps_group_offset = 0;
                    p_group->aps_group.mcast_nhid = 0;
                    p_group->aps_group.mcast_offset = 0;
                    /*ended by yejl*/
                }
            }
        }
    }

    return HSRV_E_NONE;
}
/* liwh end */

#define _____INTERNAL_API_____
int32
hsrv_openflow_del_flow(hsrv_openflow_flow_t* p_flow)
{
    HSRV_IF_ERROR_RETURN(hsrv_openflow_del_stats(p_flow));
    /* modified by liwh for bug 46591 to support mpls tpoam, 2018-03-28 */
    HSRV_IF_ERROR_RETURN(hsrv_openflow_del_aps_nexthop(p_flow));
    /* liwh end */
    /*added by wangl*/
    HSRV_IF_ERROR_RETURN(hsrv_tunneldb_cdb_oper_function(HSRV_TUNNEL_CDB_FLOW_DEL_UNMAP_ACTION, p_flow));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_unmap_action(&p_flow->u));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_del_flow(p_flow));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_flowdb_del_flow(p_flow));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_release_id(p_flow));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_release_flow_info(p_flow));

    return HSRV_E_NONE;
}

/* stats */
int32
hsrv_openflow_clear_flow_stats(hsrv_openflow_flow_t* p_flow)
{
    hsrv_of_stats_t stats;

    stats.resource_type = HSRV_OPENFLOW_STATS_RESOURCE_TYPE_FLOW;
    stats.stats_id = p_flow->u.dft.cnst.stats_id;
    stats.stats.resource_id = p_flow->flow.flow_id;
    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_clear_stats(&stats));
    return HSRV_E_NONE;
}

int32
hsrv_openflow_nexthop_add_port_to_all_action_list(uint16 gport, hsrv_of_nh_info_t* p_member_nh_array, 
                                    uint32 *member_no_use_idx)
{
    uint32 idx = 0, ifindex = 0;
    bool is_exist = FALSE;
    int32 member_all_idx = 0;
    hsrv_ofact_flex_nh_info_t* p_flex_info = NULL;
    hsrv_openflow_action_combo_t action_combo;
    hsrv_ofnh_flex_param_t param;

    hsrv_ifdb_get_ifindex_by_gportid(gport, &ifindex);

    *member_no_use_idx = HSRV_OF_NH_INVAID_INDEX;
    member_all_idx = HSRV_OF_NH_INVAID_INDEX;
    for (idx = 0; idx < GLB_OPENFLOW_MAX_OUTPUT_PER_FLOW; idx ++)
    {
        if (TRUE == p_member_nh_array[idx].to_all
            && HSRV_OF_NH_INVAID_INDEX == member_all_idx)
        {
            member_all_idx = idx;
        }

        if (NULL == p_member_nh_array[idx].share_info
            && HSRV_OF_NH_INVAID_INDEX == *member_no_use_idx)
        {
            *member_no_use_idx = idx;
        }

        if (TRUE == p_member_nh_array[idx].to_all
            && gport == p_member_nh_array[idx].gport)
        {
            is_exist = TRUE;
        }
    }

    if (HSRV_OF_NH_INVAID_INDEX == member_all_idx || HSRV_OF_NH_INVAID_INDEX == *member_no_use_idx || TRUE == is_exist)
    {
        return HSRV_E_INVALID_PARAM;
    }

    p_flex_info = (hsrv_ofact_flex_nh_info_t* )p_member_nh_array[member_all_idx].share_info;

    sal_memcpy(&action_combo, &p_flex_info->combo, sizeof(hsrv_openflow_action_combo_t));
    
    GLB_OFPACT_FLD_SET(action_combo.flag, OUTPUT);
    action_combo.output_gport = gport;

    param.p_action_combo = &action_combo;

    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_alloc_flex_nh(&param, 
        &p_member_nh_array[*member_no_use_idx]));

   p_member_nh_array[*member_no_use_idx].gport = gport;
   p_member_nh_array[*member_no_use_idx].to_all = TRUE;

   return HSRV_E_NONE;
}

int32
hsrv_openflow_nexthop_del_port_from_all_action_list(uint16 gport, hsrv_of_nh_info_t *p_member_nh_array, 
                                    int32 *member_all_idx)
{
    uint32 idx;

    *member_all_idx = HSRV_OF_NH_INVAID_INDEX;
    for (idx = 0; idx < GLB_OPENFLOW_MAX_OUTPUT_PER_FLOW; idx ++)
    {
        if (TRUE == p_member_nh_array[idx].to_all
            && p_member_nh_array[idx].gport == gport
            && HSRV_OF_NH_INVAID_INDEX == *member_all_idx)
        {
            *member_all_idx = idx;
            break;
        }
    }

    if (HSRV_OF_NH_INVAID_INDEX == *member_all_idx)
    {
        return HSRV_E_NONE;
    }

    return HSRV_E_NONE;
}

static void 
hsrv_openflow_flow_add_port_to_all_action_list(ctclib_hash_backet_t* phb, void *user_data)
{
    uint16 gport = 0;
    uint32  member_no_use_idx = 0, ret = 0;
    hsrv_ofnh_mcast_member_param_t member_param;
    hsrv_of_nh_info_t* p_member;
    hsrv_openflow_bond_data_t* p_bond_info = NULL;

    hsrv_actlist_info_t* p_list_info = (hsrv_actlist_info_t*)phb->data;
    fei_openflow_update_all_action_list_req_t* p_data = user_data; 

    hsrv_of_nh_info_t* p_member_nh_array = p_list_info->nh_info.member_nh;
    hsrv_of_nh_info_t* main_nh = &p_list_info->nh_info.main_nh;

    hsrv_openflow_bond_lookup_info(p_data->ifindex, &p_bond_info);
    if (NULL != p_bond_info)
    {
        gport = p_bond_info->gport;
    }
    else
    {
        ret = hsrv_ifdb_get_portid(p_data->ifindex, &gport);
        if (HSRV_E_NONE != ret)
        {
            return ;
        }
    }

    if (HSRV_E_NONE != hsrv_openflow_nexthop_add_port_to_all_action_list(gport, p_member_nh_array, &member_no_use_idx))
    {
        return ;
    }

    p_member = &p_member_nh_array[member_no_use_idx];
    member_param.group_nhid = main_nh->nh_offset.nhid;
    member_param.op_type = HSRV_OFNH_MCAST_OP_ADD_MEMBER;
    member_param.mem_gport = p_member->gport;
    member_param.mem_nhid = p_member->nh_offset.nhid;
    member_param.port_check_discard = p_member->nh_offset.port_check_discard;
    member_param.redirect_to_ingress_port = p_member->nh_offset.redirect_to_ingress_port;

    hsrv_openflow_nexthop_add_mcast_member(&member_param);

    p_list_info->nh_info.member_count ++;

    return ;
}

static void 
hsrv_openflow_flow_del_port_to_all_action_list(ctclib_hash_backet_t* phb, void *user_data)
{
    uint16 gport;
    int32 member_all_idx = 0, ret = 0;
    hsrv_openflow_bond_data_t* p_bond_info = NULL;

    hsrv_actlist_info_t* p_list_info = (hsrv_actlist_info_t*)phb->data;
    fei_openflow_update_all_action_list_req_t* p_data = user_data; 

    hsrv_of_nh_info_t* p_member_nh_array = p_list_info->nh_info.member_nh;
    hsrv_of_nh_info_t* main_nh = &p_list_info->nh_info.main_nh;

    hsrv_openflow_bond_lookup_info(p_data->ifindex, &p_bond_info);
    if (NULL != p_bond_info)
    {
        gport = p_bond_info->gport;
    }
    else
    {
        ret = hsrv_ifdb_get_portid(p_data->ifindex, &gport);
        if (HSRV_E_NONE != ret)
        {
            return ;
        }
    }

    if (HSRV_E_NONE != hsrv_openflow_nexthop_del_port_from_all_action_list(gport, p_member_nh_array, &member_all_idx))
    {
        return ;
    }

#if 0
    ADPT_FLOW_ERROR_RETURN(hal_nexthop_del_mcast_member(
    main_nh->nh_offset.nhid, 
    p_member_nh_array[member_all_idx].gport,
    p_member_nh_array[member_all_idx].nh_offset.nhid));

    p_member_nh_array[member_all_idx].share_info = NULL;
    p_member_nh_array[member_all_idx].gport = 0;
    p_member_nh_array[member_all_idx].to_all = false;
    memset(&p_member_nh_array[member_all_idx].nh_offset, 0, sizeof(ofp_nh_offset_t));
#endif

    if (HSRV_OF_NH_INVAID_INDEX == member_all_idx)
    {
        return ;
    }

    hsrv_of_nh_info_t *member_nh;
    hsrv_ofnh_mcast_member_param_t param;

    member_nh = &p_member_nh_array[member_all_idx];
    param.group_nhid = main_nh->nh_offset.nhid;
    param.op_type = HSRV_OFNH_MCAST_OP_DEL_MEMBER;
    param.mem_gport = member_nh->gport;
    param.mem_nhid = member_nh->nh_offset.nhid;
    param.port_check_discard = member_nh->nh_offset.port_check_discard;
    param.redirect_to_ingress_port = member_nh->nh_offset.redirect_to_ingress_port;

    hsrv_openflow_nexthop_remove_mcast_member(&param);
    hsrv_openflow_nexthop_release_nh_info(&p_member_nh_array[member_all_idx]);

    /* clear */
    p_member_nh_array[member_all_idx].share_info = NULL;
    p_member_nh_array[member_all_idx].gport = 0;
    p_member_nh_array[member_all_idx].to_all = FALSE;
    memset(&p_member_nh_array[member_all_idx].nh_offset, 0, sizeof(hsrv_of_nh_offset_t));

    p_list_info->nh_info.member_count --;

    return ;
}

#define _____HAL_APIs_____

int32 
fei_api_openflow_post_sys_startup(fei_openflow_post_sys_startup_t *pst_req)
{
    /* XXX: pass parameter if later needed. */
    
    HSRV_IF_ERROR_RETURN(hsrv_openflow_post_startup());
    
    return HSRV_E_NONE;
}

int32 
fei_api_openflow_add_flow(fei_openflow_add_flow_req_t *pst_req)
{
    hsrv_openflow_flow_t* p_flow;

    HSRV_SWITCH_DEBUG("Enter into %s. table_id %d flow id process start.", __FUNCTION__, 
                       pst_req->flow.table, pst_req->flow.flow_id);
    HSRV_IF_ERROR_RETURN(hsrv_openflow_flowdb_lookup_flow(pst_req->flow.flow_id, &p_flow));
    if (p_flow)
    {
        return HSRV_E_INVALID_PARAM;
    }

    HSRV_IF_ERROR_RETURN(hsrv_openflow_alloc_flow_info(&pst_req->flow, &p_flow));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_alloc_id(p_flow));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_map_action(p_flow));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_create_stats(p_flow));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_add_flow(p_flow));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_flowdb_add_flow(p_flow));
    
    return HSRV_E_NONE;
}

int32 
fei_api_openflow_mod_flow(fei_openflow_mod_flow_req_t *pst_req)
{
    hsrv_openflow_flow_t* p_flow = NULL;
    hsrv_openflow_flow_info_t old_info;
    
    HSRV_IF_ERROR_RETURN(hsrv_openflow_lookup_flow_info(&pst_req->flow, &p_flow));

    /* modified by liwh for bug 46591 to support mpls tpoam, 2018-03-28 */
    HSRV_IF_ERROR_RETURN(hsrv_openflow_del_aps_nexthop(p_flow));
    /* liwh end */
    
    HSRV_IF_ERROR_RETURN(hsrv_openflow_move_flow_info(&p_flow->u, &old_info));

    /* need to update some field not in u*/
    sal_memcpy(&p_flow->flow.u.dft, &pst_req->flow.u.dft, sizeof(p_flow->flow.u.dft));
    
    /* Modifed by weizj for tunnel bug 34046 */
    /*added by wangl*/
    HSRV_IF_ERROR_RETURN(hsrv_tunneldb_cdb_oper_function(HSRV_TUNNEL_CDB_FLOW_DEL_UNMAP_ACTION, p_flow));
    
    HSRV_IF_ERROR_RETURN(hsrv_openflow_unmap_action(&old_info));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_map_action(p_flow));

    /* need carry old truncation info to update */
    if (old_info.dft.var.truncate_len)
    {
        p_flow->u.dft.var.old_truncate_len = old_info.dft.var.truncate_len;
    }
    
    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_update_flow(p_flow));

    return HSRV_E_NONE;
}

int32 
fei_api_openflow_del_flow(fei_openflow_del_flow_req_t *pst_req)
{
    hsrv_openflow_flow_t* p_flow = NULL;
    
    HSRV_IF_ERROR_RETURN(hsrv_openflow_flowdb_lookup_flow(pst_req->flow_id, &p_flow));
    if (NULL == p_flow)
    {
        return HSRV_E_NONE;
    }
    HSRV_IF_ERROR_RETURN(hsrv_openflow_del_flow(p_flow));
    
    return HSRV_E_NONE;
}

int32 
fei_api_openflow_reset_flow(fei_openflow_reset_flow_req_t *pst_req)
{
    hsrv_openflow_flowdb_loop_flow(hsrv_openflow_del_flow);
    
    return HSRV_E_NONE;
}

/* Added by weizj for tunnel bug 33724 */
int32 
fei_api_openflow_set_flow_drop_ingress(fei_openflow_set_drop_ingress *pst_req)
{
    hsrv_openflow_flowdb_set_drop_ingress(pst_req->is_drop);
    
    return HSRV_E_NONE;
}

/* Added by weizj for udf */
int32 
fei_api_openflow_udf_enable(fei_openflow_set_udf_parser *pst_req)
{
    hsrv_openflow_flowdb_udf_enable(pst_req->enable);
    if (pst_req->enable)
    {
        hsrv_openflow_msg_set_udf_enable(1);
        hsrv_openflow_port_set_udf_enable();
    }
    else
    {
        hsrv_openflow_msg_set_udf_enable(0);
        hsrv_openflow_port_set_udf_disable();
    }
    
    return HSRV_E_NONE;
}
/* End by weizj for udf */

int32 
fei_api_openflow_set_udf_parser(fei_openflow_set_udf_parser *pst_req)
{
    hsrv_openflow_msg_set_udf_parser(&pst_req->parser);
    
    return HSRV_E_NONE;
}
/* End by weizj for udf */

/* stats */
int32
fei_api_openflow_clear_flow_stats(fei_openflow_clear_stats_req_t *pst_req)
{
    hsrv_openflow_flow_t* p_flow = NULL;

    HSRV_IF_ERROR_RETURN(hsrv_openflow_flowdb_lookup_flow(pst_req->id, &p_flow));
    if (NULL == p_flow)
    {
        return HSRV_E_NONE;
    }
    HSRV_IF_ERROR_RETURN(hsrv_openflow_clear_flow_stats(p_flow));

    return HSRV_E_NONE;
}

int32
fei_api_openflow_flow_update_all_action_list(fei_openflow_update_all_action_list_req_t *pst_req)
{
    if (TRUE == pst_req->enable)
    {
        ctclib_hash_iterate(HSRV_OPENFLOW_ACTLIST_HASH, 
            hsrv_openflow_flow_add_port_to_all_action_list, pst_req);
    }
    else
    {
        ctclib_hash_iterate(HSRV_OPENFLOW_ACTLIST_HASH, 
            hsrv_openflow_flow_del_port_to_all_action_list, pst_req);
    }

    return HSRV_E_NONE;
}

int32
hsrv_tunneldb_cdb_modify_flow_action(hsrv_openflow_flow_t* p_flow)
{
        /*added by wangl*/
    HSRV_IF_ERROR_RETURN(hsrv_tunneldb_cdb_oper_function(HSRV_TUNNEL_CDB_FLOW_DEL_UNMAP_ACTION, p_flow));
    
    HSRV_IF_ERROR_RETURN(hsrv_openflow_unmap_action(&(p_flow->u)));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_map_action(p_flow));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_update_flow(p_flow));

    return 0;
}


#define _____DEBUG_____
static void 
hsrv_openflow_show_flow_brief__(ctclib_hash_backet_t* phb, void *user_data)
{
    hsrv_openflow_flow_t* p_flow = (hsrv_openflow_flow_t*)phb->data;
    hsrv_openflow_flow_debug_data_t* p_data = user_data; 
    uint32 nhid = 0;
    
    sal_fprintf(p_data->fp, "%5d %10u",
            p_data->i, 
            p_flow->flow.flow_id);

    if (p_flow->u.dft.var.normal)
    {
        sal_fprintf(p_data->fp, " normal");
    }
    else
    {
        if (p_flow->u.dft.var.group_info.group_id != OFP_GROUP_INVALID_GROUP_ID)
        {
            nhid = p_flow->u.dft.var.group_info.group_nhid;
        }
        else if (p_flow->u.dft.var.actlist)
        {
            nhid = p_flow->u.dft.var.actlist->nh_info.main_nh.nh_offset.nhid;
        }
        else
        {
            nhid = p_flow->u.dft.var.nh_info.main_nh.nh_offset.nhid;
        }
        
        sal_fprintf(p_data->fp, "%10u", nhid);
    }
    if (p_flow->u.dft.var.actlist)
    {
        sal_fprintf(p_data->fp, "     %p", 
            p_flow->u.dft.var.actlist);
    }
    else
    {
        sal_fprintf(p_data->fp, "%15s", "NULL");
    }
    sal_fprintf(p_data->fp, "%9u", p_flow->flow.stats_en);
    sal_fprintf(p_data->fp, "%9u", p_flow->u.dft.cnst.stats_id);
    sal_fprintf(p_data->fp, "%30s", 
                GLB_OPENFLOW_MPLS_ACTION_SEQ_TO_STR(p_flow->flow.u.dft.mpls_output_action_seq_type));
    sal_fprintf(p_data->fp, "\n");

    p_data->i ++;
}


int32
hsrv_openflow_show_flow_brief(FILE *fp)
{
    hsrv_openflow_flow_master_t* p_master = g_pst_openflow_flow_master;
    hsrv_openflow_flow_debug_data_t data;
    
    sal_fprintf(fp, "-------------HSRV OPENFLOW FLOW---------------------\n");
    sal_fprintf(fp, "Flow hash count    : %d\n", p_master->p_flow_hash->count);
    sal_fprintf(fp, "Actlist hash count : %d\n", p_master->p_actlist_hash->count);
    sal_fprintf(fp, "Global acl label   : %d\n", p_master->global_acl_label);
    sal_fprintf(fp, "----------------------------------------------------\n");

    sal_fprintf(fp, "%5s %10s %10s %15s %9s %9s %30s\n",
        "index", "flow_id", "nhid", "actlist",
        "stats_en", 
        "stats_id",
        "mpls_seqtype");
    sal_fprintf(fp, "----------------------------------------------------\n");

    data.fp = fp;
    data.i  = 0;
    
    ctclib_hash_iterate(HSRV_OPENFLOW_FLOW_HASH, hsrv_openflow_show_flow_brief__, &data);
    
    return HSRV_E_NONE;
}

void
hsrv_openflow_show_flow_nh_info(FILE *fp, hsrv_openflow_nexthop_info_t* p_nh_info)
{
    int32 member_count = 0;

    sal_fprintf(fp, "nexthop_info  \n");
    sal_fprintf(fp, "  use_mcast  : %d\n", p_nh_info->use_mcast);
    sal_fprintf(fp, "  main_nh    : %10d %10d %10s\n", 
        p_nh_info->main_nh.nh_offset.nhid,
        p_nh_info->main_nh.nh_offset.offset,
        HSRV_OF_NH_TYPE_STR(p_nh_info->main_nh.nh_offset.nh_type));
    if (p_nh_info->use_mcast)
    {
        sal_fprintf(fp, "  members    : %10s %10s %10s %12s\n", 
            "nhid", "offset", "type", "nexthop_ptr");
        sal_fprintf(fp, "               %10s %10s %10s %12s\n", 
            "----------", "----------", "----------", "------------");
        for (member_count = 0; member_count < p_nh_info->member_count; member_count ++)
        {
            sal_fprintf(fp, "  member %3d : %10d %10d %12s ",
                member_count,
                p_nh_info->member_nh[member_count].nh_offset.nhid,
                p_nh_info->member_nh[member_count].nh_offset.offset,
                HSRV_OF_NH_TYPE_STR(p_nh_info->member_nh[member_count].nh_offset.nh_type));
            if (p_nh_info->member_nh[member_count].share_info)
            {
                sal_fprintf(fp, "  %p",
                    p_nh_info->member_nh[member_count].share_info);
            }
            else
            {
                sal_fprintf(fp, "(NULL)");
            }
            sal_fprintf(fp, "\n");
        }
    }
}

int32
hsrv_openflow_show_flow(FILE *fp, uint32 flow_id)
{
    hsrv_openflow_flow_t* p_flow;

    HSRV_IF_ERROR_RETURN(hsrv_openflow_flowdb_lookup_flow(flow_id, &p_flow));
    if (NULL == p_flow)
    {
        sal_fprintf(fp, "flow_id %u not exist in db.\n", flow_id);
        return HSRV_E_NONE;
    }
    sal_fprintf(fp, "flow_id      : %u\n", flow_id);

    sal_fprintf(fp, "const_info \n");
    sal_fprintf(fp, "  entry_id   : %u\n", p_flow->u.dft.cnst.entry_id);
    sal_fprintf(fp, "  label_id   : %u\n", p_flow->u.dft.cnst.label_id);

    sal_fprintf(fp, "var_info \n");
    sal_fprintf(fp, "  normal     : %d\n", p_flow->u.dft.var.normal);
    sal_fprintf(fp, "  output_cnt : %d\n", p_flow->u.dft.var.output_count);
    if (p_flow->u.dft.var.actlist)
    {
        sal_fprintf(fp, "  actlist    : %p\n", 
            p_flow->u.dft.var.actlist);
    }
    else
    {
        sal_fprintf(fp, "  actlist    : NULL\n");
    }
    hsrv_openflow_show_flow_nh_info(fp, &p_flow->u.dft.var.nh_info);

    sal_fprintf(fp, "group_info \n");

    sal_fprintf(fp, "meter_info \n");

    sal_fprintf(fp, "queue_info \n");

    sal_fprintf(fp, "\n");


    return HSRV_E_NONE;
}

void
hsrv_openflow_show_flow_actlist_actions__(FILE *fp, glb_openflow_flow_action_list_t* p_actlist, int prefix)
{
    int32 act_cnt;
    glb_openflow_flow_action_t* p_action;

    for (act_cnt = 0; act_cnt < p_actlist->action_count; act_cnt ++)
    {
        p_action = p_actlist->actlist + act_cnt;

        glb_openflow_print_action(fp, p_action);
        if(act_cnt == (p_actlist->action_count - 1))
        {
            break;
        }
        
        sal_fprintf(fp, ",");
        if ((act_cnt + 1) % 4 == 0)
        {
            sal_fprintf(fp, "\n");
            if (1 == prefix)
            {
                sal_fprintf(fp, "%5s %12s %7s %10s ", "", "", "", "");
            }
            else if (2 == prefix)
            {
                sal_fprintf(fp, "             : ");
            }
        }
    }
}

static void 
hsrv_openflow_show_flow_actlist_brief__(ctclib_hash_backet_t* phb, void *user_data)
{
    hsrv_actlist_info_t* p_actlist = (hsrv_actlist_info_t*)phb->data;
    hsrv_openflow_flow_debug_data_t* p_data = user_data; 
    FILE *fp = p_data->fp;
    
    sal_fprintf(fp, "%5d ", p_data->i);
    sal_fprintf(fp, " %10p ", p_actlist);
    sal_fprintf(fp, "%4d ", p_actlist->ref_cnt);
    sal_fprintf(fp, "%10d ", p_actlist->nh_info.main_nh.nh_offset.nhid);
    hsrv_openflow_show_flow_actlist_actions__(fp, &p_actlist->actlist, 1);
    sal_fprintf(fp, "\n");
    
    p_data->i ++;
}

int32
hsrv_openflow_show_flow_actlist_brief(FILE *fp)
{
    hsrv_openflow_flow_debug_data_t data;
    
    data.fp = fp;
    data.i  = 0;
    
    sal_fprintf(fp, "----------------------------------------------------\n");
    
    sal_fprintf(fp, "%5s %12s %4s %10s %10s\n",
        "index", "ptr", "ref", "nhid", "actlist");
    
    ctclib_hash_iterate(HSRV_OPENFLOW_ACTLIST_HASH, hsrv_openflow_show_flow_actlist_brief__, &data);
    sal_fprintf(fp, "----------------------------------------------------\n");

    return HSRV_E_NONE;
}

void
hsrv_openflow_show_flow_actlist_ptr__(FILE *fp, hsrv_actlist_info_t* p_actlist)
{
    sal_fprintf(fp, "ptr          : %p\n", 
        p_actlist);
    sal_fprintf(fp, "actlist      : ");
    hsrv_openflow_show_flow_actlist_actions__(fp, &p_actlist->actlist, 2);
    sal_fprintf(fp, "\n");
    
    sal_fprintf(fp, "ref_count    : %d\n", p_actlist->ref_cnt);
    sal_fprintf(fp, "info         : in_port(%d) table_id(%d) table_miss(%d) eth_type(0x%x) proto(%u)\n",
        p_actlist->mdata.in_port.ofport,
        p_actlist->mdata.table_id,
        p_actlist->mdata.is_table_mis,
        p_actlist->mdata.eth_type,
        p_actlist->mdata.ip_proto);
    hsrv_openflow_show_flow_nh_info(fp, &p_actlist->nh_info);
}

int32
hsrv_openflow_show_flow_actlist_ptr(FILE *fp, uint32 actlist_ptr)
{
    sal_fprintf(fp, "not support actlist");
    return HSRV_E_NONE;
}

#define _____INIT_____

int32
hsrv_openflow_init_global_label(void)
{
    hsrv_openflow_label_t label;

    memset(&label, 0, sizeof(hsrv_openflow_label_t));
    HSRV_OPENFLOW_PTR_CHECK(g_pst_openflow_flow_master);
    
    label.type = GLB_OPENFLOW_LABEL_GLOBAL;
    label.enable = TRUE;
    label.label = HSRV_OPENFLOW_GLOBAL_LABEL_BASE;
    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_op_label(&label));
    g_pst_openflow_flow_master->global_acl_label = label.label;

    return HSRV_E_NONE;
}

int32
hsrv_openflow_flow_start(void)
{
    g_pst_openflow_flow_master = HSRV_CALLOC(MEM_FEA_HSRV_MODULE, sizeof(hsrv_openflow_flow_master_t));
    HSRV_OPENFLOW_MEM_CHECK(g_pst_openflow_flow_master);

    HSRV_IF_ERROR_RETURN(hsrv_openflow_flowdb_start());

    return HSRV_E_NONE;
}

int32
hsrv_openflow_flow_init(hsrv_system_spec_info_t* p_system_info)
{
    HSRV_IF_ERROR_RETURN(hsrv_openflow_init_global_label());
    
    return HSRV_E_NONE;
}

#define _____END_LINE_____
