
#include "sdb.h"
#include "gen/stbl_acl_udf_entry_define.h"
#include "gen/stbl_acl_udf_entry.h"
#include "sdb_data_cmp.h"

stbl_acl_udf_entry_master_t *_g_p_stbl_acl_udf_entry_master = NULL;

int32
stbl_acl_udf_entry_add_acl_udf_entry_sync(stbl_acl_udf_entry_t *p_acl_udf_entry, uint32 sync)
{
    stbl_acl_udf_entry_master_t *p_master = _g_p_stbl_acl_udf_entry_master;
    stbl_acl_udf_entry_t *p_db_acl_udf_entry = NULL;

    /* 1. lookup entry exist */
    if (stbl_acl_udf_entry_get_acl_udf_entry(&p_acl_udf_entry->key))
    {
        return SDB_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_acl_udf_entry = XCALLOC(MEM_STBL_ACL_UDF_ENTRY, sizeof(stbl_acl_udf_entry_t));
    if (NULL == p_db_acl_udf_entry)
    {
        return SDB_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_acl_udf_entry, p_acl_udf_entry, sizeof(stbl_acl_udf_entry_t));

    /* 4. add to db */
    p_master->array[p_acl_udf_entry->key.id] = p_db_acl_udf_entry;

    return SDB_E_NONE;
}

int32
stbl_acl_udf_entry_del_acl_udf_entry_sync(stbl_acl_udf_entry_key_t *p_acl_udf_entry_key, uint32 sync)
{
    stbl_acl_udf_entry_master_t *p_master = _g_p_stbl_acl_udf_entry_master;
    stbl_acl_udf_entry_t *p_db_acl_udf_entry = NULL;

    /* 1. lookup entry exist */
    p_db_acl_udf_entry = stbl_acl_udf_entry_get_acl_udf_entry(p_acl_udf_entry_key);
    if (NULL == p_db_acl_udf_entry)
    {
        return SDB_E_NONE;
    }

    /* 3. delete from db */
    p_master->array[p_acl_udf_entry_key->id] = NULL;

    /* 4. free db entry */
    XFREE(MEM_STBL_ACL_UDF_ENTRY, p_db_acl_udf_entry);

    return SDB_E_NONE;
}

int32
stbl_acl_udf_entry_set_acl_udf_entry_field_sync(stbl_acl_udf_entry_t *p_acl_udf_entry, stbl_acl_udf_entry_field_id_t field_id, uint32 sync)
{
    stbl_acl_udf_entry_t *p_db_acl_udf_entry = NULL;

    /* 1. lookup entry exist */
    p_db_acl_udf_entry = stbl_acl_udf_entry_get_acl_udf_entry(&p_acl_udf_entry->key);
    if (NULL == p_db_acl_udf_entry)
    {
        return SDB_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case STBL_ACL_UDF_ENTRY_FLD_GROUP_TYPE:
        p_db_acl_udf_entry->group_type = p_acl_udf_entry->group_type;
        break;

    case STBL_ACL_UDF_ENTRY_FLD_GROUP_LENGTH:
        p_db_acl_udf_entry->group_length = p_acl_udf_entry->group_length;
        break;

    case STBL_ACL_UDF_ENTRY_FLD_OFFSET_TYPE:
        p_db_acl_udf_entry->offset_type = p_acl_udf_entry->offset_type;
        break;

    case STBL_ACL_UDF_ENTRY_FLD_OFFSET_NUM:
        p_db_acl_udf_entry->offset_num = p_acl_udf_entry->offset_num;
        break;

    case STBL_ACL_UDF_ENTRY_FLD_OFFSET:
        sal_memcpy(p_db_acl_udf_entry->offset, p_acl_udf_entry->offset, sizeof(p_acl_udf_entry->offset));
        break;

    case STBL_ACL_UDF_ENTRY_FLD_INSTALLED:
        p_db_acl_udf_entry->installed = p_acl_udf_entry->installed;
        break;

    case STBL_ACL_UDF_ENTRY_FLD_KEY_FLAGS:
        p_db_acl_udf_entry->key_flags = p_acl_udf_entry->key_flags;
        break;

    case STBL_ACL_UDF_ENTRY_FLD_ETHTYPE:
        sal_memcpy(p_db_acl_udf_entry->ethtype, p_acl_udf_entry->ethtype, sizeof(p_acl_udf_entry->ethtype));
        break;

    case STBL_ACL_UDF_ENTRY_FLD_VLANNUM:
        sal_memcpy(p_db_acl_udf_entry->vlannum, p_acl_udf_entry->vlannum, sizeof(p_acl_udf_entry->vlannum));
        break;

    case STBL_ACL_UDF_ENTRY_FLD_IPPROTO:
        sal_memcpy(p_db_acl_udf_entry->ipproto, p_acl_udf_entry->ipproto, sizeof(p_acl_udf_entry->ipproto));
        break;

    case STBL_ACL_UDF_ENTRY_FLD_L4_TYPE:
        sal_memcpy(p_db_acl_udf_entry->l4_type, p_acl_udf_entry->l4_type, sizeof(p_acl_udf_entry->l4_type));
        break;

    case STBL_ACL_UDF_ENTRY_FLD_SRCPORT:
        sal_memcpy(p_db_acl_udf_entry->srcport, p_acl_udf_entry->srcport, sizeof(p_acl_udf_entry->srcport));
        break;

    case STBL_ACL_UDF_ENTRY_FLD_DSTPORT:
        sal_memcpy(p_db_acl_udf_entry->dstport, p_acl_udf_entry->dstport, sizeof(p_acl_udf_entry->dstport));
        break;

    case STBL_ACL_UDF_ENTRY_FLD_MPLSNUM:
        sal_memcpy(p_db_acl_udf_entry->mplsnum, p_acl_udf_entry->mplsnum, sizeof(p_acl_udf_entry->mplsnum));
        break;

    case STBL_ACL_UDF_ENTRY_FLD_MAX:
        sal_memcpy(p_db_acl_udf_entry, p_acl_udf_entry, sizeof(stbl_acl_udf_entry_t));
        break;
    default:
        return SDB_E_NOT_SUPPORT;
    }

    return SDB_E_NONE;
}

int32
stbl_acl_udf_entry_add_acl_udf_entry(stbl_acl_udf_entry_t *p_acl_udf_entry)
{
    return stbl_acl_udf_entry_add_acl_udf_entry_sync(p_acl_udf_entry, TRUE);
}

int32
stbl_acl_udf_entry_del_acl_udf_entry(stbl_acl_udf_entry_key_t *p_acl_udf_entry_key)
{
    return stbl_acl_udf_entry_del_acl_udf_entry_sync(p_acl_udf_entry_key, TRUE);
}

int32
stbl_acl_udf_entry_set_acl_udf_entry_field(stbl_acl_udf_entry_t *p_acl_udf_entry, stbl_acl_udf_entry_field_id_t field_id)
{
    return stbl_acl_udf_entry_set_acl_udf_entry_field_sync(p_acl_udf_entry, field_id, TRUE);
}

stbl_acl_udf_entry_t*
stbl_acl_udf_entry_get_acl_udf_entry(stbl_acl_udf_entry_key_t *p_acl_udf_entry_key)
{
    stbl_acl_udf_entry_master_t *p_master = _g_p_stbl_acl_udf_entry_master;

    if (GLB_ACL_UDF_ENTRY_NUM_MAX+1 <= p_acl_udf_entry_key->id)
    {
        return NULL;
    }
    return (p_master->array[p_acl_udf_entry_key->id]);
}

char*
stbl_acl_udf_entry_key_val2str(stbl_acl_udf_entry_t *p_acl_udf_entry, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_acl_udf_entry->key.id);
    return str;
}

