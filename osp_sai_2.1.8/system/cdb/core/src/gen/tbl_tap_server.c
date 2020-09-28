
#include "proto.h"
#include "gen/tbl_tap_server_define.h"
#include "gen/tbl_tap_server.h"
#include "cdb_data_cmp.h"

tbl_tap_server_t *_g_p_tbl_tap_server = NULL;

int32
tbl_tap_server_set_tap_server_field_sync(tbl_tap_server_t *p_tap_srv, tbl_tap_server_field_id_t field_id, uint32 sync)
{
    tbl_tap_server_t *p_db_tap_srv = _g_p_tbl_tap_server;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_TAP_SERVER_FLD_SERVER_SOCK:
        p_db_tap_srv->server_sock = p_tap_srv->server_sock;
        break;

    case TBL_TAP_SERVER_FLD_CLIENT_SOCK:
        p_db_tap_srv->client_sock = p_tap_srv->client_sock;
        break;

    case TBL_TAP_SERVER_FLD_PEER_ADDR:
        sal_memcpy(&p_db_tap_srv->peer_addr, &p_tap_srv->peer_addr, sizeof(p_tap_srv->peer_addr));
        break;

    case TBL_TAP_SERVER_FLD_LASTREAD:
        sal_memcpy(&p_db_tap_srv->lastread, &p_tap_srv->lastread, sizeof(p_tap_srv->lastread));
        break;

    case TBL_TAP_SERVER_FLD_DEBUG:
        p_db_tap_srv->debug = p_tap_srv->debug;
        break;

    case TBL_TAP_SERVER_FLD_CLIENT_PEER_ADDR:
        sal_memcpy(&p_db_tap_srv->client_peer_addr, &p_tap_srv->client_peer_addr, sizeof(p_tap_srv->client_peer_addr));
        break;

    case TBL_TAP_SERVER_FLD_CLIENT_TEST:
        p_db_tap_srv->client_test = p_tap_srv->client_test;
        break;

    case TBL_TAP_SERVER_FLD_MAX:
        sal_memcpy(p_db_tap_srv, p_tap_srv, sizeof(tbl_tap_server_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_TAP_SERVER, field_id, p_db_tap_srv);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_TAP_SERVER, field_id, p_db_tap_srv);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_tap_server_set_tap_server_field(tbl_tap_server_t *p_tap_srv, tbl_tap_server_field_id_t field_id)
{
    return tbl_tap_server_set_tap_server_field_sync(p_tap_srv, field_id, TRUE);
}

tbl_tap_server_t*
tbl_tap_server_get_tap_server()
{
    return _g_p_tbl_tap_server;
}

int32
tbl_tap_server_key_str2val(char *str, tbl_tap_server_t *p_tap_srv)
{
    return PM_E_NONE;
}

char*
tbl_tap_server_key_name_dump(tbl_tap_server_t *p_tap_srv, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_TAP_SERVER);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_tap_server_key_value_dump(tbl_tap_server_t *p_tap_srv, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_tap_server_field_name_dump(tbl_tap_server_t *p_tap_srv, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_TAP_SERVER);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_TAP_SERVER_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_tap_server_field_value_dump(tbl_tap_server_t *p_tap_srv, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_TAP_SERVER_FLD_SERVER_SOCK, field_id))
    {
        sal_sprintf(str, "%d", p_tap_srv->server_sock);
    }
    if (FLD_MATCH(TBL_TAP_SERVER_FLD_CLIENT_SOCK, field_id))
    {
        sal_sprintf(str, "%d", p_tap_srv->client_sock);
    }
    if (FLD_MATCH(TBL_TAP_SERVER_FLD_PEER_ADDR, field_id))
    {
        cdb_addr_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_tap_srv->peer_addr);
    }
    if (FLD_MATCH(TBL_TAP_SERVER_FLD_LASTREAD, field_id))
    {
        cdb_sal_time_val2str(str, MAX_CMD_STR_LEN, &p_tap_srv->lastread);
    }
    if (FLD_MATCH(TBL_TAP_SERVER_FLD_DEBUG, field_id))
    {
        sal_sprintf(str, "%u", p_tap_srv->debug);
    }
    if (FLD_MATCH(TBL_TAP_SERVER_FLD_CLIENT_PEER_ADDR, field_id))
    {
        cdb_addr_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_tap_srv->client_peer_addr);
    }
    if (FLD_MATCH(TBL_TAP_SERVER_FLD_CLIENT_TEST, field_id))
    {
        sal_sprintf(str, "%u", p_tap_srv->client_test);
    }
    return str;
}

