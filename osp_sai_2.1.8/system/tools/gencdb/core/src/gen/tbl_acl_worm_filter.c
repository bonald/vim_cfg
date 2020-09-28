
#include "proto.h"
#include "gen/tbl_acl_worm_filter_define.h"
#include "gen/tbl_acl_worm_filter.h"
#include "cdb_data_cmp.h"

tbl_acl_worm_filter_master_t *_g_p_tbl_acl_worm_filter_master = NULL;

static uint32
_tbl_acl_worm_filter_hash_make(tbl_acl_worm_filter_t *p_acl_worm_filter)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_acl_worm_filter->key;
    for (index = 0; index < sizeof(p_acl_worm_filter->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_acl_worm_filter_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_acl_worm_filter_t *p_acl_worm_filter1 = (tbl_acl_worm_filter_t*)p_arg1;
    tbl_acl_worm_filter_t *p_acl_worm_filter2 = (tbl_acl_worm_filter_t*)p_arg2;

    if (0 == sal_memcmp(&p_acl_worm_filter1->key, &p_acl_worm_filter2->key, sizeof(tbl_acl_worm_filter_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_acl_worm_filter_add_acl_worm_filter_sync(tbl_acl_worm_filter_t *p_acl_worm_filter, uint32 sync)
{
    tbl_acl_worm_filter_master_t *p_master = _g_p_tbl_acl_worm_filter_master;
    tbl_acl_worm_filter_t *p_db_acl_worm_filter = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_acl_worm_filter_get_acl_worm_filter(&p_acl_worm_filter->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_acl_worm_filter = XCALLOC(MEM_TBL_ACL_WORM_FILTER, sizeof(tbl_acl_worm_filter_t));
    if (NULL == p_db_acl_worm_filter)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_acl_worm_filter, p_acl_worm_filter, sizeof(tbl_acl_worm_filter_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->acl_worm_filter_hash, (void*)p_db_acl_worm_filter, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->acl_worm_filter_list, p_db_acl_worm_filter);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_ACL_WORM_FILTER, p_db_acl_worm_filter);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_ACL_WORM_FILTER, p_db_acl_worm_filter);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_acl_worm_filter_del_acl_worm_filter_sync(tbl_acl_worm_filter_key_t *p_acl_worm_filter_key, uint32 sync)
{
    tbl_acl_worm_filter_master_t *p_master = _g_p_tbl_acl_worm_filter_master;
    tbl_acl_worm_filter_t *p_db_acl_worm_filter = NULL;

    /* 1. lookup entry exist */
    p_db_acl_worm_filter = tbl_acl_worm_filter_get_acl_worm_filter(p_acl_worm_filter_key);
    if (NULL == p_db_acl_worm_filter)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_ACL_WORM_FILTER, p_db_acl_worm_filter);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_ACL_WORM_FILTER, p_db_acl_worm_filter);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->acl_worm_filter_hash, (void*)p_db_acl_worm_filter);
    ctclib_slistnode_delete(p_master->acl_worm_filter_list, p_db_acl_worm_filter);

    /* 4. free db entry */
    XFREE(MEM_TBL_ACL_WORM_FILTER, p_db_acl_worm_filter);

    return PM_E_NONE;
}

int32
tbl_acl_worm_filter_set_acl_worm_filter_field_sync(tbl_acl_worm_filter_t *p_acl_worm_filter, tbl_acl_worm_filter_field_id_t field_id, uint32 sync)
{
    tbl_acl_worm_filter_t *p_db_acl_worm_filter = NULL;

    /* 1. lookup entry exist */
    p_db_acl_worm_filter = tbl_acl_worm_filter_get_acl_worm_filter(&p_acl_worm_filter->key);
    if (NULL == p_db_acl_worm_filter)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_ACL_WORM_FILTER_FLD_SEQ_NUM:
        p_db_acl_worm_filter->seq_num = p_acl_worm_filter->seq_num;
        break;

    case TBL_ACL_WORM_FILTER_FLD_ENABLE:
        p_db_acl_worm_filter->enable = p_acl_worm_filter->enable;
        break;

    case TBL_ACL_WORM_FILTER_FLD_ISTCP:
        p_db_acl_worm_filter->istcp = p_acl_worm_filter->istcp;
        break;

    case TBL_ACL_WORM_FILTER_FLD_DSTPORT:
        p_db_acl_worm_filter->dstport = p_acl_worm_filter->dstport;
        break;

    case TBL_ACL_WORM_FILTER_FLD_ISRANGE:
        p_db_acl_worm_filter->isrange = p_acl_worm_filter->isrange;
        break;

    case TBL_ACL_WORM_FILTER_FLD_STATS_EN:
        p_db_acl_worm_filter->stats_en = p_acl_worm_filter->stats_en;
        break;

    case TBL_ACL_WORM_FILTER_FLD_MAX:
        sal_memcpy(p_db_acl_worm_filter, p_acl_worm_filter, sizeof(tbl_acl_worm_filter_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_ACL_WORM_FILTER, field_id, p_db_acl_worm_filter);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_ACL_WORM_FILTER, field_id, p_db_acl_worm_filter);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_acl_worm_filter_add_acl_worm_filter(tbl_acl_worm_filter_t *p_acl_worm_filter)
{
    return tbl_acl_worm_filter_add_acl_worm_filter_sync(p_acl_worm_filter, TRUE);
}

int32
tbl_acl_worm_filter_del_acl_worm_filter(tbl_acl_worm_filter_key_t *p_acl_worm_filter_key)
{
    return tbl_acl_worm_filter_del_acl_worm_filter_sync(p_acl_worm_filter_key, TRUE);
}

int32
tbl_acl_worm_filter_set_acl_worm_filter_field(tbl_acl_worm_filter_t *p_acl_worm_filter, tbl_acl_worm_filter_field_id_t field_id)
{
    return tbl_acl_worm_filter_set_acl_worm_filter_field_sync(p_acl_worm_filter, field_id, TRUE);
}

tbl_acl_worm_filter_t*
tbl_acl_worm_filter_get_acl_worm_filter(tbl_acl_worm_filter_key_t *p_acl_worm_filter_key)
{
    tbl_acl_worm_filter_master_t *p_master = _g_p_tbl_acl_worm_filter_master;
    tbl_acl_worm_filter_t lkp;

    sal_memcpy(&lkp.key, p_acl_worm_filter_key, sizeof(tbl_acl_worm_filter_key_t));
    return ctclib_hash_lookup(p_master->acl_worm_filter_hash, &lkp);
}

char*
tbl_acl_worm_filter_key_val2str(tbl_acl_worm_filter_t *p_acl_worm_filter, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_acl_worm_filter->key.worm_filter_name);
    return str;
}

int32
tbl_acl_worm_filter_key_str2val(char *str, tbl_acl_worm_filter_t *p_acl_worm_filter)
{
    int32 ret = 0;

    sal_strcpy(p_acl_worm_filter->key.worm_filter_name, str);
    return ret;
}

char*
tbl_acl_worm_filter_key_name_dump(tbl_acl_worm_filter_t *p_acl_worm_filter, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_WORM_FILTER);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_acl_worm_filter->key.worm_filter_name);
    return str;
}

