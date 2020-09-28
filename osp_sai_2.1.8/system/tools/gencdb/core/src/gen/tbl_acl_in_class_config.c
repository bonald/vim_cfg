
#include "proto.h"
#include "gen/tbl_acl_in_class_config_define.h"
#include "gen/tbl_acl_in_class_config.h"
#include "cdb_data_cmp.h"

tbl_acl_in_class_config_master_t *_g_p_tbl_acl_in_class_config_master = NULL;

static uint32
_tbl_acl_in_class_config_hash_make(tbl_acl_in_class_config_t *p_acl_in_class_config)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_acl_in_class_config->key;
    for (index = 0; index < sizeof(p_acl_in_class_config->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_acl_in_class_config_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_acl_in_class_config_t *p_acl_in_class_config1 = (tbl_acl_in_class_config_t*)p_arg1;
    tbl_acl_in_class_config_t *p_acl_in_class_config2 = (tbl_acl_in_class_config_t*)p_arg2;

    if (0 == sal_memcmp(&p_acl_in_class_config1->key, &p_acl_in_class_config2->key, sizeof(tbl_acl_in_class_config_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_acl_in_class_config_add_acl_in_class_config_sync(tbl_acl_in_class_config_t *p_acl_in_class_config, uint32 sync)
{
    tbl_acl_in_class_config_master_t *p_master = _g_p_tbl_acl_in_class_config_master;
    tbl_acl_in_class_config_t *p_db_acl_in_class_config = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_acl_in_class_config_get_acl_in_class_config(&p_acl_in_class_config->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_acl_in_class_config = XCALLOC(MEM_TBL_ACL_IN_CLASS_CONFIG, sizeof(tbl_acl_in_class_config_t));
    if (NULL == p_db_acl_in_class_config)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_acl_in_class_config, p_acl_in_class_config, sizeof(tbl_acl_in_class_config_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->acl_in_class_config_hash, (void*)p_db_acl_in_class_config, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->acl_in_class_config_list, p_db_acl_in_class_config);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_ACL_IN_CLASS_CONFIG, p_db_acl_in_class_config);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_ACL_IN_CLASS_CONFIG, p_db_acl_in_class_config);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_acl_in_class_config_del_acl_in_class_config_sync(tbl_acl_in_class_config_key_t *p_acl_in_class_config_key, uint32 sync)
{
    tbl_acl_in_class_config_master_t *p_master = _g_p_tbl_acl_in_class_config_master;
    tbl_acl_in_class_config_t *p_db_acl_in_class_config = NULL;

    /* 1. lookup entry exist */
    p_db_acl_in_class_config = tbl_acl_in_class_config_get_acl_in_class_config(p_acl_in_class_config_key);
    if (NULL == p_db_acl_in_class_config)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_ACL_IN_CLASS_CONFIG, p_db_acl_in_class_config);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_ACL_IN_CLASS_CONFIG, p_db_acl_in_class_config);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->acl_in_class_config_hash, (void*)p_db_acl_in_class_config);
    ctclib_slistnode_delete(p_master->acl_in_class_config_list, p_db_acl_in_class_config);

    /* 4. free db entry */
    XFREE(MEM_TBL_ACL_IN_CLASS_CONFIG, p_db_acl_in_class_config);

    return PM_E_NONE;
}

int32
tbl_acl_in_class_config_set_acl_in_class_config_field_sync(tbl_acl_in_class_config_t *p_acl_in_class_config, tbl_acl_in_class_config_field_id_t field_id, uint32 sync)
{
    tbl_acl_in_class_config_t *p_db_acl_in_class_config = NULL;

    /* 1. lookup entry exist */
    p_db_acl_in_class_config = tbl_acl_in_class_config_get_acl_in_class_config(&p_acl_in_class_config->key);
    if (NULL == p_db_acl_in_class_config)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_ACL_IN_CLASS_CONFIG_FLD_ACL_PRIORITY:
        p_db_acl_in_class_config->acl_priority = p_acl_in_class_config->acl_priority;
        break;

    case TBL_ACL_IN_CLASS_CONFIG_FLD_MAX:
        sal_memcpy(p_db_acl_in_class_config, p_acl_in_class_config, sizeof(tbl_acl_in_class_config_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_ACL_IN_CLASS_CONFIG, field_id, p_db_acl_in_class_config);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_ACL_IN_CLASS_CONFIG, field_id, p_db_acl_in_class_config);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_acl_in_class_config_add_acl_in_class_config(tbl_acl_in_class_config_t *p_acl_in_class_config)
{
    return tbl_acl_in_class_config_add_acl_in_class_config_sync(p_acl_in_class_config, TRUE);
}

int32
tbl_acl_in_class_config_del_acl_in_class_config(tbl_acl_in_class_config_key_t *p_acl_in_class_config_key)
{
    return tbl_acl_in_class_config_del_acl_in_class_config_sync(p_acl_in_class_config_key, TRUE);
}

int32
tbl_acl_in_class_config_set_acl_in_class_config_field(tbl_acl_in_class_config_t *p_acl_in_class_config, tbl_acl_in_class_config_field_id_t field_id)
{
    return tbl_acl_in_class_config_set_acl_in_class_config_field_sync(p_acl_in_class_config, field_id, TRUE);
}

tbl_acl_in_class_config_t*
tbl_acl_in_class_config_get_acl_in_class_config(tbl_acl_in_class_config_key_t *p_acl_in_class_config_key)
{
    tbl_acl_in_class_config_master_t *p_master = _g_p_tbl_acl_in_class_config_master;
    tbl_acl_in_class_config_t lkp;

    sal_memcpy(&lkp.key, p_acl_in_class_config_key, sizeof(tbl_acl_in_class_config_key_t));
    return ctclib_hash_lookup(p_master->acl_in_class_config_hash, &lkp);
}

char*
tbl_acl_in_class_config_key_val2str(tbl_acl_in_class_config_t *p_acl_in_class_config, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_acl_in_class_config_key_val2str(buf, MAX_CMD_STR_LEN, &p_acl_in_class_config->key));
    return str;
}

int32
tbl_acl_in_class_config_key_str2val(char *str, tbl_acl_in_class_config_t *p_acl_in_class_config)
{
    int32 ret = 0;

    ret = cdb_acl_in_class_config_key_str2val(str, &p_acl_in_class_config->key);
    return ret;
}

char*
tbl_acl_in_class_config_key_name_dump(tbl_acl_in_class_config_t *p_acl_in_class_config, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_IN_CLASS_CONFIG);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_acl_in_class_config_key_val2str(buf, MAX_CMD_STR_LEN, &p_acl_in_class_config->key));
    return str;
}

