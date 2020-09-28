
#include "proto.h"
#include "gen/tbl_dot1x_global_define.h"
#include "gen/tbl_dot1x_global.h"
#include "cdb_data_cmp.h"

tbl_dot1x_global_t *_g_p_tbl_dot1x_global = NULL;

int32
tbl_dot1x_global_set_dot1x_global_field_sync(tbl_dot1x_global_t *p_dot1x_global, tbl_dot1x_global_field_id_t field_id, uint32 sync)
{
    tbl_dot1x_global_t *p_db_dot1x_global = _g_p_tbl_dot1x_global;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_DOT1X_GLOBAL_FLD_GLOBAL_AUTH_ENABLE:
        p_db_dot1x_global->global_auth_enable = p_dot1x_global->global_auth_enable;
        break;

    case TBL_DOT1X_GLOBAL_FLD_EAPOL_GROUP_ADDRESS:
        sal_memcpy(p_db_dot1x_global->eapol_group_address, p_dot1x_global->eapol_group_address, sizeof(p_dot1x_global->eapol_group_address));
        break;

    case TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_RETRY:
        p_db_dot1x_global->radius_default_retry = p_dot1x_global->radius_default_retry;
        break;

    case TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_TIMEOUT:
        p_db_dot1x_global->radius_default_timeout = p_dot1x_global->radius_default_timeout;
        break;

    case TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_DEADTIME:
        p_db_dot1x_global->radius_default_deadtime = p_dot1x_global->radius_default_deadtime;
        break;

    case TBL_DOT1X_GLOBAL_FLD_MAC_USER_COUNT_MAX:
        p_db_dot1x_global->mac_user_count_max = p_dot1x_global->mac_user_count_max;
        break;

    case TBL_DOT1X_GLOBAL_FLD_MAC_USER_COUNT_CURRENT:
        p_db_dot1x_global->mac_user_count_current = p_dot1x_global->mac_user_count_current;
        break;

    case TBL_DOT1X_GLOBAL_FLD_MAC_USER_COUNT_DB:
        p_db_dot1x_global->mac_user_count_db = p_dot1x_global->mac_user_count_db;
        break;

    case TBL_DOT1X_GLOBAL_FLD_MAC_PORT_COUNT_CURRENT:
        p_db_dot1x_global->mac_port_count_current = p_dot1x_global->mac_port_count_current;
        break;

    case TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_SHARED_SECRET_VALID:
        p_db_dot1x_global->radius_default_shared_secret_valid = p_dot1x_global->radius_default_shared_secret_valid;
        break;

    case TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_SHARED_SECRET:
        sal_memcpy(p_db_dot1x_global->radius_default_shared_secret, p_dot1x_global->radius_default_shared_secret, sizeof(p_dot1x_global->radius_default_shared_secret));
        break;

    case TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_SHARED_ENC_SECRET:
        sal_memcpy(p_db_dot1x_global->radius_default_shared_enc_secret, p_dot1x_global->radius_default_shared_enc_secret, sizeof(p_dot1x_global->radius_default_shared_enc_secret));
        break;

    case TBL_DOT1X_GLOBAL_FLD_LAST_SESSION_ID:
        p_db_dot1x_global->last_session_id = p_dot1x_global->last_session_id;
        break;

    case TBL_DOT1X_GLOBAL_FLD_SESSION_ID_ALLOCATED:
        sal_memcpy(p_db_dot1x_global->session_id_allocated, p_dot1x_global->session_id_allocated, sizeof(p_dot1x_global->session_id_allocated));
        break;

    case TBL_DOT1X_GLOBAL_FLD_DOT1X_ALL:
        CDB_FLAG_CHECK_SET(p_db_dot1x_global, p_dot1x_global, dot1x, DOT1X_FLAG_ALL)
        break;

    case TBL_DOT1X_GLOBAL_FLD_DOT1X_PACKET:
        CDB_FLAG_CHECK_SET(p_db_dot1x_global, p_dot1x_global, dot1x, DOT1X_FLAG_PACKET)
        break;

    case TBL_DOT1X_GLOBAL_FLD_DOT1X_TIMER:
        CDB_FLAG_CHECK_SET(p_db_dot1x_global, p_dot1x_global, dot1x, DOT1X_FLAG_TIMER)
        break;

    case TBL_DOT1X_GLOBAL_FLD_DOT1X_PROTOCOL:
        CDB_FLAG_CHECK_SET(p_db_dot1x_global, p_dot1x_global, dot1x, DOT1X_FLAG_PROTO)
        break;

    case TBL_DOT1X_GLOBAL_FLD_DOT1X_EVENT:
        CDB_FLAG_CHECK_SET(p_db_dot1x_global, p_dot1x_global, dot1x, DOT1X_FLAG_EVENTS)
        break;

    case TBL_DOT1X_GLOBAL_FLD_MAX:
        sal_memcpy(p_db_dot1x_global, p_dot1x_global, sizeof(tbl_dot1x_global_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_DOT1X_GLOBAL, field_id, p_db_dot1x_global);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_DOT1X_GLOBAL, field_id, p_db_dot1x_global);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_dot1x_global_set_dot1x_global_field(tbl_dot1x_global_t *p_dot1x_global, tbl_dot1x_global_field_id_t field_id)
{
    return tbl_dot1x_global_set_dot1x_global_field_sync(p_dot1x_global, field_id, TRUE);
}

tbl_dot1x_global_t*
tbl_dot1x_global_get_dot1x_global()
{
    return _g_p_tbl_dot1x_global;
}

int32
tbl_dot1x_global_key_str2val(char *str, tbl_dot1x_global_t *p_dot1x_global)
{
    return PM_E_NONE;
}

char*
tbl_dot1x_global_key_name_dump(tbl_dot1x_global_t *p_dot1x_global, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_DOT1X_GLOBAL);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_dot1x_global_key_value_dump(tbl_dot1x_global_t *p_dot1x_global, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_dot1x_global_field_name_dump(tbl_dot1x_global_t *p_dot1x_global, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_DOT1X_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_DOT1X_GLOBAL_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_dot1x_global_field_value_dump(tbl_dot1x_global_t *p_dot1x_global, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_GLOBAL_AUTH_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_global->global_auth_enable);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_EAPOL_GROUP_ADDRESS, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_dot1x_global->eapol_group_address);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_RETRY, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_global->radius_default_retry);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_TIMEOUT, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_global->radius_default_timeout);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_DEADTIME, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_global->radius_default_deadtime);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_MAC_USER_COUNT_MAX, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_global->mac_user_count_max);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_MAC_USER_COUNT_CURRENT, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_global->mac_user_count_current);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_MAC_USER_COUNT_DB, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_global->mac_user_count_db);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_MAC_PORT_COUNT_CURRENT, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_global->mac_port_count_current);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_SHARED_SECRET_VALID, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_global->radius_default_shared_secret_valid);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_SHARED_SECRET, field_id))
    {
        cdb_uint8_array_val2str(str, MAX_CMD_STR_LEN, p_dot1x_global->radius_default_shared_secret, sizeof(p_dot1x_global->radius_default_shared_secret));
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_SHARED_ENC_SECRET, field_id))
    {
        sal_sprintf(str, "%s", p_dot1x_global->radius_default_shared_enc_secret);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_LAST_SESSION_ID, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_global->last_session_id);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_SESSION_ID_ALLOCATED, field_id))
    {
        cdb_uint8_array_val2str(str, MAX_CMD_STR_LEN, p_dot1x_global->session_id_allocated, sizeof(p_dot1x_global->session_id_allocated));
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_DOT1X_ALL, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dot1x_global->dot1x, DOT1X_FLAG_ALL));
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_DOT1X_PACKET, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dot1x_global->dot1x, DOT1X_FLAG_PACKET));
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_DOT1X_TIMER, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dot1x_global->dot1x, DOT1X_FLAG_TIMER));
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_DOT1X_PROTOCOL, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dot1x_global->dot1x, DOT1X_FLAG_PROTO));
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_DOT1X_EVENT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_dot1x_global->dot1x, DOT1X_FLAG_EVENTS));
    }
    return str;
}

