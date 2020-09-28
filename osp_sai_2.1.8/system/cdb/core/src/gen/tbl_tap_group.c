
#include "proto.h"
#include "gen/tbl_tap_group_define.h"
#include "gen/tbl_tap_group.h"
#include "cdb_data_cmp.h"

tbl_tap_group_master_t *_g_p_tbl_tap_group_master = NULL;

static uint32
_tbl_tap_group_hash_make(tbl_tap_group_t *p_tap_group)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_tap_group->key;
    for (index = 0; index < sizeof(p_tap_group->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_tap_group_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_tap_group_t *p_tap_group1 = (tbl_tap_group_t*)p_arg1;
    tbl_tap_group_t *p_tap_group2 = (tbl_tap_group_t*)p_arg2;

    if (0 == sal_memcmp(&p_tap_group1->key, &p_tap_group2->key, sizeof(tbl_tap_group_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_tap_group_add_tap_group_sync(tbl_tap_group_t *p_tap_group, uint32 sync)
{
    tbl_tap_group_master_t *p_master = _g_p_tbl_tap_group_master;
    tbl_tap_group_t *p_db_tap_group = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_tap_group_get_tap_group(&p_tap_group->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_tap_group = XCALLOC(MEM_TBL_TAP_GROUP, sizeof(tbl_tap_group_t));
    if (NULL == p_db_tap_group)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_tap_group, p_tap_group, sizeof(tbl_tap_group_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->tap_group_hash, (void*)p_db_tap_group, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->tap_group_list, p_db_tap_group);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_TAP_GROUP, p_db_tap_group);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_TAP_GROUP, p_db_tap_group);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_tap_group_del_tap_group_sync(tbl_tap_group_key_t *p_tap_group_key, uint32 sync)
{
    tbl_tap_group_master_t *p_master = _g_p_tbl_tap_group_master;
    tbl_tap_group_t *p_db_tap_group = NULL;

    /* 1. lookup entry exist */
    p_db_tap_group = tbl_tap_group_get_tap_group(p_tap_group_key);
    if (NULL == p_db_tap_group)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_TAP_GROUP, p_db_tap_group);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_TAP_GROUP, p_db_tap_group);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->tap_group_hash, (void*)p_db_tap_group);
    ctclib_slistnode_delete(p_master->tap_group_list, p_db_tap_group);

    /* 4. free db entry */
    XFREE(MEM_TBL_TAP_GROUP, p_db_tap_group);

    return PM_E_NONE;
}

int32
tbl_tap_group_set_tap_group_field_sync(tbl_tap_group_t *p_tap_group, tbl_tap_group_field_id_t field_id, uint32 sync)
{
    tbl_tap_group_t *p_db_tap_group = NULL;

    /* 1. lookup entry exist */
    p_db_tap_group = tbl_tap_group_get_tap_group(&p_tap_group->key);
    if (NULL == p_db_tap_group)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_TAP_GROUP_FLD_ID:
        p_db_tap_group->id = p_tap_group->id;
        break;

    case TBL_TAP_GROUP_FLD_FLAGS:
        p_db_tap_group->flags = p_tap_group->flags;
        break;

    case TBL_TAP_GROUP_FLD_DESC:
        sal_memcpy(p_db_tap_group->desc, p_tap_group->desc, sizeof(p_tap_group->desc));
        break;

    case TBL_TAP_GROUP_FLD_TRUNCATION_USE:
        p_db_tap_group->truncation_use = p_tap_group->truncation_use;
        break;

    case TBL_TAP_GROUP_FLD_TAP_GROUP_OID:
        p_db_tap_group->tap_group_oid = p_tap_group->tap_group_oid;
        break;

    case TBL_TAP_GROUP_FLD_MAX:
        sal_memcpy(p_db_tap_group, p_tap_group, sizeof(tbl_tap_group_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_TAP_GROUP, field_id, p_db_tap_group);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_TAP_GROUP, field_id, p_db_tap_group);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_tap_group_add_tap_group(tbl_tap_group_t *p_tap_group)
{
    return tbl_tap_group_add_tap_group_sync(p_tap_group, TRUE);
}

int32
tbl_tap_group_del_tap_group(tbl_tap_group_key_t *p_tap_group_key)
{
    return tbl_tap_group_del_tap_group_sync(p_tap_group_key, TRUE);
}

int32
tbl_tap_group_set_tap_group_field(tbl_tap_group_t *p_tap_group, tbl_tap_group_field_id_t field_id)
{
    return tbl_tap_group_set_tap_group_field_sync(p_tap_group, field_id, TRUE);
}

tbl_tap_group_t*
tbl_tap_group_get_tap_group(tbl_tap_group_key_t *p_tap_group_key)
{
    tbl_tap_group_master_t *p_master = _g_p_tbl_tap_group_master;
    tbl_tap_group_t lkp;

    sal_memcpy(&lkp.key, p_tap_group_key, sizeof(tbl_tap_group_key_t));
    return ctclib_hash_lookup(p_master->tap_group_hash, &lkp);
}

char*
tbl_tap_group_key_val2str(tbl_tap_group_t *p_tap_group, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_tap_group->key.name);
    return str;
}

int32
tbl_tap_group_key_str2val(char *str, tbl_tap_group_t *p_tap_group)
{
    int32 ret = 0;

    sal_strcpy(p_tap_group->key.name, str);
    return ret;
}

char*
tbl_tap_group_key_name_dump(tbl_tap_group_t *p_tap_group, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_TAP_GROUP);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_tap_group->key.name);
    return str;
}

char*
tbl_tap_group_key_value_dump(tbl_tap_group_t *p_tap_group, char *str)
{
    sal_sprintf(str, "%s", p_tap_group->key.name);
    return str;
}

