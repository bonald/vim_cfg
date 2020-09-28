
#include "proto.h"
#include "gen/tbl_dot1x_radius_define.h"
#include "gen/tbl_dot1x_radius.h"
#include "cdb_data_cmp.h"

tbl_dot1x_radius_master_t *_g_p_tbl_dot1x_radius_master = NULL;

int32
tbl_dot1x_radius_add_dot1x_radius_sync(tbl_dot1x_radius_t *p_dot1x_radius, uint32 sync)
{
    tbl_dot1x_radius_master_t *p_master = _g_p_tbl_dot1x_radius_master;
    tbl_dot1x_radius_t *p_db_dot1x_radius = NULL;

    /* 1. lookup entry exist */
    if (tbl_dot1x_radius_get_dot1x_radius(&p_dot1x_radius->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_dot1x_radius = XCALLOC(MEM_TBL_DOT1X_RADIUS, sizeof(tbl_dot1x_radius_t));
    if (NULL == p_db_dot1x_radius)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_dot1x_radius, p_dot1x_radius, sizeof(tbl_dot1x_radius_t));

    /* 4. add to db */
    ctclib_slistnode_insert_sort(p_master->radius_list, p_db_dot1x_radius);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_DOT1X_RADIUS, p_db_dot1x_radius);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_DOT1X_RADIUS, p_db_dot1x_radius);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_dot1x_radius_del_dot1x_radius_sync(tbl_dot1x_radius_key_t *p_dot1x_radius_key, uint32 sync)
{
    tbl_dot1x_radius_master_t *p_master = _g_p_tbl_dot1x_radius_master;
    tbl_dot1x_radius_t *p_db_dot1x_radius = NULL;

    /* 1. lookup entry exist */
    p_db_dot1x_radius = tbl_dot1x_radius_get_dot1x_radius(p_dot1x_radius_key);
    if (NULL == p_db_dot1x_radius)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_DOT1X_RADIUS, p_db_dot1x_radius);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_DOT1X_RADIUS, p_db_dot1x_radius);
        #endif 
    }

    /* 3. delete from db */
    ctclib_slistnode_delete(p_master->radius_list, p_db_dot1x_radius);

    /* 4. free db entry */
    XFREE(MEM_TBL_DOT1X_RADIUS, p_db_dot1x_radius);

    return PM_E_NONE;
}

