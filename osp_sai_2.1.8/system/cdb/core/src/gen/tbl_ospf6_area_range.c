
#include "proto.h"
#include "gen/tbl_ospf6_area_range_define.h"
#include "gen/tbl_ospf6_area_range.h"
#include "cdb_data_cmp.h"

tbl_ospf6_area_range_master_t *_g_p_tbl_ospf6_area_range_master = NULL;

static uint32
_tbl_ospf6_area_range_hash_make(tbl_ospf6_area_range_t *p_ospf6_area_range)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_ospf6_area_range->key;
    for (index = 0; index < sizeof(p_ospf6_area_range->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_ospf6_area_range_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_ospf6_area_range_t *p_ospf6_area_range1 = (tbl_ospf6_area_range_t*)p_arg1;
    tbl_ospf6_area_range_t *p_ospf6_area_range2 = (tbl_ospf6_area_range_t*)p_arg2;

    if (0 == sal_memcmp(&p_ospf6_area_range1->key, &p_ospf6_area_range2->key, sizeof(tbl_ospf6_area_range_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_ospf6_area_range_add_ospf6_area_range_sync(tbl_ospf6_area_range_t *p_ospf6_area_range, uint32 sync)
{
    tbl_ospf6_area_range_master_t *p_master = _g_p_tbl_ospf6_area_range_master;
    tbl_ospf6_area_range_t *p_db_ospf6_area_range = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_ospf6_area_range_get_ospf6_area_range(&p_ospf6_area_range->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_ospf6_area_range = XCALLOC(MEM_TBL_OSPF6_AREA_RANGE, sizeof(tbl_ospf6_area_range_t));
    if (NULL == p_db_ospf6_area_range)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_ospf6_area_range, p_ospf6_area_range, sizeof(tbl_ospf6_area_range_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->ospf6_area_range_hash, (void*)p_db_ospf6_area_range, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->ospf6_area_range_list, p_db_ospf6_area_range);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_OSPF6_AREA_RANGE, p_db_ospf6_area_range);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_OSPF6_AREA_RANGE, p_db_ospf6_area_range);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ospf6_area_range_del_ospf6_area_range_sync(tbl_ospf6_area_range_key_t *p_ospf6_area_range_key, uint32 sync)
{
    tbl_ospf6_area_range_master_t *p_master = _g_p_tbl_ospf6_area_range_master;
    tbl_ospf6_area_range_t *p_db_ospf6_area_range = NULL;

    /* 1. lookup entry exist */
    p_db_ospf6_area_range = tbl_ospf6_area_range_get_ospf6_area_range(p_ospf6_area_range_key);
    if (NULL == p_db_ospf6_area_range)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_OSPF6_AREA_RANGE, p_db_ospf6_area_range);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_OSPF6_AREA_RANGE, p_db_ospf6_area_range);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->ospf6_area_range_hash, (void*)p_db_ospf6_area_range);
    ctclib_slistnode_delete(p_master->ospf6_area_range_list, p_db_ospf6_area_range);

    /* 4. free db entry */
    XFREE(MEM_TBL_OSPF6_AREA_RANGE, p_db_ospf6_area_range);

    return PM_E_NONE;
}

int32
tbl_ospf6_area_range_set_ospf6_area_range_field_sync(tbl_ospf6_area_range_t *p_ospf6_area_range, tbl_ospf6_area_range_field_id_t field_id, uint32 sync)
{
    tbl_ospf6_area_range_t *p_db_ospf6_area_range = NULL;

    /* 1. lookup entry exist */
    p_db_ospf6_area_range = tbl_ospf6_area_range_get_ospf6_area_range(&p_ospf6_area_range->key);
    if (NULL == p_db_ospf6_area_range)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_OSPF6_AREA_RANGE_FLD_ADVERTISE:
        p_db_ospf6_area_range->advertise = p_ospf6_area_range->advertise;
        break;

    case TBL_OSPF6_AREA_RANGE_FLD_MAX:
        sal_memcpy(p_db_ospf6_area_range, p_ospf6_area_range, sizeof(tbl_ospf6_area_range_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_OSPF6_AREA_RANGE, field_id, p_db_ospf6_area_range);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_OSPF6_AREA_RANGE, field_id, p_db_ospf6_area_range);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ospf6_area_range_add_ospf6_area_range(tbl_ospf6_area_range_t *p_ospf6_area_range)
{
    return tbl_ospf6_area_range_add_ospf6_area_range_sync(p_ospf6_area_range, TRUE);
}

int32
tbl_ospf6_area_range_del_ospf6_area_range(tbl_ospf6_area_range_key_t *p_ospf6_area_range_key)
{
    return tbl_ospf6_area_range_del_ospf6_area_range_sync(p_ospf6_area_range_key, TRUE);
}

int32
tbl_ospf6_area_range_set_ospf6_area_range_field(tbl_ospf6_area_range_t *p_ospf6_area_range, tbl_ospf6_area_range_field_id_t field_id)
{
    return tbl_ospf6_area_range_set_ospf6_area_range_field_sync(p_ospf6_area_range, field_id, TRUE);
}

tbl_ospf6_area_range_t*
tbl_ospf6_area_range_get_ospf6_area_range(tbl_ospf6_area_range_key_t *p_ospf6_area_range_key)
{
    tbl_ospf6_area_range_master_t *p_master = _g_p_tbl_ospf6_area_range_master;
    tbl_ospf6_area_range_t lkp;

    sal_memcpy(&lkp.key, p_ospf6_area_range_key, sizeof(tbl_ospf6_area_range_key_t));
    return ctclib_hash_lookup(p_master->ospf6_area_range_hash, &lkp);
}

char*
tbl_ospf6_area_range_key_val2str(tbl_ospf6_area_range_t *p_ospf6_area_range, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_ospf6_area_range_key_val2str(buf, MAX_CMD_STR_LEN, &p_ospf6_area_range->key));
    return str;
}

int32
tbl_ospf6_area_range_key_str2val(char *str, tbl_ospf6_area_range_t *p_ospf6_area_range)
{
    int32 ret = 0;

    ret = cdb_ospf6_area_range_key_str2val(str, &p_ospf6_area_range->key);
    return ret;
}

char*
tbl_ospf6_area_range_key_name_dump(tbl_ospf6_area_range_t *p_ospf6_area_range, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_OSPF6_AREA_RANGE);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_ospf6_area_range_key_val2str(buf, MAX_CMD_STR_LEN, &p_ospf6_area_range->key));
    return str;
}

