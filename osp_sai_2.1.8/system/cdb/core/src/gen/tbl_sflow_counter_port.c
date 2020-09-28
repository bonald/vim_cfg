
#include "proto.h"
#include "gen/tbl_sflow_counter_port_define.h"
#include "gen/tbl_sflow_counter_port.h"
#include "cdb_data_cmp.h"

tbl_sflow_counter_port_master_t *_g_p_tbl_sflow_counter_port_master = NULL;

int32
tbl_sflow_counter_port_add_sflow_counter_port_sync(tbl_sflow_counter_port_t *p_sflow_counter_port, uint32 sync)
{
    tbl_sflow_counter_port_master_t *p_master = _g_p_tbl_sflow_counter_port_master;
    tbl_sflow_counter_port_t *p_db_sflow_counter_port = NULL;

    /* 1. lookup entry exist */
    if (tbl_sflow_counter_port_get_sflow_counter_port(&p_sflow_counter_port->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_sflow_counter_port = XCALLOC(MEM_TBL_SFLOW_COUNTER_PORT, sizeof(tbl_sflow_counter_port_t));
    if (NULL == p_db_sflow_counter_port)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_sflow_counter_port, p_sflow_counter_port, sizeof(tbl_sflow_counter_port_t));

    /* 4. add to db */
    ctclib_slistnode_insert_sort(p_master->counter_port_list, p_db_sflow_counter_port);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_SFLOW_COUNTER_PORT, p_db_sflow_counter_port);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_SFLOW_COUNTER_PORT, p_db_sflow_counter_port);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_sflow_counter_port_del_sflow_counter_port_sync(tbl_sflow_counter_port_key_t *p_sflow_counter_port_key, uint32 sync)
{
    tbl_sflow_counter_port_master_t *p_master = _g_p_tbl_sflow_counter_port_master;
    tbl_sflow_counter_port_t *p_db_sflow_counter_port = NULL;

    /* 1. lookup entry exist */
    p_db_sflow_counter_port = tbl_sflow_counter_port_get_sflow_counter_port(p_sflow_counter_port_key);
    if (NULL == p_db_sflow_counter_port)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_SFLOW_COUNTER_PORT, p_db_sflow_counter_port);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_SFLOW_COUNTER_PORT, p_db_sflow_counter_port);
        #endif 
    }

    /* 3. delete from db */
    ctclib_slistnode_delete(p_master->counter_port_list, p_db_sflow_counter_port);

    /* 4. free db entry */
    XFREE(MEM_TBL_SFLOW_COUNTER_PORT, p_db_sflow_counter_port);

    return PM_E_NONE;
}

int32
tbl_sflow_counter_port_set_sflow_counter_port_field_sync(tbl_sflow_counter_port_t *p_sflow_counter_port, tbl_sflow_counter_port_field_id_t field_id, uint32 sync)
{
    tbl_sflow_counter_port_t *p_db_sflow_counter_port = NULL;

    /* 1. lookup entry exist */
    p_db_sflow_counter_port = tbl_sflow_counter_port_get_sflow_counter_port(&p_sflow_counter_port->key);
    if (NULL == p_db_sflow_counter_port)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_SFLOW_COUNTER_PORT_FLD_LEFT_INTERVAL:
        p_db_sflow_counter_port->left_interval = p_sflow_counter_port->left_interval;
        break;

    case TBL_SFLOW_COUNTER_PORT_FLD_SEQUENCE:
        p_db_sflow_counter_port->sequence = p_sflow_counter_port->sequence;
        break;

    case TBL_SFLOW_COUNTER_PORT_FLD_MAX:
        sal_memcpy(p_db_sflow_counter_port, p_sflow_counter_port, sizeof(tbl_sflow_counter_port_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_SFLOW_COUNTER_PORT, field_id, p_db_sflow_counter_port);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_SFLOW_COUNTER_PORT, field_id, p_db_sflow_counter_port);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_sflow_counter_port_add_sflow_counter_port(tbl_sflow_counter_port_t *p_sflow_counter_port)
{
    return tbl_sflow_counter_port_add_sflow_counter_port_sync(p_sflow_counter_port, TRUE);
}

int32
tbl_sflow_counter_port_del_sflow_counter_port(tbl_sflow_counter_port_key_t *p_sflow_counter_port_key)
{
    return tbl_sflow_counter_port_del_sflow_counter_port_sync(p_sflow_counter_port_key, TRUE);
}

int32
tbl_sflow_counter_port_set_sflow_counter_port_field(tbl_sflow_counter_port_t *p_sflow_counter_port, tbl_sflow_counter_port_field_id_t field_id)
{
    return tbl_sflow_counter_port_set_sflow_counter_port_field_sync(p_sflow_counter_port, field_id, TRUE);
}

tbl_sflow_counter_port_t*
tbl_sflow_counter_port_get_sflow_counter_port(tbl_sflow_counter_port_key_t *p_sflow_counter_port_key)
{
    tbl_sflow_counter_port_master_t *p_master = _g_p_tbl_sflow_counter_port_master;
    tbl_sflow_counter_port_t lkp;

    sal_memcpy(&lkp.key, p_sflow_counter_port_key, sizeof(tbl_sflow_counter_port_key_t));
    return ctclib_slistdata_lookup(p_master->counter_port_list, &lkp);
}

char*
tbl_sflow_counter_port_key_val2str(tbl_sflow_counter_port_t *p_sflow_counter_port, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_sflow_counter_port->key.ifindex);
    return str;
}

