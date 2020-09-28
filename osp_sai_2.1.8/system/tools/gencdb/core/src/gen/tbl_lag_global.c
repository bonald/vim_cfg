
#include "proto.h"
#include "gen/tbl_lag_global_define.h"
#include "gen/tbl_lag_global.h"
#include "cdb_data_cmp.h"

tbl_lag_global_t *_g_p_tbl_lag_global = NULL;

int32
tbl_lag_global_set_lag_global_field_sync(tbl_lag_global_t *p_lag_glb, tbl_lag_global_field_id_t field_id, uint32 sync)
{
    tbl_lag_global_t *p_db_lag_glb = _g_p_tbl_lag_global;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_LAG_GLOBAL_FLD_SYSTEM_PRIORITY:
        p_db_lag_glb->system_priority = p_lag_glb->system_priority;
        break;

    case TBL_LAG_GLOBAL_FLD_SYSTEM_ID:
        sal_memcpy(p_db_lag_glb->system_id, p_lag_glb->system_id, sizeof(p_lag_glb->system_id));
        break;

    case TBL_LAG_GLOBAL_FLD_MLAG_SYSTEM_PRIORITY:
        p_db_lag_glb->mlag_system_priority = p_lag_glb->mlag_system_priority;
        break;

    case TBL_LAG_GLOBAL_FLD_MLAG_SYSTEM_ID:
        sal_memcpy(p_db_lag_glb->mlag_system_id, p_lag_glb->mlag_system_id, sizeof(p_lag_glb->mlag_system_id));
        break;

    case TBL_LAG_GLOBAL_FLD_LOAD_BALANCE:
        p_db_lag_glb->load_balance = p_lag_glb->load_balance;
        break;

    case TBL_LAG_GLOBAL_FLD_LOAD_BALANCE_INNER_FIELD_EN:
        p_db_lag_glb->load_balance_inner_field_en = p_lag_glb->load_balance_inner_field_en;
        break;

    case TBL_LAG_GLOBAL_FLD_HASH_ARITHMETIC:
        p_db_lag_glb->hash_arithmetic = p_lag_glb->hash_arithmetic;
        break;

    case TBL_LAG_GLOBAL_FLD_LOAD_BALANCE_MODE:
        sal_memcpy(p_db_lag_glb->load_balance_mode, p_lag_glb->load_balance_mode, sizeof(p_lag_glb->load_balance_mode));
        break;

    case TBL_LAG_GLOBAL_FLD_MAX:
        sal_memcpy(p_db_lag_glb, p_lag_glb, sizeof(tbl_lag_global_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_LAG_GLOBAL, field_id, p_db_lag_glb);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_LAG_GLOBAL, field_id, p_db_lag_glb);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_lag_global_set_lag_global_field(tbl_lag_global_t *p_lag_glb, tbl_lag_global_field_id_t field_id)
{
    return tbl_lag_global_set_lag_global_field_sync(p_lag_glb, field_id, TRUE);
}

tbl_lag_global_t*
tbl_lag_global_get_lag_global()
{
    return _g_p_tbl_lag_global;
}

int32
tbl_lag_global_key_str2val(char *str, tbl_lag_global_t *p_lag_glb)
{
    return PM_E_NONE;
}

char*
tbl_lag_global_key_name_dump(tbl_lag_global_t *p_lag_glb, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_LAG_GLOBAL);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_lag_global_key_value_dump(tbl_lag_global_t *p_lag_glb, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_lag_global_field_name_dump(tbl_lag_global_t *p_lag_glb, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_LAG_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_LAG_GLOBAL_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_lag_global_field_value_dump(tbl_lag_global_t *p_lag_glb, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_LAG_GLOBAL_FLD_SYSTEM_PRIORITY, field_id))
    {
        sal_sprintf(str, "%u", p_lag_glb->system_priority);
    }
    if (FLD_MATCH(TBL_LAG_GLOBAL_FLD_SYSTEM_ID, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_lag_glb->system_id);
    }
    if (FLD_MATCH(TBL_LAG_GLOBAL_FLD_MLAG_SYSTEM_PRIORITY, field_id))
    {
        sal_sprintf(str, "%u", p_lag_glb->mlag_system_priority);
    }
    if (FLD_MATCH(TBL_LAG_GLOBAL_FLD_MLAG_SYSTEM_ID, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_lag_glb->mlag_system_id);
    }
    if (FLD_MATCH(TBL_LAG_GLOBAL_FLD_LOAD_BALANCE, field_id))
    {
        sal_sprintf(str, "%u", p_lag_glb->load_balance);
    }
    if (FLD_MATCH(TBL_LAG_GLOBAL_FLD_LOAD_BALANCE_INNER_FIELD_EN, field_id))
    {
        sal_sprintf(str, "%u", p_lag_glb->load_balance_inner_field_en);
    }
    if (FLD_MATCH(TBL_LAG_GLOBAL_FLD_HASH_ARITHMETIC, field_id))
    {
        sal_sprintf(str, "%u", p_lag_glb->hash_arithmetic);
    }
    if (FLD_MATCH(TBL_LAG_GLOBAL_FLD_LOAD_BALANCE_MODE, field_id))
    {
        cdb_uint8_array_val2str(str, MAX_CMD_STR_LEN, p_lag_glb->load_balance_mode, sizeof(p_lag_glb->load_balance_mode));
    }
    return str;
}

char**
tbl_lag_global_table_dump(tbl_lag_global_t *p_lag_glb, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_lag_global_key_name_dump(p_lag_glb, buf));
    for(i=1; i<TBL_LAG_GLOBAL_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_lag_global_field_name_dump(p_lag_glb, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_lag_global_field_value_dump(p_lag_glb, i, buf));
    }
    return str;
}

