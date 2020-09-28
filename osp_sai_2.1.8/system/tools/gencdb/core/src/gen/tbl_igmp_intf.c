
#include "proto.h"
#include "gen/tbl_igmp_intf_define.h"
#include "gen/tbl_igmp_intf.h"
#include "cdb_data_cmp.h"

tbl_igmp_intf_master_t *_g_p_tbl_igmp_intf_master = NULL;

static uint32
_tbl_igmp_intf_hash_make(tbl_igmp_intf_t *p_igmp_intf)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_igmp_intf->key;
    for (index = 0; index < sizeof(p_igmp_intf->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_igmp_intf_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_igmp_intf_t *p_igmp_intf1 = (tbl_igmp_intf_t*)p_arg1;
    tbl_igmp_intf_t *p_igmp_intf2 = (tbl_igmp_intf_t*)p_arg2;

    if (0 == sal_memcmp(&p_igmp_intf1->key, &p_igmp_intf2->key, sizeof(tbl_igmp_intf_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_igmp_intf_add_igmp_intf_sync(tbl_igmp_intf_t *p_igmp_intf, uint32 sync)
{
    tbl_igmp_intf_master_t *p_master = _g_p_tbl_igmp_intf_master;
    tbl_igmp_intf_t *p_db_igmp_intf = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_igmp_intf_get_igmp_intf(&p_igmp_intf->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_igmp_intf = XCALLOC(MEM_TBL_IGMP_INTF, sizeof(tbl_igmp_intf_t));
    if (NULL == p_db_igmp_intf)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_igmp_intf, p_igmp_intf, sizeof(tbl_igmp_intf_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->igmp_intf_hash, (void*)p_db_igmp_intf, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->igmp_intf_list, p_db_igmp_intf);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_IGMP_INTF, p_db_igmp_intf);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_IGMP_INTF, p_db_igmp_intf);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_igmp_intf_del_igmp_intf_sync(tbl_igmp_intf_key_t *p_igmp_intf_key, uint32 sync)
{
    tbl_igmp_intf_master_t *p_master = _g_p_tbl_igmp_intf_master;
    tbl_igmp_intf_t *p_db_igmp_intf = NULL;

    /* 1. lookup entry exist */
    p_db_igmp_intf = tbl_igmp_intf_get_igmp_intf(p_igmp_intf_key);
    if (NULL == p_db_igmp_intf)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_IGMP_INTF, p_db_igmp_intf);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_IGMP_INTF, p_db_igmp_intf);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->igmp_intf_hash, (void*)p_db_igmp_intf);
    ctclib_slistnode_delete(p_master->igmp_intf_list, p_db_igmp_intf);

    /* 4. free db entry */
    XFREE(MEM_TBL_IGMP_INTF, p_db_igmp_intf);

    return PM_E_NONE;
}

int32
tbl_igmp_intf_set_igmp_intf_field_sync(tbl_igmp_intf_t *p_igmp_intf, tbl_igmp_intf_field_id_t field_id, uint32 sync)
{
    tbl_igmp_intf_t *p_db_igmp_intf = NULL;

    /* 1. lookup entry exist */
    p_db_igmp_intf = tbl_igmp_intf_get_igmp_intf(&p_igmp_intf->key);
    if (NULL == p_db_igmp_intf)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_IGMP_INTF_FLD_QUERIER_TIMEOUT:
        p_db_igmp_intf->querier_timeout = p_igmp_intf->querier_timeout;
        break;

    case TBL_IGMP_INTF_FLD_QUERY_INTERVAL:
        p_db_igmp_intf->query_interval = p_igmp_intf->query_interval;
        break;

    case TBL_IGMP_INTF_FLD_ROBUSTNESS_VARIABLE:
        p_db_igmp_intf->robustness_variable = p_igmp_intf->robustness_variable;
        break;

    case TBL_IGMP_INTF_FLD_LAYER3_PROTOCOL_INDEX:
        p_db_igmp_intf->layer3_protocol_index = p_igmp_intf->layer3_protocol_index;
        break;

    case TBL_IGMP_INTF_FLD_LAYER3_PROTOCOL:
        p_db_igmp_intf->layer3_protocol = p_igmp_intf->layer3_protocol;
        break;

    case TBL_IGMP_INTF_FLD_ENTRY_VALID:
        p_db_igmp_intf->entry_valid = p_igmp_intf->entry_valid;
        break;

    case TBL_IGMP_INTF_FLD_ACTION_INDEX:
        p_db_igmp_intf->action_index = p_igmp_intf->action_index;
        break;

    case TBL_IGMP_INTF_FLD_LAST_MEMBER_QUERY_COUNT:
        p_db_igmp_intf->last_member_query_count = p_igmp_intf->last_member_query_count;
        break;

    case TBL_IGMP_INTF_FLD_LAST_MEMBER_QUERY_INTERVAL:
        p_db_igmp_intf->last_member_query_interval = p_igmp_intf->last_member_query_interval;
        break;

    case TBL_IGMP_INTF_FLD_GLOBAL_LIMIT_NUMBER:
        p_db_igmp_intf->global_limit_number = p_igmp_intf->global_limit_number;
        break;

    case TBL_IGMP_INTF_FLD_MAX:
        sal_memcpy(p_db_igmp_intf, p_igmp_intf, sizeof(tbl_igmp_intf_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_IGMP_INTF, field_id, p_db_igmp_intf);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_IGMP_INTF, field_id, p_db_igmp_intf);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_igmp_intf_add_igmp_intf(tbl_igmp_intf_t *p_igmp_intf)
{
    return tbl_igmp_intf_add_igmp_intf_sync(p_igmp_intf, TRUE);
}

int32
tbl_igmp_intf_del_igmp_intf(tbl_igmp_intf_key_t *p_igmp_intf_key)
{
    return tbl_igmp_intf_del_igmp_intf_sync(p_igmp_intf_key, TRUE);
}

int32
tbl_igmp_intf_set_igmp_intf_field(tbl_igmp_intf_t *p_igmp_intf, tbl_igmp_intf_field_id_t field_id)
{
    return tbl_igmp_intf_set_igmp_intf_field_sync(p_igmp_intf, field_id, TRUE);
}

tbl_igmp_intf_t*
tbl_igmp_intf_get_igmp_intf(tbl_igmp_intf_key_t *p_igmp_intf_key)
{
    tbl_igmp_intf_master_t *p_master = _g_p_tbl_igmp_intf_master;
    tbl_igmp_intf_t lkp;

    sal_memcpy(&lkp.key, p_igmp_intf_key, sizeof(tbl_igmp_intf_key_t));
    return ctclib_hash_lookup(p_master->igmp_intf_hash, &lkp);
}

char*
tbl_igmp_intf_key_val2str(tbl_igmp_intf_t *p_igmp_intf, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_igmp_intf->key.name);
    return str;
}

int32
tbl_igmp_intf_key_str2val(char *str, tbl_igmp_intf_t *p_igmp_intf)
{
    int32 ret = 0;

    sal_strcpy(p_igmp_intf->key.name, str);
    return ret;
}

char*
tbl_igmp_intf_key_name_dump(tbl_igmp_intf_t *p_igmp_intf, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_IGMP_INTF);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_igmp_intf->key.name);
    return str;
}