char**
tbl_dot1x_global_table_dump(tbl_dot1x_global_t *p_dot1x_global, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_dot1x_global_key_name_dump(p_dot1x_global, buf));
    for(i=1; i<TBL_DOT1X_GLOBAL_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_dot1x_global_field_name_dump(p_dot1x_global, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_dot1x_global_field_value_dump(p_dot1x_global, i, buf));
    }
    return str;
}

int32
tbl_dot1x_global_field_value_parser(char *str, int32 field_id, tbl_dot1x_global_t *p_dot1x_global)
{
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_GLOBAL_AUTH_ENABLE, field_id))
    {
        int32 ret;
        p_dot1x_global->global_auth_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_EAPOL_GROUP_ADDRESS, field_id))
    {
        cdb_mac_addr_str2val(p_dot1x_global->eapol_group_address, str);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_RETRY, field_id))
    {
        int32 ret;
        p_dot1x_global->radius_default_retry = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_TIMEOUT, field_id))
    {
        int32 ret;
        p_dot1x_global->radius_default_timeout = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_DEADTIME, field_id))
    {
        int32 ret;
        p_dot1x_global->radius_default_deadtime = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_MAC_USER_COUNT_MAX, field_id))
    {
        int32 ret;
        p_dot1x_global->mac_user_count_max = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_MAC_USER_COUNT_CURRENT, field_id))
    {
        int32 ret;
        p_dot1x_global->mac_user_count_current = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_MAC_USER_COUNT_DB, field_id))
    {
        int32 ret;
        p_dot1x_global->mac_user_count_db = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_MAC_PORT_COUNT_CURRENT, field_id))
    {
        int32 ret;
        p_dot1x_global->mac_port_count_current = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_SHARED_SECRET_VALID, field_id))
    {
        int32 ret;
        p_dot1x_global->radius_default_shared_secret_valid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_SHARED_SECRET, field_id))
    {
        cdb_uint8_array_str2val(str, p_dot1x_global->radius_default_shared_secret);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_SHARED_ENC_SECRET, field_id))
    {
        sal_strcpy(p_dot1x_global->radius_default_shared_enc_secret, str);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_LAST_SESSION_ID, field_id))
    {
        int32 ret;
        p_dot1x_global->last_session_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_SESSION_ID_ALLOCATED, field_id))
    {
        cdb_uint8_array_str2val(str, p_dot1x_global->session_id_allocated);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_DOT1X_ALL, field_id))
    {
        GLB_SET_FLAG(p_dot1x_global->dot1x, DOT1X_FLAG_ALL);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_DOT1X_PACKET, field_id))
    {
        GLB_SET_FLAG(p_dot1x_global->dot1x, DOT1X_FLAG_PACKET);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_DOT1X_TIMER, field_id))
    {
        GLB_SET_FLAG(p_dot1x_global->dot1x, DOT1X_FLAG_TIMER);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_DOT1X_PROTOCOL, field_id))
    {
        GLB_SET_FLAG(p_dot1x_global->dot1x, DOT1X_FLAG_PROTO);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_DOT1X_EVENT, field_id))
    {
        GLB_SET_FLAG(p_dot1x_global->dot1x, DOT1X_FLAG_EVENTS);
    }
    return PM_E_NONE;
}