int32
tbl_sflow_counter_port_key_str2val(char *str, tbl_sflow_counter_port_t *p_sflow_counter_port)
{
    int32 ret = 0;

    p_sflow_counter_port->key.ifindex = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_sflow_counter_port_key_name_dump(tbl_sflow_counter_port_t *p_sflow_counter_port, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SFLOW_COUNTER_PORT);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_sflow_counter_port->key.ifindex);
    return str;
}

char*
tbl_sflow_counter_port_key_value_dump(tbl_sflow_counter_port_t *p_sflow_counter_port, char *str)
{
    sal_sprintf(str, "%u", p_sflow_counter_port->key.ifindex);
    return str;
}

char*
tbl_sflow_counter_port_field_name_dump(tbl_sflow_counter_port_t *p_sflow_counter_port, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_SFLOW_COUNTER_PORT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_SFLOW_COUNTER_PORT_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_sflow_counter_port_field_value_dump(tbl_sflow_counter_port_t *p_sflow_counter_port, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_SFLOW_COUNTER_PORT_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_sflow_counter_port->key.ifindex);
    }
    if (FLD_MATCH(TBL_SFLOW_COUNTER_PORT_FLD_LEFT_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_sflow_counter_port->left_interval);
    }
    if (FLD_MATCH(TBL_SFLOW_COUNTER_PORT_FLD_SEQUENCE, field_id))
    {
        sal_sprintf(str, "%u", p_sflow_counter_port->sequence);
    }
    return str;
}

char**
tbl_sflow_counter_port_table_dump(tbl_sflow_counter_port_t *p_sflow_counter_port, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_sflow_counter_port_key_name_dump(p_sflow_counter_port, buf));
    for(i=1; i<TBL_SFLOW_COUNTER_PORT_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_sflow_counter_port_field_name_dump(p_sflow_counter_port, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_sflow_counter_port_field_value_dump(p_sflow_counter_port, i, buf));
    }
    return str;
}

int32
tbl_sflow_counter_port_field_value_parser(char *str, int32 field_id, tbl_sflow_counter_port_t *p_sflow_counter_port)
{
    if (FLD_MATCH(TBL_SFLOW_COUNTER_PORT_FLD_KEY, field_id))
    {
        int32 ret;
        p_sflow_counter_port->key.ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SFLOW_COUNTER_PORT_FLD_LEFT_INTERVAL, field_id))
    {
        int32 ret;
        p_sflow_counter_port->left_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SFLOW_COUNTER_PORT_FLD_SEQUENCE, field_id))
    {
        int32 ret;
        p_sflow_counter_port->sequence = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_sflow_counter_port_table_parser(char** array, char* key_value,tbl_sflow_counter_port_t *p_sflow_counter_port)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_SFLOW_COUNTER_PORT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_sflow_counter_port_key_str2val(key_value, p_sflow_counter_port));

    for(i=1; i<TBL_SFLOW_COUNTER_PORT_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_sflow_counter_port_field_value_parser( array[j++], i, p_sflow_counter_port));
    }

    return PM_E_NONE;
}

int32
tbl_sflow_counter_port_dump_one(tbl_sflow_counter_port_t *p_sflow_counter_port, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SFLOW_COUNTER_PORT);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_SFLOW_COUNTER_PORT_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SFLOW_COUNTER_PORT_FLD_KEY].name,
            p_sflow_counter_port->key.ifindex);
    }
    if (FLD_MATCH(TBL_SFLOW_COUNTER_PORT_FLD_LEFT_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SFLOW_COUNTER_PORT_FLD_LEFT_INTERVAL].name,
            p_sflow_counter_port->left_interval);
    }
    if (FLD_MATCH(TBL_SFLOW_COUNTER_PORT_FLD_SEQUENCE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SFLOW_COUNTER_PORT_FLD_SEQUENCE].name,
            p_sflow_counter_port->sequence);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_sflow_counter_port_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_sflow_counter_port_master_t *p_master = _g_p_tbl_sflow_counter_port_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_sflow_counter_port_t *p_db_sflow_counter_port = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->counter_port_list, p_db_sflow_counter_port, listnode, next)
    {
        rc |= fn(p_db_sflow_counter_port, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_sflow_counter_port_master_t*
tbl_sflow_counter_port_get_master()
{
    return _g_p_tbl_sflow_counter_port_master;
}

tbl_sflow_counter_port_master_t*
tbl_sflow_counter_port_init_sflow_counter_port()
{
    _g_p_tbl_sflow_counter_port_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_sflow_counter_port_master_t));
    _g_p_tbl_sflow_counter_port_master->counter_port_list = ctclib_slist_create(tbl_sflow_counter_port_cmp, NULL);
    return _g_p_tbl_sflow_counter_port_master;
}

