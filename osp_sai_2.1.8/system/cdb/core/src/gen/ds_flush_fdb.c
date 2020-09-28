
#include "proto.h"
#include "gen/tbl_l2_action_define.h"
#include "gen/tbl_l2_action.h"
#include "cdb_data_cmp.h"

int32
ds_flush_fdb_add_flush_fdb_sync(tbl_l2_action_t *p_l2_act, ds_flush_fdb_t *p_flush_fdb, uint32 sync)
{
    tbl_l2_action_t *p_db_l2_act = NULL;
    ds_flush_fdb_t *p_db_flush_fdb = NULL;

    /* 1. lookup entry exist */
    p_db_l2_act = tbl_l2_action_get_l2_action();
    if (NULL == p_db_l2_act)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_flush_fdb = p_db_l2_act->flush_fdb;
    if (p_db_flush_fdb)
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_flush_fdb = XCALLOC(MEM_DS_FLUSH_FDB, sizeof(ds_flush_fdb_t));
    if (NULL == p_db_flush_fdb)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_flush_fdb, p_flush_fdb, sizeof(ds_flush_fdb_t));

    /* 4. add to db */
    p_db_l2_act->flush_fdb = p_db_flush_fdb;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_add(TBL_L2_ACTION, DS_FLUSH_FDB, p_db_l2_act, p_db_flush_fdb);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_add(TBL_L2_ACTION, DS_FLUSH_FDB, p_db_l2_act, p_db_flush_fdb);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_flush_fdb_del_flush_fdb_sync(tbl_l2_action_t *p_l2_act, uint32 sync)
{
    tbl_l2_action_t *p_db_l2_act = NULL;
    ds_flush_fdb_t *p_db_flush_fdb = NULL;

    /* 1. lookup entry exist */
    p_db_l2_act = tbl_l2_action_get_l2_action();
    if (NULL == p_db_l2_act)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_flush_fdb = p_db_l2_act->flush_fdb;
    if (NULL == p_db_flush_fdb)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_del(TBL_L2_ACTION, DS_FLUSH_FDB, p_db_l2_act, p_db_flush_fdb);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_del(TBL_L2_ACTION, DS_FLUSH_FDB, p_db_l2_act, p_db_flush_fdb);
        #endif 
    }

    /* 3. delete from db */
    p_db_l2_act->flush_fdb = NULL;

    /* 4. free db entry */
    XFREE(MEM_DS_FLUSH_FDB, p_db_flush_fdb);

    return PM_E_NONE;
}

