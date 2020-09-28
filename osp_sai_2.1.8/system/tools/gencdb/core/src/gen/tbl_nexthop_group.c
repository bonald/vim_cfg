
#include "proto.h"
#include "gen/tbl_nexthop_group_define.h"
#include "gen/tbl_nexthop_group.h"
#include "cdb_data_cmp.h"

tbl_nexthop_group_master_t *_g_p_tbl_nexthop_group_master = NULL;

static uint32
_tbl_nexthop_group_hash_make(tbl_nexthop_group_t *p_nhg)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_nhg->key;
    for (index = 0; index < sizeof(p_nhg->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_nexthop_group_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_nexthop_group_t *p_nhg1 = (tbl_nexthop_group_t*)p_arg1;
    tbl_nexthop_group_t *p_nhg2 = (tbl_nexthop_group_t*)p_arg2;

    if (0 == sal_memcmp(&p_nhg1->key, &p_nhg2->key, sizeof(tbl_nexthop_group_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_nexthop_group_add_nexthop_group_sync(tbl_nexthop_group_t *p_nhg, uint32 sync)
{
    tbl_nexthop_group_master_t *p_master = _g_p_tbl_nexthop_group_master;
    tbl_nexthop_group_t *p_db_nhg = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_nexthop_group_get_nexthop_group(&p_nhg->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_nhg = XCALLOC(MEM_TBL_NEXTHOP_GROUP, sizeof(tbl_nexthop_group_t));
    if (NULL == p_db_nhg)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_nhg, p_nhg, sizeof(tbl_nexthop_group_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->nhg_hash, (void*)p_db_nhg, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->nhg_list, p_db_nhg);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_NEXTHOP_GROUP, p_db_nhg);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_NEXTHOP_GROUP, p_db_nhg);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_nexthop_group_del_nexthop_group_sync(tbl_nexthop_group_key_t *p_nhg_key, uint32 sync)
{
    tbl_nexthop_group_master_t *p_master = _g_p_tbl_nexthop_group_master;
    tbl_nexthop_group_t *p_db_nhg = NULL;

    /* 1. lookup entry exist */
    p_db_nhg = tbl_nexthop_group_get_nexthop_group(p_nhg_key);
    if (NULL == p_db_nhg)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_NEXTHOP_GROUP, p_db_nhg);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_NEXTHOP_GROUP, p_db_nhg);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->nhg_hash, (void*)p_db_nhg);
    ctclib_slistnode_delete(p_master->nhg_list, p_db_nhg);

    /* 4. free db entry */
    XFREE(MEM_TBL_NEXTHOP_GROUP, p_db_nhg);

    return PM_E_NONE;
}

int32
tbl_nexthop_group_set_nexthop_group_field_sync(tbl_nexthop_group_t *p_nhg, tbl_nexthop_group_field_id_t field_id, uint32 sync)
{
    tbl_nexthop_group_t *p_db_nhg = NULL;

    /* 1. lookup entry exist */
    p_db_nhg = tbl_nexthop_group_get_nexthop_group(&p_nhg->key);
    if (NULL == p_db_nhg)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_NEXTHOP_GROUP_FLD_NH_GROUP:
        sal_memcpy(&p_db_nhg->nh_group, &p_nhg->nh_group, sizeof(p_nhg->nh_group));
        break;

    case TBL_NEXTHOP_GROUP_FLD_NHG_OID:
        p_db_nhg->nhg_oid = p_nhg->nhg_oid;
        break;

    case TBL_NEXTHOP_GROUP_FLD_REF_CNT:
        p_db_nhg->ref_cnt = p_nhg->ref_cnt;
        break;

    case TBL_NEXTHOP_GROUP_FLD_ECMP_HASH:
        p_db_nhg->ecmp_hash = p_nhg->ecmp_hash;
        break;

    case TBL_NEXTHOP_GROUP_FLD_MAX:
        sal_memcpy(p_db_nhg, p_nhg, sizeof(tbl_nexthop_group_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_NEXTHOP_GROUP, field_id, p_db_nhg);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_NEXTHOP_GROUP, field_id, p_db_nhg);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_nexthop_group_add_nexthop_group(tbl_nexthop_group_t *p_nhg)
{
    return tbl_nexthop_group_add_nexthop_group_sync(p_nhg, TRUE);
}

int32
tbl_nexthop_group_del_nexthop_group(tbl_nexthop_group_key_t *p_nhg_key)
{
    return tbl_nexthop_group_del_nexthop_group_sync(p_nhg_key, TRUE);
}

int32
tbl_nexthop_group_set_nexthop_group_field(tbl_nexthop_group_t *p_nhg, tbl_nexthop_group_field_id_t field_id)
{
    return tbl_nexthop_group_set_nexthop_group_field_sync(p_nhg, field_id, TRUE);
}

tbl_nexthop_group_t*
tbl_nexthop_group_get_nexthop_group(tbl_nexthop_group_key_t *p_nhg_key)
{
    tbl_nexthop_group_master_t *p_master = _g_p_tbl_nexthop_group_master;
    tbl_nexthop_group_t lkp;

    sal_memcpy(&lkp.key, p_nhg_key, sizeof(tbl_nexthop_group_key_t));
    return ctclib_hash_lookup(p_master->nhg_hash, &lkp);
}

char*
tbl_nexthop_group_key_val2str(tbl_nexthop_group_t *p_nhg, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_nexthop_ecmp_val2str(buf, MAX_CMD_STR_LEN, &p_nhg->key));
    return str;
}

int32
tbl_nexthop_group_key_str2val(char *str, tbl_nexthop_group_t *p_nhg)
{
    int32 ret = 0;

    ret = cdb_nexthop_ecmp_str2val(str, &p_nhg->key);
    return ret;
}

char*
tbl_nexthop_group_key_name_dump(tbl_nexthop_group_t *p_nhg, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_NEXTHOP_GROUP);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_nexthop_ecmp_val2str(buf, MAX_CMD_STR_LEN, &p_nhg->key));
    return str;
}

