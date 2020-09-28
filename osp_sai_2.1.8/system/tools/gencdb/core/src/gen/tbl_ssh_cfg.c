
#include "proto.h"
#include "gen/tbl_ssh_cfg_define.h"
#include "gen/tbl_ssh_cfg.h"
#include "cdb_data_cmp.h"

tbl_ssh_cfg_t *_g_p_tbl_ssh_cfg = NULL;

int32
tbl_ssh_cfg_set_ssh_cfg_field_sync(tbl_ssh_cfg_t *p_ssh_cfg, tbl_ssh_cfg_field_id_t field_id, uint32 sync)
{
    tbl_ssh_cfg_t *p_db_ssh_cfg = _g_p_tbl_ssh_cfg;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_SSH_CFG_FLD_VERSION:
        p_db_ssh_cfg->version = p_ssh_cfg->version;
        break;

    case TBL_SSH_CFG_FLD_ENABLE:
        p_db_ssh_cfg->enable = p_ssh_cfg->enable;
        break;

    case TBL_SSH_CFG_FLD_AUTH_RETRY:
        p_db_ssh_cfg->auth_retry = p_ssh_cfg->auth_retry;
        break;

    case TBL_SSH_CFG_FLD_AUTH_TIMEOUT:
        p_db_ssh_cfg->auth_timeout = p_ssh_cfg->auth_timeout;
        break;

    case TBL_SSH_CFG_FLD_REKEY_INTERVAL:
        p_db_ssh_cfg->rekey_interval = p_ssh_cfg->rekey_interval;
        break;

    case TBL_SSH_CFG_FLD_AUTH_TYPE:
        p_db_ssh_cfg->auth_type = p_ssh_cfg->auth_type;
        break;

    case TBL_SSH_CFG_FLD_LISTEN_PORT:
        p_db_ssh_cfg->listen_port = p_ssh_cfg->listen_port;
        break;

    case TBL_SSH_CFG_FLD_HOSTKEY:
        sal_memcpy(p_db_ssh_cfg->hostkey, p_ssh_cfg->hostkey, sizeof(p_ssh_cfg->hostkey));
        break;

    case TBL_SSH_CFG_FLD_NETCONF_PORT:
        p_db_ssh_cfg->netconf_port = p_ssh_cfg->netconf_port;
        break;

    case TBL_SSH_CFG_FLD_NETCONF_SERVICE:
        p_db_ssh_cfg->netconf_service = p_ssh_cfg->netconf_service;
        break;

    case TBL_SSH_CFG_FLD_MAX:
        sal_memcpy(p_db_ssh_cfg, p_ssh_cfg, sizeof(tbl_ssh_cfg_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_SSH_CFG, field_id, p_db_ssh_cfg);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_SSH_CFG, field_id, p_db_ssh_cfg);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ssh_cfg_set_ssh_cfg_field(tbl_ssh_cfg_t *p_ssh_cfg, tbl_ssh_cfg_field_id_t field_id)
{
    return tbl_ssh_cfg_set_ssh_cfg_field_sync(p_ssh_cfg, field_id, TRUE);
}

tbl_ssh_cfg_t*
tbl_ssh_cfg_get_ssh_cfg()
{
    return _g_p_tbl_ssh_cfg;
}

int32
tbl_ssh_cfg_key_str2val(char *str, tbl_ssh_cfg_t *p_ssh_cfg)
{
    return PM_E_NONE;
}

char*
tbl_ssh_cfg_key_name_dump(tbl_ssh_cfg_t *p_ssh_cfg, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SSH_CFG);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_ssh_cfg_key_value_dump(tbl_ssh_cfg_t *p_ssh_cfg, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_ssh_cfg_field_name_dump(tbl_ssh_cfg_t *p_ssh_cfg, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_SSH_CFG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_SSH_CFG_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_ssh_cfg_field_value_dump(tbl_ssh_cfg_t *p_ssh_cfg, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_SSH_CFG_FLD_VERSION, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(ssh_version_strs, SSH_VERSION_MAX, p_ssh_cfg->version));
    }
    if (FLD_MATCH(TBL_SSH_CFG_FLD_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_ssh_cfg->enable);
    }
    if (FLD_MATCH(TBL_SSH_CFG_FLD_AUTH_RETRY, field_id))
    {
        sal_sprintf(str, "%u", p_ssh_cfg->auth_retry);
    }
    if (FLD_MATCH(TBL_SSH_CFG_FLD_AUTH_TIMEOUT, field_id))
    {
        sal_sprintf(str, "%u", p_ssh_cfg->auth_timeout);
    }
    if (FLD_MATCH(TBL_SSH_CFG_FLD_REKEY_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_ssh_cfg->rekey_interval);
    }
    if (FLD_MATCH(TBL_SSH_CFG_FLD_AUTH_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_ssh_cfg->auth_type);
    }
    if (FLD_MATCH(TBL_SSH_CFG_FLD_LISTEN_PORT, field_id))
    {
        sal_sprintf(str, "%u", p_ssh_cfg->listen_port);
    }
    if (FLD_MATCH(TBL_SSH_CFG_FLD_HOSTKEY, field_id))
    {
        sal_sprintf(str, "%s", p_ssh_cfg->hostkey);
    }
    if (FLD_MATCH(TBL_SSH_CFG_FLD_NETCONF_PORT, field_id))
    {
        sal_sprintf(str, "%u", p_ssh_cfg->netconf_port);
    }
    if (FLD_MATCH(TBL_SSH_CFG_FLD_NETCONF_SERVICE, field_id))
    {
        sal_sprintf(str, "%u", p_ssh_cfg->netconf_service);
    }
    return str;
}

char**
tbl_ssh_cfg_table_dump(tbl_ssh_cfg_t *p_ssh_cfg, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_ssh_cfg_key_name_dump(p_ssh_cfg, buf));
    for(i=1; i<TBL_SSH_CFG_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_ssh_cfg_field_name_dump(p_ssh_cfg, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_ssh_cfg_field_value_dump(p_ssh_cfg, i, buf));
    }
    return str;
}

