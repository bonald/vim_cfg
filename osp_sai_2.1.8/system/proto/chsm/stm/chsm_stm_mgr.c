/****************************************************************************
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision     :       R0.01
 * Author       :       Centec
 * Date         :       2015-08-06
 * Reason       :       First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "chsm_inc.h"

/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/
#define STM_LINE_LEN_MAX                128
/****************************************************************************
 *
* Global and Declaration
*
*****************************************************************************/
glb_system_fea_spec_t* g_p_shm_stm_alloc = NULL;

/****************************************************************************
 *
* Function
*
*****************************************************************************/

int32
chsm_stm_mgr_read_profile_mode(uint8* p_mode)
{
    int32 ret = PM_E_NONE;
    uint8 tcam_mode = 0;

    CHSM_CTC_CHK_PTR(p_mode);

    tcam_mode = chsm_mgt_get_stm_mode();
    if (tcam_mode >= GLB_STM_MODE_MAX)
    {
        CHSM_LOG_ERR("%% Get stm profile mode failed, use default mode.");
        tcam_mode = GLB_STM_MODE_DEFAULT;
    }
    *p_mode = tcam_mode;

    return ret;
}


int32
chsm_stm_mgr_write_profile_type(uint8 type)
{
    int32 ret = PM_E_NONE;
    char file_name[STM_LINE_LEN_MAX];
    char description[2*STM_LINE_LEN_MAX];
    char profile_mode[STM_LINE_LEN_MAX];
    FILE* fp = NULL;

    chsm_mgt_set_stm_mode(type);

    if (chsm_mgt_get_stm_mode() == type)
    {
        ret = PM_E_NONE;
    }

    /*get file path*/
    sal_snprintf(file_name, sizeof(file_name), "%s%s", GLB_STM_PROFILE_PATH, GLB_STM_PROFILE_MODE);

    /*OPEN FILE*/
    fp = sal_fopen(file_name, "w+");
    if((NULL == fp))
    {
        return PM_E_FAIL;
    }

    /*write file information*/
    sal_snprintf(description, sizeof(description), "%s\r\n%s\r\n",
              "#this file is only used for storing tcam allocation profile mode.",
              "#0=default, 1=ipv4 route, 2=vlan, 3=l2vpn, 4=ipv6.\r\n");
    sal_snprintf(profile_mode, sizeof(profile_mode), "[PROFILE_MODE] = %d", type);

    sal_fputs(description, fp);
    sal_fputs(profile_mode, fp);

    /*close file*/
    sal_fclose(fp);
    fp = NULL;


    return ret;
}

