
#include "proto.h"
#include "gen/tbl_evc_entry_define.h"
#include "gen/tbl_evc_entry.h"
#include "cdb_data_cmp.h"

tbl_evc_entry_master_t *_g_p_tbl_evc_entry_master = NULL;

static uint32
_tbl_evc_entry_hash_make(tbl_evc_entry_t *p_evc_entry)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_evc_entry->key;
    for (index = 0; index < sizeof(p_evc_entry->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_evc_entry_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_evc_entry_t *p_evc_entry1 = (tbl_evc_entry_t*)p_arg1;
    tbl_evc_entry_t *p_evc_entry2 = (tbl_evc_entry_t*)p_arg2;

    if (0 == sal_memcmp(&p_evc_entry1->key, &p_evc_entry2->key, sizeof(tbl_evc_entry_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_evc_entry_add_evc_entry_sync(tbl_evc_entry_t *p_evc_entry, uint32 sync)
{
    tbl_evc_entry_master_t *p_master = _g_p_tbl_evc_entry_master;
    tbl_evc_entry_t *p_db_evc_entry = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_evc_entry_get_evc_entry(&p_evc_entry->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_evc_entry = XCALLOC(MEM_TBL_EVC_ENTRY, sizeof(tbl_evc_entry_t));
    if (NULL == p_db_evc_entry)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_evc_entry, p_evc_entry, sizeof(tbl_evc_entry_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->evc_entry_hash, (void*)p_db_evc_entry, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->evc_entry_list, p_db_evc_entry);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_EVC_ENTRY, p_db_evc_entry);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_EVC_ENTRY, p_db_evc_entry);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_evc_entry_del_evc_entry_sync(tbl_evc_entry_key_t *p_evc_entry_key, uint32 sync)
{
    tbl_evc_entry_master_t *p_master = _g_p_tbl_evc_entry_master;
    tbl_evc_entry_t *p_db_evc_entry = NULL;

    /* 1. lookup entry exist */
    p_db_evc_entry = tbl_evc_entry_get_evc_entry(p_evc_entry_key);
    if (NULL == p_db_evc_entry)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_EVC_ENTRY, p_db_evc_entry);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_EVC_ENTRY, p_db_evc_entry);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->evc_entry_hash, (void*)p_db_evc_entry);
    ctclib_slistnode_delete(p_master->evc_entry_list, p_db_evc_entry);

    /* 4. free db entry */
    XFREE(MEM_TBL_EVC_ENTRY, p_db_evc_entry);

    return PM_E_NONE;
}

int32
tbl_evc_entry_set_evc_entry_field_sync(tbl_evc_entry_t *p_evc_entry, tbl_evc_entry_field_id_t field_id, uint32 sync)
{
    tbl_evc_entry_t *p_db_evc_entry = NULL;

    /* 1. lookup entry exist */
    p_db_evc_entry = tbl_evc_entry_get_evc_entry(&p_evc_entry->key);
    if (NULL == p_db_evc_entry)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_EVC_ENTRY_FLD_MAPPED_SVID:
        p_db_evc_entry->mapped_svid = p_evc_entry->mapped_svid;
        break;

    case TBL_EVC_ENTRY_FLD_MAPPED_CVID:
        p_db_evc_entry->mapped_cvid = p_evc_entry->mapped_cvid;
        break;

    case TBL_EVC_ENTRY_FLD_EVC_TYPE:
        p_db_evc_entry->evc_type = p_evc_entry->evc_type;
        break;

    case TBL_EVC_ENTRY_FLD_REF_COUNT:
        p_db_evc_entry->ref_count = p_evc_entry->ref_count;
        break;

    case TBL_EVC_ENTRY_FLD_MAX:
        sal_memcpy(p_db_evc_entry, p_evc_entry, sizeof(tbl_evc_entry_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_EVC_ENTRY, field_id, p_db_evc_entry);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_EVC_ENTRY, field_id, p_db_evc_entry);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_evc_entry_add_evc_entry(tbl_evc_entry_t *p_evc_entry)
{
    return tbl_evc_entry_add_evc_entry_sync(p_evc_entry, TRUE);
}

int32
tbl_evc_entry_del_evc_entry(tbl_evc_entry_key_t *p_evc_entry_key)
{
    return tbl_evc_entry_del_evc_entry_sync(p_evc_entry_key, TRUE);
}

int32
tbl_evc_entry_set_evc_entry_field(tbl_evc_entry_t *p_evc_entry, tbl_evc_entry_field_id_t field_id)
{
    return tbl_evc_entry_set_evc_entry_field_sync(p_evc_entry, field_id, TRUE);
}

tbl_evc_entry_t*
tbl_evc_entry_get_evc_entry(tbl_evc_entry_key_t *p_evc_entry_key)
{
    tbl_evc_entry_master_t *p_master = _g_p_tbl_evc_entry_master;
    tbl_evc_entry_t lkp;

    sal_memcpy(&lkp.key, p_evc_entry_key, sizeof(tbl_evc_entry_key_t));
    return ctclib_hash_lookup(p_master->evc_entry_hash, &lkp);
}

char*
tbl_evc_entry_key_val2str(tbl_evc_entry_t *p_evc_entry, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_evc_entry->key.name);
    return str;
}

int32
tbl_evc_entry_key_str2val(char *str, tbl_evc_entry_t *p_evc_entry)
{
    int32 ret = 0;

    sal_strcpy(p_evc_entry->key.name, str);
    return ret;
}

char*
tbl_evc_entry_key_name_dump(tbl_evc_entry_t *p_evc_entry, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_EVC_ENTRY);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_evc_entry->key.name);
    return str;
}