char**
tbl_tap_server_table_dump(tbl_tap_server_t *p_tap_srv, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_tap_server_key_name_dump(p_tap_srv, buf));
    for(i=1; i<TBL_TAP_SERVER_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_tap_server_field_name_dump(p_tap_srv, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_tap_server_field_value_dump(p_tap_srv, i, buf));
    }
    return str;
}

int32
tbl_tap_server_field_value_parser(char *str, int32 field_id, tbl_tap_server_t *p_tap_srv)
{
    if (FLD_MATCH(TBL_TAP_SERVER_FLD_SERVER_SOCK, field_id))
    {
    }
    if (FLD_MATCH(TBL_TAP_SERVER_FLD_CLIENT_SOCK, field_id))
    {
    }
    if (FLD_MATCH(TBL_TAP_SERVER_FLD_PEER_ADDR, field_id))
    {
        cdb_addr_ipv4_str2val(&p_tap_srv->peer_addr, str, 0);
    }
    if (FLD_MATCH(TBL_TAP_SERVER_FLD_LASTREAD, field_id))
    {
        cdb_sal_time_str2val(str, &p_tap_srv->lastread);
    }
    if (FLD_MATCH(TBL_TAP_SERVER_FLD_DEBUG, field_id))
    {
        int32 ret;
        p_tap_srv->debug = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TAP_SERVER_FLD_CLIENT_PEER_ADDR, field_id))
    {
        cdb_addr_ipv4_str2val(&p_tap_srv->client_peer_addr, str, 0);
    }
    if (FLD_MATCH(TBL_TAP_SERVER_FLD_CLIENT_TEST, field_id))
    {
        int32 ret;
        p_tap_srv->client_test = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_tap_server_table_parser(char** array, char* key_value,tbl_tap_server_t *p_tap_srv)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_TAP_SERVER);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_tap_server_key_str2val(key_value, p_tap_srv));

    for(i=1; i<TBL_TAP_SERVER_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_tap_server_field_value_parser( array[j++], i, p_tap_srv));
    }

    return PM_E_NONE;
}

int32
tbl_tap_server_dump_one(tbl_tap_server_t *p_tap_srv, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_TAP_SERVER);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_TAP_SERVER_FLD_SERVER_SOCK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_TAP_SERVER_FLD_SERVER_SOCK].name,
            p_tap_srv->server_sock);
    }
    if (FLD_MATCH(TBL_TAP_SERVER_FLD_CLIENT_SOCK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_TAP_SERVER_FLD_CLIENT_SOCK].name,
            p_tap_srv->client_sock);
    }
    if (FLD_MATCH(TBL_TAP_SERVER_FLD_PEER_ADDR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_TAP_SERVER_FLD_PEER_ADDR].name, 
            cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_tap_srv->peer_addr));
    }
    if (FLD_MATCH(TBL_TAP_SERVER_FLD_LASTREAD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_TAP_SERVER_FLD_LASTREAD].name, 
            cdb_sal_time_val2str(buf, MAX_CMD_STR_LEN, &p_tap_srv->lastread));
    }
    if (FLD_MATCH(TBL_TAP_SERVER_FLD_DEBUG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TAP_SERVER_FLD_DEBUG].name,
            p_tap_srv->debug);
    }
    if (FLD_MATCH(TBL_TAP_SERVER_FLD_CLIENT_PEER_ADDR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_TAP_SERVER_FLD_CLIENT_PEER_ADDR].name, 
            cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_tap_srv->client_peer_addr));
    }
    if (FLD_MATCH(TBL_TAP_SERVER_FLD_CLIENT_TEST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TAP_SERVER_FLD_CLIENT_TEST].name,
            p_tap_srv->client_test);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_tap_server_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_tap_server_t *p_db_tap_srv = _g_p_tbl_tap_server;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_tap_srv)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_tap_srv, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_tap_server_t*
tbl_tap_server_init_tap_server()
{
    _g_p_tbl_tap_server = XCALLOC(MEM_TBL_TAP_SERVER, sizeof(tbl_tap_server_t));
    return _g_p_tbl_tap_server;
}

