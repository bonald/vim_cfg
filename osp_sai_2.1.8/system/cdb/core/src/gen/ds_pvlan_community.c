
#include "proto.h"
#include "gen/tbl_pvlan_define.h"
#include "gen/tbl_pvlan.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "cdb_data_cmp.h"

int32
ds_pvlan_community_add_pvlan_community_sync(tbl_pvlan_t *p_pvlan, ds_pvlan_community_t *p_community, uint32 sync)
{
    tbl_pvlan_t *p_db_pvlan = NULL;
    ds_pvlan_community_t *p_db_community = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_pvlan = tbl_pvlan_get_pvlan(&p_pvlan->key);
    if (NULL == p_db_pvlan)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_community = ds_pvlan_community_get_pvlan_community(p_pvlan, p_community);
    if (p_db_community)
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_community = XCALLOC(MEM_DS_PVLAN_COMMUNITY, sizeof(ds_pvlan_community_t));
    if (NULL == p_db_community)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_community, p_community, sizeof(ds_pvlan_community_t));
    PM_E_RETURN(cdb_reference_list_init(&p_db_community->ports, CDB_NODE_TBL, TBL_INTERFACE, tbl_interface_cmp));

    /* 4. add to db */
    PM_E_RETURN(cdb_reference_list_add_object(&p_db_pvlan->community_groups, p_db_community));

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_add(TBL_PVLAN, DS_PVLAN_COMMUNITY, p_db_pvlan, p_db_community);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_add(TBL_PVLAN, DS_PVLAN_COMMUNITY, p_db_pvlan, p_db_community);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_pvlan_community_del_pvlan_community_sync(tbl_pvlan_t *p_pvlan, ds_pvlan_community_t *p_community, uint32 sync)
{
    tbl_pvlan_t *p_db_pvlan = NULL;
    ds_pvlan_community_t *p_db_community = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_pvlan = tbl_pvlan_get_pvlan(&p_pvlan->key);
    if (NULL == p_db_pvlan)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_community = ds_pvlan_community_get_pvlan_community(p_pvlan, p_community);
    if (NULL == p_db_community)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_del(TBL_PVLAN, DS_PVLAN_COMMUNITY, p_db_pvlan, p_db_community);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_del(TBL_PVLAN, DS_PVLAN_COMMUNITY, p_db_pvlan, p_db_community);
        #endif 
    }

    PM_E_RETURN(cdb_reference_list_deinit(&p_db_community->ports));

    /* 3. delete from db */
    PM_E_RETURN(cdb_reference_list_del_object(&p_db_pvlan->community_groups, p_db_community));

    /* 4. free db entry */
    XFREE(MEM_DS_PVLAN_COMMUNITY, p_db_community);

    return PM_E_NONE;
}

int32
ds_pvlan_community_set_pvlan_community_field_sync(tbl_pvlan_t *p_pvlan, ds_pvlan_community_t *p_community, ds_pvlan_community_field_id_t field_id, uint32 sync)
{
    tbl_pvlan_t *p_db_pvlan = NULL;
    ds_pvlan_community_t *p_db_community = NULL;

    /* 1. lookup entry exist */
    p_db_pvlan = tbl_pvlan_get_pvlan(&p_pvlan->key);
    if (NULL == p_db_pvlan)
    {
        return PM_E_NOT_EXIST;
    }

    p_db_community = ds_pvlan_community_get_pvlan_community(p_pvlan, p_community);
    if (NULL == p_db_community)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case DS_PVLAN_COMMUNITY_FLD_GROUP_ID:
        p_db_community->group_id = p_community->group_id;
        break;

    case DS_PVLAN_COMMUNITY_FLD_MAX:
        sal_memcpy(p_db_community, p_community, sizeof(ds_pvlan_community_t));
        break;

    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_set(TBL_PVLAN, DS_PVLAN_COMMUNITY, field_id, p_db_pvlan, p_db_community);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_set(TBL_PVLAN, DS_PVLAN_COMMUNITY, field_id, p_db_pvlan, p_db_community);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_pvlan_community_add_pvlan_community(tbl_pvlan_t *p_pvlan, ds_pvlan_community_t *p_community)
{
    return ds_pvlan_community_add_pvlan_community_sync(p_pvlan, p_community, TRUE);
}

int32
ds_pvlan_community_del_pvlan_community(tbl_pvlan_t *p_pvlan, ds_pvlan_community_t *p_community)
{
    return ds_pvlan_community_del_pvlan_community_sync(p_pvlan, p_community, TRUE);
}

int32
ds_pvlan_community_set_pvlan_community_field(tbl_pvlan_t *p_pvlan, ds_pvlan_community_t *p_community, ds_pvlan_community_field_id_t field_id)
{
    return ds_pvlan_community_set_pvlan_community_field_sync(p_pvlan, p_community, field_id, TRUE);
}

ds_pvlan_community_t*
ds_pvlan_community_get_pvlan_community(tbl_pvlan_t *p_pvlan, ds_pvlan_community_t *p_community)
{
    tbl_pvlan_t *p_db_pvlan = NULL;

    /* 1. lookup entry exist */
    p_db_pvlan = tbl_pvlan_get_pvlan(&p_pvlan->key);
    if (NULL == p_db_pvlan)
    {
        return NULL;
    }

    return cdb_reference_list_lookup_object(&p_db_pvlan->community_groups, p_community);
}

int32
ds_pvlan_community_key_str2val(char* tbl_key_value, char* str, tbl_pvlan_t *p_pvlan, ds_pvlan_community_t *p_community)
{

    int32 ret = 0;

    ret = tbl_pvlan_key_str2val(tbl_key_value, p_pvlan);

    p_community->key.community_vid = cdb_uint_str2val(str, &ret);

    return ret;
}

char*
ds_pvlan_community_key_value_dump(tbl_pvlan_t *p_pvlan, ds_pvlan_community_t *p_community, char* str)
{

    sal_sprintf(str, "%u", p_community->key.community_vid);
    return str;
}

char*
ds_pvlan_community_key_name_dump(tbl_pvlan_t *p_pvlan, ds_pvlan_community_t *p_community, char* str)
{

    cdb_node_t *p_node_tbl = cdb_get_tbl(TBL_PVLAN);
    cdb_node_t *p_node_ds = cdb_get_ds(DS_PVLAN_COMMUNITY);
    char tbl_str[MAX_CMD_STR_LEN];
    char ds_str[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s",
        p_node_tbl->name, tbl_pvlan_key_value_dump(p_pvlan, tbl_str), p_node_ds->name, ds_pvlan_community_key_value_dump(p_pvlan, p_community, ds_str));
    return str;
}

char*
ds_pvlan_community_field_name_dump(tbl_pvlan_t *p_pvlan, ds_pvlan_community_t *p_community, int32 field_id, char* str)
{

    cdb_node_t *p_node = cdb_get_ds(DS_PVLAN_COMMUNITY);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= DS_PVLAN_COMMUNITY_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
ds_pvlan_community_field_value_dump(tbl_pvlan_t *p_pvlan, ds_pvlan_community_t *p_community, int32 field_id, char* str)
{

    if (FLD_MATCH(DS_PVLAN_COMMUNITY_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_community->key.community_vid);
    }
    if (FLD_MATCH(DS_PVLAN_COMMUNITY_FLD_GROUP_ID, field_id))
    {
        sal_sprintf(str, "%u", p_community->group_id);
    }
    if (FLD_MATCH(DS_PVLAN_COMMUNITY_FLD_PORTS, field_id))
    {
    }
    return str;
}

char**
ds_pvlan_community_table_dump(tbl_pvlan_t *p_pvlan, ds_pvlan_community_t *p_community, char** str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         ds_pvlan_community_key_name_dump(p_pvlan, p_community, buf));
    for(i=1; i<DS_PVLAN_COMMUNITY_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            ds_pvlan_community_field_name_dump(p_pvlan, p_community, i, buf));
        sal_sprintf(str[j++], "%s",
            ds_pvlan_community_field_value_dump(p_pvlan, p_community, i, buf));
    }
    return str;
}