#ifndef TAPPRODUCT
static int32
_chsm_stm_mgr_init_default_spec_info(uint8 type, tbl_sys_spec_t* p_spec_info)
{
    CHSM_CTC_CHK_PTR(p_spec_info);

    p_spec_info->key.type = type;

    /*2. feature specification information*/
/*addd by wangl for hybrid profile*/
#if defined(GOLDENGATE) || defined(USW)
#ifdef OFPRODUCT
    p_spec_info->ucast_fdb = 32768;
#else
    p_spec_info->ucast_fdb = 65536;
#endif
#else
#ifdef OFPRODUCT
    p_spec_info->ucast_fdb = 16384;
#else
    p_spec_info->ucast_fdb = 32768;
#endif
#endif

/*modified by yejl for bug48406, macfilter profile, 2018-08-14*/
#ifdef OFPRODUCT
    p_spec_info->mac_filter = 128; 
#else 
    p_spec_info->mac_filter = 116; /*128 - 12 = 116: l2protocol protocl-mac and group-mac number is 12*/
#endif
    p_spec_info->mstp_instance = GLB_STP_INSTANCE_MAX;
    p_spec_info->vlan_instance = 4094;
    p_spec_info->vlan_member_ports = 16;/*now all 52 ports can be added, use bitmap, fixed*/
    p_spec_info->lag_group_num = GLB_LAG_GROUP_NUM;
        
    /*vlan class, total usrid key 4K, usrid mac keysize 160bit, */
    p_spec_info->mac_based_vlan_class = 512;/*80bit userid mac hash*/
    p_spec_info->dot1x_mac = 256;/*80bit userid mac hash*/
    p_spec_info->ipv4_based_vlan_class = 512; /*80bit userid ip hash, with default entry in tcam*/
    p_spec_info->ipv4_source_guard = 1024;/*80bit userid ip hash, with default entry in tcam*/
    p_spec_info->vlan_mapping_entry_applied_port = 1024; /*80bit userid vlan hash, use tcam to reslove conflict*/

    /*multicast and unicast*/
    p_spec_info->l2mc_entries = 1024; /*see nexthop resource of l2mc, shared with ipv6*/
    p_spec_info->l2mc_member_ports = 16; /*now all 52 ports can be added, use bitmap, fixed*/
    p_spec_info->l3mc_entries = 512; /*see key resource of ipv4mc tcam*/
    p_spec_info->l3mc_member_ports = 4; /*average 2 vlanif and 2 routed port, fixed*/
    p_spec_info->host_routes = 4096; /*see nexthop resource of host route, shared with ipv6*/
    p_spec_info->indirect_routes = 8192; /*see key resource of lpm, shared with ipv6*/
    p_spec_info->static_routes = 8192; /*see key resource of lpm, shared with ipv6*/
    p_spec_info->indirect_v6_routes = 8192; /*see key resource of lpm, shared with ipv6*/
    p_spec_info->static_v6_routes = 8192; /*see key resource of lpm, shared with ipv6*/
#if defined(GOLDENGATE) || defined(USW)
    p_spec_info->ecmp_routes = 239;
    p_spec_info->ecmp_member_maximum = IP_ECMP_MAX_PATH;
    p_spec_info->ecmp_v6_member_maximum = IP_ECMP_MAX_PATH;
#else
    p_spec_info->ecmp_routes = 239;
    p_spec_info->ecmp_member_maximum = IP_ECMP_MAX_PATH_GB;
    p_spec_info->ecmp_v6_member_maximum = IP_ECMP_MAX_PATH_GB;
#endif
    p_spec_info->ospf_area_range_num = GLB_OSPF_AREA_RANGE_MAX;

#if defined(GOLDENGATE) || defined(USW)
#ifdef OFPRODUCT
    p_spec_info->flow_entries_ingress = 100;
#else
    p_spec_info->flow_entries_ingress = 4096 - 1; /*ipv4 acl tcam, tap acl need use stats,stats support 4095*/
#endif
   /*test by wangl for hybrid*/
    p_spec_info->openflow_flow_entries = 4000; /*openflow ipv4 acl tcam*/
#else
    #ifdef OFPRODUCT
    p_spec_info->flow_entries_ingress = 100;
    #else
    p_spec_info->flow_entries_ingress = 960;
    #endif
#endif
    p_spec_info->flow_entries_egress = 256;
#ifdef OFPRODUCT
    p_spec_info->flow_entries_egress = 0;
#endif
    p_spec_info->pbr_entries = 32;
    p_spec_info->voice_vlan = 16;

    /*IPv6 features*/
    p_spec_info->ipv6_based_vlan_class = 0;/*160bit keysize, use hash usrid, default entry stored in tcam*/
    p_spec_info->ipv6_source_guard = 256; /*160bit keysize, use hash usrid, default entry stored in tcam*/
    p_spec_info->l2mc_v6_entries = 1024; /*see nexthop resource of l2mc, shared with ipv4*/
    p_spec_info->l2mc_v6_member_ports = 16;/*now all 52 ports can be added, use bitmap, fixed*/
    p_spec_info->l3mc_v6_entries = 128; /*see key resource of ipv6mc tcam*/
    p_spec_info->l3mc_v6_member_ports = 4;  /*average 2 vlanif and 2 routed port*/
    p_spec_info->flow_v6_entries = 128;
    p_spec_info->host_v6_routes = 0; /*see nexthop resource of host route, shared with ipv4*/
    p_spec_info->indirect_v6_routes = 0;/*see key resource of lpm, shared with ipv4*/
    p_spec_info->static_v6_routes = 0;/*see key resource of lpm, shared with ipv4*/
    p_spec_info->ecmp_v6_routes = 14;
    p_spec_info->ip_tunnel = 8; /*auto tunnel use tcam, static tunnel use usrid, ipsa only have 3 bits*/
    p_spec_info->ivi_peers = 32; /*always fixed*/

    /*cfm and g8031, g8032*/
    p_spec_info->cfm_lck = 48;/*48*4 acl entries*/
    p_spec_info->cfm_local_and_remote_meps = 2048;
    p_spec_info->g8031_groups = 64;
    p_spec_info->g8032_rings = 32;
    p_spec_info->g8032_member_ports_per_ring = 8;

    p_spec_info->bfd_sessions = 0;


    /*mpls*/
    p_spec_info->ftn_entries = 0;
    p_spec_info->ilm_entries = 0;
    p_spec_info->mpls_lables = 0;

    /*l2vpn*/
    p_spec_info->vpws = 0;
    p_spec_info->vpls_ac = 0;
    p_spec_info->vpls_peer = 0;
    p_spec_info->lsp_oam = 0;
    p_spec_info->lsp_p = 0;
    p_spec_info->lsp_pe = 0;
    p_spec_info->pw_oam = 0;
    p_spec_info->mpls_aps_tunnel = 0;
    p_spec_info->vsi = 0;

    /*nvgre*/
    p_spec_info->nvgre_tunnel_id_mappings = 0;
    p_spec_info->nvgre_peers = 0;
    p_spec_info->nvgre_tunnels = 0;

    /*qos*/
    p_spec_info->policer_profile_num = GLB_QOS_POLICER_PROFILE_NUM;
    p_spec_info->drop_profile_num = GLB_QOS_DROP_PROFILE_NUM;
    p_spec_info->queue_shape_profile_num = GLB_QOS_SHAPE_PROFILE_NUM; /*for queue shape profile, port profile is separate in GG*/
    p_spec_info->port_shape_profile_num = GLB_QOS_PORT_SHAPE_PROFILE_NUM;

    p_spec_info->l2protocol_num = 8;

    #if defined(DUET2)
    #ifndef OFPRODUCT
        p_spec_info->ucast_fdb = 32768;
        p_spec_info->host_routes = 3072; /*see nexthop resource of host route, shared with ipv6*/
        p_spec_info->static_routes = 6144; /*see key resource of lpm, shared with ipv6*/
        p_spec_info->indirect_routes = 6144; /*see key resource of lpm, shared with ipv6*/
        //p_spec_info->flow_entries_ingress = 2464; /*ipv4 acl tcam, tap acl need use stats,stats support 4095*/
        p_spec_info->flow_entries_ingress = 1232;
        //p_spec_info->flow_entries_egress = 928;
        p_spec_info->flow_entries_egress = 464;
        p_spec_info->lag_group_num = GLB_LAG_GROUP_NUM;
    #endif
    #endif

    #if defined(TSINGMA)
    #ifndef OFPRODUCT
        p_spec_info->ucast_fdb = 32768;
        p_spec_info->host_routes = 4096; /*see nexthop resource of host route, shared with ipv6*/
        p_spec_info->static_routes = 2048; /*see key resource of lpm, shared with ipv6*/
        p_spec_info->indirect_routes = 8192; /*see key resource of lpm, shared with ipv6*/
        //p_spec_info->flow_entries_ingress = 2464; /*ipv4 acl tcam, tap acl need use stats,stats support 4095*/
        p_spec_info->flow_entries_ingress = 2048; /*modified by yejl for bug 54220, 2019-11-09*/
        //p_spec_info->flow_entries_egress = 928;
        p_spec_info->flow_entries_egress = 464;
        p_spec_info->lag_group_num = GLB_LAG_GROUP_NUM;
    #endif
    #endif

    #if defined(TSINGMA)
    #ifdef OFPRODUCT
        p_spec_info->static_routes = 2048; /*see key resource of lpm, shared with ipv6*/
    #endif
    #endif
    
    /*begin add by taocy for BUG52801*/
    p_spec_info->nat_pool_num                     = GLB_NAT_POOL_NUM;
    p_spec_info->nat_rule_num                     = GLB_NAT_RULE_NUM_MAX;
    p_spec_info->nat_inside_rtif_num              = GLB_NAT_INSIDE_IF_NUM;
    p_spec_info->nat_outside_rtif_num             = GLB_NAT_OUTSIDE_IF_NUM;
    p_spec_info->nat_session_upstream_num         = NAT_SESSION_SPEC;
    p_spec_info->nat_session_dnstream_num         = NAT_SESSION_SPEC;
    p_spec_info->nat_session_fast_path_entry_num  = NAT_SESSION_SPEC;
    p_spec_info->nat_session_limit_node_num       = GLB_NAT_SESSION_LIMIT_NODE_NUM_MAX;
    /*end add by taocy for BUG52801*/
    return PM_E_NONE;
}
#endif

