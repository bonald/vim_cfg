
#include "proto.h"
#include "pb_cdb_inc.h"
#include "tbl.pb-c.h"
#include "ds.pb-c.h"
#include "pb_tbl.h"
#include "pb_ds.h"
#include "pb_common.h"

/* pb functions for TBL_INTERFACE DS_BRGIF */
int32
pb_ds_brgif_to_pb(tbl_interface_key_t *p_if_key, ds_brgif_t *p_brgif, Cdb__DsBrgif *p_pb_brgif)
{
    p_pb_brgif->parent_if->name = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_if_key->name)+1);
    sal_strcpy(p_pb_brgif->parent_if->name, p_if_key->name);

    p_pb_brgif->pvid = p_brgif->pvid;
    p_pb_brgif->inner_native_vid = p_brgif->inner_native_vid;
    p_pb_brgif->qinq_type = p_brgif->qinq_type;
    p_pb_brgif->qinq_default_fwd = p_brgif->qinq_default_fwd;
    p_pb_brgif->vlan_translation_en = p_brgif->vlan_translation_en;
    p_pb_brgif->vlan_trans_default_fwd = p_brgif->vlan_trans_default_fwd;
    p_pb_brgif->vlan_mapping_table = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_brgif->vlan_mapping_table)+1);
    sal_strcpy(p_pb_brgif->vlan_mapping_table, p_brgif->vlan_mapping_table);
    p_pb_brgif->dot1q_tag_native_en = p_brgif->dot1q_tag_native_en;
    pb_compose_brg_allowed_vlan_t_to_pb(&p_brgif->allowed_vlan, p_pb_brgif->allowed_vlan);
    p_pb_brgif->ingress_filter = p_brgif->ingress_filter;
    p_pb_brgif->egress_filter = p_brgif->egress_filter;
    p_pb_brgif->tpid_type = p_brgif->tpid_type;
    p_pb_brgif->mac_learning_en = p_brgif->mac_learning_en;
    p_pb_brgif->oper_mac_learning_en = p_brgif->oper_mac_learning_en;
    p_pb_brgif->port_security_en = p_brgif->port_security_en;
    p_pb_brgif->violate_mode = p_brgif->violate_mode;
    p_pb_brgif->mlag_is_peer_link = p_brgif->mlag_is_peer_link;
    p_pb_brgif->mlag_is_port_block = p_brgif->mlag_is_port_block;
    p_pb_brgif->mlag_is_group = p_brgif->mlag_is_group;
    p_pb_brgif->max_mac = p_brgif->max_mac;
    pb_compose_brg_allowed_vlan_t_to_pb(&p_brgif->untagged_vlan, p_pb_brgif->untagged_vlan);

    return PM_E_NONE;
}

int32
pb_ds_brgif_to_pb_free_packed(Cdb__DsBrgif *p_pb_brgif)
{
    if (p_pb_brgif->parent_if->name)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_brgif->parent_if->name);
        p_pb_brgif->parent_if->name = NULL;
    }

    if (p_pb_brgif->vlan_mapping_table)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_brgif->vlan_mapping_table);
        p_pb_brgif->vlan_mapping_table = NULL;
    }

    pb_compose_brg_allowed_vlan_t_to_pb_free_packed(p_pb_brgif->allowed_vlan);
    pb_compose_brg_allowed_vlan_t_to_pb_free_packed(p_pb_brgif->untagged_vlan);
    return PM_E_NONE;
}

int32
pb_ds_brgif_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len)
{
    Cdb__TblInterfaceKey pb_if_key = CDB__TBL_INTERFACE_KEY__INIT;
    Cdb__DsBrgif pb_brgif = CDB__DS_BRGIF__INIT;
    tbl_interface_key_t *p_if_key = (tbl_interface_key_t*)p_tbl;
    ds_brgif_t *p_brgif = (ds_brgif_t*)p_ds;
    int32 len = 0;
    Cdb__ComposeBrgAllowedVlanT allowed_vlan = CDB__COMPOSE_BRG_ALLOWED_VLAN_T__INIT;
    Cdb__ComposeBrgAllowedVlanT untagged_vlan = CDB__COMPOSE_BRG_ALLOWED_VLAN_T__INIT;

    pb_brgif.allowed_vlan = &allowed_vlan;
    pb_brgif.untagged_vlan = &untagged_vlan;
    pb_brgif.parent_if = &pb_if_key;
    pb_ds_brgif_to_pb(p_if_key, p_brgif, &pb_brgif);
    len = cdb__ds_brgif__pack(&pb_brgif, buf);
    pb_ds_brgif_to_pb_free_packed(&pb_brgif);

    return len;
}

int32
pb_ds_brgif_dump(Cdb__DsBrgif *p_pb_brgif, char *out)
{
    int32 offset = 0;

    out[0] = CMD_ZERO_CHAR;
    offset += sal_sprintf(out + offset, "/parent_if->name=%s", p_pb_brgif->parent_if->name);
    offset += sal_sprintf(out + offset, "/pvid=%u", p_pb_brgif->pvid);
    offset += sal_sprintf(out + offset, "/inner_native_vid=%u", p_pb_brgif->inner_native_vid);
    offset += sal_sprintf(out + offset, "/qinq_type=%u", p_pb_brgif->qinq_type);
    offset += sal_sprintf(out + offset, "/qinq_default_fwd=%u", p_pb_brgif->qinq_default_fwd);
    offset += sal_sprintf(out + offset, "/vlan_translation_en=%u", p_pb_brgif->vlan_translation_en);
    offset += sal_sprintf(out + offset, "/vlan_trans_default_fwd=%u", p_pb_brgif->vlan_trans_default_fwd);
    offset += sal_sprintf(out + offset, "/vlan_mapping_table=%s", p_pb_brgif->vlan_mapping_table);
    offset += sal_sprintf(out + offset, "/dot1q_tag_native_en=%u", p_pb_brgif->dot1q_tag_native_en);
    offset += pb_compose_brg_allowed_vlan_t_dump(p_pb_brgif->allowed_vlan, (out + offset));
    offset += sal_sprintf(out + offset, "/ingress_filter=%u", p_pb_brgif->ingress_filter);
    offset += sal_sprintf(out + offset, "/egress_filter=%u", p_pb_brgif->egress_filter);
    offset += sal_sprintf(out + offset, "/tpid_type=%u", p_pb_brgif->tpid_type);
    offset += sal_sprintf(out + offset, "/mac_learning_en=%u", p_pb_brgif->mac_learning_en);
    offset += sal_sprintf(out + offset, "/oper_mac_learning_en=%u", p_pb_brgif->oper_mac_learning_en);
    offset += sal_sprintf(out + offset, "/port_security_en=%u", p_pb_brgif->port_security_en);
    offset += sal_sprintf(out + offset, "/violate_mode=%u", p_pb_brgif->violate_mode);
    offset += sal_sprintf(out + offset, "/mlag_is_peer_link=%u", p_pb_brgif->mlag_is_peer_link);
    offset += sal_sprintf(out + offset, "/mlag_is_port_block=%u", p_pb_brgif->mlag_is_port_block);
    offset += sal_sprintf(out + offset, "/mlag_is_group=%u", p_pb_brgif->mlag_is_group);
    offset += sal_sprintf(out + offset, "/max_mac=%u", p_pb_brgif->max_mac);
    offset += pb_compose_brg_allowed_vlan_t_dump(p_pb_brgif->untagged_vlan, (out + offset));
    offset += sal_sprintf(out + offset, "\n");

    return PM_E_NONE;
}

/* pb functions for TBL_INTERFACE DS_ACLQOS_IF */
int32
pb_ds_aclqos_if_to_pb(tbl_interface_key_t *p_if_key, ds_aclqos_if_t *p_aclqos_if, Cdb__DsAclqosIf *p_pb_aclqos_if)
{
    uint32 i = 0;

    p_pb_aclqos_if->parent_if->name = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_if_key->name)+1);
    sal_strcpy(p_pb_aclqos_if->parent_if->name, p_if_key->name);

    p_pb_aclqos_if->default_cos = p_aclqos_if->default_cos;
    p_pb_aclqos_if->domain = p_aclqos_if->domain;
    p_pb_aclqos_if->trust = p_aclqos_if->trust;
    p_pb_aclqos_if->replace = p_aclqos_if->replace;
    p_pb_aclqos_if->shape_pir = p_aclqos_if->shape_pir;
    p_pb_aclqos_if->queue_stats_enable = p_aclqos_if->queue_stats_enable;
    p_pb_aclqos_if->input_policy_map = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_aclqos_if->input_policy_map)+1);
    sal_strcpy(p_pb_aclqos_if->input_policy_map, p_aclqos_if->input_policy_map);
    p_pb_aclqos_if->output_policy_map = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_aclqos_if->output_policy_map)+1);
    sal_strcpy(p_pb_aclqos_if->output_policy_map, p_aclqos_if->output_policy_map);
    p_pb_aclqos_if->input_policer = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_aclqos_if->input_policer)+1);
    sal_strcpy(p_pb_aclqos_if->input_policer, p_aclqos_if->input_policer);
    p_pb_aclqos_if->output_policer = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_aclqos_if->output_policer)+1);
    sal_strcpy(p_pb_aclqos_if->output_policer, p_aclqos_if->output_policer);
    p_pb_aclqos_if->queue_shape_profile = XCALLOC(MEM_LIB_PROTOBUF, sizeof(char*)*GLB_QOS_PORT_QUEUE_NUM);
    p_pb_aclqos_if->n_queue_shape_profile = GLB_QOS_PORT_QUEUE_NUM;
    for (i = 0; i < GLB_QOS_PORT_QUEUE_NUM; i++)
    {
        p_pb_aclqos_if->queue_shape_profile[i] = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_aclqos_if->queue_shape_profile[i])+1);
        sal_strcpy(p_pb_aclqos_if->queue_shape_profile[i], p_aclqos_if->queue_shape_profile[i]);
    }
    p_pb_aclqos_if->queue_drop_profile = XCALLOC(MEM_LIB_PROTOBUF, sizeof(char*)*GLB_QOS_PORT_QUEUE_NUM);
    p_pb_aclqos_if->n_queue_drop_profile = GLB_QOS_PORT_QUEUE_NUM;
    for (i = 0; i < GLB_QOS_PORT_QUEUE_NUM; i++)
    {
        p_pb_aclqos_if->queue_drop_profile[i] = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_aclqos_if->queue_drop_profile[i])+1);
        sal_strcpy(p_pb_aclqos_if->queue_drop_profile[i], p_aclqos_if->queue_drop_profile[i]);
    }
    p_pb_aclqos_if->queue_class = XCALLOC(MEM_LIB_PROTOBUF, sizeof(uint32_t)*GLB_QOS_PORT_QUEUE_NUM);
    p_pb_aclqos_if->n_queue_class = GLB_QOS_PORT_QUEUE_NUM;
    for (i = 0; i < GLB_QOS_PORT_QUEUE_NUM; i++)
    {
        p_pb_aclqos_if->queue_class[i] = p_aclqos_if->queue_class[i];
    }
    p_pb_aclqos_if->queue_weight = XCALLOC(MEM_LIB_PROTOBUF, sizeof(uint32_t)*GLB_QOS_PORT_QUEUE_NUM);
    p_pb_aclqos_if->n_queue_weight = GLB_QOS_PORT_QUEUE_NUM;
    for (i = 0; i < GLB_QOS_PORT_QUEUE_NUM; i++)
    {
        p_pb_aclqos_if->queue_weight[i] = p_aclqos_if->queue_weight[i];
    }
    p_pb_aclqos_if->queue_drop_mode = XCALLOC(MEM_LIB_PROTOBUF, sizeof(uint32_t)*GLB_QOS_PORT_QUEUE_NUM);
    p_pb_aclqos_if->n_queue_drop_mode = GLB_QOS_PORT_QUEUE_NUM;
    for (i = 0; i < GLB_QOS_PORT_QUEUE_NUM; i++)
    {
        p_pb_aclqos_if->queue_drop_mode[i] = p_aclqos_if->queue_drop_mode[i];
    }
    p_pb_aclqos_if->queue_ecn_enable = XCALLOC(MEM_LIB_PROTOBUF, sizeof(uint32_t)*GLB_QOS_PORT_QUEUE_NUM);
    p_pb_aclqos_if->n_queue_ecn_enable = GLB_QOS_PORT_QUEUE_NUM;
    for (i = 0; i < GLB_QOS_PORT_QUEUE_NUM; i++)
    {
        p_pb_aclqos_if->queue_ecn_enable[i] = p_aclqos_if->queue_ecn_enable[i];
    }
    p_pb_aclqos_if->queue_transimt_pkt = XCALLOC(MEM_LIB_PROTOBUF, sizeof(uint32_t)*GLB_QOS_PORT_QUEUE_NUM);
    p_pb_aclqos_if->n_queue_transimt_pkt = GLB_QOS_PORT_QUEUE_NUM;
    for (i = 0; i < GLB_QOS_PORT_QUEUE_NUM; i++)
    {
        p_pb_aclqos_if->queue_transimt_pkt[i] = p_aclqos_if->queue_transimt_pkt[i];
    }
    p_pb_aclqos_if->queue_transimt_byte = XCALLOC(MEM_LIB_PROTOBUF, sizeof(uint32_t)*GLB_QOS_PORT_QUEUE_NUM);
    p_pb_aclqos_if->n_queue_transimt_byte = GLB_QOS_PORT_QUEUE_NUM;
    for (i = 0; i < GLB_QOS_PORT_QUEUE_NUM; i++)
    {
        p_pb_aclqos_if->queue_transimt_byte[i] = p_aclqos_if->queue_transimt_byte[i];
    }
    p_pb_aclqos_if->queue_drop_pkt = XCALLOC(MEM_LIB_PROTOBUF, sizeof(uint32_t)*GLB_QOS_PORT_QUEUE_NUM);
    p_pb_aclqos_if->n_queue_drop_pkt = GLB_QOS_PORT_QUEUE_NUM;
    for (i = 0; i < GLB_QOS_PORT_QUEUE_NUM; i++)
    {
        p_pb_aclqos_if->queue_drop_pkt[i] = p_aclqos_if->queue_drop_pkt[i];
    }
    p_pb_aclqos_if->queue_drop_byte = XCALLOC(MEM_LIB_PROTOBUF, sizeof(uint32_t)*GLB_QOS_PORT_QUEUE_NUM);
    p_pb_aclqos_if->n_queue_drop_byte = GLB_QOS_PORT_QUEUE_NUM;
    for (i = 0; i < GLB_QOS_PORT_QUEUE_NUM; i++)
    {
        p_pb_aclqos_if->queue_drop_byte[i] = p_aclqos_if->queue_drop_byte[i];
    }
    p_pb_aclqos_if->queue0_transimt_pkt = p_aclqos_if->queue0_transimt_pkt;
    p_pb_aclqos_if->queue0_transimt_byte = p_aclqos_if->queue0_transimt_byte;
    p_pb_aclqos_if->queue0_drop_pkt = p_aclqos_if->queue0_drop_pkt;
    p_pb_aclqos_if->queue0_drop_byte = p_aclqos_if->queue0_drop_byte;
    p_pb_aclqos_if->queue1_transimt_pkt = p_aclqos_if->queue1_transimt_pkt;
    p_pb_aclqos_if->queue1_transimt_byte = p_aclqos_if->queue1_transimt_byte;
    p_pb_aclqos_if->queue1_drop_pkt = p_aclqos_if->queue1_drop_pkt;
    p_pb_aclqos_if->queue1_drop_byte = p_aclqos_if->queue1_drop_byte;
    p_pb_aclqos_if->queue2_transimt_pkt = p_aclqos_if->queue2_transimt_pkt;
    p_pb_aclqos_if->queue2_transimt_byte = p_aclqos_if->queue2_transimt_byte;
    p_pb_aclqos_if->queue2_drop_pkt = p_aclqos_if->queue2_drop_pkt;
    p_pb_aclqos_if->queue2_drop_byte = p_aclqos_if->queue2_drop_byte;
    p_pb_aclqos_if->queue3_transimt_pkt = p_aclqos_if->queue3_transimt_pkt;
    p_pb_aclqos_if->queue3_transimt_byte = p_aclqos_if->queue3_transimt_byte;
    p_pb_aclqos_if->queue3_drop_pkt = p_aclqos_if->queue3_drop_pkt;
    p_pb_aclqos_if->queue3_drop_byte = p_aclqos_if->queue3_drop_byte;
    p_pb_aclqos_if->queue4_transimt_pkt = p_aclqos_if->queue4_transimt_pkt;
    p_pb_aclqos_if->queue4_transimt_byte = p_aclqos_if->queue4_transimt_byte;
    p_pb_aclqos_if->queue4_drop_pkt = p_aclqos_if->queue4_drop_pkt;
    p_pb_aclqos_if->queue4_drop_byte = p_aclqos_if->queue4_drop_byte;
    p_pb_aclqos_if->queue5_transimt_pkt = p_aclqos_if->queue5_transimt_pkt;
    p_pb_aclqos_if->queue5_transimt_byte = p_aclqos_if->queue5_transimt_byte;
    p_pb_aclqos_if->queue5_drop_pkt = p_aclqos_if->queue5_drop_pkt;
    p_pb_aclqos_if->queue5_drop_byte = p_aclqos_if->queue5_drop_byte;
    p_pb_aclqos_if->queue6_transimt_pkt = p_aclqos_if->queue6_transimt_pkt;
    p_pb_aclqos_if->queue6_transimt_byte = p_aclqos_if->queue6_transimt_byte;
    p_pb_aclqos_if->queue6_drop_pkt = p_aclqos_if->queue6_drop_pkt;
    p_pb_aclqos_if->queue6_drop_byte = p_aclqos_if->queue6_drop_byte;
    p_pb_aclqos_if->queue7_transimt_pkt = p_aclqos_if->queue7_transimt_pkt;
    p_pb_aclqos_if->queue7_transimt_byte = p_aclqos_if->queue7_transimt_byte;
    p_pb_aclqos_if->queue7_drop_pkt = p_aclqos_if->queue7_drop_pkt;
    p_pb_aclqos_if->queue7_drop_byte = p_aclqos_if->queue7_drop_byte;
    p_pb_aclqos_if->phb_enable = p_aclqos_if->phb_enable;

    return PM_E_NONE;
}

