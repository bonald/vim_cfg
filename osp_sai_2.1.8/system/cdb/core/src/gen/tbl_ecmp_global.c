
#include "proto.h"
#include "gen/tbl_ecmp_global_define.h"
#include "gen/tbl_ecmp_global.h"
#include "cdb_data_cmp.h"

tbl_ecmp_global_t *_g_p_tbl_ecmp_global = NULL;

int32
tbl_ecmp_global_set_ecmp_global_field_sync(tbl_ecmp_global_t *p_ecmp_glb, tbl_ecmp_global_field_id_t field_id, uint32 sync)
{
    tbl_ecmp_global_t *p_db_ecmp_glb = _g_p_tbl_ecmp_global;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_ECMP_GLOBAL_FLD_HASH_KEY:
        p_db_ecmp_glb->hash_key = p_ecmp_glb->hash_key;
        break;

    case TBL_ECMP_GLOBAL_FLD_HASH_ALGORITHM:
        p_db_ecmp_glb->hash_algorithm = p_ecmp_glb->hash_algorithm;
        break;

    case TBL_ECMP_GLOBAL_FLD_LOAD_BALANCE_MODE:
        p_db_ecmp_glb->load_balance_mode = p_ecmp_glb->load_balance_mode;
        break;

    case TBL_ECMP_GLOBAL_FLD_DLB_MODE_TYPE:
        p_db_ecmp_glb->dlb_mode_type = p_ecmp_glb->dlb_mode_type;
        break;

    case TBL_ECMP_GLOBAL_FLD_RR_PREFIX_NUM:
        p_db_ecmp_glb->rr_prefix_num = p_ecmp_glb->rr_prefix_num;
        break;

    case TBL_ECMP_GLOBAL_FLD_MAX:
        sal_memcpy(p_db_ecmp_glb, p_ecmp_glb, sizeof(tbl_ecmp_global_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_ECMP_GLOBAL, field_id, p_db_ecmp_glb);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_ECMP_GLOBAL, field_id, p_db_ecmp_glb);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ecmp_global_set_ecmp_global_field(tbl_ecmp_global_t *p_ecmp_glb, tbl_ecmp_global_field_id_t field_id)
{
    return tbl_ecmp_global_set_ecmp_global_field_sync(p_ecmp_glb, field_id, TRUE);
}

tbl_ecmp_global_t*
tbl_ecmp_global_get_ecmp_global()
{
    return _g_p_tbl_ecmp_global;
}

int32
tbl_ecmp_global_key_str2val(char *str, tbl_ecmp_global_t *p_ecmp_glb)
{
    return PM_E_NONE;
}

char*
tbl_ecmp_global_key_name_dump(tbl_ecmp_global_t *p_ecmp_glb, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ECMP_GLOBAL);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_ecmp_global_key_value_dump(tbl_ecmp_global_t *p_ecmp_glb, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_ecmp_global_field_name_dump(tbl_ecmp_global_t *p_ecmp_glb, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_ECMP_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_ECMP_GLOBAL_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_ecmp_global_field_value_dump(tbl_ecmp_global_t *p_ecmp_glb, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_ECMP_GLOBAL_FLD_HASH_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_ecmp_glb->hash_key);
    }
    if (FLD_MATCH(TBL_ECMP_GLOBAL_FLD_HASH_ALGORITHM, field_id))
    {
        sal_sprintf(str, "%u", p_ecmp_glb->hash_algorithm);
    }
    if (FLD_MATCH(TBL_ECMP_GLOBAL_FLD_LOAD_BALANCE_MODE, field_id))
    {
        sal_sprintf(str, "%u", p_ecmp_glb->load_balance_mode);
    }
    if (FLD_MATCH(TBL_ECMP_GLOBAL_FLD_DLB_MODE_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_ecmp_glb->dlb_mode_type);
    }
    if (FLD_MATCH(TBL_ECMP_GLOBAL_FLD_RR_PREFIX_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_ecmp_glb->rr_prefix_num);
    }
    return str;
}

char**
tbl_ecmp_global_table_dump(tbl_ecmp_global_t *p_ecmp_glb, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_ecmp_global_key_name_dump(p_ecmp_glb, buf));
    for(i=1; i<TBL_ECMP_GLOBAL_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_ecmp_global_field_name_dump(p_ecmp_glb, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_ecmp_global_field_value_dump(p_ecmp_glb, i, buf));
    }
    return str;
}

int32
tbl_ecmp_global_field_value_parser(char *str, int32 field_id, tbl_ecmp_global_t *p_ecmp_glb)
{
    if (FLD_MATCH(TBL_ECMP_GLOBAL_FLD_HASH_KEY, field_id))
    {
        int32 ret;
        p_ecmp_glb->hash_key = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ECMP_GLOBAL_FLD_HASH_ALGORITHM, field_id))
    {
        int32 ret;
        p_ecmp_glb->hash_algorithm = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ECMP_GLOBAL_FLD_LOAD_BALANCE_MODE, field_id))
    {
        int32 ret;
        p_ecmp_glb->load_balance_mode = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ECMP_GLOBAL_FLD_DLB_MODE_TYPE, field_id))
    {
        int32 ret;
        p_ecmp_glb->dlb_mode_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ECMP_GLOBAL_FLD_RR_PREFIX_NUM, field_id))
    {
        int32 ret;
        p_ecmp_glb->rr_prefix_num = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_ecmp_global_table_parser(char** array, char* key_value,tbl_ecmp_global_t *p_ecmp_glb)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_ECMP_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_ecmp_global_key_str2val(key_value, p_ecmp_glb));

    for(i=1; i<TBL_ECMP_GLOBAL_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_ecmp_global_field_value_parser( array[j++], i, p_ecmp_glb));
    }

    return PM_E_NONE;
}

int32
tbl_ecmp_global_dump_one(tbl_ecmp_global_t *p_ecmp_glb, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ECMP_GLOBAL);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_ECMP_GLOBAL_FLD_HASH_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ECMP_GLOBAL_FLD_HASH_KEY].name,
            p_ecmp_glb->hash_key);
    }
    if (FLD_MATCH(TBL_ECMP_GLOBAL_FLD_HASH_ALGORITHM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ECMP_GLOBAL_FLD_HASH_ALGORITHM].name,
            p_ecmp_glb->hash_algorithm);
    }
    if (FLD_MATCH(TBL_ECMP_GLOBAL_FLD_LOAD_BALANCE_MODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ECMP_GLOBAL_FLD_LOAD_BALANCE_MODE].name,
            p_ecmp_glb->load_balance_mode);
    }
    if (FLD_MATCH(TBL_ECMP_GLOBAL_FLD_DLB_MODE_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ECMP_GLOBAL_FLD_DLB_MODE_TYPE].name,
            p_ecmp_glb->dlb_mode_type);
    }
    if (FLD_MATCH(TBL_ECMP_GLOBAL_FLD_RR_PREFIX_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ECMP_GLOBAL_FLD_RR_PREFIX_NUM].name,
            p_ecmp_glb->rr_prefix_num);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_ecmp_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_ecmp_global_t *p_db_ecmp_glb = _g_p_tbl_ecmp_global;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_ecmp_glb)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_ecmp_glb, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_ecmp_global_t*
tbl_ecmp_global_init_ecmp_global()
{
    _g_p_tbl_ecmp_global = XCALLOC(MEM_TBL_ECMP_GLOBAL, sizeof(tbl_ecmp_global_t));
    return _g_p_tbl_ecmp_global;
}

