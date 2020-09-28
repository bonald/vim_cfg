
#include "proto.h"
#include "gen/tbl_ipmc_intf_define.h"
#include "gen/tbl_ipmc_intf.h"
#include "cdb_data_cmp.h"

tbl_ipmc_intf_master_t *_g_p_tbl_ipmc_intf_master = NULL;

static uint32
_tbl_ipmc_intf_hash_make(tbl_ipmc_intf_t *p_pim_intf)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_pim_intf->key;
    for (index = 0; index < sizeof(p_pim_intf->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_ipmc_intf_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_ipmc_intf_t *p_pim_intf1 = (tbl_ipmc_intf_t*)p_arg1;
    tbl_ipmc_intf_t *p_pim_intf2 = (tbl_ipmc_intf_t*)p_arg2;

    if (0 == sal_memcmp(&p_pim_intf1->key, &p_pim_intf2->key, sizeof(tbl_ipmc_intf_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_ipmc_intf_add_ipmc_intf_sync(tbl_ipmc_intf_t *p_pim_intf, uint32 sync)
{
    tbl_ipmc_intf_master_t *p_master = _g_p_tbl_ipmc_intf_master;
    tbl_ipmc_intf_t *p_db_pim_intf = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_ipmc_intf_get_ipmc_intf(&p_pim_intf->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_pim_intf = XCALLOC(MEM_TBL_IPMC_INTF, sizeof(tbl_ipmc_intf_t));
    if (NULL == p_db_pim_intf)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_pim_intf, p_pim_intf, sizeof(tbl_ipmc_intf_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->pim_intf_hash, (void*)p_db_pim_intf, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->pim_intf_list, p_db_pim_intf);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_IPMC_INTF, p_db_pim_intf);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_IPMC_INTF, p_db_pim_intf);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ipmc_intf_del_ipmc_intf_sync(tbl_ipmc_intf_key_t *p_pim_intf_key, uint32 sync)
{
    tbl_ipmc_intf_master_t *p_master = _g_p_tbl_ipmc_intf_master;
    tbl_ipmc_intf_t *p_db_pim_intf = NULL;

    /* 1. lookup entry exist */
    p_db_pim_intf = tbl_ipmc_intf_get_ipmc_intf(p_pim_intf_key);
    if (NULL == p_db_pim_intf)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_IPMC_INTF, p_db_pim_intf);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_IPMC_INTF, p_db_pim_intf);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->pim_intf_hash, (void*)p_db_pim_intf);
    ctclib_slistnode_delete(p_master->pim_intf_list, p_db_pim_intf);

    /* 4. free db entry */
    XFREE(MEM_TBL_IPMC_INTF, p_db_pim_intf);

    return PM_E_NONE;
}

int32
tbl_ipmc_intf_set_ipmc_intf_field_sync(tbl_ipmc_intf_t *p_pim_intf, tbl_ipmc_intf_field_id_t field_id, uint32 sync)
{
    tbl_ipmc_intf_t *p_db_pim_intf = NULL;

    /* 1. lookup entry exist */
    p_db_pim_intf = tbl_ipmc_intf_get_ipmc_intf(&p_pim_intf->key);
    if (NULL == p_db_pim_intf)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_IPMC_INTF_FLD_PRIMARY_ADDRESS:
        sal_memcpy(&p_db_pim_intf->primary_address, &p_pim_intf->primary_address, sizeof(p_pim_intf->primary_address));
        break;

    case TBL_IPMC_INTF_FLD_IGMP_ENABLE:
        p_db_pim_intf->igmp_enable = p_pim_intf->igmp_enable;
        break;

    case TBL_IPMC_INTF_FLD_PIM_ENABLE:
        p_db_pim_intf->pim_enable = p_pim_intf->pim_enable;
        break;

    case TBL_IPMC_INTF_FLD_IGMP_VERSION:
        p_db_pim_intf->igmp_version = p_pim_intf->igmp_version;
        break;

    case TBL_IPMC_INTF_FLD_IGMP_QUERY_INTERVAL:
        p_db_pim_intf->igmp_query_interval = p_pim_intf->igmp_query_interval;
        break;

    case TBL_IPMC_INTF_FLD_IGMP_QUERY_MAX_RESPONSE_TIME:
        p_db_pim_intf->igmp_query_max_response_time = p_pim_intf->igmp_query_max_response_time;
        break;

    case TBL_IPMC_INTF_FLD_PIM_HELLO_PERIOD:
        p_db_pim_intf->pim_hello_period = p_pim_intf->pim_hello_period;
        break;

    case TBL_IPMC_INTF_FLD_PIM_HELLO_HOLDTIME:
        p_db_pim_intf->pim_hello_holdtime = p_pim_intf->pim_hello_holdtime;
        break;

    case TBL_IPMC_INTF_FLD_PIM_GENERATION_ID:
        p_db_pim_intf->pim_generation_id = p_pim_intf->pim_generation_id;
        break;

    case TBL_IPMC_INTF_FLD_PIM_PROPAGATION_DELAY_MSEC:
        p_db_pim_intf->pim_propagation_delay_msec = p_pim_intf->pim_propagation_delay_msec;
        break;

    case TBL_IPMC_INTF_FLD_PIM_OVERRIDE_INTERVAL_MSEC:
        p_db_pim_intf->pim_override_interval_msec = p_pim_intf->pim_override_interval_msec;
        break;

    case TBL_IPMC_INTF_FLD_PIM_DR_PRIORITY:
        p_db_pim_intf->pim_dr_priority = p_pim_intf->pim_dr_priority;
        break;

    case TBL_IPMC_INTF_FLD_MAX:
        sal_memcpy(p_db_pim_intf, p_pim_intf, sizeof(tbl_ipmc_intf_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_IPMC_INTF, field_id, p_db_pim_intf);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_IPMC_INTF, field_id, p_db_pim_intf);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ipmc_intf_add_ipmc_intf(tbl_ipmc_intf_t *p_pim_intf)
{
    return tbl_ipmc_intf_add_ipmc_intf_sync(p_pim_intf, TRUE);
}

int32
tbl_ipmc_intf_del_ipmc_intf(tbl_ipmc_intf_key_t *p_pim_intf_key)
{
    return tbl_ipmc_intf_del_ipmc_intf_sync(p_pim_intf_key, TRUE);
}

int32
tbl_ipmc_intf_set_ipmc_intf_field(tbl_ipmc_intf_t *p_pim_intf, tbl_ipmc_intf_field_id_t field_id)
{
    return tbl_ipmc_intf_set_ipmc_intf_field_sync(p_pim_intf, field_id, TRUE);
}

tbl_ipmc_intf_t*
tbl_ipmc_intf_get_ipmc_intf(tbl_ipmc_intf_key_t *p_pim_intf_key)
{
    tbl_ipmc_intf_master_t *p_master = _g_p_tbl_ipmc_intf_master;
    tbl_ipmc_intf_t lkp;

    sal_memcpy(&lkp.key, p_pim_intf_key, sizeof(tbl_ipmc_intf_key_t));
    return ctclib_hash_lookup(p_master->pim_intf_hash, &lkp);
}

char*
tbl_ipmc_intf_key_val2str(tbl_ipmc_intf_t *p_pim_intf, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_pim_intf->key.name);
    return str;
}

int32
tbl_ipmc_intf_key_str2val(char *str, tbl_ipmc_intf_t *p_pim_intf)
{
    int32 ret = 0;

    sal_strcpy(p_pim_intf->key.name, str);
    return ret;
}

char*
tbl_ipmc_intf_key_name_dump(tbl_ipmc_intf_t *p_pim_intf, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_IPMC_INTF);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_pim_intf->key.name);
    return str;
}

