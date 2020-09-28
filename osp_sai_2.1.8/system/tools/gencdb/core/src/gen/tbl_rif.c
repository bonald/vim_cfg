
#include "proto.h"
#include "gen/tbl_rif_define.h"
#include "gen/tbl_rif.h"
#include "cdb_data_cmp.h"

tbl_rif_master_t *_g_p_tbl_rif_master = NULL;

static uint32
_tbl_rif_hash_make(tbl_rif_t *p_rif)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_rif->key;
    for (index = 0; index < sizeof(p_rif->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_rif_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_rif_t *p_rif1 = (tbl_rif_t*)p_arg1;
    tbl_rif_t *p_rif2 = (tbl_rif_t*)p_arg2;

    if (0 == sal_memcmp(&p_rif1->key, &p_rif2->key, sizeof(tbl_rif_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_rif_add_rif_sync(tbl_rif_t *p_rif, uint32 sync)
{
    tbl_rif_master_t *p_master = _g_p_tbl_rif_master;
    tbl_rif_t *p_db_rif = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_rif_get_rif(&p_rif->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_rif = XCALLOC(MEM_TBL_RIF, sizeof(tbl_rif_t));
    if (NULL == p_db_rif)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_rif, p_rif, sizeof(tbl_rif_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->rif_hash, (void*)p_db_rif, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->if_list, p_db_rif);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_RIF, p_db_rif);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_RIF, p_db_rif);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_rif_del_rif_sync(tbl_rif_key_t *p_rif_key, uint32 sync)
{
    tbl_rif_master_t *p_master = _g_p_tbl_rif_master;
    tbl_rif_t *p_db_rif = NULL;

    /* 1. lookup entry exist */
    p_db_rif = tbl_rif_get_rif(p_rif_key);
    if (NULL == p_db_rif)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_RIF, p_db_rif);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_RIF, p_db_rif);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->rif_hash, (void*)p_db_rif);
    ctclib_slistnode_delete(p_master->if_list, p_db_rif);

    /* 4. free db entry */
    XFREE(MEM_TBL_RIF, p_db_rif);

    return PM_E_NONE;
}

int32
tbl_rif_set_rif_field_sync(tbl_rif_t *p_rif, tbl_rif_field_id_t field_id, uint32 sync)
{
    tbl_rif_t *p_db_rif = NULL;

    /* 1. lookup entry exist */
    p_db_rif = tbl_rif_get_rif(&p_rif->key);
    if (NULL == p_db_rif)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_RIF_FLD_RIF_ID:
        p_db_rif->rif_id = p_rif->rif_id;
        break;

    case TBL_RIF_FLD_HOSTIF_ID:
        p_db_rif->hostif_id = p_rif->hostif_id;
        break;

    case TBL_RIF_FLD_MAX:
        sal_memcpy(p_db_rif, p_rif, sizeof(tbl_rif_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_RIF, field_id, p_db_rif);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_RIF, field_id, p_db_rif);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_rif_add_rif(tbl_rif_t *p_rif)
{
    return tbl_rif_add_rif_sync(p_rif, TRUE);
}

int32
tbl_rif_del_rif(tbl_rif_key_t *p_rif_key)
{
    return tbl_rif_del_rif_sync(p_rif_key, TRUE);
}

int32
tbl_rif_set_rif_field(tbl_rif_t *p_rif, tbl_rif_field_id_t field_id)
{
    return tbl_rif_set_rif_field_sync(p_rif, field_id, TRUE);
}

tbl_rif_t*
tbl_rif_get_rif(tbl_rif_key_t *p_rif_key)
{
    tbl_rif_master_t *p_master = _g_p_tbl_rif_master;
    tbl_rif_t lkp;

    sal_memcpy(&lkp.key, p_rif_key, sizeof(tbl_rif_key_t));
    return ctclib_hash_lookup(p_master->rif_hash, &lkp);
}

char*
tbl_rif_key_val2str(tbl_rif_t *p_rif, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_rif->key.name);
    return str;
}

int32
tbl_rif_key_str2val(char *str, tbl_rif_t *p_rif)
{
    int32 ret = 0;

    sal_strcpy(p_rif->key.name, str);
    return ret;
}

char*
tbl_rif_key_name_dump(tbl_rif_t *p_rif, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_RIF);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_rif->key.name);
    return str;
}

char*
tbl_rif_key_value_dump(tbl_rif_t *p_rif, char *str)
{
    sal_sprintf(str, "%s", p_rif->key.name);
    return str;
}

char*
tbl_rif_field_name_dump(tbl_rif_t *p_rif, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_RIF);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_RIF_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_rif_field_value_dump(tbl_rif_t *p_rif, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_RIF_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_rif->key.name);
    }
    if (FLD_MATCH(TBL_RIF_FLD_RIF_ID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rif->rif_id);
    }
    if (FLD_MATCH(TBL_RIF_FLD_HOSTIF_ID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_rif->hostif_id);
    }
    return str;
}

char**
tbl_rif_table_dump(tbl_rif_t *p_rif, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_rif_key_name_dump(p_rif, buf));
    for(i=1; i<TBL_RIF_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_rif_field_name_dump(p_rif, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_rif_field_value_dump(p_rif, i, buf));
    }
    return str;
}

int32
tbl_rif_field_value_parser(char *str, int32 field_id, tbl_rif_t *p_rif)
{
    if (FLD_MATCH(TBL_RIF_FLD_KEY, field_id))
    {
        sal_strcpy(p_rif->key.name, str);
    }
    if (FLD_MATCH(TBL_RIF_FLD_RIF_ID, field_id))
    {
        int32 ret;
        p_rif->rif_id = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RIF_FLD_HOSTIF_ID, field_id))
    {
        int32 ret;
        p_rif->hostif_id = cdb_uint64_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_rif_table_parser(char** array, char* key_value,tbl_rif_t *p_rif)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_RIF);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_rif_key_str2val(key_value, p_rif));

    for(i=1; i<TBL_RIF_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_rif_field_value_parser( array[j++], i, p_rif));
    }

    return PM_E_NONE;
}

int32
tbl_rif_dump_one(tbl_rif_t *p_rif, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_RIF);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_RIF_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_RIF_FLD_KEY].name,
            p_rif->key.name);
    }
    if (FLD_MATCH(TBL_RIF_FLD_RIF_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RIF_FLD_RIF_ID].name,
            p_rif->rif_id);
    }
    if (FLD_MATCH(TBL_RIF_FLD_HOSTIF_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_RIF_FLD_HOSTIF_ID].name,
            p_rif->hostif_id);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_rif_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_rif_master_t *p_master = _g_p_tbl_rif_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_rif_t *p_db_rif = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->if_list, p_db_rif, listnode, next)
    {
        rc |= fn(p_db_rif, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_rif_master_t*
tbl_rif_get_master()
{
    return _g_p_tbl_rif_master;
}

tbl_rif_master_t*
tbl_rif_init_rif()
{
    _g_p_tbl_rif_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_rif_master_t));
    _g_p_tbl_rif_master->rif_hash = ctclib_hash_create(_tbl_rif_hash_make, _tbl_rif_hash_cmp);
    _g_p_tbl_rif_master->if_list = ctclib_slist_create(tbl_rif_cmp, NULL);
    return _g_p_tbl_rif_master;
}

