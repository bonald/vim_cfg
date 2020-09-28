
#include "proto.h"
#include "gen/tbl_l2edit_define.h"
#include "gen/tbl_l2edit.h"
#include "cdb_data_cmp.h"

tbl_l2edit_master_t *_g_p_tbl_l2edit_master = NULL;

static uint32
_tbl_l2edit_hash_make(tbl_l2edit_t *p_l2edit)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_l2edit->key;
    for (index = 0; index < sizeof(p_l2edit->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_l2edit_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_l2edit_t *p_l2edit1 = (tbl_l2edit_t*)p_arg1;
    tbl_l2edit_t *p_l2edit2 = (tbl_l2edit_t*)p_arg2;

    if (0 == sal_memcmp(&p_l2edit1->key, &p_l2edit2->key, sizeof(tbl_l2edit_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_l2edit_add_l2edit_sync(tbl_l2edit_t *p_l2edit, uint32 sync)
{
    tbl_l2edit_master_t *p_master = _g_p_tbl_l2edit_master;
    tbl_l2edit_t *p_db_l2edit = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_l2edit_get_l2edit(&p_l2edit->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_l2edit = XCALLOC(MEM_TBL_L2EDIT, sizeof(tbl_l2edit_t));
    if (NULL == p_db_l2edit)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_l2edit, p_l2edit, sizeof(tbl_l2edit_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->l2edit_hash, (void*)p_db_l2edit, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_L2EDIT, p_db_l2edit);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_L2EDIT, p_db_l2edit);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_l2edit_del_l2edit_sync(tbl_l2edit_key_t *p_l2edit_key, uint32 sync)
{
    tbl_l2edit_master_t *p_master = _g_p_tbl_l2edit_master;
    tbl_l2edit_t *p_db_l2edit = NULL;

    /* 1. lookup entry exist */
    p_db_l2edit = tbl_l2edit_get_l2edit(p_l2edit_key);
    if (NULL == p_db_l2edit)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_L2EDIT, p_db_l2edit);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_L2EDIT, p_db_l2edit);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->l2edit_hash, (void*)p_db_l2edit);

    /* 4. free db entry */
    XFREE(MEM_TBL_L2EDIT, p_db_l2edit);

    return PM_E_NONE;
}

int32
tbl_l2edit_set_l2edit_field_sync(tbl_l2edit_t *p_l2edit, tbl_l2edit_field_id_t field_id, uint32 sync)
{
    tbl_l2edit_t *p_db_l2edit = NULL;

    /* 1. lookup entry exist */
    p_db_l2edit = tbl_l2edit_get_l2edit(&p_l2edit->key);
    if (NULL == p_db_l2edit)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_L2EDIT_FLD_COUNT:
        p_db_l2edit->count = p_l2edit->count;
        break;

    case TBL_L2EDIT_FLD_MAX:
        sal_memcpy(p_db_l2edit, p_l2edit, sizeof(tbl_l2edit_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_L2EDIT, field_id, p_db_l2edit);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_L2EDIT, field_id, p_db_l2edit);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_l2edit_add_l2edit(tbl_l2edit_t *p_l2edit)
{
    return tbl_l2edit_add_l2edit_sync(p_l2edit, TRUE);
}

int32
tbl_l2edit_del_l2edit(tbl_l2edit_key_t *p_l2edit_key)
{
    return tbl_l2edit_del_l2edit_sync(p_l2edit_key, TRUE);
}

int32
tbl_l2edit_set_l2edit_field(tbl_l2edit_t *p_l2edit, tbl_l2edit_field_id_t field_id)
{
    return tbl_l2edit_set_l2edit_field_sync(p_l2edit, field_id, TRUE);
}

tbl_l2edit_t*
tbl_l2edit_get_l2edit(tbl_l2edit_key_t *p_l2edit_key)
{
    tbl_l2edit_master_t *p_master = _g_p_tbl_l2edit_master;
    tbl_l2edit_t lkp;

    sal_memcpy(&lkp.key, p_l2edit_key, sizeof(tbl_l2edit_key_t));
    return ctclib_hash_lookup(p_master->l2edit_hash, &lkp);
}

char*
tbl_l2edit_key_val2str(tbl_l2edit_t *p_l2edit, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_l2edit_key_val2str(buf, MAX_CMD_STR_LEN, &p_l2edit->key));
    return str;
}

int32
tbl_l2edit_key_str2val(char *str, tbl_l2edit_t *p_l2edit)
{
    int32 ret = 0;

    ret = cdb_l2edit_key_str2val(str, &p_l2edit->key);
    return ret;
}

char*
tbl_l2edit_key_name_dump(tbl_l2edit_t *p_l2edit, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_L2EDIT);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_l2edit_key_val2str(buf, MAX_CMD_STR_LEN, &p_l2edit->key));
    return str;
}

char*
tbl_l2edit_key_value_dump(tbl_l2edit_t *p_l2edit, char *str)
{

    cdb_l2edit_key_val2str(str, MAX_CMD_STR_LEN, &p_l2edit->key);
    return str;
}

char*
tbl_l2edit_field_name_dump(tbl_l2edit_t *p_l2edit, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_L2EDIT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_L2EDIT_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_l2edit_field_value_dump(tbl_l2edit_t *p_l2edit, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_L2EDIT_FLD_KEY, field_id))
    {
        cdb_l2edit_key_val2str(str, MAX_CMD_STR_LEN, &p_l2edit->key);
    }
    if (FLD_MATCH(TBL_L2EDIT_FLD_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_l2edit->count);
    }
    return str;
}

