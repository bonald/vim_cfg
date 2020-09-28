
#include "proto.h"
#include "gen/tbl_hash_value_global_define.h"
#include "gen/tbl_hash_value_global.h"
#include "cdb_data_cmp.h"

tbl_hash_value_global_t *_g_p_tbl_hash_value_global = NULL;

int32
tbl_hash_value_global_set_hash_value_global_field_sync(tbl_hash_value_global_t *p_hash_value_global, tbl_hash_value_global_field_id_t field_id, uint32 sync)
{
    tbl_hash_value_global_t *p_db_hash_value_global = _g_p_tbl_hash_value_global;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_HASH_VALUE_GLOBAL_FLD_BITMAP_DATA:
        p_db_hash_value_global->bitmap_data = p_hash_value_global->bitmap_data;
        break;

    case TBL_HASH_VALUE_GLOBAL_FLD_BITMAP_COMMAND:
        p_db_hash_value_global->bitmap_command = p_hash_value_global->bitmap_command;
        break;

    case TBL_HASH_VALUE_GLOBAL_FLD_AGG_HASH:
        p_db_hash_value_global->agg_hash = p_hash_value_global->agg_hash;
        break;

    case TBL_HASH_VALUE_GLOBAL_FLD_ECMP_ALL:
        p_db_hash_value_global->ecmp_all = p_hash_value_global->ecmp_all;
        break;

    case TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC:
        p_db_hash_value_global->ecmp_static = p_hash_value_global->ecmp_static;
        break;

    case TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC_L3:
        p_db_hash_value_global->ecmp_static_l3 = p_hash_value_global->ecmp_static_l3;
        break;

    case TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC_MPLS:
        p_db_hash_value_global->ecmp_static_mpls = p_hash_value_global->ecmp_static_mpls;
        break;

    case TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC_NVGRE:
        p_db_hash_value_global->ecmp_static_nvgre = p_hash_value_global->ecmp_static_nvgre;
        break;

    case TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC_VXLAN:
        p_db_hash_value_global->ecmp_static_vxlan = p_hash_value_global->ecmp_static_vxlan;
        break;

    case TBL_HASH_VALUE_GLOBAL_FLD_ECMP_DYNAMIC:
        p_db_hash_value_global->ecmp_dynamic = p_hash_value_global->ecmp_dynamic;
        break;

    case TBL_HASH_VALUE_GLOBAL_FLD_ECMP_DYNAMIC_FLOW_ID:
        p_db_hash_value_global->ecmp_dynamic_flow_id = p_hash_value_global->ecmp_dynamic_flow_id;
        break;

    case TBL_HASH_VALUE_GLOBAL_FLD_LBID:
        p_db_hash_value_global->lbid = p_hash_value_global->lbid;
        break;

    case TBL_HASH_VALUE_GLOBAL_FLD_MAX:
        sal_memcpy(p_db_hash_value_global, p_hash_value_global, sizeof(tbl_hash_value_global_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_HASH_VALUE_GLOBAL, field_id, p_db_hash_value_global);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_HASH_VALUE_GLOBAL, field_id, p_db_hash_value_global);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_hash_value_global_set_hash_value_global_field(tbl_hash_value_global_t *p_hash_value_global, tbl_hash_value_global_field_id_t field_id)
{
    return tbl_hash_value_global_set_hash_value_global_field_sync(p_hash_value_global, field_id, TRUE);
}

tbl_hash_value_global_t*
tbl_hash_value_global_get_hash_value_global()
{
    return _g_p_tbl_hash_value_global;
}

int32
tbl_hash_value_global_key_str2val(char *str, tbl_hash_value_global_t *p_hash_value_global)
{
    return PM_E_NONE;
}

char*
tbl_hash_value_global_key_name_dump(tbl_hash_value_global_t *p_hash_value_global, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_HASH_VALUE_GLOBAL);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_hash_value_global_key_value_dump(tbl_hash_value_global_t *p_hash_value_global, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_hash_value_global_field_name_dump(tbl_hash_value_global_t *p_hash_value_global, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_HASH_VALUE_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_HASH_VALUE_GLOBAL_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_hash_value_global_field_value_dump(tbl_hash_value_global_t *p_hash_value_global, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_HASH_VALUE_GLOBAL_FLD_BITMAP_DATA, field_id))
    {
        sal_sprintf(str, "%u", p_hash_value_global->bitmap_data);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_GLOBAL_FLD_BITMAP_COMMAND, field_id))
    {
        sal_sprintf(str, "%u", p_hash_value_global->bitmap_command);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_GLOBAL_FLD_AGG_HASH, field_id))
    {
        sal_sprintf(str, "%u", p_hash_value_global->agg_hash);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_GLOBAL_FLD_ECMP_ALL, field_id))
    {
        sal_sprintf(str, "%u", p_hash_value_global->ecmp_all);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC, field_id))
    {
        sal_sprintf(str, "%u", p_hash_value_global->ecmp_static);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC_L3, field_id))
    {
        sal_sprintf(str, "%u", p_hash_value_global->ecmp_static_l3);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC_MPLS, field_id))
    {
        sal_sprintf(str, "%u", p_hash_value_global->ecmp_static_mpls);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC_NVGRE, field_id))
    {
        sal_sprintf(str, "%u", p_hash_value_global->ecmp_static_nvgre);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC_VXLAN, field_id))
    {
        sal_sprintf(str, "%u", p_hash_value_global->ecmp_static_vxlan);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_GLOBAL_FLD_ECMP_DYNAMIC, field_id))
    {
        sal_sprintf(str, "%u", p_hash_value_global->ecmp_dynamic);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_GLOBAL_FLD_ECMP_DYNAMIC_FLOW_ID, field_id))
    {
        sal_sprintf(str, "%u", p_hash_value_global->ecmp_dynamic_flow_id);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_GLOBAL_FLD_LBID, field_id))
    {
        sal_sprintf(str, "%u", p_hash_value_global->lbid);
    }
    return str;
}

