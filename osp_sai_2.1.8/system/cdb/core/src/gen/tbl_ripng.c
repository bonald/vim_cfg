
#include "proto.h"
#include "gen/tbl_ripng_define.h"
#include "gen/tbl_ripng.h"
#include "cdb_data_cmp.h"

tbl_ripng_master_t *_g_p_tbl_ripng_master = NULL;

static uint32
_tbl_ripng_hash_make(tbl_ripng_t *p_ripng)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_ripng->key;
    for (index = 0; index < sizeof(p_ripng->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_ripng_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_ripng_t *p_ripng1 = (tbl_ripng_t*)p_arg1;
    tbl_ripng_t *p_ripng2 = (tbl_ripng_t*)p_arg2;

    if (0 == sal_memcmp(&p_ripng1->key, &p_ripng2->key, sizeof(tbl_ripng_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_ripng_add_ripng_sync(tbl_ripng_t *p_ripng, uint32 sync)
{
    tbl_ripng_master_t *p_master = _g_p_tbl_ripng_master;
    tbl_ripng_t *p_db_ripng = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_ripng_get_ripng(&p_ripng->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_ripng = XCALLOC(MEM_TBL_RIPNG, sizeof(tbl_ripng_t));
    if (NULL == p_db_ripng)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_ripng, p_ripng, sizeof(tbl_ripng_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->ripng_hash, (void*)p_db_ripng, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->ripng_list, p_db_ripng);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_RIPNG, p_db_ripng);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_RIPNG, p_db_ripng);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ripng_del_ripng_sync(tbl_ripng_key_t *p_ripng_key, uint32 sync)
{
    tbl_ripng_master_t *p_master = _g_p_tbl_ripng_master;
    tbl_ripng_t *p_db_ripng = NULL;

    /* 1. lookup entry exist */
    p_db_ripng = tbl_ripng_get_ripng(p_ripng_key);
    if (NULL == p_db_ripng)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_RIPNG, p_db_ripng);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_RIPNG, p_db_ripng);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->ripng_hash, (void*)p_db_ripng);
    ctclib_slistnode_delete(p_master->ripng_list, p_db_ripng);

    /* 4. free db entry */
    XFREE(MEM_TBL_RIPNG, p_db_ripng);

    return PM_E_NONE;
}

int32
tbl_ripng_set_ripng_field_sync(tbl_ripng_t *p_ripng, tbl_ripng_field_id_t field_id, uint32 sync)
{
    tbl_ripng_t *p_db_ripng = NULL;

    /* 1. lookup entry exist */
    p_db_ripng = tbl_ripng_get_ripng(&p_ripng->key);
    if (NULL == p_db_ripng)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_RIPNG_FLD_ALLOW_ECMP:
        p_db_ripng->allow_ecmp = p_ripng->allow_ecmp;
        break;

    case TBL_RIPNG_FLD_TIMERS:
        sal_memcpy(&p_db_ripng->timers, &p_ripng->timers, sizeof(p_ripng->timers));
        break;

    case TBL_RIPNG_FLD_DEFAULT_INFORMATION:
        p_db_ripng->default_information = p_ripng->default_information;
        break;

    case TBL_RIPNG_FLD_REDISTRIBUTE:
        p_db_ripng->redistribute = p_ripng->redistribute;
        break;

    case TBL_RIPNG_FLD_RED_METRIC:
        sal_memcpy(p_db_ripng->red_metric, p_ripng->red_metric, sizeof(p_ripng->red_metric));
        break;

    case TBL_RIPNG_FLD_DEFAULT_METRIC:
        p_db_ripng->default_metric = p_ripng->default_metric;
        break;

    case TBL_RIPNG_FLD_MAX:
        sal_memcpy(p_db_ripng, p_ripng, sizeof(tbl_ripng_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_RIPNG, field_id, p_db_ripng);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_RIPNG, field_id, p_db_ripng);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ripng_add_ripng(tbl_ripng_t *p_ripng)
{
    return tbl_ripng_add_ripng_sync(p_ripng, TRUE);
}

int32
tbl_ripng_del_ripng(tbl_ripng_key_t *p_ripng_key)
{
    return tbl_ripng_del_ripng_sync(p_ripng_key, TRUE);
}

int32
tbl_ripng_set_ripng_field(tbl_ripng_t *p_ripng, tbl_ripng_field_id_t field_id)
{
    return tbl_ripng_set_ripng_field_sync(p_ripng, field_id, TRUE);
}

tbl_ripng_t*
tbl_ripng_get_ripng(tbl_ripng_key_t *p_ripng_key)
{
    tbl_ripng_master_t *p_master = _g_p_tbl_ripng_master;
    tbl_ripng_t lkp;

    sal_memcpy(&lkp.key, p_ripng_key, sizeof(tbl_ripng_key_t));
    return ctclib_hash_lookup(p_master->ripng_hash, &lkp);
}

char*
tbl_ripng_key_val2str(tbl_ripng_t *p_ripng, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_ripng->key.id);
    return str;
}

int32
tbl_ripng_key_str2val(char *str, tbl_ripng_t *p_ripng)
{
    int32 ret = 0;

    p_ripng->key.id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_ripng_key_name_dump(tbl_ripng_t *p_ripng, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_RIPNG);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_ripng->key.id);
    return str;
}

char*
tbl_ripng_key_value_dump(tbl_ripng_t *p_ripng, char *str)
{
    sal_sprintf(str, "%u", p_ripng->key.id);
    return str;
}

char*
tbl_ripng_field_name_dump(tbl_ripng_t *p_ripng, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_RIPNG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_RIPNG_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_ripng_field_value_dump(tbl_ripng_t *p_ripng, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_RIPNG_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_ripng->key.id);
    }
    if (FLD_MATCH(TBL_RIPNG_FLD_ALLOW_ECMP, field_id))
    {
        sal_sprintf(str, "%u", p_ripng->allow_ecmp);
    }
    if (FLD_MATCH(TBL_RIPNG_FLD_TIMERS, field_id))
    {
        cdb_rip_timers_val2str(str, MAX_CMD_STR_LEN, &p_ripng->timers);
    }
    if (FLD_MATCH(TBL_RIPNG_FLD_DEFAULT_INFORMATION, field_id))
    {
        sal_sprintf(str, "%u", p_ripng->default_information);
    }
    if (FLD_MATCH(TBL_RIPNG_FLD_REDISTRIBUTE, field_id))
    {
        sal_sprintf(str, "%u", p_ripng->redistribute);
    }
    if (FLD_MATCH(TBL_RIPNG_FLD_RED_METRIC, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_ripng->red_metric, sizeof(p_ripng->red_metric)/4);
    }
    if (FLD_MATCH(TBL_RIPNG_FLD_DEFAULT_METRIC, field_id))
    {
        sal_sprintf(str, "%u", p_ripng->default_metric);
    }
    return str;
}