char*
tbl_ipmc_intf_key_value_dump(tbl_ipmc_intf_t *p_pim_intf, char *str)
{
    sal_sprintf(str, "%s", p_pim_intf->key.name);
    return str;
}

char*
tbl_ipmc_intf_field_name_dump(tbl_ipmc_intf_t *p_pim_intf, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_IPMC_INTF);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_IPMC_INTF_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_ipmc_intf_field_value_dump(tbl_ipmc_intf_t *p_pim_intf, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_IPMC_INTF_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_pim_intf->key.name);
    }
    if (FLD_MATCH(TBL_IPMC_INTF_FLD_PRIMARY_ADDRESS, field_id))
    {
        cdb_addr_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_pim_intf->primary_address);
    }
    if (FLD_MATCH(TBL_IPMC_INTF_FLD_IGMP_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_pim_intf->igmp_enable);
    }
    if (FLD_MATCH(TBL_IPMC_INTF_FLD_PIM_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_pim_intf->pim_enable);
    }
    if (FLD_MATCH(TBL_IPMC_INTF_FLD_IGMP_VERSION, field_id))
    {
        sal_sprintf(str, "%d", p_pim_intf->igmp_version);
    }
    if (FLD_MATCH(TBL_IPMC_INTF_FLD_IGMP_QUERY_INTERVAL, field_id))
    {
        sal_sprintf(str, "%d", p_pim_intf->igmp_query_interval);
    }
    if (FLD_MATCH(TBL_IPMC_INTF_FLD_IGMP_QUERY_MAX_RESPONSE_TIME, field_id))
    {
        sal_sprintf(str, "%d", p_pim_intf->igmp_query_max_response_time);
    }
    if (FLD_MATCH(TBL_IPMC_INTF_FLD_PIM_HELLO_PERIOD, field_id))
    {
        sal_sprintf(str, "%d", p_pim_intf->pim_hello_period);
    }
    if (FLD_MATCH(TBL_IPMC_INTF_FLD_PIM_HELLO_HOLDTIME, field_id))
    {
        sal_sprintf(str, "%d", p_pim_intf->pim_hello_holdtime);
    }
    if (FLD_MATCH(TBL_IPMC_INTF_FLD_PIM_GENERATION_ID, field_id))
    {
        sal_sprintf(str, "%u", p_pim_intf->pim_generation_id);
    }
    if (FLD_MATCH(TBL_IPMC_INTF_FLD_PIM_PROPAGATION_DELAY_MSEC, field_id))
    {
        sal_sprintf(str, "%u", p_pim_intf->pim_propagation_delay_msec);
    }
    if (FLD_MATCH(TBL_IPMC_INTF_FLD_PIM_OVERRIDE_INTERVAL_MSEC, field_id))
    {
        sal_sprintf(str, "%u", p_pim_intf->pim_override_interval_msec);
    }
    if (FLD_MATCH(TBL_IPMC_INTF_FLD_PIM_DR_PRIORITY, field_id))
    {
        sal_sprintf(str, "%u", p_pim_intf->pim_dr_priority);
    }
    return str;
}

