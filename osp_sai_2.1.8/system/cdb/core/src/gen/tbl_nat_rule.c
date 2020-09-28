
#include "proto.h"
#include "gen/tbl_nat_rule_define.h"
#include "gen/tbl_nat_rule.h"
#include "cdb_data_cmp.h"

tbl_nat_rule_master_t *_g_p_tbl_nat_rule_master = NULL;

static uint32
_tbl_nat_rule_hash_make(tbl_nat_rule_t *p_nat_rule)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_nat_rule->key;
    for (index = 0; index < sizeof(p_nat_rule->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_nat_rule_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_nat_rule_t *p_nat_rule1 = (tbl_nat_rule_t*)p_arg1;
    tbl_nat_rule_t *p_nat_rule2 = (tbl_nat_rule_t*)p_arg2;

    if (0 == sal_memcmp(&p_nat_rule1->key, &p_nat_rule2->key, sizeof(tbl_nat_rule_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_nat_rule_add_nat_rule_sync(tbl_nat_rule_t *p_nat_rule, uint32 sync)
{
    tbl_nat_rule_master_t *p_master = _g_p_tbl_nat_rule_master;
    tbl_nat_rule_t *p_db_nat_rule = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_nat_rule_get_nat_rule(&p_nat_rule->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_nat_rule = XCALLOC(MEM_TBL_NAT_RULE, sizeof(tbl_nat_rule_t));
    if (NULL == p_db_nat_rule)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_nat_rule, p_nat_rule, sizeof(tbl_nat_rule_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->nat_rule_hash, (void*)p_db_nat_rule, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->nat_rule_list, p_db_nat_rule);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_NAT_RULE, p_db_nat_rule);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_NAT_RULE, p_db_nat_rule);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_nat_rule_del_nat_rule_sync(tbl_nat_rule_key_t *p_nat_rule_key, uint32 sync)
{
    tbl_nat_rule_master_t *p_master = _g_p_tbl_nat_rule_master;
    tbl_nat_rule_t *p_db_nat_rule = NULL;

    /* 1. lookup entry exist */
    p_db_nat_rule = tbl_nat_rule_get_nat_rule(p_nat_rule_key);
    if (NULL == p_db_nat_rule)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_NAT_RULE, p_db_nat_rule);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_NAT_RULE, p_db_nat_rule);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->nat_rule_hash, (void*)p_db_nat_rule);
    ctclib_slistnode_delete(p_master->nat_rule_list, p_db_nat_rule);

    /* 4. free db entry */
    XFREE(MEM_TBL_NAT_RULE, p_db_nat_rule);

    return PM_E_NONE;
}

int32
tbl_nat_rule_set_nat_rule_field_sync(tbl_nat_rule_t *p_nat_rule, tbl_nat_rule_field_id_t field_id, uint32 sync)
{
    tbl_nat_rule_t *p_db_nat_rule = NULL;

    /* 1. lookup entry exist */
    p_db_nat_rule = tbl_nat_rule_get_nat_rule(&p_nat_rule->key);
    if (NULL == p_db_nat_rule)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_NAT_RULE_FLD_HANDLE_POSTROUTING:
        p_db_nat_rule->handle_postrouting = p_nat_rule->handle_postrouting;
        break;

    case TBL_NAT_RULE_FLD_HANDLE_PREROUTING:
        p_db_nat_rule->handle_prerouting = p_nat_rule->handle_prerouting;
        break;

    case TBL_NAT_RULE_FLD_CATEGORY_ID:
        p_db_nat_rule->category_id = p_nat_rule->category_id;
        break;

    case TBL_NAT_RULE_FLD_REFER_IIF:
        sal_memcpy(p_db_nat_rule->refer_iif, p_nat_rule->refer_iif, sizeof(p_nat_rule->refer_iif));
        break;

    case TBL_NAT_RULE_FLD_REFER_OIF:
        sal_memcpy(p_db_nat_rule->refer_oif, p_nat_rule->refer_oif, sizeof(p_nat_rule->refer_oif));
        break;

    case TBL_NAT_RULE_FLD_NFT_CHAIN_ID_ACL:
        p_db_nat_rule->nft_chain_id_acl = p_nat_rule->nft_chain_id_acl;
        break;

    case TBL_NAT_RULE_FLD_NFT_CHAIN_ID_RULE_PRE:
        p_db_nat_rule->nft_chain_id_rule_pre = p_nat_rule->nft_chain_id_rule_pre;
        break;

    case TBL_NAT_RULE_FLD_NFT_CHAIN_ID_RULE_POST:
        p_db_nat_rule->nft_chain_id_rule_post = p_nat_rule->nft_chain_id_rule_post;
        break;

    case TBL_NAT_RULE_FLD_CFG_ORDER:
        p_db_nat_rule->cfg_order = p_nat_rule->cfg_order;
        break;

    case TBL_NAT_RULE_FLD_MAX:
        sal_memcpy(p_db_nat_rule, p_nat_rule, sizeof(tbl_nat_rule_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_NAT_RULE, field_id, p_db_nat_rule);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_NAT_RULE, field_id, p_db_nat_rule);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_nat_rule_add_nat_rule(tbl_nat_rule_t *p_nat_rule)
{
    return tbl_nat_rule_add_nat_rule_sync(p_nat_rule, TRUE);
}

int32
tbl_nat_rule_del_nat_rule(tbl_nat_rule_key_t *p_nat_rule_key)
{
    return tbl_nat_rule_del_nat_rule_sync(p_nat_rule_key, TRUE);
}

int32
tbl_nat_rule_set_nat_rule_field(tbl_nat_rule_t *p_nat_rule, tbl_nat_rule_field_id_t field_id)
{
    return tbl_nat_rule_set_nat_rule_field_sync(p_nat_rule, field_id, TRUE);
}

tbl_nat_rule_t*
tbl_nat_rule_get_nat_rule(tbl_nat_rule_key_t *p_nat_rule_key)
{
    tbl_nat_rule_master_t *p_master = _g_p_tbl_nat_rule_master;
    tbl_nat_rule_t lkp;

    sal_memcpy(&lkp.key, p_nat_rule_key, sizeof(tbl_nat_rule_key_t));
    return ctclib_hash_lookup(p_master->nat_rule_hash, &lkp);
}

char*
tbl_nat_rule_key_val2str(tbl_nat_rule_t *p_nat_rule, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_nat_rule_key_val2str(buf, MAX_CMD_STR_LEN, &p_nat_rule->key));
    return str;
}

int32
tbl_nat_rule_key_str2val(char *str, tbl_nat_rule_t *p_nat_rule)
{
    int32 ret = 0;

    ret = cdb_nat_rule_key_str2val(str, &p_nat_rule->key);
    return ret;
}

char*
tbl_nat_rule_key_name_dump(tbl_nat_rule_t *p_nat_rule, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_NAT_RULE);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_nat_rule_key_val2str(buf, MAX_CMD_STR_LEN, &p_nat_rule->key));
    return str;
}

