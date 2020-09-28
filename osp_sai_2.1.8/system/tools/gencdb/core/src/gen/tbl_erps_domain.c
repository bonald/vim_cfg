
#include "proto.h"
#include "gen/tbl_erps_domain_define.h"
#include "gen/tbl_erps_domain.h"
#include "gen/tbl_erps_ring_define.h"
#include "gen/tbl_erps_ring.h"
#include "cdb_data_cmp.h"

tbl_erps_domain_master_t *_g_p_tbl_erps_domain_master = NULL;

int32
tbl_erps_domain_add_erps_domain_sync(tbl_erps_domain_t *p_erps_domain, uint32 sync)
{
    tbl_erps_domain_master_t *p_master = _g_p_tbl_erps_domain_master;
    tbl_erps_domain_t *p_db_erps_domain = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    if (tbl_erps_domain_get_erps_domain(&p_erps_domain->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_erps_domain = XCALLOC(MEM_TBL_ERPS_DOMAIN, sizeof(tbl_erps_domain_t));
    if (NULL == p_db_erps_domain)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_erps_domain, p_erps_domain, sizeof(tbl_erps_domain_t));
    PM_E_RETURN(cdb_reference_list_init(&p_db_erps_domain->erps_ring_list, CDB_NODE_TBL, TBL_ERPS_RING, tbl_erps_ring_cmp));

    /* 4. add to db */
    p_master->domain_array[p_erps_domain->key.domain_id] = p_db_erps_domain;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_ERPS_DOMAIN, p_db_erps_domain);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_ERPS_DOMAIN, p_db_erps_domain);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_erps_domain_del_erps_domain_sync(tbl_erps_domain_key_t *p_erps_domain_key, uint32 sync)
{
    tbl_erps_domain_master_t *p_master = _g_p_tbl_erps_domain_master;
    tbl_erps_domain_t *p_db_erps_domain = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_erps_domain = tbl_erps_domain_get_erps_domain(p_erps_domain_key);
    if (NULL == p_db_erps_domain)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_ERPS_DOMAIN, p_db_erps_domain);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_ERPS_DOMAIN, p_db_erps_domain);
        #endif 
    }

    /* 3. delete from db */
    p_master->domain_array[p_erps_domain_key->domain_id] = NULL;
    PM_E_RETURN(cdb_reference_list_deinit(&p_db_erps_domain->erps_ring_list));

    /* 4. free db entry */
    XFREE(MEM_TBL_ERPS_DOMAIN, p_db_erps_domain);

    return PM_E_NONE;
}