int32
tbl_dot1x_radius_set_dot1x_radius_field_sync(tbl_dot1x_radius_t *p_dot1x_radius, tbl_dot1x_radius_field_id_t field_id, uint32 sync)
{
    tbl_dot1x_radius_t *p_db_dot1x_radius = NULL;

    /* 1. lookup entry exist */
    p_db_dot1x_radius = tbl_dot1x_radius_get_dot1x_radius(&p_dot1x_radius->key);
    if (NULL == p_db_dot1x_radius)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_DOT1X_RADIUS_FLD_IDENTIFIER:
        p_db_dot1x_radius->identifier = p_dot1x_radius->identifier;
        break;

    case TBL_DOT1X_RADIUS_FLD_SERVER_STRING:
        sal_memcpy(p_db_dot1x_radius->server_string, p_dot1x_radius->server_string, sizeof(p_dot1x_radius->server_string));
        break;

    case TBL_DOT1X_RADIUS_FLD_SERVER_ADDR:
        sal_memcpy(&p_db_dot1x_radius->server_addr, &p_dot1x_radius->server_addr, sizeof(p_dot1x_radius->server_addr));
        break;

    case TBL_DOT1X_RADIUS_FLD_RADIUS_STATE_LEN:
        p_db_dot1x_radius->radius_state_len = p_dot1x_radius->radius_state_len;
        break;

    case TBL_DOT1X_RADIUS_FLD_RADIUS_STATE:
        sal_memcpy(p_db_dot1x_radius->radius_state, p_dot1x_radius->radius_state, sizeof(p_dot1x_radius->radius_state));
        break;

    case TBL_DOT1X_RADIUS_FLD_MAX_RETRY:
        p_db_dot1x_radius->max_retry = p_dot1x_radius->max_retry;
        break;

    case TBL_DOT1X_RADIUS_FLD_RADIUS_DEFALT_TIMEOUT:
        CDB_FLAG_CHECK_SET(p_db_dot1x_radius, p_dot1x_radius, radius_config, AUTH_RADIUS_CONFIG_DEFAULT_TIMEOUT)
        break;

    case TBL_DOT1X_RADIUS_FLD_RADIUS_DEFALT_RETRANSMIT:
        CDB_FLAG_CHECK_SET(p_db_dot1x_radius, p_dot1x_radius, radius_config, AUTH_RADIUS_CONFIG_DEFAULT_RETRANSMIT)
        break;

    case TBL_DOT1X_RADIUS_FLD_RADIUS_DEFALT_KEY:
        CDB_FLAG_CHECK_SET(p_db_dot1x_radius, p_dot1x_radius, radius_config, AUTH_RADIUS_CONFIG_DEFAULT_KEY)
        break;

    case TBL_DOT1X_RADIUS_FLD_TIMEOUT:
        p_db_dot1x_radius->timeout = p_dot1x_radius->timeout;
        break;

    case TBL_DOT1X_RADIUS_FLD_SHARED_SECRET_VALID:
        p_db_dot1x_radius->shared_secret_valid = p_dot1x_radius->shared_secret_valid;
        break;

    case TBL_DOT1X_RADIUS_FLD_SHARED_SECRET:
        sal_memcpy(p_db_dot1x_radius->shared_secret, p_dot1x_radius->shared_secret, sizeof(p_dot1x_radius->shared_secret));
        break;

    case TBL_DOT1X_RADIUS_FLD_SHARED_ENC_SECRET:
        sal_memcpy(p_db_dot1x_radius->shared_enc_secret, p_dot1x_radius->shared_enc_secret, sizeof(p_dot1x_radius->shared_enc_secret));
        break;

    case TBL_DOT1X_RADIUS_FLD_AUTHENTICATOR:
        sal_memcpy(p_db_dot1x_radius->authenticator, p_dot1x_radius->authenticator, sizeof(p_dot1x_radius->authenticator));
        break;

    case TBL_DOT1X_RADIUS_FLD_OAUTHENTICATOR:
        sal_memcpy(p_db_dot1x_radius->oauthenticator, p_dot1x_radius->oauthenticator, sizeof(p_dot1x_radius->oauthenticator));
        break;

    case TBL_DOT1X_RADIUS_FLD_SESSION_NUMBER:
        p_db_dot1x_radius->session_number = p_dot1x_radius->session_number;
        break;

    case TBL_DOT1X_RADIUS_FLD_SESSION_IFINDEX:
        sal_memcpy(p_db_dot1x_radius->session_ifindex, p_dot1x_radius->session_ifindex, sizeof(p_dot1x_radius->session_ifindex));
        break;

    case TBL_DOT1X_RADIUS_FLD_REACTIVATE:
        p_db_dot1x_radius->reactivate = p_dot1x_radius->reactivate;
        break;

    case TBL_DOT1X_RADIUS_FLD_IS_INBAND:
        p_db_dot1x_radius->is_inband = p_dot1x_radius->is_inband;
        break;

    case TBL_DOT1X_RADIUS_FLD_MAX:
        sal_memcpy(p_db_dot1x_radius, p_dot1x_radius, sizeof(tbl_dot1x_radius_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_DOT1X_RADIUS, field_id, p_db_dot1x_radius);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_DOT1X_RADIUS, field_id, p_db_dot1x_radius);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_dot1x_radius_add_dot1x_radius(tbl_dot1x_radius_t *p_dot1x_radius)
{
    return tbl_dot1x_radius_add_dot1x_radius_sync(p_dot1x_radius, TRUE);
}

int32
tbl_dot1x_radius_del_dot1x_radius(tbl_dot1x_radius_key_t *p_dot1x_radius_key)
{
    return tbl_dot1x_radius_del_dot1x_radius_sync(p_dot1x_radius_key, TRUE);
}

int32
tbl_dot1x_radius_set_dot1x_radius_field(tbl_dot1x_radius_t *p_dot1x_radius, tbl_dot1x_radius_field_id_t field_id)
{
    return tbl_dot1x_radius_set_dot1x_radius_field_sync(p_dot1x_radius, field_id, TRUE);
}

tbl_dot1x_radius_t*
tbl_dot1x_radius_get_dot1x_radius(tbl_dot1x_radius_key_t *p_dot1x_radius_key)
{
    tbl_dot1x_radius_master_t *p_master = _g_p_tbl_dot1x_radius_master;
    tbl_dot1x_radius_t lkp;

    sal_memcpy(&lkp.key, p_dot1x_radius_key, sizeof(tbl_dot1x_radius_key_t));
    return ctclib_slistdata_lookup(p_master->radius_list, &lkp);
}

char*
tbl_dot1x_radius_key_val2str(tbl_dot1x_radius_t *p_dot1x_radius, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_dot1x_radius_key_val2str(buf, MAX_CMD_STR_LEN, &p_dot1x_radius->key));
    return str;
}