static int32
_chsm_stm_mgr_init_ipv4_spec_info(uint8 type, tbl_sys_spec_t* p_spec_info)
{
    CHSM_CTC_CHK_PTR(p_spec_info);

    p_spec_info->key.type = type;

    /*2. feature specification information*/
    p_spec_info->ucast_fdb = 32768;
    p_spec_info->mac_filter = 64;
    p_spec_info->vlan_instance = 4094;
    p_spec_info->vlan_member_ports = 16;/*temp for testing*/
    p_spec_info->lag_group_num = GLB_LAG_GROUP_NUM;
    
    /*vlan class*/
    p_spec_info->mac_based_vlan_class = 256;
    p_spec_info->dot1x_mac = 256;/*80bit userid mac hash*/
    p_spec_info->ipv4_based_vlan_class = 512;
    p_spec_info->ipv4_source_guard = 512;
    p_spec_info->vlan_mapping_entry_applied_port = 1536;

    /*multicast and unicast*/
    p_spec_info->l2mc_entries = 1024;
    p_spec_info->l2mc_member_ports = 16;
    p_spec_info->l3mc_entries = 512;
    p_spec_info->l3mc_member_ports = 4;
    //p_spec_info->host_routes = 24576;
    p_spec_info->host_routes = 6144;
    p_spec_info->indirect_routes = 12288;
    p_spec_info->static_routes = 12288;
    p_spec_info->ecmp_routes = 30;

    p_spec_info->ospf_area_range_num = GLB_OSPF_AREA_RANGE_MAX;

    p_spec_info->flow_entries_ingress = 1536;
    p_spec_info->pbr_entries = 64;
    p_spec_info->voice_vlan = 16;

    /*IPv6 features*/
    p_spec_info->ipv6_based_vlan_class = 0;
    p_spec_info->ipv6_source_guard = 0;
    p_spec_info->l2mc_v6_entries = 0;
    p_spec_info->l2mc_v6_member_ports = 0;
    p_spec_info->l3mc_v6_entries = 0;
    p_spec_info->l3mc_v6_member_ports = 0;
    p_spec_info->flow_v6_entries = 0;
    p_spec_info->host_v6_routes = 0;
    p_spec_info->indirect_v6_routes = 0;
    p_spec_info->static_v6_routes = 0;
    p_spec_info->ecmp_v6_routes = 0;
    p_spec_info->ip_tunnel = 0;
    p_spec_info->ivi_peers = 0;

    /*cfm and g8031, g8032*/
    p_spec_info->cfm_lck = 0;
    p_spec_info->cfm_local_and_remote_meps = 0;
    p_spec_info->g8031_groups = 0;
    p_spec_info->g8032_rings = 0;
    p_spec_info->g8032_member_ports_per_ring = 0;

    /*mpls*/
    p_spec_info->ftn_entries = 0;
    p_spec_info->ilm_entries = 0;
    p_spec_info->mpls_lables = 0;

    /*l2vpn*/
    p_spec_info->vpws = 0;
    p_spec_info->vpls_ac = 0;
    p_spec_info->vpls_peer = 0;
    p_spec_info->lsp_oam = 0;
    p_spec_info->lsp_p = 0;
    p_spec_info->lsp_pe = 0;
    p_spec_info->pw_oam = 0;
    p_spec_info->mpls_aps_tunnel = 0;
    p_spec_info->vsi = 0;

    /*nvgre*/
    p_spec_info->nvgre_tunnel_id_mappings = 0;
    p_spec_info->nvgre_peers = 0;
    p_spec_info->nvgre_tunnels = 0;


    return PM_E_NONE;
}


static int32
_chsm_stm_mgr_init_vlan_spec_info(uint8 type, tbl_sys_spec_t* p_spec_info)
{
    CHSM_CTC_CHK_PTR(p_spec_info);

    p_spec_info->key.type = type;

    /*2. feature specification information*/
    p_spec_info->ucast_fdb = 65536;
    p_spec_info->mac_filter = 64;
    p_spec_info->vlan_instance = 4094;
    p_spec_info->vlan_member_ports = 16;/*temp for testing*/
    p_spec_info->lag_group_num = GLB_LAG_GROUP_NUM;
    
    /*vlan class*/
    p_spec_info->mac_based_vlan_class = 2560;
    p_spec_info->dot1x_mac = 512;/*80bit userid mac hash*/
    p_spec_info->ipv4_based_vlan_class = 1024;
    p_spec_info->ipv4_source_guard = 1024;
    p_spec_info->vlan_mapping_entry_applied_port = 2048;

    /*multicast and unicast*/
    p_spec_info->l2mc_entries = 1024;
    p_spec_info->l2mc_member_ports = 16;
    p_spec_info->l3mc_entries = 256;
    p_spec_info->l3mc_member_ports = 4;
    p_spec_info->host_routes = 1024;
    p_spec_info->indirect_routes = 2048;
    p_spec_info->static_routes = 2048;
    p_spec_info->ecmp_routes = 16;

    p_spec_info->ospf_area_range_num = GLB_OSPF_AREA_RANGE_MAX;

    p_spec_info->flow_entries_ingress = 1536;
    p_spec_info->pbr_entries = 0;
    p_spec_info->voice_vlan = 16;

    /*IPv6 features*/
    p_spec_info->ipv6_based_vlan_class = 0;
    p_spec_info->ipv6_source_guard = 0;
    p_spec_info->l2mc_v6_entries = 0;
    p_spec_info->l2mc_v6_member_ports = 0;
    p_spec_info->l3mc_v6_entries = 0;
    p_spec_info->l3mc_v6_member_ports = 0;
    p_spec_info->flow_v6_entries = 0;
    p_spec_info->host_v6_routes = 0;
    p_spec_info->static_v6_routes = 0;
    p_spec_info->indirect_v6_routes = 0;
    p_spec_info->ecmp_v6_routes = 0;
    p_spec_info->ip_tunnel = 0;
    p_spec_info->ivi_peers = 0;

    /*cfm and g8031, g8032*/
    p_spec_info->cfm_lck = 48;
    p_spec_info->cfm_local_and_remote_meps = 2048;
    p_spec_info->g8031_groups = 64;
    p_spec_info->g8032_rings = 32;
    p_spec_info->g8032_member_ports_per_ring = 8;

    /*mpls*/
    p_spec_info->ftn_entries = 0;
    p_spec_info->ilm_entries = 0;
    p_spec_info->mpls_lables = 0;

    /*l2vpn*/
    p_spec_info->vpws = 0;
    p_spec_info->vpls_ac = 0;
    p_spec_info->vpls_peer = 0;
    p_spec_info->lsp_oam = 0;
    p_spec_info->lsp_p = 0;
    p_spec_info->lsp_pe = 0;
    p_spec_info->pw_oam = 0;
    p_spec_info->mpls_aps_tunnel = 0;
    p_spec_info->vsi = 0;

    /*nvgre*/
    p_spec_info->nvgre_tunnel_id_mappings = 0;
    p_spec_info->nvgre_peers = 0;
    p_spec_info->nvgre_tunnels = 0;

    return PM_E_NONE;
}

