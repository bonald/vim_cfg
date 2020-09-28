
#include "proto.h"
#include "gen/tbl_acl_udf_entry_define.h"
#include "gen/tbl_acl_udf_entry.h"
#include "cdb_data_cmp.h"

tbl_acl_udf_entry_master_t *_g_p_tbl_acl_udf_entry_master = NULL;

int32
tbl_acl_udf_entry_add_acl_udf_entry_sync(tbl_acl_udf_entry_t *p_acl_udf_entry, uint32 sync)
{
    tbl_acl_udf_entry_master_t *p_master = _g_p_tbl_acl_udf_entry_master;
    tbl_acl_udf_entry_t *p_db_acl_udf_entry = NULL;

    /* 1. lookup entry exist */
    if (tbl_acl_udf_entry_get_acl_udf_entry(&p_acl_udf_entry->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_acl_udf_entry = XCALLOC(MEM_TBL_ACL_UDF_ENTRY, sizeof(tbl_acl_udf_entry_t));
    if (NULL == p_db_acl_udf_entry)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_acl_udf_entry, p_acl_udf_entry, sizeof(tbl_acl_udf_entry_t));

    /* 4. add to db */
    p_master->acl_udf_entry_array[p_acl_udf_entry->key.id] = p_db_acl_udf_entry;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_ACL_UDF_ENTRY, p_db_acl_udf_entry);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_ACL_UDF_ENTRY, p_db_acl_udf_entry);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_acl_udf_entry_del_acl_udf_entry_sync(tbl_acl_udf_entry_key_t *p_acl_udf_entry_key, uint32 sync)
{
    tbl_acl_udf_entry_master_t *p_master = _g_p_tbl_acl_udf_entry_master;
    tbl_acl_udf_entry_t *p_db_acl_udf_entry = NULL;

    /* 1. lookup entry exist */
    p_db_acl_udf_entry = tbl_acl_udf_entry_get_acl_udf_entry(p_acl_udf_entry_key);
    if (NULL == p_db_acl_udf_entry)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_ACL_UDF_ENTRY, p_db_acl_udf_entry);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_ACL_UDF_ENTRY, p_db_acl_udf_entry);
        #endif 
    }

    /* 3. delete from db */
    p_master->acl_udf_entry_array[p_acl_udf_entry_key->id] = NULL;

    /* 4. free db entry */
    XFREE(MEM_TBL_ACL_UDF_ENTRY, p_db_acl_udf_entry);

    return PM_E_NONE;
}

