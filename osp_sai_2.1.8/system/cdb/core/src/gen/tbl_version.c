
#include "proto.h"
#include "gen/tbl_version_define.h"
#include "gen/tbl_version.h"
#include "cdb_data_cmp.h"

tbl_version_t *_g_p_tbl_version = NULL;

int32
tbl_version_set_version_field_sync(tbl_version_t *p_ver, tbl_version_field_id_t field_id, uint32 sync)
{
    tbl_version_t *p_db_ver = _g_p_tbl_version;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_VERSION_FLD_VERSION:
        sal_memcpy(p_db_ver->version, p_ver->version, sizeof(p_ver->version));
        break;

    case TBL_VERSION_FLD_COMPANY:
        sal_memcpy(p_db_ver->company, p_ver->company, sizeof(p_ver->company));
        break;

    case TBL_VERSION_FLD_PACKAGE:
        sal_memcpy(p_db_ver->package, p_ver->package, sizeof(p_ver->package));
        break;

    case TBL_VERSION_FLD_PRODUCT:
        sal_memcpy(p_db_ver->product, p_ver->product, sizeof(p_ver->product));
        break;

    case TBL_VERSION_FLD_HW_TYPE:
        sal_memcpy(p_db_ver->hw_type, p_ver->hw_type, sizeof(p_ver->hw_type));
        break;

    case TBL_VERSION_FLD_IS_CST:
        p_db_ver->is_cst = p_ver->is_cst;
        break;

    case TBL_VERSION_FLD_CST_COMPANY:
        sal_memcpy(p_db_ver->cst_company, p_ver->cst_company, sizeof(p_ver->cst_company));
        break;

    case TBL_VERSION_FLD_CST_HW_TYPE:
        sal_memcpy(p_db_ver->cst_hw_type, p_ver->cst_hw_type, sizeof(p_ver->cst_hw_type));
        break;

    case TBL_VERSION_FLD_CST_HW_INFO:
        sal_memcpy(p_db_ver->cst_hw_info, p_ver->cst_hw_info, sizeof(p_ver->cst_hw_info));
        break;

    case TBL_VERSION_FLD_CST_HW_VER:
        sal_memcpy(p_db_ver->cst_hw_ver, p_ver->cst_hw_ver, sizeof(p_ver->cst_hw_ver));
        break;

    case TBL_VERSION_FLD_CST_SW_VER:
        sal_memcpy(p_db_ver->cst_sw_ver, p_ver->cst_sw_ver, sizeof(p_ver->cst_sw_ver));
        break;

    case TBL_VERSION_FLD_MAX:
        sal_memcpy(p_db_ver, p_ver, sizeof(tbl_version_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_VERSION, field_id, p_db_ver);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_VERSION, field_id, p_db_ver);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_version_set_version_field(tbl_version_t *p_ver, tbl_version_field_id_t field_id)
{
    return tbl_version_set_version_field_sync(p_ver, field_id, TRUE);
}

tbl_version_t*
tbl_version_get_version()
{
    return _g_p_tbl_version;
}

int32
tbl_version_key_str2val(char *str, tbl_version_t *p_ver)
{
    return PM_E_NONE;
}

char*
tbl_version_key_name_dump(tbl_version_t *p_ver, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_VERSION);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_version_key_value_dump(tbl_version_t *p_ver, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_version_field_name_dump(tbl_version_t *p_ver, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_VERSION);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_VERSION_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_version_field_value_dump(tbl_version_t *p_ver, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_VERSION_FLD_VERSION, field_id))
    {
        sal_sprintf(str, "%s", p_ver->version);
    }
    if (FLD_MATCH(TBL_VERSION_FLD_COMPANY, field_id))
    {
        sal_sprintf(str, "%s", p_ver->company);
    }
    if (FLD_MATCH(TBL_VERSION_FLD_PACKAGE, field_id))
    {
        sal_sprintf(str, "%s", p_ver->package);
    }
    if (FLD_MATCH(TBL_VERSION_FLD_PRODUCT, field_id))
    {
        sal_sprintf(str, "%s", p_ver->product);
    }
    if (FLD_MATCH(TBL_VERSION_FLD_HW_TYPE, field_id))
    {
        sal_sprintf(str, "%s", p_ver->hw_type);
    }
    if (FLD_MATCH(TBL_VERSION_FLD_IS_CST, field_id))
    {
        sal_sprintf(str, "%u", p_ver->is_cst);
    }
    if (FLD_MATCH(TBL_VERSION_FLD_CST_COMPANY, field_id))
    {
        sal_sprintf(str, "%s", p_ver->cst_company);
    }
    if (FLD_MATCH(TBL_VERSION_FLD_CST_HW_TYPE, field_id))
    {
        sal_sprintf(str, "%s", p_ver->cst_hw_type);
    }
    if (FLD_MATCH(TBL_VERSION_FLD_CST_HW_INFO, field_id))
    {
        sal_sprintf(str, "%s", p_ver->cst_hw_info);
    }
    if (FLD_MATCH(TBL_VERSION_FLD_CST_HW_VER, field_id))
    {
        sal_sprintf(str, "%s", p_ver->cst_hw_ver);
    }
    if (FLD_MATCH(TBL_VERSION_FLD_CST_SW_VER, field_id))
    {
        sal_sprintf(str, "%s", p_ver->cst_sw_ver);
    }
    return str;
}