char*
tbl_igmp_intf_key_value_dump(tbl_igmp_intf_t *p_igmp_intf, char *str)
{
    sal_sprintf(str, "%s", p_igmp_intf->key.name);
    return str;
}

char*
tbl_igmp_intf_field_name_dump(tbl_igmp_intf_t *p_igmp_intf, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_IGMP_INTF);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_IGMP_INTF_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_igmp_intf_field_value_dump(tbl_igmp_intf_t *p_igmp_intf, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_IGMP_INTF_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_igmp_intf->key.name);
    }
    if (FLD_MATCH(TBL_IGMP_INTF_FLD_QUERIER_TIMEOUT, field_id))
    {
        sal_sprintf(str, "%u", p_igmp_intf->querier_timeout);
    }
    if (FLD_MATCH(TBL_IGMP_INTF_FLD_QUERY_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_igmp_intf->query_interval);
    }
    if (FLD_MATCH(TBL_IGMP_INTF_FLD_ROBUSTNESS_VARIABLE, field_id))
    {
        sal_sprintf(str, "%u", p_igmp_intf->robustness_variable);
    }
    if (FLD_MATCH(TBL_IGMP_INTF_FLD_LAYER3_PROTOCOL_INDEX, field_id))
    {
        sal_sprintf(str, "%u", p_igmp_intf->layer3_protocol_index);
    }
    if (FLD_MATCH(TBL_IGMP_INTF_FLD_LAYER3_PROTOCOL, field_id))
    {
        sal_sprintf(str, "%u", p_igmp_intf->layer3_protocol);
    }
    if (FLD_MATCH(TBL_IGMP_INTF_FLD_ENTRY_VALID, field_id))
    {
        sal_sprintf(str, "%u", p_igmp_intf->entry_valid);
    }
    if (FLD_MATCH(TBL_IGMP_INTF_FLD_ACTION_INDEX, field_id))
    {
        sal_sprintf(str, "%u", p_igmp_intf->action_index);
    }
    if (FLD_MATCH(TBL_IGMP_INTF_FLD_LAST_MEMBER_QUERY_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_igmp_intf->last_member_query_count);
    }
    if (FLD_MATCH(TBL_IGMP_INTF_FLD_LAST_MEMBER_QUERY_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_igmp_intf->last_member_query_interval);
    }
    if (FLD_MATCH(TBL_IGMP_INTF_FLD_GLOBAL_LIMIT_NUMBER, field_id))
    {
        sal_sprintf(str, "%u", p_igmp_intf->global_limit_number);
    }
    return str;
}

