
#include "proto.h"
#include "gen/tbl_cfm_md_define.h"
#include "gen/tbl_cfm_md.h"
#include "cdb_data_cmp.h"

int32
ds_cfm_error_add_cfm_error_sync(tbl_cfm_md_t *p_md, ds_cfm_error_t *p_cfm_error, uint32 sync)
{
    tbl_cfm_md_t *p_db_md = NULL;
    ds_cfm_error_t *p_db_cfm_error = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_md = tbl_cfm_md_get_cfm_md(&p_md->key);
    if (NULL == p_db_md)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_cfm_error = ds_cfm_error_get_cfm_error(p_md, p_cfm_error);
    if (p_db_cfm_error)
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_cfm_error = XCALLOC(MEM_DS_CFM_ERROR, sizeof(ds_cfm_error_t));
    if (NULL == p_db_cfm_error)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_cfm_error, p_cfm_error, sizeof(ds_cfm_error_t));

    /* 4. add to db */
    PM_E_RETURN(cdb_reference_list_add_object(&p_db_md->cfm_error, p_db_cfm_error));

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_add(TBL_CFM_MD, DS_CFM_ERROR, p_db_md, p_db_cfm_error);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_add(TBL_CFM_MD, DS_CFM_ERROR, p_db_md, p_db_cfm_error);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_cfm_error_del_cfm_error_sync(tbl_cfm_md_t *p_md, ds_cfm_error_t *p_cfm_error, uint32 sync)
{
    tbl_cfm_md_t *p_db_md = NULL;
    ds_cfm_error_t *p_db_cfm_error = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_md = tbl_cfm_md_get_cfm_md(&p_md->key);
    if (NULL == p_db_md)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_cfm_error = ds_cfm_error_get_cfm_error(p_md, p_cfm_error);
    if (NULL == p_db_cfm_error)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_del(TBL_CFM_MD, DS_CFM_ERROR, p_db_md, p_db_cfm_error);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_del(TBL_CFM_MD, DS_CFM_ERROR, p_db_md, p_db_cfm_error);
        #endif 
    }

    /* 3. delete from db */
    PM_E_RETURN(cdb_reference_list_del_object(&p_db_md->cfm_error, p_db_cfm_error));

    /* 4. free db entry */
    XFREE(MEM_DS_CFM_ERROR, p_db_cfm_error);

    return PM_E_NONE;
}

int32
ds_cfm_error_set_cfm_error_field_sync(tbl_cfm_md_t *p_md, ds_cfm_error_t *p_cfm_error, ds_cfm_error_field_id_t field_id, uint32 sync)
{
    tbl_cfm_md_t *p_db_md = NULL;
    ds_cfm_error_t *p_db_cfm_error = NULL;

    /* 1. lookup entry exist */
    p_db_md = tbl_cfm_md_get_cfm_md(&p_md->key);
    if (NULL == p_db_md)
    {
        return PM_E_NOT_EXIST;
    }

    p_db_cfm_error = ds_cfm_error_get_cfm_error(p_md, p_cfm_error);
    if (NULL == p_db_cfm_error)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case DS_CFM_ERROR_FLD_LEVEL:
        p_db_cfm_error->level = p_cfm_error->level;
        break;

    case DS_CFM_ERROR_FLD_VLAN_ID:
        p_db_cfm_error->vlan_id = p_cfm_error->vlan_id;
        break;

    case DS_CFM_ERROR_FLD_MD_NAME:
        sal_memcpy(p_db_cfm_error->md_name, p_cfm_error->md_name, sizeof(p_cfm_error->md_name));
        break;

    case DS_CFM_ERROR_FLD_MEP_ID:
        p_db_cfm_error->mep_id = p_cfm_error->mep_id;
        break;

    case DS_CFM_ERROR_FLD_ERR_REASON:
        p_db_cfm_error->err_reason = p_cfm_error->err_reason;
        break;

    case DS_CFM_ERROR_FLD_RMEP_MAC:
        sal_memcpy(p_db_cfm_error->rmep_mac, p_cfm_error->rmep_mac, sizeof(p_cfm_error->rmep_mac));
        break;

    case DS_CFM_ERROR_FLD_MAX:
        sal_memcpy(p_db_cfm_error, p_cfm_error, sizeof(ds_cfm_error_t));
        break;

    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_ds_set(TBL_CFM_MD, DS_CFM_ERROR, field_id, p_db_md, p_db_cfm_error);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_ds_set(TBL_CFM_MD, DS_CFM_ERROR, field_id, p_db_md, p_db_cfm_error);
        #endif 
    }

    return PM_E_NONE;
}