char*
tbl_acl_worm_filter_key_value_dump(tbl_acl_worm_filter_t *p_acl_worm_filter, char *str)
{
    sal_sprintf(str, "%s", p_acl_worm_filter->key.worm_filter_name);
    return str;
}

char*
tbl_acl_worm_filter_field_name_dump(tbl_acl_worm_filter_t *p_acl_worm_filter, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_WORM_FILTER);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_ACL_WORM_FILTER_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_acl_worm_filter_field_value_dump(tbl_acl_worm_filter_t *p_acl_worm_filter, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_ACL_WORM_FILTER_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_acl_worm_filter->key.worm_filter_name);
    }
    if (FLD_MATCH(TBL_ACL_WORM_FILTER_FLD_SEQ_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_acl_worm_filter->seq_num);
    }
    if (FLD_MATCH(TBL_ACL_WORM_FILTER_FLD_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_acl_worm_filter->enable);
    }
    if (FLD_MATCH(TBL_ACL_WORM_FILTER_FLD_ISTCP, field_id))
    {
        sal_sprintf(str, "%u", p_acl_worm_filter->istcp);
    }
    if (FLD_MATCH(TBL_ACL_WORM_FILTER_FLD_DSTPORT, field_id))
    {
        sal_sprintf(str, "%u", p_acl_worm_filter->dstport);
    }
    if (FLD_MATCH(TBL_ACL_WORM_FILTER_FLD_ISRANGE, field_id))
    {
        sal_sprintf(str, "%u", p_acl_worm_filter->isrange);
    }
    if (FLD_MATCH(TBL_ACL_WORM_FILTER_FLD_STATS_EN, field_id))
    {
        sal_sprintf(str, "%u", p_acl_worm_filter->stats_en);
    }
    return str;
}

