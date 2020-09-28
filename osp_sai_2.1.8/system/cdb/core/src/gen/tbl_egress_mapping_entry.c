
#include "proto.h"
#include "gen/tbl_egress_mapping_entry_define.h"
#include "gen/tbl_egress_mapping_entry.h"
#include "cdb_data_cmp.h"

tbl_egress_mapping_entry_master_t *_g_p_tbl_egress_mapping_entry_master = NULL;

static uint32
_tbl_egress_mapping_entry_hash_make(tbl_egress_mapping_entry_t *p_egress_mapping_entry)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_egress_mapping_entry->key;
    for (index = 0; index < sizeof(p_egress_mapping_entry->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_egress_mapping_entry_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_egress_mapping_entry_t *p_egress_mapping_entry1 = (tbl_egress_mapping_entry_t*)p_arg1;
    tbl_egress_mapping_entry_t *p_egress_mapping_entry2 = (tbl_egress_mapping_entry_t*)p_arg2;

    if (0 == sal_memcmp(&p_egress_mapping_entry1->key, &p_egress_mapping_entry2->key, sizeof(tbl_egress_mapping_entry_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_egress_mapping_entry_add_egress_mapping_entry_sync(tbl_egress_mapping_entry_t *p_egress_mapping_entry, uint32 sync)
{
    tbl_egress_mapping_entry_master_t *p_master = _g_p_tbl_egress_mapping_entry_master;
    tbl_egress_mapping_entry_t *p_db_egress_mapping_entry = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_egress_mapping_entry_get_egress_mapping_entry(&p_egress_mapping_entry->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_egress_mapping_entry = XCALLOC(MEM_TBL_EGRESS_MAPPING_ENTRY, sizeof(tbl_egress_mapping_entry_t));
    if (NULL == p_db_egress_mapping_entry)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_egress_mapping_entry, p_egress_mapping_entry, sizeof(tbl_egress_mapping_entry_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->egress_mapping_entry_hash, (void*)p_db_egress_mapping_entry, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->egress_mapping_entry_list, p_db_egress_mapping_entry);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_EGRESS_MAPPING_ENTRY, p_db_egress_mapping_entry);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_EGRESS_MAPPING_ENTRY, p_db_egress_mapping_entry);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_egress_mapping_entry_del_egress_mapping_entry_sync(tbl_egress_mapping_entry_key_t *p_egress_mapping_entry_key, uint32 sync)
{
    tbl_egress_mapping_entry_master_t *p_master = _g_p_tbl_egress_mapping_entry_master;
    tbl_egress_mapping_entry_t *p_db_egress_mapping_entry = NULL;

    /* 1. lookup entry exist */
    p_db_egress_mapping_entry = tbl_egress_mapping_entry_get_egress_mapping_entry(p_egress_mapping_entry_key);
    if (NULL == p_db_egress_mapping_entry)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_EGRESS_MAPPING_ENTRY, p_db_egress_mapping_entry);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_EGRESS_MAPPING_ENTRY, p_db_egress_mapping_entry);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->egress_mapping_entry_hash, (void*)p_db_egress_mapping_entry);
    ctclib_slistnode_delete(p_master->egress_mapping_entry_list, p_db_egress_mapping_entry);

    /* 4. free db entry */
    XFREE(MEM_TBL_EGRESS_MAPPING_ENTRY, p_db_egress_mapping_entry);

    return PM_E_NONE;
}

int32
tbl_egress_mapping_entry_set_egress_mapping_entry_field_sync(tbl_egress_mapping_entry_t *p_egress_mapping_entry, tbl_egress_mapping_entry_field_id_t field_id, uint32 sync)
{
    tbl_egress_mapping_entry_t *p_db_egress_mapping_entry = NULL;

    /* 1. lookup entry exist */
    p_db_egress_mapping_entry = tbl_egress_mapping_entry_get_egress_mapping_entry(&p_egress_mapping_entry->key);
    if (NULL == p_db_egress_mapping_entry)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_EGRESS_MAPPING_ENTRY_FLD_MAPPED_VID:
        p_db_egress_mapping_entry->mapped_vid = p_egress_mapping_entry->mapped_vid;
        break;

    case TBL_EGRESS_MAPPING_ENTRY_FLD_MAX:
        sal_memcpy(p_db_egress_mapping_entry, p_egress_mapping_entry, sizeof(tbl_egress_mapping_entry_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_EGRESS_MAPPING_ENTRY, field_id, p_db_egress_mapping_entry);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_EGRESS_MAPPING_ENTRY, field_id, p_db_egress_mapping_entry);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_egress_mapping_entry_add_egress_mapping_entry(tbl_egress_mapping_entry_t *p_egress_mapping_entry)
{
    return tbl_egress_mapping_entry_add_egress_mapping_entry_sync(p_egress_mapping_entry, TRUE);
}

int32
tbl_egress_mapping_entry_del_egress_mapping_entry(tbl_egress_mapping_entry_key_t *p_egress_mapping_entry_key)
{
    return tbl_egress_mapping_entry_del_egress_mapping_entry_sync(p_egress_mapping_entry_key, TRUE);
}

int32
tbl_egress_mapping_entry_set_egress_mapping_entry_field(tbl_egress_mapping_entry_t *p_egress_mapping_entry, tbl_egress_mapping_entry_field_id_t field_id)
{
    return tbl_egress_mapping_entry_set_egress_mapping_entry_field_sync(p_egress_mapping_entry, field_id, TRUE);
}

tbl_egress_mapping_entry_t*
tbl_egress_mapping_entry_get_egress_mapping_entry(tbl_egress_mapping_entry_key_t *p_egress_mapping_entry_key)
{
    tbl_egress_mapping_entry_master_t *p_master = _g_p_tbl_egress_mapping_entry_master;
    tbl_egress_mapping_entry_t lkp;

    sal_memcpy(&lkp.key, p_egress_mapping_entry_key, sizeof(tbl_egress_mapping_entry_key_t));
    return ctclib_hash_lookup(p_master->egress_mapping_entry_hash, &lkp);
}

char*
tbl_egress_mapping_entry_key_val2str(tbl_egress_mapping_entry_t *p_egress_mapping_entry, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_egress_mapping_entry_key_val2str(buf, MAX_CMD_STR_LEN, &p_egress_mapping_entry->key));
    return str;
}

int32
tbl_egress_mapping_entry_key_str2val(char *str, tbl_egress_mapping_entry_t *p_egress_mapping_entry)
{
    int32 ret = 0;

    ret = cdb_egress_mapping_entry_key_str2val(str, &p_egress_mapping_entry->key);
    return ret;
}

char*
tbl_egress_mapping_entry_key_name_dump(tbl_egress_mapping_entry_t *p_egress_mapping_entry, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_EGRESS_MAPPING_ENTRY);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_egress_mapping_entry_key_val2str(buf, MAX_CMD_STR_LEN, &p_egress_mapping_entry->key));
    return str;
}