static int32
_chsm_stm_mgr_init_l2vpn_spec_info(uint8 type, tbl_sys_spec_t* p_spec_info)
{
    CHSM_CTC_CHK_PTR(p_spec_info);

    p_spec_info->key.type = type;

    /*2. feature specification information*/
    p_spec_info->ucast_fdb = 32768;
    p_spec_info->mac_filter = 128;
    p_spec_info->vlan_instance = 1024;
    p_spec_info->vlan_member_ports = 16;
    p_spec_info->lag_group_num = GLB_LAG_GROUP_NUM;
    
    /*vlan class*/
    p_spec_info->mac_based_vlan_class = 256;
    p_spec_info->dot1x_mac = 256;/*80bit userid mac hash*/
    p_spec_info->ipv4_based_vlan_class = 128;
    p_spec_info->ipv4_source_guard = 128;
    p_spec_info->vlan_mapping_entry_applied_port = 512;

    /*multicast and unicast*/
    p_spec_info->l2mc_entries = 256;
    p_spec_info->l2mc_member_ports = 8;
    p_spec_info->l3mc_entries = 256;
    p_spec_info->l3mc_member_ports = 8;
    p_spec_info->host_routes = 1024;
    p_spec_info->static_routes = 768;
    p_spec_info->indirect_routes = 768;
    p_spec_info->ecmp_routes = 32;

    p_spec_info->ospf_area_range_num = GLB_OSPF_AREA_RANGE_MAX;

    p_spec_info->flow_entries_ingress = 256;
    p_spec_info->pbr_entries = 128;
    p_spec_info->voice_vlan = 32;

    /*IPv6 features*/
    p_spec_info->ipv6_based_vlan_class = 128;
    p_spec_info->ipv6_source_guard = 128;
    p_spec_info->l2mc_v6_entries = 512;
    p_spec_info->l2mc_v6_member_ports = 8;
    p_spec_info->l3mc_v6_entries = 384;
    p_spec_info->l3mc_v6_member_ports = 8;
    p_spec_info->flow_v6_entries = 384;
    p_spec_info->host_v6_routes = 1024;
    p_spec_info->static_v6_routes = 2048;
    p_spec_info->indirect_v6_routes = 2048;
    p_spec_info->ecmp_v6_routes = 32;
    p_spec_info->ip_tunnel = 64;
    p_spec_info->ivi_peers = 32;

    /*cfm and g8031, g8032*/
    p_spec_info->cfm_lck = 0;
    p_spec_info->cfm_local_and_remote_meps = 0;
    p_spec_info->g8031_groups = 0;
    p_spec_info->g8032_rings = 0;
    p_spec_info->g8032_member_ports_per_ring = 0;

    /*mpls*/
    p_spec_info->ftn_entries = 0;
    p_spec_info->ilm_entries = 0;
    p_spec_info->mpls_lables = 0;

    /*l2vpn*/
    p_spec_info->vpws = 0;
    p_spec_info->vpls_ac = 0;
    p_spec_info->vpls_peer = 0;
    p_spec_info->lsp_oam = 0;
    p_spec_info->lsp_p = 0;
    p_spec_info->lsp_pe = 0;
    p_spec_info->pw_oam = 0;
    p_spec_info->mpls_aps_tunnel = 0;
    p_spec_info->vsi = 0;

    /*nvgre*/
    p_spec_info->nvgre_tunnel_id_mappings = 0;
    p_spec_info->nvgre_peers = 0;
    p_spec_info->nvgre_tunnels = 0;

    return PM_E_NONE;
}

