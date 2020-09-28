
#include "proto.h"
#include "gen/tbl_fea_qos_domain_define.h"
#include "gen/tbl_fea_qos_domain.h"
#include "cdb_data_cmp.h"

tbl_fea_qos_domain_master_t *_g_p_tbl_fea_qos_domain_master = NULL;

int32
tbl_fea_qos_domain_add_fea_qos_domain_sync(tbl_fea_qos_domain_t *p_fea_qos_domain, uint32 sync)
{
    tbl_fea_qos_domain_master_t *p_master = _g_p_tbl_fea_qos_domain_master;
    tbl_fea_qos_domain_t *p_db_fea_qos_domain = NULL;

    /* 1. lookup entry exist */
    if (tbl_fea_qos_domain_get_fea_qos_domain(&p_fea_qos_domain->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_fea_qos_domain = XCALLOC(MEM_TBL_FEA_QOS_DOMAIN, sizeof(tbl_fea_qos_domain_t));
    if (NULL == p_db_fea_qos_domain)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_fea_qos_domain, p_fea_qos_domain, sizeof(tbl_fea_qos_domain_t));

    /* 4. add to db */
    p_master->fea_domain_array[p_fea_qos_domain->key.id] = p_db_fea_qos_domain;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_FEA_QOS_DOMAIN, p_db_fea_qos_domain);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_FEA_QOS_DOMAIN, p_db_fea_qos_domain);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_fea_qos_domain_del_fea_qos_domain_sync(tbl_fea_qos_domain_key_t *p_fea_qos_domain_key, uint32 sync)
{
    tbl_fea_qos_domain_master_t *p_master = _g_p_tbl_fea_qos_domain_master;
    tbl_fea_qos_domain_t *p_db_fea_qos_domain = NULL;

    /* 1. lookup entry exist */
    p_db_fea_qos_domain = tbl_fea_qos_domain_get_fea_qos_domain(p_fea_qos_domain_key);
    if (NULL == p_db_fea_qos_domain)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_FEA_QOS_DOMAIN, p_db_fea_qos_domain);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_FEA_QOS_DOMAIN, p_db_fea_qos_domain);
        #endif 
    }

    /* 3. delete from db */
    p_master->fea_domain_array[p_fea_qos_domain_key->id] = NULL;

    /* 4. free db entry */
    XFREE(MEM_TBL_FEA_QOS_DOMAIN, p_db_fea_qos_domain);

    return PM_E_NONE;
}

int32
tbl_fea_qos_domain_set_fea_qos_domain_field_sync(tbl_fea_qos_domain_t *p_fea_qos_domain, tbl_fea_qos_domain_field_id_t field_id, uint32 sync)
{
    tbl_fea_qos_domain_t *p_db_fea_qos_domain = NULL;

    /* 1. lookup entry exist */
    p_db_fea_qos_domain = tbl_fea_qos_domain_get_fea_qos_domain(&p_fea_qos_domain->key);
    if (NULL == p_db_fea_qos_domain)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_FEA_QOS_DOMAIN_FLD_DOT1P_TO_TC_COLOR_MAP_ID:
        p_db_fea_qos_domain->dot1p_to_tc_color_map_id = p_fea_qos_domain->dot1p_to_tc_color_map_id;
        break;

    case TBL_FEA_QOS_DOMAIN_FLD_DSCP_TO_TC_COLOR_MAP_ID:
        p_db_fea_qos_domain->dscp_to_tc_color_map_id = p_fea_qos_domain->dscp_to_tc_color_map_id;
        break;

    case TBL_FEA_QOS_DOMAIN_FLD_EXP_TO_TC_COLOR_MAP_ID:
        p_db_fea_qos_domain->exp_to_tc_color_map_id = p_fea_qos_domain->exp_to_tc_color_map_id;
        break;

    case TBL_FEA_QOS_DOMAIN_FLD_TC_AND_COLOR_TO_DOT1P_MAP_ID:
        p_db_fea_qos_domain->tc_and_color_to_dot1p_map_id = p_fea_qos_domain->tc_and_color_to_dot1p_map_id;
        break;

    case TBL_FEA_QOS_DOMAIN_FLD_TC_AND_COLOR_TO_DSCP_MAP_ID:
        p_db_fea_qos_domain->tc_and_color_to_dscp_map_id = p_fea_qos_domain->tc_and_color_to_dscp_map_id;
        break;

    case TBL_FEA_QOS_DOMAIN_FLD_TC_AND_COLOR_TO_EXP_MAP_ID:
        p_db_fea_qos_domain->tc_and_color_to_exp_map_id = p_fea_qos_domain->tc_and_color_to_exp_map_id;
        break;

    case TBL_FEA_QOS_DOMAIN_FLD_MAX:
        sal_memcpy(p_db_fea_qos_domain, p_fea_qos_domain, sizeof(tbl_fea_qos_domain_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_FEA_QOS_DOMAIN, field_id, p_db_fea_qos_domain);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_FEA_QOS_DOMAIN, field_id, p_db_fea_qos_domain);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_fea_qos_domain_add_fea_qos_domain(tbl_fea_qos_domain_t *p_fea_qos_domain)
{
    return tbl_fea_qos_domain_add_fea_qos_domain_sync(p_fea_qos_domain, TRUE);
}

int32
tbl_fea_qos_domain_del_fea_qos_domain(tbl_fea_qos_domain_key_t *p_fea_qos_domain_key)
{
    return tbl_fea_qos_domain_del_fea_qos_domain_sync(p_fea_qos_domain_key, TRUE);
}

int32
tbl_fea_qos_domain_set_fea_qos_domain_field(tbl_fea_qos_domain_t *p_fea_qos_domain, tbl_fea_qos_domain_field_id_t field_id)
{
    return tbl_fea_qos_domain_set_fea_qos_domain_field_sync(p_fea_qos_domain, field_id, TRUE);
}

tbl_fea_qos_domain_t*
tbl_fea_qos_domain_get_fea_qos_domain(tbl_fea_qos_domain_key_t *p_fea_qos_domain_key)
{
    tbl_fea_qos_domain_master_t *p_master = _g_p_tbl_fea_qos_domain_master;

    if (GLB_QOS_DOMAIN_NUM <= p_fea_qos_domain_key->id)
    {
        return NULL;
    }
    return (p_master->fea_domain_array[p_fea_qos_domain_key->id]);
}

char*
tbl_fea_qos_domain_key_val2str(tbl_fea_qos_domain_t *p_fea_qos_domain, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_fea_qos_domain->key.id);
    return str;
}