int32
tbl_lag_global_field_value_parser(char *str, int32 field_id, tbl_lag_global_t *p_lag_glb)
{
    if (FLD_MATCH(TBL_LAG_GLOBAL_FLD_SYSTEM_PRIORITY, field_id))
    {
        int32 ret;
        p_lag_glb->system_priority = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LAG_GLOBAL_FLD_SYSTEM_ID, field_id))
    {
        cdb_mac_addr_str2val(p_lag_glb->system_id, str);
    }
    if (FLD_MATCH(TBL_LAG_GLOBAL_FLD_MLAG_SYSTEM_PRIORITY, field_id))
    {
        int32 ret;
        p_lag_glb->mlag_system_priority = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LAG_GLOBAL_FLD_MLAG_SYSTEM_ID, field_id))
    {
        cdb_mac_addr_str2val(p_lag_glb->mlag_system_id, str);
    }
    if (FLD_MATCH(TBL_LAG_GLOBAL_FLD_LOAD_BALANCE, field_id))
    {
        int32 ret;
        p_lag_glb->load_balance = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LAG_GLOBAL_FLD_LOAD_BALANCE_INNER_FIELD_EN, field_id))
    {
        int32 ret;
        p_lag_glb->load_balance_inner_field_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LAG_GLOBAL_FLD_HASH_ARITHMETIC, field_id))
    {
        int32 ret;
        p_lag_glb->hash_arithmetic = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LAG_GLOBAL_FLD_LOAD_BALANCE_MODE, field_id))
    {
        cdb_uint8_array_str2val(str, p_lag_glb->load_balance_mode);
    }
    return PM_E_NONE;
}

int32
tbl_lag_global_table_parser(char** array, char* key_value,tbl_lag_global_t *p_lag_glb)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_LAG_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_lag_global_key_str2val(key_value, p_lag_glb));

    for(i=1; i<TBL_LAG_GLOBAL_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_lag_global_field_value_parser( array[j++], i, p_lag_glb));
    }

    return PM_E_NONE;
}

int32
tbl_lag_global_dump_one(tbl_lag_global_t *p_lag_glb, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_LAG_GLOBAL);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_LAG_GLOBAL_FLD_SYSTEM_PRIORITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LAG_GLOBAL_FLD_SYSTEM_PRIORITY].name,
            p_lag_glb->system_priority);
    }
    if (FLD_MATCH(TBL_LAG_GLOBAL_FLD_SYSTEM_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_LAG_GLOBAL_FLD_SYSTEM_ID].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_lag_glb->system_id));
    }
    if (FLD_MATCH(TBL_LAG_GLOBAL_FLD_MLAG_SYSTEM_PRIORITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LAG_GLOBAL_FLD_MLAG_SYSTEM_PRIORITY].name,
            p_lag_glb->mlag_system_priority);
    }
    if (FLD_MATCH(TBL_LAG_GLOBAL_FLD_MLAG_SYSTEM_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_LAG_GLOBAL_FLD_MLAG_SYSTEM_ID].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_lag_glb->mlag_system_id));
    }
    if (FLD_MATCH(TBL_LAG_GLOBAL_FLD_LOAD_BALANCE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LAG_GLOBAL_FLD_LOAD_BALANCE].name,
            p_lag_glb->load_balance);
    }
    if (FLD_MATCH(TBL_LAG_GLOBAL_FLD_LOAD_BALANCE_INNER_FIELD_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LAG_GLOBAL_FLD_LOAD_BALANCE_INNER_FIELD_EN].name,
            p_lag_glb->load_balance_inner_field_en);
    }
    if (FLD_MATCH(TBL_LAG_GLOBAL_FLD_HASH_ARITHMETIC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LAG_GLOBAL_FLD_HASH_ARITHMETIC].name,
            p_lag_glb->hash_arithmetic);
    }
    if (FLD_MATCH(TBL_LAG_GLOBAL_FLD_LOAD_BALANCE_MODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_LAG_GLOBAL_FLD_LOAD_BALANCE_MODE].name, 
            cdb_uint8_array_val2str(buf, MAX_CMD_STR_LEN, p_lag_glb->load_balance_mode, sizeof(p_lag_glb->load_balance_mode)));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_lag_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_lag_global_t *p_db_lag_glb = _g_p_tbl_lag_global;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_lag_glb)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_lag_glb, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_lag_global_t*
tbl_lag_global_init_lag_global()
{
    _g_p_tbl_lag_global = XCALLOC(MEM_TBL_LAG_GLOBAL, sizeof(tbl_lag_global_t));
    return _g_p_tbl_lag_global;
}