int32
ds_cfm_error_add_cfm_error(tbl_cfm_md_t *p_md, ds_cfm_error_t *p_cfm_error)
{
    return ds_cfm_error_add_cfm_error_sync(p_md, p_cfm_error, TRUE);
}

int32
ds_cfm_error_del_cfm_error(tbl_cfm_md_t *p_md, ds_cfm_error_t *p_cfm_error)
{
    return ds_cfm_error_del_cfm_error_sync(p_md, p_cfm_error, TRUE);
}

int32
ds_cfm_error_set_cfm_error_field(tbl_cfm_md_t *p_md, ds_cfm_error_t *p_cfm_error, ds_cfm_error_field_id_t field_id)
{
    return ds_cfm_error_set_cfm_error_field_sync(p_md, p_cfm_error, field_id, TRUE);
}

ds_cfm_error_t*
ds_cfm_error_get_cfm_error(tbl_cfm_md_t *p_md, ds_cfm_error_t *p_cfm_error)
{
    tbl_cfm_md_t *p_db_md = NULL;

    /* 1. lookup entry exist */
    p_db_md = tbl_cfm_md_get_cfm_md(&p_md->key);
    if (NULL == p_db_md)
    {
        return NULL;
    }

    return cdb_reference_list_lookup_object(&p_db_md->cfm_error, p_cfm_error);
}

int32
ds_cfm_error_key_str2val(char* tbl_key_value, char* str, tbl_cfm_md_t *p_md, ds_cfm_error_t *p_cfm_error)
{

    int32 ret = 0;

    ret = tbl_cfm_md_key_str2val(tbl_key_value, p_md);

    ret = cdb_cfm_time_key_str2val(str, &p_cfm_error->key);

    return ret;
}

char*
ds_cfm_error_key_value_dump(tbl_cfm_md_t *p_md, ds_cfm_error_t *p_cfm_error, char* str)
{


    cdb_cfm_time_key_val2str(str, MAX_CMD_STR_LEN, &p_cfm_error->key);
    return str;
}

char*
ds_cfm_error_key_name_dump(tbl_cfm_md_t *p_md, ds_cfm_error_t *p_cfm_error, char* str)
{

    cdb_node_t *p_node_tbl = cdb_get_tbl(TBL_CFM_MD);
    cdb_node_t *p_node_ds = cdb_get_ds(DS_CFM_ERROR);
    char tbl_str[MAX_CMD_STR_LEN];
    char ds_str[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s"KEY_CONNECT_STR"%s",
        p_node_tbl->name, tbl_cfm_md_key_value_dump(p_md, tbl_str), p_node_ds->name, ds_cfm_error_key_value_dump(p_md, p_cfm_error, ds_str));
    return str;
}

char*
ds_cfm_error_field_name_dump(tbl_cfm_md_t *p_md, ds_cfm_error_t *p_cfm_error, int32 field_id, char* str)
{

    cdb_node_t *p_node = cdb_get_ds(DS_CFM_ERROR);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= DS_CFM_ERROR_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
ds_cfm_error_field_value_dump(tbl_cfm_md_t *p_md, ds_cfm_error_t *p_cfm_error, int32 field_id, char* str)
{

    if (FLD_MATCH(DS_CFM_ERROR_FLD_KEY, field_id))
    {
        cdb_cfm_time_key_val2str(str, MAX_CMD_STR_LEN, &p_cfm_error->key);
    }
    if (FLD_MATCH(DS_CFM_ERROR_FLD_LEVEL, field_id))
    {
        sal_sprintf(str, "%u", p_cfm_error->level);
    }
    if (FLD_MATCH(DS_CFM_ERROR_FLD_VLAN_ID, field_id))
    {
        sal_sprintf(str, "%u", p_cfm_error->vlan_id);
    }
    if (FLD_MATCH(DS_CFM_ERROR_FLD_MD_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_cfm_error->md_name);
    }
    if (FLD_MATCH(DS_CFM_ERROR_FLD_MEP_ID, field_id))
    {
        sal_sprintf(str, "%u", p_cfm_error->mep_id);
    }
    if (FLD_MATCH(DS_CFM_ERROR_FLD_ERR_REASON, field_id))
    {
        sal_sprintf(str, "%u", p_cfm_error->err_reason);
    }
    if (FLD_MATCH(DS_CFM_ERROR_FLD_RMEP_MAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_cfm_error->rmep_mac);
    }
    return str;
}

char**
ds_cfm_error_table_dump(tbl_cfm_md_t *p_md, ds_cfm_error_t *p_cfm_error, char** str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         ds_cfm_error_key_name_dump(p_md, p_cfm_error, buf));
    for(i=1; i<DS_CFM_ERROR_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            ds_cfm_error_field_name_dump(p_md, p_cfm_error, i, buf));
        sal_sprintf(str[j++], "%s",
            ds_cfm_error_field_value_dump(p_md, p_cfm_error, i, buf));
    }
    return str;
}

