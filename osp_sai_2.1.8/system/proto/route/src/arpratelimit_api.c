#include "proto.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_route_global_define.h"
#include "gen/tbl_route_global.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_iproute_node_define.h"
#include "gen/tbl_iproute_node.h"
#include "gen/tbl_arp_define.h"
#include "gen/tbl_arp.h"
#include "gen/tbl_arp_fib_define.h"
#include "gen/tbl_arp_fib.h"
#include "gen/tbl_kernel_if_define.h"
#include "gen/tbl_kernel_if.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_nexthop_define.h"
#include "gen/tbl_nexthop.h"
#include "gen/tbl_sys_spec_define.h"
#include "gen/tbl_sys_spec.h"
#include "gen/tbl_chassis_define.h"
#include "gen/tbl_chassis.h"
#include "route.h"
#include "prefix.h"
#include "rt_fea.h"
#include "lib_ioctl.h"
#include "arp_define.h"
#include "arp_debug.h"
#include "arp_api.h"
#include "arpinsp_api.h"
#include "arpratelimit_api.h"
#include "arp_ether.h"
#include "arp_sm.h"
#include "lib_netlink.h"
#include "hsrv_msg.h"

int32
arpratelimit_is_arpratelimit_l3_port(tbl_interface_t *p_db_if)
{
    char ifname_ext[IFNAME_EXT_SIZE] = {0};

    if (NULL == p_db_if)
    {
        ARPINSP_PROTO_DEBUG("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return PM_E_ARP_RATE_LIMIT_ERR;
    }
    
    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
    
    if (INTERFACE_IS_LAG_MEMBER(p_db_if))
    {
        ARPINSP_PROTO_DEBUG("(%s, %d) Interface %s is lag member port\n", __FUNCTION__, __LINE__, ifname_ext);
        return FALSE;
    }
    
    if (!(GLB_IF_TYPE_PORT_IF == p_db_if->hw_type && GLB_IF_MODE_L3 == p_db_if->mode) && 
        GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type && GLB_IF_TYPE_VLAN_IF != p_db_if->hw_type)
    {
        ARPINSP_PROTO_DEBUG("(%s, %d) Interface %s is not routed port or LAG or vlanif, hw_type(%d) mode(%d)\n", 
            __FUNCTION__, __LINE__, ifname_ext, p_db_if->hw_type, p_db_if->mode);
        return FALSE;
    }
    
    if (GLB_IF_MODE_L3 != p_db_if->mode)
    {
        ARPINSP_PROTO_DEBUG("(%s, %d) Interface %s is not routed port or LAG or vlanif \n", __FUNCTION__, __LINE__, ifname_ext);
        return FALSE;
    }

    return TRUE;
}

void
arpratelimit_init_para_for_add_if(tbl_route_if_t *p_rt_if)
{
    tbl_interface_t *p_db_if = NULL;
    tbl_route_if_t *p_rt_if_tmp = NULL;
    tbl_route_if_t *p_rt_if_eth = NULL;
    int32 memrc = PM_E_NONE;
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
        
    if (NULL == p_rt_if)
    {
        ARPINSP_PROTO_DEBUG("(%s, %d) Input parameter is null\n", __FUNCTION__, __LINE__);
        return;
    }

    p_db_if = tbl_interface_get_interface_by_name(p_rt_if->key.name);
    if (NULL == p_db_if)
    {
        IFNAME_ETH2FULL(p_rt_if->key.name, ifname_ext);
        ARPINSP_PROTO_DEBUG("(%s, %d) Interface %s not exist\n", __FUNCTION__, __LINE__, ifname_ext);
        return;
    }

    //rc = arpratelimit_is_arpratelimit_l3_port(p_db_if);

    /*not agg port, init*/
    if (GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type && 
        !(GLB_IF_TYPE_PORT_IF == p_db_if->hw_type && INTERFACE_IS_LAG_MEMBER(p_db_if)))//for eth add to agg
    {
        /*modified by chenc for bug 51010, 2019-02-14*/
        p_rt_if->arp_rate_limit_en = ARP_RATE_LIMIT_DISABLE;
        /*chenc ended*/
        p_rt_if->arp_rate_limit_violation = ARP_RATE_LIMIT_ACTION_RESTRICT;
        p_rt_if->arp_rate_limit_pkt_max = ARP_RATE_LIMIT_DEFAULT_NUMBER;
        p_rt_if->arp_rate_limit_port_abnormal_flag = ARP_RATE_LIMIT_ABNORMAL_FALSE;
        p_rt_if->arp_rate_limit_pkt_curr = 0;
    }
    else
    {
        /*set default value*/
 //       p_rt_if->arp_rate_limit_en = ARP_RATE_LIMIT_ENABLE;

        /*agg port , get eth para to init agg*/
        /*sync eth memport cfg to agg*/
        if (INTERFACE_IS_LAG_MEMBER(p_db_if))
        {
            uint8 lag_id = 0;
            char ifname[IFNAME_SIZE] = {0};
            lag_id = p_db_if->lag->lag_id;
            sal_sprintf(ifname, "agg%u", lag_id);

            p_rt_if_eth = tbl_route_if_get_route_if_by_name(p_db_if->key.name);
            p_rt_if_tmp = tbl_route_if_get_route_if_by_name(ifname);
            if (NULL != p_rt_if_tmp && NULL != p_rt_if_eth )//&& ARP_RATE_LIMIT_STATE_MAX == p_rt_if_tmp->arp_rate_limit_en)
            {
                memrc += arpratelimit_set_enable(p_rt_if_tmp, p_rt_if_eth->arp_rate_limit_en);
                memrc += arpratelimit_set_exceed_action(p_rt_if_tmp, p_rt_if_eth->arp_rate_limit_violation);
                memrc += arpratelimit_set_pkt_max(p_rt_if_tmp, p_rt_if_eth->arp_rate_limit_pkt_max);
                if (PM_E_NONE != memrc)
                {
                    ARPINSP_PROTO_DEBUG("(%s, %d) Set para err when add port %s para to agg\n", __FUNCTION__, __LINE__, ifname);
                    return ;
                }
                
                /*after eth add to agg ,clear eth pkt cur and abnormal flag*/
                p_rt_if_eth->arp_rate_limit_pkt_curr = 0;
                p_rt_if_eth->arp_rate_limit_port_abnormal_flag = ARP_RATE_LIMIT_ABNORMAL_FALSE;

                /*after eth add to agg ,set eth rate limit to default*/
                /*modified by chenc for bug 51010, 2019-02-14*/
                memrc += arpratelimit_set_enable(p_rt_if_eth, ARP_RATE_LIMIT_DISABLE);
                /*ended*/
                memrc += arpratelimit_set_exceed_action(p_rt_if_eth, ARP_RATE_LIMIT_ACTION_RESTRICT);
                memrc += arpratelimit_set_pkt_max(p_rt_if_eth, ARP_RATE_LIMIT_DEFAULT_NUMBER);
                if (PM_E_NONE != memrc)
                {    
                    ARPINSP_PROTO_DEBUG("(%s, %d) Set para default value err when add port %s para to agg\n", __FUNCTION__, __LINE__, ifname);
                    return ;
                }
            }
        }
    }
    
    return ;
}

int32
arpratelimit_set_exceed_action(tbl_route_if_t *p_db_rtif, uint8 action)
{
    int32 rc = PM_E_NONE;
    tbl_interface_t *p_db_if = NULL;
    char ifname_ext[IFNAME_EXT_SIZE] = {0};

    /*check*/    
    p_db_if = tbl_interface_get_interface_by_name(p_db_rtif->key.name);    
    if (NULL == p_db_if)    
    {        
        IFNAME_ETH2FULL(p_db_rtif->key.name, ifname_ext);
        ARPINSP_PROTO_DEBUG("(%s, %d) Interface %s not exist \n", __FUNCTION__, __LINE__, ifname_ext);        
        return PM_E_ARP_RATE_LIMIT_ERR;    
    }
    
    /* 2. update local CDB */
    p_db_rtif->arp_rate_limit_violation = action;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ARP_RATE_LIMIT_VIOLATION));
    
    return rc;
}