static int32
_chsm_stm_mgr_init_ipv6_spec_info(uint8 type, tbl_sys_spec_t* p_spec_info)
{
    CHSM_CTC_CHK_PTR(p_spec_info);

    p_spec_info->key.type = type;

    /*2. feature specification information*/
/*addd by wangl for hybrid profile*/
#if defined(GOLDENGATE) || defined(USW)
#ifdef OFPRODUCT
    p_spec_info->ucast_fdb = 32768;
#else
    p_spec_info->ucast_fdb = 65536;
#endif
#else
#ifdef OFPRODUCT
    p_spec_info->ucast_fdb = 16384;
#else
    p_spec_info->ucast_fdb = 32768;
#endif
#endif

    p_spec_info->mac_filter = 116; /*128 - 12 = 116: l2protocol protocl-mac and group-mac number is 12*/
    p_spec_info->mstp_instance = GLB_STP_INSTANCE_MAX;
    p_spec_info->vlan_instance = 4094;
    p_spec_info->vlan_member_ports = 16;/*now all 52 ports can be added, use bitmap, fixed*/
    p_spec_info->lag_group_num = GLB_LAG_GROUP_NUM;
        
    /*vlan class, total usrid key 4K, usrid mac keysize 160bit, */
    p_spec_info->mac_based_vlan_class = 512;/*80bit userid mac hash*/
    p_spec_info->dot1x_mac = 256;/*80bit userid mac hash*/
    p_spec_info->ipv4_based_vlan_class = 512; /*80bit userid ip hash, with default entry in tcam*/
    p_spec_info->ipv4_source_guard = 1024;/*80bit userid ip hash, with default entry in tcam*/
    p_spec_info->vlan_mapping_entry_applied_port = 1024; /*80bit userid vlan hash, use tcam to reslove conflict*/

    /*multicast and unicast*/
    p_spec_info->l2mc_entries = 1024; /*see nexthop resource of l2mc, shared with ipv6*/
    p_spec_info->l2mc_member_ports = 16; /*now all 52 ports can be added, use bitmap, fixed*/
    p_spec_info->l3mc_entries = 512; /*see key resource of ipv4mc tcam*/
    p_spec_info->l3mc_member_ports = 4; /*average 2 vlanif and 2 routed port, fixed*/
    p_spec_info->host_routes = 2048; /*see nexthop resource of host route, shared with ipv6*/
    p_spec_info->indirect_routes = 4096; /*see key resource of lpm, shared with ipv6*/
    p_spec_info->static_routes = 4096; /*see key resource of lpm, shared with ipv6*/
#if defined(GOLDENGATE) || defined(USW)
    p_spec_info->ecmp_routes = 239;
    p_spec_info->ecmp_member_maximum = IP_ECMP_MAX_PATH;
#else
    p_spec_info->ecmp_routes = 239;
    p_spec_info->ecmp_member_maximum = IP_ECMP_MAX_PATH_GB;
#endif
    p_spec_info->ospf_area_range_num = GLB_OSPF_AREA_RANGE_MAX;

#if defined(GOLDENGATE) || defined(USW)
#ifdef OFPRODUCT
    p_spec_info->flow_entries_ingress = 100;
#else
    p_spec_info->flow_entries_ingress = 4096 - 1; /*ipv4 acl tcam, tap acl need use stats,stats support 4095*/
#endif
   /*test by wangl for hybrid*/
    p_spec_info->openflow_flow_entries = 4000; /*openflow ipv4 acl tcam*/
#else
    #ifdef OFPRODUCT
    p_spec_info->flow_entries_ingress = 100;
    #else
    /* modified by liwh for bug 48514, 2018-08-31 
         for support nd copp, SYS_FTM_ADD_KEY(key, 4, 1536); 1536 => 384, 
         change 960 => 240 */
    //p_spec_info->flow_entries_ingress = 191;
    p_spec_info->flow_entries_ingress = 191; /*modified by yejl for bug 49927, 2018-11-15*/
    /* liwh end */
    #endif
#endif
    /* modified by liwh for bug 48514, 2018-08-31 
         for support nd copp, SYS_FTM_ADD_KEY(key, 4, 1536); 1536 => 384, 
         change 256 => 64 */
    //p_spec_info->flow_entries_egress = 64;
    p_spec_info->flow_entries_egress = 64;
    /* liwh end */
#ifdef OFPRODUCT
    p_spec_info->flow_entries_egress = 0;
#endif
    p_spec_info->pbr_entries = 32;
    p_spec_info->voice_vlan = 16;

    /*added by yejl for ipv6 acl*/
    #ifdef GREATBELT
    p_spec_info->flow6_entries_ingress  = 95;			/* 95 */ /*modified by yejl for bug 49927, 2018-11-15*/
	p_spec_info->flow6_entries_egress   = 32;			/* 32 */
    #endif
    /*ended by yejl*/

    /*IPv6 features*/
    p_spec_info->ipv6_based_vlan_class = 0;/*160bit keysize, use hash usrid, default entry stored in tcam*/
    p_spec_info->ipv6_source_guard = 256; /*160bit keysize, use hash usrid, default entry stored in tcam*/
    p_spec_info->l2mc_v6_entries = 1024; /*see nexthop resource of l2mc, shared with ipv4*/
    p_spec_info->l2mc_v6_member_ports = 16;/*now all 52 ports can be added, use bitmap, fixed*/
    p_spec_info->l3mc_v6_entries = 128; /*see key resource of ipv6mc tcam*/
    p_spec_info->l3mc_v6_member_ports = 4;  /*average 2 vlanif and 2 routed port*/
    p_spec_info->flow_v6_entries = 128;
    p_spec_info->host_v6_routes = 1024; /*see nexthop resource of host route, shared with ipv4*/
    p_spec_info->indirect_v6_routes = 1024;/*see key resource of lpm, shared with ipv4*/
    p_spec_info->static_v6_routes = 1024;/*see key resource of lpm, shared with ipv4*/
    p_spec_info->ecmp_v6_routes = 14;
    p_spec_info->ecmp_v6_member_maximum = IP_ECMP_MAX_PATH_GB;
    p_spec_info->ip_tunnel = 8; /*auto tunnel use tcam, static tunnel use usrid, ipsa only have 3 bits*/
    p_spec_info->ivi_peers = 32; /*always fixed*/

    /*cfm and g8031, g8032*/
    p_spec_info->cfm_lck = 48;/*48*4 acl entries*/
    p_spec_info->cfm_local_and_remote_meps = 2048;
    p_spec_info->g8031_groups = 64;
    p_spec_info->g8032_rings = 32;
    p_spec_info->g8032_member_ports_per_ring = 8;

    p_spec_info->bfd_sessions = 0;


    /*mpls*/
    p_spec_info->ftn_entries = 0;
    p_spec_info->ilm_entries = 0;
    p_spec_info->mpls_lables = 0;

    /*l2vpn*/
    p_spec_info->vpws = 0;
    p_spec_info->vpls_ac = 0;
    p_spec_info->vpls_peer = 0;
    p_spec_info->lsp_oam = 0;
    p_spec_info->lsp_p = 0;
    p_spec_info->lsp_pe = 0;
    p_spec_info->pw_oam = 0;
    p_spec_info->mpls_aps_tunnel = 0;
    p_spec_info->vsi = 0;

    /*nvgre*/
    p_spec_info->nvgre_tunnel_id_mappings = 0;
    p_spec_info->nvgre_peers = 0;
    p_spec_info->nvgre_tunnels = 0;

    /*qos*/
    p_spec_info->policer_profile_num = GLB_QOS_POLICER_PROFILE_NUM;
    p_spec_info->drop_profile_num = GLB_QOS_DROP_PROFILE_NUM;
    p_spec_info->queue_shape_profile_num = GLB_QOS_SHAPE_PROFILE_NUM; /*for queue shape profile, port profile is separate in GG*/
    p_spec_info->port_shape_profile_num = GLB_QOS_PORT_SHAPE_PROFILE_NUM;

    p_spec_info->l2protocol_num = 8;

    #if defined(DUET2)
        //p_spec_info->ucast_fdb = 18431;/*modyfied by yangl for bug 50224 2018-12-07*/
        p_spec_info->ucast_fdb = 32767;
        p_spec_info->host_routes = 4096; /*see nexthop resource of host route, shared with ipv6*/
        p_spec_info->indirect_routes = 2048; /*see key resource of lpm, shared with ipv6*/
        p_spec_info->static_routes = 2048; /*see key resource of lpm, shared with ipv6*/
        p_spec_info->host_v6_routes = 4096; /*see nexthop resource of host route, shared with ipv4*/
        p_spec_info->static_v6_routes = 1536;/*see key resource of lpm, shared with ipv4*/
        p_spec_info->indirect_v6_routes = 1536;/*see key resource of lpm, shared with ipv4*/
        //p_spec_info->flow_entries_ingress = 2416; /*ipv4 acl tcam, tap acl need use stats,stats support 4095*/
        p_spec_info->flow_entries_ingress = 1208;
        //p_spec_info->flow_entries_egress = 880;
        p_spec_info->flow_entries_egress = 440;
        p_spec_info->lag_group_num = GLB_LAG_GROUP_NUM;

        p_spec_info->ecmp_v6_member_maximum = IP_ECMP_MAX_PATH;
    #endif
    #if defined(TSINGMA)
        //p_spec_info->ucast_fdb = 18431;/*modyfied by yangl for bug 50224 2018-12-07*/
        p_spec_info->ucast_fdb = 32767;
        p_spec_info->host_routes = 4096; /*see nexthop resource of host route, shared with ipv6*/
        p_spec_info->indirect_routes = 8192; /*see key resource of lpm, shared with ipv6*/
        p_spec_info->static_routes = 2048; /*see key resource of lpm, shared with ipv6*/
        p_spec_info->host_v6_routes = 4096; /*see nexthop resource of host route, shared with ipv4*/
        p_spec_info->static_v6_routes = 2048;/*see key resource of lpm, shared with ipv4*/
        p_spec_info->indirect_v6_routes = 2048;/*see key resource of lpm, shared with ipv4*/
        //p_spec_info->flow_entries_ingress = 2416; /*ipv4 acl tcam, tap acl need use stats,stats support 4095*/
        p_spec_info->flow_entries_ingress = 2048; /*modified by yejl for bug 54220, 2019-11-09*/
        //p_spec_info->flow_entries_egress = 880;
        p_spec_info->flow_entries_egress = 440;
        p_spec_info->lag_group_num = GLB_LAG_GROUP_NUM;

        p_spec_info->ecmp_v6_member_maximum = IP_ECMP_MAX_PATH;
    #endif
    return PM_E_NONE;
}

