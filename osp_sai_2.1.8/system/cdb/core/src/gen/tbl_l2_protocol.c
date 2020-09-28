
#include "proto.h"
#include "gen/tbl_l2_protocol_define.h"
#include "gen/tbl_l2_protocol.h"
#include "cdb_data_cmp.h"

tbl_l2_protocol_master_t *_g_p_tbl_l2_protocol_master = NULL;

static uint32
_tbl_l2_protocol_hash_make(tbl_l2_protocol_t *p_l2_protocol)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_l2_protocol->key;
    for (index = 0; index < sizeof(p_l2_protocol->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_l2_protocol_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_l2_protocol_t *p_l2_protocol1 = (tbl_l2_protocol_t*)p_arg1;
    tbl_l2_protocol_t *p_l2_protocol2 = (tbl_l2_protocol_t*)p_arg2;

    if (0 == sal_memcmp(&p_l2_protocol1->key, &p_l2_protocol2->key, sizeof(tbl_l2_protocol_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_l2_protocol_add_l2_protocol_sync(tbl_l2_protocol_t *p_l2_protocol, uint32 sync)
{
    tbl_l2_protocol_master_t *p_master = _g_p_tbl_l2_protocol_master;
    tbl_l2_protocol_t *p_db_l2_protocol = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_l2_protocol_get_l2_protocol(&p_l2_protocol->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_l2_protocol = XCALLOC(MEM_TBL_L2_PROTOCOL, sizeof(tbl_l2_protocol_t));
    if (NULL == p_db_l2_protocol)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_l2_protocol, p_l2_protocol, sizeof(tbl_l2_protocol_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->l2_protocol_hash, (void*)p_db_l2_protocol, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->l2_protocol_list, p_db_l2_protocol);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_L2_PROTOCOL, p_db_l2_protocol);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_L2_PROTOCOL, p_db_l2_protocol);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_l2_protocol_del_l2_protocol_sync(tbl_l2_protocol_key_t *p_l2_protocol_key, uint32 sync)
{
    tbl_l2_protocol_master_t *p_master = _g_p_tbl_l2_protocol_master;
    tbl_l2_protocol_t *p_db_l2_protocol = NULL;

    /* 1. lookup entry exist */
    p_db_l2_protocol = tbl_l2_protocol_get_l2_protocol(p_l2_protocol_key);
    if (NULL == p_db_l2_protocol)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_L2_PROTOCOL, p_db_l2_protocol);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_L2_PROTOCOL, p_db_l2_protocol);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->l2_protocol_hash, (void*)p_db_l2_protocol);
    ctclib_slistnode_delete(p_master->l2_protocol_list, p_db_l2_protocol);

    /* 4. free db entry */
    XFREE(MEM_TBL_L2_PROTOCOL, p_db_l2_protocol);

    return PM_E_NONE;
}

int32
tbl_l2_protocol_set_l2_protocol_field_sync(tbl_l2_protocol_t *p_l2_protocol, tbl_l2_protocol_field_id_t field_id, uint32 sync)
{
    tbl_l2_protocol_t *p_db_l2_protocol = NULL;

    /* 1. lookup entry exist */
    p_db_l2_protocol = tbl_l2_protocol_get_l2_protocol(&p_l2_protocol->key);
    if (NULL == p_db_l2_protocol)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_L2_PROTOCOL_FLD_PROTOCOL_MAC:
        sal_memcpy(p_db_l2_protocol->protocol_mac, p_l2_protocol->protocol_mac, sizeof(p_l2_protocol->protocol_mac));
        break;

    case TBL_L2_PROTOCOL_FLD_GROUP_MAC:
        sal_memcpy(p_db_l2_protocol->group_mac, p_l2_protocol->group_mac, sizeof(p_l2_protocol->group_mac));
        break;

    case TBL_L2_PROTOCOL_FLD_IS_STANDARD_PRO:
        p_db_l2_protocol->is_standard_pro = p_l2_protocol->is_standard_pro;
        break;

    case TBL_L2_PROTOCOL_FLD_PROTOCOL_INDEX:
        p_db_l2_protocol->protocol_index = p_l2_protocol->protocol_index;
        break;

    case TBL_L2_PROTOCOL_FLD_MAX:
        sal_memcpy(p_db_l2_protocol, p_l2_protocol, sizeof(tbl_l2_protocol_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_L2_PROTOCOL, field_id, p_db_l2_protocol);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_L2_PROTOCOL, field_id, p_db_l2_protocol);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_l2_protocol_add_l2_protocol(tbl_l2_protocol_t *p_l2_protocol)
{
    return tbl_l2_protocol_add_l2_protocol_sync(p_l2_protocol, TRUE);
}

int32
tbl_l2_protocol_del_l2_protocol(tbl_l2_protocol_key_t *p_l2_protocol_key)
{
    return tbl_l2_protocol_del_l2_protocol_sync(p_l2_protocol_key, TRUE);
}

int32
tbl_l2_protocol_set_l2_protocol_field(tbl_l2_protocol_t *p_l2_protocol, tbl_l2_protocol_field_id_t field_id)
{
    return tbl_l2_protocol_set_l2_protocol_field_sync(p_l2_protocol, field_id, TRUE);
}

tbl_l2_protocol_t*
tbl_l2_protocol_get_l2_protocol(tbl_l2_protocol_key_t *p_l2_protocol_key)
{
    tbl_l2_protocol_master_t *p_master = _g_p_tbl_l2_protocol_master;
    tbl_l2_protocol_t lkp;

    sal_memcpy(&lkp.key, p_l2_protocol_key, sizeof(tbl_l2_protocol_key_t));
    return ctclib_hash_lookup(p_master->l2_protocol_hash, &lkp);
}

char*
tbl_l2_protocol_key_val2str(tbl_l2_protocol_t *p_l2_protocol, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_l2_protocol->key.name);
    return str;
}

int32
tbl_l2_protocol_key_str2val(char *str, tbl_l2_protocol_t *p_l2_protocol)
{
    int32 ret = 0;

    sal_strcpy(p_l2_protocol->key.name, str);
    return ret;
}

char*
tbl_l2_protocol_key_name_dump(tbl_l2_protocol_t *p_l2_protocol, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_L2_PROTOCOL);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_l2_protocol->key.name);
    return str;
}

