
#include "proto.h"
#include "gen/tbl_acl_entry_define.h"
#include "gen/tbl_acl_entry.h"
#include "cdb_data_cmp.h"

tbl_acl_entry_master_t *_g_p_tbl_acl_entry_master = NULL;

static uint32
_tbl_acl_entry_hash_make(tbl_acl_entry_t *p_acl_entry)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_acl_entry->key;
    for (index = 0; index < sizeof(p_acl_entry->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_acl_entry_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_acl_entry_t *p_acl_entry1 = (tbl_acl_entry_t*)p_arg1;
    tbl_acl_entry_t *p_acl_entry2 = (tbl_acl_entry_t*)p_arg2;

    if (0 == sal_memcmp(&p_acl_entry1->key, &p_acl_entry2->key, sizeof(tbl_acl_entry_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_acl_entry_add_acl_entry_sync(tbl_acl_entry_t *p_acl_entry, uint32 sync)
{
    tbl_acl_entry_master_t *p_master = _g_p_tbl_acl_entry_master;
    tbl_acl_entry_t *p_db_acl_entry = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_acl_entry_get_acl_entry(&p_acl_entry->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_acl_entry = XCALLOC(MEM_TBL_ACL_ENTRY, sizeof(tbl_acl_entry_t));
    if (NULL == p_db_acl_entry)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_acl_entry, p_acl_entry, sizeof(tbl_acl_entry_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->acl_entry_hash, (void*)p_db_acl_entry, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->acl_entry_list, p_db_acl_entry);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_ACL_ENTRY, p_db_acl_entry);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_ACL_ENTRY, p_db_acl_entry);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_acl_entry_del_acl_entry_sync(tbl_acl_entry_key_t *p_acl_entry_key, uint32 sync)
{
    tbl_acl_entry_master_t *p_master = _g_p_tbl_acl_entry_master;
    tbl_acl_entry_t *p_db_acl_entry = NULL;

    /* 1. lookup entry exist */
    p_db_acl_entry = tbl_acl_entry_get_acl_entry(p_acl_entry_key);
    if (NULL == p_db_acl_entry)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_ACL_ENTRY, p_db_acl_entry);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_ACL_ENTRY, p_db_acl_entry);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->acl_entry_hash, (void*)p_db_acl_entry);
    ctclib_slistnode_delete(p_master->acl_entry_list, p_db_acl_entry);

    /* 4. free db entry */
    XFREE(MEM_TBL_ACL_ENTRY, p_db_acl_entry);

    return PM_E_NONE;
}

int32
tbl_acl_entry_set_acl_entry_field_sync(tbl_acl_entry_t *p_acl_entry, tbl_acl_entry_field_id_t field_id, uint32 sync)
{
    tbl_acl_entry_t *p_db_acl_entry = NULL;

    /* 1. lookup entry exist */
    p_db_acl_entry = tbl_acl_entry_get_acl_entry(&p_acl_entry->key);
    if (NULL == p_db_acl_entry)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_ACL_ENTRY_FLD_TBLID:
        p_db_acl_entry->tblid = p_acl_entry->tblid;
        break;

    case TBL_ACL_ENTRY_FLD_ENTRY_PRIORITY:
        p_db_acl_entry->entry_priority = p_acl_entry->entry_priority;
        break;

    case TBL_ACL_ENTRY_FLD_TIME_RANGE:
        sal_memcpy(p_db_acl_entry->time_range, p_acl_entry->time_range, sizeof(p_acl_entry->time_range));
        break;

    case TBL_ACL_ENTRY_FLD_KEY_FLAGS:
        p_db_acl_entry->key_flags = p_acl_entry->key_flags;
        break;

    case TBL_ACL_ENTRY_FLD_KEY_FLAGS2:
        p_db_acl_entry->key_flags2 = p_acl_entry->key_flags2;
        break;

    case TBL_ACL_ENTRY_FLD_KEY_IPV6_FLAGS:
        p_db_acl_entry->key_ipv6_flags = p_acl_entry->key_ipv6_flags;
        break;

    case TBL_ACL_ENTRY_FLD_IS_IPV6:
        p_db_acl_entry->is_ipv6 = p_acl_entry->is_ipv6;
        break;

    case TBL_ACL_ENTRY_FLD_ACL_TYPE:
        p_db_acl_entry->acl_type = p_acl_entry->acl_type;
        break;

    case TBL_ACL_ENTRY_FLD_KEY_ARP_FLAGS:
        p_db_acl_entry->key_arp_flags = p_acl_entry->key_arp_flags;
        break;

    case TBL_ACL_ENTRY_FLD_ARP_OP_CODE:
        p_db_acl_entry->arp_op_code = p_acl_entry->arp_op_code;
        break;

    case TBL_ACL_ENTRY_FLD_ARP_OP_CODE_MASK:
        p_db_acl_entry->arp_op_code_mask = p_acl_entry->arp_op_code_mask;
        break;

    case TBL_ACL_ENTRY_FLD_INVALID:
        p_db_acl_entry->invalid = p_acl_entry->invalid;
        break;

    case TBL_ACL_ENTRY_FLD_IS_COPP:
        p_db_acl_entry->is_copp = p_acl_entry->is_copp;
        break;

    case TBL_ACL_ENTRY_FLD_IN_PORT:
        p_db_acl_entry->in_port = p_acl_entry->in_port;
        break;

    case TBL_ACL_ENTRY_FLD_OUT_PORT:
        p_db_acl_entry->out_port = p_acl_entry->out_port;
        break;

    case TBL_ACL_ENTRY_FLD_ETHER_TYPE:
        p_db_acl_entry->ether_type = p_acl_entry->ether_type;
        break;

    case TBL_ACL_ENTRY_FLD_ETHER_TYPE_MASK:
        p_db_acl_entry->ether_type_mask = p_acl_entry->ether_type_mask;
        break;

    case TBL_ACL_ENTRY_FLD_SRC_MAC:
        sal_memcpy(p_db_acl_entry->src_mac, p_acl_entry->src_mac, sizeof(p_acl_entry->src_mac));
        break;

    case TBL_ACL_ENTRY_FLD_SRC_MAC_MASK:
        sal_memcpy(p_db_acl_entry->src_mac_mask, p_acl_entry->src_mac_mask, sizeof(p_acl_entry->src_mac_mask));
        break;

    case TBL_ACL_ENTRY_FLD_DST_MAC:
        sal_memcpy(p_db_acl_entry->dst_mac, p_acl_entry->dst_mac, sizeof(p_acl_entry->dst_mac));
        break;

    case TBL_ACL_ENTRY_FLD_DST_MAC_MASK:
        sal_memcpy(p_db_acl_entry->dst_mac_mask, p_acl_entry->dst_mac_mask, sizeof(p_acl_entry->dst_mac_mask));
        break;

    case TBL_ACL_ENTRY_FLD_SVLAN:
        p_db_acl_entry->svlan = p_acl_entry->svlan;
        break;

    case TBL_ACL_ENTRY_FLD_SVLAN_MASK:
        p_db_acl_entry->svlan_mask = p_acl_entry->svlan_mask;
        break;

    case TBL_ACL_ENTRY_FLD_SVLAN_COS:
        p_db_acl_entry->svlan_cos = p_acl_entry->svlan_cos;
        break;

    case TBL_ACL_ENTRY_FLD_SVLAN_COS_MASK:
        p_db_acl_entry->svlan_cos_mask = p_acl_entry->svlan_cos_mask;
        break;

    case TBL_ACL_ENTRY_FLD_CVLAN:
        p_db_acl_entry->cvlan = p_acl_entry->cvlan;
        break;

    case TBL_ACL_ENTRY_FLD_CVLAN_MASK:
        p_db_acl_entry->cvlan_mask = p_acl_entry->cvlan_mask;
        break;

    case TBL_ACL_ENTRY_FLD_CVLAN_COS:
        p_db_acl_entry->cvlan_cos = p_acl_entry->cvlan_cos;
        break;

    case TBL_ACL_ENTRY_FLD_CVLAN_COS_MASK:
        p_db_acl_entry->cvlan_cos_mask = p_acl_entry->cvlan_cos_mask;
        break;

    case TBL_ACL_ENTRY_FLD_UNTAG_VLAN:
        p_db_acl_entry->untag_vlan = p_acl_entry->untag_vlan;
        break;

    case TBL_ACL_ENTRY_FLD_IP_TYPE:
        p_db_acl_entry->ip_type = p_acl_entry->ip_type;
        break;

    case TBL_ACL_ENTRY_FLD_IP_TYPE_MASK:
        p_db_acl_entry->ip_type_mask = p_acl_entry->ip_type_mask;
        break;

    case TBL_ACL_ENTRY_FLD_IP_FLAGS:
        p_db_acl_entry->ip_flags = p_acl_entry->ip_flags;
        break;

    case TBL_ACL_ENTRY_FLD_IP_FLAGS_MASK:
        p_db_acl_entry->ip_flags_mask = p_acl_entry->ip_flags_mask;
        break;

    case TBL_ACL_ENTRY_FLD_SRC_IP:
        sal_memcpy(&p_db_acl_entry->src_ip, &p_acl_entry->src_ip, sizeof(p_acl_entry->src_ip));
        break;

    case TBL_ACL_ENTRY_FLD_SRC_IP_MASK:
        sal_memcpy(&p_db_acl_entry->src_ip_mask, &p_acl_entry->src_ip_mask, sizeof(p_acl_entry->src_ip_mask));
        break;

    case TBL_ACL_ENTRY_FLD_DST_IP:
        sal_memcpy(&p_db_acl_entry->dst_ip, &p_acl_entry->dst_ip, sizeof(p_acl_entry->dst_ip));
        break;

    case TBL_ACL_ENTRY_FLD_DST_IP_MASK:
        sal_memcpy(&p_db_acl_entry->dst_ip_mask, &p_acl_entry->dst_ip_mask, sizeof(p_acl_entry->dst_ip_mask));
        break;

    case TBL_ACL_ENTRY_FLD_IPV6_FLOW_LABEL:
        p_db_acl_entry->ipv6_flow_label = p_acl_entry->ipv6_flow_label;
        break;

    case TBL_ACL_ENTRY_FLD_IPV6_FLOW_LABEL_MASK:
        p_db_acl_entry->ipv6_flow_label_mask = p_acl_entry->ipv6_flow_label_mask;
        break;

    case TBL_ACL_ENTRY_FLD_DSCP:
        p_db_acl_entry->dscp = p_acl_entry->dscp;
        break;

    case TBL_ACL_ENTRY_FLD_DSCP_MASK:
        p_db_acl_entry->dscp_mask = p_acl_entry->dscp_mask;
        break;

    case TBL_ACL_ENTRY_FLD_IP_PRECEDENCE:
        p_db_acl_entry->ip_precedence = p_acl_entry->ip_precedence;
        break;

    case TBL_ACL_ENTRY_FLD_IP_PRECEDENCE_MASK:
        p_db_acl_entry->ip_precedence_mask = p_acl_entry->ip_precedence_mask;
        break;

    case TBL_ACL_ENTRY_FLD_IP_PROTOCOL:
        p_db_acl_entry->ip_protocol = p_acl_entry->ip_protocol;
        break;

    case TBL_ACL_ENTRY_FLD_IP_PROTOCOL_MASK:
        p_db_acl_entry->ip_protocol_mask = p_acl_entry->ip_protocol_mask;
        break;

    case TBL_ACL_ENTRY_FLD_L4_SRC_PORT_TYPE:
        p_db_acl_entry->l4_src_port_type = p_acl_entry->l4_src_port_type;
        break;

    case TBL_ACL_ENTRY_FLD_L4_SRC_PORT:
        p_db_acl_entry->l4_src_port = p_acl_entry->l4_src_port;
        break;

    case TBL_ACL_ENTRY_FLD_L4_SRC_PORT_MASK:
        p_db_acl_entry->l4_src_port_mask = p_acl_entry->l4_src_port_mask;
        break;

    case TBL_ACL_ENTRY_FLD_L4_DST_PORT_TYPE:
        p_db_acl_entry->l4_dst_port_type = p_acl_entry->l4_dst_port_type;
        break;

    case TBL_ACL_ENTRY_FLD_L4_DST_PORT:
        p_db_acl_entry->l4_dst_port = p_acl_entry->l4_dst_port;
        break;

    case TBL_ACL_ENTRY_FLD_L4_DST_PORT_MASK:
        p_db_acl_entry->l4_dst_port_mask = p_acl_entry->l4_dst_port_mask;
        break;

    case TBL_ACL_ENTRY_FLD_IP_FRAG:
        p_db_acl_entry->ip_frag = p_acl_entry->ip_frag;
        break;

    case TBL_ACL_ENTRY_FLD_TCP_FLAGS:
        p_db_acl_entry->tcp_flags = p_acl_entry->tcp_flags;
        break;

    case TBL_ACL_ENTRY_FLD_TCP_FLAGS_MASK:
        p_db_acl_entry->tcp_flags_mask = p_acl_entry->tcp_flags_mask;
        break;

    case TBL_ACL_ENTRY_FLD_IGMP_TYPE:
        p_db_acl_entry->igmp_type = p_acl_entry->igmp_type;
        break;

    case TBL_ACL_ENTRY_FLD_ICMP_TYPE:
        p_db_acl_entry->icmp_type = p_acl_entry->icmp_type;
        break;

    case TBL_ACL_ENTRY_FLD_ICMP_CODE:
        p_db_acl_entry->icmp_code = p_acl_entry->icmp_code;
        break;

    case TBL_ACL_ENTRY_FLD_UDF_ID:
        p_db_acl_entry->udf_id = p_acl_entry->udf_id;
        break;

    case TBL_ACL_ENTRY_FLD_UDF_TYPE:
        p_db_acl_entry->udf_type = p_acl_entry->udf_type;
        break;

    case TBL_ACL_ENTRY_FLD_UDF_OFFSET:
        p_db_acl_entry->udf_offset = p_acl_entry->udf_offset;
        break;

    case TBL_ACL_ENTRY_FLD_UDF_VALUE:
        sal_memcpy(p_db_acl_entry->udf_value, p_acl_entry->udf_value, sizeof(p_acl_entry->udf_value));
        break;

    case TBL_ACL_ENTRY_FLD_UDF_MASK:
        sal_memcpy(p_db_acl_entry->udf_mask, p_acl_entry->udf_mask, sizeof(p_acl_entry->udf_mask));
        break;

    case TBL_ACL_ENTRY_FLD_UDF_VALUE_VALID:
        sal_memcpy(p_db_acl_entry->udf_value_valid, p_acl_entry->udf_value_valid, sizeof(p_acl_entry->udf_value_valid));
        break;

    case TBL_ACL_ENTRY_FLD_ERSPAN_ID:
        p_db_acl_entry->erspan_id = p_acl_entry->erspan_id;
        break;

    case TBL_ACL_ENTRY_FLD_ERSPAN_ID_MASK:
        p_db_acl_entry->erspan_id_mask = p_acl_entry->erspan_id_mask;
        break;

    case TBL_ACL_ENTRY_FLD_STRIP_HEADER_POS:
        p_db_acl_entry->strip_header_pos = p_acl_entry->strip_header_pos;
        break;

    case TBL_ACL_ENTRY_FLD_STRIP_HEADER_OFFSET:
        p_db_acl_entry->strip_header_offset = p_acl_entry->strip_header_offset;
        break;

    case TBL_ACL_ENTRY_FLD_L4_VXLAN_VNI:
        p_db_acl_entry->l4_vxlan_vni = p_acl_entry->l4_vxlan_vni;
        break;

    case TBL_ACL_ENTRY_FLD_L4_VXLAN_VNI_MASK:
        p_db_acl_entry->l4_vxlan_vni_mask = p_acl_entry->l4_vxlan_vni_mask;
        break;

    case TBL_ACL_ENTRY_FLD_L4_GRE_TYPE:
        p_db_acl_entry->l4_gre_type = p_acl_entry->l4_gre_type;
        break;

    case TBL_ACL_ENTRY_FLD_L4_GRE_KEY:
        p_db_acl_entry->l4_gre_key = p_acl_entry->l4_gre_key;
        break;

    case TBL_ACL_ENTRY_FLD_L4_GRE_KEY_MASK:
        p_db_acl_entry->l4_gre_key_mask = p_acl_entry->l4_gre_key_mask;
        break;

    case TBL_ACL_ENTRY_FLD_L2GRE_ADD_HEADER:
        p_db_acl_entry->l2gre_add_header = p_acl_entry->l2gre_add_header;
        break;

    case TBL_ACL_ENTRY_FLD_L2GRE_SRC_IP:
        sal_memcpy(&p_db_acl_entry->l2gre_src_ip, &p_acl_entry->l2gre_src_ip, sizeof(p_acl_entry->l2gre_src_ip));
        break;

    case TBL_ACL_ENTRY_FLD_L2GRE_DST_IP:
        sal_memcpy(&p_db_acl_entry->l2gre_dst_ip, &p_acl_entry->l2gre_dst_ip, sizeof(p_acl_entry->l2gre_dst_ip));
        break;

    case TBL_ACL_ENTRY_FLD_L2GRE_DST_MAC:
        sal_memcpy(p_db_acl_entry->l2gre_dst_mac, p_acl_entry->l2gre_dst_mac, sizeof(p_acl_entry->l2gre_dst_mac));
        break;

    case TBL_ACL_ENTRY_FLD_L2GRE_KEY_NUM:
        p_db_acl_entry->l2gre_key_num = p_acl_entry->l2gre_key_num;
        break;

    case TBL_ACL_ENTRY_FLD_L2GRE_KEY_LEN:
        p_db_acl_entry->l2gre_key_len = p_acl_entry->l2gre_key_len;
        break;

    case TBL_ACL_ENTRY_FLD_L3GRE_ADD_HEADER:
        p_db_acl_entry->l3gre_add_header = p_acl_entry->l3gre_add_header;
        break;

    case TBL_ACL_ENTRY_FLD_L3GRE_SRC_IP:
        sal_memcpy(&p_db_acl_entry->l3gre_src_ip, &p_acl_entry->l3gre_src_ip, sizeof(p_acl_entry->l3gre_src_ip));
        break;

    case TBL_ACL_ENTRY_FLD_L3GRE_DST_IP:
        sal_memcpy(&p_db_acl_entry->l3gre_dst_ip, &p_acl_entry->l3gre_dst_ip, sizeof(p_acl_entry->l3gre_dst_ip));
        break;

    case TBL_ACL_ENTRY_FLD_L3GRE_DST_MAC:
        sal_memcpy(p_db_acl_entry->l3gre_dst_mac, p_acl_entry->l3gre_dst_mac, sizeof(p_acl_entry->l3gre_dst_mac));
        break;

    case TBL_ACL_ENTRY_FLD_CATEGORY_ID:
        p_db_acl_entry->category_id = p_acl_entry->category_id;
        break;

    case TBL_ACL_ENTRY_FLD_DENY:
        p_db_acl_entry->deny = p_acl_entry->deny;
        break;

    case TBL_ACL_ENTRY_FLD_STATS_EN:
        p_db_acl_entry->stats_en = p_acl_entry->stats_en;
        break;

    case TBL_ACL_ENTRY_FLD_OPTIONS:
        p_db_acl_entry->options = p_acl_entry->options;
        break;

    case TBL_ACL_ENTRY_FLD_INNER_KEY_FLAGS:
        p_db_acl_entry->inner_key_flags = p_acl_entry->inner_key_flags;
        break;

    case TBL_ACL_ENTRY_FLD_INNER_KEY_IPV6_FLAGS:
        p_db_acl_entry->inner_key_ipv6_flags = p_acl_entry->inner_key_ipv6_flags;
        break;

    case TBL_ACL_ENTRY_FLD_INNER_IS_IPV6:
        p_db_acl_entry->inner_is_ipv6 = p_acl_entry->inner_is_ipv6;
        break;

    case TBL_ACL_ENTRY_FLD_INNER_ETHER_TYPE:
        p_db_acl_entry->inner_ether_type = p_acl_entry->inner_ether_type;
        break;

    case TBL_ACL_ENTRY_FLD_INNER_ETHER_TYPE_MASK:
        p_db_acl_entry->inner_ether_type_mask = p_acl_entry->inner_ether_type_mask;
        break;

    case TBL_ACL_ENTRY_FLD_INNER_SRC_MAC:
        sal_memcpy(p_db_acl_entry->inner_src_mac, p_acl_entry->inner_src_mac, sizeof(p_acl_entry->inner_src_mac));
        break;

    case TBL_ACL_ENTRY_FLD_INNER_SRC_MAC_MASK:
        sal_memcpy(p_db_acl_entry->inner_src_mac_mask, p_acl_entry->inner_src_mac_mask, sizeof(p_acl_entry->inner_src_mac_mask));
        break;

    case TBL_ACL_ENTRY_FLD_INNER_DST_MAC:
        sal_memcpy(p_db_acl_entry->inner_dst_mac, p_acl_entry->inner_dst_mac, sizeof(p_acl_entry->inner_dst_mac));
        break;

    case TBL_ACL_ENTRY_FLD_INNER_DST_MAC_MASK:
        sal_memcpy(p_db_acl_entry->inner_dst_mac_mask, p_acl_entry->inner_dst_mac_mask, sizeof(p_acl_entry->inner_dst_mac_mask));
        break;

    case TBL_ACL_ENTRY_FLD_INNER_SVLAN:
        p_db_acl_entry->inner_svlan = p_acl_entry->inner_svlan;
        break;

    case TBL_ACL_ENTRY_FLD_INNER_SVLAN_MASK:
        p_db_acl_entry->inner_svlan_mask = p_acl_entry->inner_svlan_mask;
        break;

    case TBL_ACL_ENTRY_FLD_INNER_SVLAN_COS:
        p_db_acl_entry->inner_svlan_cos = p_acl_entry->inner_svlan_cos;
        break;

    case TBL_ACL_ENTRY_FLD_INNER_SVLAN_COS_MASK:
        p_db_acl_entry->inner_svlan_cos_mask = p_acl_entry->inner_svlan_cos_mask;
        break;

    case TBL_ACL_ENTRY_FLD_INNER_CVLAN:
        p_db_acl_entry->inner_cvlan = p_acl_entry->inner_cvlan;
        break;

    case TBL_ACL_ENTRY_FLD_INNER_CVLAN_MASK:
        p_db_acl_entry->inner_cvlan_mask = p_acl_entry->inner_cvlan_mask;
        break;

    case TBL_ACL_ENTRY_FLD_INNER_CVLAN_COS:
        p_db_acl_entry->inner_cvlan_cos = p_acl_entry->inner_cvlan_cos;
        break;

    case TBL_ACL_ENTRY_FLD_INNER_CVLAN_COS_MASK:
        p_db_acl_entry->inner_cvlan_cos_mask = p_acl_entry->inner_cvlan_cos_mask;
        break;

    case TBL_ACL_ENTRY_FLD_INNER_IP_TYPE:
        p_db_acl_entry->inner_ip_type = p_acl_entry->inner_ip_type;
        break;

    case TBL_ACL_ENTRY_FLD_INNER_IP_TYPE_MASK:
        p_db_acl_entry->inner_ip_type_mask = p_acl_entry->inner_ip_type_mask;
        break;

    case TBL_ACL_ENTRY_FLD_INNER_IP_FLAGS:
        p_db_acl_entry->inner_ip_flags = p_acl_entry->inner_ip_flags;
        break;

    case TBL_ACL_ENTRY_FLD_INNER_IP_FLAGS_MASK:
        p_db_acl_entry->inner_ip_flags_mask = p_acl_entry->inner_ip_flags_mask;
        break;

    case TBL_ACL_ENTRY_FLD_INNER_SRC_IP:
        sal_memcpy(&p_db_acl_entry->inner_src_ip, &p_acl_entry->inner_src_ip, sizeof(p_acl_entry->inner_src_ip));
        break;

    case TBL_ACL_ENTRY_FLD_INNER_SRC_IP_MASK:
        sal_memcpy(&p_db_acl_entry->inner_src_ip_mask, &p_acl_entry->inner_src_ip_mask, sizeof(p_acl_entry->inner_src_ip_mask));
        break;

    case TBL_ACL_ENTRY_FLD_INNER_DST_IP:
        sal_memcpy(&p_db_acl_entry->inner_dst_ip, &p_acl_entry->inner_dst_ip, sizeof(p_acl_entry->inner_dst_ip));
        break;

    case TBL_ACL_ENTRY_FLD_INNER_DST_IP_MASK:
        sal_memcpy(&p_db_acl_entry->inner_dst_ip_mask, &p_acl_entry->inner_dst_ip_mask, sizeof(p_acl_entry->inner_dst_ip_mask));
        break;

    case TBL_ACL_ENTRY_FLD_INNER_IPV6_FLOW_LABEL:
        p_db_acl_entry->inner_ipv6_flow_label = p_acl_entry->inner_ipv6_flow_label;
        break;

    case TBL_ACL_ENTRY_FLD_INNER_IPV6_FLOW_LABEL_MASK:
        p_db_acl_entry->inner_ipv6_flow_label_mask = p_acl_entry->inner_ipv6_flow_label_mask;
        break;

    case TBL_ACL_ENTRY_FLD_INNER_DSCP:
        p_db_acl_entry->inner_dscp = p_acl_entry->inner_dscp;
        break;

    case TBL_ACL_ENTRY_FLD_INNER_DSCP_MASK:
        p_db_acl_entry->inner_dscp_mask = p_acl_entry->inner_dscp_mask;
        break;

    case TBL_ACL_ENTRY_FLD_INNER_IP_PRECEDENCE:
        p_db_acl_entry->inner_ip_precedence = p_acl_entry->inner_ip_precedence;
        break;

    case TBL_ACL_ENTRY_FLD_INNER_IP_PRECEDENCE_MASK:
        p_db_acl_entry->inner_ip_precedence_mask = p_acl_entry->inner_ip_precedence_mask;
        break;

    case TBL_ACL_ENTRY_FLD_INNER_IP_PROTOCOL:
        p_db_acl_entry->inner_ip_protocol = p_acl_entry->inner_ip_protocol;
        break;

    case TBL_ACL_ENTRY_FLD_INNER_IP_PROTOCOL_MASK:
        p_db_acl_entry->inner_ip_protocol_mask = p_acl_entry->inner_ip_protocol_mask;
        break;

    case TBL_ACL_ENTRY_FLD_INNER_L4_SRC_PORT_TYPE:
        p_db_acl_entry->inner_l4_src_port_type = p_acl_entry->inner_l4_src_port_type;
        break;

    case TBL_ACL_ENTRY_FLD_INNER_L4_SRC_PORT:
        p_db_acl_entry->inner_l4_src_port = p_acl_entry->inner_l4_src_port;
        break;

    case TBL_ACL_ENTRY_FLD_INNER_L4_SRC_PORT_MASK:
        p_db_acl_entry->inner_l4_src_port_mask = p_acl_entry->inner_l4_src_port_mask;
        break;

    case TBL_ACL_ENTRY_FLD_INNER_L4_DST_PORT_TYPE:
        p_db_acl_entry->inner_l4_dst_port_type = p_acl_entry->inner_l4_dst_port_type;
        break;

    case TBL_ACL_ENTRY_FLD_INNER_L4_DST_PORT:
        p_db_acl_entry->inner_l4_dst_port = p_acl_entry->inner_l4_dst_port;
        break;

    case TBL_ACL_ENTRY_FLD_INNER_L4_DST_PORT_MASK:
        p_db_acl_entry->inner_l4_dst_port_mask = p_acl_entry->inner_l4_dst_port_mask;
        break;

    case TBL_ACL_ENTRY_FLD_INNER_IP_FRAG:
        p_db_acl_entry->inner_ip_frag = p_acl_entry->inner_ip_frag;
        break;

    case TBL_ACL_ENTRY_FLD_INNER_TCP_FLAGS:
        p_db_acl_entry->inner_tcp_flags = p_acl_entry->inner_tcp_flags;
        break;

    case TBL_ACL_ENTRY_FLD_INNER_TCP_FLAGS_MASK:
        p_db_acl_entry->inner_tcp_flags_mask = p_acl_entry->inner_tcp_flags_mask;
        break;

    case TBL_ACL_ENTRY_FLD_INNER_IGMP_TYPE:
        p_db_acl_entry->inner_igmp_type = p_acl_entry->inner_igmp_type;
        break;

    case TBL_ACL_ENTRY_FLD_INNER_ICMP_TYPE:
        p_db_acl_entry->inner_icmp_type = p_acl_entry->inner_icmp_type;
        break;

    case TBL_ACL_ENTRY_FLD_INNER_ICMP_CODE:
        p_db_acl_entry->inner_icmp_code = p_acl_entry->inner_icmp_code;
        break;

    case TBL_ACL_ENTRY_FLD_INNER_OPTIONS:
        p_db_acl_entry->inner_options = p_acl_entry->inner_options;
        break;

    case TBL_ACL_ENTRY_FLD_ACTION_STRIP_HEADER:
        p_db_acl_entry->action_strip_header = p_acl_entry->action_strip_header;
        break;

    case TBL_ACL_ENTRY_FLD_TAP_ACTION_REDIRECT:
        p_db_acl_entry->tap_action_redirect = p_acl_entry->tap_action_redirect;
        break;

    case TBL_ACL_ENTRY_FLD_TAP_ACTION_MARK_VLAN:
        p_db_acl_entry->tap_action_mark_vlan = p_acl_entry->tap_action_mark_vlan;
        break;

    case TBL_ACL_ENTRY_FLD_TAP_ACTION_UNTAG:
        p_db_acl_entry->tap_action_untag = p_acl_entry->tap_action_untag;
        break;

    case TBL_ACL_ENTRY_FLD_TAP_ACTION_TRUNCTION:
        p_db_acl_entry->tap_action_trunction = p_acl_entry->tap_action_trunction;
        break;

    case TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_DEST_MAC_EN:
        p_db_acl_entry->tap_action_edit_dest_mac_en = p_acl_entry->tap_action_edit_dest_mac_en;
        break;

    case TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_DEST_MAC:
        sal_memcpy(p_db_acl_entry->tap_action_edit_dest_mac, p_acl_entry->tap_action_edit_dest_mac, sizeof(p_acl_entry->tap_action_edit_dest_mac));
        break;

    case TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_SRC_MAC_EN:
        p_db_acl_entry->tap_action_edit_src_mac_en = p_acl_entry->tap_action_edit_src_mac_en;
        break;

    case TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_SRC_MAC:
        sal_memcpy(p_db_acl_entry->tap_action_edit_src_mac, p_acl_entry->tap_action_edit_src_mac, sizeof(p_acl_entry->tap_action_edit_src_mac));
        break;

    case TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_IPDA_EN:
        p_db_acl_entry->tap_action_edit_ipda_en = p_acl_entry->tap_action_edit_ipda_en;
        break;

    case TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_IPDA:
        sal_memcpy(&p_db_acl_entry->tap_action_edit_ipda, &p_acl_entry->tap_action_edit_ipda, sizeof(p_acl_entry->tap_action_edit_ipda));
        break;

    case TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_IPSA_EN:
        p_db_acl_entry->tap_action_edit_ipsa_en = p_acl_entry->tap_action_edit_ipsa_en;
        break;

    case TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_IPSA:
        sal_memcpy(&p_db_acl_entry->tap_action_edit_ipsa, &p_acl_entry->tap_action_edit_ipsa, sizeof(p_acl_entry->tap_action_edit_ipsa));
        break;

    case TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_VLAN_EN:
        p_db_acl_entry->tap_action_edit_vlan_en = p_acl_entry->tap_action_edit_vlan_en;
        break;

    case TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_VLAN:
        p_db_acl_entry->tap_action_edit_vlan = p_acl_entry->tap_action_edit_vlan;
        break;

    case TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_VLAN_ACTION:
        p_db_acl_entry->tap_action_edit_vlan_action = p_acl_entry->tap_action_edit_vlan_action;
        break;

    case TBL_ACL_ENTRY_FLD_TAP_GROUP_OID:
        p_db_acl_entry->tap_group_oid = p_acl_entry->tap_group_oid;
        break;

    case TBL_ACL_ENTRY_FLD_ADD_VXLAN_HEADER:
        p_db_acl_entry->add_vxlan_header = p_acl_entry->add_vxlan_header;
        break;

    case TBL_ACL_ENTRY_FLD_VXLAN_DST_MAC:
        sal_memcpy(p_db_acl_entry->vxlan_dst_mac, p_acl_entry->vxlan_dst_mac, sizeof(p_acl_entry->vxlan_dst_mac));
        break;

    case TBL_ACL_ENTRY_FLD_VXLAN_SRC_IP:
        sal_memcpy(&p_db_acl_entry->vxlan_src_ip, &p_acl_entry->vxlan_src_ip, sizeof(p_acl_entry->vxlan_src_ip));
        break;

    case TBL_ACL_ENTRY_FLD_VXLAN_DST_IP:
        sal_memcpy(&p_db_acl_entry->vxlan_dst_ip, &p_acl_entry->vxlan_dst_ip, sizeof(p_acl_entry->vxlan_dst_ip));
        break;

    case TBL_ACL_ENTRY_FLD_VXLAN_SRCPORT:
        p_db_acl_entry->vxlan_srcport = p_acl_entry->vxlan_srcport;
        break;

    case TBL_ACL_ENTRY_FLD_VXLAN_DESTPORT:
        p_db_acl_entry->vxlan_destport = p_acl_entry->vxlan_destport;
        break;

    case TBL_ACL_ENTRY_FLD_VXLAN_VNI:
        p_db_acl_entry->vxlan_vni = p_acl_entry->vxlan_vni;
        break;

    case TBL_ACL_ENTRY_FLD_MAX:
        sal_memcpy(p_db_acl_entry, p_acl_entry, sizeof(tbl_acl_entry_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_ACL_ENTRY, field_id, p_db_acl_entry);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_ACL_ENTRY, field_id, p_db_acl_entry);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_acl_entry_add_acl_entry(tbl_acl_entry_t *p_acl_entry)
{
    return tbl_acl_entry_add_acl_entry_sync(p_acl_entry, TRUE);
}

int32
tbl_acl_entry_del_acl_entry(tbl_acl_entry_key_t *p_acl_entry_key)
{
    return tbl_acl_entry_del_acl_entry_sync(p_acl_entry_key, TRUE);
}

int32
tbl_acl_entry_set_acl_entry_field(tbl_acl_entry_t *p_acl_entry, tbl_acl_entry_field_id_t field_id)
{
    return tbl_acl_entry_set_acl_entry_field_sync(p_acl_entry, field_id, TRUE);
}

tbl_acl_entry_t*
tbl_acl_entry_get_acl_entry(tbl_acl_entry_key_t *p_acl_entry_key)
{
    tbl_acl_entry_master_t *p_master = _g_p_tbl_acl_entry_master;
    tbl_acl_entry_t lkp;

    sal_memcpy(&lkp.key, p_acl_entry_key, sizeof(tbl_acl_entry_key_t));
    return ctclib_hash_lookup(p_master->acl_entry_hash, &lkp);
}

char*
tbl_acl_entry_key_val2str(tbl_acl_entry_t *p_acl_entry, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%"PRIu64, p_acl_entry->key.aclid);
    return str;
}

int32
tbl_acl_entry_key_str2val(char *str, tbl_acl_entry_t *p_acl_entry)
{
    int32 ret = 0;

    p_acl_entry->key.aclid = cdb_uint64_str2val(str, &ret);
    return ret;
}

char*
tbl_acl_entry_key_name_dump(tbl_acl_entry_t *p_acl_entry, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_ENTRY);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
        "%"PRIu64, p_node->name, p_acl_entry->key.aclid);
    return str;
}

char*
tbl_acl_entry_key_value_dump(tbl_acl_entry_t *p_acl_entry, char *str)
{
    sal_sprintf(str, "%"PRIu64, p_acl_entry->key.aclid);
    return str;
}

char*
tbl_acl_entry_field_name_dump(tbl_acl_entry_t *p_acl_entry, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_ENTRY);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_ACL_ENTRY_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_acl_entry_field_value_dump(tbl_acl_entry_t *p_acl_entry, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_acl_entry->key.aclid);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TBLID, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->tblid);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_ENTRY_PRIORITY, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->entry_priority);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TIME_RANGE, field_id))
    {
        sal_sprintf(str, "%s", p_acl_entry->time_range);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_KEY_FLAGS, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->key_flags);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_KEY_FLAGS2, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->key_flags2);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_KEY_IPV6_FLAGS, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->key_ipv6_flags);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IS_IPV6, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->is_ipv6);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_ACL_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->acl_type);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_KEY_ARP_FLAGS, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->key_arp_flags);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_ARP_OP_CODE, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->arp_op_code);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_ARP_OP_CODE_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->arp_op_code_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INVALID, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->invalid);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IS_COPP, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->is_copp);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IN_PORT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_acl_entry->in_port);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_OUT_PORT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_acl_entry->out_port);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_ETHER_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->ether_type);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_ETHER_TYPE_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->ether_type_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_SRC_MAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_acl_entry->src_mac);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_SRC_MAC_MASK, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_acl_entry->src_mac_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_DST_MAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_acl_entry->dst_mac);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_DST_MAC_MASK, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_acl_entry->dst_mac_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_SVLAN, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->svlan);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_SVLAN_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->svlan_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_SVLAN_COS, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->svlan_cos);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_SVLAN_COS_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->svlan_cos_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_CVLAN, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->cvlan);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_CVLAN_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->cvlan_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_CVLAN_COS, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->cvlan_cos);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_CVLAN_COS_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->cvlan_cos_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_UNTAG_VLAN, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->untag_vlan);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IP_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->ip_type);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IP_TYPE_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->ip_type_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IP_FLAGS, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->ip_flags);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IP_FLAGS_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->ip_flags_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_SRC_IP, field_id))
    {
        cdb_prefix_val2str(str, MAX_CMD_STR_LEN, &p_acl_entry->src_ip);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_SRC_IP_MASK, field_id))
    {
        cdb_prefix_val2str(str, MAX_CMD_STR_LEN, &p_acl_entry->src_ip_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_DST_IP, field_id))
    {
        cdb_prefix_val2str(str, MAX_CMD_STR_LEN, &p_acl_entry->dst_ip);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_DST_IP_MASK, field_id))
    {
        cdb_prefix_val2str(str, MAX_CMD_STR_LEN, &p_acl_entry->dst_ip_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IPV6_FLOW_LABEL, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->ipv6_flow_label);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IPV6_FLOW_LABEL_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->ipv6_flow_label_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_DSCP, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->dscp);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_DSCP_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->dscp_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IP_PRECEDENCE, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->ip_precedence);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IP_PRECEDENCE_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->ip_precedence_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IP_PROTOCOL, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->ip_protocol);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IP_PROTOCOL_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->ip_protocol_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L4_SRC_PORT_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->l4_src_port_type);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L4_SRC_PORT, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->l4_src_port);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L4_SRC_PORT_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->l4_src_port_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L4_DST_PORT_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->l4_dst_port_type);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L4_DST_PORT, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->l4_dst_port);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L4_DST_PORT_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->l4_dst_port_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IP_FRAG, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->ip_frag);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TCP_FLAGS, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->tcp_flags);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TCP_FLAGS_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->tcp_flags_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IGMP_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->igmp_type);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_ICMP_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->icmp_type);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_ICMP_CODE, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->icmp_code);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_UDF_ID, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->udf_id);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_UDF_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->udf_type);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_UDF_OFFSET, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->udf_offset);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_UDF_VALUE, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_acl_entry->udf_value, sizeof(p_acl_entry->udf_value)/4);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_UDF_MASK, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_acl_entry->udf_mask, sizeof(p_acl_entry->udf_mask)/4);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_UDF_VALUE_VALID, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_acl_entry->udf_value_valid, sizeof(p_acl_entry->udf_value_valid)/4);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_ERSPAN_ID, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->erspan_id);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_ERSPAN_ID_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->erspan_id_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_STRIP_HEADER_POS, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->strip_header_pos);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_STRIP_HEADER_OFFSET, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->strip_header_offset);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L4_VXLAN_VNI, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->l4_vxlan_vni);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L4_VXLAN_VNI_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->l4_vxlan_vni_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L4_GRE_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->l4_gre_type);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L4_GRE_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->l4_gre_key);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L4_GRE_KEY_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->l4_gre_key_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L2GRE_ADD_HEADER, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->l2gre_add_header);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L2GRE_SRC_IP, field_id))
    {
        cdb_addr_val2str(str, MAX_CMD_STR_LEN, &p_acl_entry->l2gre_src_ip);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L2GRE_DST_IP, field_id))
    {
        cdb_addr_val2str(str, MAX_CMD_STR_LEN, &p_acl_entry->l2gre_dst_ip);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L2GRE_DST_MAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_acl_entry->l2gre_dst_mac);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L2GRE_KEY_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->l2gre_key_num);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L2GRE_KEY_LEN, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->l2gre_key_len);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L3GRE_ADD_HEADER, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->l3gre_add_header);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L3GRE_SRC_IP, field_id))
    {
        cdb_addr_val2str(str, MAX_CMD_STR_LEN, &p_acl_entry->l3gre_src_ip);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L3GRE_DST_IP, field_id))
    {
        cdb_addr_val2str(str, MAX_CMD_STR_LEN, &p_acl_entry->l3gre_dst_ip);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L3GRE_DST_MAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_acl_entry->l3gre_dst_mac);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_CATEGORY_ID, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->category_id);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_DENY, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->deny);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_STATS_EN, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->stats_en);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_OPTIONS, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->options);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_KEY_FLAGS, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_acl_entry->inner_key_flags);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_KEY_IPV6_FLAGS, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->inner_key_ipv6_flags);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_IS_IPV6, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->inner_is_ipv6);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_ETHER_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->inner_ether_type);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_ETHER_TYPE_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->inner_ether_type_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_SRC_MAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_acl_entry->inner_src_mac);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_SRC_MAC_MASK, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_acl_entry->inner_src_mac_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_DST_MAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_acl_entry->inner_dst_mac);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_DST_MAC_MASK, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_acl_entry->inner_dst_mac_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_SVLAN, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->inner_svlan);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_SVLAN_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->inner_svlan_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_SVLAN_COS, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->inner_svlan_cos);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_SVLAN_COS_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->inner_svlan_cos_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_CVLAN, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->inner_cvlan);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_CVLAN_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->inner_cvlan_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_CVLAN_COS, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->inner_cvlan_cos);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_CVLAN_COS_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->inner_cvlan_cos_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_IP_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->inner_ip_type);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_IP_TYPE_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->inner_ip_type_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_IP_FLAGS, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->inner_ip_flags);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_IP_FLAGS_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->inner_ip_flags_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_SRC_IP, field_id))
    {
        cdb_prefix_val2str(str, MAX_CMD_STR_LEN, &p_acl_entry->inner_src_ip);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_SRC_IP_MASK, field_id))
    {
        cdb_prefix_val2str(str, MAX_CMD_STR_LEN, &p_acl_entry->inner_src_ip_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_DST_IP, field_id))
    {
        cdb_prefix_val2str(str, MAX_CMD_STR_LEN, &p_acl_entry->inner_dst_ip);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_DST_IP_MASK, field_id))
    {
        cdb_prefix_val2str(str, MAX_CMD_STR_LEN, &p_acl_entry->inner_dst_ip_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_IPV6_FLOW_LABEL, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->inner_ipv6_flow_label);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_IPV6_FLOW_LABEL_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->inner_ipv6_flow_label_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_DSCP, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->inner_dscp);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_DSCP_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->inner_dscp_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_IP_PRECEDENCE, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->inner_ip_precedence);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_IP_PRECEDENCE_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->inner_ip_precedence_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_IP_PROTOCOL, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->inner_ip_protocol);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_IP_PROTOCOL_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->inner_ip_protocol_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_L4_SRC_PORT_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->inner_l4_src_port_type);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_L4_SRC_PORT, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->inner_l4_src_port);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_L4_SRC_PORT_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->inner_l4_src_port_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_L4_DST_PORT_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->inner_l4_dst_port_type);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_L4_DST_PORT, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->inner_l4_dst_port);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_L4_DST_PORT_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->inner_l4_dst_port_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_IP_FRAG, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->inner_ip_frag);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_TCP_FLAGS, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->inner_tcp_flags);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_TCP_FLAGS_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->inner_tcp_flags_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_IGMP_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->inner_igmp_type);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_ICMP_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->inner_icmp_type);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_ICMP_CODE, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->inner_icmp_code);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_OPTIONS, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->inner_options);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_ACTION_STRIP_HEADER, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->action_strip_header);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_REDIRECT, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->tap_action_redirect);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_MARK_VLAN, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->tap_action_mark_vlan);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_UNTAG, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->tap_action_untag);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_TRUNCTION, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->tap_action_trunction);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_DEST_MAC_EN, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->tap_action_edit_dest_mac_en);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_DEST_MAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_acl_entry->tap_action_edit_dest_mac);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_SRC_MAC_EN, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->tap_action_edit_src_mac_en);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_SRC_MAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_acl_entry->tap_action_edit_src_mac);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_IPDA_EN, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->tap_action_edit_ipda_en);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_IPDA, field_id))
    {
        cdb_addr_val2str(str, MAX_CMD_STR_LEN, &p_acl_entry->tap_action_edit_ipda);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_IPSA_EN, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->tap_action_edit_ipsa_en);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_IPSA, field_id))
    {
        cdb_addr_val2str(str, MAX_CMD_STR_LEN, &p_acl_entry->tap_action_edit_ipsa);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_VLAN_EN, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->tap_action_edit_vlan_en);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_VLAN, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->tap_action_edit_vlan);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_VLAN_ACTION, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->tap_action_edit_vlan_action);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_GROUP_OID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_acl_entry->tap_group_oid);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_ADD_VXLAN_HEADER, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->add_vxlan_header);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_VXLAN_DST_MAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_acl_entry->vxlan_dst_mac);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_VXLAN_SRC_IP, field_id))
    {
        cdb_addr_val2str(str, MAX_CMD_STR_LEN, &p_acl_entry->vxlan_src_ip);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_VXLAN_DST_IP, field_id))
    {
        cdb_addr_val2str(str, MAX_CMD_STR_LEN, &p_acl_entry->vxlan_dst_ip);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_VXLAN_SRCPORT, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->vxlan_srcport);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_VXLAN_DESTPORT, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->vxlan_destport);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_VXLAN_VNI, field_id))
    {
        sal_sprintf(str, "%u", p_acl_entry->vxlan_vni);
    }
    return str;
}

