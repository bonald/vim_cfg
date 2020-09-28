
#include "proto.h"
#include "gen/tbl_vty_define.h"
#include "gen/tbl_vty.h"
#include "cdb_data_cmp.h"

tbl_vty_master_t *_g_p_tbl_vty_master = NULL;

int32
tbl_vty_add_vty_sync(tbl_vty_t *p_vty, uint32 sync)
{
    tbl_vty_master_t *p_master = _g_p_tbl_vty_master;
    tbl_vty_t *p_db_vty = NULL;

    /* 1. lookup entry exist */
    if (tbl_vty_get_vty(&p_vty->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_vty = XCALLOC(MEM_TBL_VTY, sizeof(tbl_vty_t));
    if (NULL == p_db_vty)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_vty, p_vty, sizeof(tbl_vty_t));

    /* 4. add to db */
    p_master->vty_array[p_vty->key.id] = p_db_vty;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_VTY, p_db_vty);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_VTY, p_db_vty);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_vty_del_vty_sync(tbl_vty_key_t *p_vty_key, uint32 sync)
{
    tbl_vty_master_t *p_master = _g_p_tbl_vty_master;
    tbl_vty_t *p_db_vty = NULL;

    /* 1. lookup entry exist */
    p_db_vty = tbl_vty_get_vty(p_vty_key);
    if (NULL == p_db_vty)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_VTY, p_db_vty);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_VTY, p_db_vty);
        #endif 
    }

    /* 3. delete from db */
    p_master->vty_array[p_vty_key->id] = NULL;

    /* 4. free db entry */
    XFREE(MEM_TBL_VTY, p_db_vty);

    return PM_E_NONE;
}

int32
tbl_vty_set_vty_field_sync(tbl_vty_t *p_vty, tbl_vty_field_id_t field_id, uint32 sync)
{
    tbl_vty_t *p_db_vty = NULL;

    /* 1. lookup entry exist */
    p_db_vty = tbl_vty_get_vty(&p_vty->key);
    if (NULL == p_db_vty)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_VTY_FLD_TIMEOUT_MIN:
        p_db_vty->timeout_min = p_vty->timeout_min;
        break;

    case TBL_VTY_FLD_TIMEOUT_SEC:
        p_db_vty->timeout_sec = p_vty->timeout_sec;
        break;

    case TBL_VTY_FLD_PROTOCOL:
        sal_memcpy(p_db_vty->protocol, p_vty->protocol, sizeof(p_vty->protocol));
        break;

    case TBL_VTY_FLD_LOGIN:
        p_db_vty->login = p_vty->login;
        break;

    case TBL_VTY_FLD_PRIVILEGE:
        p_db_vty->privilege = p_vty->privilege;
        break;

    case TBL_VTY_FLD_ENC_PASSWD:
        sal_memcpy(p_db_vty->enc_passwd, p_vty->enc_passwd, sizeof(p_vty->enc_passwd));
        break;

    case TBL_VTY_FLD_PASSWD:
        sal_memcpy(p_db_vty->passwd, p_vty->passwd, sizeof(p_vty->passwd));
        break;

    case TBL_VTY_FLD_AUTH_METHOD:
        sal_memcpy(p_db_vty->auth_method, p_vty->auth_method, sizeof(p_vty->auth_method));
        break;

    case TBL_VTY_FLD_INUSE:
        p_db_vty->inuse = p_vty->inuse;
        break;

    case TBL_VTY_FLD_AUTHOR_METHOD:
        sal_memcpy(p_db_vty->author_method, p_vty->author_method, sizeof(p_vty->author_method));
        break;

    case TBL_VTY_FLD_ACCOUNT_METHOD:
        sal_memcpy(p_db_vty->account_method, p_vty->account_method, sizeof(p_vty->account_method));
        break;

    case TBL_VTY_FLD_ACCOUNTCMD_METHOD:
        sal_memcpy(p_db_vty->accountcmd_method, p_vty->accountcmd_method, sizeof(p_vty->accountcmd_method));
        break;

    case TBL_VTY_FLD_IPV4_ACL:
        sal_memcpy(p_db_vty->ipv4_acl, p_vty->ipv4_acl, sizeof(p_vty->ipv4_acl));
        break;

    case TBL_VTY_FLD_IPV6_ACL:
        sal_memcpy(p_db_vty->ipv6_acl, p_vty->ipv6_acl, sizeof(p_vty->ipv6_acl));
        break;

    case TBL_VTY_FLD_AAA_PRIVILEGE:
        p_db_vty->aaa_privilege = p_vty->aaa_privilege;
        break;

    case TBL_VTY_FLD_CONFIG_PRIVILEGE:
        p_db_vty->config_privilege = p_vty->config_privilege;
        break;

    case TBL_VTY_FLD_AAA_START_TIME:
        p_db_vty->aaa_start_time = p_vty->aaa_start_time;
        break;

    case TBL_VTY_FLD_CMD_BUF:
        sal_memcpy(p_db_vty->cmd_buf, p_vty->cmd_buf, sizeof(p_vty->cmd_buf));
        break;

    case TBL_VTY_FLD_OLD_CMD_BUF:
        sal_memcpy(p_db_vty->old_cmd_buf, p_vty->old_cmd_buf, sizeof(p_vty->old_cmd_buf));
        break;

    case TBL_VTY_FLD_ACCT_STOP_IPADDR:
        sal_memcpy(p_db_vty->acct_stop_ipaddr, p_vty->acct_stop_ipaddr, sizeof(p_vty->acct_stop_ipaddr));
        break;

    case TBL_VTY_FLD_ACCT_STOP_PID:
        p_db_vty->acct_stop_pid = p_vty->acct_stop_pid;
        break;

    case TBL_VTY_FLD_ACCT_STOP_PRIVILEGE:
        p_db_vty->acct_stop_privilege = p_vty->acct_stop_privilege;
        break;

    case TBL_VTY_FLD_IS_NOT_SSH_KEY:
        p_db_vty->is_not_ssh_key = p_vty->is_not_ssh_key;
        break;

    case TBL_VTY_FLD_IS_SSH_LOGIN:
        p_db_vty->is_ssh_login = p_vty->is_ssh_login;
        break;

    case TBL_VTY_FLD_MAX:
        sal_memcpy(p_db_vty, p_vty, sizeof(tbl_vty_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_VTY, field_id, p_db_vty);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_VTY, field_id, p_db_vty);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_vty_add_vty(tbl_vty_t *p_vty)
{
    return tbl_vty_add_vty_sync(p_vty, TRUE);
}

int32
tbl_vty_del_vty(tbl_vty_key_t *p_vty_key)
{
    return tbl_vty_del_vty_sync(p_vty_key, TRUE);
}

int32
tbl_vty_set_vty_field(tbl_vty_t *p_vty, tbl_vty_field_id_t field_id)
{
    return tbl_vty_set_vty_field_sync(p_vty, field_id, TRUE);
}

tbl_vty_t*
tbl_vty_get_vty(tbl_vty_key_t *p_vty_key)
{
    tbl_vty_master_t *p_master = _g_p_tbl_vty_master;

    if (GLB_MAX_VTY_NUM <= p_vty_key->id)
    {
        return NULL;
    }
    return (p_master->vty_array[p_vty_key->id]);
}

char*
tbl_vty_key_val2str(tbl_vty_t *p_vty, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_vty->key.id);
    return str;
}

