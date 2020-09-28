
#include "proto.h"
#include "gen/tbl_fdb_define.h"
#include "gen/tbl_fdb.h"
#include "cdb_data_cmp.h"

tbl_fdb_master_t *_g_p_tbl_fdb_master = NULL;

static uint32
_tbl_fdb_hash_make(tbl_fdb_t *p_fdb)
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
_tbl_fdb_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_fdb_t *p_fdb1 = (tbl_fdb_t*)p_arg1;
    tbl_fdb_t *p_fdb2 = (tbl_fdb_t*)p_arg2;

    if (0 == sal_memcmp(&p_fdb1->key, &p_fdb2->key, sizeof(tbl_fdb_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_fdb_add_fdb_sync(tbl_fdb_t *p_fdb, uint32 sync)
{
    tbl_fdb_master_t *p_master = _g_p_tbl_fdb_master;
    tbl_fdb_t *p_db_fdb = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_fdb_get_fdb(&p_fdb->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_fdb = XCALLOC(MEM_TBL_FDB, sizeof(tbl_fdb_t));
    if (NULL == p_db_fdb)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_fdb, p_fdb, sizeof(tbl_fdb_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->fdb_hash, (void*)p_db_fdb, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_FDB, p_db_fdb);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_FDB, p_db_fdb);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_fdb_del_fdb_sync(tbl_fdb_key_t *p_fdb_key, uint32 sync)
{
    tbl_fdb_master_t *p_master = _g_p_tbl_fdb_master;
    tbl_fdb_t *p_db_fdb = NULL;

    /* 1. lookup entry exist */
    p_db_fdb = tbl_fdb_get_fdb(p_fdb_key);
    if (NULL == p_db_fdb)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_FDB, p_db_fdb);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_FDB, p_db_fdb);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->fdb_hash, (void*)p_db_fdb);

    /* 4. free db entry */
    XFREE(MEM_TBL_FDB, p_db_fdb);

    return PM_E_NONE;
}

int32
tbl_fdb_set_fdb_field_sync(tbl_fdb_t *p_fdb, tbl_fdb_field_id_t field_id, uint32 sync)
{
    tbl_fdb_t *p_db_fdb = NULL;

    /* 1. lookup entry exist */
    p_db_fdb = tbl_fdb_get_fdb(&p_fdb->key);
    if (NULL == p_db_fdb)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_FDB_FLD_IFINDEX:
        p_db_fdb->ifindex = p_fdb->ifindex;
        break;

    case TBL_FDB_FLD_STATIC:
        CDB_FLAG_CHECK_SET(p_db_fdb, p_fdb, flags, GLB_FDB_FLAG_STATIC)
        break;

    case TBL_FDB_FLD_SECURITY:
        CDB_FLAG_CHECK_SET(p_db_fdb, p_fdb, flags, GLB_FDB_FLAG_MAC_SECURITY_EN)
        break;

    case TBL_FDB_FLD_HYBRID_FLOW:
        CDB_FLAG_CHECK_SET(p_db_fdb, p_fdb, flags, GLB_FDB_FLAG_HYBRID_FLOW)
        break;

    case TBL_FDB_FLD_MAX:
        sal_memcpy(p_db_fdb, p_fdb, sizeof(tbl_fdb_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_FDB, field_id, p_db_fdb);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_FDB, field_id, p_db_fdb);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_fdb_add_fdb(tbl_fdb_t *p_fdb)
{
    return tbl_fdb_add_fdb_sync(p_fdb, TRUE);
}

int32
tbl_fdb_del_fdb(tbl_fdb_key_t *p_fdb_key)
{
    return tbl_fdb_del_fdb_sync(p_fdb_key, TRUE);
}

int32
tbl_fdb_set_fdb_field(tbl_fdb_t *p_fdb, tbl_fdb_field_id_t field_id)
{
    return tbl_fdb_set_fdb_field_sync(p_fdb, field_id, TRUE);
}

tbl_fdb_t*
tbl_fdb_get_fdb(tbl_fdb_key_t *p_fdb_key)
{
    tbl_fdb_master_t *p_master = _g_p_tbl_fdb_master;
    tbl_fdb_t lkp;

    sal_memcpy(&lkp.key, p_fdb_key, sizeof(tbl_fdb_key_t));
    return ctclib_hash_lookup(p_master->fdb_hash, &lkp);
}

char*
tbl_fdb_key_val2str(tbl_fdb_t *p_fdb, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_fdb_key_val2str(buf, MAX_CMD_STR_LEN, &p_fdb->key));
    return str;
}

int32
tbl_fdb_key_str2val(char *str, tbl_fdb_t *p_fdb)
{
    int32 ret = 0;

    char err[MAX_CMD_STR_LEN];
    ret = cdb_fdb_key_str2val(&p_fdb->key, str, err);
    return ret;
}

char*
tbl_fdb_key_name_dump(tbl_fdb_t *p_fdb, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_FDB);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_fdb_key_val2str(buf, MAX_CMD_STR_LEN, &p_fdb->key));
    return str;
}

