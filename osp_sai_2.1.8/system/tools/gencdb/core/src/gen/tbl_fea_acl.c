
#include "proto.h"
#include "gen/tbl_fea_acl_define.h"
#include "gen/tbl_fea_acl.h"
#include "cdb_data_cmp.h"

tbl_fea_acl_master_t *_g_p_tbl_fea_acl_master = NULL;

static uint32
_tbl_fea_acl_hash_make(tbl_fea_acl_t *p_fea_acl)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_fea_acl->key;
    for (index = 0; index < sizeof(p_fea_acl->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_fea_acl_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_fea_acl_t *p_fea_acl1 = (tbl_fea_acl_t*)p_arg1;
    tbl_fea_acl_t *p_fea_acl2 = (tbl_fea_acl_t*)p_arg2;

    if (0 == sal_memcmp(&p_fea_acl1->key, &p_fea_acl2->key, sizeof(tbl_fea_acl_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_fea_acl_add_fea_acl_sync(tbl_fea_acl_t *p_fea_acl, uint32 sync)
{
    tbl_fea_acl_master_t *p_master = _g_p_tbl_fea_acl_master;
    tbl_fea_acl_t *p_db_fea_acl = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_fea_acl_get_fea_acl(&p_fea_acl->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_fea_acl = XCALLOC(MEM_TBL_FEA_ACL, sizeof(tbl_fea_acl_t));
    if (NULL == p_db_fea_acl)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_fea_acl, p_fea_acl, sizeof(tbl_fea_acl_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->fea_acl_hash, (void*)p_db_fea_acl, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_FEA_ACL, p_db_fea_acl);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_FEA_ACL, p_db_fea_acl);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_fea_acl_del_fea_acl_sync(tbl_fea_acl_key_t *p_fea_acl_key, uint32 sync)
{
    tbl_fea_acl_master_t *p_master = _g_p_tbl_fea_acl_master;
    tbl_fea_acl_t *p_db_fea_acl = NULL;

    /* 1. lookup entry exist */
    p_db_fea_acl = tbl_fea_acl_get_fea_acl(p_fea_acl_key);
    if (NULL == p_db_fea_acl)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_FEA_ACL, p_db_fea_acl);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_FEA_ACL, p_db_fea_acl);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->fea_acl_hash, (void*)p_db_fea_acl);

    /* 4. free db entry */
    XFREE(MEM_TBL_FEA_ACL, p_db_fea_acl);

    return PM_E_NONE;
}

int32
tbl_fea_acl_set_fea_acl_field_sync(tbl_fea_acl_t *p_fea_acl, tbl_fea_acl_field_id_t field_id, uint32 sync)
{
    tbl_fea_acl_t *p_db_fea_acl = NULL;

    /* 1. lookup entry exist */
    p_db_fea_acl = tbl_fea_acl_get_fea_acl(&p_fea_acl->key);
    if (NULL == p_db_fea_acl)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_FEA_ACL_FLD_IS_MAC_ACL:
        p_db_fea_acl->is_mac_acl = p_fea_acl->is_mac_acl;
        break;

    case TBL_FEA_ACL_FLD_SAI_ACL_ID:
        p_db_fea_acl->sai_acl_id = p_fea_acl->sai_acl_id;
        break;

    case TBL_FEA_ACL_FLD_SAI_ACL_ID_IPV4:
        p_db_fea_acl->sai_acl_id_ipv4 = p_fea_acl->sai_acl_id_ipv4;
        break;

    case TBL_FEA_ACL_FLD_SAI_ACL_ID_IPV6:
        p_db_fea_acl->sai_acl_id_ipv6 = p_fea_acl->sai_acl_id_ipv6;
        break;

    case TBL_FEA_ACL_FLD_COUNTER_ID:
        p_db_fea_acl->counter_id = p_fea_acl->counter_id;
        break;

    case TBL_FEA_ACL_FLD_STATS_PACKET:
        p_db_fea_acl->stats_packet = p_fea_acl->stats_packet;
        break;

    case TBL_FEA_ACL_FLD_STATS_BYTE:
        p_db_fea_acl->stats_byte = p_fea_acl->stats_byte;
        break;

    case TBL_FEA_ACL_FLD_MAX:
        sal_memcpy(p_db_fea_acl, p_fea_acl, sizeof(tbl_fea_acl_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_FEA_ACL, field_id, p_db_fea_acl);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_FEA_ACL, field_id, p_db_fea_acl);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_fea_acl_add_fea_acl(tbl_fea_acl_t *p_fea_acl)
{
    return tbl_fea_acl_add_fea_acl_sync(p_fea_acl, TRUE);
}

int32
tbl_fea_acl_del_fea_acl(tbl_fea_acl_key_t *p_fea_acl_key)
{
    return tbl_fea_acl_del_fea_acl_sync(p_fea_acl_key, TRUE);
}

int32
tbl_fea_acl_set_fea_acl_field(tbl_fea_acl_t *p_fea_acl, tbl_fea_acl_field_id_t field_id)
{
    return tbl_fea_acl_set_fea_acl_field_sync(p_fea_acl, field_id, TRUE);
}

tbl_fea_acl_t*
tbl_fea_acl_get_fea_acl(tbl_fea_acl_key_t *p_fea_acl_key)
{
    tbl_fea_acl_master_t *p_master = _g_p_tbl_fea_acl_master;
    tbl_fea_acl_t lkp;

    sal_memcpy(&lkp.key, p_fea_acl_key, sizeof(tbl_fea_acl_key_t));
    return ctclib_hash_lookup(p_master->fea_acl_hash, &lkp);
}

char*
tbl_fea_acl_key_val2str(tbl_fea_acl_t *p_fea_acl, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%"PRIu64, p_fea_acl->key.acl_id);
    return str;
}

int32
tbl_fea_acl_key_str2val(char *str, tbl_fea_acl_t *p_fea_acl)
{
    int32 ret = 0;

    p_fea_acl->key.acl_id = cdb_uint64_str2val(str, &ret);
    return ret;
}

char*
tbl_fea_acl_key_name_dump(tbl_fea_acl_t *p_fea_acl, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_ACL);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
        "%"PRIu64, p_node->name, p_fea_acl->key.acl_id);
    return str;
}

char*
tbl_fea_acl_key_value_dump(tbl_fea_acl_t *p_fea_acl, char *str)
{
    sal_sprintf(str, "%"PRIu64, p_fea_acl->key.acl_id);
    return str;
}

char*
tbl_fea_acl_field_name_dump(tbl_fea_acl_t *p_fea_acl, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_ACL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_FEA_ACL_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_fea_acl_field_value_dump(tbl_fea_acl_t *p_fea_acl, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_FEA_ACL_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_fea_acl->key.acl_id);
    }
    if (FLD_MATCH(TBL_FEA_ACL_FLD_IS_MAC_ACL, field_id))
    {
        sal_sprintf(str, "%u", p_fea_acl->is_mac_acl);
    }
    if (FLD_MATCH(TBL_FEA_ACL_FLD_SAI_ACL_ID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_fea_acl->sai_acl_id);
    }
    if (FLD_MATCH(TBL_FEA_ACL_FLD_SAI_ACL_ID_IPV4, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_fea_acl->sai_acl_id_ipv4);
    }
    if (FLD_MATCH(TBL_FEA_ACL_FLD_SAI_ACL_ID_IPV6, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_fea_acl->sai_acl_id_ipv6);
    }
    if (FLD_MATCH(TBL_FEA_ACL_FLD_COUNTER_ID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_fea_acl->counter_id);
    }
    if (FLD_MATCH(TBL_FEA_ACL_FLD_STATS_PACKET, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_fea_acl->stats_packet);
    }
    if (FLD_MATCH(TBL_FEA_ACL_FLD_STATS_BYTE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_fea_acl->stats_byte);
    }
    return str;
}

char**
tbl_fea_acl_table_dump(tbl_fea_acl_t *p_fea_acl, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_fea_acl_key_name_dump(p_fea_acl, buf));
    for(i=1; i<TBL_FEA_ACL_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_fea_acl_field_name_dump(p_fea_acl, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_fea_acl_field_value_dump(p_fea_acl, i, buf));
    }
    return str;
}

