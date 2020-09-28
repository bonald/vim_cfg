
#include "proto.h"
#include "lib_tblinc.h"
#include "glb_if_define.h"
#include "gen/tbl_vlan_define.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_brg_global_define.h"
#include "gen/tbl_brg_global.h"
#include "gen/tbl_fdb_define.h"
#include "gen/tbl_mcfdb_define.h"
#include "gen/tbl_macfilter_define.h"
#include "gen/tbl_l2_action_define.h"
#include "gen/tbl_l2_action.h"
#include "gen/tbl_psfdb_define.h"
#include "gen/tbl_psfdb.h"
#include "switch.h"
#include "if_mgr.h"
#include "if_agg.h"
#include "brg_if.h"
#include "brg_fdb.h"
#include "switch_api.h"
#include "gen/tbl_kernel_if_define.h"
#include "gen/tbl_kernel_if.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "ipsour/ip_source.h"
#include "glb_openflow_define.h"
#include "fei_openflow_port.h"
#include "fei_openflow_tunnel.h"
#include "gen/tbl_vlan_mapping_table_define.h"
#include "gen/tbl_vlan_mapping_table.h"
#include "gen/tbl_vlan_mapping_entry_define.h"
#include "gen/tbl_vlan_mapping_entry.h"
#include "gen/tbl_egress_mapping_entry_define.h"
#include "gen/tbl_egress_mapping_entry.h"
#include "gen/tbl_vlan_group_define.h"
#include "gen/tbl_vlan_group.h"
#include "gen/tbl_evc_entry_define.h"
#include "gen/tbl_evc_entry.h"
#include "vlan_mapping.h"
#include "hash_field_profile.h"
#include "hash_value_profile_and_global.h"

extern int32 if_agg_aggregator_bw_update(tbl_interface_t *p_db_if_agg);

static AGG_CB_NOTIFY_FUNC g_agg_cb_notify_array[IF_AGG_CB_NOTIFY_OP_MAX][IF_AGG_CB_MODULE_MAX];

extern int32
policy_map_detach_interface(char *policy_map_name, char *interface_name, bool direct_in);

extern int32 
acl_detach_interface(char *acl_name, char *interface_name, uint32 type, bool direct_in);

int32
_if_agg_create_agg_sync_prop(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if)
{
    int32 i = 0;
    
    for (i = 0; i < IF_AGG_CB_MODULE_MAX; i++)
    {
        if (g_agg_cb_notify_array[IF_AGG_CB_NOTIFY_CREATE_AGGREGATOR][i])
        {
            g_agg_cb_notify_array[IF_AGG_CB_NOTIFY_CREATE_AGGREGATOR][i](p_db_agg_if, p_db_mem_if, NULL);
        }
    }
    
    return PM_E_NONE;
}

int32
_if_agg_destory_agg_sync_prop(tbl_interface_t *p_db_agg_if)
{
    int32 i = 0;
    
    for (i = 0; i < IF_AGG_CB_MODULE_MAX; i++)
    {
        if (g_agg_cb_notify_array[IF_AGG_CB_NOTIFY_DESTROY_AGGREGATOR][i])
        {
            g_agg_cb_notify_array[IF_AGG_CB_NOTIFY_DESTROY_AGGREGATOR][i](p_db_agg_if, NULL, NULL);
        }
    }
    
    return PM_E_NONE;
}

int32
_if_agg_add_member_sync_prop(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if)
{
    int32 i = 0;

    for (i = 0; i < IF_AGG_CB_MODULE_MAX; i++)
    {
        if (g_agg_cb_notify_array[IF_AGG_CB_NOTIFY_ADD_MEMBER_SYNC][i])
        {
            g_agg_cb_notify_array[IF_AGG_CB_NOTIFY_ADD_MEMBER_SYNC][i](p_db_agg_if, p_db_mem_if, NULL);
        }
    }

    return PM_E_NONE;
}

int32
_if_agg_del_member_sync_prop(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if)
{
    int32 i = 0;

    for (i = 0; i < IF_AGG_CB_MODULE_MAX; i++)
    {
        if (g_agg_cb_notify_array[IF_AGG_CB_NOTIFY_DEL_MEMBER_SYNC][i])
        {
            g_agg_cb_notify_array[IF_AGG_CB_NOTIFY_DEL_MEMBER_SYNC][i](p_db_agg_if, p_db_mem_if, NULL);
        }
    }

    return PM_E_NONE;
}

int32
_if_agg_attach_member_sync_prop(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if)
{
    int32 i = 0;

    for (i = 0; i < IF_AGG_CB_MODULE_MAX; i++)
    {
        if (g_agg_cb_notify_array[IF_AGG_CB_NOTIFY_ATTACH_MEMBER_SYNC][i])
        {
            g_agg_cb_notify_array[IF_AGG_CB_NOTIFY_ATTACH_MEMBER_SYNC][i](p_db_agg_if, p_db_mem_if, NULL);
        }
    }

    return PM_E_NONE;
}

int32
_if_agg_detach_member_sync_prop(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if)
{
    int32 i = 0;

    for (i = 0; i < IF_AGG_CB_MODULE_MAX; i++)
    {
        if (g_agg_cb_notify_array[IF_AGG_CB_NOTIFY_DETACH_MEMBER_SYNC][i])
        {
            g_agg_cb_notify_array[IF_AGG_CB_NOTIFY_DETACH_MEMBER_SYNC][i](p_db_agg_if, p_db_mem_if, NULL);
        }
    }

    return PM_E_NONE;
}

int32
_if_agg_add_member_check(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if, cfg_cmd_para_t *para)
{
    int32 i = 0;
    int32 rc = PM_E_NONE;
    
    for (i = 0; i < IF_AGG_CB_MODULE_MAX; i++)
    {
        if (g_agg_cb_notify_array[IF_AGG_CB_NOTIFY_ADD_MEMBER_CHECK][i])
        {
            rc = g_agg_cb_notify_array[IF_AGG_CB_NOTIFY_ADD_MEMBER_CHECK][i](p_db_agg_if, p_db_mem_if, 
                                                                             (void *)para);
            if (rc < 0)
            {
                return rc;
            }
        }
    }

    return rc;
}

int32
_if_agg_sync_agg_prop(tbl_interface_t *p_db_if_lag, tbl_interface_t *p_db_if)
{
    return PM_E_NONE;
}

/*added by yejl for bug 54170, 2019-10-30*/
int32
_if_agg_sync_when_detach_acl_and_policy_map(tbl_interface_t *p_db_if_lag)
{
    int32 rc = PM_E_NONE;
    
    if (NULL == p_db_if_lag)
    {
        return PM_E_NOT_EXIST;    
    }
    if (sal_strlen(p_db_if_lag->igs_acl))
    {
        PM_E_RETURN(acl_detach_interface(p_db_if_lag->igs_acl, p_db_if_lag->key.name, GLB_ACL_CONFIG_TYPE_NORMAL, TRUE));
    }
    if (sal_strlen(p_db_if_lag->egs_acl))
    {
        PM_E_RETURN(acl_detach_interface(p_db_if_lag->egs_acl, p_db_if_lag->key.name, GLB_ACL_CONFIG_TYPE_NORMAL, FALSE));
    }
    if (sal_strlen(p_db_if_lag->igs_mac_acl))
    {
        PM_E_RETURN(acl_detach_interface(p_db_if_lag->igs_mac_acl, p_db_if_lag->key.name, GLB_ACL_CONFIG_TYPE_MAC, TRUE));
    }
    if (sal_strlen(p_db_if_lag->egs_mac_acl))
    {
        PM_E_RETURN(acl_detach_interface(p_db_if_lag->egs_mac_acl, p_db_if_lag->key.name, GLB_ACL_CONFIG_TYPE_MAC, FALSE));
    }
    if (sal_strlen(p_db_if_lag->igs_ipv6_acl))
    {
        PM_E_RETURN(acl_detach_interface(p_db_if_lag->igs_ipv6_acl, p_db_if_lag->key.name, GLB_ACL_CONFIG_TYPE_IPV6, TRUE));
    }
    if (sal_strlen(p_db_if_lag->egs_ipv6_acl))
    {
        PM_E_RETURN(acl_detach_interface(p_db_if_lag->egs_ipv6_acl, p_db_if_lag->key.name, GLB_ACL_CONFIG_TYPE_IPV6, FALSE));
    }
    if (sal_strlen(p_db_if_lag->igs_policy_map))
    {
        PM_E_RETURN(policy_map_detach_interface(p_db_if_lag->igs_policy_map, p_db_if_lag->key.name, TRUE));
    }
    if (sal_strlen(p_db_if_lag->egs_policy_map))
    {
        PM_E_RETURN(policy_map_detach_interface(p_db_if_lag->egs_policy_map, p_db_if_lag->key.name, FALSE));
    }
    return PM_E_NONE;
}
/*ended by yejl*/

static int32
_if_agg_sync_vlan_when_add(tbl_interface_t *p_db_if_lag, tbl_interface_t *p_db_if)
{
#ifdef CONFIG_L2
    vid_t vid = 0;

    /*update vlan port bmp */
    if (GLB_IF_MODE_L2 == p_db_if->mode)
    {
        GLB_BMP_ITER_BEGIN(p_db_if_lag->brgif->allowed_vlan.vlan_bmp, vid)
        {
            brg_if_set_vlan_port_bmp(p_db_if_lag->ifindex, vid, TRUE);
        }
        GLB_BMP_ITER_END(p_db_if_lag->brgif->allowed_vlan.vlan_bmp, vid);
    }

#endif /* CONFIG_L2 */


    return PM_E_NONE;
}

static int32
_if_agg_sync_l2db_when_add(tbl_interface_t *p_db_if_lag, tbl_interface_t *p_db_if)
{
#ifdef CONFIG_L2
    //vid_t vid = 0;
    int32 rc = PM_E_NONE;

    /* 1. update local CDB */    
    PM_E_RETURN(ds_brgif_add_brgif(p_db_if_lag, p_db_if->brgif));

    /* 2. set learning enable */
    PM_E_RETURN(brg_if_set_mac_learning_en(p_db_if_lag, p_db_if->brgif->mac_learning_en, TRUE));

    /*Added by yejl to fix bug40732, 2016-09-14*/
    /*sync port-secuirty to agg*/
    if (p_db_if->brgif->port_security_en)
    {
        PM_E_RETURN(brg_if_set_port_security_en(p_db_if_lag, p_db_if->brgif->port_security_en, FALSE));
    }
    PM_E_RETURN(brg_if_set_port_security_mac_maximum(p_db_if_lag, p_db_if->brgif->max_mac, FALSE));
    PM_E_RETURN(brg_if_set_port_security_voilation(p_db_if_lag, p_db_if->brgif->violate_mode, FALSE));
    /*sync arpinspection trust to agg*/
    PM_E_RETURN(if_mgr_set_arpinsp_trust_enable(p_db_if_lag, p_db_if->arpinsp_trust_enable, FALSE));

    /* sync qinq type and vlan translation enable to agg */
    if (GLB_VLAN_PORT_TYPE_QINQ == p_db_if->vlan_type)
    {
        PM_E_RETURN(brg_if_set_qinq_type(p_db_if_lag, p_db_if->brgif->qinq_type, FALSE));
        
        if (p_db_if->brgif->inner_native_vid)
        {
            brg_if_set_inner_native_vlan(p_db_if_lag, p_db_if->brgif->inner_native_vid, FALSE);
        }
    }

    if (GLB_VLAN_PORT_TYPE_TRUNK == p_db_if->vlan_type)
    {
        PM_E_RETURN(brg_if_set_vlan_translation_en(p_db_if_lag, p_db_if->brgif->vlan_translation_en, FALSE));
    }

    /*added by wangl for bug 49324*/
    if(DS_BRGIF_ALLOWED_VLAN_ACT_NONE != p_db_if->brgif->allowed_vlan.action)
    {
        if (p_db_if->brgif->pvid)
        {
            brg_if_set_pvid(p_db_if_lag, p_db_if->brgif->pvid, FALSE);
        }
    }

    /* LAG should not inherit its member port's VLAN mapping table,
       VLAN mapping table should be unapplied from port if user adds the port to LAG */
    if (sal_strlen(p_db_if->brgif->vlan_mapping_table))
    {
        sal_memset(p_db_if_lag->brgif->vlan_mapping_table, 0, GLB_VLAN_MAPPING_TAB_NAME_MAX);
        PM_E_RETURN(ds_brgif_set_brgif_field(p_db_if_lag, p_db_if_lag->brgif, DS_BRGIF_FLD_VLAN_MAPPING_TABLE));

        PM_E_RETURN(vlanmap_api_if_delete_vmt(p_db_if, NULL));
    }

    /* 4. update vlan port bmp */
    #if 0
    GLB_BMP_ITER_BEGIN(p_db_if_lag->brgif->allowed_vlan.vlan_bmp, vid)
    {
        brg_if_set_vlan_port_bmp(p_db_if_lag->ifindex, vid, TRUE);
    }
    GLB_BMP_ITER_END(p_db_if_lag->brgif->allowed_vlan.vlan_bmp, vid);
    #endif
#endif /* CONFIG_L2 */

    PM_E_RETURN(swth_mstp_api_add_port(p_db_if_lag));

    return PM_E_NONE;
}

