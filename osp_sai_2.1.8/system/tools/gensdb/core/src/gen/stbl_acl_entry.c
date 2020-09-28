
#include "sdb.h"
#include "gen/stbl_acl_entry_define.h"
#include "gen/stbl_acl_entry.h"
#include "sdb_data_cmp.h"

stbl_acl_entry_master_t *_g_p_stbl_acl_entry_master = NULL;

int32
stbl_acl_entry_add_acl_entry_sync(stbl_acl_entry_t *p_ace, uint32 sync)
{
    stbl_acl_entry_master_t *p_master = _g_p_stbl_acl_entry_master;
    stbl_acl_entry_t *p_db_ace = NULL;

    /* 1. lookup entry exist */
    if (stbl_acl_entry_get_acl_entry(&p_ace->key))
    {
        return SDB_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_ace = XCALLOC(MEM_STBL_ACL_ENTRY, sizeof(stbl_acl_entry_t));
    if (NULL == p_db_ace)
    {
        return SDB_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_ace, p_ace, sizeof(stbl_acl_entry_t));

    /* 4. add to db */
    p_master->array[p_ace->key.entry_id] = p_db_ace;

    return SDB_E_NONE;
}

int32
stbl_acl_entry_del_acl_entry_sync(stbl_acl_entry_key_t *p_ace_key, uint32 sync)
{
    stbl_acl_entry_master_t *p_master = _g_p_stbl_acl_entry_master;
    stbl_acl_entry_t *p_db_ace = NULL;

    /* 1. lookup entry exist */
    p_db_ace = stbl_acl_entry_get_acl_entry(p_ace_key);
    if (NULL == p_db_ace)
    {
        return SDB_E_NONE;
    }

    /* 3. delete from db */
    p_master->array[p_ace_key->entry_id] = NULL;

    /* 4. free db entry */
    XFREE(MEM_STBL_ACL_ENTRY, p_db_ace);

    return SDB_E_NONE;
}

int32
stbl_acl_entry_set_acl_entry_field_sync(stbl_acl_entry_t *p_ace, stbl_acl_entry_field_id_t field_id, uint32 sync)
{
    stbl_acl_entry_t *p_db_ace = NULL;

    /* 1. lookup entry exist */
    p_db_ace = stbl_acl_entry_get_acl_entry(&p_ace->key);
    if (NULL == p_db_ace)
    {
        return SDB_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case STBL_ACL_ENTRY_FLD_TBL_OID:
        p_db_ace->tbl_oid = p_ace->tbl_oid;
        break;

    case STBL_ACL_ENTRY_FLD_MIRROR_OID:
        p_db_ace->mirror_oid = p_ace->mirror_oid;
        break;

    case STBL_ACL_ENTRY_FLD_PRIORITY:
        p_db_ace->priority = p_ace->priority;
        break;

    case STBL_ACL_ENTRY_FLD_DIRCTION:
        p_db_ace->dirction = p_ace->dirction;
        break;

    case STBL_ACL_ENTRY_FLD_ENABLE:
        p_db_ace->enable = p_ace->enable;
        break;

    case STBL_ACL_ENTRY_FLD_APPLY_INNER_LOOKUP:
        p_db_ace->apply_inner_lookup = p_ace->apply_inner_lookup;
        break;

    case STBL_ACL_ENTRY_FLD_IN_PORT:
        sal_memcpy(&p_db_ace->in_port, &p_ace->in_port, sizeof(p_ace->in_port));
        break;

    case STBL_ACL_ENTRY_FLD_CTC_ENTRY_ID_LIST:
        sal_memcpy(p_db_ace->ctc_entry_id_list, p_ace->ctc_entry_id_list, sizeof(p_ace->ctc_entry_id_list));
        break;

    case STBL_ACL_ENTRY_FLD_CTC_ENTRY_ID_COUNT:
        p_db_ace->ctc_entry_id_count = p_ace->ctc_entry_id_count;
        break;

    case STBL_ACL_ENTRY_FLD_PORT_OID_LIST:
        sal_memcpy(p_db_ace->port_oid_list, p_ace->port_oid_list, sizeof(p_ace->port_oid_list));
        break;

    case STBL_ACL_ENTRY_FLD_PORT_OID_COUNT:
        p_db_ace->port_oid_count = p_ace->port_oid_count;
        break;

    case STBL_ACL_ENTRY_FLD_OUTER_FIELDS:
        sal_memcpy(&p_db_ace->outer_fields, &p_ace->outer_fields, sizeof(p_ace->outer_fields));
        break;

    case STBL_ACL_ENTRY_FLD_INNER_FIELDS:
        sal_memcpy(&p_db_ace->inner_fields, &p_ace->inner_fields, sizeof(p_ace->inner_fields));
        break;

    case STBL_ACL_ENTRY_FLD_ACTION:
        sal_memcpy(&p_db_ace->action, &p_ace->action, sizeof(p_ace->action));
        break;

    case STBL_ACL_ENTRY_FLD_ACTION_ATTR:
        sal_memcpy(&p_db_ace->action_attr, &p_ace->action_attr, sizeof(p_ace->action_attr));
        break;

    case STBL_ACL_ENTRY_FLD_TAP:
        sal_memcpy(&p_db_ace->tap, &p_ace->tap, sizeof(p_ace->tap));
        break;

    case STBL_ACL_ENTRY_FLD_MAX:
        sal_memcpy(p_db_ace, p_ace, sizeof(stbl_acl_entry_t));
        break;
    default:
        return SDB_E_NOT_SUPPORT;
    }

    return SDB_E_NONE;
}

int32
stbl_acl_entry_add_acl_entry(stbl_acl_entry_t *p_ace)
{
    return stbl_acl_entry_add_acl_entry_sync(p_ace, TRUE);
}

int32
stbl_acl_entry_del_acl_entry(stbl_acl_entry_key_t *p_ace_key)
{
    return stbl_acl_entry_del_acl_entry_sync(p_ace_key, TRUE);
}

int32
stbl_acl_entry_set_acl_entry_field(stbl_acl_entry_t *p_ace, stbl_acl_entry_field_id_t field_id)
{
    return stbl_acl_entry_set_acl_entry_field_sync(p_ace, field_id, TRUE);
}

stbl_acl_entry_t*
stbl_acl_entry_get_acl_entry(stbl_acl_entry_key_t *p_ace_key)
{
    stbl_acl_entry_master_t *p_master = _g_p_stbl_acl_entry_master;

    if (GLB_SAI_ACE_MAX <= p_ace_key->entry_id)
    {
        return NULL;
    }
    return (p_master->array[p_ace_key->entry_id]);
}

char*
stbl_acl_entry_key_val2str(stbl_acl_entry_t *p_ace, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_ace->key.entry_id);
    return str;
}