int32
ds_flush_fdb_set_flush_fdb_field_sync(tbl_l2_action_t *p_l2_act, ds_flush_fdb_t *p_flush_fdb, ds_flush_fdb_field_id_t field_id, uint32 sync)
{
    tbl_l2_action_t *p_db_l2_act = NULL;
    ds_flush_fdb_t *p_db_flush_fdb = NULL;

    /* 1. lookup entry exist */
    p_db_l2_act = tbl_l2_action_get_l2_action();
    if (NULL == p_db_l2_act)
    {
        return PM_E_NOT_EXIST;
    }

    p_db_flush_fdb = ds_flush_fdb_get_flush_fdb(p_l2_act);
    if (NULL == p_db_flush_fdb)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case DS_FLUSH_FDB_FLD_TYPE:
        p_db_flush_fdb->type = p_flush_fdb->type;
        break;

    case DS_FLUSH_FDB_FLD_SUBTYPE:
        p_db_flush_fdb->subtype = p_flush_fdb->subtype;
        break;

    case DS_FLUSH_FDB_FLD_MODE:
        p_db_flush_fdb->mode = p_flush_fdb->mode;
        break;

    case DS_FLUSH_FDB_FLD_IFINDEX:
        p_db_flush_fdb->ifindex = p_flush_fdb->ifindex;
        break;

    case DS_FLUSH_FDB_FLD_VID:
        p_db_flush_fdb->vid = p_flush_fdb->vid;
        break;

    case DS_FLUSH_FDB_FLD_MAC:
        sal_memcpy(p_db_flush_fdb->mac, p_flush_fdb->mac, sizeof(p_flush_fdb->mac));
        break;

    case DS_FLUSH_FDB_FLD_MAX:
        sal_memcpy(p_db_flush_fdb, p_flush_fdb, sizeof(ds_flush_fdb_t));
        break;

    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_set(TBL_L2_ACTION, DS_FLUSH_FDB, field_id, p_db_l2_act, p_db_flush_fdb);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_set(TBL_L2_ACTION, DS_FLUSH_FDB, field_id, p_db_l2_act, p_db_flush_fdb);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_flush_fdb_add_flush_fdb(tbl_l2_action_t *p_l2_act, ds_flush_fdb_t *p_flush_fdb)
{
    return ds_flush_fdb_add_flush_fdb_sync(p_l2_act, p_flush_fdb, TRUE);
}

int32
ds_flush_fdb_del_flush_fdb(tbl_l2_action_t *p_l2_act)
{
    return ds_flush_fdb_del_flush_fdb_sync(p_l2_act, TRUE);
}

int32
ds_flush_fdb_set_flush_fdb_field(tbl_l2_action_t *p_l2_act, ds_flush_fdb_t *p_flush_fdb, ds_flush_fdb_field_id_t field_id)
{
    return ds_flush_fdb_set_flush_fdb_field_sync(p_l2_act, p_flush_fdb, field_id, TRUE);
}

ds_flush_fdb_t*
ds_flush_fdb_get_flush_fdb(tbl_l2_action_t *p_l2_act)
{
    tbl_l2_action_t *p_db_l2_act = NULL;

    /* 1. lookup entry exist */
    p_db_l2_act = tbl_l2_action_get_l2_action();
    if (NULL == p_db_l2_act)
    {
        return NULL;
    }

    return p_db_l2_act->flush_fdb;
}

int32
ds_flush_fdb_key_str2val(char* tbl_key_value, char* str, tbl_l2_action_t *p_l2_act, ds_flush_fdb_t *p_flush_fdb)
{

    int32 ret = 0;

    ret = tbl_l2_action_key_str2val(tbl_key_value, p_l2_act);
    return ret;
}
char*
ds_flush_fdb_key_value_dump(tbl_l2_action_t *p_l2_act, ds_flush_fdb_t *p_flush_fdb, char* str)
{

    sal_sprintf(str, "%s",
        tbl_l2_action_key_value_dump(p_l2_act, str));

    return str;
}

char*
ds_flush_fdb_key_name_dump(tbl_l2_action_t *p_l2_act, ds_flush_fdb_t *p_flush_fdb, char* str)
{

    cdb_node_t *p_node_tbl = cdb_get_tbl(TBL_L2_ACTION);
    cdb_node_t *p_node_ds = cdb_get_ds(DS_FLUSH_FDB);
    char tbl_str[MAX_CMD_STR_LEN];
    char ds_str[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s",
        p_node_tbl->name, tbl_l2_action_key_value_dump(p_l2_act, tbl_str), p_node_ds->name, ds_flush_fdb_key_value_dump(p_l2_act, p_flush_fdb, ds_str));
    return str;
}

char*
ds_flush_fdb_field_name_dump(tbl_l2_action_t *p_l2_act, ds_flush_fdb_t *p_flush_fdb, int32 field_id, char* str)
{

    cdb_node_t *p_node = cdb_get_ds(DS_FLUSH_FDB);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= DS_FLUSH_FDB_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
ds_flush_fdb_field_value_dump(tbl_l2_action_t *p_l2_act, ds_flush_fdb_t *p_flush_fdb, int32 field_id, char* str)
{

    if (FLD_MATCH(DS_FLUSH_FDB_FLD_TYPE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(flush_fdb_type_strs, FLUSH_FDB_TYPE_MAX, p_flush_fdb->type));
    }
    if (FLD_MATCH(DS_FLUSH_FDB_FLD_SUBTYPE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(flush_fdb_subtype_strs, FLUSH_FDB_SUBTYPE_MAX, p_flush_fdb->subtype));
    }
    if (FLD_MATCH(DS_FLUSH_FDB_FLD_MODE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(flush_fdb_mode_strs, FLUSH_FDB_MODE_MAX, p_flush_fdb->mode));
    }
    if (FLD_MATCH(DS_FLUSH_FDB_FLD_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_flush_fdb->ifindex);
    }
    if (FLD_MATCH(DS_FLUSH_FDB_FLD_VID, field_id))
    {
        sal_sprintf(str, "%u", p_flush_fdb->vid);
    }
    if (FLD_MATCH(DS_FLUSH_FDB_FLD_MAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_flush_fdb->mac);
    }
    return str;
}

char**
ds_flush_fdb_table_dump(tbl_l2_action_t *p_l2_act, ds_flush_fdb_t *p_flush_fdb, char** str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         ds_flush_fdb_key_name_dump(p_l2_act, p_flush_fdb, buf));
    for(i=1; i<DS_FLUSH_FDB_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            ds_flush_fdb_field_name_dump(p_l2_act, p_flush_fdb, i, buf));
        sal_sprintf(str[j++], "%s",
            ds_flush_fdb_field_value_dump(p_l2_act, p_flush_fdb, i, buf));
    }
    return str;
}