/* NVGRE_DEV. */
static int32
_chsm_stm_mgr_init_nvgre_spec_info(uint8 type, tbl_sys_spec_t* p_spec_info)
{
    CHSM_CTC_CHK_PTR(p_spec_info);

    p_spec_info->key.type = type;

    /*2. feature specification information*/
    p_spec_info->ucast_fdb = 20*1024;
    p_spec_info->mac_filter = 64;
    p_spec_info->vlan_instance = 2*1024 - 2; /* Exclude the resolved 2 mcast group used by SDK,
                                                         ref to GLB_MCAST_GROUPID_START. */
    p_spec_info->lag_group_num = GLB_LAG_GROUP_NUM;
    p_spec_info->vlan_member_ports = 52;/*now all 52 ports can be added, use bitmap, fixed*/

    /*vlan class, total usrid key 4K, usrid mac keysize 160bit, */
    p_spec_info->mac_based_vlan_class = 0;/*80bit userid mac hash*/
    p_spec_info->dot1x_mac = 0;/*80bit userid mac hash*/
    p_spec_info->ipv4_based_vlan_class = 0; /*80bit userid ip hash, with default entry in tcam*/
    p_spec_info->ipv4_source_guard = 0;/*80bit userid ip hash, with default entry in tcam*/
    p_spec_info->vlan_mapping_entry_applied_port = 0; /*80bit userid vlan hash, use tcam to reslove conflict*/

    /*multicast and unicast*/
    p_spec_info->l2mc_entries = 0; /*see nexthop resource of l2mc, shared with ipv6*/
    p_spec_info->l2mc_member_ports = 0; /*now all 52 ports can be added, use bitmap, fixed*/
    p_spec_info->l3mc_entries = 0; /*see key resource of ipv4mc tcam*/
    p_spec_info->l3mc_member_ports = 0; /*average 2 vlanif and 2 routed port, fixed*/
    /* Disable lpm allocation on NVGRE profile, since there are much less
       routes support needed, all routes will be installed to TCAM. */
    p_spec_info->host_routes = 512; /*see nexthop resource of host route, shared with ipv6*/
    p_spec_info->static_routes = 256; /*see key resource of lpm, shared with ipv6*/
    p_spec_info->indirect_routes = 256; /*see key resource of lpm, shared with ipv6*/
    p_spec_info->ecmp_routes = 64; /* Better same as nvgre_peers. */

    p_spec_info->ospf_area_range_num = GLB_OSPF_AREA_RANGE_MAX;

    p_spec_info->flow_entries_ingress = 704; /*ipv4 acl tcam*/
    p_spec_info->pbr_entries = 0;
    p_spec_info->voice_vlan = 0;

    /*IPv6 features*/
    p_spec_info->ipv6_based_vlan_class = 0;/*160bit keysize, use hash usrid, default entry stored in tcam*/
    p_spec_info->ipv6_source_guard = 0; /*160bit keysize, use hash usrid, default entry stored in tcam*/           p_spec_info->l2mc_v6_entries = 0; /*see nexthop resource of l2mc, shared with ipv4*/
    p_spec_info->l2mc_v6_member_ports = 0;/*now all 52 ports can be added, use bitmap, fixed*/
    p_spec_info->l3mc_v6_entries = 0; /*see key resource of ipv6mc tcam*/
    p_spec_info->l3mc_v6_member_ports = 0;  /*average 2 vlanif and 2 routed port*/
    /* XXX: in access_set_ace_max_num_in_acl, if we does not allocate v6 acl entries,
       */
    p_spec_info->flow_v6_entries = 0;
    p_spec_info->host_v6_routes = 0; /*see nexthop resource of host route, shared with ipv4*/
    p_spec_info->indirect_v6_routes = 0;/*see key resource of lpm, shared with ipv4*/
    p_spec_info->static_v6_routes = 0;/*see key resource of lpm, shared with ipv4*/
    /*if RPF is supported, only 31 rpf profile can be used for ipuc,
    and 1 is reserved for ipuc route update*/
    p_spec_info->ecmp_v6_routes = 0;
    p_spec_info->ip_tunnel = 0; /*auto tunnel use tcam, static tunnel use usrid, ipsa only have 3 bits*/
    p_spec_info->ivi_peers = 0; /*always fixed*/

    /*cfm and g8031, g8032*/
    p_spec_info->cfm_lck = 0;
    p_spec_info->cfm_local_and_remote_meps = 0;
    p_spec_info->g8031_groups = 0;
    p_spec_info->g8032_rings = 0;
    p_spec_info->g8032_member_ports_per_ring = 0;


    /*mpls*/
    p_spec_info->ftn_entries = 0;
    p_spec_info->ilm_entries = 0;
    p_spec_info->mpls_lables = 0;

    /*l2vpn*/
    p_spec_info->vpws = 0;
    p_spec_info->vpls_ac = 0;
    p_spec_info->vpls_peer = 0;
    p_spec_info->lsp_oam = 0;
    p_spec_info->lsp_p = 0;
    p_spec_info->lsp_pe = 0;
    p_spec_info->pw_oam = 0;
    p_spec_info->mpls_aps_tunnel = 0;
    p_spec_info->vsi = 0;

    /*nvgre*/
    p_spec_info->nvgre_tunnel_id_mappings = 2000;
    p_spec_info->nvgre_peers = 64;
    p_spec_info->nvgre_tunnels = 20000;

    return PM_E_NONE;
}

static int32
_chsm_stm_mgr_init_openflow_default_spec_info(uint8 type, tbl_sys_spec_t* p_spec_info)
{
    CHSM_CTC_CHK_PTR(p_spec_info);

    p_spec_info->key.type = type;

    /*2. feature specification information*/
    p_spec_info->ucast_fdb = 16384;
    p_spec_info->mac_filter = 64;
    p_spec_info->vlan_instance = 4094;
    p_spec_info->vlan_member_ports = 16;/*now all 52 ports can be added, use bitmap, fixed*/
    p_spec_info->lag_group_num = GLB_LAG_GROUP_NUM;
    
    /*vlan class, total usrid key 4K, usrid mac keysize 160bit, */
    p_spec_info->mac_based_vlan_class = 256;/*80bit userid mac hash*/
    p_spec_info->dot1x_mac = 256;/*80bit userid mac hash*/
    p_spec_info->ipv4_based_vlan_class = 512; /*80bit userid ip hash, with default entry in tcam*/
    p_spec_info->ipv4_source_guard = 1024;/*80bit userid ip hash, with default entry in tcam*/
    p_spec_info->vlan_mapping_entry_applied_port = 1024; /*80bit userid vlan hash, use tcam to reslove conflict*/

    /*multicast and unicast*/
    p_spec_info->l2mc_entries = 1024; /*see nexthop resource of l2mc, shared with ipv6*/
    p_spec_info->l2mc_member_ports = 16; /*now all 52 ports can be added, use bitmap, fixed*/
    p_spec_info->l3mc_entries = 512; /*see key resource of ipv4mc tcam*/
    p_spec_info->l3mc_member_ports = 4; /*average 2 vlanif and 2 routed port, fixed*/
    p_spec_info->host_routes = 3072; /*see nexthop resource of host route, shared with ipv6*/
    p_spec_info->indirect_routes = 6144; /*see key resource of lpm, shared with ipv6*/
    p_spec_info->static_routes = 6144; /*see key resource of lpm, shared with ipv6*/
    /*if RPF is supported, only 31 rpf profile can be used for ipuc,
    and 1 is reserved for ipuc route update*/
    p_spec_info->ecmp_routes = 16;

    p_spec_info->ospf_area_range_num = GLB_OSPF_AREA_RANGE_MAX;

    p_spec_info->flow_entries_ingress = 32; /*ipv4 acl tcam*/

    p_spec_info->openflow_flow_entries = 4000; /*openflow ipv4 acl tcam*/
    p_spec_info->pbr_entries = 16;
    p_spec_info->voice_vlan = 16;

    /*IPv6 features*/
    p_spec_info->ipv6_based_vlan_class = 256;/*160bit keysize, use hash usrid, default entry stored in tcam*/
    p_spec_info->ipv6_source_guard = 256; /*160bit keysize, use hash usrid, default entry stored in tcam*/
    p_spec_info->l2mc_v6_entries = 1024; /*see nexthop resource of l2mc, shared with ipv4*/
    p_spec_info->l2mc_v6_member_ports = 16;/*now all 52 ports can be added, use bitmap, fixed*/
    p_spec_info->l3mc_v6_entries = 128; /*see key resource of ipv6mc tcam*/
    p_spec_info->l3mc_v6_member_ports = 4;  /*average 2 vlanif and 2 routed port*/
    p_spec_info->flow_v6_entries = 128;
    p_spec_info->host_v6_routes = 1024; /*see nexthop resource of host route, shared with ipv4*/
    p_spec_info->static_v6_routes = 2048;/*see key resource of lpm, shared with ipv4*/
    p_spec_info->indirect_v6_routes = 2048;/*see key resource of lpm, shared with ipv4*/
    /*if RPF is supported, only 31 rpf profile can be used for ipuc,
    and 1 is reserved for ipuc route update*/
    p_spec_info->ecmp_v6_routes = 14;
    p_spec_info->ip_tunnel = 8; /*auto tunnel use tcam, static tunnel use usrid, ipsa only have 3 bits*/
    p_spec_info->ivi_peers = 32; /*always fixed*/

    /*cfm and g8031, g8032*/
    p_spec_info->cfm_lck = 48;/*48*4 acl entries*/
    p_spec_info->cfm_local_and_remote_meps = 2048;
    p_spec_info->g8031_groups = 64;
    p_spec_info->g8032_rings = 32;
    p_spec_info->g8032_member_ports_per_ring = 8;

    /*qos*/
    p_spec_info->policer_profile_num = GLB_QOS_POLICER_PROFILE_NUM;/*added by yejl for bug 52691, 2019-06-25*/
    p_spec_info->drop_profile_num = 4;
    p_spec_info->queue_shape_profile_num = 60; /*for queue shape profile, port profile is separate in GG*/
    p_spec_info->port_shape_profile_num = 128;

    return PM_E_NONE;
}