static int32
_if_agg_sync_l2hal_when_add(tbl_interface_t *p_db_if_lag, tbl_interface_t *p_db_if)
{

    return PM_E_NONE;
}

static int32
_if_agg_sync_l3hal_when_add(tbl_interface_t *p_db_if_lag, tbl_interface_t *p_db_if)
{
    int32 rc = PM_E_NONE;
    
    if_mgr_set_mtu(p_db_if_lag, p_db_if->mtu, FALSE);

    return rc;
}


static int32
_if_agg_sync_hal_when_add(tbl_interface_t *p_db_if_lag, tbl_interface_t *p_db_if)
{
    int32 rc = PM_E_NONE;
    
   // PM_E_RETURN(fei_agg_sync_up_agg_prop(p_db_if_lag->ifindex, p_db_if->ifindex));
    if (GLB_IF_MODE_L2 == p_db_if->mode)
    {
        PM_E_RETURN(_if_agg_sync_l2hal_when_add(p_db_if_lag, p_db_if));
    }
    else if (GLB_IF_MODE_L3 == p_db_if->mode)
    {
        PM_E_RETURN(_if_agg_sync_l3hal_when_add(p_db_if_lag, p_db_if));
    }

    return PM_E_NONE;    
}

static int32
_if_agg_sync_l2hal_when_del(tbl_interface_t *p_db_if_lag)
{
    return PM_E_NONE;
}

static int32
_if_agg_sync_l3hal_when_del(tbl_interface_t *p_db_if_lag)
{
    return PM_E_NONE;
}

static int32
_if_agg_sync_hal_when_del(tbl_interface_t *p_db_if_lag)
{
    //int32 rc = PM_E_NONE;
    
    //PM_E_RETURN(fei_agg_clear_agg_prop(p_db_if_lag->ifindex));
    
    if (GLB_IF_MODE_L2 == p_db_if_lag->mode)
    {
        return _if_agg_sync_l2hal_when_del(p_db_if_lag);
    }
    else if (GLB_IF_MODE_L3 == p_db_if_lag->mode)
    {
        return _if_agg_sync_l3hal_when_del(p_db_if_lag);
    }

    return PM_E_NONE;    
}

static int32
_if_agg_sync_arpnumlimit_when_add(tbl_interface_t *p_db_if_lag, tbl_interface_t *p_db_if, uint32 first_add)
{
    int32 rc = PM_E_NONE;

    /*sync arp number limit to agg*/
    /*modified by yejl for bug 51990, 2019-05-30*/
    if (first_add)
    {
        if (p_db_if->arp_numberlimit_enable)
        {
            PM_E_RETURN(if_mgr_set_arp_numberlimit_enable(p_db_if_lag, p_db_if->arp_numberlimit_enable));
        }
        PM_E_RETURN(if_mgr_set_arp_numberlimit_number(p_db_if_lag, p_db_if->arp_numberlimit_number));
        PM_E_RETURN(if_mgr_set_arp_numberlimit_violate(p_db_if_lag, p_db_if->arp_numberlimit_violate));
    }
    else
    {
        if (p_db_if_lag->arp_numberlimit_enable)
        {
            PM_E_RETURN(if_mgr_set_arp_numberlimit_enable(p_db_if_lag, p_db_if_lag->arp_numberlimit_enable));
        }
        PM_E_RETURN(if_mgr_set_arp_numberlimit_number(p_db_if_lag, p_db_if_lag->arp_numberlimit_number));
        PM_E_RETURN(if_mgr_set_arp_numberlimit_violate(p_db_if_lag, p_db_if_lag->arp_numberlimit_violate));
    }
    /*ended by yejl*/
    /*set the the arp number limit configuration of agg member port to default*/
    if (p_db_if->arp_numberlimit_enable)
    {
        PM_E_RETURN(if_mgr_set_arp_numberlimit_enable(p_db_if, ARP_PORT_NUMBERLIMIT_DISABLE));
    }
    PM_E_RETURN(if_mgr_set_arp_numberlimit_number(p_db_if, ARP_PORT_NUMBERLIMIT_NUMBER));
    PM_E_RETURN(if_mgr_set_arp_numberlimit_violate(p_db_if, ARP_PORT_NUMBERLIMIT_PROTECT));  

    return PM_E_NONE;
}

static int32
_if_agg_sync_mac_addr(tbl_interface_t *p_db_if_lag, tbl_interface_t *p_db_if)
{
   // int32 rc = PM_E_NONE;
    
    sal_memcpy(p_db_if_lag->mac_addr, p_db_if->mac_addr, MAC_ADDR_LEN);
    sal_memcpy(p_db_if_lag->hw_mac_addr, p_db_if->hw_mac_addr, MAC_ADDR_LEN);
    tbl_interface_set_interface_field(p_db_if_lag, TBL_INTERFACE_FLD_MAC_ADDR);
    tbl_interface_set_interface_field(p_db_if_lag, TBL_INTERFACE_FLD_HW_MAC_ADDR);
    //PM_E_RETURN(if_mgr_set_kernel_mac_addr(p_db_if_lag));

    return PM_E_NONE;
}

static int32
_if_agg_sync_port_isolate(tbl_interface_t *p_db_if_lag, tbl_interface_t *p_db_if)
{
    return if_mgr_set_isolate_group_id(p_db_if_lag, p_db_if->isolate_group_id, FALSE);
}

static int32
_if_agg_sync_db_when_add(tbl_interface_t *p_db_if_lag, tbl_interface_t *p_db_if)
{
    int32 rc = PM_E_NONE;
    
    PM_E_RETURN(_if_agg_sync_agg_prop(p_db_if_lag, p_db_if));

    p_db_if_lag->mode = p_db_if->mode;
    p_db_if_lag->speed = p_db_if->speed;
    p_db_if_lag->duplex = p_db_if->duplex;
    p_db_if_lag->oper_speed = p_db_if->oper_speed;
    p_db_if_lag->oper_bandwidth = p_db_if->oper_bandwidth;
    p_db_if_lag->oper_duplex = p_db_if->oper_duplex;
    p_db_if_lag->vlan_type = p_db_if->vlan_type;
    p_db_if_lag->jumboframe_en = p_db_if->jumboframe_en;
    //p_db_if_lag->bandwidth = p_db_if->bandwidth;
    p_db_if_lag->mtu = p_db_if->mtu;
    p_db_if_lag->unicast_rpf_en = p_db_if->unicast_rpf_en;

    tbl_interface_set_interface_field(p_db_if_lag, TBL_INTERFACE_FLD_MODE);
    tbl_interface_set_interface_field(p_db_if_lag, TBL_INTERFACE_FLD_SPEED);
    tbl_interface_set_interface_field(p_db_if_lag, TBL_INTERFACE_FLD_UNICAST_RPF_EN);
    tbl_interface_set_interface_field(p_db_if_lag, TBL_INTERFACE_FLD_VLAN_TYPE);
    tbl_interface_set_interface_field(p_db_if_lag, TBL_INTERFACE_FLD_JUMBOFRAME_EN);
    tbl_interface_set_interface_field(p_db_if_lag, TBL_INTERFACE_FLD_BANDWIDTH);
    tbl_interface_set_interface_field(p_db_if_lag, TBL_INTERFACE_FLD_OPER_BANDWIDTH);
    tbl_interface_set_interface_field(p_db_if_lag, TBL_INTERFACE_FLD_OPER_DUPLEX);
    tbl_interface_set_interface_field(p_db_if_lag, TBL_INTERFACE_FLD_MTU);

    p_db_if_lag->svlan_tpid = p_db_if->svlan_tpid;
    tbl_interface_set_interface_field(p_db_if_lag, TBL_INTERFACE_FLD_SVLAN_TPID);
#if 0
    /* fix for bug 48027 configure ethertype on aggport */
    if (p_db_if->svlan_tpid == p_brg_global->vlan_tpid_type1)
    {
        p_brg_global->tpid_type1_ref_cnt++;
        PM_E_RETURN(tbl_brg_global_set_brg_global_field(p_brg_global, TBL_BRG_GLOBAL_FLD_TPID_TYPE1_REF_CNT));
    }
    else
    {
        p_brg_global->tpid_type2_ref_cnt++;
        PM_E_RETURN(tbl_brg_global_set_brg_global_field(p_brg_global, TBL_BRG_GLOBAL_FLD_TPID_TYPE2_REF_CNT));
    }
#endif
    sal_memcpy(p_db_if_lag->mac_addr, p_db_if->mac_addr, MAC_ADDR_LEN);
    PM_E_RETURN(_if_agg_sync_mac_addr(p_db_if_lag, p_db_if));
    PM_E_RETURN(_if_agg_sync_port_isolate(p_db_if_lag, p_db_if));
       
    if (GLB_IF_MODE_L2 == p_db_if->mode)
    {
        PM_E_RETURN(_if_agg_sync_l2db_when_add(p_db_if_lag, p_db_if));
    }
    else if (GLB_IF_MODE_L3 == p_db_if->mode)
    {
        /*added by yejl to fix bug44830, sync arp number limit*/
        PM_E_RETURN(_if_agg_sync_arpnumlimit_when_add(p_db_if_lag, p_db_if, TRUE));
    }
    PM_E_RETURN(_if_agg_create_agg_sync_prop(p_db_if_lag, p_db_if));
    
    return PM_E_NONE;
}

static int32
_if_agg_del_agg_update_mac_addr(tbl_interface_t *p_db_if_lag, ds_lag_t *p_db_lag, tbl_interface_t *p_if_mem_del)
{
    ctclib_slistnode_t *p_head = NULL;
    tbl_interface_t *p_if_mem = NULL;
    
    /* bug51995 remove check for l3 port update */
    /* if is L2 port, should check mac addr update */
    /* if (GLB_IF_MODE_L2 != p_db_if_lag->mode)
    {
        return PM_E_NONE;
    } */

    /* if is LAG mac_addr is same to p_if_mem_del mac_addr, should update */
    if (0 != sal_memcmp(p_db_if_lag->mac_addr, p_if_mem_del->mac_addr, MAC_ADDR_LEN))
    {
        return PM_E_NONE;
    }

    /* find first member */
    p_head = CTCLIB_SLISTHEAD(p_db_lag->member_ports.obj_list);
    if (NULL == p_head || NULL == p_head->data)
    {
        return PM_E_NONE;
    }

    p_if_mem = p_head->data;
    _if_agg_sync_mac_addr(p_db_if_lag, p_if_mem);
    sal_memcpy(p_db_if_lag->mac_addr, p_if_mem->mac_addr, MAC_ADDR_LEN);
    sal_memcpy(p_db_if_lag->hw_mac_addr, p_if_mem->hw_mac_addr, MAC_ADDR_LEN);

    return PM_E_NONE;
}

