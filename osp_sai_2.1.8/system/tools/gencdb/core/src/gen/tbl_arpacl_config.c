
#include "proto.h"
#include "gen/tbl_arpacl_config_define.h"
#include "gen/tbl_arpacl_config.h"
#include "cdb_data_cmp.h"

tbl_arpacl_config_master_t *_g_p_tbl_arpacl_config_master = NULL;

static uint32
_tbl_arpacl_config_hash_make(tbl_arpacl_config_t *p_arpacl_config)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_arpacl_config->key;
    for (index = 0; index < sizeof(p_arpacl_config->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_arpacl_config_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_arpacl_config_t *p_arpacl_config1 = (tbl_arpacl_config_t*)p_arg1;
    tbl_arpacl_config_t *p_arpacl_config2 = (tbl_arpacl_config_t*)p_arg2;

    if (0 == sal_memcmp(&p_arpacl_config1->key, &p_arpacl_config2->key, sizeof(tbl_arpacl_config_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_arpacl_config_add_arpacl_config_sync(tbl_arpacl_config_t *p_arpacl_config, uint32 sync)
{
    tbl_arpacl_config_master_t *p_master = _g_p_tbl_arpacl_config_master;
    tbl_arpacl_config_t *p_db_arpacl_config = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_arpacl_config_get_arpacl_config(&p_arpacl_config->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_arpacl_config = XCALLOC(MEM_TBL_ARPACL_CONFIG, sizeof(tbl_arpacl_config_t));
    if (NULL == p_db_arpacl_config)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_arpacl_config, p_arpacl_config, sizeof(tbl_arpacl_config_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->arpacl_config_hash, (void*)p_db_arpacl_config, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->arpacl_config_list, p_db_arpacl_config);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_ARPACL_CONFIG, p_db_arpacl_config);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_ARPACL_CONFIG, p_db_arpacl_config);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_arpacl_config_del_arpacl_config_sync(tbl_arpacl_config_key_t *p_arpacl_config_key, uint32 sync)
{
    tbl_arpacl_config_master_t *p_master = _g_p_tbl_arpacl_config_master;
    tbl_arpacl_config_t *p_db_arpacl_config = NULL;

    /* 1. lookup entry exist */
    p_db_arpacl_config = tbl_arpacl_config_get_arpacl_config(p_arpacl_config_key);
    if (NULL == p_db_arpacl_config)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_ARPACL_CONFIG, p_db_arpacl_config);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_ARPACL_CONFIG, p_db_arpacl_config);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->arpacl_config_hash, (void*)p_db_arpacl_config);
    ctclib_slistnode_delete(p_master->arpacl_config_list, p_db_arpacl_config);

    /* 4. free db entry */
    XFREE(MEM_TBL_ARPACL_CONFIG, p_db_arpacl_config);

    return PM_E_NONE;
}

int32
tbl_arpacl_config_set_arpacl_config_field_sync(tbl_arpacl_config_t *p_arpacl_config, tbl_arpacl_config_field_id_t field_id, uint32 sync)
{
    tbl_arpacl_config_t *p_db_arpacl_config = NULL;

    /* 1. lookup entry exist */
    p_db_arpacl_config = tbl_arpacl_config_get_arpacl_config(&p_arpacl_config->key);
    if (NULL == p_db_arpacl_config)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_ARPACL_CONFIG_FLD_SEQ_REF:
        p_db_arpacl_config->seq_ref = p_arpacl_config->seq_ref;
        break;

    case TBL_ARPACL_CONFIG_FLD_MAX:
        sal_memcpy(p_db_arpacl_config, p_arpacl_config, sizeof(tbl_arpacl_config_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_ARPACL_CONFIG, field_id, p_db_arpacl_config);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_ARPACL_CONFIG, field_id, p_db_arpacl_config);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_arpacl_config_add_arpacl_config(tbl_arpacl_config_t *p_arpacl_config)
{
    return tbl_arpacl_config_add_arpacl_config_sync(p_arpacl_config, TRUE);
}

int32
tbl_arpacl_config_del_arpacl_config(tbl_arpacl_config_key_t *p_arpacl_config_key)
{
    return tbl_arpacl_config_del_arpacl_config_sync(p_arpacl_config_key, TRUE);
}

int32
tbl_arpacl_config_set_arpacl_config_field(tbl_arpacl_config_t *p_arpacl_config, tbl_arpacl_config_field_id_t field_id)
{
    return tbl_arpacl_config_set_arpacl_config_field_sync(p_arpacl_config, field_id, TRUE);
}

tbl_arpacl_config_t*
tbl_arpacl_config_get_arpacl_config(tbl_arpacl_config_key_t *p_arpacl_config_key)
{
    tbl_arpacl_config_master_t *p_master = _g_p_tbl_arpacl_config_master;
    tbl_arpacl_config_t lkp;

    sal_memcpy(&lkp.key, p_arpacl_config_key, sizeof(tbl_arpacl_config_key_t));
    return ctclib_hash_lookup(p_master->arpacl_config_hash, &lkp);
}

char*
tbl_arpacl_config_key_val2str(tbl_arpacl_config_t *p_arpacl_config, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_arpacl_config->key.name);
    return str;
}

int32
tbl_arpacl_config_key_str2val(char *str, tbl_arpacl_config_t *p_arpacl_config)
{
    int32 ret = 0;

    sal_strcpy(p_arpacl_config->key.name, str);
    return ret;
}

char*
tbl_arpacl_config_key_name_dump(tbl_arpacl_config_t *p_arpacl_config, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ARPACL_CONFIG);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_arpacl_config->key.name);
    return str;
}