int32
ds_flush_fdb_field_value_parser(char *str, int32 field_id, tbl_l2_action_t *p_l2_act, ds_flush_fdb_t *p_flush_fdb)
{
    int32 ret=0;
    if (FLD_MATCH(DS_FLUSH_FDB_FLD_TYPE, field_id))
    {
        cdb_enum_str2val(flush_fdb_type_strs, FLUSH_FDB_TYPE_MAX, str);
    }
    if (FLD_MATCH(DS_FLUSH_FDB_FLD_SUBTYPE, field_id))
    {
        cdb_enum_str2val(flush_fdb_subtype_strs, FLUSH_FDB_SUBTYPE_MAX, str);
    }
    if (FLD_MATCH(DS_FLUSH_FDB_FLD_MODE, field_id))
    {
        cdb_enum_str2val(flush_fdb_mode_strs, FLUSH_FDB_MODE_MAX, str);
    }
    if (FLD_MATCH(DS_FLUSH_FDB_FLD_IFINDEX, field_id))
    {
        p_flush_fdb->ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_FLUSH_FDB_FLD_VID, field_id))
    {
        p_flush_fdb->vid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_FLUSH_FDB_FLD_MAC, field_id))
    {
        cdb_mac_addr_str2val(p_flush_fdb->mac, str);
    }
    return ret;
}

int32
ds_flush_fdb_table_parser(char** array, char* tbl_key_value, char* ds_key_value,tbl_l2_action_t *p_l2_act, ds_flush_fdb_t *p_flush_fdb)
{
    int32 rc;
    int32 i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_ds(DS_FLUSH_FDB);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(ds_flush_fdb_key_str2val(tbl_key_value, ds_key_value, p_l2_act, p_flush_fdb));

    for(i=1; i<DS_FLUSH_FDB_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(ds_flush_fdb_field_value_parser( array[j++], i, p_l2_act, p_flush_fdb));
    }

    return PM_E_NONE;
}

int32
ds_flush_fdb_dump_one(ds_flush_fdb_t *p_flush_fdb, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_ds(DS_FLUSH_FDB);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(DS_FLUSH_FDB_FLD_TYPE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_FLUSH_FDB_FLD_TYPE].name,
            cdb_enum_val2str(flush_fdb_type_strs, FLUSH_FDB_TYPE_MAX, p_flush_fdb->type));
    }
    if (FLD_MATCH(DS_FLUSH_FDB_FLD_SUBTYPE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_FLUSH_FDB_FLD_SUBTYPE].name,
            cdb_enum_val2str(flush_fdb_subtype_strs, FLUSH_FDB_SUBTYPE_MAX, p_flush_fdb->subtype));
    }
    if (FLD_MATCH(DS_FLUSH_FDB_FLD_MODE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_FLUSH_FDB_FLD_MODE].name,
            cdb_enum_val2str(flush_fdb_mode_strs, FLUSH_FDB_MODE_MAX, p_flush_fdb->mode));
    }
    if (FLD_MATCH(DS_FLUSH_FDB_FLD_IFINDEX, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_FLUSH_FDB_FLD_IFINDEX].name,
            p_flush_fdb->ifindex);
    }
    if (FLD_MATCH(DS_FLUSH_FDB_FLD_VID, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_FLUSH_FDB_FLD_VID].name,
            p_flush_fdb->vid);
    }
    if (FLD_MATCH(DS_FLUSH_FDB_FLD_MAC, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_FLUSH_FDB_FLD_MAC].name,
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_flush_fdb->mac));
    }

    return PM_E_NONE;
}
