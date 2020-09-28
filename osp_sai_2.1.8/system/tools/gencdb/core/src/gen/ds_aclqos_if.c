
#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "cdb_data_cmp.h"

int32
ds_aclqos_if_add_aclqos_if_sync(tbl_interface_t *p_if, ds_aclqos_if_t *p_aclqos_if, uint32 sync)
{
    tbl_interface_t *p_db_if = NULL;
    ds_aclqos_if_t *p_db_aclqos_if = NULL;

    /* 1. lookup entry exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_aclqos_if = p_db_if->aclqos_if;
    if (p_db_aclqos_if)
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_aclqos_if = XCALLOC(MEM_DS_ACLQOS_IF, sizeof(ds_aclqos_if_t));
    if (NULL == p_db_aclqos_if)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_aclqos_if, p_aclqos_if, sizeof(ds_aclqos_if_t));

    /* 4. add to db */
    p_db_if->aclqos_if = p_db_aclqos_if;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_add(TBL_INTERFACE, DS_ACLQOS_IF, p_db_if, p_db_aclqos_if);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_add(TBL_INTERFACE, DS_ACLQOS_IF, p_db_if, p_db_aclqos_if);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_aclqos_if_del_aclqos_if_sync(tbl_interface_t *p_if, uint32 sync)
{
    tbl_interface_t *p_db_if = NULL;
    ds_aclqos_if_t *p_db_aclqos_if = NULL;

    /* 1. lookup entry exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_aclqos_if = p_db_if->aclqos_if;
    if (NULL == p_db_aclqos_if)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_del(TBL_INTERFACE, DS_ACLQOS_IF, p_db_if, p_db_aclqos_if);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_del(TBL_INTERFACE, DS_ACLQOS_IF, p_db_if, p_db_aclqos_if);
        #endif 
    }

    /* 3. delete from db */
    p_db_if->aclqos_if = NULL;

    /* 4. free db entry */
    XFREE(MEM_DS_ACLQOS_IF, p_db_aclqos_if);

    return PM_E_NONE;
}

int32
ds_aclqos_if_set_aclqos_if_field_sync(tbl_interface_t *p_if, ds_aclqos_if_t *p_aclqos_if, ds_aclqos_if_field_id_t field_id, uint32 sync)
{
    tbl_interface_t *p_db_if = NULL;
    ds_aclqos_if_t *p_db_aclqos_if = NULL;

    /* 1. lookup entry exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }

    p_db_aclqos_if = ds_aclqos_if_get_aclqos_if(p_if);
    if (NULL == p_db_aclqos_if)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case DS_ACLQOS_IF_FLD_DEFAULT_COS:
        p_db_aclqos_if->default_cos = p_aclqos_if->default_cos;
        break;

    case DS_ACLQOS_IF_FLD_DOMAIN:
        p_db_aclqos_if->domain = p_aclqos_if->domain;
        break;

    case DS_ACLQOS_IF_FLD_TRUST:
        p_db_aclqos_if->trust = p_aclqos_if->trust;
        break;

    case DS_ACLQOS_IF_FLD_REPLACE:
        p_db_aclqos_if->replace = p_aclqos_if->replace;
        break;

    case DS_ACLQOS_IF_FLD_SHAPE_PIR:
        p_db_aclqos_if->shape_pir = p_aclqos_if->shape_pir;
        break;

    case DS_ACLQOS_IF_FLD_QUEUE_STATS_ENABLE:
        p_db_aclqos_if->queue_stats_enable = p_aclqos_if->queue_stats_enable;
        break;

    case DS_ACLQOS_IF_FLD_INPUT_POLICY_MAP:
        sal_memcpy(p_db_aclqos_if->input_policy_map, p_aclqos_if->input_policy_map, sizeof(p_aclqos_if->input_policy_map));
        break;

    case DS_ACLQOS_IF_FLD_OUTPUT_POLICY_MAP:
        sal_memcpy(p_db_aclqos_if->output_policy_map, p_aclqos_if->output_policy_map, sizeof(p_aclqos_if->output_policy_map));
        break;

    case DS_ACLQOS_IF_FLD_INPUT_POLICER:
        sal_memcpy(p_db_aclqos_if->input_policer, p_aclqos_if->input_policer, sizeof(p_aclqos_if->input_policer));
        break;

    case DS_ACLQOS_IF_FLD_OUTPUT_POLICER:
        sal_memcpy(p_db_aclqos_if->output_policer, p_aclqos_if->output_policer, sizeof(p_aclqos_if->output_policer));
        break;

    case DS_ACLQOS_IF_FLD_QUEUE_SHAPE_PROFILE:
        sal_memcpy(p_db_aclqos_if->queue_shape_profile, p_aclqos_if->queue_shape_profile, sizeof(p_aclqos_if->queue_shape_profile));
        break;

    case DS_ACLQOS_IF_FLD_QUEUE_DROP_PROFILE:
        sal_memcpy(p_db_aclqos_if->queue_drop_profile, p_aclqos_if->queue_drop_profile, sizeof(p_aclqos_if->queue_drop_profile));
        break;

    case DS_ACLQOS_IF_FLD_QUEUE_CLASS:
        sal_memcpy(p_db_aclqos_if->queue_class, p_aclqos_if->queue_class, sizeof(p_aclqos_if->queue_class));
        break;

    case DS_ACLQOS_IF_FLD_QUEUE_WEIGHT:
        sal_memcpy(p_db_aclqos_if->queue_weight, p_aclqos_if->queue_weight, sizeof(p_aclqos_if->queue_weight));
        break;

    case DS_ACLQOS_IF_FLD_QUEUE_DROP_MODE:
        sal_memcpy(p_db_aclqos_if->queue_drop_mode, p_aclqos_if->queue_drop_mode, sizeof(p_aclqos_if->queue_drop_mode));
        break;

    case DS_ACLQOS_IF_FLD_QUEUE_ECN_ENABLE:
        sal_memcpy(p_db_aclqos_if->queue_ecn_enable, p_aclqos_if->queue_ecn_enable, sizeof(p_aclqos_if->queue_ecn_enable));
        break;

    case DS_ACLQOS_IF_FLD_QUEUE_TRANSIMT_PKT:
        sal_memcpy(p_db_aclqos_if->queue_transimt_pkt, p_aclqos_if->queue_transimt_pkt, sizeof(p_aclqos_if->queue_transimt_pkt));
        break;

    case DS_ACLQOS_IF_FLD_QUEUE_TRANSIMT_BYTE:
        sal_memcpy(p_db_aclqos_if->queue_transimt_byte, p_aclqos_if->queue_transimt_byte, sizeof(p_aclqos_if->queue_transimt_byte));
        break;

    case DS_ACLQOS_IF_FLD_QUEUE_DROP_PKT:
        sal_memcpy(p_db_aclqos_if->queue_drop_pkt, p_aclqos_if->queue_drop_pkt, sizeof(p_aclqos_if->queue_drop_pkt));
        break;

    case DS_ACLQOS_IF_FLD_QUEUE_DROP_BYTE:
        sal_memcpy(p_db_aclqos_if->queue_drop_byte, p_aclqos_if->queue_drop_byte, sizeof(p_aclqos_if->queue_drop_byte));
        break;

    case DS_ACLQOS_IF_FLD_QUEUE0_TRANSIMT_PKT:
        p_db_aclqos_if->queue0_transimt_pkt = p_aclqos_if->queue0_transimt_pkt;
        break;

    case DS_ACLQOS_IF_FLD_QUEUE0_TRANSIMT_BYTE:
        p_db_aclqos_if->queue0_transimt_byte = p_aclqos_if->queue0_transimt_byte;
        break;

    case DS_ACLQOS_IF_FLD_QUEUE0_DROP_PKT:
        p_db_aclqos_if->queue0_drop_pkt = p_aclqos_if->queue0_drop_pkt;
        break;

    case DS_ACLQOS_IF_FLD_QUEUE0_DROP_BYTE:
        p_db_aclqos_if->queue0_drop_byte = p_aclqos_if->queue0_drop_byte;
        break;

    case DS_ACLQOS_IF_FLD_QUEUE1_TRANSIMT_PKT:
        p_db_aclqos_if->queue1_transimt_pkt = p_aclqos_if->queue1_transimt_pkt;
        break;

    case DS_ACLQOS_IF_FLD_QUEUE1_TRANSIMT_BYTE:
        p_db_aclqos_if->queue1_transimt_byte = p_aclqos_if->queue1_transimt_byte;
        break;

    case DS_ACLQOS_IF_FLD_QUEUE1_DROP_PKT:
        p_db_aclqos_if->queue1_drop_pkt = p_aclqos_if->queue1_drop_pkt;
        break;

    case DS_ACLQOS_IF_FLD_QUEUE1_DROP_BYTE:
        p_db_aclqos_if->queue1_drop_byte = p_aclqos_if->queue1_drop_byte;
        break;

    case DS_ACLQOS_IF_FLD_QUEUE2_TRANSIMT_PKT:
        p_db_aclqos_if->queue2_transimt_pkt = p_aclqos_if->queue2_transimt_pkt;
        break;

    case DS_ACLQOS_IF_FLD_QUEUE2_TRANSIMT_BYTE:
        p_db_aclqos_if->queue2_transimt_byte = p_aclqos_if->queue2_transimt_byte;
        break;

    case DS_ACLQOS_IF_FLD_QUEUE2_DROP_PKT:
        p_db_aclqos_if->queue2_drop_pkt = p_aclqos_if->queue2_drop_pkt;
        break;

    case DS_ACLQOS_IF_FLD_QUEUE2_DROP_BYTE:
        p_db_aclqos_if->queue2_drop_byte = p_aclqos_if->queue2_drop_byte;
        break;

    case DS_ACLQOS_IF_FLD_QUEUE3_TRANSIMT_PKT:
        p_db_aclqos_if->queue3_transimt_pkt = p_aclqos_if->queue3_transimt_pkt;
        break;

    case DS_ACLQOS_IF_FLD_QUEUE3_TRANSIMT_BYTE:
        p_db_aclqos_if->queue3_transimt_byte = p_aclqos_if->queue3_transimt_byte;
        break;

    case DS_ACLQOS_IF_FLD_QUEUE3_DROP_PKT:
        p_db_aclqos_if->queue3_drop_pkt = p_aclqos_if->queue3_drop_pkt;
        break;

    case DS_ACLQOS_IF_FLD_QUEUE3_DROP_BYTE:
        p_db_aclqos_if->queue3_drop_byte = p_aclqos_if->queue3_drop_byte;
        break;

    case DS_ACLQOS_IF_FLD_QUEUE4_TRANSIMT_PKT:
        p_db_aclqos_if->queue4_transimt_pkt = p_aclqos_if->queue4_transimt_pkt;
        break;

    case DS_ACLQOS_IF_FLD_QUEUE4_TRANSIMT_BYTE:
        p_db_aclqos_if->queue4_transimt_byte = p_aclqos_if->queue4_transimt_byte;
        break;

    case DS_ACLQOS_IF_FLD_QUEUE4_DROP_PKT:
        p_db_aclqos_if->queue4_drop_pkt = p_aclqos_if->queue4_drop_pkt;
        break;

    case DS_ACLQOS_IF_FLD_QUEUE4_DROP_BYTE:
        p_db_aclqos_if->queue4_drop_byte = p_aclqos_if->queue4_drop_byte;
        break;

    case DS_ACLQOS_IF_FLD_QUEUE5_TRANSIMT_PKT:
        p_db_aclqos_if->queue5_transimt_pkt = p_aclqos_if->queue5_transimt_pkt;
        break;

    case DS_ACLQOS_IF_FLD_QUEUE5_TRANSIMT_BYTE:
        p_db_aclqos_if->queue5_transimt_byte = p_aclqos_if->queue5_transimt_byte;
        break;

    case DS_ACLQOS_IF_FLD_QUEUE5_DROP_PKT:
        p_db_aclqos_if->queue5_drop_pkt = p_aclqos_if->queue5_drop_pkt;
        break;

    case DS_ACLQOS_IF_FLD_QUEUE5_DROP_BYTE:
        p_db_aclqos_if->queue5_drop_byte = p_aclqos_if->queue5_drop_byte;
        break;

    case DS_ACLQOS_IF_FLD_QUEUE6_TRANSIMT_PKT:
        p_db_aclqos_if->queue6_transimt_pkt = p_aclqos_if->queue6_transimt_pkt;
        break;

    case DS_ACLQOS_IF_FLD_QUEUE6_TRANSIMT_BYTE:
        p_db_aclqos_if->queue6_transimt_byte = p_aclqos_if->queue6_transimt_byte;
        break;

    case DS_ACLQOS_IF_FLD_QUEUE6_DROP_PKT:
        p_db_aclqos_if->queue6_drop_pkt = p_aclqos_if->queue6_drop_pkt;
        break;

    case DS_ACLQOS_IF_FLD_QUEUE6_DROP_BYTE:
        p_db_aclqos_if->queue6_drop_byte = p_aclqos_if->queue6_drop_byte;
        break;

    case DS_ACLQOS_IF_FLD_QUEUE7_TRANSIMT_PKT:
        p_db_aclqos_if->queue7_transimt_pkt = p_aclqos_if->queue7_transimt_pkt;
        break;

    case DS_ACLQOS_IF_FLD_QUEUE7_TRANSIMT_BYTE:
        p_db_aclqos_if->queue7_transimt_byte = p_aclqos_if->queue7_transimt_byte;
        break;

    case DS_ACLQOS_IF_FLD_QUEUE7_DROP_PKT:
        p_db_aclqos_if->queue7_drop_pkt = p_aclqos_if->queue7_drop_pkt;
        break;

    case DS_ACLQOS_IF_FLD_QUEUE7_DROP_BYTE:
        p_db_aclqos_if->queue7_drop_byte = p_aclqos_if->queue7_drop_byte;
        break;

    case DS_ACLQOS_IF_FLD_PHB_ENABLE:
        p_db_aclqos_if->phb_enable = p_aclqos_if->phb_enable;
        break;

    case DS_ACLQOS_IF_FLD_MAX:
        sal_memcpy(p_db_aclqos_if, p_aclqos_if, sizeof(ds_aclqos_if_t));
        break;

    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_set(TBL_INTERFACE, DS_ACLQOS_IF, field_id, p_db_if, p_db_aclqos_if);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_set(TBL_INTERFACE, DS_ACLQOS_IF, field_id, p_db_if, p_db_aclqos_if);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_aclqos_if_add_aclqos_if(tbl_interface_t *p_if, ds_aclqos_if_t *p_aclqos_if)
{
    return ds_aclqos_if_add_aclqos_if_sync(p_if, p_aclqos_if, TRUE);
}

int32
ds_aclqos_if_del_aclqos_if(tbl_interface_t *p_if)
{
    return ds_aclqos_if_del_aclqos_if_sync(p_if, TRUE);
}

int32
ds_aclqos_if_set_aclqos_if_field(tbl_interface_t *p_if, ds_aclqos_if_t *p_aclqos_if, ds_aclqos_if_field_id_t field_id)
{
    return ds_aclqos_if_set_aclqos_if_field_sync(p_if, p_aclqos_if, field_id, TRUE);
}

ds_aclqos_if_t*
ds_aclqos_if_get_aclqos_if(tbl_interface_t *p_if)
{
    tbl_interface_t *p_db_if = NULL;

    /* 1. lookup entry exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return NULL;
    }

    return p_db_if->aclqos_if;
}

int32
ds_aclqos_if_key_str2val(char* tbl_key_value, char* str, tbl_interface_t *p_if, ds_aclqos_if_t *p_aclqos_if)
{

    int32 ret = 0;

    ret = tbl_interface_key_str2val(tbl_key_value, p_if);
    return ret;
}
char*
ds_aclqos_if_key_value_dump(tbl_interface_t *p_if, ds_aclqos_if_t *p_aclqos_if, char* str)
{

    sal_sprintf(str, "%s",
        tbl_interface_key_value_dump(p_if, str));

    return str;
}

char*
ds_aclqos_if_key_name_dump(tbl_interface_t *p_if, ds_aclqos_if_t *p_aclqos_if, char* str)
{

    cdb_node_t *p_node_tbl = cdb_get_tbl(TBL_INTERFACE);
    cdb_node_t *p_node_ds = cdb_get_ds(DS_ACLQOS_IF);
    char tbl_str[MAX_CMD_STR_LEN];
    char ds_str[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s",
        p_node_tbl->name, tbl_interface_key_value_dump(p_if, tbl_str), p_node_ds->name, ds_aclqos_if_key_value_dump(p_if, p_aclqos_if, ds_str));
    return str;
}

char*
ds_aclqos_if_field_name_dump(tbl_interface_t *p_if, ds_aclqos_if_t *p_aclqos_if, int32 field_id, char* str)
{

    cdb_node_t *p_node = cdb_get_ds(DS_ACLQOS_IF);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= DS_ACLQOS_IF_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
ds_aclqos_if_field_value_dump(tbl_interface_t *p_if, ds_aclqos_if_t *p_aclqos_if, int32 field_id, char* str)
{

    if (FLD_MATCH(DS_ACLQOS_IF_FLD_DEFAULT_COS, field_id))
    {
        sal_sprintf(str, "%u", p_aclqos_if->default_cos);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_DOMAIN, field_id))
    {
        sal_sprintf(str, "%u", p_aclqos_if->domain);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_TRUST, field_id))
    {
        sal_sprintf(str, "%u", p_aclqos_if->trust);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_REPLACE, field_id))
    {
        sal_sprintf(str, "%u", p_aclqos_if->replace);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_SHAPE_PIR, field_id))
    {
        sal_sprintf(str, "%u", p_aclqos_if->shape_pir);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE_STATS_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_aclqos_if->queue_stats_enable);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_INPUT_POLICY_MAP, field_id))
    {
        sal_sprintf(str, "%s", p_aclqos_if->input_policy_map);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_OUTPUT_POLICY_MAP, field_id))
    {
        sal_sprintf(str, "%s", p_aclqos_if->output_policy_map);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_INPUT_POLICER, field_id))
    {
        sal_sprintf(str, "%s", p_aclqos_if->input_policer);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_OUTPUT_POLICER, field_id))
    {
        sal_sprintf(str, "%s", p_aclqos_if->output_policer);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE_SHAPE_PROFILE, field_id))
    {
        cdb_string_array_val2str(str, MAX_CMD_STR_LEN, (const char*)p_aclqos_if->queue_shape_profile,
        sizeof(p_aclqos_if->queue_shape_profile[0]), sizeof(p_aclqos_if->queue_shape_profile)/sizeof(p_aclqos_if->queue_shape_profile[0]));
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE_DROP_PROFILE, field_id))
    {
        cdb_string_array_val2str(str, MAX_CMD_STR_LEN, (const char*)p_aclqos_if->queue_drop_profile,
        sizeof(p_aclqos_if->queue_drop_profile[0]), sizeof(p_aclqos_if->queue_drop_profile)/sizeof(p_aclqos_if->queue_drop_profile[0]));
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE_CLASS, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_aclqos_if->queue_class, sizeof(p_aclqos_if->queue_class)/4);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE_WEIGHT, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_aclqos_if->queue_weight, sizeof(p_aclqos_if->queue_weight)/4);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE_DROP_MODE, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_aclqos_if->queue_drop_mode, sizeof(p_aclqos_if->queue_drop_mode)/4);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE_ECN_ENABLE, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_aclqos_if->queue_ecn_enable, sizeof(p_aclqos_if->queue_ecn_enable)/4);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE_TRANSIMT_PKT, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_aclqos_if->queue_transimt_pkt, sizeof(p_aclqos_if->queue_transimt_pkt)/4);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE_TRANSIMT_BYTE, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_aclqos_if->queue_transimt_byte, sizeof(p_aclqos_if->queue_transimt_byte)/4);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE_DROP_PKT, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_aclqos_if->queue_drop_pkt, sizeof(p_aclqos_if->queue_drop_pkt)/4);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE_DROP_BYTE, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_aclqos_if->queue_drop_byte, sizeof(p_aclqos_if->queue_drop_byte)/4);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE0_TRANSIMT_PKT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if->queue0_transimt_pkt);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE0_TRANSIMT_BYTE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if->queue0_transimt_byte);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE0_DROP_PKT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if->queue0_drop_pkt);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE0_DROP_BYTE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if->queue0_drop_byte);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE1_TRANSIMT_PKT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if->queue1_transimt_pkt);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE1_TRANSIMT_BYTE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if->queue1_transimt_byte);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE1_DROP_PKT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if->queue1_drop_pkt);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE1_DROP_BYTE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if->queue1_drop_byte);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE2_TRANSIMT_PKT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if->queue2_transimt_pkt);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE2_TRANSIMT_BYTE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if->queue2_transimt_byte);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE2_DROP_PKT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if->queue2_drop_pkt);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE2_DROP_BYTE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if->queue2_drop_byte);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE3_TRANSIMT_PKT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if->queue3_transimt_pkt);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE3_TRANSIMT_BYTE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if->queue3_transimt_byte);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE3_DROP_PKT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if->queue3_drop_pkt);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE3_DROP_BYTE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if->queue3_drop_byte);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE4_TRANSIMT_PKT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if->queue4_transimt_pkt);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE4_TRANSIMT_BYTE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if->queue4_transimt_byte);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE4_DROP_PKT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if->queue4_drop_pkt);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE4_DROP_BYTE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if->queue4_drop_byte);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE5_TRANSIMT_PKT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if->queue5_transimt_pkt);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE5_TRANSIMT_BYTE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if->queue5_transimt_byte);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE5_DROP_PKT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if->queue5_drop_pkt);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE5_DROP_BYTE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if->queue5_drop_byte);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE6_TRANSIMT_PKT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if->queue6_transimt_pkt);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE6_TRANSIMT_BYTE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if->queue6_transimt_byte);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE6_DROP_PKT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if->queue6_drop_pkt);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE6_DROP_BYTE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if->queue6_drop_byte);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE7_TRANSIMT_PKT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if->queue7_transimt_pkt);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE7_TRANSIMT_BYTE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if->queue7_transimt_byte);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE7_DROP_PKT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if->queue7_drop_pkt);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE7_DROP_BYTE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_aclqos_if->queue7_drop_byte);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_PHB_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_aclqos_if->phb_enable);
    }
    return str;
}

char**
ds_aclqos_if_table_dump(tbl_interface_t *p_if, ds_aclqos_if_t *p_aclqos_if, char** str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         ds_aclqos_if_key_name_dump(p_if, p_aclqos_if, buf));
    for(i=1; i<DS_ACLQOS_IF_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            ds_aclqos_if_field_name_dump(p_if, p_aclqos_if, i, buf));
        sal_sprintf(str[j++], "%s",
            ds_aclqos_if_field_value_dump(p_if, p_aclqos_if, i, buf));
    }
    return str;
}

int32
ds_aclqos_if_field_value_parser(char *str, int32 field_id, tbl_interface_t *p_if, ds_aclqos_if_t *p_aclqos_if)
{
    int32 ret=0;
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_DEFAULT_COS, field_id))
    {
        p_aclqos_if->default_cos = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_DOMAIN, field_id))
    {
        p_aclqos_if->domain = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_TRUST, field_id))
    {
        p_aclqos_if->trust = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_REPLACE, field_id))
    {
        p_aclqos_if->replace = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_SHAPE_PIR, field_id))
    {
        p_aclqos_if->shape_pir = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE_STATS_ENABLE, field_id))
    {
        p_aclqos_if->queue_stats_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_INPUT_POLICY_MAP, field_id))
    {
        sal_strcpy(p_aclqos_if->input_policy_map, str);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_OUTPUT_POLICY_MAP, field_id))
    {
        sal_strcpy(p_aclqos_if->output_policy_map, str);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_INPUT_POLICER, field_id))
    {
        sal_strcpy(p_aclqos_if->input_policer, str);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_OUTPUT_POLICER, field_id))
    {
        sal_strcpy(p_aclqos_if->output_policer, str);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE_SHAPE_PROFILE, field_id))
    {
        cdb_string_array_str2val(str, (char**)(&p_aclqos_if->queue_shape_profile));
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE_DROP_PROFILE, field_id))
    {
        cdb_string_array_str2val(str, (char**)(&p_aclqos_if->queue_drop_profile));
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE_CLASS, field_id))
    {
        cdb_uint32_array_str2val(str, p_aclqos_if->queue_class);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE_WEIGHT, field_id))
    {
        cdb_uint32_array_str2val(str, p_aclqos_if->queue_weight);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE_DROP_MODE, field_id))
    {
        cdb_uint32_array_str2val(str, p_aclqos_if->queue_drop_mode);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE_ECN_ENABLE, field_id))
    {
        cdb_uint32_array_str2val(str, p_aclqos_if->queue_ecn_enable);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE_TRANSIMT_PKT, field_id))
    {
        cdb_uint32_array_str2val(str, p_aclqos_if->queue_transimt_pkt);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE_TRANSIMT_BYTE, field_id))
    {
        cdb_uint32_array_str2val(str, p_aclqos_if->queue_transimt_byte);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE_DROP_PKT, field_id))
    {
        cdb_uint32_array_str2val(str, p_aclqos_if->queue_drop_pkt);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE_DROP_BYTE, field_id))
    {
        cdb_uint32_array_str2val(str, p_aclqos_if->queue_drop_byte);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE0_TRANSIMT_PKT, field_id))
    {
        p_aclqos_if->queue0_transimt_pkt = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE0_TRANSIMT_BYTE, field_id))
    {
        p_aclqos_if->queue0_transimt_byte = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE0_DROP_PKT, field_id))
    {
        p_aclqos_if->queue0_drop_pkt = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE0_DROP_BYTE, field_id))
    {
        p_aclqos_if->queue0_drop_byte = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE1_TRANSIMT_PKT, field_id))
    {
        p_aclqos_if->queue1_transimt_pkt = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE1_TRANSIMT_BYTE, field_id))
    {
        p_aclqos_if->queue1_transimt_byte = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE1_DROP_PKT, field_id))
    {
        p_aclqos_if->queue1_drop_pkt = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE1_DROP_BYTE, field_id))
    {
        p_aclqos_if->queue1_drop_byte = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE2_TRANSIMT_PKT, field_id))
    {
        p_aclqos_if->queue2_transimt_pkt = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE2_TRANSIMT_BYTE, field_id))
    {
        p_aclqos_if->queue2_transimt_byte = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE2_DROP_PKT, field_id))
    {
        p_aclqos_if->queue2_drop_pkt = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE2_DROP_BYTE, field_id))
    {
        p_aclqos_if->queue2_drop_byte = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE3_TRANSIMT_PKT, field_id))
    {
        p_aclqos_if->queue3_transimt_pkt = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE3_TRANSIMT_BYTE, field_id))
    {
        p_aclqos_if->queue3_transimt_byte = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE3_DROP_PKT, field_id))
    {
        p_aclqos_if->queue3_drop_pkt = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE3_DROP_BYTE, field_id))
    {
        p_aclqos_if->queue3_drop_byte = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE4_TRANSIMT_PKT, field_id))
    {
        p_aclqos_if->queue4_transimt_pkt = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE4_TRANSIMT_BYTE, field_id))
    {
        p_aclqos_if->queue4_transimt_byte = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE4_DROP_PKT, field_id))
    {
        p_aclqos_if->queue4_drop_pkt = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE4_DROP_BYTE, field_id))
    {
        p_aclqos_if->queue4_drop_byte = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE5_TRANSIMT_PKT, field_id))
    {
        p_aclqos_if->queue5_transimt_pkt = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE5_TRANSIMT_BYTE, field_id))
    {
        p_aclqos_if->queue5_transimt_byte = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE5_DROP_PKT, field_id))
    {
        p_aclqos_if->queue5_drop_pkt = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE5_DROP_BYTE, field_id))
    {
        p_aclqos_if->queue5_drop_byte = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE6_TRANSIMT_PKT, field_id))
    {
        p_aclqos_if->queue6_transimt_pkt = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE6_TRANSIMT_BYTE, field_id))
    {
        p_aclqos_if->queue6_transimt_byte = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE6_DROP_PKT, field_id))
    {
        p_aclqos_if->queue6_drop_pkt = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE6_DROP_BYTE, field_id))
    {
        p_aclqos_if->queue6_drop_byte = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE7_TRANSIMT_PKT, field_id))
    {
        p_aclqos_if->queue7_transimt_pkt = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE7_TRANSIMT_BYTE, field_id))
    {
        p_aclqos_if->queue7_transimt_byte = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE7_DROP_PKT, field_id))
    {
        p_aclqos_if->queue7_drop_pkt = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE7_DROP_BYTE, field_id))
    {
        p_aclqos_if->queue7_drop_byte = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_PHB_ENABLE, field_id))
    {
        p_aclqos_if->phb_enable = cdb_uint_str2val(str, &ret);
    }
    return ret;
}

int32
ds_aclqos_if_table_parser(char** array, char* tbl_key_value, char* ds_key_value,tbl_interface_t *p_if, ds_aclqos_if_t *p_aclqos_if)
{
    int32 rc;
    int32 i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_ds(DS_ACLQOS_IF);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(ds_aclqos_if_key_str2val(tbl_key_value, ds_key_value, p_if, p_aclqos_if));

    for(i=1; i<DS_ACLQOS_IF_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(ds_aclqos_if_field_value_parser( array[j++], i, p_if, p_aclqos_if));
    }

    return PM_E_NONE;
}

int32
ds_aclqos_if_dump_one(ds_aclqos_if_t *p_aclqos_if, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_ds(DS_ACLQOS_IF);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(DS_ACLQOS_IF_FLD_DEFAULT_COS, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_ACLQOS_IF_FLD_DEFAULT_COS].name,
            p_aclqos_if->default_cos);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_DOMAIN, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_ACLQOS_IF_FLD_DOMAIN].name,
            p_aclqos_if->domain);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_TRUST, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_ACLQOS_IF_FLD_TRUST].name,
            p_aclqos_if->trust);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_REPLACE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_ACLQOS_IF_FLD_REPLACE].name,
            p_aclqos_if->replace);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_SHAPE_PIR, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_ACLQOS_IF_FLD_SHAPE_PIR].name,
            p_aclqos_if->shape_pir);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE_STATS_ENABLE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE_STATS_ENABLE].name,
            p_aclqos_if->queue_stats_enable);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_INPUT_POLICY_MAP, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_ACLQOS_IF_FLD_INPUT_POLICY_MAP].name,
            p_aclqos_if->input_policy_map);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_OUTPUT_POLICY_MAP, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_ACLQOS_IF_FLD_OUTPUT_POLICY_MAP].name,
            p_aclqos_if->output_policy_map);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_INPUT_POLICER, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_ACLQOS_IF_FLD_INPUT_POLICER].name,
            p_aclqos_if->input_policer);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_OUTPUT_POLICER, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_ACLQOS_IF_FLD_OUTPUT_POLICER].name,
            p_aclqos_if->output_policer);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE_SHAPE_PROFILE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE_SHAPE_PROFILE].name,
            cdb_string_array_val2str(buf, MAX_CMD_STR_LEN, (const char*)p_aclqos_if->queue_shape_profile,
            sizeof(p_aclqos_if->queue_shape_profile[0]), sizeof(p_aclqos_if->queue_shape_profile)/sizeof(p_aclqos_if->queue_shape_profile[0])));
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE_DROP_PROFILE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE_DROP_PROFILE].name,
            cdb_string_array_val2str(buf, MAX_CMD_STR_LEN, (const char*)p_aclqos_if->queue_drop_profile,
            sizeof(p_aclqos_if->queue_drop_profile[0]), sizeof(p_aclqos_if->queue_drop_profile)/sizeof(p_aclqos_if->queue_drop_profile[0])));
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE_CLASS, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE_CLASS].name,
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_aclqos_if->queue_class, sizeof(p_aclqos_if->queue_class)/4));
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE_WEIGHT, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE_WEIGHT].name,
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_aclqos_if->queue_weight, sizeof(p_aclqos_if->queue_weight)/4));
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE_DROP_MODE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE_DROP_MODE].name,
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_aclqos_if->queue_drop_mode, sizeof(p_aclqos_if->queue_drop_mode)/4));
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE_ECN_ENABLE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE_ECN_ENABLE].name,
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_aclqos_if->queue_ecn_enable, sizeof(p_aclqos_if->queue_ecn_enable)/4));
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE_TRANSIMT_PKT, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE_TRANSIMT_PKT].name,
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_aclqos_if->queue_transimt_pkt, sizeof(p_aclqos_if->queue_transimt_pkt)/4));
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE_TRANSIMT_BYTE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE_TRANSIMT_BYTE].name,
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_aclqos_if->queue_transimt_byte, sizeof(p_aclqos_if->queue_transimt_byte)/4));
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE_DROP_PKT, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE_DROP_PKT].name,
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_aclqos_if->queue_drop_pkt, sizeof(p_aclqos_if->queue_drop_pkt)/4));
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE_DROP_BYTE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE_DROP_BYTE].name,
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_aclqos_if->queue_drop_byte, sizeof(p_aclqos_if->queue_drop_byte)/4));
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE0_TRANSIMT_PKT, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64,
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE0_TRANSIMT_PKT].name,
            p_aclqos_if->queue0_transimt_pkt);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE0_TRANSIMT_BYTE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64,
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE0_TRANSIMT_BYTE].name,
            p_aclqos_if->queue0_transimt_byte);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE0_DROP_PKT, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64,
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE0_DROP_PKT].name,
            p_aclqos_if->queue0_drop_pkt);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE0_DROP_BYTE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64,
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE0_DROP_BYTE].name,
            p_aclqos_if->queue0_drop_byte);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE1_TRANSIMT_PKT, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64,
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE1_TRANSIMT_PKT].name,
            p_aclqos_if->queue1_transimt_pkt);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE1_TRANSIMT_BYTE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64,
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE1_TRANSIMT_BYTE].name,
            p_aclqos_if->queue1_transimt_byte);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE1_DROP_PKT, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64,
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE1_DROP_PKT].name,
            p_aclqos_if->queue1_drop_pkt);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE1_DROP_BYTE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64,
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE1_DROP_BYTE].name,
            p_aclqos_if->queue1_drop_byte);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE2_TRANSIMT_PKT, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64,
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE2_TRANSIMT_PKT].name,
            p_aclqos_if->queue2_transimt_pkt);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE2_TRANSIMT_BYTE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64,
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE2_TRANSIMT_BYTE].name,
            p_aclqos_if->queue2_transimt_byte);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE2_DROP_PKT, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64,
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE2_DROP_PKT].name,
            p_aclqos_if->queue2_drop_pkt);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE2_DROP_BYTE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64,
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE2_DROP_BYTE].name,
            p_aclqos_if->queue2_drop_byte);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE3_TRANSIMT_PKT, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64,
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE3_TRANSIMT_PKT].name,
            p_aclqos_if->queue3_transimt_pkt);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE3_TRANSIMT_BYTE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64,
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE3_TRANSIMT_BYTE].name,
            p_aclqos_if->queue3_transimt_byte);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE3_DROP_PKT, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64,
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE3_DROP_PKT].name,
            p_aclqos_if->queue3_drop_pkt);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE3_DROP_BYTE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64,
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE3_DROP_BYTE].name,
            p_aclqos_if->queue3_drop_byte);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE4_TRANSIMT_PKT, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64,
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE4_TRANSIMT_PKT].name,
            p_aclqos_if->queue4_transimt_pkt);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE4_TRANSIMT_BYTE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64,
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE4_TRANSIMT_BYTE].name,
            p_aclqos_if->queue4_transimt_byte);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE4_DROP_PKT, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64,
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE4_DROP_PKT].name,
            p_aclqos_if->queue4_drop_pkt);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE4_DROP_BYTE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64,
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE4_DROP_BYTE].name,
            p_aclqos_if->queue4_drop_byte);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE5_TRANSIMT_PKT, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64,
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE5_TRANSIMT_PKT].name,
            p_aclqos_if->queue5_transimt_pkt);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE5_TRANSIMT_BYTE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64,
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE5_TRANSIMT_BYTE].name,
            p_aclqos_if->queue5_transimt_byte);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE5_DROP_PKT, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64,
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE5_DROP_PKT].name,
            p_aclqos_if->queue5_drop_pkt);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE5_DROP_BYTE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64,
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE5_DROP_BYTE].name,
            p_aclqos_if->queue5_drop_byte);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE6_TRANSIMT_PKT, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64,
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE6_TRANSIMT_PKT].name,
            p_aclqos_if->queue6_transimt_pkt);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE6_TRANSIMT_BYTE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64,
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE6_TRANSIMT_BYTE].name,
            p_aclqos_if->queue6_transimt_byte);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE6_DROP_PKT, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64,
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE6_DROP_PKT].name,
            p_aclqos_if->queue6_drop_pkt);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE6_DROP_BYTE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64,
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE6_DROP_BYTE].name,
            p_aclqos_if->queue6_drop_byte);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE7_TRANSIMT_PKT, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64,
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE7_TRANSIMT_PKT].name,
            p_aclqos_if->queue7_transimt_pkt);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE7_TRANSIMT_BYTE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64,
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE7_TRANSIMT_BYTE].name,
            p_aclqos_if->queue7_transimt_byte);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE7_DROP_PKT, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64,
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE7_DROP_PKT].name,
            p_aclqos_if->queue7_drop_pkt);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_QUEUE7_DROP_BYTE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64,
            p_tbl_info->field[DS_ACLQOS_IF_FLD_QUEUE7_DROP_BYTE].name,
            p_aclqos_if->queue7_drop_byte);
    }
    if (FLD_MATCH(DS_ACLQOS_IF_FLD_PHB_ENABLE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_ACLQOS_IF_FLD_PHB_ENABLE].name,
            p_aclqos_if->phb_enable);
    }

    return PM_E_NONE;
}
int32
ds_aclqos_if_get_aclqos_if_field(tbl_interface_t *p_db_if, ds_aclqos_if_field_id_t field_id, void* field_value)
{
    ds_aclqos_if_t *p_db_aclqos_if = NULL;

    p_db_aclqos_if = ds_aclqos_if_get_aclqos_if(p_db_if);
    if (NULL == p_db_aclqos_if)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case DS_ACLQOS_IF_FLD_QUEUE_SHAPE_PROFILE:
        sal_memcpy(field_value, p_db_aclqos_if->queue_shape_profile, sizeof(p_db_aclqos_if->queue_shape_profile));
        break;

    case DS_ACLQOS_IF_FLD_QUEUE_DROP_PROFILE:
        sal_memcpy(field_value, p_db_aclqos_if->queue_drop_profile, sizeof(p_db_aclqos_if->queue_drop_profile));
        break;

    case DS_ACLQOS_IF_FLD_QUEUE_CLASS:
        sal_memcpy(field_value, p_db_aclqos_if->queue_class, sizeof(p_db_aclqos_if->queue_class));
        break;

    case DS_ACLQOS_IF_FLD_QUEUE_WEIGHT:
        sal_memcpy(field_value, p_db_aclqos_if->queue_weight, sizeof(p_db_aclqos_if->queue_weight));
        break;

    case DS_ACLQOS_IF_FLD_QUEUE_DROP_MODE:
        sal_memcpy(field_value, p_db_aclqos_if->queue_drop_mode, sizeof(p_db_aclqos_if->queue_drop_mode));
        break;

    case DS_ACLQOS_IF_FLD_QUEUE_ECN_ENABLE:
        sal_memcpy(field_value, p_db_aclqos_if->queue_ecn_enable, sizeof(p_db_aclqos_if->queue_ecn_enable));
        break;

    case DS_ACLQOS_IF_FLD_MAX:
        sal_memcpy(field_value, p_db_aclqos_if, sizeof(ds_aclqos_if_t));
        break;

    default:
        return PM_E_NOT_SUPPORT;
    }

    return PM_E_NONE;
}

