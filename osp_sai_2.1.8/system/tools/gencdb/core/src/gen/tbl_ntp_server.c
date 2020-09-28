
#include "proto.h"
#include "gen/tbl_ntp_server_define.h"
#include "gen/tbl_ntp_server.h"
#include "cdb_data_cmp.h"

tbl_ntp_server_master_t *_g_p_tbl_ntp_server_master = NULL;

int32
tbl_ntp_server_add_ntp_server_sync(tbl_ntp_server_t *p_ntp_server, uint32 sync)
{
    tbl_ntp_server_master_t *p_master = _g_p_tbl_ntp_server_master;
    tbl_ntp_server_t *p_db_ntp_server = NULL;

    /* 1. lookup entry exist */
    if (tbl_ntp_server_get_ntp_server(&p_ntp_server->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_ntp_server = XCALLOC(MEM_TBL_NTP_SERVER, sizeof(tbl_ntp_server_t));
    if (NULL == p_db_ntp_server)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_ntp_server, p_ntp_server, sizeof(tbl_ntp_server_t));

    /* 4. add to db */
    ctclib_slistnode_insert_sort(p_master->ntp_server_list, p_db_ntp_server);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_NTP_SERVER, p_db_ntp_server);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_NTP_SERVER, p_db_ntp_server);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ntp_server_del_ntp_server_sync(tbl_ntp_server_key_t *p_ntp_server_key, uint32 sync)
{
    tbl_ntp_server_master_t *p_master = _g_p_tbl_ntp_server_master;
    tbl_ntp_server_t *p_db_ntp_server = NULL;

    /* 1. lookup entry exist */
    p_db_ntp_server = tbl_ntp_server_get_ntp_server(p_ntp_server_key);
    if (NULL == p_db_ntp_server)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_NTP_SERVER, p_db_ntp_server);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_NTP_SERVER, p_db_ntp_server);
        #endif 
    }

    /* 3. delete from db */
    ctclib_slistnode_delete(p_master->ntp_server_list, p_db_ntp_server);

    /* 4. free db entry */
    XFREE(MEM_TBL_NTP_SERVER, p_db_ntp_server);

    return PM_E_NONE;
}

int32
tbl_ntp_server_set_ntp_server_field_sync(tbl_ntp_server_t *p_ntp_server, tbl_ntp_server_field_id_t field_id, uint32 sync)
{
    tbl_ntp_server_t *p_db_ntp_server = NULL;

    /* 1. lookup entry exist */
    p_db_ntp_server = tbl_ntp_server_get_ntp_server(&p_ntp_server->key);
    if (NULL == p_db_ntp_server)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_NTP_SERVER_FLD_KEYID:
        p_db_ntp_server->keyid = p_ntp_server->keyid;
        break;

    case TBL_NTP_SERVER_FLD_VERSION:
        p_db_ntp_server->version = p_ntp_server->version;
        break;

    case TBL_NTP_SERVER_FLD_PREFER:
        p_db_ntp_server->prefer = p_ntp_server->prefer;
        break;

    case TBL_NTP_SERVER_FLD_TYPE:
        p_db_ntp_server->type = p_ntp_server->type;
        break;

    case TBL_NTP_SERVER_FLD_IS_INBAND:
        p_db_ntp_server->is_inband = p_ntp_server->is_inband;
        break;

    case TBL_NTP_SERVER_FLD_MAX:
        sal_memcpy(p_db_ntp_server, p_ntp_server, sizeof(tbl_ntp_server_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_NTP_SERVER, field_id, p_db_ntp_server);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_NTP_SERVER, field_id, p_db_ntp_server);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ntp_server_add_ntp_server(tbl_ntp_server_t *p_ntp_server)
{
    return tbl_ntp_server_add_ntp_server_sync(p_ntp_server, TRUE);
}

int32
tbl_ntp_server_del_ntp_server(tbl_ntp_server_key_t *p_ntp_server_key)
{
    return tbl_ntp_server_del_ntp_server_sync(p_ntp_server_key, TRUE);
}

int32
tbl_ntp_server_set_ntp_server_field(tbl_ntp_server_t *p_ntp_server, tbl_ntp_server_field_id_t field_id)
{
    return tbl_ntp_server_set_ntp_server_field_sync(p_ntp_server, field_id, TRUE);
}

tbl_ntp_server_t*
tbl_ntp_server_get_ntp_server(tbl_ntp_server_key_t *p_ntp_server_key)
{
    tbl_ntp_server_master_t *p_master = _g_p_tbl_ntp_server_master;
    tbl_ntp_server_t lkp;

    sal_memcpy(&lkp.key, p_ntp_server_key, sizeof(tbl_ntp_server_key_t));
    return ctclib_slistdata_lookup(p_master->ntp_server_list, &lkp);
}

char*
tbl_ntp_server_key_val2str(tbl_ntp_server_t *p_ntp_server, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_ntp_server->key.host);
    return str;
}

int32
tbl_ntp_server_key_str2val(char *str, tbl_ntp_server_t *p_ntp_server)
{
    int32 ret = 0;

    sal_strcpy(p_ntp_server->key.host, str);
    return ret;
}