char*
tbl_nexthop_group_key_value_dump(tbl_nexthop_group_t *p_nhg, char *str)
{

    cdb_nexthop_ecmp_val2str(str, MAX_CMD_STR_LEN, &p_nhg->key);
    return str;
}

char*
tbl_nexthop_group_field_name_dump(tbl_nexthop_group_t *p_nhg, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_NEXTHOP_GROUP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_NEXTHOP_GROUP_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_nexthop_group_field_value_dump(tbl_nexthop_group_t *p_nhg, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_NEXTHOP_GROUP_FLD_KEY, field_id))
    {
        cdb_nexthop_ecmp_val2str(str, MAX_CMD_STR_LEN, &p_nhg->key);
    }
    if (FLD_MATCH(TBL_NEXTHOP_GROUP_FLD_NH_GROUP, field_id))
    {
        cdb_nexthop_ecmp_val2str(str, MAX_CMD_STR_LEN, &p_nhg->nh_group);
    }
    if (FLD_MATCH(TBL_NEXTHOP_GROUP_FLD_NHG_OID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_nhg->nhg_oid);
    }
    if (FLD_MATCH(TBL_NEXTHOP_GROUP_FLD_REF_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_nhg->ref_cnt);
    }
    if (FLD_MATCH(TBL_NEXTHOP_GROUP_FLD_ECMP_HASH, field_id))
    {
        sal_sprintf(str, "%u", p_nhg->ecmp_hash);
    }
    return str;
}

char**
tbl_nexthop_group_table_dump(tbl_nexthop_group_t *p_nhg, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_nexthop_group_key_name_dump(p_nhg, buf));
    for(i=1; i<TBL_NEXTHOP_GROUP_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_nexthop_group_field_name_dump(p_nhg, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_nexthop_group_field_value_dump(p_nhg, i, buf));
    }
    return str;
}

int32
tbl_nexthop_group_field_value_parser(char *str, int32 field_id, tbl_nexthop_group_t *p_nhg)
{
    if (FLD_MATCH(TBL_NEXTHOP_GROUP_FLD_KEY, field_id))
    {
        cdb_nexthop_ecmp_str2val(str, &p_nhg->key);
    }
    if (FLD_MATCH(TBL_NEXTHOP_GROUP_FLD_NH_GROUP, field_id))
    {
        cdb_nexthop_ecmp_str2val(str, &p_nhg->nh_group);
    }
    if (FLD_MATCH(TBL_NEXTHOP_GROUP_FLD_NHG_OID, field_id))
    {
        int32 ret;
        p_nhg->nhg_oid = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NEXTHOP_GROUP_FLD_REF_CNT, field_id))
    {
        int32 ret;
        p_nhg->ref_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NEXTHOP_GROUP_FLD_ECMP_HASH, field_id))
    {
        int32 ret;
        p_nhg->ecmp_hash = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_nexthop_group_table_parser(char** array, char* key_value,tbl_nexthop_group_t *p_nhg)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_NEXTHOP_GROUP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_nexthop_group_key_str2val(key_value, p_nhg));

    for(i=1; i<TBL_NEXTHOP_GROUP_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_nexthop_group_field_value_parser( array[j++], i, p_nhg));
    }

    return PM_E_NONE;
}

int32
tbl_nexthop_group_dump_one(tbl_nexthop_group_t *p_nhg, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_NEXTHOP_GROUP);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_NEXTHOP_GROUP_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_NEXTHOP_GROUP_FLD_KEY].name, 
            cdb_nexthop_ecmp_val2str(buf, MAX_CMD_STR_LEN, &p_nhg->key));
    }
    if (FLD_MATCH(TBL_NEXTHOP_GROUP_FLD_NH_GROUP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_NEXTHOP_GROUP_FLD_NH_GROUP].name, 
            cdb_nexthop_ecmp_val2str(buf, MAX_CMD_STR_LEN, &p_nhg->nh_group));
    }
    if (FLD_MATCH(TBL_NEXTHOP_GROUP_FLD_NHG_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_NEXTHOP_GROUP_FLD_NHG_OID].name,
            p_nhg->nhg_oid);
    }
    if (FLD_MATCH(TBL_NEXTHOP_GROUP_FLD_REF_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NEXTHOP_GROUP_FLD_REF_CNT].name,
            p_nhg->ref_cnt);
    }
    if (FLD_MATCH(TBL_NEXTHOP_GROUP_FLD_ECMP_HASH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NEXTHOP_GROUP_FLD_ECMP_HASH].name,
            p_nhg->ecmp_hash);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_nexthop_group_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_nexthop_group_master_t *p_master = _g_p_tbl_nexthop_group_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_nexthop_group_t *p_db_nhg = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->nhg_list, p_db_nhg, listnode, next)
    {
        rc |= fn(p_db_nhg, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_nexthop_group_master_t*
tbl_nexthop_group_get_master()
{
    return _g_p_tbl_nexthop_group_master;
}

tbl_nexthop_group_master_t*
tbl_nexthop_group_init_nexthop_group()
{
    _g_p_tbl_nexthop_group_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_nexthop_group_master_t));
    _g_p_tbl_nexthop_group_master->nhg_hash = ctclib_hash_create(_tbl_nexthop_group_hash_make, _tbl_nexthop_group_hash_cmp);
    _g_p_tbl_nexthop_group_master->nhg_list = ctclib_slist_create(tbl_nexthop_group_cmp, NULL);
    return _g_p_tbl_nexthop_group_master;
}

