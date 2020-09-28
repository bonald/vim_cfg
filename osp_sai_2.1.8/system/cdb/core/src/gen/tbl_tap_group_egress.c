
#include "proto.h"
#include "gen/tbl_tap_group_egress_define.h"
#include "gen/tbl_tap_group_egress.h"
#include "cdb_data_cmp.h"

tbl_tap_group_egress_master_t *_g_p_tbl_tap_group_egress_master = NULL;

static uint32
_tbl_tap_group_egress_hash_make(tbl_tap_group_egress_t *p_tap_group_egress)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_tap_group_egress->key;
    for (index = 0; index < sizeof(p_tap_group_egress->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_tap_group_egress_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_tap_group_egress_t *p_tap_group_egress1 = (tbl_tap_group_egress_t*)p_arg1;
    tbl_tap_group_egress_t *p_tap_group_egress2 = (tbl_tap_group_egress_t*)p_arg2;

    if (0 == sal_memcmp(&p_tap_group_egress1->key, &p_tap_group_egress2->key, sizeof(tbl_tap_group_egress_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_tap_group_egress_add_tap_group_egress_sync(tbl_tap_group_egress_t *p_tap_group_egress, uint32 sync)
{
    tbl_tap_group_egress_master_t *p_master = _g_p_tbl_tap_group_egress_master;
    tbl_tap_group_egress_t *p_db_tap_group_egress = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_tap_group_egress_get_tap_group_egress(&p_tap_group_egress->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_tap_group_egress = XCALLOC(MEM_TBL_TAP_GROUP_EGRESS, sizeof(tbl_tap_group_egress_t));
    if (NULL == p_db_tap_group_egress)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_tap_group_egress, p_tap_group_egress, sizeof(tbl_tap_group_egress_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->tap_group_egress_hash, (void*)p_db_tap_group_egress, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->tap_group_egress_list, p_db_tap_group_egress);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_TAP_GROUP_EGRESS, p_db_tap_group_egress);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_TAP_GROUP_EGRESS, p_db_tap_group_egress);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_tap_group_egress_del_tap_group_egress_sync(tbl_tap_group_egress_key_t *p_tap_group_egress_key, uint32 sync)
{
    tbl_tap_group_egress_master_t *p_master = _g_p_tbl_tap_group_egress_master;
    tbl_tap_group_egress_t *p_db_tap_group_egress = NULL;

    /* 1. lookup entry exist */
    p_db_tap_group_egress = tbl_tap_group_egress_get_tap_group_egress(p_tap_group_egress_key);
    if (NULL == p_db_tap_group_egress)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_TAP_GROUP_EGRESS, p_db_tap_group_egress);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_TAP_GROUP_EGRESS, p_db_tap_group_egress);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->tap_group_egress_hash, (void*)p_db_tap_group_egress);
    ctclib_slistnode_delete(p_master->tap_group_egress_list, p_db_tap_group_egress);

    /* 4. free db entry */
    XFREE(MEM_TBL_TAP_GROUP_EGRESS, p_db_tap_group_egress);

    return PM_E_NONE;
}

int32
tbl_tap_group_egress_set_tap_group_egress_field_sync(tbl_tap_group_egress_t *p_tap_group_egress, tbl_tap_group_egress_field_id_t field_id, uint32 sync)
{
    tbl_tap_group_egress_t *p_db_tap_group_egress = NULL;

    /* 1. lookup entry exist */
    p_db_tap_group_egress = tbl_tap_group_egress_get_tap_group_egress(&p_tap_group_egress->key);
    if (NULL == p_db_tap_group_egress)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_TAP_GROUP_EGRESS_FLD_TIMESTAMP_EN:
        p_db_tap_group_egress->timestamp_en = p_tap_group_egress->timestamp_en;
        break;

    case TBL_TAP_GROUP_EGRESS_FLD_TAP_GROUP_MEMBER_OID:
        p_db_tap_group_egress->tap_group_member_oid = p_tap_group_egress->tap_group_member_oid;
        break;

    case TBL_TAP_GROUP_EGRESS_FLD_MAX:
        sal_memcpy(p_db_tap_group_egress, p_tap_group_egress, sizeof(tbl_tap_group_egress_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_TAP_GROUP_EGRESS, field_id, p_db_tap_group_egress);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_TAP_GROUP_EGRESS, field_id, p_db_tap_group_egress);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_tap_group_egress_add_tap_group_egress(tbl_tap_group_egress_t *p_tap_group_egress)
{
    return tbl_tap_group_egress_add_tap_group_egress_sync(p_tap_group_egress, TRUE);
}

int32
tbl_tap_group_egress_del_tap_group_egress(tbl_tap_group_egress_key_t *p_tap_group_egress_key)
{
    return tbl_tap_group_egress_del_tap_group_egress_sync(p_tap_group_egress_key, TRUE);
}

int32
tbl_tap_group_egress_set_tap_group_egress_field(tbl_tap_group_egress_t *p_tap_group_egress, tbl_tap_group_egress_field_id_t field_id)
{
    return tbl_tap_group_egress_set_tap_group_egress_field_sync(p_tap_group_egress, field_id, TRUE);
}

tbl_tap_group_egress_t*
tbl_tap_group_egress_get_tap_group_egress(tbl_tap_group_egress_key_t *p_tap_group_egress_key)
{
    tbl_tap_group_egress_master_t *p_master = _g_p_tbl_tap_group_egress_master;
    tbl_tap_group_egress_t lkp;

    sal_memcpy(&lkp.key, p_tap_group_egress_key, sizeof(tbl_tap_group_egress_key_t));
    return ctclib_hash_lookup(p_master->tap_group_egress_hash, &lkp);
}

char*
tbl_tap_group_egress_key_val2str(tbl_tap_group_egress_t *p_tap_group_egress, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_tap_group_egress_key_val2str(buf, MAX_CMD_STR_LEN, &p_tap_group_egress->key));
    return str;
}

int32
tbl_tap_group_egress_key_str2val(char *str, tbl_tap_group_egress_t *p_tap_group_egress)
{
    int32 ret = 0;

    ret = cdb_tap_group_egress_key_str2val(str, &p_tap_group_egress->key);
    return ret;
}

char*
tbl_tap_group_egress_key_name_dump(tbl_tap_group_egress_t *p_tap_group_egress, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_TAP_GROUP_EGRESS);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_tap_group_egress_key_val2str(buf, MAX_CMD_STR_LEN, &p_tap_group_egress->key));
    return str;
}

