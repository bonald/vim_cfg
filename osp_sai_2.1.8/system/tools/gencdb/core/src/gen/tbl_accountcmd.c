
#include "proto.h"
#include "gen/tbl_accountcmd_define.h"
#include "gen/tbl_accountcmd.h"
#include "cdb_data_cmp.h"

tbl_accountcmd_master_t *_g_p_tbl_accountcmd_master = NULL;

int32
tbl_accountcmd_add_accountcmd_sync(tbl_accountcmd_t *p_accountcmd, uint32 sync)
{
    tbl_accountcmd_master_t *p_master = _g_p_tbl_accountcmd_master;
    tbl_accountcmd_t *p_db_accountcmd = NULL;

    /* 1. lookup entry exist */
    if (tbl_accountcmd_get_accountcmd(&p_accountcmd->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_accountcmd = XCALLOC(MEM_TBL_ACCOUNTCMD, sizeof(tbl_accountcmd_t));
    if (NULL == p_db_accountcmd)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_accountcmd, p_accountcmd, sizeof(tbl_accountcmd_t));

    /* 4. add to db */
    ctclib_slistnode_insert_sort(p_master->accountcmd_list, p_db_accountcmd);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_ACCOUNTCMD, p_db_accountcmd);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_ACCOUNTCMD, p_db_accountcmd);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_accountcmd_del_accountcmd_sync(tbl_accountcmd_key_t *p_accountcmd_key, uint32 sync)
{
    tbl_accountcmd_master_t *p_master = _g_p_tbl_accountcmd_master;
    tbl_accountcmd_t *p_db_accountcmd = NULL;

    /* 1. lookup entry exist */
    p_db_accountcmd = tbl_accountcmd_get_accountcmd(p_accountcmd_key);
    if (NULL == p_db_accountcmd)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_ACCOUNTCMD, p_db_accountcmd);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_ACCOUNTCMD, p_db_accountcmd);
        #endif 
    }

    /* 3. delete from db */
    ctclib_slistnode_delete(p_master->accountcmd_list, p_db_accountcmd);

    /* 4. free db entry */
    XFREE(MEM_TBL_ACCOUNTCMD, p_db_accountcmd);

    return PM_E_NONE;
}

int32
tbl_accountcmd_set_accountcmd_field_sync(tbl_accountcmd_t *p_accountcmd, tbl_accountcmd_field_id_t field_id, uint32 sync)
{
    tbl_accountcmd_t *p_db_accountcmd = NULL;

    /* 1. lookup entry exist */
    p_db_accountcmd = tbl_accountcmd_get_accountcmd(&p_accountcmd->key);
    if (NULL == p_db_accountcmd)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_ACCOUNTCMD_FLD_METHODS:
        sal_memcpy(p_db_accountcmd->methods, p_accountcmd->methods, sizeof(p_accountcmd->methods));
        break;

    case TBL_ACCOUNTCMD_FLD_MAX:
        sal_memcpy(p_db_accountcmd, p_accountcmd, sizeof(tbl_accountcmd_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_ACCOUNTCMD, field_id, p_db_accountcmd);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_ACCOUNTCMD, field_id, p_db_accountcmd);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_accountcmd_add_accountcmd(tbl_accountcmd_t *p_accountcmd)
{
    return tbl_accountcmd_add_accountcmd_sync(p_accountcmd, TRUE);
}

int32
tbl_accountcmd_del_accountcmd(tbl_accountcmd_key_t *p_accountcmd_key)
{
    return tbl_accountcmd_del_accountcmd_sync(p_accountcmd_key, TRUE);
}

int32
tbl_accountcmd_set_accountcmd_field(tbl_accountcmd_t *p_accountcmd, tbl_accountcmd_field_id_t field_id)
{
    return tbl_accountcmd_set_accountcmd_field_sync(p_accountcmd, field_id, TRUE);
}

tbl_accountcmd_t*
tbl_accountcmd_get_accountcmd(tbl_accountcmd_key_t *p_accountcmd_key)
{
    tbl_accountcmd_master_t *p_master = _g_p_tbl_accountcmd_master;
    tbl_accountcmd_t lkp;

    sal_memcpy(&lkp.key, p_accountcmd_key, sizeof(tbl_accountcmd_key_t));
    return ctclib_slistdata_lookup(p_master->accountcmd_list, &lkp);
}

char*
tbl_accountcmd_key_val2str(tbl_accountcmd_t *p_accountcmd, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_accountcmd->key.name);
    return str;
}