char**
tbl_acl_entry_table_dump(tbl_acl_entry_t *p_acl_entry, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_acl_entry_key_name_dump(p_acl_entry, buf));
    for(i=1; i<TBL_ACL_ENTRY_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_acl_entry_field_name_dump(p_acl_entry, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_acl_entry_field_value_dump(p_acl_entry, i, buf));
    }
    return str;
}

int32
tbl_acl_entry_field_value_parser(char *str, int32 field_id, tbl_acl_entry_t *p_acl_entry)
{
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_KEY, field_id))
    {
        int32 ret;
        p_acl_entry->key.aclid = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TBLID, field_id))
    {
        int32 ret;
        p_acl_entry->tblid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_ENTRY_PRIORITY, field_id))
    {
        int32 ret;
        p_acl_entry->entry_priority = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TIME_RANGE, field_id))
    {
        sal_strcpy(p_acl_entry->time_range, str);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_KEY_FLAGS, field_id))
    {
        int32 ret;
        p_acl_entry->key_flags = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_KEY_FLAGS2, field_id))
    {
        int32 ret;
        p_acl_entry->key_flags2 = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_KEY_IPV6_FLAGS, field_id))
    {
        int32 ret;
        p_acl_entry->key_ipv6_flags = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IS_IPV6, field_id))
    {
        int32 ret;
        p_acl_entry->is_ipv6 = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_ACL_TYPE, field_id))
    {
        int32 ret;
        p_acl_entry->acl_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_KEY_ARP_FLAGS, field_id))
    {
        int32 ret;
        p_acl_entry->key_arp_flags = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_ARP_OP_CODE, field_id))
    {
        int32 ret;
        p_acl_entry->arp_op_code = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_ARP_OP_CODE_MASK, field_id))
    {
        int32 ret;
        p_acl_entry->arp_op_code_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INVALID, field_id))
    {
        int32 ret;
        p_acl_entry->invalid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IS_COPP, field_id))
    {
        int32 ret;
        p_acl_entry->is_copp = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IN_PORT, field_id))
    {
        int32 ret;
        p_acl_entry->in_port = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_OUT_PORT, field_id))
    {
        int32 ret;
        p_acl_entry->out_port = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_ETHER_TYPE, field_id))
    {
        int32 ret;
        p_acl_entry->ether_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_ETHER_TYPE_MASK, field_id))
    {
        int32 ret;
        p_acl_entry->ether_type_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_SRC_MAC, field_id))
    {
        cdb_mac_addr_str2val(p_acl_entry->src_mac, str);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_SRC_MAC_MASK, field_id))
    {
        cdb_mac_addr_str2val(p_acl_entry->src_mac_mask, str);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_DST_MAC, field_id))
    {
        cdb_mac_addr_str2val(p_acl_entry->dst_mac, str);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_DST_MAC_MASK, field_id))
    {
        cdb_mac_addr_str2val(p_acl_entry->dst_mac_mask, str);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_SVLAN, field_id))
    {
        int32 ret;
        p_acl_entry->svlan = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_SVLAN_MASK, field_id))
    {
        int32 ret;
        p_acl_entry->svlan_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_SVLAN_COS, field_id))
    {
        int32 ret;
        p_acl_entry->svlan_cos = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_SVLAN_COS_MASK, field_id))
    {
        int32 ret;
        p_acl_entry->svlan_cos_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_CVLAN, field_id))
    {
        int32 ret;
        p_acl_entry->cvlan = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_CVLAN_MASK, field_id))
    {
        int32 ret;
        p_acl_entry->cvlan_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_CVLAN_COS, field_id))
    {
        int32 ret;
        p_acl_entry->cvlan_cos = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_CVLAN_COS_MASK, field_id))
    {
        int32 ret;
        p_acl_entry->cvlan_cos_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_UNTAG_VLAN, field_id))
    {
        int32 ret;
        p_acl_entry->untag_vlan = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IP_TYPE, field_id))
    {
        int32 ret;
        p_acl_entry->ip_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IP_TYPE_MASK, field_id))
    {
        int32 ret;
        p_acl_entry->ip_type_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IP_FLAGS, field_id))
    {
        int32 ret;
        p_acl_entry->ip_flags = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IP_FLAGS_MASK, field_id))
    {
        int32 ret;
        p_acl_entry->ip_flags_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_SRC_IP, field_id))
    {
        cdb_prefix_str2val_wo_masklen(str, &p_acl_entry->src_ip);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_SRC_IP_MASK, field_id))
    {
        cdb_prefix_str2val_wo_masklen(str, &p_acl_entry->src_ip_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_DST_IP, field_id))
    {
        cdb_prefix_str2val_wo_masklen(str, &p_acl_entry->dst_ip);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_DST_IP_MASK, field_id))
    {
        cdb_prefix_str2val_wo_masklen(str, &p_acl_entry->dst_ip_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IPV6_FLOW_LABEL, field_id))
    {
        int32 ret;
        p_acl_entry->ipv6_flow_label = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IPV6_FLOW_LABEL_MASK, field_id))
    {
        int32 ret;
        p_acl_entry->ipv6_flow_label_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_DSCP, field_id))
    {
        int32 ret;
        p_acl_entry->dscp = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_DSCP_MASK, field_id))
    {
        int32 ret;
        p_acl_entry->dscp_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IP_PRECEDENCE, field_id))
    {
        int32 ret;
        p_acl_entry->ip_precedence = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IP_PRECEDENCE_MASK, field_id))
    {
        int32 ret;
        p_acl_entry->ip_precedence_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IP_PROTOCOL, field_id))
    {
        int32 ret;
        p_acl_entry->ip_protocol = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IP_PROTOCOL_MASK, field_id))
    {
        int32 ret;
        p_acl_entry->ip_protocol_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L4_SRC_PORT_TYPE, field_id))
    {
        int32 ret;
        p_acl_entry->l4_src_port_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L4_SRC_PORT, field_id))
    {
        int32 ret;
        p_acl_entry->l4_src_port = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L4_SRC_PORT_MASK, field_id))
    {
        int32 ret;
        p_acl_entry->l4_src_port_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L4_DST_PORT_TYPE, field_id))
    {
        int32 ret;
        p_acl_entry->l4_dst_port_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L4_DST_PORT, field_id))
    {
        int32 ret;
        p_acl_entry->l4_dst_port = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L4_DST_PORT_MASK, field_id))
    {
        int32 ret;
        p_acl_entry->l4_dst_port_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IP_FRAG, field_id))
    {
        int32 ret;
        p_acl_entry->ip_frag = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TCP_FLAGS, field_id))
    {
        int32 ret;
        p_acl_entry->tcp_flags = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TCP_FLAGS_MASK, field_id))
    {
        int32 ret;
        p_acl_entry->tcp_flags_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IGMP_TYPE, field_id))
    {
        int32 ret;
        p_acl_entry->igmp_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_ICMP_TYPE, field_id))
    {
        int32 ret;
        p_acl_entry->icmp_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_ICMP_CODE, field_id))
    {
        int32 ret;
        p_acl_entry->icmp_code = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_UDF_ID, field_id))
    {
        int32 ret;
        p_acl_entry->udf_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_UDF_TYPE, field_id))
    {
        int32 ret;
        p_acl_entry->udf_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_UDF_OFFSET, field_id))
    {
        int32 ret;
        p_acl_entry->udf_offset = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_UDF_VALUE, field_id))
    {
        cdb_uint32_array_str2val(str, p_acl_entry->udf_value);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_UDF_MASK, field_id))
    {
        cdb_uint32_array_str2val(str, p_acl_entry->udf_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_UDF_VALUE_VALID, field_id))
    {
        cdb_uint32_array_str2val(str, p_acl_entry->udf_value_valid);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_ERSPAN_ID, field_id))
    {
        int32 ret;
        p_acl_entry->erspan_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_ERSPAN_ID_MASK, field_id))
    {
        int32 ret;
        p_acl_entry->erspan_id_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_STRIP_HEADER_POS, field_id))
    {
        int32 ret;
        p_acl_entry->strip_header_pos = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_STRIP_HEADER_OFFSET, field_id))
    {
        int32 ret;
        p_acl_entry->strip_header_offset = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L4_VXLAN_VNI, field_id))
    {
        int32 ret;
        p_acl_entry->l4_vxlan_vni = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L4_VXLAN_VNI_MASK, field_id))
    {
        int32 ret;
        p_acl_entry->l4_vxlan_vni_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L4_GRE_TYPE, field_id))
    {
        int32 ret;
        p_acl_entry->l4_gre_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L4_GRE_KEY, field_id))
    {
        int32 ret;
        p_acl_entry->l4_gre_key = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L4_GRE_KEY_MASK, field_id))
    {
        int32 ret;
        p_acl_entry->l4_gre_key_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L2GRE_ADD_HEADER, field_id))
    {
        int32 ret;
        p_acl_entry->l2gre_add_header = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L2GRE_SRC_IP, field_id))
    {
        cdb_addr_str2val(&p_acl_entry->l2gre_src_ip, str, 0);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L2GRE_DST_IP, field_id))
    {
        cdb_addr_str2val(&p_acl_entry->l2gre_dst_ip, str, 0);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L2GRE_DST_MAC, field_id))
    {
        cdb_mac_addr_str2val(p_acl_entry->l2gre_dst_mac, str);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L2GRE_KEY_NUM, field_id))
    {
        int32 ret;
        p_acl_entry->l2gre_key_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L2GRE_KEY_LEN, field_id))
    {
        int32 ret;
        p_acl_entry->l2gre_key_len = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L3GRE_ADD_HEADER, field_id))
    {
        int32 ret;
        p_acl_entry->l3gre_add_header = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L3GRE_SRC_IP, field_id))
    {
        cdb_addr_str2val(&p_acl_entry->l3gre_src_ip, str, 0);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L3GRE_DST_IP, field_id))
    {
        cdb_addr_str2val(&p_acl_entry->l3gre_dst_ip, str, 0);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L3GRE_DST_MAC, field_id))
    {
        cdb_mac_addr_str2val(p_acl_entry->l3gre_dst_mac, str);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_CATEGORY_ID, field_id))
    {
        int32 ret;
        p_acl_entry->category_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_DENY, field_id))
    {
        int32 ret;
        p_acl_entry->deny = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_STATS_EN, field_id))
    {
        int32 ret;
        p_acl_entry->stats_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_OPTIONS, field_id))
    {
        int32 ret;
        p_acl_entry->options = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_KEY_FLAGS, field_id))
    {
        int32 ret;
        p_acl_entry->inner_key_flags = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_KEY_IPV6_FLAGS, field_id))
    {
        int32 ret;
        p_acl_entry->inner_key_ipv6_flags = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_IS_IPV6, field_id))
    {
        int32 ret;
        p_acl_entry->inner_is_ipv6 = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_ETHER_TYPE, field_id))
    {
        int32 ret;
        p_acl_entry->inner_ether_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_ETHER_TYPE_MASK, field_id))
    {
        int32 ret;
        p_acl_entry->inner_ether_type_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_SRC_MAC, field_id))
    {
        cdb_mac_addr_str2val(p_acl_entry->inner_src_mac, str);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_SRC_MAC_MASK, field_id))
    {
        cdb_mac_addr_str2val(p_acl_entry->inner_src_mac_mask, str);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_DST_MAC, field_id))
    {
        cdb_mac_addr_str2val(p_acl_entry->inner_dst_mac, str);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_DST_MAC_MASK, field_id))
    {
        cdb_mac_addr_str2val(p_acl_entry->inner_dst_mac_mask, str);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_SVLAN, field_id))
    {
        int32 ret;
        p_acl_entry->inner_svlan = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_SVLAN_MASK, field_id))
    {
        int32 ret;
        p_acl_entry->inner_svlan_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_SVLAN_COS, field_id))
    {
        int32 ret;
        p_acl_entry->inner_svlan_cos = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_SVLAN_COS_MASK, field_id))
    {
        int32 ret;
        p_acl_entry->inner_svlan_cos_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_CVLAN, field_id))
    {
        int32 ret;
        p_acl_entry->inner_cvlan = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_CVLAN_MASK, field_id))
    {
        int32 ret;
        p_acl_entry->inner_cvlan_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_CVLAN_COS, field_id))
    {
        int32 ret;
        p_acl_entry->inner_cvlan_cos = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_CVLAN_COS_MASK, field_id))
    {
        int32 ret;
        p_acl_entry->inner_cvlan_cos_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_IP_TYPE, field_id))
    {
        int32 ret;
        p_acl_entry->inner_ip_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_IP_TYPE_MASK, field_id))
    {
        int32 ret;
        p_acl_entry->inner_ip_type_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_IP_FLAGS, field_id))
    {
        int32 ret;
        p_acl_entry->inner_ip_flags = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_IP_FLAGS_MASK, field_id))
    {
        int32 ret;
        p_acl_entry->inner_ip_flags_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_SRC_IP, field_id))
    {
        cdb_prefix_str2val_wo_masklen(str, &p_acl_entry->inner_src_ip);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_SRC_IP_MASK, field_id))
    {
        cdb_prefix_str2val_wo_masklen(str, &p_acl_entry->inner_src_ip_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_DST_IP, field_id))
    {
        cdb_prefix_str2val_wo_masklen(str, &p_acl_entry->inner_dst_ip);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_DST_IP_MASK, field_id))
    {
        cdb_prefix_str2val_wo_masklen(str, &p_acl_entry->inner_dst_ip_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_IPV6_FLOW_LABEL, field_id))
    {
        int32 ret;
        p_acl_entry->inner_ipv6_flow_label = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_IPV6_FLOW_LABEL_MASK, field_id))
    {
        int32 ret;
        p_acl_entry->inner_ipv6_flow_label_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_DSCP, field_id))
    {
        int32 ret;
        p_acl_entry->inner_dscp = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_DSCP_MASK, field_id))
    {
        int32 ret;
        p_acl_entry->inner_dscp_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_IP_PRECEDENCE, field_id))
    {
        int32 ret;
        p_acl_entry->inner_ip_precedence = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_IP_PRECEDENCE_MASK, field_id))
    {
        int32 ret;
        p_acl_entry->inner_ip_precedence_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_IP_PROTOCOL, field_id))
    {
        int32 ret;
        p_acl_entry->inner_ip_protocol = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_IP_PROTOCOL_MASK, field_id))
    {
        int32 ret;
        p_acl_entry->inner_ip_protocol_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_L4_SRC_PORT_TYPE, field_id))
    {
        int32 ret;
        p_acl_entry->inner_l4_src_port_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_L4_SRC_PORT, field_id))
    {
        int32 ret;
        p_acl_entry->inner_l4_src_port = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_L4_SRC_PORT_MASK, field_id))
    {
        int32 ret;
        p_acl_entry->inner_l4_src_port_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_L4_DST_PORT_TYPE, field_id))
    {
        int32 ret;
        p_acl_entry->inner_l4_dst_port_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_L4_DST_PORT, field_id))
    {
        int32 ret;
        p_acl_entry->inner_l4_dst_port = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_L4_DST_PORT_MASK, field_id))
    {
        int32 ret;
        p_acl_entry->inner_l4_dst_port_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_IP_FRAG, field_id))
    {
        int32 ret;
        p_acl_entry->inner_ip_frag = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_TCP_FLAGS, field_id))
    {
        int32 ret;
        p_acl_entry->inner_tcp_flags = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_TCP_FLAGS_MASK, field_id))
    {
        int32 ret;
        p_acl_entry->inner_tcp_flags_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_IGMP_TYPE, field_id))
    {
        int32 ret;
        p_acl_entry->inner_igmp_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_ICMP_TYPE, field_id))
    {
        int32 ret;
        p_acl_entry->inner_icmp_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_ICMP_CODE, field_id))
    {
        int32 ret;
        p_acl_entry->inner_icmp_code = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_OPTIONS, field_id))
    {
        int32 ret;
        p_acl_entry->inner_options = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_ACTION_STRIP_HEADER, field_id))
    {
        int32 ret;
        p_acl_entry->action_strip_header = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_REDIRECT, field_id))
    {
        int32 ret;
        p_acl_entry->tap_action_redirect = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_MARK_VLAN, field_id))
    {
        int32 ret;
        p_acl_entry->tap_action_mark_vlan = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_UNTAG, field_id))
    {
        int32 ret;
        p_acl_entry->tap_action_untag = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_TRUNCTION, field_id))
    {
        int32 ret;
        p_acl_entry->tap_action_trunction = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_DEST_MAC_EN, field_id))
    {
        int32 ret;
        p_acl_entry->tap_action_edit_dest_mac_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_DEST_MAC, field_id))
    {
        cdb_mac_addr_str2val(p_acl_entry->tap_action_edit_dest_mac, str);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_SRC_MAC_EN, field_id))
    {
        int32 ret;
        p_acl_entry->tap_action_edit_src_mac_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_SRC_MAC, field_id))
    {
        cdb_mac_addr_str2val(p_acl_entry->tap_action_edit_src_mac, str);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_IPDA_EN, field_id))
    {
        int32 ret;
        p_acl_entry->tap_action_edit_ipda_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_IPDA, field_id))
    {
        cdb_addr_str2val(&p_acl_entry->tap_action_edit_ipda, str, 0);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_IPSA_EN, field_id))
    {
        int32 ret;
        p_acl_entry->tap_action_edit_ipsa_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_IPSA, field_id))
    {
        cdb_addr_str2val(&p_acl_entry->tap_action_edit_ipsa, str, 0);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_VLAN_EN, field_id))
    {
        int32 ret;
        p_acl_entry->tap_action_edit_vlan_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_VLAN, field_id))
    {
        int32 ret;
        p_acl_entry->tap_action_edit_vlan = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_VLAN_ACTION, field_id))
    {
        int32 ret;
        p_acl_entry->tap_action_edit_vlan_action = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_GROUP_OID, field_id))
    {
        int32 ret;
        p_acl_entry->tap_group_oid = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_ADD_VXLAN_HEADER, field_id))
    {
        int32 ret;
        p_acl_entry->add_vxlan_header = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_VXLAN_DST_MAC, field_id))
    {
        cdb_mac_addr_str2val(p_acl_entry->vxlan_dst_mac, str);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_VXLAN_SRC_IP, field_id))
    {
        cdb_addr_str2val(&p_acl_entry->vxlan_src_ip, str, 0);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_VXLAN_DST_IP, field_id))
    {
        cdb_addr_str2val(&p_acl_entry->vxlan_dst_ip, str, 0);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_VXLAN_SRCPORT, field_id))
    {
        int32 ret;
        p_acl_entry->vxlan_srcport = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_VXLAN_DESTPORT, field_id))
    {
        int32 ret;
        p_acl_entry->vxlan_destport = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_VXLAN_VNI, field_id))
    {
        int32 ret;
        p_acl_entry->vxlan_vni = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_acl_entry_table_parser(char** array, char* key_value,tbl_acl_entry_t *p_acl_entry)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_ENTRY);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_acl_entry_key_str2val(key_value, p_acl_entry));

    for(i=1; i<TBL_ACL_ENTRY_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_acl_entry_field_value_parser( array[j++], i, p_acl_entry));
    }

    return PM_E_NONE;
}

