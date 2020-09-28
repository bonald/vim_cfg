
#include "sdb.h"
#include "gen/stbl_copp_trap_define.h"
#include "gen/stbl_copp_trap.h"
#include "sdb_data_cmp.h"

stbl_copp_trap_master_t *_g_p_stbl_copp_trap_master = NULL;

int32
stbl_copp_trap_add_copp_trap_sync(stbl_copp_trap_t *p_trap, uint32 sync)
{
    stbl_copp_trap_master_t *p_master = _g_p_stbl_copp_trap_master;
    stbl_copp_trap_t *p_db_trap = NULL;

    /* 1. lookup entry exist */
    if (stbl_copp_trap_get_copp_trap(&p_trap->key))
    {
        return SDB_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_trap = XCALLOC(MEM_STBL_COPP_TRAP, sizeof(stbl_copp_trap_t));
    if (NULL == p_db_trap)
    {
        return SDB_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_trap, p_trap, sizeof(stbl_copp_trap_t));

    /* 4. add to db */
    p_master->array[p_trap->key.trap_id] = p_db_trap;

    return SDB_E_NONE;
}

int32
stbl_copp_trap_del_copp_trap_sync(stbl_copp_trap_key_t *p_trap_key, uint32 sync)
{
    stbl_copp_trap_master_t *p_master = _g_p_stbl_copp_trap_master;
    stbl_copp_trap_t *p_db_trap = NULL;

    /* 1. lookup entry exist */
    p_db_trap = stbl_copp_trap_get_copp_trap(p_trap_key);
    if (NULL == p_db_trap)
    {
        return SDB_E_NONE;
    }

    /* 3. delete from db */
    p_master->array[p_trap_key->trap_id] = NULL;

    /* 4. free db entry */
    XFREE(MEM_STBL_COPP_TRAP, p_db_trap);

    return SDB_E_NONE;
}

int32
stbl_copp_trap_set_copp_trap_field_sync(stbl_copp_trap_t *p_trap, stbl_copp_trap_field_id_t field_id, uint32 sync)
{
    stbl_copp_trap_t *p_db_trap = NULL;

    /* 1. lookup entry exist */
    p_db_trap = stbl_copp_trap_get_copp_trap(&p_trap->key);
    if (NULL == p_db_trap)
    {
        return SDB_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case STBL_COPP_TRAP_FLD_GROUP_OID:
        p_db_trap->group_oid = p_trap->group_oid;
        break;

    case STBL_COPP_TRAP_FLD_CTC_REASON_ID:
        p_db_trap->ctc_reason_id = p_trap->ctc_reason_id;
        break;

    case STBL_COPP_TRAP_FLD_CHANNEL:
        p_db_trap->channel = p_trap->channel;
        break;

    case STBL_COPP_TRAP_FLD_MAX:
        sal_memcpy(p_db_trap, p_trap, sizeof(stbl_copp_trap_t));
        break;
    default:
        return SDB_E_NOT_SUPPORT;
    }

    return SDB_E_NONE;
}

int32
stbl_copp_trap_add_copp_trap(stbl_copp_trap_t *p_trap)
{
    return stbl_copp_trap_add_copp_trap_sync(p_trap, TRUE);
}

int32
stbl_copp_trap_del_copp_trap(stbl_copp_trap_key_t *p_trap_key)
{
    return stbl_copp_trap_del_copp_trap_sync(p_trap_key, TRUE);
}

int32
stbl_copp_trap_set_copp_trap_field(stbl_copp_trap_t *p_trap, stbl_copp_trap_field_id_t field_id)
{
    return stbl_copp_trap_set_copp_trap_field_sync(p_trap, field_id, TRUE);
}

stbl_copp_trap_t*
stbl_copp_trap_get_copp_trap(stbl_copp_trap_key_t *p_trap_key)
{
    stbl_copp_trap_master_t *p_master = _g_p_stbl_copp_trap_master;

    if (GLB_COPP_TRAP_MAX <= p_trap_key->trap_id)
    {
        return NULL;
    }
    return (p_master->array[p_trap_key->trap_id]);
}

char*
stbl_copp_trap_key_val2str(stbl_copp_trap_t *p_trap, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_trap->key.trap_id);
    return str;
}

int32
stbl_copp_trap_dump_one(stbl_copp_trap_t *p_trap, stbl_iter_args_t *pargs)
{
    sdb_node_t *p_node = sdb_get_tbl(STBL_COPP_TRAP);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    sdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[SDB_MAX_CMD_STR_LEN];

    if (SDB_FLD_MATCH(STBL_COPP_TRAP_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_COPP_TRAP_FLD_KEY].name,
            p_trap->key.trap_id);
    }
    if (SDB_FLD_MATCH(STBL_COPP_TRAP_FLD_GROUP_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_COPP_TRAP_FLD_GROUP_OID].name, 
            sdb_object_id_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_trap->group_oid));
    }
    if (SDB_FLD_MATCH(STBL_COPP_TRAP_FLD_CTC_REASON_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_COPP_TRAP_FLD_CTC_REASON_ID].name,
            p_trap->ctc_reason_id);
    }
    if (SDB_FLD_MATCH(STBL_COPP_TRAP_FLD_CHANNEL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_COPP_TRAP_FLD_CHANNEL].name,
            p_trap->channel);
    }

    sal_fprintf(fp, "\n");

    return SDB_E_NONE;
}

int32
stbl_copp_trap_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs)
{
    stbl_copp_trap_master_t *p_master = _g_p_stbl_copp_trap_master;
    stbl_copp_trap_t *p_db_trap = NULL;
    uint32 trap_id = 0;
    int32 rc = SDB_E_NONE;

    if (NULL == p_master)
    {
        return SDB_E_NONE;
    }

    for (trap_id = 0; trap_id < GLB_COPP_TRAP_MAX; trap_id++)
    {
        p_db_trap = p_master->array[trap_id];
        if (NULL == p_db_trap)
        {
            continue;
        }
        rc |= fn(p_db_trap, pargs);
    }

    return (rc) ? SDB_E_FAIL : SDB_E_NONE;
}

stbl_copp_trap_master_t*
stbl_copp_trap_get_master()
{
    return _g_p_stbl_copp_trap_master;
}

stbl_copp_trap_master_t*
stbl_copp_trap_init_copp_trap()
{
    _g_p_stbl_copp_trap_master = XCALLOC(MEM_TBL_MASTER, sizeof(stbl_copp_trap_master_t));
    return _g_p_stbl_copp_trap_master;
}

