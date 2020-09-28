
#include "proto.h"
#include "gen/tbl_mirror_mac_escape_define.h"
#include "gen/tbl_mirror_mac_escape.h"
#include "cdb_data_cmp.h"

tbl_mirror_mac_escape_master_t *_g_p_tbl_mirror_mac_escape_master = NULL;

int32
tbl_mirror_mac_escape_add_mirror_mac_escape_sync(tbl_mirror_mac_escape_t *p_mirror_mac_escape, uint32 sync)
{
    tbl_mirror_mac_escape_master_t *p_master = _g_p_tbl_mirror_mac_escape_master;
    tbl_mirror_mac_escape_t *p_db_mirror_mac_escape = NULL;

    /* 1. lookup entry exist */
    if (tbl_mirror_mac_escape_get_mirror_mac_escape(&p_mirror_mac_escape->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_mirror_mac_escape = XCALLOC(MEM_TBL_MIRROR_MAC_ESCAPE, sizeof(tbl_mirror_mac_escape_t));
    if (NULL == p_db_mirror_mac_escape)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_mirror_mac_escape, p_mirror_mac_escape, sizeof(tbl_mirror_mac_escape_t));

    /* 4. add to db */
    ctclib_slistnode_insert_sort(p_master->mirror_mac_escape_list, p_db_mirror_mac_escape);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_MIRROR_MAC_ESCAPE, p_db_mirror_mac_escape);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_MIRROR_MAC_ESCAPE, p_db_mirror_mac_escape);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_mirror_mac_escape_del_mirror_mac_escape_sync(tbl_mirror_mac_escape_key_t *p_mirror_mac_escape_key, uint32 sync)
{
    tbl_mirror_mac_escape_master_t *p_master = _g_p_tbl_mirror_mac_escape_master;
    tbl_mirror_mac_escape_t *p_db_mirror_mac_escape = NULL;

    /* 1. lookup entry exist */
    p_db_mirror_mac_escape = tbl_mirror_mac_escape_get_mirror_mac_escape(p_mirror_mac_escape_key);
    if (NULL == p_db_mirror_mac_escape)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_MIRROR_MAC_ESCAPE, p_db_mirror_mac_escape);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_MIRROR_MAC_ESCAPE, p_db_mirror_mac_escape);
        #endif 
    }

    /* 3. delete from db */
    ctclib_slistnode_delete(p_master->mirror_mac_escape_list, p_db_mirror_mac_escape);

    /* 4. free db entry */
    XFREE(MEM_TBL_MIRROR_MAC_ESCAPE, p_db_mirror_mac_escape);

    return PM_E_NONE;
}

int32
tbl_mirror_mac_escape_set_mirror_mac_escape_field_sync(tbl_mirror_mac_escape_t *p_mirror_mac_escape, tbl_mirror_mac_escape_field_id_t field_id, uint32 sync)
{
    tbl_mirror_mac_escape_t *p_db_mirror_mac_escape = NULL;

    /* 1. lookup entry exist */
    p_db_mirror_mac_escape = tbl_mirror_mac_escape_get_mirror_mac_escape(&p_mirror_mac_escape->key);
    if (NULL == p_db_mirror_mac_escape)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_MIRROR_MAC_ESCAPE_FLD_MAX:
        sal_memcpy(p_db_mirror_mac_escape, p_mirror_mac_escape, sizeof(tbl_mirror_mac_escape_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_MIRROR_MAC_ESCAPE, field_id, p_db_mirror_mac_escape);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_MIRROR_MAC_ESCAPE, field_id, p_db_mirror_mac_escape);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_mirror_mac_escape_add_mirror_mac_escape(tbl_mirror_mac_escape_t *p_mirror_mac_escape)
{
    return tbl_mirror_mac_escape_add_mirror_mac_escape_sync(p_mirror_mac_escape, TRUE);
}

int32
tbl_mirror_mac_escape_del_mirror_mac_escape(tbl_mirror_mac_escape_key_t *p_mirror_mac_escape_key)
{
    return tbl_mirror_mac_escape_del_mirror_mac_escape_sync(p_mirror_mac_escape_key, TRUE);
}

int32
tbl_mirror_mac_escape_set_mirror_mac_escape_field(tbl_mirror_mac_escape_t *p_mirror_mac_escape, tbl_mirror_mac_escape_field_id_t field_id)
{
    return tbl_mirror_mac_escape_set_mirror_mac_escape_field_sync(p_mirror_mac_escape, field_id, TRUE);
}

tbl_mirror_mac_escape_t*
tbl_mirror_mac_escape_get_mirror_mac_escape(tbl_mirror_mac_escape_key_t *p_mirror_mac_escape_key)
{
    tbl_mirror_mac_escape_master_t *p_master = _g_p_tbl_mirror_mac_escape_master;
    tbl_mirror_mac_escape_t lkp;

    sal_memcpy(&lkp.key, p_mirror_mac_escape_key, sizeof(tbl_mirror_mac_escape_key_t));
    return ctclib_slistdata_lookup(p_master->mirror_mac_escape_list, &lkp);
}

char*
tbl_mirror_mac_escape_key_val2str(tbl_mirror_mac_escape_t *p_mirror_mac_escape, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_mirror_mac_escape_key_val2str(buf, MAX_CMD_STR_LEN, &p_mirror_mac_escape->key));
    return str;
}

