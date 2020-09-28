
#include "proto.h"
#include "gen/tbl_vlanclass_rule_define.h"
#include "gen/tbl_vlanclass_rule.h"
#include "cdb_data_cmp.h"

tbl_vlanclass_rule_master_t *_g_p_tbl_vlanclass_rule_master = NULL;

int32
tbl_vlanclass_rule_add_vlanclass_rule_sync(tbl_vlanclass_rule_t *p_vlanclass_rule, uint32 sync)
{
    tbl_vlanclass_rule_master_t *p_master = _g_p_tbl_vlanclass_rule_master;
    tbl_vlanclass_rule_t *p_db_vlanclass_rule = NULL;

    /* 1. lookup entry exist */
    if (tbl_vlanclass_rule_get_vlanclass_rule(&p_vlanclass_rule->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_vlanclass_rule = XCALLOC(MEM_TBL_VLANCLASS_RULE, sizeof(tbl_vlanclass_rule_t));
    if (NULL == p_db_vlanclass_rule)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_vlanclass_rule, p_vlanclass_rule, sizeof(tbl_vlanclass_rule_t));

    /* 4. add to db */
    p_master->vlanclass_rule_array[p_vlanclass_rule->key.rule_id] = p_db_vlanclass_rule;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_VLANCLASS_RULE, p_db_vlanclass_rule);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_VLANCLASS_RULE, p_db_vlanclass_rule);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_vlanclass_rule_del_vlanclass_rule_sync(tbl_vlanclass_rule_key_t *p_vlanclass_rule_key, uint32 sync)
{
    tbl_vlanclass_rule_master_t *p_master = _g_p_tbl_vlanclass_rule_master;
    tbl_vlanclass_rule_t *p_db_vlanclass_rule = NULL;

    /* 1. lookup entry exist */
    p_db_vlanclass_rule = tbl_vlanclass_rule_get_vlanclass_rule(p_vlanclass_rule_key);
    if (NULL == p_db_vlanclass_rule)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_VLANCLASS_RULE, p_db_vlanclass_rule);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_VLANCLASS_RULE, p_db_vlanclass_rule);
        #endif 
    }

    /* 3. delete from db */
    p_master->vlanclass_rule_array[p_vlanclass_rule_key->rule_id] = NULL;

    /* 4. free db entry */
    XFREE(MEM_TBL_VLANCLASS_RULE, p_db_vlanclass_rule);

    return PM_E_NONE;
}

int32
tbl_vlanclass_rule_set_vlanclass_rule_field_sync(tbl_vlanclass_rule_t *p_vlanclass_rule, tbl_vlanclass_rule_field_id_t field_id, uint32 sync)
{
    tbl_vlanclass_rule_t *p_db_vlanclass_rule = NULL;

    /* 1. lookup entry exist */
    p_db_vlanclass_rule = tbl_vlanclass_rule_get_vlanclass_rule(&p_vlanclass_rule->key);
    if (NULL == p_db_vlanclass_rule)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_VLANCLASS_RULE_FLD_RULE_TYPE:
        p_db_vlanclass_rule->rule_type = p_vlanclass_rule->rule_type;
        break;

    case TBL_VLANCLASS_RULE_FLD_VLAN_ID:
        p_db_vlanclass_rule->vlan_id = p_vlanclass_rule->vlan_id;
        break;

    case TBL_VLANCLASS_RULE_FLD_MAC:
        sal_memcpy(p_db_vlanclass_rule->mac, p_vlanclass_rule->mac, sizeof(p_vlanclass_rule->mac));
        break;

    case TBL_VLANCLASS_RULE_FLD_IP_ADDRESS:
        sal_memcpy(&p_db_vlanclass_rule->ip_address, &p_vlanclass_rule->ip_address, sizeof(p_vlanclass_rule->ip_address));
        break;

    case TBL_VLANCLASS_RULE_FLD_PROTOCOL_TYPE:
        p_db_vlanclass_rule->protocol_type = p_vlanclass_rule->protocol_type;
        break;

    case TBL_VLANCLASS_RULE_FLD_PROTOCOL_ACTION:
        p_db_vlanclass_rule->protocol_action = p_vlanclass_rule->protocol_action;
        break;

    case TBL_VLANCLASS_RULE_FLD_RULE_OID:
        p_db_vlanclass_rule->rule_oid = p_vlanclass_rule->rule_oid;
        break;

    case TBL_VLANCLASS_RULE_FLD_MAX:
        sal_memcpy(p_db_vlanclass_rule, p_vlanclass_rule, sizeof(tbl_vlanclass_rule_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_VLANCLASS_RULE, field_id, p_db_vlanclass_rule);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_VLANCLASS_RULE, field_id, p_db_vlanclass_rule);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_vlanclass_rule_add_vlanclass_rule(tbl_vlanclass_rule_t *p_vlanclass_rule)
{
    return tbl_vlanclass_rule_add_vlanclass_rule_sync(p_vlanclass_rule, TRUE);
}

int32
tbl_vlanclass_rule_del_vlanclass_rule(tbl_vlanclass_rule_key_t *p_vlanclass_rule_key)
{
    return tbl_vlanclass_rule_del_vlanclass_rule_sync(p_vlanclass_rule_key, TRUE);
}

int32
tbl_vlanclass_rule_set_vlanclass_rule_field(tbl_vlanclass_rule_t *p_vlanclass_rule, tbl_vlanclass_rule_field_id_t field_id)
{
    return tbl_vlanclass_rule_set_vlanclass_rule_field_sync(p_vlanclass_rule, field_id, TRUE);
}

tbl_vlanclass_rule_t*
tbl_vlanclass_rule_get_vlanclass_rule(tbl_vlanclass_rule_key_t *p_vlanclass_rule_key)
{
    tbl_vlanclass_rule_master_t *p_master = _g_p_tbl_vlanclass_rule_master;

    if (GLB_VLANCLASS_RULE_ID_MAX <= p_vlanclass_rule_key->rule_id)
    {
        return NULL;
    }
    return (p_master->vlanclass_rule_array[p_vlanclass_rule_key->rule_id]);
}

char*
tbl_vlanclass_rule_key_val2str(tbl_vlanclass_rule_t *p_vlanclass_rule, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_vlanclass_rule->key.rule_id);
    return str;
}

