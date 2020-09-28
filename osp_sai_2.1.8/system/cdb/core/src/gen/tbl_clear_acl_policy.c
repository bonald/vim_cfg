
#include "proto.h"
#include "gen/tbl_clear_acl_policy_define.h"
#include "gen/tbl_clear_acl_policy.h"
#include "cdb_data_cmp.h"

tbl_clear_acl_policy_t *_g_p_tbl_clear_acl_policy = NULL;

int32
tbl_clear_acl_policy_set_clear_acl_policy_field_sync(tbl_clear_acl_policy_t *p_clear_acl_policy, tbl_clear_acl_policy_field_id_t field_id, uint32 sync)
{
    tbl_clear_acl_policy_t *p_db_clear_acl_policy = _g_p_tbl_clear_acl_policy;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_CLEAR_ACL_POLICY_FLD_COUNTER_ID:
        p_db_clear_acl_policy->counter_id = p_clear_acl_policy->counter_id;
        break;

    case TBL_CLEAR_ACL_POLICY_FLD_ACL_ID:
        p_db_clear_acl_policy->acl_id = p_clear_acl_policy->acl_id;
        break;

    case TBL_CLEAR_ACL_POLICY_FLD_MAX:
        sal_memcpy(p_db_clear_acl_policy, p_clear_acl_policy, sizeof(tbl_clear_acl_policy_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_CLEAR_ACL_POLICY, field_id, p_db_clear_acl_policy);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_CLEAR_ACL_POLICY, field_id, p_db_clear_acl_policy);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_clear_acl_policy_set_clear_acl_policy_field(tbl_clear_acl_policy_t *p_clear_acl_policy, tbl_clear_acl_policy_field_id_t field_id)
{
    return tbl_clear_acl_policy_set_clear_acl_policy_field_sync(p_clear_acl_policy, field_id, TRUE);
}

tbl_clear_acl_policy_t*
tbl_clear_acl_policy_get_clear_acl_policy()
{
    return _g_p_tbl_clear_acl_policy;
}

int32
tbl_clear_acl_policy_key_str2val(char *str, tbl_clear_acl_policy_t *p_clear_acl_policy)
{
    return PM_E_NONE;
}

char*
tbl_clear_acl_policy_key_name_dump(tbl_clear_acl_policy_t *p_clear_acl_policy, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CLEAR_ACL_POLICY);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_clear_acl_policy_key_value_dump(tbl_clear_acl_policy_t *p_clear_acl_policy, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_clear_acl_policy_field_name_dump(tbl_clear_acl_policy_t *p_clear_acl_policy, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_CLEAR_ACL_POLICY);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_CLEAR_ACL_POLICY_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_clear_acl_policy_field_value_dump(tbl_clear_acl_policy_t *p_clear_acl_policy, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_CLEAR_ACL_POLICY_FLD_COUNTER_ID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_clear_acl_policy->counter_id);
    }
    if (FLD_MATCH(TBL_CLEAR_ACL_POLICY_FLD_ACL_ID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_clear_acl_policy->acl_id);
    }
    return str;
}

char**
tbl_clear_acl_policy_table_dump(tbl_clear_acl_policy_t *p_clear_acl_policy, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_clear_acl_policy_key_name_dump(p_clear_acl_policy, buf));
    for(i=1; i<TBL_CLEAR_ACL_POLICY_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_clear_acl_policy_field_name_dump(p_clear_acl_policy, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_clear_acl_policy_field_value_dump(p_clear_acl_policy, i, buf));
    }
    return str;
}

int32
tbl_clear_acl_policy_field_value_parser(char *str, int32 field_id, tbl_clear_acl_policy_t *p_clear_acl_policy)
{
    if (FLD_MATCH(TBL_CLEAR_ACL_POLICY_FLD_COUNTER_ID, field_id))
    {
        int32 ret;
        p_clear_acl_policy->counter_id = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CLEAR_ACL_POLICY_FLD_ACL_ID, field_id))
    {
        int32 ret;
        p_clear_acl_policy->acl_id = cdb_uint64_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_clear_acl_policy_table_parser(char** array, char* key_value,tbl_clear_acl_policy_t *p_clear_acl_policy)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_CLEAR_ACL_POLICY);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_clear_acl_policy_key_str2val(key_value, p_clear_acl_policy));

    for(i=1; i<TBL_CLEAR_ACL_POLICY_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_clear_acl_policy_field_value_parser( array[j++], i, p_clear_acl_policy));
    }

    return PM_E_NONE;
}

int32
tbl_clear_acl_policy_dump_one(tbl_clear_acl_policy_t *p_clear_acl_policy, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CLEAR_ACL_POLICY);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_CLEAR_ACL_POLICY_FLD_COUNTER_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CLEAR_ACL_POLICY_FLD_COUNTER_ID].name,
            p_clear_acl_policy->counter_id);
    }
    if (FLD_MATCH(TBL_CLEAR_ACL_POLICY_FLD_ACL_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CLEAR_ACL_POLICY_FLD_ACL_ID].name,
            p_clear_acl_policy->acl_id);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_clear_acl_policy_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_clear_acl_policy_t *p_db_clear_acl_policy = _g_p_tbl_clear_acl_policy;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_clear_acl_policy)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_clear_acl_policy, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_clear_acl_policy_t*
tbl_clear_acl_policy_init_clear_acl_policy()
{
    _g_p_tbl_clear_acl_policy = XCALLOC(MEM_TBL_CLEAR_ACL_POLICY, sizeof(tbl_clear_acl_policy_t));
    return _g_p_tbl_clear_acl_policy;
}

