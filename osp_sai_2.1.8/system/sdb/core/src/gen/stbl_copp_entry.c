
#include "sdb.h"
#include "gen/stbl_copp_entry_define.h"
#include "gen/stbl_copp_entry.h"
#include "sdb_data_cmp.h"

stbl_copp_entry_master_t *_g_p_stbl_copp_entry_master = NULL;

int32
stbl_copp_entry_add_copp_entry_sync(stbl_copp_entry_t *p_copp_entry, uint32 sync)
{
    stbl_copp_entry_master_t *p_master = _g_p_stbl_copp_entry_master;
    stbl_copp_entry_t *p_db_copp_entry = NULL;

    /* 1. lookup entry exist */
    if (stbl_copp_entry_get_copp_entry(&p_copp_entry->key))
    {
        return SDB_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_copp_entry = XCALLOC(MEM_STBL_COPP_ENTRY, sizeof(stbl_copp_entry_t));
    if (NULL == p_db_copp_entry)
    {
        return SDB_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_copp_entry, p_copp_entry, sizeof(stbl_copp_entry_t));

    /* 4. add to db */
    p_master->array[p_copp_entry->key.copp_entry_id] = p_db_copp_entry;

    return SDB_E_NONE;
}

int32
stbl_copp_entry_del_copp_entry_sync(stbl_copp_entry_key_t *p_copp_entry_key, uint32 sync)
{
    stbl_copp_entry_master_t *p_master = _g_p_stbl_copp_entry_master;
    stbl_copp_entry_t *p_db_copp_entry = NULL;

    /* 1. lookup entry exist */
    p_db_copp_entry = stbl_copp_entry_get_copp_entry(p_copp_entry_key);
    if (NULL == p_db_copp_entry)
    {
        return SDB_E_NONE;
    }

    /* 3. delete from db */
    p_master->array[p_copp_entry_key->copp_entry_id] = NULL;

    /* 4. free db entry */
    XFREE(MEM_STBL_COPP_ENTRY, p_db_copp_entry);

    return SDB_E_NONE;
}

int32
stbl_copp_entry_set_copp_entry_field_sync(stbl_copp_entry_t *p_copp_entry, stbl_copp_entry_field_id_t field_id, uint32 sync)
{
    stbl_copp_entry_t *p_db_copp_entry = NULL;

    /* 1. lookup entry exist */
    p_db_copp_entry = stbl_copp_entry_get_copp_entry(&p_copp_entry->key);
    if (NULL == p_db_copp_entry)
    {
        return SDB_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case STBL_COPP_ENTRY_FLD_ENTRY_OID:
        p_db_copp_entry->entry_oid = p_copp_entry->entry_oid;
        break;

    case STBL_COPP_ENTRY_FLD_COUNTER_OID:
        p_db_copp_entry->counter_oid = p_copp_entry->counter_oid;
        break;

    case STBL_COPP_ENTRY_FLD_NH_OID:
        p_db_copp_entry->nh_oid = p_copp_entry->nh_oid;
        break;

    case STBL_COPP_ENTRY_FLD_TYPE:
        p_db_copp_entry->type = p_copp_entry->type;
        break;

    case STBL_COPP_ENTRY_FLD_IP_TYPE:
        p_db_copp_entry->ip_type = p_copp_entry->ip_type;
        break;

    case STBL_COPP_ENTRY_FLD_PROTOCOL_NUM:
        p_db_copp_entry->protocol_num = p_copp_entry->protocol_num;
        break;

    case STBL_COPP_ENTRY_FLD_DST_PORT:
        p_db_copp_entry->dst_port = p_copp_entry->dst_port;
        break;

    case STBL_COPP_ENTRY_FLD_MAX:
        sal_memcpy(p_db_copp_entry, p_copp_entry, sizeof(stbl_copp_entry_t));
        break;
    default:
        return SDB_E_NOT_SUPPORT;
    }

    return SDB_E_NONE;
}

int32
stbl_copp_entry_add_copp_entry(stbl_copp_entry_t *p_copp_entry)
{
    return stbl_copp_entry_add_copp_entry_sync(p_copp_entry, TRUE);
}

int32
stbl_copp_entry_del_copp_entry(stbl_copp_entry_key_t *p_copp_entry_key)
{
    return stbl_copp_entry_del_copp_entry_sync(p_copp_entry_key, TRUE);
}

int32
stbl_copp_entry_set_copp_entry_field(stbl_copp_entry_t *p_copp_entry, stbl_copp_entry_field_id_t field_id)
{
    return stbl_copp_entry_set_copp_entry_field_sync(p_copp_entry, field_id, TRUE);
}

stbl_copp_entry_t*
stbl_copp_entry_get_copp_entry(stbl_copp_entry_key_t *p_copp_entry_key)
{
    stbl_copp_entry_master_t *p_master = _g_p_stbl_copp_entry_master;

    if (GLB_COPP_ENTRY_MAX <= p_copp_entry_key->copp_entry_id)
    {
        return NULL;
    }
    return (p_master->array[p_copp_entry_key->copp_entry_id]);
}

char*
stbl_copp_entry_key_val2str(stbl_copp_entry_t *p_copp_entry, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_copp_entry->key.copp_entry_id);
    return str;
}

