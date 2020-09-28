
#include "proto.h"
#include "gen/tbl_tap_five_tuple_define.h"
#include "gen/tbl_tap_five_tuple.h"
#include "cdb_data_cmp.h"

tbl_tap_five_tuple_master_t *_g_p_tbl_tap_five_tuple_master = NULL;

static uint32
_tbl_tap_five_tuple_hash_make(tbl_tap_five_tuple_t *p_tap_five_tuple)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_tap_five_tuple->key;
    for (index = 0; index < sizeof(p_tap_five_tuple->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_tap_five_tuple_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_tap_five_tuple_t *p_tap_five_tuple1 = (tbl_tap_five_tuple_t*)p_arg1;
    tbl_tap_five_tuple_t *p_tap_five_tuple2 = (tbl_tap_five_tuple_t*)p_arg2;

    if (0 == sal_memcmp(&p_tap_five_tuple1->key, &p_tap_five_tuple2->key, sizeof(tbl_tap_five_tuple_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_tap_five_tuple_add_tap_five_tuple_sync(tbl_tap_five_tuple_t *p_tap_five_tuple, uint32 sync)
{
    tbl_tap_five_tuple_master_t *p_master = _g_p_tbl_tap_five_tuple_master;
    tbl_tap_five_tuple_t *p_db_tap_five_tuple = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_tap_five_tuple_get_tap_five_tuple(&p_tap_five_tuple->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_tap_five_tuple = XCALLOC(MEM_TBL_TAP_FIVE_TUPLE, sizeof(tbl_tap_five_tuple_t));
    if (NULL == p_db_tap_five_tuple)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_tap_five_tuple, p_tap_five_tuple, sizeof(tbl_tap_five_tuple_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->tap_five_tuple_hash, (void*)p_db_tap_five_tuple, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->tap_five_tuple_list, p_db_tap_five_tuple);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_TAP_FIVE_TUPLE, p_db_tap_five_tuple);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_TAP_FIVE_TUPLE, p_db_tap_five_tuple);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_tap_five_tuple_del_tap_five_tuple_sync(tbl_tap_five_tuple_key_t *p_tap_five_tuple_key, uint32 sync)
{
    tbl_tap_five_tuple_master_t *p_master = _g_p_tbl_tap_five_tuple_master;
    tbl_tap_five_tuple_t *p_db_tap_five_tuple = NULL;

    /* 1. lookup entry exist */
    p_db_tap_five_tuple = tbl_tap_five_tuple_get_tap_five_tuple(p_tap_five_tuple_key);
    if (NULL == p_db_tap_five_tuple)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_TAP_FIVE_TUPLE, p_db_tap_five_tuple);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_TAP_FIVE_TUPLE, p_db_tap_five_tuple);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->tap_five_tuple_hash, (void*)p_db_tap_five_tuple);
    ctclib_slistnode_delete(p_master->tap_five_tuple_list, p_db_tap_five_tuple);

    /* 4. free db entry */
    XFREE(MEM_TBL_TAP_FIVE_TUPLE, p_db_tap_five_tuple);

    return PM_E_NONE;
}

int32
tbl_tap_five_tuple_set_tap_five_tuple_field_sync(tbl_tap_five_tuple_t *p_tap_five_tuple, tbl_tap_five_tuple_field_id_t field_id, uint32 sync)
{
    tbl_tap_five_tuple_t *p_db_tap_five_tuple = NULL;

    /* 1. lookup entry exist */
    p_db_tap_five_tuple = tbl_tap_five_tuple_get_tap_five_tuple(&p_tap_five_tuple->key);
    if (NULL == p_db_tap_five_tuple)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_TAP_FIVE_TUPLE_FLD_ACL_ENTRY_ID:
        p_db_tap_five_tuple->acl_entry_id = p_tap_five_tuple->acl_entry_id;
        break;

    case TBL_TAP_FIVE_TUPLE_FLD_STATS_ID:
        p_db_tap_five_tuple->stats_id = p_tap_five_tuple->stats_id;
        break;

    case TBL_TAP_FIVE_TUPLE_FLD_PACKET_COUNT:
        p_db_tap_five_tuple->packet_count = p_tap_five_tuple->packet_count;
        break;

    case TBL_TAP_FIVE_TUPLE_FLD_BYTE_COUNT:
        p_db_tap_five_tuple->byte_count = p_tap_five_tuple->byte_count;
        break;

    case TBL_TAP_FIVE_TUPLE_FLD_MAX:
        sal_memcpy(p_db_tap_five_tuple, p_tap_five_tuple, sizeof(tbl_tap_five_tuple_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_TAP_FIVE_TUPLE, field_id, p_db_tap_five_tuple);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_TAP_FIVE_TUPLE, field_id, p_db_tap_five_tuple);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_tap_five_tuple_add_tap_five_tuple(tbl_tap_five_tuple_t *p_tap_five_tuple)
{
    return tbl_tap_five_tuple_add_tap_five_tuple_sync(p_tap_five_tuple, TRUE);
}

int32
tbl_tap_five_tuple_del_tap_five_tuple(tbl_tap_five_tuple_key_t *p_tap_five_tuple_key)
{
    return tbl_tap_five_tuple_del_tap_five_tuple_sync(p_tap_five_tuple_key, TRUE);
}

int32
tbl_tap_five_tuple_set_tap_five_tuple_field(tbl_tap_five_tuple_t *p_tap_five_tuple, tbl_tap_five_tuple_field_id_t field_id)
{
    return tbl_tap_five_tuple_set_tap_five_tuple_field_sync(p_tap_five_tuple, field_id, TRUE);
}

tbl_tap_five_tuple_t*
tbl_tap_five_tuple_get_tap_five_tuple(tbl_tap_five_tuple_key_t *p_tap_five_tuple_key)
{
    tbl_tap_five_tuple_master_t *p_master = _g_p_tbl_tap_five_tuple_master;
    tbl_tap_five_tuple_t lkp;

    sal_memcpy(&lkp.key, p_tap_five_tuple_key, sizeof(tbl_tap_five_tuple_key_t));
    return ctclib_hash_lookup(p_master->tap_five_tuple_hash, &lkp);
}

char*
tbl_tap_five_tuple_key_val2str(tbl_tap_five_tuple_t *p_tap_five_tuple, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_tap_five_tuple_key_val2str(buf, MAX_CMD_STR_LEN, &p_tap_five_tuple->key));
    return str;
}

int32
tbl_tap_five_tuple_key_str2val(char *str, tbl_tap_five_tuple_t *p_tap_five_tuple)
{
    int32 ret = 0;

    ret = cdb_tap_five_tuple_key_str2val(str, &p_tap_five_tuple->key);
    return ret;
}

char*
tbl_tap_five_tuple_key_name_dump(tbl_tap_five_tuple_t *p_tap_five_tuple, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_TAP_FIVE_TUPLE);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_tap_five_tuple_key_val2str(buf, MAX_CMD_STR_LEN, &p_tap_five_tuple->key));
    return str;
}