char**
tbl_version_table_dump(tbl_version_t *p_ver, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_version_key_name_dump(p_ver, buf));
    for(i=1; i<TBL_VERSION_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_version_field_name_dump(p_ver, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_version_field_value_dump(p_ver, i, buf));
    }
    return str;
}

int32
tbl_version_field_value_parser(char *str, int32 field_id, tbl_version_t *p_ver)
{
    if (FLD_MATCH(TBL_VERSION_FLD_VERSION, field_id))
    {
        sal_strcpy(p_ver->version, str);
    }
    if (FLD_MATCH(TBL_VERSION_FLD_COMPANY, field_id))
    {
        sal_strcpy(p_ver->company, str);
    }
    if (FLD_MATCH(TBL_VERSION_FLD_PACKAGE, field_id))
    {
        sal_strcpy(p_ver->package, str);
    }
    if (FLD_MATCH(TBL_VERSION_FLD_PRODUCT, field_id))
    {
        sal_strcpy(p_ver->product, str);
    }
    if (FLD_MATCH(TBL_VERSION_FLD_HW_TYPE, field_id))
    {
        sal_strcpy(p_ver->hw_type, str);
    }
    if (FLD_MATCH(TBL_VERSION_FLD_IS_CST, field_id))
    {
        int32 ret;
        p_ver->is_cst = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VERSION_FLD_CST_COMPANY, field_id))
    {
        sal_strcpy(p_ver->cst_company, str);
    }
    if (FLD_MATCH(TBL_VERSION_FLD_CST_HW_TYPE, field_id))
    {
        sal_strcpy(p_ver->cst_hw_type, str);
    }
    if (FLD_MATCH(TBL_VERSION_FLD_CST_HW_INFO, field_id))
    {
        sal_strcpy(p_ver->cst_hw_info, str);
    }
    if (FLD_MATCH(TBL_VERSION_FLD_CST_HW_VER, field_id))
    {
        sal_strcpy(p_ver->cst_hw_ver, str);
    }
    if (FLD_MATCH(TBL_VERSION_FLD_CST_SW_VER, field_id))
    {
        sal_strcpy(p_ver->cst_sw_ver, str);
    }
    return PM_E_NONE;
}

int32
tbl_version_table_parser(char** array, char* key_value,tbl_version_t *p_ver)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_VERSION);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_version_key_str2val(key_value, p_ver));

    for(i=1; i<TBL_VERSION_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_version_field_value_parser( array[j++], i, p_ver));
    }

    return PM_E_NONE;
}

int32
tbl_version_dump_one(tbl_version_t *p_ver, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_VERSION);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_VERSION_FLD_VERSION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VERSION_FLD_VERSION].name,
            p_ver->version);
    }
    if (FLD_MATCH(TBL_VERSION_FLD_COMPANY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VERSION_FLD_COMPANY].name,
            p_ver->company);
    }
    if (FLD_MATCH(TBL_VERSION_FLD_PACKAGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VERSION_FLD_PACKAGE].name,
            p_ver->package);
    }
    if (FLD_MATCH(TBL_VERSION_FLD_PRODUCT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VERSION_FLD_PRODUCT].name,
            p_ver->product);
    }
    if (FLD_MATCH(TBL_VERSION_FLD_HW_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VERSION_FLD_HW_TYPE].name,
            p_ver->hw_type);
    }
    if (FLD_MATCH(TBL_VERSION_FLD_IS_CST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VERSION_FLD_IS_CST].name,
            p_ver->is_cst);
    }
    if (FLD_MATCH(TBL_VERSION_FLD_CST_COMPANY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VERSION_FLD_CST_COMPANY].name,
            p_ver->cst_company);
    }
    if (FLD_MATCH(TBL_VERSION_FLD_CST_HW_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VERSION_FLD_CST_HW_TYPE].name,
            p_ver->cst_hw_type);
    }
    if (FLD_MATCH(TBL_VERSION_FLD_CST_HW_INFO, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VERSION_FLD_CST_HW_INFO].name,
            p_ver->cst_hw_info);
    }
    if (FLD_MATCH(TBL_VERSION_FLD_CST_HW_VER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VERSION_FLD_CST_HW_VER].name,
            p_ver->cst_hw_ver);
    }
    if (FLD_MATCH(TBL_VERSION_FLD_CST_SW_VER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VERSION_FLD_CST_SW_VER].name,
            p_ver->cst_sw_ver);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_version_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_version_t *p_db_ver = _g_p_tbl_version;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_ver)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_ver, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_version_t*
tbl_version_init_version()
{
    _g_p_tbl_version = XCALLOC(MEM_TBL_VERSION, sizeof(tbl_version_t));
    return _g_p_tbl_version;
}

