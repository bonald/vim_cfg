
#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "cdb_data_cmp.h"

int32
ds_lag_add_lag_sync(tbl_interface_t *p_if, ds_lag_t *p_lag, uint32 sync)
{
    tbl_interface_t *p_db_if = NULL;
    ds_lag_t *p_db_lag = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_lag = p_db_if->lag;
    if (p_db_lag)
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_lag = XCALLOC(MEM_DS_LAG, sizeof(ds_lag_t));
    if (NULL == p_db_lag)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_lag, p_lag, sizeof(ds_lag_t));
    PM_E_RETURN(cdb_reference_list_init(&p_db_lag->member_ports, CDB_NODE_TBL, TBL_INTERFACE, tbl_interface_cmp));

    /* 4. add to db */
    p_db_if->lag = p_db_lag;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_add(TBL_INTERFACE, DS_LAG, p_db_if, p_db_lag);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_add(TBL_INTERFACE, DS_LAG, p_db_if, p_db_lag);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_lag_del_lag_sync(tbl_interface_t *p_if, uint32 sync)
{
    tbl_interface_t *p_db_if = NULL;
    ds_lag_t *p_db_lag = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_lag = p_db_if->lag;
    if (NULL == p_db_lag)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_del(TBL_INTERFACE, DS_LAG, p_db_if, p_db_lag);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_del(TBL_INTERFACE, DS_LAG, p_db_if, p_db_lag);
        #endif 
    }

    PM_E_RETURN(cdb_reference_list_deinit(&p_db_lag->member_ports));

    /* 3. delete from db */
    p_db_if->lag = NULL;

    /* 4. free db entry */
    XFREE(MEM_DS_LAG, p_db_lag);

    return PM_E_NONE;
}

