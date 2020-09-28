
#include "proto.h"
#include "gen/tbl_static_rt_cnt_define.h"
#include "gen/tbl_static_rt_cnt.h"
#include "cdb_data_cmp.h"

tbl_static_rt_cnt_master_t *_g_p_tbl_static_rt_cnt_master = NULL;

static uint32
_tbl_static_rt_cnt_hash_make(tbl_static_rt_cnt_t *p_rt_cnt)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_rt_cnt->key;
    for (index = 0; index < sizeof(p_rt_cnt->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_static_rt_cnt_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_static_rt_cnt_t *p_rt_cnt1 = (tbl_static_rt_cnt_t*)p_arg1;
    tbl_static_rt_cnt_t *p_rt_cnt2 = (tbl_static_rt_cnt_t*)p_arg2;

    if (0 == sal_memcmp(&p_rt_cnt1->key, &p_rt_cnt2->key, sizeof(tbl_static_rt_cnt_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_static_rt_cnt_add_static_rt_cnt_sync(tbl_static_rt_cnt_t *p_rt_cnt, uint32 sync)
{
    tbl_static_rt_cnt_master_t *p_master = _g_p_tbl_static_rt_cnt_master;
    tbl_static_rt_cnt_t *p_db_rt_cnt = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_static_rt_cnt_get_static_rt_cnt(&p_rt_cnt->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_rt_cnt = XCALLOC(MEM_TBL_STATIC_RT_CNT, sizeof(tbl_static_rt_cnt_t));
    if (NULL == p_db_rt_cnt)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_rt_cnt, p_rt_cnt, sizeof(tbl_static_rt_cnt_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->rt_cnt_hash, (void*)p_db_rt_cnt, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_STATIC_RT_CNT, p_db_rt_cnt);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_STATIC_RT_CNT, p_db_rt_cnt);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_static_rt_cnt_del_static_rt_cnt_sync(tbl_static_rt_cnt_key_t *p_rt_cnt_key, uint32 sync)
{
    tbl_static_rt_cnt_master_t *p_master = _g_p_tbl_static_rt_cnt_master;
    tbl_static_rt_cnt_t *p_db_rt_cnt = NULL;

    /* 1. lookup entry exist */
    p_db_rt_cnt = tbl_static_rt_cnt_get_static_rt_cnt(p_rt_cnt_key);
    if (NULL == p_db_rt_cnt)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_STATIC_RT_CNT, p_db_rt_cnt);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_STATIC_RT_CNT, p_db_rt_cnt);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->rt_cnt_hash, (void*)p_db_rt_cnt);

    /* 4. free db entry */
    XFREE(MEM_TBL_STATIC_RT_CNT, p_db_rt_cnt);

    return PM_E_NONE;
}

int32
tbl_static_rt_cnt_set_static_rt_cnt_field_sync(tbl_static_rt_cnt_t *p_rt_cnt, tbl_static_rt_cnt_field_id_t field_id, uint32 sync)
{
    tbl_static_rt_cnt_t *p_db_rt_cnt = NULL;

    /* 1. lookup entry exist */
    p_db_rt_cnt = tbl_static_rt_cnt_get_static_rt_cnt(&p_rt_cnt->key);
    if (NULL == p_db_rt_cnt)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_STATIC_RT_CNT_FLD_REFER_CNT:
        p_db_rt_cnt->refer_cnt = p_rt_cnt->refer_cnt;
        break;

    case TBL_STATIC_RT_CNT_FLD_HAS_BLACKHOLE:
        p_db_rt_cnt->has_blackhole = p_rt_cnt->has_blackhole;
        break;

    case TBL_STATIC_RT_CNT_FLD_NH_GROUP:
        sal_memcpy(&p_db_rt_cnt->nh_group, &p_rt_cnt->nh_group, sizeof(p_rt_cnt->nh_group));
        break;

    case TBL_STATIC_RT_CNT_FLD_MAX:
        sal_memcpy(p_db_rt_cnt, p_rt_cnt, sizeof(tbl_static_rt_cnt_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_STATIC_RT_CNT, field_id, p_db_rt_cnt);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_STATIC_RT_CNT, field_id, p_db_rt_cnt);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_static_rt_cnt_add_static_rt_cnt(tbl_static_rt_cnt_t *p_rt_cnt)
{
    return tbl_static_rt_cnt_add_static_rt_cnt_sync(p_rt_cnt, TRUE);
}

int32
tbl_static_rt_cnt_del_static_rt_cnt(tbl_static_rt_cnt_key_t *p_rt_cnt_key)
{
    return tbl_static_rt_cnt_del_static_rt_cnt_sync(p_rt_cnt_key, TRUE);
}

int32
tbl_static_rt_cnt_set_static_rt_cnt_field(tbl_static_rt_cnt_t *p_rt_cnt, tbl_static_rt_cnt_field_id_t field_id)
{
    return tbl_static_rt_cnt_set_static_rt_cnt_field_sync(p_rt_cnt, field_id, TRUE);
}

tbl_static_rt_cnt_t*
tbl_static_rt_cnt_get_static_rt_cnt(tbl_static_rt_cnt_key_t *p_rt_cnt_key)
{
    tbl_static_rt_cnt_master_t *p_master = _g_p_tbl_static_rt_cnt_master;
    tbl_static_rt_cnt_t lkp;

    sal_memcpy(&lkp.key, p_rt_cnt_key, sizeof(tbl_static_rt_cnt_key_t));
    return ctclib_hash_lookup(p_master->rt_cnt_hash, &lkp);
}

char*
tbl_static_rt_cnt_key_val2str(tbl_static_rt_cnt_t *p_rt_cnt, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_route_node_key_with_ad_val2str(buf, MAX_CMD_STR_LEN, &p_rt_cnt->key));
    return str;
}

int32
tbl_static_rt_cnt_key_str2val(char *str, tbl_static_rt_cnt_t *p_rt_cnt)
{
    int32 ret = 0;

    ret = cdb_route_node_key_with_ad_str2val(str, &p_rt_cnt->key);
    return ret;
}

char*
tbl_static_rt_cnt_key_name_dump(tbl_static_rt_cnt_t *p_rt_cnt, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_STATIC_RT_CNT);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_route_node_key_with_ad_val2str(buf, MAX_CMD_STR_LEN, &p_rt_cnt->key));
    return str;
}

char*
tbl_static_rt_cnt_key_value_dump(tbl_static_rt_cnt_t *p_rt_cnt, char *str)
{

    cdb_route_node_key_with_ad_val2str(str, MAX_CMD_STR_LEN, &p_rt_cnt->key);
    return str;
}

char*
tbl_static_rt_cnt_field_name_dump(tbl_static_rt_cnt_t *p_rt_cnt, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_STATIC_RT_CNT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_STATIC_RT_CNT_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_static_rt_cnt_field_value_dump(tbl_static_rt_cnt_t *p_rt_cnt, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_STATIC_RT_CNT_FLD_KEY, field_id))
    {
        cdb_route_node_key_with_ad_val2str(str, MAX_CMD_STR_LEN, &p_rt_cnt->key);
    }
    if (FLD_MATCH(TBL_STATIC_RT_CNT_FLD_REFER_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_rt_cnt->refer_cnt);
    }
    if (FLD_MATCH(TBL_STATIC_RT_CNT_FLD_HAS_BLACKHOLE, field_id))
    {
        sal_sprintf(str, "%u", p_rt_cnt->has_blackhole);
    }
    if (FLD_MATCH(TBL_STATIC_RT_CNT_FLD_NH_GROUP, field_id))
    {
        cdb_nexthop_ecmp_routed_val2str(str, MAX_CMD_STR_LEN, &p_rt_cnt->nh_group);
    }
    return str;
}