char*
tbl_l2_protocol_key_value_dump(tbl_l2_protocol_t *p_l2_protocol, char *str)
{
    sal_sprintf(str, "%s", p_l2_protocol->key.name);
    return str;
}

char*
tbl_l2_protocol_field_name_dump(tbl_l2_protocol_t *p_l2_protocol, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_L2_PROTOCOL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_L2_PROTOCOL_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_l2_protocol_field_value_dump(tbl_l2_protocol_t *p_l2_protocol, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_L2_PROTOCOL_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_l2_protocol->key.name);
    }
    if (FLD_MATCH(TBL_L2_PROTOCOL_FLD_PROTOCOL_MAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_l2_protocol->protocol_mac);
    }
    if (FLD_MATCH(TBL_L2_PROTOCOL_FLD_GROUP_MAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_l2_protocol->group_mac);
    }
    if (FLD_MATCH(TBL_L2_PROTOCOL_FLD_IS_STANDARD_PRO, field_id))
    {
        sal_sprintf(str, "%u", p_l2_protocol->is_standard_pro);
    }
    if (FLD_MATCH(TBL_L2_PROTOCOL_FLD_PROTOCOL_INDEX, field_id))
    {
        sal_sprintf(str, "%u", p_l2_protocol->protocol_index);
    }
    return str;
}