char**
tbl_ipmc_intf_table_dump(tbl_ipmc_intf_t *p_pim_intf, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_ipmc_intf_key_name_dump(p_pim_intf, buf));
    for(i=1; i<TBL_IPMC_INTF_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_ipmc_intf_field_name_dump(p_pim_intf, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_ipmc_intf_field_value_dump(p_pim_intf, i, buf));
    }
    return str;
}

int32
tbl_ipmc_intf_field_value_parser(char *str, int32 field_id, tbl_ipmc_intf_t *p_pim_intf)
{
    if (FLD_MATCH(TBL_IPMC_INTF_FLD_KEY, field_id))
    {
        sal_strcpy(p_pim_intf->key.name, str);
    }
    if (FLD_MATCH(TBL_IPMC_INTF_FLD_PRIMARY_ADDRESS, field_id))
    {
        cdb_addr_ipv4_str2val(&p_pim_intf->primary_address, str, 0);
    }
    if (FLD_MATCH(TBL_IPMC_INTF_FLD_IGMP_ENABLE, field_id))
    {
        int32 ret;
        p_pim_intf->igmp_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPMC_INTF_FLD_PIM_ENABLE, field_id))
    {
        int32 ret;
        p_pim_intf->pim_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPMC_INTF_FLD_IGMP_VERSION, field_id))
    {
    }
    if (FLD_MATCH(TBL_IPMC_INTF_FLD_IGMP_QUERY_INTERVAL, field_id))
    {
    }
    if (FLD_MATCH(TBL_IPMC_INTF_FLD_IGMP_QUERY_MAX_RESPONSE_TIME, field_id))
    {
    }
    if (FLD_MATCH(TBL_IPMC_INTF_FLD_PIM_HELLO_PERIOD, field_id))
    {
    }
    if (FLD_MATCH(TBL_IPMC_INTF_FLD_PIM_HELLO_HOLDTIME, field_id))
    {
    }
    if (FLD_MATCH(TBL_IPMC_INTF_FLD_PIM_GENERATION_ID, field_id))
    {
        int32 ret;
        p_pim_intf->pim_generation_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPMC_INTF_FLD_PIM_PROPAGATION_DELAY_MSEC, field_id))
    {
        int32 ret;
        p_pim_intf->pim_propagation_delay_msec = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPMC_INTF_FLD_PIM_OVERRIDE_INTERVAL_MSEC, field_id))
    {
        int32 ret;
        p_pim_intf->pim_override_interval_msec = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPMC_INTF_FLD_PIM_DR_PRIORITY, field_id))
    {
        int32 ret;
        p_pim_intf->pim_dr_priority = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_ipmc_intf_table_parser(char** array, char* key_value,tbl_ipmc_intf_t *p_pim_intf)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_IPMC_INTF);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_ipmc_intf_key_str2val(key_value, p_pim_intf));

    for(i=1; i<TBL_IPMC_INTF_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_ipmc_intf_field_value_parser( array[j++], i, p_pim_intf));
    }

    return PM_E_NONE;
}