static int32
_chsm_stm_mgr_init_openflow_ipv6_spec_info(uint8 type, tbl_sys_spec_t* p_spec_info)
{
    CHSM_CTC_CHK_PTR(p_spec_info);

    p_spec_info->key.type = type;

    /*2. feature specification information*/
/*addd by wangl for hybrid profile*/
#if defined(GOLDENGATE)
#ifdef OFPRODUCT
    p_spec_info->ucast_fdb = 32768;
#else
    p_spec_info->ucast_fdb = 65536;
#endif
#else
#ifdef OFPRODUCT
    /* modified for hybrid 10k fdb_based flow, GB 16k-10000 */
    p_spec_info->ucast_fdb = 16384;
#else
    p_spec_info->ucast_fdb = 32768;
#endif
#endif

#if defined(USW)
    p_spec_info->ucast_fdb = 32768;
#endif

    p_spec_info->mac_filter = 128;
    p_spec_info->mstp_instance = GLB_STP_INSTANCE_MAX;
    p_spec_info->vlan_instance = 4094;
    p_spec_info->vlan_member_ports = 16;/*now all 52 ports can be added, use bitmap, fixed*/
    p_spec_info->lag_group_num = GLB_LAG_GROUP_NUM;
        
    /*vlan class, total usrid key 4K, usrid mac keysize 160bit, */
    p_spec_info->mac_based_vlan_class = 512;/*80bit userid mac hash*/
    p_spec_info->dot1x_mac = 256;/*80bit userid mac hash*/
    p_spec_info->ipv4_based_vlan_class = 512; /*80bit userid ip hash, with default entry in tcam*/
    p_spec_info->ipv4_source_guard = 1024;/*80bit userid ip hash, with default entry in tcam*/
    p_spec_info->vlan_mapping_entry_applied_port = 1024; /*80bit userid vlan hash, use tcam to reslove conflict*/

    /*multicast and unicast*/
    p_spec_info->l2mc_entries = 1024; /*see nexthop resource of l2mc, shared with ipv6*/
    p_spec_info->l2mc_member_ports = 16; /*now all 52 ports can be added, use bitmap, fixed*/
    p_spec_info->l3mc_entries = 512; /*see key resource of ipv4mc tcam*/
    p_spec_info->l3mc_member_ports = 4; /*average 2 vlanif and 2 routed port, fixed*/
    p_spec_info->host_routes = 4096; /*see nexthop resource of host route, shared with ipv6*/
    p_spec_info->static_routes = 8192; /*see key resource of lpm, shared with ipv6*/
    #if defined(TSINGMA)
    #ifdef OFPRODUCT
        p_spec_info->static_routes = 2048; /*see key resource of lpm, shared with ipv6*/
    #endif
    #endif
    p_spec_info->indirect_routes = 8192; /*see key resource of lpm, shared with ipv6*/
    p_spec_info->static_v6_routes = 8192; /*see key resource of lpm, shared with ipv6*/
    p_spec_info->indirect_v6_routes = 8192; /*see key resource of lpm, shared with ipv6*/
#if defined(GOLDENGATE) || defined(USW)
    p_spec_info->ecmp_routes = 239;
    p_spec_info->ecmp_member_maximum = IP_ECMP_MAX_PATH;
#else
    p_spec_info->ecmp_routes = 239;
    p_spec_info->ecmp_member_maximum = IP_ECMP_MAX_PATH_GB;
#endif
    p_spec_info->ospf_area_range_num = GLB_OSPF_AREA_RANGE_MAX;

#if defined(GOLDENGATE) || defined(USW)
#ifdef OFPRODUCT
    p_spec_info->flow_entries_ingress = 100;
#else
    p_spec_info->flow_entries_ingress = 4096 - 1; /*ipv4 acl tcam, tap acl need use stats,stats support 4095*/
#endif
   /*test by wangl for hybrid*/
    p_spec_info->openflow_flow_entries = 1300; /*openflow ipv4 acl tcam*/
#else
    #ifdef OFPRODUCT
    p_spec_info->flow_entries_ingress = 100;
    #else
    p_spec_info->flow_entries_ingress = 960;
    #endif
#endif
    p_spec_info->flow_entries_egress = 256;
#ifdef OFPRODUCT
    p_spec_info->flow_entries_egress = 0;
#endif
    p_spec_info->pbr_entries = 32;
    p_spec_info->voice_vlan = 16;

    /*IPv6 features*/
    p_spec_info->ipv6_based_vlan_class = 0;/*160bit keysize, use hash usrid, default entry stored in tcam*/
    p_spec_info->ipv6_source_guard = 256; /*160bit keysize, use hash usrid, default entry stored in tcam*/
    p_spec_info->l2mc_v6_entries = 1024; /*see nexthop resource of l2mc, shared with ipv4*/
    p_spec_info->l2mc_v6_member_ports = 16;/*now all 52 ports can be added, use bitmap, fixed*/
    p_spec_info->l3mc_v6_entries = 128; /*see key resource of ipv6mc tcam*/
    p_spec_info->l3mc_v6_member_ports = 4;  /*average 2 vlanif and 2 routed port*/
    p_spec_info->flow_v6_entries = 128;
    p_spec_info->host_v6_routes = 1024; /*see nexthop resource of host route, shared with ipv4*/
    p_spec_info->static_v6_routes = 2048;/*see key resource of lpm, shared with ipv4*/
    p_spec_info->indirect_v6_routes = 2048;/*see key resource of lpm, shared with ipv4*/
    p_spec_info->ecmp_v6_routes = 14;
    p_spec_info->ip_tunnel = 8; /*auto tunnel use tcam, static tunnel use usrid, ipsa only have 3 bits*/
    p_spec_info->ivi_peers = 32; /*always fixed*/

    /*cfm and g8031, g8032*/
    p_spec_info->cfm_lck = 48;/*48*4 acl entries*/
    p_spec_info->cfm_local_and_remote_meps = 2048;
    p_spec_info->g8031_groups = 64;
    p_spec_info->g8032_rings = 32;
    p_spec_info->g8032_member_ports_per_ring = 8;

    p_spec_info->bfd_sessions = 0;


    /*mpls*/
    p_spec_info->ftn_entries = 0;
    p_spec_info->ilm_entries = 0;
    p_spec_info->mpls_lables = 0;

    /*l2vpn*/
    p_spec_info->vpws = 0;
    p_spec_info->vpls_ac = 0;
    p_spec_info->vpls_peer = 0;
    p_spec_info->lsp_oam = 0;
    p_spec_info->lsp_p = 0;
    p_spec_info->lsp_pe = 0;
    p_spec_info->pw_oam = 0;
    p_spec_info->mpls_aps_tunnel = 0;
    p_spec_info->vsi = 0;

    /*nvgre*/
    p_spec_info->nvgre_tunnel_id_mappings = 0;
    p_spec_info->nvgre_peers = 0;
    p_spec_info->nvgre_tunnels = 0;

    /*qos*/
    p_spec_info->policer_profile_num = GLB_QOS_POLICER_PROFILE_NUM;
    p_spec_info->drop_profile_num = GLB_QOS_DROP_PROFILE_NUM;
    p_spec_info->queue_shape_profile_num = GLB_QOS_SHAPE_PROFILE_NUM; /*for queue shape profile, port profile is separate in GG*/
    p_spec_info->port_shape_profile_num = GLB_QOS_PORT_SHAPE_PROFILE_NUM;

    p_spec_info->l2protocol_num = 8;
    return PM_E_NONE;
}