int32
if_mgr_lag_admin_key_check(tbl_interface_t *p_db_if_lag, tbl_interface_t *p_mem_if2, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_mem_if1 = NULL;
    ds_brgif_t *p_mem_brgif1 = NULL;
    ds_brgif_t *p_mem_brgif2 = NULL;
    ds_lag_t *p_lag = NULL;
    char agg_group_str[16];
    char mem_group_str[16];
    char ifname_ext[IFNAME_EXT_SIZE];
    char ifname_ext1[IFNAME_EXT_SIZE];

    char *agg_ifname = IFNAME_ETH2SHORT(p_db_if_lag->key.name, ifname_ext);
    char *mem_ifname = IFNAME_ETH2SHORT(p_mem_if2->key.name, ifname_ext1);
    
    p_lag = p_db_if_lag->lag;
    p_mem_if1 = GETDATA(CTCLIB_SLISTHEAD(p_lag->member_ports.obj_list));
    if (NULL == p_mem_if1)
    {
        CFG_CONFLICT_RET("Not found first member port of LAG");
    }

    if (p_mem_if1->speed != p_mem_if2->speed)
    {
        CFG_CONFLICT_RET("Speed mismatch, %s's Speed is %s, %s's Speed is %s",
            agg_ifname, 
            cdb_enum_val2str(glb_port_speed_strs, GLB_PORT_SPEED_MAX, p_db_if_lag->speed),
            mem_ifname, 
            cdb_enum_val2str(glb_port_speed_strs, GLB_PORT_SPEED_MAX, p_mem_if2->speed));
    }
    
    if (p_mem_if1->duplex != p_mem_if2->duplex)
    {
        CFG_CONFLICT_RET("Duplex mismatch, %s's Duplex is %s, %s's Duplex is %s",
            agg_ifname, 
            cdb_enum_val2str(glb_port_duplex_strs, GLB_PORT_DUPLEX_MAX, p_db_if_lag->duplex),
            mem_ifname, 
            cdb_enum_val2str(glb_port_duplex_strs, GLB_PORT_DUPLEX_MAX, p_mem_if2->duplex));
    }

    if (p_mem_if1->jumboframe_en != p_mem_if2->jumboframe_en)
    {
        CFG_CONFLICT_RET("Max frame size mismatch, %s's Max frame size is %u Bytes, %s's Max frame size is %u Bytes", 
            agg_ifname, if_mgr_get_max_frame_size(p_db_if_lag), mem_ifname, if_mgr_get_max_frame_size(p_mem_if2));
    }
    /* Modified by kcao 2015-03-25 for bug 36404, not check phy_type */
#if 0    
    if (p_mem_if1->phy_type != p_mem_if2->phy_type)
    {
        CFG_CONFLICT_RET("Hardware type mismatch, %s's type is %s, %s's type is %s",
            agg_ifname, 
            cdb_enum_val2str(glb_phy_type_strs, GLB_PHY_TYPE_MAX, p_mem_if1->phy_type),
            mem_ifname, 
            cdb_enum_val2str(glb_phy_type_strs, GLB_PHY_TYPE_MAX, p_mem_if2->phy_type));
    }
#endif
    if (p_mem_if1->mode != p_mem_if2->mode)
    {
        CFG_CONFLICT_RET("Port type mismatch, %s's type is %s, %s's type is %s",
            agg_ifname, 
            cdb_enum_val2str(glb_if_mode_strs, GLB_IF_MODE_MAX, p_db_if_lag->mode),
            mem_ifname, 
            cdb_enum_val2str(glb_if_mode_strs, GLB_IF_MODE_MAX, p_mem_if2->mode));
    }
    if (p_mem_if1->isolate_group_id != p_mem_if2->isolate_group_id)
    {
        agg_group_str[0] = CMD_ZERO_CHAR;
        mem_group_str[0] = CMD_ZERO_CHAR;
        (p_db_if_lag->isolate_group_id) ? 
            sal_snprintf(agg_group_str, 16, "%u", p_db_if_lag->isolate_group_id) : 
            sal_snprintf(agg_group_str, 16, "%s", "not specified");
        (p_mem_if2->isolate_group_id) ? 
            sal_snprintf(mem_group_str, 16, "%u", p_mem_if2->isolate_group_id) : 
            sal_snprintf(mem_group_str, 16, "%s", "not specified");
        CFG_CONFLICT_RET("Port isolate group mismatch, %s's isolate group is %s, %s's isolate group is %s",
            agg_ifname, 
            agg_group_str,
            mem_ifname, 
            mem_group_str);
    }

    /* physical port's dot1q ethtype should be equal LAG's */
    if (p_mem_if1->svlan_tpid != p_mem_if2->svlan_tpid)
    {
        CFG_CONFLICT_RET("Port dot1q ethertype mismatch, %s's type is 0x%x, %s's type is 0x%x",
            agg_ifname, p_db_if_lag->svlan_tpid, mem_ifname, p_mem_if2->svlan_tpid);
    }
    
    if (GLB_IF_MODE_L2 == p_mem_if1->mode)
    {
        p_mem_brgif1 = p_db_if_lag->brgif;
        p_mem_brgif2 = p_mem_if2->brgif;

        if (p_mem_if1->vlan_type != p_mem_if2->vlan_type)
        {
            CFG_CONFLICT_RET("Vlan port type mismatch, %s is %s port, %s is %s port",
                agg_ifname, 
                cdb_enum_val2str(glb_vlan_port_type_strs, GLB_VLAN_PORT_TYPE_MAX, p_db_if_lag->vlan_type),
                mem_ifname, 
                cdb_enum_val2str(glb_vlan_port_type_strs, GLB_VLAN_PORT_TYPE_MAX, p_mem_if2->vlan_type));
        }
        if (p_mem_brgif1->pvid != p_mem_brgif2->pvid)
        {
            CFG_CONFLICT_RET("PVID mismatch, %s's PVID is %u, %s's PVID is %u",
                agg_ifname, p_mem_brgif1->pvid, mem_ifname, p_mem_brgif2->pvid);
        }
        /* physical port's dot1q tunnel native vlan should be equal LAG's, same as inner native vlan */
        if (p_mem_brgif1->inner_native_vid != p_mem_brgif2->inner_native_vid)
        {
            CFG_CONFLICT_RET("Inner-native-vid mismatch, %s's PVID is %u, %s's PVID is %u",
                agg_ifname, p_mem_brgif1->inner_native_vid, mem_ifname, p_mem_brgif2->inner_native_vid);
        }
        #if 0
        /* physical port's qinq_type and vlan translaiton no need to be equal LAG's */
        if (p_mem_brgif1->qinq_type != p_mem_brgif2->qinq_type)
        {
            CFG_CONFLICT_RET("QinQ type mismatch, %s's QinQ type is %s, %s's QinQ type is %s",
                agg_ifname, cdb_enum_val2str(glb_vlan_qinq_type_strs, GLB_VLAN_QINQ_TYPE_MAX, p_mem_brgif1->qinq_type),
                mem_ifname, cdb_enum_val2str(glb_vlan_qinq_type_strs, GLB_VLAN_QINQ_TYPE_MAX, p_mem_brgif2->qinq_type));
        }
        /* The trunk port which does not enable VLAN translation can be allowed to add to 
           VLAN translation LAG when its VLAN attribute is same with the LAG */
        if (p_mem_brgif1->vlan_translation_en != p_mem_brgif2->vlan_translation_en)
        {
            CFG_CONFLICT_RET("VLAN translation enable mismatch, %s's is %s, %s's is %s",
                agg_ifname, p_mem_brgif1->vlan_translation_en ? "enable" : "disable",
                mem_ifname, p_mem_brgif2->vlan_translation_en ? "enable" : "disable");
        }
        /* no need to check default fwd configuration either */
        if (p_mem_brgif1->qinq_default_fwd != p_mem_brgif2->qinq_default_fwd)
        {
            CFG_CONFLICT_RET("QinQ default forward enable mismatch, %s's is %s, %s's is %s",
                agg_ifname, p_mem_brgif1->qinq_default_fwd ? "enable" : "disable",
                mem_ifname, p_mem_brgif2->qinq_default_fwd ? "enable" : "disable");
        }
        if (p_mem_brgif1->vlan_trans_default_fwd != p_mem_brgif2->vlan_trans_default_fwd)
        {
            CFG_CONFLICT_RET("VLAN translation default forward enable mismatch, %s's is %s, %s's is %s",
                agg_ifname, p_mem_brgif1->vlan_trans_default_fwd ? "enable" : "disable",
                mem_ifname, p_mem_brgif2->vlan_trans_default_fwd ? "enable" : "disable");
        }
        #endif
        /* physical port's tag native vlan configuration  and vlan translaiton should be equal LAG's */
        if (p_mem_brgif1->dot1q_tag_native_en != p_mem_brgif2->dot1q_tag_native_en)
        {
            CFG_CONFLICT_RET("Dot1q tag native enable mismatch, %s's is %s, %s's is %s",
                agg_ifname, p_mem_brgif1->dot1q_tag_native_en ? "enable" : "disable",
                mem_ifname, p_mem_brgif2->dot1q_tag_native_en ? "enable" : "disable");
        }
        if (p_mem_brgif1->ingress_filter != p_mem_brgif2->ingress_filter)        
        {
            CFG_CONFLICT_RET("Ingress filter mismatch, %s's is %s, %s's is %s",
                agg_ifname, enable_str(p_mem_brgif1->ingress_filter),
                mem_ifname, enable_str(p_mem_brgif2->ingress_filter));
        }
        if (p_mem_brgif1->mac_learning_en != p_mem_brgif2->mac_learning_en)        
        {
            CFG_CONFLICT_RET("MAC learning enable mismatch, %s's is %s, %s's is %s",
                agg_ifname, enable_str(p_mem_brgif1->mac_learning_en),
                mem_ifname, enable_str(p_mem_brgif2->mac_learning_en));
        }
        if (0 != sal_memcmp(&p_mem_brgif1->allowed_vlan.vlan_bmp, &p_mem_brgif2->allowed_vlan.vlan_bmp, sizeof(vlan_bmp_t))
            || (p_mem_brgif1->allowed_vlan.action != p_mem_brgif2->allowed_vlan.action))
        {
            CFG_CONFLICT_RET("Vlan membership doesn't match, Please use vlan command to show detail");
        }
        if (0 != sal_memcmp(&p_mem_brgif1->untagged_vlan.vlan_bmp, &p_mem_brgif2->untagged_vlan.vlan_bmp, sizeof(vlan_bmp_t)))
        {
            CFG_CONFLICT_RET("Vlan untagged vlan doesn't match");
        }
        
        if (p_mem_brgif1->port_security_en != p_mem_brgif2->port_security_en)
        {
            CFG_CONFLICT_RET("Port security enable mismatch, %s's port-security is %s, %s's port-security is %s",
                agg_ifname, 
                cdb_enum_val2str(glb_port_security_enable_strs, GLB_PORT_SECURITY_MAX, p_mem_brgif1->port_security_en),
                mem_ifname, 
                cdb_enum_val2str(glb_port_security_enable_strs, GLB_PORT_SECURITY_MAX, p_mem_brgif2->port_security_en))
        }
        if (p_mem_brgif1->max_mac != p_mem_brgif2->max_mac)
        {
            CFG_CONFLICT_RET("Port security maxmum mismatch, %s's maxmum is %u, %s's maxmum is %u",
                agg_ifname, p_mem_brgif1->max_mac,
                mem_ifname, p_mem_brgif2->max_mac);
        }
        if (p_mem_brgif1->violate_mode != p_mem_brgif2->violate_mode)
        {
            CFG_CONFLICT_RET("Port security violation mismatch, %s's violation is %s, %s's violation is %s",
                agg_ifname, 
                cdb_enum_val2str(glb_port_security_violation_strs, GLB_PORT_SECURITY_VIOLATION_MODE_MAX,
                p_mem_brgif1->violate_mode),
                mem_ifname, 
                cdb_enum_val2str(glb_port_security_violation_strs, GLB_PORT_SECURITY_VIOLATION_MODE_MAX,
                p_mem_brgif2->violate_mode));
        }
        if (p_mem_if1->arpinsp_trust_enable != p_mem_if2->arpinsp_trust_enable)
        {
            CFG_CONFLICT_RET("Arp inspection trust mismatch, %s is %s, %s is %s",
                agg_ifname, 
                cdb_enum_val2str(arp_inspection_trust_enable_strs, ARPINSP_PORT_MAX,
                p_mem_if1->arpinsp_trust_enable),
                mem_ifname, 
                cdb_enum_val2str(arp_inspection_trust_enable_strs, ARPINSP_PORT_MAX,
                p_mem_if2->arpinsp_trust_enable));
        }
            
    }
    else
    {
    /*fix bug 45170 by chenc, when l3 routed port add to agg, not check config of num-limit and rate-limit */
     #if 0
        if (p_db_if_lag->arp_numberlimit_enable != p_mem_if2->arp_numberlimit_enable)
        {
            CFG_CONFLICT_RET("Arp numberlimit mismatch, %s's numberlimit is %s, %s's numberlimit is %s",
                agg_ifname, 
                cdb_enum_val2str(arp_port_numberlimit_enable_strs, ARP_PORT_NUMBERLIMIT_MAX,
                p_db_if_lag->arp_numberlimit_enable),
                mem_ifname, 
                cdb_enum_val2str(arp_port_numberlimit_enable_strs, ARP_PORT_NUMBERLIMIT_MAX,
                p_mem_if2->arp_numberlimit_enable));
        }

        if (p_db_if_lag->arp_numberlimit_number != p_mem_if2->arp_numberlimit_number)
        {
            CFG_CONFLICT_RET("Arp numberlimit number mismatch, %s's number is %d, %s's number is %d",
                agg_ifname, p_db_if_lag->arp_numberlimit_number,
                mem_ifname, p_mem_if2->arp_numberlimit_number);
        }

        if (p_db_if_lag->arp_numberlimit_violate != p_mem_if2->arp_numberlimit_violate)
        {
            CFG_CONFLICT_RET("Arp numberlimit violate mismatch, %s's violate is %s, %s's violate is %s",
                agg_ifname, 
                cdb_enum_val2str(arp_port_numberlimit_violate_strs, ARP_PORT_NUMBERLIMIT_VIOLATE_MAX,
                p_db_if_lag->arp_numberlimit_violate),
                mem_ifname, 
                cdb_enum_val2str(arp_port_numberlimit_violate_strs, ARP_PORT_NUMBERLIMIT_VIOLATE_MAX,
                p_mem_if2->arp_numberlimit_violate));
        }

        /*arp rate limit start*/     
        sal_memset(&rt_if_key, 0, sizeof(rt_if_key));        
        sal_memcpy(rt_if_key.name, p_db_if_lag->key.name, IFNAME_SIZE);        
        p_lag_rt_if =  tbl_route_if_get_route_if(&rt_if_key);        

        sal_memset(&rt_if_key, 0, sizeof(rt_if_key));        
        sal_memcpy(rt_if_key.name, p_mem_if2->key.name, IFNAME_SIZE);        
        p_mem_rt_if2 =  tbl_route_if_get_route_if(&rt_if_key);
     
        if (p_lag_rt_if->arp_rate_limit_en != p_mem_rt_if2->arp_rate_limit_en)
        {
            CFG_CONFLICT_RET("Arp rate limit enable mismatch, %s's state is %s, %s's state is %s",
                agg_ifname, 
                cdb_enum_val2str(arpratelimit_enable_str, ARP_RATE_LIMIT_STATE_MAX,
                p_lag_rt_if->arp_rate_limit_en),
                mem_ifname, 
                cdb_enum_val2str(arpratelimit_enable_str, ARP_RATE_LIMIT_STATE_MAX,
                p_mem_rt_if2->arp_rate_limit_en));
        }

        if (p_lag_rt_if->arp_rate_limit_violation != p_mem_rt_if2->arp_rate_limit_violation)
        {
            CFG_CONFLICT_RET("Arp rate limit violation mode mismatch, %s's violation mode is %s, %s's violation mode is %s",
                agg_ifname, 
                cdb_enum_val2str(arpratelimit_exceed_action_str, ARP_RATE_LIMIT_ACTION_MAX,
                p_lag_rt_if->arp_rate_limit_violation),
                mem_ifname, 
                cdb_enum_val2str(arpratelimit_exceed_action_str, ARP_RATE_LIMIT_ACTION_MAX,
                p_mem_rt_if2->arp_rate_limit_violation));
        }

        if (p_lag_rt_if->arp_rate_limit_pkt_max != p_mem_rt_if2->arp_rate_limit_pkt_max)
        {
            CFG_CONFLICT_RET("Arp rate limit pkt max mismatch, %s's pkt max is %d, %s's pkt max is %d",
                agg_ifname, 
                p_lag_rt_if->arp_rate_limit_pkt_max,
                mem_ifname, 
                p_mem_rt_if2->arp_rate_limit_pkt_max);
        }
        /*arp rate limit end*/
        #endif
    }

    return PM_E_NONE;  
}

