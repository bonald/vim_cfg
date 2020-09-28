
#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "cdb_data_cmp.h"

int32
ds_openflow_if_add_openflow_if_sync(tbl_interface_t *p_if, ds_openflow_if_t *p_openflow, uint32 sync)
{
    tbl_interface_t *p_db_if = NULL;
    ds_openflow_if_t *p_db_openflow = NULL;

    /* 1. lookup entry exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_openflow = p_db_if->openflow_if;
    if (p_db_openflow)
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_openflow = XCALLOC(MEM_DS_OPENFLOW_IF, sizeof(ds_openflow_if_t));
    if (NULL == p_db_openflow)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_openflow, p_openflow, sizeof(ds_openflow_if_t));

    /* 4. add to db */
    p_db_if->openflow_if = p_db_openflow;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_add(TBL_INTERFACE, DS_OPENFLOW_IF, p_db_if, p_db_openflow);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_add(TBL_INTERFACE, DS_OPENFLOW_IF, p_db_if, p_db_openflow);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_openflow_if_del_openflow_if_sync(tbl_interface_t *p_if, uint32 sync)
{
    tbl_interface_t *p_db_if = NULL;
    ds_openflow_if_t *p_db_openflow = NULL;

    /* 1. lookup entry exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_openflow = p_db_if->openflow_if;
    if (NULL == p_db_openflow)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_del(TBL_INTERFACE, DS_OPENFLOW_IF, p_db_if, p_db_openflow);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_del(TBL_INTERFACE, DS_OPENFLOW_IF, p_db_if, p_db_openflow);
        #endif 
    }

    /* 3. delete from db */
    p_db_if->openflow_if = NULL;

    /* 4. free db entry */
    XFREE(MEM_DS_OPENFLOW_IF, p_db_openflow);

    return PM_E_NONE;
}

int32
ds_openflow_if_set_openflow_if_field_sync(tbl_interface_t *p_if, ds_openflow_if_t *p_openflow, ds_openflow_if_field_id_t field_id, uint32 sync)
{
    tbl_interface_t *p_db_if = NULL;
    ds_openflow_if_t *p_db_openflow = NULL;

    /* 1. lookup entry exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }

    p_db_openflow = ds_openflow_if_get_openflow_if(p_if);
    if (NULL == p_db_openflow)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case DS_OPENFLOW_IF_FLD_ENABLE:
        p_db_openflow->enable = p_openflow->enable;
        break;

    case DS_OPENFLOW_IF_FLD_OBEY_VLAN_FILTER:
        p_db_openflow->obey_vlan_filter = p_openflow->obey_vlan_filter;
        break;

    case DS_OPENFLOW_IF_FLD_MAX:
        sal_memcpy(p_db_openflow, p_openflow, sizeof(ds_openflow_if_t));
        break;

    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_set(TBL_INTERFACE, DS_OPENFLOW_IF, field_id, p_db_if, p_db_openflow);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_set(TBL_INTERFACE, DS_OPENFLOW_IF, field_id, p_db_if, p_db_openflow);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_openflow_if_add_openflow_if(tbl_interface_t *p_if, ds_openflow_if_t *p_openflow)
{
    return ds_openflow_if_add_openflow_if_sync(p_if, p_openflow, TRUE);
}

int32
ds_openflow_if_del_openflow_if(tbl_interface_t *p_if)
{
    return ds_openflow_if_del_openflow_if_sync(p_if, TRUE);
}

int32
ds_openflow_if_set_openflow_if_field(tbl_interface_t *p_if, ds_openflow_if_t *p_openflow, ds_openflow_if_field_id_t field_id)
{
    return ds_openflow_if_set_openflow_if_field_sync(p_if, p_openflow, field_id, TRUE);
}

ds_openflow_if_t*
ds_openflow_if_get_openflow_if(tbl_interface_t *p_if)
{
    tbl_interface_t *p_db_if = NULL;

    /* 1. lookup entry exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return NULL;
    }

    return p_db_if->openflow_if;
}

int32
ds_openflow_if_key_str2val(char* tbl_key_value, char* str, tbl_interface_t *p_if, ds_openflow_if_t *p_openflow)
{

    int32 ret = 0;

    ret = tbl_interface_key_str2val(tbl_key_value, p_if);
    return ret;
}
char*
ds_openflow_if_key_value_dump(tbl_interface_t *p_if, ds_openflow_if_t *p_openflow, char* str)
{

    sal_sprintf(str, "%s",
        tbl_interface_key_value_dump(p_if, str));

    return str;
}

char*
ds_openflow_if_key_name_dump(tbl_interface_t *p_if, ds_openflow_if_t *p_openflow, char* str)
{

    cdb_node_t *p_node_tbl = cdb_get_tbl(TBL_INTERFACE);
    cdb_node_t *p_node_ds = cdb_get_ds(DS_OPENFLOW_IF);
    char tbl_str[MAX_CMD_STR_LEN];
    char ds_str[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s",
        p_node_tbl->name, tbl_interface_key_value_dump(p_if, tbl_str), p_node_ds->name, ds_openflow_if_key_value_dump(p_if, p_openflow, ds_str));
    return str;
}

char*
ds_openflow_if_field_name_dump(tbl_interface_t *p_if, ds_openflow_if_t *p_openflow, int32 field_id, char* str)
{

    cdb_node_t *p_node = cdb_get_ds(DS_OPENFLOW_IF);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= DS_OPENFLOW_IF_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
ds_openflow_if_field_value_dump(tbl_interface_t *p_if, ds_openflow_if_t *p_openflow, int32 field_id, char* str)
{

    if (FLD_MATCH(DS_OPENFLOW_IF_FLD_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->enable);
    }
    if (FLD_MATCH(DS_OPENFLOW_IF_FLD_OBEY_VLAN_FILTER, field_id))
    {
        sal_sprintf(str, "%u", p_openflow->obey_vlan_filter);
    }
    return str;
}

char**
ds_openflow_if_table_dump(tbl_interface_t *p_if, ds_openflow_if_t *p_openflow, char** str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         ds_openflow_if_key_name_dump(p_if, p_openflow, buf));
    for(i=1; i<DS_OPENFLOW_IF_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            ds_openflow_if_field_name_dump(p_if, p_openflow, i, buf));
        sal_sprintf(str[j++], "%s",
            ds_openflow_if_field_value_dump(p_if, p_openflow, i, buf));
    }
    return str;
}

int32
ds_openflow_if_field_value_parser(char *str, int32 field_id, tbl_interface_t *p_if, ds_openflow_if_t *p_openflow)
{
    int32 ret=0;
    if (FLD_MATCH(DS_OPENFLOW_IF_FLD_ENABLE, field_id))
    {
        p_openflow->enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_OPENFLOW_IF_FLD_OBEY_VLAN_FILTER, field_id))
    {
        p_openflow->obey_vlan_filter = cdb_uint_str2val(str, &ret);
    }
    return ret;
}

int32
ds_openflow_if_table_parser(char** array, char* tbl_key_value, char* ds_key_value,tbl_interface_t *p_if, ds_openflow_if_t *p_openflow)
{
    int32 rc;
    int32 i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_ds(DS_OPENFLOW_IF);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(ds_openflow_if_key_str2val(tbl_key_value, ds_key_value, p_if, p_openflow));

    for(i=1; i<DS_OPENFLOW_IF_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(ds_openflow_if_field_value_parser( array[j++], i, p_if, p_openflow));
    }

    return PM_E_NONE;
}

int32
ds_openflow_if_dump_one(ds_openflow_if_t *p_openflow, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_ds(DS_OPENFLOW_IF);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(DS_OPENFLOW_IF_FLD_ENABLE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_OPENFLOW_IF_FLD_ENABLE].name,
            p_openflow->enable);
    }
    if (FLD_MATCH(DS_OPENFLOW_IF_FLD_OBEY_VLAN_FILTER, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_OPENFLOW_IF_FLD_OBEY_VLAN_FILTER].name,
            p_openflow->obey_vlan_filter);
    }

    return PM_E_NONE;
}