int32
tbl_fea_qos_domain_key_str2val(char *str, tbl_fea_qos_domain_t *p_fea_qos_domain)
{
    int32 ret = 0;

    p_fea_qos_domain->key.id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_fea_qos_domain_key_name_dump(tbl_fea_qos_domain_t *p_fea_qos_domain, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_QOS_DOMAIN);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_fea_qos_domain->key.id);
    return str;
}

char*
tbl_fea_qos_domain_key_value_dump(tbl_fea_qos_domain_t *p_fea_qos_domain, char *str)
{
    sal_sprintf(str, "%u", p_fea_qos_domain->key.id);
    return str;
}

char*
tbl_fea_qos_domain_field_name_dump(tbl_fea_qos_domain_t *p_fea_qos_domain, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_QOS_DOMAIN);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_FEA_QOS_DOMAIN_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_fea_qos_domain_field_value_dump(tbl_fea_qos_domain_t *p_fea_qos_domain, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_FEA_QOS_DOMAIN_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_fea_qos_domain->key.id);
    }
    if (FLD_MATCH(TBL_FEA_QOS_DOMAIN_FLD_DOT1P_TO_TC_COLOR_MAP_ID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_fea_qos_domain->dot1p_to_tc_color_map_id);
    }
    if (FLD_MATCH(TBL_FEA_QOS_DOMAIN_FLD_DSCP_TO_TC_COLOR_MAP_ID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_fea_qos_domain->dscp_to_tc_color_map_id);
    }
    if (FLD_MATCH(TBL_FEA_QOS_DOMAIN_FLD_EXP_TO_TC_COLOR_MAP_ID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_fea_qos_domain->exp_to_tc_color_map_id);
    }
    if (FLD_MATCH(TBL_FEA_QOS_DOMAIN_FLD_TC_AND_COLOR_TO_DOT1P_MAP_ID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_fea_qos_domain->tc_and_color_to_dot1p_map_id);
    }
    if (FLD_MATCH(TBL_FEA_QOS_DOMAIN_FLD_TC_AND_COLOR_TO_DSCP_MAP_ID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_fea_qos_domain->tc_and_color_to_dscp_map_id);
    }
    if (FLD_MATCH(TBL_FEA_QOS_DOMAIN_FLD_TC_AND_COLOR_TO_EXP_MAP_ID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_fea_qos_domain->tc_and_color_to_exp_map_id);
    }
    return str;
}

char**
tbl_fea_qos_domain_table_dump(tbl_fea_qos_domain_t *p_fea_qos_domain, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_fea_qos_domain_key_name_dump(p_fea_qos_domain, buf));
    for(i=1; i<TBL_FEA_QOS_DOMAIN_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_fea_qos_domain_field_name_dump(p_fea_qos_domain, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_fea_qos_domain_field_value_dump(p_fea_qos_domain, i, buf));
    }
    return str;
}