int32
tbl_vlanclass_rule_key_str2val(char *str, tbl_vlanclass_rule_t *p_vlanclass_rule)
{
    int32 ret = 0;

    p_vlanclass_rule->key.rule_id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_vlanclass_rule_key_name_dump(tbl_vlanclass_rule_t *p_vlanclass_rule, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_VLANCLASS_RULE);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_vlanclass_rule->key.rule_id);
    return str;
}

char*
tbl_vlanclass_rule_key_value_dump(tbl_vlanclass_rule_t *p_vlanclass_rule, char *str)
{
    sal_sprintf(str, "%u", p_vlanclass_rule->key.rule_id);
    return str;
}

char*
tbl_vlanclass_rule_field_name_dump(tbl_vlanclass_rule_t *p_vlanclass_rule, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_VLANCLASS_RULE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_VLANCLASS_RULE_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_vlanclass_rule_field_value_dump(tbl_vlanclass_rule_t *p_vlanclass_rule, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_VLANCLASS_RULE_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_vlanclass_rule->key.rule_id);
    }
    if (FLD_MATCH(TBL_VLANCLASS_RULE_FLD_RULE_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_vlanclass_rule->rule_type);
    }
    if (FLD_MATCH(TBL_VLANCLASS_RULE_FLD_VLAN_ID, field_id))
    {
        sal_sprintf(str, "%u", p_vlanclass_rule->vlan_id);
    }
    if (FLD_MATCH(TBL_VLANCLASS_RULE_FLD_MAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_vlanclass_rule->mac);
    }
    if (FLD_MATCH(TBL_VLANCLASS_RULE_FLD_IP_ADDRESS, field_id))
    {
        cdb_addr_val2str(str, MAX_CMD_STR_LEN, &p_vlanclass_rule->ip_address);
    }
    if (FLD_MATCH(TBL_VLANCLASS_RULE_FLD_PROTOCOL_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_vlanclass_rule->protocol_type);
    }
    if (FLD_MATCH(TBL_VLANCLASS_RULE_FLD_PROTOCOL_ACTION, field_id))
    {
        sal_sprintf(str, "%u", p_vlanclass_rule->protocol_action);
    }
    if (FLD_MATCH(TBL_VLANCLASS_RULE_FLD_RULE_OID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_vlanclass_rule->rule_oid);
    }
    return str;
}

char**
tbl_vlanclass_rule_table_dump(tbl_vlanclass_rule_t *p_vlanclass_rule, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_vlanclass_rule_key_name_dump(p_vlanclass_rule, buf));
    for(i=1; i<TBL_VLANCLASS_RULE_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_vlanclass_rule_field_name_dump(p_vlanclass_rule, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_vlanclass_rule_field_value_dump(p_vlanclass_rule, i, buf));
    }
    return str;
}

