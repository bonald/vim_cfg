
#include "proto.h"
#include "gen/tbl_igsp_intf_define.h"
#include "gen/tbl_igsp_intf.h"
#include "cdb_data_cmp.h"

int32
ds_mrouter_port_add_mrouter_port_sync(tbl_igsp_intf_t *p_if, ds_mrouter_port_t *p_mr_port, uint32 sync)
{
    tbl_igsp_intf_t *p_db_if = NULL;
    ds_mrouter_port_t *p_db_mr_port = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_if = tbl_igsp_intf_get_igsp_intf(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_mr_port = ds_mrouter_port_get_mrouter_port(p_if, p_mr_port);
    if (p_db_mr_port)
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_mr_port = XCALLOC(MEM_DS_MROUTER_PORT, sizeof(ds_mrouter_port_t));
    if (NULL == p_db_mr_port)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_mr_port, p_mr_port, sizeof(ds_mrouter_port_t));

    /* 4. add to db */
    PM_E_RETURN(cdb_reference_list_add_object(&p_db_if->mrouter_list, p_db_mr_port));

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_add(TBL_IGSP_INTF, DS_MROUTER_PORT, p_db_if, p_db_mr_port);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_add(TBL_IGSP_INTF, DS_MROUTER_PORT, p_db_if, p_db_mr_port);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_mrouter_port_del_mrouter_port_sync(tbl_igsp_intf_t *p_if, ds_mrouter_port_t *p_mr_port, uint32 sync)
{
    tbl_igsp_intf_t *p_db_if = NULL;
    ds_mrouter_port_t *p_db_mr_port = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_if = tbl_igsp_intf_get_igsp_intf(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_mr_port = ds_mrouter_port_get_mrouter_port(p_if, p_mr_port);
    if (NULL == p_db_mr_port)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_del(TBL_IGSP_INTF, DS_MROUTER_PORT, p_db_if, p_db_mr_port);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_del(TBL_IGSP_INTF, DS_MROUTER_PORT, p_db_if, p_db_mr_port);
        #endif 
    }

    /* 3. delete from db */
    PM_E_RETURN(cdb_reference_list_del_object(&p_db_if->mrouter_list, p_db_mr_port));

    /* 4. free db entry */
    XFREE(MEM_DS_MROUTER_PORT, p_db_mr_port);

    return PM_E_NONE;
}

int32
ds_mrouter_port_set_mrouter_port_field_sync(tbl_igsp_intf_t *p_if, ds_mrouter_port_t *p_mr_port, ds_mrouter_port_field_id_t field_id, uint32 sync)
{
    tbl_igsp_intf_t *p_db_if = NULL;
    ds_mrouter_port_t *p_db_mr_port = NULL;

    /* 1. lookup entry exist */
    p_db_if = tbl_igsp_intf_get_igsp_intf(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }

    p_db_mr_port = ds_mrouter_port_get_mrouter_port(p_if, p_mr_port);
    if (NULL == p_db_mr_port)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case DS_MROUTER_PORT_FLD_TYPE:
        p_db_mr_port->type = p_mr_port->type;
        break;

    case DS_MROUTER_PORT_FLD_UPTIME:
        sal_memcpy(&p_db_mr_port->uptime, &p_mr_port->uptime, sizeof(p_mr_port->uptime));
        break;

    case DS_MROUTER_PORT_FLD_EXPIRE_TIMER:
        p_db_mr_port->expire_timer = p_mr_port->expire_timer;
        break;

    case DS_MROUTER_PORT_FLD_MAX:
        sal_memcpy(p_db_mr_port, p_mr_port, sizeof(ds_mrouter_port_t));
        break;

    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_set(TBL_IGSP_INTF, DS_MROUTER_PORT, field_id, p_db_if, p_db_mr_port);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_set(TBL_IGSP_INTF, DS_MROUTER_PORT, field_id, p_db_if, p_db_mr_port);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_mrouter_port_add_mrouter_port(tbl_igsp_intf_t *p_if, ds_mrouter_port_t *p_mr_port)
{
    return ds_mrouter_port_add_mrouter_port_sync(p_if, p_mr_port, TRUE);
}

int32
ds_mrouter_port_del_mrouter_port(tbl_igsp_intf_t *p_if, ds_mrouter_port_t *p_mr_port)
{
    return ds_mrouter_port_del_mrouter_port_sync(p_if, p_mr_port, TRUE);
}

int32
ds_mrouter_port_set_mrouter_port_field(tbl_igsp_intf_t *p_if, ds_mrouter_port_t *p_mr_port, ds_mrouter_port_field_id_t field_id)
{
    return ds_mrouter_port_set_mrouter_port_field_sync(p_if, p_mr_port, field_id, TRUE);
}

ds_mrouter_port_t*
ds_mrouter_port_get_mrouter_port(tbl_igsp_intf_t *p_if, ds_mrouter_port_t *p_mr_port)
{
    tbl_igsp_intf_t *p_db_if = NULL;

    /* 1. lookup entry exist */
    p_db_if = tbl_igsp_intf_get_igsp_intf(&p_if->key);
    if (NULL == p_db_if)
    {
        return NULL;
    }

    return cdb_reference_list_lookup_object(&p_db_if->mrouter_list, p_mr_port);
}

int32
ds_mrouter_port_key_str2val(char* tbl_key_value, char* str, tbl_igsp_intf_t *p_if, ds_mrouter_port_t *p_mr_port)
{

    int32 ret = 0;

    ret = tbl_igsp_intf_key_str2val(tbl_key_value, p_if);

    sal_strcpy(p_mr_port->key.name, str);

    return ret;
}

char*
ds_mrouter_port_key_value_dump(tbl_igsp_intf_t *p_if, ds_mrouter_port_t *p_mr_port, char* str)
{

    sal_sprintf(str, "%s", p_mr_port->key.name);
    return str;
}

char*
ds_mrouter_port_key_name_dump(tbl_igsp_intf_t *p_if, ds_mrouter_port_t *p_mr_port, char* str)
{

    cdb_node_t *p_node_tbl = cdb_get_tbl(TBL_IGSP_INTF);
    cdb_node_t *p_node_ds = cdb_get_ds(DS_MROUTER_PORT);
    char tbl_str[MAX_CMD_STR_LEN];
    char ds_str[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s",
        p_node_tbl->name, tbl_igsp_intf_key_value_dump(p_if, tbl_str), p_node_ds->name, ds_mrouter_port_key_value_dump(p_if, p_mr_port, ds_str));
    return str;
}

char*
ds_mrouter_port_field_name_dump(tbl_igsp_intf_t *p_if, ds_mrouter_port_t *p_mr_port, int32 field_id, char* str)
{

    cdb_node_t *p_node = cdb_get_ds(DS_MROUTER_PORT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= DS_MROUTER_PORT_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
ds_mrouter_port_field_value_dump(tbl_igsp_intf_t *p_if, ds_mrouter_port_t *p_mr_port, int32 field_id, char* str)
{

    if (FLD_MATCH(DS_MROUTER_PORT_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_mr_port->key.name);
    }
    if (FLD_MATCH(DS_MROUTER_PORT_FLD_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_mr_port->type);
    }
    if (FLD_MATCH(DS_MROUTER_PORT_FLD_UPTIME, field_id))
    {
        cdb_sal_time_val2str(str, MAX_CMD_STR_LEN, &p_mr_port->uptime);
    }
    if (FLD_MATCH(DS_MROUTER_PORT_FLD_EXPIRE_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_mr_port->expire_timer);
    }
    return str;
}

char**
ds_mrouter_port_table_dump(tbl_igsp_intf_t *p_if, ds_mrouter_port_t *p_mr_port, char** str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         ds_mrouter_port_key_name_dump(p_if, p_mr_port, buf));
    for(i=1; i<DS_MROUTER_PORT_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            ds_mrouter_port_field_name_dump(p_if, p_mr_port, i, buf));
        sal_sprintf(str[j++], "%s",
            ds_mrouter_port_field_value_dump(p_if, p_mr_port, i, buf));
    }
    return str;
}

int32
ds_mrouter_port_field_value_parser(char *str, int32 field_id, tbl_igsp_intf_t *p_if, ds_mrouter_port_t *p_mr_port)
{
    int32 ret=0;
    if (FLD_MATCH(DS_MROUTER_PORT_FLD_KEY, field_id))
    {
        sal_strcpy(p_mr_port->key.name, str);
    }
    if (FLD_MATCH(DS_MROUTER_PORT_FLD_TYPE, field_id))
    {
        p_mr_port->type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_MROUTER_PORT_FLD_UPTIME, field_id))
    {
        cdb_sal_time_str2val(str, &p_mr_port->uptime);
    }
    if (FLD_MATCH(DS_MROUTER_PORT_FLD_EXPIRE_TIMER, field_id))
    {
        ctc_task_str2val(str, p_mr_port->expire_timer);
    }
    return ret;
}

int32
ds_mrouter_port_table_parser(char** array, char* tbl_key_value, char* ds_key_value,tbl_igsp_intf_t *p_if, ds_mrouter_port_t *p_mr_port)
{
    int32 rc;
    int32 i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_ds(DS_MROUTER_PORT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(ds_mrouter_port_key_str2val(tbl_key_value, ds_key_value, p_if, p_mr_port));

    for(i=1; i<DS_MROUTER_PORT_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(ds_mrouter_port_field_value_parser( array[j++], i, p_if, p_mr_port));
    }

    return PM_E_NONE;
}

int32
ds_mrouter_port_dump_one(ds_mrouter_port_t *p_mr_port, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_ds(DS_MROUTER_PORT);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(DS_MROUTER_PORT_FLD_KEY, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_MROUTER_PORT_FLD_KEY].name,
            p_mr_port->key.name);
    }
    if (FLD_MATCH(DS_MROUTER_PORT_FLD_TYPE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_MROUTER_PORT_FLD_TYPE].name,
            p_mr_port->type);
    }
    if (FLD_MATCH(DS_MROUTER_PORT_FLD_UPTIME, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_MROUTER_PORT_FLD_UPTIME].name,
            cdb_sal_time_val2str(buf, MAX_CMD_STR_LEN, &p_mr_port->uptime));
    }
    if (FLD_MATCH(DS_MROUTER_PORT_FLD_EXPIRE_TIMER, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_MROUTER_PORT_FLD_EXPIRE_TIMER].name,
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_mr_port->expire_timer));
    }

    return PM_E_NONE;
}