int32
tbl_vty_key_str2val(char *str, tbl_vty_t *p_vty)
{
    int32 ret = 0;

    p_vty->key.id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_vty_key_name_dump(tbl_vty_t *p_vty, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_VTY);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_vty->key.id);
    return str;
}

char*
tbl_vty_key_value_dump(tbl_vty_t *p_vty, char *str)
{
    sal_sprintf(str, "%u", p_vty->key.id);
    return str;
}

char*
tbl_vty_field_name_dump(tbl_vty_t *p_vty, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_VTY);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_VTY_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_vty_field_value_dump(tbl_vty_t *p_vty, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_VTY_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_vty->key.id);
    }
    if (FLD_MATCH(TBL_VTY_FLD_TIMEOUT_MIN, field_id))
    {
        sal_sprintf(str, "%u", p_vty->timeout_min);
    }
    if (FLD_MATCH(TBL_VTY_FLD_TIMEOUT_SEC, field_id))
    {
        sal_sprintf(str, "%u", p_vty->timeout_sec);
    }
    if (FLD_MATCH(TBL_VTY_FLD_PROTOCOL, field_id))
    {
        sal_sprintf(str, "%s", p_vty->protocol);
    }
    if (FLD_MATCH(TBL_VTY_FLD_LOGIN, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(login_method_strs, LOGIN_METHOD_MAX, p_vty->login));
    }
    if (FLD_MATCH(TBL_VTY_FLD_PRIVILEGE, field_id))
    {
        sal_sprintf(str, "%u", p_vty->privilege);
    }
    if (FLD_MATCH(TBL_VTY_FLD_ENC_PASSWD, field_id))
    {
        sal_sprintf(str, "%s", p_vty->enc_passwd);
    }
    if (FLD_MATCH(TBL_VTY_FLD_PASSWD, field_id))
    {
        sal_sprintf(str, "%s", p_vty->passwd);
    }
    if (FLD_MATCH(TBL_VTY_FLD_AUTH_METHOD, field_id))
    {
        sal_sprintf(str, "%s", p_vty->auth_method);
    }
    if (FLD_MATCH(TBL_VTY_FLD_INUSE, field_id))
    {
        sal_sprintf(str, "%u", p_vty->inuse);
    }
    if (FLD_MATCH(TBL_VTY_FLD_AUTHOR_METHOD, field_id))
    {
        sal_sprintf(str, "%s", p_vty->author_method);
    }
    if (FLD_MATCH(TBL_VTY_FLD_ACCOUNT_METHOD, field_id))
    {
        sal_sprintf(str, "%s", p_vty->account_method);
    }
    if (FLD_MATCH(TBL_VTY_FLD_ACCOUNTCMD_METHOD, field_id))
    {
        sal_sprintf(str, "%s", p_vty->accountcmd_method);
    }
    if (FLD_MATCH(TBL_VTY_FLD_IPV4_ACL, field_id))
    {
        sal_sprintf(str, "%s", p_vty->ipv4_acl);
    }
    if (FLD_MATCH(TBL_VTY_FLD_IPV6_ACL, field_id))
    {
        sal_sprintf(str, "%s", p_vty->ipv6_acl);
    }
    if (FLD_MATCH(TBL_VTY_FLD_AAA_PRIVILEGE, field_id))
    {
        sal_sprintf(str, "%u", p_vty->aaa_privilege);
    }
    if (FLD_MATCH(TBL_VTY_FLD_CONFIG_PRIVILEGE, field_id))
    {
        sal_sprintf(str, "%u", p_vty->config_privilege);
    }
    if (FLD_MATCH(TBL_VTY_FLD_AAA_START_TIME, field_id))
    {
        sal_sprintf(str, "%u", p_vty->aaa_start_time);
    }
    if (FLD_MATCH(TBL_VTY_FLD_CMD_BUF, field_id))
    {
        cdb_uint8_array_val2str(str, MAX_CMD_STR_LEN, p_vty->cmd_buf, sizeof(p_vty->cmd_buf));
    }
    if (FLD_MATCH(TBL_VTY_FLD_OLD_CMD_BUF, field_id))
    {
        cdb_uint8_array_val2str(str, MAX_CMD_STR_LEN, p_vty->old_cmd_buf, sizeof(p_vty->old_cmd_buf));
    }
    if (FLD_MATCH(TBL_VTY_FLD_ACCT_STOP_IPADDR, field_id))
    {
        sal_sprintf(str, "%s", p_vty->acct_stop_ipaddr);
    }
    if (FLD_MATCH(TBL_VTY_FLD_ACCT_STOP_PID, field_id))
    {
        sal_sprintf(str, "%u", p_vty->acct_stop_pid);
    }
    if (FLD_MATCH(TBL_VTY_FLD_ACCT_STOP_PRIVILEGE, field_id))
    {
        sal_sprintf(str, "%u", p_vty->acct_stop_privilege);
    }
    if (FLD_MATCH(TBL_VTY_FLD_IS_NOT_SSH_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_vty->is_not_ssh_key);
    }
    if (FLD_MATCH(TBL_VTY_FLD_IS_SSH_LOGIN, field_id))
    {
        sal_sprintf(str, "%u", p_vty->is_ssh_login);
    }
    return str;
}

