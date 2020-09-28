
#include "sdb.h"
#include "gen/stbl_copp_group_define.h"
#include "gen/stbl_copp_group.h"
#include "sdb_data_cmp.h"

stbl_copp_group_master_t *_g_p_stbl_copp_group_master = NULL;

int32
stbl_copp_group_add_copp_group_sync(stbl_copp_group_t *p_group, uint32 sync)
{
    stbl_copp_group_master_t *p_master = _g_p_stbl_copp_group_master;
    stbl_copp_group_t *p_db_group = NULL;

    /* 1. lookup entry exist */
    if (stbl_copp_group_get_copp_group(&p_group->key))
    {
        return SDB_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_group = XCALLOC(MEM_STBL_COPP_GROUP, sizeof(stbl_copp_group_t));
    if (NULL == p_db_group)
    {
        return SDB_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_group, p_group, sizeof(stbl_copp_group_t));

    /* 4. add to db */
    p_master->array[p_group->key.group_id] = p_db_group;

    return SDB_E_NONE;
}

int32
stbl_copp_group_del_copp_group_sync(stbl_copp_group_key_t *p_group_key, uint32 sync)
{
    stbl_copp_group_master_t *p_master = _g_p_stbl_copp_group_master;
    stbl_copp_group_t *p_db_group = NULL;

    /* 1. lookup entry exist */
    p_db_group = stbl_copp_group_get_copp_group(p_group_key);
    if (NULL == p_db_group)
    {
        return SDB_E_NONE;
    }

    /* 3. delete from db */
    p_master->array[p_group_key->group_id] = NULL;

    /* 4. free db entry */
    XFREE(MEM_STBL_COPP_GROUP, p_db_group);

    return SDB_E_NONE;
}

int32
stbl_copp_group_set_copp_group_field_sync(stbl_copp_group_t *p_group, stbl_copp_group_field_id_t field_id, uint32 sync)
{
    stbl_copp_group_t *p_db_group = NULL;

    /* 1. lookup entry exist */
    p_db_group = stbl_copp_group_get_copp_group(&p_group->key);
    if (NULL == p_db_group)
    {
        return SDB_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case STBL_COPP_GROUP_FLD_POLICE_OID:
        p_db_group->police_oid = p_group->police_oid;
        break;

    case STBL_COPP_GROUP_FLD_POLICE_SSH_OID:
        p_db_group->police_ssh_oid = p_group->police_ssh_oid;
        break;

    case STBL_COPP_GROUP_FLD_POLICE_TELNET_OID:
        p_db_group->police_telnet_oid = p_group->police_telnet_oid;
        break;

    case STBL_COPP_GROUP_FLD_POLICE_MLAG_OID:
        p_db_group->police_mlag_oid = p_group->police_mlag_oid;
        break;

    case STBL_COPP_GROUP_FLD_QUEUE_ID:
        p_db_group->queue_id = p_group->queue_id;
        break;

    case STBL_COPP_GROUP_FLD_CTC_REASON_ID:
        p_db_group->ctc_reason_id = p_group->ctc_reason_id;
        break;

    case STBL_COPP_GROUP_FLD_CHANNEL:
        p_db_group->channel = p_group->channel;
        break;

    case STBL_COPP_GROUP_FLD_MAX:
        sal_memcpy(p_db_group, p_group, sizeof(stbl_copp_group_t));
        break;
    default:
        return SDB_E_NOT_SUPPORT;
    }

    return SDB_E_NONE;
}

int32
stbl_copp_group_add_copp_group(stbl_copp_group_t *p_group)
{
    return stbl_copp_group_add_copp_group_sync(p_group, TRUE);
}

int32
stbl_copp_group_del_copp_group(stbl_copp_group_key_t *p_group_key)
{
    return stbl_copp_group_del_copp_group_sync(p_group_key, TRUE);
}

int32
stbl_copp_group_set_copp_group_field(stbl_copp_group_t *p_group, stbl_copp_group_field_id_t field_id)
{
    return stbl_copp_group_set_copp_group_field_sync(p_group, field_id, TRUE);
}

stbl_copp_group_t*
stbl_copp_group_get_copp_group(stbl_copp_group_key_t *p_group_key)
{
    stbl_copp_group_master_t *p_master = _g_p_stbl_copp_group_master;

    if (GLB_COPP_GROUP_MAX <= p_group_key->group_id)
    {
        return NULL;
    }
    return (p_master->array[p_group_key->group_id]);
}

char*
stbl_copp_group_key_val2str(stbl_copp_group_t *p_group, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_group->key.group_id);
    return str;
}