int32
pb_ds_aclqos_if_to_pb_free_packed(Cdb__DsAclqosIf *p_pb_aclqos_if)
{
    uint32 i = 0;

    if (p_pb_aclqos_if->parent_if->name)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_aclqos_if->parent_if->name);
        p_pb_aclqos_if->parent_if->name = NULL;
    }

    if (p_pb_aclqos_if->input_policy_map)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_aclqos_if->input_policy_map);
        p_pb_aclqos_if->input_policy_map = NULL;
    }

    if (p_pb_aclqos_if->output_policy_map)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_aclqos_if->output_policy_map);
        p_pb_aclqos_if->output_policy_map = NULL;
    }

    if (p_pb_aclqos_if->input_policer)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_aclqos_if->input_policer);
        p_pb_aclqos_if->input_policer = NULL;
    }

    if (p_pb_aclqos_if->output_policer)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_aclqos_if->output_policer);
        p_pb_aclqos_if->output_policer = NULL;
    }

    if (p_pb_aclqos_if->queue_shape_profile)
    {
        for (i = 0; i < p_pb_aclqos_if->n_queue_shape_profile; i++)
        {
            if (p_pb_aclqos_if->queue_shape_profile[i])
            {
                XFREE(MEM_LIB_PROTOBUF, p_pb_aclqos_if->queue_shape_profile[i]);
                p_pb_aclqos_if->queue_shape_profile[i] = NULL;
            }
        }
        XFREE(MEM_LIB_PROTOBUF, p_pb_aclqos_if->queue_shape_profile);
        p_pb_aclqos_if->queue_shape_profile = NULL;
    }

    if (p_pb_aclqos_if->queue_drop_profile)
    {
        for (i = 0; i < p_pb_aclqos_if->n_queue_drop_profile; i++)
        {
            if (p_pb_aclqos_if->queue_drop_profile[i])
            {
                XFREE(MEM_LIB_PROTOBUF, p_pb_aclqos_if->queue_drop_profile[i]);
                p_pb_aclqos_if->queue_drop_profile[i] = NULL;
            }
        }
        XFREE(MEM_LIB_PROTOBUF, p_pb_aclqos_if->queue_drop_profile);
        p_pb_aclqos_if->queue_drop_profile = NULL;
    }

    if (p_pb_aclqos_if->queue_class)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_aclqos_if->queue_class);
        p_pb_aclqos_if->queue_class = NULL;
    }

    if (p_pb_aclqos_if->queue_weight)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_aclqos_if->queue_weight);
        p_pb_aclqos_if->queue_weight = NULL;
    }

    if (p_pb_aclqos_if->queue_drop_mode)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_aclqos_if->queue_drop_mode);
        p_pb_aclqos_if->queue_drop_mode = NULL;
    }

    if (p_pb_aclqos_if->queue_ecn_enable)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_aclqos_if->queue_ecn_enable);
        p_pb_aclqos_if->queue_ecn_enable = NULL;
    }

    if (p_pb_aclqos_if->queue_transimt_pkt)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_aclqos_if->queue_transimt_pkt);
        p_pb_aclqos_if->queue_transimt_pkt = NULL;
    }

    if (p_pb_aclqos_if->queue_transimt_byte)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_aclqos_if->queue_transimt_byte);
        p_pb_aclqos_if->queue_transimt_byte = NULL;
    }

    if (p_pb_aclqos_if->queue_drop_pkt)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_aclqos_if->queue_drop_pkt);
        p_pb_aclqos_if->queue_drop_pkt = NULL;
    }

    if (p_pb_aclqos_if->queue_drop_byte)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_aclqos_if->queue_drop_byte);
        p_pb_aclqos_if->queue_drop_byte = NULL;
    }

    return PM_E_NONE;
}

int32
pb_ds_aclqos_if_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len)
{
    Cdb__TblInterfaceKey pb_if_key = CDB__TBL_INTERFACE_KEY__INIT;
    Cdb__DsAclqosIf pb_aclqos_if = CDB__DS_ACLQOS_IF__INIT;
    tbl_interface_key_t *p_if_key = (tbl_interface_key_t*)p_tbl;
    ds_aclqos_if_t *p_aclqos_if = (ds_aclqos_if_t*)p_ds;
    int32 len = 0;

    pb_aclqos_if.parent_if = &pb_if_key;
    pb_ds_aclqos_if_to_pb(p_if_key, p_aclqos_if, &pb_aclqos_if);
    len = cdb__ds_aclqos_if__pack(&pb_aclqos_if, buf);
    pb_ds_aclqos_if_to_pb_free_packed(&pb_aclqos_if);

    return len;
}

int32
pb_ds_aclqos_if_dump(Cdb__DsAclqosIf *p_pb_aclqos_if, char *out)
{
    int32 offset = 0;

    out[0] = CMD_ZERO_CHAR;
    offset += sal_sprintf(out + offset, "/parent_if->name=%s", p_pb_aclqos_if->parent_if->name);
    offset += sal_sprintf(out + offset, "/default_cos=%u", p_pb_aclqos_if->default_cos);
    offset += sal_sprintf(out + offset, "/domain=%u", p_pb_aclqos_if->domain);
    offset += sal_sprintf(out + offset, "/trust=%u", p_pb_aclqos_if->trust);
    offset += sal_sprintf(out + offset, "/replace=%u", p_pb_aclqos_if->replace);
    offset += sal_sprintf(out + offset, "/shape_pir=%u", p_pb_aclqos_if->shape_pir);
    offset += sal_sprintf(out + offset, "/queue_stats_enable=%u", p_pb_aclqos_if->queue_stats_enable);
    offset += sal_sprintf(out + offset, "/input_policy_map=%s", p_pb_aclqos_if->input_policy_map);
    offset += sal_sprintf(out + offset, "/output_policy_map=%s", p_pb_aclqos_if->output_policy_map);
    offset += sal_sprintf(out + offset, "/input_policer=%s", p_pb_aclqos_if->input_policer);
    offset += sal_sprintf(out + offset, "/output_policer=%s", p_pb_aclqos_if->output_policer);
    offset += sal_sprintf(out + offset, "/queue_shape_profile=");
    offset += pb_string_array_dump(p_pb_aclqos_if->queue_shape_profile, p_pb_aclqos_if->n_queue_shape_profile, (out + offset));
    offset += sal_sprintf(out + offset, "/queue_drop_profile=");
    offset += pb_string_array_dump(p_pb_aclqos_if->queue_drop_profile, p_pb_aclqos_if->n_queue_drop_profile, (out + offset));
    offset += sal_sprintf(out + offset, "/queue_class=");
    offset += pb_uint32_array_dump(p_pb_aclqos_if->queue_class, sizeof(p_pb_aclqos_if->queue_class), (out + offset));
    offset += sal_sprintf(out + offset, "/queue_weight=");
    offset += pb_uint32_array_dump(p_pb_aclqos_if->queue_weight, sizeof(p_pb_aclqos_if->queue_weight), (out + offset));
    offset += sal_sprintf(out + offset, "/queue_drop_mode=");
    offset += pb_uint32_array_dump(p_pb_aclqos_if->queue_drop_mode, sizeof(p_pb_aclqos_if->queue_drop_mode), (out + offset));
    offset += sal_sprintf(out + offset, "/queue_ecn_enable=");
    offset += pb_uint32_array_dump(p_pb_aclqos_if->queue_ecn_enable, sizeof(p_pb_aclqos_if->queue_ecn_enable), (out + offset));
    offset += sal_sprintf(out + offset, "/queue_transimt_pkt=");
    offset += pb_uint32_array_dump(p_pb_aclqos_if->queue_transimt_pkt, sizeof(p_pb_aclqos_if->queue_transimt_pkt), (out + offset));
    offset += sal_sprintf(out + offset, "/queue_transimt_byte=");
    offset += pb_uint32_array_dump(p_pb_aclqos_if->queue_transimt_byte, sizeof(p_pb_aclqos_if->queue_transimt_byte), (out + offset));
    offset += sal_sprintf(out + offset, "/queue_drop_pkt=");
    offset += pb_uint32_array_dump(p_pb_aclqos_if->queue_drop_pkt, sizeof(p_pb_aclqos_if->queue_drop_pkt), (out + offset));
    offset += sal_sprintf(out + offset, "/queue_drop_byte=");
    offset += pb_uint32_array_dump(p_pb_aclqos_if->queue_drop_byte, sizeof(p_pb_aclqos_if->queue_drop_byte), (out + offset));
    offset += sal_sprintf(out + offset, "/queue0_transimt_pkt=%"PRIu64, p_pb_aclqos_if->queue0_transimt_pkt);
    offset += sal_sprintf(out + offset, "/queue0_transimt_byte=%"PRIu64, p_pb_aclqos_if->queue0_transimt_byte);
    offset += sal_sprintf(out + offset, "/queue0_drop_pkt=%"PRIu64, p_pb_aclqos_if->queue0_drop_pkt);
    offset += sal_sprintf(out + offset, "/queue0_drop_byte=%"PRIu64, p_pb_aclqos_if->queue0_drop_byte);
    offset += sal_sprintf(out + offset, "/queue1_transimt_pkt=%"PRIu64, p_pb_aclqos_if->queue1_transimt_pkt);
    offset += sal_sprintf(out + offset, "/queue1_transimt_byte=%"PRIu64, p_pb_aclqos_if->queue1_transimt_byte);
    offset += sal_sprintf(out + offset, "/queue1_drop_pkt=%"PRIu64, p_pb_aclqos_if->queue1_drop_pkt);
    offset += sal_sprintf(out + offset, "/queue1_drop_byte=%"PRIu64, p_pb_aclqos_if->queue1_drop_byte);
    offset += sal_sprintf(out + offset, "/queue2_transimt_pkt=%"PRIu64, p_pb_aclqos_if->queue2_transimt_pkt);
    offset += sal_sprintf(out + offset, "/queue2_transimt_byte=%"PRIu64, p_pb_aclqos_if->queue2_transimt_byte);
    offset += sal_sprintf(out + offset, "/queue2_drop_pkt=%"PRIu64, p_pb_aclqos_if->queue2_drop_pkt);
    offset += sal_sprintf(out + offset, "/queue2_drop_byte=%"PRIu64, p_pb_aclqos_if->queue2_drop_byte);
    offset += sal_sprintf(out + offset, "/queue3_transimt_pkt=%"PRIu64, p_pb_aclqos_if->queue3_transimt_pkt);
    offset += sal_sprintf(out + offset, "/queue3_transimt_byte=%"PRIu64, p_pb_aclqos_if->queue3_transimt_byte);
    offset += sal_sprintf(out + offset, "/queue3_drop_pkt=%"PRIu64, p_pb_aclqos_if->queue3_drop_pkt);
    offset += sal_sprintf(out + offset, "/queue3_drop_byte=%"PRIu64, p_pb_aclqos_if->queue3_drop_byte);
    offset += sal_sprintf(out + offset, "/queue4_transimt_pkt=%"PRIu64, p_pb_aclqos_if->queue4_transimt_pkt);
    offset += sal_sprintf(out + offset, "/queue4_transimt_byte=%"PRIu64, p_pb_aclqos_if->queue4_transimt_byte);
    offset += sal_sprintf(out + offset, "/queue4_drop_pkt=%"PRIu64, p_pb_aclqos_if->queue4_drop_pkt);
    offset += sal_sprintf(out + offset, "/queue4_drop_byte=%"PRIu64, p_pb_aclqos_if->queue4_drop_byte);
    offset += sal_sprintf(out + offset, "/queue5_transimt_pkt=%"PRIu64, p_pb_aclqos_if->queue5_transimt_pkt);
    offset += sal_sprintf(out + offset, "/queue5_transimt_byte=%"PRIu64, p_pb_aclqos_if->queue5_transimt_byte);
    offset += sal_sprintf(out + offset, "/queue5_drop_pkt=%"PRIu64, p_pb_aclqos_if->queue5_drop_pkt);
    offset += sal_sprintf(out + offset, "/queue5_drop_byte=%"PRIu64, p_pb_aclqos_if->queue5_drop_byte);
    offset += sal_sprintf(out + offset, "/queue6_transimt_pkt=%"PRIu64, p_pb_aclqos_if->queue6_transimt_pkt);
    offset += sal_sprintf(out + offset, "/queue6_transimt_byte=%"PRIu64, p_pb_aclqos_if->queue6_transimt_byte);
    offset += sal_sprintf(out + offset, "/queue6_drop_pkt=%"PRIu64, p_pb_aclqos_if->queue6_drop_pkt);
    offset += sal_sprintf(out + offset, "/queue6_drop_byte=%"PRIu64, p_pb_aclqos_if->queue6_drop_byte);
    offset += sal_sprintf(out + offset, "/queue7_transimt_pkt=%"PRIu64, p_pb_aclqos_if->queue7_transimt_pkt);
    offset += sal_sprintf(out + offset, "/queue7_transimt_byte=%"PRIu64, p_pb_aclqos_if->queue7_transimt_byte);
    offset += sal_sprintf(out + offset, "/queue7_drop_pkt=%"PRIu64, p_pb_aclqos_if->queue7_drop_pkt);
    offset += sal_sprintf(out + offset, "/queue7_drop_byte=%"PRIu64, p_pb_aclqos_if->queue7_drop_byte);
    offset += sal_sprintf(out + offset, "/phb_enable=%u", p_pb_aclqos_if->phb_enable);
    offset += sal_sprintf(out + offset, "\n");

    return PM_E_NONE;
}