int32
tbl_accountcmd_key_str2val(char *str, tbl_accountcmd_t *p_accountcmd)
{
    int32 ret = 0;

    sal_strcpy(p_accountcmd->key.name, str);
    return ret;
}

char*
tbl_accountcmd_key_name_dump(tbl_accountcmd_t *p_accountcmd, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACCOUNTCMD);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_accountcmd->key.name);
    return str;
}

char*
tbl_accountcmd_key_value_dump(tbl_accountcmd_t *p_accountcmd, char *str)
{
    sal_sprintf(str, "%s", p_accountcmd->key.name);
    return str;
}

char*
tbl_accountcmd_field_name_dump(tbl_accountcmd_t *p_accountcmd, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_ACCOUNTCMD);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_ACCOUNTCMD_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_accountcmd_field_value_dump(tbl_accountcmd_t *p_accountcmd, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_ACCOUNTCMD_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_accountcmd->key.name);
    }
    if (FLD_MATCH(TBL_ACCOUNTCMD_FLD_METHODS, field_id))
    {
        sal_sprintf(str, "%s", p_accountcmd->methods);
    }
    return str;
}

char**
tbl_accountcmd_table_dump(tbl_accountcmd_t *p_accountcmd, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_accountcmd_key_name_dump(p_accountcmd, buf));
    for(i=1; i<TBL_ACCOUNTCMD_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_accountcmd_field_name_dump(p_accountcmd, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_accountcmd_field_value_dump(p_accountcmd, i, buf));
    }
    return str;
}

int32
tbl_accountcmd_field_value_parser(char *str, int32 field_id, tbl_accountcmd_t *p_accountcmd)
{
    if (FLD_MATCH(TBL_ACCOUNTCMD_FLD_KEY, field_id))
    {
        sal_strcpy(p_accountcmd->key.name, str);
    }
    if (FLD_MATCH(TBL_ACCOUNTCMD_FLD_METHODS, field_id))
    {
        sal_strcpy(p_accountcmd->methods, str);
    }
    return PM_E_NONE;
}

int32
tbl_accountcmd_table_parser(char** array, char* key_value,tbl_accountcmd_t *p_accountcmd)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACCOUNTCMD);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_accountcmd_key_str2val(key_value, p_accountcmd));

    for(i=1; i<TBL_ACCOUNTCMD_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_accountcmd_field_value_parser( array[j++], i, p_accountcmd));
    }

    return PM_E_NONE;
}

int32
tbl_accountcmd_dump_one(tbl_accountcmd_t *p_accountcmd, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACCOUNTCMD);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_ACCOUNTCMD_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACCOUNTCMD_FLD_KEY].name,
            p_accountcmd->key.name);
    }
    if (FLD_MATCH(TBL_ACCOUNTCMD_FLD_METHODS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACCOUNTCMD_FLD_METHODS].name,
            p_accountcmd->methods);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_accountcmd_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_accountcmd_master_t *p_master = _g_p_tbl_accountcmd_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_accountcmd_t *p_db_accountcmd = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->accountcmd_list, p_db_accountcmd, listnode, next)
    {
        rc |= fn(p_db_accountcmd, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_accountcmd_master_t*
tbl_accountcmd_get_master()
{
    return _g_p_tbl_accountcmd_master;
}

tbl_accountcmd_master_t*
tbl_accountcmd_init_accountcmd()
{
    _g_p_tbl_accountcmd_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_accountcmd_master_t));
    _g_p_tbl_accountcmd_master->accountcmd_list = ctclib_slist_create(tbl_accountcmd_cmp, NULL);
    return _g_p_tbl_accountcmd_master;
}