char**
tbl_acl_worm_filter_table_dump(tbl_acl_worm_filter_t *p_acl_worm_filter, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_acl_worm_filter_key_name_dump(p_acl_worm_filter, buf));
    for(i=1; i<TBL_ACL_WORM_FILTER_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_acl_worm_filter_field_name_dump(p_acl_worm_filter, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_acl_worm_filter_field_value_dump(p_acl_worm_filter, i, buf));
    }
    return str;
}

int32
tbl_acl_worm_filter_field_value_parser(char *str, int32 field_id, tbl_acl_worm_filter_t *p_acl_worm_filter)
{
    if (FLD_MATCH(TBL_ACL_WORM_FILTER_FLD_KEY, field_id))
    {
        sal_strcpy(p_acl_worm_filter->key.worm_filter_name, str);
    }
    if (FLD_MATCH(TBL_ACL_WORM_FILTER_FLD_SEQ_NUM, field_id))
    {
        int32 ret;
        p_acl_worm_filter->seq_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_WORM_FILTER_FLD_ENABLE, field_id))
    {
        int32 ret;
        p_acl_worm_filter->enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_WORM_FILTER_FLD_ISTCP, field_id))
    {
        int32 ret;
        p_acl_worm_filter->istcp = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_WORM_FILTER_FLD_DSTPORT, field_id))
    {
        int32 ret;
        p_acl_worm_filter->dstport = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_WORM_FILTER_FLD_ISRANGE, field_id))
    {
        int32 ret;
        p_acl_worm_filter->isrange = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_WORM_FILTER_FLD_STATS_EN, field_id))
    {
        int32 ret;
        p_acl_worm_filter->stats_en = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_acl_worm_filter_table_parser(char** array, char* key_value,tbl_acl_worm_filter_t *p_acl_worm_filter)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_WORM_FILTER);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_acl_worm_filter_key_str2val(key_value, p_acl_worm_filter));

    for(i=1; i<TBL_ACL_WORM_FILTER_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_acl_worm_filter_field_value_parser( array[j++], i, p_acl_worm_filter));
    }

    return PM_E_NONE;
}