char*
tbl_fdb_key_value_dump(tbl_fdb_t *p_fdb, char *str)
{

    cdb_fdb_key_val2str(str, MAX_CMD_STR_LEN, &p_fdb->key);
    return str;
}

char*
tbl_fdb_field_name_dump(tbl_fdb_t *p_fdb, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_FDB);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_FDB_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_fdb_field_value_dump(tbl_fdb_t *p_fdb, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_FDB_FLD_KEY, field_id))
    {
        cdb_fdb_key_val2str(str, MAX_CMD_STR_LEN, &p_fdb->key);
    }
    if (FLD_MATCH(TBL_FDB_FLD_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_fdb->ifindex);
    }
    if (FLD_MATCH(TBL_FDB_FLD_STATIC, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC));
    }
    if (FLD_MATCH(TBL_FDB_FLD_SECURITY, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN));
    }
    if (FLD_MATCH(TBL_FDB_FLD_HYBRID_FLOW, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_HYBRID_FLOW));
    }
    return str;
}

char**
tbl_fdb_table_dump(tbl_fdb_t *p_fdb, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_fdb_key_name_dump(p_fdb, buf));
    for(i=1; i<TBL_FDB_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_fdb_field_name_dump(p_fdb, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_fdb_field_value_dump(p_fdb, i, buf));
    }
    return str;
}

int32
tbl_fdb_field_value_parser(char *str, int32 field_id, tbl_fdb_t *p_fdb)
{
    if (FLD_MATCH(TBL_FDB_FLD_KEY, field_id))
    {
        char err[MAX_CMD_STR_LEN];
        cdb_fdb_key_str2val(&p_fdb->key, str, err);
    }
    if (FLD_MATCH(TBL_FDB_FLD_IFINDEX, field_id))
    {
        int32 ret;
        p_fdb->ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FDB_FLD_STATIC, field_id))
    {
        GLB_SET_FLAG(p_fdb->flags, GLB_FDB_FLAG_STATIC);
    }
    if (FLD_MATCH(TBL_FDB_FLD_SECURITY, field_id))
    {
        GLB_SET_FLAG(p_fdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN);
    }
    if (FLD_MATCH(TBL_FDB_FLD_HYBRID_FLOW, field_id))
    {
        GLB_SET_FLAG(p_fdb->flags, GLB_FDB_FLAG_HYBRID_FLOW);
    }
    return PM_E_NONE;
}

int32
tbl_fdb_table_parser(char** array, char* key_value,tbl_fdb_t *p_fdb)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_FDB);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_fdb_key_str2val(key_value, p_fdb));

    for(i=1; i<TBL_FDB_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_fdb_field_value_parser( array[j++], i, p_fdb));
    }

    return PM_E_NONE;
}

int32
tbl_fdb_dump_one(tbl_fdb_t *p_fdb, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_FDB);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_FDB_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_FDB_FLD_KEY].name, 
            cdb_fdb_key_val2str(buf, MAX_CMD_STR_LEN, &p_fdb->key));
    }
    if (FLD_MATCH(TBL_FDB_FLD_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FDB_FLD_IFINDEX].name,
            p_fdb->ifindex);
    }
    if (FLD_MATCH(TBL_FDB_FLD_STATIC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FDB_FLD_STATIC].name,
            GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC));
    }
    if (FLD_MATCH(TBL_FDB_FLD_SECURITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FDB_FLD_SECURITY].name,
            GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN));
    }
    if (FLD_MATCH(TBL_FDB_FLD_HYBRID_FLOW, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FDB_FLD_HYBRID_FLOW].name,
            GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_HYBRID_FLOW));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_fdb_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_fdb_master_t *p_master = _g_p_tbl_fdb_master;
    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    ctclib_hash_iterate2(p_master->fdb_hash, tbl_hash_iter_adpt_fn, fn, pargs);
    return PM_E_NONE;
}

tbl_fdb_master_t*
tbl_fdb_get_master()
{
    return _g_p_tbl_fdb_master;
}

tbl_fdb_master_t*
tbl_fdb_init_fdb()
{
    _g_p_tbl_fdb_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_fdb_master_t));
    _g_p_tbl_fdb_master->fdb_hash = ctclib_hash_create(_tbl_fdb_hash_make, _tbl_fdb_hash_cmp);
    return _g_p_tbl_fdb_master;
}

