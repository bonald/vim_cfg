
#include "proto.h"
#include "gen/tbl_nat_session_limit_define.h"
#include "gen/tbl_nat_session_limit.h"
#include "cdb_data_cmp.h"

tbl_nat_session_limit_master_t *_g_p_tbl_nat_session_limit_master = NULL;

static uint32
_tbl_nat_session_limit_hash_make(tbl_nat_session_limit_t *p_nat_session_limit)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_nat_session_limit->key;
    for (index = 0; index < sizeof(p_nat_session_limit->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_nat_session_limit_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_nat_session_limit_t *p_nat_session_limit1 = (tbl_nat_session_limit_t*)p_arg1;
    tbl_nat_session_limit_t *p_nat_session_limit2 = (tbl_nat_session_limit_t*)p_arg2;

    if (0 == sal_memcmp(&p_nat_session_limit1->key, &p_nat_session_limit2->key, sizeof(tbl_nat_session_limit_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_nat_session_limit_add_nat_session_limit_sync(tbl_nat_session_limit_t *p_nat_session_limit, uint32 sync)
{
    tbl_nat_session_limit_master_t *p_master = _g_p_tbl_nat_session_limit_master;
    tbl_nat_session_limit_t *p_db_nat_session_limit = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_nat_session_limit_get_nat_session_limit(&p_nat_session_limit->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_nat_session_limit = XCALLOC(MEM_TBL_NAT_SESSION_LIMIT, sizeof(tbl_nat_session_limit_t));
    if (NULL == p_db_nat_session_limit)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_nat_session_limit, p_nat_session_limit, sizeof(tbl_nat_session_limit_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->nat_session_limit_hash, (void*)p_db_nat_session_limit, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->nat_session_limit_list, p_db_nat_session_limit);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_NAT_SESSION_LIMIT, p_db_nat_session_limit);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_NAT_SESSION_LIMIT, p_db_nat_session_limit);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_nat_session_limit_del_nat_session_limit_sync(tbl_nat_session_limit_key_t *p_nat_session_limit_key, uint32 sync)
{
    tbl_nat_session_limit_master_t *p_master = _g_p_tbl_nat_session_limit_master;
    tbl_nat_session_limit_t *p_db_nat_session_limit = NULL;

    /* 1. lookup entry exist */
    p_db_nat_session_limit = tbl_nat_session_limit_get_nat_session_limit(p_nat_session_limit_key);
    if (NULL == p_db_nat_session_limit)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_NAT_SESSION_LIMIT, p_db_nat_session_limit);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_NAT_SESSION_LIMIT, p_db_nat_session_limit);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->nat_session_limit_hash, (void*)p_db_nat_session_limit);
    ctclib_slistnode_delete(p_master->nat_session_limit_list, p_db_nat_session_limit);

    /* 4. free db entry */
    XFREE(MEM_TBL_NAT_SESSION_LIMIT, p_db_nat_session_limit);

    return PM_E_NONE;
}

int32
tbl_nat_session_limit_set_nat_session_limit_field_sync(tbl_nat_session_limit_t *p_nat_session_limit, tbl_nat_session_limit_field_id_t field_id, uint32 sync)
{
    tbl_nat_session_limit_t *p_db_nat_session_limit = NULL;

    /* 1. lookup entry exist */
    p_db_nat_session_limit = tbl_nat_session_limit_get_nat_session_limit(&p_nat_session_limit->key);
    if (NULL == p_db_nat_session_limit)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_NAT_SESSION_LIMIT_FLD_LIMIT:
        p_db_nat_session_limit->limit = p_nat_session_limit->limit;
        break;

    case TBL_NAT_SESSION_LIMIT_FLD_COUNT_UPSTREAM:
        p_db_nat_session_limit->count_upstream = p_nat_session_limit->count_upstream;
        break;

    case TBL_NAT_SESSION_LIMIT_FLD_COUNT_DNSTREAM:
        p_db_nat_session_limit->count_dnstream = p_nat_session_limit->count_dnstream;
        break;

    case TBL_NAT_SESSION_LIMIT_FLD_NFT_HDL_POST:
        p_db_nat_session_limit->nft_hdl_post = p_nat_session_limit->nft_hdl_post;
        break;

    case TBL_NAT_SESSION_LIMIT_FLD_NFT_HDL_PRE:
        p_db_nat_session_limit->nft_hdl_pre = p_nat_session_limit->nft_hdl_pre;
        break;

    case TBL_NAT_SESSION_LIMIT_FLD_MAX:
        sal_memcpy(p_db_nat_session_limit, p_nat_session_limit, sizeof(tbl_nat_session_limit_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_NAT_SESSION_LIMIT, field_id, p_db_nat_session_limit);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_NAT_SESSION_LIMIT, field_id, p_db_nat_session_limit);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_nat_session_limit_add_nat_session_limit(tbl_nat_session_limit_t *p_nat_session_limit)
{
    return tbl_nat_session_limit_add_nat_session_limit_sync(p_nat_session_limit, TRUE);
}

int32
tbl_nat_session_limit_del_nat_session_limit(tbl_nat_session_limit_key_t *p_nat_session_limit_key)
{
    return tbl_nat_session_limit_del_nat_session_limit_sync(p_nat_session_limit_key, TRUE);
}

int32
tbl_nat_session_limit_set_nat_session_limit_field(tbl_nat_session_limit_t *p_nat_session_limit, tbl_nat_session_limit_field_id_t field_id)
{
    return tbl_nat_session_limit_set_nat_session_limit_field_sync(p_nat_session_limit, field_id, TRUE);
}

tbl_nat_session_limit_t*
tbl_nat_session_limit_get_nat_session_limit(tbl_nat_session_limit_key_t *p_nat_session_limit_key)
{
    tbl_nat_session_limit_master_t *p_master = _g_p_tbl_nat_session_limit_master;
    tbl_nat_session_limit_t lkp;

    sal_memcpy(&lkp.key, p_nat_session_limit_key, sizeof(tbl_nat_session_limit_key_t));
    return ctclib_hash_lookup(p_master->nat_session_limit_hash, &lkp);
}

char*
tbl_nat_session_limit_key_val2str(tbl_nat_session_limit_t *p_nat_session_limit, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_nat_session_limit->key));
    return str;
}

int32
tbl_nat_session_limit_key_str2val(char *str, tbl_nat_session_limit_t *p_nat_session_limit)
{
    int32 ret = 0;

    ret = cdb_addr_str2val(&p_nat_session_limit->key, str, 0);
    return ret;
}

char*
tbl_nat_session_limit_key_name_dump(tbl_nat_session_limit_t *p_nat_session_limit, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_NAT_SESSION_LIMIT);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_nat_session_limit->key));
    return str;
}

