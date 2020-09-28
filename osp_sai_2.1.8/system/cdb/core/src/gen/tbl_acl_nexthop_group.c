
#include "proto.h"
#include "gen/tbl_acl_nexthop_group_define.h"
#include "gen/tbl_acl_nexthop_group.h"
#include "cdb_data_cmp.h"

tbl_acl_nexthop_group_master_t *_g_p_tbl_acl_nexthop_group_master = NULL;

static uint32
_tbl_acl_nexthop_group_hash_make(tbl_acl_nexthop_group_t *p_acl_nexthop_group)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_acl_nexthop_group->key;
    for (index = 0; index < sizeof(p_acl_nexthop_group->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_acl_nexthop_group_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_acl_nexthop_group_t *p_acl_nexthop_group1 = (tbl_acl_nexthop_group_t*)p_arg1;
    tbl_acl_nexthop_group_t *p_acl_nexthop_group2 = (tbl_acl_nexthop_group_t*)p_arg2;

    if (0 == sal_memcmp(&p_acl_nexthop_group1->key, &p_acl_nexthop_group2->key, sizeof(tbl_acl_nexthop_group_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_acl_nexthop_group_add_acl_nexthop_group_sync(tbl_acl_nexthop_group_t *p_acl_nexthop_group, uint32 sync)
{
    tbl_acl_nexthop_group_master_t *p_master = _g_p_tbl_acl_nexthop_group_master;
    tbl_acl_nexthop_group_t *p_db_acl_nexthop_group = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_acl_nexthop_group_get_acl_nexthop_group(&p_acl_nexthop_group->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_acl_nexthop_group = XCALLOC(MEM_TBL_ACL_NEXTHOP_GROUP, sizeof(tbl_acl_nexthop_group_t));
    if (NULL == p_db_acl_nexthop_group)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_acl_nexthop_group, p_acl_nexthop_group, sizeof(tbl_acl_nexthop_group_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->acl_nexthop_group_hash, (void*)p_db_acl_nexthop_group, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_ACL_NEXTHOP_GROUP, p_db_acl_nexthop_group);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_ACL_NEXTHOP_GROUP, p_db_acl_nexthop_group);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_acl_nexthop_group_del_acl_nexthop_group_sync(tbl_acl_nexthop_group_key_t *p_acl_nexthop_group_key, uint32 sync)
{
    tbl_acl_nexthop_group_master_t *p_master = _g_p_tbl_acl_nexthop_group_master;
    tbl_acl_nexthop_group_t *p_db_acl_nexthop_group = NULL;

    /* 1. lookup entry exist */
    p_db_acl_nexthop_group = tbl_acl_nexthop_group_get_acl_nexthop_group(p_acl_nexthop_group_key);
    if (NULL == p_db_acl_nexthop_group)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_ACL_NEXTHOP_GROUP, p_db_acl_nexthop_group);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_ACL_NEXTHOP_GROUP, p_db_acl_nexthop_group);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->acl_nexthop_group_hash, (void*)p_db_acl_nexthop_group);

    /* 4. free db entry */
    XFREE(MEM_TBL_ACL_NEXTHOP_GROUP, p_db_acl_nexthop_group);

    return PM_E_NONE;
}

int32
tbl_acl_nexthop_group_set_acl_nexthop_group_field_sync(tbl_acl_nexthop_group_t *p_acl_nexthop_group, tbl_acl_nexthop_group_field_id_t field_id, uint32 sync)
{
    tbl_acl_nexthop_group_t *p_db_acl_nexthop_group = NULL;

    /* 1. lookup entry exist */
    p_db_acl_nexthop_group = tbl_acl_nexthop_group_get_acl_nexthop_group(&p_acl_nexthop_group->key);
    if (NULL == p_db_acl_nexthop_group)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_ACL_NEXTHOP_GROUP_FLD_ACL_NEXTHOP_KEY:
        sal_memcpy(p_db_acl_nexthop_group->acl_nexthop_key, p_acl_nexthop_group->acl_nexthop_key, sizeof(p_acl_nexthop_group->acl_nexthop_key));
        break;

    case TBL_ACL_NEXTHOP_GROUP_FLD_MAX:
        sal_memcpy(p_db_acl_nexthop_group, p_acl_nexthop_group, sizeof(tbl_acl_nexthop_group_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_ACL_NEXTHOP_GROUP, field_id, p_db_acl_nexthop_group);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_ACL_NEXTHOP_GROUP, field_id, p_db_acl_nexthop_group);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_acl_nexthop_group_add_acl_nexthop_group(tbl_acl_nexthop_group_t *p_acl_nexthop_group)
{
    return tbl_acl_nexthop_group_add_acl_nexthop_group_sync(p_acl_nexthop_group, TRUE);
}

int32
tbl_acl_nexthop_group_del_acl_nexthop_group(tbl_acl_nexthop_group_key_t *p_acl_nexthop_group_key)
{
    return tbl_acl_nexthop_group_del_acl_nexthop_group_sync(p_acl_nexthop_group_key, TRUE);
}

int32
tbl_acl_nexthop_group_set_acl_nexthop_group_field(tbl_acl_nexthop_group_t *p_acl_nexthop_group, tbl_acl_nexthop_group_field_id_t field_id)
{
    return tbl_acl_nexthop_group_set_acl_nexthop_group_field_sync(p_acl_nexthop_group, field_id, TRUE);
}

tbl_acl_nexthop_group_t*
tbl_acl_nexthop_group_get_acl_nexthop_group(tbl_acl_nexthop_group_key_t *p_acl_nexthop_group_key)
{
    tbl_acl_nexthop_group_master_t *p_master = _g_p_tbl_acl_nexthop_group_master;
    tbl_acl_nexthop_group_t lkp;

    sal_memcpy(&lkp.key, p_acl_nexthop_group_key, sizeof(tbl_acl_nexthop_group_key_t));
    return ctclib_hash_lookup(p_master->acl_nexthop_group_hash, &lkp);
}

char*
tbl_acl_nexthop_group_key_val2str(tbl_acl_nexthop_group_t *p_acl_nexthop_group, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_acl_nexthop_group->key.nexthop_group);
    return str;
}

int32
tbl_acl_nexthop_group_key_str2val(char *str, tbl_acl_nexthop_group_t *p_acl_nexthop_group)
{
    int32 ret = 0;

    p_acl_nexthop_group->key.nexthop_group = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_acl_nexthop_group_key_name_dump(tbl_acl_nexthop_group_t *p_acl_nexthop_group, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_NEXTHOP_GROUP);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_acl_nexthop_group->key.nexthop_group);
    return str;
}

char*
tbl_acl_nexthop_group_key_value_dump(tbl_acl_nexthop_group_t *p_acl_nexthop_group, char *str)
{
    sal_sprintf(str, "%u", p_acl_nexthop_group->key.nexthop_group);
    return str;
}

char*
tbl_acl_nexthop_group_field_name_dump(tbl_acl_nexthop_group_t *p_acl_nexthop_group, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_NEXTHOP_GROUP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_ACL_NEXTHOP_GROUP_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_acl_nexthop_group_field_value_dump(tbl_acl_nexthop_group_t *p_acl_nexthop_group, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_ACL_NEXTHOP_GROUP_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_acl_nexthop_group->key.nexthop_group);
    }
    if (FLD_MATCH(TBL_ACL_NEXTHOP_GROUP_FLD_ACL_NEXTHOP_KEY, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_acl_nexthop_group->acl_nexthop_key, sizeof(p_acl_nexthop_group->acl_nexthop_key)/4);
    }
    return str;
}

char**
tbl_acl_nexthop_group_table_dump(tbl_acl_nexthop_group_t *p_acl_nexthop_group, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_acl_nexthop_group_key_name_dump(p_acl_nexthop_group, buf));
    for(i=1; i<TBL_ACL_NEXTHOP_GROUP_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_acl_nexthop_group_field_name_dump(p_acl_nexthop_group, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_acl_nexthop_group_field_value_dump(p_acl_nexthop_group, i, buf));
    }
    return str;
}

