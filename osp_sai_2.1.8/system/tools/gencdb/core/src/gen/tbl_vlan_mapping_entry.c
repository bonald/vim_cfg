
#include "proto.h"
#include "gen/tbl_vlan_mapping_entry_define.h"
#include "gen/tbl_vlan_mapping_entry.h"
#include "cdb_data_cmp.h"

tbl_vlan_mapping_entry_master_t *_g_p_tbl_vlan_mapping_entry_master = NULL;

static uint32
_tbl_vlan_mapping_entry_hash_make(tbl_vlan_mapping_entry_t *p_vlan_mapping_entry)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_vlan_mapping_entry->key;
    for (index = 0; index < sizeof(p_vlan_mapping_entry->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_vlan_mapping_entry_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_vlan_mapping_entry_t *p_vlan_mapping_entry1 = (tbl_vlan_mapping_entry_t*)p_arg1;
    tbl_vlan_mapping_entry_t *p_vlan_mapping_entry2 = (tbl_vlan_mapping_entry_t*)p_arg2;

    if (0 == sal_memcmp(&p_vlan_mapping_entry1->key, &p_vlan_mapping_entry2->key, sizeof(tbl_vlan_mapping_entry_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_vlan_mapping_entry_add_vlan_mapping_entry_sync(tbl_vlan_mapping_entry_t *p_vlan_mapping_entry, uint32 sync)
{
    tbl_vlan_mapping_entry_master_t *p_master = _g_p_tbl_vlan_mapping_entry_master;
    tbl_vlan_mapping_entry_t *p_db_vlan_mapping_entry = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_vlan_mapping_entry_get_vlan_mapping_entry(&p_vlan_mapping_entry->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_vlan_mapping_entry = XCALLOC(MEM_TBL_VLAN_MAPPING_ENTRY, sizeof(tbl_vlan_mapping_entry_t));
    if (NULL == p_db_vlan_mapping_entry)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_vlan_mapping_entry, p_vlan_mapping_entry, sizeof(tbl_vlan_mapping_entry_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->vlan_mapping_entry_hash, (void*)p_db_vlan_mapping_entry, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->vlan_mapping_entry_list, p_db_vlan_mapping_entry);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_VLAN_MAPPING_ENTRY, p_db_vlan_mapping_entry);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_VLAN_MAPPING_ENTRY, p_db_vlan_mapping_entry);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_vlan_mapping_entry_del_vlan_mapping_entry_sync(tbl_vlan_mapping_entry_key_t *p_vlan_mapping_entry_key, uint32 sync)
{
    tbl_vlan_mapping_entry_master_t *p_master = _g_p_tbl_vlan_mapping_entry_master;
    tbl_vlan_mapping_entry_t *p_db_vlan_mapping_entry = NULL;

    /* 1. lookup entry exist */
    p_db_vlan_mapping_entry = tbl_vlan_mapping_entry_get_vlan_mapping_entry(p_vlan_mapping_entry_key);
    if (NULL == p_db_vlan_mapping_entry)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_VLAN_MAPPING_ENTRY, p_db_vlan_mapping_entry);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_VLAN_MAPPING_ENTRY, p_db_vlan_mapping_entry);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->vlan_mapping_entry_hash, (void*)p_db_vlan_mapping_entry);
    ctclib_slistnode_delete(p_master->vlan_mapping_entry_list, p_db_vlan_mapping_entry);

    /* 4. free db entry */
    XFREE(MEM_TBL_VLAN_MAPPING_ENTRY, p_db_vlan_mapping_entry);

    return PM_E_NONE;
}

int32
tbl_vlan_mapping_entry_set_vlan_mapping_entry_field_sync(tbl_vlan_mapping_entry_t *p_vlan_mapping_entry, tbl_vlan_mapping_entry_field_id_t field_id, uint32 sync)
{
    tbl_vlan_mapping_entry_t *p_db_vlan_mapping_entry = NULL;

    /* 1. lookup entry exist */
    p_db_vlan_mapping_entry = tbl_vlan_mapping_entry_get_vlan_mapping_entry(&p_vlan_mapping_entry->key);
    if (NULL == p_db_vlan_mapping_entry)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_VLAN_MAPPING_ENTRY_FLD_MAPPED_SVID:
        p_db_vlan_mapping_entry->mapped_svid = p_vlan_mapping_entry->mapped_svid;
        break;

    case TBL_VLAN_MAPPING_ENTRY_FLD_MAPPED_CVID:
        p_db_vlan_mapping_entry->mapped_cvid = p_vlan_mapping_entry->mapped_cvid;
        break;

    case TBL_VLAN_MAPPING_ENTRY_FLD_EVC_ENTRY:
        sal_memcpy(p_db_vlan_mapping_entry->evc_entry, p_vlan_mapping_entry->evc_entry, sizeof(p_vlan_mapping_entry->evc_entry));
        break;

    case TBL_VLAN_MAPPING_ENTRY_FLD_MAX:
        sal_memcpy(p_db_vlan_mapping_entry, p_vlan_mapping_entry, sizeof(tbl_vlan_mapping_entry_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_VLAN_MAPPING_ENTRY, field_id, p_db_vlan_mapping_entry);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_VLAN_MAPPING_ENTRY, field_id, p_db_vlan_mapping_entry);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_vlan_mapping_entry_add_vlan_mapping_entry(tbl_vlan_mapping_entry_t *p_vlan_mapping_entry)
{
    return tbl_vlan_mapping_entry_add_vlan_mapping_entry_sync(p_vlan_mapping_entry, TRUE);
}

int32
tbl_vlan_mapping_entry_del_vlan_mapping_entry(tbl_vlan_mapping_entry_key_t *p_vlan_mapping_entry_key)
{
    return tbl_vlan_mapping_entry_del_vlan_mapping_entry_sync(p_vlan_mapping_entry_key, TRUE);
}

int32
tbl_vlan_mapping_entry_set_vlan_mapping_entry_field(tbl_vlan_mapping_entry_t *p_vlan_mapping_entry, tbl_vlan_mapping_entry_field_id_t field_id)
{
    return tbl_vlan_mapping_entry_set_vlan_mapping_entry_field_sync(p_vlan_mapping_entry, field_id, TRUE);
}

tbl_vlan_mapping_entry_t*
tbl_vlan_mapping_entry_get_vlan_mapping_entry(tbl_vlan_mapping_entry_key_t *p_vlan_mapping_entry_key)
{
    tbl_vlan_mapping_entry_master_t *p_master = _g_p_tbl_vlan_mapping_entry_master;
    tbl_vlan_mapping_entry_t lkp;

    sal_memcpy(&lkp.key, p_vlan_mapping_entry_key, sizeof(tbl_vlan_mapping_entry_key_t));
    return ctclib_hash_lookup(p_master->vlan_mapping_entry_hash, &lkp);
}

char*
tbl_vlan_mapping_entry_key_val2str(tbl_vlan_mapping_entry_t *p_vlan_mapping_entry, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_vlan_mapping_entry_key_val2str(buf, MAX_CMD_STR_LEN, &p_vlan_mapping_entry->key));
    return str;
}

int32
tbl_vlan_mapping_entry_key_str2val(char *str, tbl_vlan_mapping_entry_t *p_vlan_mapping_entry)
{
    int32 ret = 0;

    ret = cdb_vlan_mapping_entry_key_str2val(str, &p_vlan_mapping_entry->key);
    return ret;
}

char*
tbl_vlan_mapping_entry_key_name_dump(tbl_vlan_mapping_entry_t *p_vlan_mapping_entry, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_VLAN_MAPPING_ENTRY);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_vlan_mapping_entry_key_val2str(buf, MAX_CMD_STR_LEN, &p_vlan_mapping_entry->key));
    return str;
}

