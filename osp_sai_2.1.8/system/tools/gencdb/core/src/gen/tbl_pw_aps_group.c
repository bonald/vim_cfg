
#include "proto.h"
#include "gen/tbl_pw_aps_group_define.h"
#include "gen/tbl_pw_aps_group.h"
#include "cdb_data_cmp.h"

tbl_pw_aps_group_master_t *_g_p_tbl_pw_aps_group_master = NULL;

static uint32
_tbl_pw_aps_group_hash_make(tbl_pw_aps_group_t *p_pw_aps_group)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_pw_aps_group->key;
    for (index = 0; index < sizeof(p_pw_aps_group->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_pw_aps_group_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_pw_aps_group_t *p_pw_aps_group1 = (tbl_pw_aps_group_t*)p_arg1;
    tbl_pw_aps_group_t *p_pw_aps_group2 = (tbl_pw_aps_group_t*)p_arg2;

    if (0 == sal_memcmp(&p_pw_aps_group1->key, &p_pw_aps_group2->key, sizeof(tbl_pw_aps_group_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_pw_aps_group_add_pw_aps_group_sync(tbl_pw_aps_group_t *p_pw_aps_group, uint32 sync)
{
    tbl_pw_aps_group_master_t *p_master = _g_p_tbl_pw_aps_group_master;
    tbl_pw_aps_group_t *p_db_pw_aps_group = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_pw_aps_group_get_pw_aps_group(&p_pw_aps_group->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_pw_aps_group = XCALLOC(MEM_TBL_PW_APS_GROUP, sizeof(tbl_pw_aps_group_t));
    if (NULL == p_db_pw_aps_group)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_pw_aps_group, p_pw_aps_group, sizeof(tbl_pw_aps_group_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->pw_aps_group_hash, (void*)p_db_pw_aps_group, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->pw_aps_group_list, p_db_pw_aps_group);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_PW_APS_GROUP, p_db_pw_aps_group);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_PW_APS_GROUP, p_db_pw_aps_group);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_pw_aps_group_del_pw_aps_group_sync(tbl_pw_aps_group_key_t *p_pw_aps_group_key, uint32 sync)
{
    tbl_pw_aps_group_master_t *p_master = _g_p_tbl_pw_aps_group_master;
    tbl_pw_aps_group_t *p_db_pw_aps_group = NULL;

    /* 1. lookup entry exist */
    p_db_pw_aps_group = tbl_pw_aps_group_get_pw_aps_group(p_pw_aps_group_key);
    if (NULL == p_db_pw_aps_group)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_PW_APS_GROUP, p_db_pw_aps_group);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_PW_APS_GROUP, p_db_pw_aps_group);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->pw_aps_group_hash, (void*)p_db_pw_aps_group);
    ctclib_slistnode_delete(p_master->pw_aps_group_list, p_db_pw_aps_group);

    /* 4. free db entry */
    XFREE(MEM_TBL_PW_APS_GROUP, p_db_pw_aps_group);

    return PM_E_NONE;
}

int32
tbl_pw_aps_group_set_pw_aps_group_field_sync(tbl_pw_aps_group_t *p_pw_aps_group, tbl_pw_aps_group_field_id_t field_id, uint32 sync)
{
    tbl_pw_aps_group_t *p_db_pw_aps_group = NULL;

    /* 1. lookup entry exist */
    p_db_pw_aps_group = tbl_pw_aps_group_get_pw_aps_group(&p_pw_aps_group->key);
    if (NULL == p_db_pw_aps_group)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_PW_APS_GROUP_FLD_WORKING_PW_NAME:
        sal_memcpy(p_db_pw_aps_group->working_pw_name, p_pw_aps_group->working_pw_name, sizeof(p_pw_aps_group->working_pw_name));
        break;

    case TBL_PW_APS_GROUP_FLD_PROTECTION_PW_NAME:
        sal_memcpy(p_db_pw_aps_group->protection_pw_name, p_pw_aps_group->protection_pw_name, sizeof(p_pw_aps_group->protection_pw_name));
        break;

    case TBL_PW_APS_GROUP_FLD_REF_CNT:
        p_db_pw_aps_group->ref_cnt = p_pw_aps_group->ref_cnt;
        break;

    case TBL_PW_APS_GROUP_FLD_FLOW_FLAG:
        p_db_pw_aps_group->flow_flag = p_pw_aps_group->flow_flag;
        break;

    case TBL_PW_APS_GROUP_FLD_WAIT_TO_RESTORE_TIME:
        p_db_pw_aps_group->wait_to_restore_time = p_pw_aps_group->wait_to_restore_time;
        break;

    case TBL_PW_APS_GROUP_FLD_HOLD_OFF_TIME:
        p_db_pw_aps_group->hold_off_time = p_pw_aps_group->hold_off_time;
        break;

    case TBL_PW_APS_GROUP_FLD_IS_REVERTIVE:
        p_db_pw_aps_group->is_revertive = p_pw_aps_group->is_revertive;
        break;

    case TBL_PW_APS_GROUP_FLD_ACTIVE_PATH:
        p_db_pw_aps_group->active_path = p_pw_aps_group->active_path;
        break;

    case TBL_PW_APS_GROUP_FLD_BRIDGE_PERMANENT:
        p_db_pw_aps_group->bridge_permanent = p_pw_aps_group->bridge_permanent;
        break;

    case TBL_PW_APS_GROUP_FLD_CURRENT_STATE:
        p_db_pw_aps_group->current_state = p_pw_aps_group->current_state;
        break;

    case TBL_PW_APS_GROUP_FLD_MAX:
        sal_memcpy(p_db_pw_aps_group, p_pw_aps_group, sizeof(tbl_pw_aps_group_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_PW_APS_GROUP, field_id, p_db_pw_aps_group);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_PW_APS_GROUP, field_id, p_db_pw_aps_group);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_pw_aps_group_add_pw_aps_group(tbl_pw_aps_group_t *p_pw_aps_group)
{
    return tbl_pw_aps_group_add_pw_aps_group_sync(p_pw_aps_group, TRUE);
}

int32
tbl_pw_aps_group_del_pw_aps_group(tbl_pw_aps_group_key_t *p_pw_aps_group_key)
{
    return tbl_pw_aps_group_del_pw_aps_group_sync(p_pw_aps_group_key, TRUE);
}

int32
tbl_pw_aps_group_set_pw_aps_group_field(tbl_pw_aps_group_t *p_pw_aps_group, tbl_pw_aps_group_field_id_t field_id)
{
    return tbl_pw_aps_group_set_pw_aps_group_field_sync(p_pw_aps_group, field_id, TRUE);
}

tbl_pw_aps_group_t*
tbl_pw_aps_group_get_pw_aps_group(tbl_pw_aps_group_key_t *p_pw_aps_group_key)
{
    tbl_pw_aps_group_master_t *p_master = _g_p_tbl_pw_aps_group_master;
    tbl_pw_aps_group_t lkp;

    sal_memcpy(&lkp.key, p_pw_aps_group_key, sizeof(tbl_pw_aps_group_key_t));
    return ctclib_hash_lookup(p_master->pw_aps_group_hash, &lkp);
}

char*
tbl_pw_aps_group_key_val2str(tbl_pw_aps_group_t *p_pw_aps_group, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_pw_aps_group->key.group_id);
    return str;
}

int32
tbl_pw_aps_group_key_str2val(char *str, tbl_pw_aps_group_t *p_pw_aps_group)
{
    int32 ret = 0;

    p_pw_aps_group->key.group_id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_pw_aps_group_key_name_dump(tbl_pw_aps_group_t *p_pw_aps_group, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_PW_APS_GROUP);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_pw_aps_group->key.group_id);
    return str;
}

char*
tbl_pw_aps_group_key_value_dump(tbl_pw_aps_group_t *p_pw_aps_group, char *str)
{
    sal_sprintf(str, "%u", p_pw_aps_group->key.group_id);
    return str;
}

char*
tbl_pw_aps_group_field_name_dump(tbl_pw_aps_group_t *p_pw_aps_group, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_PW_APS_GROUP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_PW_APS_GROUP_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_pw_aps_group_field_value_dump(tbl_pw_aps_group_t *p_pw_aps_group, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_PW_APS_GROUP_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_pw_aps_group->key.group_id);
    }
    if (FLD_MATCH(TBL_PW_APS_GROUP_FLD_WORKING_PW_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_pw_aps_group->working_pw_name);
    }
    if (FLD_MATCH(TBL_PW_APS_GROUP_FLD_PROTECTION_PW_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_pw_aps_group->protection_pw_name);
    }
    if (FLD_MATCH(TBL_PW_APS_GROUP_FLD_REF_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_pw_aps_group->ref_cnt);
    }
    if (FLD_MATCH(TBL_PW_APS_GROUP_FLD_FLOW_FLAG, field_id))
    {
        sal_sprintf(str, "%u", p_pw_aps_group->flow_flag);
    }
    if (FLD_MATCH(TBL_PW_APS_GROUP_FLD_WAIT_TO_RESTORE_TIME, field_id))
    {
        sal_sprintf(str, "%u", p_pw_aps_group->wait_to_restore_time);
    }
    if (FLD_MATCH(TBL_PW_APS_GROUP_FLD_HOLD_OFF_TIME, field_id))
    {
        sal_sprintf(str, "%u", p_pw_aps_group->hold_off_time);
    }
    if (FLD_MATCH(TBL_PW_APS_GROUP_FLD_IS_REVERTIVE, field_id))
    {
        sal_sprintf(str, "%u", p_pw_aps_group->is_revertive);
    }
    if (FLD_MATCH(TBL_PW_APS_GROUP_FLD_ACTIVE_PATH, field_id))
    {
        sal_sprintf(str, "%u", p_pw_aps_group->active_path);
    }
    if (FLD_MATCH(TBL_PW_APS_GROUP_FLD_BRIDGE_PERMANENT, field_id))
    {
        sal_sprintf(str, "%u", p_pw_aps_group->bridge_permanent);
    }
    if (FLD_MATCH(TBL_PW_APS_GROUP_FLD_CURRENT_STATE, field_id))
    {
        sal_sprintf(str, "%u", p_pw_aps_group->current_state);
    }
    return str;
}