int32
ds_lag_set_lag_field_sync(tbl_interface_t *p_if, ds_lag_t *p_lag, ds_lag_field_id_t field_id, uint32 sync)
{
    tbl_interface_t *p_db_if = NULL;
    ds_lag_t *p_db_lag = NULL;

    /* 1. lookup entry exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }

    p_db_lag = ds_lag_get_lag(p_if);
    if (NULL == p_db_lag)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case DS_LAG_FLD_MODE:
        p_db_lag->mode = p_lag->mode;
        break;

    case DS_LAG_FLD_LAG_ID:
        p_db_lag->lag_id = p_lag->lag_id;
        break;

    case DS_LAG_FLD_OPER_STATE:
        p_db_lag->oper_state = p_lag->oper_state;
        break;

    case DS_LAG_FLD_MISMATCH:
        p_db_lag->mismatch = p_lag->mismatch;
        break;

    case DS_LAG_FLD_BUNDLE_PORTS_COUNT:
        p_db_lag->bundle_ports_count = p_lag->bundle_ports_count;
        break;

    case DS_LAG_FLD_LOAD_BALANCE_MODE:
        p_db_lag->load_balance_mode = p_lag->load_balance_mode;
        break;

    case DS_LAG_FLD_IS_ACTIVE_BACKUP_MODE:
        p_db_lag->is_active_backup_mode = p_lag->is_active_backup_mode;
        break;

    case DS_LAG_FLD_MAX_ACTIVE_NUMBER:
        p_db_lag->max_active_number = p_lag->max_active_number;
        break;

    case DS_LAG_FLD_CUR_ACTIVE_NUMBER:
        p_db_lag->cur_active_number = p_lag->cur_active_number;
        break;

    case DS_LAG_FLD_CUR_ACTIVE_MAX_PRIORITY:
        p_db_lag->cur_active_max_priority = p_lag->cur_active_max_priority;
        break;

    case DS_LAG_FLD_MAX:
        sal_memcpy(p_db_lag, p_lag, sizeof(ds_lag_t));
        break;

    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_set(TBL_INTERFACE, DS_LAG, field_id, p_db_if, p_db_lag);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_set(TBL_INTERFACE, DS_LAG, field_id, p_db_if, p_db_lag);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_lag_add_lag(tbl_interface_t *p_if, ds_lag_t *p_lag)
{
    return ds_lag_add_lag_sync(p_if, p_lag, TRUE);
}

int32
ds_lag_del_lag(tbl_interface_t *p_if)
{
    return ds_lag_del_lag_sync(p_if, TRUE);
}

int32
ds_lag_set_lag_field(tbl_interface_t *p_if, ds_lag_t *p_lag, ds_lag_field_id_t field_id)
{
    return ds_lag_set_lag_field_sync(p_if, p_lag, field_id, TRUE);
}

ds_lag_t*
ds_lag_get_lag(tbl_interface_t *p_if)
{
    tbl_interface_t *p_db_if = NULL;

    /* 1. lookup entry exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return NULL;
    }

    return p_db_if->lag;
}

int32
ds_lag_key_str2val(char* tbl_key_value, char* str, tbl_interface_t *p_if, ds_lag_t *p_lag)
{

    int32 ret = 0;

    ret = tbl_interface_key_str2val(tbl_key_value, p_if);
    return ret;
}
char*
ds_lag_key_value_dump(tbl_interface_t *p_if, ds_lag_t *p_lag, char* str)
{

    sal_sprintf(str, "%s",
        tbl_interface_key_value_dump(p_if, str));

    return str;
}

char*
ds_lag_key_name_dump(tbl_interface_t *p_if, ds_lag_t *p_lag, char* str)
{

    cdb_node_t *p_node_tbl = cdb_get_tbl(TBL_INTERFACE);
    cdb_node_t *p_node_ds = cdb_get_ds(DS_LAG);
    char tbl_str[MAX_CMD_STR_LEN];
    char ds_str[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s",
        p_node_tbl->name, tbl_interface_key_value_dump(p_if, tbl_str), p_node_ds->name, ds_lag_key_value_dump(p_if, p_lag, ds_str));
    return str;
}

char*
ds_lag_field_name_dump(tbl_interface_t *p_if, ds_lag_t *p_lag, int32 field_id, char* str)
{

    cdb_node_t *p_node = cdb_get_ds(DS_LAG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= DS_LAG_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
ds_lag_field_value_dump(tbl_interface_t *p_if, ds_lag_t *p_lag, int32 field_id, char* str)
{

    if (FLD_MATCH(DS_LAG_FLD_MODE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(glb_agg_mode_strs, GLB_AGG_MODE_MAX, p_lag->mode));
    }
    if (FLD_MATCH(DS_LAG_FLD_LAG_ID, field_id))
    {
        sal_sprintf(str, "%u", p_lag->lag_id);
    }
    if (FLD_MATCH(DS_LAG_FLD_OPER_STATE, field_id))
    {
        sal_sprintf(str, "%u", p_lag->oper_state);
    }
    if (FLD_MATCH(DS_LAG_FLD_MISMATCH, field_id))
    {
        sal_sprintf(str, "%u", p_lag->mismatch);
    }
    if (FLD_MATCH(DS_LAG_FLD_BUNDLE_PORTS_COUNT, field_id))
    {
        sal_sprintf(str, "%d", p_lag->bundle_ports_count);
    }
    if (FLD_MATCH(DS_LAG_FLD_LOAD_BALANCE_MODE, field_id))
    {
        sal_sprintf(str, "%u", p_lag->load_balance_mode);
    }
    if (FLD_MATCH(DS_LAG_FLD_IS_ACTIVE_BACKUP_MODE, field_id))
    {
        sal_sprintf(str, "%u", p_lag->is_active_backup_mode);
    }
    if (FLD_MATCH(DS_LAG_FLD_MAX_ACTIVE_NUMBER, field_id))
    {
        sal_sprintf(str, "%u", p_lag->max_active_number);
    }
    if (FLD_MATCH(DS_LAG_FLD_CUR_ACTIVE_NUMBER, field_id))
    {
        sal_sprintf(str, "%u", p_lag->cur_active_number);
    }
    if (FLD_MATCH(DS_LAG_FLD_CUR_ACTIVE_MAX_PRIORITY, field_id))
    {
        sal_sprintf(str, "%u", p_lag->cur_active_max_priority);
    }
    if (FLD_MATCH(DS_LAG_FLD_MEMBER_PORTS, field_id))
    {
    }
    return str;
}

char**
ds_lag_table_dump(tbl_interface_t *p_if, ds_lag_t *p_lag, char** str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         ds_lag_key_name_dump(p_if, p_lag, buf));
    for(i=1; i<DS_LAG_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            ds_lag_field_name_dump(p_if, p_lag, i, buf));
        sal_sprintf(str[j++], "%s",
            ds_lag_field_value_dump(p_if, p_lag, i, buf));
    }
    return str;
}

int32
ds_lag_field_value_parser(char *str, int32 field_id, tbl_interface_t *p_if, ds_lag_t *p_lag)
{
    int32 ret=0;
    if (FLD_MATCH(DS_LAG_FLD_MODE, field_id))
    {
        cdb_enum_str2val(glb_agg_mode_strs, GLB_AGG_MODE_MAX, str);
    }
    if (FLD_MATCH(DS_LAG_FLD_LAG_ID, field_id))
    {
        p_lag->lag_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LAG_FLD_OPER_STATE, field_id))
    {
        p_lag->oper_state = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LAG_FLD_MISMATCH, field_id))
    {
        p_lag->mismatch = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LAG_FLD_BUNDLE_PORTS_COUNT, field_id))
    {
    }
    if (FLD_MATCH(DS_LAG_FLD_LOAD_BALANCE_MODE, field_id))
    {
        p_lag->load_balance_mode = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LAG_FLD_IS_ACTIVE_BACKUP_MODE, field_id))
    {
        p_lag->is_active_backup_mode = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LAG_FLD_MAX_ACTIVE_NUMBER, field_id))
    {
        p_lag->max_active_number = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LAG_FLD_CUR_ACTIVE_NUMBER, field_id))
    {
        p_lag->cur_active_number = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LAG_FLD_CUR_ACTIVE_MAX_PRIORITY, field_id))
    {
        p_lag->cur_active_max_priority = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_LAG_FLD_MEMBER_PORTS, field_id))
    {
    }
    return ret;
}

int32
ds_lag_table_parser(char** array, char* tbl_key_value, char* ds_key_value,tbl_interface_t *p_if, ds_lag_t *p_lag)
{
    int32 rc;
    int32 i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_ds(DS_LAG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(ds_lag_key_str2val(tbl_key_value, ds_key_value, p_if, p_lag));

    for(i=1; i<DS_LAG_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(ds_lag_field_value_parser( array[j++], i, p_if, p_lag));
    }

    return PM_E_NONE;
}

int32
ds_lag_dump_one(ds_lag_t *p_lag, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_ds(DS_LAG);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(DS_LAG_FLD_MODE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_LAG_FLD_MODE].name,
            cdb_enum_val2str(glb_agg_mode_strs, GLB_AGG_MODE_MAX, p_lag->mode));
    }
    if (FLD_MATCH(DS_LAG_FLD_LAG_ID, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LAG_FLD_LAG_ID].name,
            p_lag->lag_id);
    }
    if (FLD_MATCH(DS_LAG_FLD_OPER_STATE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LAG_FLD_OPER_STATE].name,
            p_lag->oper_state);
    }
    if (FLD_MATCH(DS_LAG_FLD_MISMATCH, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LAG_FLD_MISMATCH].name,
            p_lag->mismatch);
    }
    if (FLD_MATCH(DS_LAG_FLD_BUNDLE_PORTS_COUNT, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d",
            p_tbl_info->field[DS_LAG_FLD_BUNDLE_PORTS_COUNT].name,
            p_lag->bundle_ports_count);
    }
    if (FLD_MATCH(DS_LAG_FLD_LOAD_BALANCE_MODE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LAG_FLD_LOAD_BALANCE_MODE].name,
            p_lag->load_balance_mode);
    }
    if (FLD_MATCH(DS_LAG_FLD_IS_ACTIVE_BACKUP_MODE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LAG_FLD_IS_ACTIVE_BACKUP_MODE].name,
            p_lag->is_active_backup_mode);
    }
    if (FLD_MATCH(DS_LAG_FLD_MAX_ACTIVE_NUMBER, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LAG_FLD_MAX_ACTIVE_NUMBER].name,
            p_lag->max_active_number);
    }
    if (FLD_MATCH(DS_LAG_FLD_CUR_ACTIVE_NUMBER, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LAG_FLD_CUR_ACTIVE_NUMBER].name,
            p_lag->cur_active_number);
    }
    if (FLD_MATCH(DS_LAG_FLD_CUR_ACTIVE_MAX_PRIORITY, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_LAG_FLD_CUR_ACTIVE_MAX_PRIORITY].name,
            p_lag->cur_active_max_priority);
    }
    if (FLD_MATCH(DS_LAG_FLD_MEMBER_PORTS, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_LAG_FLD_MEMBER_PORTS].name,
            cdb_reference_list_val2str(buf, MAX_CMD_STR_LEN, &p_lag->member_ports));
    }

    return PM_E_NONE;
}

int32
ds_lag_set_member_ports_sync(tbl_interface_t *p_if, ds_lag_t *p_lag, uint32 add, void *p_object, uint32 sync)
{
    cdb_sync_refer_hdr_t refer;
    ds_lag_t *p_db_lag = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_lag = ds_lag_get_lag(p_if);
    if (NULL == p_db_lag)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    if (add)
    {
        PM_E_RETURN(cdb_reference_list_add_object(&p_db_lag->member_ports, p_object));
    }
    else
    {
        PM_E_RETURN(cdb_reference_list_del_object(&p_db_lag->member_ports, p_object));
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_build_refer_tbl(TBL_INTERFACE, add, &refer);
        cdb_sync_ds_set_refer(TBL_INTERFACE, DS_LAG, DS_LAG_FLD_MEMBER_PORTS, p_if, p_lag, &refer, p_object);
    }

    return PM_E_NONE;
}

int32
ds_lag_set_member_ports(tbl_interface_t *p_if, ds_lag_t *p_lag, uint32 add, void *p_object)
{
    return ds_lag_set_member_ports_sync(p_if, p_lag, add, p_object, TRUE);
}