char**
tbl_igmp_intf_table_dump(tbl_igmp_intf_t *p_igmp_intf, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_igmp_intf_key_name_dump(p_igmp_intf, buf));
    for(i=1; i<TBL_IGMP_INTF_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_igmp_intf_field_name_dump(p_igmp_intf, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_igmp_intf_field_value_dump(p_igmp_intf, i, buf));
    }
    return str;
}

int32
tbl_igmp_intf_field_value_parser(char *str, int32 field_id, tbl_igmp_intf_t *p_igmp_intf)
{
    if (FLD_MATCH(TBL_IGMP_INTF_FLD_KEY, field_id))
    {
        sal_strcpy(p_igmp_intf->key.name, str);
    }
    if (FLD_MATCH(TBL_IGMP_INTF_FLD_QUERIER_TIMEOUT, field_id))
    {
        int32 ret;
        p_igmp_intf->querier_timeout = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGMP_INTF_FLD_QUERY_INTERVAL, field_id))
    {
        int32 ret;
        p_igmp_intf->query_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGMP_INTF_FLD_ROBUSTNESS_VARIABLE, field_id))
    {
        int32 ret;
        p_igmp_intf->robustness_variable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGMP_INTF_FLD_LAYER3_PROTOCOL_INDEX, field_id))
    {
        int32 ret;
        p_igmp_intf->layer3_protocol_index = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGMP_INTF_FLD_LAYER3_PROTOCOL, field_id))
    {
        int32 ret;
        p_igmp_intf->layer3_protocol = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGMP_INTF_FLD_ENTRY_VALID, field_id))
    {
        int32 ret;
        p_igmp_intf->entry_valid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGMP_INTF_FLD_ACTION_INDEX, field_id))
    {
        int32 ret;
        p_igmp_intf->action_index = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGMP_INTF_FLD_LAST_MEMBER_QUERY_COUNT, field_id))
    {
        int32 ret;
        p_igmp_intf->last_member_query_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGMP_INTF_FLD_LAST_MEMBER_QUERY_INTERVAL, field_id))
    {
        int32 ret;
        p_igmp_intf->last_member_query_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IGMP_INTF_FLD_GLOBAL_LIMIT_NUMBER, field_id))
    {
        int32 ret;
        p_igmp_intf->global_limit_number = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_igmp_intf_table_parser(char** array, char* key_value,tbl_igmp_intf_t *p_igmp_intf)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_IGMP_INTF);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_igmp_intf_key_str2val(key_value, p_igmp_intf));

    for(i=1; i<TBL_IGMP_INTF_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_igmp_intf_field_value_parser( array[j++], i, p_igmp_intf));
    }

    return PM_E_NONE;
}