int32
tbl_erps_domain_set_erps_domain_field_sync(tbl_erps_domain_t *p_erps_domain, tbl_erps_domain_field_id_t field_id, uint32 sync)
{
    tbl_erps_domain_t *p_db_erps_domain = NULL;

    /* 1. lookup entry exist */
    p_db_erps_domain = tbl_erps_domain_get_erps_domain(&p_erps_domain->key);
    if (NULL == p_db_erps_domain)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_ERPS_DOMAIN_FLD_NAME:
        sal_memcpy(p_db_erps_domain->name, p_erps_domain->name, sizeof(p_erps_domain->name));
        break;

    case TBL_ERPS_DOMAIN_FLD_INSTANCE_ID_NUM:
        p_db_erps_domain->instance_id_num = p_erps_domain->instance_id_num;
        break;

    case TBL_ERPS_DOMAIN_FLD_INSTANCE_ID:
        sal_memcpy(p_db_erps_domain->instance_id, p_erps_domain->instance_id, sizeof(p_erps_domain->instance_id));
        break;

    case TBL_ERPS_DOMAIN_FLD_PRIMARY_CONTROL_VLAN_ID:
        p_db_erps_domain->primary_control_vlan_id = p_erps_domain->primary_control_vlan_id;
        break;

    case TBL_ERPS_DOMAIN_FLD_SUB_CONTROL_VLAN_ID:
        p_db_erps_domain->sub_control_vlan_id = p_erps_domain->sub_control_vlan_id;
        break;

    case TBL_ERPS_DOMAIN_FLD_HELLO_TIMER_INTERVAL:
        p_db_erps_domain->hello_timer_interval = p_erps_domain->hello_timer_interval;
        break;

    case TBL_ERPS_DOMAIN_FLD_FAILURE_TIMER_INTERVAL:
        p_db_erps_domain->failure_timer_interval = p_erps_domain->failure_timer_interval;
        break;

    case TBL_ERPS_DOMAIN_FLD_PRE_FORWARDING_TIMER_INTERVAL:
        p_db_erps_domain->pre_forwarding_timer_interval = p_erps_domain->pre_forwarding_timer_interval;
        break;

    case TBL_ERPS_DOMAIN_FLD_FLUSH_TIMER_INTERVAL:
        p_db_erps_domain->flush_timer_interval = p_erps_domain->flush_timer_interval;
        break;

    case TBL_ERPS_DOMAIN_FLD_EDGE_HELLO_TIMER_INTERVAL:
        p_db_erps_domain->edge_hello_timer_interval = p_erps_domain->edge_hello_timer_interval;
        break;

    case TBL_ERPS_DOMAIN_FLD_EDGE_FAILURE_TIMER_INTERVAL:
        p_db_erps_domain->edge_failure_timer_interval = p_erps_domain->edge_failure_timer_interval;
        break;

    case TBL_ERPS_DOMAIN_FLD_ERPS_ENABLE:
        p_db_erps_domain->erps_enable = p_erps_domain->erps_enable;
        break;

    case TBL_ERPS_DOMAIN_FLD_MAX:
        sal_memcpy(p_db_erps_domain, p_erps_domain, sizeof(tbl_erps_domain_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_ERPS_DOMAIN, field_id, p_db_erps_domain);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_ERPS_DOMAIN, field_id, p_db_erps_domain);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_erps_domain_add_erps_domain(tbl_erps_domain_t *p_erps_domain)
{
    return tbl_erps_domain_add_erps_domain_sync(p_erps_domain, TRUE);
}

int32
tbl_erps_domain_del_erps_domain(tbl_erps_domain_key_t *p_erps_domain_key)
{
    return tbl_erps_domain_del_erps_domain_sync(p_erps_domain_key, TRUE);
}

int32
tbl_erps_domain_set_erps_domain_field(tbl_erps_domain_t *p_erps_domain, tbl_erps_domain_field_id_t field_id)
{
    return tbl_erps_domain_set_erps_domain_field_sync(p_erps_domain, field_id, TRUE);
}

tbl_erps_domain_t*
tbl_erps_domain_get_erps_domain(tbl_erps_domain_key_t *p_erps_domain_key)
{
    tbl_erps_domain_master_t *p_master = _g_p_tbl_erps_domain_master;

    if (GLB_MAX_ERPS_DOMAIN <= p_erps_domain_key->domain_id)
    {
        return NULL;
    }
    return (p_master->domain_array[p_erps_domain_key->domain_id]);
}

char*
tbl_erps_domain_key_val2str(tbl_erps_domain_t *p_erps_domain, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_erps_domain->key.domain_id);
    return str;
}

int32
tbl_erps_domain_key_str2val(char *str, tbl_erps_domain_t *p_erps_domain)
{
    int32 ret = 0;

    p_erps_domain->key.domain_id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_erps_domain_key_name_dump(tbl_erps_domain_t *p_erps_domain, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ERPS_DOMAIN);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_erps_domain->key.domain_id);
    return str;
}

char*
tbl_erps_domain_key_value_dump(tbl_erps_domain_t *p_erps_domain, char *str)
{
    sal_sprintf(str, "%u", p_erps_domain->key.domain_id);
    return str;
}