char**
tbl_ripng_table_dump(tbl_ripng_t *p_ripng, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_ripng_key_name_dump(p_ripng, buf));
    for(i=1; i<TBL_RIPNG_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_ripng_field_name_dump(p_ripng, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_ripng_field_value_dump(p_ripng, i, buf));
    }
    return str;
}

int32
tbl_ripng_field_value_parser(char *str, int32 field_id, tbl_ripng_t *p_ripng)
{
    if (FLD_MATCH(TBL_RIPNG_FLD_KEY, field_id))
    {
        int32 ret;
        p_ripng->key.id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RIPNG_FLD_ALLOW_ECMP, field_id))
    {
        int32 ret;
        p_ripng->allow_ecmp = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RIPNG_FLD_TIMERS, field_id))
    {
        cdb_rip_timers_str2val(str, &p_ripng->timers);
    }
    if (FLD_MATCH(TBL_RIPNG_FLD_DEFAULT_INFORMATION, field_id))
    {
        int32 ret;
        p_ripng->default_information = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RIPNG_FLD_REDISTRIBUTE, field_id))
    {
        int32 ret;
        p_ripng->redistribute = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_RIPNG_FLD_RED_METRIC, field_id))
    {
        cdb_uint32_array_str2val(str, p_ripng->red_metric);
    }
    if (FLD_MATCH(TBL_RIPNG_FLD_DEFAULT_METRIC, field_id))
    {
        int32 ret;
        p_ripng->default_metric = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_ripng_table_parser(char** array, char* key_value,tbl_ripng_t *p_ripng)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_RIPNG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_ripng_key_str2val(key_value, p_ripng));

    for(i=1; i<TBL_RIPNG_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_ripng_field_value_parser( array[j++], i, p_ripng));
    }

    return PM_E_NONE;
}

int32
tbl_ripng_dump_one(tbl_ripng_t *p_ripng, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_RIPNG);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_RIPNG_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_RIPNG_FLD_KEY].name,
            p_ripng->key.id);
    }
    if (FLD_MATCH(TBL_RIPNG_FLD_ALLOW_ECMP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_RIPNG_FLD_ALLOW_ECMP].name,
            p_ripng->allow_ecmp);
    }
    if (FLD_MATCH(TBL_RIPNG_FLD_TIMERS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_RIPNG_FLD_TIMERS].name, 
            cdb_rip_timers_val2str(buf, MAX_CMD_STR_LEN, &p_ripng->timers));
    }
    if (FLD_MATCH(TBL_RIPNG_FLD_DEFAULT_INFORMATION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_RIPNG_FLD_DEFAULT_INFORMATION].name,
            p_ripng->default_information);
    }
    if (FLD_MATCH(TBL_RIPNG_FLD_REDISTRIBUTE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_RIPNG_FLD_REDISTRIBUTE].name,
            p_ripng->redistribute);
    }
    if (FLD_MATCH(TBL_RIPNG_FLD_RED_METRIC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_RIPNG_FLD_RED_METRIC].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_ripng->red_metric, sizeof(p_ripng->red_metric)/4));
    }
    if (FLD_MATCH(TBL_RIPNG_FLD_DEFAULT_METRIC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_RIPNG_FLD_DEFAULT_METRIC].name,
            p_ripng->default_metric);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_ripng_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_ripng_master_t *p_master = _g_p_tbl_ripng_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_ripng_t *p_db_ripng = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->ripng_list, p_db_ripng, listnode, next)
    {
        rc |= fn(p_db_ripng, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_ripng_master_t*
tbl_ripng_get_master()
{
    return _g_p_tbl_ripng_master;
}

tbl_ripng_master_t*
tbl_ripng_init_ripng()
{
    _g_p_tbl_ripng_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_ripng_master_t));
    _g_p_tbl_ripng_master->ripng_hash = ctclib_hash_create(_tbl_ripng_hash_make, _tbl_ripng_hash_cmp);
    _g_p_tbl_ripng_master->ripng_list = ctclib_slist_create(tbl_ripng_cmp, NULL);
    return _g_p_tbl_ripng_master;
}