int32
tbl_acl_udf_entry_set_acl_udf_entry_field_sync(tbl_acl_udf_entry_t *p_acl_udf_entry, tbl_acl_udf_entry_field_id_t field_id, uint32 sync)
{
    tbl_acl_udf_entry_t *p_db_acl_udf_entry = NULL;

    /* 1. lookup entry exist */
    p_db_acl_udf_entry = tbl_acl_udf_entry_get_acl_udf_entry(&p_acl_udf_entry->key);
    if (NULL == p_db_acl_udf_entry)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_ACL_UDF_ENTRY_FLD_ACL_REF_CNT:
        p_db_acl_udf_entry->acl_ref_cnt = p_acl_udf_entry->acl_ref_cnt;
        break;

    case TBL_ACL_UDF_ENTRY_FLD_PRIORITY:
        p_db_acl_udf_entry->priority = p_acl_udf_entry->priority;
        break;

    case TBL_ACL_UDF_ENTRY_FLD_OFFSET_TYPE:
        p_db_acl_udf_entry->offset_type = p_acl_udf_entry->offset_type;
        break;

    case TBL_ACL_UDF_ENTRY_FLD_OFFSET_NUM:
        p_db_acl_udf_entry->offset_num = p_acl_udf_entry->offset_num;
        break;

    case TBL_ACL_UDF_ENTRY_FLD_VALID:
        sal_memcpy(p_db_acl_udf_entry->valid, p_acl_udf_entry->valid, sizeof(p_acl_udf_entry->valid));
        break;

    case TBL_ACL_UDF_ENTRY_FLD_OFFSET:
        sal_memcpy(p_db_acl_udf_entry->offset, p_acl_udf_entry->offset, sizeof(p_acl_udf_entry->offset));
        break;

    case TBL_ACL_UDF_ENTRY_FLD_KEY_FLAGS:
        p_db_acl_udf_entry->key_flags = p_acl_udf_entry->key_flags;
        break;

    case TBL_ACL_UDF_ENTRY_FLD_ETHTYPE:
        p_db_acl_udf_entry->ethtype = p_acl_udf_entry->ethtype;
        break;

    case TBL_ACL_UDF_ENTRY_FLD_ETHTYPE_MASK:
        p_db_acl_udf_entry->ethtype_mask = p_acl_udf_entry->ethtype_mask;
        break;

    case TBL_ACL_UDF_ENTRY_FLD_VLANNUM:
        p_db_acl_udf_entry->vlannum = p_acl_udf_entry->vlannum;
        break;

    case TBL_ACL_UDF_ENTRY_FLD_VLANNUM_MASK:
        p_db_acl_udf_entry->vlannum_mask = p_acl_udf_entry->vlannum_mask;
        break;

    case TBL_ACL_UDF_ENTRY_FLD_IPPROTO:
        p_db_acl_udf_entry->ipproto = p_acl_udf_entry->ipproto;
        break;

    case TBL_ACL_UDF_ENTRY_FLD_IPPROTO_MASK:
        p_db_acl_udf_entry->ipproto_mask = p_acl_udf_entry->ipproto_mask;
        break;

    case TBL_ACL_UDF_ENTRY_FLD_L4_TYPE:
        p_db_acl_udf_entry->l4_type = p_acl_udf_entry->l4_type;
        break;

    case TBL_ACL_UDF_ENTRY_FLD_L4_TYPE_MASK:
        p_db_acl_udf_entry->l4_type_mask = p_acl_udf_entry->l4_type_mask;
        break;

    case TBL_ACL_UDF_ENTRY_FLD_SRCPORT:
        p_db_acl_udf_entry->srcport = p_acl_udf_entry->srcport;
        break;

    case TBL_ACL_UDF_ENTRY_FLD_SRCPORT_MASK:
        p_db_acl_udf_entry->srcport_mask = p_acl_udf_entry->srcport_mask;
        break;

    case TBL_ACL_UDF_ENTRY_FLD_DSTPORT:
        p_db_acl_udf_entry->dstport = p_acl_udf_entry->dstport;
        break;

    case TBL_ACL_UDF_ENTRY_FLD_DSTPORT_MASK:
        p_db_acl_udf_entry->dstport_mask = p_acl_udf_entry->dstport_mask;
        break;

    case TBL_ACL_UDF_ENTRY_FLD_MPLSNUM:
        p_db_acl_udf_entry->mplsnum = p_acl_udf_entry->mplsnum;
        break;

    case TBL_ACL_UDF_ENTRY_FLD_MPLSNUM_MASK:
        p_db_acl_udf_entry->mplsnum_mask = p_acl_udf_entry->mplsnum_mask;
        break;

    case TBL_ACL_UDF_ENTRY_FLD_MAX:
        sal_memcpy(p_db_acl_udf_entry, p_acl_udf_entry, sizeof(tbl_acl_udf_entry_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_ACL_UDF_ENTRY, field_id, p_db_acl_udf_entry);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_ACL_UDF_ENTRY, field_id, p_db_acl_udf_entry);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_acl_udf_entry_add_acl_udf_entry(tbl_acl_udf_entry_t *p_acl_udf_entry)
{
    return tbl_acl_udf_entry_add_acl_udf_entry_sync(p_acl_udf_entry, TRUE);
}

int32
tbl_acl_udf_entry_del_acl_udf_entry(tbl_acl_udf_entry_key_t *p_acl_udf_entry_key)
{
    return tbl_acl_udf_entry_del_acl_udf_entry_sync(p_acl_udf_entry_key, TRUE);
}

int32
tbl_acl_udf_entry_set_acl_udf_entry_field(tbl_acl_udf_entry_t *p_acl_udf_entry, tbl_acl_udf_entry_field_id_t field_id)
{
    return tbl_acl_udf_entry_set_acl_udf_entry_field_sync(p_acl_udf_entry, field_id, TRUE);
}

tbl_acl_udf_entry_t*
tbl_acl_udf_entry_get_acl_udf_entry(tbl_acl_udf_entry_key_t *p_acl_udf_entry_key)
{
    tbl_acl_udf_entry_master_t *p_master = _g_p_tbl_acl_udf_entry_master;

    if (GLB_ACL_UDF_ENTRY_NUM_MAX+1 <= p_acl_udf_entry_key->id)
    {
        return NULL;
    }
    return (p_master->acl_udf_entry_array[p_acl_udf_entry_key->id]);
}

char*
tbl_acl_udf_entry_key_val2str(tbl_acl_udf_entry_t *p_acl_udf_entry, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_acl_udf_entry->key.id);
    return str;
}

