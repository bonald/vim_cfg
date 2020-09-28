
#include "proto.h"
#include "gen/tbl_authen_define.h"
#include "gen/tbl_authen.h"
#include "cdb_data_cmp.h"

tbl_authen_master_t *_g_p_tbl_authen_master = NULL;

int32
tbl_authen_add_authen_sync(tbl_authen_t *p_authen, uint32 sync)
{
    tbl_authen_master_t *p_master = _g_p_tbl_authen_master;
    tbl_authen_t *p_db_authen = NULL;

    /* 1. lookup entry exist */
    if (tbl_authen_get_authen(&p_authen->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_authen = XCALLOC(MEM_TBL_AUTHEN, sizeof(tbl_authen_t));
    if (NULL == p_db_authen)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_authen, p_authen, sizeof(tbl_authen_t));

    /* 4. add to db */
    ctclib_slistnode_insert_sort(p_master->authen_list, p_db_authen);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_AUTHEN, p_db_authen);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_AUTHEN, p_db_authen);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_authen_del_authen_sync(tbl_authen_key_t *p_authen_key, uint32 sync)
{
    tbl_authen_master_t *p_master = _g_p_tbl_authen_master;
    tbl_authen_t *p_db_authen = NULL;

    /* 1. lookup entry exist */
    p_db_authen = tbl_authen_get_authen(p_authen_key);
    if (NULL == p_db_authen)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_AUTHEN, p_db_authen);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_AUTHEN, p_db_authen);
        #endif 
    }

    /* 3. delete from db */
    ctclib_slistnode_delete(p_master->authen_list, p_db_authen);

    /* 4. free db entry */
    XFREE(MEM_TBL_AUTHEN, p_db_authen);

    return PM_E_NONE;
}

int32
tbl_authen_set_authen_field_sync(tbl_authen_t *p_authen, tbl_authen_field_id_t field_id, uint32 sync)
{
    tbl_authen_t *p_db_authen = NULL;

    /* 1. lookup entry exist */
    p_db_authen = tbl_authen_get_authen(&p_authen->key);
    if (NULL == p_db_authen)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_AUTHEN_FLD_METHODS:
        sal_memcpy(p_db_authen->methods, p_authen->methods, sizeof(p_authen->methods));
        break;

    case TBL_AUTHEN_FLD_MAX:
        sal_memcpy(p_db_authen, p_authen, sizeof(tbl_authen_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_AUTHEN, field_id, p_db_authen);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_AUTHEN, field_id, p_db_authen);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_authen_add_authen(tbl_authen_t *p_authen)
{
    return tbl_authen_add_authen_sync(p_authen, TRUE);
}

int32
tbl_authen_del_authen(tbl_authen_key_t *p_authen_key)
{
    return tbl_authen_del_authen_sync(p_authen_key, TRUE);
}

int32
tbl_authen_set_authen_field(tbl_authen_t *p_authen, tbl_authen_field_id_t field_id)
{
    return tbl_authen_set_authen_field_sync(p_authen, field_id, TRUE);
}

tbl_authen_t*
tbl_authen_get_authen(tbl_authen_key_t *p_authen_key)
{
    tbl_authen_master_t *p_master = _g_p_tbl_authen_master;
    tbl_authen_t lkp;

    sal_memcpy(&lkp.key, p_authen_key, sizeof(tbl_authen_key_t));
    return ctclib_slistdata_lookup(p_master->authen_list, &lkp);
}

char*
tbl_authen_key_val2str(tbl_authen_t *p_authen, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_authen->key.name);
    return str;
}

int32
tbl_authen_key_str2val(char *str, tbl_authen_t *p_authen)
{
    int32 ret = 0;

    sal_strcpy(p_authen->key.name, str);
    return ret;
}

char*
tbl_authen_key_name_dump(tbl_authen_t *p_authen, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_AUTHEN);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_authen->key.name);
    return str;
}

char*
tbl_authen_key_value_dump(tbl_authen_t *p_authen, char *str)
{
    sal_sprintf(str, "%s", p_authen->key.name);
    return str;
}

char*
tbl_authen_field_name_dump(tbl_authen_t *p_authen, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_AUTHEN);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_AUTHEN_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_authen_field_value_dump(tbl_authen_t *p_authen, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_AUTHEN_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_authen->key.name);
    }
    if (FLD_MATCH(TBL_AUTHEN_FLD_METHODS, field_id))
    {
        sal_sprintf(str, "%s", p_authen->methods);
    }
    return str;
}

char**
tbl_authen_table_dump(tbl_authen_t *p_authen, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_authen_key_name_dump(p_authen, buf));
    for(i=1; i<TBL_AUTHEN_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_authen_field_name_dump(p_authen, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_authen_field_value_dump(p_authen, i, buf));
    }
    return str;
}

int32
tbl_authen_field_value_parser(char *str, int32 field_id, tbl_authen_t *p_authen)
{
    if (FLD_MATCH(TBL_AUTHEN_FLD_KEY, field_id))
    {
        sal_strcpy(p_authen->key.name, str);
    }
    if (FLD_MATCH(TBL_AUTHEN_FLD_METHODS, field_id))
    {
        sal_strcpy(p_authen->methods, str);
    }
    return PM_E_NONE;
}

int32
tbl_authen_table_parser(char** array, char* key_value,tbl_authen_t *p_authen)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_AUTHEN);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_authen_key_str2val(key_value, p_authen));

    for(i=1; i<TBL_AUTHEN_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_authen_field_value_parser( array[j++], i, p_authen));
    }

    return PM_E_NONE;
}

int32
tbl_authen_dump_one(tbl_authen_t *p_authen, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_AUTHEN);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_AUTHEN_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_AUTHEN_FLD_KEY].name,
            p_authen->key.name);
    }
    if (FLD_MATCH(TBL_AUTHEN_FLD_METHODS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_AUTHEN_FLD_METHODS].name,
            p_authen->methods);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_authen_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_authen_master_t *p_master = _g_p_tbl_authen_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_authen_t *p_db_authen = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->authen_list, p_db_authen, listnode, next)
    {
        rc |= fn(p_db_authen, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_authen_master_t*
tbl_authen_get_master()
{
    return _g_p_tbl_authen_master;
}

tbl_authen_master_t*
tbl_authen_init_authen()
{
    _g_p_tbl_authen_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_authen_master_t));
    _g_p_tbl_authen_master->authen_list = ctclib_slist_create(tbl_authen_cmp, NULL);
    return _g_p_tbl_authen_master;
}