int32
tbl_igmp_intf_dump_one(tbl_igmp_intf_t *p_igmp_intf, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_IGMP_INTF);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_IGMP_INTF_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IGMP_INTF_FLD_KEY].name,
            p_igmp_intf->key.name);
    }
    if (FLD_MATCH(TBL_IGMP_INTF_FLD_QUERIER_TIMEOUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGMP_INTF_FLD_QUERIER_TIMEOUT].name,
            p_igmp_intf->querier_timeout);
    }
    if (FLD_MATCH(TBL_IGMP_INTF_FLD_QUERY_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGMP_INTF_FLD_QUERY_INTERVAL].name,
            p_igmp_intf->query_interval);
    }
    if (FLD_MATCH(TBL_IGMP_INTF_FLD_ROBUSTNESS_VARIABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGMP_INTF_FLD_ROBUSTNESS_VARIABLE].name,
            p_igmp_intf->robustness_variable);
    }
    if (FLD_MATCH(TBL_IGMP_INTF_FLD_LAYER3_PROTOCOL_INDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGMP_INTF_FLD_LAYER3_PROTOCOL_INDEX].name,
            p_igmp_intf->layer3_protocol_index);
    }
    if (FLD_MATCH(TBL_IGMP_INTF_FLD_LAYER3_PROTOCOL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGMP_INTF_FLD_LAYER3_PROTOCOL].name,
            p_igmp_intf->layer3_protocol);
    }
    if (FLD_MATCH(TBL_IGMP_INTF_FLD_ENTRY_VALID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGMP_INTF_FLD_ENTRY_VALID].name,
            p_igmp_intf->entry_valid);
    }
    if (FLD_MATCH(TBL_IGMP_INTF_FLD_ACTION_INDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGMP_INTF_FLD_ACTION_INDEX].name,
            p_igmp_intf->action_index);
    }
    if (FLD_MATCH(TBL_IGMP_INTF_FLD_LAST_MEMBER_QUERY_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGMP_INTF_FLD_LAST_MEMBER_QUERY_COUNT].name,
            p_igmp_intf->last_member_query_count);
    }
    if (FLD_MATCH(TBL_IGMP_INTF_FLD_LAST_MEMBER_QUERY_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGMP_INTF_FLD_LAST_MEMBER_QUERY_INTERVAL].name,
            p_igmp_intf->last_member_query_interval);
    }
    if (FLD_MATCH(TBL_IGMP_INTF_FLD_GLOBAL_LIMIT_NUMBER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IGMP_INTF_FLD_GLOBAL_LIMIT_NUMBER].name,
            p_igmp_intf->global_limit_number);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_igmp_intf_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_igmp_intf_master_t *p_master = _g_p_tbl_igmp_intf_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_igmp_intf_t *p_db_igmp_intf = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->igmp_intf_list, p_db_igmp_intf, listnode, next)
    {
        rc |= fn(p_db_igmp_intf, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_igmp_intf_master_t*
tbl_igmp_intf_get_master()
{
    return _g_p_tbl_igmp_intf_master;
}

tbl_igmp_intf_master_t*
tbl_igmp_intf_init_igmp_intf()
{
    _g_p_tbl_igmp_intf_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_igmp_intf_master_t));
    _g_p_tbl_igmp_intf_master->igmp_intf_hash = ctclib_hash_create(_tbl_igmp_intf_hash_make, _tbl_igmp_intf_hash_cmp);
    _g_p_tbl_igmp_intf_master->igmp_intf_list = ctclib_slist_create(tbl_igmp_intf_cmp, NULL);
    return _g_p_tbl_igmp_intf_master;
}

