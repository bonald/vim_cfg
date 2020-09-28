
#include "proto.h"
#include "gen/tbl_cfm_maid_define.h"
#include "gen/tbl_cfm_maid.h"
#include "cdb_data_cmp.h"

tbl_cfm_maid_master_t *_g_p_tbl_cfm_maid_master = NULL;

static uint32
_tbl_cfm_maid_hash_make(tbl_cfm_maid_t *p_maid)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_maid->key;
    for (index = 0; index < sizeof(p_maid->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_cfm_maid_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_cfm_maid_t *p_maid1 = (tbl_cfm_maid_t*)p_arg1;
    tbl_cfm_maid_t *p_maid2 = (tbl_cfm_maid_t*)p_arg2;

    if (0 == sal_memcmp(&p_maid1->key, &p_maid2->key, sizeof(tbl_cfm_maid_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_cfm_maid_add_cfm_maid_sync(tbl_cfm_maid_t *p_maid, uint32 sync)
{
    tbl_cfm_maid_master_t *p_master = _g_p_tbl_cfm_maid_master;
    tbl_cfm_maid_t *p_db_maid = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_cfm_maid_get_cfm_maid(&p_maid->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_maid = XCALLOC(MEM_TBL_CFM_MAID, sizeof(tbl_cfm_maid_t));
    if (NULL == p_db_maid)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_maid, p_maid, sizeof(tbl_cfm_maid_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->cfm_maid_hash, (void*)p_db_maid, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->cfm_maid_list, p_db_maid);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_CFM_MAID, p_db_maid);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_CFM_MAID, p_db_maid);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_cfm_maid_del_cfm_maid_sync(tbl_cfm_maid_key_t *p_maid_key, uint32 sync)
{
    tbl_cfm_maid_master_t *p_master = _g_p_tbl_cfm_maid_master;
    tbl_cfm_maid_t *p_db_maid = NULL;

    /* 1. lookup entry exist */
    p_db_maid = tbl_cfm_maid_get_cfm_maid(p_maid_key);
    if (NULL == p_db_maid)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_CFM_MAID, p_db_maid);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_CFM_MAID, p_db_maid);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->cfm_maid_hash, (void*)p_db_maid);
    ctclib_slistnode_delete(p_master->cfm_maid_list, p_db_maid);

    /* 4. free db entry */
    XFREE(MEM_TBL_CFM_MAID, p_db_maid);

    return PM_E_NONE;
}

int32
tbl_cfm_maid_set_cfm_maid_field_sync(tbl_cfm_maid_t *p_maid, tbl_cfm_maid_field_id_t field_id, uint32 sync)
{
    tbl_cfm_maid_t *p_db_maid = NULL;

    /* 1. lookup entry exist */
    p_db_maid = tbl_cfm_maid_get_cfm_maid(&p_maid->key);
    if (NULL == p_db_maid)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_CFM_MAID_FLD_MAX:
        sal_memcpy(p_db_maid, p_maid, sizeof(tbl_cfm_maid_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_CFM_MAID, field_id, p_db_maid);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_CFM_MAID, field_id, p_db_maid);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_cfm_maid_add_cfm_maid(tbl_cfm_maid_t *p_maid)
{
    return tbl_cfm_maid_add_cfm_maid_sync(p_maid, TRUE);
}

int32
tbl_cfm_maid_del_cfm_maid(tbl_cfm_maid_key_t *p_maid_key)
{
    return tbl_cfm_maid_del_cfm_maid_sync(p_maid_key, TRUE);
}

int32
tbl_cfm_maid_set_cfm_maid_field(tbl_cfm_maid_t *p_maid, tbl_cfm_maid_field_id_t field_id)
{
    return tbl_cfm_maid_set_cfm_maid_field_sync(p_maid, field_id, TRUE);
}

tbl_cfm_maid_t*
tbl_cfm_maid_get_cfm_maid(tbl_cfm_maid_key_t *p_maid_key)
{
    tbl_cfm_maid_master_t *p_master = _g_p_tbl_cfm_maid_master;
    tbl_cfm_maid_t lkp;

    sal_memcpy(&lkp.key, p_maid_key, sizeof(tbl_cfm_maid_key_t));
    return ctclib_hash_lookup(p_master->cfm_maid_hash, &lkp);
}

char*
tbl_cfm_maid_key_val2str(tbl_cfm_maid_t *p_maid, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_maid->key.name);
    return str;
}

int32
tbl_cfm_maid_key_str2val(char *str, tbl_cfm_maid_t *p_maid)
{
    int32 ret = 0;

    sal_strcpy(p_maid->key.name, str);
    return ret;
}

char*
tbl_cfm_maid_key_name_dump(tbl_cfm_maid_t *p_maid, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CFM_MAID);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_maid->key.name);
    return str;
}

char*
tbl_cfm_maid_key_value_dump(tbl_cfm_maid_t *p_maid, char *str)
{
    sal_sprintf(str, "%s", p_maid->key.name);
    return str;
}

char*
tbl_cfm_maid_field_name_dump(tbl_cfm_maid_t *p_maid, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_CFM_MAID);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_CFM_MAID_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_cfm_maid_field_value_dump(tbl_cfm_maid_t *p_maid, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_CFM_MAID_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_maid->key.name);
    }
    return str;
}

char**
tbl_cfm_maid_table_dump(tbl_cfm_maid_t *p_maid, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_cfm_maid_key_name_dump(p_maid, buf));
    for(i=1; i<TBL_CFM_MAID_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_cfm_maid_field_name_dump(p_maid, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_cfm_maid_field_value_dump(p_maid, i, buf));
    }
    return str;
}

int32
tbl_cfm_maid_field_value_parser(char *str, int32 field_id, tbl_cfm_maid_t *p_maid)
{
    if (FLD_MATCH(TBL_CFM_MAID_FLD_KEY, field_id))
    {
        sal_strcpy(p_maid->key.name, str);
    }
    return PM_E_NONE;
}

int32
tbl_cfm_maid_table_parser(char** array, char* key_value,tbl_cfm_maid_t *p_maid)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_CFM_MAID);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_cfm_maid_key_str2val(key_value, p_maid));

    for(i=1; i<TBL_CFM_MAID_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_cfm_maid_field_value_parser( array[j++], i, p_maid));
    }

    return PM_E_NONE;
}

int32
tbl_cfm_maid_dump_one(tbl_cfm_maid_t *p_maid, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CFM_MAID);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_CFM_MAID_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CFM_MAID_FLD_KEY].name,
            p_maid->key.name);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_cfm_maid_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_cfm_maid_master_t *p_master = _g_p_tbl_cfm_maid_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_cfm_maid_t *p_db_maid = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->cfm_maid_list, p_db_maid, listnode, next)
    {
        rc |= fn(p_db_maid, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_cfm_maid_master_t*
tbl_cfm_maid_get_master()
{
    return _g_p_tbl_cfm_maid_master;
}

tbl_cfm_maid_master_t*
tbl_cfm_maid_init_cfm_maid()
{
    _g_p_tbl_cfm_maid_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_cfm_maid_master_t));
    _g_p_tbl_cfm_maid_master->cfm_maid_hash = ctclib_hash_create(_tbl_cfm_maid_hash_make, _tbl_cfm_maid_hash_cmp);
    _g_p_tbl_cfm_maid_master->cfm_maid_list = ctclib_slist_create(tbl_cfm_maid_cmp, NULL);
    return _g_p_tbl_cfm_maid_master;
}