char**
tbl_static_rt_cnt_table_dump(tbl_static_rt_cnt_t *p_rt_cnt, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_static_rt_cnt_key_name_dump(p_rt_cnt, buf));
    for(i=1; i<TBL_STATIC_RT_CNT_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_static_rt_cnt_field_name_dump(p_rt_cnt, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_static_rt_cnt_field_value_dump(p_rt_cnt, i, buf));
    }
    return str;
}

int32
tbl_static_rt_cnt_field_value_parser(char *str, int32 field_id, tbl_static_rt_cnt_t *p_rt_cnt)
{
    if (FLD_MATCH(TBL_STATIC_RT_CNT_FLD_KEY, field_id))
    {
        cdb_route_node_key_with_ad_str2val(str, &p_rt_cnt->key);
    }
    if (FLD_MATCH(TBL_STATIC_RT_CNT_FLD_REFER_CNT, field_id))
    {
        int32 ret;
        p_rt_cnt->refer_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_STATIC_RT_CNT_FLD_HAS_BLACKHOLE, field_id))
    {
        int32 ret;
        p_rt_cnt->has_blackhole = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_STATIC_RT_CNT_FLD_NH_GROUP, field_id))
    {
        cdb_nexthop_ecmp_routed_str2val(str, &p_rt_cnt->nh_group);
    }
    return PM_E_NONE;
}