char*
tbl_tap_group_egress_key_value_dump(tbl_tap_group_egress_t *p_tap_group_egress, char *str)
{

    cdb_tap_group_egress_key_val2str(str, MAX_CMD_STR_LEN, &p_tap_group_egress->key);
    return str;
}

char*
tbl_tap_group_egress_field_name_dump(tbl_tap_group_egress_t *p_tap_group_egress, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_TAP_GROUP_EGRESS);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_TAP_GROUP_EGRESS_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_tap_group_egress_field_value_dump(tbl_tap_group_egress_t *p_tap_group_egress, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_TAP_GROUP_EGRESS_FLD_KEY, field_id))
    {
        cdb_tap_group_egress_key_val2str(str, MAX_CMD_STR_LEN, &p_tap_group_egress->key);
    }
    if (FLD_MATCH(TBL_TAP_GROUP_EGRESS_FLD_TIMESTAMP_EN, field_id))
    {
        sal_sprintf(str, "%u", p_tap_group_egress->timestamp_en);
    }
    if (FLD_MATCH(TBL_TAP_GROUP_EGRESS_FLD_TAP_GROUP_MEMBER_OID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_tap_group_egress->tap_group_member_oid);
    }
    return str;
}

char**
tbl_tap_group_egress_table_dump(tbl_tap_group_egress_t *p_tap_group_egress, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_tap_group_egress_key_name_dump(p_tap_group_egress, buf));
    for(i=1; i<TBL_TAP_GROUP_EGRESS_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_tap_group_egress_field_name_dump(p_tap_group_egress, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_tap_group_egress_field_value_dump(p_tap_group_egress, i, buf));
    }
    return str;
}

int32
tbl_tap_group_egress_field_value_parser(char *str, int32 field_id, tbl_tap_group_egress_t *p_tap_group_egress)
{
    if (FLD_MATCH(TBL_TAP_GROUP_EGRESS_FLD_KEY, field_id))
    {
        cdb_tap_group_egress_key_str2val(str, &p_tap_group_egress->key);
    }
    if (FLD_MATCH(TBL_TAP_GROUP_EGRESS_FLD_TIMESTAMP_EN, field_id))
    {
        int32 ret;
        p_tap_group_egress->timestamp_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TAP_GROUP_EGRESS_FLD_TAP_GROUP_MEMBER_OID, field_id))
    {
        int32 ret;
        p_tap_group_egress->tap_group_member_oid = cdb_uint64_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_tap_group_egress_table_parser(char** array, char* key_value,tbl_tap_group_egress_t *p_tap_group_egress)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_TAP_GROUP_EGRESS);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_tap_group_egress_key_str2val(key_value, p_tap_group_egress));

    for(i=1; i<TBL_TAP_GROUP_EGRESS_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_tap_group_egress_field_value_parser( array[j++], i, p_tap_group_egress));
    }

    return PM_E_NONE;
}

int32
tbl_tap_group_egress_dump_one(tbl_tap_group_egress_t *p_tap_group_egress, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_TAP_GROUP_EGRESS);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_TAP_GROUP_EGRESS_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_TAP_GROUP_EGRESS_FLD_KEY].name, 
            cdb_tap_group_egress_key_val2str(buf, MAX_CMD_STR_LEN, &p_tap_group_egress->key));
    }
    if (FLD_MATCH(TBL_TAP_GROUP_EGRESS_FLD_TIMESTAMP_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TAP_GROUP_EGRESS_FLD_TIMESTAMP_EN].name,
            p_tap_group_egress->timestamp_en);
    }
    if (FLD_MATCH(TBL_TAP_GROUP_EGRESS_FLD_TAP_GROUP_MEMBER_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_TAP_GROUP_EGRESS_FLD_TAP_GROUP_MEMBER_OID].name,
            p_tap_group_egress->tap_group_member_oid);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_tap_group_egress_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_tap_group_egress_master_t *p_master = _g_p_tbl_tap_group_egress_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_tap_group_egress_t *p_db_tap_group_egress = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->tap_group_egress_list, p_db_tap_group_egress, listnode, next)
    {
        rc |= fn(p_db_tap_group_egress, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_tap_group_egress_master_t*
tbl_tap_group_egress_get_master()
{
    return _g_p_tbl_tap_group_egress_master;
}

tbl_tap_group_egress_master_t*
tbl_tap_group_egress_init_tap_group_egress()
{
    _g_p_tbl_tap_group_egress_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_tap_group_egress_master_t));
    _g_p_tbl_tap_group_egress_master->tap_group_egress_hash = ctclib_hash_create(_tbl_tap_group_egress_hash_make, _tbl_tap_group_egress_hash_cmp);
    _g_p_tbl_tap_group_egress_master->tap_group_egress_list = ctclib_slist_create(tbl_tap_group_egress_cmp, NULL);
    return _g_p_tbl_tap_group_egress_master;
}

