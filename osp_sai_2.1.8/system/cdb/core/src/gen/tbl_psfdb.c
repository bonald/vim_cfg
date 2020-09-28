
#include "proto.h"
#include "gen/tbl_psfdb_define.h"
#include "gen/tbl_psfdb.h"
#include "cdb_data_cmp.h"

tbl_psfdb_master_t *_g_p_tbl_psfdb_master = NULL;

static uint32
_tbl_psfdb_hash_make(tbl_psfdb_t *p_psfdb)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_psfdb->key;
    for (index = 0; index < sizeof(p_psfdb->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_psfdb_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_psfdb_t *p_psfdb1 = (tbl_psfdb_t*)p_arg1;
    tbl_psfdb_t *p_psfdb2 = (tbl_psfdb_t*)p_arg2;

    if (0 == sal_memcmp(&p_psfdb1->key, &p_psfdb2->key, sizeof(tbl_psfdb_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_psfdb_add_psfdb_sync(tbl_psfdb_t *p_psfdb, uint32 sync)
{
    tbl_psfdb_master_t *p_master = _g_p_tbl_psfdb_master;
    tbl_psfdb_t *p_db_psfdb = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_psfdb_get_psfdb(&p_psfdb->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_psfdb = XCALLOC(MEM_TBL_PSFDB, sizeof(tbl_psfdb_t));
    if (NULL == p_db_psfdb)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_psfdb, p_psfdb, sizeof(tbl_psfdb_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->psfdb_hash, (void*)p_db_psfdb, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_PSFDB, p_db_psfdb);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_PSFDB, p_db_psfdb);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_psfdb_del_psfdb_sync(tbl_psfdb_key_t *p_psfdb_key, uint32 sync)
{
    tbl_psfdb_master_t *p_master = _g_p_tbl_psfdb_master;
    tbl_psfdb_t *p_db_psfdb = NULL;

    /* 1. lookup entry exist */
    p_db_psfdb = tbl_psfdb_get_psfdb(p_psfdb_key);
    if (NULL == p_db_psfdb)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_PSFDB, p_db_psfdb);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_PSFDB, p_db_psfdb);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->psfdb_hash, (void*)p_db_psfdb);

    /* 4. free db entry */
    XFREE(MEM_TBL_PSFDB, p_db_psfdb);

    return PM_E_NONE;
}

int32
tbl_psfdb_set_psfdb_field_sync(tbl_psfdb_t *p_psfdb, tbl_psfdb_field_id_t field_id, uint32 sync)
{
    tbl_psfdb_t *p_db_psfdb = NULL;

    /* 1. lookup entry exist */
    p_db_psfdb = tbl_psfdb_get_psfdb(&p_psfdb->key);
    if (NULL == p_db_psfdb)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_PSFDB_FLD_IFINDEX:
        p_db_psfdb->ifindex = p_psfdb->ifindex;
        break;

    case TBL_PSFDB_FLD_PORT_SECURITY:
        CDB_FLAG_CHECK_SET(p_db_psfdb, p_psfdb, flags, GLB_FDB_FLAG_MAC_SECURITY_EN)
        break;

    case TBL_PSFDB_FLD_MAX:
        sal_memcpy(p_db_psfdb, p_psfdb, sizeof(tbl_psfdb_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_PSFDB, field_id, p_db_psfdb);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_PSFDB, field_id, p_db_psfdb);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_psfdb_add_psfdb(tbl_psfdb_t *p_psfdb)
{
    return tbl_psfdb_add_psfdb_sync(p_psfdb, TRUE);
}

int32
tbl_psfdb_del_psfdb(tbl_psfdb_key_t *p_psfdb_key)
{
    return tbl_psfdb_del_psfdb_sync(p_psfdb_key, TRUE);
}

int32
tbl_psfdb_set_psfdb_field(tbl_psfdb_t *p_psfdb, tbl_psfdb_field_id_t field_id)
{
    return tbl_psfdb_set_psfdb_field_sync(p_psfdb, field_id, TRUE);
}

tbl_psfdb_t*
tbl_psfdb_get_psfdb(tbl_psfdb_key_t *p_psfdb_key)
{
    tbl_psfdb_master_t *p_master = _g_p_tbl_psfdb_master;
    tbl_psfdb_t lkp;

    sal_memcpy(&lkp.key, p_psfdb_key, sizeof(tbl_psfdb_key_t));
    return ctclib_hash_lookup(p_master->psfdb_hash, &lkp);
}

char*
tbl_psfdb_key_val2str(tbl_psfdb_t *p_psfdb, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_fdb_key_val2str(buf, MAX_CMD_STR_LEN, &p_psfdb->key));
    return str;
}

int32
tbl_psfdb_key_str2val(char *str, tbl_psfdb_t *p_psfdb)
{
    int32 ret = 0;

    char err[MAX_CMD_STR_LEN];
    ret = cdb_fdb_key_str2val(&p_psfdb->key, str, err);
    return ret;
}

char*
tbl_psfdb_key_name_dump(tbl_psfdb_t *p_psfdb, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_PSFDB);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_fdb_key_val2str(buf, MAX_CMD_STR_LEN, &p_psfdb->key));
    return str;
}

char*
tbl_psfdb_key_value_dump(tbl_psfdb_t *p_psfdb, char *str)
{

    cdb_fdb_key_val2str(str, MAX_CMD_STR_LEN, &p_psfdb->key);
    return str;
}

char*
tbl_psfdb_field_name_dump(tbl_psfdb_t *p_psfdb, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_PSFDB);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_PSFDB_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_psfdb_field_value_dump(tbl_psfdb_t *p_psfdb, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_PSFDB_FLD_KEY, field_id))
    {
        cdb_fdb_key_val2str(str, MAX_CMD_STR_LEN, &p_psfdb->key);
    }
    if (FLD_MATCH(TBL_PSFDB_FLD_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_psfdb->ifindex);
    }
    if (FLD_MATCH(TBL_PSFDB_FLD_PORT_SECURITY, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_psfdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN));
    }
    return str;
}