char**
tbl_l2_protocol_table_dump(tbl_l2_protocol_t *p_l2_protocol, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_l2_protocol_key_name_dump(p_l2_protocol, buf));
    for(i=1; i<TBL_L2_PROTOCOL_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_l2_protocol_field_name_dump(p_l2_protocol, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_l2_protocol_field_value_dump(p_l2_protocol, i, buf));
    }
    return str;
}

int32
tbl_l2_protocol_field_value_parser(char *str, int32 field_id, tbl_l2_protocol_t *p_l2_protocol)
{
    if (FLD_MATCH(TBL_L2_PROTOCOL_FLD_KEY, field_id))
    {
        sal_strcpy(p_l2_protocol->key.name, str);
    }
    if (FLD_MATCH(TBL_L2_PROTOCOL_FLD_PROTOCOL_MAC, field_id))
    {
        cdb_mac_addr_str2val(p_l2_protocol->protocol_mac, str);
    }
    if (FLD_MATCH(TBL_L2_PROTOCOL_FLD_GROUP_MAC, field_id))
    {
        cdb_mac_addr_str2val(p_l2_protocol->group_mac, str);
    }
    if (FLD_MATCH(TBL_L2_PROTOCOL_FLD_IS_STANDARD_PRO, field_id))
    {
        int32 ret;
        p_l2_protocol->is_standard_pro = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_L2_PROTOCOL_FLD_PROTOCOL_INDEX, field_id))
    {
        int32 ret;
        p_l2_protocol->protocol_index = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_l2_protocol_table_parser(char** array, char* key_value,tbl_l2_protocol_t *p_l2_protocol)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_L2_PROTOCOL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_l2_protocol_key_str2val(key_value, p_l2_protocol));

    for(i=1; i<TBL_L2_PROTOCOL_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_l2_protocol_field_value_parser( array[j++], i, p_l2_protocol));
    }

    return PM_E_NONE;
}

int32
tbl_l2_protocol_dump_one(tbl_l2_protocol_t *p_l2_protocol, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_L2_PROTOCOL);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_L2_PROTOCOL_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_L2_PROTOCOL_FLD_KEY].name,
            p_l2_protocol->key.name);
    }
    if (FLD_MATCH(TBL_L2_PROTOCOL_FLD_PROTOCOL_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_L2_PROTOCOL_FLD_PROTOCOL_MAC].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_l2_protocol->protocol_mac));
    }
    if (FLD_MATCH(TBL_L2_PROTOCOL_FLD_GROUP_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_L2_PROTOCOL_FLD_GROUP_MAC].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_l2_protocol->group_mac));
    }
    if (FLD_MATCH(TBL_L2_PROTOCOL_FLD_IS_STANDARD_PRO, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_L2_PROTOCOL_FLD_IS_STANDARD_PRO].name,
            p_l2_protocol->is_standard_pro);
    }
    if (FLD_MATCH(TBL_L2_PROTOCOL_FLD_PROTOCOL_INDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_L2_PROTOCOL_FLD_PROTOCOL_INDEX].name,
            p_l2_protocol->protocol_index);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_l2_protocol_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_l2_protocol_master_t *p_master = _g_p_tbl_l2_protocol_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_l2_protocol_t *p_db_l2_protocol = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->l2_protocol_list, p_db_l2_protocol, listnode, next)
    {
        rc |= fn(p_db_l2_protocol, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_l2_protocol_master_t*
tbl_l2_protocol_get_master()
{
    return _g_p_tbl_l2_protocol_master;
}

tbl_l2_protocol_master_t*
tbl_l2_protocol_init_l2_protocol()
{
    _g_p_tbl_l2_protocol_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_l2_protocol_master_t));
    _g_p_tbl_l2_protocol_master->l2_protocol_hash = ctclib_hash_create(_tbl_l2_protocol_hash_make, _tbl_l2_protocol_hash_cmp);
    _g_p_tbl_l2_protocol_master->l2_protocol_list = ctclib_slist_create(tbl_l2_protocol_cmp, NULL);
    return _g_p_tbl_l2_protocol_master;
}