char*
tbl_vlan_mapping_entry_key_value_dump(tbl_vlan_mapping_entry_t *p_vlan_mapping_entry, char *str)
{

    cdb_vlan_mapping_entry_key_val2str(str, MAX_CMD_STR_LEN, &p_vlan_mapping_entry->key);
    return str;
}

char*
tbl_vlan_mapping_entry_field_name_dump(tbl_vlan_mapping_entry_t *p_vlan_mapping_entry, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_VLAN_MAPPING_ENTRY);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_VLAN_MAPPING_ENTRY_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_vlan_mapping_entry_field_value_dump(tbl_vlan_mapping_entry_t *p_vlan_mapping_entry, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_VLAN_MAPPING_ENTRY_FLD_KEY, field_id))
    {
        cdb_vlan_mapping_entry_key_val2str(str, MAX_CMD_STR_LEN, &p_vlan_mapping_entry->key);
    }
    if (FLD_MATCH(TBL_VLAN_MAPPING_ENTRY_FLD_MAPPED_SVID, field_id))
    {
        sal_sprintf(str, "%u", p_vlan_mapping_entry->mapped_svid);
    }
    if (FLD_MATCH(TBL_VLAN_MAPPING_ENTRY_FLD_MAPPED_CVID, field_id))
    {
        sal_sprintf(str, "%u", p_vlan_mapping_entry->mapped_cvid);
    }
    if (FLD_MATCH(TBL_VLAN_MAPPING_ENTRY_FLD_EVC_ENTRY, field_id))
    {
        sal_sprintf(str, "%s", p_vlan_mapping_entry->evc_entry);
    }
    return str;
}