/* pb functions for TBL_INTERFACE DS_LAG */
int32
pb_ds_lag_to_pb(tbl_interface_key_t *p_if_key, ds_lag_t *p_lag, Cdb__DsLag *p_pb_lag)
{
    p_pb_lag->parent_if->name = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_if_key->name)+1);
    sal_strcpy(p_pb_lag->parent_if->name, p_if_key->name);

    p_pb_lag->mode = p_lag->mode;
    p_pb_lag->lag_id = p_lag->lag_id;
    p_pb_lag->oper_state = p_lag->oper_state;
    p_pb_lag->mismatch = p_lag->mismatch;
    p_pb_lag->bundle_ports_count = p_lag->bundle_ports_count;
    p_pb_lag->load_balance_mode = p_lag->load_balance_mode;
    p_pb_lag->is_active_backup_mode = p_lag->is_active_backup_mode;
    p_pb_lag->max_active_number = p_lag->max_active_number;
    p_pb_lag->cur_active_number = p_lag->cur_active_number;
    p_pb_lag->cur_active_max_priority = p_lag->cur_active_max_priority;

    return PM_E_NONE;
}

int32
pb_ds_lag_to_pb_free_packed(Cdb__DsLag *p_pb_lag)
{
    if (p_pb_lag->parent_if->name)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_lag->parent_if->name);
        p_pb_lag->parent_if->name = NULL;
    }

    return PM_E_NONE;
}

int32
pb_ds_lag_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len)
{
    Cdb__TblInterfaceKey pb_if_key = CDB__TBL_INTERFACE_KEY__INIT;
    Cdb__DsLag pb_lag = CDB__DS_LAG__INIT;
    tbl_interface_key_t *p_if_key = (tbl_interface_key_t*)p_tbl;
    ds_lag_t *p_lag = (ds_lag_t*)p_ds;
    int32 len = 0;

    pb_lag.parent_if = &pb_if_key;
    pb_ds_lag_to_pb(p_if_key, p_lag, &pb_lag);
    len = cdb__ds_lag__pack(&pb_lag, buf);
    pb_ds_lag_to_pb_free_packed(&pb_lag);

    return len;
}

int32
pb_ds_lag_dump(Cdb__DsLag *p_pb_lag, char *out)
{
    int32 offset = 0;

    out[0] = CMD_ZERO_CHAR;
    offset += sal_sprintf(out + offset, "/parent_if->name=%s", p_pb_lag->parent_if->name);
    offset += sal_sprintf(out + offset, "/mode=%u", p_pb_lag->mode);
    offset += sal_sprintf(out + offset, "/lag_id=%u", p_pb_lag->lag_id);
    offset += sal_sprintf(out + offset, "/oper_state=%u", p_pb_lag->oper_state);
    offset += sal_sprintf(out + offset, "/mismatch=%u", p_pb_lag->mismatch);
    offset += sal_sprintf(out + offset, "/bundle_ports_count=%d", p_pb_lag->bundle_ports_count);
    offset += sal_sprintf(out + offset, "/load_balance_mode=%u", p_pb_lag->load_balance_mode);
    offset += sal_sprintf(out + offset, "/is_active_backup_mode=%u", p_pb_lag->is_active_backup_mode);
    offset += sal_sprintf(out + offset, "/max_active_number=%u", p_pb_lag->max_active_number);
    offset += sal_sprintf(out + offset, "/cur_active_number=%u", p_pb_lag->cur_active_number);
    offset += sal_sprintf(out + offset, "/cur_active_max_priority=%u", p_pb_lag->cur_active_max_priority);
    offset += sal_sprintf(out + offset, "\n");

    return PM_E_NONE;
}

/* pb functions for TBL_INTERFACE DS_LACP */
int32
pb_ds_lacp_to_pb(tbl_interface_key_t *p_if_key, ds_lacp_t *p_lacp, Cdb__DsLacp *p_pb_lacp)
{
    p_pb_lacp->parent_if->name = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_if_key->name)+1);
    sal_strcpy(p_pb_lacp->parent_if->name, p_if_key->name);

    p_pb_lacp->actor_port_priority = p_lacp->actor_port_priority;
    p_pb_lacp->actor_oper_port_key = p_lacp->actor_oper_port_key;
    p_pb_lacp->actor_port_number = p_lacp->actor_port_number;
    p_pb_lacp->short_timeout = p_lacp->short_timeout;
    p_pb_lacp->actor_oper_port_state = p_lacp->actor_oper_port_state;
    p_pb_lacp->partner_admin_system_priority = p_lacp->partner_admin_system_priority;
    pb_compose_mac_addr_t_to_pb(p_lacp->partner_admin_system, p_pb_lacp->partner_admin_system);
    p_pb_lacp->partner_admin_key = p_lacp->partner_admin_key;
    p_pb_lacp->partner_admin_port_priority = p_lacp->partner_admin_port_priority;
    p_pb_lacp->partner_admin_port_number = p_lacp->partner_admin_port_number;
    p_pb_lacp->partner_admin_port_state = p_lacp->partner_admin_port_state;
    p_pb_lacp->partner_oper_system_priority = p_lacp->partner_oper_system_priority;
    pb_compose_mac_addr_t_to_pb(p_lacp->partner_oper_system, p_pb_lacp->partner_oper_system);
    p_pb_lacp->partner_oper_key = p_lacp->partner_oper_key;
    p_pb_lacp->partner_oper_port_priority = p_lacp->partner_oper_port_priority;
    p_pb_lacp->partner_oper_port_number = p_lacp->partner_oper_port_number;
    p_pb_lacp->partner_oper_port_state = p_lacp->partner_oper_port_state;
    p_pb_lacp->tx_lacp_count = p_lacp->tx_lacp_count;
    p_pb_lacp->tx_error_count = p_lacp->tx_error_count;
    p_pb_lacp->rx_lacp_count = p_lacp->rx_lacp_count;
    p_pb_lacp->rx_error_count = p_lacp->rx_error_count;
    p_pb_lacp->mlag_id = p_lacp->mlag_id;
    p_pb_lacp->ntt = p_lacp->ntt;
    p_pb_lacp->lacp_enabled = p_lacp->lacp_enabled;
    p_pb_lacp->ready_n = p_lacp->ready_n;
    p_pb_lacp->port_moved = p_lacp->port_moved;
    p_pb_lacp->periodic_tx_state = p_lacp->periodic_tx_state;
    p_pb_lacp->receive_state = p_lacp->receive_state;
    p_pb_lacp->mux_state = p_lacp->mux_state;
    p_pb_lacp->selected = p_lacp->selected;

    return PM_E_NONE;
}

int32
pb_ds_lacp_to_pb_free_packed(Cdb__DsLacp *p_pb_lacp)
{
    if (p_pb_lacp->parent_if->name)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_lacp->parent_if->name);
        p_pb_lacp->parent_if->name = NULL;
    }

    pb_compose_mac_addr_t_to_pb_free_packed(p_pb_lacp->partner_admin_system);
    pb_compose_mac_addr_t_to_pb_free_packed(p_pb_lacp->partner_oper_system);
    return PM_E_NONE;
}

int32
pb_ds_lacp_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len)
{
    Cdb__TblInterfaceKey pb_if_key = CDB__TBL_INTERFACE_KEY__INIT;
    Cdb__DsLacp pb_lacp = CDB__DS_LACP__INIT;
    tbl_interface_key_t *p_if_key = (tbl_interface_key_t*)p_tbl;
    ds_lacp_t *p_lacp = (ds_lacp_t*)p_ds;
    int32 len = 0;
    Cdb__ComposeMacAddrT partner_admin_system = CDB__COMPOSE_MAC_ADDR_T__INIT;
    Cdb__ComposeMacAddrT partner_oper_system = CDB__COMPOSE_MAC_ADDR_T__INIT;

    pb_lacp.partner_admin_system = &partner_admin_system;
    pb_lacp.partner_oper_system = &partner_oper_system;
    pb_lacp.parent_if = &pb_if_key;
    pb_ds_lacp_to_pb(p_if_key, p_lacp, &pb_lacp);
    len = cdb__ds_lacp__pack(&pb_lacp, buf);
    pb_ds_lacp_to_pb_free_packed(&pb_lacp);

    return len;
}

int32
pb_ds_lacp_dump(Cdb__DsLacp *p_pb_lacp, char *out)
{
    int32 offset = 0;

    out[0] = CMD_ZERO_CHAR;
    offset += sal_sprintf(out + offset, "/parent_if->name=%s", p_pb_lacp->parent_if->name);
    offset += sal_sprintf(out + offset, "/actor_port_priority=%u", p_pb_lacp->actor_port_priority);
    offset += sal_sprintf(out + offset, "/actor_oper_port_key=%u", p_pb_lacp->actor_oper_port_key);
    offset += sal_sprintf(out + offset, "/actor_port_number=%u", p_pb_lacp->actor_port_number);
    offset += sal_sprintf(out + offset, "/short_timeout=%u", p_pb_lacp->short_timeout);
    offset += sal_sprintf(out + offset, "/actor_oper_port_state=%u", p_pb_lacp->actor_oper_port_state);
    offset += sal_sprintf(out + offset, "/partner_admin_system_priority=%u", p_pb_lacp->partner_admin_system_priority);
    offset += pb_compose_mac_addr_t_dump(p_pb_lacp->partner_admin_system, (out + offset));
    offset += sal_sprintf(out + offset, "/partner_admin_key=%u", p_pb_lacp->partner_admin_key);
    offset += sal_sprintf(out + offset, "/partner_admin_port_priority=%u", p_pb_lacp->partner_admin_port_priority);
    offset += sal_sprintf(out + offset, "/partner_admin_port_number=%u", p_pb_lacp->partner_admin_port_number);
    offset += sal_sprintf(out + offset, "/partner_admin_port_state=%u", p_pb_lacp->partner_admin_port_state);
    offset += sal_sprintf(out + offset, "/partner_oper_system_priority=%u", p_pb_lacp->partner_oper_system_priority);
    offset += pb_compose_mac_addr_t_dump(p_pb_lacp->partner_oper_system, (out + offset));
    offset += sal_sprintf(out + offset, "/partner_oper_key=%u", p_pb_lacp->partner_oper_key);
    offset += sal_sprintf(out + offset, "/partner_oper_port_priority=%u", p_pb_lacp->partner_oper_port_priority);
    offset += sal_sprintf(out + offset, "/partner_oper_port_number=%u", p_pb_lacp->partner_oper_port_number);
    offset += sal_sprintf(out + offset, "/partner_oper_port_state=%u", p_pb_lacp->partner_oper_port_state);
    offset += sal_sprintf(out + offset, "/tx_lacp_count=%u", p_pb_lacp->tx_lacp_count);
    offset += sal_sprintf(out + offset, "/tx_error_count=%u", p_pb_lacp->tx_error_count);
    offset += sal_sprintf(out + offset, "/rx_lacp_count=%u", p_pb_lacp->rx_lacp_count);
    offset += sal_sprintf(out + offset, "/rx_error_count=%u", p_pb_lacp->rx_error_count);
    offset += sal_sprintf(out + offset, "/mlag_id=%u", p_pb_lacp->mlag_id);
    offset += sal_sprintf(out + offset, "/ntt=%u", p_pb_lacp->ntt);
    offset += sal_sprintf(out + offset, "/lacp_enabled=%u", p_pb_lacp->lacp_enabled);
    offset += sal_sprintf(out + offset, "/ready_n=%u", p_pb_lacp->ready_n);
    offset += sal_sprintf(out + offset, "/port_moved=%u", p_pb_lacp->port_moved);
    offset += sal_sprintf(out + offset, "/periodic_tx_state=%u", p_pb_lacp->periodic_tx_state);
    offset += sal_sprintf(out + offset, "/receive_state=%u", p_pb_lacp->receive_state);
    offset += sal_sprintf(out + offset, "/mux_state=%u", p_pb_lacp->mux_state);
    offset += sal_sprintf(out + offset, "/selected=%u", p_pb_lacp->selected);
    offset += sal_sprintf(out + offset, "\n");

    return PM_E_NONE;
}

/* pb functions for TBL_ROUTE_IF DS_CONNECTED */
int32
pb_ds_connected_to_pb(tbl_route_if_key_t *p_rtif_key, ds_connected_t *p_connect, Cdb__DsConnected *p_pb_connect)
{
    p_pb_connect->parent_rtif->name = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_rtif_key->name)+1);
    sal_strcpy(p_pb_connect->parent_rtif->name, p_rtif_key->name);

    pb_compose_prefix_t_to_pb(&p_connect->key.address, p_pb_connect->key->address);

    pb_compose_prefix_t_to_pb(&p_connect->destination, p_pb_connect->destination);
    p_pb_connect->flags_secondary = GLB_FLAG_ISSET(p_connect->flags, RT_IF_ADDR_SECONDARY) ? TRUE : FALSE;
    p_pb_connect->flags_anycast = GLB_FLAG_ISSET(p_connect->flags, RT_IF_ADDR_ANYCAST) ? TRUE : FALSE;
    p_pb_connect->flags_virtual = GLB_FLAG_ISSET(p_connect->flags, RT_IF_ADDR_VIRTUAL) ? TRUE : FALSE;
    p_pb_connect->flags_duplicate = GLB_FLAG_ISSET(p_connect->flags, RT_IF_ADDR_DUPLICATE) ? TRUE : FALSE;
    p_pb_connect->installed = p_connect->installed;

    return PM_E_NONE;
}

int32
pb_ds_connected_to_pb_free_packed(Cdb__DsConnected *p_pb_connect)
{
    if (p_pb_connect->parent_rtif->name)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_connect->parent_rtif->name);
        p_pb_connect->parent_rtif->name = NULL;
    }

    pb_compose_prefix_t_to_pb_free_packed(p_pb_connect->key->address);
    pb_compose_prefix_t_to_pb_free_packed(p_pb_connect->destination);
    return PM_E_NONE;
}

int32
pb_ds_connected_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len)
{
    Cdb__TblRouteIfKey pb_rtif_key = CDB__TBL_ROUTE_IF_KEY__INIT;
    Cdb__ComposePrefixT address = CDB__COMPOSE_PREFIX_T__INIT;
    Cdb__DsConnectedKey pb_connect_key = CDB__DS_CONNECTED_KEY__INIT;
    Cdb__DsConnected pb_connect = CDB__DS_CONNECTED__INIT;
    tbl_route_if_key_t *p_rtif_key = (tbl_route_if_key_t*)p_tbl;
    ds_connected_t *p_connect = (ds_connected_t*)p_ds;
    int32 len = 0;
    Cdb__ComposePrefixT destination = CDB__COMPOSE_PREFIX_T__INIT;

    pb_connect_key.address = &address;
    pb_connect.key = &pb_connect_key;
    pb_connect.destination = &destination;
    pb_connect.parent_rtif = &pb_rtif_key;
    pb_ds_connected_to_pb(p_rtif_key, p_connect, &pb_connect);
    len = cdb__ds_connected__pack(&pb_connect, buf);
    pb_ds_connected_to_pb_free_packed(&pb_connect);

    return len;
}

