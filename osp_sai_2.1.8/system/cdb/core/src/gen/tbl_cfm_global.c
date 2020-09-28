
#include "proto.h"
#include "gen/tbl_cfm_global_define.h"
#include "gen/tbl_cfm_global.h"
#include "cdb_data_cmp.h"

tbl_cfm_global_t *_g_p_tbl_cfm_global = NULL;

int32
tbl_cfm_global_set_cfm_global_field_sync(tbl_cfm_global_t *p_cfm_glb, tbl_cfm_global_field_id_t field_id, uint32 sync)
{
    tbl_cfm_global_t *p_db_cfm_glb = _g_p_tbl_cfm_global;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_CFM_GLOBAL_FLD_CFM_ENABLE:
        p_db_cfm_glb->cfm_enable = p_cfm_glb->cfm_enable;
        break;

    case TBL_CFM_GLOBAL_FLD_CFM_MODE:
        p_db_cfm_glb->cfm_mode = p_cfm_glb->cfm_mode;
        break;

    case TBL_CFM_GLOBAL_FLD_MAX:
        sal_memcpy(p_db_cfm_glb, p_cfm_glb, sizeof(tbl_cfm_global_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_CFM_GLOBAL, field_id, p_db_cfm_glb);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_CFM_GLOBAL, field_id, p_db_cfm_glb);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_cfm_global_set_cfm_global_field(tbl_cfm_global_t *p_cfm_glb, tbl_cfm_global_field_id_t field_id)
{
    return tbl_cfm_global_set_cfm_global_field_sync(p_cfm_glb, field_id, TRUE);
}

tbl_cfm_global_t*
tbl_cfm_global_get_cfm_global()
{
    return _g_p_tbl_cfm_global;
}

int32
tbl_cfm_global_key_str2val(char *str, tbl_cfm_global_t *p_cfm_glb)
{
    return PM_E_NONE;
}

char*
tbl_cfm_global_key_name_dump(tbl_cfm_global_t *p_cfm_glb, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CFM_GLOBAL);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_cfm_global_key_value_dump(tbl_cfm_global_t *p_cfm_glb, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_cfm_global_field_name_dump(tbl_cfm_global_t *p_cfm_glb, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_CFM_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_CFM_GLOBAL_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_cfm_global_field_value_dump(tbl_cfm_global_t *p_cfm_glb, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_CFM_GLOBAL_FLD_CFM_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_cfm_glb->cfm_enable);
    }
    if (FLD_MATCH(TBL_CFM_GLOBAL_FLD_CFM_MODE, field_id))
    {
        sal_sprintf(str, "%u", p_cfm_glb->cfm_mode);
    }
    return str;
}

char**
tbl_cfm_global_table_dump(tbl_cfm_global_t *p_cfm_glb, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_cfm_global_key_name_dump(p_cfm_glb, buf));
    for(i=1; i<TBL_CFM_GLOBAL_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_cfm_global_field_name_dump(p_cfm_glb, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_cfm_global_field_value_dump(p_cfm_glb, i, buf));
    }
    return str;
}

int32
tbl_cfm_global_field_value_parser(char *str, int32 field_id, tbl_cfm_global_t *p_cfm_glb)
{
    if (FLD_MATCH(TBL_CFM_GLOBAL_FLD_CFM_ENABLE, field_id))
    {
        int32 ret;
        p_cfm_glb->cfm_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CFM_GLOBAL_FLD_CFM_MODE, field_id))
    {
        int32 ret;
        p_cfm_glb->cfm_mode = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_cfm_global_table_parser(char** array, char* key_value,tbl_cfm_global_t *p_cfm_glb)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_CFM_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_cfm_global_key_str2val(key_value, p_cfm_glb));

    for(i=1; i<TBL_CFM_GLOBAL_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_cfm_global_field_value_parser( array[j++], i, p_cfm_glb));
    }

    return PM_E_NONE;
}

int32
tbl_cfm_global_dump_one(tbl_cfm_global_t *p_cfm_glb, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CFM_GLOBAL);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_CFM_GLOBAL_FLD_CFM_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CFM_GLOBAL_FLD_CFM_ENABLE].name,
            p_cfm_glb->cfm_enable);
    }
    if (FLD_MATCH(TBL_CFM_GLOBAL_FLD_CFM_MODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CFM_GLOBAL_FLD_CFM_MODE].name,
            p_cfm_glb->cfm_mode);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_cfm_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_cfm_global_t *p_db_cfm_glb = _g_p_tbl_cfm_global;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_cfm_glb)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_cfm_glb, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_cfm_global_t*
tbl_cfm_global_init_cfm_global()
{
    _g_p_tbl_cfm_global = XCALLOC(MEM_TBL_CFM_GLOBAL, sizeof(tbl_cfm_global_t));
    return _g_p_tbl_cfm_global;
}

