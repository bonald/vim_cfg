
#include "proto.h"
#include "gen/tbl_pim_dm_define.h"
#include "gen/tbl_pim_dm.h"
#include "cdb_data_cmp.h"

tbl_pim_dm_master_t *_g_p_tbl_pim_dm_master = NULL;

static uint32
_tbl_pim_dm_hash_make(tbl_pim_dm_t *p_pim_dm)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_pim_dm->key;
    for (index = 0; index < sizeof(p_pim_dm->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_pim_dm_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_pim_dm_t *p_pim_dm1 = (tbl_pim_dm_t*)p_arg1;
    tbl_pim_dm_t *p_pim_dm2 = (tbl_pim_dm_t*)p_arg2;

    if (0 == sal_memcmp(&p_pim_dm1->key, &p_pim_dm2->key, sizeof(tbl_pim_dm_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_pim_dm_add_pim_dm_sync(tbl_pim_dm_t *p_pim_dm, uint32 sync)
{
    tbl_pim_dm_master_t *p_master = _g_p_tbl_pim_dm_master;
    tbl_pim_dm_t *p_db_pim_dm = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_pim_dm_get_pim_dm(&p_pim_dm->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_pim_dm = XCALLOC(MEM_TBL_PIM_DM, sizeof(tbl_pim_dm_t));
    if (NULL == p_db_pim_dm)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_pim_dm, p_pim_dm, sizeof(tbl_pim_dm_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->pim_dm_hash, (void*)p_db_pim_dm, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->pim_dm_list, p_db_pim_dm);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_PIM_DM, p_db_pim_dm);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_PIM_DM, p_db_pim_dm);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_pim_dm_del_pim_dm_sync(tbl_pim_dm_key_t *p_pim_dm_key, uint32 sync)
{
    tbl_pim_dm_master_t *p_master = _g_p_tbl_pim_dm_master;
    tbl_pim_dm_t *p_db_pim_dm = NULL;

    /* 1. lookup entry exist */
    p_db_pim_dm = tbl_pim_dm_get_pim_dm(p_pim_dm_key);
    if (NULL == p_db_pim_dm)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_PIM_DM, p_db_pim_dm);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_PIM_DM, p_db_pim_dm);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->pim_dm_hash, (void*)p_db_pim_dm);
    ctclib_slistnode_delete(p_master->pim_dm_list, p_db_pim_dm);

    /* 4. free db entry */
    XFREE(MEM_TBL_PIM_DM, p_db_pim_dm);

    return PM_E_NONE;
}

int32
tbl_pim_dm_set_pim_dm_field_sync(tbl_pim_dm_t *p_pim_dm, tbl_pim_dm_field_id_t field_id, uint32 sync)
{
    tbl_pim_dm_t *p_db_pim_dm = NULL;

    /* 1. lookup entry exist */
    p_db_pim_dm = tbl_pim_dm_get_pim_dm(&p_pim_dm->key);
    if (NULL == p_db_pim_dm)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_PIM_DM_FLD_MAX:
        sal_memcpy(p_db_pim_dm, p_pim_dm, sizeof(tbl_pim_dm_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_PIM_DM, field_id, p_db_pim_dm);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_PIM_DM, field_id, p_db_pim_dm);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_pim_dm_add_pim_dm(tbl_pim_dm_t *p_pim_dm)
{
    return tbl_pim_dm_add_pim_dm_sync(p_pim_dm, TRUE);
}

int32
tbl_pim_dm_del_pim_dm(tbl_pim_dm_key_t *p_pim_dm_key)
{
    return tbl_pim_dm_del_pim_dm_sync(p_pim_dm_key, TRUE);
}

int32
tbl_pim_dm_set_pim_dm_field(tbl_pim_dm_t *p_pim_dm, tbl_pim_dm_field_id_t field_id)
{
    return tbl_pim_dm_set_pim_dm_field_sync(p_pim_dm, field_id, TRUE);
}

tbl_pim_dm_t*
tbl_pim_dm_get_pim_dm(tbl_pim_dm_key_t *p_pim_dm_key)
{
    tbl_pim_dm_master_t *p_master = _g_p_tbl_pim_dm_master;
    tbl_pim_dm_t lkp;

    sal_memcpy(&lkp.key, p_pim_dm_key, sizeof(tbl_pim_dm_key_t));
    return ctclib_hash_lookup(p_master->pim_dm_hash, &lkp);
}

char*
tbl_pim_dm_key_val2str(tbl_pim_dm_t *p_pim_dm, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_pim_dm->key.id);
    return str;
}

int32
tbl_pim_dm_key_str2val(char *str, tbl_pim_dm_t *p_pim_dm)
{
    int32 ret = 0;

    p_pim_dm->key.id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_pim_dm_key_name_dump(tbl_pim_dm_t *p_pim_dm, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_PIM_DM);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_pim_dm->key.id);
    return str;
}

char*
tbl_pim_dm_key_value_dump(tbl_pim_dm_t *p_pim_dm, char *str)
{
    sal_sprintf(str, "%u", p_pim_dm->key.id);
    return str;
}

char*
tbl_pim_dm_field_name_dump(tbl_pim_dm_t *p_pim_dm, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_PIM_DM);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_PIM_DM_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_pim_dm_field_value_dump(tbl_pim_dm_t *p_pim_dm, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_PIM_DM_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_pim_dm->key.id);
    }
    return str;
}

char**
tbl_pim_dm_table_dump(tbl_pim_dm_t *p_pim_dm, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_pim_dm_key_name_dump(p_pim_dm, buf));
    for(i=1; i<TBL_PIM_DM_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_pim_dm_field_name_dump(p_pim_dm, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_pim_dm_field_value_dump(p_pim_dm, i, buf));
    }
    return str;
}

int32
tbl_pim_dm_field_value_parser(char *str, int32 field_id, tbl_pim_dm_t *p_pim_dm)
{
    if (FLD_MATCH(TBL_PIM_DM_FLD_KEY, field_id))
    {
        int32 ret;
        p_pim_dm->key.id = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_pim_dm_table_parser(char** array, char* key_value,tbl_pim_dm_t *p_pim_dm)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_PIM_DM);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_pim_dm_key_str2val(key_value, p_pim_dm));

    for(i=1; i<TBL_PIM_DM_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_pim_dm_field_value_parser( array[j++], i, p_pim_dm));
    }

    return PM_E_NONE;
}

int32
tbl_pim_dm_dump_one(tbl_pim_dm_t *p_pim_dm, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_PIM_DM);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_PIM_DM_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PIM_DM_FLD_KEY].name,
            p_pim_dm->key.id);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_pim_dm_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_pim_dm_master_t *p_master = _g_p_tbl_pim_dm_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_pim_dm_t *p_db_pim_dm = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->pim_dm_list, p_db_pim_dm, listnode, next)
    {
        rc |= fn(p_db_pim_dm, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_pim_dm_master_t*
tbl_pim_dm_get_master()
{
    return _g_p_tbl_pim_dm_master;
}

tbl_pim_dm_master_t*
tbl_pim_dm_init_pim_dm()
{
    _g_p_tbl_pim_dm_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_pim_dm_master_t));
    _g_p_tbl_pim_dm_master->pim_dm_hash = ctclib_hash_create(_tbl_pim_dm_hash_make, _tbl_pim_dm_hash_cmp);
    _g_p_tbl_pim_dm_master->pim_dm_list = ctclib_slist_create(tbl_pim_dm_cmp, NULL);
    return _g_p_tbl_pim_dm_master;
}