char*
tbl_erps_domain_field_name_dump(tbl_erps_domain_t *p_erps_domain, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_ERPS_DOMAIN);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_ERPS_DOMAIN_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_erps_domain_field_value_dump(tbl_erps_domain_t *p_erps_domain, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_erps_domain->key.domain_id);
    }
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_erps_domain->name);
    }
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_INSTANCE_ID_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_erps_domain->instance_id_num);
    }
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_INSTANCE_ID, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_erps_domain->instance_id, sizeof(p_erps_domain->instance_id)/4);
    }
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_PRIMARY_CONTROL_VLAN_ID, field_id))
    {
        sal_sprintf(str, "%u", p_erps_domain->primary_control_vlan_id);
    }
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_SUB_CONTROL_VLAN_ID, field_id))
    {
        sal_sprintf(str, "%u", p_erps_domain->sub_control_vlan_id);
    }
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_HELLO_TIMER_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_erps_domain->hello_timer_interval);
    }
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_FAILURE_TIMER_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_erps_domain->failure_timer_interval);
    }
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_PRE_FORWARDING_TIMER_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_erps_domain->pre_forwarding_timer_interval);
    }
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_FLUSH_TIMER_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_erps_domain->flush_timer_interval);
    }
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_EDGE_HELLO_TIMER_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_erps_domain->edge_hello_timer_interval);
    }
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_EDGE_FAILURE_TIMER_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_erps_domain->edge_failure_timer_interval);
    }
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_ERPS_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_erps_domain->erps_enable);
    }
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_ERPS_RING_LIST, field_id))
    {
    }
    return str;
}

char**
tbl_erps_domain_table_dump(tbl_erps_domain_t *p_erps_domain, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_erps_domain_key_name_dump(p_erps_domain, buf));
    for(i=1; i<TBL_ERPS_DOMAIN_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_erps_domain_field_name_dump(p_erps_domain, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_erps_domain_field_value_dump(p_erps_domain, i, buf));
    }
    return str;
}

int32
tbl_erps_domain_field_value_parser(char *str, int32 field_id, tbl_erps_domain_t *p_erps_domain)
{
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_KEY, field_id))
    {
        int32 ret;
        p_erps_domain->key.domain_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_NAME, field_id))
    {
        sal_strcpy(p_erps_domain->name, str);
    }
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_INSTANCE_ID_NUM, field_id))
    {
        int32 ret;
        p_erps_domain->instance_id_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_INSTANCE_ID, field_id))
    {
        cdb_uint32_array_str2val(str, p_erps_domain->instance_id);
    }
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_PRIMARY_CONTROL_VLAN_ID, field_id))
    {
        int32 ret;
        p_erps_domain->primary_control_vlan_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_SUB_CONTROL_VLAN_ID, field_id))
    {
        int32 ret;
        p_erps_domain->sub_control_vlan_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_HELLO_TIMER_INTERVAL, field_id))
    {
        int32 ret;
        p_erps_domain->hello_timer_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_FAILURE_TIMER_INTERVAL, field_id))
    {
        int32 ret;
        p_erps_domain->failure_timer_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_PRE_FORWARDING_TIMER_INTERVAL, field_id))
    {
        int32 ret;
        p_erps_domain->pre_forwarding_timer_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_FLUSH_TIMER_INTERVAL, field_id))
    {
        int32 ret;
        p_erps_domain->flush_timer_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_EDGE_HELLO_TIMER_INTERVAL, field_id))
    {
        int32 ret;
        p_erps_domain->edge_hello_timer_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_EDGE_FAILURE_TIMER_INTERVAL, field_id))
    {
        int32 ret;
        p_erps_domain->edge_failure_timer_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_ERPS_ENABLE, field_id))
    {
        int32 ret;
        p_erps_domain->erps_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_ERPS_RING_LIST, field_id))
    {
    }
    return PM_E_NONE;
}

int32
tbl_erps_domain_table_parser(char** array, char* key_value,tbl_erps_domain_t *p_erps_domain)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_ERPS_DOMAIN);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_erps_domain_key_str2val(key_value, p_erps_domain));

    for(i=1; i<TBL_ERPS_DOMAIN_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_erps_domain_field_value_parser( array[j++], i, p_erps_domain));
    }

    return PM_E_NONE;
}

