
#include "sdb.h"
#include "gen/stbl_qos_mpls_flow_define.h"
#include "gen/stbl_qos_mpls_flow.h"
#include "sdb_data_cmp.h"

stbl_qos_mpls_flow_master_t *_g_p_stbl_qos_mpls_flow_master = NULL;

static uint32
_stbl_qos_mpls_flow_hash_make(stbl_qos_mpls_flow_t *p_qos_mpls_flow)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_qos_mpls_flow->key;
    for (index = 0; index < sizeof(p_qos_mpls_flow->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_stbl_qos_mpls_flow_hash_cmp(void *p_arg1, void *p_arg2)
{
    stbl_qos_mpls_flow_t *p_qos_mpls_flow1 = (stbl_qos_mpls_flow_t*)p_arg1;
    stbl_qos_mpls_flow_t *p_qos_mpls_flow2 = (stbl_qos_mpls_flow_t*)p_arg2;

    if (0 == sal_memcmp(&p_qos_mpls_flow1->key, &p_qos_mpls_flow2->key, sizeof(stbl_qos_mpls_flow_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
stbl_qos_mpls_flow_add_qos_mpls_flow_sync(stbl_qos_mpls_flow_t *p_qos_mpls_flow, uint32 sync)
{
    stbl_qos_mpls_flow_master_t *p_master = _g_p_stbl_qos_mpls_flow_master;
    stbl_qos_mpls_flow_t *p_db_qos_mpls_flow = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (stbl_qos_mpls_flow_get_qos_mpls_flow(&p_qos_mpls_flow->key))
    {
        return SDB_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_qos_mpls_flow = XCALLOC(MEM_STBL_QOS_MPLS_FLOW, sizeof(stbl_qos_mpls_flow_t));
    if (NULL == p_db_qos_mpls_flow)
    {
        return SDB_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_qos_mpls_flow, p_qos_mpls_flow, sizeof(stbl_qos_mpls_flow_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->qos_mpls_flow_hash, (void*)p_db_qos_mpls_flow, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return SDB_E_NO_MEMORY;
    }

    return SDB_E_NONE;
}

int32
stbl_qos_mpls_flow_del_qos_mpls_flow_sync(stbl_qos_mpls_flow_key_t *p_qos_mpls_flow_key, uint32 sync)
{
    stbl_qos_mpls_flow_master_t *p_master = _g_p_stbl_qos_mpls_flow_master;
    stbl_qos_mpls_flow_t *p_db_qos_mpls_flow = NULL;

    /* 1. lookup entry exist */
    p_db_qos_mpls_flow = stbl_qos_mpls_flow_get_qos_mpls_flow(p_qos_mpls_flow_key);
    if (NULL == p_db_qos_mpls_flow)
    {
        return SDB_E_NONE;
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->qos_mpls_flow_hash, (void*)p_db_qos_mpls_flow);

    /* 4. free db entry */
    XFREE(MEM_STBL_QOS_MPLS_FLOW, p_db_qos_mpls_flow);

    return SDB_E_NONE;
}

int32
stbl_qos_mpls_flow_set_qos_mpls_flow_field_sync(stbl_qos_mpls_flow_t *p_qos_mpls_flow, stbl_qos_mpls_flow_field_id_t field_id, uint32 sync)
{
    stbl_qos_mpls_flow_t *p_db_qos_mpls_flow = NULL;

    /* 1. lookup entry exist */
    p_db_qos_mpls_flow = stbl_qos_mpls_flow_get_qos_mpls_flow(&p_qos_mpls_flow->key);
    if (NULL == p_db_qos_mpls_flow)
    {
        return SDB_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case STBL_QOS_MPLS_FLOW_FLD_MPLS_FLOW_EN:
        p_db_qos_mpls_flow->mpls_flow_en = p_qos_mpls_flow->mpls_flow_en;
        break;

    case STBL_QOS_MPLS_FLOW_FLD_MAX:
        sal_memcpy(p_db_qos_mpls_flow, p_qos_mpls_flow, sizeof(stbl_qos_mpls_flow_t));
        break;
    default:
        return SDB_E_NOT_SUPPORT;
    }

    return SDB_E_NONE;
}

int32
stbl_qos_mpls_flow_add_qos_mpls_flow(stbl_qos_mpls_flow_t *p_qos_mpls_flow)
{
    return stbl_qos_mpls_flow_add_qos_mpls_flow_sync(p_qos_mpls_flow, TRUE);
}

int32
stbl_qos_mpls_flow_del_qos_mpls_flow(stbl_qos_mpls_flow_key_t *p_qos_mpls_flow_key)
{
    return stbl_qos_mpls_flow_del_qos_mpls_flow_sync(p_qos_mpls_flow_key, TRUE);
}

int32
stbl_qos_mpls_flow_set_qos_mpls_flow_field(stbl_qos_mpls_flow_t *p_qos_mpls_flow, stbl_qos_mpls_flow_field_id_t field_id)
{
    return stbl_qos_mpls_flow_set_qos_mpls_flow_field_sync(p_qos_mpls_flow, field_id, TRUE);
}

stbl_qos_mpls_flow_t*
stbl_qos_mpls_flow_get_qos_mpls_flow(stbl_qos_mpls_flow_key_t *p_qos_mpls_flow_key)
{
    stbl_qos_mpls_flow_master_t *p_master = _g_p_stbl_qos_mpls_flow_master;
    stbl_qos_mpls_flow_t lkp;

    sal_memcpy(&lkp.key, p_qos_mpls_flow_key, sizeof(stbl_qos_mpls_flow_key_t));
    return ctclib_hash_lookup(p_master->qos_mpls_flow_hash, &lkp);
}

char*
stbl_qos_mpls_flow_key_val2str(stbl_qos_mpls_flow_t *p_qos_mpls_flow, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_qos_mpls_flow->key.mpls_label);
    return str;
}

int32
stbl_qos_mpls_flow_dump_one(stbl_qos_mpls_flow_t *p_qos_mpls_flow, stbl_iter_args_t *pargs)
{
    sdb_node_t *p_node = sdb_get_tbl(STBL_QOS_MPLS_FLOW);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    sdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (SDB_FLD_MATCH(STBL_QOS_MPLS_FLOW_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_QOS_MPLS_FLOW_FLD_KEY].name,
            p_qos_mpls_flow->key.mpls_label);
    }
    if (SDB_FLD_MATCH(STBL_QOS_MPLS_FLOW_FLD_MPLS_FLOW_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_QOS_MPLS_FLOW_FLD_MPLS_FLOW_EN].name,
            p_qos_mpls_flow->mpls_flow_en);
    }

    sal_fprintf(fp, "\n");

    return SDB_E_NONE;
}

int32
stbl_qos_mpls_flow_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs)
{
    stbl_qos_mpls_flow_master_t *p_master = _g_p_stbl_qos_mpls_flow_master;
    if (NULL == p_master)
    {
        return SDB_E_NONE;
    }

    ctclib_hash_iterate2(p_master->qos_mpls_flow_hash, stbl_hash_iter_adpt_fn, fn, pargs);
    return SDB_E_NONE;
}

stbl_qos_mpls_flow_master_t*
stbl_qos_mpls_flow_get_master()
{
    return _g_p_stbl_qos_mpls_flow_master;
}

stbl_qos_mpls_flow_master_t*
stbl_qos_mpls_flow_init_qos_mpls_flow()
{
    _g_p_stbl_qos_mpls_flow_master = XCALLOC(MEM_TBL_MASTER, sizeof(stbl_qos_mpls_flow_master_t));
    _g_p_stbl_qos_mpls_flow_master->qos_mpls_flow_hash = ctclib_hash_create(_stbl_qos_mpls_flow_hash_make, _stbl_qos_mpls_flow_hash_cmp);
    return _g_p_stbl_qos_mpls_flow_master;
}