int32
ds_pvlan_community_field_value_parser(char *str, int32 field_id, tbl_pvlan_t *p_pvlan, ds_pvlan_community_t *p_community)
{
    int32 ret=0;
    if (FLD_MATCH(DS_PVLAN_COMMUNITY_FLD_KEY, field_id))
    {
        p_community->key.community_vid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_PVLAN_COMMUNITY_FLD_GROUP_ID, field_id))
    {
        p_community->group_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_PVLAN_COMMUNITY_FLD_PORTS, field_id))
    {
    }
    return ret;
}

int32
ds_pvlan_community_table_parser(char** array, char* tbl_key_value, char* ds_key_value,tbl_pvlan_t *p_pvlan, ds_pvlan_community_t *p_community)
{
    int32 rc;
    int32 i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_ds(DS_PVLAN_COMMUNITY);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(ds_pvlan_community_key_str2val(tbl_key_value, ds_key_value, p_pvlan, p_community));

    for(i=1; i<DS_PVLAN_COMMUNITY_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(ds_pvlan_community_field_value_parser( array[j++], i, p_pvlan, p_community));
    }

    return PM_E_NONE;
}

int32
ds_pvlan_community_dump_one(ds_pvlan_community_t *p_community, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_ds(DS_PVLAN_COMMUNITY);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(DS_PVLAN_COMMUNITY_FLD_KEY, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_PVLAN_COMMUNITY_FLD_KEY].name,
            p_community->key.community_vid);
    }
    if (FLD_MATCH(DS_PVLAN_COMMUNITY_FLD_GROUP_ID, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_PVLAN_COMMUNITY_FLD_GROUP_ID].name,
            p_community->group_id);
    }
    if (FLD_MATCH(DS_PVLAN_COMMUNITY_FLD_PORTS, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_PVLAN_COMMUNITY_FLD_PORTS].name,
            cdb_reference_list_val2str(buf, MAX_CMD_STR_LEN, &p_community->ports));
    }

    return PM_E_NONE;
}

int32
ds_pvlan_community_set_ports_sync(tbl_pvlan_t *p_pvlan, ds_pvlan_community_t *p_community, uint32 add, void *p_object, uint32 sync)
{
    cdb_sync_refer_hdr_t refer;
    ds_pvlan_community_t *p_db_community = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_community = ds_pvlan_community_get_pvlan_community(p_pvlan, p_community);
    if (NULL == p_db_community)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    if (add)
    {
        PM_E_RETURN(cdb_reference_list_add_object(&p_db_community->ports, p_object));
    }
    else
    {
        PM_E_RETURN(cdb_reference_list_del_object(&p_db_community->ports, p_object));
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_build_refer_tbl(TBL_INTERFACE, add, &refer);
        cdb_sync_ds_set_refer(TBL_PVLAN, DS_PVLAN_COMMUNITY, DS_PVLAN_COMMUNITY_FLD_PORTS, p_pvlan, p_community, &refer, p_object);
    }

    return PM_E_NONE;
}

int32
ds_pvlan_community_set_ports(tbl_pvlan_t *p_pvlan, ds_pvlan_community_t *p_community, uint32 add, void *p_object)
{
    return ds_pvlan_community_set_ports_sync(p_pvlan, p_community, add, p_object, TRUE);
}