char*
tbl_nat_rule_key_value_dump(tbl_nat_rule_t *p_nat_rule, char *str)
{

    cdb_nat_rule_key_val2str(str, MAX_CMD_STR_LEN, &p_nat_rule->key);
    return str;
}

char*
tbl_nat_rule_field_name_dump(tbl_nat_rule_t *p_nat_rule, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_NAT_RULE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_NAT_RULE_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_nat_rule_field_value_dump(tbl_nat_rule_t *p_nat_rule, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_NAT_RULE_FLD_KEY, field_id))
    {
        cdb_nat_rule_key_val2str(str, MAX_CMD_STR_LEN, &p_nat_rule->key);
    }
    if (FLD_MATCH(TBL_NAT_RULE_FLD_HANDLE_POSTROUTING, field_id))
    {
        sal_sprintf(str, "%u", p_nat_rule->handle_postrouting);
    }
    if (FLD_MATCH(TBL_NAT_RULE_FLD_HANDLE_PREROUTING, field_id))
    {
        sal_sprintf(str, "%u", p_nat_rule->handle_prerouting);
    }
    if (FLD_MATCH(TBL_NAT_RULE_FLD_CATEGORY_ID, field_id))
    {
        sal_sprintf(str, "%u", p_nat_rule->category_id);
    }
    if (FLD_MATCH(TBL_NAT_RULE_FLD_REFER_IIF, field_id))
    {
        sal_sprintf(str, "%s", p_nat_rule->refer_iif);
    }
    if (FLD_MATCH(TBL_NAT_RULE_FLD_REFER_OIF, field_id))
    {
        sal_sprintf(str, "%s", p_nat_rule->refer_oif);
    }
    if (FLD_MATCH(TBL_NAT_RULE_FLD_NFT_CHAIN_ID_ACL, field_id))
    {
        sal_sprintf(str, "%u", p_nat_rule->nft_chain_id_acl);
    }
    if (FLD_MATCH(TBL_NAT_RULE_FLD_NFT_CHAIN_ID_RULE_PRE, field_id))
    {
        sal_sprintf(str, "%u", p_nat_rule->nft_chain_id_rule_pre);
    }
    if (FLD_MATCH(TBL_NAT_RULE_FLD_NFT_CHAIN_ID_RULE_POST, field_id))
    {
        sal_sprintf(str, "%u", p_nat_rule->nft_chain_id_rule_post);
    }
    if (FLD_MATCH(TBL_NAT_RULE_FLD_CFG_ORDER, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_nat_rule->cfg_order);
    }
    return str;
}

char**
tbl_nat_rule_table_dump(tbl_nat_rule_t *p_nat_rule, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_nat_rule_key_name_dump(p_nat_rule, buf));
    for(i=1; i<TBL_NAT_RULE_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_nat_rule_field_name_dump(p_nat_rule, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_nat_rule_field_value_dump(p_nat_rule, i, buf));
    }
    return str;
}