char*
tbl_nat_session_limit_key_value_dump(tbl_nat_session_limit_t *p_nat_session_limit, char *str)
{

    cdb_addr_val2str(str, MAX_CMD_STR_LEN, &p_nat_session_limit->key);
    return str;
}

char*
tbl_nat_session_limit_field_name_dump(tbl_nat_session_limit_t *p_nat_session_limit, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_NAT_SESSION_LIMIT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_NAT_SESSION_LIMIT_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_nat_session_limit_field_value_dump(tbl_nat_session_limit_t *p_nat_session_limit, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_NAT_SESSION_LIMIT_FLD_KEY, field_id))
    {
        cdb_addr_val2str(str, MAX_CMD_STR_LEN, &p_nat_session_limit->key);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_LIMIT_FLD_LIMIT, field_id))
    {
        sal_sprintf(str, "%u", p_nat_session_limit->limit);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_LIMIT_FLD_COUNT_UPSTREAM, field_id))
    {
        sal_sprintf(str, "%u", p_nat_session_limit->count_upstream);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_LIMIT_FLD_COUNT_DNSTREAM, field_id))
    {
        sal_sprintf(str, "%u", p_nat_session_limit->count_dnstream);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_LIMIT_FLD_NFT_HDL_POST, field_id))
    {
        sal_sprintf(str, "%u", p_nat_session_limit->nft_hdl_post);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_LIMIT_FLD_NFT_HDL_PRE, field_id))
    {
        sal_sprintf(str, "%u", p_nat_session_limit->nft_hdl_pre);
    }
    return str;
}

