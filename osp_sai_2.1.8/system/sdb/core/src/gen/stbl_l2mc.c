
#include "sdb.h"
#include "gen/stbl_l2mc_define.h"
#include "gen/stbl_l2mc.h"
#include "sdb_data_cmp.h"

stbl_l2mc_master_t *_g_p_stbl_l2mc_master = NULL;

static uint32
_stbl_l2mc_hash_make(stbl_l2mc_t *p_l2mc)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_l2mc->key;
    for (index = 0; index < sizeof(p_l2mc->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_stbl_l2mc_hash_cmp(void *p_arg1, void *p_arg2)
{
    stbl_l2mc_t *p_l2mc1 = (stbl_l2mc_t*)p_arg1;
    stbl_l2mc_t *p_l2mc2 = (stbl_l2mc_t*)p_arg2;

    if (0 == sal_memcmp(&p_l2mc1->key, &p_l2mc2->key, sizeof(stbl_l2mc_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
stbl_l2mc_add_l2mc_sync(stbl_l2mc_t *p_l2mc, uint32 sync)
{
    stbl_l2mc_master_t *p_master = _g_p_stbl_l2mc_master;
    stbl_l2mc_t *p_db_l2mc = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (stbl_l2mc_get_l2mc(&p_l2mc->key))
    {
        return SDB_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_l2mc = XCALLOC(MEM_STBL_L2MC, sizeof(stbl_l2mc_t));
    if (NULL == p_db_l2mc)
    {
        return SDB_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_l2mc, p_l2mc, sizeof(stbl_l2mc_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->l2mc_hash, (void*)p_db_l2mc, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return SDB_E_NO_MEMORY;
    }

    return SDB_E_NONE;
}

int32
stbl_l2mc_del_l2mc_sync(stbl_l2mc_key_t *p_l2mc_key, uint32 sync)
{
    stbl_l2mc_master_t *p_master = _g_p_stbl_l2mc_master;
    stbl_l2mc_t *p_db_l2mc = NULL;

    /* 1. lookup entry exist */
    p_db_l2mc = stbl_l2mc_get_l2mc(p_l2mc_key);
    if (NULL == p_db_l2mc)
    {
        return SDB_E_NONE;
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->l2mc_hash, (void*)p_db_l2mc);

    /* 4. free db entry */
    XFREE(MEM_STBL_L2MC, p_db_l2mc);

    return SDB_E_NONE;
}

int32
stbl_l2mc_set_l2mc_field_sync(stbl_l2mc_t *p_l2mc, stbl_l2mc_field_id_t field_id, uint32 sync)
{
    stbl_l2mc_t *p_db_l2mc = NULL;

    /* 1. lookup entry exist */
    p_db_l2mc = stbl_l2mc_get_l2mc(&p_l2mc->key);
    if (NULL == p_db_l2mc)
    {
        return SDB_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case STBL_L2MC_FLD_GROUP_ID:
        p_db_l2mc->group_id = p_l2mc->group_id;
        break;

    case STBL_L2MC_FLD_MAX:
        sal_memcpy(p_db_l2mc, p_l2mc, sizeof(stbl_l2mc_t));
        break;
    default:
        return SDB_E_NOT_SUPPORT;
    }

    return SDB_E_NONE;
}

int32
stbl_l2mc_add_l2mc(stbl_l2mc_t *p_l2mc)
{
    return stbl_l2mc_add_l2mc_sync(p_l2mc, TRUE);
}

int32
stbl_l2mc_del_l2mc(stbl_l2mc_key_t *p_l2mc_key)
{
    return stbl_l2mc_del_l2mc_sync(p_l2mc_key, TRUE);
}

int32
stbl_l2mc_set_l2mc_field(stbl_l2mc_t *p_l2mc, stbl_l2mc_field_id_t field_id)
{
    return stbl_l2mc_set_l2mc_field_sync(p_l2mc, field_id, TRUE);
}

stbl_l2mc_t*
stbl_l2mc_get_l2mc(stbl_l2mc_key_t *p_l2mc_key)
{
    stbl_l2mc_master_t *p_master = _g_p_stbl_l2mc_master;
    stbl_l2mc_t lkp;

    sal_memcpy(&lkp.key, p_l2mc_key, sizeof(stbl_l2mc_key_t));
    return ctclib_hash_lookup(p_master->l2mc_hash, &lkp);
}

char*
stbl_l2mc_key_val2str(stbl_l2mc_t *p_l2mc, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[SDB_MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            sdb_l2mc_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_l2mc->key));
    return str;
}

int32
stbl_l2mc_dump_one(stbl_l2mc_t *p_l2mc, stbl_iter_args_t *pargs)
{
    sdb_node_t *p_node = sdb_get_tbl(STBL_L2MC);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    sdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[SDB_MAX_CMD_STR_LEN];

    if (SDB_FLD_MATCH(STBL_L2MC_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_L2MC_FLD_KEY].name, 
            sdb_l2mc_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_l2mc->key));
    }
    if (SDB_FLD_MATCH(STBL_L2MC_FLD_GROUP_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_L2MC_FLD_GROUP_ID].name,
            p_l2mc->group_id);
    }

    sal_fprintf(fp, "\n");

    return SDB_E_NONE;
}

int32
stbl_l2mc_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs)
{
    stbl_l2mc_master_t *p_master = _g_p_stbl_l2mc_master;
    if (NULL == p_master)
    {
        return SDB_E_NONE;
    }

    ctclib_hash_iterate2(p_master->l2mc_hash, stbl_hash_iter_adpt_fn, fn, pargs);
    return SDB_E_NONE;
}

stbl_l2mc_master_t*
stbl_l2mc_get_master()
{
    return _g_p_stbl_l2mc_master;
}

stbl_l2mc_master_t*
stbl_l2mc_init_l2mc()
{
    _g_p_stbl_l2mc_master = XCALLOC(MEM_TBL_MASTER, sizeof(stbl_l2mc_master_t));
    _g_p_stbl_l2mc_master->l2mc_hash = ctclib_hash_create(_stbl_l2mc_hash_make, _stbl_l2mc_hash_cmp);
    return _g_p_stbl_l2mc_master;
}