int32
stbl_acl_udf_entry_dump_one(stbl_acl_udf_entry_t *p_acl_udf_entry, stbl_iter_args_t *pargs)
{
    sdb_node_t *p_node = sdb_get_tbl(STBL_ACL_UDF_ENTRY);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    sdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[SDB_MAX_CMD_STR_LEN];

    if (SDB_FLD_MATCH(STBL_ACL_UDF_ENTRY_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_ACL_UDF_ENTRY_FLD_KEY].name,
            p_acl_udf_entry->key.id);
    }
    if (SDB_FLD_MATCH(STBL_ACL_UDF_ENTRY_FLD_GROUP_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_ACL_UDF_ENTRY_FLD_GROUP_TYPE].name,
            p_acl_udf_entry->group_type);
    }
    if (SDB_FLD_MATCH(STBL_ACL_UDF_ENTRY_FLD_GROUP_LENGTH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_ACL_UDF_ENTRY_FLD_GROUP_LENGTH].name,
            p_acl_udf_entry->group_length);
    }
    if (SDB_FLD_MATCH(STBL_ACL_UDF_ENTRY_FLD_OFFSET_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_ACL_UDF_ENTRY_FLD_OFFSET_TYPE].name,
            p_acl_udf_entry->offset_type);
    }
    if (SDB_FLD_MATCH(STBL_ACL_UDF_ENTRY_FLD_OFFSET_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_ACL_UDF_ENTRY_FLD_OFFSET_NUM].name,
            p_acl_udf_entry->offset_num);
    }
    if (SDB_FLD_MATCH(STBL_ACL_UDF_ENTRY_FLD_OFFSET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_ACL_UDF_ENTRY_FLD_OFFSET].name, 
            sdb_uint8_array_val2str(buf, SDB_MAX_CMD_STR_LEN, p_acl_udf_entry->offset, sizeof(p_acl_udf_entry->offset)));
    }
    if (SDB_FLD_MATCH(STBL_ACL_UDF_ENTRY_FLD_INSTALLED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_ACL_UDF_ENTRY_FLD_INSTALLED].name,
            p_acl_udf_entry->installed);
    }
    if (SDB_FLD_MATCH(STBL_ACL_UDF_ENTRY_FLD_KEY_FLAGS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_ACL_UDF_ENTRY_FLD_KEY_FLAGS].name,
            p_acl_udf_entry->key_flags);
    }
    if (SDB_FLD_MATCH(STBL_ACL_UDF_ENTRY_FLD_ETHTYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_ACL_UDF_ENTRY_FLD_ETHTYPE].name, 
            sdb_uint32_array_val2str(buf, SDB_MAX_CMD_STR_LEN, p_acl_udf_entry->ethtype, sizeof(p_acl_udf_entry->ethtype)/4));
    }
    if (SDB_FLD_MATCH(STBL_ACL_UDF_ENTRY_FLD_VLANNUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_ACL_UDF_ENTRY_FLD_VLANNUM].name, 
            sdb_uint32_array_val2str(buf, SDB_MAX_CMD_STR_LEN, p_acl_udf_entry->vlannum, sizeof(p_acl_udf_entry->vlannum)/4));
    }
    if (SDB_FLD_MATCH(STBL_ACL_UDF_ENTRY_FLD_IPPROTO, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_ACL_UDF_ENTRY_FLD_IPPROTO].name, 
            sdb_uint32_array_val2str(buf, SDB_MAX_CMD_STR_LEN, p_acl_udf_entry->ipproto, sizeof(p_acl_udf_entry->ipproto)/4));
    }
    if (SDB_FLD_MATCH(STBL_ACL_UDF_ENTRY_FLD_L4_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_ACL_UDF_ENTRY_FLD_L4_TYPE].name, 
            sdb_uint32_array_val2str(buf, SDB_MAX_CMD_STR_LEN, p_acl_udf_entry->l4_type, sizeof(p_acl_udf_entry->l4_type)/4));
    }
    if (SDB_FLD_MATCH(STBL_ACL_UDF_ENTRY_FLD_SRCPORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_ACL_UDF_ENTRY_FLD_SRCPORT].name, 
            sdb_uint32_array_val2str(buf, SDB_MAX_CMD_STR_LEN, p_acl_udf_entry->srcport, sizeof(p_acl_udf_entry->srcport)/4));
    }
    if (SDB_FLD_MATCH(STBL_ACL_UDF_ENTRY_FLD_DSTPORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_ACL_UDF_ENTRY_FLD_DSTPORT].name, 
            sdb_uint32_array_val2str(buf, SDB_MAX_CMD_STR_LEN, p_acl_udf_entry->dstport, sizeof(p_acl_udf_entry->dstport)/4));
    }
    if (SDB_FLD_MATCH(STBL_ACL_UDF_ENTRY_FLD_MPLSNUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_ACL_UDF_ENTRY_FLD_MPLSNUM].name, 
            sdb_uint32_array_val2str(buf, SDB_MAX_CMD_STR_LEN, p_acl_udf_entry->mplsnum, sizeof(p_acl_udf_entry->mplsnum)/4));
    }

    sal_fprintf(fp, "\n");

    return SDB_E_NONE;
}