char**
tbl_pw_aps_group_table_dump(tbl_pw_aps_group_t *p_pw_aps_group, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_pw_aps_group_key_name_dump(p_pw_aps_group, buf));
    for(i=1; i<TBL_PW_APS_GROUP_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_pw_aps_group_field_name_dump(p_pw_aps_group, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_pw_aps_group_field_value_dump(p_pw_aps_group, i, buf));
    }
    return str;
}

int32
tbl_pw_aps_group_field_value_parser(char *str, int32 field_id, tbl_pw_aps_group_t *p_pw_aps_group)
{
    if (FLD_MATCH(TBL_PW_APS_GROUP_FLD_KEY, field_id))
    {
        int32 ret;
        p_pw_aps_group->key.group_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PW_APS_GROUP_FLD_WORKING_PW_NAME, field_id))
    {
        sal_strcpy(p_pw_aps_group->working_pw_name, str);
    }
    if (FLD_MATCH(TBL_PW_APS_GROUP_FLD_PROTECTION_PW_NAME, field_id))
    {
        sal_strcpy(p_pw_aps_group->protection_pw_name, str);
    }
    if (FLD_MATCH(TBL_PW_APS_GROUP_FLD_REF_CNT, field_id))
    {
        int32 ret;
        p_pw_aps_group->ref_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PW_APS_GROUP_FLD_FLOW_FLAG, field_id))
    {
        int32 ret;
        p_pw_aps_group->flow_flag = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PW_APS_GROUP_FLD_WAIT_TO_RESTORE_TIME, field_id))
    {
        int32 ret;
        p_pw_aps_group->wait_to_restore_time = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PW_APS_GROUP_FLD_HOLD_OFF_TIME, field_id))
    {
        int32 ret;
        p_pw_aps_group->hold_off_time = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PW_APS_GROUP_FLD_IS_REVERTIVE, field_id))
    {
        int32 ret;
        p_pw_aps_group->is_revertive = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PW_APS_GROUP_FLD_ACTIVE_PATH, field_id))
    {
        int32 ret;
        p_pw_aps_group->active_path = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PW_APS_GROUP_FLD_BRIDGE_PERMANENT, field_id))
    {
        int32 ret;
        p_pw_aps_group->bridge_permanent = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PW_APS_GROUP_FLD_CURRENT_STATE, field_id))
    {
        int32 ret;
        p_pw_aps_group->current_state = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_pw_aps_group_table_parser(char** array, char* key_value,tbl_pw_aps_group_t *p_pw_aps_group)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_PW_APS_GROUP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_pw_aps_group_key_str2val(key_value, p_pw_aps_group));

    for(i=1; i<TBL_PW_APS_GROUP_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_pw_aps_group_field_value_parser( array[j++], i, p_pw_aps_group));
    }

    return PM_E_NONE;
}

