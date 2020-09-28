
#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "cdb_data_cmp.h"

int32
ds_l2protocol_add_l2protocol_sync(tbl_interface_t *p_if, ds_l2protocol_t *p_l2protocol, uint32 sync)
{
    tbl_interface_t *p_db_if = NULL;
    ds_l2protocol_t *p_db_l2protocol = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_l2protocol = ds_l2protocol_get_l2protocol(p_if, p_l2protocol);
    if (p_db_l2protocol)
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_l2protocol = XCALLOC(MEM_DS_L2PROTOCOL, sizeof(ds_l2protocol_t));
    if (NULL == p_db_l2protocol)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_l2protocol, p_l2protocol, sizeof(ds_l2protocol_t));

    /* 4. add to db */
    PM_E_RETURN(cdb_reference_list_add_object(&p_db_if->l2protocol, p_db_l2protocol));

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_add(TBL_INTERFACE, DS_L2PROTOCOL, p_db_if, p_db_l2protocol);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_add(TBL_INTERFACE, DS_L2PROTOCOL, p_db_if, p_db_l2protocol);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_l2protocol_del_l2protocol_sync(tbl_interface_t *p_if, ds_l2protocol_t *p_l2protocol, uint32 sync)
{
    tbl_interface_t *p_db_if = NULL;
    ds_l2protocol_t *p_db_l2protocol = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_l2protocol = ds_l2protocol_get_l2protocol(p_if, p_l2protocol);
    if (NULL == p_db_l2protocol)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_del(TBL_INTERFACE, DS_L2PROTOCOL, p_db_if, p_db_l2protocol);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_del(TBL_INTERFACE, DS_L2PROTOCOL, p_db_if, p_db_l2protocol);
        #endif 
    }

    /* 3. delete from db */
    PM_E_RETURN(cdb_reference_list_del_object(&p_db_if->l2protocol, p_db_l2protocol));

    /* 4. free db entry */
    XFREE(MEM_DS_L2PROTOCOL, p_db_l2protocol);

    return PM_E_NONE;
}

