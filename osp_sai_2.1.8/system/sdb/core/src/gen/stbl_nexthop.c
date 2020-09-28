
#include "sdb.h"
#include "gen/stbl_nexthop_define.h"
#include "gen/stbl_nexthop.h"
#include "sdb_data_cmp.h"

stbl_nexthop_master_t *_g_p_stbl_nexthop_master = NULL;

static uint32
_stbl_nexthop_hash_make(stbl_nexthop_t *p_nexthop)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_nexthop->key;
    for (index = 0; index < sizeof(p_nexthop->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_stbl_nexthop_hash_cmp(void *p_arg1, void *p_arg2)
{
    stbl_nexthop_t *p_nexthop1 = (stbl_nexthop_t*)p_arg1;
    stbl_nexthop_t *p_nexthop2 = (stbl_nexthop_t*)p_arg2;

    if (0 == sal_memcmp(&p_nexthop1->key, &p_nexthop2->key, sizeof(stbl_nexthop_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
stbl_nexthop_add_nexthop_sync(stbl_nexthop_t *p_nexthop, uint32 sync)
{
    stbl_nexthop_master_t *p_master = _g_p_stbl_nexthop_master;
    stbl_nexthop_t *p_db_nexthop = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (stbl_nexthop_get_nexthop(&p_nexthop->key))
    {
        return SDB_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_nexthop = XCALLOC(MEM_STBL_NEXTHOP, sizeof(stbl_nexthop_t));
    if (NULL == p_db_nexthop)
    {
        return SDB_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_nexthop, p_nexthop, sizeof(stbl_nexthop_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->nexthop_hash, (void*)p_db_nexthop, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return SDB_E_NO_MEMORY;
    }

    return SDB_E_NONE;
}

int32
stbl_nexthop_del_nexthop_sync(stbl_nexthop_key_t *p_nexthop_key, uint32 sync)
{
    stbl_nexthop_master_t *p_master = _g_p_stbl_nexthop_master;
    stbl_nexthop_t *p_db_nexthop = NULL;

    /* 1. lookup entry exist */
    p_db_nexthop = stbl_nexthop_get_nexthop(p_nexthop_key);
    if (NULL == p_db_nexthop)
    {
        return SDB_E_NONE;
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->nexthop_hash, (void*)p_db_nexthop);

    /* 4. free db entry */
    XFREE(MEM_STBL_NEXTHOP, p_db_nexthop);

    return SDB_E_NONE;
}

int32
stbl_nexthop_set_nexthop_field_sync(stbl_nexthop_t *p_nexthop, stbl_nexthop_field_id_t field_id, uint32 sync)
{
    stbl_nexthop_t *p_db_nexthop = NULL;

    /* 1. lookup entry exist */
    p_db_nexthop = stbl_nexthop_get_nexthop(&p_nexthop->key);
    if (NULL == p_db_nexthop)
    {
        return SDB_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case STBL_NEXTHOP_FLD_NH_OID:
        p_db_nexthop->nh_oid = p_nexthop->nh_oid;
        break;

    case STBL_NEXTHOP_FLD_NH_TYPE:
        p_db_nexthop->nh_type = p_nexthop->nh_type;
        break;

    case STBL_NEXTHOP_FLD_MAX:
        sal_memcpy(p_db_nexthop, p_nexthop, sizeof(stbl_nexthop_t));
        break;
    default:
        return SDB_E_NOT_SUPPORT;
    }

    return SDB_E_NONE;
}

int32
stbl_nexthop_add_nexthop(stbl_nexthop_t *p_nexthop)
{
    return stbl_nexthop_add_nexthop_sync(p_nexthop, TRUE);
}

int32
stbl_nexthop_del_nexthop(stbl_nexthop_key_t *p_nexthop_key)
{
    return stbl_nexthop_del_nexthop_sync(p_nexthop_key, TRUE);
}

int32
stbl_nexthop_set_nexthop_field(stbl_nexthop_t *p_nexthop, stbl_nexthop_field_id_t field_id)
{
    return stbl_nexthop_set_nexthop_field_sync(p_nexthop, field_id, TRUE);
}

stbl_nexthop_t*
stbl_nexthop_get_nexthop(stbl_nexthop_key_t *p_nexthop_key)
{
    stbl_nexthop_master_t *p_master = _g_p_stbl_nexthop_master;
    stbl_nexthop_t lkp;

    sal_memcpy(&lkp.key, p_nexthop_key, sizeof(stbl_nexthop_key_t));
    return ctclib_hash_lookup(p_master->nexthop_hash, &lkp);
}

char*
stbl_nexthop_key_val2str(stbl_nexthop_t *p_nexthop, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[SDB_MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            sdb_neighbor_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_nexthop->key));
    return str;
}

int32
stbl_nexthop_dump_one(stbl_nexthop_t *p_nexthop, stbl_iter_args_t *pargs)
{
    sdb_node_t *p_node = sdb_get_tbl(STBL_NEXTHOP);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    sdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[SDB_MAX_CMD_STR_LEN];

    if (SDB_FLD_MATCH(STBL_NEXTHOP_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_NEXTHOP_FLD_KEY].name, 
            sdb_neighbor_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_nexthop->key));
    }
    if (SDB_FLD_MATCH(STBL_NEXTHOP_FLD_NH_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_NEXTHOP_FLD_NH_OID].name, 
            sdb_object_id_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_nexthop->nh_oid));
    }
    if (SDB_FLD_MATCH(STBL_NEXTHOP_FLD_NH_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_NEXTHOP_FLD_NH_TYPE].name,
            p_nexthop->nh_type);
    }

    sal_fprintf(fp, "\n");

    return SDB_E_NONE;
}

int32
stbl_nexthop_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs)
{
    stbl_nexthop_master_t *p_master = _g_p_stbl_nexthop_master;
    if (NULL == p_master)
    {
        return SDB_E_NONE;
    }

    ctclib_hash_iterate2(p_master->nexthop_hash, stbl_hash_iter_adpt_fn, fn, pargs);
    return SDB_E_NONE;
}

stbl_nexthop_master_t*
stbl_nexthop_get_master()
{
    return _g_p_stbl_nexthop_master;
}

stbl_nexthop_master_t*
stbl_nexthop_init_nexthop()
{
    _g_p_stbl_nexthop_master = XCALLOC(MEM_TBL_MASTER, sizeof(stbl_nexthop_master_t));
    _g_p_stbl_nexthop_master->nexthop_hash = ctclib_hash_create(_stbl_nexthop_hash_make, _stbl_nexthop_hash_cmp);
    return _g_p_stbl_nexthop_master;
}

