
#include "sdb.h"
#include "gen/stbl_acl_tbl_define.h"
#include "gen/stbl_acl_tbl.h"
#include "sdb_data_cmp.h"

stbl_acl_tbl_master_t *_g_p_stbl_acl_tbl_master = NULL;

int32
stbl_acl_tbl_add_acl_tbl_sync(stbl_acl_tbl_t *p_acl, uint32 sync)
{
    stbl_acl_tbl_master_t *p_master = _g_p_stbl_acl_tbl_master;
    stbl_acl_tbl_t *p_db_acl = NULL;

    /* 1. lookup entry exist */
    if (stbl_acl_tbl_get_acl_tbl(&p_acl->key))
    {
        return SDB_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_acl = XCALLOC(MEM_STBL_ACL_TBL, sizeof(stbl_acl_tbl_t));
    if (NULL == p_db_acl)
    {
        return SDB_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_acl, p_acl, sizeof(stbl_acl_tbl_t));

    /* 4. add to db */
    p_master->array[p_acl->key.tbl_id] = p_db_acl;

    return SDB_E_NONE;
}

int32
stbl_acl_tbl_del_acl_tbl_sync(stbl_acl_tbl_key_t *p_acl_key, uint32 sync)
{
    stbl_acl_tbl_master_t *p_master = _g_p_stbl_acl_tbl_master;
    stbl_acl_tbl_t *p_db_acl = NULL;

    /* 1. lookup entry exist */
    p_db_acl = stbl_acl_tbl_get_acl_tbl(p_acl_key);
    if (NULL == p_db_acl)
    {
        return SDB_E_NONE;
    }

    /* 3. delete from db */
    p_master->array[p_acl_key->tbl_id] = NULL;

    /* 4. free db entry */
    XFREE(MEM_STBL_ACL_TBL, p_db_acl);

    return SDB_E_NONE;
}

int32
stbl_acl_tbl_set_acl_tbl_field_sync(stbl_acl_tbl_t *p_acl, stbl_acl_tbl_field_id_t field_id, uint32 sync)
{
    stbl_acl_tbl_t *p_db_acl = NULL;

    /* 1. lookup entry exist */
    p_db_acl = stbl_acl_tbl_get_acl_tbl(&p_acl->key);
    if (NULL == p_db_acl)
    {
        return SDB_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case STBL_ACL_TBL_FLD_PRIORITY:
        p_db_acl->priority = p_acl->priority;
        break;

    case STBL_ACL_TBL_FLD_FIELD_BITMAP:
        p_db_acl->field_bitmap = p_acl->field_bitmap;
        break;

    case STBL_ACL_TBL_FLD_STAGE:
        p_db_acl->stage = p_acl->stage;
        break;

    case STBL_ACL_TBL_FLD_MAX:
        sal_memcpy(p_db_acl, p_acl, sizeof(stbl_acl_tbl_t));
        break;
    default:
        return SDB_E_NOT_SUPPORT;
    }

    return SDB_E_NONE;
}

int32
stbl_acl_tbl_add_acl_tbl(stbl_acl_tbl_t *p_acl)
{
    return stbl_acl_tbl_add_acl_tbl_sync(p_acl, TRUE);
}

int32
stbl_acl_tbl_del_acl_tbl(stbl_acl_tbl_key_t *p_acl_key)
{
    return stbl_acl_tbl_del_acl_tbl_sync(p_acl_key, TRUE);
}

int32
stbl_acl_tbl_set_acl_tbl_field(stbl_acl_tbl_t *p_acl, stbl_acl_tbl_field_id_t field_id)
{
    return stbl_acl_tbl_set_acl_tbl_field_sync(p_acl, field_id, TRUE);
}

stbl_acl_tbl_t*
stbl_acl_tbl_get_acl_tbl(stbl_acl_tbl_key_t *p_acl_key)
{
    stbl_acl_tbl_master_t *p_master = _g_p_stbl_acl_tbl_master;

    if (GLB_SAI_ACL_MAX <= p_acl_key->tbl_id)
    {
        return NULL;
    }
    return (p_master->array[p_acl_key->tbl_id]);
}

char*
stbl_acl_tbl_key_val2str(stbl_acl_tbl_t *p_acl, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_acl->key.tbl_id);
    return str;
}

int32
stbl_acl_tbl_dump_one(stbl_acl_tbl_t *p_acl, stbl_iter_args_t *pargs)
{
    sdb_node_t *p_node = sdb_get_tbl(STBL_ACL_TBL);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    sdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (SDB_FLD_MATCH(STBL_ACL_TBL_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_ACL_TBL_FLD_KEY].name,
            p_acl->key.tbl_id);
    }
    if (SDB_FLD_MATCH(STBL_ACL_TBL_FLD_PRIORITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_ACL_TBL_FLD_PRIORITY].name,
            p_acl->priority);
    }
    if (SDB_FLD_MATCH(STBL_ACL_TBL_FLD_FIELD_BITMAP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_ACL_TBL_FLD_FIELD_BITMAP].name,
            p_acl->field_bitmap);
    }
    if (SDB_FLD_MATCH(STBL_ACL_TBL_FLD_STAGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_ACL_TBL_FLD_STAGE].name,
            p_acl->stage);
    }

    sal_fprintf(fp, "\n");

    return SDB_E_NONE;
}

int32
stbl_acl_tbl_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs)
{
    stbl_acl_tbl_master_t *p_master = _g_p_stbl_acl_tbl_master;
    stbl_acl_tbl_t *p_db_acl = NULL;
    uint32 tbl_id = 0;
    int32 rc = SDB_E_NONE;

    if (NULL == p_master)
    {
        return SDB_E_NONE;
    }

    for (tbl_id = 0; tbl_id < GLB_SAI_ACL_MAX; tbl_id++)
    {
        p_db_acl = p_master->array[tbl_id];
        if (NULL == p_db_acl)
        {
            continue;
        }
        rc |= fn(p_db_acl, pargs);
    }

    return (rc) ? SDB_E_FAIL : SDB_E_NONE;
}

stbl_acl_tbl_master_t*
stbl_acl_tbl_get_master()
{
    return _g_p_stbl_acl_tbl_master;
}

stbl_acl_tbl_master_t*
stbl_acl_tbl_init_acl_tbl()
{
    _g_p_stbl_acl_tbl_master = XCALLOC(MEM_TBL_MASTER, sizeof(stbl_acl_tbl_master_t));
    return _g_p_stbl_acl_tbl_master;
}