extern int32
acl_api_agg_set_redirect_cb(tbl_interface_t *p_agg_if);

tbl_interface_t* _if_agg_get_agg_interface_by_member(tbl_interface_t *p_db_if_mem)
{
    tbl_interface_master_t *p_master = NULL;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_interface_t *p_db_if = NULL;

    p_master = tbl_interface_get_master();

    if (NULL == p_master)
    {
        return NULL;
    }
    
    if(NULL == p_db_if_mem->lag)
    {
        return NULL;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->if_list, p_db_if, listnode, next)
    {
        if(GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
        {
            if(p_db_if_mem->lag->lag_id == p_db_if->lag->lag_id)
            {
                return p_db_if;
            }
        }
    }

    return NULL;
}

uint32 _if_active_backup_agg_get_active_max_priority_num(tbl_interface_t *p_db_if_lag)
{
    tbl_interface_t *p_db_mem = NULL;
    ctclib_slistnode_t *listnode = NULL;
    uint32 max_priority = 0;
    
    CTCLIB_SLIST_LOOP(p_db_if_lag->lag->member_ports.obj_list, p_db_mem, listnode)
    {
        if((p_db_mem->static_agg_is_active) && (p_db_mem->static_agg_priority > max_priority))
        {
            max_priority = p_db_mem->static_agg_priority;
        }
    }

    return max_priority;
}

tbl_interface_t* _if_active_backup_agg_get_active_max_priority_port(tbl_interface_t *p_db_if_lag)
{
    tbl_interface_t *p_db_mem = NULL;
    ctclib_slistnode_t *listnode = NULL;
    uint32 max_priority = 0;
    tbl_interface_t *p_db_mem_max_priority = NULL;
    
    CTCLIB_SLIST_LOOP(p_db_if_lag->lag->member_ports.obj_list, p_db_mem, listnode)
    {
        if((p_db_mem->static_agg_is_active) && (p_db_mem->static_agg_priority > max_priority))
        {
            max_priority = p_db_mem->static_agg_priority;
            p_db_mem_max_priority = p_db_mem;
        }
    }

    return p_db_mem_max_priority;
}

tbl_interface_t* _if_active_backup_agg_get_backup_running_min_priority_port(tbl_interface_t *p_db_if_lag)
{
    tbl_interface_t *p_db_mem = NULL;
    ctclib_slistnode_t *listnode = NULL;
    uint32 min_priority = 0xFFFFFFFF;
    tbl_interface_t *p_db_mem_min_priority = NULL;
    uint32 mem_is_up = 0;
    
    CTCLIB_SLIST_LOOP(p_db_if_lag->lag->member_ports.obj_list, p_db_mem, listnode)
    {
        mem_is_up = tbl_interface_is_running(p_db_mem);
        if((!p_db_mem->static_agg_is_active) && (p_db_mem->static_agg_priority < min_priority) && mem_is_up)
        {
            min_priority = p_db_mem->static_agg_priority;
            p_db_mem_min_priority = p_db_mem;
        }
    }

    return p_db_mem_min_priority;
}

int32 _if_active_backup_agg_oper_add_mem(tbl_interface_t *p_db_if_lag, tbl_interface_t *p_db_if_mem, uint32 agg_member_priority)
{
    char mem_ifname_ext[IFNAME_SIZE];
    uint32 add_memeber_priority_db = 0;
    uint32 mem_is_up = 0;

    tbl_interface_t *p_db_mem = NULL;
    ctclib_slistnode_t *listnode = NULL;
    
    add_memeber_priority_db = ((agg_member_priority << 16) + p_db_if_mem->ifindex);
    mem_is_up = tbl_interface_is_running(p_db_if_mem);
    if(!mem_is_up)
    {
        /*sync mem port db*/
        p_db_if_mem->static_agg_is_active = 0;
        p_db_if_mem->static_agg_priority = add_memeber_priority_db;
        tbl_interface_set_interface_field(p_db_if_mem, TBL_INTERFACE_FLD_STATIC_AGG_PRIORITY);
        tbl_interface_set_interface_field(p_db_if_mem, TBL_INTERFACE_FLD_STATIC_AGG_IS_ACTIVE);
        /* add by wulx for bug 52152, 2019-08-01, add log */
        IFNAME_ETH2SHORT(p_db_if_mem->key.name, mem_ifname_ext);
        logid_sys(LOG_LAG_4_CHANGE_INACTIVE, p_db_if_lag->key.name, mem_ifname_ext);
    }
    else
    {
        if(p_db_if_lag->lag->cur_active_number < p_db_if_lag->lag->max_active_number)
        {
            /*sync mem port db*/
            p_db_if_mem->static_agg_is_active = 1;
            p_db_if_mem->static_agg_priority = add_memeber_priority_db;
            tbl_interface_set_interface_field(p_db_if_mem, TBL_INTERFACE_FLD_STATIC_AGG_PRIORITY);
            tbl_interface_set_interface_field(p_db_if_mem, TBL_INTERFACE_FLD_STATIC_AGG_IS_ACTIVE);
            /*sync agg db*/
            p_db_if_lag->lag->cur_active_number ++;
            ds_lag_set_lag_field(p_db_if_lag, p_db_if_lag->lag, DS_LAG_FLD_CUR_ACTIVE_NUMBER);
            if(add_memeber_priority_db > p_db_if_lag->lag->cur_active_max_priority)
            {
                p_db_if_lag->lag->cur_active_max_priority = add_memeber_priority_db;
            }
            ds_lag_set_lag_field(p_db_if_lag, p_db_if_lag->lag, DS_LAG_FLD_CUR_ACTIVE_MAX_PRIORITY);
            /* add by wulx for bug 52152, 2019-08-01, add log */
            IFNAME_ETH2SHORT(p_db_if_mem->key.name, mem_ifname_ext);
            logid_sys(LOG_LAG_4_CHANGE_ACTIVE, p_db_if_lag->key.name, mem_ifname_ext);
        }
        else
        {
            if(add_memeber_priority_db > p_db_if_lag->lag->cur_active_max_priority) /*not need change active port*/
            {
                /*sync mem port db*/
                p_db_if_mem->static_agg_is_active = 0;
                p_db_if_mem->static_agg_priority = add_memeber_priority_db;
                tbl_interface_set_interface_field(p_db_if_mem, TBL_INTERFACE_FLD_STATIC_AGG_PRIORITY);
                tbl_interface_set_interface_field(p_db_if_mem, TBL_INTERFACE_FLD_STATIC_AGG_IS_ACTIVE);
                /* add by wulx for bug 52152, 2019-08-01, add log */
                IFNAME_ETH2SHORT(p_db_if_mem->key.name, mem_ifname_ext);
                logid_sys(LOG_LAG_4_CHANGE_INACTIVE, p_db_if_lag->key.name, mem_ifname_ext);
            }
            else
            {
                /*should find the mem_port whitch should be replaced*/
                CTCLIB_SLIST_LOOP(p_db_if_lag->lag->member_ports.obj_list, p_db_mem, listnode)
                {
                    if((p_db_mem->static_agg_priority == p_db_if_lag->lag->cur_active_max_priority) && (p_db_mem->static_agg_is_active))
                    {
                        p_db_mem->static_agg_is_active = 0;
                        tbl_interface_set_interface_field(p_db_mem, TBL_INTERFACE_FLD_STATIC_AGG_IS_ACTIVE);
                        /* add by wulx for bug 52152, 2019-08-01, add log */
                        IFNAME_ETH2SHORT(p_db_mem->key.name, mem_ifname_ext);
                        logid_sys(LOG_LAG_4_CHANGE_INACTIVE, p_db_if_lag->key.name, mem_ifname_ext);

                        /*sync mem port db*/
                        p_db_if_mem->static_agg_is_active = 1;
                        p_db_if_mem->static_agg_priority = add_memeber_priority_db;
                        tbl_interface_set_interface_field(p_db_if_mem, TBL_INTERFACE_FLD_STATIC_AGG_PRIORITY);
                        tbl_interface_set_interface_field(p_db_if_mem, TBL_INTERFACE_FLD_STATIC_AGG_IS_ACTIVE);
                        /* add by wulx for bug 52152, 2019-08-01, add log */
                        IFNAME_ETH2SHORT(p_db_if_mem->key.name, mem_ifname_ext);
                        logid_sys(LOG_LAG_4_CHANGE_ACTIVE, p_db_if_lag->key.name, mem_ifname_ext);

                        /*sync agg db*/
                        if(add_memeber_priority_db > _if_active_backup_agg_get_active_max_priority_num(p_db_if_lag))
                        {
                            p_db_if_lag->lag->cur_active_max_priority = add_memeber_priority_db;
                        }
                        else
                        {
                            p_db_if_lag->lag->cur_active_max_priority = _if_active_backup_agg_get_active_max_priority_num(p_db_if_lag);
                        }
                        ds_lag_set_lag_field(p_db_if_lag, p_db_if_lag->lag, DS_LAG_FLD_CUR_ACTIVE_MAX_PRIORITY);

                        break;
                    }
                }
            }
        }
    }       
    if(p_db_if_lag)
    {
        if_agg_aggregator_bw_update(p_db_if_lag);
    }
    return 0;
}

int32 _if_active_backup_agg_oper_del_mem(tbl_interface_t *p_db_if_lag, tbl_interface_t *p_db_if_mem, char *agg_name)
{
    char mem_ifname_ext[IFNAME_SIZE];
    uint32 mem_is_up = 0;
    tbl_interface_t* p_db_backup_min_priority = NULL;
    
    mem_is_up = tbl_interface_is_running(p_db_if_mem);
    if((NULL == p_db_if_lag) || (!mem_is_up) || (!p_db_if_mem->static_agg_is_active))
    {
        /*DEL ALL MEM or removed mem port is down or is not active*/
        /*sync mem port db*/
        p_db_if_mem->static_agg_is_active = 0;
        p_db_if_mem->static_agg_priority = 0;
        tbl_interface_set_interface_field(p_db_if_mem, TBL_INTERFACE_FLD_STATIC_AGG_PRIORITY);
        tbl_interface_set_interface_field(p_db_if_mem, TBL_INTERFACE_FLD_STATIC_AGG_IS_ACTIVE);
        /* add by wulx for bug 52152, 2019-08-01, add log */
        IFNAME_ETH2SHORT(p_db_if_mem->key.name, mem_ifname_ext);
        if ((NULL != p_db_if_lag))
        {
            logid_sys(LOG_LAG_4_CHANGE_INACTIVE, p_db_if_lag->key.name, mem_ifname_ext);
        }
        else if (NULL != agg_name)
        {
            logid_sys(LOG_LAG_4_CHANGE_INACTIVE, agg_name, mem_ifname_ext);
        }

        return 0;
    }


    p_db_backup_min_priority = _if_active_backup_agg_get_backup_running_min_priority_port(p_db_if_lag);
    if(p_db_backup_min_priority) /*running backup port is exist*/
    {
         /*sync mem port db*/
        p_db_backup_min_priority->static_agg_is_active = 1;
        tbl_interface_set_interface_field(p_db_backup_min_priority, TBL_INTERFACE_FLD_STATIC_AGG_IS_ACTIVE);
        /* add by wulx for bug 52152, 2019-08-01, add log */
        IFNAME_ETH2SHORT(p_db_backup_min_priority->key.name, mem_ifname_ext);
        logid_sys(LOG_LAG_4_CHANGE_ACTIVE, p_db_if_lag->key.name, mem_ifname_ext);
    }
    else
    {
        p_db_if_lag->lag->cur_active_number --;
        ds_lag_set_lag_field(p_db_if_lag, p_db_if_lag->lag, DS_LAG_FLD_CUR_ACTIVE_NUMBER);
    }

    /*sync max priority*/
    p_db_if_lag->lag->cur_active_max_priority = _if_active_backup_agg_get_active_max_priority_num(p_db_if_lag);
    ds_lag_set_lag_field(p_db_if_lag, p_db_if_lag->lag, DS_LAG_FLD_CUR_ACTIVE_MAX_PRIORITY);

    if_agg_aggregator_bw_update(p_db_if_lag);

    return 0;
    
}

int32 _if_active_backup_agg_oper_change_num(tbl_interface_t *p_db_if_lag, uint32 max_active_number)
{
    char mem_ifname_ext[IFNAME_SIZE];
    uint32 add_num = 1;
    uint32 del_num = 1;
    tbl_interface_t* p_backup_ready_mem_db = NULL;
    tbl_interface_t* p_active_priority_db = NULL;
    
    if(max_active_number > p_db_if_lag->lag->cur_active_number)
    {
        while(add_num)
        {
            add_num = 0;
            if(p_db_if_lag->lag->cur_active_number != max_active_number)
            {
                p_backup_ready_mem_db = _if_active_backup_agg_get_backup_running_min_priority_port(p_db_if_lag);
                if(p_backup_ready_mem_db)
                {
                    /*sync mem port db*/
                    p_backup_ready_mem_db->static_agg_is_active = 1;
                    tbl_interface_set_interface_field(p_backup_ready_mem_db, TBL_INTERFACE_FLD_STATIC_AGG_IS_ACTIVE);
                    /* add by wulx for bug 52152, 2019-08-01, add log */
                    IFNAME_ETH2SHORT(p_backup_ready_mem_db->key.name, mem_ifname_ext);
                    logid_sys(LOG_LAG_4_CHANGE_ACTIVE, p_db_if_lag->key.name, mem_ifname_ext);

                    p_db_if_lag->lag->cur_active_number ++;
                    ds_lag_set_lag_field(p_db_if_lag, p_db_if_lag->lag, DS_LAG_FLD_CUR_ACTIVE_NUMBER);
                    add_num = 1;
                }
            }
        }
        /*sync max priority*/
        p_db_if_lag->lag->cur_active_max_priority = _if_active_backup_agg_get_active_max_priority_num(p_db_if_lag);
        ds_lag_set_lag_field(p_db_if_lag, p_db_if_lag->lag, DS_LAG_FLD_CUR_ACTIVE_MAX_PRIORITY);
    }
    else if(max_active_number < p_db_if_lag->lag->cur_active_number)
    {
        while(del_num)
        {
            del_num = 0;
            if(p_db_if_lag->lag->cur_active_number != max_active_number)
            {
                p_active_priority_db = _if_active_backup_agg_get_active_max_priority_port(p_db_if_lag);
                if(p_active_priority_db)
                {
                    p_active_priority_db->static_agg_is_active = 0;
                    tbl_interface_set_interface_field(p_active_priority_db, TBL_INTERFACE_FLD_STATIC_AGG_IS_ACTIVE);
                    /* add by wulx for bug 52152, 2019-08-01, add log */
                    IFNAME_ETH2SHORT(p_active_priority_db->key.name, mem_ifname_ext);
                    logid_sys(LOG_LAG_4_CHANGE_INACTIVE, p_db_if_lag->key.name, mem_ifname_ext);

                    p_db_if_lag->lag->cur_active_number --;
                    ds_lag_set_lag_field(p_db_if_lag, p_db_if_lag->lag, DS_LAG_FLD_CUR_ACTIVE_NUMBER);
                    del_num = 1;
                }
            }
        }

        /*sync max priority*/
        p_db_if_lag->lag->cur_active_max_priority = _if_active_backup_agg_get_active_max_priority_num(p_db_if_lag);
        ds_lag_set_lag_field(p_db_if_lag, p_db_if_lag->lag, DS_LAG_FLD_CUR_ACTIVE_MAX_PRIORITY);
    }

    p_db_if_lag->lag->max_active_number = max_active_number;
    ds_lag_set_lag_field(p_db_if_lag, p_db_if_lag->lag, DS_LAG_FLD_MAX_ACTIVE_NUMBER);

    if_agg_aggregator_bw_update(p_db_if_lag);
    return 0;
}

int32 _if_active_backup_agg_oper_port_up_down(tbl_interface_t *p_db_if)
{
    char mem_ifname_ext[IFNAME_SIZE];
    tbl_interface_t *p_db_backup_min_priority = NULL;
    tbl_interface_t *p_db_active_max_priority = NULL;
    tbl_interface_t *p_db_lag = NULL;
    uint32 is_running = 0;
    
    if(GLB_IF_TYPE_PORT_IF == p_db_if->hw_type)
    {
        is_running = tbl_interface_is_running(p_db_if);
        if(NULL == p_db_if->lag)
        {
            return 0;
        }
        p_db_lag = _if_agg_get_agg_interface_by_member(p_db_if);
        if(NULL == p_db_lag)
        {
            return 0;
        }
        if(is_running)
        {
            if((p_db_lag->lag->cur_active_number < p_db_lag->lag->max_active_number) && (!p_db_if->static_agg_is_active))
            {
                /*sync mem port db*/
                p_db_if->static_agg_is_active = 1;
                tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_STATIC_AGG_IS_ACTIVE);
                /* add by wulx for bug 52152, 2019-08-01, add log */
                IFNAME_ETH2SHORT(p_db_if->key.name, mem_ifname_ext);
                logid_sys(LOG_LAG_4_CHANGE_ACTIVE, p_db_lag->key.name, mem_ifname_ext);

                p_db_lag->lag->cur_active_number ++;
                ds_lag_set_lag_field(p_db_lag, p_db_lag->lag, DS_LAG_FLD_CUR_ACTIVE_NUMBER);
                /*sync max priority*/
                p_db_lag->lag->cur_active_max_priority = _if_active_backup_agg_get_active_max_priority_num(p_db_lag);
                ds_lag_set_lag_field(p_db_lag, p_db_lag->lag, DS_LAG_FLD_CUR_ACTIVE_MAX_PRIORITY);             
            }
            else if((p_db_if->static_agg_priority < p_db_lag->lag->cur_active_max_priority) && (!p_db_if->static_agg_is_active)) /*need insert*/
            {
                 p_db_active_max_priority = _if_active_backup_agg_get_active_max_priority_port(p_db_lag);
                 if(p_db_active_max_priority) /*if p_db_active_max_priority = 0, mean max_active_number = 0*/
                 {
                     /*sync mem port db*/
                     p_db_if->static_agg_is_active = 1;
                     tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_STATIC_AGG_IS_ACTIVE);
                     /* add by wulx for bug 52152, 2019-08-01, add log */
                     IFNAME_ETH2SHORT(p_db_if->key.name, mem_ifname_ext);
                     logid_sys(LOG_LAG_4_CHANGE_ACTIVE, p_db_lag->key.name, mem_ifname_ext);

                     /*sync mem port db*/
                     p_db_active_max_priority->static_agg_is_active = 0;
                     tbl_interface_set_interface_field(p_db_active_max_priority, TBL_INTERFACE_FLD_STATIC_AGG_IS_ACTIVE);

                     /*sync max priority*/
                     p_db_lag->lag->cur_active_max_priority = _if_active_backup_agg_get_active_max_priority_num(p_db_lag);
                     ds_lag_set_lag_field(p_db_lag, p_db_lag->lag, DS_LAG_FLD_CUR_ACTIVE_MAX_PRIORITY);
                 }
            }
        }
        else
        {
            if(p_db_if->static_agg_is_active)
            {
                /*sync mem port db*/
                p_db_if->static_agg_is_active = 0;
                tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_STATIC_AGG_IS_ACTIVE);
                /* add by wulx for bug 52152, 2019-08-01, add log */
                IFNAME_ETH2SHORT(p_db_if->key.name, mem_ifname_ext);
                logid_sys(LOG_LAG_4_CHANGE_INACTIVE, p_db_lag->key.name, mem_ifname_ext);
                
                p_db_lag->lag->cur_active_number --;
                ds_lag_set_lag_field(p_db_lag, p_db_lag->lag, DS_LAG_FLD_CUR_ACTIVE_NUMBER);

                p_db_backup_min_priority = _if_active_backup_agg_get_backup_running_min_priority_port(p_db_lag);
                if(p_db_backup_min_priority)
                {
                    /*sync mem port db*/
                    p_db_backup_min_priority->static_agg_is_active = 1;
                    tbl_interface_set_interface_field(p_db_backup_min_priority, TBL_INTERFACE_FLD_STATIC_AGG_IS_ACTIVE);
                    /* add by wulx for bug 52152, 2019-08-01, add log */
                    IFNAME_ETH2SHORT(p_db_backup_min_priority->key.name, mem_ifname_ext);
                    logid_sys(LOG_LAG_4_CHANGE_ACTIVE, p_db_lag->key.name, mem_ifname_ext);

                    p_db_lag->lag->cur_active_number ++;
                    ds_lag_set_lag_field(p_db_lag, p_db_lag->lag, DS_LAG_FLD_CUR_ACTIVE_NUMBER);
                }

                /*sync max priority*/
                p_db_lag->lag->cur_active_max_priority = _if_active_backup_agg_get_active_max_priority_num(p_db_lag);
                ds_lag_set_lag_field(p_db_lag, p_db_lag->lag, DS_LAG_FLD_CUR_ACTIVE_MAX_PRIORITY);
            }
        }

        if_agg_aggregator_bw_update(p_db_lag);
    }

    return 0;
}