int32
tbl_dot1x_radius_key_str2val(char *str, tbl_dot1x_radius_t *p_dot1x_radius)
{
    int32 ret = 0;

    ret = cdb_dot1x_radius_key_str2val(str, &p_dot1x_radius->key);
    return ret;
}

char*
tbl_dot1x_radius_key_name_dump(tbl_dot1x_radius_t *p_dot1x_radius, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_DOT1X_RADIUS);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_dot1x_radius_key_val2str(buf, MAX_CMD_STR_LEN, &p_dot1x_radius->key));
    return str;
}

char*
tbl_dot1x_radius_key_value_dump(tbl_dot1x_radius_t *p_dot1x_radius, char *str)
{

    cdb_dot1x_radius_key_val2str(str, MAX_CMD_STR_LEN, &p_dot1x_radius->key);
    return str;
}

char*
tbl_dot1x_radius_field_name_dump(tbl_dot1x_radius_t *p_dot1x_radius, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_DOT1X_RADIUS);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_DOT1X_RADIUS_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_dot1x_radius_field_value_dump(tbl_dot1x_radius_t *p_dot1x_radius, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_KEY, field_id))
    {
        cdb_dot1x_radius_key_val2str(str, MAX_CMD_STR_LEN, &p_dot1x_radius->key);
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_IDENTIFIER, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_radius->identifier);
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_SERVER_STRING, field_id))
    {
        cdb_uint8_array_val2str(str, MAX_CMD_STR_LEN, p_dot1x_radius->server_string, sizeof(p_dot1x_radius->server_string));
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_SERVER_ADDR, field_id))
    {
        cdb_addr_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_dot1x_radius->server_addr);
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_RADIUS_STATE_LEN, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_radius->radius_state_len);
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_RADIUS_STATE, field_id))
    {
        cdb_uint8_array_val2str(str, MAX_CMD_STR_LEN, p_dot1x_radius->radius_state, sizeof(p_dot1x_radius->radius_state));
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_MAX_RETRY, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_radius->max_retry);
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_RADIUS_DEFALT_TIMEOUT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dot1x_radius->radius_config, AUTH_RADIUS_CONFIG_DEFAULT_TIMEOUT));
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_RADIUS_DEFALT_RETRANSMIT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dot1x_radius->radius_config, AUTH_RADIUS_CONFIG_DEFAULT_RETRANSMIT));
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_RADIUS_DEFALT_KEY, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dot1x_radius->radius_config, AUTH_RADIUS_CONFIG_DEFAULT_KEY));
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_TIMEOUT, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_radius->timeout);
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_SHARED_SECRET_VALID, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_radius->shared_secret_valid);
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_SHARED_SECRET, field_id))
    {
        cdb_uint8_array_val2str(str, MAX_CMD_STR_LEN, p_dot1x_radius->shared_secret, sizeof(p_dot1x_radius->shared_secret));
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_SHARED_ENC_SECRET, field_id))
    {
        sal_sprintf(str, "%s", p_dot1x_radius->shared_enc_secret);
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_AUTHENTICATOR, field_id))
    {
        cdb_uint8_array_val2str(str, MAX_CMD_STR_LEN, p_dot1x_radius->authenticator, sizeof(p_dot1x_radius->authenticator));
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_OAUTHENTICATOR, field_id))
    {
        cdb_uint8_array_val2str(str, MAX_CMD_STR_LEN, p_dot1x_radius->oauthenticator, sizeof(p_dot1x_radius->oauthenticator));
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_SESSION_NUMBER, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_radius->session_number);
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_SESSION_IFINDEX, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_dot1x_radius->session_ifindex, sizeof(p_dot1x_radius->session_ifindex)/4);
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_REACTIVATE, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_radius->reactivate);
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_IS_INBAND, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_radius->is_inband);
    }
    return str;
}