int32
tbl_acl_udf_entry_key_str2val(char *str, tbl_acl_udf_entry_t *p_acl_udf_entry)
{
    int32 ret = 0;

    p_acl_udf_entry->key.id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_acl_udf_entry_key_name_dump(tbl_acl_udf_entry_t *p_acl_udf_entry, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_UDF_ENTRY);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_acl_udf_entry->key.id);
    return str;
}

char*
tbl_acl_udf_entry_key_value_dump(tbl_acl_udf_entry_t *p_acl_udf_entry, char *str)
{
    sal_sprintf(str, "%u", p_acl_udf_entry->key.id);
    return str;
}

char*
tbl_acl_udf_entry_field_name_dump(tbl_acl_udf_entry_t *p_acl_udf_entry, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_UDF_ENTRY);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_ACL_UDF_ENTRY_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_acl_udf_entry_field_value_dump(tbl_acl_udf_entry_t *p_acl_udf_entry, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_acl_udf_entry->key.id);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_ACL_REF_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_acl_udf_entry->acl_ref_cnt);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_PRIORITY, field_id))
    {
        sal_sprintf(str, "%u", p_acl_udf_entry->priority);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_OFFSET_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_acl_udf_entry->offset_type);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_OFFSET_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_acl_udf_entry->offset_num);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_VALID, field_id))
    {
        cdb_uint8_array_val2str(str, MAX_CMD_STR_LEN, p_acl_udf_entry->valid, sizeof(p_acl_udf_entry->valid));
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_OFFSET, field_id))
    {
        cdb_uint8_array_val2str(str, MAX_CMD_STR_LEN, p_acl_udf_entry->offset, sizeof(p_acl_udf_entry->offset));
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_KEY_FLAGS, field_id))
    {
        sal_sprintf(str, "%u", p_acl_udf_entry->key_flags);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_ETHTYPE, field_id))
    {
        sal_sprintf(str, "%u", p_acl_udf_entry->ethtype);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_ETHTYPE_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_acl_udf_entry->ethtype_mask);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_VLANNUM, field_id))
    {
        sal_sprintf(str, "%u", p_acl_udf_entry->vlannum);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_VLANNUM_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_acl_udf_entry->vlannum_mask);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_IPPROTO, field_id))
    {
        sal_sprintf(str, "%u", p_acl_udf_entry->ipproto);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_IPPROTO_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_acl_udf_entry->ipproto_mask);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_L4_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_acl_udf_entry->l4_type);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_L4_TYPE_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_acl_udf_entry->l4_type_mask);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_SRCPORT, field_id))
    {
        sal_sprintf(str, "%u", p_acl_udf_entry->srcport);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_SRCPORT_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_acl_udf_entry->srcport_mask);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_DSTPORT, field_id))
    {
        sal_sprintf(str, "%u", p_acl_udf_entry->dstport);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_DSTPORT_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_acl_udf_entry->dstport_mask);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_MPLSNUM, field_id))
    {
        sal_sprintf(str, "%u", p_acl_udf_entry->mplsnum);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_MPLSNUM_MASK, field_id))
    {
        sal_sprintf(str, "%u", p_acl_udf_entry->mplsnum_mask);
    }
    return str;
}

