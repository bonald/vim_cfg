
#include "proto.h"
#include "gen/tbl_copp_cfg_define.h"
#include "gen/tbl_copp_cfg.h"
#include "cdb_data_cmp.h"

tbl_copp_cfg_t *_g_p_tbl_copp_cfg = NULL;

int32
tbl_copp_cfg_set_copp_cfg_field_sync(tbl_copp_cfg_t *p_copp_cfg, tbl_copp_cfg_field_id_t field_id, uint32 sync)
{
    tbl_copp_cfg_t *p_db_copp_cfg = _g_p_tbl_copp_cfg;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_COPP_CFG_FLD_COPP_PNAME:
        sal_memcpy(p_db_copp_cfg->copp_pname, p_copp_cfg->copp_pname, sizeof(p_copp_cfg->copp_pname));
        break;

    case TBL_COPP_CFG_FLD_COPP_IFIDX:
        p_db_copp_cfg->copp_ifidx = p_copp_cfg->copp_ifidx;
        break;

    case TBL_COPP_CFG_FLD_COPP_TOTAL_RATE:
        p_db_copp_cfg->copp_total_rate = p_copp_cfg->copp_total_rate;
        break;

    case TBL_COPP_CFG_FLD_MAX:
        sal_memcpy(p_db_copp_cfg, p_copp_cfg, sizeof(tbl_copp_cfg_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_COPP_CFG, field_id, p_db_copp_cfg);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_COPP_CFG, field_id, p_db_copp_cfg);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_copp_cfg_set_copp_cfg_field(tbl_copp_cfg_t *p_copp_cfg, tbl_copp_cfg_field_id_t field_id)
{
    return tbl_copp_cfg_set_copp_cfg_field_sync(p_copp_cfg, field_id, TRUE);
}

tbl_copp_cfg_t*
tbl_copp_cfg_get_copp_cfg()
{
    return _g_p_tbl_copp_cfg;
}

int32
tbl_copp_cfg_key_str2val(char *str, tbl_copp_cfg_t *p_copp_cfg)
{
    return PM_E_NONE;
}

char*
tbl_copp_cfg_key_name_dump(tbl_copp_cfg_t *p_copp_cfg, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_COPP_CFG);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_copp_cfg_key_value_dump(tbl_copp_cfg_t *p_copp_cfg, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_copp_cfg_field_name_dump(tbl_copp_cfg_t *p_copp_cfg, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_COPP_CFG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_COPP_CFG_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_copp_cfg_field_value_dump(tbl_copp_cfg_t *p_copp_cfg, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_COPP_CFG_FLD_COPP_PNAME, field_id))
    {
        sal_sprintf(str, "%s", p_copp_cfg->copp_pname);
    }
    if (FLD_MATCH(TBL_COPP_CFG_FLD_COPP_IFIDX, field_id))
    {
        sal_sprintf(str, "%u", p_copp_cfg->copp_ifidx);
    }
    if (FLD_MATCH(TBL_COPP_CFG_FLD_COPP_TOTAL_RATE, field_id))
    {
        sal_sprintf(str, "%u", p_copp_cfg->copp_total_rate);
    }
    return str;
}

char**
tbl_copp_cfg_table_dump(tbl_copp_cfg_t *p_copp_cfg, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_copp_cfg_key_name_dump(p_copp_cfg, buf));
    for(i=1; i<TBL_COPP_CFG_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_copp_cfg_field_name_dump(p_copp_cfg, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_copp_cfg_field_value_dump(p_copp_cfg, i, buf));
    }
    return str;
}

int32
tbl_copp_cfg_field_value_parser(char *str, int32 field_id, tbl_copp_cfg_t *p_copp_cfg)
{
    if (FLD_MATCH(TBL_COPP_CFG_FLD_COPP_PNAME, field_id))
    {
        sal_strcpy(p_copp_cfg->copp_pname, str);
    }
    if (FLD_MATCH(TBL_COPP_CFG_FLD_COPP_IFIDX, field_id))
    {
        int32 ret;
        p_copp_cfg->copp_ifidx = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_COPP_CFG_FLD_COPP_TOTAL_RATE, field_id))
    {
        int32 ret;
        p_copp_cfg->copp_total_rate = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_copp_cfg_table_parser(char** array, char* key_value,tbl_copp_cfg_t *p_copp_cfg)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_COPP_CFG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_copp_cfg_key_str2val(key_value, p_copp_cfg));

    for(i=1; i<TBL_COPP_CFG_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_copp_cfg_field_value_parser( array[j++], i, p_copp_cfg));
    }

    return PM_E_NONE;
}

int32
tbl_copp_cfg_dump_one(tbl_copp_cfg_t *p_copp_cfg, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_COPP_CFG);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_COPP_CFG_FLD_COPP_PNAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_COPP_CFG_FLD_COPP_PNAME].name,
            p_copp_cfg->copp_pname);
    }
    if (FLD_MATCH(TBL_COPP_CFG_FLD_COPP_IFIDX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_COPP_CFG_FLD_COPP_IFIDX].name,
            p_copp_cfg->copp_ifidx);
    }
    if (FLD_MATCH(TBL_COPP_CFG_FLD_COPP_TOTAL_RATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_COPP_CFG_FLD_COPP_TOTAL_RATE].name,
            p_copp_cfg->copp_total_rate);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_copp_cfg_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_copp_cfg_t *p_db_copp_cfg = _g_p_tbl_copp_cfg;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_copp_cfg)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_copp_cfg, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_copp_cfg_t*
tbl_copp_cfg_init_copp_cfg()
{
    _g_p_tbl_copp_cfg = XCALLOC(MEM_TBL_COPP_CFG, sizeof(tbl_copp_cfg_t));
    return _g_p_tbl_copp_cfg;
}