int32
ds_l2protocol_set_l2protocol_field_sync(tbl_interface_t *p_if, ds_l2protocol_t *p_l2protocol, ds_l2protocol_field_id_t field_id, uint32 sync)
{
    tbl_interface_t *p_db_if = NULL;
    ds_l2protocol_t *p_db_l2protocol = NULL;

    /* 1. lookup entry exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }

    p_db_l2protocol = ds_l2protocol_get_l2protocol(p_if, p_l2protocol);
    if (NULL == p_db_l2protocol)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case DS_L2PROTOCOL_FLD_IS_STANDARD_PRO:
        p_db_l2protocol->is_standard_pro = p_l2protocol->is_standard_pro;
        break;

    case DS_L2PROTOCOL_FLD_L2PRO_ACTION:
        p_db_l2protocol->l2pro_action = p_l2protocol->l2pro_action;
        break;

    case DS_L2PROTOCOL_FLD_L2PRO_VLAN:
        p_db_l2protocol->l2pro_vlan = p_l2protocol->l2pro_vlan;
        break;

    case DS_L2PROTOCOL_FLD_L2PRO_EVC_NAME:
        sal_memcpy(p_db_l2protocol->l2pro_evc_name, p_l2protocol->l2pro_evc_name, sizeof(p_l2protocol->l2pro_evc_name));
        break;

    case DS_L2PROTOCOL_FLD_MAX:
        sal_memcpy(p_db_l2protocol, p_l2protocol, sizeof(ds_l2protocol_t));
        break;

    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_set(TBL_INTERFACE, DS_L2PROTOCOL, field_id, p_db_if, p_db_l2protocol);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_set(TBL_INTERFACE, DS_L2PROTOCOL, field_id, p_db_if, p_db_l2protocol);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_l2protocol_add_l2protocol(tbl_interface_t *p_if, ds_l2protocol_t *p_l2protocol)
{
    return ds_l2protocol_add_l2protocol_sync(p_if, p_l2protocol, TRUE);
}

int32
ds_l2protocol_del_l2protocol(tbl_interface_t *p_if, ds_l2protocol_t *p_l2protocol)
{
    return ds_l2protocol_del_l2protocol_sync(p_if, p_l2protocol, TRUE);
}

int32
ds_l2protocol_set_l2protocol_field(tbl_interface_t *p_if, ds_l2protocol_t *p_l2protocol, ds_l2protocol_field_id_t field_id)
{
    return ds_l2protocol_set_l2protocol_field_sync(p_if, p_l2protocol, field_id, TRUE);
}

ds_l2protocol_t*
ds_l2protocol_get_l2protocol(tbl_interface_t *p_if, ds_l2protocol_t *p_l2protocol)
{
    tbl_interface_t *p_db_if = NULL;

    /* 1. lookup entry exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return NULL;
    }

    return cdb_reference_list_lookup_object(&p_db_if->l2protocol, p_l2protocol);
}

int32
ds_l2protocol_key_str2val(char* tbl_key_value, char* str, tbl_interface_t *p_if, ds_l2protocol_t *p_l2protocol)
{

    int32 ret = 0;

    ret = tbl_interface_key_str2val(tbl_key_value, p_if);

    sal_strcpy(p_l2protocol->key.l2pro_name, str);

    return ret;
}

char*
ds_l2protocol_key_value_dump(tbl_interface_t *p_if, ds_l2protocol_t *p_l2protocol, char* str)
{

    sal_sprintf(str, "%s", p_l2protocol->key.l2pro_name);
    return str;
}

char*
ds_l2protocol_key_name_dump(tbl_interface_t *p_if, ds_l2protocol_t *p_l2protocol, char* str)
{

    cdb_node_t *p_node_tbl = cdb_get_tbl(TBL_INTERFACE);
    cdb_node_t *p_node_ds = cdb_get_ds(DS_L2PROTOCOL);
    char tbl_str[MAX_CMD_STR_LEN];
    char ds_str[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s",
        p_node_tbl->name, tbl_interface_key_value_dump(p_if, tbl_str), p_node_ds->name, ds_l2protocol_key_value_dump(p_if, p_l2protocol, ds_str));
    return str;
}

char*
ds_l2protocol_field_name_dump(tbl_interface_t *p_if, ds_l2protocol_t *p_l2protocol, int32 field_id, char* str)
{

    cdb_node_t *p_node = cdb_get_ds(DS_L2PROTOCOL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= DS_L2PROTOCOL_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
ds_l2protocol_field_value_dump(tbl_interface_t *p_if, ds_l2protocol_t *p_l2protocol, int32 field_id, char* str)
{

    if (FLD_MATCH(DS_L2PROTOCOL_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_l2protocol->key.l2pro_name);
    }
    if (FLD_MATCH(DS_L2PROTOCOL_FLD_IS_STANDARD_PRO, field_id))
    {
        sal_sprintf(str, "%u", p_l2protocol->is_standard_pro);
    }
    if (FLD_MATCH(DS_L2PROTOCOL_FLD_L2PRO_ACTION, field_id))
    {
        sal_sprintf(str, "%u", p_l2protocol->l2pro_action);
    }
    if (FLD_MATCH(DS_L2PROTOCOL_FLD_L2PRO_VLAN, field_id))
    {
        sal_sprintf(str, "%u", p_l2protocol->l2pro_vlan);
    }
    if (FLD_MATCH(DS_L2PROTOCOL_FLD_L2PRO_EVC_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_l2protocol->l2pro_evc_name);
    }
    return str;
}

char**
ds_l2protocol_table_dump(tbl_interface_t *p_if, ds_l2protocol_t *p_l2protocol, char** str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         ds_l2protocol_key_name_dump(p_if, p_l2protocol, buf));
    for(i=1; i<DS_L2PROTOCOL_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            ds_l2protocol_field_name_dump(p_if, p_l2protocol, i, buf));
        sal_sprintf(str[j++], "%s",
            ds_l2protocol_field_value_dump(p_if, p_l2protocol, i, buf));
    }
    return str;
}

int32
ds_l2protocol_field_value_parser(char *str, int32 field_id, tbl_interface_t *p_if, ds_l2protocol_t *p_l2protocol)
{
    int32 ret=0;
    if (FLD_MATCH(DS_L2PROTOCOL_FLD_KEY, field_id))
    {
        sal_strcpy(p_l2protocol->key.l2pro_name, str);
    }
    if (FLD_MATCH(DS_L2PROTOCOL_FLD_IS_STANDARD_PRO, field_id))
    {
        p_l2protocol->is_standard_pro = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_L2PROTOCOL_FLD_L2PRO_ACTION, field_id))
    {
        p_l2protocol->l2pro_action = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_L2PROTOCOL_FLD_L2PRO_VLAN, field_id))
    {
        p_l2protocol->l2pro_vlan = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_L2PROTOCOL_FLD_L2PRO_EVC_NAME, field_id))
    {
        sal_strcpy(p_l2protocol->l2pro_evc_name, str);
    }
    return ret;
}

int32
ds_l2protocol_table_parser(char** array, char* tbl_key_value, char* ds_key_value,tbl_interface_t *p_if, ds_l2protocol_t *p_l2protocol)
{
    int32 rc;
    int32 i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_ds(DS_L2PROTOCOL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(ds_l2protocol_key_str2val(tbl_key_value, ds_key_value, p_if, p_l2protocol));

    for(i=1; i<DS_L2PROTOCOL_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(ds_l2protocol_field_value_parser( array[j++], i, p_if, p_l2protocol));
    }

    return PM_E_NONE;
}

int32
ds_l2protocol_dump_one(ds_l2protocol_t *p_l2protocol, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_ds(DS_L2PROTOCOL);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(DS_L2PROTOCOL_FLD_KEY, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_L2PROTOCOL_FLD_KEY].name,
            p_l2protocol->key.l2pro_name);
    }
    if (FLD_MATCH(DS_L2PROTOCOL_FLD_IS_STANDARD_PRO, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_L2PROTOCOL_FLD_IS_STANDARD_PRO].name,
            p_l2protocol->is_standard_pro);
    }
    if (FLD_MATCH(DS_L2PROTOCOL_FLD_L2PRO_ACTION, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_L2PROTOCOL_FLD_L2PRO_ACTION].name,
            p_l2protocol->l2pro_action);
    }
    if (FLD_MATCH(DS_L2PROTOCOL_FLD_L2PRO_VLAN, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_L2PROTOCOL_FLD_L2PRO_VLAN].name,
            p_l2protocol->l2pro_vlan);
    }
    if (FLD_MATCH(DS_L2PROTOCOL_FLD_L2PRO_EVC_NAME, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_L2PROTOCOL_FLD_L2PRO_EVC_NAME].name,
            p_l2protocol->l2pro_evc_name);
    }

    return PM_E_NONE;
}
