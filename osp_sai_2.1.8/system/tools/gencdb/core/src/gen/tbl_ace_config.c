
#include "proto.h"
#include "gen/tbl_ace_config_define.h"
#include "gen/tbl_ace_config.h"
#include "cdb_data_cmp.h"

tbl_ace_config_master_t *_g_p_tbl_ace_config_master = NULL;

static uint32
_tbl_ace_config_hash_make(tbl_ace_config_t *p_ace_config)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_ace_config->key;
    for (index = 0; index < sizeof(p_ace_config->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_ace_config_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_ace_config_t *p_ace_config1 = (tbl_ace_config_t*)p_arg1;
    tbl_ace_config_t *p_ace_config2 = (tbl_ace_config_t*)p_arg2;

    if (0 == sal_memcmp(&p_ace_config1->key, &p_ace_config2->key, sizeof(tbl_ace_config_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_ace_config_add_ace_config_sync(tbl_ace_config_t *p_ace_config, uint32 sync)
{
    tbl_ace_config_master_t *p_master = _g_p_tbl_ace_config_master;
    tbl_ace_config_t *p_db_ace_config = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_ace_config_get_ace_config(&p_ace_config->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_ace_config = XCALLOC(MEM_TBL_ACE_CONFIG, sizeof(tbl_ace_config_t));
    if (NULL == p_db_ace_config)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_ace_config, p_ace_config, sizeof(tbl_ace_config_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->ace_config_hash, (void*)p_db_ace_config, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->ace_config_list, p_db_ace_config);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_ACE_CONFIG, p_db_ace_config);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_ACE_CONFIG, p_db_ace_config);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ace_config_del_ace_config_sync(tbl_ace_config_key_t *p_ace_config_key, uint32 sync)
{
    tbl_ace_config_master_t *p_master = _g_p_tbl_ace_config_master;
    tbl_ace_config_t *p_db_ace_config = NULL;

    /* 1. lookup entry exist */
    p_db_ace_config = tbl_ace_config_get_ace_config(p_ace_config_key);
    if (NULL == p_db_ace_config)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_ACE_CONFIG, p_db_ace_config);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_ACE_CONFIG, p_db_ace_config);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->ace_config_hash, (void*)p_db_ace_config);
    ctclib_slistnode_delete(p_master->ace_config_list, p_db_ace_config);

    /* 4. free db entry */
    XFREE(MEM_TBL_ACE_CONFIG, p_db_ace_config);

    return PM_E_NONE;
}

int32
tbl_ace_config_set_ace_config_field_sync(tbl_ace_config_t *p_ace_config, tbl_ace_config_field_id_t field_id, uint32 sync)
{
    tbl_ace_config_t *p_db_ace_config = NULL;

    /* 1. lookup entry exist */
    p_db_ace_config = tbl_ace_config_get_ace_config(&p_ace_config->key);
    if (NULL == p_db_ace_config)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_ACE_CONFIG_FLD_KEY_FLAGS:
        p_db_ace_config->key_flags = p_ace_config->key_flags;
        break;

    case TBL_ACE_CONFIG_FLD_KEY_FLAGS2:
        p_db_ace_config->key_flags2 = p_ace_config->key_flags2;
        break;

    case TBL_ACE_CONFIG_FLD_KEY_IPV6_FLAGS:
        p_db_ace_config->key_ipv6_flags = p_ace_config->key_ipv6_flags;
        break;

    case TBL_ACE_CONFIG_FLD_IS_IPV6:
        p_db_ace_config->is_ipv6 = p_ace_config->is_ipv6;
        break;

    case TBL_ACE_CONFIG_FLD_ACL_TYPE:
        p_db_ace_config->acl_type = p_ace_config->acl_type;
        break;

    case TBL_ACE_CONFIG_FLD_KEY_ARP_FLAGS:
        p_db_ace_config->key_arp_flags = p_ace_config->key_arp_flags;
        break;

    case TBL_ACE_CONFIG_FLD_ARP_OP_CODE:
        p_db_ace_config->arp_op_code = p_ace_config->arp_op_code;
        break;

    case TBL_ACE_CONFIG_FLD_ARP_OP_CODE_MASK:
        p_db_ace_config->arp_op_code_mask = p_ace_config->arp_op_code_mask;
        break;

    case TBL_ACE_CONFIG_FLD_TIME_RANGE:
        sal_memcpy(p_db_ace_config->time_range, p_ace_config->time_range, sizeof(p_ace_config->time_range));
        break;

    case TBL_ACE_CONFIG_FLD_APPLY_CNT:
        p_db_ace_config->apply_cnt = p_ace_config->apply_cnt;
        break;

    case TBL_ACE_CONFIG_FLD_IN_PORT:
        p_db_ace_config->in_port = p_ace_config->in_port;
        break;

    case TBL_ACE_CONFIG_FLD_OUT_PORT:
        p_db_ace_config->out_port = p_ace_config->out_port;
        break;

    case TBL_ACE_CONFIG_FLD_ETHER_TYPE:
        p_db_ace_config->ether_type = p_ace_config->ether_type;
        break;

    case TBL_ACE_CONFIG_FLD_ETHER_TYPE_MASK:
        p_db_ace_config->ether_type_mask = p_ace_config->ether_type_mask;
        break;

    case TBL_ACE_CONFIG_FLD_SRC_MAC:
        sal_memcpy(p_db_ace_config->src_mac, p_ace_config->src_mac, sizeof(p_ace_config->src_mac));
        break;

    case TBL_ACE_CONFIG_FLD_SRC_MAC_MASK:
        sal_memcpy(p_db_ace_config->src_mac_mask, p_ace_config->src_mac_mask, sizeof(p_ace_config->src_mac_mask));
        break;

    case TBL_ACE_CONFIG_FLD_DST_MAC:
        sal_memcpy(p_db_ace_config->dst_mac, p_ace_config->dst_mac, sizeof(p_ace_config->dst_mac));
        break;

    case TBL_ACE_CONFIG_FLD_DST_MAC_MASK:
        sal_memcpy(p_db_ace_config->dst_mac_mask, p_ace_config->dst_mac_mask, sizeof(p_ace_config->dst_mac_mask));
        break;

    case TBL_ACE_CONFIG_FLD_SVLAN:
        p_db_ace_config->svlan = p_ace_config->svlan;
        break;

    case TBL_ACE_CONFIG_FLD_SVLAN_MASK:
        p_db_ace_config->svlan_mask = p_ace_config->svlan_mask;
        break;

    case TBL_ACE_CONFIG_FLD_SVLAN_COS:
        p_db_ace_config->svlan_cos = p_ace_config->svlan_cos;
        break;

    case TBL_ACE_CONFIG_FLD_SVLAN_COS_MASK:
        p_db_ace_config->svlan_cos_mask = p_ace_config->svlan_cos_mask;
        break;

    case TBL_ACE_CONFIG_FLD_CVLAN:
        p_db_ace_config->cvlan = p_ace_config->cvlan;
        break;

    case TBL_ACE_CONFIG_FLD_CVLAN_MASK:
        p_db_ace_config->cvlan_mask = p_ace_config->cvlan_mask;
        break;

    case TBL_ACE_CONFIG_FLD_CVLAN_COS:
        p_db_ace_config->cvlan_cos = p_ace_config->cvlan_cos;
        break;

    case TBL_ACE_CONFIG_FLD_CVLAN_COS_MASK:
        p_db_ace_config->cvlan_cos_mask = p_ace_config->cvlan_cos_mask;
        break;

    case TBL_ACE_CONFIG_FLD_UNTAG_VLAN:
        p_db_ace_config->untag_vlan = p_ace_config->untag_vlan;
        break;

    case TBL_ACE_CONFIG_FLD_IP_TYPE:
        p_db_ace_config->ip_type = p_ace_config->ip_type;
        break;

    case TBL_ACE_CONFIG_FLD_IP_TYPE_MASK:
        p_db_ace_config->ip_type_mask = p_ace_config->ip_type_mask;
        break;

    case TBL_ACE_CONFIG_FLD_IP_FLAGS:
        p_db_ace_config->ip_flags = p_ace_config->ip_flags;
        break;

    case TBL_ACE_CONFIG_FLD_IP_FLAGS_MASK:
        p_db_ace_config->ip_flags_mask = p_ace_config->ip_flags_mask;
        break;

    case TBL_ACE_CONFIG_FLD_SRC_IP:
        sal_memcpy(&p_db_ace_config->src_ip, &p_ace_config->src_ip, sizeof(p_ace_config->src_ip));
        break;

    case TBL_ACE_CONFIG_FLD_SRC_IP_MASK:
        sal_memcpy(&p_db_ace_config->src_ip_mask, &p_ace_config->src_ip_mask, sizeof(p_ace_config->src_ip_mask));
        break;

    case TBL_ACE_CONFIG_FLD_DST_IP:
        sal_memcpy(&p_db_ace_config->dst_ip, &p_ace_config->dst_ip, sizeof(p_ace_config->dst_ip));
        break;

    case TBL_ACE_CONFIG_FLD_DST_IP_MASK:
        sal_memcpy(&p_db_ace_config->dst_ip_mask, &p_ace_config->dst_ip_mask, sizeof(p_ace_config->dst_ip_mask));
        break;

    case TBL_ACE_CONFIG_FLD_IPV6_FLOW_LABEL:
        p_db_ace_config->ipv6_flow_label = p_ace_config->ipv6_flow_label;
        break;

    case TBL_ACE_CONFIG_FLD_IPV6_FLOW_LABEL_MASK:
        p_db_ace_config->ipv6_flow_label_mask = p_ace_config->ipv6_flow_label_mask;
        break;

    case TBL_ACE_CONFIG_FLD_DSCP:
        p_db_ace_config->dscp = p_ace_config->dscp;
        break;

    case TBL_ACE_CONFIG_FLD_DSCP_MASK:
        p_db_ace_config->dscp_mask = p_ace_config->dscp_mask;
        break;

    case TBL_ACE_CONFIG_FLD_IP_PRECEDENCE:
        p_db_ace_config->ip_precedence = p_ace_config->ip_precedence;
        break;

    case TBL_ACE_CONFIG_FLD_IP_PRECEDENCE_MASK:
        p_db_ace_config->ip_precedence_mask = p_ace_config->ip_precedence_mask;
        break;

    case TBL_ACE_CONFIG_FLD_IP_PROTOCOL:
        p_db_ace_config->ip_protocol = p_ace_config->ip_protocol;
        break;

    case TBL_ACE_CONFIG_FLD_IP_PROTOCOL_MASK:
        p_db_ace_config->ip_protocol_mask = p_ace_config->ip_protocol_mask;
        break;

    case TBL_ACE_CONFIG_FLD_L4_SRC_PORT_TYPE:
        p_db_ace_config->l4_src_port_type = p_ace_config->l4_src_port_type;
        break;

    case TBL_ACE_CONFIG_FLD_L4_SRC_PORT:
        p_db_ace_config->l4_src_port = p_ace_config->l4_src_port;
        break;

    case TBL_ACE_CONFIG_FLD_L4_SRC_PORT_MASK:
        p_db_ace_config->l4_src_port_mask = p_ace_config->l4_src_port_mask;
        break;

    case TBL_ACE_CONFIG_FLD_L4_DST_PORT_TYPE:
        p_db_ace_config->l4_dst_port_type = p_ace_config->l4_dst_port_type;
        break;

    case TBL_ACE_CONFIG_FLD_L4_DST_PORT:
        p_db_ace_config->l4_dst_port = p_ace_config->l4_dst_port;
        break;

    case TBL_ACE_CONFIG_FLD_L4_DST_PORT_MASK:
        p_db_ace_config->l4_dst_port_mask = p_ace_config->l4_dst_port_mask;
        break;

    case TBL_ACE_CONFIG_FLD_IP_FRAG:
        p_db_ace_config->ip_frag = p_ace_config->ip_frag;
        break;

    case TBL_ACE_CONFIG_FLD_TCP_FLAGS:
        p_db_ace_config->tcp_flags = p_ace_config->tcp_flags;
        break;

    case TBL_ACE_CONFIG_FLD_TCP_FLAGS_MASK:
        p_db_ace_config->tcp_flags_mask = p_ace_config->tcp_flags_mask;
        break;

    case TBL_ACE_CONFIG_FLD_IGMP_TYPE:
        p_db_ace_config->igmp_type = p_ace_config->igmp_type;
        break;

    case TBL_ACE_CONFIG_FLD_ICMP_TYPE:
        p_db_ace_config->icmp_type = p_ace_config->icmp_type;
        break;

    case TBL_ACE_CONFIG_FLD_ICMP_CODE:
        p_db_ace_config->icmp_code = p_ace_config->icmp_code;
        break;

    case TBL_ACE_CONFIG_FLD_UDF_ID:
        p_db_ace_config->udf_id = p_ace_config->udf_id;
        break;

    case TBL_ACE_CONFIG_FLD_UDF_TYPE:
        p_db_ace_config->udf_type = p_ace_config->udf_type;
        break;

    case TBL_ACE_CONFIG_FLD_UDF_OFFSET:
        p_db_ace_config->udf_offset = p_ace_config->udf_offset;
        break;

    case TBL_ACE_CONFIG_FLD_UDF_VALUE:
        sal_memcpy(p_db_ace_config->udf_value, p_ace_config->udf_value, sizeof(p_ace_config->udf_value));
        break;

    case TBL_ACE_CONFIG_FLD_UDF_MASK:
        sal_memcpy(p_db_ace_config->udf_mask, p_ace_config->udf_mask, sizeof(p_ace_config->udf_mask));
        break;

    case TBL_ACE_CONFIG_FLD_UDF_VALUE_VALID:
        sal_memcpy(p_db_ace_config->udf_value_valid, p_ace_config->udf_value_valid, sizeof(p_ace_config->udf_value_valid));
        break;

    case TBL_ACE_CONFIG_FLD_ERSPAN_ID:
        p_db_ace_config->erspan_id = p_ace_config->erspan_id;
        break;

    case TBL_ACE_CONFIG_FLD_ERSPAN_ID_MASK:
        p_db_ace_config->erspan_id_mask = p_ace_config->erspan_id_mask;
        break;

    case TBL_ACE_CONFIG_FLD_STRIP_HEADER_POS:
        p_db_ace_config->strip_header_pos = p_ace_config->strip_header_pos;
        break;

    case TBL_ACE_CONFIG_FLD_STRIP_HEADER_OFFSET:
        p_db_ace_config->strip_header_offset = p_ace_config->strip_header_offset;
        break;

    case TBL_ACE_CONFIG_FLD_L4_VXLAN_VNI:
        p_db_ace_config->l4_vxlan_vni = p_ace_config->l4_vxlan_vni;
        break;

    case TBL_ACE_CONFIG_FLD_L4_VXLAN_VNI_MASK:
        p_db_ace_config->l4_vxlan_vni_mask = p_ace_config->l4_vxlan_vni_mask;
        break;

    case TBL_ACE_CONFIG_FLD_L4_GRE_TYPE:
        p_db_ace_config->l4_gre_type = p_ace_config->l4_gre_type;
        break;

    case TBL_ACE_CONFIG_FLD_L4_GRE_KEY:
        p_db_ace_config->l4_gre_key = p_ace_config->l4_gre_key;
        break;

    case TBL_ACE_CONFIG_FLD_L4_GRE_KEY_MASK:
        p_db_ace_config->l4_gre_key_mask = p_ace_config->l4_gre_key_mask;
        break;

    case TBL_ACE_CONFIG_FLD_L2GRE_ADD_HEADER:
        p_db_ace_config->l2gre_add_header = p_ace_config->l2gre_add_header;
        break;

    case TBL_ACE_CONFIG_FLD_L2GRE_SRC_IP:
        sal_memcpy(&p_db_ace_config->l2gre_src_ip, &p_ace_config->l2gre_src_ip, sizeof(p_ace_config->l2gre_src_ip));
        break;

    case TBL_ACE_CONFIG_FLD_L2GRE_DST_IP:
        sal_memcpy(&p_db_ace_config->l2gre_dst_ip, &p_ace_config->l2gre_dst_ip, sizeof(p_ace_config->l2gre_dst_ip));
        break;

    case TBL_ACE_CONFIG_FLD_L2GRE_DST_MAC:
        sal_memcpy(p_db_ace_config->l2gre_dst_mac, p_ace_config->l2gre_dst_mac, sizeof(p_ace_config->l2gre_dst_mac));
        break;

    case TBL_ACE_CONFIG_FLD_L2GRE_KEY_NUM:
        p_db_ace_config->l2gre_key_num = p_ace_config->l2gre_key_num;
        break;

    case TBL_ACE_CONFIG_FLD_L2GRE_KEY_LEN:
        p_db_ace_config->l2gre_key_len = p_ace_config->l2gre_key_len;
        break;

    case TBL_ACE_CONFIG_FLD_L3GRE_ADD_HEADER:
        p_db_ace_config->l3gre_add_header = p_ace_config->l3gre_add_header;
        break;

    case TBL_ACE_CONFIG_FLD_L3GRE_SRC_IP:
        sal_memcpy(&p_db_ace_config->l3gre_src_ip, &p_ace_config->l3gre_src_ip, sizeof(p_ace_config->l3gre_src_ip));
        break;

    case TBL_ACE_CONFIG_FLD_L3GRE_DST_IP:
        sal_memcpy(&p_db_ace_config->l3gre_dst_ip, &p_ace_config->l3gre_dst_ip, sizeof(p_ace_config->l3gre_dst_ip));
        break;

    case TBL_ACE_CONFIG_FLD_L3GRE_DST_MAC:
        sal_memcpy(p_db_ace_config->l3gre_dst_mac, p_ace_config->l3gre_dst_mac, sizeof(p_ace_config->l3gre_dst_mac));
        break;

    case TBL_ACE_CONFIG_FLD_DENY:
        p_db_ace_config->deny = p_ace_config->deny;
        break;

    case TBL_ACE_CONFIG_FLD_STATS_EN:
        p_db_ace_config->stats_en = p_ace_config->stats_en;
        break;

    case TBL_ACE_CONFIG_FLD_OPTIONS:
        p_db_ace_config->options = p_ace_config->options;
        break;

    case TBL_ACE_CONFIG_FLD_FLOW_MATCH_NAME:
        sal_memcpy(p_db_ace_config->flow_match_name, p_ace_config->flow_match_name, sizeof(p_ace_config->flow_match_name));
        break;

    case TBL_ACE_CONFIG_FLD_ACTION_STRIP_HEADER:
        p_db_ace_config->action_strip_header = p_ace_config->action_strip_header;
        break;

    case TBL_ACE_CONFIG_FLD_TAP_ACTION_REDIRECT:
        p_db_ace_config->tap_action_redirect = p_ace_config->tap_action_redirect;
        break;

    case TBL_ACE_CONFIG_FLD_TAP_ACTION_MARK_VLAN:
        p_db_ace_config->tap_action_mark_vlan = p_ace_config->tap_action_mark_vlan;
        break;

    case TBL_ACE_CONFIG_FLD_TAP_ACTION_UNTAG:
        p_db_ace_config->tap_action_untag = p_ace_config->tap_action_untag;
        break;

    case TBL_ACE_CONFIG_FLD_TAP_ACTION_TRUNCTION:
        p_db_ace_config->tap_action_trunction = p_ace_config->tap_action_trunction;
        break;

    case TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_DEST_MAC_EN:
        p_db_ace_config->tap_action_edit_dest_mac_en = p_ace_config->tap_action_edit_dest_mac_en;
        break;

    case TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_DEST_MAC:
        sal_memcpy(p_db_ace_config->tap_action_edit_dest_mac, p_ace_config->tap_action_edit_dest_mac, sizeof(p_ace_config->tap_action_edit_dest_mac));
        break;

    case TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_SRC_MAC_EN:
        p_db_ace_config->tap_action_edit_src_mac_en = p_ace_config->tap_action_edit_src_mac_en;
        break;

    case TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_SRC_MAC:
        sal_memcpy(p_db_ace_config->tap_action_edit_src_mac, p_ace_config->tap_action_edit_src_mac, sizeof(p_ace_config->tap_action_edit_src_mac));
        break;

    case TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_IPDA_EN:
        p_db_ace_config->tap_action_edit_ipda_en = p_ace_config->tap_action_edit_ipda_en;
        break;

    case TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_IPDA:
        sal_memcpy(&p_db_ace_config->tap_action_edit_ipda, &p_ace_config->tap_action_edit_ipda, sizeof(p_ace_config->tap_action_edit_ipda));
        break;

    case TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_IPSA_EN:
        p_db_ace_config->tap_action_edit_ipsa_en = p_ace_config->tap_action_edit_ipsa_en;
        break;

    case TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_IPSA:
        sal_memcpy(&p_db_ace_config->tap_action_edit_ipsa, &p_ace_config->tap_action_edit_ipsa, sizeof(p_ace_config->tap_action_edit_ipsa));
        break;

    case TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_VLAN_EN:
        p_db_ace_config->tap_action_edit_vlan_en = p_ace_config->tap_action_edit_vlan_en;
        break;

    case TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_VLAN:
        p_db_ace_config->tap_action_edit_vlan = p_ace_config->tap_action_edit_vlan;
        break;

    case TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_VLAN_ACTION:
        p_db_ace_config->tap_action_edit_vlan_action = p_ace_config->tap_action_edit_vlan_action;
        break;

    case TBL_ACE_CONFIG_FLD_TAP_SNMP_SET:
        p_db_ace_config->tap_snmp_set = p_ace_config->tap_snmp_set;
        break;

    case TBL_ACE_CONFIG_FLD_ADD_VXLAN_HEADER:
        p_db_ace_config->add_vxlan_header = p_ace_config->add_vxlan_header;
        break;

    case TBL_ACE_CONFIG_FLD_VXLAN_DST_MAC:
        sal_memcpy(p_db_ace_config->vxlan_dst_mac, p_ace_config->vxlan_dst_mac, sizeof(p_ace_config->vxlan_dst_mac));
        break;

    case TBL_ACE_CONFIG_FLD_VXLAN_SRC_IP:
        sal_memcpy(&p_db_ace_config->vxlan_src_ip, &p_ace_config->vxlan_src_ip, sizeof(p_ace_config->vxlan_src_ip));
        break;

    case TBL_ACE_CONFIG_FLD_VXLAN_DST_IP:
        sal_memcpy(&p_db_ace_config->vxlan_dst_ip, &p_ace_config->vxlan_dst_ip, sizeof(p_ace_config->vxlan_dst_ip));
        break;

    case TBL_ACE_CONFIG_FLD_VXLAN_SRCPORT:
        p_db_ace_config->vxlan_srcport = p_ace_config->vxlan_srcport;
        break;

    case TBL_ACE_CONFIG_FLD_VXLAN_DESTPORT:
        p_db_ace_config->vxlan_destport = p_ace_config->vxlan_destport;
        break;

    case TBL_ACE_CONFIG_FLD_VXLAN_VNI:
        p_db_ace_config->vxlan_vni = p_ace_config->vxlan_vni;
        break;

    case TBL_ACE_CONFIG_FLD_MAX:
        sal_memcpy(p_db_ace_config, p_ace_config, sizeof(tbl_ace_config_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_ACE_CONFIG, field_id, p_db_ace_config);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_ACE_CONFIG, field_id, p_db_ace_config);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ace_config_add_ace_config(tbl_ace_config_t *p_ace_config)
{
    return tbl_ace_config_add_ace_config_sync(p_ace_config, TRUE);
}

int32
tbl_ace_config_del_ace_config(tbl_ace_config_key_t *p_ace_config_key)
{
    return tbl_ace_config_del_ace_config_sync(p_ace_config_key, TRUE);
}

int32
tbl_ace_config_set_ace_config_field(tbl_ace_config_t *p_ace_config, tbl_ace_config_field_id_t field_id)
{
    return tbl_ace_config_set_ace_config_field_sync(p_ace_config, field_id, TRUE);
}

tbl_ace_config_t*
tbl_ace_config_get_ace_config(tbl_ace_config_key_t *p_ace_config_key)
{
    tbl_ace_config_master_t *p_master = _g_p_tbl_ace_config_master;
    tbl_ace_config_t lkp;

    sal_memcpy(&lkp.key, p_ace_config_key, sizeof(tbl_ace_config_key_t));
    return ctclib_hash_lookup(p_master->ace_config_hash, &lkp);
}

char*
tbl_ace_config_key_val2str(tbl_ace_config_t *p_ace_config, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_ace_config_key_val2str(buf, MAX_CMD_STR_LEN, &p_ace_config->key));
    return str;
}

int32
tbl_ace_config_key_str2val(char *str, tbl_ace_config_t *p_ace_config)
{
    int32 ret = 0;

    ret = cdb_ace_config_key_str2val(str, &p_ace_config->key);
    return ret;
}

char*
tbl_ace_config_key_name_dump(tbl_ace_config_t *p_ace_config, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACE_CONFIG);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_ace_config_key_val2str(buf, MAX_CMD_STR_LEN, &p_ace_config->key));
    return str;
}

