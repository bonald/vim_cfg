
#include "sdb.h"
#include "gen/stbl_qos_mpls_domain_define.h"
#include "gen/stbl_qos_mpls_domain.h"
#include "sdb_data_cmp.h"

stbl_qos_mpls_domain_master_t *_g_p_stbl_qos_mpls_domain_master = NULL;

static uint32
_stbl_qos_mpls_domain_hash_make(stbl_qos_mpls_domain_t *p_qos_mpls_domain)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_qos_mpls_domain->key;
    for (index = 0; index < sizeof(p_qos_mpls_domain->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_stbl_qos_mpls_domain_hash_cmp(void *p_arg1, void *p_arg2)
{
    stbl_qos_mpls_domain_t *p_qos_mpls_domain1 = (stbl_qos_mpls_domain_t*)p_arg1;
    stbl_qos_mpls_domain_t *p_qos_mpls_domain2 = (stbl_qos_mpls_domain_t*)p_arg2;

    if (0 == sal_memcmp(&p_qos_mpls_domain1->key, &p_qos_mpls_domain2->key, sizeof(stbl_qos_mpls_domain_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
stbl_qos_mpls_domain_add_qos_mpls_domain_sync(stbl_qos_mpls_domain_t *p_qos_mpls_domain, uint32 sync)
{
    stbl_qos_mpls_domain_master_t *p_master = _g_p_stbl_qos_mpls_domain_master;
    stbl_qos_mpls_domain_t *p_db_qos_mpls_domain = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (stbl_qos_mpls_domain_get_qos_mpls_domain(&p_qos_mpls_domain->key))
    {
        return SDB_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_qos_mpls_domain = XCALLOC(MEM_STBL_QOS_MPLS_DOMAIN, sizeof(stbl_qos_mpls_domain_t));
    if (NULL == p_db_qos_mpls_domain)
    {
        return SDB_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_qos_mpls_domain, p_qos_mpls_domain, sizeof(stbl_qos_mpls_domain_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->qos_mpls_domain_hash, (void*)p_db_qos_mpls_domain, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return SDB_E_NO_MEMORY;
    }

    return SDB_E_NONE;
}

int32
stbl_qos_mpls_domain_del_qos_mpls_domain_sync(stbl_qos_mpls_domain_key_t *p_qos_mpls_domain_key, uint32 sync)
{
    stbl_qos_mpls_domain_master_t *p_master = _g_p_stbl_qos_mpls_domain_master;
    stbl_qos_mpls_domain_t *p_db_qos_mpls_domain = NULL;

    /* 1. lookup entry exist */
    p_db_qos_mpls_domain = stbl_qos_mpls_domain_get_qos_mpls_domain(p_qos_mpls_domain_key);
    if (NULL == p_db_qos_mpls_domain)
    {
        return SDB_E_NONE;
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->qos_mpls_domain_hash, (void*)p_db_qos_mpls_domain);

    /* 4. free db entry */
    XFREE(MEM_STBL_QOS_MPLS_DOMAIN, p_db_qos_mpls_domain);

    return SDB_E_NONE;
}

int32
stbl_qos_mpls_domain_set_qos_mpls_domain_field_sync(stbl_qos_mpls_domain_t *p_qos_mpls_domain, stbl_qos_mpls_domain_field_id_t field_id, uint32 sync)
{
    stbl_qos_mpls_domain_t *p_db_qos_mpls_domain = NULL;

    /* 1. lookup entry exist */
    p_db_qos_mpls_domain = stbl_qos_mpls_domain_get_qos_mpls_domain(&p_qos_mpls_domain->key);
    if (NULL == p_db_qos_mpls_domain)
    {
        return SDB_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case STBL_QOS_MPLS_DOMAIN_FLD_EXP_DOMAIN:
        p_db_qos_mpls_domain->exp_domain = p_qos_mpls_domain->exp_domain;
        break;

    case STBL_QOS_MPLS_DOMAIN_FLD_MAX:
        sal_memcpy(p_db_qos_mpls_domain, p_qos_mpls_domain, sizeof(stbl_qos_mpls_domain_t));
        break;
    default:
        return SDB_E_NOT_SUPPORT;
    }

    return SDB_E_NONE;
}

int32
stbl_qos_mpls_domain_add_qos_mpls_domain(stbl_qos_mpls_domain_t *p_qos_mpls_domain)
{
    return stbl_qos_mpls_domain_add_qos_mpls_domain_sync(p_qos_mpls_domain, TRUE);
}

int32
stbl_qos_mpls_domain_del_qos_mpls_domain(stbl_qos_mpls_domain_key_t *p_qos_mpls_domain_key)
{
    return stbl_qos_mpls_domain_del_qos_mpls_domain_sync(p_qos_mpls_domain_key, TRUE);
}

int32
stbl_qos_mpls_domain_set_qos_mpls_domain_field(stbl_qos_mpls_domain_t *p_qos_mpls_domain, stbl_qos_mpls_domain_field_id_t field_id)
{
    return stbl_qos_mpls_domain_set_qos_mpls_domain_field_sync(p_qos_mpls_domain, field_id, TRUE);
}

stbl_qos_mpls_domain_t*
stbl_qos_mpls_domain_get_qos_mpls_domain(stbl_qos_mpls_domain_key_t *p_qos_mpls_domain_key)
{
    stbl_qos_mpls_domain_master_t *p_master = _g_p_stbl_qos_mpls_domain_master;
    stbl_qos_mpls_domain_t lkp;

    sal_memcpy(&lkp.key, p_qos_mpls_domain_key, sizeof(stbl_qos_mpls_domain_key_t));
    return ctclib_hash_lookup(p_master->qos_mpls_domain_hash, &lkp);
}

char*
stbl_qos_mpls_domain_key_val2str(stbl_qos_mpls_domain_t *p_qos_mpls_domain, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_qos_mpls_domain->key.mpls_label);
    return str;
}

int32
stbl_qos_mpls_domain_dump_one(stbl_qos_mpls_domain_t *p_qos_mpls_domain, stbl_iter_args_t *pargs)
{
    sdb_node_t *p_node = sdb_get_tbl(STBL_QOS_MPLS_DOMAIN);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    sdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (SDB_FLD_MATCH(STBL_QOS_MPLS_DOMAIN_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_QOS_MPLS_DOMAIN_FLD_KEY].name,
            p_qos_mpls_domain->key.mpls_label);
    }
    if (SDB_FLD_MATCH(STBL_QOS_MPLS_DOMAIN_FLD_EXP_DOMAIN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_QOS_MPLS_DOMAIN_FLD_EXP_DOMAIN].name,
            p_qos_mpls_domain->exp_domain);
    }

    sal_fprintf(fp, "\n");

    return SDB_E_NONE;
}

int32
stbl_qos_mpls_domain_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs)
{
    stbl_qos_mpls_domain_master_t *p_master = _g_p_stbl_qos_mpls_domain_master;
    if (NULL == p_master)
    {
        return SDB_E_NONE;
    }

    ctclib_hash_iterate2(p_master->qos_mpls_domain_hash, stbl_hash_iter_adpt_fn, fn, pargs);
    return SDB_E_NONE;
}

stbl_qos_mpls_domain_master_t*
stbl_qos_mpls_domain_get_master()
{
    return _g_p_stbl_qos_mpls_domain_master;
}

stbl_qos_mpls_domain_master_t*
stbl_qos_mpls_domain_init_qos_mpls_domain()
{
    _g_p_stbl_qos_mpls_domain_master = XCALLOC(MEM_TBL_MASTER, sizeof(stbl_qos_mpls_domain_master_t));
    _g_p_stbl_qos_mpls_domain_master->qos_mpls_domain_hash = ctclib_hash_create(_stbl_qos_mpls_domain_hash_make, _stbl_qos_mpls_domain_hash_cmp);
    return _g_p_stbl_qos_mpls_domain_master;
}