int32
tbl_acl_entry_dump_one(tbl_acl_entry_t *p_acl_entry, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_ENTRY);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ACL_ENTRY_FLD_KEY].name,
            p_acl_entry->key.aclid);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TBLID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_TBLID].name,
            p_acl_entry->tblid);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_ENTRY_PRIORITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_ENTRY_PRIORITY].name,
            p_acl_entry->entry_priority);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TIME_RANGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_ENTRY_FLD_TIME_RANGE].name,
            p_acl_entry->time_range);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_KEY_FLAGS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_KEY_FLAGS].name,
            p_acl_entry->key_flags);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_KEY_FLAGS2, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_KEY_FLAGS2].name,
            p_acl_entry->key_flags2);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_KEY_IPV6_FLAGS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_KEY_IPV6_FLAGS].name,
            p_acl_entry->key_ipv6_flags);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IS_IPV6, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_IS_IPV6].name,
            p_acl_entry->is_ipv6);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_ACL_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_ACL_TYPE].name,
            p_acl_entry->acl_type);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_KEY_ARP_FLAGS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_KEY_ARP_FLAGS].name,
            p_acl_entry->key_arp_flags);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_ARP_OP_CODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_ARP_OP_CODE].name,
            p_acl_entry->arp_op_code);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_ARP_OP_CODE_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_ARP_OP_CODE_MASK].name,
            p_acl_entry->arp_op_code_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INVALID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INVALID].name,
            p_acl_entry->invalid);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IS_COPP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_IS_COPP].name,
            p_acl_entry->is_copp);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IN_PORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ACL_ENTRY_FLD_IN_PORT].name,
            p_acl_entry->in_port);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_OUT_PORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ACL_ENTRY_FLD_OUT_PORT].name,
            p_acl_entry->out_port);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_ETHER_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_ETHER_TYPE].name,
            p_acl_entry->ether_type);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_ETHER_TYPE_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_ETHER_TYPE_MASK].name,
            p_acl_entry->ether_type_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_SRC_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_ENTRY_FLD_SRC_MAC].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_acl_entry->src_mac));
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_SRC_MAC_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_ENTRY_FLD_SRC_MAC_MASK].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_acl_entry->src_mac_mask));
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_DST_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_ENTRY_FLD_DST_MAC].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_acl_entry->dst_mac));
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_DST_MAC_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_ENTRY_FLD_DST_MAC_MASK].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_acl_entry->dst_mac_mask));
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_SVLAN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_SVLAN].name,
            p_acl_entry->svlan);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_SVLAN_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_SVLAN_MASK].name,
            p_acl_entry->svlan_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_SVLAN_COS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_SVLAN_COS].name,
            p_acl_entry->svlan_cos);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_SVLAN_COS_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_SVLAN_COS_MASK].name,
            p_acl_entry->svlan_cos_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_CVLAN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_CVLAN].name,
            p_acl_entry->cvlan);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_CVLAN_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_CVLAN_MASK].name,
            p_acl_entry->cvlan_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_CVLAN_COS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_CVLAN_COS].name,
            p_acl_entry->cvlan_cos);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_CVLAN_COS_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_CVLAN_COS_MASK].name,
            p_acl_entry->cvlan_cos_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_UNTAG_VLAN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_UNTAG_VLAN].name,
            p_acl_entry->untag_vlan);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IP_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_IP_TYPE].name,
            p_acl_entry->ip_type);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IP_TYPE_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_IP_TYPE_MASK].name,
            p_acl_entry->ip_type_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IP_FLAGS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_IP_FLAGS].name,
            p_acl_entry->ip_flags);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IP_FLAGS_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_IP_FLAGS_MASK].name,
            p_acl_entry->ip_flags_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_SRC_IP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_ENTRY_FLD_SRC_IP].name, 
            cdb_prefix_val2str(buf, MAX_CMD_STR_LEN, &p_acl_entry->src_ip));
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_SRC_IP_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_ENTRY_FLD_SRC_IP_MASK].name, 
            cdb_prefix_val2str(buf, MAX_CMD_STR_LEN, &p_acl_entry->src_ip_mask));
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_DST_IP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_ENTRY_FLD_DST_IP].name, 
            cdb_prefix_val2str(buf, MAX_CMD_STR_LEN, &p_acl_entry->dst_ip));
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_DST_IP_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_ENTRY_FLD_DST_IP_MASK].name, 
            cdb_prefix_val2str(buf, MAX_CMD_STR_LEN, &p_acl_entry->dst_ip_mask));
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IPV6_FLOW_LABEL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_IPV6_FLOW_LABEL].name,
            p_acl_entry->ipv6_flow_label);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IPV6_FLOW_LABEL_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_IPV6_FLOW_LABEL_MASK].name,
            p_acl_entry->ipv6_flow_label_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_DSCP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_DSCP].name,
            p_acl_entry->dscp);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_DSCP_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_DSCP_MASK].name,
            p_acl_entry->dscp_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IP_PRECEDENCE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_IP_PRECEDENCE].name,
            p_acl_entry->ip_precedence);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IP_PRECEDENCE_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_IP_PRECEDENCE_MASK].name,
            p_acl_entry->ip_precedence_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IP_PROTOCOL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_IP_PROTOCOL].name,
            p_acl_entry->ip_protocol);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IP_PROTOCOL_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_IP_PROTOCOL_MASK].name,
            p_acl_entry->ip_protocol_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L4_SRC_PORT_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_L4_SRC_PORT_TYPE].name,
            p_acl_entry->l4_src_port_type);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L4_SRC_PORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_L4_SRC_PORT].name,
            p_acl_entry->l4_src_port);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L4_SRC_PORT_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_L4_SRC_PORT_MASK].name,
            p_acl_entry->l4_src_port_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L4_DST_PORT_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_L4_DST_PORT_TYPE].name,
            p_acl_entry->l4_dst_port_type);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L4_DST_PORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_L4_DST_PORT].name,
            p_acl_entry->l4_dst_port);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L4_DST_PORT_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_L4_DST_PORT_MASK].name,
            p_acl_entry->l4_dst_port_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IP_FRAG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_IP_FRAG].name,
            p_acl_entry->ip_frag);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TCP_FLAGS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_TCP_FLAGS].name,
            p_acl_entry->tcp_flags);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TCP_FLAGS_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_TCP_FLAGS_MASK].name,
            p_acl_entry->tcp_flags_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_IGMP_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_IGMP_TYPE].name,
            p_acl_entry->igmp_type);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_ICMP_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_ICMP_TYPE].name,
            p_acl_entry->icmp_type);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_ICMP_CODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_ICMP_CODE].name,
            p_acl_entry->icmp_code);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_UDF_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_UDF_ID].name,
            p_acl_entry->udf_id);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_UDF_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_UDF_TYPE].name,
            p_acl_entry->udf_type);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_UDF_OFFSET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_UDF_OFFSET].name,
            p_acl_entry->udf_offset);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_UDF_VALUE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_ENTRY_FLD_UDF_VALUE].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_acl_entry->udf_value, sizeof(p_acl_entry->udf_value)/4));
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_UDF_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_ENTRY_FLD_UDF_MASK].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_acl_entry->udf_mask, sizeof(p_acl_entry->udf_mask)/4));
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_UDF_VALUE_VALID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_ENTRY_FLD_UDF_VALUE_VALID].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_acl_entry->udf_value_valid, sizeof(p_acl_entry->udf_value_valid)/4));
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_ERSPAN_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_ERSPAN_ID].name,
            p_acl_entry->erspan_id);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_ERSPAN_ID_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_ERSPAN_ID_MASK].name,
            p_acl_entry->erspan_id_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_STRIP_HEADER_POS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_STRIP_HEADER_POS].name,
            p_acl_entry->strip_header_pos);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_STRIP_HEADER_OFFSET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_STRIP_HEADER_OFFSET].name,
            p_acl_entry->strip_header_offset);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L4_VXLAN_VNI, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_L4_VXLAN_VNI].name,
            p_acl_entry->l4_vxlan_vni);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L4_VXLAN_VNI_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_L4_VXLAN_VNI_MASK].name,
            p_acl_entry->l4_vxlan_vni_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L4_GRE_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_L4_GRE_TYPE].name,
            p_acl_entry->l4_gre_type);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L4_GRE_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_L4_GRE_KEY].name,
            p_acl_entry->l4_gre_key);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L4_GRE_KEY_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_L4_GRE_KEY_MASK].name,
            p_acl_entry->l4_gre_key_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L2GRE_ADD_HEADER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_L2GRE_ADD_HEADER].name,
            p_acl_entry->l2gre_add_header);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L2GRE_SRC_IP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_ENTRY_FLD_L2GRE_SRC_IP].name, 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_acl_entry->l2gre_src_ip));
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L2GRE_DST_IP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_ENTRY_FLD_L2GRE_DST_IP].name, 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_acl_entry->l2gre_dst_ip));
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L2GRE_DST_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_ENTRY_FLD_L2GRE_DST_MAC].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_acl_entry->l2gre_dst_mac));
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L2GRE_KEY_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_L2GRE_KEY_NUM].name,
            p_acl_entry->l2gre_key_num);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L2GRE_KEY_LEN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_L2GRE_KEY_LEN].name,
            p_acl_entry->l2gre_key_len);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L3GRE_ADD_HEADER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_L3GRE_ADD_HEADER].name,
            p_acl_entry->l3gre_add_header);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L3GRE_SRC_IP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_ENTRY_FLD_L3GRE_SRC_IP].name, 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_acl_entry->l3gre_src_ip));
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L3GRE_DST_IP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_ENTRY_FLD_L3GRE_DST_IP].name, 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_acl_entry->l3gre_dst_ip));
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_L3GRE_DST_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_ENTRY_FLD_L3GRE_DST_MAC].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_acl_entry->l3gre_dst_mac));
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_CATEGORY_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_CATEGORY_ID].name,
            p_acl_entry->category_id);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_DENY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_DENY].name,
            p_acl_entry->deny);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_STATS_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_STATS_EN].name,
            p_acl_entry->stats_en);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_OPTIONS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_OPTIONS].name,
            p_acl_entry->options);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_KEY_FLAGS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_KEY_FLAGS].name,
            p_acl_entry->inner_key_flags);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_KEY_IPV6_FLAGS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_KEY_IPV6_FLAGS].name,
            p_acl_entry->inner_key_ipv6_flags);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_IS_IPV6, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_IS_IPV6].name,
            p_acl_entry->inner_is_ipv6);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_ETHER_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_ETHER_TYPE].name,
            p_acl_entry->inner_ether_type);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_ETHER_TYPE_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_ETHER_TYPE_MASK].name,
            p_acl_entry->inner_ether_type_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_SRC_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_SRC_MAC].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_acl_entry->inner_src_mac));
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_SRC_MAC_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_SRC_MAC_MASK].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_acl_entry->inner_src_mac_mask));
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_DST_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_DST_MAC].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_acl_entry->inner_dst_mac));
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_DST_MAC_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_DST_MAC_MASK].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_acl_entry->inner_dst_mac_mask));
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_SVLAN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_SVLAN].name,
            p_acl_entry->inner_svlan);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_SVLAN_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_SVLAN_MASK].name,
            p_acl_entry->inner_svlan_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_SVLAN_COS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_SVLAN_COS].name,
            p_acl_entry->inner_svlan_cos);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_SVLAN_COS_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_SVLAN_COS_MASK].name,
            p_acl_entry->inner_svlan_cos_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_CVLAN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_CVLAN].name,
            p_acl_entry->inner_cvlan);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_CVLAN_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_CVLAN_MASK].name,
            p_acl_entry->inner_cvlan_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_CVLAN_COS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_CVLAN_COS].name,
            p_acl_entry->inner_cvlan_cos);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_CVLAN_COS_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_CVLAN_COS_MASK].name,
            p_acl_entry->inner_cvlan_cos_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_IP_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_IP_TYPE].name,
            p_acl_entry->inner_ip_type);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_IP_TYPE_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_IP_TYPE_MASK].name,
            p_acl_entry->inner_ip_type_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_IP_FLAGS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_IP_FLAGS].name,
            p_acl_entry->inner_ip_flags);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_IP_FLAGS_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_IP_FLAGS_MASK].name,
            p_acl_entry->inner_ip_flags_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_SRC_IP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_SRC_IP].name, 
            cdb_prefix_val2str(buf, MAX_CMD_STR_LEN, &p_acl_entry->inner_src_ip));
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_SRC_IP_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_SRC_IP_MASK].name, 
            cdb_prefix_val2str(buf, MAX_CMD_STR_LEN, &p_acl_entry->inner_src_ip_mask));
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_DST_IP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_DST_IP].name, 
            cdb_prefix_val2str(buf, MAX_CMD_STR_LEN, &p_acl_entry->inner_dst_ip));
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_DST_IP_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_DST_IP_MASK].name, 
            cdb_prefix_val2str(buf, MAX_CMD_STR_LEN, &p_acl_entry->inner_dst_ip_mask));
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_IPV6_FLOW_LABEL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_IPV6_FLOW_LABEL].name,
            p_acl_entry->inner_ipv6_flow_label);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_IPV6_FLOW_LABEL_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_IPV6_FLOW_LABEL_MASK].name,
            p_acl_entry->inner_ipv6_flow_label_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_DSCP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_DSCP].name,
            p_acl_entry->inner_dscp);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_DSCP_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_DSCP_MASK].name,
            p_acl_entry->inner_dscp_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_IP_PRECEDENCE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_IP_PRECEDENCE].name,
            p_acl_entry->inner_ip_precedence);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_IP_PRECEDENCE_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_IP_PRECEDENCE_MASK].name,
            p_acl_entry->inner_ip_precedence_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_IP_PROTOCOL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_IP_PROTOCOL].name,
            p_acl_entry->inner_ip_protocol);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_IP_PROTOCOL_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_IP_PROTOCOL_MASK].name,
            p_acl_entry->inner_ip_protocol_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_L4_SRC_PORT_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_L4_SRC_PORT_TYPE].name,
            p_acl_entry->inner_l4_src_port_type);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_L4_SRC_PORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_L4_SRC_PORT].name,
            p_acl_entry->inner_l4_src_port);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_L4_SRC_PORT_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_L4_SRC_PORT_MASK].name,
            p_acl_entry->inner_l4_src_port_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_L4_DST_PORT_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_L4_DST_PORT_TYPE].name,
            p_acl_entry->inner_l4_dst_port_type);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_L4_DST_PORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_L4_DST_PORT].name,
            p_acl_entry->inner_l4_dst_port);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_L4_DST_PORT_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_L4_DST_PORT_MASK].name,
            p_acl_entry->inner_l4_dst_port_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_IP_FRAG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_IP_FRAG].name,
            p_acl_entry->inner_ip_frag);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_TCP_FLAGS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_TCP_FLAGS].name,
            p_acl_entry->inner_tcp_flags);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_TCP_FLAGS_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_TCP_FLAGS_MASK].name,
            p_acl_entry->inner_tcp_flags_mask);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_IGMP_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_IGMP_TYPE].name,
            p_acl_entry->inner_igmp_type);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_ICMP_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_ICMP_TYPE].name,
            p_acl_entry->inner_icmp_type);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_ICMP_CODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_ICMP_CODE].name,
            p_acl_entry->inner_icmp_code);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_INNER_OPTIONS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_INNER_OPTIONS].name,
            p_acl_entry->inner_options);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_ACTION_STRIP_HEADER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_ACTION_STRIP_HEADER].name,
            p_acl_entry->action_strip_header);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_REDIRECT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_TAP_ACTION_REDIRECT].name,
            p_acl_entry->tap_action_redirect);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_MARK_VLAN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_TAP_ACTION_MARK_VLAN].name,
            p_acl_entry->tap_action_mark_vlan);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_UNTAG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_TAP_ACTION_UNTAG].name,
            p_acl_entry->tap_action_untag);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_TRUNCTION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_TAP_ACTION_TRUNCTION].name,
            p_acl_entry->tap_action_trunction);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_DEST_MAC_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_DEST_MAC_EN].name,
            p_acl_entry->tap_action_edit_dest_mac_en);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_DEST_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_DEST_MAC].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_acl_entry->tap_action_edit_dest_mac));
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_SRC_MAC_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_SRC_MAC_EN].name,
            p_acl_entry->tap_action_edit_src_mac_en);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_SRC_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_SRC_MAC].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_acl_entry->tap_action_edit_src_mac));
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_IPDA_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_IPDA_EN].name,
            p_acl_entry->tap_action_edit_ipda_en);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_IPDA, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_IPDA].name, 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_acl_entry->tap_action_edit_ipda));
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_IPSA_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_IPSA_EN].name,
            p_acl_entry->tap_action_edit_ipsa_en);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_IPSA, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_IPSA].name, 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_acl_entry->tap_action_edit_ipsa));
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_VLAN_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_VLAN_EN].name,
            p_acl_entry->tap_action_edit_vlan_en);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_VLAN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_VLAN].name,
            p_acl_entry->tap_action_edit_vlan);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_VLAN_ACTION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_TAP_ACTION_EDIT_VLAN_ACTION].name,
            p_acl_entry->tap_action_edit_vlan_action);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_TAP_GROUP_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ACL_ENTRY_FLD_TAP_GROUP_OID].name,
            p_acl_entry->tap_group_oid);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_ADD_VXLAN_HEADER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_ADD_VXLAN_HEADER].name,
            p_acl_entry->add_vxlan_header);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_VXLAN_DST_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_ENTRY_FLD_VXLAN_DST_MAC].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_acl_entry->vxlan_dst_mac));
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_VXLAN_SRC_IP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_ENTRY_FLD_VXLAN_SRC_IP].name, 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_acl_entry->vxlan_src_ip));
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_VXLAN_DST_IP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_ENTRY_FLD_VXLAN_DST_IP].name, 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_acl_entry->vxlan_dst_ip));
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_VXLAN_SRCPORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_VXLAN_SRCPORT].name,
            p_acl_entry->vxlan_srcport);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_VXLAN_DESTPORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_VXLAN_DESTPORT].name,
            p_acl_entry->vxlan_destport);
    }
    if (FLD_MATCH(TBL_ACL_ENTRY_FLD_VXLAN_VNI, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_ENTRY_FLD_VXLAN_VNI].name,
            p_acl_entry->vxlan_vni);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_acl_entry_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_acl_entry_master_t *p_master = _g_p_tbl_acl_entry_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_acl_entry_t *p_db_acl_entry = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->acl_entry_list, p_db_acl_entry, listnode, next)
    {
        rc |= fn(p_db_acl_entry, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_acl_entry_master_t*
tbl_acl_entry_get_master()
{
    return _g_p_tbl_acl_entry_master;
}

tbl_acl_entry_master_t*
tbl_acl_entry_init_acl_entry()
{
    _g_p_tbl_acl_entry_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_acl_entry_master_t));
    _g_p_tbl_acl_entry_master->acl_entry_hash = ctclib_hash_create(_tbl_acl_entry_hash_make, _tbl_acl_entry_hash_cmp);
    _g_p_tbl_acl_entry_master->acl_entry_list = ctclib_slist_create(tbl_acl_entry_cmp, NULL);
    return _g_p_tbl_acl_entry_master;
}