char**
tbl_vty_table_dump(tbl_vty_t *p_vty, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_vty_key_name_dump(p_vty, buf));
    for(i=1; i<TBL_VTY_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_vty_field_name_dump(p_vty, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_vty_field_value_dump(p_vty, i, buf));
    }
    return str;
}

int32
tbl_vty_field_value_parser(char *str, int32 field_id, tbl_vty_t *p_vty)
{
    if (FLD_MATCH(TBL_VTY_FLD_KEY, field_id))
    {
        int32 ret;
        p_vty->key.id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VTY_FLD_TIMEOUT_MIN, field_id))
    {
        int32 ret;
        p_vty->timeout_min = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VTY_FLD_TIMEOUT_SEC, field_id))
    {
        int32 ret;
        p_vty->timeout_sec = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VTY_FLD_PROTOCOL, field_id))
    {
        sal_strcpy(p_vty->protocol, str);
    }
    if (FLD_MATCH(TBL_VTY_FLD_LOGIN, field_id))
    {
        cdb_enum_str2val(login_method_strs, LOGIN_METHOD_MAX, str);
    }
    if (FLD_MATCH(TBL_VTY_FLD_PRIVILEGE, field_id))
    {
        int32 ret;
        p_vty->privilege = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VTY_FLD_ENC_PASSWD, field_id))
    {
        sal_strcpy(p_vty->enc_passwd, str);
    }
    if (FLD_MATCH(TBL_VTY_FLD_PASSWD, field_id))
    {
        sal_strcpy(p_vty->passwd, str);
    }
    if (FLD_MATCH(TBL_VTY_FLD_AUTH_METHOD, field_id))
    {
        sal_strcpy(p_vty->auth_method, str);
    }
    if (FLD_MATCH(TBL_VTY_FLD_INUSE, field_id))
    {
        int32 ret;
        p_vty->inuse = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VTY_FLD_AUTHOR_METHOD, field_id))
    {
        sal_strcpy(p_vty->author_method, str);
    }
    if (FLD_MATCH(TBL_VTY_FLD_ACCOUNT_METHOD, field_id))
    {
        sal_strcpy(p_vty->account_method, str);
    }
    if (FLD_MATCH(TBL_VTY_FLD_ACCOUNTCMD_METHOD, field_id))
    {
        sal_strcpy(p_vty->accountcmd_method, str);
    }
    if (FLD_MATCH(TBL_VTY_FLD_IPV4_ACL, field_id))
    {
        sal_strcpy(p_vty->ipv4_acl, str);
    }
    if (FLD_MATCH(TBL_VTY_FLD_IPV6_ACL, field_id))
    {
        sal_strcpy(p_vty->ipv6_acl, str);
    }
    if (FLD_MATCH(TBL_VTY_FLD_AAA_PRIVILEGE, field_id))
    {
        int32 ret;
        p_vty->aaa_privilege = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VTY_FLD_CONFIG_PRIVILEGE, field_id))
    {
        int32 ret;
        p_vty->config_privilege = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VTY_FLD_AAA_START_TIME, field_id))
    {
        int32 ret;
        p_vty->aaa_start_time = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VTY_FLD_CMD_BUF, field_id))
    {
        cdb_uint8_array_str2val(str, p_vty->cmd_buf);
    }
    if (FLD_MATCH(TBL_VTY_FLD_OLD_CMD_BUF, field_id))
    {
        cdb_uint8_array_str2val(str, p_vty->old_cmd_buf);
    }
    if (FLD_MATCH(TBL_VTY_FLD_ACCT_STOP_IPADDR, field_id))
    {
        sal_strcpy(p_vty->acct_stop_ipaddr, str);
    }
    if (FLD_MATCH(TBL_VTY_FLD_ACCT_STOP_PID, field_id))
    {
        int32 ret;
        p_vty->acct_stop_pid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VTY_FLD_ACCT_STOP_PRIVILEGE, field_id))
    {
        int32 ret;
        p_vty->acct_stop_privilege = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VTY_FLD_IS_NOT_SSH_KEY, field_id))
    {
        int32 ret;
        p_vty->is_not_ssh_key = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VTY_FLD_IS_SSH_LOGIN, field_id))
    {
        int32 ret;
        p_vty->is_ssh_login = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_vty_table_parser(char** array, char* key_value,tbl_vty_t *p_vty)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_VTY);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_vty_key_str2val(key_value, p_vty));

    for(i=1; i<TBL_VTY_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_vty_field_value_parser( array[j++], i, p_vty));
    }

    return PM_E_NONE;
}