int32
tbl_vlanclass_rule_field_value_parser(char *str, int32 field_id, tbl_vlanclass_rule_t *p_vlanclass_rule)
{
    if (FLD_MATCH(TBL_VLANCLASS_RULE_FLD_KEY, field_id))
    {
        int32 ret;
        p_vlanclass_rule->key.rule_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VLANCLASS_RULE_FLD_RULE_TYPE, field_id))
    {
        int32 ret;
        p_vlanclass_rule->rule_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VLANCLASS_RULE_FLD_VLAN_ID, field_id))
    {
        int32 ret;
        p_vlanclass_rule->vlan_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VLANCLASS_RULE_FLD_MAC, field_id))
    {
        cdb_mac_addr_str2val(p_vlanclass_rule->mac, str);
    }
    if (FLD_MATCH(TBL_VLANCLASS_RULE_FLD_IP_ADDRESS, field_id))
    {
        cdb_addr_str2val(&p_vlanclass_rule->ip_address, str, 0);
    }
    if (FLD_MATCH(TBL_VLANCLASS_RULE_FLD_PROTOCOL_TYPE, field_id))
    {
        int32 ret;
        p_vlanclass_rule->protocol_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VLANCLASS_RULE_FLD_PROTOCOL_ACTION, field_id))
    {
        int32 ret;
        p_vlanclass_rule->protocol_action = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VLANCLASS_RULE_FLD_RULE_OID, field_id))
    {
        int32 ret;
        p_vlanclass_rule->rule_oid = cdb_uint64_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_vlanclass_rule_table_parser(char** array, char* key_value,tbl_vlanclass_rule_t *p_vlanclass_rule)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_VLANCLASS_RULE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_vlanclass_rule_key_str2val(key_value, p_vlanclass_rule));

    for(i=1; i<TBL_VLANCLASS_RULE_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_vlanclass_rule_field_value_parser( array[j++], i, p_vlanclass_rule));
    }

    return PM_E_NONE;
}

int32
tbl_vlanclass_rule_dump_one(tbl_vlanclass_rule_t *p_vlanclass_rule, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_VLANCLASS_RULE);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_VLANCLASS_RULE_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VLANCLASS_RULE_FLD_KEY].name,
            p_vlanclass_rule->key.rule_id);
    }
    if (FLD_MATCH(TBL_VLANCLASS_RULE_FLD_RULE_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VLANCLASS_RULE_FLD_RULE_TYPE].name,
            p_vlanclass_rule->rule_type);
    }
    if (FLD_MATCH(TBL_VLANCLASS_RULE_FLD_VLAN_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VLANCLASS_RULE_FLD_VLAN_ID].name,
            p_vlanclass_rule->vlan_id);
    }
    if (FLD_MATCH(TBL_VLANCLASS_RULE_FLD_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VLANCLASS_RULE_FLD_MAC].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_vlanclass_rule->mac));
    }
    if (FLD_MATCH(TBL_VLANCLASS_RULE_FLD_IP_ADDRESS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VLANCLASS_RULE_FLD_IP_ADDRESS].name, 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_vlanclass_rule->ip_address));
    }
    if (FLD_MATCH(TBL_VLANCLASS_RULE_FLD_PROTOCOL_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VLANCLASS_RULE_FLD_PROTOCOL_TYPE].name,
            p_vlanclass_rule->protocol_type);
    }
    if (FLD_MATCH(TBL_VLANCLASS_RULE_FLD_PROTOCOL_ACTION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VLANCLASS_RULE_FLD_PROTOCOL_ACTION].name,
            p_vlanclass_rule->protocol_action);
    }
    if (FLD_MATCH(TBL_VLANCLASS_RULE_FLD_RULE_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_VLANCLASS_RULE_FLD_RULE_OID].name,
            p_vlanclass_rule->rule_oid);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_vlanclass_rule_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_vlanclass_rule_master_t *p_master = _g_p_tbl_vlanclass_rule_master;
    tbl_vlanclass_rule_t *p_db_vlanclass_rule = NULL;
    uint32 rule_id = 0;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    for (rule_id = 0; rule_id < GLB_VLANCLASS_RULE_ID_MAX; rule_id++)
    {
        p_db_vlanclass_rule = p_master->vlanclass_rule_array[rule_id];
        if (NULL == p_db_vlanclass_rule)
        {
            continue;
        }
        rc |= fn(p_db_vlanclass_rule, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_vlanclass_rule_master_t*
tbl_vlanclass_rule_get_master()
{
    return _g_p_tbl_vlanclass_rule_master;
}

tbl_vlanclass_rule_master_t*
tbl_vlanclass_rule_init_vlanclass_rule()
{
    _g_p_tbl_vlanclass_rule_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_vlanclass_rule_master_t));
    return _g_p_tbl_vlanclass_rule_master;
}