int32
tbl_acl_worm_filter_dump_one(tbl_acl_worm_filter_t *p_acl_worm_filter, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_WORM_FILTER);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_ACL_WORM_FILTER_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_WORM_FILTER_FLD_KEY].name,
            p_acl_worm_filter->key.worm_filter_name);
    }
    if (FLD_MATCH(TBL_ACL_WORM_FILTER_FLD_SEQ_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_WORM_FILTER_FLD_SEQ_NUM].name,
            p_acl_worm_filter->seq_num);
    }
    if (FLD_MATCH(TBL_ACL_WORM_FILTER_FLD_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_WORM_FILTER_FLD_ENABLE].name,
            p_acl_worm_filter->enable);
    }
    if (FLD_MATCH(TBL_ACL_WORM_FILTER_FLD_ISTCP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_WORM_FILTER_FLD_ISTCP].name,
            p_acl_worm_filter->istcp);
    }
    if (FLD_MATCH(TBL_ACL_WORM_FILTER_FLD_DSTPORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_WORM_FILTER_FLD_DSTPORT].name,
            p_acl_worm_filter->dstport);
    }
    if (FLD_MATCH(TBL_ACL_WORM_FILTER_FLD_ISRANGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_WORM_FILTER_FLD_ISRANGE].name,
            p_acl_worm_filter->isrange);
    }
    if (FLD_MATCH(TBL_ACL_WORM_FILTER_FLD_STATS_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_WORM_FILTER_FLD_STATS_EN].name,
            p_acl_worm_filter->stats_en);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_acl_worm_filter_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_acl_worm_filter_master_t *p_master = _g_p_tbl_acl_worm_filter_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_acl_worm_filter_t *p_db_acl_worm_filter = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->acl_worm_filter_list, p_db_acl_worm_filter, listnode, next)
    {
        rc |= fn(p_db_acl_worm_filter, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_acl_worm_filter_master_t*
tbl_acl_worm_filter_get_master()
{
    return _g_p_tbl_acl_worm_filter_master;
}

tbl_acl_worm_filter_master_t*
tbl_acl_worm_filter_init_acl_worm_filter()
{
    _g_p_tbl_acl_worm_filter_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_acl_worm_filter_master_t));
    _g_p_tbl_acl_worm_filter_master->acl_worm_filter_hash = ctclib_hash_create(_tbl_acl_worm_filter_hash_make, _tbl_acl_worm_filter_hash_cmp);
    _g_p_tbl_acl_worm_filter_master->acl_worm_filter_list = ctclib_slist_create(tbl_acl_worm_filter_cmp, NULL);
    return _g_p_tbl_acl_worm_filter_master;
}

tbl_acl_worm_filter_t*
tbl_acl_worm_filter_get_by_name(char *name)
{
    tbl_acl_worm_filter_key_t acl_worm_filter_key;

    sal_memset(&acl_worm_filter_key, 0, sizeof(acl_worm_filter_key));
    sal_strcpy(acl_worm_filter_key.worm_filter_name, name);
    return tbl_acl_worm_filter_get_acl_worm_filter(&acl_worm_filter_key);
}


int32
_acl_worm_filter_seq_check(tbl_acl_worm_filter_t *p_acl_worm_filter, tbl_iter_args_t *pargs)
{
    tbl_acl_worm_filter_t *pst_new_worm_filter = pargs->argv[0];

    if (pst_new_worm_filter->seq_num == p_acl_worm_filter->seq_num) 
    {
        pargs->argv[1] = p_acl_worm_filter;
    }

    return PM_E_NONE;
}

tbl_acl_worm_filter_t*
tbl_acl_worm_filter_get_by_seq(uint32 seq_num)
{
    tbl_iter_args_t args;
    tbl_acl_worm_filter_t *pst_same_worm_filter = NULL;
    tbl_acl_worm_filter_t acl_worm_fitler;

    sal_memset(&acl_worm_fitler, 0, sizeof(acl_worm_fitler));
    sal_memset(&args, 0, sizeof(args));
    acl_worm_fitler.seq_num = seq_num;

    args.argv[0] = &acl_worm_fitler;
    args.argv[1] = pst_same_worm_filter;
    tbl_acl_worm_filter_iterate((TBL_ITER_CB_FUNC)_acl_worm_filter_seq_check, &args);
    return args.argv[1];
}


int32
tbl_acl_worm_filter_get_range_one(tbl_acl_worm_filter_t *p_acl_worm_filter, tbl_iter_args_t *pargs)
{
    uint32 *num = pargs->argv[0];
    
    *num += p_acl_worm_filter->isrange ? 1 : 0;
    return PM_E_NONE;
}

int32
tbl_acl_worm_filter_get_range_count()
{
    tbl_iter_args_t args;
    int32 num = 0;
    
    sal_memset(&args, 0, sizeof(args));
    args.argv[0] = &num;
    tbl_acl_worm_filter_iterate((TBL_ITER_CB_FUNC)tbl_acl_worm_filter_get_range_one, &args);
    return num;
}