char*
tbl_egress_mapping_entry_key_value_dump(tbl_egress_mapping_entry_t *p_egress_mapping_entry, char *str)
{

    cdb_egress_mapping_entry_key_val2str(str, MAX_CMD_STR_LEN, &p_egress_mapping_entry->key);
    return str;
}

char*
tbl_egress_mapping_entry_field_name_dump(tbl_egress_mapping_entry_t *p_egress_mapping_entry, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_EGRESS_MAPPING_ENTRY);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_EGRESS_MAPPING_ENTRY_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_egress_mapping_entry_field_value_dump(tbl_egress_mapping_entry_t *p_egress_mapping_entry, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_EGRESS_MAPPING_ENTRY_FLD_KEY, field_id))
    {
        cdb_egress_mapping_entry_key_val2str(str, MAX_CMD_STR_LEN, &p_egress_mapping_entry->key);
    }
    if (FLD_MATCH(TBL_EGRESS_MAPPING_ENTRY_FLD_MAPPED_VID, field_id))
    {
        sal_sprintf(str, "%u", p_egress_mapping_entry->mapped_vid);
    }
    return str;
}

char**
tbl_egress_mapping_entry_table_dump(tbl_egress_mapping_entry_t *p_egress_mapping_entry, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_egress_mapping_entry_key_name_dump(p_egress_mapping_entry, buf));
    for(i=1; i<TBL_EGRESS_MAPPING_ENTRY_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_egress_mapping_entry_field_name_dump(p_egress_mapping_entry, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_egress_mapping_entry_field_value_dump(p_egress_mapping_entry, i, buf));
    }
    return str;
}