char**
tbl_l2edit_table_dump(tbl_l2edit_t *p_l2edit, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_l2edit_key_name_dump(p_l2edit, buf));
    for(i=1; i<TBL_L2EDIT_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_l2edit_field_name_dump(p_l2edit, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_l2edit_field_value_dump(p_l2edit, i, buf));
    }
    return str;
}

int32
tbl_l2edit_field_value_parser(char *str, int32 field_id, tbl_l2edit_t *p_l2edit)
{
    if (FLD_MATCH(TBL_L2EDIT_FLD_KEY, field_id))
    {
        cdb_l2edit_key_str2val(str, &p_l2edit->key);
    }
    if (FLD_MATCH(TBL_L2EDIT_FLD_COUNT, field_id))
    {
        int32 ret;
        p_l2edit->count = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_l2edit_table_parser(char** array, char* key_value,tbl_l2edit_t *p_l2edit)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_L2EDIT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_l2edit_key_str2val(key_value, p_l2edit));

    for(i=1; i<TBL_L2EDIT_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_l2edit_field_value_parser( array[j++], i, p_l2edit));
    }

    return PM_E_NONE;
}

int32
tbl_l2edit_dump_one(tbl_l2edit_t *p_l2edit, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_L2EDIT);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_L2EDIT_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_L2EDIT_FLD_KEY].name, 
            cdb_l2edit_key_val2str(buf, MAX_CMD_STR_LEN, &p_l2edit->key));
    }
    if (FLD_MATCH(TBL_L2EDIT_FLD_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_L2EDIT_FLD_COUNT].name,
            p_l2edit->count);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_l2edit_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_l2edit_master_t *p_master = _g_p_tbl_l2edit_master;
    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    ctclib_hash_iterate2(p_master->l2edit_hash, tbl_hash_iter_adpt_fn, fn, pargs);
    return PM_E_NONE;
}

tbl_l2edit_master_t*
tbl_l2edit_get_master()
{
    return _g_p_tbl_l2edit_master;
}

tbl_l2edit_master_t*
tbl_l2edit_init_l2edit()
{
    _g_p_tbl_l2edit_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_l2edit_master_t));
    _g_p_tbl_l2edit_master->l2edit_hash = ctclib_hash_create(_tbl_l2edit_hash_make, _tbl_l2edit_hash_cmp);
    return _g_p_tbl_l2edit_master;
}

