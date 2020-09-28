
#include "proto.h"
#include "gen/tbl_openflow_manager_define.h"
#include "gen/tbl_openflow_manager.h"
#include "cdb_data_cmp.h"

tbl_openflow_manager_master_t *_g_p_tbl_openflow_manager_master = NULL;

static uint32
_tbl_openflow_manager_hash_make(tbl_openflow_manager_t *p_openflow_manager)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_openflow_manager->key;
    for (index = 0; index < sizeof(p_openflow_manager->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_openflow_manager_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_openflow_manager_t *p_openflow_manager1 = (tbl_openflow_manager_t*)p_arg1;
    tbl_openflow_manager_t *p_openflow_manager2 = (tbl_openflow_manager_t*)p_arg2;

    if (0 == sal_memcmp(&p_openflow_manager1->key, &p_openflow_manager2->key, sizeof(tbl_openflow_manager_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_openflow_manager_add_openflow_manager_sync(tbl_openflow_manager_t *p_openflow_manager, uint32 sync)
{
    tbl_openflow_manager_master_t *p_master = _g_p_tbl_openflow_manager_master;
    tbl_openflow_manager_t *p_db_openflow_manager = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_openflow_manager_get_openflow_manager(&p_openflow_manager->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_openflow_manager = XCALLOC(MEM_TBL_OPENFLOW_MANAGER, sizeof(tbl_openflow_manager_t));
    if (NULL == p_db_openflow_manager)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_openflow_manager, p_openflow_manager, sizeof(tbl_openflow_manager_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->openflow_manager_hash, (void*)p_db_openflow_manager, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->openflow_manager_list, p_db_openflow_manager);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_OPENFLOW_MANAGER, p_db_openflow_manager);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_OPENFLOW_MANAGER, p_db_openflow_manager);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_openflow_manager_del_openflow_manager_sync(tbl_openflow_manager_key_t *p_openflow_manager_key, uint32 sync)
{
    tbl_openflow_manager_master_t *p_master = _g_p_tbl_openflow_manager_master;
    tbl_openflow_manager_t *p_db_openflow_manager = NULL;

    /* 1. lookup entry exist */
    p_db_openflow_manager = tbl_openflow_manager_get_openflow_manager(p_openflow_manager_key);
    if (NULL == p_db_openflow_manager)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_OPENFLOW_MANAGER, p_db_openflow_manager);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_OPENFLOW_MANAGER, p_db_openflow_manager);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->openflow_manager_hash, (void*)p_db_openflow_manager);
    ctclib_slistnode_delete(p_master->openflow_manager_list, p_db_openflow_manager);

    /* 4. free db entry */
    XFREE(MEM_TBL_OPENFLOW_MANAGER, p_db_openflow_manager);

    return PM_E_NONE;
}

int32
tbl_openflow_manager_set_openflow_manager_field_sync(tbl_openflow_manager_t *p_openflow_manager, tbl_openflow_manager_field_id_t field_id, uint32 sync)
{
    tbl_openflow_manager_t *p_db_openflow_manager = NULL;

    /* 1. lookup entry exist */
    p_db_openflow_manager = tbl_openflow_manager_get_openflow_manager(&p_openflow_manager->key);
    if (NULL == p_db_openflow_manager)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_OPENFLOW_MANAGER_FLD_MODE:
        p_db_openflow_manager->mode = p_openflow_manager->mode;
        break;

    case TBL_OPENFLOW_MANAGER_FLD_IS_INBAND:
        p_db_openflow_manager->is_inband = p_openflow_manager->is_inband;
        break;

    case TBL_OPENFLOW_MANAGER_FLD_MAX:
        sal_memcpy(p_db_openflow_manager, p_openflow_manager, sizeof(tbl_openflow_manager_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_OPENFLOW_MANAGER, field_id, p_db_openflow_manager);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_OPENFLOW_MANAGER, field_id, p_db_openflow_manager);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_openflow_manager_add_openflow_manager(tbl_openflow_manager_t *p_openflow_manager)
{
    return tbl_openflow_manager_add_openflow_manager_sync(p_openflow_manager, TRUE);
}

int32
tbl_openflow_manager_del_openflow_manager(tbl_openflow_manager_key_t *p_openflow_manager_key)
{
    return tbl_openflow_manager_del_openflow_manager_sync(p_openflow_manager_key, TRUE);
}

int32
tbl_openflow_manager_set_openflow_manager_field(tbl_openflow_manager_t *p_openflow_manager, tbl_openflow_manager_field_id_t field_id)
{
    return tbl_openflow_manager_set_openflow_manager_field_sync(p_openflow_manager, field_id, TRUE);
}

tbl_openflow_manager_t*
tbl_openflow_manager_get_openflow_manager(tbl_openflow_manager_key_t *p_openflow_manager_key)
{
    tbl_openflow_manager_master_t *p_master = _g_p_tbl_openflow_manager_master;
    tbl_openflow_manager_t lkp;

    sal_memcpy(&lkp.key, p_openflow_manager_key, sizeof(tbl_openflow_manager_key_t));
    return ctclib_hash_lookup(p_master->openflow_manager_hash, &lkp);
}

char*
tbl_openflow_manager_key_val2str(tbl_openflow_manager_t *p_openflow_manager, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_openflow_manager_key_val2str(buf, MAX_CMD_STR_LEN, &p_openflow_manager->key));
    return str;
}

int32
tbl_openflow_manager_key_str2val(char *str, tbl_openflow_manager_t *p_openflow_manager)
{
    int32 ret = 0;

    ret = cdb_openflow_manager_key_str2val(str, &p_openflow_manager->key);
    return ret;
}

char*
tbl_openflow_manager_key_name_dump(tbl_openflow_manager_t *p_openflow_manager, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_OPENFLOW_MANAGER);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_openflow_manager_key_val2str(buf, MAX_CMD_STR_LEN, &p_openflow_manager->key));
    return str;
}

