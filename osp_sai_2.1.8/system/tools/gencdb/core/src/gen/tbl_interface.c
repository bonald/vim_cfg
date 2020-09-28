
#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/ds_circuit_id_define.h"
#include "gen/ds_circuit_id.h"
#include "gen/ds_l2protocol_define.h"
#include "gen/ds_l2protocol.h"
#include "cdb_data_cmp.h"

tbl_interface_master_t *_g_p_tbl_interface_master = NULL;

static uint32
_tbl_interface_hash_make(tbl_interface_t *p_if)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_if->key;
    for (index = 0; index < sizeof(p_if->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_interface_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_interface_t *p_if1 = (tbl_interface_t*)p_arg1;
    tbl_interface_t *p_if2 = (tbl_interface_t*)p_arg2;

    if (0 == sal_memcmp(&p_if1->key, &p_if2->key, sizeof(tbl_interface_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_interface_add_interface_sync(tbl_interface_t *p_if, uint32 sync)
{
    tbl_interface_master_t *p_master = _g_p_tbl_interface_master;
    tbl_interface_t *p_db_if = NULL;
    void *p_ret = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    if (tbl_interface_get_interface(&p_if->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_if = XCALLOC(MEM_TBL_INTERFACE, sizeof(tbl_interface_t));
    if (NULL == p_db_if)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_if, p_if, sizeof(tbl_interface_t));
    PM_E_RETURN(cdb_reference_list_init(&p_db_if->circuit_id, CDB_NODE_DS, DS_CIRCUIT_ID, ds_circuit_id_cmp));
    PM_E_RETURN(cdb_reference_list_init(&p_db_if->l2protocol, CDB_NODE_DS, DS_L2PROTOCOL, ds_l2protocol_cmp));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->if_hash, (void*)p_db_if, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->if_list, p_db_if);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_INTERFACE, p_db_if);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_INTERFACE, p_db_if);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_interface_del_interface_sync(tbl_interface_key_t *p_if_key, uint32 sync)
{
    tbl_interface_master_t *p_master = _g_p_tbl_interface_master;
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_if = tbl_interface_get_interface(p_if_key);
    if (NULL == p_db_if)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_INTERFACE, p_db_if);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_INTERFACE, p_db_if);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->if_hash, (void*)p_db_if);
    ctclib_slistnode_delete(p_master->if_list, p_db_if);
    PM_E_RETURN(cdb_reference_list_deinit(&p_db_if->circuit_id));
    PM_E_RETURN(cdb_reference_list_deinit(&p_db_if->l2protocol));

    /* 4. free db entry */
    XFREE(MEM_TBL_INTERFACE, p_db_if);

    return PM_E_NONE;
}

int32
tbl_interface_set_interface_field_sync(tbl_interface_t *p_if, tbl_interface_field_id_t field_id, uint32 sync)
{
    tbl_interface_t *p_db_if = NULL;

    /* 1. lookup entry exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_INTERFACE_FLD_IFINDEX:
        p_db_if->ifindex = p_if->ifindex;
        break;

    case TBL_INTERFACE_FLD_PORTID:
        p_db_if->portid = p_if->portid;
        break;

    case TBL_INTERFACE_FLD_HOSTIFID:
        p_db_if->hostifid = p_if->hostifid;
        break;

    case TBL_INTERFACE_FLD_DESC:
        sal_memcpy(p_db_if->desc, p_if->desc, sizeof(p_if->desc));
        break;

    case TBL_INTERFACE_FLD_SPEED:
        p_db_if->speed = p_if->speed;
        break;

    case TBL_INTERFACE_FLD_DUPLEX:
        p_db_if->duplex = p_if->duplex;
        break;

    case TBL_INTERFACE_FLD_UNIDIR:
        p_db_if->unidir = p_if->unidir;
        break;

    case TBL_INTERFACE_FLD_FLOWCONTROL_SEND:
        CDB_FLAG_CHECK_SET(p_db_if, p_if, flowcontrol, GLB_IF_FLOW_CONTROL_SEND)
        break;

    case TBL_INTERFACE_FLD_FLOWCONTROL_RECEIVE:
        CDB_FLAG_CHECK_SET(p_db_if, p_if, flowcontrol, GLB_IF_FLOW_CONTROL_RECEIVE)
        break;

    case TBL_INTERFACE_FLD_FLOWCONTROL_OPER_SEND:
        CDB_FLAG_CHECK_SET(p_db_if, p_if, flowcontrol, GLB_IF_FLOW_OPER_CONTROL_SEND)
        break;

    case TBL_INTERFACE_FLD_FLOWCONTROL_OPER_RECEIVE:
        CDB_FLAG_CHECK_SET(p_db_if, p_if, flowcontrol, GLB_IF_FLOW_OPER_CONTROL_RECEIVE)
        break;

    case TBL_INTERFACE_FLD_FEC:
        p_db_if->fec = p_if->fec;
        break;

    case TBL_INTERFACE_FLD_PFC_ADMIN:
        p_db_if->pfc_admin = p_if->pfc_admin;
        break;

    case TBL_INTERFACE_FLD_PFC_OPER:
        p_db_if->pfc_oper = p_if->pfc_oper;
        break;

    case TBL_INTERFACE_FLD_PFC_PRIORITY:
        p_db_if->pfc_priority = p_if->pfc_priority;
        break;

    case TBL_INTERFACE_FLD_JUMBOFRAME_EN:
        p_db_if->jumboframe_en = p_if->jumboframe_en;
        break;

    case TBL_INTERFACE_FLD_ISOLATE_GROUP_ID:
        p_db_if->isolate_group_id = p_if->isolate_group_id;
        break;

    case TBL_INTERFACE_FLD_DHCP_SNOOPING_TRUST:
        p_db_if->dhcp_snooping_trust = p_if->dhcp_snooping_trust;
        break;

    case TBL_INTERFACE_FLD_PHY_STATUS:
        p_db_if->phy_status = p_if->phy_status;
        break;

    case TBL_INTERFACE_FLD_PHY_LOOPBACK_MODE:
        p_db_if->phy_loopback_mode = p_if->phy_loopback_mode;
        break;

    case TBL_INTERFACE_FLD_INTERNAL_LP_IFINDEX:
        p_db_if->internal_lp_ifindex = p_if->internal_lp_ifindex;
        break;

    case TBL_INTERFACE_FLD_PVLAN_TYPE:
        p_db_if->pvlan_type = p_if->pvlan_type;
        break;

    case TBL_INTERFACE_FLD_PRIVATE_VLAN:
        p_db_if->private_vlan = p_if->private_vlan;
        break;

    case TBL_INTERFACE_FLD_COMMUNITY_VLAN:
        p_db_if->community_vlan = p_if->community_vlan;
        break;

    case TBL_INTERFACE_FLD_NTP_DISABLE:
        p_db_if->ntp_disable = p_if->ntp_disable;
        break;

    case TBL_INTERFACE_FLD_NTP_BROADCAST_CLIENT:
        p_db_if->ntp_broadcast_client = p_if->ntp_broadcast_client;
        break;

    case TBL_INTERFACE_FLD_PVLAN_GROUP_ID:
        p_db_if->pvlan_group_id = p_if->pvlan_group_id;
        break;

    case TBL_INTERFACE_FLD_SUPPORT_SPEED:
        p_db_if->support_speed = p_if->support_speed;
        break;

    case TBL_INTERFACE_FLD_MODE:
        p_db_if->mode = p_if->mode;
        break;

    case TBL_INTERFACE_FLD_VLAN_TYPE:
        p_db_if->vlan_type = p_if->vlan_type;
        break;

    case TBL_INTERFACE_FLD_UP:
        CDB_FLAG_CHECK_SET(p_db_if, p_if, flags, GLB_IFF_UP)
        break;

    case TBL_INTERFACE_FLD_RUNNING:
        CDB_FLAG_CHECK_SET(p_db_if, p_if, flags, GLB_IFF_RUNNING)
        break;

    case TBL_INTERFACE_FLD_MTU:
        p_db_if->mtu = p_if->mtu;
        break;

    case TBL_INTERFACE_FLD_OPER_SPEED:
        p_db_if->oper_speed = p_if->oper_speed;
        break;

    case TBL_INTERFACE_FLD_PREV_OPER_SPEED:
        p_db_if->prev_oper_speed = p_if->prev_oper_speed;
        break;

    case TBL_INTERFACE_FLD_OPER_DUPLEX:
        p_db_if->oper_duplex = p_if->oper_duplex;
        break;

    case TBL_INTERFACE_FLD_OPER_UNIDIR:
        p_db_if->oper_unidir = p_if->oper_unidir;
        break;

    case TBL_INTERFACE_FLD_OPER_FEC:
        p_db_if->oper_fec = p_if->oper_fec;
        break;

    case TBL_INTERFACE_FLD_OPER_LOOPBACK:
        p_db_if->oper_loopback = p_if->oper_loopback;
        break;

    case TBL_INTERFACE_FLD_BANDWIDTH:
        p_db_if->bandwidth = p_if->bandwidth;
        break;

    case TBL_INTERFACE_FLD_TAP_USED:
        p_db_if->tap_used = p_if->tap_used;
        break;

    case TBL_INTERFACE_FLD_TAP_INGRESS_ENABLE:
        p_db_if->tap_ingress_enable = p_if->tap_ingress_enable;
        break;

    case TBL_INTERFACE_FLD_TAP:
        sal_memcpy(p_db_if->tap, p_if->tap, sizeof(p_if->tap));
        break;

    case TBL_INTERFACE_FLD_TAP_MARK_VLAN:
        p_db_if->tap_mark_vlan = p_if->tap_mark_vlan;
        break;

    case TBL_INTERFACE_FLD_MIRROR_ENABLE:
        p_db_if->mirror_enable = p_if->mirror_enable;
        break;

    case TBL_INTERFACE_FLD_MIRROR_IGS_ID:
        p_db_if->mirror_igs_id = p_if->mirror_igs_id;
        break;

    case TBL_INTERFACE_FLD_MIRROR_EGS_ID:
        p_db_if->mirror_egs_id = p_if->mirror_egs_id;
        break;

    case TBL_INTERFACE_FLD_MIRROR_DEST_ID:
        p_db_if->mirror_dest_id = p_if->mirror_dest_id;
        break;

    case TBL_INTERFACE_FLD_ADMIN_UP:
        p_db_if->admin_up = p_if->admin_up;
        break;

    case TBL_INTERFACE_FLD_IP_SOURCE_EN:
        p_db_if->ip_source_en = p_if->ip_source_en;
        break;

    case TBL_INTERFACE_FLD_IP_SOURCE_TYPE:
        p_db_if->ip_source_type = p_if->ip_source_type;
        break;

    case TBL_INTERFACE_FLD_IP_SOURCE_ITEM_NUMBER:
        p_db_if->ip_source_item_number = p_if->ip_source_item_number;
        break;

    case TBL_INTERFACE_FLD_UNICAST_RPF_EN:
        p_db_if->unicast_rpf_en = p_if->unicast_rpf_en;
        break;

    case TBL_INTERFACE_FLD_OPER_BANDWIDTH:
        p_db_if->oper_bandwidth = p_if->oper_bandwidth;
        break;

    case TBL_INTERFACE_FLD_BANDWIDTH_MAY_CHANGE:
        p_db_if->bandwidth_may_change = p_if->bandwidth_may_change;
        break;

    case TBL_INTERFACE_FLD_ERRDISABLE_REASON:
        p_db_if->errdisable_reason = p_if->errdisable_reason;
        break;

    case TBL_INTERFACE_FLD_FDB_LOOP_OCCUR:
        p_db_if->fdb_loop_occur = p_if->fdb_loop_occur;
        break;

    case TBL_INTERFACE_FLD_LINK_FLAP_CNT:
        p_db_if->link_flap_cnt = p_if->link_flap_cnt;
        break;

    case TBL_INTERFACE_FLD_STATIC_SECURITY_NUM:
        p_db_if->static_security_num = p_if->static_security_num;
        break;

    case TBL_INTERFACE_FLD_ERRDISABLE_TIMER:
        p_db_if->errdisable_timer = p_if->errdisable_timer;
        break;

    case TBL_INTERFACE_FLD_SFLOW_FLOW_EN:
        p_db_if->sflow_flow_en = p_if->sflow_flow_en;
        break;

    case TBL_INTERFACE_FLD_SFLOW_FLOW_INGRESS:
        p_db_if->sflow_flow_ingress = p_if->sflow_flow_ingress;
        break;

    case TBL_INTERFACE_FLD_SFLOW_FLOW_EGRESS:
        p_db_if->sflow_flow_egress = p_if->sflow_flow_egress;
        break;

    case TBL_INTERFACE_FLD_SFLOW_COUNTER_EN:
        p_db_if->sflow_counter_en = p_if->sflow_counter_en;
        break;

    case TBL_INTERFACE_FLD_SFLOW_RATE:
        p_db_if->sflow_rate = p_if->sflow_rate;
        break;

    case TBL_INTERFACE_FLD_SFLOW_SEQUENCE:
        p_db_if->sflow_sequence = p_if->sflow_sequence;
        break;

    case TBL_INTERFACE_FLD_SFLOW_SAMPLE_POOL:
        p_db_if->sflow_sample_pool = p_if->sflow_sample_pool;
        break;

    case TBL_INTERFACE_FLD_MAC_ADDR:
        sal_memcpy(p_db_if->mac_addr, p_if->mac_addr, sizeof(p_if->mac_addr));
        break;

    case TBL_INTERFACE_FLD_HW_MAC_ADDR:
        sal_memcpy(p_db_if->hw_mac_addr, p_if->hw_mac_addr, sizeof(p_if->hw_mac_addr));
        break;

    case TBL_INTERFACE_FLD_HW_TYPE:
        p_db_if->hw_type = p_if->hw_type;
        break;

    case TBL_INTERFACE_FLD_PHY_TYPE:
        p_db_if->phy_type = p_if->phy_type;
        break;

    case TBL_INTERFACE_FLD_MEDIA:
        p_db_if->media = p_if->media;
        break;

    case TBL_INTERFACE_FLD_PHYPORT_COMBO:
        CDB_FLAG_CHECK_SET(p_db_if, p_if, phyport_flag, GLB_PHYPORT_FLAG_COMBO)
        break;

    case TBL_INTERFACE_FLD_PHYPORT_LOOPBACK:
        CDB_FLAG_CHECK_SET(p_db_if, p_if, phyport_flag, GLB_PHYPORT_FLAG_LOOPBACK)
        break;

    case TBL_INTERFACE_FLD_PHYPORT_EEE:
        CDB_FLAG_CHECK_SET(p_db_if, p_if, phyport_flag, GLB_PHYPORT_FLAG_EEE)
        break;

    case TBL_INTERFACE_FLD_PHYPORT_POE:
        CDB_FLAG_CHECK_SET(p_db_if, p_if, phyport_flag, GLB_PHYPORT_FLAG_POE)
        break;

    case TBL_INTERFACE_FLD_PHYPORT_MASTER:
        CDB_FLAG_CHECK_SET(p_db_if, p_if, phyport_flag, GLB_PHYPORT_FLAG_MASTER)
        break;

    case TBL_INTERFACE_FLD_PHYPORT_SLAVE:
        CDB_FLAG_CHECK_SET(p_db_if, p_if, phyport_flag, GLB_PHYPORT_FLAG_SLAVE)
        break;

    case TBL_INTERFACE_FLD_PHYPORT_UNIDIR:
        CDB_FLAG_CHECK_SET(p_db_if, p_if, phyport_flag, GLB_PHYPORT_FLAG_UNIDIR)
        break;

    case TBL_INTERFACE_FLD_SUPPORT_100M:
        CDB_FLAG_CHECK_SET(p_db_if, p_if, support_speed_flag, GLB_SUPPORT_SPEED_100M)
        break;

    case TBL_INTERFACE_FLD_SUPPORT_1G:
        CDB_FLAG_CHECK_SET(p_db_if, p_if, support_speed_flag, GLB_SUPPORT_SPEED_1G)
        break;

    case TBL_INTERFACE_FLD_SUPPORT_10G:
        CDB_FLAG_CHECK_SET(p_db_if, p_if, support_speed_flag, GLB_SUPPORT_SPEED_10G)
        break;

    case TBL_INTERFACE_FLD_SUPPORT_40G:
        CDB_FLAG_CHECK_SET(p_db_if, p_if, support_speed_flag, GLB_SUPPORT_SPEED_40G)
        break;

    case TBL_INTERFACE_FLD_SUPPORT_100G:
        CDB_FLAG_CHECK_SET(p_db_if, p_if, support_speed_flag, GLB_SUPPORT_SPEED_100G)
        break;

    case TBL_INTERFACE_FLD_SUPPORT_2_5G:
        CDB_FLAG_CHECK_SET(p_db_if, p_if, support_speed_flag, GLB_SUPPORT_SPEED_2_5G)
        break;

    case TBL_INTERFACE_FLD_SUPPORT_5G:
        CDB_FLAG_CHECK_SET(p_db_if, p_if, support_speed_flag, GLB_SUPPORT_SPEED_5G)
        break;

    case TBL_INTERFACE_FLD_SUPPORT_25G:
        CDB_FLAG_CHECK_SET(p_db_if, p_if, support_speed_flag, GLB_SUPPORT_SPEED_25G)
        break;

    case TBL_INTERFACE_FLD_SUPPORT_50G:
        CDB_FLAG_CHECK_SET(p_db_if, p_if, support_speed_flag, GLB_SUPPORT_SPEED_50G)
        break;

    case TBL_INTERFACE_FLD_IGS_ACL:
        sal_memcpy(p_db_if->igs_acl, p_if->igs_acl, sizeof(p_if->igs_acl));
        break;

    case TBL_INTERFACE_FLD_EGS_ACL:
        sal_memcpy(p_db_if->egs_acl, p_if->egs_acl, sizeof(p_if->egs_acl));
        break;

    case TBL_INTERFACE_FLD_IGS_IPV6_ACL:
        sal_memcpy(p_db_if->igs_ipv6_acl, p_if->igs_ipv6_acl, sizeof(p_if->igs_ipv6_acl));
        break;

    case TBL_INTERFACE_FLD_EGS_IPV6_ACL:
        sal_memcpy(p_db_if->egs_ipv6_acl, p_if->egs_ipv6_acl, sizeof(p_if->egs_ipv6_acl));
        break;

    case TBL_INTERFACE_FLD_IGS_MAC_ACL:
        sal_memcpy(p_db_if->igs_mac_acl, p_if->igs_mac_acl, sizeof(p_if->igs_mac_acl));
        break;

    case TBL_INTERFACE_FLD_EGS_MAC_ACL:
        sal_memcpy(p_db_if->egs_mac_acl, p_if->egs_mac_acl, sizeof(p_if->egs_mac_acl));
        break;

    case TBL_INTERFACE_FLD_IGS_POLICY_MAP:
        sal_memcpy(p_db_if->igs_policy_map, p_if->igs_policy_map, sizeof(p_if->igs_policy_map));
        break;

    case TBL_INTERFACE_FLD_EGS_POLICY_MAP:
        sal_memcpy(p_db_if->egs_policy_map, p_if->egs_policy_map, sizeof(p_if->egs_policy_map));
        break;

    case TBL_INTERFACE_FLD_CRC_ERROR_CHECK_EN:
        p_db_if->crc_error_check_en = p_if->crc_error_check_en;
        break;

    case TBL_INTERFACE_FLD_CRC_RECALCULATION_EN:
        p_db_if->crc_recalculation_en = p_if->crc_recalculation_en;
        break;

    case TBL_INTERFACE_FLD_MLAG_ID:
        p_db_if->mlag_id = p_if->mlag_id;
        break;

    case TBL_INTERFACE_FLD_ERPS_ENABLE:
        p_db_if->erps_enable = p_if->erps_enable;
        break;

    case TBL_INTERFACE_FLD_ERPS_PDU_ACTION:
        p_db_if->erps_pdu_action = p_if->erps_pdu_action;
        break;

    case TBL_INTERFACE_FLD_DOT1X_ENABLE:
        p_db_if->dot1x_enable = p_if->dot1x_enable;
        break;

    case TBL_INTERFACE_FLD_DOT1X_UNAUTHORIZED:
        CDB_FLAG_CHECK_SET(p_db_if, p_if, dot1x_flag, GLB_IF_DOT1X_UNAUTHED)
        break;

    case TBL_INTERFACE_FLD_DOT1X_DIR_IN:
        CDB_FLAG_CHECK_SET(p_db_if, p_if, dot1x_flag, GLB_IF_DOT1X_DIR_IN)
        break;

    case TBL_INTERFACE_FLD_DOT1X_DIR_OUT:
        CDB_FLAG_CHECK_SET(p_db_if, p_if, dot1x_flag, GLB_IF_DOT1X_DIR_OUT)
        break;

    case TBL_INTERFACE_FLD_DOT1X_MODE_MAC:
        CDB_FLAG_CHECK_SET(p_db_if, p_if, dot1x_flag, GLB_IF_DOT1X_MODE_MAC)
        break;

    case TBL_INTERFACE_FLD_DOT1X_GUEST_VLAN:
        p_db_if->dot1x_guest_vlan = p_if->dot1x_guest_vlan;
        break;

    case TBL_INTERFACE_FLD_VLANCLASS_GROUP_ID:
        p_db_if->vlanclass_group_id = p_if->vlanclass_group_id;
        break;

    case TBL_INTERFACE_FLD_VLANCLASS_ACTIVE_TYPE:
        p_db_if->vlanclass_active_type = p_if->vlanclass_active_type;
        break;

    case TBL_INTERFACE_FLD_VLANCLASS_DEFAULT_ACTION:
        p_db_if->vlanclass_default_action = p_if->vlanclass_default_action;
        break;

    case TBL_INTERFACE_FLD_DOT1X_CFG_GUEST_VLAN:
        p_db_if->dot1x_cfg_guest_vlan = p_if->dot1x_cfg_guest_vlan;
        break;

    case TBL_INTERFACE_FLD_EGS_ACL_EN:
        p_db_if->egs_acl_en = p_if->egs_acl_en;
        break;

    case TBL_INTERFACE_FLD_TAP_GROUP_IGS_REF_CNT:
        p_db_if->tap_group_igs_ref_cnt = p_if->tap_group_igs_ref_cnt;
        break;

    case TBL_INTERFACE_FLD_TAP_GROUP_EGS_REF_CNT:
        p_db_if->tap_group_egs_ref_cnt = p_if->tap_group_egs_ref_cnt;
        break;

    case TBL_INTERFACE_FLD_TAP_FLOW_DECAP_CNT:
        p_db_if->tap_flow_decap_cnt = p_if->tap_flow_decap_cnt;
        break;

    case TBL_INTERFACE_FLD_TAP_FLOW_UDF_CNT:
        p_db_if->tap_flow_udf_cnt = p_if->tap_flow_udf_cnt;
        break;

    case TBL_INTERFACE_FLD_METADATA_ENABLE:
        p_db_if->metadata_enable = p_if->metadata_enable;
        break;

    case TBL_INTERFACE_FLD_ARPINSP_TRUST_ENABLE:
        p_db_if->arpinsp_trust_enable = p_if->arpinsp_trust_enable;
        break;

    case TBL_INTERFACE_FLD_ARP_NUMBERLIMIT_ENABLE:
        p_db_if->arp_numberlimit_enable = p_if->arp_numberlimit_enable;
        break;

    case TBL_INTERFACE_FLD_ARP_NUMBERLIMIT_NUMBER:
        p_db_if->arp_numberlimit_number = p_if->arp_numberlimit_number;
        break;

    case TBL_INTERFACE_FLD_ARP_NUMBERLIMIT_VIOLATE:
        p_db_if->arp_numberlimit_violate = p_if->arp_numberlimit_violate;
        break;

    case TBL_INTERFACE_FLD_ARP_NUMBERLIMIT_OCCUR:
        p_db_if->arp_numberlimit_occur = p_if->arp_numberlimit_occur;
        break;

    case TBL_INTERFACE_FLD_ARP_RATE_LIMIT_ERRDISABLE_OCCUR:
        p_db_if->arp_rate_limit_errdisable_occur = p_if->arp_rate_limit_errdisable_occur;
        break;

    case TBL_INTERFACE_FLD_INBAND_SNAT_TYPE:
        p_db_if->inband_snat_type = p_if->inband_snat_type;
        break;

    case TBL_INTERFACE_FLD_LATENCY_EVENT_HIGH:
        p_db_if->latency_event_high = p_if->latency_event_high;
        break;

    case TBL_INTERFACE_FLD_LATENCY_EVENT_LOW:
        p_db_if->latency_event_low = p_if->latency_event_low;
        break;

    case TBL_INTERFACE_FLD_LATENCY_EVENT_ENABLE:
        p_db_if->latency_event_enable = p_if->latency_event_enable;
        break;

    case TBL_INTERFACE_FLD_REFLECTIVE_BRIDGE_EN:
        p_db_if->reflective_bridge_en = p_if->reflective_bridge_en;
        break;

    case TBL_INTERFACE_FLD_SUPPORT_SSM_EN:
        p_db_if->support_ssm_en = p_if->support_ssm_en;
        break;

    case TBL_INTERFACE_FLD_SVLAN_TPID:
        p_db_if->svlan_tpid = p_if->svlan_tpid;
        break;

    case TBL_INTERFACE_FLD_L2PRO_IF_ACTION_VLAN:
        sal_memcpy(&p_db_if->l2pro_if_action_vlan, &p_if->l2pro_if_action_vlan, sizeof(p_if->l2pro_if_action_vlan));
        break;

    case TBL_INTERFACE_FLD_L2PRO_UPLINK:
        p_db_if->l2pro_uplink = p_if->l2pro_uplink;
        break;

    case TBL_INTERFACE_FLD_STATIC_AGG_PRIORITY:
        p_db_if->static_agg_priority = p_if->static_agg_priority;
        break;

    case TBL_INTERFACE_FLD_STATIC_AGG_IS_ACTIVE:
        p_db_if->static_agg_is_active = p_if->static_agg_is_active;
        break;

    case TBL_INTERFACE_FLD_FEC_STATUS:
        p_db_if->fec_status = p_if->fec_status;
        break;

    case TBL_INTERFACE_FLD_CL73_ABILITY:
        p_db_if->cl73_ability = p_if->cl73_ability;
        break;

    case TBL_INTERFACE_FLD_IFG_STATS_EN:
        p_db_if->ifg_stats_en = p_if->ifg_stats_en;
        break;

    case TBL_INTERFACE_FLD_OPENFLOW_EN:
        p_db_if->openflow_en = p_if->openflow_en;
        break;

    case TBL_INTERFACE_FLD_OPENFLOW_OPERATION_AGG:
        p_db_if->openflow_operation_agg = p_if->openflow_operation_agg;
        break;

    case TBL_INTERFACE_FLD_OPENFLOW_OPERATION_AGG_MEM_IFINDEX:
        p_db_if->openflow_operation_agg_mem_ifindex = p_if->openflow_operation_agg_mem_ifindex;
        break;

    case TBL_INTERFACE_FLD_CARRIER_DOWN_HOLDTIME_INTERVAL:
        p_db_if->carrier_down_holdtime_interval = p_if->carrier_down_holdtime_interval;
        break;

    case TBL_INTERFACE_FLD_HASH_VALUE_PROFILE_ID:
        p_db_if->hash_value_profile_id = p_if->hash_value_profile_id;
        break;

    case TBL_INTERFACE_FLD_HASH_VALUE_PROFILE_DIR:
        p_db_if->hash_value_profile_dir = p_if->hash_value_profile_dir;
        break;

    case TBL_INTERFACE_FLD_HASH_VALUE_PROFILE_ID_AGG:
        p_db_if->hash_value_profile_id_agg = p_if->hash_value_profile_id_agg;
        break;

    case TBL_INTERFACE_FLD_HASH_VALUE_AGG_HASH:
        p_db_if->hash_value_agg_hash = p_if->hash_value_agg_hash;
        break;

    case TBL_INTERFACE_FLD_POE:
        sal_memcpy(&p_db_if->poe, &p_if->poe, sizeof(p_if->poe));
        break;

    case TBL_INTERFACE_FLD_CURRENT_G8032_RING_COUNT:
        p_db_if->current_g8032_ring_count = p_if->current_g8032_ring_count;
        break;

    case TBL_INTERFACE_FLD_G8032_RING_FIRST:
        sal_memcpy(p_db_if->g8032_ring_first, p_if->g8032_ring_first, sizeof(p_if->g8032_ring_first));
        break;

    case TBL_INTERFACE_FLD_CFM_LMEP_NUM:
        p_db_if->cfm_lmep_num = p_if->cfm_lmep_num;
        break;

    case TBL_INTERFACE_FLD_CFM_RMEP_NUM:
        p_db_if->cfm_rmep_num = p_if->cfm_rmep_num;
        break;

    case TBL_INTERFACE_FLD_MAX:
        sal_memcpy(p_db_if, p_if, sizeof(tbl_interface_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_INTERFACE, field_id, p_db_if);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_INTERFACE, field_id, p_db_if);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_interface_add_interface(tbl_interface_t *p_if)
{
    return tbl_interface_add_interface_sync(p_if, TRUE);
}

int32
tbl_interface_del_interface(tbl_interface_key_t *p_if_key)
{
    return tbl_interface_del_interface_sync(p_if_key, TRUE);
}

int32
tbl_interface_set_interface_field(tbl_interface_t *p_if, tbl_interface_field_id_t field_id)
{
    return tbl_interface_set_interface_field_sync(p_if, field_id, TRUE);
}

tbl_interface_t*
tbl_interface_get_interface(tbl_interface_key_t *p_if_key)
{
    tbl_interface_master_t *p_master = _g_p_tbl_interface_master;
    tbl_interface_t lkp;

    sal_memcpy(&lkp.key, p_if_key, sizeof(tbl_interface_key_t));
    return ctclib_hash_lookup(p_master->if_hash, &lkp);
}

char*
tbl_interface_key_val2str(tbl_interface_t *p_if, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_if->key.name);
    return str;
}

int32
tbl_interface_key_str2val(char *str, tbl_interface_t *p_if)
{
    int32 ret = 0;

    sal_strcpy(p_if->key.name, str);
    return ret;
}

char*
tbl_interface_key_name_dump(tbl_interface_t *p_if, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_INTERFACE);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_if->key.name);
    return str;
}

char*
tbl_interface_key_value_dump(tbl_interface_t *p_if, char *str)
{
    sal_sprintf(str, "%s", p_if->key.name);
    return str;
}

char*
tbl_interface_field_name_dump(tbl_interface_t *p_if, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_INTERFACE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_INTERFACE_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_interface_field_value_dump(tbl_interface_t *p_if, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_INTERFACE_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_if->key.name);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_if->ifindex);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PORTID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_if->portid);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_HOSTIFID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_if->hostifid);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_DESC, field_id))
    {
        sal_sprintf(str, "%s", p_if->desc);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SPEED, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(glb_port_speed_strs, GLB_PORT_SPEED_MAX, p_if->speed));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_DUPLEX, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(glb_port_duplex_strs, GLB_PORT_DUPLEX_MAX, p_if->duplex));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_UNIDIR, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(glb_port_unidir_strs, GLB_PORT_UNIDIR_MAX, p_if->unidir));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_FLOWCONTROL_SEND, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_if->flowcontrol, GLB_IF_FLOW_CONTROL_SEND));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_FLOWCONTROL_RECEIVE, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_if->flowcontrol, GLB_IF_FLOW_CONTROL_RECEIVE));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_FLOWCONTROL_OPER_SEND, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_if->flowcontrol, GLB_IF_FLOW_OPER_CONTROL_SEND));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_FLOWCONTROL_OPER_RECEIVE, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_if->flowcontrol, GLB_IF_FLOW_OPER_CONTROL_RECEIVE));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_FEC, field_id))
    {
        sal_sprintf(str, "%u", p_if->fec);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PFC_ADMIN, field_id))
    {
        sal_sprintf(str, "%u", p_if->pfc_admin);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PFC_OPER, field_id))
    {
        sal_sprintf(str, "%u", p_if->pfc_oper);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PFC_PRIORITY, field_id))
    {
        sal_sprintf(str, "%u", p_if->pfc_priority);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_JUMBOFRAME_EN, field_id))
    {
        sal_sprintf(str, "%u", p_if->jumboframe_en);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_ISOLATE_GROUP_ID, field_id))
    {
        sal_sprintf(str, "%u", p_if->isolate_group_id);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_DHCP_SNOOPING_TRUST, field_id))
    {
        sal_sprintf(str, "%u", p_if->dhcp_snooping_trust);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PHY_STATUS, field_id))
    {
        sal_sprintf(str, "%u", p_if->phy_status);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PHY_LOOPBACK_MODE, field_id))
    {
        sal_sprintf(str, "%u", p_if->phy_loopback_mode);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_INTERNAL_LP_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_if->internal_lp_ifindex);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PVLAN_TYPE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(glb_if_pvlan_type_strs, GLB_IF_PVLAN_TYPE_MAX, p_if->pvlan_type));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PRIVATE_VLAN, field_id))
    {
        sal_sprintf(str, "%u", p_if->private_vlan);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_COMMUNITY_VLAN, field_id))
    {
        sal_sprintf(str, "%u", p_if->community_vlan);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_NTP_DISABLE, field_id))
    {
        sal_sprintf(str, "%u", p_if->ntp_disable);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_NTP_BROADCAST_CLIENT, field_id))
    {
        sal_sprintf(str, "%u", p_if->ntp_broadcast_client);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PVLAN_GROUP_ID, field_id))
    {
        sal_sprintf(str, "%u", p_if->pvlan_group_id);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SUPPORT_SPEED, field_id))
    {
        sal_sprintf(str, "%u", p_if->support_speed);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_MODE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(glb_if_mode_strs, GLB_IF_MODE_MAX, p_if->mode));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_VLAN_TYPE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(glb_vlan_port_type_strs, GLB_VLAN_PORT_TYPE_MAX, p_if->vlan_type));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_UP, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_if->flags, GLB_IFF_UP));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_RUNNING, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_if->flags, GLB_IFF_RUNNING));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_MTU, field_id))
    {
        sal_sprintf(str, "%u", p_if->mtu);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_OPER_SPEED, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(glb_port_speed_strs, GLB_PORT_SPEED_MAX, p_if->oper_speed));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PREV_OPER_SPEED, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(glb_port_speed_strs, GLB_PORT_SPEED_MAX, p_if->prev_oper_speed));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_OPER_DUPLEX, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(glb_port_duplex_strs, GLB_PORT_DUPLEX_MAX, p_if->oper_duplex));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_OPER_UNIDIR, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(glb_port_unidir_strs, GLB_PORT_UNIDIR_MAX, p_if->oper_unidir));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_OPER_FEC, field_id))
    {
        sal_sprintf(str, "%u", p_if->oper_fec);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_OPER_LOOPBACK, field_id))
    {
        sal_sprintf(str, "%u", p_if->oper_loopback);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_BANDWIDTH, field_id))
    {
        sal_sprintf(str, "%u", p_if->bandwidth);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_TAP_USED, field_id))
    {
        sal_sprintf(str, "%u", p_if->tap_used);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_TAP_INGRESS_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_if->tap_ingress_enable);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_TAP, field_id))
    {
        sal_sprintf(str, "%s", p_if->tap);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_TAP_MARK_VLAN, field_id))
    {
        sal_sprintf(str, "%u", p_if->tap_mark_vlan);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_MIRROR_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_if->mirror_enable);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_MIRROR_IGS_ID, field_id))
    {
        sal_sprintf(str, "%u", p_if->mirror_igs_id);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_MIRROR_EGS_ID, field_id))
    {
        sal_sprintf(str, "%u", p_if->mirror_egs_id);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_MIRROR_DEST_ID, field_id))
    {
        sal_sprintf(str, "%u", p_if->mirror_dest_id);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_ADMIN_UP, field_id))
    {
        sal_sprintf(str, "%u", p_if->admin_up);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_IP_SOURCE_EN, field_id))
    {
        sal_sprintf(str, "%u", p_if->ip_source_en);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_IP_SOURCE_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_if->ip_source_type);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_IP_SOURCE_ITEM_NUMBER, field_id))
    {
        sal_sprintf(str, "%u", p_if->ip_source_item_number);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_UNICAST_RPF_EN, field_id))
    {
        sal_sprintf(str, "%u", p_if->unicast_rpf_en);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_OPER_BANDWIDTH, field_id))
    {
        sal_sprintf(str, "%u", p_if->oper_bandwidth);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_BANDWIDTH_MAY_CHANGE, field_id))
    {
        sal_sprintf(str, "%u", p_if->bandwidth_may_change);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_ERRDISABLE_REASON, field_id))
    {
        sal_sprintf(str, "%u", p_if->errdisable_reason);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_FDB_LOOP_OCCUR, field_id))
    {
        sal_sprintf(str, "%u", p_if->fdb_loop_occur);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_LINK_FLAP_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_if->link_flap_cnt);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_STATIC_SECURITY_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_if->static_security_num);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_ERRDISABLE_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_if->errdisable_timer);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SFLOW_FLOW_EN, field_id))
    {
        sal_sprintf(str, "%u", p_if->sflow_flow_en);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SFLOW_FLOW_INGRESS, field_id))
    {
        sal_sprintf(str, "%u", p_if->sflow_flow_ingress);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SFLOW_FLOW_EGRESS, field_id))
    {
        sal_sprintf(str, "%u", p_if->sflow_flow_egress);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SFLOW_COUNTER_EN, field_id))
    {
        sal_sprintf(str, "%u", p_if->sflow_counter_en);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SFLOW_RATE, field_id))
    {
        sal_sprintf(str, "%u", p_if->sflow_rate);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SFLOW_SEQUENCE, field_id))
    {
        sal_sprintf(str, "%u", p_if->sflow_sequence);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SFLOW_SAMPLE_POOL, field_id))
    {
        sal_sprintf(str, "%u", p_if->sflow_sample_pool);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_MAC_ADDR, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_if->mac_addr);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_HW_MAC_ADDR, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_if->hw_mac_addr);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_HW_TYPE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(glb_if_type_strs, GLB_IF_TYPE_MAX, p_if->hw_type));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PHY_TYPE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(glb_phy_type_strs, GLB_PHY_TYPE_MAX, p_if->phy_type));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_MEDIA, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(glb_port_media_strs, GLB_PORT_MEDIA_MAX, p_if->media));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PHYPORT_COMBO, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_if->phyport_flag, GLB_PHYPORT_FLAG_COMBO));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PHYPORT_LOOPBACK, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_if->phyport_flag, GLB_PHYPORT_FLAG_LOOPBACK));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PHYPORT_EEE, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_if->phyport_flag, GLB_PHYPORT_FLAG_EEE));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PHYPORT_POE, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_if->phyport_flag, GLB_PHYPORT_FLAG_POE));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PHYPORT_MASTER, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_if->phyport_flag, GLB_PHYPORT_FLAG_MASTER));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PHYPORT_SLAVE, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_if->phyport_flag, GLB_PHYPORT_FLAG_SLAVE));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PHYPORT_UNIDIR, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_if->phyport_flag, GLB_PHYPORT_FLAG_UNIDIR));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SUPPORT_100M, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_if->support_speed_flag, GLB_SUPPORT_SPEED_100M));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SUPPORT_1G, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_if->support_speed_flag, GLB_SUPPORT_SPEED_1G));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SUPPORT_10G, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_if->support_speed_flag, GLB_SUPPORT_SPEED_10G));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SUPPORT_40G, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_if->support_speed_flag, GLB_SUPPORT_SPEED_40G));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SUPPORT_100G, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_if->support_speed_flag, GLB_SUPPORT_SPEED_100G));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SUPPORT_2_5G, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_if->support_speed_flag, GLB_SUPPORT_SPEED_2_5G));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SUPPORT_5G, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_if->support_speed_flag, GLB_SUPPORT_SPEED_5G));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SUPPORT_25G, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_if->support_speed_flag, GLB_SUPPORT_SPEED_25G));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SUPPORT_50G, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_if->support_speed_flag, GLB_SUPPORT_SPEED_50G));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_CIRCUIT_ID, field_id))
    {
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_IGS_ACL, field_id))
    {
        sal_sprintf(str, "%s", p_if->igs_acl);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_EGS_ACL, field_id))
    {
        sal_sprintf(str, "%s", p_if->egs_acl);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_IGS_IPV6_ACL, field_id))
    {
        sal_sprintf(str, "%s", p_if->igs_ipv6_acl);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_EGS_IPV6_ACL, field_id))
    {
        sal_sprintf(str, "%s", p_if->egs_ipv6_acl);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_IGS_MAC_ACL, field_id))
    {
        sal_sprintf(str, "%s", p_if->igs_mac_acl);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_EGS_MAC_ACL, field_id))
    {
        sal_sprintf(str, "%s", p_if->egs_mac_acl);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_IGS_POLICY_MAP, field_id))
    {
        sal_sprintf(str, "%s", p_if->igs_policy_map);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_EGS_POLICY_MAP, field_id))
    {
        sal_sprintf(str, "%s", p_if->egs_policy_map);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_BRGIF, field_id))
    {
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_LAG, field_id))
    {
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_LACP, field_id))
    {
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_STORM_CONTROL, field_id))
    {
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_ACLQOS_IF, field_id))
    {
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_OPENFLOW_IF, field_id))
    {
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_DHCLIENT_IF, field_id))
    {
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_L2PROTOCOL, field_id))
    {
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_CRC_ERROR_CHECK_EN, field_id))
    {
        sal_sprintf(str, "%u", p_if->crc_error_check_en);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_CRC_RECALCULATION_EN, field_id))
    {
        sal_sprintf(str, "%u", p_if->crc_recalculation_en);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_MLAG_ID, field_id))
    {
        sal_sprintf(str, "%u", p_if->mlag_id);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_ERPS_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_if->erps_enable);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_ERPS_PDU_ACTION, field_id))
    {
        sal_sprintf(str, "%u", p_if->erps_pdu_action);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_DOT1X_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_if->dot1x_enable);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_DOT1X_UNAUTHORIZED, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_if->dot1x_flag, GLB_IF_DOT1X_UNAUTHED));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_DOT1X_DIR_IN, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_if->dot1x_flag, GLB_IF_DOT1X_DIR_IN));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_DOT1X_DIR_OUT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_if->dot1x_flag, GLB_IF_DOT1X_DIR_OUT));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_DOT1X_MODE_MAC, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_if->dot1x_flag, GLB_IF_DOT1X_MODE_MAC));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_DOT1X_GUEST_VLAN, field_id))
    {
        sal_sprintf(str, "%u", p_if->dot1x_guest_vlan);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_VLANCLASS_GROUP_ID, field_id))
    {
        sal_sprintf(str, "%u", p_if->vlanclass_group_id);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_VLANCLASS_ACTIVE_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_if->vlanclass_active_type);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_VLANCLASS_DEFAULT_ACTION, field_id))
    {
        sal_sprintf(str, "%u", p_if->vlanclass_default_action);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_DOT1X_CFG_GUEST_VLAN, field_id))
    {
        sal_sprintf(str, "%u", p_if->dot1x_cfg_guest_vlan);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_EGS_ACL_EN, field_id))
    {
        sal_sprintf(str, "%u", p_if->egs_acl_en);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_TAP_GROUP_IGS_REF_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_if->tap_group_igs_ref_cnt);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_TAP_GROUP_EGS_REF_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_if->tap_group_egs_ref_cnt);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_TAP_FLOW_DECAP_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_if->tap_flow_decap_cnt);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_TAP_FLOW_UDF_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_if->tap_flow_udf_cnt);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_METADATA_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_if->metadata_enable);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_ARPINSP_TRUST_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_if->arpinsp_trust_enable);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_ARP_NUMBERLIMIT_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_if->arp_numberlimit_enable);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_ARP_NUMBERLIMIT_NUMBER, field_id))
    {
        sal_sprintf(str, "%u", p_if->arp_numberlimit_number);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_ARP_NUMBERLIMIT_VIOLATE, field_id))
    {
        sal_sprintf(str, "%u", p_if->arp_numberlimit_violate);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_ARP_NUMBERLIMIT_OCCUR, field_id))
    {
        sal_sprintf(str, "%u", p_if->arp_numberlimit_occur);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_ARP_RATE_LIMIT_ERRDISABLE_OCCUR, field_id))
    {
        sal_sprintf(str, "%u", p_if->arp_rate_limit_errdisable_occur);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_INBAND_SNAT_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_if->inband_snat_type);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_LATENCY_EVENT_HIGH, field_id))
    {
        sal_sprintf(str, "%u", p_if->latency_event_high);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_LATENCY_EVENT_LOW, field_id))
    {
        sal_sprintf(str, "%u", p_if->latency_event_low);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_LATENCY_EVENT_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_if->latency_event_enable);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_REFLECTIVE_BRIDGE_EN, field_id))
    {
        sal_sprintf(str, "%u", p_if->reflective_bridge_en);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SUPPORT_SSM_EN, field_id))
    {
        sal_sprintf(str, "%u", p_if->support_ssm_en);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SVLAN_TPID, field_id))
    {
        sal_sprintf(str, "%u", p_if->svlan_tpid);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_L2PRO_IF_ACTION_VLAN, field_id))
    {
        cdb_l2pro_if_action_vlan_val2str(str, MAX_CMD_STR_LEN, &p_if->l2pro_if_action_vlan);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_L2PRO_UPLINK, field_id))
    {
        sal_sprintf(str, "%u", p_if->l2pro_uplink);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_STATIC_AGG_PRIORITY, field_id))
    {
        sal_sprintf(str, "%u", p_if->static_agg_priority);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_STATIC_AGG_IS_ACTIVE, field_id))
    {
        sal_sprintf(str, "%u", p_if->static_agg_is_active);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_FEC_STATUS, field_id))
    {
        sal_sprintf(str, "%u", p_if->fec_status);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_CL73_ABILITY, field_id))
    {
        sal_sprintf(str, "%u", p_if->cl73_ability);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_IFG_STATS_EN, field_id))
    {
        sal_sprintf(str, "%u", p_if->ifg_stats_en);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_OPENFLOW_EN, field_id))
    {
        sal_sprintf(str, "%u", p_if->openflow_en);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_OPENFLOW_OPERATION_AGG, field_id))
    {
        sal_sprintf(str, "%u", p_if->openflow_operation_agg);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_OPENFLOW_OPERATION_AGG_MEM_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_if->openflow_operation_agg_mem_ifindex);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_CARRIER_DOWN_HOLDTIME_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_if->carrier_down_holdtime_interval);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_HASH_VALUE_PROFILE_ID, field_id))
    {
        sal_sprintf(str, "%u", p_if->hash_value_profile_id);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_HASH_VALUE_PROFILE_DIR, field_id))
    {
        sal_sprintf(str, "%u", p_if->hash_value_profile_dir);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_HASH_VALUE_PROFILE_ID_AGG, field_id))
    {
        sal_sprintf(str, "%u", p_if->hash_value_profile_id_agg);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_HASH_VALUE_AGG_HASH, field_id))
    {
        sal_sprintf(str, "%u", p_if->hash_value_agg_hash);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_POE, field_id))
    {
        cdb_poe_cfg_val2str(str, MAX_CMD_STR_LEN, &p_if->poe);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_CURRENT_G8032_RING_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_if->current_g8032_ring_count);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_G8032_RING_FIRST, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_if->g8032_ring_first, sizeof(p_if->g8032_ring_first)/4);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_CFM_LMEP_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_if->cfm_lmep_num);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_CFM_RMEP_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_if->cfm_rmep_num);
    }
    return str;
}

char**
tbl_interface_table_dump(tbl_interface_t *p_if, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_interface_key_name_dump(p_if, buf));
    for(i=1; i<TBL_INTERFACE_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_interface_field_name_dump(p_if, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_interface_field_value_dump(p_if, i, buf));
    }
    return str;
}

int32
tbl_interface_field_value_parser(char *str, int32 field_id, tbl_interface_t *p_if)
{
    if (FLD_MATCH(TBL_INTERFACE_FLD_KEY, field_id))
    {
        sal_strcpy(p_if->key.name, str);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_IFINDEX, field_id))
    {
        int32 ret;
        p_if->ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PORTID, field_id))
    {
        int32 ret;
        p_if->portid = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_HOSTIFID, field_id))
    {
        int32 ret;
        p_if->hostifid = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_DESC, field_id))
    {
        sal_strcpy(p_if->desc, str);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SPEED, field_id))
    {
        cdb_enum_str2val(glb_port_speed_strs, GLB_PORT_SPEED_MAX, str);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_DUPLEX, field_id))
    {
        cdb_enum_str2val(glb_port_duplex_strs, GLB_PORT_DUPLEX_MAX, str);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_UNIDIR, field_id))
    {
        cdb_enum_str2val(glb_port_unidir_strs, GLB_PORT_UNIDIR_MAX, str);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_FLOWCONTROL_SEND, field_id))
    {
        GLB_SET_FLAG(p_if->flowcontrol, GLB_IF_FLOW_CONTROL_SEND);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_FLOWCONTROL_RECEIVE, field_id))
    {
        GLB_SET_FLAG(p_if->flowcontrol, GLB_IF_FLOW_CONTROL_RECEIVE);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_FLOWCONTROL_OPER_SEND, field_id))
    {
        GLB_SET_FLAG(p_if->flowcontrol, GLB_IF_FLOW_OPER_CONTROL_SEND);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_FLOWCONTROL_OPER_RECEIVE, field_id))
    {
        GLB_SET_FLAG(p_if->flowcontrol, GLB_IF_FLOW_OPER_CONTROL_RECEIVE);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_FEC, field_id))
    {
        int32 ret;
        p_if->fec = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PFC_ADMIN, field_id))
    {
        int32 ret;
        p_if->pfc_admin = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PFC_OPER, field_id))
    {
        int32 ret;
        p_if->pfc_oper = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PFC_PRIORITY, field_id))
    {
        int32 ret;
        p_if->pfc_priority = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_JUMBOFRAME_EN, field_id))
    {
        int32 ret;
        p_if->jumboframe_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_ISOLATE_GROUP_ID, field_id))
    {
        int32 ret;
        p_if->isolate_group_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_DHCP_SNOOPING_TRUST, field_id))
    {
        int32 ret;
        p_if->dhcp_snooping_trust = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PHY_STATUS, field_id))
    {
        int32 ret;
        p_if->phy_status = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PHY_LOOPBACK_MODE, field_id))
    {
        int32 ret;
        p_if->phy_loopback_mode = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_INTERNAL_LP_IFINDEX, field_id))
    {
        int32 ret;
        p_if->internal_lp_ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PVLAN_TYPE, field_id))
    {
        cdb_enum_str2val(glb_if_pvlan_type_strs, GLB_IF_PVLAN_TYPE_MAX, str);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PRIVATE_VLAN, field_id))
    {
        int32 ret;
        p_if->private_vlan = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_COMMUNITY_VLAN, field_id))
    {
        int32 ret;
        p_if->community_vlan = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_NTP_DISABLE, field_id))
    {
        int32 ret;
        p_if->ntp_disable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_NTP_BROADCAST_CLIENT, field_id))
    {
        int32 ret;
        p_if->ntp_broadcast_client = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PVLAN_GROUP_ID, field_id))
    {
        int32 ret;
        p_if->pvlan_group_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SUPPORT_SPEED, field_id))
    {
        int32 ret;
        p_if->support_speed = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_MODE, field_id))
    {
        cdb_enum_str2val(glb_if_mode_strs, GLB_IF_MODE_MAX, str);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_VLAN_TYPE, field_id))
    {
        cdb_enum_str2val(glb_vlan_port_type_strs, GLB_VLAN_PORT_TYPE_MAX, str);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_UP, field_id))
    {
        GLB_SET_FLAG(p_if->flags, GLB_IFF_UP);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_RUNNING, field_id))
    {
        GLB_SET_FLAG(p_if->flags, GLB_IFF_RUNNING);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_MTU, field_id))
    {
        int32 ret;
        p_if->mtu = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_OPER_SPEED, field_id))
    {
        cdb_enum_str2val(glb_port_speed_strs, GLB_PORT_SPEED_MAX, str);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PREV_OPER_SPEED, field_id))
    {
        cdb_enum_str2val(glb_port_speed_strs, GLB_PORT_SPEED_MAX, str);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_OPER_DUPLEX, field_id))
    {
        cdb_enum_str2val(glb_port_duplex_strs, GLB_PORT_DUPLEX_MAX, str);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_OPER_UNIDIR, field_id))
    {
        cdb_enum_str2val(glb_port_unidir_strs, GLB_PORT_UNIDIR_MAX, str);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_OPER_FEC, field_id))
    {
        int32 ret;
        p_if->oper_fec = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_OPER_LOOPBACK, field_id))
    {
        int32 ret;
        p_if->oper_loopback = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_BANDWIDTH, field_id))
    {
        int32 ret;
        p_if->bandwidth = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_TAP_USED, field_id))
    {
        int32 ret;
        p_if->tap_used = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_TAP_INGRESS_ENABLE, field_id))
    {
        int32 ret;
        p_if->tap_ingress_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_TAP, field_id))
    {
        sal_strcpy(p_if->tap, str);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_TAP_MARK_VLAN, field_id))
    {
        int32 ret;
        p_if->tap_mark_vlan = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_MIRROR_ENABLE, field_id))
    {
        int32 ret;
        p_if->mirror_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_MIRROR_IGS_ID, field_id))
    {
        int32 ret;
        p_if->mirror_igs_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_MIRROR_EGS_ID, field_id))
    {
        int32 ret;
        p_if->mirror_egs_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_MIRROR_DEST_ID, field_id))
    {
        int32 ret;
        p_if->mirror_dest_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_ADMIN_UP, field_id))
    {
        int32 ret;
        p_if->admin_up = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_IP_SOURCE_EN, field_id))
    {
        int32 ret;
        p_if->ip_source_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_IP_SOURCE_TYPE, field_id))
    {
        int32 ret;
        p_if->ip_source_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_IP_SOURCE_ITEM_NUMBER, field_id))
    {
        int32 ret;
        p_if->ip_source_item_number = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_UNICAST_RPF_EN, field_id))
    {
        int32 ret;
        p_if->unicast_rpf_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_OPER_BANDWIDTH, field_id))
    {
        int32 ret;
        p_if->oper_bandwidth = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_BANDWIDTH_MAY_CHANGE, field_id))
    {
        int32 ret;
        p_if->bandwidth_may_change = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_ERRDISABLE_REASON, field_id))
    {
        int32 ret;
        p_if->errdisable_reason = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_FDB_LOOP_OCCUR, field_id))
    {
        int32 ret;
        p_if->fdb_loop_occur = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_LINK_FLAP_CNT, field_id))
    {
        int32 ret;
        p_if->link_flap_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_STATIC_SECURITY_NUM, field_id))
    {
        int32 ret;
        p_if->static_security_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_ERRDISABLE_TIMER, field_id))
    {
        ctc_task_str2val(str, p_if->errdisable_timer);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SFLOW_FLOW_EN, field_id))
    {
        int32 ret;
        p_if->sflow_flow_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SFLOW_FLOW_INGRESS, field_id))
    {
        int32 ret;
        p_if->sflow_flow_ingress = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SFLOW_FLOW_EGRESS, field_id))
    {
        int32 ret;
        p_if->sflow_flow_egress = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SFLOW_COUNTER_EN, field_id))
    {
        int32 ret;
        p_if->sflow_counter_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SFLOW_RATE, field_id))
    {
        int32 ret;
        p_if->sflow_rate = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SFLOW_SEQUENCE, field_id))
    {
        int32 ret;
        p_if->sflow_sequence = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SFLOW_SAMPLE_POOL, field_id))
    {
        int32 ret;
        p_if->sflow_sample_pool = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_MAC_ADDR, field_id))
    {
        cdb_mac_addr_str2val(p_if->mac_addr, str);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_HW_MAC_ADDR, field_id))
    {
        cdb_mac_addr_str2val(p_if->hw_mac_addr, str);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_HW_TYPE, field_id))
    {
        cdb_enum_str2val(glb_if_type_strs, GLB_IF_TYPE_MAX, str);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PHY_TYPE, field_id))
    {
        cdb_enum_str2val(glb_phy_type_strs, GLB_PHY_TYPE_MAX, str);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_MEDIA, field_id))
    {
        cdb_enum_str2val(glb_port_media_strs, GLB_PORT_MEDIA_MAX, str);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PHYPORT_COMBO, field_id))
    {
        GLB_SET_FLAG(p_if->phyport_flag, GLB_PHYPORT_FLAG_COMBO);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PHYPORT_LOOPBACK, field_id))
    {
        GLB_SET_FLAG(p_if->phyport_flag, GLB_PHYPORT_FLAG_LOOPBACK);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PHYPORT_EEE, field_id))
    {
        GLB_SET_FLAG(p_if->phyport_flag, GLB_PHYPORT_FLAG_EEE);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PHYPORT_POE, field_id))
    {
        GLB_SET_FLAG(p_if->phyport_flag, GLB_PHYPORT_FLAG_POE);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PHYPORT_MASTER, field_id))
    {
        GLB_SET_FLAG(p_if->phyport_flag, GLB_PHYPORT_FLAG_MASTER);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PHYPORT_SLAVE, field_id))
    {
        GLB_SET_FLAG(p_if->phyport_flag, GLB_PHYPORT_FLAG_SLAVE);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PHYPORT_UNIDIR, field_id))
    {
        GLB_SET_FLAG(p_if->phyport_flag, GLB_PHYPORT_FLAG_UNIDIR);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SUPPORT_100M, field_id))
    {
        GLB_SET_FLAG(p_if->support_speed_flag, GLB_SUPPORT_SPEED_100M);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SUPPORT_1G, field_id))
    {
        GLB_SET_FLAG(p_if->support_speed_flag, GLB_SUPPORT_SPEED_1G);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SUPPORT_10G, field_id))
    {
        GLB_SET_FLAG(p_if->support_speed_flag, GLB_SUPPORT_SPEED_10G);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SUPPORT_40G, field_id))
    {
        GLB_SET_FLAG(p_if->support_speed_flag, GLB_SUPPORT_SPEED_40G);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SUPPORT_100G, field_id))
    {
        GLB_SET_FLAG(p_if->support_speed_flag, GLB_SUPPORT_SPEED_100G);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SUPPORT_2_5G, field_id))
    {
        GLB_SET_FLAG(p_if->support_speed_flag, GLB_SUPPORT_SPEED_2_5G);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SUPPORT_5G, field_id))
    {
        GLB_SET_FLAG(p_if->support_speed_flag, GLB_SUPPORT_SPEED_5G);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SUPPORT_25G, field_id))
    {
        GLB_SET_FLAG(p_if->support_speed_flag, GLB_SUPPORT_SPEED_25G);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SUPPORT_50G, field_id))
    {
        GLB_SET_FLAG(p_if->support_speed_flag, GLB_SUPPORT_SPEED_50G);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_CIRCUIT_ID, field_id))
    {
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_IGS_ACL, field_id))
    {
        sal_strcpy(p_if->igs_acl, str);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_EGS_ACL, field_id))
    {
        sal_strcpy(p_if->egs_acl, str);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_IGS_IPV6_ACL, field_id))
    {
        sal_strcpy(p_if->igs_ipv6_acl, str);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_EGS_IPV6_ACL, field_id))
    {
        sal_strcpy(p_if->egs_ipv6_acl, str);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_IGS_MAC_ACL, field_id))
    {
        sal_strcpy(p_if->igs_mac_acl, str);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_EGS_MAC_ACL, field_id))
    {
        sal_strcpy(p_if->egs_mac_acl, str);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_IGS_POLICY_MAP, field_id))
    {
        sal_strcpy(p_if->igs_policy_map, str);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_EGS_POLICY_MAP, field_id))
    {
        sal_strcpy(p_if->egs_policy_map, str);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_BRGIF, field_id))
    {
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_LAG, field_id))
    {
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_LACP, field_id))
    {
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_STORM_CONTROL, field_id))
    {
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_ACLQOS_IF, field_id))
    {
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_OPENFLOW_IF, field_id))
    {
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_DHCLIENT_IF, field_id))
    {
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_L2PROTOCOL, field_id))
    {
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_CRC_ERROR_CHECK_EN, field_id))
    {
        int32 ret;
        p_if->crc_error_check_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_CRC_RECALCULATION_EN, field_id))
    {
        int32 ret;
        p_if->crc_recalculation_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_MLAG_ID, field_id))
    {
        int32 ret;
        p_if->mlag_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_ERPS_ENABLE, field_id))
    {
        int32 ret;
        p_if->erps_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_ERPS_PDU_ACTION, field_id))
    {
        int32 ret;
        p_if->erps_pdu_action = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_DOT1X_ENABLE, field_id))
    {
        int32 ret;
        p_if->dot1x_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_DOT1X_UNAUTHORIZED, field_id))
    {
        GLB_SET_FLAG(p_if->dot1x_flag, GLB_IF_DOT1X_UNAUTHED);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_DOT1X_DIR_IN, field_id))
    {
        GLB_SET_FLAG(p_if->dot1x_flag, GLB_IF_DOT1X_DIR_IN);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_DOT1X_DIR_OUT, field_id))
    {
        GLB_SET_FLAG(p_if->dot1x_flag, GLB_IF_DOT1X_DIR_OUT);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_DOT1X_MODE_MAC, field_id))
    {
        GLB_SET_FLAG(p_if->dot1x_flag, GLB_IF_DOT1X_MODE_MAC);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_DOT1X_GUEST_VLAN, field_id))
    {
        int32 ret;
        p_if->dot1x_guest_vlan = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_VLANCLASS_GROUP_ID, field_id))
    {
        int32 ret;
        p_if->vlanclass_group_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_VLANCLASS_ACTIVE_TYPE, field_id))
    {
        int32 ret;
        p_if->vlanclass_active_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_VLANCLASS_DEFAULT_ACTION, field_id))
    {
        int32 ret;
        p_if->vlanclass_default_action = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_DOT1X_CFG_GUEST_VLAN, field_id))
    {
        int32 ret;
        p_if->dot1x_cfg_guest_vlan = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_EGS_ACL_EN, field_id))
    {
        int32 ret;
        p_if->egs_acl_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_TAP_GROUP_IGS_REF_CNT, field_id))
    {
        int32 ret;
        p_if->tap_group_igs_ref_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_TAP_GROUP_EGS_REF_CNT, field_id))
    {
        int32 ret;
        p_if->tap_group_egs_ref_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_TAP_FLOW_DECAP_CNT, field_id))
    {
        int32 ret;
        p_if->tap_flow_decap_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_TAP_FLOW_UDF_CNT, field_id))
    {
        int32 ret;
        p_if->tap_flow_udf_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_METADATA_ENABLE, field_id))
    {
        int32 ret;
        p_if->metadata_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_ARPINSP_TRUST_ENABLE, field_id))
    {
        int32 ret;
        p_if->arpinsp_trust_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_ARP_NUMBERLIMIT_ENABLE, field_id))
    {
        int32 ret;
        p_if->arp_numberlimit_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_ARP_NUMBERLIMIT_NUMBER, field_id))
    {
        int32 ret;
        p_if->arp_numberlimit_number = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_ARP_NUMBERLIMIT_VIOLATE, field_id))
    {
        int32 ret;
        p_if->arp_numberlimit_violate = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_ARP_NUMBERLIMIT_OCCUR, field_id))
    {
        int32 ret;
        p_if->arp_numberlimit_occur = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_ARP_RATE_LIMIT_ERRDISABLE_OCCUR, field_id))
    {
        int32 ret;
        p_if->arp_rate_limit_errdisable_occur = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_INBAND_SNAT_TYPE, field_id))
    {
        int32 ret;
        p_if->inband_snat_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_LATENCY_EVENT_HIGH, field_id))
    {
        int32 ret;
        p_if->latency_event_high = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_LATENCY_EVENT_LOW, field_id))
    {
        int32 ret;
        p_if->latency_event_low = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_LATENCY_EVENT_ENABLE, field_id))
    {
        int32 ret;
        p_if->latency_event_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_REFLECTIVE_BRIDGE_EN, field_id))
    {
        int32 ret;
        p_if->reflective_bridge_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SUPPORT_SSM_EN, field_id))
    {
        int32 ret;
        p_if->support_ssm_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SVLAN_TPID, field_id))
    {
        int32 ret;
        p_if->svlan_tpid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_L2PRO_IF_ACTION_VLAN, field_id))
    {
        cdb_l2pro_if_action_vlan_str2val(str, &p_if->l2pro_if_action_vlan);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_L2PRO_UPLINK, field_id))
    {
        int32 ret;
        p_if->l2pro_uplink = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_STATIC_AGG_PRIORITY, field_id))
    {
        int32 ret;
        p_if->static_agg_priority = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_STATIC_AGG_IS_ACTIVE, field_id))
    {
        int32 ret;
        p_if->static_agg_is_active = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_FEC_STATUS, field_id))
    {
        int32 ret;
        p_if->fec_status = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_CL73_ABILITY, field_id))
    {
        int32 ret;
        p_if->cl73_ability = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_IFG_STATS_EN, field_id))
    {
        int32 ret;
        p_if->ifg_stats_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_OPENFLOW_EN, field_id))
    {
        int32 ret;
        p_if->openflow_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_OPENFLOW_OPERATION_AGG, field_id))
    {
        int32 ret;
        p_if->openflow_operation_agg = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_OPENFLOW_OPERATION_AGG_MEM_IFINDEX, field_id))
    {
        int32 ret;
        p_if->openflow_operation_agg_mem_ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_CARRIER_DOWN_HOLDTIME_INTERVAL, field_id))
    {
        int32 ret;
        p_if->carrier_down_holdtime_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_HASH_VALUE_PROFILE_ID, field_id))
    {
        int32 ret;
        p_if->hash_value_profile_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_HASH_VALUE_PROFILE_DIR, field_id))
    {
        int32 ret;
        p_if->hash_value_profile_dir = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_HASH_VALUE_PROFILE_ID_AGG, field_id))
    {
        int32 ret;
        p_if->hash_value_profile_id_agg = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_HASH_VALUE_AGG_HASH, field_id))
    {
        int32 ret;
        p_if->hash_value_agg_hash = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_POE, field_id))
    {
        cdb_poe_cfg_str2val(str, &p_if->poe);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_CURRENT_G8032_RING_COUNT, field_id))
    {
        int32 ret;
        p_if->current_g8032_ring_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_G8032_RING_FIRST, field_id))
    {
        cdb_uint32_array_str2val(str, p_if->g8032_ring_first);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_CFM_LMEP_NUM, field_id))
    {
        int32 ret;
        p_if->cfm_lmep_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_CFM_RMEP_NUM, field_id))
    {
        int32 ret;
        p_if->cfm_rmep_num = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_interface_table_parser(char** array, char* key_value,tbl_interface_t *p_if)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_INTERFACE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_interface_key_str2val(key_value, p_if));

    for(i=1; i<TBL_INTERFACE_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_interface_field_value_parser( array[j++], i, p_if));
    }

    return PM_E_NONE;
}

int32
tbl_interface_dump_one(tbl_interface_t *p_if, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_INTERFACE);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    ds_circuit_id_t *p_db_circuit_id = NULL;
    ds_l2protocol_t *p_db_l2protocol = NULL;
    ctclib_slistnode_t *listnode = NULL;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_INTERFACE_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_INTERFACE_FLD_KEY].name,
            p_if->key.name);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_IFINDEX].name,
            p_if->ifindex);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PORTID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_INTERFACE_FLD_PORTID].name,
            p_if->portid);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_HOSTIFID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_INTERFACE_FLD_HOSTIFID].name,
            p_if->hostifid);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_DESC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_INTERFACE_FLD_DESC].name,
            p_if->desc);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SPEED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_INTERFACE_FLD_SPEED].name, 
            cdb_enum_val2str(glb_port_speed_strs, GLB_PORT_SPEED_MAX, p_if->speed));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_DUPLEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_INTERFACE_FLD_DUPLEX].name, 
            cdb_enum_val2str(glb_port_duplex_strs, GLB_PORT_DUPLEX_MAX, p_if->duplex));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_UNIDIR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_INTERFACE_FLD_UNIDIR].name, 
            cdb_enum_val2str(glb_port_unidir_strs, GLB_PORT_UNIDIR_MAX, p_if->unidir));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_FLOWCONTROL_SEND, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_FLOWCONTROL_SEND].name,
            GLB_FLAG_ISSET(p_if->flowcontrol, GLB_IF_FLOW_CONTROL_SEND));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_FLOWCONTROL_RECEIVE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_FLOWCONTROL_RECEIVE].name,
            GLB_FLAG_ISSET(p_if->flowcontrol, GLB_IF_FLOW_CONTROL_RECEIVE));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_FLOWCONTROL_OPER_SEND, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_FLOWCONTROL_OPER_SEND].name,
            GLB_FLAG_ISSET(p_if->flowcontrol, GLB_IF_FLOW_OPER_CONTROL_SEND));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_FLOWCONTROL_OPER_RECEIVE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_FLOWCONTROL_OPER_RECEIVE].name,
            GLB_FLAG_ISSET(p_if->flowcontrol, GLB_IF_FLOW_OPER_CONTROL_RECEIVE));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_FEC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_FEC].name,
            p_if->fec);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PFC_ADMIN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_PFC_ADMIN].name,
            p_if->pfc_admin);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PFC_OPER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_PFC_OPER].name,
            p_if->pfc_oper);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PFC_PRIORITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_PFC_PRIORITY].name,
            p_if->pfc_priority);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_JUMBOFRAME_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_JUMBOFRAME_EN].name,
            p_if->jumboframe_en);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_ISOLATE_GROUP_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_ISOLATE_GROUP_ID].name,
            p_if->isolate_group_id);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_DHCP_SNOOPING_TRUST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_DHCP_SNOOPING_TRUST].name,
            p_if->dhcp_snooping_trust);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PHY_STATUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_PHY_STATUS].name,
            p_if->phy_status);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PHY_LOOPBACK_MODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_PHY_LOOPBACK_MODE].name,
            p_if->phy_loopback_mode);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_INTERNAL_LP_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_INTERNAL_LP_IFINDEX].name,
            p_if->internal_lp_ifindex);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PVLAN_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_INTERFACE_FLD_PVLAN_TYPE].name, 
            cdb_enum_val2str(glb_if_pvlan_type_strs, GLB_IF_PVLAN_TYPE_MAX, p_if->pvlan_type));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PRIVATE_VLAN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_PRIVATE_VLAN].name,
            p_if->private_vlan);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_COMMUNITY_VLAN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_COMMUNITY_VLAN].name,
            p_if->community_vlan);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_NTP_DISABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_NTP_DISABLE].name,
            p_if->ntp_disable);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_NTP_BROADCAST_CLIENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_NTP_BROADCAST_CLIENT].name,
            p_if->ntp_broadcast_client);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PVLAN_GROUP_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_PVLAN_GROUP_ID].name,
            p_if->pvlan_group_id);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SUPPORT_SPEED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_SUPPORT_SPEED].name,
            p_if->support_speed);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_MODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_INTERFACE_FLD_MODE].name, 
            cdb_enum_val2str(glb_if_mode_strs, GLB_IF_MODE_MAX, p_if->mode));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_VLAN_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_INTERFACE_FLD_VLAN_TYPE].name, 
            cdb_enum_val2str(glb_vlan_port_type_strs, GLB_VLAN_PORT_TYPE_MAX, p_if->vlan_type));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_UP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_UP].name,
            GLB_FLAG_ISSET(p_if->flags, GLB_IFF_UP));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_RUNNING, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_RUNNING].name,
            GLB_FLAG_ISSET(p_if->flags, GLB_IFF_RUNNING));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_MTU, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_MTU].name,
            p_if->mtu);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_OPER_SPEED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_INTERFACE_FLD_OPER_SPEED].name, 
            cdb_enum_val2str(glb_port_speed_strs, GLB_PORT_SPEED_MAX, p_if->oper_speed));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PREV_OPER_SPEED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_INTERFACE_FLD_PREV_OPER_SPEED].name, 
            cdb_enum_val2str(glb_port_speed_strs, GLB_PORT_SPEED_MAX, p_if->prev_oper_speed));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_OPER_DUPLEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_INTERFACE_FLD_OPER_DUPLEX].name, 
            cdb_enum_val2str(glb_port_duplex_strs, GLB_PORT_DUPLEX_MAX, p_if->oper_duplex));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_OPER_UNIDIR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_INTERFACE_FLD_OPER_UNIDIR].name, 
            cdb_enum_val2str(glb_port_unidir_strs, GLB_PORT_UNIDIR_MAX, p_if->oper_unidir));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_OPER_FEC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_OPER_FEC].name,
            p_if->oper_fec);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_OPER_LOOPBACK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_OPER_LOOPBACK].name,
            p_if->oper_loopback);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_BANDWIDTH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_BANDWIDTH].name,
            p_if->bandwidth);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_TAP_USED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_TAP_USED].name,
            p_if->tap_used);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_TAP_INGRESS_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_TAP_INGRESS_ENABLE].name,
            p_if->tap_ingress_enable);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_TAP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_INTERFACE_FLD_TAP].name,
            p_if->tap);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_TAP_MARK_VLAN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_TAP_MARK_VLAN].name,
            p_if->tap_mark_vlan);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_MIRROR_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_MIRROR_ENABLE].name,
            p_if->mirror_enable);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_MIRROR_IGS_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_MIRROR_IGS_ID].name,
            p_if->mirror_igs_id);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_MIRROR_EGS_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_MIRROR_EGS_ID].name,
            p_if->mirror_egs_id);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_MIRROR_DEST_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_MIRROR_DEST_ID].name,
            p_if->mirror_dest_id);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_ADMIN_UP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_ADMIN_UP].name,
            p_if->admin_up);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_IP_SOURCE_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_IP_SOURCE_EN].name,
            p_if->ip_source_en);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_IP_SOURCE_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_IP_SOURCE_TYPE].name,
            p_if->ip_source_type);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_IP_SOURCE_ITEM_NUMBER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_IP_SOURCE_ITEM_NUMBER].name,
            p_if->ip_source_item_number);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_UNICAST_RPF_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_UNICAST_RPF_EN].name,
            p_if->unicast_rpf_en);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_OPER_BANDWIDTH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_OPER_BANDWIDTH].name,
            p_if->oper_bandwidth);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_BANDWIDTH_MAY_CHANGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_BANDWIDTH_MAY_CHANGE].name,
            p_if->bandwidth_may_change);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_ERRDISABLE_REASON, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_ERRDISABLE_REASON].name,
            p_if->errdisable_reason);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_FDB_LOOP_OCCUR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_FDB_LOOP_OCCUR].name,
            p_if->fdb_loop_occur);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_LINK_FLAP_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_LINK_FLAP_CNT].name,
            p_if->link_flap_cnt);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_STATIC_SECURITY_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_STATIC_SECURITY_NUM].name,
            p_if->static_security_num);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_ERRDISABLE_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_INTERFACE_FLD_ERRDISABLE_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_if->errdisable_timer));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SFLOW_FLOW_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_SFLOW_FLOW_EN].name,
            p_if->sflow_flow_en);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SFLOW_FLOW_INGRESS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_SFLOW_FLOW_INGRESS].name,
            p_if->sflow_flow_ingress);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SFLOW_FLOW_EGRESS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_SFLOW_FLOW_EGRESS].name,
            p_if->sflow_flow_egress);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SFLOW_COUNTER_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_SFLOW_COUNTER_EN].name,
            p_if->sflow_counter_en);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SFLOW_RATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_SFLOW_RATE].name,
            p_if->sflow_rate);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SFLOW_SEQUENCE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_SFLOW_SEQUENCE].name,
            p_if->sflow_sequence);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SFLOW_SAMPLE_POOL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_SFLOW_SAMPLE_POOL].name,
            p_if->sflow_sample_pool);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_MAC_ADDR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_INTERFACE_FLD_MAC_ADDR].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_if->mac_addr));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_HW_MAC_ADDR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_INTERFACE_FLD_HW_MAC_ADDR].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_if->hw_mac_addr));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_HW_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_INTERFACE_FLD_HW_TYPE].name, 
            cdb_enum_val2str(glb_if_type_strs, GLB_IF_TYPE_MAX, p_if->hw_type));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PHY_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_INTERFACE_FLD_PHY_TYPE].name, 
            cdb_enum_val2str(glb_phy_type_strs, GLB_PHY_TYPE_MAX, p_if->phy_type));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_MEDIA, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_INTERFACE_FLD_MEDIA].name, 
            cdb_enum_val2str(glb_port_media_strs, GLB_PORT_MEDIA_MAX, p_if->media));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PHYPORT_COMBO, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_PHYPORT_COMBO].name,
            GLB_FLAG_ISSET(p_if->phyport_flag, GLB_PHYPORT_FLAG_COMBO));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PHYPORT_LOOPBACK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_PHYPORT_LOOPBACK].name,
            GLB_FLAG_ISSET(p_if->phyport_flag, GLB_PHYPORT_FLAG_LOOPBACK));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PHYPORT_EEE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_PHYPORT_EEE].name,
            GLB_FLAG_ISSET(p_if->phyport_flag, GLB_PHYPORT_FLAG_EEE));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PHYPORT_POE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_PHYPORT_POE].name,
            GLB_FLAG_ISSET(p_if->phyport_flag, GLB_PHYPORT_FLAG_POE));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PHYPORT_MASTER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_PHYPORT_MASTER].name,
            GLB_FLAG_ISSET(p_if->phyport_flag, GLB_PHYPORT_FLAG_MASTER));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PHYPORT_SLAVE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_PHYPORT_SLAVE].name,
            GLB_FLAG_ISSET(p_if->phyport_flag, GLB_PHYPORT_FLAG_SLAVE));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_PHYPORT_UNIDIR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_PHYPORT_UNIDIR].name,
            GLB_FLAG_ISSET(p_if->phyport_flag, GLB_PHYPORT_FLAG_UNIDIR));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SUPPORT_100M, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_SUPPORT_100M].name,
            GLB_FLAG_ISSET(p_if->support_speed_flag, GLB_SUPPORT_SPEED_100M));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SUPPORT_1G, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_SUPPORT_1G].name,
            GLB_FLAG_ISSET(p_if->support_speed_flag, GLB_SUPPORT_SPEED_1G));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SUPPORT_10G, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_SUPPORT_10G].name,
            GLB_FLAG_ISSET(p_if->support_speed_flag, GLB_SUPPORT_SPEED_10G));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SUPPORT_40G, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_SUPPORT_40G].name,
            GLB_FLAG_ISSET(p_if->support_speed_flag, GLB_SUPPORT_SPEED_40G));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SUPPORT_100G, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_SUPPORT_100G].name,
            GLB_FLAG_ISSET(p_if->support_speed_flag, GLB_SUPPORT_SPEED_100G));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SUPPORT_2_5G, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_SUPPORT_2_5G].name,
            GLB_FLAG_ISSET(p_if->support_speed_flag, GLB_SUPPORT_SPEED_2_5G));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SUPPORT_5G, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_SUPPORT_5G].name,
            GLB_FLAG_ISSET(p_if->support_speed_flag, GLB_SUPPORT_SPEED_5G));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SUPPORT_25G, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_SUPPORT_25G].name,
            GLB_FLAG_ISSET(p_if->support_speed_flag, GLB_SUPPORT_SPEED_25G));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SUPPORT_50G, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_SUPPORT_50G].name,
            GLB_FLAG_ISSET(p_if->support_speed_flag, GLB_SUPPORT_SPEED_50G));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_CIRCUIT_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"{", p_tbl_info->field[TBL_INTERFACE_FLD_CIRCUIT_ID].name);
        if (p_if->circuit_id.obj_list)
        {
            p_field->field_id[1] = FLD_ID_ALL;
            CTCLIB_SLIST_LOOP(p_if->circuit_id.obj_list, p_db_circuit_id, listnode)
            {
                ds_circuit_id_dump_one(p_db_circuit_id, pargs);
                sal_fprintf(fp, "/,");
            }
        }
        sal_fprintf(fp, "}");
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_IGS_ACL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_INTERFACE_FLD_IGS_ACL].name,
            p_if->igs_acl);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_EGS_ACL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_INTERFACE_FLD_EGS_ACL].name,
            p_if->egs_acl);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_IGS_IPV6_ACL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_INTERFACE_FLD_IGS_IPV6_ACL].name,
            p_if->igs_ipv6_acl);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_EGS_IPV6_ACL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_INTERFACE_FLD_EGS_IPV6_ACL].name,
            p_if->egs_ipv6_acl);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_IGS_MAC_ACL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_INTERFACE_FLD_IGS_MAC_ACL].name,
            p_if->igs_mac_acl);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_EGS_MAC_ACL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_INTERFACE_FLD_EGS_MAC_ACL].name,
            p_if->egs_mac_acl);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_IGS_POLICY_MAP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_INTERFACE_FLD_IGS_POLICY_MAP].name,
            p_if->igs_policy_map);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_EGS_POLICY_MAP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_INTERFACE_FLD_EGS_POLICY_MAP].name,
            p_if->egs_policy_map);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_BRGIF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"{", p_tbl_info->field[TBL_INTERFACE_FLD_BRGIF].name);
        if (p_if->brgif)
        {
            ds_brgif_dump_one(p_if->brgif, pargs);
        }
        sal_fprintf(fp, "}");
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_LAG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"{", p_tbl_info->field[TBL_INTERFACE_FLD_LAG].name);
        if (p_if->lag)
        {
            ds_lag_dump_one(p_if->lag, pargs);
        }
        sal_fprintf(fp, "}");
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_LACP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"{", p_tbl_info->field[TBL_INTERFACE_FLD_LACP].name);
        if (p_if->lacp)
        {
            ds_lacp_dump_one(p_if->lacp, pargs);
        }
        sal_fprintf(fp, "}");
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_STORM_CONTROL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"{", p_tbl_info->field[TBL_INTERFACE_FLD_STORM_CONTROL].name);
        if (p_if->storm_control)
        {
            ds_storm_control_dump_one(p_if->storm_control, pargs);
        }
        sal_fprintf(fp, "}");
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_ACLQOS_IF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"{", p_tbl_info->field[TBL_INTERFACE_FLD_ACLQOS_IF].name);
        if (p_if->aclqos_if)
        {
            ds_aclqos_if_dump_one(p_if->aclqos_if, pargs);
        }
        sal_fprintf(fp, "}");
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_OPENFLOW_IF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"{", p_tbl_info->field[TBL_INTERFACE_FLD_OPENFLOW_IF].name);
        if (p_if->openflow_if)
        {
            ds_openflow_if_dump_one(p_if->openflow_if, pargs);
        }
        sal_fprintf(fp, "}");
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_DHCLIENT_IF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"{", p_tbl_info->field[TBL_INTERFACE_FLD_DHCLIENT_IF].name);
        if (p_if->dhclient_if)
        {
            ds_dhclient_if_dump_one(p_if->dhclient_if, pargs);
        }
        sal_fprintf(fp, "}");
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_L2PROTOCOL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"{", p_tbl_info->field[TBL_INTERFACE_FLD_L2PROTOCOL].name);
        if (p_if->l2protocol.obj_list)
        {
            p_field->field_id[1] = FLD_ID_ALL;
            CTCLIB_SLIST_LOOP(p_if->l2protocol.obj_list, p_db_l2protocol, listnode)
            {
                ds_l2protocol_dump_one(p_db_l2protocol, pargs);
                sal_fprintf(fp, "/,");
            }
        }
        sal_fprintf(fp, "}");
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_CRC_ERROR_CHECK_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_CRC_ERROR_CHECK_EN].name,
            p_if->crc_error_check_en);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_CRC_RECALCULATION_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_CRC_RECALCULATION_EN].name,
            p_if->crc_recalculation_en);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_MLAG_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_MLAG_ID].name,
            p_if->mlag_id);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_ERPS_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_ERPS_ENABLE].name,
            p_if->erps_enable);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_ERPS_PDU_ACTION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_ERPS_PDU_ACTION].name,
            p_if->erps_pdu_action);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_DOT1X_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_DOT1X_ENABLE].name,
            p_if->dot1x_enable);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_DOT1X_UNAUTHORIZED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_DOT1X_UNAUTHORIZED].name,
            GLB_FLAG_ISSET(p_if->dot1x_flag, GLB_IF_DOT1X_UNAUTHED));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_DOT1X_DIR_IN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_DOT1X_DIR_IN].name,
            GLB_FLAG_ISSET(p_if->dot1x_flag, GLB_IF_DOT1X_DIR_IN));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_DOT1X_DIR_OUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_DOT1X_DIR_OUT].name,
            GLB_FLAG_ISSET(p_if->dot1x_flag, GLB_IF_DOT1X_DIR_OUT));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_DOT1X_MODE_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_DOT1X_MODE_MAC].name,
            GLB_FLAG_ISSET(p_if->dot1x_flag, GLB_IF_DOT1X_MODE_MAC));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_DOT1X_GUEST_VLAN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_DOT1X_GUEST_VLAN].name,
            p_if->dot1x_guest_vlan);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_VLANCLASS_GROUP_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_VLANCLASS_GROUP_ID].name,
            p_if->vlanclass_group_id);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_VLANCLASS_ACTIVE_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_VLANCLASS_ACTIVE_TYPE].name,
            p_if->vlanclass_active_type);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_VLANCLASS_DEFAULT_ACTION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_VLANCLASS_DEFAULT_ACTION].name,
            p_if->vlanclass_default_action);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_DOT1X_CFG_GUEST_VLAN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_DOT1X_CFG_GUEST_VLAN].name,
            p_if->dot1x_cfg_guest_vlan);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_EGS_ACL_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_EGS_ACL_EN].name,
            p_if->egs_acl_en);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_TAP_GROUP_IGS_REF_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_TAP_GROUP_IGS_REF_CNT].name,
            p_if->tap_group_igs_ref_cnt);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_TAP_GROUP_EGS_REF_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_TAP_GROUP_EGS_REF_CNT].name,
            p_if->tap_group_egs_ref_cnt);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_TAP_FLOW_DECAP_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_TAP_FLOW_DECAP_CNT].name,
            p_if->tap_flow_decap_cnt);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_TAP_FLOW_UDF_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_TAP_FLOW_UDF_CNT].name,
            p_if->tap_flow_udf_cnt);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_METADATA_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_METADATA_ENABLE].name,
            p_if->metadata_enable);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_ARPINSP_TRUST_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_ARPINSP_TRUST_ENABLE].name,
            p_if->arpinsp_trust_enable);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_ARP_NUMBERLIMIT_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_ARP_NUMBERLIMIT_ENABLE].name,
            p_if->arp_numberlimit_enable);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_ARP_NUMBERLIMIT_NUMBER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_ARP_NUMBERLIMIT_NUMBER].name,
            p_if->arp_numberlimit_number);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_ARP_NUMBERLIMIT_VIOLATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_ARP_NUMBERLIMIT_VIOLATE].name,
            p_if->arp_numberlimit_violate);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_ARP_NUMBERLIMIT_OCCUR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_ARP_NUMBERLIMIT_OCCUR].name,
            p_if->arp_numberlimit_occur);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_ARP_RATE_LIMIT_ERRDISABLE_OCCUR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_ARP_RATE_LIMIT_ERRDISABLE_OCCUR].name,
            p_if->arp_rate_limit_errdisable_occur);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_INBAND_SNAT_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_INBAND_SNAT_TYPE].name,
            p_if->inband_snat_type);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_LATENCY_EVENT_HIGH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_LATENCY_EVENT_HIGH].name,
            p_if->latency_event_high);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_LATENCY_EVENT_LOW, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_LATENCY_EVENT_LOW].name,
            p_if->latency_event_low);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_LATENCY_EVENT_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_LATENCY_EVENT_ENABLE].name,
            p_if->latency_event_enable);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_REFLECTIVE_BRIDGE_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_REFLECTIVE_BRIDGE_EN].name,
            p_if->reflective_bridge_en);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SUPPORT_SSM_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_SUPPORT_SSM_EN].name,
            p_if->support_ssm_en);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_SVLAN_TPID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_SVLAN_TPID].name,
            p_if->svlan_tpid);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_L2PRO_IF_ACTION_VLAN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_INTERFACE_FLD_L2PRO_IF_ACTION_VLAN].name, 
            cdb_l2pro_if_action_vlan_val2str(buf, MAX_CMD_STR_LEN, &p_if->l2pro_if_action_vlan));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_L2PRO_UPLINK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_L2PRO_UPLINK].name,
            p_if->l2pro_uplink);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_STATIC_AGG_PRIORITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_STATIC_AGG_PRIORITY].name,
            p_if->static_agg_priority);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_STATIC_AGG_IS_ACTIVE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_STATIC_AGG_IS_ACTIVE].name,
            p_if->static_agg_is_active);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_FEC_STATUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_FEC_STATUS].name,
            p_if->fec_status);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_CL73_ABILITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_CL73_ABILITY].name,
            p_if->cl73_ability);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_IFG_STATS_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_IFG_STATS_EN].name,
            p_if->ifg_stats_en);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_OPENFLOW_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_OPENFLOW_EN].name,
            p_if->openflow_en);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_OPENFLOW_OPERATION_AGG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_OPENFLOW_OPERATION_AGG].name,
            p_if->openflow_operation_agg);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_OPENFLOW_OPERATION_AGG_MEM_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_OPENFLOW_OPERATION_AGG_MEM_IFINDEX].name,
            p_if->openflow_operation_agg_mem_ifindex);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_CARRIER_DOWN_HOLDTIME_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_CARRIER_DOWN_HOLDTIME_INTERVAL].name,
            p_if->carrier_down_holdtime_interval);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_HASH_VALUE_PROFILE_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_HASH_VALUE_PROFILE_ID].name,
            p_if->hash_value_profile_id);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_HASH_VALUE_PROFILE_DIR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_HASH_VALUE_PROFILE_DIR].name,
            p_if->hash_value_profile_dir);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_HASH_VALUE_PROFILE_ID_AGG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_HASH_VALUE_PROFILE_ID_AGG].name,
            p_if->hash_value_profile_id_agg);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_HASH_VALUE_AGG_HASH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_HASH_VALUE_AGG_HASH].name,
            p_if->hash_value_agg_hash);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_POE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_INTERFACE_FLD_POE].name, 
            cdb_poe_cfg_val2str(buf, MAX_CMD_STR_LEN, &p_if->poe));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_CURRENT_G8032_RING_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_CURRENT_G8032_RING_COUNT].name,
            p_if->current_g8032_ring_count);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_G8032_RING_FIRST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_INTERFACE_FLD_G8032_RING_FIRST].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_if->g8032_ring_first, sizeof(p_if->g8032_ring_first)/4));
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_CFM_LMEP_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_CFM_LMEP_NUM].name,
            p_if->cfm_lmep_num);
    }
    if (FLD_MATCH(TBL_INTERFACE_FLD_CFM_RMEP_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_INTERFACE_FLD_CFM_RMEP_NUM].name,
            p_if->cfm_rmep_num);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_interface_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_interface_master_t *p_master = _g_p_tbl_interface_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->if_list, p_db_if, listnode, next)
    {
        rc |= fn(p_db_if, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_interface_master_t*
tbl_interface_get_master()
{
    return _g_p_tbl_interface_master;
}

tbl_interface_master_t*
tbl_interface_init_interface()
{
    _g_p_tbl_interface_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_interface_master_t));
    _g_p_tbl_interface_master->if_hash = ctclib_hash_create(_tbl_interface_hash_make, _tbl_interface_hash_cmp);
    _g_p_tbl_interface_master->if_list = ctclib_slist_create(tbl_interface_cmp, NULL);
    return _g_p_tbl_interface_master;
}

#define CTC_IFINDEX_VALID_CHECK(_ifindex, _min, _max, _name)  \
if (_ifindex < _min || _ifindex > _max)\
{\
    return (-1);\
}

int32
tbl_interface_get_ifindex_by_name(const char *ifname)
{
    int32 ifindex = 0;
    int32 rc = PM_E_NONE;

    if (0 == sal_memcmp(ifname, GLB_IFNAME_ETH_PREFIX, GLB_IFNAME_ETH_PREFIX_LEN))
    {
#if 0
        int32 slot_index = 0;
        rc = sal_sscanf(ifname, GLB_IFNAME_ETH_PREFIX"%u-%u", &slot_index, &ifindex);
        ifindex = GLB_PHY_IFINDEX_MIN + (slot_index * GLB_PORT_NUM_PER_SLOT + ifindex);
        CTC_IFINDEX_VALID_CHECK(ifindex, GLB_PHY_IFINDEX_MIN, GLB_PHY_IFINDEX_MAX, ifname);
#else
        ifindex = if_get_ifindex_by_name(ifname);
#endif
        return ifindex;
    }
    else if (0 == sal_memcmp(ifname, GLB_IFNAME_AGG_PREFIX, GLB_IFNAME_AGG_PREFIX_LEN))
    {
        rc = sal_sscanf(ifname, GLB_IFNAME_AGG_PREFIX"%u", &ifindex);
        ifindex += GLB_AGG_IFINDEX_MIN;
        CTC_IFINDEX_VALID_CHECK(ifindex, GLB_AGG_IFINDEX_MIN, GLB_AGG_IFINDEX_MAX, ifname);
        return ifindex;
    }
    else if (0 == sal_memcmp(ifname, GLB_IFNAME_VLAN_PREFIX, GLB_IFNAME_VLAN_PREFIX_LEN))
    {
        rc = sal_sscanf(ifname, GLB_IFNAME_VLAN_PREFIX"%u", &ifindex);
        ifindex += GLB_VLAN_IFINDEX_MIN;
        CTC_IFINDEX_VALID_CHECK(ifindex, GLB_VLAN_IFINDEX_MIN, GLB_VLAN_IFINDEX_MAX, ifname);
        return ifindex;
    }
    else if (0 == sal_memcmp(ifname, GLB_IFNAME_TUNNEL_PREFIX, GLB_IFNAME_TUNNEL_PREFIX_LEN))
    {
        rc = sal_sscanf(ifname, GLB_IFNAME_TUNNEL_PREFIX"%u", &ifindex);
        ifindex += GLB_TUNNEL_IFINDEX_MIN;
        CTC_IFINDEX_VALID_CHECK(ifindex, GLB_TUNNEL_IFINDEX_MIN, GLB_TUNNEL_IFINDEX_MAX, ifname);
        return ifindex;
    }
    else if (0 == sal_memcmp(ifname, GLB_IFNAME_LOOPBACK_PREFIX, GLB_IFNAME_LOOPBACK_PREFIX_LEN))
    {
        rc = sal_sscanf(ifname, GLB_IFNAME_LOOPBACK_PREFIX"%u", &ifindex);
        ifindex += GLB_LB_IFINDEX_MIN;
        CTC_IFINDEX_VALID_CHECK(ifindex, GLB_LB_IFINDEX_MIN, GLB_LB_IFINDEX_MAX, ifname);
        return ifindex;
    }
    else if (0 == sal_memcmp(ifname, GLB_IFNAME_L2GRE_PREFIX, GLB_IFNAME_L2GRE_PREFIX_LEN))
    {
        rc = sal_sscanf(ifname, GLB_IFNAME_L2GRE_PREFIX"%u", &ifindex);
        ifindex += GLB_L2GRE_IFINDEX_MIN;
        CTC_IFINDEX_VALID_CHECK(ifindex, GLB_L2GRE_IFINDEX_MIN, GLB_L2GRE_IFINDEX_MAX, ifname);
        return ifindex;
    }
    else if (0 == sal_memcmp(ifname, GLB_IFNAME_L3GRE_PREFIX, GLB_IFNAME_L3GRE_PREFIX_LEN))
    {
        rc = sal_sscanf(ifname, GLB_IFNAME_L3GRE_PREFIX"%u", &ifindex);
        ifindex += GLB_L3GRE_IFINDEX_MIN;
        CTC_IFINDEX_VALID_CHECK(ifindex, GLB_L3GRE_IFINDEX_MIN, GLB_L3GRE_IFINDEX_MAX, ifname);
        return ifindex;
    }
    else if (0 == sal_memcmp(ifname, GLB_IFNAME_NVGRE_PREFIX, GLB_IFNAME_NVGRE_PREFIX_LEN))
    {
        rc = sal_sscanf(ifname, GLB_IFNAME_NVGRE_PREFIX"%u", &ifindex);
        ifindex += GLB_NVGRE_IFINDEX_MIN;
        CTC_IFINDEX_VALID_CHECK(ifindex, GLB_NVGRE_IFINDEX_MIN, GLB_NVGRE_IFINDEX_MAX, ifname);
        return ifindex;
    }
    else if (0 == sal_memcmp(ifname, GLB_IFNAME_VXLAN_PREFIX, GLB_IFNAME_VXLAN_PREFIX_LEN))
    {
        rc = sal_sscanf(ifname, GLB_IFNAME_VXLAN_PREFIX"%u", &ifindex);
        ifindex += GLB_VXLAN_IFINDEX_MIN;
        CTC_IFINDEX_VALID_CHECK(ifindex, GLB_VXLAN_IFINDEX_MIN, GLB_VXLAN_IFINDEX_MAX, ifname);
        return ifindex;
    }
    else if (0 == sal_memcmp(ifname, GLB_IFNAME_CPU_PREFIX, GLB_IFNAME_CPU_PREFIX_LEN))
    {
        ifindex = GLB_CPU_IFINDEX_MIN;
        return ifindex;
    }
    else
    {
        return PM_E_INVALID_PARAM;
    }
    
    (void)rc;
    return PM_E_NONE;
}

uint32
tbl_interface_get_type_by_ifindex(uint32 ifindex)
{
    if (GLB_PHY_IFINDEX_MIN < ifindex && ifindex < GLB_PHY_IFINDEX_MAX)
    {
        return GLB_IF_TYPE_PORT_IF;
    }
    else if (GLB_AGG_IFINDEX_MIN < ifindex && ifindex < GLB_AGG_IFINDEX_MAX)
    {
        return GLB_IF_TYPE_LINKAGG_IF;
    }
    else if (GLB_VLAN_IFINDEX_MIN < ifindex && ifindex < GLB_VLAN_IFINDEX_MAX)
    {
        return GLB_IF_TYPE_VLAN_IF;
    }
    else if (GLB_TUNNEL_IFINDEX_MIN < ifindex && ifindex < GLB_TUNNEL_IFINDEX_MAX)
    {
        return GLB_IF_TYPE_TUNNEL_IF;
    }
    else if (GLB_LB_IFINDEX_MIN <= ifindex && ifindex < GLB_LB_IFINDEX_MAX)
    {
        return GLB_IF_TYPE_LOOPBACK_IF;
    }
    else if (GLB_L2GRE_IFINDEX_MIN < ifindex && ifindex < GLB_L2GRE_IFINDEX_MAX)
    {
        return GLB_IF_TYPE_L2GRE_IF;
    }
    else if (GLB_L3GRE_IFINDEX_MIN < ifindex && ifindex < GLB_L3GRE_IFINDEX_MAX)
    {
        return GLB_IF_TYPE_L3GRE_IF;
    }
    else if (GLB_NVGRE_IFINDEX_MIN < ifindex && ifindex < GLB_NVGRE_IFINDEX_MAX)
    {
        return GLB_IF_TYPE_NVGRE_IF;
    }
    else if (GLB_VXLAN_IFINDEX_MIN < ifindex && ifindex < GLB_VXLAN_IFINDEX_MAX)
    {
        return GLB_IF_TYPE_VXLAN_IF;
    }
    else if (GLB_CPU_IFINDEX_MIN == ifindex)
    {
        return GLB_IF_TYPE_CPU_IF;
    }
    else
    {
        return GLB_IF_TYPE_NULL;
    }
    
    return GLB_IF_TYPE_NULL;
}

vid_t
tbl_interface_get_vid_by_ifindex(uint32 ifindex)
{
    if (GLB_VLAN_IFINDEX_MIN < ifindex && ifindex < GLB_VLAN_IFINDEX_MAX)
    {
        return (ifindex - GLB_VLAN_IFINDEX_MIN);
    }
    else
    {
        return GLB_ALL_VLAN_ID;
    }
}

int32
tbl_interface_get_name_by_ifindex(uint32 ifindex, char *name, uint32 len)
{
    int32 index = 0;

    sal_memset(name, 0, len);
    if (GLB_PHY_IFINDEX_MIN < ifindex && ifindex < GLB_PHY_IFINDEX_MAX)
    {
#if 0
        int32 slot_index = 0;
        slot_index = ifindex / GLB_PORT_NUM_PER_SLOT;
        index = ifindex % GLB_PORT_NUM_PER_SLOT;
        sal_snprintf(name, len, GLB_IFNAME_ETH_PREFIX"%u-%u", slot_index, index);
#else
        if_get_name_by_ifindex(ifindex, name, len);
#endif
    }
    else if (GLB_AGG_IFINDEX_MIN < ifindex && ifindex < GLB_AGG_IFINDEX_MAX)
    {
        index = ifindex - GLB_AGG_IFINDEX_MIN;
        sal_snprintf(name, len, GLB_IFNAME_AGG_PREFIX"%u", index);
    }
    else if (GLB_VLAN_IFINDEX_MIN < ifindex && ifindex < GLB_VLAN_IFINDEX_MAX)
    {
        index = ifindex - GLB_VLAN_IFINDEX_MIN;
        sal_snprintf(name, len, GLB_IFNAME_VLAN_PREFIX"%u", index);
    }
    else if (GLB_TUNNEL_IFINDEX_MIN < ifindex && ifindex < GLB_TUNNEL_IFINDEX_MAX)
    {
        index = ifindex - GLB_TUNNEL_IFINDEX_MIN;
        sal_snprintf(name, len, GLB_IFNAME_TUNNEL_PREFIX"%u", index);
    }
    else if (GLB_LB_IFINDEX_MIN < ifindex && ifindex < GLB_LB_IFINDEX_MAX)
    {
        index = ifindex - GLB_LB_IFINDEX_MIN;
        sal_snprintf(name, len, GLB_IFNAME_LOOPBACK_PREFIX"%u", index);
    }
    else if (GLB_L2GRE_IFINDEX_MIN < ifindex && ifindex < GLB_L2GRE_IFINDEX_MAX)
    {
        index = ifindex - GLB_L2GRE_IFINDEX_MIN;
        sal_snprintf(name, len, GLB_IFNAME_L2GRE_PREFIX"%u", index);
    }
    else if (GLB_L3GRE_IFINDEX_MIN < ifindex && ifindex < GLB_L3GRE_IFINDEX_MAX)
    {
        index = ifindex - GLB_L3GRE_IFINDEX_MIN;
        sal_snprintf(name, len, GLB_IFNAME_L3GRE_PREFIX"%u", index);
    }
    else if (GLB_NVGRE_IFINDEX_MIN < ifindex && ifindex < GLB_NVGRE_IFINDEX_MAX)
    {
        index = ifindex - GLB_NVGRE_IFINDEX_MIN;
        sal_snprintf(name, len, GLB_IFNAME_NVGRE_PREFIX"%u", index);
    }
    else if (GLB_VXLAN_IFINDEX_MIN < ifindex && ifindex < GLB_VXLAN_IFINDEX_MAX)
    {
        index = ifindex - GLB_VXLAN_IFINDEX_MIN;
        sal_snprintf(name, len, GLB_IFNAME_VXLAN_PREFIX"%u", index);
    }
    else if (GLB_CPU_IFINDEX_MIN == ifindex)
    {
        sal_snprintf(name, len, GLB_IFNAME_CPU_PREFIX);
    }
    else
    {
        return PM_E_INVALID_PARAM;
    }

    return PM_E_NONE;
}

tbl_interface_t*
tbl_interface_get_interface_by_name(const char* ifname)
{
    tbl_interface_key_t key;

    sal_memset(&key, 0, sizeof(key));
    sal_strncpy(key.name, ifname, IFNAME_SIZE);
    return tbl_interface_get_interface(&key);
}

tbl_interface_t*
tbl_interface_get_interface_by_ifindex(uint32 ifindex)
{
    tbl_interface_key_t key;
    int32 rc = PM_E_NONE;

    sal_memset(&key, 0, sizeof(key));
    rc = tbl_interface_get_name_by_ifindex(ifindex, key.name, IFNAME_SIZE);
    if (rc < 0)
    {
        return NULL;
    }

    return tbl_interface_get_interface(&key);
}

uint32
tbl_interface_is_up(tbl_interface_t *p_if)
{
    return (p_if->flags & GLB_IFF_UP);
}

uint32
tbl_interface_is_running(tbl_interface_t *p_if)
{
    if (GLB_IF_TYPE_LOOPBACK_IF == p_if->hw_type)
    {
        return tbl_interface_is_up(p_if);
    }

    return (p_if->flags & (GLB_IFF_UP|GLB_IFF_RUNNING)) == (GLB_IFF_UP|GLB_IFF_RUNNING);
}

tbl_interface_t*
tbl_interface_get_interface_by_port_id(uint64 port_id)
{
    tbl_interface_master_t *p_master = _g_p_tbl_interface_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_interface_t *p_db_if = NULL;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->if_list, p_db_if, listnode, next)
    {
        if(p_db_if->portid == port_id)
        {
            return p_db_if;
        }
    }

    return NULL;
}

int32
tbl_interface_get_lag_1st_member_ifindex(uint32 lag_ifindex, uint32 *mem_ifindex)
{
    tbl_interface_t *p_db_lag = NULL;
    tbl_interface_t *p_db_mem = NULL;
    ctclib_slistnode_t *listnode = NULL;
    
    p_db_lag = tbl_interface_get_interface_by_ifindex(lag_ifindex);
    if ((NULL == p_db_lag) || (NULL == p_db_lag->lag))
    {
        return PM_E_FAIL; 
    }
    if (GLB_IF_TYPE_LINKAGG_IF != p_db_lag->hw_type)
    {
        return PM_E_FAIL; 
    }
    
    CTCLIB_SLIST_LOOP(p_db_lag->lag->member_ports.obj_list, p_db_mem, listnode)
    {
        if (NULL == p_db_mem->lag)
        {
            continue;
        }
        if (p_db_mem->lag->oper_state)
        {
            *mem_ifindex = p_db_mem->ifindex;
            return PM_E_NONE;
        }
    }
    
    return PM_E_FAIL;
}


