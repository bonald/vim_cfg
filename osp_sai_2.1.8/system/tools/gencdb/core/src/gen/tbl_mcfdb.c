
#include "proto.h"
#include "gen/tbl_mcfdb_define.h"
#include "gen/tbl_mcfdb.h"
#include "cdb_data_cmp.h"

tbl_mcfdb_master_t *_g_p_tbl_mcfdb_master = NULL;

static uint32
_tbl_mcfdb_hash_make(tbl_mcfdb_t *p_mcfdb)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_mcfdb->key;
    for (index = 0; index < sizeof(p_mcfdb->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_mcfdb_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_mcfdb_t *p_mcfdb1 = (tbl_mcfdb_t*)p_arg1;
    tbl_mcfdb_t *p_mcfdb2 = (tbl_mcfdb_t*)p_arg2;

    if (0 == sal_memcmp(&p_mcfdb1->key, &p_mcfdb2->key, sizeof(tbl_mcfdb_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_mcfdb_add_mcfdb_sync(tbl_mcfdb_t *p_mcfdb, uint32 sync)
{
    tbl_mcfdb_master_t *p_master = _g_p_tbl_mcfdb_master;
    tbl_mcfdb_t *p_db_mcfdb = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_mcfdb_get_mcfdb(&p_mcfdb->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_mcfdb = XCALLOC(MEM_TBL_MCFDB, sizeof(tbl_mcfdb_t));
    if (NULL == p_db_mcfdb)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_mcfdb, p_mcfdb, sizeof(tbl_mcfdb_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->mcfdb_hash, (void*)p_db_mcfdb, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_MCFDB, p_db_mcfdb);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_MCFDB, p_db_mcfdb);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_mcfdb_del_mcfdb_sync(tbl_mcfdb_key_t *p_mcfdb_key, uint32 sync)
{
    tbl_mcfdb_master_t *p_master = _g_p_tbl_mcfdb_master;
    tbl_mcfdb_t *p_db_mcfdb = NULL;

    /* 1. lookup entry exist */
    p_db_mcfdb = tbl_mcfdb_get_mcfdb(p_mcfdb_key);
    if (NULL == p_db_mcfdb)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_MCFDB, p_db_mcfdb);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_MCFDB, p_db_mcfdb);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->mcfdb_hash, (void*)p_db_mcfdb);

    /* 4. free db entry */
    XFREE(MEM_TBL_MCFDB, p_db_mcfdb);

    return PM_E_NONE;
}

int32
tbl_mcfdb_set_mcfdb_field_sync(tbl_mcfdb_t *p_mcfdb, tbl_mcfdb_field_id_t field_id, uint32 sync)
{
    tbl_mcfdb_t *p_db_mcfdb = NULL;

    /* 1. lookup entry exist */
    p_db_mcfdb = tbl_mcfdb_get_mcfdb(&p_mcfdb->key);
    if (NULL == p_db_mcfdb)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_MCFDB_FLD_L2MC_PORT:
        sal_memcpy(p_db_mcfdb->l2mc_port, p_mcfdb->l2mc_port, sizeof(p_mcfdb->l2mc_port));
        break;

    case TBL_MCFDB_FLD_ACTION_IFINDEX:
        p_db_mcfdb->action_ifindex = p_mcfdb->action_ifindex;
        break;

    case TBL_MCFDB_FLD_MANUAL:
        p_db_mcfdb->manual = p_mcfdb->manual;
        break;

    case TBL_MCFDB_FLD_MAX:
        sal_memcpy(p_db_mcfdb, p_mcfdb, sizeof(tbl_mcfdb_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_MCFDB, field_id, p_db_mcfdb);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_MCFDB, field_id, p_db_mcfdb);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_mcfdb_add_mcfdb(tbl_mcfdb_t *p_mcfdb)
{
    return tbl_mcfdb_add_mcfdb_sync(p_mcfdb, TRUE);
}

int32
tbl_mcfdb_del_mcfdb(tbl_mcfdb_key_t *p_mcfdb_key)
{
    return tbl_mcfdb_del_mcfdb_sync(p_mcfdb_key, TRUE);
}

int32
tbl_mcfdb_set_mcfdb_field(tbl_mcfdb_t *p_mcfdb, tbl_mcfdb_field_id_t field_id)
{
    return tbl_mcfdb_set_mcfdb_field_sync(p_mcfdb, field_id, TRUE);
}

tbl_mcfdb_t*
tbl_mcfdb_get_mcfdb(tbl_mcfdb_key_t *p_mcfdb_key)
{
    tbl_mcfdb_master_t *p_master = _g_p_tbl_mcfdb_master;
    tbl_mcfdb_t lkp;

    sal_memcpy(&lkp.key, p_mcfdb_key, sizeof(tbl_mcfdb_key_t));
    return ctclib_hash_lookup(p_master->mcfdb_hash, &lkp);
}

char*
tbl_mcfdb_key_val2str(tbl_mcfdb_t *p_mcfdb, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_fdb_key_val2str(buf, MAX_CMD_STR_LEN, &p_mcfdb->key));
    return str;
}

int32
tbl_mcfdb_key_str2val(char *str, tbl_mcfdb_t *p_mcfdb)
{
    int32 ret = 0;

    char err[MAX_CMD_STR_LEN];
    ret = cdb_fdb_key_str2val(&p_mcfdb->key, str, err);
    return ret;
}

char*
tbl_mcfdb_key_name_dump(tbl_mcfdb_t *p_mcfdb, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_MCFDB);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_fdb_key_val2str(buf, MAX_CMD_STR_LEN, &p_mcfdb->key));
    return str;
}

char*
tbl_mcfdb_key_value_dump(tbl_mcfdb_t *p_mcfdb, char *str)
{

    cdb_fdb_key_val2str(str, MAX_CMD_STR_LEN, &p_mcfdb->key);
    return str;
}

char*
tbl_mcfdb_field_name_dump(tbl_mcfdb_t *p_mcfdb, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_MCFDB);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_MCFDB_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_mcfdb_field_value_dump(tbl_mcfdb_t *p_mcfdb, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_MCFDB_FLD_KEY, field_id))
    {
        cdb_fdb_key_val2str(str, MAX_CMD_STR_LEN, &p_mcfdb->key);
    }
    if (FLD_MATCH(TBL_MCFDB_FLD_L2MC_PORT, field_id))
    {
        cdb_bitmap_val2str(str, MAX_CMD_STR_LEN, p_mcfdb->l2mc_port, sizeof(p_mcfdb->l2mc_port)/4);
    }
    if (FLD_MATCH(TBL_MCFDB_FLD_ACTION_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_mcfdb->action_ifindex);
    }
    if (FLD_MATCH(TBL_MCFDB_FLD_MANUAL, field_id))
    {
        sal_sprintf(str, "%u", p_mcfdb->manual);
    }
    return str;
}