int32 _if_active_backup_agg_oper_change_mem_priority(tbl_interface_t *p_db_if_mem, uint32 agg_member_priority)
{
    char mem_ifname_ext[IFNAME_SIZE];
    tbl_interface_t *p_db_lag = NULL;
    tbl_interface_t * p_db_active_max_priority = NULL;
    tbl_interface_t * p_db_backup_min_priority = NULL;
    uint32 is_running = 0;
    uint32 add_memeber_priority_db = 0;

    add_memeber_priority_db = ((agg_member_priority << 16) + p_db_if_mem->ifindex);

    if(p_db_if_mem->static_agg_priority == add_memeber_priority_db)
    {
        return 0;
    }
    
    p_db_lag = _if_agg_get_agg_interface_by_member(p_db_if_mem); /*the agg must be exist*/
    
    if(!p_db_if_mem->static_agg_is_active)
    {
        is_running = tbl_interface_is_running(p_db_if_mem);
        if((is_running) && (add_memeber_priority_db < p_db_lag->lag->cur_active_max_priority))
        {
            /*sync mem port db*/
            p_db_if_mem->static_agg_is_active = 1;
            tbl_interface_set_interface_field(p_db_if_mem, TBL_INTERFACE_FLD_STATIC_AGG_IS_ACTIVE);
            p_db_if_mem->static_agg_priority = add_memeber_priority_db;
            tbl_interface_set_interface_field(p_db_if_mem, TBL_INTERFACE_FLD_STATIC_AGG_PRIORITY);
            /* add by wulx for bug 52152, 2019-08-01, add log */
            IFNAME_ETH2SHORT(p_db_if_mem->key.name, mem_ifname_ext);
            logid_sys(LOG_LAG_4_CHANGE_ACTIVE, p_db_lag->key.name, mem_ifname_ext);

            p_db_active_max_priority = _if_active_backup_agg_get_active_max_priority_port(p_db_lag);
            if(p_db_active_max_priority)
            {
                p_db_active_max_priority->static_agg_is_active =0;
                tbl_interface_set_interface_field(p_db_active_max_priority, TBL_INTERFACE_FLD_STATIC_AGG_IS_ACTIVE);
                /* add by wulx for bug 52152, 2019-08-01, add log */
                IFNAME_ETH2SHORT(p_db_active_max_priority->key.name, mem_ifname_ext);
                logid_sys(LOG_LAG_4_CHANGE_INACTIVE, p_db_lag->key.name, mem_ifname_ext);
            }

            /*sync max priority*/
            p_db_lag->lag->cur_active_max_priority = _if_active_backup_agg_get_active_max_priority_num(p_db_lag);
            ds_lag_set_lag_field(p_db_lag, p_db_lag->lag, DS_LAG_FLD_CUR_ACTIVE_MAX_PRIORITY);
            
        }
        else
        {
            p_db_if_mem->static_agg_priority = add_memeber_priority_db;
            tbl_interface_set_interface_field(p_db_if_mem, TBL_INTERFACE_FLD_STATIC_AGG_PRIORITY);
        }
        return 0;
    }
    else
    {
        p_db_backup_min_priority = _if_active_backup_agg_get_backup_running_min_priority_port(p_db_lag);
        if(NULL == p_db_backup_min_priority)
        {
            p_db_if_mem->static_agg_priority = add_memeber_priority_db;
            tbl_interface_set_interface_field(p_db_if_mem, TBL_INTERFACE_FLD_STATIC_AGG_PRIORITY);
            /*sync max priority*/
            p_db_lag->lag->cur_active_max_priority = _if_active_backup_agg_get_active_max_priority_num(p_db_lag);
            ds_lag_set_lag_field(p_db_lag, p_db_lag->lag, DS_LAG_FLD_CUR_ACTIVE_MAX_PRIORITY);
        }
        else
        {
            if(add_memeber_priority_db < p_db_backup_min_priority->static_agg_priority)
            {
                p_db_if_mem->static_agg_priority = add_memeber_priority_db;
                tbl_interface_set_interface_field(p_db_if_mem, TBL_INTERFACE_FLD_STATIC_AGG_PRIORITY);
                /*sync max priority*/
                p_db_lag->lag->cur_active_max_priority = _if_active_backup_agg_get_active_max_priority_num(p_db_lag);
                ds_lag_set_lag_field(p_db_lag, p_db_lag->lag, DS_LAG_FLD_CUR_ACTIVE_MAX_PRIORITY);
            }
            else
            {
                /*sync mem port db*/
                p_db_if_mem->static_agg_is_active = 0;
                tbl_interface_set_interface_field(p_db_if_mem, TBL_INTERFACE_FLD_STATIC_AGG_IS_ACTIVE);
                p_db_if_mem->static_agg_priority = add_memeber_priority_db;
                tbl_interface_set_interface_field(p_db_if_mem, TBL_INTERFACE_FLD_STATIC_AGG_PRIORITY);
                /* add by wulx for bug 52152, 2019-08-01, add log */
                IFNAME_ETH2SHORT(p_db_if_mem->key.name, mem_ifname_ext);
                logid_sys(LOG_LAG_4_CHANGE_INACTIVE, p_db_lag->key.name, mem_ifname_ext);

                /*sync backup port db*/
                p_db_backup_min_priority->static_agg_is_active = 1;
                tbl_interface_set_interface_field(p_db_backup_min_priority, TBL_INTERFACE_FLD_STATIC_AGG_IS_ACTIVE);
                /* add by wulx for bug 52152, 2019-08-01, add log */
                IFNAME_ETH2SHORT(p_db_backup_min_priority->key.name, mem_ifname_ext);
                logid_sys(LOG_LAG_4_CHANGE_ACTIVE, p_db_lag->key.name, mem_ifname_ext);

                /*sync max priority*/
                p_db_lag->lag->cur_active_max_priority = _if_active_backup_agg_get_active_max_priority_num(p_db_lag);
                ds_lag_set_lag_field(p_db_lag, p_db_lag->lag, DS_LAG_FLD_CUR_ACTIVE_MAX_PRIORITY);
            }
        }
    }

    if(p_db_lag)
    {
        if_agg_aggregator_bw_update(p_db_lag);
    }
    return 0;
}

