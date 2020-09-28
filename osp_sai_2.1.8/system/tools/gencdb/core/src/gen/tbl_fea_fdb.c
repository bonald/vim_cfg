
#include "proto.h"
#include "gen/tbl_fea_fdb_define.h"
#include "gen/tbl_fea_fdb.h"
#include "cdb_data_cmp.h"

tbl_fea_fdb_master_t *_g_p_tbl_fea_fdb_master = NULL;

static uint32
_tbl_fea_fdb_hash_make(tbl_fea_fdb_t *p_fdb)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_fdb->key;
    for (index = 0; index < sizeof(p_fdb->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_fea_fdb_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_fea_fdb_t *p_fdb1 = (tbl_fea_fdb_t*)p_arg1;
    tbl_fea_fdb_t *p_fdb2 = (tbl_fea_fdb_t*)p_arg2;

    if (0 == sal_memcmp(&p_fdb1->key, &p_fdb2->key, sizeof(tbl_fea_fdb_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_fea_fdb_add_fea_fdb_sync(tbl_fea_fdb_t *p_fdb, uint32 sync)
{
    tbl_fea_fdb_master_t *p_master = _g_p_tbl_fea_fdb_master;
    tbl_fea_fdb_t *p_db_fdb = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_fea_fdb_get_fea_fdb(&p_fdb->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_fdb = XCALLOC(MEM_TBL_FEA_FDB, sizeof(tbl_fea_fdb_t));
    if (NULL == p_db_fdb)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_fdb, p_fdb, sizeof(tbl_fea_fdb_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->fdb_hash, (void*)p_db_fdb, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_FEA_FDB, p_db_fdb);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_FEA_FDB, p_db_fdb);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_fea_fdb_del_fea_fdb_sync(tbl_fea_fdb_key_t *p_fdb_key, uint32 sync)
{
    tbl_fea_fdb_master_t *p_master = _g_p_tbl_fea_fdb_master;
    tbl_fea_fdb_t *p_db_fdb = NULL;

    /* 1. lookup entry exist */
    p_db_fdb = tbl_fea_fdb_get_fea_fdb(p_fdb_key);
    if (NULL == p_db_fdb)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_FEA_FDB, p_db_fdb);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_FEA_FDB, p_db_fdb);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->fdb_hash, (void*)p_db_fdb);

    /* 4. free db entry */
    XFREE(MEM_TBL_FEA_FDB, p_db_fdb);

    return PM_E_NONE;
}

int32
tbl_fea_fdb_set_fea_fdb_field_sync(tbl_fea_fdb_t *p_fdb, tbl_fea_fdb_field_id_t field_id, uint32 sync)
{
    tbl_fea_fdb_t *p_db_fdb = NULL;

    /* 1. lookup entry exist */
    p_db_fdb = tbl_fea_fdb_get_fea_fdb(&p_fdb->key);
    if (NULL == p_db_fdb)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_FEA_FDB_FLD_IFINDEX:
        p_db_fdb->ifindex = p_fdb->ifindex;
        break;

    case TBL_FEA_FDB_FLD_MLAG_SOURCE_IFINDEX:
        p_db_fdb->mlag_source_ifindex = p_fdb->mlag_source_ifindex;
        break;

    case TBL_FEA_FDB_FLD_PORTID:
        p_db_fdb->portid = p_fdb->portid;
        break;

    case TBL_FEA_FDB_FLD_FLAGS:
        p_db_fdb->flags = p_fdb->flags;
        break;

    case TBL_FEA_FDB_FLD_FEA_FAIL:
        p_db_fdb->fea_fail = p_fdb->fea_fail;
        break;

    case TBL_FEA_FDB_FLD_MAX:
        sal_memcpy(p_db_fdb, p_fdb, sizeof(tbl_fea_fdb_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_FEA_FDB, field_id, p_db_fdb);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_FEA_FDB, field_id, p_db_fdb);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_fea_fdb_add_fea_fdb(tbl_fea_fdb_t *p_fdb)
{
    return tbl_fea_fdb_add_fea_fdb_sync(p_fdb, TRUE);
}

int32
tbl_fea_fdb_del_fea_fdb(tbl_fea_fdb_key_t *p_fdb_key)
{
    return tbl_fea_fdb_del_fea_fdb_sync(p_fdb_key, TRUE);
}

int32
tbl_fea_fdb_set_fea_fdb_field(tbl_fea_fdb_t *p_fdb, tbl_fea_fdb_field_id_t field_id)
{
    return tbl_fea_fdb_set_fea_fdb_field_sync(p_fdb, field_id, TRUE);
}

tbl_fea_fdb_t*
tbl_fea_fdb_get_fea_fdb(tbl_fea_fdb_key_t *p_fdb_key)
{
    tbl_fea_fdb_master_t *p_master = _g_p_tbl_fea_fdb_master;
    tbl_fea_fdb_t lkp;

    sal_memcpy(&lkp.key, p_fdb_key, sizeof(tbl_fea_fdb_key_t));
    return ctclib_hash_lookup(p_master->fdb_hash, &lkp);
}

char*
tbl_fea_fdb_key_val2str(tbl_fea_fdb_t *p_fdb, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_fdb_key_val2str(buf, MAX_CMD_STR_LEN, &p_fdb->key));
    return str;
}

int32
tbl_fea_fdb_key_str2val(char *str, tbl_fea_fdb_t *p_fdb)
{
    int32 ret = 0;

    char err[MAX_CMD_STR_LEN];
    ret = cdb_fdb_key_str2val(&p_fdb->key, str, err);
    return ret;
}

char*
tbl_fea_fdb_key_name_dump(tbl_fea_fdb_t *p_fdb, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_FDB);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_fdb_key_val2str(buf, MAX_CMD_STR_LEN, &p_fdb->key));
    return str;
}

char*
tbl_fea_fdb_key_value_dump(tbl_fea_fdb_t *p_fdb, char *str)
{

    cdb_fdb_key_val2str(str, MAX_CMD_STR_LEN, &p_fdb->key);
    return str;
}

char*
tbl_fea_fdb_field_name_dump(tbl_fea_fdb_t *p_fdb, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_FDB);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_FEA_FDB_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_fea_fdb_field_value_dump(tbl_fea_fdb_t *p_fdb, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_FEA_FDB_FLD_KEY, field_id))
    {
        cdb_fdb_key_val2str(str, MAX_CMD_STR_LEN, &p_fdb->key);
    }
    if (FLD_MATCH(TBL_FEA_FDB_FLD_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_fdb->ifindex);
    }
    if (FLD_MATCH(TBL_FEA_FDB_FLD_MLAG_SOURCE_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_fdb->mlag_source_ifindex);
    }
    if (FLD_MATCH(TBL_FEA_FDB_FLD_PORTID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_fdb->portid);
    }
    if (FLD_MATCH(TBL_FEA_FDB_FLD_FLAGS, field_id))
    {
        sal_sprintf(str, "%u", p_fdb->flags);
    }
    if (FLD_MATCH(TBL_FEA_FDB_FLD_FEA_FAIL, field_id))
    {
        sal_sprintf(str, "%u", p_fdb->fea_fail);
    }
    return str;
}