int32
tbl_ipmc_intf_dump_one(tbl_ipmc_intf_t *p_pim_intf, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_IPMC_INTF);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_IPMC_INTF_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IPMC_INTF_FLD_KEY].name,
            p_pim_intf->key.name);
    }
    if (FLD_MATCH(TBL_IPMC_INTF_FLD_PRIMARY_ADDRESS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IPMC_INTF_FLD_PRIMARY_ADDRESS].name, 
            cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_pim_intf->primary_address));
    }
    if (FLD_MATCH(TBL_IPMC_INTF_FLD_IGMP_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPMC_INTF_FLD_IGMP_ENABLE].name,
            p_pim_intf->igmp_enable);
    }
    if (FLD_MATCH(TBL_IPMC_INTF_FLD_PIM_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPMC_INTF_FLD_PIM_ENABLE].name,
            p_pim_intf->pim_enable);
    }
    if (FLD_MATCH(TBL_IPMC_INTF_FLD_IGMP_VERSION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_IPMC_INTF_FLD_IGMP_VERSION].name,
            p_pim_intf->igmp_version);
    }
    if (FLD_MATCH(TBL_IPMC_INTF_FLD_IGMP_QUERY_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_IPMC_INTF_FLD_IGMP_QUERY_INTERVAL].name,
            p_pim_intf->igmp_query_interval);
    }
    if (FLD_MATCH(TBL_IPMC_INTF_FLD_IGMP_QUERY_MAX_RESPONSE_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_IPMC_INTF_FLD_IGMP_QUERY_MAX_RESPONSE_TIME].name,
            p_pim_intf->igmp_query_max_response_time);
    }
    if (FLD_MATCH(TBL_IPMC_INTF_FLD_PIM_HELLO_PERIOD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_IPMC_INTF_FLD_PIM_HELLO_PERIOD].name,
            p_pim_intf->pim_hello_period);
    }
    if (FLD_MATCH(TBL_IPMC_INTF_FLD_PIM_HELLO_HOLDTIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_IPMC_INTF_FLD_PIM_HELLO_HOLDTIME].name,
            p_pim_intf->pim_hello_holdtime);
    }
    if (FLD_MATCH(TBL_IPMC_INTF_FLD_PIM_GENERATION_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPMC_INTF_FLD_PIM_GENERATION_ID].name,
            p_pim_intf->pim_generation_id);
    }
    if (FLD_MATCH(TBL_IPMC_INTF_FLD_PIM_PROPAGATION_DELAY_MSEC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPMC_INTF_FLD_PIM_PROPAGATION_DELAY_MSEC].name,
            p_pim_intf->pim_propagation_delay_msec);
    }
    if (FLD_MATCH(TBL_IPMC_INTF_FLD_PIM_OVERRIDE_INTERVAL_MSEC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPMC_INTF_FLD_PIM_OVERRIDE_INTERVAL_MSEC].name,
            p_pim_intf->pim_override_interval_msec);
    }
    if (FLD_MATCH(TBL_IPMC_INTF_FLD_PIM_DR_PRIORITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPMC_INTF_FLD_PIM_DR_PRIORITY].name,
            p_pim_intf->pim_dr_priority);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_ipmc_intf_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_ipmc_intf_master_t *p_master = _g_p_tbl_ipmc_intf_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_ipmc_intf_t *p_db_pim_intf = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->pim_intf_list, p_db_pim_intf, listnode, next)
    {
        rc |= fn(p_db_pim_intf, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_ipmc_intf_master_t*
tbl_ipmc_intf_get_master()
{
    return _g_p_tbl_ipmc_intf_master;
}

tbl_ipmc_intf_master_t*
tbl_ipmc_intf_init_ipmc_intf()
{
    _g_p_tbl_ipmc_intf_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_ipmc_intf_master_t));
    _g_p_tbl_ipmc_intf_master->pim_intf_hash = ctclib_hash_create(_tbl_ipmc_intf_hash_make, _tbl_ipmc_intf_hash_cmp);
    _g_p_tbl_ipmc_intf_master->pim_intf_list = ctclib_slist_create(tbl_ipmc_intf_cmp, NULL);
    return _g_p_tbl_ipmc_intf_master;
}