int32
pb_ds_connected_dump(Cdb__DsConnected *p_pb_connect, char *out)
{
    int32 offset = 0;

    out[0] = CMD_ZERO_CHAR;
    offset += sal_sprintf(out + offset, "/parent_rtif->name=%s", p_pb_connect->parent_rtif->name);
    offset += pb_compose_prefix_t_dump(p_pb_connect->key->address, (out + offset));
    offset += pb_compose_prefix_t_dump(p_pb_connect->destination, (out + offset));
    offset += sal_sprintf(out + offset, "/flags_secondary=%u", p_pb_connect->flags_secondary);
    offset += sal_sprintf(out + offset, "/flags_anycast=%u", p_pb_connect->flags_anycast);
    offset += sal_sprintf(out + offset, "/flags_virtual=%u", p_pb_connect->flags_virtual);
    offset += sal_sprintf(out + offset, "/flags_duplicate=%u", p_pb_connect->flags_duplicate);
    offset += sal_sprintf(out + offset, "/installed=%u", p_pb_connect->installed);
    offset += sal_sprintf(out + offset, "\n");

    return PM_E_NONE;
}

/* pb functions for TBL_ROUTE_IF DS_CONNECTED_V6 */
int32
pb_ds_connected_v6_to_pb(tbl_route_if_key_t *p_rtif_key, ds_connected_v6_t *p_connect_v6, Cdb__DsConnectedV6 *p_pb_connect_v6)
{
    p_pb_connect_v6->parent_rtif->name = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_rtif_key->name)+1);
    sal_strcpy(p_pb_connect_v6->parent_rtif->name, p_rtif_key->name);

    pb_compose_prefix_t_to_pb(&p_connect_v6->key.address, p_pb_connect_v6->key->address);

    pb_compose_prefix_t_to_pb(&p_connect_v6->destination, p_pb_connect_v6->destination);
    p_pb_connect_v6->flags_secondary = GLB_FLAG_ISSET(p_connect_v6->flags, RT_IF_ADDR_SECONDARY) ? TRUE : FALSE;
    p_pb_connect_v6->flags_anycast = GLB_FLAG_ISSET(p_connect_v6->flags, RT_IF_ADDR_ANYCAST) ? TRUE : FALSE;
    p_pb_connect_v6->flags_virtual = GLB_FLAG_ISSET(p_connect_v6->flags, RT_IF_ADDR_VIRTUAL) ? TRUE : FALSE;
    p_pb_connect_v6->flags_duplicate = GLB_FLAG_ISSET(p_connect_v6->flags, RT_IF_ADDR_DUPLICATE) ? TRUE : FALSE;
    p_pb_connect_v6->flags_v6_global = GLB_FLAG_ISSET(p_connect_v6->flags, RT_IF_ADDRV6_GLOBAL) ? TRUE : FALSE;
    p_pb_connect_v6->flags_v6_linklocal = GLB_FLAG_ISSET(p_connect_v6->flags, RT_IF_ADDRV6_LINKLOCAL) ? TRUE : FALSE;
    p_pb_connect_v6->flags_v6_auto_linklocal = GLB_FLAG_ISSET(p_connect_v6->flags, RT_IF_ADDRV6_AUTO_LINKLOCAL) ? TRUE : FALSE;
    p_pb_connect_v6->flags_v6_global_eui64 = GLB_FLAG_ISSET(p_connect_v6->flags, RT_IF_ADDRV6_GLOBAL_EUI64) ? TRUE : FALSE;
    p_pb_connect_v6->flags_v6_dad_in_progress = GLB_FLAG_ISSET(p_connect_v6->flags, RT_IF_ADDRV6_DAD_IN_PROGRESS) ? TRUE : FALSE;
    p_pb_connect_v6->flags_v6_na_in_progress = GLB_FLAG_ISSET(p_connect_v6->flags, RT_IF_ADDRV6_NA_IN_PROGRESS) ? TRUE : FALSE;
    p_pb_connect_v6->flags_v6_configured = GLB_FLAG_ISSET(p_connect_v6->flags, RT_IF_ADDRV6_CONFIGURED) ? TRUE : FALSE;
    p_pb_connect_v6->flags_v6_real = GLB_FLAG_ISSET(p_connect_v6->flags, RT_IF_ADDRV6_REAL) ? TRUE : FALSE;
    p_pb_connect_v6->installed = p_connect_v6->installed;
    p_pb_connect_v6->ifname = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_connect_v6->ifname)+1);
    sal_strcpy(p_pb_connect_v6->ifname, p_connect_v6->ifname);
    p_pb_connect_v6->ifindex = p_connect_v6->ifindex;
    p_pb_connect_v6->dad_ns_send = p_connect_v6->dad_ns_send;
    p_pb_connect_v6->unsolicited_na = p_connect_v6->unsolicited_na;

    return PM_E_NONE;
}

int32
pb_ds_connected_v6_to_pb_free_packed(Cdb__DsConnectedV6 *p_pb_connect_v6)
{
    if (p_pb_connect_v6->parent_rtif->name)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_connect_v6->parent_rtif->name);
        p_pb_connect_v6->parent_rtif->name = NULL;
    }

    pb_compose_prefix_t_to_pb_free_packed(p_pb_connect_v6->key->address);
    pb_compose_prefix_t_to_pb_free_packed(p_pb_connect_v6->destination);
    if (p_pb_connect_v6->ifname)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_connect_v6->ifname);
        p_pb_connect_v6->ifname = NULL;
    }

    return PM_E_NONE;
}

int32
pb_ds_connected_v6_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len)
{
    Cdb__TblRouteIfKey pb_rtif_key = CDB__TBL_ROUTE_IF_KEY__INIT;
    Cdb__ComposePrefixT address = CDB__COMPOSE_PREFIX_T__INIT;
    Cdb__DsConnectedV6Key pb_connect_v6_key = CDB__DS_CONNECTED_V6_KEY__INIT;
    Cdb__DsConnectedV6 pb_connect_v6 = CDB__DS_CONNECTED_V6__INIT;
    tbl_route_if_key_t *p_rtif_key = (tbl_route_if_key_t*)p_tbl;
    ds_connected_v6_t *p_connect_v6 = (ds_connected_v6_t*)p_ds;
    int32 len = 0;
    Cdb__ComposePrefixT destination = CDB__COMPOSE_PREFIX_T__INIT;

    pb_connect_v6_key.address = &address;
    pb_connect_v6.key = &pb_connect_v6_key;
    pb_connect_v6.destination = &destination;
    pb_connect_v6.parent_rtif = &pb_rtif_key;
    pb_ds_connected_v6_to_pb(p_rtif_key, p_connect_v6, &pb_connect_v6);
    len = cdb__ds_connected_v6__pack(&pb_connect_v6, buf);
    pb_ds_connected_v6_to_pb_free_packed(&pb_connect_v6);

    return len;
}

int32
pb_ds_connected_v6_dump(Cdb__DsConnectedV6 *p_pb_connect_v6, char *out)
{
    int32 offset = 0;

    out[0] = CMD_ZERO_CHAR;
    offset += sal_sprintf(out + offset, "/parent_rtif->name=%s", p_pb_connect_v6->parent_rtif->name);
    offset += pb_compose_prefix_t_dump(p_pb_connect_v6->key->address, (out + offset));
    offset += pb_compose_prefix_t_dump(p_pb_connect_v6->destination, (out + offset));
    offset += sal_sprintf(out + offset, "/flags_secondary=%u", p_pb_connect_v6->flags_secondary);
    offset += sal_sprintf(out + offset, "/flags_anycast=%u", p_pb_connect_v6->flags_anycast);
    offset += sal_sprintf(out + offset, "/flags_virtual=%u", p_pb_connect_v6->flags_virtual);
    offset += sal_sprintf(out + offset, "/flags_duplicate=%u", p_pb_connect_v6->flags_duplicate);
    offset += sal_sprintf(out + offset, "/flags_v6_global=%u", p_pb_connect_v6->flags_v6_global);
    offset += sal_sprintf(out + offset, "/flags_v6_linklocal=%u", p_pb_connect_v6->flags_v6_linklocal);
    offset += sal_sprintf(out + offset, "/flags_v6_auto_linklocal=%u", p_pb_connect_v6->flags_v6_auto_linklocal);
    offset += sal_sprintf(out + offset, "/flags_v6_global_eui64=%u", p_pb_connect_v6->flags_v6_global_eui64);
    offset += sal_sprintf(out + offset, "/flags_v6_dad_in_progress=%u", p_pb_connect_v6->flags_v6_dad_in_progress);
    offset += sal_sprintf(out + offset, "/flags_v6_na_in_progress=%u", p_pb_connect_v6->flags_v6_na_in_progress);
    offset += sal_sprintf(out + offset, "/flags_v6_configured=%u", p_pb_connect_v6->flags_v6_configured);
    offset += sal_sprintf(out + offset, "/flags_v6_real=%u", p_pb_connect_v6->flags_v6_real);
    offset += sal_sprintf(out + offset, "/installed=%u", p_pb_connect_v6->installed);
    offset += sal_sprintf(out + offset, "/ifname=%s", p_pb_connect_v6->ifname);
    offset += sal_sprintf(out + offset, "/ifindex=%u", p_pb_connect_v6->ifindex);
    offset += sal_sprintf(out + offset, "/dad_ns_send=%u", p_pb_connect_v6->dad_ns_send);
    offset += sal_sprintf(out + offset, "/unsolicited_na=%u", p_pb_connect_v6->unsolicited_na);
    offset += sal_sprintf(out + offset, "\n");

    return PM_E_NONE;
}

/* pb functions for TBL_ARP DS_STATIC_ROUTE_DUP_IPV4_NH */
int32
pb_ds_static_route_dup_ipv4_nh_to_pb(tbl_arp_key_t *p_arp_key, ds_static_route_dup_ipv4_nh_t *p_static_route_dup_ipv4_nh, Cdb__DsStaticRouteDupIpv4Nh *p_pb_static_route_dup_ipv4_nh)
{
    pb_compose_addr_ipv4_t_to_pb(&p_arp_key->ip, p_pb_static_route_dup_ipv4_nh->parent_arp->ip);

    pb_compose_addr_ipv4_t_to_pb(&p_static_route_dup_ipv4_nh->key.address, p_pb_static_route_dup_ipv4_nh->key->address);


    return PM_E_NONE;
}

int32
pb_ds_static_route_dup_ipv4_nh_to_pb_free_packed(Cdb__DsStaticRouteDupIpv4Nh *p_pb_static_route_dup_ipv4_nh)
{
    pb_compose_addr_ipv4_t_to_pb_free_packed(p_pb_static_route_dup_ipv4_nh->parent_arp->ip);
    pb_compose_addr_ipv4_t_to_pb_free_packed(p_pb_static_route_dup_ipv4_nh->key->address);
    return PM_E_NONE;
}

int32
pb_ds_static_route_dup_ipv4_nh_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len)
{
    Cdb__TblArpKey pb_arp_key = CDB__TBL_ARP_KEY__INIT;
    Cdb__ComposeAddrIpv4T address = CDB__COMPOSE_ADDR_IPV4_T__INIT;
    Cdb__DsStaticRouteDupIpv4NhKey pb_static_route_dup_ipv4_nh_key = CDB__DS_STATIC_ROUTE_DUP_IPV4_NH_KEY__INIT;
    Cdb__DsStaticRouteDupIpv4Nh pb_static_route_dup_ipv4_nh = CDB__DS_STATIC_ROUTE_DUP_IPV4_NH__INIT;
    tbl_arp_key_t *p_arp_key = (tbl_arp_key_t*)p_tbl;
    ds_static_route_dup_ipv4_nh_t *p_static_route_dup_ipv4_nh = (ds_static_route_dup_ipv4_nh_t*)p_ds;
    int32 len = 0;

    pb_static_route_dup_ipv4_nh_key.address = &address;
    pb_static_route_dup_ipv4_nh.key = &pb_static_route_dup_ipv4_nh_key;
    pb_static_route_dup_ipv4_nh.parent_arp = &pb_arp_key;
    pb_ds_static_route_dup_ipv4_nh_to_pb(p_arp_key, p_static_route_dup_ipv4_nh, &pb_static_route_dup_ipv4_nh);
    len = cdb__ds_static_route_dup_ipv4_nh__pack(&pb_static_route_dup_ipv4_nh, buf);
    pb_ds_static_route_dup_ipv4_nh_to_pb_free_packed(&pb_static_route_dup_ipv4_nh);

    return len;
}

int32
pb_ds_static_route_dup_ipv4_nh_dump(Cdb__DsStaticRouteDupIpv4Nh *p_pb_static_route_dup_ipv4_nh, char *out)
{
    int32 offset = 0;

    out[0] = CMD_ZERO_CHAR;
    offset += pb_compose_addr_ipv4_t_dump(p_pb_static_route_dup_ipv4_nh->parent_arp->ip, (out + offset));
    offset += pb_compose_addr_ipv4_t_dump(p_pb_static_route_dup_ipv4_nh->key->address, (out + offset));
    offset += sal_sprintf(out + offset, "\n");

    return PM_E_NONE;
}

/* pb functions for TBL_ROUTE_IF DS_OSPF_AUTH */
int32
pb_ds_ospf_auth_to_pb(tbl_route_if_key_t *p_rtif_key, ds_ospf_auth_t *p_ospf_auth, Cdb__DsOspfAuth *p_pb_ospf_auth)
{
    p_pb_ospf_auth->parent_rtif->name = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_rtif_key->name)+1);
    sal_strcpy(p_pb_ospf_auth->parent_rtif->name, p_rtif_key->name);

    p_pb_ospf_auth->key->id = p_ospf_auth->key.id;

    p_pb_ospf_auth->ospf_auth_md5_key = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_ospf_auth->ospf_auth_md5_key)+1);
    sal_strcpy(p_pb_ospf_auth->ospf_auth_md5_key, p_ospf_auth->ospf_auth_md5_key);

    return PM_E_NONE;
}

int32
pb_ds_ospf_auth_to_pb_free_packed(Cdb__DsOspfAuth *p_pb_ospf_auth)
{
    if (p_pb_ospf_auth->parent_rtif->name)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_ospf_auth->parent_rtif->name);
        p_pb_ospf_auth->parent_rtif->name = NULL;
    }

    if (p_pb_ospf_auth->ospf_auth_md5_key)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_ospf_auth->ospf_auth_md5_key);
        p_pb_ospf_auth->ospf_auth_md5_key = NULL;
    }

    return PM_E_NONE;
}

int32
pb_ds_ospf_auth_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len)
{
    Cdb__TblRouteIfKey pb_rtif_key = CDB__TBL_ROUTE_IF_KEY__INIT;
    Cdb__DsOspfAuthKey pb_ospf_auth_key = CDB__DS_OSPF_AUTH_KEY__INIT;
    Cdb__DsOspfAuth pb_ospf_auth = CDB__DS_OSPF_AUTH__INIT;
    tbl_route_if_key_t *p_rtif_key = (tbl_route_if_key_t*)p_tbl;
    ds_ospf_auth_t *p_ospf_auth = (ds_ospf_auth_t*)p_ds;
    int32 len = 0;

    pb_ospf_auth.key = &pb_ospf_auth_key;
    pb_ospf_auth.parent_rtif = &pb_rtif_key;
    pb_ds_ospf_auth_to_pb(p_rtif_key, p_ospf_auth, &pb_ospf_auth);
    len = cdb__ds_ospf_auth__pack(&pb_ospf_auth, buf);
    pb_ds_ospf_auth_to_pb_free_packed(&pb_ospf_auth);

    return len;
}

int32
pb_ds_ospf_auth_dump(Cdb__DsOspfAuth *p_pb_ospf_auth, char *out)
{
    int32 offset = 0;

    out[0] = CMD_ZERO_CHAR;
    offset += sal_sprintf(out + offset, "/parent_rtif->name=%s", p_pb_ospf_auth->parent_rtif->name);
    offset += sal_sprintf(out + offset, "/key->id=%u", p_pb_ospf_auth->key->id);
    offset += sal_sprintf(out + offset, "/ospf_auth_md5_key=%s", p_pb_ospf_auth->ospf_auth_md5_key);
    offset += sal_sprintf(out + offset, "\n");

    return PM_E_NONE;
}