int32
tbl_acl_entry_get_acl_entry_count()
{
    tbl_acl_entry_master_t *p_master = _g_p_tbl_acl_entry_master;

    return p_master->acl_entry_hash->count;
}


int32
tbl_acl_entry_get_count(tbl_acl_entry_t *p_acl_entry, tbl_iter_args_t *pargs)
{
    uint32 *num_ingress = pargs->argv[0];
    uint32 *num_egress = pargs->argv[1];
    uint32 *num_global_ingress = pargs->argv[2];

    if (p_acl_entry->in_port)
    {
        if (p_acl_entry->is_ipv6 || p_acl_entry->inner_is_ipv6)
        {
            *num_ingress += 2;
        }
        else
        {
            *num_ingress += 1;
        }     
    }
    if (p_acl_entry->out_port)
    {
        if (p_acl_entry->is_ipv6 || p_acl_entry->inner_is_ipv6)
        {
            *num_egress += 2;
        }
        else
        {
            *num_egress += 1;
        }
    }
    if (0 == p_acl_entry->in_port && 0 == p_acl_entry->out_port)
    {
        if (p_acl_entry->is_ipv6 || p_acl_entry->inner_is_ipv6)
        {
            *num_global_ingress += 2;
        }
        else
        {
            *num_global_ingress += 1;
        }
    }
    return PM_E_NONE;
}