int32 _if_active_backup_agg_oper(tbl_interface_t *p_db_if_lag, tbl_interface_t *p_db_if_mem, if_agg_active_backup_oper_t op, uint32 agg_member_priority, char *agg_name)
{
    if(IF_AGG_ACTIVE_BACKUP_OP_ADD_MEMBER == op)
    {
        _if_active_backup_agg_oper_add_mem(p_db_if_lag, p_db_if_mem, agg_member_priority);
    }
    else if(IF_AGG_ACTIVE_BACKUP_OP_DEL_MEMBER == op)
    {
        _if_active_backup_agg_oper_del_mem(p_db_if_lag, p_db_if_mem, agg_name);
    }
    else if(IF_AGG_ACTIVE_BACKUP_OP_CHANGE_MAX_NUMBER == op)
    {
         /*for this operation ,agg_member_priority is used for max_active_number*/
        _if_active_backup_agg_oper_change_num(p_db_if_lag, agg_member_priority);
    }
    else if(IF_AGG_ACTIVE_BACKUP_OP_MEMBER_LINK_DOWN == op)
    {
        _if_active_backup_agg_oper_port_up_down(p_db_if_mem);
    }
    else if(IF_AGG_ACTIVE_BACKUP_OP_CHANGE_MEM_PRIORITY == op)
    {
        _if_active_backup_agg_oper_change_mem_priority(p_db_if_mem, agg_member_priority);
    }
    return 0;
}

