
#include "proto.h"
#include "gen/tbl_qos_policer_res_define.h"
#include "gen/tbl_qos_policer_res.h"
#include "cdb_data_cmp.h"

tbl_qos_policer_res_master_t *_g_p_tbl_qos_policer_res_master = NULL;

static uint32
_tbl_qos_policer_res_hash_make(tbl_qos_policer_res_t *p_qos_policer_res)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_qos_policer_res->key;
    for (index = 0; index < sizeof(p_qos_policer_res->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_qos_policer_res_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_qos_policer_res_t *p_qos_policer_res1 = (tbl_qos_policer_res_t*)p_arg1;
    tbl_qos_policer_res_t *p_qos_policer_res2 = (tbl_qos_policer_res_t*)p_arg2;

    if (0 == sal_memcmp(&p_qos_policer_res1->key, &p_qos_policer_res2->key, sizeof(tbl_qos_policer_res_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_qos_policer_res_add_qos_policer_res_sync(tbl_qos_policer_res_t *p_qos_policer_res, uint32 sync)
{
    tbl_qos_policer_res_master_t *p_master = _g_p_tbl_qos_policer_res_master;
    tbl_qos_policer_res_t *p_db_qos_policer_res = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_qos_policer_res_get_qos_policer_res(&p_qos_policer_res->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_qos_policer_res = XCALLOC(MEM_TBL_QOS_POLICER_RES, sizeof(tbl_qos_policer_res_t));
    if (NULL == p_db_qos_policer_res)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_qos_policer_res, p_qos_policer_res, sizeof(tbl_qos_policer_res_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->qos_policer_res_hash, (void*)p_db_qos_policer_res, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_QOS_POLICER_RES, p_db_qos_policer_res);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_QOS_POLICER_RES, p_db_qos_policer_res);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_qos_policer_res_del_qos_policer_res_sync(tbl_qos_policer_res_key_t *p_qos_policer_res_key, uint32 sync)
{
    tbl_qos_policer_res_master_t *p_master = _g_p_tbl_qos_policer_res_master;
    tbl_qos_policer_res_t *p_db_qos_policer_res = NULL;

    /* 1. lookup entry exist */
    p_db_qos_policer_res = tbl_qos_policer_res_get_qos_policer_res(p_qos_policer_res_key);
    if (NULL == p_db_qos_policer_res)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_QOS_POLICER_RES, p_db_qos_policer_res);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_QOS_POLICER_RES, p_db_qos_policer_res);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->qos_policer_res_hash, (void*)p_db_qos_policer_res);

    /* 4. free db entry */
    XFREE(MEM_TBL_QOS_POLICER_RES, p_db_qos_policer_res);

    return PM_E_NONE;
}

int32
tbl_qos_policer_res_set_qos_policer_res_field_sync(tbl_qos_policer_res_t *p_qos_policer_res, tbl_qos_policer_res_field_id_t field_id, uint32 sync)
{
    tbl_qos_policer_res_t *p_db_qos_policer_res = NULL;

    /* 1. lookup entry exist */
    p_db_qos_policer_res = tbl_qos_policer_res_get_qos_policer_res(&p_qos_policer_res->key);
    if (NULL == p_db_qos_policer_res)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_QOS_POLICER_RES_FLD_REF_CNT:
        p_db_qos_policer_res->ref_cnt = p_qos_policer_res->ref_cnt;
        break;

    case TBL_QOS_POLICER_RES_FLD_MAX:
        sal_memcpy(p_db_qos_policer_res, p_qos_policer_res, sizeof(tbl_qos_policer_res_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_QOS_POLICER_RES, field_id, p_db_qos_policer_res);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_QOS_POLICER_RES, field_id, p_db_qos_policer_res);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_qos_policer_res_add_qos_policer_res(tbl_qos_policer_res_t *p_qos_policer_res)
{
    return tbl_qos_policer_res_add_qos_policer_res_sync(p_qos_policer_res, TRUE);
}

int32
tbl_qos_policer_res_del_qos_policer_res(tbl_qos_policer_res_key_t *p_qos_policer_res_key)
{
    return tbl_qos_policer_res_del_qos_policer_res_sync(p_qos_policer_res_key, TRUE);
}

int32
tbl_qos_policer_res_set_qos_policer_res_field(tbl_qos_policer_res_t *p_qos_policer_res, tbl_qos_policer_res_field_id_t field_id)
{
    return tbl_qos_policer_res_set_qos_policer_res_field_sync(p_qos_policer_res, field_id, TRUE);
}

tbl_qos_policer_res_t*
tbl_qos_policer_res_get_qos_policer_res(tbl_qos_policer_res_key_t *p_qos_policer_res_key)
{
    tbl_qos_policer_res_master_t *p_master = _g_p_tbl_qos_policer_res_master;
    tbl_qos_policer_res_t lkp;

    sal_memcpy(&lkp.key, p_qos_policer_res_key, sizeof(tbl_qos_policer_res_key_t));
    return ctclib_hash_lookup(p_master->qos_policer_res_hash, &lkp);
}

char*
tbl_qos_policer_res_key_val2str(tbl_qos_policer_res_t *p_qos_policer_res, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_qos_policer_res_key_val2str(buf, MAX_CMD_STR_LEN, &p_qos_policer_res->key));
    return str;
}

int32
tbl_qos_policer_res_key_str2val(char *str, tbl_qos_policer_res_t *p_qos_policer_res)
{
    int32 ret = 0;

    ret = cdb_qos_policer_res_key_str2val(str, &p_qos_policer_res->key);
    return ret;
}

char*
tbl_qos_policer_res_key_name_dump(tbl_qos_policer_res_t *p_qos_policer_res, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_QOS_POLICER_RES);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_qos_policer_res_key_val2str(buf, MAX_CMD_STR_LEN, &p_qos_policer_res->key));
    return str;
}

char*
tbl_qos_policer_res_key_value_dump(tbl_qos_policer_res_t *p_qos_policer_res, char *str)
{

    cdb_qos_policer_res_key_val2str(str, MAX_CMD_STR_LEN, &p_qos_policer_res->key);
    return str;
}

char*
tbl_qos_policer_res_field_name_dump(tbl_qos_policer_res_t *p_qos_policer_res, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_QOS_POLICER_RES);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_QOS_POLICER_RES_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_qos_policer_res_field_value_dump(tbl_qos_policer_res_t *p_qos_policer_res, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_QOS_POLICER_RES_FLD_KEY, field_id))
    {
        cdb_qos_policer_res_key_val2str(str, MAX_CMD_STR_LEN, &p_qos_policer_res->key);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_RES_FLD_REF_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_qos_policer_res->ref_cnt);
    }
    return str;
}