char*
tbl_openflow_manager_key_value_dump(tbl_openflow_manager_t *p_openflow_manager, char *str)
{

    cdb_openflow_manager_key_val2str(str, MAX_CMD_STR_LEN, &p_openflow_manager->key);
    return str;
}

char*
tbl_openflow_manager_field_name_dump(tbl_openflow_manager_t *p_openflow_manager, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_OPENFLOW_MANAGER);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_OPENFLOW_MANAGER_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_openflow_manager_field_value_dump(tbl_openflow_manager_t *p_openflow_manager, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_OPENFLOW_MANAGER_FLD_KEY, field_id))
    {
        cdb_openflow_manager_key_val2str(str, MAX_CMD_STR_LEN, &p_openflow_manager->key);
    }
    if (FLD_MATCH(TBL_OPENFLOW_MANAGER_FLD_MODE, field_id))
    {
        sal_sprintf(str, "%u", p_openflow_manager->mode);
    }
    if (FLD_MATCH(TBL_OPENFLOW_MANAGER_FLD_IS_INBAND, field_id))
    {
        sal_sprintf(str, "%u", p_openflow_manager->is_inband);
    }
    return str;
}

char**
tbl_openflow_manager_table_dump(tbl_openflow_manager_t *p_openflow_manager, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_openflow_manager_key_name_dump(p_openflow_manager, buf));
    for(i=1; i<TBL_OPENFLOW_MANAGER_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_openflow_manager_field_name_dump(p_openflow_manager, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_openflow_manager_field_value_dump(p_openflow_manager, i, buf));
    }
    return str;
}

int32
tbl_openflow_manager_field_value_parser(char *str, int32 field_id, tbl_openflow_manager_t *p_openflow_manager)
{
    if (FLD_MATCH(TBL_OPENFLOW_MANAGER_FLD_KEY, field_id))
    {
        cdb_openflow_manager_key_str2val(str, &p_openflow_manager->key);
    }
    if (FLD_MATCH(TBL_OPENFLOW_MANAGER_FLD_MODE, field_id))
    {
        int32 ret;
        p_openflow_manager->mode = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_OPENFLOW_MANAGER_FLD_IS_INBAND, field_id))
    {
        int32 ret;
        p_openflow_manager->is_inband = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_openflow_manager_table_parser(char** array, char* key_value,tbl_openflow_manager_t *p_openflow_manager)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_OPENFLOW_MANAGER);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_openflow_manager_key_str2val(key_value, p_openflow_manager));

    for(i=1; i<TBL_OPENFLOW_MANAGER_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_openflow_manager_field_value_parser( array[j++], i, p_openflow_manager));
    }

    return PM_E_NONE;
}

int32
tbl_openflow_manager_dump_one(tbl_openflow_manager_t *p_openflow_manager, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_OPENFLOW_MANAGER);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_OPENFLOW_MANAGER_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OPENFLOW_MANAGER_FLD_KEY].name, 
            cdb_openflow_manager_key_val2str(buf, MAX_CMD_STR_LEN, &p_openflow_manager->key));
    }
    if (FLD_MATCH(TBL_OPENFLOW_MANAGER_FLD_MODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_MANAGER_FLD_MODE].name,
            p_openflow_manager->mode);
    }
    if (FLD_MATCH(TBL_OPENFLOW_MANAGER_FLD_IS_INBAND, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPENFLOW_MANAGER_FLD_IS_INBAND].name,
            p_openflow_manager->is_inband);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_openflow_manager_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_openflow_manager_master_t *p_master = _g_p_tbl_openflow_manager_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_openflow_manager_t *p_db_openflow_manager = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->openflow_manager_list, p_db_openflow_manager, listnode, next)
    {
        rc |= fn(p_db_openflow_manager, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_openflow_manager_master_t*
tbl_openflow_manager_get_master()
{
    return _g_p_tbl_openflow_manager_master;
}

tbl_openflow_manager_master_t*
tbl_openflow_manager_init_openflow_manager()
{
    _g_p_tbl_openflow_manager_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_openflow_manager_master_t));
    _g_p_tbl_openflow_manager_master->openflow_manager_hash = ctclib_hash_create(_tbl_openflow_manager_hash_make, _tbl_openflow_manager_hash_cmp);
    _g_p_tbl_openflow_manager_master->openflow_manager_list = ctclib_slist_create(tbl_openflow_manager_cmp, NULL);
    return _g_p_tbl_openflow_manager_master;
}

