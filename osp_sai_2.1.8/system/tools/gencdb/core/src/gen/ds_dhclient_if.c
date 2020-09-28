
#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "cdb_data_cmp.h"

int32
ds_dhclient_if_add_dhclient_if_sync(tbl_interface_t *p_if, ds_dhclient_if_t *p_dhclient, uint32 sync)
{
    tbl_interface_t *p_db_if = NULL;
    ds_dhclient_if_t *p_db_dhclient = NULL;

    /* 1. lookup entry exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_dhclient = p_db_if->dhclient_if;
    if (p_db_dhclient)
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_dhclient = XCALLOC(MEM_DS_DHCLIENT_IF, sizeof(ds_dhclient_if_t));
    if (NULL == p_db_dhclient)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_dhclient, p_dhclient, sizeof(ds_dhclient_if_t));

    /* 4. add to db */
    p_db_if->dhclient_if = p_db_dhclient;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_add(TBL_INTERFACE, DS_DHCLIENT_IF, p_db_if, p_db_dhclient);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_add(TBL_INTERFACE, DS_DHCLIENT_IF, p_db_if, p_db_dhclient);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_dhclient_if_del_dhclient_if_sync(tbl_interface_t *p_if, uint32 sync)
{
    tbl_interface_t *p_db_if = NULL;
    ds_dhclient_if_t *p_db_dhclient = NULL;

    /* 1. lookup entry exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_dhclient = p_db_if->dhclient_if;
    if (NULL == p_db_dhclient)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_del(TBL_INTERFACE, DS_DHCLIENT_IF, p_db_if, p_db_dhclient);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_del(TBL_INTERFACE, DS_DHCLIENT_IF, p_db_if, p_db_dhclient);
        #endif 
    }

    /* 3. delete from db */
    p_db_if->dhclient_if = NULL;

    /* 4. free db entry */
    XFREE(MEM_DS_DHCLIENT_IF, p_db_dhclient);

    return PM_E_NONE;
}

int32
ds_dhclient_if_set_dhclient_if_field_sync(tbl_interface_t *p_if, ds_dhclient_if_t *p_dhclient, ds_dhclient_if_field_id_t field_id, uint32 sync)
{
    tbl_interface_t *p_db_if = NULL;
    ds_dhclient_if_t *p_db_dhclient = NULL;

    /* 1. lookup entry exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }

    p_db_dhclient = ds_dhclient_if_get_dhclient_if(p_if);
    if (NULL == p_db_dhclient)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case DS_DHCLIENT_IF_FLD_REQUESTED_FLAGS:
        p_db_dhclient->requested_flags = p_dhclient->requested_flags;
        break;

    case DS_DHCLIENT_IF_FLD_LEASE:
        p_db_dhclient->lease = p_dhclient->lease;
        break;

    case DS_DHCLIENT_IF_FLD_STATUS:
        p_db_dhclient->status = p_dhclient->status;
        break;

    case DS_DHCLIENT_IF_FLD_OPTION:
        p_db_dhclient->option = p_dhclient->option;
        break;

    case DS_DHCLIENT_IF_FLD_CLIENT_FLAGS:
        p_db_dhclient->client_flags = p_dhclient->client_flags;
        break;

    case DS_DHCLIENT_IF_FLD_CLIENT_ID:
        sal_memcpy(p_db_dhclient->client_id, p_dhclient->client_id, sizeof(p_dhclient->client_id));
        break;

    case DS_DHCLIENT_IF_FLD_CLASS_FLAGS:
        p_db_dhclient->class_flags = p_dhclient->class_flags;
        break;

    case DS_DHCLIENT_IF_FLD_CLASS_ID:
        sal_memcpy(p_db_dhclient->class_id, p_dhclient->class_id, sizeof(p_dhclient->class_id));
        break;

    case DS_DHCLIENT_IF_FLD_HOSTNAME:
        sal_memcpy(p_db_dhclient->hostname, p_dhclient->hostname, sizeof(p_dhclient->hostname));
        break;

    case DS_DHCLIENT_IF_FLD_IANA_T1:
        p_db_dhclient->iana_t1 = p_dhclient->iana_t1;
        break;

    case DS_DHCLIENT_IF_FLD_IANA_T2:
        p_db_dhclient->iana_t2 = p_dhclient->iana_t2;
        break;

    case DS_DHCLIENT_IF_FLD_MAX:
        sal_memcpy(p_db_dhclient, p_dhclient, sizeof(ds_dhclient_if_t));
        break;

    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_set(TBL_INTERFACE, DS_DHCLIENT_IF, field_id, p_db_if, p_db_dhclient);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_set(TBL_INTERFACE, DS_DHCLIENT_IF, field_id, p_db_if, p_db_dhclient);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_dhclient_if_add_dhclient_if(tbl_interface_t *p_if, ds_dhclient_if_t *p_dhclient)
{
    return ds_dhclient_if_add_dhclient_if_sync(p_if, p_dhclient, TRUE);
}

int32
ds_dhclient_if_del_dhclient_if(tbl_interface_t *p_if)
{
    return ds_dhclient_if_del_dhclient_if_sync(p_if, TRUE);
}

int32
ds_dhclient_if_set_dhclient_if_field(tbl_interface_t *p_if, ds_dhclient_if_t *p_dhclient, ds_dhclient_if_field_id_t field_id)
{
    return ds_dhclient_if_set_dhclient_if_field_sync(p_if, p_dhclient, field_id, TRUE);
}

ds_dhclient_if_t*
ds_dhclient_if_get_dhclient_if(tbl_interface_t *p_if)
{
    tbl_interface_t *p_db_if = NULL;

    /* 1. lookup entry exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return NULL;
    }

    return p_db_if->dhclient_if;
}

int32
ds_dhclient_if_key_str2val(char* tbl_key_value, char* str, tbl_interface_t *p_if, ds_dhclient_if_t *p_dhclient)
{

    int32 ret = 0;

    ret = tbl_interface_key_str2val(tbl_key_value, p_if);
    return ret;
}
char*
ds_dhclient_if_key_value_dump(tbl_interface_t *p_if, ds_dhclient_if_t *p_dhclient, char* str)
{

    sal_sprintf(str, "%s",
        tbl_interface_key_value_dump(p_if, str));

    return str;
}

char*
ds_dhclient_if_key_name_dump(tbl_interface_t *p_if, ds_dhclient_if_t *p_dhclient, char* str)
{

    cdb_node_t *p_node_tbl = cdb_get_tbl(TBL_INTERFACE);
    cdb_node_t *p_node_ds = cdb_get_ds(DS_DHCLIENT_IF);
    char tbl_str[MAX_CMD_STR_LEN];
    char ds_str[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s",
        p_node_tbl->name, tbl_interface_key_value_dump(p_if, tbl_str), p_node_ds->name, ds_dhclient_if_key_value_dump(p_if, p_dhclient, ds_str));
    return str;
}

char*
ds_dhclient_if_field_name_dump(tbl_interface_t *p_if, ds_dhclient_if_t *p_dhclient, int32 field_id, char* str)
{

    cdb_node_t *p_node = cdb_get_ds(DS_DHCLIENT_IF);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= DS_DHCLIENT_IF_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
ds_dhclient_if_field_value_dump(tbl_interface_t *p_if, ds_dhclient_if_t *p_dhclient, int32 field_id, char* str)
{

    if (FLD_MATCH(DS_DHCLIENT_IF_FLD_REQUESTED_FLAGS, field_id))
    {
        sal_sprintf(str, "%u", p_dhclient->requested_flags);
    }
    if (FLD_MATCH(DS_DHCLIENT_IF_FLD_LEASE, field_id))
    {
        sal_sprintf(str, "%u", p_dhclient->lease);
    }
    if (FLD_MATCH(DS_DHCLIENT_IF_FLD_STATUS, field_id))
    {
        sal_sprintf(str, "%u", p_dhclient->status);
    }
    if (FLD_MATCH(DS_DHCLIENT_IF_FLD_OPTION, field_id))
    {
        sal_sprintf(str, "%u", p_dhclient->option);
    }
    if (FLD_MATCH(DS_DHCLIENT_IF_FLD_CLIENT_FLAGS, field_id))
    {
        sal_sprintf(str, "%u", p_dhclient->client_flags);
    }
    if (FLD_MATCH(DS_DHCLIENT_IF_FLD_CLIENT_ID, field_id))
    {
        sal_sprintf(str, "%s", p_dhclient->client_id);
    }
    if (FLD_MATCH(DS_DHCLIENT_IF_FLD_CLASS_FLAGS, field_id))
    {
        sal_sprintf(str, "%u", p_dhclient->class_flags);
    }
    if (FLD_MATCH(DS_DHCLIENT_IF_FLD_CLASS_ID, field_id))
    {
        sal_sprintf(str, "%s", p_dhclient->class_id);
    }
    if (FLD_MATCH(DS_DHCLIENT_IF_FLD_HOSTNAME, field_id))
    {
        sal_sprintf(str, "%s", p_dhclient->hostname);
    }
    if (FLD_MATCH(DS_DHCLIENT_IF_FLD_IANA_T1, field_id))
    {
        sal_sprintf(str, "%u", p_dhclient->iana_t1);
    }
    if (FLD_MATCH(DS_DHCLIENT_IF_FLD_IANA_T2, field_id))
    {
        sal_sprintf(str, "%u", p_dhclient->iana_t2);
    }
    return str;
}

char**
ds_dhclient_if_table_dump(tbl_interface_t *p_if, ds_dhclient_if_t *p_dhclient, char** str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         ds_dhclient_if_key_name_dump(p_if, p_dhclient, buf));
    for(i=1; i<DS_DHCLIENT_IF_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            ds_dhclient_if_field_name_dump(p_if, p_dhclient, i, buf));
        sal_sprintf(str[j++], "%s",
            ds_dhclient_if_field_value_dump(p_if, p_dhclient, i, buf));
    }
    return str;
}

int32
ds_dhclient_if_field_value_parser(char *str, int32 field_id, tbl_interface_t *p_if, ds_dhclient_if_t *p_dhclient)
{
    int32 ret=0;
    if (FLD_MATCH(DS_DHCLIENT_IF_FLD_REQUESTED_FLAGS, field_id))
    {
        p_dhclient->requested_flags = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_DHCLIENT_IF_FLD_LEASE, field_id))
    {
        p_dhclient->lease = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_DHCLIENT_IF_FLD_STATUS, field_id))
    {
        p_dhclient->status = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_DHCLIENT_IF_FLD_OPTION, field_id))
    {
        p_dhclient->option = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_DHCLIENT_IF_FLD_CLIENT_FLAGS, field_id))
    {
        p_dhclient->client_flags = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_DHCLIENT_IF_FLD_CLIENT_ID, field_id))
    {
        sal_strcpy(p_dhclient->client_id, str);
    }
    if (FLD_MATCH(DS_DHCLIENT_IF_FLD_CLASS_FLAGS, field_id))
    {
        p_dhclient->class_flags = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_DHCLIENT_IF_FLD_CLASS_ID, field_id))
    {
        sal_strcpy(p_dhclient->class_id, str);
    }
    if (FLD_MATCH(DS_DHCLIENT_IF_FLD_HOSTNAME, field_id))
    {
        sal_strcpy(p_dhclient->hostname, str);
    }
    if (FLD_MATCH(DS_DHCLIENT_IF_FLD_IANA_T1, field_id))
    {
        p_dhclient->iana_t1 = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_DHCLIENT_IF_FLD_IANA_T2, field_id))
    {
        p_dhclient->iana_t2 = cdb_uint_str2val(str, &ret);
    }
    return ret;
}

int32
ds_dhclient_if_table_parser(char** array, char* tbl_key_value, char* ds_key_value,tbl_interface_t *p_if, ds_dhclient_if_t *p_dhclient)
{
    int32 rc;
    int32 i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_ds(DS_DHCLIENT_IF);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(ds_dhclient_if_key_str2val(tbl_key_value, ds_key_value, p_if, p_dhclient));

    for(i=1; i<DS_DHCLIENT_IF_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(ds_dhclient_if_field_value_parser( array[j++], i, p_if, p_dhclient));
    }

    return PM_E_NONE;
}

int32
ds_dhclient_if_dump_one(ds_dhclient_if_t *p_dhclient, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_ds(DS_DHCLIENT_IF);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(DS_DHCLIENT_IF_FLD_REQUESTED_FLAGS, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_DHCLIENT_IF_FLD_REQUESTED_FLAGS].name,
            p_dhclient->requested_flags);
    }
    if (FLD_MATCH(DS_DHCLIENT_IF_FLD_LEASE, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_DHCLIENT_IF_FLD_LEASE].name,
            p_dhclient->lease);
    }
    if (FLD_MATCH(DS_DHCLIENT_IF_FLD_STATUS, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_DHCLIENT_IF_FLD_STATUS].name,
            p_dhclient->status);
    }
    if (FLD_MATCH(DS_DHCLIENT_IF_FLD_OPTION, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_DHCLIENT_IF_FLD_OPTION].name,
            p_dhclient->option);
    }
    if (FLD_MATCH(DS_DHCLIENT_IF_FLD_CLIENT_FLAGS, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_DHCLIENT_IF_FLD_CLIENT_FLAGS].name,
            p_dhclient->client_flags);
    }
    if (FLD_MATCH(DS_DHCLIENT_IF_FLD_CLIENT_ID, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_DHCLIENT_IF_FLD_CLIENT_ID].name,
            p_dhclient->client_id);
    }
    if (FLD_MATCH(DS_DHCLIENT_IF_FLD_CLASS_FLAGS, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_DHCLIENT_IF_FLD_CLASS_FLAGS].name,
            p_dhclient->class_flags);
    }
    if (FLD_MATCH(DS_DHCLIENT_IF_FLD_CLASS_ID, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_DHCLIENT_IF_FLD_CLASS_ID].name,
            p_dhclient->class_id);
    }
    if (FLD_MATCH(DS_DHCLIENT_IF_FLD_HOSTNAME, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_DHCLIENT_IF_FLD_HOSTNAME].name,
            p_dhclient->hostname);
    }
    if (FLD_MATCH(DS_DHCLIENT_IF_FLD_IANA_T1, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_DHCLIENT_IF_FLD_IANA_T1].name,
            p_dhclient->iana_t1);
    }
    if (FLD_MATCH(DS_DHCLIENT_IF_FLD_IANA_T2, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_DHCLIENT_IF_FLD_IANA_T2].name,
            p_dhclient->iana_t2);
    }

    return PM_E_NONE;
}