char**
tbl_acl_udf_entry_table_dump(tbl_acl_udf_entry_t *p_acl_udf_entry, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_acl_udf_entry_key_name_dump(p_acl_udf_entry, buf));
    for(i=1; i<TBL_ACL_UDF_ENTRY_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_acl_udf_entry_field_name_dump(p_acl_udf_entry, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_acl_udf_entry_field_value_dump(p_acl_udf_entry, i, buf));
    }
    return str;
}

int32
tbl_acl_udf_entry_field_value_parser(char *str, int32 field_id, tbl_acl_udf_entry_t *p_acl_udf_entry)
{
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_KEY, field_id))
    {
        int32 ret;
        p_acl_udf_entry->key.id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_ACL_REF_CNT, field_id))
    {
        int32 ret;
        p_acl_udf_entry->acl_ref_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_PRIORITY, field_id))
    {
        int32 ret;
        p_acl_udf_entry->priority = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_OFFSET_TYPE, field_id))
    {
        int32 ret;
        p_acl_udf_entry->offset_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_OFFSET_NUM, field_id))
    {
        int32 ret;
        p_acl_udf_entry->offset_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_VALID, field_id))
    {
        cdb_uint8_array_str2val(str, p_acl_udf_entry->valid);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_OFFSET, field_id))
    {
        cdb_uint8_array_str2val(str, p_acl_udf_entry->offset);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_KEY_FLAGS, field_id))
    {
        int32 ret;
        p_acl_udf_entry->key_flags = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_ETHTYPE, field_id))
    {
        int32 ret;
        p_acl_udf_entry->ethtype = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_ETHTYPE_MASK, field_id))
    {
        int32 ret;
        p_acl_udf_entry->ethtype_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_VLANNUM, field_id))
    {
        int32 ret;
        p_acl_udf_entry->vlannum = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_VLANNUM_MASK, field_id))
    {
        int32 ret;
        p_acl_udf_entry->vlannum_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_IPPROTO, field_id))
    {
        int32 ret;
        p_acl_udf_entry->ipproto = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_IPPROTO_MASK, field_id))
    {
        int32 ret;
        p_acl_udf_entry->ipproto_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_L4_TYPE, field_id))
    {
        int32 ret;
        p_acl_udf_entry->l4_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_L4_TYPE_MASK, field_id))
    {
        int32 ret;
        p_acl_udf_entry->l4_type_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_SRCPORT, field_id))
    {
        int32 ret;
        p_acl_udf_entry->srcport = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_SRCPORT_MASK, field_id))
    {
        int32 ret;
        p_acl_udf_entry->srcport_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_DSTPORT, field_id))
    {
        int32 ret;
        p_acl_udf_entry->dstport = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_DSTPORT_MASK, field_id))
    {
        int32 ret;
        p_acl_udf_entry->dstport_mask = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_MPLSNUM, field_id))
    {
        int32 ret;
        p_acl_udf_entry->mplsnum = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_MPLSNUM_MASK, field_id))
    {
        int32 ret;
        p_acl_udf_entry->mplsnum_mask = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_acl_udf_entry_table_parser(char** array, char* key_value,tbl_acl_udf_entry_t *p_acl_udf_entry)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_UDF_ENTRY);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_acl_udf_entry_key_str2val(key_value, p_acl_udf_entry));

    for(i=1; i<TBL_ACL_UDF_ENTRY_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_acl_udf_entry_field_value_parser( array[j++], i, p_acl_udf_entry));
    }

    return PM_E_NONE;
}