int32
tbl_nat_rule_field_value_parser(char *str, int32 field_id, tbl_nat_rule_t *p_nat_rule)
{
    if (FLD_MATCH(TBL_NAT_RULE_FLD_KEY, field_id))
    {
        cdb_nat_rule_key_str2val(str, &p_nat_rule->key);
    }
    if (FLD_MATCH(TBL_NAT_RULE_FLD_HANDLE_POSTROUTING, field_id))
    {
        int32 ret;
        p_nat_rule->handle_postrouting = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NAT_RULE_FLD_HANDLE_PREROUTING, field_id))
    {
        int32 ret;
        p_nat_rule->handle_prerouting = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NAT_RULE_FLD_CATEGORY_ID, field_id))
    {
        int32 ret;
        p_nat_rule->category_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NAT_RULE_FLD_REFER_IIF, field_id))
    {
        sal_strcpy(p_nat_rule->refer_iif, str);
    }
    if (FLD_MATCH(TBL_NAT_RULE_FLD_REFER_OIF, field_id))
    {
        sal_strcpy(p_nat_rule->refer_oif, str);
    }
    if (FLD_MATCH(TBL_NAT_RULE_FLD_NFT_CHAIN_ID_ACL, field_id))
    {
        int32 ret;
        p_nat_rule->nft_chain_id_acl = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NAT_RULE_FLD_NFT_CHAIN_ID_RULE_PRE, field_id))
    {
        int32 ret;
        p_nat_rule->nft_chain_id_rule_pre = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NAT_RULE_FLD_NFT_CHAIN_ID_RULE_POST, field_id))
    {
        int32 ret;
        p_nat_rule->nft_chain_id_rule_post = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NAT_RULE_FLD_CFG_ORDER, field_id))
    {
        int32 ret;
        p_nat_rule->cfg_order = cdb_uint64_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_nat_rule_table_parser(char** array, char* key_value,tbl_nat_rule_t *p_nat_rule)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_NAT_RULE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_nat_rule_key_str2val(key_value, p_nat_rule));

    for(i=1; i<TBL_NAT_RULE_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_nat_rule_field_value_parser( array[j++], i, p_nat_rule));
    }

    return PM_E_NONE;
}

int32
tbl_nat_rule_dump_one(tbl_nat_rule_t *p_nat_rule, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_NAT_RULE);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_NAT_RULE_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_NAT_RULE_FLD_KEY].name, 
            cdb_nat_rule_key_val2str(buf, MAX_CMD_STR_LEN, &p_nat_rule->key));
    }
    if (FLD_MATCH(TBL_NAT_RULE_FLD_HANDLE_POSTROUTING, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_RULE_FLD_HANDLE_POSTROUTING].name,
            p_nat_rule->handle_postrouting);
    }
    if (FLD_MATCH(TBL_NAT_RULE_FLD_HANDLE_PREROUTING, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_RULE_FLD_HANDLE_PREROUTING].name,
            p_nat_rule->handle_prerouting);
    }
    if (FLD_MATCH(TBL_NAT_RULE_FLD_CATEGORY_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_RULE_FLD_CATEGORY_ID].name,
            p_nat_rule->category_id);
    }
    if (FLD_MATCH(TBL_NAT_RULE_FLD_REFER_IIF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_NAT_RULE_FLD_REFER_IIF].name,
            p_nat_rule->refer_iif);
    }
    if (FLD_MATCH(TBL_NAT_RULE_FLD_REFER_OIF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_NAT_RULE_FLD_REFER_OIF].name,
            p_nat_rule->refer_oif);
    }
    if (FLD_MATCH(TBL_NAT_RULE_FLD_NFT_CHAIN_ID_ACL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_RULE_FLD_NFT_CHAIN_ID_ACL].name,
            p_nat_rule->nft_chain_id_acl);
    }
    if (FLD_MATCH(TBL_NAT_RULE_FLD_NFT_CHAIN_ID_RULE_PRE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_RULE_FLD_NFT_CHAIN_ID_RULE_PRE].name,
            p_nat_rule->nft_chain_id_rule_pre);
    }
    if (FLD_MATCH(TBL_NAT_RULE_FLD_NFT_CHAIN_ID_RULE_POST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_RULE_FLD_NFT_CHAIN_ID_RULE_POST].name,
            p_nat_rule->nft_chain_id_rule_post);
    }
    if (FLD_MATCH(TBL_NAT_RULE_FLD_CFG_ORDER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_NAT_RULE_FLD_CFG_ORDER].name,
            p_nat_rule->cfg_order);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_nat_rule_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_nat_rule_master_t *p_master = _g_p_tbl_nat_rule_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_nat_rule_t *p_db_nat_rule = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->nat_rule_list, p_db_nat_rule, listnode, next)
    {
        rc |= fn(p_db_nat_rule, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_nat_rule_master_t*
tbl_nat_rule_get_master()
{
    return _g_p_tbl_nat_rule_master;
}

tbl_nat_rule_master_t*
tbl_nat_rule_init_nat_rule()
{
    _g_p_tbl_nat_rule_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_nat_rule_master_t));
    _g_p_tbl_nat_rule_master->nat_rule_hash = ctclib_hash_create(_tbl_nat_rule_hash_make, _tbl_nat_rule_hash_cmp);
    _g_p_tbl_nat_rule_master->nat_rule_list = ctclib_slist_create(tbl_nat_rule_cmp, NULL);
    return _g_p_tbl_nat_rule_master;
}