int32
stbl_acl_entry_dump_one(stbl_acl_entry_t *p_ace, stbl_iter_args_t *pargs)
{
    sdb_node_t *p_node = sdb_get_tbl(STBL_ACL_ENTRY);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    sdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[SDB_MAX_CMD_STR_LEN];

    if (SDB_FLD_MATCH(STBL_ACL_ENTRY_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_ACL_ENTRY_FLD_KEY].name,
            p_ace->key.entry_id);
    }
    if (SDB_FLD_MATCH(STBL_ACL_ENTRY_FLD_TBL_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_ACL_ENTRY_FLD_TBL_OID].name, 
            sdb_object_id_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_ace->tbl_oid));
    }
    if (SDB_FLD_MATCH(STBL_ACL_ENTRY_FLD_MIRROR_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_ACL_ENTRY_FLD_MIRROR_OID].name, 
            sdb_object_id_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_ace->mirror_oid));
    }
    if (SDB_FLD_MATCH(STBL_ACL_ENTRY_FLD_PRIORITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_ACL_ENTRY_FLD_PRIORITY].name,
            p_ace->priority);
    }
    if (SDB_FLD_MATCH(STBL_ACL_ENTRY_FLD_DIRCTION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_ACL_ENTRY_FLD_DIRCTION].name,
            p_ace->dirction);
    }
    if (SDB_FLD_MATCH(STBL_ACL_ENTRY_FLD_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_ACL_ENTRY_FLD_ENABLE].name,
            p_ace->enable);
    }
    if (SDB_FLD_MATCH(STBL_ACL_ENTRY_FLD_APPLY_INNER_LOOKUP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_ACL_ENTRY_FLD_APPLY_INNER_LOOKUP].name,
            p_ace->apply_inner_lookup);
    }
    if (SDB_FLD_MATCH(STBL_ACL_ENTRY_FLD_IN_PORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_ACL_ENTRY_FLD_IN_PORT].name, 
            sdb_sai_acl_field_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_ace->in_port));
    }
    if (SDB_FLD_MATCH(STBL_ACL_ENTRY_FLD_CTC_ENTRY_ID_LIST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_ACL_ENTRY_FLD_CTC_ENTRY_ID_LIST].name, 
            sdb_uint32_array_val2str(buf, SDB_MAX_CMD_STR_LEN, p_ace->ctc_entry_id_list, sizeof(p_ace->ctc_entry_id_list)/4));
    }
    if (SDB_FLD_MATCH(STBL_ACL_ENTRY_FLD_CTC_ENTRY_ID_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_ACL_ENTRY_FLD_CTC_ENTRY_ID_COUNT].name,
            p_ace->ctc_entry_id_count);
    }
    if (SDB_FLD_MATCH(STBL_ACL_ENTRY_FLD_PORT_OID_LIST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_ACL_ENTRY_FLD_PORT_OID_LIST].name, 
            sdb_uint64_array_val2str(buf, SDB_MAX_CMD_STR_LEN, p_ace->port_oid_list, sizeof(p_ace->port_oid_list)/4));
    }
    if (SDB_FLD_MATCH(STBL_ACL_ENTRY_FLD_PORT_OID_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_ACL_ENTRY_FLD_PORT_OID_COUNT].name,
            p_ace->port_oid_count);
    }
    if (SDB_FLD_MATCH(STBL_ACL_ENTRY_FLD_OUTER_FIELDS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_ACL_ENTRY_FLD_OUTER_FIELDS].name, 
            sdb_sai_acl_field_list_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_ace->outer_fields));
    }
    if (SDB_FLD_MATCH(STBL_ACL_ENTRY_FLD_INNER_FIELDS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_ACL_ENTRY_FLD_INNER_FIELDS].name, 
            sdb_sai_acl_field_list_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_ace->inner_fields));
    }
    if (SDB_FLD_MATCH(STBL_ACL_ENTRY_FLD_ACTION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_ACL_ENTRY_FLD_ACTION].name, 
            sdb_sai_acl_action_list_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_ace->action));
    }
    if (SDB_FLD_MATCH(STBL_ACL_ENTRY_FLD_ACTION_ATTR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_ACL_ENTRY_FLD_ACTION_ATTR].name, 
            sdb_sai_acl_action_attrlist_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_ace->action_attr));
    }
    if (SDB_FLD_MATCH(STBL_ACL_ENTRY_FLD_TAP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_ACL_ENTRY_FLD_TAP].name, 
            sdb_sai_acl_tap_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_ace->tap));
    }

    sal_fprintf(fp, "\n");

    return SDB_E_NONE;
}

int32
stbl_acl_entry_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs)
{
    stbl_acl_entry_master_t *p_master = _g_p_stbl_acl_entry_master;
    stbl_acl_entry_t *p_db_ace = NULL;
    uint32 entry_id = 0;
    int32 rc = SDB_E_NONE;

    if (NULL == p_master)
    {
        return SDB_E_NONE;
    }

    for (entry_id = 0; entry_id < GLB_SAI_ACE_MAX; entry_id++)
    {
        p_db_ace = p_master->array[entry_id];
        if (NULL == p_db_ace)
        {
            continue;
        }
        rc |= fn(p_db_ace, pargs);
    }

    return (rc) ? SDB_E_FAIL : SDB_E_NONE;
}

stbl_acl_entry_master_t*
stbl_acl_entry_get_master()
{
    return _g_p_stbl_acl_entry_master;
}

stbl_acl_entry_master_t*
stbl_acl_entry_init_acl_entry()
{
    _g_p_stbl_acl_entry_master = XCALLOC(MEM_TBL_MASTER, sizeof(stbl_acl_entry_master_t));
    return _g_p_stbl_acl_entry_master;
}

