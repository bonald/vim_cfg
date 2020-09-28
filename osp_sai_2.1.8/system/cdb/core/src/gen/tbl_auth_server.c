
#include "proto.h"
#include "gen/tbl_auth_server_define.h"
#include "gen/tbl_auth_server.h"
#include "cdb_data_cmp.h"

tbl_auth_server_master_t *_g_p_tbl_auth_server_master = NULL;

int32
tbl_auth_server_add_auth_server_sync(tbl_auth_server_t *p_auth_server, uint32 sync)
{
    tbl_auth_server_master_t *p_master = _g_p_tbl_auth_server_master;
    tbl_auth_server_t *p_db_auth_server = NULL;

    /* 1. lookup entry exist */
    if (tbl_auth_server_get_auth_server(&p_auth_server->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_auth_server = XCALLOC(MEM_TBL_AUTH_SERVER, sizeof(tbl_auth_server_t));
    if (NULL == p_db_auth_server)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_auth_server, p_auth_server, sizeof(tbl_auth_server_t));

    /* 4. add to db */
    ctclib_slistnode_insert_sort(p_master->auth_servers, p_db_auth_server);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_AUTH_SERVER, p_db_auth_server);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_AUTH_SERVER, p_db_auth_server);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_auth_server_del_auth_server_sync(tbl_auth_server_key_t *p_auth_server_key, uint32 sync)
{
    tbl_auth_server_master_t *p_master = _g_p_tbl_auth_server_master;
    tbl_auth_server_t *p_db_auth_server = NULL;

    /* 1. lookup entry exist */
    p_db_auth_server = tbl_auth_server_get_auth_server(p_auth_server_key);
    if (NULL == p_db_auth_server)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_AUTH_SERVER, p_db_auth_server);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_AUTH_SERVER, p_db_auth_server);
        #endif 
    }

    /* 3. delete from db */
    ctclib_slistnode_delete(p_master->auth_servers, p_db_auth_server);

    /* 4. free db entry */
    XFREE(MEM_TBL_AUTH_SERVER, p_db_auth_server);

    return PM_E_NONE;
}

int32
tbl_auth_server_set_auth_server_field_sync(tbl_auth_server_t *p_auth_server, tbl_auth_server_field_id_t field_id, uint32 sync)
{
    tbl_auth_server_t *p_db_auth_server = NULL;

    /* 1. lookup entry exist */
    p_db_auth_server = tbl_auth_server_get_auth_server(&p_auth_server->key);
    if (NULL == p_db_auth_server)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_AUTH_SERVER_FLD_SECRET:
        sal_memcpy(p_db_auth_server->secret, p_auth_server->secret, sizeof(p_auth_server->secret));
        break;

    case TBL_AUTH_SERVER_FLD_ENC_SECRET:
        sal_memcpy(p_db_auth_server->enc_secret, p_auth_server->enc_secret, sizeof(p_auth_server->enc_secret));
        break;

    case TBL_AUTH_SERVER_FLD_PORT:
        p_db_auth_server->port = p_auth_server->port;
        break;

    case TBL_AUTH_SERVER_FLD_TIMEOUT:
        p_db_auth_server->timeout = p_auth_server->timeout;
        break;

    case TBL_AUTH_SERVER_FLD_RETRIES:
        p_db_auth_server->retries = p_auth_server->retries;
        break;

    case TBL_AUTH_SERVER_FLD_DEAD:
        p_db_auth_server->dead = p_auth_server->dead;
        break;

    case TBL_AUTH_SERVER_FLD_IS_INBAND:
        p_db_auth_server->is_inband = p_auth_server->is_inband;
        break;

    case TBL_AUTH_SERVER_FLD_REAWAKE_TIME:
        p_db_auth_server->reawake_time = p_auth_server->reawake_time;
        break;

    case TBL_AUTH_SERVER_FLD_MAX:
        sal_memcpy(p_db_auth_server, p_auth_server, sizeof(tbl_auth_server_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_AUTH_SERVER, field_id, p_db_auth_server);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_AUTH_SERVER, field_id, p_db_auth_server);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_auth_server_add_auth_server(tbl_auth_server_t *p_auth_server)
{
    return tbl_auth_server_add_auth_server_sync(p_auth_server, TRUE);
}

int32
tbl_auth_server_del_auth_server(tbl_auth_server_key_t *p_auth_server_key)
{
    return tbl_auth_server_del_auth_server_sync(p_auth_server_key, TRUE);
}

int32
tbl_auth_server_set_auth_server_field(tbl_auth_server_t *p_auth_server, tbl_auth_server_field_id_t field_id)
{
    return tbl_auth_server_set_auth_server_field_sync(p_auth_server, field_id, TRUE);
}

tbl_auth_server_t*
tbl_auth_server_get_auth_server(tbl_auth_server_key_t *p_auth_server_key)
{
    tbl_auth_server_master_t *p_master = _g_p_tbl_auth_server_master;
    tbl_auth_server_t lkp;

    sal_memcpy(&lkp.key, p_auth_server_key, sizeof(tbl_auth_server_key_t));
    return ctclib_slistdata_lookup(p_master->auth_servers, &lkp);
}

char*
tbl_auth_server_key_val2str(tbl_auth_server_t *p_auth_server, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_auth_server_key_val2str(buf, MAX_CMD_STR_LEN, &p_auth_server->key));
    return str;
}