char*
tbl_acl_in_class_config_key_value_dump(tbl_acl_in_class_config_t *p_acl_in_class_config, char *str)
{

    cdb_acl_in_class_config_key_val2str(str, MAX_CMD_STR_LEN, &p_acl_in_class_config->key);
    return str;
}

char*
tbl_acl_in_class_config_field_name_dump(tbl_acl_in_class_config_t *p_acl_in_class_config, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_IN_CLASS_CONFIG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_ACL_IN_CLASS_CONFIG_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_acl_in_class_config_field_value_dump(tbl_acl_in_class_config_t *p_acl_in_class_config, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_ACL_IN_CLASS_CONFIG_FLD_KEY, field_id))
    {
        cdb_acl_in_class_config_key_val2str(str, MAX_CMD_STR_LEN, &p_acl_in_class_config->key);
    }
    if (FLD_MATCH(TBL_ACL_IN_CLASS_CONFIG_FLD_ACL_PRIORITY, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_acl_in_class_config->acl_priority);
    }
    return str;
}

char**
tbl_acl_in_class_config_table_dump(tbl_acl_in_class_config_t *p_acl_in_class_config, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_acl_in_class_config_key_name_dump(p_acl_in_class_config, buf));
    for(i=1; i<TBL_ACL_IN_CLASS_CONFIG_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_acl_in_class_config_field_name_dump(p_acl_in_class_config, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_acl_in_class_config_field_value_dump(p_acl_in_class_config, i, buf));
    }
    return str;
}