int32
if_agg_add_agg(ds_lag_t *p_lag, cfg_cmd_para_t *para, tbl_interface_t *p_db_if, uint32 agg_member_priority)
{
    tbl_interface_key_t if_key;
    tbl_interface_t *p_db_if_lag = NULL;
    uint32 first_add = TRUE;
    uint32 port_num = 0;
    int32 rc = PM_E_NONE;
#ifdef OFPRODUCT
    tbl_openflow_interface_key_t openflow_if_key;
    tbl_openflow_interface_t* p_openflow_if = NULL;
#endif
    sal_memset(&if_key, 0, sizeof(if_key));
    sal_snprintf(if_key.name, IFNAME_SIZE, "agg%u", p_lag->lag_id);

    /* lookup whether the first add */
    p_db_if_lag = tbl_interface_get_interface(&if_key);

    /* add member check */
    rc = _if_agg_add_member_check(p_db_if_lag, p_db_if, para);
    if (rc < 0)
    {
        return rc;
    }

#ifdef TAPPRODUCT
    extern int32 tap_cmd_process_interface_pmap(char *pmap, bool isadd, tbl_interface_t *p_db_if);
    tap_cmd_process_interface_pmap("", FALSE, p_db_if);
#endif

    /* do add logic */    
    if (NULL == p_db_if_lag)
    {
        /*init max_active_number*/
        p_lag->max_active_number = GLB_LINKAGG_MAX_MEMBER;
        PM_E_RETURN(if_mgr_add_lag_if(if_key.name, p_db_if, p_lag));
        ctc_sock_schedule();
        p_db_if_lag = tbl_interface_get_interface(&if_key);
        if (NULL == p_db_if_lag)
        {
            return PM_E_NO_MEMORY;
        }
        PM_E_RETURN(_if_agg_sync_db_when_add(p_db_if_lag, p_db_if));
        first_add = TRUE;

        /* sync first member port to LAG port */
        PM_E_RETURN(_if_agg_sync_hal_when_add(p_db_if_lag, p_db_if));
    }
    else
    {
        first_add = FALSE;
        if (p_db_if_lag->lag->mode != p_lag->mode)
        {
            if (GLB_AGG_MODE_STATIC == p_db_if_lag->lag->mode)
            {
                CFG_CONFLICT_RET("Interface %s LAG mode is static", p_db_if_lag->key.name);
            }
            else if ( GLB_AGG_MODE_DYNAMIC_ACTIVE == p_db_if_lag->lag->mode 
                      || GLB_AGG_MODE_DYNAMIC_PASSIVE == p_db_if_lag->lag->mode)
            {
                /* Modified by kcao for bug 41921, not check dynamic active type */
                if (GLB_AGG_MODE_STATIC == p_lag->mode)
                {
                    CFG_CONFLICT_RET("Interface %s LAG mode is dynamic", p_db_if_lag->key.name);
                }
            }
            else
            {
                CFG_CONFLICT_RET("Interface %s LAG mode is invalid", p_db_if_lag->key.name);
            }
        }

        port_num = CTCLIB_SLISTCOUNT(p_db_if_lag->lag->member_ports.obj_list);
        if (port_num >= GLB_LINKAGG_MAX_MEMBER)
        {
            CFG_CONFLICT_RET("Interface %s reaches max %u member ports", p_db_if_lag->key.name, GLB_LINKAGG_MAX_MEMBER);
        }
    }

    swth_if_add_to_agg_cb(p_db_if);

    if (!first_add)
    {
        /* check whether admin key is matched */
        PM_E_RETURN(if_mgr_lag_admin_key_check(p_db_if_lag, p_db_if, para));

        if (GLB_IF_MODE_L3 == p_db_if->mode)
        {
            /*added by yejl to fix bug44830, sync arp number limit*/
            PM_E_RETURN(_if_agg_sync_arpnumlimit_when_add(p_db_if_lag, p_db_if, FALSE));
        }
    }
    PM_E_RETURN(if_mgr_add_if_to_lag(p_db_if_lag, p_db_if, p_lag));
   
#if 0
    if(first_add)
    {
        /* sync first member port to LAG port */
        PM_E_RETURN(_if_agg_sync_db_when_add(p_db_if_lag, p_db_if));

        /* sync first member port to LAG port */
        PM_E_RETURN(_if_agg_sync_hal_when_add(p_db_if_lag, p_db_if));
    }
#endif

    if (!first_add)
    {
        /* sync LAG port to other member port */
        PM_E_RETURN(_if_agg_add_member_sync_prop(p_db_if_lag, p_db_if));
    }
    else
    {
        PM_E_RETURN(_if_agg_sync_vlan_when_add(p_db_if_lag,p_db_if));
    }
    
    if (GLB_IF_MODE_L2 == p_db_if->mode)
    {
        PM_E_RETURN(swth_mstp_api_del_port(p_db_if));
    }

#ifdef TAPPRODUCT
    if (first_add)
    {
        tap_cmd_process_interface_pmap(p_db_if_lag->key.name, TRUE, p_db_if_lag);
    }
#endif

#ifdef OFPRODUCT
    sal_strncpy(openflow_if_key.name, if_key.name, IFNAME_SIZE);
    p_openflow_if = tbl_openflow_interface_get_openflow_interface(&openflow_if_key);
    if(p_openflow_if)
    {
        if(p_openflow_if->openflow_enable)
        {
            p_db_if_lag->openflow_operation_agg_mem_ifindex = p_db_if->ifindex;
            tbl_interface_set_interface_field(p_db_if_lag, TBL_INTERFACE_FLD_OPENFLOW_OPERATION_AGG_MEM_IFINDEX);
            p_db_if_lag ->openflow_operation_agg = 1;
            tbl_interface_set_interface_field(p_db_if_lag, TBL_INTERFACE_FLD_OPENFLOW_OPERATION_AGG);
        }
    }
#endif

    if (first_add)
    {
        acl_api_agg_set_redirect_cb(p_db_if_lag);
        sal_task_sleep(1000);
    }

    PM_E_RETURN(_if_active_backup_agg_oper(p_db_if_lag, p_db_if, IF_AGG_ACTIVE_BACKUP_OP_ADD_MEMBER, agg_member_priority, NULL));
    return PM_E_NONE;
}

int32
if_agg_del_agg(tbl_interface_t *p_db_if)
{
    tbl_interface_key_t if_key;
    tbl_interface_t     *p_db_if_lag = NULL;
    ds_lag_t            *p_db_lag = NULL;
    tbl_interface_t     if_lag;
    int32 rc = PM_E_NONE;
#ifdef OFPRODUCT
    tbl_openflow_interface_key_t openflow_if_key;
    tbl_openflow_interface_t* p_openflow_if = NULL;
#endif
    char ifname[IFNAME_SIZE] = {0};
    
    sal_memset(&if_lag, 0, sizeof(if_lag));
    sal_memset(&if_key, 0, sizeof(if_key));
    sal_snprintf(if_key.name, IFNAME_SIZE, "agg%u", p_db_if->lag->lag_id);
    p_db_if_lag = tbl_interface_get_interface(&if_key);
    if (NULL == p_db_if_lag)
    {
        SAL_ASSERT(0);
    }
    
    PM_E_RETURN(_if_agg_del_member_sync_prop(p_db_if_lag, p_db_if));
    PM_E_RETURN(if_mgr_del_if_from_lag(p_db_if_lag, p_db_if));

#ifdef OFPRODUCT
    sal_strncpy(openflow_if_key.name, if_key.name, IFNAME_SIZE);
    p_openflow_if = tbl_openflow_interface_get_openflow_interface(&openflow_if_key);
    if(p_openflow_if)
    {
        if(p_openflow_if->openflow_enable)
        {
            p_db_if_lag->openflow_operation_agg_mem_ifindex = p_db_if->ifindex;
            tbl_interface_set_interface_field(p_db_if_lag, TBL_INTERFACE_FLD_OPENFLOW_OPERATION_AGG_MEM_IFINDEX);
            p_db_if_lag->openflow_operation_agg = 0;
            tbl_interface_set_interface_field(p_db_if_lag, TBL_INTERFACE_FLD_OPENFLOW_OPERATION_AGG);            
        }
    }
#endif

    /* check member port is empty */
    p_db_lag = p_db_if_lag->lag;
    if (CDB_REF_LIST_ISEMPTY(&p_db_lag->member_ports))
    {
        sal_memcpy(&if_lag, p_db_if_lag, sizeof(if_lag));
        rc = ipsour_if_delete_cb2(&if_lag);
#ifdef CONFIG_L2
        /* kcao for bug 29726, member port has moved from LAG
         * so flush FDB in before remove LAG does no matter */
        PM_E_RETURN(brg_fdb_flush_l2mc_fdb_port(p_db_if_lag->ifindex));
        PM_E_RETURN(brg_fdb_flush_static_fdb_port(p_db_if_lag->ifindex));
        PM_E_RETURN(brg_psfdb_flush_static_fdb_port(p_db_if_lag->ifindex));
        PM_E_RETURN(brg_fdb_flush_dynamic_fdb_port(p_db_if_lag->ifindex));
#endif /* CONFIG_L2 */

        /*added by yejl for bug 54170, 2019-10-30*/
        PM_E_RETURN(_if_agg_sync_when_detach_acl_and_policy_map(p_db_if_lag));

#ifdef TSINGMA
        if (p_db_if_lag->hash_value_profile_id != 0 || 
            p_db_if_lag->hash_value_profile_id_agg != 0)
        {
            if (p_db_if_lag->hash_value_profile_id && p_db_if_lag->hash_value_profile_id_agg)
            {
                PM_E_RETURN(hash_value_profile_unlock(p_db_if_lag->hash_value_profile_id));
                PM_E_RETURN(hash_value_profile_unlock(p_db_if_lag->hash_value_profile_id_agg));
            }
            else if (p_db_if_lag->hash_value_profile_id)
            {
                PM_E_RETURN(hash_value_profile_unlock(p_db_if_lag->hash_value_profile_id));
            }
            else if (p_db_if_lag->hash_value_profile_id_agg)
            {
                PM_E_RETURN(hash_value_profile_unlock(p_db_if_lag->hash_value_profile_id_agg));
            }   
            p_db_if_lag->hash_value_profile_id = 0;
            p_db_if_lag->hash_value_profile_id_agg = 0;
            p_db_if_lag->hash_value_profile_dir = GLB_INGRESS;
            p_db_if_lag->hash_value_agg_hash = 0;
            PM_E_RETURN(tbl_interface_set_interface_field(p_db_if_lag, TBL_INTERFACE_FLD_HASH_VALUE_PROFILE_DIR));
            PM_E_RETURN(tbl_interface_set_interface_field(p_db_if_lag, TBL_INTERFACE_FLD_HASH_VALUE_PROFILE_ID));
            PM_E_RETURN(tbl_interface_set_interface_field(p_db_if_lag, TBL_INTERFACE_FLD_HASH_VALUE_PROFILE_ID_AGG));
            PM_E_RETURN(tbl_interface_set_interface_field(p_db_if_lag, TBL_INTERFACE_FLD_HASH_VALUE_AGG_HASH));
        }
#endif

        PM_E_RETURN(_if_agg_destory_agg_sync_prop(p_db_if_lag));            
        PM_E_RETURN(_if_agg_sync_hal_when_del(p_db_if_lag));
        sal_strcpy(ifname, p_db_if_lag->key.name);
        PM_E_RETURN(if_mgr_del_lag_if(p_db_if_lag, p_db_if));
        p_db_if_lag = NULL;
    }
    else
    {
        _if_agg_del_agg_update_mac_addr(p_db_if_lag, p_db_lag, p_db_if);
    }

    if (GLB_IF_MODE_L2 == p_db_if->mode)
    {
        PM_E_RETURN(swth_mstp_api_add_port(p_db_if));
    }

    PM_E_RETURN(_if_active_backup_agg_oper(p_db_if_lag, p_db_if, IF_AGG_ACTIVE_BACKUP_OP_DEL_MEMBER, 0, ifname));
    return PM_E_NONE;
}

int32
if_agg_aggregator_bw_update(tbl_interface_t *p_db_if_agg)
{
    ds_lag_t *p_lag = p_db_if_agg->lag;
    ds_lag_t *p_lag_mem = NULL;
    uint32 bandwidth = 0;
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_if_mem = NULL;
    uint32 first_oper_mem = TRUE;
    int32 rc = PM_E_NONE;

    /* no configured bandwidth, update oper bandwidth */
    CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
    {
        p_lag_mem = p_if_mem->lag;
        if ((!p_lag_mem->oper_state) || (!p_if_mem->static_agg_is_active))
        {
            continue;
        }
        bandwidth += p_if_mem->oper_bandwidth;

        /* Added by kcao for bug 50918, sync from first oper member port */
        if (TRUE == first_oper_mem)
        {
            first_oper_mem = FALSE;
            p_db_if_agg->oper_speed = p_if_mem->oper_speed;
            p_db_if_agg->oper_duplex = p_if_mem->oper_duplex;
            PM_E_RETURN(tbl_interface_set_interface_field(p_db_if_agg, TBL_INTERFACE_FLD_OPER_SPEED));
            PM_E_RETURN(tbl_interface_set_interface_field(p_db_if_agg, TBL_INTERFACE_FLD_OPER_DUPLEX));
        }
    }

    /* configured bandwidth */
    if (p_db_if_agg->bandwidth)
    {
        return PM_E_NONE;
    }

    /* same to E580, down agg uses 1000Mbps */
    if (0 == bandwidth)
    {
        bandwidth = 1000000;    /* 1000Mbps */
    }
    p_db_if_agg->oper_bandwidth = bandwidth;
    PM_E_RETURN(tbl_interface_set_interface_field(p_db_if_agg, TBL_INTERFACE_FLD_OPER_BANDWIDTH));

    if(p_db_if_agg->bandwidth_may_change)
    {
        p_db_if_agg->bandwidth_may_change = 0;
    }
    else
    {
        p_db_if_agg->bandwidth_may_change = 1;
    }
    PM_E_RETURN(tbl_interface_set_interface_field(p_db_if_agg, TBL_INTERFACE_FLD_BANDWIDTH_MAY_CHANGE));

    swth_mstp_api_update_if_path_cost(p_db_if_agg);

    return PM_E_NONE;
}