char*
tbl_tap_five_tuple_key_value_dump(tbl_tap_five_tuple_t *p_tap_five_tuple, char *str)
{

    cdb_tap_five_tuple_key_val2str(str, MAX_CMD_STR_LEN, &p_tap_five_tuple->key);
    return str;
}

char*
tbl_tap_five_tuple_field_name_dump(tbl_tap_five_tuple_t *p_tap_five_tuple, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_TAP_FIVE_TUPLE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_TAP_FIVE_TUPLE_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_tap_five_tuple_field_value_dump(tbl_tap_five_tuple_t *p_tap_five_tuple, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_TAP_FIVE_TUPLE_FLD_KEY, field_id))
    {
        cdb_tap_five_tuple_key_val2str(str, MAX_CMD_STR_LEN, &p_tap_five_tuple->key);
    }
    if (FLD_MATCH(TBL_TAP_FIVE_TUPLE_FLD_ACL_ENTRY_ID, field_id))
    {
        sal_sprintf(str, "%u", p_tap_five_tuple->acl_entry_id);
    }
    if (FLD_MATCH(TBL_TAP_FIVE_TUPLE_FLD_STATS_ID, field_id))
    {
        sal_sprintf(str, "%u", p_tap_five_tuple->stats_id);
    }
    if (FLD_MATCH(TBL_TAP_FIVE_TUPLE_FLD_PACKET_COUNT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_tap_five_tuple->packet_count);
    }
    if (FLD_MATCH(TBL_TAP_FIVE_TUPLE_FLD_BYTE_COUNT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_tap_five_tuple->byte_count);
    }
    return str;
}

