
#include "sdb.h"
#include "gen/stbl_route_define.h"
#include "gen/stbl_route.h"
#include "sdb_data_cmp.h"

stbl_route_master_t *_g_p_stbl_route_master = NULL;

static uint32
_stbl_route_hash_make(stbl_route_t *p_route)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_route->key;
    for (index = 0; index < sizeof(p_route->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_stbl_route_hash_cmp(void *p_arg1, void *p_arg2)
{
    stbl_route_t *p_route1 = (stbl_route_t*)p_arg1;
    stbl_route_t *p_route2 = (stbl_route_t*)p_arg2;

    if (0 == sal_memcmp(&p_route1->key, &p_route2->key, sizeof(stbl_route_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
stbl_route_add_route_sync(stbl_route_t *p_route, uint32 sync)
{
    stbl_route_master_t *p_master = _g_p_stbl_route_master;
    stbl_route_t *p_db_route = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (stbl_route_get_route(&p_route->key))
    {
        return SDB_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_route = XCALLOC(MEM_STBL_ROUTE, sizeof(stbl_route_t));
    if (NULL == p_db_route)
    {
        return SDB_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_route, p_route, sizeof(stbl_route_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->route_hash, (void*)p_db_route, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return SDB_E_NO_MEMORY;
    }

    return SDB_E_NONE;
}

int32
stbl_route_del_route_sync(stbl_route_key_t *p_route_key, uint32 sync)
{
    stbl_route_master_t *p_master = _g_p_stbl_route_master;
    stbl_route_t *p_db_route = NULL;

    /* 1. lookup entry exist */
    p_db_route = stbl_route_get_route(p_route_key);
    if (NULL == p_db_route)
    {
        return SDB_E_NONE;
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->route_hash, (void*)p_db_route);

    /* 4. free db entry */
    XFREE(MEM_STBL_ROUTE, p_db_route);

    return SDB_E_NONE;
}

int32
stbl_route_set_route_field_sync(stbl_route_t *p_route, stbl_route_field_id_t field_id, uint32 sync)
{
    stbl_route_t *p_db_route = NULL;

    /* 1. lookup entry exist */
    p_db_route = stbl_route_get_route(&p_route->key);
    if (NULL == p_db_route)
    {
        return SDB_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case STBL_ROUTE_FLD_RIF_OID:
        p_db_route->rif_oid = p_route->rif_oid;
        break;

    case STBL_ROUTE_FLD_NH_OID:
        p_db_route->nh_oid = p_route->nh_oid;
        break;

    case STBL_ROUTE_FLD_PRIORITY:
        p_db_route->priority = p_route->priority;
        break;

    case STBL_ROUTE_FLD_ACTION:
        p_db_route->action = p_route->action;
        break;

    case STBL_ROUTE_FLD_L3IF_TYPE:
        p_db_route->l3if_type = p_route->l3if_type;
        break;

    case STBL_ROUTE_FLD_CATEGORY_ID:
        p_db_route->category_id = p_route->category_id;
        break;

    case STBL_ROUTE_FLD_MAX:
        sal_memcpy(p_db_route, p_route, sizeof(stbl_route_t));
        break;
    default:
        return SDB_E_NOT_SUPPORT;
    }

    return SDB_E_NONE;
}

int32
stbl_route_add_route(stbl_route_t *p_route)
{
    return stbl_route_add_route_sync(p_route, TRUE);
}

int32
stbl_route_del_route(stbl_route_key_t *p_route_key)
{
    return stbl_route_del_route_sync(p_route_key, TRUE);
}

int32
stbl_route_set_route_field(stbl_route_t *p_route, stbl_route_field_id_t field_id)
{
    return stbl_route_set_route_field_sync(p_route, field_id, TRUE);
}

stbl_route_t*
stbl_route_get_route(stbl_route_key_t *p_route_key)
{
    stbl_route_master_t *p_master = _g_p_stbl_route_master;
    stbl_route_t lkp;

    sal_memcpy(&lkp.key, p_route_key, sizeof(stbl_route_key_t));
    return ctclib_hash_lookup(p_master->route_hash, &lkp);
}

char*
stbl_route_key_val2str(stbl_route_t *p_route, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[SDB_MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            sdb_ipuc_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_route->key));
    return str;
}

int32
stbl_route_dump_one(stbl_route_t *p_route, stbl_iter_args_t *pargs)
{
    sdb_node_t *p_node = sdb_get_tbl(STBL_ROUTE);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    sdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[SDB_MAX_CMD_STR_LEN];

    if (SDB_FLD_MATCH(STBL_ROUTE_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_ROUTE_FLD_KEY].name, 
            sdb_ipuc_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_route->key));
    }
    if (SDB_FLD_MATCH(STBL_ROUTE_FLD_RIF_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_ROUTE_FLD_RIF_OID].name, 
            sdb_object_id_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_route->rif_oid));
    }
    if (SDB_FLD_MATCH(STBL_ROUTE_FLD_NH_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_ROUTE_FLD_NH_OID].name, 
            sdb_object_id_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_route->nh_oid));
    }
    if (SDB_FLD_MATCH(STBL_ROUTE_FLD_PRIORITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_ROUTE_FLD_PRIORITY].name,
            p_route->priority);
    }
    if (SDB_FLD_MATCH(STBL_ROUTE_FLD_ACTION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_ROUTE_FLD_ACTION].name,
            p_route->action);
    }
    if (SDB_FLD_MATCH(STBL_ROUTE_FLD_L3IF_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_ROUTE_FLD_L3IF_TYPE].name,
            p_route->l3if_type);
    }
    if (SDB_FLD_MATCH(STBL_ROUTE_FLD_CATEGORY_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_ROUTE_FLD_CATEGORY_ID].name,
            p_route->category_id);
    }

    sal_fprintf(fp, "\n");

    return SDB_E_NONE;
}

int32
stbl_route_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs)
{
    stbl_route_master_t *p_master = _g_p_stbl_route_master;
    if (NULL == p_master)
    {
        return SDB_E_NONE;
    }

    ctclib_hash_iterate2(p_master->route_hash, stbl_hash_iter_adpt_fn, fn, pargs);
    return SDB_E_NONE;
}

stbl_route_master_t*
stbl_route_get_master()
{
    return _g_p_stbl_route_master;
}

stbl_route_master_t*
stbl_route_init_route()
{
    _g_p_stbl_route_master = XCALLOC(MEM_TBL_MASTER, sizeof(stbl_route_master_t));
    _g_p_stbl_route_master->route_hash = ctclib_hash_create(_stbl_route_hash_make, _stbl_route_hash_cmp);
    return _g_p_stbl_route_master;
}