int32
arpratelimit_set_pkt_max(tbl_route_if_t *p_db_rtif, uint32 pktmax)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;
    char ifname_ext[IFNAME_EXT_SIZE] = {0};

    /*check*/    
    p_db_if = tbl_interface_get_interface_by_name(p_db_rtif->key.name);
    if (NULL == p_db_if)    
    {        
        IFNAME_ETH2FULL(p_db_rtif->key.name, ifname_ext);
        ARPINSP_PROTO_DEBUG("(%s, %d) Interface %s not exist\n", __FUNCTION__, __LINE__, ifname_ext);        
        return PM_E_ARP_RATE_LIMIT_ERR;    
    }
    
    /* 2. update local CDB */
    p_db_rtif->arp_rate_limit_pkt_max = pktmax;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ARP_RATE_LIMIT_PKT_MAX));
    
    return rc;
}

int32
arpratelimit_set_enable(tbl_route_if_t *p_db_rtif, uint8 enable)
{          
    int32 rc = PM_E_NONE;
    tbl_interface_t *p_db_if = NULL;
    char ifname_ext[IFNAME_EXT_SIZE] = {0};

    /*check*/    
    p_db_if = tbl_interface_get_interface_by_name(p_db_rtif->key.name);    
    if (NULL == p_db_if)    
    {        
        IFNAME_ETH2FULL(p_db_rtif->key.name, ifname_ext);
        ARPINSP_PROTO_DEBUG("(%s, %d) Interface %s not exist\n", __FUNCTION__, __LINE__, ifname_ext);        
        return PM_E_ARP_RATE_LIMIT_ERR;    
    }
    
    if (FALSE == enable)
    {
        /*clean the port rate limit data when disable arp rate limit*/
        p_db_rtif->arp_rate_limit_pkt_curr = 0;
        p_db_rtif->arp_rate_limit_port_abnormal_flag = ARP_RATE_LIMIT_ABNORMAL_FALSE;
    }
    
    /* 2. update local CDB */
    p_db_rtif->arp_rate_limit_en = enable;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ARP_RATE_LIMIT_EN));
    
    return rc;
}