char*
tbl_ntp_server_key_name_dump(tbl_ntp_server_t *p_ntp_server, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_NTP_SERVER);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_ntp_server->key.host);
    return str;
}

char*
tbl_ntp_server_key_value_dump(tbl_ntp_server_t *p_ntp_server, char *str)
{
    sal_sprintf(str, "%s", p_ntp_server->key.host);
    return str;
}

char*
tbl_ntp_server_field_name_dump(tbl_ntp_server_t *p_ntp_server, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_NTP_SERVER);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_NTP_SERVER_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_ntp_server_field_value_dump(tbl_ntp_server_t *p_ntp_server, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_NTP_SERVER_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_ntp_server->key.host);
    }
    if (FLD_MATCH(TBL_NTP_SERVER_FLD_KEYID, field_id))
    {
        sal_sprintf(str, "%u", p_ntp_server->keyid);
    }
    if (FLD_MATCH(TBL_NTP_SERVER_FLD_VERSION, field_id))
    {
        sal_sprintf(str, "%u", p_ntp_server->version);
    }
    if (FLD_MATCH(TBL_NTP_SERVER_FLD_PREFER, field_id))
    {
        sal_sprintf(str, "%u", p_ntp_server->prefer);
    }
    if (FLD_MATCH(TBL_NTP_SERVER_FLD_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_ntp_server->type);
    }
    if (FLD_MATCH(TBL_NTP_SERVER_FLD_IS_INBAND, field_id))
    {
        sal_sprintf(str, "%u", p_ntp_server->is_inband);
    }
    return str;
}

char**
tbl_ntp_server_table_dump(tbl_ntp_server_t *p_ntp_server, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_ntp_server_key_name_dump(p_ntp_server, buf));
    for(i=1; i<TBL_NTP_SERVER_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_ntp_server_field_name_dump(p_ntp_server, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_ntp_server_field_value_dump(p_ntp_server, i, buf));
    }
    return str;
}

int32
tbl_ntp_server_field_value_parser(char *str, int32 field_id, tbl_ntp_server_t *p_ntp_server)
{
    if (FLD_MATCH(TBL_NTP_SERVER_FLD_KEY, field_id))
    {
        sal_strcpy(p_ntp_server->key.host, str);
    }
    if (FLD_MATCH(TBL_NTP_SERVER_FLD_KEYID, field_id))
    {
        int32 ret;
        p_ntp_server->keyid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NTP_SERVER_FLD_VERSION, field_id))
    {
        int32 ret;
        p_ntp_server->version = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NTP_SERVER_FLD_PREFER, field_id))
    {
        int32 ret;
        p_ntp_server->prefer = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NTP_SERVER_FLD_TYPE, field_id))
    {
        int32 ret;
        p_ntp_server->type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NTP_SERVER_FLD_IS_INBAND, field_id))
    {
        int32 ret;
        p_ntp_server->is_inband = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_ntp_server_table_parser(char** array, char* key_value,tbl_ntp_server_t *p_ntp_server)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_NTP_SERVER);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_ntp_server_key_str2val(key_value, p_ntp_server));

    for(i=1; i<TBL_NTP_SERVER_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_ntp_server_field_value_parser( array[j++], i, p_ntp_server));
    }

    return PM_E_NONE;
}

int32
tbl_ntp_server_dump_one(tbl_ntp_server_t *p_ntp_server, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_NTP_SERVER);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_NTP_SERVER_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_NTP_SERVER_FLD_KEY].name,
            p_ntp_server->key.host);
    }
    if (FLD_MATCH(TBL_NTP_SERVER_FLD_KEYID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NTP_SERVER_FLD_KEYID].name,
            p_ntp_server->keyid);
    }
    if (FLD_MATCH(TBL_NTP_SERVER_FLD_VERSION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NTP_SERVER_FLD_VERSION].name,
            p_ntp_server->version);
    }
    if (FLD_MATCH(TBL_NTP_SERVER_FLD_PREFER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NTP_SERVER_FLD_PREFER].name,
            p_ntp_server->prefer);
    }
    if (FLD_MATCH(TBL_NTP_SERVER_FLD_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NTP_SERVER_FLD_TYPE].name,
            p_ntp_server->type);
    }
    if (FLD_MATCH(TBL_NTP_SERVER_FLD_IS_INBAND, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NTP_SERVER_FLD_IS_INBAND].name,
            p_ntp_server->is_inband);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_ntp_server_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_ntp_server_master_t *p_master = _g_p_tbl_ntp_server_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_ntp_server_t *p_db_ntp_server = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->ntp_server_list, p_db_ntp_server, listnode, next)
    {
        rc |= fn(p_db_ntp_server, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_ntp_server_master_t*
tbl_ntp_server_get_master()
{
    return _g_p_tbl_ntp_server_master;
}

tbl_ntp_server_master_t*
tbl_ntp_server_init_ntp_server()
{
    _g_p_tbl_ntp_server_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_ntp_server_master_t));
    _g_p_tbl_ntp_server_master->ntp_server_list = ctclib_slist_create(tbl_ntp_server_cmp, NULL);
    return _g_p_tbl_ntp_server_master;
}

