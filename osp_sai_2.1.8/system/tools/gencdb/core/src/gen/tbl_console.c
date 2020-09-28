
#include "proto.h"
#include "gen/tbl_console_define.h"
#include "gen/tbl_console.h"
#include "cdb_data_cmp.h"

tbl_console_t *_g_p_tbl_console = NULL;

int32
tbl_console_set_console_field_sync(tbl_console_t *p_console, tbl_console_field_id_t field_id, uint32 sync)
{
    tbl_console_t *p_db_console = _g_p_tbl_console;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_CONSOLE_FLD_TIMEOUT_MIN:
        p_db_console->timeout_min = p_console->timeout_min;
        break;

    case TBL_CONSOLE_FLD_TIMEOUT_SEC:
        p_db_console->timeout_sec = p_console->timeout_sec;
        break;

    case TBL_CONSOLE_FLD_LOGIN:
        p_db_console->login = p_console->login;
        break;

    case TBL_CONSOLE_FLD_PRIVILEGE:
        p_db_console->privilege = p_console->privilege;
        break;

    case TBL_CONSOLE_FLD_ENC_PASSWD:
        sal_memcpy(p_db_console->enc_passwd, p_console->enc_passwd, sizeof(p_console->enc_passwd));
        break;

    case TBL_CONSOLE_FLD_PASSWD:
        sal_memcpy(p_db_console->passwd, p_console->passwd, sizeof(p_console->passwd));
        break;

    case TBL_CONSOLE_FLD_AUTH_METHOD:
        sal_memcpy(p_db_console->auth_method, p_console->auth_method, sizeof(p_console->auth_method));
        break;

    case TBL_CONSOLE_FLD_STOPBITS:
        p_db_console->stopbits = p_console->stopbits;
        break;

    case TBL_CONSOLE_FLD_PARITY:
        p_db_console->parity = p_console->parity;
        break;

    case TBL_CONSOLE_FLD_DATABITS:
        p_db_console->databits = p_console->databits;
        break;

    case TBL_CONSOLE_FLD_BAUDRATE:
        p_db_console->baudrate = p_console->baudrate;
        break;

    case TBL_CONSOLE_FLD_INUSE:
        p_db_console->inuse = p_console->inuse;
        break;

    case TBL_CONSOLE_FLD_MAX:
        sal_memcpy(p_db_console, p_console, sizeof(tbl_console_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_CONSOLE, field_id, p_db_console);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_CONSOLE, field_id, p_db_console);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_console_set_console_field(tbl_console_t *p_console, tbl_console_field_id_t field_id)
{
    return tbl_console_set_console_field_sync(p_console, field_id, TRUE);
}

tbl_console_t*
tbl_console_get_console()
{
    return _g_p_tbl_console;
}

int32
tbl_console_key_str2val(char *str, tbl_console_t *p_console)
{
    return PM_E_NONE;
}

char*
tbl_console_key_name_dump(tbl_console_t *p_console, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CONSOLE);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_console_key_value_dump(tbl_console_t *p_console, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_console_field_name_dump(tbl_console_t *p_console, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_CONSOLE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_CONSOLE_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_console_field_value_dump(tbl_console_t *p_console, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_CONSOLE_FLD_TIMEOUT_MIN, field_id))
    {
        sal_sprintf(str, "%u", p_console->timeout_min);
    }
    if (FLD_MATCH(TBL_CONSOLE_FLD_TIMEOUT_SEC, field_id))
    {
        sal_sprintf(str, "%u", p_console->timeout_sec);
    }
    if (FLD_MATCH(TBL_CONSOLE_FLD_LOGIN, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(login_method_strs, LOGIN_METHOD_MAX, p_console->login));
    }
    if (FLD_MATCH(TBL_CONSOLE_FLD_PRIVILEGE, field_id))
    {
        sal_sprintf(str, "%u", p_console->privilege);
    }
    if (FLD_MATCH(TBL_CONSOLE_FLD_ENC_PASSWD, field_id))
    {
        sal_sprintf(str, "%s", p_console->enc_passwd);
    }
    if (FLD_MATCH(TBL_CONSOLE_FLD_PASSWD, field_id))
    {
        sal_sprintf(str, "%s", p_console->passwd);
    }
    if (FLD_MATCH(TBL_CONSOLE_FLD_AUTH_METHOD, field_id))
    {
        sal_sprintf(str, "%s", p_console->auth_method);
    }
    if (FLD_MATCH(TBL_CONSOLE_FLD_STOPBITS, field_id))
    {
        sal_sprintf(str, "%u", p_console->stopbits);
    }
    if (FLD_MATCH(TBL_CONSOLE_FLD_PARITY, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(parity_strs, PARITY_MAX, p_console->parity));
    }
    if (FLD_MATCH(TBL_CONSOLE_FLD_DATABITS, field_id))
    {
        sal_sprintf(str, "%u", p_console->databits);
    }
    if (FLD_MATCH(TBL_CONSOLE_FLD_BAUDRATE, field_id))
    {
        sal_sprintf(str, "%u", p_console->baudrate);
    }
    if (FLD_MATCH(TBL_CONSOLE_FLD_INUSE, field_id))
    {
        sal_sprintf(str, "%u", p_console->inuse);
    }
    return str;
}

char**
tbl_console_table_dump(tbl_console_t *p_console, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_console_key_name_dump(p_console, buf));
    for(i=1; i<TBL_CONSOLE_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_console_field_name_dump(p_console, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_console_field_value_dump(p_console, i, buf));
    }
    return str;
}