int32
tbl_erps_domain_dump_one(tbl_erps_domain_t *p_erps_domain, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ERPS_DOMAIN);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERPS_DOMAIN_FLD_KEY].name,
            p_erps_domain->key.domain_id);
    }
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ERPS_DOMAIN_FLD_NAME].name,
            p_erps_domain->name);
    }
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_INSTANCE_ID_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERPS_DOMAIN_FLD_INSTANCE_ID_NUM].name,
            p_erps_domain->instance_id_num);
    }
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_INSTANCE_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ERPS_DOMAIN_FLD_INSTANCE_ID].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_erps_domain->instance_id, sizeof(p_erps_domain->instance_id)/4));
    }
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_PRIMARY_CONTROL_VLAN_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERPS_DOMAIN_FLD_PRIMARY_CONTROL_VLAN_ID].name,
            p_erps_domain->primary_control_vlan_id);
    }
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_SUB_CONTROL_VLAN_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERPS_DOMAIN_FLD_SUB_CONTROL_VLAN_ID].name,
            p_erps_domain->sub_control_vlan_id);
    }
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_HELLO_TIMER_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERPS_DOMAIN_FLD_HELLO_TIMER_INTERVAL].name,
            p_erps_domain->hello_timer_interval);
    }
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_FAILURE_TIMER_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERPS_DOMAIN_FLD_FAILURE_TIMER_INTERVAL].name,
            p_erps_domain->failure_timer_interval);
    }
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_PRE_FORWARDING_TIMER_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERPS_DOMAIN_FLD_PRE_FORWARDING_TIMER_INTERVAL].name,
            p_erps_domain->pre_forwarding_timer_interval);
    }
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_FLUSH_TIMER_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERPS_DOMAIN_FLD_FLUSH_TIMER_INTERVAL].name,
            p_erps_domain->flush_timer_interval);
    }
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_EDGE_HELLO_TIMER_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERPS_DOMAIN_FLD_EDGE_HELLO_TIMER_INTERVAL].name,
            p_erps_domain->edge_hello_timer_interval);
    }
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_EDGE_FAILURE_TIMER_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERPS_DOMAIN_FLD_EDGE_FAILURE_TIMER_INTERVAL].name,
            p_erps_domain->edge_failure_timer_interval);
    }
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_ERPS_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERPS_DOMAIN_FLD_ERPS_ENABLE].name,
            p_erps_domain->erps_enable);
    }
    if (FLD_MATCH(TBL_ERPS_DOMAIN_FLD_ERPS_RING_LIST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_node->tbl_info->field[TBL_ERPS_DOMAIN_FLD_ERPS_RING_LIST].name,
            cdb_reference_list_val2str(buf, MAX_CMD_STR_LEN, &p_erps_domain->erps_ring_list));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_erps_domain_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_erps_domain_master_t *p_master = _g_p_tbl_erps_domain_master;
    tbl_erps_domain_t *p_db_erps_domain = NULL;
    uint32 domain_id = 0;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    for (domain_id = 0; domain_id < GLB_MAX_ERPS_DOMAIN; domain_id++)
    {
        p_db_erps_domain = p_master->domain_array[domain_id];
        if (NULL == p_db_erps_domain)
        {
            continue;
        }
        rc |= fn(p_db_erps_domain, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_erps_domain_master_t*
tbl_erps_domain_get_master()
{
    return _g_p_tbl_erps_domain_master;
}

tbl_erps_domain_master_t*
tbl_erps_domain_init_erps_domain()
{
    _g_p_tbl_erps_domain_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_erps_domain_master_t));
    return _g_p_tbl_erps_domain_master;
}

int32
tbl_erps_domain_set_erps_ring_list_sync(tbl_erps_domain_t *p_erps_domain, uint32 add, void *p_object, uint32 sync)
{
    cdb_sync_refer_hdr_t refer;
    tbl_erps_domain_t *p_db_erps_domain = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_erps_domain = tbl_erps_domain_get_erps_domain(&p_erps_domain->key);
    if (NULL == p_db_erps_domain)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    if (add)
    {
        PM_E_RETURN(cdb_reference_list_add_object(&p_db_erps_domain->erps_ring_list, p_object));
    }
    else
    {
        PM_E_RETURN(cdb_reference_list_del_object(&p_db_erps_domain->erps_ring_list, p_object));
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_build_refer_tbl(TBL_ERPS_RING, add, &refer);
        cdb_sync_tbl_set_refer(TBL_ERPS_DOMAIN, TBL_ERPS_DOMAIN_FLD_ERPS_RING_LIST, p_erps_domain, &refer, p_object);
    }

    return PM_E_NONE;
}

int32
tbl_erps_domain_set_erps_ring_list(tbl_erps_domain_t *p_erps_domain, uint32 add, void *p_object)
{
    return tbl_erps_domain_set_erps_ring_list_sync(p_erps_domain, add, p_object, TRUE);
}