int32
tbl_vty_dump_one(tbl_vty_t *p_vty, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_VTY);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_VTY_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VTY_FLD_KEY].name,
            p_vty->key.id);
    }
    if (FLD_MATCH(TBL_VTY_FLD_TIMEOUT_MIN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VTY_FLD_TIMEOUT_MIN].name,
            p_vty->timeout_min);
    }
    if (FLD_MATCH(TBL_VTY_FLD_TIMEOUT_SEC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VTY_FLD_TIMEOUT_SEC].name,
            p_vty->timeout_sec);
    }
    if (FLD_MATCH(TBL_VTY_FLD_PROTOCOL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VTY_FLD_PROTOCOL].name,
            p_vty->protocol);
    }
    if (FLD_MATCH(TBL_VTY_FLD_LOGIN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VTY_FLD_LOGIN].name, 
            cdb_enum_val2str(login_method_strs, LOGIN_METHOD_MAX, p_vty->login));
    }
    if (FLD_MATCH(TBL_VTY_FLD_PRIVILEGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VTY_FLD_PRIVILEGE].name,
            p_vty->privilege);
    }
    if (FLD_MATCH(TBL_VTY_FLD_ENC_PASSWD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VTY_FLD_ENC_PASSWD].name,
            p_vty->enc_passwd);
    }
    if (FLD_MATCH(TBL_VTY_FLD_PASSWD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VTY_FLD_PASSWD].name,
            p_vty->passwd);
    }
    if (FLD_MATCH(TBL_VTY_FLD_AUTH_METHOD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VTY_FLD_AUTH_METHOD].name,
            p_vty->auth_method);
    }
    if (FLD_MATCH(TBL_VTY_FLD_INUSE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VTY_FLD_INUSE].name,
            p_vty->inuse);
    }
    if (FLD_MATCH(TBL_VTY_FLD_AUTHOR_METHOD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VTY_FLD_AUTHOR_METHOD].name,
            p_vty->author_method);
    }
    if (FLD_MATCH(TBL_VTY_FLD_ACCOUNT_METHOD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VTY_FLD_ACCOUNT_METHOD].name,
            p_vty->account_method);
    }
    if (FLD_MATCH(TBL_VTY_FLD_ACCOUNTCMD_METHOD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VTY_FLD_ACCOUNTCMD_METHOD].name,
            p_vty->accountcmd_method);
    }
    if (FLD_MATCH(TBL_VTY_FLD_IPV4_ACL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VTY_FLD_IPV4_ACL].name,
            p_vty->ipv4_acl);
    }
    if (FLD_MATCH(TBL_VTY_FLD_IPV6_ACL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VTY_FLD_IPV6_ACL].name,
            p_vty->ipv6_acl);
    }
    if (FLD_MATCH(TBL_VTY_FLD_AAA_PRIVILEGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VTY_FLD_AAA_PRIVILEGE].name,
            p_vty->aaa_privilege);
    }
    if (FLD_MATCH(TBL_VTY_FLD_CONFIG_PRIVILEGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VTY_FLD_CONFIG_PRIVILEGE].name,
            p_vty->config_privilege);
    }
    if (FLD_MATCH(TBL_VTY_FLD_AAA_START_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VTY_FLD_AAA_START_TIME].name,
            p_vty->aaa_start_time);
    }
    if (FLD_MATCH(TBL_VTY_FLD_CMD_BUF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VTY_FLD_CMD_BUF].name, 
            cdb_uint8_array_val2str(buf, MAX_CMD_STR_LEN, p_vty->cmd_buf, sizeof(p_vty->cmd_buf)));
    }
    if (FLD_MATCH(TBL_VTY_FLD_OLD_CMD_BUF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VTY_FLD_OLD_CMD_BUF].name, 
            cdb_uint8_array_val2str(buf, MAX_CMD_STR_LEN, p_vty->old_cmd_buf, sizeof(p_vty->old_cmd_buf)));
    }
    if (FLD_MATCH(TBL_VTY_FLD_ACCT_STOP_IPADDR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VTY_FLD_ACCT_STOP_IPADDR].name,
            p_vty->acct_stop_ipaddr);
    }
    if (FLD_MATCH(TBL_VTY_FLD_ACCT_STOP_PID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VTY_FLD_ACCT_STOP_PID].name,
            p_vty->acct_stop_pid);
    }
    if (FLD_MATCH(TBL_VTY_FLD_ACCT_STOP_PRIVILEGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VTY_FLD_ACCT_STOP_PRIVILEGE].name,
            p_vty->acct_stop_privilege);
    }
    if (FLD_MATCH(TBL_VTY_FLD_IS_NOT_SSH_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VTY_FLD_IS_NOT_SSH_KEY].name,
            p_vty->is_not_ssh_key);
    }
    if (FLD_MATCH(TBL_VTY_FLD_IS_SSH_LOGIN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VTY_FLD_IS_SSH_LOGIN].name,
            p_vty->is_ssh_login);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_vty_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_vty_master_t *p_master = _g_p_tbl_vty_master;
    tbl_vty_t *p_db_vty = NULL;
    uint32 id = 0;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    for (id = 0; id < GLB_MAX_VTY_NUM; id++)
    {
        p_db_vty = p_master->vty_array[id];
        if (NULL == p_db_vty)
        {
            continue;
        }
        rc |= fn(p_db_vty, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_vty_master_t*
tbl_vty_get_master()
{
    return _g_p_tbl_vty_master;
}

tbl_vty_master_t*
tbl_vty_init_vty()
{
    _g_p_tbl_vty_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_vty_master_t));
    return _g_p_tbl_vty_master;
}