char**
tbl_fea_fdb_table_dump(tbl_fea_fdb_t *p_fdb, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_fea_fdb_key_name_dump(p_fdb, buf));
    for(i=1; i<TBL_FEA_FDB_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_fea_fdb_field_name_dump(p_fdb, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_fea_fdb_field_value_dump(p_fdb, i, buf));
    }
    return str;
}

int32
tbl_fea_fdb_field_value_parser(char *str, int32 field_id, tbl_fea_fdb_t *p_fdb)
{
    if (FLD_MATCH(TBL_FEA_FDB_FLD_KEY, field_id))
    {
        char err[MAX_CMD_STR_LEN];
        cdb_fdb_key_str2val(&p_fdb->key, str, err);
    }
    if (FLD_MATCH(TBL_FEA_FDB_FLD_IFINDEX, field_id))
    {
        int32 ret;
        p_fdb->ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_FDB_FLD_MLAG_SOURCE_IFINDEX, field_id))
    {
        int32 ret;
        p_fdb->mlag_source_ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_FDB_FLD_PORTID, field_id))
    {
        int32 ret;
        p_fdb->portid = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_FDB_FLD_FLAGS, field_id))
    {
        int32 ret;
        p_fdb->flags = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_FDB_FLD_FEA_FAIL, field_id))
    {
        int32 ret;
        p_fdb->fea_fail = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_fea_fdb_table_parser(char** array, char* key_value,tbl_fea_fdb_t *p_fdb)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_FDB);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_fea_fdb_key_str2val(key_value, p_fdb));

    for(i=1; i<TBL_FEA_FDB_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_fea_fdb_field_value_parser( array[j++], i, p_fdb));
    }

    return PM_E_NONE;
}

int32
tbl_fea_fdb_dump_one(tbl_fea_fdb_t *p_fdb, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_FDB);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_FEA_FDB_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_FEA_FDB_FLD_KEY].name, 
            cdb_fdb_key_val2str(buf, MAX_CMD_STR_LEN, &p_fdb->key));
    }
    if (FLD_MATCH(TBL_FEA_FDB_FLD_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_FDB_FLD_IFINDEX].name,
            p_fdb->ifindex);
    }
    if (FLD_MATCH(TBL_FEA_FDB_FLD_MLAG_SOURCE_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_FDB_FLD_MLAG_SOURCE_IFINDEX].name,
            p_fdb->mlag_source_ifindex);
    }
    if (FLD_MATCH(TBL_FEA_FDB_FLD_PORTID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_FEA_FDB_FLD_PORTID].name,
            p_fdb->portid);
    }
    if (FLD_MATCH(TBL_FEA_FDB_FLD_FLAGS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_FDB_FLD_FLAGS].name,
            p_fdb->flags);
    }
    if (FLD_MATCH(TBL_FEA_FDB_FLD_FEA_FAIL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_FDB_FLD_FEA_FAIL].name,
            p_fdb->fea_fail);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_fea_fdb_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_fea_fdb_master_t *p_master = _g_p_tbl_fea_fdb_master;
    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    ctclib_hash_iterate2(p_master->fdb_hash, tbl_hash_iter_adpt_fn, fn, pargs);
    return PM_E_NONE;
}

tbl_fea_fdb_master_t*
tbl_fea_fdb_get_master()
{
    return _g_p_tbl_fea_fdb_master;
}

tbl_fea_fdb_master_t*
tbl_fea_fdb_init_fea_fdb()
{
    _g_p_tbl_fea_fdb_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_fea_fdb_master_t));
    _g_p_tbl_fea_fdb_master->fdb_hash = ctclib_hash_create(_tbl_fea_fdb_hash_make, _tbl_fea_fdb_hash_cmp);
    return _g_p_tbl_fea_fdb_master;
}