char**
tbl_hash_value_global_table_dump(tbl_hash_value_global_t *p_hash_value_global, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_hash_value_global_key_name_dump(p_hash_value_global, buf));
    for(i=1; i<TBL_HASH_VALUE_GLOBAL_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_hash_value_global_field_name_dump(p_hash_value_global, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_hash_value_global_field_value_dump(p_hash_value_global, i, buf));
    }
    return str;
}

int32
tbl_hash_value_global_field_value_parser(char *str, int32 field_id, tbl_hash_value_global_t *p_hash_value_global)
{
    if (FLD_MATCH(TBL_HASH_VALUE_GLOBAL_FLD_BITMAP_DATA, field_id))
    {
        int32 ret;
        p_hash_value_global->bitmap_data = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_GLOBAL_FLD_BITMAP_COMMAND, field_id))
    {
        int32 ret;
        p_hash_value_global->bitmap_command = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_GLOBAL_FLD_AGG_HASH, field_id))
    {
        int32 ret;
        p_hash_value_global->agg_hash = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_GLOBAL_FLD_ECMP_ALL, field_id))
    {
        int32 ret;
        p_hash_value_global->ecmp_all = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC, field_id))
    {
        int32 ret;
        p_hash_value_global->ecmp_static = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC_L3, field_id))
    {
        int32 ret;
        p_hash_value_global->ecmp_static_l3 = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC_MPLS, field_id))
    {
        int32 ret;
        p_hash_value_global->ecmp_static_mpls = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC_NVGRE, field_id))
    {
        int32 ret;
        p_hash_value_global->ecmp_static_nvgre = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC_VXLAN, field_id))
    {
        int32 ret;
        p_hash_value_global->ecmp_static_vxlan = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_GLOBAL_FLD_ECMP_DYNAMIC, field_id))
    {
        int32 ret;
        p_hash_value_global->ecmp_dynamic = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_GLOBAL_FLD_ECMP_DYNAMIC_FLOW_ID, field_id))
    {
        int32 ret;
        p_hash_value_global->ecmp_dynamic_flow_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_GLOBAL_FLD_LBID, field_id))
    {
        int32 ret;
        p_hash_value_global->lbid = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_hash_value_global_table_parser(char** array, char* key_value,tbl_hash_value_global_t *p_hash_value_global)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_HASH_VALUE_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_hash_value_global_key_str2val(key_value, p_hash_value_global));

    for(i=1; i<TBL_HASH_VALUE_GLOBAL_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_hash_value_global_field_value_parser( array[j++], i, p_hash_value_global));
    }

    return PM_E_NONE;
}

int32
tbl_hash_value_global_dump_one(tbl_hash_value_global_t *p_hash_value_global, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_HASH_VALUE_GLOBAL);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_HASH_VALUE_GLOBAL_FLD_BITMAP_DATA, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_VALUE_GLOBAL_FLD_BITMAP_DATA].name,
            p_hash_value_global->bitmap_data);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_GLOBAL_FLD_BITMAP_COMMAND, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_VALUE_GLOBAL_FLD_BITMAP_COMMAND].name,
            p_hash_value_global->bitmap_command);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_GLOBAL_FLD_AGG_HASH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_VALUE_GLOBAL_FLD_AGG_HASH].name,
            p_hash_value_global->agg_hash);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_GLOBAL_FLD_ECMP_ALL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_VALUE_GLOBAL_FLD_ECMP_ALL].name,
            p_hash_value_global->ecmp_all);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC].name,
            p_hash_value_global->ecmp_static);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC_L3, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC_L3].name,
            p_hash_value_global->ecmp_static_l3);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC_MPLS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC_MPLS].name,
            p_hash_value_global->ecmp_static_mpls);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC_NVGRE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC_NVGRE].name,
            p_hash_value_global->ecmp_static_nvgre);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC_VXLAN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC_VXLAN].name,
            p_hash_value_global->ecmp_static_vxlan);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_GLOBAL_FLD_ECMP_DYNAMIC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_VALUE_GLOBAL_FLD_ECMP_DYNAMIC].name,
            p_hash_value_global->ecmp_dynamic);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_GLOBAL_FLD_ECMP_DYNAMIC_FLOW_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_VALUE_GLOBAL_FLD_ECMP_DYNAMIC_FLOW_ID].name,
            p_hash_value_global->ecmp_dynamic_flow_id);
    }
    if (FLD_MATCH(TBL_HASH_VALUE_GLOBAL_FLD_LBID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_HASH_VALUE_GLOBAL_FLD_LBID].name,
            p_hash_value_global->lbid);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_hash_value_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_hash_value_global_t *p_db_hash_value_global = _g_p_tbl_hash_value_global;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_hash_value_global)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_hash_value_global, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_hash_value_global_t*
tbl_hash_value_global_init_hash_value_global()
{
    _g_p_tbl_hash_value_global = XCALLOC(MEM_TBL_HASH_VALUE_GLOBAL, sizeof(tbl_hash_value_global_t));
    return _g_p_tbl_hash_value_global;
}

