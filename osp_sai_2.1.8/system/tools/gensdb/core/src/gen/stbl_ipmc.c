
#include "sdb.h"
#include "gen/stbl_ipmc_define.h"
#include "gen/stbl_ipmc.h"
#include "sdb_data_cmp.h"

stbl_ipmc_master_t *_g_p_stbl_ipmc_master = NULL;

static uint32
_stbl_ipmc_hash_make(stbl_ipmc_t *p_ipmc)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_ipmc->key;
    for (index = 0; index < sizeof(p_ipmc->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_stbl_ipmc_hash_cmp(void *p_arg1, void *p_arg2)
{
    stbl_ipmc_t *p_ipmc1 = (stbl_ipmc_t*)p_arg1;
    stbl_ipmc_t *p_ipmc2 = (stbl_ipmc_t*)p_arg2;

    if (0 == sal_memcmp(&p_ipmc1->key, &p_ipmc2->key, sizeof(stbl_ipmc_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
stbl_ipmc_add_ipmc_sync(stbl_ipmc_t *p_ipmc, uint32 sync)
{
    stbl_ipmc_master_t *p_master = _g_p_stbl_ipmc_master;
    stbl_ipmc_t *p_db_ipmc = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (stbl_ipmc_get_ipmc(&p_ipmc->key))
    {
        return SDB_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_ipmc = XCALLOC(MEM_STBL_IPMC, sizeof(stbl_ipmc_t));
    if (NULL == p_db_ipmc)
    {
        return SDB_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_ipmc, p_ipmc, sizeof(stbl_ipmc_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->ipmc_hash, (void*)p_db_ipmc, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return SDB_E_NO_MEMORY;
    }

    return SDB_E_NONE;
}

int32
stbl_ipmc_del_ipmc_sync(stbl_ipmc_key_t *p_ipmc_key, uint32 sync)
{
    stbl_ipmc_master_t *p_master = _g_p_stbl_ipmc_master;
    stbl_ipmc_t *p_db_ipmc = NULL;

    /* 1. lookup entry exist */
    p_db_ipmc = stbl_ipmc_get_ipmc(p_ipmc_key);
    if (NULL == p_db_ipmc)
    {
        return SDB_E_NONE;
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->ipmc_hash, (void*)p_db_ipmc);

    /* 4. free db entry */
    XFREE(MEM_STBL_IPMC, p_db_ipmc);

    return SDB_E_NONE;
}

int32
stbl_ipmc_set_ipmc_field_sync(stbl_ipmc_t *p_ipmc, stbl_ipmc_field_id_t field_id, uint32 sync)
{
    stbl_ipmc_t *p_db_ipmc = NULL;

    /* 1. lookup entry exist */
    p_db_ipmc = stbl_ipmc_get_ipmc(&p_ipmc->key);
    if (NULL == p_db_ipmc)
    {
        return SDB_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case STBL_IPMC_FLD_GROUP_ID:
        p_db_ipmc->group_id = p_ipmc->group_id;
        break;

    case STBL_IPMC_FLD_NEXTHOP_ID:
        p_db_ipmc->nexthop_id = p_ipmc->nexthop_id;
        break;

    case STBL_IPMC_FLD_COUNTER_ID:
        p_db_ipmc->counter_id = p_ipmc->counter_id;
        break;

    case STBL_IPMC_FLD_NEXTHOP_GROUP_ID:
        p_db_ipmc->nexthop_group_id = p_ipmc->nexthop_group_id;
        break;

    case STBL_IPMC_FLD_RPF_ROUTER_INTERFACE:
        p_db_ipmc->rpf_router_interface = p_ipmc->rpf_router_interface;
        break;

    case STBL_IPMC_FLD_MAX:
        sal_memcpy(p_db_ipmc, p_ipmc, sizeof(stbl_ipmc_t));
        break;
    default:
        return SDB_E_NOT_SUPPORT;
    }

    return SDB_E_NONE;
}

int32
stbl_ipmc_add_ipmc(stbl_ipmc_t *p_ipmc)
{
    return stbl_ipmc_add_ipmc_sync(p_ipmc, TRUE);
}

int32
stbl_ipmc_del_ipmc(stbl_ipmc_key_t *p_ipmc_key)
{
    return stbl_ipmc_del_ipmc_sync(p_ipmc_key, TRUE);
}

int32
stbl_ipmc_set_ipmc_field(stbl_ipmc_t *p_ipmc, stbl_ipmc_field_id_t field_id)
{
    return stbl_ipmc_set_ipmc_field_sync(p_ipmc, field_id, TRUE);
}

stbl_ipmc_t*
stbl_ipmc_get_ipmc(stbl_ipmc_key_t *p_ipmc_key)
{
    stbl_ipmc_master_t *p_master = _g_p_stbl_ipmc_master;
    stbl_ipmc_t lkp;

    sal_memcpy(&lkp.key, p_ipmc_key, sizeof(stbl_ipmc_key_t));
    return ctclib_hash_lookup(p_master->ipmc_hash, &lkp);
}

char*
stbl_ipmc_key_val2str(stbl_ipmc_t *p_ipmc, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[SDB_MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            sdb_ipmc_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_ipmc->key));
    return str;
}

int32
stbl_ipmc_dump_one(stbl_ipmc_t *p_ipmc, stbl_iter_args_t *pargs)
{
    sdb_node_t *p_node = sdb_get_tbl(STBL_IPMC);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    sdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[SDB_MAX_CMD_STR_LEN];

    if (SDB_FLD_MATCH(STBL_IPMC_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_IPMC_FLD_KEY].name, 
            sdb_ipmc_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_ipmc->key));
    }
    if (SDB_FLD_MATCH(STBL_IPMC_FLD_GROUP_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_IPMC_FLD_GROUP_ID].name,
            p_ipmc->group_id);
    }
    if (SDB_FLD_MATCH(STBL_IPMC_FLD_NEXTHOP_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_IPMC_FLD_NEXTHOP_ID].name,
            p_ipmc->nexthop_id);
    }
    if (SDB_FLD_MATCH(STBL_IPMC_FLD_COUNTER_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_IPMC_FLD_COUNTER_ID].name,
            p_ipmc->counter_id);
    }
    if (SDB_FLD_MATCH(STBL_IPMC_FLD_NEXTHOP_GROUP_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_IPMC_FLD_NEXTHOP_GROUP_ID].name, 
            sdb_object_id_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_ipmc->nexthop_group_id));
    }
    if (SDB_FLD_MATCH(STBL_IPMC_FLD_RPF_ROUTER_INTERFACE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_IPMC_FLD_RPF_ROUTER_INTERFACE].name, 
            sdb_object_id_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_ipmc->rpf_router_interface));
    }

    sal_fprintf(fp, "\n");

    return SDB_E_NONE;
}

int32
stbl_ipmc_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs)
{
    stbl_ipmc_master_t *p_master = _g_p_stbl_ipmc_master;
    if (NULL == p_master)
    {
        return SDB_E_NONE;
    }

    ctclib_hash_iterate2(p_master->ipmc_hash, stbl_hash_iter_adpt_fn, fn, pargs);
    return SDB_E_NONE;
}

stbl_ipmc_master_t*
stbl_ipmc_get_master()
{
    return _g_p_stbl_ipmc_master;
}

stbl_ipmc_master_t*
stbl_ipmc_init_ipmc()
{
    _g_p_stbl_ipmc_master = XCALLOC(MEM_TBL_MASTER, sizeof(stbl_ipmc_master_t));
    _g_p_stbl_ipmc_master->ipmc_hash = ctclib_hash_create(_stbl_ipmc_hash_make, _stbl_ipmc_hash_cmp);
    return _g_p_stbl_ipmc_master;
}