char**
tbl_dot1x_radius_table_dump(tbl_dot1x_radius_t *p_dot1x_radius, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_dot1x_radius_key_name_dump(p_dot1x_radius, buf));
    for(i=1; i<TBL_DOT1X_RADIUS_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_dot1x_radius_field_name_dump(p_dot1x_radius, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_dot1x_radius_field_value_dump(p_dot1x_radius, i, buf));
    }
    return str;
}

int32
tbl_dot1x_radius_field_value_parser(char *str, int32 field_id, tbl_dot1x_radius_t *p_dot1x_radius)
{
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_KEY, field_id))
    {
        cdb_dot1x_radius_key_str2val(str, &p_dot1x_radius->key);
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_IDENTIFIER, field_id))
    {
        int32 ret;
        p_dot1x_radius->identifier = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_SERVER_STRING, field_id))
    {
        cdb_uint8_array_str2val(str, p_dot1x_radius->server_string);
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_SERVER_ADDR, field_id))
    {
        cdb_addr_ipv4_str2val(&p_dot1x_radius->server_addr, str, 0);
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_RADIUS_STATE_LEN, field_id))
    {
        int32 ret;
        p_dot1x_radius->radius_state_len = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_RADIUS_STATE, field_id))
    {
        cdb_uint8_array_str2val(str, p_dot1x_radius->radius_state);
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_MAX_RETRY, field_id))
    {
        int32 ret;
        p_dot1x_radius->max_retry = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_RADIUS_DEFALT_TIMEOUT, field_id))
    {
        GLB_SET_FLAG(p_dot1x_radius->radius_config, AUTH_RADIUS_CONFIG_DEFAULT_TIMEOUT);
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_RADIUS_DEFALT_RETRANSMIT, field_id))
    {
        GLB_SET_FLAG(p_dot1x_radius->radius_config, AUTH_RADIUS_CONFIG_DEFAULT_RETRANSMIT);
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_RADIUS_DEFALT_KEY, field_id))
    {
        GLB_SET_FLAG(p_dot1x_radius->radius_config, AUTH_RADIUS_CONFIG_DEFAULT_KEY);
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_TIMEOUT, field_id))
    {
        int32 ret;
        p_dot1x_radius->timeout = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_SHARED_SECRET_VALID, field_id))
    {
        int32 ret;
        p_dot1x_radius->shared_secret_valid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_SHARED_SECRET, field_id))
    {
        cdb_uint8_array_str2val(str, p_dot1x_radius->shared_secret);
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_SHARED_ENC_SECRET, field_id))
    {
        sal_strcpy(p_dot1x_radius->shared_enc_secret, str);
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_AUTHENTICATOR, field_id))
    {
        cdb_uint8_array_str2val(str, p_dot1x_radius->authenticator);
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_OAUTHENTICATOR, field_id))
    {
        cdb_uint8_array_str2val(str, p_dot1x_radius->oauthenticator);
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_SESSION_NUMBER, field_id))
    {
        int32 ret;
        p_dot1x_radius->session_number = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_SESSION_IFINDEX, field_id))
    {
        cdb_uint32_array_str2val(str, p_dot1x_radius->session_ifindex);
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_REACTIVATE, field_id))
    {
        int32 ret;
        p_dot1x_radius->reactivate = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_IS_INBAND, field_id))
    {
        int32 ret;
        p_dot1x_radius->is_inband = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_dot1x_radius_table_parser(char** array, char* key_value,tbl_dot1x_radius_t *p_dot1x_radius)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_DOT1X_RADIUS);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_dot1x_radius_key_str2val(key_value, p_dot1x_radius));

    for(i=1; i<TBL_DOT1X_RADIUS_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_dot1x_radius_field_value_parser( array[j++], i, p_dot1x_radius));
    }

    return PM_E_NONE;
}

