
#include "proto.h"
#include "gen/tbl_fea_global_define.h"
#include "gen/tbl_fea_global.h"
#include "cdb_data_cmp.h"

tbl_fea_global_t *_g_p_tbl_fea_global = NULL;

int32
tbl_fea_global_set_fea_global_field_sync(tbl_fea_global_t *p_fea_global, tbl_fea_global_field_id_t field_id, uint32 sync)
{
    tbl_fea_global_t *p_db_fea_global = _g_p_tbl_fea_global;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_FEA_GLOBAL_FLD_ROUTER_ID:
        p_db_fea_global->router_id = p_fea_global->router_id;
        break;

    case TBL_FEA_GLOBAL_FLD_PORT_STATS_MS:
        p_db_fea_global->port_stats_ms = p_fea_global->port_stats_ms;
        break;

    case TBL_FEA_GLOBAL_FLD_MAX:
        sal_memcpy(p_db_fea_global, p_fea_global, sizeof(tbl_fea_global_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_FEA_GLOBAL, field_id, p_db_fea_global);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_FEA_GLOBAL, field_id, p_db_fea_global);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_fea_global_set_fea_global_field(tbl_fea_global_t *p_fea_global, tbl_fea_global_field_id_t field_id)
{
    return tbl_fea_global_set_fea_global_field_sync(p_fea_global, field_id, TRUE);
}

tbl_fea_global_t*
tbl_fea_global_get_fea_global()
{
    return _g_p_tbl_fea_global;
}

int32
tbl_fea_global_key_str2val(char *str, tbl_fea_global_t *p_fea_global)
{
    return PM_E_NONE;
}

char*
tbl_fea_global_key_name_dump(tbl_fea_global_t *p_fea_global, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_GLOBAL);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_fea_global_key_value_dump(tbl_fea_global_t *p_fea_global, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_fea_global_field_name_dump(tbl_fea_global_t *p_fea_global, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_FEA_GLOBAL_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_fea_global_field_value_dump(tbl_fea_global_t *p_fea_global, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_FEA_GLOBAL_FLD_ROUTER_ID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_fea_global->router_id);
    }
    if (FLD_MATCH(TBL_FEA_GLOBAL_FLD_PORT_STATS_MS, field_id))
    {
        sal_sprintf(str, "%u", p_fea_global->port_stats_ms);
    }
    return str;
}

char**
tbl_fea_global_table_dump(tbl_fea_global_t *p_fea_global, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_fea_global_key_name_dump(p_fea_global, buf));
    for(i=1; i<TBL_FEA_GLOBAL_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_fea_global_field_name_dump(p_fea_global, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_fea_global_field_value_dump(p_fea_global, i, buf));
    }
    return str;
}

int32
tbl_fea_global_field_value_parser(char *str, int32 field_id, tbl_fea_global_t *p_fea_global)
{
    if (FLD_MATCH(TBL_FEA_GLOBAL_FLD_ROUTER_ID, field_id))
    {
        int32 ret;
        p_fea_global->router_id = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_GLOBAL_FLD_PORT_STATS_MS, field_id))
    {
        int32 ret;
        p_fea_global->port_stats_ms = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_fea_global_table_parser(char** array, char* key_value,tbl_fea_global_t *p_fea_global)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_fea_global_key_str2val(key_value, p_fea_global));

    for(i=1; i<TBL_FEA_GLOBAL_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_fea_global_field_value_parser( array[j++], i, p_fea_global));
    }

    return PM_E_NONE;
}

int32
tbl_fea_global_dump_one(tbl_fea_global_t *p_fea_global, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_GLOBAL);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_FEA_GLOBAL_FLD_ROUTER_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_FEA_GLOBAL_FLD_ROUTER_ID].name,
            p_fea_global->router_id);
    }
    if (FLD_MATCH(TBL_FEA_GLOBAL_FLD_PORT_STATS_MS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_GLOBAL_FLD_PORT_STATS_MS].name,
            p_fea_global->port_stats_ms);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_fea_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_fea_global_t *p_db_fea_global = _g_p_tbl_fea_global;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_fea_global)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_fea_global, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_fea_global_t*
tbl_fea_global_init_fea_global()
{
    _g_p_tbl_fea_global = XCALLOC(MEM_TBL_FEA_GLOBAL, sizeof(tbl_fea_global_t));
    return _g_p_tbl_fea_global;
}