/* pb functions for TBL_INTERFACE DS_STORM_CONTROL */
int32
pb_ds_storm_control_to_pb(tbl_interface_key_t *p_if_key, ds_storm_control_t *p_storm_control, Cdb__DsStormControl *p_pb_storm_control)
{
    p_pb_storm_control->parent_if->name = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_if_key->name)+1);
    sal_strcpy(p_pb_storm_control->parent_if->name, p_if_key->name);

    p_pb_storm_control->bcast_mode = p_storm_control->bcast_mode;
    p_pb_storm_control->mcast_mode = p_storm_control->mcast_mode;
    p_pb_storm_control->ucast_mode = p_storm_control->ucast_mode;
    p_pb_storm_control->bcast_rate = p_storm_control->bcast_rate;
    p_pb_storm_control->mcast_rate = p_storm_control->mcast_rate;
    p_pb_storm_control->ucast_rate = p_storm_control->ucast_rate;

    return PM_E_NONE;
}

int32
pb_ds_storm_control_to_pb_free_packed(Cdb__DsStormControl *p_pb_storm_control)
{
    if (p_pb_storm_control->parent_if->name)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_storm_control->parent_if->name);
        p_pb_storm_control->parent_if->name = NULL;
    }

    return PM_E_NONE;
}

int32
pb_ds_storm_control_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len)
{
    Cdb__TblInterfaceKey pb_if_key = CDB__TBL_INTERFACE_KEY__INIT;
    Cdb__DsStormControl pb_storm_control = CDB__DS_STORM_CONTROL__INIT;
    tbl_interface_key_t *p_if_key = (tbl_interface_key_t*)p_tbl;
    ds_storm_control_t *p_storm_control = (ds_storm_control_t*)p_ds;
    int32 len = 0;

    pb_storm_control.parent_if = &pb_if_key;
    pb_ds_storm_control_to_pb(p_if_key, p_storm_control, &pb_storm_control);
    len = cdb__ds_storm_control__pack(&pb_storm_control, buf);
    pb_ds_storm_control_to_pb_free_packed(&pb_storm_control);

    return len;
}

int32
pb_ds_storm_control_dump(Cdb__DsStormControl *p_pb_storm_control, char *out)
{
    int32 offset = 0;

    out[0] = CMD_ZERO_CHAR;
    offset += sal_sprintf(out + offset, "/parent_if->name=%s", p_pb_storm_control->parent_if->name);
    offset += sal_sprintf(out + offset, "/bcast_mode=%u", p_pb_storm_control->bcast_mode);
    offset += sal_sprintf(out + offset, "/mcast_mode=%u", p_pb_storm_control->mcast_mode);
    offset += sal_sprintf(out + offset, "/ucast_mode=%u", p_pb_storm_control->ucast_mode);
    offset += sal_sprintf(out + offset, "/bcast_rate=%u", p_pb_storm_control->bcast_rate);
    offset += sal_sprintf(out + offset, "/mcast_rate=%u", p_pb_storm_control->mcast_rate);
    offset += sal_sprintf(out + offset, "/ucast_rate=%u", p_pb_storm_control->ucast_rate);
    offset += sal_sprintf(out + offset, "\n");

    return PM_E_NONE;
}

/* pb functions for TBL_INTERFACE DS_OPENFLOW_IF */
int32
pb_ds_openflow_if_to_pb(tbl_interface_key_t *p_if_key, ds_openflow_if_t *p_openflow, Cdb__DsOpenflowIf *p_pb_openflow)
{
    p_pb_openflow->parent_if->name = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_if_key->name)+1);
    sal_strcpy(p_pb_openflow->parent_if->name, p_if_key->name);

    p_pb_openflow->enable = p_openflow->enable;
    p_pb_openflow->obey_vlan_filter = p_openflow->obey_vlan_filter;

    return PM_E_NONE;
}

int32
pb_ds_openflow_if_to_pb_free_packed(Cdb__DsOpenflowIf *p_pb_openflow)
{
    if (p_pb_openflow->parent_if->name)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_openflow->parent_if->name);
        p_pb_openflow->parent_if->name = NULL;
    }

    return PM_E_NONE;
}

int32
pb_ds_openflow_if_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len)
{
    Cdb__TblInterfaceKey pb_if_key = CDB__TBL_INTERFACE_KEY__INIT;
    Cdb__DsOpenflowIf pb_openflow = CDB__DS_OPENFLOW_IF__INIT;
    tbl_interface_key_t *p_if_key = (tbl_interface_key_t*)p_tbl;
    ds_openflow_if_t *p_openflow = (ds_openflow_if_t*)p_ds;
    int32 len = 0;

    pb_openflow.parent_if = &pb_if_key;
    pb_ds_openflow_if_to_pb(p_if_key, p_openflow, &pb_openflow);
    len = cdb__ds_openflow_if__pack(&pb_openflow, buf);
    pb_ds_openflow_if_to_pb_free_packed(&pb_openflow);

    return len;
}

int32
pb_ds_openflow_if_dump(Cdb__DsOpenflowIf *p_pb_openflow, char *out)
{
    int32 offset = 0;

    out[0] = CMD_ZERO_CHAR;
    offset += sal_sprintf(out + offset, "/parent_if->name=%s", p_pb_openflow->parent_if->name);
    offset += sal_sprintf(out + offset, "/enable=%u", p_pb_openflow->enable);
    offset += sal_sprintf(out + offset, "/obey_vlan_filter=%u", p_pb_openflow->obey_vlan_filter);
    offset += sal_sprintf(out + offset, "\n");

    return PM_E_NONE;
}

/* pb functions for TBL_INTERFACE DS_DHCLIENT_IF */
int32
pb_ds_dhclient_if_to_pb(tbl_interface_key_t *p_if_key, ds_dhclient_if_t *p_dhclient, Cdb__DsDhclientIf *p_pb_dhclient)
{
    p_pb_dhclient->parent_if->name = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_if_key->name)+1);
    sal_strcpy(p_pb_dhclient->parent_if->name, p_if_key->name);

    p_pb_dhclient->requested_flags = p_dhclient->requested_flags;
    p_pb_dhclient->lease = p_dhclient->lease;
    p_pb_dhclient->status = p_dhclient->status;
    p_pb_dhclient->option = p_dhclient->option;
    p_pb_dhclient->client_flags = p_dhclient->client_flags;
    p_pb_dhclient->client_id = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_dhclient->client_id)+1);
    sal_strcpy(p_pb_dhclient->client_id, p_dhclient->client_id);
    p_pb_dhclient->class_flags = p_dhclient->class_flags;
    p_pb_dhclient->class_id = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_dhclient->class_id)+1);
    sal_strcpy(p_pb_dhclient->class_id, p_dhclient->class_id);
    p_pb_dhclient->hostname = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_dhclient->hostname)+1);
    sal_strcpy(p_pb_dhclient->hostname, p_dhclient->hostname);
    p_pb_dhclient->iana_t1 = p_dhclient->iana_t1;
    p_pb_dhclient->iana_t2 = p_dhclient->iana_t2;

    return PM_E_NONE;
}

int32
pb_ds_dhclient_if_to_pb_free_packed(Cdb__DsDhclientIf *p_pb_dhclient)
{
    if (p_pb_dhclient->parent_if->name)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_dhclient->parent_if->name);
        p_pb_dhclient->parent_if->name = NULL;
    }

    if (p_pb_dhclient->client_id)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_dhclient->client_id);
        p_pb_dhclient->client_id = NULL;
    }

    if (p_pb_dhclient->class_id)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_dhclient->class_id);
        p_pb_dhclient->class_id = NULL;
    }

    if (p_pb_dhclient->hostname)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_dhclient->hostname);
        p_pb_dhclient->hostname = NULL;
    }

    return PM_E_NONE;
}

int32
pb_ds_dhclient_if_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len)
{
    Cdb__TblInterfaceKey pb_if_key = CDB__TBL_INTERFACE_KEY__INIT;
    Cdb__DsDhclientIf pb_dhclient = CDB__DS_DHCLIENT_IF__INIT;
    tbl_interface_key_t *p_if_key = (tbl_interface_key_t*)p_tbl;
    ds_dhclient_if_t *p_dhclient = (ds_dhclient_if_t*)p_ds;
    int32 len = 0;

    pb_dhclient.parent_if = &pb_if_key;
    pb_ds_dhclient_if_to_pb(p_if_key, p_dhclient, &pb_dhclient);
    len = cdb__ds_dhclient_if__pack(&pb_dhclient, buf);
    pb_ds_dhclient_if_to_pb_free_packed(&pb_dhclient);

    return len;
}

int32
pb_ds_dhclient_if_dump(Cdb__DsDhclientIf *p_pb_dhclient, char *out)
{
    int32 offset = 0;

    out[0] = CMD_ZERO_CHAR;
    offset += sal_sprintf(out + offset, "/parent_if->name=%s", p_pb_dhclient->parent_if->name);
    offset += sal_sprintf(out + offset, "/requested_flags=%u", p_pb_dhclient->requested_flags);
    offset += sal_sprintf(out + offset, "/lease=%u", p_pb_dhclient->lease);
    offset += sal_sprintf(out + offset, "/status=%u", p_pb_dhclient->status);
    offset += sal_sprintf(out + offset, "/option=%u", p_pb_dhclient->option);
    offset += sal_sprintf(out + offset, "/client_flags=%u", p_pb_dhclient->client_flags);
    offset += sal_sprintf(out + offset, "/client_id=%s", p_pb_dhclient->client_id);
    offset += sal_sprintf(out + offset, "/class_flags=%u", p_pb_dhclient->class_flags);
    offset += sal_sprintf(out + offset, "/class_id=%s", p_pb_dhclient->class_id);
    offset += sal_sprintf(out + offset, "/hostname=%s", p_pb_dhclient->hostname);
    offset += sal_sprintf(out + offset, "/iana_t1=%u", p_pb_dhclient->iana_t1);
    offset += sal_sprintf(out + offset, "/iana_t2=%u", p_pb_dhclient->iana_t2);
    offset += sal_sprintf(out + offset, "\n");

    return PM_E_NONE;
}

/* pb functions for TBL_PVLAN DS_PVLAN_COMMUNITY */
int32
pb_ds_pvlan_community_to_pb(tbl_pvlan_key_t *p_pvlan_key, ds_pvlan_community_t *p_community, Cdb__DsPvlanCommunity *p_pb_community)
{
    p_pb_community->parent_pvlan->primary_vid = p_pvlan_key->primary_vid;

    p_pb_community->key->community_vid = p_community->key.community_vid;

    p_pb_community->group_id = p_community->group_id;

    return PM_E_NONE;
}

int32
pb_ds_pvlan_community_to_pb_free_packed(Cdb__DsPvlanCommunity *p_pb_community)
{
    return PM_E_NONE;
}

int32
pb_ds_pvlan_community_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len)
{
    Cdb__TblPvlanKey pb_pvlan_key = CDB__TBL_PVLAN_KEY__INIT;
    Cdb__DsPvlanCommunityKey pb_community_key = CDB__DS_PVLAN_COMMUNITY_KEY__INIT;
    Cdb__DsPvlanCommunity pb_community = CDB__DS_PVLAN_COMMUNITY__INIT;
    tbl_pvlan_key_t *p_pvlan_key = (tbl_pvlan_key_t*)p_tbl;
    ds_pvlan_community_t *p_community = (ds_pvlan_community_t*)p_ds;
    int32 len = 0;

    pb_community.key = &pb_community_key;
    pb_community.parent_pvlan = &pb_pvlan_key;
    pb_ds_pvlan_community_to_pb(p_pvlan_key, p_community, &pb_community);
    len = cdb__ds_pvlan_community__pack(&pb_community, buf);
    pb_ds_pvlan_community_to_pb_free_packed(&pb_community);

    return len;
}

int32
pb_ds_pvlan_community_dump(Cdb__DsPvlanCommunity *p_pb_community, char *out)
{
    int32 offset = 0;

    out[0] = CMD_ZERO_CHAR;
    offset += sal_sprintf(out + offset, "/parent_pvlan->primary_vid=%u", p_pb_community->parent_pvlan->primary_vid);
    offset += sal_sprintf(out + offset, "/key->community_vid=%u", p_pb_community->key->community_vid);
    offset += sal_sprintf(out + offset, "/group_id=%u", p_pb_community->group_id);
    offset += sal_sprintf(out + offset, "\n");

    return PM_E_NONE;
}

/* pb functions for TBL_INTERFACE DS_CIRCUIT_ID */
int32
pb_ds_circuit_id_to_pb(tbl_interface_key_t *p_if_key, ds_circuit_id_t *p_circuit_id, Cdb__DsCircuitId *p_pb_circuit_id)
{
    p_pb_circuit_id->parent_if->name = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_if_key->name)+1);
    sal_strcpy(p_pb_circuit_id->parent_if->name, p_if_key->name);

    p_pb_circuit_id->key->vid = p_circuit_id->key.vid;

    p_pb_circuit_id->circuit = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_circuit_id->circuit)+1);
    sal_strcpy(p_pb_circuit_id->circuit, p_circuit_id->circuit);

    return PM_E_NONE;
}

int32
pb_ds_circuit_id_to_pb_free_packed(Cdb__DsCircuitId *p_pb_circuit_id)
{
    if (p_pb_circuit_id->parent_if->name)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_circuit_id->parent_if->name);
        p_pb_circuit_id->parent_if->name = NULL;
    }

    if (p_pb_circuit_id->circuit)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_circuit_id->circuit);
        p_pb_circuit_id->circuit = NULL;
    }

    return PM_E_NONE;
}

int32
pb_ds_circuit_id_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len)
{
    Cdb__TblInterfaceKey pb_if_key = CDB__TBL_INTERFACE_KEY__INIT;
    Cdb__DsCircuitIdKey pb_circuit_id_key = CDB__DS_CIRCUIT_ID_KEY__INIT;
    Cdb__DsCircuitId pb_circuit_id = CDB__DS_CIRCUIT_ID__INIT;
    tbl_interface_key_t *p_if_key = (tbl_interface_key_t*)p_tbl;
    ds_circuit_id_t *p_circuit_id = (ds_circuit_id_t*)p_ds;
    int32 len = 0;

    pb_circuit_id.key = &pb_circuit_id_key;
    pb_circuit_id.parent_if = &pb_if_key;
    pb_ds_circuit_id_to_pb(p_if_key, p_circuit_id, &pb_circuit_id);
    len = cdb__ds_circuit_id__pack(&pb_circuit_id, buf);
    pb_ds_circuit_id_to_pb_free_packed(&pb_circuit_id);

    return len;
}

int32
pb_ds_circuit_id_dump(Cdb__DsCircuitId *p_pb_circuit_id, char *out)
{
    int32 offset = 0;

    out[0] = CMD_ZERO_CHAR;
    offset += sal_sprintf(out + offset, "/parent_if->name=%s", p_pb_circuit_id->parent_if->name);
    offset += sal_sprintf(out + offset, "/key->vid=%u", p_pb_circuit_id->key->vid);
    offset += sal_sprintf(out + offset, "/circuit=%s", p_pb_circuit_id->circuit);
    offset += sal_sprintf(out + offset, "\n");

    return PM_E_NONE;
}