int32
stbl_acl_udf_entry_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs)
{
    stbl_acl_udf_entry_master_t *p_master = _g_p_stbl_acl_udf_entry_master;
    stbl_acl_udf_entry_t *p_db_acl_udf_entry = NULL;
    uint32 id = 0;
    int32 rc = SDB_E_NONE;

    if (NULL == p_master)
    {
        return SDB_E_NONE;
    }

    for (id = 0; id < GLB_ACL_UDF_ENTRY_NUM_MAX+1; id++)
    {
        p_db_acl_udf_entry = p_master->array[id];
        if (NULL == p_db_acl_udf_entry)
        {
            continue;
        }
        rc |= fn(p_db_acl_udf_entry, pargs);
    }

    return (rc) ? SDB_E_FAIL : SDB_E_NONE;
}

stbl_acl_udf_entry_master_t*
stbl_acl_udf_entry_get_master()
{
    return _g_p_stbl_acl_udf_entry_master;
}

stbl_acl_udf_entry_master_t*
stbl_acl_udf_entry_init_acl_udf_entry()
{
    _g_p_stbl_acl_udf_entry_master = XCALLOC(MEM_TBL_MASTER, sizeof(stbl_acl_udf_entry_master_t));
    return _g_p_stbl_acl_udf_entry_master;
}

