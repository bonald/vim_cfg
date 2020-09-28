
#include "sdb.h"
#include "gen/stbl_acl_tunnel_define.h"
#include "gen/stbl_acl_tunnel.h"
#include "sdb_data_cmp.h"

stbl_acl_tunnel_master_t *_g_p_stbl_acl_tunnel_master = NULL;

static uint32
_stbl_acl_tunnel_hash_make(stbl_acl_tunnel_t *p_tunnel)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_tunnel->key;
    for (index = 0; index < sizeof(p_tunnel->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_stbl_acl_tunnel_hash_cmp(void *p_arg1, void *p_arg2)
{
    stbl_acl_tunnel_t *p_tunnel1 = (stbl_acl_tunnel_t*)p_arg1;
    stbl_acl_tunnel_t *p_tunnel2 = (stbl_acl_tunnel_t*)p_arg2;

    if (0 == sal_memcmp(&p_tunnel1->key, &p_tunnel2->key, sizeof(stbl_acl_tunnel_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
stbl_acl_tunnel_add_acl_tunnel_sync(stbl_acl_tunnel_t *p_tunnel, uint32 sync)
{
    stbl_acl_tunnel_master_t *p_master = _g_p_stbl_acl_tunnel_master;
    stbl_acl_tunnel_t *p_db_tunnel = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (stbl_acl_tunnel_get_acl_tunnel(&p_tunnel->key))
    {
        return SDB_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_tunnel = XCALLOC(MEM_STBL_ACL_TUNNEL, sizeof(stbl_acl_tunnel_t));
    if (NULL == p_db_tunnel)
    {
        return SDB_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_tunnel, p_tunnel, sizeof(stbl_acl_tunnel_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->tunnel_hash, (void*)p_db_tunnel, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return SDB_E_NO_MEMORY;
    }

    return SDB_E_NONE;
}

int32
stbl_acl_tunnel_del_acl_tunnel_sync(stbl_acl_tunnel_key_t *p_tunnel_key, uint32 sync)
{
    stbl_acl_tunnel_master_t *p_master = _g_p_stbl_acl_tunnel_master;
    stbl_acl_tunnel_t *p_db_tunnel = NULL;

    /* 1. lookup entry exist */
    p_db_tunnel = stbl_acl_tunnel_get_acl_tunnel(p_tunnel_key);
    if (NULL == p_db_tunnel)
    {
        return SDB_E_NONE;
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->tunnel_hash, (void*)p_db_tunnel);

    /* 4. free db entry */
    XFREE(MEM_STBL_ACL_TUNNEL, p_db_tunnel);

    return SDB_E_NONE;
}

int32
stbl_acl_tunnel_set_acl_tunnel_field_sync(stbl_acl_tunnel_t *p_tunnel, stbl_acl_tunnel_field_id_t field_id, uint32 sync)
{
    stbl_acl_tunnel_t *p_db_tunnel = NULL;

    /* 1. lookup entry exist */
    p_db_tunnel = stbl_acl_tunnel_get_acl_tunnel(&p_tunnel->key);
    if (NULL == p_db_tunnel)
    {
        return SDB_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case STBL_ACL_TUNNEL_FLD_COUNT:
        p_db_tunnel->count = p_tunnel->count;
        break;

    case STBL_ACL_TUNNEL_FLD_ENTRY_ID:
        p_db_tunnel->entry_id = p_tunnel->entry_id;
        break;

    case STBL_ACL_TUNNEL_FLD_ERSPAN_SERVICE_ID:
        p_db_tunnel->erspan_service_id = p_tunnel->erspan_service_id;
        break;

    case STBL_ACL_TUNNEL_FLD_ERSPAN_SERVICE_GROUP_ID:
        p_db_tunnel->erspan_service_group_id = p_tunnel->erspan_service_group_id;
        break;

    case STBL_ACL_TUNNEL_FLD_ERSPAN_NEXTHOP_ID:
        p_db_tunnel->erspan_nexthop_id = p_tunnel->erspan_nexthop_id;
        break;

    case STBL_ACL_TUNNEL_FLD_MAX:
        sal_memcpy(p_db_tunnel, p_tunnel, sizeof(stbl_acl_tunnel_t));
        break;
    default:
        return SDB_E_NOT_SUPPORT;
    }

    return SDB_E_NONE;
}

int32
stbl_acl_tunnel_add_acl_tunnel(stbl_acl_tunnel_t *p_tunnel)
{
    return stbl_acl_tunnel_add_acl_tunnel_sync(p_tunnel, TRUE);
}

int32
stbl_acl_tunnel_del_acl_tunnel(stbl_acl_tunnel_key_t *p_tunnel_key)
{
    return stbl_acl_tunnel_del_acl_tunnel_sync(p_tunnel_key, TRUE);
}

int32
stbl_acl_tunnel_set_acl_tunnel_field(stbl_acl_tunnel_t *p_tunnel, stbl_acl_tunnel_field_id_t field_id)
{
    return stbl_acl_tunnel_set_acl_tunnel_field_sync(p_tunnel, field_id, TRUE);
}

stbl_acl_tunnel_t*
stbl_acl_tunnel_get_acl_tunnel(stbl_acl_tunnel_key_t *p_tunnel_key)
{
    stbl_acl_tunnel_master_t *p_master = _g_p_stbl_acl_tunnel_master;
    stbl_acl_tunnel_t lkp;

    sal_memcpy(&lkp.key, p_tunnel_key, sizeof(stbl_acl_tunnel_key_t));
    return ctclib_hash_lookup(p_master->tunnel_hash, &lkp);
}

char*
stbl_acl_tunnel_key_val2str(stbl_acl_tunnel_t *p_tunnel, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[SDB_MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            sdb_sai_acl_tunnel_key_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_tunnel->key));
    return str;
}

int32
stbl_acl_tunnel_dump_one(stbl_acl_tunnel_t *p_tunnel, stbl_iter_args_t *pargs)
{
    sdb_node_t *p_node = sdb_get_tbl(STBL_ACL_TUNNEL);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    sdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[SDB_MAX_CMD_STR_LEN];

    if (SDB_FLD_MATCH(STBL_ACL_TUNNEL_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_ACL_TUNNEL_FLD_KEY].name, 
            sdb_sai_acl_tunnel_key_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_tunnel->key));
    }
    if (SDB_FLD_MATCH(STBL_ACL_TUNNEL_FLD_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[STBL_ACL_TUNNEL_FLD_COUNT].name,
            p_tunnel->count);
    }
    if (SDB_FLD_MATCH(STBL_ACL_TUNNEL_FLD_ENTRY_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_ACL_TUNNEL_FLD_ENTRY_ID].name,
            p_tunnel->entry_id);
    }
    if (SDB_FLD_MATCH(STBL_ACL_TUNNEL_FLD_ERSPAN_SERVICE_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_ACL_TUNNEL_FLD_ERSPAN_SERVICE_ID].name,
            p_tunnel->erspan_service_id);
    }
    if (SDB_FLD_MATCH(STBL_ACL_TUNNEL_FLD_ERSPAN_SERVICE_GROUP_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_ACL_TUNNEL_FLD_ERSPAN_SERVICE_GROUP_ID].name,
            p_tunnel->erspan_service_group_id);
    }
    if (SDB_FLD_MATCH(STBL_ACL_TUNNEL_FLD_ERSPAN_NEXTHOP_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_ACL_TUNNEL_FLD_ERSPAN_NEXTHOP_ID].name,
            p_tunnel->erspan_nexthop_id);
    }

    sal_fprintf(fp, "\n");

    return SDB_E_NONE;
}

int32
stbl_acl_tunnel_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs)
{
    stbl_acl_tunnel_master_t *p_master = _g_p_stbl_acl_tunnel_master;
    if (NULL == p_master)
    {
        return SDB_E_NONE;
    }

    ctclib_hash_iterate2(p_master->tunnel_hash, stbl_hash_iter_adpt_fn, fn, pargs);
    return SDB_E_NONE;
}

stbl_acl_tunnel_master_t*
stbl_acl_tunnel_get_master()
{
    return _g_p_stbl_acl_tunnel_master;
}

stbl_acl_tunnel_master_t*
stbl_acl_tunnel_init_acl_tunnel()
{
    _g_p_stbl_acl_tunnel_master = XCALLOC(MEM_TBL_MASTER, sizeof(stbl_acl_tunnel_master_t));
    _g_p_stbl_acl_tunnel_master->tunnel_hash = ctclib_hash_create(_stbl_acl_tunnel_hash_make, _stbl_acl_tunnel_hash_cmp);
    return _g_p_stbl_acl_tunnel_master;
}