int32
tbl_fea_acl_field_value_parser(char *str, int32 field_id, tbl_fea_acl_t *p_fea_acl)
{
    if (FLD_MATCH(TBL_FEA_ACL_FLD_KEY, field_id))
    {
        int32 ret;
        p_fea_acl->key.acl_id = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_ACL_FLD_IS_MAC_ACL, field_id))
    {
        int32 ret;
        p_fea_acl->is_mac_acl = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_ACL_FLD_SAI_ACL_ID, field_id))
    {
        int32 ret;
        p_fea_acl->sai_acl_id = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_ACL_FLD_SAI_ACL_ID_IPV4, field_id))
    {
        int32 ret;
        p_fea_acl->sai_acl_id_ipv4 = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_ACL_FLD_SAI_ACL_ID_IPV6, field_id))
    {
        int32 ret;
        p_fea_acl->sai_acl_id_ipv6 = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_ACL_FLD_COUNTER_ID, field_id))
    {
        int32 ret;
        p_fea_acl->counter_id = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_ACL_FLD_STATS_PACKET, field_id))
    {
        int32 ret;
        p_fea_acl->stats_packet = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_ACL_FLD_STATS_BYTE, field_id))
    {
        int32 ret;
        p_fea_acl->stats_byte = cdb_uint64_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_fea_acl_table_parser(char** array, char* key_value,tbl_fea_acl_t *p_fea_acl)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_ACL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_fea_acl_key_str2val(key_value, p_fea_acl));

    for(i=1; i<TBL_FEA_ACL_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_fea_acl_field_value_parser( array[j++], i, p_fea_acl));
    }

    return PM_E_NONE;
}