int32
tbl_acl_in_class_config_field_value_parser(char *str, int32 field_id, tbl_acl_in_class_config_t *p_acl_in_class_config)
{
    if (FLD_MATCH(TBL_ACL_IN_CLASS_CONFIG_FLD_KEY, field_id))
    {
        cdb_acl_in_class_config_key_str2val(str, &p_acl_in_class_config->key);
    }
    if (FLD_MATCH(TBL_ACL_IN_CLASS_CONFIG_FLD_ACL_PRIORITY, field_id))
    {
        int32 ret;
        p_acl_in_class_config->acl_priority = cdb_uint64_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_acl_in_class_config_table_parser(char** array, char* key_value,tbl_acl_in_class_config_t *p_acl_in_class_config)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_IN_CLASS_CONFIG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_acl_in_class_config_key_str2val(key_value, p_acl_in_class_config));

    for(i=1; i<TBL_ACL_IN_CLASS_CONFIG_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_acl_in_class_config_field_value_parser( array[j++], i, p_acl_in_class_config));
    }

    return PM_E_NONE;
}

int32
tbl_acl_in_class_config_dump_one(tbl_acl_in_class_config_t *p_acl_in_class_config, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_IN_CLASS_CONFIG);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_ACL_IN_CLASS_CONFIG_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_IN_CLASS_CONFIG_FLD_KEY].name, 
            cdb_acl_in_class_config_key_val2str(buf, MAX_CMD_STR_LEN, &p_acl_in_class_config->key));
    }
    if (FLD_MATCH(TBL_ACL_IN_CLASS_CONFIG_FLD_ACL_PRIORITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ACL_IN_CLASS_CONFIG_FLD_ACL_PRIORITY].name,
            p_acl_in_class_config->acl_priority);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_acl_in_class_config_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_acl_in_class_config_master_t *p_master = _g_p_tbl_acl_in_class_config_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_acl_in_class_config_t *p_db_acl_in_class_config = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->acl_in_class_config_list, p_db_acl_in_class_config, listnode, next)
    {
        rc |= fn(p_db_acl_in_class_config, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_acl_in_class_config_master_t*
tbl_acl_in_class_config_get_master()
{
    return _g_p_tbl_acl_in_class_config_master;
}

tbl_acl_in_class_config_master_t*
tbl_acl_in_class_config_init_acl_in_class_config()
{
    _g_p_tbl_acl_in_class_config_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_acl_in_class_config_master_t));
    _g_p_tbl_acl_in_class_config_master->acl_in_class_config_hash = ctclib_hash_create(_tbl_acl_in_class_config_hash_make, _tbl_acl_in_class_config_hash_cmp);
    _g_p_tbl_acl_in_class_config_master->acl_in_class_config_list = ctclib_slist_create(tbl_acl_in_class_config_cmp, NULL);
    return _g_p_tbl_acl_in_class_config_master;
}


tbl_acl_in_class_config_t*
tbl_acl_in_class_config_get_acl_in_class_config_by_name(const char* class_map_name, const char* acl_name)
{
    tbl_acl_in_class_config_key_t key;

    sal_memset(&key, 0, sizeof(key));
    sal_strncpy(key.class_name, class_map_name, CMAP_NAME_SIZE);
    sal_strncpy(key.acl_name, acl_name, ACL_NAME_SIZE);
    return tbl_acl_in_class_config_get_acl_in_class_config(&key);
}

tbl_acl_in_class_config_t*
tbl_acl_in_class_config_get_acl_in_class_config_by_acl_priority(const uint64 acl_priority)
{
    tbl_acl_in_class_config_master_t *p_master = _g_p_tbl_acl_in_class_config_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_acl_in_class_config_t *p_db_acl_in_class_config = NULL;

    CTCLIB_SLIST_LOOP_DEL(p_master->acl_in_class_config_list, p_db_acl_in_class_config, listnode, next)
    {
        if(acl_priority == p_db_acl_in_class_config->acl_priority)
        {
            return p_db_acl_in_class_config;
        }
    }

    return NULL;
}