char**
tbl_qos_policer_res_table_dump(tbl_qos_policer_res_t *p_qos_policer_res, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_qos_policer_res_key_name_dump(p_qos_policer_res, buf));
    for(i=1; i<TBL_QOS_POLICER_RES_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_qos_policer_res_field_name_dump(p_qos_policer_res, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_qos_policer_res_field_value_dump(p_qos_policer_res, i, buf));
    }
    return str;
}

int32
tbl_qos_policer_res_field_value_parser(char *str, int32 field_id, tbl_qos_policer_res_t *p_qos_policer_res)
{
    if (FLD_MATCH(TBL_QOS_POLICER_RES_FLD_KEY, field_id))
    {
        cdb_qos_policer_res_key_str2val(str, &p_qos_policer_res->key);
    }
    if (FLD_MATCH(TBL_QOS_POLICER_RES_FLD_REF_CNT, field_id))
    {
        int32 ret;
        p_qos_policer_res->ref_cnt = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_qos_policer_res_table_parser(char** array, char* key_value,tbl_qos_policer_res_t *p_qos_policer_res)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_QOS_POLICER_RES);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_qos_policer_res_key_str2val(key_value, p_qos_policer_res));

    for(i=1; i<TBL_QOS_POLICER_RES_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_qos_policer_res_field_value_parser( array[j++], i, p_qos_policer_res));
    }

    return PM_E_NONE;
}

int32
tbl_qos_policer_res_dump_one(tbl_qos_policer_res_t *p_qos_policer_res, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_QOS_POLICER_RES);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_QOS_POLICER_RES_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_QOS_POLICER_RES_FLD_KEY].name, 
            cdb_qos_policer_res_key_val2str(buf, MAX_CMD_STR_LEN, &p_qos_policer_res->key));
    }
    if (FLD_MATCH(TBL_QOS_POLICER_RES_FLD_REF_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_QOS_POLICER_RES_FLD_REF_CNT].name,
            p_qos_policer_res->ref_cnt);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_qos_policer_res_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_qos_policer_res_master_t *p_master = _g_p_tbl_qos_policer_res_master;
    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    ctclib_hash_iterate2(p_master->qos_policer_res_hash, tbl_hash_iter_adpt_fn, fn, pargs);
    return PM_E_NONE;
}

tbl_qos_policer_res_master_t*
tbl_qos_policer_res_get_master()
{
    return _g_p_tbl_qos_policer_res_master;
}

tbl_qos_policer_res_master_t*
tbl_qos_policer_res_init_qos_policer_res()
{
    _g_p_tbl_qos_policer_res_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_qos_policer_res_master_t));
    _g_p_tbl_qos_policer_res_master->qos_policer_res_hash = ctclib_hash_create(_tbl_qos_policer_res_hash_make, _tbl_qos_policer_res_hash_cmp);
    return _g_p_tbl_qos_policer_res_master;
}