int32
tbl_acl_udf_entry_dump_one(tbl_acl_udf_entry_t *p_acl_udf_entry, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_UDF_ENTRY);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_UDF_ENTRY_FLD_KEY].name,
            p_acl_udf_entry->key.id);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_ACL_REF_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_UDF_ENTRY_FLD_ACL_REF_CNT].name,
            p_acl_udf_entry->acl_ref_cnt);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_PRIORITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_UDF_ENTRY_FLD_PRIORITY].name,
            p_acl_udf_entry->priority);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_OFFSET_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_UDF_ENTRY_FLD_OFFSET_TYPE].name,
            p_acl_udf_entry->offset_type);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_OFFSET_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_UDF_ENTRY_FLD_OFFSET_NUM].name,
            p_acl_udf_entry->offset_num);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_VALID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_UDF_ENTRY_FLD_VALID].name, 
            cdb_uint8_array_val2str(buf, MAX_CMD_STR_LEN, p_acl_udf_entry->valid, sizeof(p_acl_udf_entry->valid)));
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_OFFSET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_UDF_ENTRY_FLD_OFFSET].name, 
            cdb_uint8_array_val2str(buf, MAX_CMD_STR_LEN, p_acl_udf_entry->offset, sizeof(p_acl_udf_entry->offset)));
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_KEY_FLAGS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_UDF_ENTRY_FLD_KEY_FLAGS].name,
            p_acl_udf_entry->key_flags);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_ETHTYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_UDF_ENTRY_FLD_ETHTYPE].name,
            p_acl_udf_entry->ethtype);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_ETHTYPE_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_UDF_ENTRY_FLD_ETHTYPE_MASK].name,
            p_acl_udf_entry->ethtype_mask);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_VLANNUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_UDF_ENTRY_FLD_VLANNUM].name,
            p_acl_udf_entry->vlannum);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_VLANNUM_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_UDF_ENTRY_FLD_VLANNUM_MASK].name,
            p_acl_udf_entry->vlannum_mask);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_IPPROTO, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_UDF_ENTRY_FLD_IPPROTO].name,
            p_acl_udf_entry->ipproto);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_IPPROTO_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_UDF_ENTRY_FLD_IPPROTO_MASK].name,
            p_acl_udf_entry->ipproto_mask);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_L4_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_UDF_ENTRY_FLD_L4_TYPE].name,
            p_acl_udf_entry->l4_type);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_L4_TYPE_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_UDF_ENTRY_FLD_L4_TYPE_MASK].name,
            p_acl_udf_entry->l4_type_mask);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_SRCPORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_UDF_ENTRY_FLD_SRCPORT].name,
            p_acl_udf_entry->srcport);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_SRCPORT_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_UDF_ENTRY_FLD_SRCPORT_MASK].name,
            p_acl_udf_entry->srcport_mask);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_DSTPORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_UDF_ENTRY_FLD_DSTPORT].name,
            p_acl_udf_entry->dstport);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_DSTPORT_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_UDF_ENTRY_FLD_DSTPORT_MASK].name,
            p_acl_udf_entry->dstport_mask);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_MPLSNUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_UDF_ENTRY_FLD_MPLSNUM].name,
            p_acl_udf_entry->mplsnum);
    }
    if (FLD_MATCH(TBL_ACL_UDF_ENTRY_FLD_MPLSNUM_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_UDF_ENTRY_FLD_MPLSNUM_MASK].name,
            p_acl_udf_entry->mplsnum_mask);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_acl_udf_entry_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_acl_udf_entry_master_t *p_master = _g_p_tbl_acl_udf_entry_master;
    tbl_acl_udf_entry_t *p_db_acl_udf_entry = NULL;
    uint32 id = 0;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    for (id = 0; id < GLB_ACL_UDF_ENTRY_NUM_MAX+1; id++)
    {
        p_db_acl_udf_entry = p_master->acl_udf_entry_array[id];
        if (NULL == p_db_acl_udf_entry)
        {
            continue;
        }
        rc |= fn(p_db_acl_udf_entry, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_acl_udf_entry_master_t*
tbl_acl_udf_entry_get_master()
{
    return _g_p_tbl_acl_udf_entry_master;
}

tbl_acl_udf_entry_master_t*
tbl_acl_udf_entry_init_acl_udf_entry()
{
    _g_p_tbl_acl_udf_entry_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_acl_udf_entry_master_t));
    return _g_p_tbl_acl_udf_entry_master;
}