int32
stbl_copp_entry_dump_one(stbl_copp_entry_t *p_copp_entry, stbl_iter_args_t *pargs)
{
    sdb_node_t *p_node = sdb_get_tbl(STBL_COPP_ENTRY);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    sdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[SDB_MAX_CMD_STR_LEN];

    if (SDB_FLD_MATCH(STBL_COPP_ENTRY_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_COPP_ENTRY_FLD_KEY].name,
            p_copp_entry->key.copp_entry_id);
    }
    if (SDB_FLD_MATCH(STBL_COPP_ENTRY_FLD_ENTRY_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_COPP_ENTRY_FLD_ENTRY_OID].name, 
            sdb_object_id_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_copp_entry->entry_oid));
    }
    if (SDB_FLD_MATCH(STBL_COPP_ENTRY_FLD_COUNTER_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_COPP_ENTRY_FLD_COUNTER_OID].name, 
            sdb_object_id_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_copp_entry->counter_oid));
    }
    if (SDB_FLD_MATCH(STBL_COPP_ENTRY_FLD_NH_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_COPP_ENTRY_FLD_NH_OID].name, 
            sdb_object_id_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_copp_entry->nh_oid));
    }
    if (SDB_FLD_MATCH(STBL_COPP_ENTRY_FLD_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_COPP_ENTRY_FLD_TYPE].name,
            p_copp_entry->type);
    }
    if (SDB_FLD_MATCH(STBL_COPP_ENTRY_FLD_IP_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_COPP_ENTRY_FLD_IP_TYPE].name,
            p_copp_entry->ip_type);
    }
    if (SDB_FLD_MATCH(STBL_COPP_ENTRY_FLD_PROTOCOL_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_COPP_ENTRY_FLD_PROTOCOL_NUM].name,
            p_copp_entry->protocol_num);
    }
    if (SDB_FLD_MATCH(STBL_COPP_ENTRY_FLD_DST_PORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_COPP_ENTRY_FLD_DST_PORT].name,
            p_copp_entry->dst_port);
    }

    sal_fprintf(fp, "\n");

    return SDB_E_NONE;
}

int32
stbl_copp_entry_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs)
{
    stbl_copp_entry_master_t *p_master = _g_p_stbl_copp_entry_master;
    stbl_copp_entry_t *p_db_copp_entry = NULL;
    uint32 copp_entry_id = 0;
    int32 rc = SDB_E_NONE;

    if (NULL == p_master)
    {
        return SDB_E_NONE;
    }

    for (copp_entry_id = 0; copp_entry_id < GLB_COPP_ENTRY_MAX; copp_entry_id++)
    {
        p_db_copp_entry = p_master->array[copp_entry_id];
        if (NULL == p_db_copp_entry)
        {
            continue;
        }
        rc |= fn(p_db_copp_entry, pargs);
    }

    return (rc) ? SDB_E_FAIL : SDB_E_NONE;
}

stbl_copp_entry_master_t*
stbl_copp_entry_get_master()
{
    return _g_p_stbl_copp_entry_master;
}

stbl_copp_entry_master_t*
stbl_copp_entry_init_copp_entry()
{
    _g_p_stbl_copp_entry_master = XCALLOC(MEM_TBL_MASTER, sizeof(stbl_copp_entry_master_t));
    return _g_p_stbl_copp_entry_master;
}

