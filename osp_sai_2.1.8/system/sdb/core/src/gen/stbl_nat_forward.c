
#include "sdb.h"
#include "gen/stbl_nat_forward_define.h"
#include "gen/stbl_nat_forward.h"
#include "sdb_data_cmp.h"

stbl_nat_forward_master_t *_g_p_stbl_nat_forward_master = NULL;

static uint32
_stbl_nat_forward_hash_make(stbl_nat_forward_t *p_nat_forward)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_nat_forward->key;
    for (index = 0; index < sizeof(p_nat_forward->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_stbl_nat_forward_hash_cmp(void *p_arg1, void *p_arg2)
{
    stbl_nat_forward_t *p_nat_forward1 = (stbl_nat_forward_t*)p_arg1;
    stbl_nat_forward_t *p_nat_forward2 = (stbl_nat_forward_t*)p_arg2;

    if (0 == sal_memcmp(&p_nat_forward1->key, &p_nat_forward2->key, sizeof(stbl_nat_forward_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
stbl_nat_forward_add_nat_forward_sync(stbl_nat_forward_t *p_nat_forward, uint32 sync)
{
    stbl_nat_forward_master_t *p_master = _g_p_stbl_nat_forward_master;
    stbl_nat_forward_t *p_db_nat_forward = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (stbl_nat_forward_get_nat_forward(&p_nat_forward->key))
    {
        return SDB_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_nat_forward = XCALLOC(MEM_STBL_NAT_FORWARD, sizeof(stbl_nat_forward_t));
    if (NULL == p_db_nat_forward)
    {
        return SDB_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_nat_forward, p_nat_forward, sizeof(stbl_nat_forward_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->nat_forward_hash, (void*)p_db_nat_forward, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return SDB_E_NO_MEMORY;
    }

    return SDB_E_NONE;
}

int32
stbl_nat_forward_del_nat_forward_sync(stbl_nat_forward_key_t *p_nat_forward_key, uint32 sync)
{
    stbl_nat_forward_master_t *p_master = _g_p_stbl_nat_forward_master;
    stbl_nat_forward_t *p_db_nat_forward = NULL;

    /* 1. lookup entry exist */
    p_db_nat_forward = stbl_nat_forward_get_nat_forward(p_nat_forward_key);
    if (NULL == p_db_nat_forward)
    {
        return SDB_E_NONE;
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->nat_forward_hash, (void*)p_db_nat_forward);

    /* 4. free db entry */
    XFREE(MEM_STBL_NAT_FORWARD, p_db_nat_forward);

    return SDB_E_NONE;
}

int32
stbl_nat_forward_set_nat_forward_field_sync(stbl_nat_forward_t *p_nat_forward, stbl_nat_forward_field_id_t field_id, uint32 sync)
{
    stbl_nat_forward_t *p_db_nat_forward = NULL;

    /* 1. lookup entry exist */
    p_db_nat_forward = stbl_nat_forward_get_nat_forward(&p_nat_forward->key);
    if (NULL == p_db_nat_forward)
    {
        return SDB_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case STBL_NAT_FORWARD_FLD_IIF_OID:
        p_db_nat_forward->iif_oid = p_nat_forward->iif_oid;
        break;

    case STBL_NAT_FORWARD_FLD_OIF_OID:
        p_db_nat_forward->oif_oid = p_nat_forward->oif_oid;
        break;

    case STBL_NAT_FORWARD_FLD_NHID:
        p_db_nat_forward->nhid = p_nat_forward->nhid;
        break;

    case STBL_NAT_FORWARD_FLD_USE_CNT:
        p_db_nat_forward->use_cnt = p_nat_forward->use_cnt;
        break;

    case STBL_NAT_FORWARD_FLD_MAX:
        sal_memcpy(p_db_nat_forward, p_nat_forward, sizeof(stbl_nat_forward_t));
        break;
    default:
        return SDB_E_NOT_SUPPORT;
    }

    return SDB_E_NONE;
}

int32
stbl_nat_forward_add_nat_forward(stbl_nat_forward_t *p_nat_forward)
{
    return stbl_nat_forward_add_nat_forward_sync(p_nat_forward, TRUE);
}

int32
stbl_nat_forward_del_nat_forward(stbl_nat_forward_key_t *p_nat_forward_key)
{
    return stbl_nat_forward_del_nat_forward_sync(p_nat_forward_key, TRUE);
}

int32
stbl_nat_forward_set_nat_forward_field(stbl_nat_forward_t *p_nat_forward, stbl_nat_forward_field_id_t field_id)
{
    return stbl_nat_forward_set_nat_forward_field_sync(p_nat_forward, field_id, TRUE);
}

stbl_nat_forward_t*
stbl_nat_forward_get_nat_forward(stbl_nat_forward_key_t *p_nat_forward_key)
{
    stbl_nat_forward_master_t *p_master = _g_p_stbl_nat_forward_master;
    stbl_nat_forward_t lkp;

    sal_memcpy(&lkp.key, p_nat_forward_key, sizeof(stbl_nat_forward_key_t));
    return ctclib_hash_lookup(p_master->nat_forward_hash, &lkp);
}

char*
stbl_nat_forward_key_val2str(stbl_nat_forward_t *p_nat_forward, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[SDB_MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            sdb_nat_forward_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_nat_forward->key));
    return str;
}

int32
stbl_nat_forward_dump_one(stbl_nat_forward_t *p_nat_forward, stbl_iter_args_t *pargs)
{
    sdb_node_t *p_node = sdb_get_tbl(STBL_NAT_FORWARD);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    sdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[SDB_MAX_CMD_STR_LEN];

    if (SDB_FLD_MATCH(STBL_NAT_FORWARD_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_NAT_FORWARD_FLD_KEY].name, 
            sdb_nat_forward_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_nat_forward->key));
    }
    if (SDB_FLD_MATCH(STBL_NAT_FORWARD_FLD_IIF_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_NAT_FORWARD_FLD_IIF_OID].name,
            p_nat_forward->iif_oid);
    }
    if (SDB_FLD_MATCH(STBL_NAT_FORWARD_FLD_OIF_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_NAT_FORWARD_FLD_OIF_OID].name,
            p_nat_forward->oif_oid);
    }
    if (SDB_FLD_MATCH(STBL_NAT_FORWARD_FLD_NHID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_NAT_FORWARD_FLD_NHID].name,
            p_nat_forward->nhid);
    }
    if (SDB_FLD_MATCH(STBL_NAT_FORWARD_FLD_USE_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_NAT_FORWARD_FLD_USE_CNT].name,
            p_nat_forward->use_cnt);
    }

    sal_fprintf(fp, "\n");

    return SDB_E_NONE;
}

int32
stbl_nat_forward_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs)
{
    stbl_nat_forward_master_t *p_master = _g_p_stbl_nat_forward_master;
    if (NULL == p_master)
    {
        return SDB_E_NONE;
    }

    ctclib_hash_iterate2(p_master->nat_forward_hash, stbl_hash_iter_adpt_fn, fn, pargs);
    return SDB_E_NONE;
}

stbl_nat_forward_master_t*
stbl_nat_forward_get_master()
{
    return _g_p_stbl_nat_forward_master;
}

stbl_nat_forward_master_t*
stbl_nat_forward_init_nat_forward()
{
    _g_p_stbl_nat_forward_master = XCALLOC(MEM_TBL_MASTER, sizeof(stbl_nat_forward_master_t));
    _g_p_stbl_nat_forward_master->nat_forward_hash = ctclib_hash_create(_stbl_nat_forward_hash_make, _stbl_nat_forward_hash_cmp);
    return _g_p_stbl_nat_forward_master;
}