int32
tbl_mirror_mac_escape_key_str2val(char *str, tbl_mirror_mac_escape_t *p_mirror_mac_escape)
{
    int32 ret = 0;

    ret = cdb_mirror_mac_escape_key_str2val(str, &p_mirror_mac_escape->key);
    return ret;
}

char*
tbl_mirror_mac_escape_key_name_dump(tbl_mirror_mac_escape_t *p_mirror_mac_escape, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_MIRROR_MAC_ESCAPE);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_mirror_mac_escape_key_val2str(buf, MAX_CMD_STR_LEN, &p_mirror_mac_escape->key));
    return str;
}

char*
tbl_mirror_mac_escape_key_value_dump(tbl_mirror_mac_escape_t *p_mirror_mac_escape, char *str)
{

    cdb_mirror_mac_escape_key_val2str(str, MAX_CMD_STR_LEN, &p_mirror_mac_escape->key);
    return str;
}

char*
tbl_mirror_mac_escape_field_name_dump(tbl_mirror_mac_escape_t *p_mirror_mac_escape, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_MIRROR_MAC_ESCAPE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_MIRROR_MAC_ESCAPE_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_mirror_mac_escape_field_value_dump(tbl_mirror_mac_escape_t *p_mirror_mac_escape, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_MIRROR_MAC_ESCAPE_FLD_KEY, field_id))
    {
        cdb_mirror_mac_escape_key_val2str(str, MAX_CMD_STR_LEN, &p_mirror_mac_escape->key);
    }
    return str;
}

char**
tbl_mirror_mac_escape_table_dump(tbl_mirror_mac_escape_t *p_mirror_mac_escape, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_mirror_mac_escape_key_name_dump(p_mirror_mac_escape, buf));
    for(i=1; i<TBL_MIRROR_MAC_ESCAPE_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_mirror_mac_escape_field_name_dump(p_mirror_mac_escape, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_mirror_mac_escape_field_value_dump(p_mirror_mac_escape, i, buf));
    }
    return str;
}

int32
tbl_mirror_mac_escape_field_value_parser(char *str, int32 field_id, tbl_mirror_mac_escape_t *p_mirror_mac_escape)
{
    if (FLD_MATCH(TBL_MIRROR_MAC_ESCAPE_FLD_KEY, field_id))
    {
        cdb_mirror_mac_escape_key_str2val(str, &p_mirror_mac_escape->key);
    }
    return PM_E_NONE;
}

int32
tbl_mirror_mac_escape_table_parser(char** array, char* key_value,tbl_mirror_mac_escape_t *p_mirror_mac_escape)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_MIRROR_MAC_ESCAPE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_mirror_mac_escape_key_str2val(key_value, p_mirror_mac_escape));

    for(i=1; i<TBL_MIRROR_MAC_ESCAPE_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_mirror_mac_escape_field_value_parser( array[j++], i, p_mirror_mac_escape));
    }

    return PM_E_NONE;
}

int32
tbl_mirror_mac_escape_dump_one(tbl_mirror_mac_escape_t *p_mirror_mac_escape, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_MIRROR_MAC_ESCAPE);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_MIRROR_MAC_ESCAPE_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MIRROR_MAC_ESCAPE_FLD_KEY].name, 
            cdb_mirror_mac_escape_key_val2str(buf, MAX_CMD_STR_LEN, &p_mirror_mac_escape->key));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_mirror_mac_escape_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_mirror_mac_escape_master_t *p_master = _g_p_tbl_mirror_mac_escape_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_mirror_mac_escape_t *p_db_mirror_mac_escape = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->mirror_mac_escape_list, p_db_mirror_mac_escape, listnode, next)
    {
        rc |= fn(p_db_mirror_mac_escape, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_mirror_mac_escape_master_t*
tbl_mirror_mac_escape_get_master()
{
    return _g_p_tbl_mirror_mac_escape_master;
}

tbl_mirror_mac_escape_master_t*
tbl_mirror_mac_escape_init_mirror_mac_escape()
{
    _g_p_tbl_mirror_mac_escape_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_mirror_mac_escape_master_t));
    _g_p_tbl_mirror_mac_escape_master->mirror_mac_escape_list = ctclib_slist_create(tbl_mirror_mac_escape_cmp, NULL);
    return _g_p_tbl_mirror_mac_escape_master;
}

