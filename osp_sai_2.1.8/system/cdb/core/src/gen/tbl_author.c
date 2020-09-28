
#include "proto.h"
#include "gen/tbl_author_define.h"
#include "gen/tbl_author.h"
#include "cdb_data_cmp.h"

tbl_author_master_t *_g_p_tbl_author_master = NULL;

int32
tbl_author_add_author_sync(tbl_author_t *p_author, uint32 sync)
{
    tbl_author_master_t *p_master = _g_p_tbl_author_master;
    tbl_author_t *p_db_author = NULL;

    /* 1. lookup entry exist */
    if (tbl_author_get_author(&p_author->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_author = XCALLOC(MEM_TBL_AUTHOR, sizeof(tbl_author_t));
    if (NULL == p_db_author)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_author, p_author, sizeof(tbl_author_t));

    /* 4. add to db */
    ctclib_slistnode_insert_sort(p_master->author_list, p_db_author);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_AUTHOR, p_db_author);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_AUTHOR, p_db_author);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_author_del_author_sync(tbl_author_key_t *p_author_key, uint32 sync)
{
    tbl_author_master_t *p_master = _g_p_tbl_author_master;
    tbl_author_t *p_db_author = NULL;

    /* 1. lookup entry exist */
    p_db_author = tbl_author_get_author(p_author_key);
    if (NULL == p_db_author)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_AUTHOR, p_db_author);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_AUTHOR, p_db_author);
        #endif 
    }

    /* 3. delete from db */
    ctclib_slistnode_delete(p_master->author_list, p_db_author);

    /* 4. free db entry */
    XFREE(MEM_TBL_AUTHOR, p_db_author);

    return PM_E_NONE;
}

int32
tbl_author_set_author_field_sync(tbl_author_t *p_author, tbl_author_field_id_t field_id, uint32 sync)
{
    tbl_author_t *p_db_author = NULL;

    /* 1. lookup entry exist */
    p_db_author = tbl_author_get_author(&p_author->key);
    if (NULL == p_db_author)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_AUTHOR_FLD_METHODS:
        sal_memcpy(p_db_author->methods, p_author->methods, sizeof(p_author->methods));
        break;

    case TBL_AUTHOR_FLD_MAX:
        sal_memcpy(p_db_author, p_author, sizeof(tbl_author_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_AUTHOR, field_id, p_db_author);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_AUTHOR, field_id, p_db_author);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_author_add_author(tbl_author_t *p_author)
{
    return tbl_author_add_author_sync(p_author, TRUE);
}

int32
tbl_author_del_author(tbl_author_key_t *p_author_key)
{
    return tbl_author_del_author_sync(p_author_key, TRUE);
}

int32
tbl_author_set_author_field(tbl_author_t *p_author, tbl_author_field_id_t field_id)
{
    return tbl_author_set_author_field_sync(p_author, field_id, TRUE);
}

tbl_author_t*
tbl_author_get_author(tbl_author_key_t *p_author_key)
{
    tbl_author_master_t *p_master = _g_p_tbl_author_master;
    tbl_author_t lkp;

    sal_memcpy(&lkp.key, p_author_key, sizeof(tbl_author_key_t));
    return ctclib_slistdata_lookup(p_master->author_list, &lkp);
}

char*
tbl_author_key_val2str(tbl_author_t *p_author, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_author->key.name);
    return str;
}

int32
tbl_author_key_str2val(char *str, tbl_author_t *p_author)
{
    int32 ret = 0;

    sal_strcpy(p_author->key.name, str);
    return ret;
}

char*
tbl_author_key_name_dump(tbl_author_t *p_author, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_AUTHOR);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_author->key.name);
    return str;
}

char*
tbl_author_key_value_dump(tbl_author_t *p_author, char *str)
{
    sal_sprintf(str, "%s", p_author->key.name);
    return str;
}

char*
tbl_author_field_name_dump(tbl_author_t *p_author, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_AUTHOR);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_AUTHOR_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_author_field_value_dump(tbl_author_t *p_author, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_AUTHOR_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_author->key.name);
    }
    if (FLD_MATCH(TBL_AUTHOR_FLD_METHODS, field_id))
    {
        sal_sprintf(str, "%s", p_author->methods);
    }
    return str;
}

char**
tbl_author_table_dump(tbl_author_t *p_author, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_author_key_name_dump(p_author, buf));
    for(i=1; i<TBL_AUTHOR_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_author_field_name_dump(p_author, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_author_field_value_dump(p_author, i, buf));
    }
    return str;
}

int32
tbl_author_field_value_parser(char *str, int32 field_id, tbl_author_t *p_author)
{
    if (FLD_MATCH(TBL_AUTHOR_FLD_KEY, field_id))
    {
        sal_strcpy(p_author->key.name, str);
    }
    if (FLD_MATCH(TBL_AUTHOR_FLD_METHODS, field_id))
    {
        sal_strcpy(p_author->methods, str);
    }
    return PM_E_NONE;
}

int32
tbl_author_table_parser(char** array, char* key_value,tbl_author_t *p_author)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_AUTHOR);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_author_key_str2val(key_value, p_author));

    for(i=1; i<TBL_AUTHOR_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_author_field_value_parser( array[j++], i, p_author));
    }

    return PM_E_NONE;
}

int32
tbl_author_dump_one(tbl_author_t *p_author, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_AUTHOR);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_AUTHOR_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_AUTHOR_FLD_KEY].name,
            p_author->key.name);
    }
    if (FLD_MATCH(TBL_AUTHOR_FLD_METHODS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_AUTHOR_FLD_METHODS].name,
            p_author->methods);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_author_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_author_master_t *p_master = _g_p_tbl_author_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_author_t *p_db_author = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->author_list, p_db_author, listnode, next)
    {
        rc |= fn(p_db_author, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_author_master_t*
tbl_author_get_master()
{
    return _g_p_tbl_author_master;
}

tbl_author_master_t*
tbl_author_init_author()
{
    _g_p_tbl_author_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_author_master_t));
    _g_p_tbl_author_master->author_list = ctclib_slist_create(tbl_author_cmp, NULL);
    return _g_p_tbl_author_master;
}