char*
tbl_ace_config_key_value_dump(tbl_ace_config_t *p_ace_config, char *str)
{

    cdb_ace_config_key_val2str(str, MAX_CMD_STR_LEN, &p_ace_config->key);
    return str;
}

char*
tbl_ace_config_field_name_dump(tbl_ace_config_t *p_ace_config, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_ACE_CONFIG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_ACE_CONFIG_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_ace_config_field_value_dump(tbl_ace_config_t *p_ace_config, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_KEY, field_id))
    {
        cdb_ace_config_key_val2str(str, MAX_CMD_STR_LEN, &p_ace_config->key);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_KEY_FLAGS, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->key_flags);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_KEY_FLAGS2, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->key_flags2);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_KEY_IPV6_FLAGS, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->key_ipv6_flags);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IS_IPV6, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->is_ipv6);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_ACL_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->acl_type);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_KEY_ARP_FLAGS, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->key_arp_flags);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_ARP_OP_CODE, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->arp_op_code);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_ARP_OP_CODE_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->arp_op_code_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TIME_RANGE, field_id))
    {
        sal_sprintf(str, "%s", p_ace_config->time_range);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_APPLY_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->apply_cnt);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IN_PORT, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->in_port);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_OUT_PORT, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->out_port);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_ETHER_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->ether_type);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_ETHER_TYPE_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->ether_type_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_SRC_MAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_ace_config->src_mac);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_SRC_MAC_MASK, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_ace_config->src_mac_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_DST_MAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_ace_config->dst_mac);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_DST_MAC_MASK, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_ace_config->dst_mac_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_SVLAN, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->svlan);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_SVLAN_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->svlan_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_SVLAN_COS, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->svlan_cos);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_SVLAN_COS_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->svlan_cos_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_CVLAN, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->cvlan);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_CVLAN_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->cvlan_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_CVLAN_COS, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->cvlan_cos);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_CVLAN_COS_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->cvlan_cos_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_UNTAG_VLAN, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->untag_vlan);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IP_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->ip_type);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IP_TYPE_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->ip_type_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IP_FLAGS, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->ip_flags);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IP_FLAGS_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->ip_flags_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_SRC_IP, field_id))
    {
        cdb_prefix_val2str(str, MAX_CMD_STR_LEN, &p_ace_config->src_ip);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_SRC_IP_MASK, field_id))
    {
        cdb_prefix_val2str(str, MAX_CMD_STR_LEN, &p_ace_config->src_ip_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_DST_IP, field_id))
    {
        cdb_prefix_val2str(str, MAX_CMD_STR_LEN, &p_ace_config->dst_ip);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_DST_IP_MASK, field_id))
    {
        cdb_prefix_val2str(str, MAX_CMD_STR_LEN, &p_ace_config->dst_ip_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IPV6_FLOW_LABEL, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->ipv6_flow_label);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IPV6_FLOW_LABEL_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->ipv6_flow_label_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_DSCP, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->dscp);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_DSCP_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->dscp_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IP_PRECEDENCE, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->ip_precedence);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IP_PRECEDENCE_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->ip_precedence_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IP_PROTOCOL, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->ip_protocol);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IP_PROTOCOL_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->ip_protocol_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L4_SRC_PORT_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->l4_src_port_type);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L4_SRC_PORT, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->l4_src_port);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L4_SRC_PORT_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->l4_src_port_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L4_DST_PORT_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->l4_dst_port_type);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L4_DST_PORT, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->l4_dst_port);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L4_DST_PORT_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->l4_dst_port_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IP_FRAG, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->ip_frag);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TCP_FLAGS, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->tcp_flags);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TCP_FLAGS_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->tcp_flags_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IGMP_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->igmp_type);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_ICMP_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->icmp_type);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_ICMP_CODE, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->icmp_code);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_UDF_ID, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->udf_id);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_UDF_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->udf_type);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_UDF_OFFSET, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->udf_offset);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_UDF_VALUE, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_ace_config->udf_value, sizeof(p_ace_config->udf_value)/4);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_UDF_MASK, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_ace_config->udf_mask, sizeof(p_ace_config->udf_mask)/4);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_UDF_VALUE_VALID, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_ace_config->udf_value_valid, sizeof(p_ace_config->udf_value_valid)/4);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_ERSPAN_ID, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->erspan_id);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_ERSPAN_ID_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->erspan_id_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_STRIP_HEADER_POS, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->strip_header_pos);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_STRIP_HEADER_OFFSET, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->strip_header_offset);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L4_VXLAN_VNI, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->l4_vxlan_vni);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L4_VXLAN_VNI_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->l4_vxlan_vni_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L4_GRE_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->l4_gre_type);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L4_GRE_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->l4_gre_key);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L4_GRE_KEY_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->l4_gre_key_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L2GRE_ADD_HEADER, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->l2gre_add_header);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L2GRE_SRC_IP, field_id))
    {
        cdb_addr_val2str(str, MAX_CMD_STR_LEN, &p_ace_config->l2gre_src_ip);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L2GRE_DST_IP, field_id))
    {
        cdb_addr_val2str(str, MAX_CMD_STR_LEN, &p_ace_config->l2gre_dst_ip);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L2GRE_DST_MAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_ace_config->l2gre_dst_mac);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L2GRE_KEY_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->l2gre_key_num);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L2GRE_KEY_LEN, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->l2gre_key_len);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L3GRE_ADD_HEADER, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->l3gre_add_header);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L3GRE_SRC_IP, field_id))
    {
        cdb_addr_val2str(str, MAX_CMD_STR_LEN, &p_ace_config->l3gre_src_ip);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L3GRE_DST_IP, field_id))
    {
        cdb_addr_val2str(str, MAX_CMD_STR_LEN, &p_ace_config->l3gre_dst_ip);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L3GRE_DST_MAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_ace_config->l3gre_dst_mac);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_DENY, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->deny);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_STATS_EN, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->stats_en);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_OPTIONS, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->options);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_FLOW_MATCH_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_ace_config->flow_match_name);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_ACTION_STRIP_HEADER, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->action_strip_header);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_REDIRECT, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->tap_action_redirect);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_MARK_VLAN, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->tap_action_mark_vlan);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_UNTAG, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->tap_action_untag);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_TRUNCTION, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->tap_action_trunction);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_DEST_MAC_EN, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->tap_action_edit_dest_mac_en);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_DEST_MAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_ace_config->tap_action_edit_dest_mac);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_SRC_MAC_EN, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->tap_action_edit_src_mac_en);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_SRC_MAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_ace_config->tap_action_edit_src_mac);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_IPDA_EN, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->tap_action_edit_ipda_en);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_IPDA, field_id))
    {
        cdb_addr_val2str(str, MAX_CMD_STR_LEN, &p_ace_config->tap_action_edit_ipda);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_IPSA_EN, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->tap_action_edit_ipsa_en);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_IPSA, field_id))
    {
        cdb_addr_val2str(str, MAX_CMD_STR_LEN, &p_ace_config->tap_action_edit_ipsa);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_VLAN_EN, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->tap_action_edit_vlan_en);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_VLAN, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->tap_action_edit_vlan);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_VLAN_ACTION, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->tap_action_edit_vlan_action);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_SNMP_SET, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->tap_snmp_set);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_ADD_VXLAN_HEADER, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->add_vxlan_header);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_VXLAN_DST_MAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_ace_config->vxlan_dst_mac);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_VXLAN_SRC_IP, field_id))
    {
        cdb_addr_val2str(str, MAX_CMD_STR_LEN, &p_ace_config->vxlan_src_ip);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_VXLAN_DST_IP, field_id))
    {
        cdb_addr_val2str(str, MAX_CMD_STR_LEN, &p_ace_config->vxlan_dst_ip);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_VXLAN_SRCPORT, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->vxlan_srcport);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_VXLAN_DESTPORT, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->vxlan_destport);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_VXLAN_VNI, field_id))
    {
        sal_sprintf(str, "%u", p_ace_config->vxlan_vni);
    }
    return str;
}