char**
tbl_nat_session_limit_table_dump(tbl_nat_session_limit_t *p_nat_session_limit, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_nat_session_limit_key_name_dump(p_nat_session_limit, buf));
    for(i=1; i<TBL_NAT_SESSION_LIMIT_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_nat_session_limit_field_name_dump(p_nat_session_limit, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_nat_session_limit_field_value_dump(p_nat_session_limit, i, buf));
    }
    return str;
}

int32
tbl_nat_session_limit_field_value_parser(char *str, int32 field_id, tbl_nat_session_limit_t *p_nat_session_limit)
{
    if (FLD_MATCH(TBL_NAT_SESSION_LIMIT_FLD_KEY, field_id))
    {
        cdb_addr_str2val(&p_nat_session_limit->key, str, 0);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_LIMIT_FLD_LIMIT, field_id))
    {
        int32 ret;
        p_nat_session_limit->limit = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_LIMIT_FLD_COUNT_UPSTREAM, field_id))
    {
        int32 ret;
        p_nat_session_limit->count_upstream = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_LIMIT_FLD_COUNT_DNSTREAM, field_id))
    {
        int32 ret;
        p_nat_session_limit->count_dnstream = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_LIMIT_FLD_NFT_HDL_POST, field_id))
    {
        int32 ret;
        p_nat_session_limit->nft_hdl_post = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_LIMIT_FLD_NFT_HDL_PRE, field_id))
    {
        int32 ret;
        p_nat_session_limit->nft_hdl_pre = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_nat_session_limit_table_parser(char** array, char* key_value,tbl_nat_session_limit_t *p_nat_session_limit)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_NAT_SESSION_LIMIT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_nat_session_limit_key_str2val(key_value, p_nat_session_limit));

    for(i=1; i<TBL_NAT_SESSION_LIMIT_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_nat_session_limit_field_value_parser( array[j++], i, p_nat_session_limit));
    }

    return PM_E_NONE;
}

int32
tbl_nat_session_limit_dump_one(tbl_nat_session_limit_t *p_nat_session_limit, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_NAT_SESSION_LIMIT);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_NAT_SESSION_LIMIT_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_NAT_SESSION_LIMIT_FLD_KEY].name, 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_nat_session_limit->key));
    }
    if (FLD_MATCH(TBL_NAT_SESSION_LIMIT_FLD_LIMIT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_SESSION_LIMIT_FLD_LIMIT].name,
            p_nat_session_limit->limit);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_LIMIT_FLD_COUNT_UPSTREAM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_SESSION_LIMIT_FLD_COUNT_UPSTREAM].name,
            p_nat_session_limit->count_upstream);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_LIMIT_FLD_COUNT_DNSTREAM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_SESSION_LIMIT_FLD_COUNT_DNSTREAM].name,
            p_nat_session_limit->count_dnstream);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_LIMIT_FLD_NFT_HDL_POST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_SESSION_LIMIT_FLD_NFT_HDL_POST].name,
            p_nat_session_limit->nft_hdl_post);
    }
    if (FLD_MATCH(TBL_NAT_SESSION_LIMIT_FLD_NFT_HDL_PRE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_SESSION_LIMIT_FLD_NFT_HDL_PRE].name,
            p_nat_session_limit->nft_hdl_pre);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_nat_session_limit_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_nat_session_limit_master_t *p_master = _g_p_tbl_nat_session_limit_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_nat_session_limit_t *p_db_nat_session_limit = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->nat_session_limit_list, p_db_nat_session_limit, listnode, next)
    {
        rc |= fn(p_db_nat_session_limit, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_nat_session_limit_master_t*
tbl_nat_session_limit_get_master()
{
    return _g_p_tbl_nat_session_limit_master;
}

tbl_nat_session_limit_master_t*
tbl_nat_session_limit_init_nat_session_limit()
{
    _g_p_tbl_nat_session_limit_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_nat_session_limit_master_t));
    _g_p_tbl_nat_session_limit_master->nat_session_limit_hash = ctclib_hash_create(_tbl_nat_session_limit_hash_make, _tbl_nat_session_limit_hash_cmp);
    _g_p_tbl_nat_session_limit_master->nat_session_limit_list = ctclib_slist_create(tbl_nat_session_limit_cmp, NULL);
    return _g_p_tbl_nat_session_limit_master;
}