int32
stbl_copp_group_dump_one(stbl_copp_group_t *p_group, stbl_iter_args_t *pargs)
{
    sdb_node_t *p_node = sdb_get_tbl(STBL_COPP_GROUP);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    sdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[SDB_MAX_CMD_STR_LEN];

    if (SDB_FLD_MATCH(STBL_COPP_GROUP_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_COPP_GROUP_FLD_KEY].name,
            p_group->key.group_id);
    }
    if (SDB_FLD_MATCH(STBL_COPP_GROUP_FLD_POLICE_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_COPP_GROUP_FLD_POLICE_OID].name, 
            sdb_object_id_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_group->police_oid));
    }
    if (SDB_FLD_MATCH(STBL_COPP_GROUP_FLD_POLICE_SSH_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_COPP_GROUP_FLD_POLICE_SSH_OID].name, 
            sdb_object_id_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_group->police_ssh_oid));
    }
    if (SDB_FLD_MATCH(STBL_COPP_GROUP_FLD_POLICE_TELNET_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_COPP_GROUP_FLD_POLICE_TELNET_OID].name, 
            sdb_object_id_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_group->police_telnet_oid));
    }
    if (SDB_FLD_MATCH(STBL_COPP_GROUP_FLD_POLICE_MLAG_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_COPP_GROUP_FLD_POLICE_MLAG_OID].name, 
            sdb_object_id_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_group->police_mlag_oid));
    }
    if (SDB_FLD_MATCH(STBL_COPP_GROUP_FLD_QUEUE_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_COPP_GROUP_FLD_QUEUE_ID].name,
            p_group->queue_id);
    }
    if (SDB_FLD_MATCH(STBL_COPP_GROUP_FLD_CTC_REASON_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_COPP_GROUP_FLD_CTC_REASON_ID].name,
            p_group->ctc_reason_id);
    }
    if (SDB_FLD_MATCH(STBL_COPP_GROUP_FLD_CHANNEL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_COPP_GROUP_FLD_CHANNEL].name,
            p_group->channel);
    }

    sal_fprintf(fp, "\n");

    return SDB_E_NONE;
}

int32
stbl_copp_group_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs)
{
    stbl_copp_group_master_t *p_master = _g_p_stbl_copp_group_master;
    stbl_copp_group_t *p_db_group = NULL;
    uint32 group_id = 0;
    int32 rc = SDB_E_NONE;

    if (NULL == p_master)
    {
        return SDB_E_NONE;
    }

    for (group_id = 0; group_id < GLB_COPP_GROUP_MAX; group_id++)
    {
        p_db_group = p_master->array[group_id];
        if (NULL == p_db_group)
        {
            continue;
        }
        rc |= fn(p_db_group, pargs);
    }

    return (rc) ? SDB_E_FAIL : SDB_E_NONE;
}

stbl_copp_group_master_t*
stbl_copp_group_get_master()
{
    return _g_p_stbl_copp_group_master;
}

stbl_copp_group_master_t*
stbl_copp_group_init_copp_group()
{
    _g_p_stbl_copp_group_master = XCALLOC(MEM_TBL_MASTER, sizeof(stbl_copp_group_master_t));
    return _g_p_stbl_copp_group_master;
}