int32
tbl_pw_aps_group_dump_one(tbl_pw_aps_group_t *p_pw_aps_group, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_PW_APS_GROUP);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_PW_APS_GROUP_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PW_APS_GROUP_FLD_KEY].name,
            p_pw_aps_group->key.group_id);
    }
    if (FLD_MATCH(TBL_PW_APS_GROUP_FLD_WORKING_PW_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PW_APS_GROUP_FLD_WORKING_PW_NAME].name,
            p_pw_aps_group->working_pw_name);
    }
    if (FLD_MATCH(TBL_PW_APS_GROUP_FLD_PROTECTION_PW_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PW_APS_GROUP_FLD_PROTECTION_PW_NAME].name,
            p_pw_aps_group->protection_pw_name);
    }
    if (FLD_MATCH(TBL_PW_APS_GROUP_FLD_REF_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PW_APS_GROUP_FLD_REF_CNT].name,
            p_pw_aps_group->ref_cnt);
    }
    if (FLD_MATCH(TBL_PW_APS_GROUP_FLD_FLOW_FLAG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PW_APS_GROUP_FLD_FLOW_FLAG].name,
            p_pw_aps_group->flow_flag);
    }
    if (FLD_MATCH(TBL_PW_APS_GROUP_FLD_WAIT_TO_RESTORE_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PW_APS_GROUP_FLD_WAIT_TO_RESTORE_TIME].name,
            p_pw_aps_group->wait_to_restore_time);
    }
    if (FLD_MATCH(TBL_PW_APS_GROUP_FLD_HOLD_OFF_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PW_APS_GROUP_FLD_HOLD_OFF_TIME].name,
            p_pw_aps_group->hold_off_time);
    }
    if (FLD_MATCH(TBL_PW_APS_GROUP_FLD_IS_REVERTIVE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PW_APS_GROUP_FLD_IS_REVERTIVE].name,
            p_pw_aps_group->is_revertive);
    }
    if (FLD_MATCH(TBL_PW_APS_GROUP_FLD_ACTIVE_PATH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PW_APS_GROUP_FLD_ACTIVE_PATH].name,
            p_pw_aps_group->active_path);
    }
    if (FLD_MATCH(TBL_PW_APS_GROUP_FLD_BRIDGE_PERMANENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PW_APS_GROUP_FLD_BRIDGE_PERMANENT].name,
            p_pw_aps_group->bridge_permanent);
    }
    if (FLD_MATCH(TBL_PW_APS_GROUP_FLD_CURRENT_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PW_APS_GROUP_FLD_CURRENT_STATE].name,
            p_pw_aps_group->current_state);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_pw_aps_group_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_pw_aps_group_master_t *p_master = _g_p_tbl_pw_aps_group_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_pw_aps_group_t *p_db_pw_aps_group = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->pw_aps_group_list, p_db_pw_aps_group, listnode, next)
    {
        rc |= fn(p_db_pw_aps_group, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_pw_aps_group_master_t*
tbl_pw_aps_group_get_master()
{
    return _g_p_tbl_pw_aps_group_master;
}

tbl_pw_aps_group_master_t*
tbl_pw_aps_group_init_pw_aps_group()
{
    _g_p_tbl_pw_aps_group_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_pw_aps_group_master_t));
    _g_p_tbl_pw_aps_group_master->pw_aps_group_hash = ctclib_hash_create(_tbl_pw_aps_group_hash_make, _tbl_pw_aps_group_hash_cmp);
    _g_p_tbl_pw_aps_group_master->pw_aps_group_list = ctclib_slist_create(tbl_pw_aps_group_cmp, NULL);
    return _g_p_tbl_pw_aps_group_master;
}