int32
tbl_dot1x_radius_dump_one(tbl_dot1x_radius_t *p_dot1x_radius, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_DOT1X_RADIUS);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DOT1X_RADIUS_FLD_KEY].name, 
            cdb_dot1x_radius_key_val2str(buf, MAX_CMD_STR_LEN, &p_dot1x_radius->key));
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_IDENTIFIER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_RADIUS_FLD_IDENTIFIER].name,
            p_dot1x_radius->identifier);
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_SERVER_STRING, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DOT1X_RADIUS_FLD_SERVER_STRING].name, 
            cdb_uint8_array_val2str(buf, MAX_CMD_STR_LEN, p_dot1x_radius->server_string, sizeof(p_dot1x_radius->server_string)));
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_SERVER_ADDR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DOT1X_RADIUS_FLD_SERVER_ADDR].name, 
            cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_dot1x_radius->server_addr));
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_RADIUS_STATE_LEN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_RADIUS_FLD_RADIUS_STATE_LEN].name,
            p_dot1x_radius->radius_state_len);
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_RADIUS_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DOT1X_RADIUS_FLD_RADIUS_STATE].name, 
            cdb_uint8_array_val2str(buf, MAX_CMD_STR_LEN, p_dot1x_radius->radius_state, sizeof(p_dot1x_radius->radius_state)));
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_MAX_RETRY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_RADIUS_FLD_MAX_RETRY].name,
            p_dot1x_radius->max_retry);
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_RADIUS_DEFALT_TIMEOUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_RADIUS_FLD_RADIUS_DEFALT_TIMEOUT].name,
            GLB_FLAG_ISSET(p_dot1x_radius->radius_config, AUTH_RADIUS_CONFIG_DEFAULT_TIMEOUT));
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_RADIUS_DEFALT_RETRANSMIT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_RADIUS_FLD_RADIUS_DEFALT_RETRANSMIT].name,
            GLB_FLAG_ISSET(p_dot1x_radius->radius_config, AUTH_RADIUS_CONFIG_DEFAULT_RETRANSMIT));
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_RADIUS_DEFALT_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_RADIUS_FLD_RADIUS_DEFALT_KEY].name,
            GLB_FLAG_ISSET(p_dot1x_radius->radius_config, AUTH_RADIUS_CONFIG_DEFAULT_KEY));
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_TIMEOUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_RADIUS_FLD_TIMEOUT].name,
            p_dot1x_radius->timeout);
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_SHARED_SECRET_VALID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_RADIUS_FLD_SHARED_SECRET_VALID].name,
            p_dot1x_radius->shared_secret_valid);
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_SHARED_SECRET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DOT1X_RADIUS_FLD_SHARED_SECRET].name, 
            cdb_uint8_array_val2str(buf, MAX_CMD_STR_LEN, p_dot1x_radius->shared_secret, sizeof(p_dot1x_radius->shared_secret)));
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_SHARED_ENC_SECRET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DOT1X_RADIUS_FLD_SHARED_ENC_SECRET].name,
            p_dot1x_radius->shared_enc_secret);
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_AUTHENTICATOR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DOT1X_RADIUS_FLD_AUTHENTICATOR].name, 
            cdb_uint8_array_val2str(buf, MAX_CMD_STR_LEN, p_dot1x_radius->authenticator, sizeof(p_dot1x_radius->authenticator)));
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_OAUTHENTICATOR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DOT1X_RADIUS_FLD_OAUTHENTICATOR].name, 
            cdb_uint8_array_val2str(buf, MAX_CMD_STR_LEN, p_dot1x_radius->oauthenticator, sizeof(p_dot1x_radius->oauthenticator)));
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_SESSION_NUMBER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_RADIUS_FLD_SESSION_NUMBER].name,
            p_dot1x_radius->session_number);
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_SESSION_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DOT1X_RADIUS_FLD_SESSION_IFINDEX].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_dot1x_radius->session_ifindex, sizeof(p_dot1x_radius->session_ifindex)/4));
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_REACTIVATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_RADIUS_FLD_REACTIVATE].name,
            p_dot1x_radius->reactivate);
    }
    if (FLD_MATCH(TBL_DOT1X_RADIUS_FLD_IS_INBAND, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_RADIUS_FLD_IS_INBAND].name,
            p_dot1x_radius->is_inband);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_dot1x_radius_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_dot1x_radius_master_t *p_master = _g_p_tbl_dot1x_radius_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_dot1x_radius_t *p_db_dot1x_radius = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->radius_list, p_db_dot1x_radius, listnode, next)
    {
        rc |= fn(p_db_dot1x_radius, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_dot1x_radius_master_t*
tbl_dot1x_radius_get_master()
{
    return _g_p_tbl_dot1x_radius_master;
}

tbl_dot1x_radius_master_t*
tbl_dot1x_radius_init_dot1x_radius()
{
    _g_p_tbl_dot1x_radius_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_dot1x_radius_master_t));
    _g_p_tbl_dot1x_radius_master->radius_list = ctclib_slist_create(tbl_dot1x_radius_cmp, NULL);
    return _g_p_tbl_dot1x_radius_master;
}