char*
tbl_arpacl_config_key_value_dump(tbl_arpacl_config_t *p_arpacl_config, char *str)
{
    sal_sprintf(str, "%s", p_arpacl_config->key.name);
    return str;
}

char*
tbl_arpacl_config_field_name_dump(tbl_arpacl_config_t *p_arpacl_config, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_ARPACL_CONFIG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_ARPACL_CONFIG_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_arpacl_config_field_value_dump(tbl_arpacl_config_t *p_arpacl_config, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_ARPACL_CONFIG_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_arpacl_config->key.name);
    }
    if (FLD_MATCH(TBL_ARPACL_CONFIG_FLD_SEQ_REF, field_id))
    {
        sal_sprintf(str, "%u", p_arpacl_config->seq_ref);
    }
    return str;
}

char**
tbl_arpacl_config_table_dump(tbl_arpacl_config_t *p_arpacl_config, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_arpacl_config_key_name_dump(p_arpacl_config, buf));
    for(i=1; i<TBL_ARPACL_CONFIG_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_arpacl_config_field_name_dump(p_arpacl_config, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_arpacl_config_field_value_dump(p_arpacl_config, i, buf));
    }
    return str;
}

int32
tbl_arpacl_config_field_value_parser(char *str, int32 field_id, tbl_arpacl_config_t *p_arpacl_config)
{
    if (FLD_MATCH(TBL_ARPACL_CONFIG_FLD_KEY, field_id))
    {
        sal_strcpy(p_arpacl_config->key.name, str);
    }
    if (FLD_MATCH(TBL_ARPACL_CONFIG_FLD_SEQ_REF, field_id))
    {
        int32 ret;
        p_arpacl_config->seq_ref = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_arpacl_config_table_parser(char** array, char* key_value,tbl_arpacl_config_t *p_arpacl_config)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_ARPACL_CONFIG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_arpacl_config_key_str2val(key_value, p_arpacl_config));

    for(i=1; i<TBL_ARPACL_CONFIG_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_arpacl_config_field_value_parser( array[j++], i, p_arpacl_config));
    }

    return PM_E_NONE;
}

int32
tbl_arpacl_config_dump_one(tbl_arpacl_config_t *p_arpacl_config, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ARPACL_CONFIG);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_ARPACL_CONFIG_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ARPACL_CONFIG_FLD_KEY].name,
            p_arpacl_config->key.name);
    }
    if (FLD_MATCH(TBL_ARPACL_CONFIG_FLD_SEQ_REF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ARPACL_CONFIG_FLD_SEQ_REF].name,
            p_arpacl_config->seq_ref);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_arpacl_config_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_arpacl_config_master_t *p_master = _g_p_tbl_arpacl_config_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_arpacl_config_t *p_db_arpacl_config = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->arpacl_config_list, p_db_arpacl_config, listnode, next)
    {
        rc |= fn(p_db_arpacl_config, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_arpacl_config_master_t*
tbl_arpacl_config_get_master()
{
    return _g_p_tbl_arpacl_config_master;
}

tbl_arpacl_config_master_t*
tbl_arpacl_config_init_arpacl_config()
{
    _g_p_tbl_arpacl_config_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_arpacl_config_master_t));
    _g_p_tbl_arpacl_config_master->arpacl_config_hash = ctclib_hash_create(_tbl_arpacl_config_hash_make, _tbl_arpacl_config_hash_cmp);
    _g_p_tbl_arpacl_config_master->arpacl_config_list = ctclib_slist_create(tbl_arpacl_config_cmp, NULL);
    return _g_p_tbl_arpacl_config_master;
}

