
#include "proto.h"
#include "gen/tbl_opm_global_define.h"
#include "gen/tbl_opm_global.h"
#include "cdb_data_cmp.h"

tbl_opm_global_t *_g_p_tbl_opm_global = NULL;

int32
tbl_opm_global_set_opm_global_field_sync(tbl_opm_global_t *p_opmglb, tbl_opm_global_field_id_t field_id, uint32 sync)
{
    tbl_opm_global_t *p_db_opmglb = _g_p_tbl_opm_global;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_OPM_GLOBAL_FLD_ERPS_MODE:
        CDB_FLAG_CHECK_SET(p_db_opmglb, p_opmglb, erps, OPMGLB_FLAG_ERPS_MODE_HUAWEI)
        break;

    case TBL_OPM_GLOBAL_FLD_ERPS_UNRELOAD:
        CDB_FLAG_CHECK_SET(p_db_opmglb, p_opmglb, erps, OPMGLB_FLAG_ERPS_MODE_UNRELOAD)
        break;

    case TBL_OPM_GLOBAL_FLD_MAX:
        sal_memcpy(p_db_opmglb, p_opmglb, sizeof(tbl_opm_global_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_OPM_GLOBAL, field_id, p_db_opmglb);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_OPM_GLOBAL, field_id, p_db_opmglb);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_opm_global_set_opm_global_field(tbl_opm_global_t *p_opmglb, tbl_opm_global_field_id_t field_id)
{
    return tbl_opm_global_set_opm_global_field_sync(p_opmglb, field_id, TRUE);
}

tbl_opm_global_t*
tbl_opm_global_get_opm_global()
{
    return _g_p_tbl_opm_global;
}

int32
tbl_opm_global_key_str2val(char *str, tbl_opm_global_t *p_opmglb)
{
    return PM_E_NONE;
}

char*
tbl_opm_global_key_name_dump(tbl_opm_global_t *p_opmglb, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_OPM_GLOBAL);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_opm_global_key_value_dump(tbl_opm_global_t *p_opmglb, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_opm_global_field_name_dump(tbl_opm_global_t *p_opmglb, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_OPM_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_OPM_GLOBAL_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_opm_global_field_value_dump(tbl_opm_global_t *p_opmglb, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_OPM_GLOBAL_FLD_ERPS_MODE, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_opmglb->erps, OPMGLB_FLAG_ERPS_MODE_HUAWEI));
    }
    if (FLD_MATCH(TBL_OPM_GLOBAL_FLD_ERPS_UNRELOAD, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_opmglb->erps, OPMGLB_FLAG_ERPS_MODE_UNRELOAD));
    }
    return str;
}

char**
tbl_opm_global_table_dump(tbl_opm_global_t *p_opmglb, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_opm_global_key_name_dump(p_opmglb, buf));
    for(i=1; i<TBL_OPM_GLOBAL_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_opm_global_field_name_dump(p_opmglb, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_opm_global_field_value_dump(p_opmglb, i, buf));
    }
    return str;
}

int32
tbl_opm_global_field_value_parser(char *str, int32 field_id, tbl_opm_global_t *p_opmglb)
{
    if (FLD_MATCH(TBL_OPM_GLOBAL_FLD_ERPS_MODE, field_id))
    {
        GLB_SET_FLAG(p_opmglb->erps, OPMGLB_FLAG_ERPS_MODE_HUAWEI);
    }
    if (FLD_MATCH(TBL_OPM_GLOBAL_FLD_ERPS_UNRELOAD, field_id))
    {
        GLB_SET_FLAG(p_opmglb->erps, OPMGLB_FLAG_ERPS_MODE_UNRELOAD);
    }
    return PM_E_NONE;
}

int32
tbl_opm_global_table_parser(char** array, char* key_value,tbl_opm_global_t *p_opmglb)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_OPM_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_opm_global_key_str2val(key_value, p_opmglb));

    for(i=1; i<TBL_OPM_GLOBAL_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_opm_global_field_value_parser( array[j++], i, p_opmglb));
    }

    return PM_E_NONE;
}

int32
tbl_opm_global_dump_one(tbl_opm_global_t *p_opmglb, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_OPM_GLOBAL);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_OPM_GLOBAL_FLD_ERPS_MODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPM_GLOBAL_FLD_ERPS_MODE].name,
            GLB_FLAG_ISSET(p_opmglb->erps, OPMGLB_FLAG_ERPS_MODE_HUAWEI));
    }
    if (FLD_MATCH(TBL_OPM_GLOBAL_FLD_ERPS_UNRELOAD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OPM_GLOBAL_FLD_ERPS_UNRELOAD].name,
            GLB_FLAG_ISSET(p_opmglb->erps, OPMGLB_FLAG_ERPS_MODE_UNRELOAD));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_opm_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_opm_global_t *p_db_opmglb = _g_p_tbl_opm_global;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_opmglb)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_opmglb, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_opm_global_t*
tbl_opm_global_init_opm_global()
{
    _g_p_tbl_opm_global = XCALLOC(MEM_TBL_OPM_GLOBAL, sizeof(tbl_opm_global_t));
    return _g_p_tbl_opm_global;
}