int32
tbl_acl_nexthop_group_field_value_parser(char *str, int32 field_id, tbl_acl_nexthop_group_t *p_acl_nexthop_group)
{
    if (FLD_MATCH(TBL_ACL_NEXTHOP_GROUP_FLD_KEY, field_id))
    {
        int32 ret;
        p_acl_nexthop_group->key.nexthop_group = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_NEXTHOP_GROUP_FLD_ACL_NEXTHOP_KEY, field_id))
    {
        cdb_uint32_array_str2val(str, p_acl_nexthop_group->acl_nexthop_key);
    }
    return PM_E_NONE;
}

int32
tbl_acl_nexthop_group_table_parser(char** array, char* key_value,tbl_acl_nexthop_group_t *p_acl_nexthop_group)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_NEXTHOP_GROUP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_acl_nexthop_group_key_str2val(key_value, p_acl_nexthop_group));

    for(i=1; i<TBL_ACL_NEXTHOP_GROUP_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_acl_nexthop_group_field_value_parser( array[j++], i, p_acl_nexthop_group));
    }

    return PM_E_NONE;
}

int32
tbl_acl_nexthop_group_dump_one(tbl_acl_nexthop_group_t *p_acl_nexthop_group, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_NEXTHOP_GROUP);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_ACL_NEXTHOP_GROUP_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_NEXTHOP_GROUP_FLD_KEY].name,
            p_acl_nexthop_group->key.nexthop_group);
    }
    if (FLD_MATCH(TBL_ACL_NEXTHOP_GROUP_FLD_ACL_NEXTHOP_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_NEXTHOP_GROUP_FLD_ACL_NEXTHOP_KEY].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_acl_nexthop_group->acl_nexthop_key, sizeof(p_acl_nexthop_group->acl_nexthop_key)/4));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_acl_nexthop_group_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_acl_nexthop_group_master_t *p_master = _g_p_tbl_acl_nexthop_group_master;
    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    ctclib_hash_iterate2(p_master->acl_nexthop_group_hash, tbl_hash_iter_adpt_fn, fn, pargs);
    return PM_E_NONE;
}

tbl_acl_nexthop_group_master_t*
tbl_acl_nexthop_group_get_master()
{
    return _g_p_tbl_acl_nexthop_group_master;
}

tbl_acl_nexthop_group_master_t*
tbl_acl_nexthop_group_init_acl_nexthop_group()
{
    _g_p_tbl_acl_nexthop_group_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_acl_nexthop_group_master_t));
    _g_p_tbl_acl_nexthop_group_master->acl_nexthop_group_hash = ctclib_hash_create(_tbl_acl_nexthop_group_hash_make, _tbl_acl_nexthop_group_hash_cmp);
    return _g_p_tbl_acl_nexthop_group_master;
}