int32
tbl_dot1x_global_table_parser(char** array, char* key_value,tbl_dot1x_global_t *p_dot1x_global)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_DOT1X_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_dot1x_global_key_str2val(key_value, p_dot1x_global));

    for(i=1; i<TBL_DOT1X_GLOBAL_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_dot1x_global_field_value_parser( array[j++], i, p_dot1x_global));
    }

    return PM_E_NONE;
}

int32
tbl_dot1x_global_dump_one(tbl_dot1x_global_t *p_dot1x_global, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_DOT1X_GLOBAL);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_GLOBAL_AUTH_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_GLOBAL_FLD_GLOBAL_AUTH_ENABLE].name,
            p_dot1x_global->global_auth_enable);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_EAPOL_GROUP_ADDRESS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DOT1X_GLOBAL_FLD_EAPOL_GROUP_ADDRESS].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_dot1x_global->eapol_group_address));
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_RETRY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_RETRY].name,
            p_dot1x_global->radius_default_retry);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_TIMEOUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_TIMEOUT].name,
            p_dot1x_global->radius_default_timeout);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_DEADTIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_DEADTIME].name,
            p_dot1x_global->radius_default_deadtime);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_MAC_USER_COUNT_MAX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_GLOBAL_FLD_MAC_USER_COUNT_MAX].name,
            p_dot1x_global->mac_user_count_max);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_MAC_USER_COUNT_CURRENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_GLOBAL_FLD_MAC_USER_COUNT_CURRENT].name,
            p_dot1x_global->mac_user_count_current);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_MAC_USER_COUNT_DB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_GLOBAL_FLD_MAC_USER_COUNT_DB].name,
            p_dot1x_global->mac_user_count_db);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_MAC_PORT_COUNT_CURRENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_GLOBAL_FLD_MAC_PORT_COUNT_CURRENT].name,
            p_dot1x_global->mac_port_count_current);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_SHARED_SECRET_VALID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_SHARED_SECRET_VALID].name,
            p_dot1x_global->radius_default_shared_secret_valid);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_SHARED_SECRET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_SHARED_SECRET].name, 
            cdb_uint8_array_val2str(buf, MAX_CMD_STR_LEN, p_dot1x_global->radius_default_shared_secret, sizeof(p_dot1x_global->radius_default_shared_secret)));
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_SHARED_ENC_SECRET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DOT1X_GLOBAL_FLD_RADIUS_DEFAULT_SHARED_ENC_SECRET].name,
            p_dot1x_global->radius_default_shared_enc_secret);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_LAST_SESSION_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_GLOBAL_FLD_LAST_SESSION_ID].name,
            p_dot1x_global->last_session_id);
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_SESSION_ID_ALLOCATED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DOT1X_GLOBAL_FLD_SESSION_ID_ALLOCATED].name, 
            cdb_uint8_array_val2str(buf, MAX_CMD_STR_LEN, p_dot1x_global->session_id_allocated, sizeof(p_dot1x_global->session_id_allocated)));
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_DOT1X_ALL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_GLOBAL_FLD_DOT1X_ALL].name,
            GLB_FLAG_ISSET(p_dot1x_global->dot1x, DOT1X_FLAG_ALL));
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_DOT1X_PACKET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_GLOBAL_FLD_DOT1X_PACKET].name,
            GLB_FLAG_ISSET(p_dot1x_global->dot1x, DOT1X_FLAG_PACKET));
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_DOT1X_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_GLOBAL_FLD_DOT1X_TIMER].name,
            GLB_FLAG_ISSET(p_dot1x_global->dot1x, DOT1X_FLAG_TIMER));
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_DOT1X_PROTOCOL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_GLOBAL_FLD_DOT1X_PROTOCOL].name,
            GLB_FLAG_ISSET(p_dot1x_global->dot1x, DOT1X_FLAG_PROTO));
    }
    if (FLD_MATCH(TBL_DOT1X_GLOBAL_FLD_DOT1X_EVENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_GLOBAL_FLD_DOT1X_EVENT].name,
            GLB_FLAG_ISSET(p_dot1x_global->dot1x, DOT1X_FLAG_EVENTS));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_dot1x_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_dot1x_global_t *p_db_dot1x_global = _g_p_tbl_dot1x_global;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_dot1x_global)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_dot1x_global, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_dot1x_global_t*
tbl_dot1x_global_init_dot1x_global()
{
    _g_p_tbl_dot1x_global = XCALLOC(MEM_TBL_DOT1X_GLOBAL, sizeof(tbl_dot1x_global_t));
    return _g_p_tbl_dot1x_global;
}