/* if lag is down, should check whether has mismatch member port */
int32
if_agg_if_running_update_mismatch(tbl_interface_t *p_db_if_lag)
{
    ds_lag_t *p_lag_group = p_db_if_lag->lag;
    ctclib_slistnode_t  *listnode = NULL;
    tbl_interface_t     *p_if_mem = NULL;
    uint32 new_running = FALSE;

    /* no configured bandwidth, update oper bandwidth */
    CTCLIB_SLIST_LOOP(p_lag_group->member_ports.obj_list, p_if_mem, listnode)
    {
        if (p_if_mem->lag->mismatch)
        {
            if_agg_attach_mux_to_aggregator(p_db_if_lag, p_if_mem);
        }
    }

    if (p_lag_group->bundle_ports_count > 0)
    {
        CTCLIB_SLIST_LOOP(p_lag_group->member_ports.obj_list, p_if_mem, listnode)
        {
            if (p_if_mem->lag->oper_state)
            {
                if (p_db_if_lag->oper_duplex != p_if_mem->oper_duplex)
                {
                    p_db_if_lag->oper_duplex = p_if_mem->oper_duplex;
                    tbl_interface_set_interface_field(p_db_if_lag, TBL_INTERFACE_FLD_OPER_DUPLEX);
                }
                
                if (p_db_if_lag->oper_speed != p_if_mem->oper_speed)
                {
                    p_db_if_lag->oper_speed = p_if_mem->oper_speed;
                    tbl_interface_set_interface_field(p_db_if_lag, TBL_INTERFACE_FLD_OPER_SPEED);
                }
                break;
            }
        }
        new_running = TRUE;
    }
    /* no active member */
    return new_running; 
}

int32
if_agg_check_oper_speed(tbl_interface_t *p_db_if_agg, tbl_interface_t *p_db_if_mem)
{
    ds_lag_t *p_lag_group = p_db_if_agg->lag;
    ctclib_slistnode_t  *listnode = NULL;
    tbl_interface_t     *p_if_mem = NULL;

    /* no configured bandwidth, update oper bandwidth */
    CTCLIB_SLIST_LOOP(p_lag_group->member_ports.obj_list, p_if_mem, listnode)
    {
        if (p_if_mem == p_db_if_mem)
        {
            continue;
        }
        if (p_if_mem->lag->oper_state)
        {
            if (p_if_mem->oper_speed == p_db_if_mem->oper_speed)
            {
                p_db_if_mem->lag->mismatch = FALSE;
                return TRUE;
            }
            else
            {
                p_db_if_mem->lag->mismatch = TRUE;
                return FALSE;
            }
        }
    }

    /* no active member */
    p_db_if_mem->lag->mismatch = FALSE;
    return TRUE;    
}

int32
if_agg_attach_mux_to_aggregator(tbl_interface_t *p_db_if_agg, tbl_interface_t *p_db_if_mem)
{
    char mem_ifname_ext[IFNAME_SIZE];
    ds_lag_t *p_lag = p_db_if_mem->lag;
    uint32 oper_speed_match = FALSE;
    int32 rc = PM_E_NONE;
    
    if (tbl_interface_is_running(p_db_if_mem) 
        && (p_db_if_agg->speed == p_db_if_mem->speed)
        && (p_db_if_agg->duplex == p_db_if_mem->duplex))
    {
        if (GLB_SPEED_AUTO == p_db_if_agg->speed)
        {
            oper_speed_match = if_agg_check_oper_speed(p_db_if_agg, p_db_if_mem);
        }
        else
        {
            oper_speed_match = TRUE;
        }

        if (oper_speed_match)
        {
            p_lag->oper_state = TRUE;
            p_db_if_agg->lag->bundle_ports_count++;
            PM_E_RETURN(ds_lag_set_lag_field(p_db_if_mem, p_lag, DS_LAG_FLD_OPER_STATE));
            if_mgr_lag_if_running_update(p_db_if_agg);

            IFNAME_ETH2SHORT(p_db_if_mem->key.name, mem_ifname_ext);
            logid_sys(LOG_LAG_4_ATTACH_MEMBER, p_db_if_agg->key.name, mem_ifname_ext);
        }
    }

    PM_E_RETURN(if_agg_aggregator_bw_update(p_db_if_agg));
    
    //PM_E_RETURN(_if_agg_attach_member_sync_prop(p_db_if_agg, p_db_if_mem));
    
    return PM_E_NONE;
}

int32
if_agg_detach_mux_from_aggregator(tbl_interface_t *p_db_if_agg, tbl_interface_t *p_db_if_mem)
{
    char mem_ifname_ext[IFNAME_SIZE];
    ds_lag_t *p_lag = p_db_if_mem->lag;
    int32 rc = PM_E_NONE;
    
    if (p_lag->oper_state)
    {
        p_lag->oper_state = FALSE;
        p_db_if_agg->lag->bundle_ports_count--;
        PM_E_RETURN(ds_lag_set_lag_field(p_db_if_mem, p_lag, DS_LAG_FLD_OPER_STATE));
        if_mgr_lag_if_running_update(p_db_if_agg);
        
        IFNAME_ETH2SHORT(p_db_if_mem->key.name, mem_ifname_ext);
        logid_sys(LOG_LAG_4_DETACH_MEMBER, p_db_if_agg->key.name, mem_ifname_ext);
    }

    PM_E_RETURN(if_agg_aggregator_bw_update(p_db_if_agg));
    
    //PM_E_RETURN(_if_agg_detach_member_sync_prop(p_db_if_agg, p_db_if_mem));

    return PM_E_NONE;
}

int32
if_agg_static_agg_mux_aggregator_update(tbl_interface_t *p_db_if_mem)
{
    char mem_ifname_ext[IFNAME_SIZE];
    tbl_interface_key_t if_key;
    tbl_interface_t *p_db_if_agg = NULL;
    ds_lag_t *p_lag_mem = p_db_if_mem->lag;
    uint32 oper_speed_match = 0;
    int32 rc = PM_E_NONE;
    
    if (NULL == p_lag_mem)
    {
        return PM_E_NONE;
    }

    if (GLB_IF_TYPE_PORT_IF != p_db_if_mem->hw_type)
    {
        return PM_E_NONE;
    }

    sal_memset(&if_key, 0, sizeof(if_key));
    sal_snprintf(if_key.name, IFNAME_SIZE, GLB_IFNAME_AGG_PREFIX"%u", p_lag_mem->lag_id);

    p_db_if_agg = tbl_interface_get_interface(&if_key);
    if (NULL == p_db_if_agg)
    {
        return PM_E_FAIL;
    }

    if (!p_lag_mem->oper_state)
    {
        if (tbl_interface_is_running(p_db_if_mem) 
            && (p_db_if_agg->speed == p_db_if_mem->speed)
            && (p_db_if_agg->duplex == p_db_if_mem->duplex))
        {        
            if (GLB_SPEED_AUTO == p_db_if_agg->speed)
            {
                oper_speed_match = if_agg_check_oper_speed(p_db_if_agg, p_db_if_mem);
            }
            else
            {
                oper_speed_match = TRUE;
            }

            if (oper_speed_match)
            {
                p_lag_mem->oper_state = TRUE;
                PM_E_RETURN(ds_lag_set_lag_field(p_db_if_mem, p_lag_mem, DS_LAG_FLD_OPER_STATE));
                p_db_if_agg->lag->bundle_ports_count++;

                IFNAME_ETH2SHORT(p_db_if_mem->key.name, mem_ifname_ext);
                logid_sys(LOG_LAG_4_ATTACH_MEMBER, p_db_if_agg->key.name, mem_ifname_ext);
            }
        }
    }
    else
    {
        if (!tbl_interface_is_running(p_db_if_mem))
        {
            p_lag_mem->oper_state = FALSE;
            PM_E_RETURN(ds_lag_set_lag_field(p_db_if_mem, p_lag_mem, DS_LAG_FLD_OPER_STATE));
            p_db_if_agg->lag->bundle_ports_count--;
            
            IFNAME_ETH2SHORT(p_db_if_mem->key.name, mem_ifname_ext);
            logid_sys(LOG_LAG_4_DETACH_MEMBER, p_db_if_agg->key.name, mem_ifname_ext);
        }
    }
    
    PM_E_RETURN(if_agg_aggregator_bw_update(p_db_if_agg));

    return PM_E_NONE;
}

int32
if_agg_create_agg_sync_prop_set_cb(if_agg_cb_module_id_t module_id, AGG_CB_NOTIFY_FUNC func)
{
    if ((IF_AGG_CB_MODULE_MAX <= module_id) || (NULL == func))
    {
        return PM_E_FAIL;
    }
    g_agg_cb_notify_array[IF_AGG_CB_NOTIFY_CREATE_AGGREGATOR][module_id] = func;
    
    return PM_E_NONE;
}

int32
if_agg_destory_agg_sync_prop_set_cb(if_agg_cb_module_id_t module_id, AGG_CB_NOTIFY_FUNC func)
{
    if ((IF_AGG_CB_MODULE_MAX <= module_id) || (NULL == func))
    {
        return PM_E_FAIL;
    }
    g_agg_cb_notify_array[IF_AGG_CB_NOTIFY_DESTROY_AGGREGATOR][module_id] = func;
    
    return PM_E_NONE;
}

int32
if_agg_add_member_sync_prop_set_cb(if_agg_cb_module_id_t module_id, AGG_CB_NOTIFY_FUNC func)
{
    if ((IF_AGG_CB_MODULE_MAX <= module_id) || (NULL == func))
    {
        return PM_E_FAIL;
    }
    g_agg_cb_notify_array[IF_AGG_CB_NOTIFY_ADD_MEMBER_SYNC][module_id] = func;
    
    return PM_E_NONE;
}

int32
if_agg_del_member_sync_prop_set_cb(if_agg_cb_module_id_t module_id, AGG_CB_NOTIFY_FUNC func)
{
    if ((IF_AGG_CB_MODULE_MAX <= module_id) || (NULL == func))
    {
        return PM_E_FAIL;
    }
    g_agg_cb_notify_array[IF_AGG_CB_NOTIFY_DEL_MEMBER_SYNC][module_id] = func;
    
    return PM_E_NONE;
}

int32
if_agg_attach_member_sync_prop_set_cb(if_agg_cb_module_id_t module_id, AGG_CB_NOTIFY_FUNC func)
{
    if ((IF_AGG_CB_MODULE_MAX <= module_id) || (NULL == func))
    {
        return PM_E_FAIL;
    }
    g_agg_cb_notify_array[IF_AGG_CB_NOTIFY_ATTACH_MEMBER_SYNC][module_id] = func;
    
    return PM_E_NONE;
}

int32
if_agg_detach_member_sync_prop_set_cb(if_agg_cb_module_id_t module_id, AGG_CB_NOTIFY_FUNC func)
{
    if ((IF_AGG_CB_MODULE_MAX <= module_id) || (NULL == func))
    {
        return PM_E_FAIL;
    }
    g_agg_cb_notify_array[IF_AGG_CB_NOTIFY_DETACH_MEMBER_SYNC][module_id] = func;
    
    return PM_E_NONE;
}

int32
if_agg_add_member_check_set_cb(if_agg_cb_module_id_t module_id, AGG_CB_NOTIFY_FUNC func)
{
    if ((IF_AGG_CB_MODULE_MAX <= module_id) || (NULL == func))
    {
        return PM_E_FAIL;
    }
    g_agg_cb_notify_array[IF_AGG_CB_NOTIFY_ADD_MEMBER_CHECK][module_id] = func;
    
    return PM_E_NONE;
}