char**
tbl_mcfdb_table_dump(tbl_mcfdb_t *p_mcfdb, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_mcfdb_key_name_dump(p_mcfdb, buf));
    for(i=1; i<TBL_MCFDB_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_mcfdb_field_name_dump(p_mcfdb, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_mcfdb_field_value_dump(p_mcfdb, i, buf));
    }
    return str;
}

int32
tbl_mcfdb_field_value_parser(char *str, int32 field_id, tbl_mcfdb_t *p_mcfdb)
{
    if (FLD_MATCH(TBL_MCFDB_FLD_KEY, field_id))
    {
        char err[MAX_CMD_STR_LEN];
        cdb_fdb_key_str2val(&p_mcfdb->key, str, err);
    }
    if (FLD_MATCH(TBL_MCFDB_FLD_L2MC_PORT, field_id))
    {
        char err[MAX_CMD_STR_LEN];
        cdb_bitmap_str2val(str, 0, GLB_UINT16_BMP_WORD_MAX, p_mcfdb->l2mc_port, sizeof(p_mcfdb->l2mc_port)/4, err);
    }
    if (FLD_MATCH(TBL_MCFDB_FLD_ACTION_IFINDEX, field_id))
    {
        int32 ret;
        p_mcfdb->action_ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MCFDB_FLD_MANUAL, field_id))
    {
        int32 ret;
        p_mcfdb->manual = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_mcfdb_table_parser(char** array, char* key_value,tbl_mcfdb_t *p_mcfdb)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_MCFDB);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_mcfdb_key_str2val(key_value, p_mcfdb));

    for(i=1; i<TBL_MCFDB_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_mcfdb_field_value_parser( array[j++], i, p_mcfdb));
    }

    return PM_E_NONE;
}

int32
tbl_mcfdb_dump_one(tbl_mcfdb_t *p_mcfdb, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_MCFDB);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_MCFDB_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MCFDB_FLD_KEY].name, 
            cdb_fdb_key_val2str(buf, MAX_CMD_STR_LEN, &p_mcfdb->key));
    }
    if (FLD_MATCH(TBL_MCFDB_FLD_L2MC_PORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MCFDB_FLD_L2MC_PORT].name, 
            cdb_bitmap_val2str(buf, MAX_CMD_STR_LEN, p_mcfdb->l2mc_port, sizeof(p_mcfdb->l2mc_port)/4));
    }
    if (FLD_MATCH(TBL_MCFDB_FLD_ACTION_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MCFDB_FLD_ACTION_IFINDEX].name,
            p_mcfdb->action_ifindex);
    }
    if (FLD_MATCH(TBL_MCFDB_FLD_MANUAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MCFDB_FLD_MANUAL].name,
            p_mcfdb->manual);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_mcfdb_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_mcfdb_master_t *p_master = _g_p_tbl_mcfdb_master;
    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    ctclib_hash_iterate2(p_master->mcfdb_hash, tbl_hash_iter_adpt_fn, fn, pargs);
    return PM_E_NONE;
}

tbl_mcfdb_master_t*
tbl_mcfdb_get_master()
{
    return _g_p_tbl_mcfdb_master;
}

tbl_mcfdb_master_t*
tbl_mcfdb_init_mcfdb()
{
    _g_p_tbl_mcfdb_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_mcfdb_master_t));
    _g_p_tbl_mcfdb_master->mcfdb_hash = ctclib_hash_create(_tbl_mcfdb_hash_make, _tbl_mcfdb_hash_cmp);
    return _g_p_tbl_mcfdb_master;
}