#ifdef TAPPRODUCT
static int32
_chsm_stm_mgr_init_tap_default_spec_info(uint8 type, tbl_sys_spec_t* p_spec_info)
{
    CHSM_CTC_CHK_PTR(p_spec_info);

    p_spec_info->key.type = type;

    /*1. linkagg specification information*/
    p_spec_info->lag_group_num = GLB_LAG_GROUP_NUM;
    
    /*2. tag group specification information*/
    p_spec_info->tap_group_profile_num = GLB_TAP_GROUP_MAX;

    /*3. tag flow specification information*/
    p_spec_info->tap_flow_num = GLB_ACL_CONFIG_MAX;
    p_spec_info->tap_flow_entry_num = GLB_ACE_CONFIG_MAX;


#ifdef GOLDENGATE
    p_spec_info->flow_entries_ingress = 2048;
#endif
#if defined(GREATBELT) || defined(DUET2)
    p_spec_info->flow_entries_ingress = 1024; 
#endif
#ifdef TSINGMA
    p_spec_info->flow_entries_ingress = 1800; /*modified by yejl for bug 54289, 2019-11-07*/
#endif
    p_spec_info->flow_entries_egress = 256 - 1 - 2;     /* egress default entry need use 1(ipv4) 2(ipv6) */
    return PM_E_NONE;
}
#endif

static int32
_chsm_stm_mgr_init_tap_ipv4_ipv6_spec_info(uint8 type, tbl_sys_spec_t* p_spec_info)
{
    CHSM_CTC_CHK_PTR(p_spec_info);

    p_spec_info->key.type = type;

    /*1. linkagg specification information*/
    p_spec_info->lag_group_num = GLB_LAG_GROUP_NUM;
    
    /*2. tag group specification information*/
    p_spec_info->tap_group_profile_num = GLB_TAP_GROUP_MAX;

    /*3. tag flow specification information*/
    p_spec_info->tap_flow_num = GLB_ACL_CONFIG_MAX;
    p_spec_info->tap_flow_entry_num = GLB_ACE_CONFIG_MAX;

    p_spec_info->flow_entries_ingress   = 800;		    /* 800 */
	p_spec_info->flow_entries_egress    = 200;     	/* 200 */
	p_spec_info->flow6_entries_ingress  = 400;			/* 400 */
	p_spec_info->flow6_entries_egress   = 100;			/* 100 */
	
    return PM_E_NONE;
}


static int32
_chsm_stm_mgr_init_sys_spec(uint8 type, tbl_sys_spec_t* p_spec_info)
{
    switch (type)
    {
        case GLB_STM_MODE_DEFAULT:
#ifdef TAPPRODUCT
            _chsm_stm_mgr_init_tap_default_spec_info(type, p_spec_info);
#else
            _chsm_stm_mgr_init_default_spec_info(type, p_spec_info);
#endif
            break;

        case GLB_STM_MODE_IPUCV4:
            _chsm_stm_mgr_init_ipv4_spec_info(type, p_spec_info);
            break;

        case GLB_STM_MODE_VLAN:
            _chsm_stm_mgr_init_vlan_spec_info(type, p_spec_info);
            break;

        case GLB_STM_MODE_L2VPN:
            _chsm_stm_mgr_init_l2vpn_spec_info(type, p_spec_info);
            break;

        case GLB_STM_MODE_IPUCV6:
            _chsm_stm_mgr_init_ipv6_spec_info(type, p_spec_info);
            break;

        case GLB_STM_MODE_NVGRE:
            _chsm_stm_mgr_init_nvgre_spec_info(type, p_spec_info);
            break;

        case GLB_STM_MODE_OF_DEFAULT:
            _chsm_stm_mgr_init_openflow_default_spec_info(type, p_spec_info);
            break;

        case GLB_STM_MODE_OF_HYBRID:
            _chsm_stm_mgr_init_openflow_default_spec_info(type, p_spec_info);
            break;

        case GLB_STM_MODE_OF_IPV6:
            _chsm_stm_mgr_init_openflow_ipv6_spec_info(type, p_spec_info);
            break;
			
	  case GLB_STM_MODE_TAP_IPV4_IPV6:
            _chsm_stm_mgr_init_tap_ipv4_ipv6_spec_info(type, p_spec_info);
            break;
    }

    return PM_E_NONE;
}


int32
chsm_stm_mgr_init(void)
{
    uint32 i = 0;
    tbl_sys_spec_t sys_spec;

    for (i = 0; i < GLB_STM_MODE_MAX; i++)
    {
        sal_memset(&sys_spec, 0, sizeof(sys_spec));
        _chsm_stm_mgr_init_sys_spec(i, &sys_spec);
        tbl_sys_spec_add_sys_spec(&sys_spec);
    }

    cdb_register_cfg_tbl_cb(TBL_SYS_SPEC, chsm_stm_mgr_cmd_process_sys_spec);

    return PM_E_NONE;
}
