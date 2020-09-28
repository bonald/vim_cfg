
#include "sdb.h"
#include "gen/stbl_lag_memport_define.h"
#include "gen/stbl_lag_memport.h"
#include "sdb_data_cmp.h"

stbl_lag_memport_master_t *_g_p_stbl_lag_memport_master = NULL;

static uint32
_stbl_lag_memport_hash_make(stbl_lag_memport_t *p_memport)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_memport->key;
    for (index = 0; index < sizeof(p_memport->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_stbl_lag_memport_hash_cmp(void *p_arg1, void *p_arg2)
{
    stbl_lag_memport_t *p_memport1 = (stbl_lag_memport_t*)p_arg1;
    stbl_lag_memport_t *p_memport2 = (stbl_lag_memport_t*)p_arg2;

    if (0 == sal_memcmp(&p_memport1->key, &p_memport2->key, sizeof(stbl_lag_memport_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
stbl_lag_memport_add_lag_memport_sync(stbl_lag_memport_t *p_memport, uint32 sync)
{
    stbl_lag_memport_master_t *p_master = _g_p_stbl_lag_memport_master;
    stbl_lag_memport_t *p_db_memport = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (stbl_lag_memport_get_lag_memport(&p_memport->key))
    {
        return SDB_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_memport = XCALLOC(MEM_STBL_LAG_MEMPORT, sizeof(stbl_lag_memport_t));
    if (NULL == p_db_memport)
    {
        return SDB_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_memport, p_memport, sizeof(stbl_lag_memport_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->memport_hash, (void*)p_db_memport, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return SDB_E_NO_MEMORY;
    }

    return SDB_E_NONE;
}

int32
stbl_lag_memport_del_lag_memport_sync(stbl_lag_memport_key_t *p_memport_key, uint32 sync)
{
    stbl_lag_memport_master_t *p_master = _g_p_stbl_lag_memport_master;
    stbl_lag_memport_t *p_db_memport = NULL;

    /* 1. lookup entry exist */
    p_db_memport = stbl_lag_memport_get_lag_memport(p_memport_key);
    if (NULL == p_db_memport)
    {
        return SDB_E_NONE;
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->memport_hash, (void*)p_db_memport);

    /* 4. free db entry */
    XFREE(MEM_STBL_LAG_MEMPORT, p_db_memport);

    return SDB_E_NONE;
}

int32
stbl_lag_memport_set_lag_memport_field_sync(stbl_lag_memport_t *p_memport, stbl_lag_memport_field_id_t field_id, uint32 sync)
{
    stbl_lag_memport_t *p_db_memport = NULL;

    /* 1. lookup entry exist */
    p_db_memport = stbl_lag_memport_get_lag_memport(&p_memport->key);
    if (NULL == p_db_memport)
    {
        return SDB_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case STBL_LAG_MEMPORT_FLD_PORT_OID:
        p_db_memport->port_oid = p_memport->port_oid;
        break;

    case STBL_LAG_MEMPORT_FLD_TID:
        p_db_memport->tid = p_memport->tid;
        break;

    case STBL_LAG_MEMPORT_FLD_MAX:
        sal_memcpy(p_db_memport, p_memport, sizeof(stbl_lag_memport_t));
        break;
    default:
        return SDB_E_NOT_SUPPORT;
    }

    return SDB_E_NONE;
}

int32
stbl_lag_memport_add_lag_memport(stbl_lag_memport_t *p_memport)
{
    return stbl_lag_memport_add_lag_memport_sync(p_memport, TRUE);
}

int32
stbl_lag_memport_del_lag_memport(stbl_lag_memport_key_t *p_memport_key)
{
    return stbl_lag_memport_del_lag_memport_sync(p_memport_key, TRUE);
}

int32
stbl_lag_memport_set_lag_memport_field(stbl_lag_memport_t *p_memport, stbl_lag_memport_field_id_t field_id)
{
    return stbl_lag_memport_set_lag_memport_field_sync(p_memport, field_id, TRUE);
}

stbl_lag_memport_t*
stbl_lag_memport_get_lag_memport(stbl_lag_memport_key_t *p_memport_key)
{
    stbl_lag_memport_master_t *p_master = _g_p_stbl_lag_memport_master;
    stbl_lag_memport_t lkp;

    sal_memcpy(&lkp.key, p_memport_key, sizeof(stbl_lag_memport_key_t));
    return ctclib_hash_lookup(p_master->memport_hash, &lkp);
}

char*
stbl_lag_memport_key_val2str(stbl_lag_memport_t *p_memport, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_memport->key.gport);
    return str;
}

int32
stbl_lag_memport_dump_one(stbl_lag_memport_t *p_memport, stbl_iter_args_t *pargs)
{
    sdb_node_t *p_node = sdb_get_tbl(STBL_LAG_MEMPORT);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    sdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[SDB_MAX_CMD_STR_LEN];

    if (SDB_FLD_MATCH(STBL_LAG_MEMPORT_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_LAG_MEMPORT_FLD_KEY].name,
            p_memport->key.gport);
    }
    if (SDB_FLD_MATCH(STBL_LAG_MEMPORT_FLD_PORT_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_LAG_MEMPORT_FLD_PORT_OID].name, 
            sdb_object_id_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_memport->port_oid));
    }
    if (SDB_FLD_MATCH(STBL_LAG_MEMPORT_FLD_TID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_LAG_MEMPORT_FLD_TID].name,
            p_memport->tid);
    }

    sal_fprintf(fp, "\n");

    return SDB_E_NONE;
}

int32
stbl_lag_memport_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs)
{
    stbl_lag_memport_master_t *p_master = _g_p_stbl_lag_memport_master;
    if (NULL == p_master)
    {
        return SDB_E_NONE;
    }

    ctclib_hash_iterate2(p_master->memport_hash, stbl_hash_iter_adpt_fn, fn, pargs);
    return SDB_E_NONE;
}

stbl_lag_memport_master_t*
stbl_lag_memport_get_master()
{
    return _g_p_stbl_lag_memport_master;
}

stbl_lag_memport_master_t*
stbl_lag_memport_init_lag_memport()
{
    _g_p_stbl_lag_memport_master = XCALLOC(MEM_TBL_MASTER, sizeof(stbl_lag_memport_master_t));
    _g_p_stbl_lag_memport_master->memport_hash = ctclib_hash_create(_stbl_lag_memport_hash_make, _stbl_lag_memport_hash_cmp);
    return _g_p_stbl_lag_memport_master;
}