char*
tbl_tap_group_field_name_dump(tbl_tap_group_t *p_tap_group, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_TAP_GROUP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_TAP_GROUP_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_tap_group_field_value_dump(tbl_tap_group_t *p_tap_group, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_TAP_GROUP_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_tap_group->key.name);
    }
    if (FLD_MATCH(TBL_TAP_GROUP_FLD_ID, field_id))
    {
        sal_sprintf(str, "%u", p_tap_group->id);
    }
    if (FLD_MATCH(TBL_TAP_GROUP_FLD_FLAGS, field_id))
    {
        sal_sprintf(str, "%u", p_tap_group->flags);
    }
    if (FLD_MATCH(TBL_TAP_GROUP_FLD_DESC, field_id))
    {
        sal_sprintf(str, "%s", p_tap_group->desc);
    }
    if (FLD_MATCH(TBL_TAP_GROUP_FLD_TRUNCATION_USE, field_id))
    {
        sal_sprintf(str, "%u", p_tap_group->truncation_use);
    }
    if (FLD_MATCH(TBL_TAP_GROUP_FLD_TAP_GROUP_OID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_tap_group->tap_group_oid);
    }
    return str;
}

char**
tbl_tap_group_table_dump(tbl_tap_group_t *p_tap_group, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_tap_group_key_name_dump(p_tap_group, buf));
    for(i=1; i<TBL_TAP_GROUP_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_tap_group_field_name_dump(p_tap_group, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_tap_group_field_value_dump(p_tap_group, i, buf));
    }
    return str;
}

int32
tbl_tap_group_field_value_parser(char *str, int32 field_id, tbl_tap_group_t *p_tap_group)
{
    if (FLD_MATCH(TBL_TAP_GROUP_FLD_KEY, field_id))
    {
        sal_strcpy(p_tap_group->key.name, str);
    }
    if (FLD_MATCH(TBL_TAP_GROUP_FLD_ID, field_id))
    {
        int32 ret;
        p_tap_group->id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TAP_GROUP_FLD_FLAGS, field_id))
    {
        int32 ret;
        p_tap_group->flags = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TAP_GROUP_FLD_DESC, field_id))
    {
        sal_strcpy(p_tap_group->desc, str);
    }
    if (FLD_MATCH(TBL_TAP_GROUP_FLD_TRUNCATION_USE, field_id))
    {
        int32 ret;
        p_tap_group->truncation_use = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TAP_GROUP_FLD_TAP_GROUP_OID, field_id))
    {
        int32 ret;
        p_tap_group->tap_group_oid = cdb_uint64_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_tap_group_table_parser(char** array, char* key_value,tbl_tap_group_t *p_tap_group)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_TAP_GROUP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_tap_group_key_str2val(key_value, p_tap_group));

    for(i=1; i<TBL_TAP_GROUP_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_tap_group_field_value_parser( array[j++], i, p_tap_group));
    }

    return PM_E_NONE;
}

int32
tbl_tap_group_dump_one(tbl_tap_group_t *p_tap_group, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_TAP_GROUP);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_TAP_GROUP_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_TAP_GROUP_FLD_KEY].name,
            p_tap_group->key.name);
    }
    if (FLD_MATCH(TBL_TAP_GROUP_FLD_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TAP_GROUP_FLD_ID].name,
            p_tap_group->id);
    }
    if (FLD_MATCH(TBL_TAP_GROUP_FLD_FLAGS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TAP_GROUP_FLD_FLAGS].name,
            p_tap_group->flags);
    }
    if (FLD_MATCH(TBL_TAP_GROUP_FLD_DESC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_TAP_GROUP_FLD_DESC].name,
            p_tap_group->desc);
    }
    if (FLD_MATCH(TBL_TAP_GROUP_FLD_TRUNCATION_USE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TAP_GROUP_FLD_TRUNCATION_USE].name,
            p_tap_group->truncation_use);
    }
    if (FLD_MATCH(TBL_TAP_GROUP_FLD_TAP_GROUP_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_TAP_GROUP_FLD_TAP_GROUP_OID].name,
            p_tap_group->tap_group_oid);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_tap_group_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_tap_group_master_t *p_master = _g_p_tbl_tap_group_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_tap_group_t *p_db_tap_group = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->tap_group_list, p_db_tap_group, listnode, next)
    {
        rc |= fn(p_db_tap_group, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_tap_group_master_t*
tbl_tap_group_get_master()
{
    return _g_p_tbl_tap_group_master;
}

tbl_tap_group_master_t*
tbl_tap_group_init_tap_group()
{
    _g_p_tbl_tap_group_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_tap_group_master_t));
    _g_p_tbl_tap_group_master->tap_group_hash = ctclib_hash_create(_tbl_tap_group_hash_make, _tbl_tap_group_hash_cmp);
    _g_p_tbl_tap_group_master->tap_group_list = ctclib_slist_create(tbl_tap_group_cmp, NULL);
    return _g_p_tbl_tap_group_master;
}


int32
tbl_tap_group_get_truncation_rcs(tbl_tap_group_t *p_tap_grp, tbl_iter_args_t *pargs)
{
    uint32 *num = pargs->argv[0];

    if (p_tap_grp->truncation_use)
    {
        *num += 1;
    }
    return PM_E_NONE;
}

int32
tbl_tap_group_get_truncation_count()
{
    tbl_iter_args_t args;
    int32 num = 0;
    args.argv[0] = &num;
    tbl_tap_group_iterate((TBL_ITER_CB_FUNC)tbl_tap_group_get_truncation_rcs, &args);
    return num;
}