/* pb functions for TBL_L2_ACTION DS_FLUSH_FDB */
int32
pb_ds_flush_fdb_to_pb(ds_flush_fdb_t *p_flush_fdb, Cdb__DsFlushFdb *p_pb_flush_fdb)
{
    p_pb_flush_fdb->type = p_flush_fdb->type;
    p_pb_flush_fdb->subtype = p_flush_fdb->subtype;
    p_pb_flush_fdb->mode = p_flush_fdb->mode;
    p_pb_flush_fdb->ifindex = p_flush_fdb->ifindex;
    p_pb_flush_fdb->vid = p_flush_fdb->vid;
    pb_compose_mac_addr_t_to_pb(p_flush_fdb->mac, p_pb_flush_fdb->mac);

    return PM_E_NONE;
}

int32
pb_ds_flush_fdb_to_pb_free_packed(Cdb__DsFlushFdb *p_pb_flush_fdb)
{
    pb_compose_mac_addr_t_to_pb_free_packed(p_pb_flush_fdb->mac);
    return PM_E_NONE;
}

int32
pb_ds_flush_fdb_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len)
{
    Cdb__DsFlushFdb pb_flush_fdb = CDB__DS_FLUSH_FDB__INIT;
    ds_flush_fdb_t *p_flush_fdb = (ds_flush_fdb_t*)p_ds;
    int32 len = 0;
    Cdb__ComposeMacAddrT mac = CDB__COMPOSE_MAC_ADDR_T__INIT;

    pb_flush_fdb.mac = &mac;
    pb_ds_flush_fdb_to_pb(p_flush_fdb, &pb_flush_fdb);
    len = cdb__ds_flush_fdb__pack(&pb_flush_fdb, buf);
    pb_ds_flush_fdb_to_pb_free_packed(&pb_flush_fdb);

    return len;
}

int32
pb_ds_flush_fdb_dump(Cdb__DsFlushFdb *p_pb_flush_fdb, char *out)
{
    int32 offset = 0;

    out[0] = CMD_ZERO_CHAR;
    offset += sal_sprintf(out + offset, "/type=%u", p_pb_flush_fdb->type);
    offset += sal_sprintf(out + offset, "/subtype=%u", p_pb_flush_fdb->subtype);
    offset += sal_sprintf(out + offset, "/mode=%u", p_pb_flush_fdb->mode);
    offset += sal_sprintf(out + offset, "/ifindex=%u", p_pb_flush_fdb->ifindex);
    offset += sal_sprintf(out + offset, "/vid=%u", p_pb_flush_fdb->vid);
    offset += pb_compose_mac_addr_t_dump(p_pb_flush_fdb->mac, (out + offset));
    offset += sal_sprintf(out + offset, "\n");

    return PM_E_NONE;
}

/* pb functions for TBL_LLDP_IF DS_LLDP_NEIGHBOUR */
int32
pb_ds_lldp_neighbour_to_pb(tbl_lldp_if_key_t *p_lldp_if_key, ds_lldp_neighbour_t *p_neigh, Cdb__DsLldpNeighbour *p_pb_neigh)
{
    p_pb_neigh->parent_lldp_if->ifindex = p_lldp_if_key->ifindex;

    pb_compose_lldp_msap_id_t_to_pb(&p_neigh->key, p_pb_neigh->key);

    p_pb_neigh->rx_ifindex = p_neigh->rx_ifindex;
    pb_compose_mac_addr_t_to_pb(p_neigh->mac_addr, p_pb_neigh->mac_addr);
    p_pb_neigh->rx_ttl = p_neigh->rx_ttl;
    p_pb_neigh->chassis_id_sub_type = p_neigh->chassis_id_sub_type;
    p_pb_neigh->port_id_sub_type = p_neigh->port_id_sub_type;
    p_pb_neigh->system_name = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_neigh->system_name)+1);
    sal_strcpy(p_pb_neigh->system_name, p_neigh->system_name);
    p_pb_neigh->system_desc = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_neigh->system_desc)+1);
    sal_strcpy(p_pb_neigh->system_desc, p_neigh->system_desc);
    p_pb_neigh->port_desc = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_neigh->port_desc)+1);
    sal_strcpy(p_pb_neigh->port_desc, p_neigh->port_desc);
    p_pb_neigh->ifname = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_neigh->ifname)+1);
    sal_strcpy(p_pb_neigh->ifname, p_neigh->ifname);
    p_pb_neigh->vlan_name = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_neigh->vlan_name)+1);
    sal_strcpy(p_pb_neigh->vlan_name, p_neigh->vlan_name);
    p_pb_neigh->vlan_id = p_neigh->vlan_id;
    p_pb_neigh->sys_cap = p_neigh->sys_cap;
    p_pb_neigh->sys_cap_en = p_neigh->sys_cap_en;
    p_pb_neigh->mant_addr = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_neigh->mant_addr)+1);
    sal_strcpy(p_pb_neigh->mant_addr, p_neigh->mant_addr);
    p_pb_neigh->oid = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_neigh->oid)+1);
    sal_strcpy(p_pb_neigh->oid, p_neigh->oid);
    p_pb_neigh->oid_len = p_neigh->oid_len;
    p_pb_neigh->mgmt_addr_sub_type = p_neigh->mgmt_addr_sub_type;
    p_pb_neigh->if_numbering = p_neigh->if_numbering;
    p_pb_neigh->if_number = p_neigh->if_number;
    p_pb_neigh->pvid = p_neigh->pvid;
    p_pb_neigh->ppvid_flag = p_neigh->ppvid_flag;
    p_pb_neigh->ppvid = p_neigh->ppvid;
    p_pb_neigh->protocol_id = p_neigh->protocol_id;
    p_pb_neigh->autonego_support = p_neigh->autonego_support;
    p_pb_neigh->autonego_cap = p_neigh->autonego_cap;
    p_pb_neigh->oper_mau_type = p_neigh->oper_mau_type;
    p_pb_neigh->link_aggr_status = p_neigh->link_aggr_status;
    p_pb_neigh->link_aggr_id = p_neigh->link_aggr_id;
    p_pb_neigh->max_frame_size = p_neigh->max_frame_size;
    p_pb_neigh->power_flag = p_neigh->power_flag;
    p_pb_neigh->power_pair = p_neigh->power_pair;
    p_pb_neigh->power_class = p_neigh->power_class;
    p_pb_neigh->med_capbility = p_neigh->med_capbility;
    p_pb_neigh->med_dev_type = p_neigh->med_dev_type;
    p_pb_neigh->med_policy_flag = p_neigh->med_policy_flag;
    p_pb_neigh->med_power_flag = p_neigh->med_power_flag;
    p_pb_neigh->med_power_value = p_neigh->med_power_value;
    p_pb_neigh->med_lci_format = p_neigh->med_lci_format;
    p_pb_neigh->med_hard_ver = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_neigh->med_hard_ver)+1);
    sal_strcpy(p_pb_neigh->med_hard_ver, p_neigh->med_hard_ver);
    p_pb_neigh->med_firm_ver = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_neigh->med_firm_ver)+1);
    sal_strcpy(p_pb_neigh->med_firm_ver, p_neigh->med_firm_ver);
    p_pb_neigh->med_soft_ver = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_neigh->med_soft_ver)+1);
    sal_strcpy(p_pb_neigh->med_soft_ver, p_neigh->med_soft_ver);
    p_pb_neigh->med_serial_ver = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_neigh->med_serial_ver)+1);
    sal_strcpy(p_pb_neigh->med_serial_ver, p_neigh->med_serial_ver);
    p_pb_neigh->med_manufac_ver = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_neigh->med_manufac_ver)+1);
    sal_strcpy(p_pb_neigh->med_manufac_ver, p_neigh->med_manufac_ver);
    p_pb_neigh->med_moname_ver = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_neigh->med_moname_ver)+1);
    sal_strcpy(p_pb_neigh->med_moname_ver, p_neigh->med_moname_ver);
    p_pb_neigh->med_asset_ver = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_neigh->med_asset_ver)+1);
    sal_strcpy(p_pb_neigh->med_asset_ver, p_neigh->med_asset_ver);
    p_pb_neigh->med_lci_coordinate = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_neigh->med_lci_coordinate)+1);
    sal_strcpy(p_pb_neigh->med_lci_coordinate, p_neigh->med_lci_coordinate);
    p_pb_neigh->med_lci_elin = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_neigh->med_lci_elin)+1);
    sal_strcpy(p_pb_neigh->med_lci_elin, p_neigh->med_lci_elin);
    pb_compose_lldp_civic_address_t_to_pb(&p_neigh->med_lci_civic, p_pb_neigh->med_lci_civic);

    return PM_E_NONE;
}

int32
pb_ds_lldp_neighbour_to_pb_free_packed(Cdb__DsLldpNeighbour *p_pb_neigh)
{
    pb_compose_lldp_msap_id_t_to_pb_free_packed(p_pb_neigh->key);
    pb_compose_mac_addr_t_to_pb_free_packed(p_pb_neigh->mac_addr);
    if (p_pb_neigh->system_name)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_neigh->system_name);
        p_pb_neigh->system_name = NULL;
    }

    if (p_pb_neigh->system_desc)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_neigh->system_desc);
        p_pb_neigh->system_desc = NULL;
    }

    if (p_pb_neigh->port_desc)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_neigh->port_desc);
        p_pb_neigh->port_desc = NULL;
    }

    if (p_pb_neigh->ifname)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_neigh->ifname);
        p_pb_neigh->ifname = NULL;
    }

    if (p_pb_neigh->vlan_name)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_neigh->vlan_name);
        p_pb_neigh->vlan_name = NULL;
    }

    if (p_pb_neigh->mant_addr)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_neigh->mant_addr);
        p_pb_neigh->mant_addr = NULL;
    }

    if (p_pb_neigh->oid)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_neigh->oid);
        p_pb_neigh->oid = NULL;
    }

    if (p_pb_neigh->med_hard_ver)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_neigh->med_hard_ver);
        p_pb_neigh->med_hard_ver = NULL;
    }

    if (p_pb_neigh->med_firm_ver)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_neigh->med_firm_ver);
        p_pb_neigh->med_firm_ver = NULL;
    }

    if (p_pb_neigh->med_soft_ver)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_neigh->med_soft_ver);
        p_pb_neigh->med_soft_ver = NULL;
    }

    if (p_pb_neigh->med_serial_ver)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_neigh->med_serial_ver);
        p_pb_neigh->med_serial_ver = NULL;
    }

    if (p_pb_neigh->med_manufac_ver)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_neigh->med_manufac_ver);
        p_pb_neigh->med_manufac_ver = NULL;
    }

    if (p_pb_neigh->med_moname_ver)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_neigh->med_moname_ver);
        p_pb_neigh->med_moname_ver = NULL;
    }

    if (p_pb_neigh->med_asset_ver)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_neigh->med_asset_ver);
        p_pb_neigh->med_asset_ver = NULL;
    }

    if (p_pb_neigh->med_lci_coordinate)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_neigh->med_lci_coordinate);
        p_pb_neigh->med_lci_coordinate = NULL;
    }

    if (p_pb_neigh->med_lci_elin)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_neigh->med_lci_elin);
        p_pb_neigh->med_lci_elin = NULL;
    }

    pb_compose_lldp_civic_address_t_to_pb_free_packed(p_pb_neigh->med_lci_civic);
    return PM_E_NONE;
}

int32
pb_ds_lldp_neighbour_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len)
{
    Cdb__TblLldpIfKey pb_lldp_if_key = CDB__TBL_LLDP_IF_KEY__INIT;
    Cdb__DsLldpNeighbourKey pb_neigh_key = CDB__DS_LLDP_NEIGHBOUR_KEY__INIT;
    Cdb__DsLldpNeighbour pb_neigh = CDB__DS_LLDP_NEIGHBOUR__INIT;
    tbl_lldp_if_key_t *p_lldp_if_key = (tbl_lldp_if_key_t*)p_tbl;
    ds_lldp_neighbour_t *p_neigh = (ds_lldp_neighbour_t*)p_ds;
    int32 len = 0;
    Cdb__ComposeMacAddrT mac_addr = CDB__COMPOSE_MAC_ADDR_T__INIT;
    Cdb__ComposeLldpCivicAddressT med_lci_civic = CDB__COMPOSE_LLDP_CIVIC_ADDRESS_T__INIT;

    pb_neigh.key = &pb_neigh_key;
    pb_neigh.mac_addr = &mac_addr;
    pb_neigh.med_lci_civic = &med_lci_civic;
    pb_neigh.parent_lldp_if = &pb_lldp_if_key;
    pb_ds_lldp_neighbour_to_pb(p_lldp_if_key, p_neigh, &pb_neigh);
    len = cdb__ds_lldp_neighbour__pack(&pb_neigh, buf);
    pb_ds_lldp_neighbour_to_pb_free_packed(&pb_neigh);

    return len;
}

int32
pb_ds_lldp_neighbour_dump(Cdb__DsLldpNeighbour *p_pb_neigh, char *out)
{
    int32 offset = 0;

    out[0] = CMD_ZERO_CHAR;
    offset += sal_sprintf(out + offset, "/parent_lldp_if->ifindex=%u", p_pb_neigh->parent_lldp_if->ifindex);
    offset += pb_compose_lldp_msap_id_t_dump(p_pb_neigh->key, (out + offset));
    offset += sal_sprintf(out + offset, "/rx_ifindex=%u", p_pb_neigh->rx_ifindex);
    offset += pb_compose_mac_addr_t_dump(p_pb_neigh->mac_addr, (out + offset));
    offset += sal_sprintf(out + offset, "/rx_ttl=%u", p_pb_neigh->rx_ttl);
    offset += sal_sprintf(out + offset, "/chassis_id_sub_type=%u", p_pb_neigh->chassis_id_sub_type);
    offset += sal_sprintf(out + offset, "/port_id_sub_type=%u", p_pb_neigh->port_id_sub_type);
    offset += sal_sprintf(out + offset, "/system_name=%s", p_pb_neigh->system_name);
    offset += sal_sprintf(out + offset, "/system_desc=%s", p_pb_neigh->system_desc);
    offset += sal_sprintf(out + offset, "/port_desc=%s", p_pb_neigh->port_desc);
    offset += sal_sprintf(out + offset, "/ifname=%s", p_pb_neigh->ifname);
    offset += sal_sprintf(out + offset, "/vlan_name=%s", p_pb_neigh->vlan_name);
    offset += sal_sprintf(out + offset, "/vlan_id=%u", p_pb_neigh->vlan_id);
    offset += sal_sprintf(out + offset, "/sys_cap=%u", p_pb_neigh->sys_cap);
    offset += sal_sprintf(out + offset, "/sys_cap_en=%u", p_pb_neigh->sys_cap_en);
    offset += sal_sprintf(out + offset, "/mant_addr=%s", p_pb_neigh->mant_addr);
    offset += sal_sprintf(out + offset, "/oid=%s", p_pb_neigh->oid);
    offset += sal_sprintf(out + offset, "/oid_len=%u", p_pb_neigh->oid_len);
    offset += sal_sprintf(out + offset, "/mgmt_addr_sub_type=%u", p_pb_neigh->mgmt_addr_sub_type);
    offset += sal_sprintf(out + offset, "/if_numbering=%u", p_pb_neigh->if_numbering);
    offset += sal_sprintf(out + offset, "/if_number=%u", p_pb_neigh->if_number);
    offset += sal_sprintf(out + offset, "/pvid=%u", p_pb_neigh->pvid);
    offset += sal_sprintf(out + offset, "/ppvid_flag=%u", p_pb_neigh->ppvid_flag);
    offset += sal_sprintf(out + offset, "/ppvid=%u", p_pb_neigh->ppvid);
    offset += sal_sprintf(out + offset, "/protocol_id=%u", p_pb_neigh->protocol_id);
    offset += sal_sprintf(out + offset, "/autonego_support=%u", p_pb_neigh->autonego_support);
    offset += sal_sprintf(out + offset, "/autonego_cap=%u", p_pb_neigh->autonego_cap);
    offset += sal_sprintf(out + offset, "/oper_mau_type=%u", p_pb_neigh->oper_mau_type);
    offset += sal_sprintf(out + offset, "/link_aggr_status=%u", p_pb_neigh->link_aggr_status);
    offset += sal_sprintf(out + offset, "/link_aggr_id=%u", p_pb_neigh->link_aggr_id);
    offset += sal_sprintf(out + offset, "/max_frame_size=%u", p_pb_neigh->max_frame_size);
    offset += sal_sprintf(out + offset, "/power_flag=%u", p_pb_neigh->power_flag);
    offset += sal_sprintf(out + offset, "/power_pair=%u", p_pb_neigh->power_pair);
    offset += sal_sprintf(out + offset, "/power_class=%u", p_pb_neigh->power_class);
    offset += sal_sprintf(out + offset, "/med_capbility=%u", p_pb_neigh->med_capbility);
    offset += sal_sprintf(out + offset, "/med_dev_type=%u", p_pb_neigh->med_dev_type);
    offset += sal_sprintf(out + offset, "/med_policy_flag=%u", p_pb_neigh->med_policy_flag);
    offset += sal_sprintf(out + offset, "/med_power_flag=%u", p_pb_neigh->med_power_flag);
    offset += sal_sprintf(out + offset, "/med_power_value=%u", p_pb_neigh->med_power_value);
    offset += sal_sprintf(out + offset, "/med_lci_format=%u", p_pb_neigh->med_lci_format);
    offset += sal_sprintf(out + offset, "/med_hard_ver=%s", p_pb_neigh->med_hard_ver);
    offset += sal_sprintf(out + offset, "/med_firm_ver=%s", p_pb_neigh->med_firm_ver);
    offset += sal_sprintf(out + offset, "/med_soft_ver=%s", p_pb_neigh->med_soft_ver);
    offset += sal_sprintf(out + offset, "/med_serial_ver=%s", p_pb_neigh->med_serial_ver);
    offset += sal_sprintf(out + offset, "/med_manufac_ver=%s", p_pb_neigh->med_manufac_ver);
    offset += sal_sprintf(out + offset, "/med_moname_ver=%s", p_pb_neigh->med_moname_ver);
    offset += sal_sprintf(out + offset, "/med_asset_ver=%s", p_pb_neigh->med_asset_ver);
    offset += sal_sprintf(out + offset, "/med_lci_coordinate=%s", p_pb_neigh->med_lci_coordinate);
    offset += sal_sprintf(out + offset, "/med_lci_elin=%s", p_pb_neigh->med_lci_elin);
    offset += pb_compose_lldp_civic_address_t_dump(p_pb_neigh->med_lci_civic, (out + offset));
    offset += sal_sprintf(out + offset, "\n");

    return PM_E_NONE;
}

