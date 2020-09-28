
#include "proto.h"
#include "gen/tbl_macfilter_define.h"
#include "gen/tbl_macfilter.h"
#include "cdb_data_cmp.h"

tbl_macfilter_master_t *_g_p_tbl_macfilter_master = NULL;

int32
tbl_macfilter_add_macfilter_sync(tbl_macfilter_t *p_macflt, uint32 sync)
{
    tbl_macfilter_master_t *p_master = _g_p_tbl_macfilter_master;
    tbl_macfilter_t *p_db_macflt = NULL;

    /* 1. lookup entry exist */
    if (tbl_macfilter_get_macfilter(&p_macflt->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_macflt = XCALLOC(MEM_TBL_MACFILTER, sizeof(tbl_macfilter_t));
    if (NULL == p_db_macflt)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_macflt, p_macflt, sizeof(tbl_macfilter_t));

    /* 4. add to db */
    ctclib_slistnode_insert_sort(p_master->macflt_list, p_db_macflt);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_MACFILTER, p_db_macflt);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_MACFILTER, p_db_macflt);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_macfilter_del_macfilter_sync(tbl_macfilter_key_t *p_macflt_key, uint32 sync)
{
    tbl_macfilter_master_t *p_master = _g_p_tbl_macfilter_master;
    tbl_macfilter_t *p_db_macflt = NULL;

    /* 1. lookup entry exist */
    p_db_macflt = tbl_macfilter_get_macfilter(p_macflt_key);
    if (NULL == p_db_macflt)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_MACFILTER, p_db_macflt);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_MACFILTER, p_db_macflt);
        #endif 
    }

    /* 3. delete from db */
    ctclib_slistnode_delete(p_master->macflt_list, p_db_macflt);

    /* 4. free db entry */
    XFREE(MEM_TBL_MACFILTER, p_db_macflt);

    return PM_E_NONE;
}

int32
tbl_macfilter_set_macfilter_field_sync(tbl_macfilter_t *p_macflt, tbl_macfilter_field_id_t field_id, uint32 sync)
{
    tbl_macfilter_t *p_db_macflt = NULL;

    /* 1. lookup entry exist */
    p_db_macflt = tbl_macfilter_get_macfilter(&p_macflt->key);
    if (NULL == p_db_macflt)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_MACFILTER_FLD_MAX:
        sal_memcpy(p_db_macflt, p_macflt, sizeof(tbl_macfilter_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_MACFILTER, field_id, p_db_macflt);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_MACFILTER, field_id, p_db_macflt);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_macfilter_add_macfilter(tbl_macfilter_t *p_macflt)
{
    return tbl_macfilter_add_macfilter_sync(p_macflt, TRUE);
}

int32
tbl_macfilter_del_macfilter(tbl_macfilter_key_t *p_macflt_key)
{
    return tbl_macfilter_del_macfilter_sync(p_macflt_key, TRUE);
}

int32
tbl_macfilter_set_macfilter_field(tbl_macfilter_t *p_macflt, tbl_macfilter_field_id_t field_id)
{
    return tbl_macfilter_set_macfilter_field_sync(p_macflt, field_id, TRUE);
}

tbl_macfilter_t*
tbl_macfilter_get_macfilter(tbl_macfilter_key_t *p_macflt_key)
{
    tbl_macfilter_master_t *p_master = _g_p_tbl_macfilter_master;
    tbl_macfilter_t lkp;

    sal_memcpy(&lkp.key, p_macflt_key, sizeof(tbl_macfilter_key_t));
    return ctclib_slistdata_lookup(p_master->macflt_list, &lkp);
}

char*
tbl_macfilter_key_val2str(tbl_macfilter_t *p_macflt, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_macflt->key.mac));
    return str;
}

int32
tbl_macfilter_key_str2val(char *str, tbl_macfilter_t *p_macflt)
{
    int32 ret = 0;

    ret = cdb_mac_addr_str2val(p_macflt->key.mac, str);
    return ret;
}

char*
tbl_macfilter_key_name_dump(tbl_macfilter_t *p_macflt, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_MACFILTER);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_macflt->key.mac));
    return str;
}

char*
tbl_macfilter_key_value_dump(tbl_macfilter_t *p_macflt, char *str)
{

    cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_macflt->key.mac);
    return str;
}

char*
tbl_macfilter_field_name_dump(tbl_macfilter_t *p_macflt, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_MACFILTER);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_MACFILTER_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_macfilter_field_value_dump(tbl_macfilter_t *p_macflt, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_MACFILTER_FLD_KEY, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_macflt->key.mac);
    }
    return str;
}

char**
tbl_macfilter_table_dump(tbl_macfilter_t *p_macflt, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_macfilter_key_name_dump(p_macflt, buf));
    for(i=1; i<TBL_MACFILTER_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_macfilter_field_name_dump(p_macflt, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_macfilter_field_value_dump(p_macflt, i, buf));
    }
    return str;
}

int32
tbl_macfilter_field_value_parser(char *str, int32 field_id, tbl_macfilter_t *p_macflt)
{
    if (FLD_MATCH(TBL_MACFILTER_FLD_KEY, field_id))
    {
        cdb_mac_addr_str2val(p_macflt->key.mac, str);
    }
    return PM_E_NONE;
}

int32
tbl_macfilter_table_parser(char** array, char* key_value,tbl_macfilter_t *p_macflt)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_MACFILTER);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_macfilter_key_str2val(key_value, p_macflt));

    for(i=1; i<TBL_MACFILTER_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_macfilter_field_value_parser( array[j++], i, p_macflt));
    }

    return PM_E_NONE;
}

int32
tbl_macfilter_dump_one(tbl_macfilter_t *p_macflt, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_MACFILTER);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_MACFILTER_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MACFILTER_FLD_KEY].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_macflt->key.mac));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_macfilter_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_macfilter_master_t *p_master = _g_p_tbl_macfilter_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_macfilter_t *p_db_macflt = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->macflt_list, p_db_macflt, listnode, next)
    {
        rc |= fn(p_db_macflt, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_macfilter_master_t*
tbl_macfilter_get_master()
{
    return _g_p_tbl_macfilter_master;
}

tbl_macfilter_master_t*
tbl_macfilter_init_macfilter()
{
    _g_p_tbl_macfilter_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_macfilter_master_t));
    _g_p_tbl_macfilter_master->macflt_list = ctclib_slist_create(tbl_macfilter_cmp, NULL);
    return _g_p_tbl_macfilter_master;
}