char**
tbl_tap_five_tuple_table_dump(tbl_tap_five_tuple_t *p_tap_five_tuple, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_tap_five_tuple_key_name_dump(p_tap_five_tuple, buf));
    for(i=1; i<TBL_TAP_FIVE_TUPLE_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_tap_five_tuple_field_name_dump(p_tap_five_tuple, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_tap_five_tuple_field_value_dump(p_tap_five_tuple, i, buf));
    }
    return str;
}

int32
tbl_tap_five_tuple_field_value_parser(char *str, int32 field_id, tbl_tap_five_tuple_t *p_tap_five_tuple)
{
    if (FLD_MATCH(TBL_TAP_FIVE_TUPLE_FLD_KEY, field_id))
    {
        cdb_tap_five_tuple_key_str2val(str, &p_tap_five_tuple->key);
    }
    if (FLD_MATCH(TBL_TAP_FIVE_TUPLE_FLD_ACL_ENTRY_ID, field_id))
    {
        int32 ret;
        p_tap_five_tuple->acl_entry_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TAP_FIVE_TUPLE_FLD_STATS_ID, field_id))
    {
        int32 ret;
        p_tap_five_tuple->stats_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TAP_FIVE_TUPLE_FLD_PACKET_COUNT, field_id))
    {
        int32 ret;
        p_tap_five_tuple->packet_count = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TAP_FIVE_TUPLE_FLD_BYTE_COUNT, field_id))
    {
        int32 ret;
        p_tap_five_tuple->byte_count = cdb_uint64_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_tap_five_tuple_table_parser(char** array, char* key_value,tbl_tap_five_tuple_t *p_tap_five_tuple)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_TAP_FIVE_TUPLE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_tap_five_tuple_key_str2val(key_value, p_tap_five_tuple));

    for(i=1; i<TBL_TAP_FIVE_TUPLE_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_tap_five_tuple_field_value_parser( array[j++], i, p_tap_five_tuple));
    }

    return PM_E_NONE;
}

int32
tbl_tap_five_tuple_dump_one(tbl_tap_five_tuple_t *p_tap_five_tuple, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_TAP_FIVE_TUPLE);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_TAP_FIVE_TUPLE_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_TAP_FIVE_TUPLE_FLD_KEY].name, 
            cdb_tap_five_tuple_key_val2str(buf, MAX_CMD_STR_LEN, &p_tap_five_tuple->key));
    }
    if (FLD_MATCH(TBL_TAP_FIVE_TUPLE_FLD_ACL_ENTRY_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TAP_FIVE_TUPLE_FLD_ACL_ENTRY_ID].name,
            p_tap_five_tuple->acl_entry_id);
    }
    if (FLD_MATCH(TBL_TAP_FIVE_TUPLE_FLD_STATS_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TAP_FIVE_TUPLE_FLD_STATS_ID].name,
            p_tap_five_tuple->stats_id);
    }
    if (FLD_MATCH(TBL_TAP_FIVE_TUPLE_FLD_PACKET_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_TAP_FIVE_TUPLE_FLD_PACKET_COUNT].name,
            p_tap_five_tuple->packet_count);
    }
    if (FLD_MATCH(TBL_TAP_FIVE_TUPLE_FLD_BYTE_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_TAP_FIVE_TUPLE_FLD_BYTE_COUNT].name,
            p_tap_five_tuple->byte_count);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_tap_five_tuple_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_tap_five_tuple_master_t *p_master = _g_p_tbl_tap_five_tuple_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_tap_five_tuple_t *p_db_tap_five_tuple = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->tap_five_tuple_list, p_db_tap_five_tuple, listnode, next)
    {
        rc |= fn(p_db_tap_five_tuple, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_tap_five_tuple_master_t*
tbl_tap_five_tuple_get_master()
{
    return _g_p_tbl_tap_five_tuple_master;
}

tbl_tap_five_tuple_master_t*
tbl_tap_five_tuple_init_tap_five_tuple()
{
    _g_p_tbl_tap_five_tuple_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_tap_five_tuple_master_t));
    _g_p_tbl_tap_five_tuple_master->tap_five_tuple_hash = ctclib_hash_create(_tbl_tap_five_tuple_hash_make, _tbl_tap_five_tuple_hash_cmp);
    _g_p_tbl_tap_five_tuple_master->tap_five_tuple_list = ctclib_slist_create(tbl_tap_five_tuple_cmp, NULL);
    return _g_p_tbl_tap_five_tuple_master;
}
