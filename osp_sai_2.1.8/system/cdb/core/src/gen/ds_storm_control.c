
#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "cdb_data_cmp.h"

int32
ds_storm_control_add_storm_control_sync(tbl_interface_t *p_if, ds_storm_control_t *p_storm_control, uint32 sync)
{
    tbl_interface_t *p_db_if = NULL;
    ds_storm_control_t *p_db_storm_control = NULL;

    /* 1. lookup entry exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_storm_control = p_db_if->storm_control;
    if (p_db_storm_control)
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_storm_control = XCALLOC(MEM_DS_STORM_CONTROL, sizeof(ds_storm_control_t));
    if (NULL == p_db_storm_control)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_storm_control, p_storm_control, sizeof(ds_storm_control_t));

    /* 4. add to db */
    p_db_if->storm_control = p_db_storm_control;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_add(TBL_INTERFACE, DS_STORM_CONTROL, p_db_if, p_db_storm_control);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_add(TBL_INTERFACE, DS_STORM_CONTROL, p_db_if, p_db_storm_control);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_storm_control_del_storm_control_sync(tbl_interface_t *p_if, uint32 sync)
{
    tbl_interface_t *p_db_if = NULL;
    ds_storm_control_t *p_db_storm_control = NULL;

    /* 1. lookup entry exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_storm_control = p_db_if->storm_control;
    if (NULL == p_db_storm_control)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_del(TBL_INTERFACE, DS_STORM_CONTROL, p_db_if, p_db_storm_control);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_del(TBL_INTERFACE, DS_STORM_CONTROL, p_db_if, p_db_storm_control);
        #endif 
    }

    /* 3. delete from db */
    p_db_if->storm_control = NULL;

    /* 4. free db entry */
    XFREE(MEM_DS_STORM_CONTROL, p_db_storm_control);

    return PM_E_NONE;
}

int32
ds_storm_control_set_storm_control_field_sync(tbl_interface_t *p_if, ds_storm_control_t *p_storm_control, ds_storm_control_field_id_t field_id, uint32 sync)
{
    tbl_interface_t *p_db_if = NULL;
    ds_storm_control_t *p_db_storm_control = NULL;

    /* 1. lookup entry exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }

    p_db_storm_control = ds_storm_control_get_storm_control(p_if);
    if (NULL == p_db_storm_control)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case DS_STORM_CONTROL_FLD_BCAST_MODE:
        p_db_storm_control->bcast_mode = p_storm_control->bcast_mode;
        break;

    case DS_STORM_CONTROL_FLD_MCAST_MODE:
        p_db_storm_control->mcast_mode = p_storm_control->mcast_mode;
        break;

    case DS_STORM_CONTROL_FLD_UCAST_MODE:
        p_db_storm_control->ucast_mode = p_storm_control->ucast_mode;
        break;

    case DS_STORM_CONTROL_FLD_BCAST_RATE:
        p_db_storm_control->bcast_rate = p_storm_control->bcast_rate;
        break;

    case DS_STORM_CONTROL_FLD_MCAST_RATE:
        p_db_storm_control->mcast_rate = p_storm_control->mcast_rate;
        break;

    case DS_STORM_CONTROL_FLD_UCAST_RATE:
        p_db_storm_control->ucast_rate = p_storm_control->ucast_rate;
        break;

    case DS_STORM_CONTROL_FLD_MAX:
        sal_memcpy(p_db_storm_control, p_storm_control, sizeof(ds_storm_control_t));
        break;

    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_set(TBL_INTERFACE, DS_STORM_CONTROL, field_id, p_db_if, p_db_storm_control);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_set(TBL_INTERFACE, DS_STORM_CONTROL, field_id, p_db_if, p_db_storm_control);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_storm_control_add_storm_control(tbl_interface_t *p_if, ds_storm_control_t *p_storm_control)
{
    return ds_storm_control_add_storm_control_sync(p_if, p_storm_control, TRUE);
}

int32
ds_storm_control_del_storm_control(tbl_interface_t *p_if)
{
    return ds_storm_control_del_storm_control_sync(p_if, TRUE);
}

int32
ds_storm_control_set_storm_control_field(tbl_interface_t *p_if, ds_storm_control_t *p_storm_control, ds_storm_control_field_id_t field_id)
{
    return ds_storm_control_set_storm_control_field_sync(p_if, p_storm_control, field_id, TRUE);
}

ds_storm_control_t*
ds_storm_control_get_storm_control(tbl_interface_t *p_if)
{
    tbl_interface_t *p_db_if = NULL;

    /* 1. lookup entry exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return NULL;
    }

    return p_db_if->storm_control;
}

int32
ds_storm_control_key_str2val(char* tbl_key_value, char* str, tbl_interface_t *p_if, ds_storm_control_t *p_storm_control)
{

    int32 ret = 0;

    ret = tbl_interface_key_str2val(tbl_key_value, p_if);
    return ret;
}
char*
ds_storm_control_key_value_dump(tbl_interface_t *p_if, ds_storm_control_t *p_storm_control, char* str)
{

    sal_sprintf(str, "%s",
        tbl_interface_key_value_dump(p_if, str));

    return str;
}

char*
ds_storm_control_key_name_dump(tbl_interface_t *p_if, ds_storm_control_t *p_storm_control, char* str)
{

    cdb_node_t *p_node_tbl = cdb_get_tbl(TBL_INTERFACE);
    cdb_node_t *p_node_ds = cdb_get_ds(DS_STORM_CONTROL);
    char tbl_str[MAX_CMD_STR_LEN];
    char ds_str[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s",
        p_node_tbl->name, tbl_interface_key_value_dump(p_if, tbl_str), p_node_ds->name, ds_storm_control_key_value_dump(p_if, p_storm_control, ds_str));
    return str;
}

char*
ds_storm_control_field_name_dump(tbl_interface_t *p_if, ds_storm_control_t *p_storm_control, int32 field_id, char* str)
{

    cdb_node_t *p_node = cdb_get_ds(DS_STORM_CONTROL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= DS_STORM_CONTROL_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
ds_storm_control_field_value_dump(tbl_interface_t *p_if, ds_storm_control_t *p_storm_control, int32 field_id, char* str)
{

    if (FLD_MATCH(DS_STORM_CONTROL_FLD_BCAST_MODE, field_id))
    {
        sal_sprintf(str, "%u", p_storm_control->bcast_mode);
    }
    if (FLD_MATCH(DS_STORM_CONTROL_FLD_MCAST_MODE, field_id))
    {
        sal_sprintf(str, "%u", p_storm_control->mcast_mode);
    }
    if (FLD_MATCH(DS_STORM_CONTROL_FLD_UCAST_MODE, field_id))
    {
        sal_sprintf(str, "%u", p_storm_control->ucast_mode);
    }
    if (FLD_MATCH(DS_STORM_CONTROL_FLD_BCAST_RATE, field_id))
    {
        sal_sprintf(str, "%u", p_storm_control->bcast_rate);
    }
    if (FLD_MATCH(DS_STORM_CONTROL_FLD_MCAST_RATE, field_id))
    {
        sal_sprintf(str, "%u", p_storm_control->mcast_rate);
    }
    if (FLD_MATCH(DS_STORM_CONTROL_FLD_UCAST_RATE, field_id))
    {
        sal_sprintf(str, "%u", p_storm_control->ucast_rate);
    }
    return str;
}

char**
ds_storm_control_table_dump(tbl_interface_t *p_if, ds_storm_control_t *p_storm_control, char** str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         ds_storm_control_key_name_dump(p_if, p_storm_control, buf));
    for(i=1; i<DS_STORM_CONTROL_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            ds_storm_control_field_name_dump(p_if, p_storm_control, i, buf));
        sal_sprintf(str[j++], "%s",
            ds_storm_control_field_value_dump(p_if, p_storm_control, i, buf));
    }
    return str;
}

int32
ds_storm_control_field_value_parser(char *str, int32 field_id, tbl_interface_t *p_if, ds_storm_control_t *p_storm_control)
{
    int32 ret=0;
    if (FLD_MATCH(DS_STORM_CONTROL_FLD_BCAST_MODE, field_id))
    {
        p_storm_control->bcast_mode = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_STORM_CONTROL_FLD_MCAST_MODE, field_id))
    {
        p_storm_control->mcast_mode = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_STORM_CONTROL_FLD_UCAST_MODE, field_id))
    {
        p_storm_control->ucast_mode = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_STORM_CONTROL_FLD_BCAST_RATE, field_id))
    {
        p_storm_control->bcast_rate = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_STORM_CONTROL_FLD_MCAST_RATE, field_id))
    {
        p_storm_control->mcast_rate = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_STORM_CONTROL_FLD_UCAST_RATE, field_id))
    {
        p_storm_control->ucast_rate = cdb_uint_str2val(str, &ret);
    }
    return ret;
}

int32
ds_storm_control_table_parser(char** array, char* tbl_key_value, char* ds_key_value,tbl_interface_t *p_if, ds_storm_control_t *p_storm_control)
{
    int32 rc;
    int32 i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_ds(DS_STORM_CONTROL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(ds_storm_control_key_str2val(tbl_key_value, ds_key_value, p_if, p_storm_control));

    for(i=1; i<DS_STORM_CONTROL_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(ds_storm_control_field_value_parser( array[j++], i, p_if, p_storm_control));
    }

    return PM_E_NONE;
}

int32
ds_storm_control_dump_one(ds_storm_control_t *p_storm_control, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_ds(DS_STORM_CONTROL);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(DS_STORM_CONTROL_FLD_BCAST_MODE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_STORM_CONTROL_FLD_BCAST_MODE].name,
            p_storm_control->bcast_mode);
    }
    if (FLD_MATCH(DS_STORM_CONTROL_FLD_MCAST_MODE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_STORM_CONTROL_FLD_MCAST_MODE].name,
            p_storm_control->mcast_mode);
    }
    if (FLD_MATCH(DS_STORM_CONTROL_FLD_UCAST_MODE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_STORM_CONTROL_FLD_UCAST_MODE].name,
            p_storm_control->ucast_mode);
    }
    if (FLD_MATCH(DS_STORM_CONTROL_FLD_BCAST_RATE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_STORM_CONTROL_FLD_BCAST_RATE].name,
            p_storm_control->bcast_rate);
    }
    if (FLD_MATCH(DS_STORM_CONTROL_FLD_MCAST_RATE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_STORM_CONTROL_FLD_MCAST_RATE].name,
            p_storm_control->mcast_rate);
    }
    if (FLD_MATCH(DS_STORM_CONTROL_FLD_UCAST_RATE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_STORM_CONTROL_FLD_UCAST_RATE].name,
            p_storm_control->ucast_rate);
    }

    return PM_E_NONE;
}