char**
tbl_ace_config_table_dump(tbl_ace_config_t *p_ace_config, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_ace_config_key_name_dump(p_ace_config, buf));
    for(i=1; i<TBL_ACE_CONFIG_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_ace_config_field_name_dump(p_ace_config, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_ace_config_field_value_dump(p_ace_config, i, buf));
    }
    return str;
}

int32
tbl_ace_config_field_value_parser(char *str, int32 field_id, tbl_ace_config_t *p_ace_config)
{
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_KEY, field_id))
    {
        cdb_ace_config_key_str2val(str, &p_ace_config->key);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_KEY_FLAGS, field_id))
    {
        int32 ret;
        p_ace_config->key_flags = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_KEY_FLAGS2, field_id))
    {
        int32 ret;
        p_ace_config->key_flags2 = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_KEY_IPV6_FLAGS, field_id))
    {
        int32 ret;
        p_ace_config->key_ipv6_flags = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IS_IPV6, field_id))
    {
        int32 ret;
        p_ace_config->is_ipv6 = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_ACL_TYPE, field_id))
    {
        int32 ret;
        p_ace_config->acl_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_KEY_ARP_FLAGS, field_id))
    {
        int32 ret;
        p_ace_config->key_arp_flags = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_ARP_OP_CODE, field_id))
    {
        int32 ret;
        p_ace_config->arp_op_code = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_ARP_OP_CODE_MASK, field_id))
    {
        int32 ret;
        p_ace_config->arp_op_code_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TIME_RANGE, field_id))
    {
        sal_strcpy(p_ace_config->time_range, str);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_APPLY_CNT, field_id))
    {
        int32 ret;
        p_ace_config->apply_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IN_PORT, field_id))
    {
        int32 ret;
        p_ace_config->in_port = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_OUT_PORT, field_id))
    {
        int32 ret;
        p_ace_config->out_port = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_ETHER_TYPE, field_id))
    {
        int32 ret;
        p_ace_config->ether_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_ETHER_TYPE_MASK, field_id))
    {
        int32 ret;
        p_ace_config->ether_type_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_SRC_MAC, field_id))
    {
        cdb_mac_addr_str2val(p_ace_config->src_mac, str);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_SRC_MAC_MASK, field_id))
    {
        cdb_mac_addr_str2val(p_ace_config->src_mac_mask, str);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_DST_MAC, field_id))
    {
        cdb_mac_addr_str2val(p_ace_config->dst_mac, str);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_DST_MAC_MASK, field_id))
    {
        cdb_mac_addr_str2val(p_ace_config->dst_mac_mask, str);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_SVLAN, field_id))
    {
        int32 ret;
        p_ace_config->svlan = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_SVLAN_MASK, field_id))
    {
        int32 ret;
        p_ace_config->svlan_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_SVLAN_COS, field_id))
    {
        int32 ret;
        p_ace_config->svlan_cos = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_SVLAN_COS_MASK, field_id))
    {
        int32 ret;
        p_ace_config->svlan_cos_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_CVLAN, field_id))
    {
        int32 ret;
        p_ace_config->cvlan = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_CVLAN_MASK, field_id))
    {
        int32 ret;
        p_ace_config->cvlan_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_CVLAN_COS, field_id))
    {
        int32 ret;
        p_ace_config->cvlan_cos = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_CVLAN_COS_MASK, field_id))
    {
        int32 ret;
        p_ace_config->cvlan_cos_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_UNTAG_VLAN, field_id))
    {
        int32 ret;
        p_ace_config->untag_vlan = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IP_TYPE, field_id))
    {
        int32 ret;
        p_ace_config->ip_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IP_TYPE_MASK, field_id))
    {
        int32 ret;
        p_ace_config->ip_type_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IP_FLAGS, field_id))
    {
        int32 ret;
        p_ace_config->ip_flags = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IP_FLAGS_MASK, field_id))
    {
        int32 ret;
        p_ace_config->ip_flags_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_SRC_IP, field_id))
    {
        cdb_prefix_str2val_wo_masklen(str, &p_ace_config->src_ip);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_SRC_IP_MASK, field_id))
    {
        cdb_prefix_str2val_wo_masklen(str, &p_ace_config->src_ip_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_DST_IP, field_id))
    {
        cdb_prefix_str2val_wo_masklen(str, &p_ace_config->dst_ip);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_DST_IP_MASK, field_id))
    {
        cdb_prefix_str2val_wo_masklen(str, &p_ace_config->dst_ip_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IPV6_FLOW_LABEL, field_id))
    {
        int32 ret;
        p_ace_config->ipv6_flow_label = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IPV6_FLOW_LABEL_MASK, field_id))
    {
        int32 ret;
        p_ace_config->ipv6_flow_label_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_DSCP, field_id))
    {
        int32 ret;
        p_ace_config->dscp = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_DSCP_MASK, field_id))
    {
        int32 ret;
        p_ace_config->dscp_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IP_PRECEDENCE, field_id))
    {
        int32 ret;
        p_ace_config->ip_precedence = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IP_PRECEDENCE_MASK, field_id))
    {
        int32 ret;
        p_ace_config->ip_precedence_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IP_PROTOCOL, field_id))
    {
        int32 ret;
        p_ace_config->ip_protocol = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IP_PROTOCOL_MASK, field_id))
    {
        int32 ret;
        p_ace_config->ip_protocol_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L4_SRC_PORT_TYPE, field_id))
    {
        int32 ret;
        p_ace_config->l4_src_port_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L4_SRC_PORT, field_id))
    {
        int32 ret;
        p_ace_config->l4_src_port = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L4_SRC_PORT_MASK, field_id))
    {
        int32 ret;
        p_ace_config->l4_src_port_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L4_DST_PORT_TYPE, field_id))
    {
        int32 ret;
        p_ace_config->l4_dst_port_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L4_DST_PORT, field_id))
    {
        int32 ret;
        p_ace_config->l4_dst_port = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L4_DST_PORT_MASK, field_id))
    {
        int32 ret;
        p_ace_config->l4_dst_port_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IP_FRAG, field_id))
    {
        int32 ret;
        p_ace_config->ip_frag = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TCP_FLAGS, field_id))
    {
        int32 ret;
        p_ace_config->tcp_flags = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TCP_FLAGS_MASK, field_id))
    {
        int32 ret;
        p_ace_config->tcp_flags_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IGMP_TYPE, field_id))
    {
        int32 ret;
        p_ace_config->igmp_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_ICMP_TYPE, field_id))
    {
        int32 ret;
        p_ace_config->icmp_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_ICMP_CODE, field_id))
    {
        int32 ret;
        p_ace_config->icmp_code = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_UDF_ID, field_id))
    {
        int32 ret;
        p_ace_config->udf_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_UDF_TYPE, field_id))
    {
        int32 ret;
        p_ace_config->udf_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_UDF_OFFSET, field_id))
    {
        int32 ret;
        p_ace_config->udf_offset = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_UDF_VALUE, field_id))
    {
        cdb_uint32_array_str2val(str, p_ace_config->udf_value);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_UDF_MASK, field_id))
    {
        cdb_uint32_array_str2val(str, p_ace_config->udf_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_UDF_VALUE_VALID, field_id))
    {
        cdb_uint32_array_str2val(str, p_ace_config->udf_value_valid);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_ERSPAN_ID, field_id))
    {
        int32 ret;
        p_ace_config->erspan_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_ERSPAN_ID_MASK, field_id))
    {
        int32 ret;
        p_ace_config->erspan_id_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_STRIP_HEADER_POS, field_id))
    {
        int32 ret;
        p_ace_config->strip_header_pos = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_STRIP_HEADER_OFFSET, field_id))
    {
        int32 ret;
        p_ace_config->strip_header_offset = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L4_VXLAN_VNI, field_id))
    {
        int32 ret;
        p_ace_config->l4_vxlan_vni = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L4_VXLAN_VNI_MASK, field_id))
    {
        int32 ret;
        p_ace_config->l4_vxlan_vni_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L4_GRE_TYPE, field_id))
    {
        int32 ret;
        p_ace_config->l4_gre_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L4_GRE_KEY, field_id))
    {
        int32 ret;
        p_ace_config->l4_gre_key = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L4_GRE_KEY_MASK, field_id))
    {
        int32 ret;
        p_ace_config->l4_gre_key_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L2GRE_ADD_HEADER, field_id))
    {
        int32 ret;
        p_ace_config->l2gre_add_header = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L2GRE_SRC_IP, field_id))
    {
        cdb_addr_str2val(&p_ace_config->l2gre_src_ip, str, 0);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L2GRE_DST_IP, field_id))
    {
        cdb_addr_str2val(&p_ace_config->l2gre_dst_ip, str, 0);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L2GRE_DST_MAC, field_id))
    {
        cdb_mac_addr_str2val(p_ace_config->l2gre_dst_mac, str);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L2GRE_KEY_NUM, field_id))
    {
        int32 ret;
        p_ace_config->l2gre_key_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L2GRE_KEY_LEN, field_id))
    {
        int32 ret;
        p_ace_config->l2gre_key_len = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L3GRE_ADD_HEADER, field_id))
    {
        int32 ret;
        p_ace_config->l3gre_add_header = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L3GRE_SRC_IP, field_id))
    {
        cdb_addr_str2val(&p_ace_config->l3gre_src_ip, str, 0);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L3GRE_DST_IP, field_id))
    {
        cdb_addr_str2val(&p_ace_config->l3gre_dst_ip, str, 0);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L3GRE_DST_MAC, field_id))
    {
        cdb_mac_addr_str2val(p_ace_config->l3gre_dst_mac, str);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_DENY, field_id))
    {
        int32 ret;
        p_ace_config->deny = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_STATS_EN, field_id))
    {
        int32 ret;
        p_ace_config->stats_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_OPTIONS, field_id))
    {
        int32 ret;
        p_ace_config->options = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_FLOW_MATCH_NAME, field_id))
    {
        sal_strcpy(p_ace_config->flow_match_name, str);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_ACTION_STRIP_HEADER, field_id))
    {
        int32 ret;
        p_ace_config->action_strip_header = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_REDIRECT, field_id))
    {
        int32 ret;
        p_ace_config->tap_action_redirect = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_MARK_VLAN, field_id))
    {
        int32 ret;
        p_ace_config->tap_action_mark_vlan = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_UNTAG, field_id))
    {
        int32 ret;
        p_ace_config->tap_action_untag = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_TRUNCTION, field_id))
    {
        int32 ret;
        p_ace_config->tap_action_trunction = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_DEST_MAC_EN, field_id))
    {
        int32 ret;
        p_ace_config->tap_action_edit_dest_mac_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_DEST_MAC, field_id))
    {
        cdb_mac_addr_str2val(p_ace_config->tap_action_edit_dest_mac, str);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_SRC_MAC_EN, field_id))
    {
        int32 ret;
        p_ace_config->tap_action_edit_src_mac_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_SRC_MAC, field_id))
    {
        cdb_mac_addr_str2val(p_ace_config->tap_action_edit_src_mac, str);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_IPDA_EN, field_id))
    {
        int32 ret;
        p_ace_config->tap_action_edit_ipda_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_IPDA, field_id))
    {
        cdb_addr_str2val(&p_ace_config->tap_action_edit_ipda, str, 0);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_IPSA_EN, field_id))
    {
        int32 ret;
        p_ace_config->tap_action_edit_ipsa_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_IPSA, field_id))
    {
        cdb_addr_str2val(&p_ace_config->tap_action_edit_ipsa, str, 0);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_VLAN_EN, field_id))
    {
        int32 ret;
        p_ace_config->tap_action_edit_vlan_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_VLAN, field_id))
    {
        int32 ret;
        p_ace_config->tap_action_edit_vlan = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_VLAN_ACTION, field_id))
    {
        int32 ret;
        p_ace_config->tap_action_edit_vlan_action = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_SNMP_SET, field_id))
    {
        int32 ret;
        p_ace_config->tap_snmp_set = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_ADD_VXLAN_HEADER, field_id))
    {
        int32 ret;
        p_ace_config->add_vxlan_header = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_VXLAN_DST_MAC, field_id))
    {
        cdb_mac_addr_str2val(p_ace_config->vxlan_dst_mac, str);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_VXLAN_SRC_IP, field_id))
    {
        cdb_addr_str2val(&p_ace_config->vxlan_src_ip, str, 0);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_VXLAN_DST_IP, field_id))
    {
        cdb_addr_str2val(&p_ace_config->vxlan_dst_ip, str, 0);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_VXLAN_SRCPORT, field_id))
    {
        int32 ret;
        p_ace_config->vxlan_srcport = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_VXLAN_DESTPORT, field_id))
    {
        int32 ret;
        p_ace_config->vxlan_destport = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_VXLAN_VNI, field_id))
    {
        int32 ret;
        p_ace_config->vxlan_vni = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_ace_config_table_parser(char** array, char* key_value,tbl_ace_config_t *p_ace_config)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACE_CONFIG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_ace_config_key_str2val(key_value, p_ace_config));

    for(i=1; i<TBL_ACE_CONFIG_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_ace_config_field_value_parser( array[j++], i, p_ace_config));
    }

    return PM_E_NONE;
}