char**
tbl_vlan_mapping_entry_table_dump(tbl_vlan_mapping_entry_t *p_vlan_mapping_entry, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_vlan_mapping_entry_key_name_dump(p_vlan_mapping_entry, buf));
    for(i=1; i<TBL_VLAN_MAPPING_ENTRY_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_vlan_mapping_entry_field_name_dump(p_vlan_mapping_entry, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_vlan_mapping_entry_field_value_dump(p_vlan_mapping_entry, i, buf));
    }
    return str;
}

int32
tbl_vlan_mapping_entry_field_value_parser(char *str, int32 field_id, tbl_vlan_mapping_entry_t *p_vlan_mapping_entry)
{
    if (FLD_MATCH(TBL_VLAN_MAPPING_ENTRY_FLD_KEY, field_id))
    {
        cdb_vlan_mapping_entry_key_str2val(str, &p_vlan_mapping_entry->key);
    }
    if (FLD_MATCH(TBL_VLAN_MAPPING_ENTRY_FLD_MAPPED_SVID, field_id))
    {
        int32 ret;
        p_vlan_mapping_entry->mapped_svid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VLAN_MAPPING_ENTRY_FLD_MAPPED_CVID, field_id))
    {
        int32 ret;
        p_vlan_mapping_entry->mapped_cvid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_VLAN_MAPPING_ENTRY_FLD_EVC_ENTRY, field_id))
    {
        sal_strcpy(p_vlan_mapping_entry->evc_entry, str);
    }
    return PM_E_NONE;
}

int32
tbl_vlan_mapping_entry_table_parser(char** array, char* key_value,tbl_vlan_mapping_entry_t *p_vlan_mapping_entry)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_VLAN_MAPPING_ENTRY);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_vlan_mapping_entry_key_str2val(key_value, p_vlan_mapping_entry));

    for(i=1; i<TBL_VLAN_MAPPING_ENTRY_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_vlan_mapping_entry_field_value_parser( array[j++], i, p_vlan_mapping_entry));
    }

    return PM_E_NONE;
}

int32
tbl_vlan_mapping_entry_dump_one(tbl_vlan_mapping_entry_t *p_vlan_mapping_entry, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_VLAN_MAPPING_ENTRY);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_VLAN_MAPPING_ENTRY_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VLAN_MAPPING_ENTRY_FLD_KEY].name, 
            cdb_vlan_mapping_entry_key_val2str(buf, MAX_CMD_STR_LEN, &p_vlan_mapping_entry->key));
    }
    if (FLD_MATCH(TBL_VLAN_MAPPING_ENTRY_FLD_MAPPED_SVID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VLAN_MAPPING_ENTRY_FLD_MAPPED_SVID].name,
            p_vlan_mapping_entry->mapped_svid);
    }
    if (FLD_MATCH(TBL_VLAN_MAPPING_ENTRY_FLD_MAPPED_CVID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_VLAN_MAPPING_ENTRY_FLD_MAPPED_CVID].name,
            p_vlan_mapping_entry->mapped_cvid);
    }
    if (FLD_MATCH(TBL_VLAN_MAPPING_ENTRY_FLD_EVC_ENTRY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_VLAN_MAPPING_ENTRY_FLD_EVC_ENTRY].name,
            p_vlan_mapping_entry->evc_entry);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_vlan_mapping_entry_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_vlan_mapping_entry_master_t *p_master = _g_p_tbl_vlan_mapping_entry_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_vlan_mapping_entry_t *p_db_vlan_mapping_entry = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->vlan_mapping_entry_list, p_db_vlan_mapping_entry, listnode, next)
    {
        rc |= fn(p_db_vlan_mapping_entry, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_vlan_mapping_entry_master_t*
tbl_vlan_mapping_entry_get_master()
{
    return _g_p_tbl_vlan_mapping_entry_master;
}

tbl_vlan_mapping_entry_master_t*
tbl_vlan_mapping_entry_init_vlan_mapping_entry()
{
    _g_p_tbl_vlan_mapping_entry_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_vlan_mapping_entry_master_t));
    _g_p_tbl_vlan_mapping_entry_master->vlan_mapping_entry_hash = ctclib_hash_create(_tbl_vlan_mapping_entry_hash_make, _tbl_vlan_mapping_entry_hash_cmp);
    _g_p_tbl_vlan_mapping_entry_master->vlan_mapping_entry_list = ctclib_slist_create(tbl_vlan_mapping_entry_cmp, NULL);
    return _g_p_tbl_vlan_mapping_entry_master;
}