int32
tbl_auth_server_key_str2val(char *str, tbl_auth_server_t *p_auth_server)
{
    int32 ret = 0;

    char err[MAX_CMD_STR_LEN];
    ret = cdb_auth_server_key_str2val(&p_auth_server->key, str, err);
    return ret;
}

char*
tbl_auth_server_key_name_dump(tbl_auth_server_t *p_auth_server, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_AUTH_SERVER);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_auth_server_key_val2str(buf, MAX_CMD_STR_LEN, &p_auth_server->key));
    return str;
}

char*
tbl_auth_server_key_value_dump(tbl_auth_server_t *p_auth_server, char *str)
{

    cdb_auth_server_key_val2str(str, MAX_CMD_STR_LEN, &p_auth_server->key);
    return str;
}

char*
tbl_auth_server_field_name_dump(tbl_auth_server_t *p_auth_server, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_AUTH_SERVER);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_AUTH_SERVER_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_auth_server_field_value_dump(tbl_auth_server_t *p_auth_server, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_AUTH_SERVER_FLD_KEY, field_id))
    {
        cdb_auth_server_key_val2str(str, MAX_CMD_STR_LEN, &p_auth_server->key);
    }
    if (FLD_MATCH(TBL_AUTH_SERVER_FLD_SECRET, field_id))
    {
        sal_sprintf(str, "%s", p_auth_server->secret);
    }
    if (FLD_MATCH(TBL_AUTH_SERVER_FLD_ENC_SECRET, field_id))
    {
        sal_sprintf(str, "%s", p_auth_server->enc_secret);
    }
    if (FLD_MATCH(TBL_AUTH_SERVER_FLD_PORT, field_id))
    {
        sal_sprintf(str, "%u", p_auth_server->port);
    }
    if (FLD_MATCH(TBL_AUTH_SERVER_FLD_TIMEOUT, field_id))
    {
        sal_sprintf(str, "%u", p_auth_server->timeout);
    }
    if (FLD_MATCH(TBL_AUTH_SERVER_FLD_RETRIES, field_id))
    {
        sal_sprintf(str, "%u", p_auth_server->retries);
    }
    if (FLD_MATCH(TBL_AUTH_SERVER_FLD_DEAD, field_id))
    {
        sal_sprintf(str, "%u", p_auth_server->dead);
    }
    if (FLD_MATCH(TBL_AUTH_SERVER_FLD_IS_INBAND, field_id))
    {
        sal_sprintf(str, "%u", p_auth_server->is_inband);
    }
    if (FLD_MATCH(TBL_AUTH_SERVER_FLD_REAWAKE_TIME, field_id))
    {
        sal_sprintf(str, "%u", p_auth_server->reawake_time);
    }
    return str;
}

char**
tbl_auth_server_table_dump(tbl_auth_server_t *p_auth_server, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_auth_server_key_name_dump(p_auth_server, buf));
    for(i=1; i<TBL_AUTH_SERVER_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_auth_server_field_name_dump(p_auth_server, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_auth_server_field_value_dump(p_auth_server, i, buf));
    }
    return str;
}