/* pb functions for TBL_IGSP_INTF DS_MROUTER_PORT */
int32
pb_ds_mrouter_port_to_pb(tbl_igsp_intf_key_t *p_if_key, ds_mrouter_port_t *p_mr_port, Cdb__DsMrouterPort *p_pb_mr_port)
{
    p_pb_mr_port->parent_if->vid = p_if_key->vid;

    p_pb_mr_port->key->name = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_mr_port->key.name)+1);
    sal_strcpy(p_pb_mr_port->key->name, p_mr_port->key.name);

    p_pb_mr_port->type = p_mr_port->type;
    pb_compose_sal_time_t_to_pb(&p_mr_port->uptime, p_pb_mr_port->uptime);

    return PM_E_NONE;
}

int32
pb_ds_mrouter_port_to_pb_free_packed(Cdb__DsMrouterPort *p_pb_mr_port)
{
    if (p_pb_mr_port->key->name)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_mr_port->key->name);
        p_pb_mr_port->key->name = NULL;
    }

    pb_compose_sal_time_t_to_pb_free_packed(p_pb_mr_port->uptime);
    return PM_E_NONE;
}

int32
pb_ds_mrouter_port_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len)
{
    Cdb__TblIgspIntfKey pb_if_key = CDB__TBL_IGSP_INTF_KEY__INIT;
    Cdb__DsMrouterPortKey pb_mr_port_key = CDB__DS_MROUTER_PORT_KEY__INIT;
    Cdb__DsMrouterPort pb_mr_port = CDB__DS_MROUTER_PORT__INIT;
    tbl_igsp_intf_key_t *p_if_key = (tbl_igsp_intf_key_t*)p_tbl;
    ds_mrouter_port_t *p_mr_port = (ds_mrouter_port_t*)p_ds;
    int32 len = 0;
    Cdb__ComposeSalTimeT uptime = CDB__COMPOSE_SAL_TIME_T__INIT;

    pb_mr_port.key = &pb_mr_port_key;
    pb_mr_port.uptime = &uptime;
    pb_mr_port.parent_if = &pb_if_key;
    pb_ds_mrouter_port_to_pb(p_if_key, p_mr_port, &pb_mr_port);
    len = cdb__ds_mrouter_port__pack(&pb_mr_port, buf);
    pb_ds_mrouter_port_to_pb_free_packed(&pb_mr_port);

    return len;
}

int32
pb_ds_mrouter_port_dump(Cdb__DsMrouterPort *p_pb_mr_port, char *out)
{
    int32 offset = 0;

    out[0] = CMD_ZERO_CHAR;
    offset += sal_sprintf(out + offset, "/parent_if->vid=%u", p_pb_mr_port->parent_if->vid);
    offset += sal_sprintf(out + offset, "/key->name=%s", p_pb_mr_port->key->name);
    offset += sal_sprintf(out + offset, "/type=%u", p_pb_mr_port->type);
    offset += pb_compose_sal_time_t_dump(p_pb_mr_port->uptime, (out + offset));
    offset += sal_sprintf(out + offset, "\n");

    return PM_E_NONE;
}

/* pb functions for TBL_IGSP_INTF DS_QUERY */
int32
pb_ds_query_to_pb(tbl_igsp_intf_key_t *p_if_key, ds_query_t *p_query, Cdb__DsQuery *p_pb_query)
{
    p_pb_query->parent_if->vid = p_if_key->vid;

    pb_compose_addr_ipv4_t_to_pb(&p_query->key.group, p_pb_query->key->group);

    p_pb_query->max_resp_time = p_query->max_resp_time;
    p_pb_query->vid = p_query->vid;

    return PM_E_NONE;
}

int32
pb_ds_query_to_pb_free_packed(Cdb__DsQuery *p_pb_query)
{
    pb_compose_addr_ipv4_t_to_pb_free_packed(p_pb_query->key->group);
    return PM_E_NONE;
}

int32
pb_ds_query_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len)
{
    Cdb__TblIgspIntfKey pb_if_key = CDB__TBL_IGSP_INTF_KEY__INIT;
    Cdb__ComposeAddrIpv4T group = CDB__COMPOSE_ADDR_IPV4_T__INIT;
    Cdb__DsQueryKey pb_query_key = CDB__DS_QUERY_KEY__INIT;
    Cdb__DsQuery pb_query = CDB__DS_QUERY__INIT;
    tbl_igsp_intf_key_t *p_if_key = (tbl_igsp_intf_key_t*)p_tbl;
    ds_query_t *p_query = (ds_query_t*)p_ds;
    int32 len = 0;

    pb_query_key.group = &group;
    pb_query.key = &pb_query_key;
    pb_query.parent_if = &pb_if_key;
    pb_ds_query_to_pb(p_if_key, p_query, &pb_query);
    len = cdb__ds_query__pack(&pb_query, buf);
    pb_ds_query_to_pb_free_packed(&pb_query);

    return len;
}

int32
pb_ds_query_dump(Cdb__DsQuery *p_pb_query, char *out)
{
    int32 offset = 0;

    out[0] = CMD_ZERO_CHAR;
    offset += sal_sprintf(out + offset, "/parent_if->vid=%u", p_pb_query->parent_if->vid);
    offset += pb_compose_addr_ipv4_t_dump(p_pb_query->key->group, (out + offset));
    offset += sal_sprintf(out + offset, "/max_resp_time=%u", p_pb_query->max_resp_time);
    offset += sal_sprintf(out + offset, "/vid=%u", p_pb_query->vid);
    offset += sal_sprintf(out + offset, "\n");

    return PM_E_NONE;
}

/* pb functions for TBL_INTERFACE DS_L2PROTOCOL */
int32
pb_ds_l2protocol_to_pb(tbl_interface_key_t *p_if_key, ds_l2protocol_t *p_l2protocol, Cdb__DsL2protocol *p_pb_l2protocol)
{
    p_pb_l2protocol->parent_if->name = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_if_key->name)+1);
    sal_strcpy(p_pb_l2protocol->parent_if->name, p_if_key->name);

    p_pb_l2protocol->key->l2pro_name = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_l2protocol->key.l2pro_name)+1);
    sal_strcpy(p_pb_l2protocol->key->l2pro_name, p_l2protocol->key.l2pro_name);

    p_pb_l2protocol->is_standard_pro = p_l2protocol->is_standard_pro;
    p_pb_l2protocol->l2pro_action = p_l2protocol->l2pro_action;
    p_pb_l2protocol->l2pro_vlan = p_l2protocol->l2pro_vlan;
    p_pb_l2protocol->l2pro_evc_name = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_l2protocol->l2pro_evc_name)+1);
    sal_strcpy(p_pb_l2protocol->l2pro_evc_name, p_l2protocol->l2pro_evc_name);

    return PM_E_NONE;
}

int32
pb_ds_l2protocol_to_pb_free_packed(Cdb__DsL2protocol *p_pb_l2protocol)
{
    if (p_pb_l2protocol->parent_if->name)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_l2protocol->parent_if->name);
        p_pb_l2protocol->parent_if->name = NULL;
    }

    if (p_pb_l2protocol->key->l2pro_name)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_l2protocol->key->l2pro_name);
        p_pb_l2protocol->key->l2pro_name = NULL;
    }

    if (p_pb_l2protocol->l2pro_evc_name)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_l2protocol->l2pro_evc_name);
        p_pb_l2protocol->l2pro_evc_name = NULL;
    }

    return PM_E_NONE;
}

int32
pb_ds_l2protocol_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len)
{
    Cdb__TblInterfaceKey pb_if_key = CDB__TBL_INTERFACE_KEY__INIT;
    Cdb__DsL2protocolKey pb_l2protocol_key = CDB__DS_L2PROTOCOL_KEY__INIT;
    Cdb__DsL2protocol pb_l2protocol = CDB__DS_L2PROTOCOL__INIT;
    tbl_interface_key_t *p_if_key = (tbl_interface_key_t*)p_tbl;
    ds_l2protocol_t *p_l2protocol = (ds_l2protocol_t*)p_ds;
    int32 len = 0;

    pb_l2protocol.key = &pb_l2protocol_key;
    pb_l2protocol.parent_if = &pb_if_key;
    pb_ds_l2protocol_to_pb(p_if_key, p_l2protocol, &pb_l2protocol);
    len = cdb__ds_l2protocol__pack(&pb_l2protocol, buf);
    pb_ds_l2protocol_to_pb_free_packed(&pb_l2protocol);

    return len;
}

int32
pb_ds_l2protocol_dump(Cdb__DsL2protocol *p_pb_l2protocol, char *out)
{
    int32 offset = 0;

    out[0] = CMD_ZERO_CHAR;
    offset += sal_sprintf(out + offset, "/parent_if->name=%s", p_pb_l2protocol->parent_if->name);
    offset += sal_sprintf(out + offset, "/key->l2pro_name=%s", p_pb_l2protocol->key->l2pro_name);
    offset += sal_sprintf(out + offset, "/is_standard_pro=%u", p_pb_l2protocol->is_standard_pro);
    offset += sal_sprintf(out + offset, "/l2pro_action=%u", p_pb_l2protocol->l2pro_action);
    offset += sal_sprintf(out + offset, "/l2pro_vlan=%u", p_pb_l2protocol->l2pro_vlan);
    offset += sal_sprintf(out + offset, "/l2pro_evc_name=%s", p_pb_l2protocol->l2pro_evc_name);
    offset += sal_sprintf(out + offset, "\n");

    return PM_E_NONE;
}

/* pb functions for TBL_ROUTE_IF DS_ND_PREFIX */
int32
pb_ds_nd_prefix_to_pb(tbl_route_if_key_t *p_rtif_key, ds_nd_prefix_t *p_nd_prefix, Cdb__DsNdPrefix *p_pb_nd_prefix)
{
    p_pb_nd_prefix->parent_rtif->name = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_rtif_key->name)+1);
    sal_strcpy(p_pb_nd_prefix->parent_rtif->name, p_rtif_key->name);

    pb_compose_nd_prefix_key_t_to_pb(&p_nd_prefix->key, p_pb_nd_prefix->key);

    p_pb_nd_prefix->config = p_nd_prefix->config;
    p_pb_nd_prefix->vlifetime = p_nd_prefix->vlifetime;
    p_pb_nd_prefix->plifetime = p_nd_prefix->plifetime;
    p_pb_nd_prefix->onlink = p_nd_prefix->onlink;
    p_pb_nd_prefix->autoconfig = p_nd_prefix->autoconfig;

    return PM_E_NONE;
}

int32
pb_ds_nd_prefix_to_pb_free_packed(Cdb__DsNdPrefix *p_pb_nd_prefix)
{
    if (p_pb_nd_prefix->parent_rtif->name)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_nd_prefix->parent_rtif->name);
        p_pb_nd_prefix->parent_rtif->name = NULL;
    }

    pb_compose_nd_prefix_key_t_to_pb_free_packed(p_pb_nd_prefix->key);
    return PM_E_NONE;
}

int32
pb_ds_nd_prefix_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len)
{
    Cdb__TblRouteIfKey pb_rtif_key = CDB__TBL_ROUTE_IF_KEY__INIT;
    Cdb__DsNdPrefixKey pb_nd_prefix_key = CDB__DS_ND_PREFIX_KEY__INIT;
    Cdb__DsNdPrefix pb_nd_prefix = CDB__DS_ND_PREFIX__INIT;
    tbl_route_if_key_t *p_rtif_key = (tbl_route_if_key_t*)p_tbl;
    ds_nd_prefix_t *p_nd_prefix = (ds_nd_prefix_t*)p_ds;
    int32 len = 0;

    pb_nd_prefix.key = &pb_nd_prefix_key;
    pb_nd_prefix.parent_rtif = &pb_rtif_key;
    pb_ds_nd_prefix_to_pb(p_rtif_key, p_nd_prefix, &pb_nd_prefix);
    len = cdb__ds_nd_prefix__pack(&pb_nd_prefix, buf);
    pb_ds_nd_prefix_to_pb_free_packed(&pb_nd_prefix);

    return len;
}

int32
pb_ds_nd_prefix_dump(Cdb__DsNdPrefix *p_pb_nd_prefix, char *out)
{
    int32 offset = 0;

    out[0] = CMD_ZERO_CHAR;
    offset += sal_sprintf(out + offset, "/parent_rtif->name=%s", p_pb_nd_prefix->parent_rtif->name);
    offset += pb_compose_nd_prefix_key_t_dump(p_pb_nd_prefix->key, (out + offset));
    offset += sal_sprintf(out + offset, "/config=%u", p_pb_nd_prefix->config);
    offset += sal_sprintf(out + offset, "/vlifetime=%u", p_pb_nd_prefix->vlifetime);
    offset += sal_sprintf(out + offset, "/plifetime=%u", p_pb_nd_prefix->plifetime);
    offset += sal_sprintf(out + offset, "/onlink=%u", p_pb_nd_prefix->onlink);
    offset += sal_sprintf(out + offset, "/autoconfig=%u", p_pb_nd_prefix->autoconfig);
    offset += sal_sprintf(out + offset, "\n");

    return PM_E_NONE;
}