char*
tbl_evc_entry_key_value_dump(tbl_evc_entry_t *p_evc_entry, char *str)
{
    sal_sprintf(str, "%s", p_evc_entry->key.name);
    return str;
}

char*
tbl_evc_entry_field_name_dump(tbl_evc_entry_t *p_evc_entry, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_EVC_ENTRY);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_EVC_ENTRY_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_evc_entry_field_value_dump(tbl_evc_entry_t *p_evc_entry, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_EVC_ENTRY_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_evc_entry->key.name);
    }
    if (FLD_MATCH(TBL_EVC_ENTRY_FLD_MAPPED_SVID, field_id))
    {
        sal_sprintf(str, "%u", p_evc_entry->mapped_svid);
    }
    if (FLD_MATCH(TBL_EVC_ENTRY_FLD_MAPPED_CVID, field_id))
    {
        sal_sprintf(str, "%u", p_evc_entry->mapped_cvid);
    }
    if (FLD_MATCH(TBL_EVC_ENTRY_FLD_EVC_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_evc_entry->evc_type);
    }
    if (FLD_MATCH(TBL_EVC_ENTRY_FLD_REF_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_evc_entry->ref_count);
    }
    return str;
}

char**
tbl_evc_entry_table_dump(tbl_evc_entry_t *p_evc_entry, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_evc_entry_key_name_dump(p_evc_entry, buf));
    for(i=1; i<TBL_EVC_ENTRY_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_evc_entry_field_name_dump(p_evc_entry, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_evc_entry_field_value_dump(p_evc_entry, i, buf));
    }
    return str;
}

int32
tbl_evc_entry_field_value_parser(char *str, int32 field_id, tbl_evc_entry_t *p_evc_entry)
{
    if (FLD_MATCH(TBL_EVC_ENTRY_FLD_KEY, field_id))
    {
        sal_strcpy(p_evc_entry->key.name, str);
    }
    if (FLD_MATCH(TBL_EVC_ENTRY_FLD_MAPPED_SVID, field_id))
    {
        int32 ret;
        p_evc_entry->mapped_svid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_EVC_ENTRY_FLD_MAPPED_CVID, field_id))
    {
        int32 ret;
        p_evc_entry->mapped_cvid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_EVC_ENTRY_FLD_EVC_TYPE, field_id))
    {
        int32 ret;
        p_evc_entry->evc_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_EVC_ENTRY_FLD_REF_COUNT, field_id))
    {
        int32 ret;
        p_evc_entry->ref_count = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_evc_entry_table_parser(char** array, char* key_value,tbl_evc_entry_t *p_evc_entry)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_EVC_ENTRY);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_evc_entry_key_str2val(key_value, p_evc_entry));

    for(i=1; i<TBL_EVC_ENTRY_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_evc_entry_field_value_parser( array[j++], i, p_evc_entry));
    }

    return PM_E_NONE;
}

int32
tbl_evc_entry_dump_one(tbl_evc_entry_t *p_evc_entry, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_EVC_ENTRY);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_EVC_ENTRY_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_EVC_ENTRY_FLD_KEY].name,
            p_evc_entry->key.name);
    }
    if (FLD_MATCH(TBL_EVC_ENTRY_FLD_MAPPED_SVID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_EVC_ENTRY_FLD_MAPPED_SVID].name,
            p_evc_entry->mapped_svid);
    }
    if (FLD_MATCH(TBL_EVC_ENTRY_FLD_MAPPED_CVID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_EVC_ENTRY_FLD_MAPPED_CVID].name,
            p_evc_entry->mapped_cvid);
    }
    if (FLD_MATCH(TBL_EVC_ENTRY_FLD_EVC_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_EVC_ENTRY_FLD_EVC_TYPE].name,
            p_evc_entry->evc_type);
    }
    if (FLD_MATCH(TBL_EVC_ENTRY_FLD_REF_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_EVC_ENTRY_FLD_REF_COUNT].name,
            p_evc_entry->ref_count);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_evc_entry_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_evc_entry_master_t *p_master = _g_p_tbl_evc_entry_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_evc_entry_t *p_db_evc_entry = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->evc_entry_list, p_db_evc_entry, listnode, next)
    {
        rc |= fn(p_db_evc_entry, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_evc_entry_master_t*
tbl_evc_entry_get_master()
{
    return _g_p_tbl_evc_entry_master;
}

tbl_evc_entry_master_t*
tbl_evc_entry_init_evc_entry()
{
    _g_p_tbl_evc_entry_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_evc_entry_master_t));
    _g_p_tbl_evc_entry_master->evc_entry_hash = ctclib_hash_create(_tbl_evc_entry_hash_make, _tbl_evc_entry_hash_cmp);
    _g_p_tbl_evc_entry_master->evc_entry_list = ctclib_slist_create(tbl_evc_entry_cmp, NULL);
    return _g_p_tbl_evc_entry_master;
}