int32
tbl_console_field_value_parser(char *str, int32 field_id, tbl_console_t *p_console)
{
    if (FLD_MATCH(TBL_CONSOLE_FLD_TIMEOUT_MIN, field_id))
    {
        int32 ret;
        p_console->timeout_min = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CONSOLE_FLD_TIMEOUT_SEC, field_id))
    {
        int32 ret;
        p_console->timeout_sec = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CONSOLE_FLD_LOGIN, field_id))
    {
        cdb_enum_str2val(login_method_strs, LOGIN_METHOD_MAX, str);
    }
    if (FLD_MATCH(TBL_CONSOLE_FLD_PRIVILEGE, field_id))
    {
        int32 ret;
        p_console->privilege = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CONSOLE_FLD_ENC_PASSWD, field_id))
    {
        sal_strcpy(p_console->enc_passwd, str);
    }
    if (FLD_MATCH(TBL_CONSOLE_FLD_PASSWD, field_id))
    {
        sal_strcpy(p_console->passwd, str);
    }
    if (FLD_MATCH(TBL_CONSOLE_FLD_AUTH_METHOD, field_id))
    {
        sal_strcpy(p_console->auth_method, str);
    }
    if (FLD_MATCH(TBL_CONSOLE_FLD_STOPBITS, field_id))
    {
        int32 ret;
        p_console->stopbits = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CONSOLE_FLD_PARITY, field_id))
    {
        cdb_enum_str2val(parity_strs, PARITY_MAX, str);
    }
    if (FLD_MATCH(TBL_CONSOLE_FLD_DATABITS, field_id))
    {
        int32 ret;
        p_console->databits = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CONSOLE_FLD_BAUDRATE, field_id))
    {
        int32 ret;
        p_console->baudrate = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CONSOLE_FLD_INUSE, field_id))
    {
        int32 ret;
        p_console->inuse = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_console_table_parser(char** array, char* key_value,tbl_console_t *p_console)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_CONSOLE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_console_key_str2val(key_value, p_console));

    for(i=1; i<TBL_CONSOLE_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_console_field_value_parser( array[j++], i, p_console));
    }

    return PM_E_NONE;
}

int32
tbl_console_dump_one(tbl_console_t *p_console, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CONSOLE);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_CONSOLE_FLD_TIMEOUT_MIN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CONSOLE_FLD_TIMEOUT_MIN].name,
            p_console->timeout_min);
    }
    if (FLD_MATCH(TBL_CONSOLE_FLD_TIMEOUT_SEC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CONSOLE_FLD_TIMEOUT_SEC].name,
            p_console->timeout_sec);
    }
    if (FLD_MATCH(TBL_CONSOLE_FLD_LOGIN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CONSOLE_FLD_LOGIN].name, 
            cdb_enum_val2str(login_method_strs, LOGIN_METHOD_MAX, p_console->login));
    }
    if (FLD_MATCH(TBL_CONSOLE_FLD_PRIVILEGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CONSOLE_FLD_PRIVILEGE].name,
            p_console->privilege);
    }
    if (FLD_MATCH(TBL_CONSOLE_FLD_ENC_PASSWD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CONSOLE_FLD_ENC_PASSWD].name,
            p_console->enc_passwd);
    }
    if (FLD_MATCH(TBL_CONSOLE_FLD_PASSWD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CONSOLE_FLD_PASSWD].name,
            p_console->passwd);
    }
    if (FLD_MATCH(TBL_CONSOLE_FLD_AUTH_METHOD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CONSOLE_FLD_AUTH_METHOD].name,
            p_console->auth_method);
    }
    if (FLD_MATCH(TBL_CONSOLE_FLD_STOPBITS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CONSOLE_FLD_STOPBITS].name,
            p_console->stopbits);
    }
    if (FLD_MATCH(TBL_CONSOLE_FLD_PARITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CONSOLE_FLD_PARITY].name, 
            cdb_enum_val2str(parity_strs, PARITY_MAX, p_console->parity));
    }
    if (FLD_MATCH(TBL_CONSOLE_FLD_DATABITS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CONSOLE_FLD_DATABITS].name,
            p_console->databits);
    }
    if (FLD_MATCH(TBL_CONSOLE_FLD_BAUDRATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CONSOLE_FLD_BAUDRATE].name,
            p_console->baudrate);
    }
    if (FLD_MATCH(TBL_CONSOLE_FLD_INUSE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CONSOLE_FLD_INUSE].name,
            p_console->inuse);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_console_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_console_t *p_db_console = _g_p_tbl_console;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_console)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_console, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_console_t*
tbl_console_init_console()
{
    _g_p_tbl_console = XCALLOC(MEM_TBL_CONSOLE, sizeof(tbl_console_t));
    return _g_p_tbl_console;
}