int32
tbl_fea_acl_dump_one(tbl_fea_acl_t *p_fea_acl, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_ACL);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_FEA_ACL_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_FEA_ACL_FLD_KEY].name,
            p_fea_acl->key.acl_id);
    }
    if (FLD_MATCH(TBL_FEA_ACL_FLD_IS_MAC_ACL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_ACL_FLD_IS_MAC_ACL].name,
            p_fea_acl->is_mac_acl);
    }
    if (FLD_MATCH(TBL_FEA_ACL_FLD_SAI_ACL_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_FEA_ACL_FLD_SAI_ACL_ID].name,
            p_fea_acl->sai_acl_id);
    }
    if (FLD_MATCH(TBL_FEA_ACL_FLD_SAI_ACL_ID_IPV4, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_FEA_ACL_FLD_SAI_ACL_ID_IPV4].name,
            p_fea_acl->sai_acl_id_ipv4);
    }
    if (FLD_MATCH(TBL_FEA_ACL_FLD_SAI_ACL_ID_IPV6, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_FEA_ACL_FLD_SAI_ACL_ID_IPV6].name,
            p_fea_acl->sai_acl_id_ipv6);
    }
    if (FLD_MATCH(TBL_FEA_ACL_FLD_COUNTER_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_FEA_ACL_FLD_COUNTER_ID].name,
            p_fea_acl->counter_id);
    }
    if (FLD_MATCH(TBL_FEA_ACL_FLD_STATS_PACKET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_FEA_ACL_FLD_STATS_PACKET].name,
            p_fea_acl->stats_packet);
    }
    if (FLD_MATCH(TBL_FEA_ACL_FLD_STATS_BYTE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_FEA_ACL_FLD_STATS_BYTE].name,
            p_fea_acl->stats_byte);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_fea_acl_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_fea_acl_master_t *p_master = _g_p_tbl_fea_acl_master;
    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    ctclib_hash_iterate2(p_master->fea_acl_hash, tbl_hash_iter_adpt_fn, fn, pargs);
    return PM_E_NONE;
}

tbl_fea_acl_master_t*
tbl_fea_acl_get_master()
{
    return _g_p_tbl_fea_acl_master;
}

tbl_fea_acl_master_t*
tbl_fea_acl_init_fea_acl()
{
    _g_p_tbl_fea_acl_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_fea_acl_master_t));
    _g_p_tbl_fea_acl_master->fea_acl_hash = ctclib_hash_create(_tbl_fea_acl_hash_make, _tbl_fea_acl_hash_cmp);
    return _g_p_tbl_fea_acl_master;
}