int32
tbl_auth_server_field_value_parser(char *str, int32 field_id, tbl_auth_server_t *p_auth_server)
{
    if (FLD_MATCH(TBL_AUTH_SERVER_FLD_KEY, field_id))
    {
        char err[MAX_CMD_STR_LEN];
        cdb_auth_server_key_str2val(&p_auth_server->key, str, err);
    }
    if (FLD_MATCH(TBL_AUTH_SERVER_FLD_SECRET, field_id))
    {
        sal_strcpy(p_auth_server->secret, str);
    }
    if (FLD_MATCH(TBL_AUTH_SERVER_FLD_ENC_SECRET, field_id))
    {
        sal_strcpy(p_auth_server->enc_secret, str);
    }
    if (FLD_MATCH(TBL_AUTH_SERVER_FLD_PORT, field_id))
    {
        int32 ret;
        p_auth_server->port = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_AUTH_SERVER_FLD_TIMEOUT, field_id))
    {
        int32 ret;
        p_auth_server->timeout = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_AUTH_SERVER_FLD_RETRIES, field_id))
    {
        int32 ret;
        p_auth_server->retries = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_AUTH_SERVER_FLD_DEAD, field_id))
    {
        int32 ret;
        p_auth_server->dead = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_AUTH_SERVER_FLD_IS_INBAND, field_id))
    {
        int32 ret;
        p_auth_server->is_inband = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_AUTH_SERVER_FLD_REAWAKE_TIME, field_id))
    {
        int32 ret;
        p_auth_server->reawake_time = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_auth_server_table_parser(char** array, char* key_value,tbl_auth_server_t *p_auth_server)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_AUTH_SERVER);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_auth_server_key_str2val(key_value, p_auth_server));

    for(i=1; i<TBL_AUTH_SERVER_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_auth_server_field_value_parser( array[j++], i, p_auth_server));
    }

    return PM_E_NONE;
}

int32
tbl_auth_server_dump_one(tbl_auth_server_t *p_auth_server, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_AUTH_SERVER);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_AUTH_SERVER_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_AUTH_SERVER_FLD_KEY].name, 
            cdb_auth_server_key_val2str(buf, MAX_CMD_STR_LEN, &p_auth_server->key));
    }
    if (FLD_MATCH(TBL_AUTH_SERVER_FLD_SECRET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_AUTH_SERVER_FLD_SECRET].name,
            p_auth_server->secret);
    }
    if (FLD_MATCH(TBL_AUTH_SERVER_FLD_ENC_SECRET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_AUTH_SERVER_FLD_ENC_SECRET].name,
            p_auth_server->enc_secret);
    }
    if (FLD_MATCH(TBL_AUTH_SERVER_FLD_PORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_AUTH_SERVER_FLD_PORT].name,
            p_auth_server->port);
    }
    if (FLD_MATCH(TBL_AUTH_SERVER_FLD_TIMEOUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_AUTH_SERVER_FLD_TIMEOUT].name,
            p_auth_server->timeout);
    }
    if (FLD_MATCH(TBL_AUTH_SERVER_FLD_RETRIES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_AUTH_SERVER_FLD_RETRIES].name,
            p_auth_server->retries);
    }
    if (FLD_MATCH(TBL_AUTH_SERVER_FLD_DEAD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_AUTH_SERVER_FLD_DEAD].name,
            p_auth_server->dead);
    }
    if (FLD_MATCH(TBL_AUTH_SERVER_FLD_IS_INBAND, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_AUTH_SERVER_FLD_IS_INBAND].name,
            p_auth_server->is_inband);
    }
    if (FLD_MATCH(TBL_AUTH_SERVER_FLD_REAWAKE_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_AUTH_SERVER_FLD_REAWAKE_TIME].name,
            p_auth_server->reawake_time);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_auth_server_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_auth_server_master_t *p_master = _g_p_tbl_auth_server_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_auth_server_t *p_db_auth_server = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->auth_servers, p_db_auth_server, listnode, next)
    {
        rc |= fn(p_db_auth_server, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_auth_server_master_t*
tbl_auth_server_get_master()
{
    return _g_p_tbl_auth_server_master;
}

tbl_auth_server_master_t*
tbl_auth_server_init_auth_server()
{
    _g_p_tbl_auth_server_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_auth_server_master_t));
    _g_p_tbl_auth_server_master->auth_servers = ctclib_slist_create(tbl_auth_server_cmp, NULL);
    return _g_p_tbl_auth_server_master;
}

