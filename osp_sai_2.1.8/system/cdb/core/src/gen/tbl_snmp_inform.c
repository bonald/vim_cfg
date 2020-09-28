
#include "proto.h"
#include "gen/tbl_snmp_inform_define.h"
#include "gen/tbl_snmp_inform.h"
#include "cdb_data_cmp.h"

tbl_snmp_inform_master_t *_g_p_tbl_snmp_inform_master = NULL;

int32
tbl_snmp_inform_add_snmp_inform_sync(tbl_snmp_inform_t *p_snmp_inform, uint32 sync)
{
    tbl_snmp_inform_master_t *p_master = _g_p_tbl_snmp_inform_master;
    tbl_snmp_inform_t *p_db_snmp_inform = NULL;

    /* 1. lookup entry exist */
    if (tbl_snmp_inform_get_snmp_inform(&p_snmp_inform->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_snmp_inform = XCALLOC(MEM_TBL_SNMP_INFORM, sizeof(tbl_snmp_inform_t));
    if (NULL == p_db_snmp_inform)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_snmp_inform, p_snmp_inform, sizeof(tbl_snmp_inform_t));

    /* 4. add to db */
    ctclib_slistnode_insert(p_master->snmp_inform_list, p_db_snmp_inform);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_SNMP_INFORM, p_db_snmp_inform);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_SNMP_INFORM, p_db_snmp_inform);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_snmp_inform_del_snmp_inform_sync(tbl_snmp_inform_key_t *p_snmp_inform_key, uint32 sync)
{
    tbl_snmp_inform_master_t *p_master = _g_p_tbl_snmp_inform_master;
    tbl_snmp_inform_t *p_db_snmp_inform = NULL;

    /* 1. lookup entry exist */
    p_db_snmp_inform = tbl_snmp_inform_get_snmp_inform(p_snmp_inform_key);
    if (NULL == p_db_snmp_inform)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_SNMP_INFORM, p_db_snmp_inform);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_SNMP_INFORM, p_db_snmp_inform);
        #endif 
    }

    /* 3. delete from db */
    ctclib_slistnode_delete(p_master->snmp_inform_list, p_db_snmp_inform);

    /* 4. free db entry */
    XFREE(MEM_TBL_SNMP_INFORM, p_db_snmp_inform);

    return PM_E_NONE;
}

int32
tbl_snmp_inform_set_snmp_inform_field_sync(tbl_snmp_inform_t *p_snmp_inform, tbl_snmp_inform_field_id_t field_id, uint32 sync)
{
    tbl_snmp_inform_t *p_db_snmp_inform = NULL;

    /* 1. lookup entry exist */
    p_db_snmp_inform = tbl_snmp_inform_get_snmp_inform(&p_snmp_inform->key);
    if (NULL == p_db_snmp_inform)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_SNMP_INFORM_FLD_IS_INBAND:
        p_db_snmp_inform->is_inband = p_snmp_inform->is_inband;
        break;

    case TBL_SNMP_INFORM_FLD_TIMEOUT:
        p_db_snmp_inform->timeout = p_snmp_inform->timeout;
        break;

    case TBL_SNMP_INFORM_FLD_RETRIES:
        p_db_snmp_inform->retries = p_snmp_inform->retries;
        break;

    case TBL_SNMP_INFORM_FLD_MAX:
        sal_memcpy(p_db_snmp_inform, p_snmp_inform, sizeof(tbl_snmp_inform_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_SNMP_INFORM, field_id, p_db_snmp_inform);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_SNMP_INFORM, field_id, p_db_snmp_inform);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_snmp_inform_add_snmp_inform(tbl_snmp_inform_t *p_snmp_inform)
{
    return tbl_snmp_inform_add_snmp_inform_sync(p_snmp_inform, TRUE);
}

int32
tbl_snmp_inform_del_snmp_inform(tbl_snmp_inform_key_t *p_snmp_inform_key)
{
    return tbl_snmp_inform_del_snmp_inform_sync(p_snmp_inform_key, TRUE);
}

int32
tbl_snmp_inform_set_snmp_inform_field(tbl_snmp_inform_t *p_snmp_inform, tbl_snmp_inform_field_id_t field_id)
{
    return tbl_snmp_inform_set_snmp_inform_field_sync(p_snmp_inform, field_id, TRUE);
}

tbl_snmp_inform_t*
tbl_snmp_inform_get_snmp_inform(tbl_snmp_inform_key_t *p_snmp_inform_key)
{
    tbl_snmp_inform_master_t *p_master = _g_p_tbl_snmp_inform_master;
    tbl_snmp_inform_t lkp;

    sal_memcpy(&lkp.key, p_snmp_inform_key, sizeof(tbl_snmp_inform_key_t));
    return ctclib_slistdata_lookup(p_master->snmp_inform_list, &lkp);
}

char*
tbl_snmp_inform_key_val2str(tbl_snmp_inform_t *p_snmp_inform, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_snmp_inform_val2str(buf, MAX_CMD_STR_LEN, &p_snmp_inform->key.inform));
    return str;
}