int32
ds_cfm_error_field_value_parser(char *str, int32 field_id, tbl_cfm_md_t *p_md, ds_cfm_error_t *p_cfm_error)
{
    int32 ret=0;
    if (FLD_MATCH(DS_CFM_ERROR_FLD_KEY, field_id))
    {
        cdb_cfm_time_key_str2val(str, &p_cfm_error->key);
    }
    if (FLD_MATCH(DS_CFM_ERROR_FLD_LEVEL, field_id))
    {
        p_cfm_error->level = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_CFM_ERROR_FLD_VLAN_ID, field_id))
    {
        p_cfm_error->vlan_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_CFM_ERROR_FLD_MD_NAME, field_id))
    {
        sal_strcpy(p_cfm_error->md_name, str);
    }
    if (FLD_MATCH(DS_CFM_ERROR_FLD_MEP_ID, field_id))
    {
        p_cfm_error->mep_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_CFM_ERROR_FLD_ERR_REASON, field_id))
    {
        p_cfm_error->err_reason = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(DS_CFM_ERROR_FLD_RMEP_MAC, field_id))
    {
        cdb_mac_addr_str2val(p_cfm_error->rmep_mac, str);
    }
    return ret;
}

int32
ds_cfm_error_table_parser(char** array, char* tbl_key_value, char* ds_key_value,tbl_cfm_md_t *p_md, ds_cfm_error_t *p_cfm_error)
{
    int32 rc;
    int32 i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_ds(DS_CFM_ERROR);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(ds_cfm_error_key_str2val(tbl_key_value, ds_key_value, p_md, p_cfm_error));

    for(i=1; i<DS_CFM_ERROR_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(ds_cfm_error_field_value_parser( array[j++], i, p_md, p_cfm_error));
    }

    return PM_E_NONE;
}

int32
ds_cfm_error_dump_one(ds_cfm_error_t *p_cfm_error, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_ds(DS_CFM_ERROR);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(DS_CFM_ERROR_FLD_KEY, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_CFM_ERROR_FLD_KEY].name,
            cdb_cfm_time_key_val2str(buf, MAX_CMD_STR_LEN, &p_cfm_error->key));
    }
    if (FLD_MATCH(DS_CFM_ERROR_FLD_LEVEL, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_CFM_ERROR_FLD_LEVEL].name,
            p_cfm_error->level);
    }
    if (FLD_MATCH(DS_CFM_ERROR_FLD_VLAN_ID, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_CFM_ERROR_FLD_VLAN_ID].name,
            p_cfm_error->vlan_id);
    }
    if (FLD_MATCH(DS_CFM_ERROR_FLD_MD_NAME, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_CFM_ERROR_FLD_MD_NAME].name,
            p_cfm_error->md_name);
    }
    if (FLD_MATCH(DS_CFM_ERROR_FLD_MEP_ID, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_CFM_ERROR_FLD_MEP_ID].name,
            p_cfm_error->mep_id);
    }
    if (FLD_MATCH(DS_CFM_ERROR_FLD_ERR_REASON, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",
            p_tbl_info->field[DS_CFM_ERROR_FLD_ERR_REASON].name,
            p_cfm_error->err_reason);
    }
    if (FLD_MATCH(DS_CFM_ERROR_FLD_RMEP_MAC, p_field->field_id[1]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_tbl_info->field[DS_CFM_ERROR_FLD_RMEP_MAC].name,
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_cfm_error->rmep_mac));
    }

    return PM_E_NONE;
}