char*
tbl_ospf6_area_range_key_value_dump(tbl_ospf6_area_range_t *p_ospf6_area_range, char *str)
{

    cdb_ospf6_area_range_key_val2str(str, MAX_CMD_STR_LEN, &p_ospf6_area_range->key);
    return str;
}

char*
tbl_ospf6_area_range_field_name_dump(tbl_ospf6_area_range_t *p_ospf6_area_range, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_OSPF6_AREA_RANGE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_OSPF6_AREA_RANGE_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_ospf6_area_range_field_value_dump(tbl_ospf6_area_range_t *p_ospf6_area_range, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_OSPF6_AREA_RANGE_FLD_KEY, field_id))
    {
        cdb_ospf6_area_range_key_val2str(str, MAX_CMD_STR_LEN, &p_ospf6_area_range->key);
    }
    if (FLD_MATCH(TBL_OSPF6_AREA_RANGE_FLD_ADVERTISE, field_id))
    {
        sal_sprintf(str, "%u", p_ospf6_area_range->advertise);
    }
    return str;
}

char**
tbl_ospf6_area_range_table_dump(tbl_ospf6_area_range_t *p_ospf6_area_range, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_ospf6_area_range_key_name_dump(p_ospf6_area_range, buf));
    for(i=1; i<TBL_OSPF6_AREA_RANGE_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_ospf6_area_range_field_name_dump(p_ospf6_area_range, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_ospf6_area_range_field_value_dump(p_ospf6_area_range, i, buf));
    }
    return str;
}

int32
tbl_ospf6_area_range_field_value_parser(char *str, int32 field_id, tbl_ospf6_area_range_t *p_ospf6_area_range)
{
    if (FLD_MATCH(TBL_OSPF6_AREA_RANGE_FLD_KEY, field_id))
    {
        cdb_ospf6_area_range_key_str2val(str, &p_ospf6_area_range->key);
    }
    if (FLD_MATCH(TBL_OSPF6_AREA_RANGE_FLD_ADVERTISE, field_id))
    {
        int32 ret;
        p_ospf6_area_range->advertise = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_ospf6_area_range_table_parser(char** array, char* key_value,tbl_ospf6_area_range_t *p_ospf6_area_range)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_OSPF6_AREA_RANGE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_ospf6_area_range_key_str2val(key_value, p_ospf6_area_range));

    for(i=1; i<TBL_OSPF6_AREA_RANGE_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_ospf6_area_range_field_value_parser( array[j++], i, p_ospf6_area_range));
    }

    return PM_E_NONE;
}

int32
tbl_ospf6_area_range_dump_one(tbl_ospf6_area_range_t *p_ospf6_area_range, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_OSPF6_AREA_RANGE);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_OSPF6_AREA_RANGE_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_OSPF6_AREA_RANGE_FLD_KEY].name, 
            cdb_ospf6_area_range_key_val2str(buf, MAX_CMD_STR_LEN, &p_ospf6_area_range->key));
    }
    if (FLD_MATCH(TBL_OSPF6_AREA_RANGE_FLD_ADVERTISE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_OSPF6_AREA_RANGE_FLD_ADVERTISE].name,
            p_ospf6_area_range->advertise);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_ospf6_area_range_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_ospf6_area_range_master_t *p_master = _g_p_tbl_ospf6_area_range_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_ospf6_area_range_t *p_db_ospf6_area_range = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->ospf6_area_range_list, p_db_ospf6_area_range, listnode, next)
    {
        rc |= fn(p_db_ospf6_area_range, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_ospf6_area_range_master_t*
tbl_ospf6_area_range_get_master()
{
    return _g_p_tbl_ospf6_area_range_master;
}

tbl_ospf6_area_range_master_t*
tbl_ospf6_area_range_init_ospf6_area_range()
{
    _g_p_tbl_ospf6_area_range_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_ospf6_area_range_master_t));
    _g_p_tbl_ospf6_area_range_master->ospf6_area_range_hash = ctclib_hash_create(_tbl_ospf6_area_range_hash_make, _tbl_ospf6_area_range_hash_cmp);
    _g_p_tbl_ospf6_area_range_master->ospf6_area_range_list = ctclib_slist_create(tbl_ospf6_area_range_cmp, NULL);
    return _g_p_tbl_ospf6_area_range_master;
}