int32
tbl_ace_config_dump_one(tbl_ace_config_t *p_ace_config, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACE_CONFIG);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACE_CONFIG_FLD_KEY].name, 
            cdb_ace_config_key_val2str(buf, MAX_CMD_STR_LEN, &p_ace_config->key));
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_KEY_FLAGS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_KEY_FLAGS].name,
            p_ace_config->key_flags);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_KEY_FLAGS2, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_KEY_FLAGS2].name,
            p_ace_config->key_flags2);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_KEY_IPV6_FLAGS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_KEY_IPV6_FLAGS].name,
            p_ace_config->key_ipv6_flags);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IS_IPV6, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_IS_IPV6].name,
            p_ace_config->is_ipv6);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_ACL_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_ACL_TYPE].name,
            p_ace_config->acl_type);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_KEY_ARP_FLAGS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_KEY_ARP_FLAGS].name,
            p_ace_config->key_arp_flags);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_ARP_OP_CODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_ARP_OP_CODE].name,
            p_ace_config->arp_op_code);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_ARP_OP_CODE_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_ARP_OP_CODE_MASK].name,
            p_ace_config->arp_op_code_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TIME_RANGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACE_CONFIG_FLD_TIME_RANGE].name,
            p_ace_config->time_range);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_APPLY_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_APPLY_CNT].name,
            p_ace_config->apply_cnt);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IN_PORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_IN_PORT].name,
            p_ace_config->in_port);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_OUT_PORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_OUT_PORT].name,
            p_ace_config->out_port);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_ETHER_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_ETHER_TYPE].name,
            p_ace_config->ether_type);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_ETHER_TYPE_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_ETHER_TYPE_MASK].name,
            p_ace_config->ether_type_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_SRC_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACE_CONFIG_FLD_SRC_MAC].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_ace_config->src_mac));
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_SRC_MAC_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACE_CONFIG_FLD_SRC_MAC_MASK].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_ace_config->src_mac_mask));
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_DST_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACE_CONFIG_FLD_DST_MAC].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_ace_config->dst_mac));
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_DST_MAC_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACE_CONFIG_FLD_DST_MAC_MASK].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_ace_config->dst_mac_mask));
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_SVLAN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_SVLAN].name,
            p_ace_config->svlan);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_SVLAN_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_SVLAN_MASK].name,
            p_ace_config->svlan_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_SVLAN_COS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_SVLAN_COS].name,
            p_ace_config->svlan_cos);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_SVLAN_COS_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_SVLAN_COS_MASK].name,
            p_ace_config->svlan_cos_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_CVLAN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_CVLAN].name,
            p_ace_config->cvlan);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_CVLAN_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_CVLAN_MASK].name,
            p_ace_config->cvlan_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_CVLAN_COS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_CVLAN_COS].name,
            p_ace_config->cvlan_cos);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_CVLAN_COS_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_CVLAN_COS_MASK].name,
            p_ace_config->cvlan_cos_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_UNTAG_VLAN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_UNTAG_VLAN].name,
            p_ace_config->untag_vlan);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IP_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_IP_TYPE].name,
            p_ace_config->ip_type);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IP_TYPE_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_IP_TYPE_MASK].name,
            p_ace_config->ip_type_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IP_FLAGS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_IP_FLAGS].name,
            p_ace_config->ip_flags);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IP_FLAGS_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_IP_FLAGS_MASK].name,
            p_ace_config->ip_flags_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_SRC_IP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACE_CONFIG_FLD_SRC_IP].name, 
            cdb_prefix_val2str(buf, MAX_CMD_STR_LEN, &p_ace_config->src_ip));
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_SRC_IP_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACE_CONFIG_FLD_SRC_IP_MASK].name, 
            cdb_prefix_val2str(buf, MAX_CMD_STR_LEN, &p_ace_config->src_ip_mask));
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_DST_IP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACE_CONFIG_FLD_DST_IP].name, 
            cdb_prefix_val2str(buf, MAX_CMD_STR_LEN, &p_ace_config->dst_ip));
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_DST_IP_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACE_CONFIG_FLD_DST_IP_MASK].name, 
            cdb_prefix_val2str(buf, MAX_CMD_STR_LEN, &p_ace_config->dst_ip_mask));
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IPV6_FLOW_LABEL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_IPV6_FLOW_LABEL].name,
            p_ace_config->ipv6_flow_label);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IPV6_FLOW_LABEL_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_IPV6_FLOW_LABEL_MASK].name,
            p_ace_config->ipv6_flow_label_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_DSCP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_DSCP].name,
            p_ace_config->dscp);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_DSCP_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_DSCP_MASK].name,
            p_ace_config->dscp_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IP_PRECEDENCE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_IP_PRECEDENCE].name,
            p_ace_config->ip_precedence);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IP_PRECEDENCE_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_IP_PRECEDENCE_MASK].name,
            p_ace_config->ip_precedence_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IP_PROTOCOL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_IP_PROTOCOL].name,
            p_ace_config->ip_protocol);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IP_PROTOCOL_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_IP_PROTOCOL_MASK].name,
            p_ace_config->ip_protocol_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L4_SRC_PORT_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_L4_SRC_PORT_TYPE].name,
            p_ace_config->l4_src_port_type);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L4_SRC_PORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_L4_SRC_PORT].name,
            p_ace_config->l4_src_port);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L4_SRC_PORT_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_L4_SRC_PORT_MASK].name,
            p_ace_config->l4_src_port_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L4_DST_PORT_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_L4_DST_PORT_TYPE].name,
            p_ace_config->l4_dst_port_type);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L4_DST_PORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_L4_DST_PORT].name,
            p_ace_config->l4_dst_port);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L4_DST_PORT_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_L4_DST_PORT_MASK].name,
            p_ace_config->l4_dst_port_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IP_FRAG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_IP_FRAG].name,
            p_ace_config->ip_frag);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TCP_FLAGS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_TCP_FLAGS].name,
            p_ace_config->tcp_flags);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TCP_FLAGS_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_TCP_FLAGS_MASK].name,
            p_ace_config->tcp_flags_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_IGMP_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_IGMP_TYPE].name,
            p_ace_config->igmp_type);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_ICMP_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_ICMP_TYPE].name,
            p_ace_config->icmp_type);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_ICMP_CODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_ICMP_CODE].name,
            p_ace_config->icmp_code);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_UDF_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_UDF_ID].name,
            p_ace_config->udf_id);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_UDF_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_UDF_TYPE].name,
            p_ace_config->udf_type);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_UDF_OFFSET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_UDF_OFFSET].name,
            p_ace_config->udf_offset);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_UDF_VALUE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACE_CONFIG_FLD_UDF_VALUE].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_ace_config->udf_value, sizeof(p_ace_config->udf_value)/4));
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_UDF_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACE_CONFIG_FLD_UDF_MASK].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_ace_config->udf_mask, sizeof(p_ace_config->udf_mask)/4));
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_UDF_VALUE_VALID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACE_CONFIG_FLD_UDF_VALUE_VALID].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_ace_config->udf_value_valid, sizeof(p_ace_config->udf_value_valid)/4));
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_ERSPAN_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_ERSPAN_ID].name,
            p_ace_config->erspan_id);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_ERSPAN_ID_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_ERSPAN_ID_MASK].name,
            p_ace_config->erspan_id_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_STRIP_HEADER_POS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_STRIP_HEADER_POS].name,
            p_ace_config->strip_header_pos);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_STRIP_HEADER_OFFSET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_STRIP_HEADER_OFFSET].name,
            p_ace_config->strip_header_offset);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L4_VXLAN_VNI, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_L4_VXLAN_VNI].name,
            p_ace_config->l4_vxlan_vni);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L4_VXLAN_VNI_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_L4_VXLAN_VNI_MASK].name,
            p_ace_config->l4_vxlan_vni_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L4_GRE_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_L4_GRE_TYPE].name,
            p_ace_config->l4_gre_type);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L4_GRE_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_L4_GRE_KEY].name,
            p_ace_config->l4_gre_key);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L4_GRE_KEY_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_L4_GRE_KEY_MASK].name,
            p_ace_config->l4_gre_key_mask);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L2GRE_ADD_HEADER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_L2GRE_ADD_HEADER].name,
            p_ace_config->l2gre_add_header);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L2GRE_SRC_IP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACE_CONFIG_FLD_L2GRE_SRC_IP].name, 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_ace_config->l2gre_src_ip));
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L2GRE_DST_IP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACE_CONFIG_FLD_L2GRE_DST_IP].name, 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_ace_config->l2gre_dst_ip));
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L2GRE_DST_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACE_CONFIG_FLD_L2GRE_DST_MAC].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_ace_config->l2gre_dst_mac));
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L2GRE_KEY_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_L2GRE_KEY_NUM].name,
            p_ace_config->l2gre_key_num);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L2GRE_KEY_LEN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_L2GRE_KEY_LEN].name,
            p_ace_config->l2gre_key_len);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L3GRE_ADD_HEADER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_L3GRE_ADD_HEADER].name,
            p_ace_config->l3gre_add_header);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L3GRE_SRC_IP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACE_CONFIG_FLD_L3GRE_SRC_IP].name, 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_ace_config->l3gre_src_ip));
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L3GRE_DST_IP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACE_CONFIG_FLD_L3GRE_DST_IP].name, 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_ace_config->l3gre_dst_ip));
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_L3GRE_DST_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACE_CONFIG_FLD_L3GRE_DST_MAC].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_ace_config->l3gre_dst_mac));
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_DENY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_DENY].name,
            p_ace_config->deny);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_STATS_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_STATS_EN].name,
            p_ace_config->stats_en);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_OPTIONS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_OPTIONS].name,
            p_ace_config->options);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_FLOW_MATCH_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACE_CONFIG_FLD_FLOW_MATCH_NAME].name,
            p_ace_config->flow_match_name);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_ACTION_STRIP_HEADER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_ACTION_STRIP_HEADER].name,
            p_ace_config->action_strip_header);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_REDIRECT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_TAP_ACTION_REDIRECT].name,
            p_ace_config->tap_action_redirect);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_MARK_VLAN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_TAP_ACTION_MARK_VLAN].name,
            p_ace_config->tap_action_mark_vlan);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_UNTAG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_TAP_ACTION_UNTAG].name,
            p_ace_config->tap_action_untag);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_TRUNCTION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_TAP_ACTION_TRUNCTION].name,
            p_ace_config->tap_action_trunction);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_DEST_MAC_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_DEST_MAC_EN].name,
            p_ace_config->tap_action_edit_dest_mac_en);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_DEST_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_DEST_MAC].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_ace_config->tap_action_edit_dest_mac));
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_SRC_MAC_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_SRC_MAC_EN].name,
            p_ace_config->tap_action_edit_src_mac_en);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_SRC_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_SRC_MAC].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_ace_config->tap_action_edit_src_mac));
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_IPDA_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_IPDA_EN].name,
            p_ace_config->tap_action_edit_ipda_en);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_IPDA, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_IPDA].name, 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_ace_config->tap_action_edit_ipda));
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_IPSA_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_IPSA_EN].name,
            p_ace_config->tap_action_edit_ipsa_en);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_IPSA, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_IPSA].name, 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_ace_config->tap_action_edit_ipsa));
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_VLAN_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_VLAN_EN].name,
            p_ace_config->tap_action_edit_vlan_en);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_VLAN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_VLAN].name,
            p_ace_config->tap_action_edit_vlan);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_VLAN_ACTION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_TAP_ACTION_EDIT_VLAN_ACTION].name,
            p_ace_config->tap_action_edit_vlan_action);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_TAP_SNMP_SET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_TAP_SNMP_SET].name,
            p_ace_config->tap_snmp_set);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_ADD_VXLAN_HEADER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_ADD_VXLAN_HEADER].name,
            p_ace_config->add_vxlan_header);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_VXLAN_DST_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACE_CONFIG_FLD_VXLAN_DST_MAC].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_ace_config->vxlan_dst_mac));
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_VXLAN_SRC_IP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACE_CONFIG_FLD_VXLAN_SRC_IP].name, 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_ace_config->vxlan_src_ip));
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_VXLAN_DST_IP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACE_CONFIG_FLD_VXLAN_DST_IP].name, 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_ace_config->vxlan_dst_ip));
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_VXLAN_SRCPORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_VXLAN_SRCPORT].name,
            p_ace_config->vxlan_srcport);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_VXLAN_DESTPORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_VXLAN_DESTPORT].name,
            p_ace_config->vxlan_destport);
    }
    if (FLD_MATCH(TBL_ACE_CONFIG_FLD_VXLAN_VNI, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACE_CONFIG_FLD_VXLAN_VNI].name,
            p_ace_config->vxlan_vni);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_ace_config_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_ace_config_master_t *p_master = _g_p_tbl_ace_config_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_ace_config_t *p_db_ace_config = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->ace_config_list, p_db_ace_config, listnode, next)
    {
        rc |= fn(p_db_ace_config, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_ace_config_master_t*
tbl_ace_config_get_master()
{
    return _g_p_tbl_ace_config_master;
}

tbl_ace_config_master_t*
tbl_ace_config_init_ace_config()
{
    _g_p_tbl_ace_config_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_ace_config_master_t));
    _g_p_tbl_ace_config_master->ace_config_hash = ctclib_hash_create(_tbl_ace_config_hash_make, _tbl_ace_config_hash_cmp);
    _g_p_tbl_ace_config_master->ace_config_list = ctclib_slist_create(tbl_ace_config_cmp, NULL);
    return _g_p_tbl_ace_config_master;
}

int32
tbl_ace_config_get_ace_config_count()
{
    tbl_ace_config_master_t *p_master = _g_p_tbl_ace_config_master;

    return p_master->ace_config_hash->count;
}

tbl_ace_config_t*
tbl_ace_config_get_ace_config_by_name(const char* acl_name, const uint32 seq_no)
{
    tbl_ace_config_key_t key;

    sal_memset(&key, 0, sizeof(key));
    sal_strncpy(key.acl_name, acl_name, ACL_NAME_SIZE);
    key.seq_no = seq_no;
    return tbl_ace_config_get_ace_config(&key);
}