int32
tbl_fea_qos_domain_field_value_parser(char *str, int32 field_id, tbl_fea_qos_domain_t *p_fea_qos_domain)
{
    if (FLD_MATCH(TBL_FEA_QOS_DOMAIN_FLD_KEY, field_id))
    {
        int32 ret;
        p_fea_qos_domain->key.id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_QOS_DOMAIN_FLD_DOT1P_TO_TC_COLOR_MAP_ID, field_id))
    {
        int32 ret;
        p_fea_qos_domain->dot1p_to_tc_color_map_id = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_QOS_DOMAIN_FLD_DSCP_TO_TC_COLOR_MAP_ID, field_id))
    {
        int32 ret;
        p_fea_qos_domain->dscp_to_tc_color_map_id = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_QOS_DOMAIN_FLD_EXP_TO_TC_COLOR_MAP_ID, field_id))
    {
        int32 ret;
        p_fea_qos_domain->exp_to_tc_color_map_id = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_QOS_DOMAIN_FLD_TC_AND_COLOR_TO_DOT1P_MAP_ID, field_id))
    {
        int32 ret;
        p_fea_qos_domain->tc_and_color_to_dot1p_map_id = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_QOS_DOMAIN_FLD_TC_AND_COLOR_TO_DSCP_MAP_ID, field_id))
    {
        int32 ret;
        p_fea_qos_domain->tc_and_color_to_dscp_map_id = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_QOS_DOMAIN_FLD_TC_AND_COLOR_TO_EXP_MAP_ID, field_id))
    {
        int32 ret;
        p_fea_qos_domain->tc_and_color_to_exp_map_id = cdb_uint64_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_fea_qos_domain_table_parser(char** array, char* key_value,tbl_fea_qos_domain_t *p_fea_qos_domain)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_QOS_DOMAIN);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_fea_qos_domain_key_str2val(key_value, p_fea_qos_domain));

    for(i=1; i<TBL_FEA_QOS_DOMAIN_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_fea_qos_domain_field_value_parser( array[j++], i, p_fea_qos_domain));
    }

    return PM_E_NONE;
}

int32
tbl_fea_qos_domain_dump_one(tbl_fea_qos_domain_t *p_fea_qos_domain, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_QOS_DOMAIN);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_FEA_QOS_DOMAIN_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_QOS_DOMAIN_FLD_KEY].name,
            p_fea_qos_domain->key.id);
    }
    if (FLD_MATCH(TBL_FEA_QOS_DOMAIN_FLD_DOT1P_TO_TC_COLOR_MAP_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_FEA_QOS_DOMAIN_FLD_DOT1P_TO_TC_COLOR_MAP_ID].name,
            p_fea_qos_domain->dot1p_to_tc_color_map_id);
    }
    if (FLD_MATCH(TBL_FEA_QOS_DOMAIN_FLD_DSCP_TO_TC_COLOR_MAP_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_FEA_QOS_DOMAIN_FLD_DSCP_TO_TC_COLOR_MAP_ID].name,
            p_fea_qos_domain->dscp_to_tc_color_map_id);
    }
    if (FLD_MATCH(TBL_FEA_QOS_DOMAIN_FLD_EXP_TO_TC_COLOR_MAP_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_FEA_QOS_DOMAIN_FLD_EXP_TO_TC_COLOR_MAP_ID].name,
            p_fea_qos_domain->exp_to_tc_color_map_id);
    }
    if (FLD_MATCH(TBL_FEA_QOS_DOMAIN_FLD_TC_AND_COLOR_TO_DOT1P_MAP_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_FEA_QOS_DOMAIN_FLD_TC_AND_COLOR_TO_DOT1P_MAP_ID].name,
            p_fea_qos_domain->tc_and_color_to_dot1p_map_id);
    }
    if (FLD_MATCH(TBL_FEA_QOS_DOMAIN_FLD_TC_AND_COLOR_TO_DSCP_MAP_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_FEA_QOS_DOMAIN_FLD_TC_AND_COLOR_TO_DSCP_MAP_ID].name,
            p_fea_qos_domain->tc_and_color_to_dscp_map_id);
    }
    if (FLD_MATCH(TBL_FEA_QOS_DOMAIN_FLD_TC_AND_COLOR_TO_EXP_MAP_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_FEA_QOS_DOMAIN_FLD_TC_AND_COLOR_TO_EXP_MAP_ID].name,
            p_fea_qos_domain->tc_and_color_to_exp_map_id);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_fea_qos_domain_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_fea_qos_domain_master_t *p_master = _g_p_tbl_fea_qos_domain_master;
    tbl_fea_qos_domain_t *p_db_fea_qos_domain = NULL;
    uint32 id = 0;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    for (id = 0; id < GLB_QOS_DOMAIN_NUM; id++)
    {
        p_db_fea_qos_domain = p_master->fea_domain_array[id];
        if (NULL == p_db_fea_qos_domain)
        {
            continue;
        }
        rc |= fn(p_db_fea_qos_domain, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_fea_qos_domain_master_t*
tbl_fea_qos_domain_get_master()
{
    return _g_p_tbl_fea_qos_domain_master;
}

tbl_fea_qos_domain_master_t*
tbl_fea_qos_domain_init_fea_qos_domain()
{
    _g_p_tbl_fea_qos_domain_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_fea_qos_domain_master_t));
    return _g_p_tbl_fea_qos_domain_master;
}