int32
tbl_egress_mapping_entry_field_value_parser(char *str, int32 field_id, tbl_egress_mapping_entry_t *p_egress_mapping_entry)
{
    if (FLD_MATCH(TBL_EGRESS_MAPPING_ENTRY_FLD_KEY, field_id))
    {
        cdb_egress_mapping_entry_key_str2val(str, &p_egress_mapping_entry->key);
    }
    if (FLD_MATCH(TBL_EGRESS_MAPPING_ENTRY_FLD_MAPPED_VID, field_id))
    {
        int32 ret;
        p_egress_mapping_entry->mapped_vid = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_egress_mapping_entry_table_parser(char** array, char* key_value,tbl_egress_mapping_entry_t *p_egress_mapping_entry)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_EGRESS_MAPPING_ENTRY);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_egress_mapping_entry_key_str2val(key_value, p_egress_mapping_entry));

    for(i=1; i<TBL_EGRESS_MAPPING_ENTRY_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_egress_mapping_entry_field_value_parser( array[j++], i, p_egress_mapping_entry));
    }

    return PM_E_NONE;
}

int32
tbl_egress_mapping_entry_dump_one(tbl_egress_mapping_entry_t *p_egress_mapping_entry, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_EGRESS_MAPPING_ENTRY);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_EGRESS_MAPPING_ENTRY_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_EGRESS_MAPPING_ENTRY_FLD_KEY].name, 
            cdb_egress_mapping_entry_key_val2str(buf, MAX_CMD_STR_LEN, &p_egress_mapping_entry->key));
    }
    if (FLD_MATCH(TBL_EGRESS_MAPPING_ENTRY_FLD_MAPPED_VID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_EGRESS_MAPPING_ENTRY_FLD_MAPPED_VID].name,
            p_egress_mapping_entry->mapped_vid);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_egress_mapping_entry_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_egress_mapping_entry_master_t *p_master = _g_p_tbl_egress_mapping_entry_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_egress_mapping_entry_t *p_db_egress_mapping_entry = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->egress_mapping_entry_list, p_db_egress_mapping_entry, listnode, next)
    {
        rc |= fn(p_db_egress_mapping_entry, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_egress_mapping_entry_master_t*
tbl_egress_mapping_entry_get_master()
{
    return _g_p_tbl_egress_mapping_entry_master;
}

tbl_egress_mapping_entry_master_t*
tbl_egress_mapping_entry_init_egress_mapping_entry()
{
    _g_p_tbl_egress_mapping_entry_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_egress_mapping_entry_master_t));
    _g_p_tbl_egress_mapping_entry_master->egress_mapping_entry_hash = ctclib_hash_create(_tbl_egress_mapping_entry_hash_make, _tbl_egress_mapping_entry_hash_cmp);
    _g_p_tbl_egress_mapping_entry_master->egress_mapping_entry_list = ctclib_slist_create(tbl_egress_mapping_entry_cmp, NULL);
    return _g_p_tbl_egress_mapping_entry_master;
}