int32
tbl_ssh_cfg_field_value_parser(char *str, int32 field_id, tbl_ssh_cfg_t *p_ssh_cfg)
{
    if (FLD_MATCH(TBL_SSH_CFG_FLD_VERSION, field_id))
    {
        cdb_enum_str2val(ssh_version_strs, SSH_VERSION_MAX, str);
    }
    if (FLD_MATCH(TBL_SSH_CFG_FLD_ENABLE, field_id))
    {
        int32 ret;
        p_ssh_cfg->enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSH_CFG_FLD_AUTH_RETRY, field_id))
    {
        int32 ret;
        p_ssh_cfg->auth_retry = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSH_CFG_FLD_AUTH_TIMEOUT, field_id))
    {
        int32 ret;
        p_ssh_cfg->auth_timeout = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSH_CFG_FLD_REKEY_INTERVAL, field_id))
    {
        int32 ret;
        p_ssh_cfg->rekey_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSH_CFG_FLD_AUTH_TYPE, field_id))
    {
        int32 ret;
        p_ssh_cfg->auth_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSH_CFG_FLD_LISTEN_PORT, field_id))
    {
        int32 ret;
        p_ssh_cfg->listen_port = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSH_CFG_FLD_HOSTKEY, field_id))
    {
        sal_strcpy(p_ssh_cfg->hostkey, str);
    }
    if (FLD_MATCH(TBL_SSH_CFG_FLD_NETCONF_PORT, field_id))
    {
        int32 ret;
        p_ssh_cfg->netconf_port = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSH_CFG_FLD_NETCONF_SERVICE, field_id))
    {
        int32 ret;
        p_ssh_cfg->netconf_service = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_ssh_cfg_table_parser(char** array, char* key_value,tbl_ssh_cfg_t *p_ssh_cfg)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_SSH_CFG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_ssh_cfg_key_str2val(key_value, p_ssh_cfg));

    for(i=1; i<TBL_SSH_CFG_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_ssh_cfg_field_value_parser( array[j++], i, p_ssh_cfg));
    }

    return PM_E_NONE;
}

int32
tbl_ssh_cfg_dump_one(tbl_ssh_cfg_t *p_ssh_cfg, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SSH_CFG);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_SSH_CFG_FLD_VERSION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SSH_CFG_FLD_VERSION].name, 
            cdb_enum_val2str(ssh_version_strs, SSH_VERSION_MAX, p_ssh_cfg->version));
    }
    if (FLD_MATCH(TBL_SSH_CFG_FLD_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSH_CFG_FLD_ENABLE].name,
            p_ssh_cfg->enable);
    }
    if (FLD_MATCH(TBL_SSH_CFG_FLD_AUTH_RETRY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSH_CFG_FLD_AUTH_RETRY].name,
            p_ssh_cfg->auth_retry);
    }
    if (FLD_MATCH(TBL_SSH_CFG_FLD_AUTH_TIMEOUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSH_CFG_FLD_AUTH_TIMEOUT].name,
            p_ssh_cfg->auth_timeout);
    }
    if (FLD_MATCH(TBL_SSH_CFG_FLD_REKEY_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSH_CFG_FLD_REKEY_INTERVAL].name,
            p_ssh_cfg->rekey_interval);
    }
    if (FLD_MATCH(TBL_SSH_CFG_FLD_AUTH_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSH_CFG_FLD_AUTH_TYPE].name,
            p_ssh_cfg->auth_type);
    }
    if (FLD_MATCH(TBL_SSH_CFG_FLD_LISTEN_PORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSH_CFG_FLD_LISTEN_PORT].name,
            p_ssh_cfg->listen_port);
    }
    if (FLD_MATCH(TBL_SSH_CFG_FLD_HOSTKEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SSH_CFG_FLD_HOSTKEY].name,
            p_ssh_cfg->hostkey);
    }
    if (FLD_MATCH(TBL_SSH_CFG_FLD_NETCONF_PORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSH_CFG_FLD_NETCONF_PORT].name,
            p_ssh_cfg->netconf_port);
    }
    if (FLD_MATCH(TBL_SSH_CFG_FLD_NETCONF_SERVICE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSH_CFG_FLD_NETCONF_SERVICE].name,
            p_ssh_cfg->netconf_service);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_ssh_cfg_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_ssh_cfg_t *p_db_ssh_cfg = _g_p_tbl_ssh_cfg;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_ssh_cfg)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_ssh_cfg, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_ssh_cfg_t*
tbl_ssh_cfg_init_ssh_cfg()
{
    _g_p_tbl_ssh_cfg = XCALLOC(MEM_TBL_SSH_CFG, sizeof(tbl_ssh_cfg_t));
    return _g_p_tbl_ssh_cfg;
}