char**
tbl_psfdb_table_dump(tbl_psfdb_t *p_psfdb, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_psfdb_key_name_dump(p_psfdb, buf));
    for(i=1; i<TBL_PSFDB_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_psfdb_field_name_dump(p_psfdb, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_psfdb_field_value_dump(p_psfdb, i, buf));
    }
    return str;
}

int32
tbl_psfdb_field_value_parser(char *str, int32 field_id, tbl_psfdb_t *p_psfdb)
{
    if (FLD_MATCH(TBL_PSFDB_FLD_KEY, field_id))
    {
        char err[MAX_CMD_STR_LEN];
        cdb_fdb_key_str2val(&p_psfdb->key, str, err);
    }
    if (FLD_MATCH(TBL_PSFDB_FLD_IFINDEX, field_id))
    {
        int32 ret;
        p_psfdb->ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PSFDB_FLD_PORT_SECURITY, field_id))
    {
        GLB_SET_FLAG(p_psfdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN);
    }
    return PM_E_NONE;
}

int32
tbl_psfdb_table_parser(char** array, char* key_value,tbl_psfdb_t *p_psfdb)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_PSFDB);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_psfdb_key_str2val(key_value, p_psfdb));

    for(i=1; i<TBL_PSFDB_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_psfdb_field_value_parser( array[j++], i, p_psfdb));
    }

    return PM_E_NONE;
}

int32
tbl_psfdb_dump_one(tbl_psfdb_t *p_psfdb, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_PSFDB);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_PSFDB_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PSFDB_FLD_KEY].name, 
            cdb_fdb_key_val2str(buf, MAX_CMD_STR_LEN, &p_psfdb->key));
    }
    if (FLD_MATCH(TBL_PSFDB_FLD_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PSFDB_FLD_IFINDEX].name,
            p_psfdb->ifindex);
    }
    if (FLD_MATCH(TBL_PSFDB_FLD_PORT_SECURITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PSFDB_FLD_PORT_SECURITY].name,
            GLB_FLAG_ISSET(p_psfdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_psfdb_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_psfdb_master_t *p_master = _g_p_tbl_psfdb_master;
    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    ctclib_hash_iterate2(p_master->psfdb_hash, tbl_hash_iter_adpt_fn, fn, pargs);
    return PM_E_NONE;
}

tbl_psfdb_master_t*
tbl_psfdb_get_master()
{
    return _g_p_tbl_psfdb_master;
}

tbl_psfdb_master_t*
tbl_psfdb_init_psfdb()
{
    _g_p_tbl_psfdb_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_psfdb_master_t));
    _g_p_tbl_psfdb_master->psfdb_hash = ctclib_hash_create(_tbl_psfdb_hash_make, _tbl_psfdb_hash_cmp);
    return _g_p_tbl_psfdb_master;
}