int32
tbl_static_rt_cnt_table_parser(char** array, char* key_value,tbl_static_rt_cnt_t *p_rt_cnt)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_STATIC_RT_CNT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_static_rt_cnt_key_str2val(key_value, p_rt_cnt));

    for(i=1; i<TBL_STATIC_RT_CNT_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_static_rt_cnt_field_value_parser( array[j++], i, p_rt_cnt));
    }

    return PM_E_NONE;
}

int32
tbl_static_rt_cnt_dump_one(tbl_static_rt_cnt_t *p_rt_cnt, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_STATIC_RT_CNT);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_STATIC_RT_CNT_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_STATIC_RT_CNT_FLD_KEY].name, 
            cdb_route_node_key_with_ad_val2str(buf, MAX_CMD_STR_LEN, &p_rt_cnt->key));
    }
    if (FLD_MATCH(TBL_STATIC_RT_CNT_FLD_REFER_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_STATIC_RT_CNT_FLD_REFER_CNT].name,
            p_rt_cnt->refer_cnt);
    }
    if (FLD_MATCH(TBL_STATIC_RT_CNT_FLD_HAS_BLACKHOLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_STATIC_RT_CNT_FLD_HAS_BLACKHOLE].name,
            p_rt_cnt->has_blackhole);
    }
    if (FLD_MATCH(TBL_STATIC_RT_CNT_FLD_NH_GROUP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_STATIC_RT_CNT_FLD_NH_GROUP].name, 
            cdb_nexthop_ecmp_routed_val2str(buf, MAX_CMD_STR_LEN, &p_rt_cnt->nh_group));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_static_rt_cnt_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_static_rt_cnt_master_t *p_master = _g_p_tbl_static_rt_cnt_master;
    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    ctclib_hash_iterate2(p_master->rt_cnt_hash, tbl_hash_iter_adpt_fn, fn, pargs);
    return PM_E_NONE;
}

tbl_static_rt_cnt_master_t*
tbl_static_rt_cnt_get_master()
{
    return _g_p_tbl_static_rt_cnt_master;
}

tbl_static_rt_cnt_master_t*
tbl_static_rt_cnt_init_static_rt_cnt()
{
    _g_p_tbl_static_rt_cnt_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_static_rt_cnt_master_t));
    _g_p_tbl_static_rt_cnt_master->rt_cnt_hash = ctclib_hash_create(_tbl_static_rt_cnt_hash_make, _tbl_static_rt_cnt_hash_cmp);
    return _g_p_tbl_static_rt_cnt_master;
}