int32
tbl_acl_entry_get_entry_dir_count(uint32* cnt_ingress, uint32* cnt_egress, uint32* cnt_worm_filter)
{
    tbl_iter_args_t args;
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = cnt_ingress;
    args.argv[1] = cnt_egress;
    args.argv[2] = cnt_worm_filter;
    tbl_acl_entry_iterate((TBL_ITER_CB_FUNC)tbl_acl_entry_get_count, &args);
    return PM_E_NONE;
}

int32
tbl_acl_entry_get_count_1(tbl_acl_entry_t *p_acl_entry, tbl_iter_args_t *pargs)
{
    acl_entry_count_t *entry_count = pargs->argv[0];

    if (p_acl_entry->in_port)
    {
        if (p_acl_entry->is_ipv6 || p_acl_entry->inner_is_ipv6)
        {
            entry_count->cur_ingress6 += 1;
        }
        else
        {
            entry_count->cur_ingress += 1;
        }     
    }
    if (p_acl_entry->out_port)
    {
        if (p_acl_entry->is_ipv6 || p_acl_entry->inner_is_ipv6)
        {
            entry_count->cur_egress6 += 1;
        }
        else
        {
            entry_count->cur_egress += 1;
        }
    }

	if (0 == p_acl_entry->in_port && 0 == p_acl_entry->out_port)
    {
        if (p_acl_entry->is_ipv6 || p_acl_entry->inner_is_ipv6)
        {
            entry_count->cur_worm_filter6 += 1;
        }
        else
        {
            entry_count->cur_worm_filter += 1;
        }
    }

    return PM_E_NONE;
}


int32
tbl_acl_entry_get_entry_dir_count_1(acl_entry_count_t *entry_count)
{
    tbl_iter_args_t args;
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = entry_count;
    tbl_acl_entry_iterate((TBL_ITER_CB_FUNC)tbl_acl_entry_get_count_1, &args);
    return PM_E_NONE;
}