/* pb functions for TBL_IGSP_INTF DS_JOIN_GROUP */
int32
pb_ds_join_group_to_pb(tbl_igsp_intf_key_t *p_if_key, ds_join_group_t *p_join, Cdb__DsJoinGroup *p_pb_join)
{
    p_pb_join->parent_if->vid = p_if_key->vid;

    pb_compose_addr_ipv4_t_to_pb(&p_join->key.group, p_pb_join->key->group);

    p_pb_join->vid = p_join->vid;
    p_pb_join->type = p_join->type;

    return PM_E_NONE;
}

int32
pb_ds_join_group_to_pb_free_packed(Cdb__DsJoinGroup *p_pb_join)
{
    pb_compose_addr_ipv4_t_to_pb_free_packed(p_pb_join->key->group);
    return PM_E_NONE;
}

int32
pb_ds_join_group_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len)
{
    Cdb__TblIgspIntfKey pb_if_key = CDB__TBL_IGSP_INTF_KEY__INIT;
    Cdb__ComposeAddrIpv4T group = CDB__COMPOSE_ADDR_IPV4_T__INIT;
    Cdb__DsJoinGroupKey pb_join_key = CDB__DS_JOIN_GROUP_KEY__INIT;
    Cdb__DsJoinGroup pb_join = CDB__DS_JOIN_GROUP__INIT;
    tbl_igsp_intf_key_t *p_if_key = (tbl_igsp_intf_key_t*)p_tbl;
    ds_join_group_t *p_join = (ds_join_group_t*)p_ds;
    int32 len = 0;

    pb_join_key.group = &group;
    pb_join.key = &pb_join_key;
    pb_join.parent_if = &pb_if_key;
    pb_ds_join_group_to_pb(p_if_key, p_join, &pb_join);
    len = cdb__ds_join_group__pack(&pb_join, buf);
    pb_ds_join_group_to_pb_free_packed(&pb_join);

    return len;
}

int32
pb_ds_join_group_dump(Cdb__DsJoinGroup *p_pb_join, char *out)
{
    int32 offset = 0;

    out[0] = CMD_ZERO_CHAR;
    offset += sal_sprintf(out + offset, "/parent_if->vid=%u", p_pb_join->parent_if->vid);
    offset += pb_compose_addr_ipv4_t_dump(p_pb_join->key->group, (out + offset));
    offset += sal_sprintf(out + offset, "/vid=%u", p_pb_join->vid);
    offset += sal_sprintf(out + offset, "/type=%u", p_pb_join->type);
    offset += sal_sprintf(out + offset, "\n");

    return PM_E_NONE;
}

/* pb functions for TBL_CFM_MD DS_CFM_ERROR */
int32
pb_ds_cfm_error_to_pb(tbl_cfm_md_key_t *p_md_key, ds_cfm_error_t *p_cfm_error, Cdb__DsCfmError *p_pb_cfm_error)
{
    p_pb_cfm_error->parent_md->name = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_md_key->name)+1);
    sal_strcpy(p_pb_cfm_error->parent_md->name, p_md_key->name);

    pb_compose_cfm_time_key_t_to_pb(&p_cfm_error->key, p_pb_cfm_error->key);

    p_pb_cfm_error->level = p_cfm_error->level;
    p_pb_cfm_error->vlan_id = p_cfm_error->vlan_id;
    p_pb_cfm_error->md_name = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(p_cfm_error->md_name)+1);
    sal_strcpy(p_pb_cfm_error->md_name, p_cfm_error->md_name);
    p_pb_cfm_error->mep_id = p_cfm_error->mep_id;
    p_pb_cfm_error->err_reason = p_cfm_error->err_reason;
    pb_compose_mac_addr_t_to_pb(p_cfm_error->rmep_mac, p_pb_cfm_error->rmep_mac);

    return PM_E_NONE;
}

int32
pb_ds_cfm_error_to_pb_free_packed(Cdb__DsCfmError *p_pb_cfm_error)
{
    if (p_pb_cfm_error->parent_md->name)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_cfm_error->parent_md->name);
        p_pb_cfm_error->parent_md->name = NULL;
    }

    pb_compose_cfm_time_key_t_to_pb_free_packed(p_pb_cfm_error->key);
    if (p_pb_cfm_error->md_name)
    {
        XFREE(MEM_LIB_PROTOBUF, p_pb_cfm_error->md_name);
        p_pb_cfm_error->md_name = NULL;
    }

    pb_compose_mac_addr_t_to_pb_free_packed(p_pb_cfm_error->rmep_mac);
    return PM_E_NONE;
}

int32
pb_ds_cfm_error_pack(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len)
{
    Cdb__TblCfmMdKey pb_md_key = CDB__TBL_CFM_MD_KEY__INIT;
    Cdb__DsCfmErrorKey pb_cfm_error_key = CDB__DS_CFM_ERROR_KEY__INIT;
    Cdb__DsCfmError pb_cfm_error = CDB__DS_CFM_ERROR__INIT;
    tbl_cfm_md_key_t *p_md_key = (tbl_cfm_md_key_t*)p_tbl;
    ds_cfm_error_t *p_cfm_error = (ds_cfm_error_t*)p_ds;
    int32 len = 0;
    Cdb__ComposeMacAddrT rmep_mac = CDB__COMPOSE_MAC_ADDR_T__INIT;

    pb_cfm_error.key = &pb_cfm_error_key;
    pb_cfm_error.rmep_mac = &rmep_mac;
    pb_cfm_error.parent_md = &pb_md_key;
    pb_ds_cfm_error_to_pb(p_md_key, p_cfm_error, &pb_cfm_error);
    len = cdb__ds_cfm_error__pack(&pb_cfm_error, buf);
    pb_ds_cfm_error_to_pb_free_packed(&pb_cfm_error);

    return len;
}

int32
pb_ds_cfm_error_dump(Cdb__DsCfmError *p_pb_cfm_error, char *out)
{
    int32 offset = 0;

    out[0] = CMD_ZERO_CHAR;
    offset += sal_sprintf(out + offset, "/parent_md->name=%s", p_pb_cfm_error->parent_md->name);
    offset += pb_compose_cfm_time_key_t_dump(p_pb_cfm_error->key, (out + offset));
    offset += sal_sprintf(out + offset, "/level=%u", p_pb_cfm_error->level);
    offset += sal_sprintf(out + offset, "/vlan_id=%u", p_pb_cfm_error->vlan_id);
    offset += sal_sprintf(out + offset, "/md_name=%s", p_pb_cfm_error->md_name);
    offset += sal_sprintf(out + offset, "/mep_id=%u", p_pb_cfm_error->mep_id);
    offset += sal_sprintf(out + offset, "/err_reason=%u", p_pb_cfm_error->err_reason);
    offset += pb_compose_mac_addr_t_dump(p_pb_cfm_error->rmep_mac, (out + offset));
    offset += sal_sprintf(out + offset, "\n");

    return PM_E_NONE;
}

PB_DS_PACK_FUNC g_pb_pack_ds_func[DS_MAX] =
{
    pb_ds_brgif_pack,
    pb_ds_aclqos_if_pack,
    pb_ds_lag_pack,
    pb_ds_lacp_pack,
    pb_ds_connected_pack,
    pb_ds_connected_v6_pack,
    pb_ds_static_route_dup_ipv4_nh_pack,
    pb_ds_ospf_auth_pack,
    pb_ds_storm_control_pack,
    pb_ds_openflow_if_pack,
    pb_ds_dhclient_if_pack,
    pb_ds_pvlan_community_pack,
    pb_ds_circuit_id_pack,
    pb_ds_flush_fdb_pack,
    pb_ds_lldp_neighbour_pack,
    pb_ds_mrouter_port_pack,
    pb_ds_query_pack,
    pb_ds_l2protocol_pack,
    pb_ds_nd_prefix_pack,
    pb_ds_join_group_pack,
    pb_ds_cfm_error_pack,
};

PB_DS_UNPACK_FUNC g_pb_unpack_ds_func[DS_MAX] =
{
    (PB_DS_UNPACK_FUNC)cdb__ds_brgif__unpack,
    (PB_DS_UNPACK_FUNC)cdb__ds_aclqos_if__unpack,
    (PB_DS_UNPACK_FUNC)cdb__ds_lag__unpack,
    (PB_DS_UNPACK_FUNC)cdb__ds_lacp__unpack,
    (PB_DS_UNPACK_FUNC)cdb__ds_connected__unpack,
    (PB_DS_UNPACK_FUNC)cdb__ds_connected_v6__unpack,
    (PB_DS_UNPACK_FUNC)cdb__ds_static_route_dup_ipv4_nh__unpack,
    (PB_DS_UNPACK_FUNC)cdb__ds_ospf_auth__unpack,
    (PB_DS_UNPACK_FUNC)cdb__ds_storm_control__unpack,
    (PB_DS_UNPACK_FUNC)cdb__ds_openflow_if__unpack,
    (PB_DS_UNPACK_FUNC)cdb__ds_dhclient_if__unpack,
    (PB_DS_UNPACK_FUNC)cdb__ds_pvlan_community__unpack,
    (PB_DS_UNPACK_FUNC)cdb__ds_circuit_id__unpack,
    (PB_DS_UNPACK_FUNC)cdb__ds_flush_fdb__unpack,
    (PB_DS_UNPACK_FUNC)cdb__ds_lldp_neighbour__unpack,
    (PB_DS_UNPACK_FUNC)cdb__ds_mrouter_port__unpack,
    (PB_DS_UNPACK_FUNC)cdb__ds_query__unpack,
    (PB_DS_UNPACK_FUNC)cdb__ds_l2protocol__unpack,
    (PB_DS_UNPACK_FUNC)cdb__ds_nd_prefix__unpack,
    (PB_DS_UNPACK_FUNC)cdb__ds_join_group__unpack,
    (PB_DS_UNPACK_FUNC)cdb__ds_cfm_error__unpack,
};

PB_DS_DUMP_FUNC g_pb_dump_ds_func[DS_MAX] =
{
    (PB_DS_DUMP_FUNC)pb_ds_brgif_dump,
    (PB_DS_DUMP_FUNC)pb_ds_aclqos_if_dump,
    (PB_DS_DUMP_FUNC)pb_ds_lag_dump,
    (PB_DS_DUMP_FUNC)pb_ds_lacp_dump,
    (PB_DS_DUMP_FUNC)pb_ds_connected_dump,
    (PB_DS_DUMP_FUNC)pb_ds_connected_v6_dump,
    (PB_DS_DUMP_FUNC)pb_ds_static_route_dup_ipv4_nh_dump,
    (PB_DS_DUMP_FUNC)pb_ds_ospf_auth_dump,
    (PB_DS_DUMP_FUNC)pb_ds_storm_control_dump,
    (PB_DS_DUMP_FUNC)pb_ds_openflow_if_dump,
    (PB_DS_DUMP_FUNC)pb_ds_dhclient_if_dump,
    (PB_DS_DUMP_FUNC)pb_ds_pvlan_community_dump,
    (PB_DS_DUMP_FUNC)pb_ds_circuit_id_dump,
    (PB_DS_DUMP_FUNC)pb_ds_flush_fdb_dump,
    (PB_DS_DUMP_FUNC)pb_ds_lldp_neighbour_dump,
    (PB_DS_DUMP_FUNC)pb_ds_mrouter_port_dump,
    (PB_DS_DUMP_FUNC)pb_ds_query_dump,
    (PB_DS_DUMP_FUNC)pb_ds_l2protocol_dump,
    (PB_DS_DUMP_FUNC)pb_ds_nd_prefix_dump,
    (PB_DS_DUMP_FUNC)pb_ds_join_group_dump,
    (PB_DS_DUMP_FUNC)pb_ds_cfm_error_dump,
};

PB_DS_FREE_UNPACKED_FUNC g_pb_free_unpacked_ds_func[DS_MAX] =
{
    (PB_DS_FREE_UNPACKED_FUNC)cdb__ds_brgif__free_unpacked,
    (PB_DS_FREE_UNPACKED_FUNC)cdb__ds_aclqos_if__free_unpacked,
    (PB_DS_FREE_UNPACKED_FUNC)cdb__ds_lag__free_unpacked,
    (PB_DS_FREE_UNPACKED_FUNC)cdb__ds_lacp__free_unpacked,
    (PB_DS_FREE_UNPACKED_FUNC)cdb__ds_connected__free_unpacked,
    (PB_DS_FREE_UNPACKED_FUNC)cdb__ds_connected_v6__free_unpacked,
    (PB_DS_FREE_UNPACKED_FUNC)cdb__ds_static_route_dup_ipv4_nh__free_unpacked,
    (PB_DS_FREE_UNPACKED_FUNC)cdb__ds_ospf_auth__free_unpacked,
    (PB_DS_FREE_UNPACKED_FUNC)cdb__ds_storm_control__free_unpacked,
    (PB_DS_FREE_UNPACKED_FUNC)cdb__ds_openflow_if__free_unpacked,
    (PB_DS_FREE_UNPACKED_FUNC)cdb__ds_dhclient_if__free_unpacked,
    (PB_DS_FREE_UNPACKED_FUNC)cdb__ds_pvlan_community__free_unpacked,
    (PB_DS_FREE_UNPACKED_FUNC)cdb__ds_circuit_id__free_unpacked,
    (PB_DS_FREE_UNPACKED_FUNC)cdb__ds_flush_fdb__free_unpacked,
    (PB_DS_FREE_UNPACKED_FUNC)cdb__ds_lldp_neighbour__free_unpacked,
    (PB_DS_FREE_UNPACKED_FUNC)cdb__ds_mrouter_port__free_unpacked,
    (PB_DS_FREE_UNPACKED_FUNC)cdb__ds_query__free_unpacked,
    (PB_DS_FREE_UNPACKED_FUNC)cdb__ds_l2protocol__free_unpacked,
    (PB_DS_FREE_UNPACKED_FUNC)cdb__ds_nd_prefix__free_unpacked,
    (PB_DS_FREE_UNPACKED_FUNC)cdb__ds_join_group__free_unpacked,
    (PB_DS_FREE_UNPACKED_FUNC)cdb__ds_cfm_error__free_unpacked,
};

int32
pb_ds_pack(uint32 ds_id, void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len)
{
    if (ds_id >= DS_MAX)
    {
        return PM_E_INVALID_PARAM;
    }

    if (NULL == g_pb_pack_ds_func[ds_id])
    {
        return PM_E_NOT_SUPPORT;
    }

    return g_pb_pack_ds_func[ds_id](p_tbl, p_ds, p_ds2, buf, buf_len);
}

void*
pb_ds_unpack(uint32 ds_id, ProtobufCAllocator *allocator, uint32 len, uint8 *data)
{
    if (ds_id >= DS_MAX)
    {
        return NULL;
    }

    if (NULL == g_pb_unpack_ds_func[ds_id])
    {
        return NULL;
    }

    return g_pb_unpack_ds_func[ds_id](allocator, len, data);
}

int32
pb_ds_dump(uint32 ds_id, void *object, char *out)
{
    if (ds_id >= DS_MAX)
    {
        return PM_E_INVALID_PARAM;
    }

    if (NULL == g_pb_dump_ds_func[ds_id])
    {
        return PM_E_NOT_SUPPORT;
    }

    return g_pb_dump_ds_func[ds_id](object, out);
}

int32
pb_ds_free_unpacked(uint32 ds_id, void *object, ProtobufCAllocator *allocator)
{
    if (ds_id >= DS_MAX)
    {
        return PM_E_INVALID_PARAM;
    }

    if (NULL == g_pb_free_unpacked_ds_func[ds_id])
    {
        return PM_E_NOT_SUPPORT;
    }

    g_pb_free_unpacked_ds_func[ds_id](object, allocator);

    return PM_E_NONE;
}

