
#include "proto.h"
#include "gen/tbl_auth_cfg_define.h"
#include "gen/tbl_auth_cfg.h"
#include "cdb_data_cmp.h"

tbl_auth_cfg_t *_g_p_tbl_auth_cfg = NULL;

int32
tbl_auth_cfg_set_auth_cfg_field_sync(tbl_auth_cfg_t *p_auth_cfg, tbl_auth_cfg_field_id_t field_id, uint32 sync)
{
    tbl_auth_cfg_t *p_db_auth_cfg = _g_p_tbl_auth_cfg;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_AUTH_CFG_FLD_SECRET:
        sal_memcpy(p_db_auth_cfg->secret, p_auth_cfg->secret, sizeof(p_auth_cfg->secret));
        break;

    case TBL_AUTH_CFG_FLD_ENC_SECRET:
        sal_memcpy(p_db_auth_cfg->enc_secret, p_auth_cfg->enc_secret, sizeof(p_auth_cfg->enc_secret));
        break;

    case TBL_AUTH_CFG_FLD_TIMEOUT:
        p_db_auth_cfg->timeout = p_auth_cfg->timeout;
        break;

    case TBL_AUTH_CFG_FLD_RETRIES:
        p_db_auth_cfg->retries = p_auth_cfg->retries;
        break;

    case TBL_AUTH_CFG_FLD_DEADTIME:
        p_db_auth_cfg->deadtime = p_auth_cfg->deadtime;
        break;

    case TBL_AUTH_CFG_FLD_MAX:
        sal_memcpy(p_db_auth_cfg, p_auth_cfg, sizeof(tbl_auth_cfg_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_AUTH_CFG, field_id, p_db_auth_cfg);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_AUTH_CFG, field_id, p_db_auth_cfg);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_auth_cfg_set_auth_cfg_field(tbl_auth_cfg_t *p_auth_cfg, tbl_auth_cfg_field_id_t field_id)
{
    return tbl_auth_cfg_set_auth_cfg_field_sync(p_auth_cfg, field_id, TRUE);
}

tbl_auth_cfg_t*
tbl_auth_cfg_get_auth_cfg()
{
    return _g_p_tbl_auth_cfg;
}

int32
tbl_auth_cfg_key_str2val(char *str, tbl_auth_cfg_t *p_auth_cfg)
{
    return PM_E_NONE;
}

char*
tbl_auth_cfg_key_name_dump(tbl_auth_cfg_t *p_auth_cfg, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_AUTH_CFG);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_auth_cfg_key_value_dump(tbl_auth_cfg_t *p_auth_cfg, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_auth_cfg_field_name_dump(tbl_auth_cfg_t *p_auth_cfg, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_AUTH_CFG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_AUTH_CFG_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_auth_cfg_field_value_dump(tbl_auth_cfg_t *p_auth_cfg, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_AUTH_CFG_FLD_SECRET, field_id))
    {
        sal_sprintf(str, "%s", p_auth_cfg->secret);
    }
    if (FLD_MATCH(TBL_AUTH_CFG_FLD_ENC_SECRET, field_id))
    {
        sal_sprintf(str, "%s", p_auth_cfg->enc_secret);
    }
    if (FLD_MATCH(TBL_AUTH_CFG_FLD_TIMEOUT, field_id))
    {
        sal_sprintf(str, "%u", p_auth_cfg->timeout);
    }
    if (FLD_MATCH(TBL_AUTH_CFG_FLD_RETRIES, field_id))
    {
        sal_sprintf(str, "%u", p_auth_cfg->retries);
    }
    if (FLD_MATCH(TBL_AUTH_CFG_FLD_DEADTIME, field_id))
    {
        sal_sprintf(str, "%u", p_auth_cfg->deadtime);
    }
    return str;
}

char**
tbl_auth_cfg_table_dump(tbl_auth_cfg_t *p_auth_cfg, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_auth_cfg_key_name_dump(p_auth_cfg, buf));
    for(i=1; i<TBL_AUTH_CFG_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_auth_cfg_field_name_dump(p_auth_cfg, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_auth_cfg_field_value_dump(p_auth_cfg, i, buf));
    }
    return str;
}

int32
tbl_auth_cfg_field_value_parser(char *str, int32 field_id, tbl_auth_cfg_t *p_auth_cfg)
{
    if (FLD_MATCH(TBL_AUTH_CFG_FLD_SECRET, field_id))
    {
        sal_strcpy(p_auth_cfg->secret, str);
    }
    if (FLD_MATCH(TBL_AUTH_CFG_FLD_ENC_SECRET, field_id))
    {
        sal_strcpy(p_auth_cfg->enc_secret, str);
    }
    if (FLD_MATCH(TBL_AUTH_CFG_FLD_TIMEOUT, field_id))
    {
        int32 ret;
        p_auth_cfg->timeout = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_AUTH_CFG_FLD_RETRIES, field_id))
    {
        int32 ret;
        p_auth_cfg->retries = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_AUTH_CFG_FLD_DEADTIME, field_id))
    {
        int32 ret;
        p_auth_cfg->deadtime = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_auth_cfg_table_parser(char** array, char* key_value,tbl_auth_cfg_t *p_auth_cfg)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_AUTH_CFG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_auth_cfg_key_str2val(key_value, p_auth_cfg));

    for(i=1; i<TBL_AUTH_CFG_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_auth_cfg_field_value_parser( array[j++], i, p_auth_cfg));
    }

    return PM_E_NONE;
}

int32
tbl_auth_cfg_dump_one(tbl_auth_cfg_t *p_auth_cfg, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_AUTH_CFG);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_AUTH_CFG_FLD_SECRET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_AUTH_CFG_FLD_SECRET].name,
            p_auth_cfg->secret);
    }
    if (FLD_MATCH(TBL_AUTH_CFG_FLD_ENC_SECRET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_AUTH_CFG_FLD_ENC_SECRET].name,
            p_auth_cfg->enc_secret);
    }
    if (FLD_MATCH(TBL_AUTH_CFG_FLD_TIMEOUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_AUTH_CFG_FLD_TIMEOUT].name,
            p_auth_cfg->timeout);
    }
    if (FLD_MATCH(TBL_AUTH_CFG_FLD_RETRIES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_AUTH_CFG_FLD_RETRIES].name,
            p_auth_cfg->retries);
    }
    if (FLD_MATCH(TBL_AUTH_CFG_FLD_DEADTIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_AUTH_CFG_FLD_DEADTIME].name,
            p_auth_cfg->deadtime);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_auth_cfg_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_auth_cfg_t *p_db_auth_cfg = _g_p_tbl_auth_cfg;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_auth_cfg)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_auth_cfg, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_auth_cfg_t*
tbl_auth_cfg_init_auth_cfg()
{
    _g_p_tbl_auth_cfg = XCALLOC(MEM_TBL_AUTH_CFG, sizeof(tbl_auth_cfg_t));
    return _g_p_tbl_auth_cfg;
}