int32
tbl_snmp_inform_key_str2val(char *str, tbl_snmp_inform_t *p_snmp_inform)
{
    int32 ret = 0;

    ret = cdb_snmp_inform_str2val(str, &p_snmp_inform->key.inform);
    return ret;
}

char*
tbl_snmp_inform_key_name_dump(tbl_snmp_inform_t *p_snmp_inform, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_INFORM);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_snmp_inform_val2str(buf, MAX_CMD_STR_LEN, &p_snmp_inform->key.inform));
    return str;
}

char*
tbl_snmp_inform_key_value_dump(tbl_snmp_inform_t *p_snmp_inform, char *str)
{

    cdb_snmp_inform_val2str(str, MAX_CMD_STR_LEN, &p_snmp_inform->key.inform);
    return str;
}

char*
tbl_snmp_inform_field_name_dump(tbl_snmp_inform_t *p_snmp_inform, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_INFORM);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_SNMP_INFORM_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_snmp_inform_field_value_dump(tbl_snmp_inform_t *p_snmp_inform, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_SNMP_INFORM_FLD_KEY, field_id))
    {
        cdb_snmp_inform_val2str(str, MAX_CMD_STR_LEN, &p_snmp_inform->key.inform);
    }
    if (FLD_MATCH(TBL_SNMP_INFORM_FLD_IS_INBAND, field_id))
    {
        sal_sprintf(str, "%u", p_snmp_inform->is_inband);
    }
    if (FLD_MATCH(TBL_SNMP_INFORM_FLD_TIMEOUT, field_id))
    {
        sal_sprintf(str, "%u", p_snmp_inform->timeout);
    }
    if (FLD_MATCH(TBL_SNMP_INFORM_FLD_RETRIES, field_id))
    {
        sal_sprintf(str, "%u", p_snmp_inform->retries);
    }
    return str;
}

char**
tbl_snmp_inform_table_dump(tbl_snmp_inform_t *p_snmp_inform, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_snmp_inform_key_name_dump(p_snmp_inform, buf));
    for(i=1; i<TBL_SNMP_INFORM_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_snmp_inform_field_name_dump(p_snmp_inform, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_snmp_inform_field_value_dump(p_snmp_inform, i, buf));
    }
    return str;
}

int32
tbl_snmp_inform_field_value_parser(char *str, int32 field_id, tbl_snmp_inform_t *p_snmp_inform)
{
    if (FLD_MATCH(TBL_SNMP_INFORM_FLD_KEY, field_id))
    {
        cdb_snmp_inform_str2val(str, &p_snmp_inform->key.inform);
    }
    if (FLD_MATCH(TBL_SNMP_INFORM_FLD_IS_INBAND, field_id))
    {
        int32 ret;
        p_snmp_inform->is_inband = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SNMP_INFORM_FLD_TIMEOUT, field_id))
    {
        int32 ret;
        p_snmp_inform->timeout = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SNMP_INFORM_FLD_RETRIES, field_id))
    {
        int32 ret;
        p_snmp_inform->retries = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_snmp_inform_table_parser(char** array, char* key_value,tbl_snmp_inform_t *p_snmp_inform)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_INFORM);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_snmp_inform_key_str2val(key_value, p_snmp_inform));

    for(i=1; i<TBL_SNMP_INFORM_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_snmp_inform_field_value_parser( array[j++], i, p_snmp_inform));
    }

    return PM_E_NONE;
}

int32
tbl_snmp_inform_dump_one(tbl_snmp_inform_t *p_snmp_inform, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_INFORM);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_SNMP_INFORM_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SNMP_INFORM_FLD_KEY].name, 
            cdb_snmp_inform_val2str(buf, MAX_CMD_STR_LEN, &p_snmp_inform->key.inform));
    }
    if (FLD_MATCH(TBL_SNMP_INFORM_FLD_IS_INBAND, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SNMP_INFORM_FLD_IS_INBAND].name,
            p_snmp_inform->is_inband);
    }
    if (FLD_MATCH(TBL_SNMP_INFORM_FLD_TIMEOUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SNMP_INFORM_FLD_TIMEOUT].name,
            p_snmp_inform->timeout);
    }
    if (FLD_MATCH(TBL_SNMP_INFORM_FLD_RETRIES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SNMP_INFORM_FLD_RETRIES].name,
            p_snmp_inform->retries);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_snmp_inform_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_snmp_inform_master_t *p_master = _g_p_tbl_snmp_inform_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_snmp_inform_t *p_db_snmp_inform = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->snmp_inform_list, p_db_snmp_inform, listnode, next)
    {
        rc |= fn(p_db_snmp_inform, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_snmp_inform_master_t*
tbl_snmp_inform_get_master()
{
    return _g_p_tbl_snmp_inform_master;
}

tbl_snmp_inform_master_t*
tbl_snmp_inform_init_snmp_inform()
{
    _g_p_tbl_snmp_inform_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_snmp_inform_master_t));
    _g_p_tbl_snmp_inform_master->snmp_inform_list = ctclib_slist_create(tbl_snmp_inform_cmp, NULL);
    return _g_p_tbl_snmp_inform_master;
}

