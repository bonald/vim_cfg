
#include "sdb.h"
#include "gen/stbl_copp_reason_define.h"
#include "gen/stbl_copp_reason.h"
#include "sdb_data_cmp.h"

stbl_copp_reason_master_t *_g_p_stbl_copp_reason_master = NULL;

int32
stbl_copp_reason_add_copp_reason_sync(stbl_copp_reason_t *p_reason, uint32 sync)
{
    stbl_copp_reason_master_t *p_master = _g_p_stbl_copp_reason_master;
    stbl_copp_reason_t *p_db_reason = NULL;

    /* 1. lookup entry exist */
    if (stbl_copp_reason_get_copp_reason(&p_reason->key))
    {
        return SDB_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_reason = XCALLOC(MEM_STBL_COPP_REASON, sizeof(stbl_copp_reason_t));
    if (NULL == p_db_reason)
    {
        return SDB_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_reason, p_reason, sizeof(stbl_copp_reason_t));

    /* 4. add to db */
    p_master->array[p_reason->key.reason_id] = p_db_reason;

    return SDB_E_NONE;
}

int32
stbl_copp_reason_del_copp_reason_sync(stbl_copp_reason_key_t *p_reason_key, uint32 sync)
{
    stbl_copp_reason_master_t *p_master = _g_p_stbl_copp_reason_master;
    stbl_copp_reason_t *p_db_reason = NULL;

    /* 1. lookup entry exist */
    p_db_reason = stbl_copp_reason_get_copp_reason(p_reason_key);
    if (NULL == p_db_reason)
    {
        return SDB_E_NONE;
    }

    /* 3. delete from db */
    p_master->array[p_reason_key->reason_id] = NULL;

    /* 4. free db entry */
    XFREE(MEM_STBL_COPP_REASON, p_db_reason);

    return SDB_E_NONE;
}

int32
stbl_copp_reason_set_copp_reason_field_sync(stbl_copp_reason_t *p_reason, stbl_copp_reason_field_id_t field_id, uint32 sync)
{
    stbl_copp_reason_t *p_db_reason = NULL;

    /* 1. lookup entry exist */
    p_db_reason = stbl_copp_reason_get_copp_reason(&p_reason->key);
    if (NULL == p_db_reason)
    {
        return SDB_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case STBL_COPP_REASON_FLD_CHANNEL:
        p_db_reason->channel = p_reason->channel;
        break;

    case STBL_COPP_REASON_FLD_TRAP_ID:
        p_db_reason->trap_id = p_reason->trap_id;
        break;

    case STBL_COPP_REASON_FLD_MAX:
        sal_memcpy(p_db_reason, p_reason, sizeof(stbl_copp_reason_t));
        break;
    default:
        return SDB_E_NOT_SUPPORT;
    }

    return SDB_E_NONE;
}

int32
stbl_copp_reason_add_copp_reason(stbl_copp_reason_t *p_reason)
{
    return stbl_copp_reason_add_copp_reason_sync(p_reason, TRUE);
}

int32
stbl_copp_reason_del_copp_reason(stbl_copp_reason_key_t *p_reason_key)
{
    return stbl_copp_reason_del_copp_reason_sync(p_reason_key, TRUE);
}

int32
stbl_copp_reason_set_copp_reason_field(stbl_copp_reason_t *p_reason, stbl_copp_reason_field_id_t field_id)
{
    return stbl_copp_reason_set_copp_reason_field_sync(p_reason, field_id, TRUE);
}

stbl_copp_reason_t*
stbl_copp_reason_get_copp_reason(stbl_copp_reason_key_t *p_reason_key)
{
    stbl_copp_reason_master_t *p_master = _g_p_stbl_copp_reason_master;

    if (GLB_COPP_REASON_MAX <= p_reason_key->reason_id)
    {
        return NULL;
    }
    return (p_master->array[p_reason_key->reason_id]);
}

char*
stbl_copp_reason_key_val2str(stbl_copp_reason_t *p_reason, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_reason->key.reason_id);
    return str;
}

int32
stbl_copp_reason_dump_one(stbl_copp_reason_t *p_reason, stbl_iter_args_t *pargs)
{
    sdb_node_t *p_node = sdb_get_tbl(STBL_COPP_REASON);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    sdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (SDB_FLD_MATCH(STBL_COPP_REASON_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_COPP_REASON_FLD_KEY].name,
            p_reason->key.reason_id);
    }
    if (SDB_FLD_MATCH(STBL_COPP_REASON_FLD_CHANNEL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_COPP_REASON_FLD_CHANNEL].name,
            p_reason->channel);
    }
    if (SDB_FLD_MATCH(STBL_COPP_REASON_FLD_TRAP_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_COPP_REASON_FLD_TRAP_ID].name,
            p_reason->trap_id);
    }

    sal_fprintf(fp, "\n");

    return SDB_E_NONE;
}

int32
stbl_copp_reason_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs)
{
    stbl_copp_reason_master_t *p_master = _g_p_stbl_copp_reason_master;
    stbl_copp_reason_t *p_db_reason = NULL;
    uint32 reason_id = 0;
    int32 rc = SDB_E_NONE;

    if (NULL == p_master)
    {
        return SDB_E_NONE;
    }

    for (reason_id = 0; reason_id < GLB_COPP_REASON_MAX; reason_id++)
    {
        p_db_reason = p_master->array[reason_id];
        if (NULL == p_db_reason)
        {
            continue;
        }
        rc |= fn(p_db_reason, pargs);
    }

    return (rc) ? SDB_E_FAIL : SDB_E_NONE;
}

stbl_copp_reason_master_t*
stbl_copp_reason_get_master()
{
    return _g_p_stbl_copp_reason_master;
}

stbl_copp_reason_master_t*
stbl_copp_reason_init_copp_reason()
{
    _g_p_stbl_copp_reason_master = XCALLOC(MEM_TBL_MASTER, sizeof(stbl_copp_reason_master_t));
    return _g_p_stbl_copp_reason_master;
}

