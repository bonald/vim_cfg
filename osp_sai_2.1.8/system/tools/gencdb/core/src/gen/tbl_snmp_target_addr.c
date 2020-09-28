
#include "proto.h"
#include "gen/tbl_snmp_target_addr_define.h"
#include "gen/tbl_snmp_target_addr.h"
#include "cdb_data_cmp.h"

tbl_snmp_target_addr_master_t *_g_p_tbl_snmp_target_addr_master = NULL;

static uint32
_tbl_snmp_target_addr_hash_make(tbl_snmp_target_addr_t *p_snmp_target_addr)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_snmp_target_addr->key;
    for (index = 0; index < sizeof(p_snmp_target_addr->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_snmp_target_addr_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_snmp_target_addr_t *p_snmp_target_addr1 = (tbl_snmp_target_addr_t*)p_arg1;
    tbl_snmp_target_addr_t *p_snmp_target_addr2 = (tbl_snmp_target_addr_t*)p_arg2;

    if (0 == sal_memcmp(&p_snmp_target_addr1->key, &p_snmp_target_addr2->key, sizeof(tbl_snmp_target_addr_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_snmp_target_addr_add_snmp_target_addr_sync(tbl_snmp_target_addr_t *p_snmp_target_addr, uint32 sync)
{
    tbl_snmp_target_addr_master_t *p_master = _g_p_tbl_snmp_target_addr_master;
    tbl_snmp_target_addr_t *p_db_snmp_target_addr = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_snmp_target_addr_get_snmp_target_addr(&p_snmp_target_addr->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_snmp_target_addr = XCALLOC(MEM_TBL_SNMP_TARGET_ADDR, sizeof(tbl_snmp_target_addr_t));
    if (NULL == p_db_snmp_target_addr)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_snmp_target_addr, p_snmp_target_addr, sizeof(tbl_snmp_target_addr_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->snmp_target_addr_hash, (void*)p_db_snmp_target_addr, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->snmp_target_addr_list, p_db_snmp_target_addr);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_SNMP_TARGET_ADDR, p_db_snmp_target_addr);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_SNMP_TARGET_ADDR, p_db_snmp_target_addr);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_snmp_target_addr_del_snmp_target_addr_sync(tbl_snmp_target_addr_key_t *p_snmp_target_addr_key, uint32 sync)
{
    tbl_snmp_target_addr_master_t *p_master = _g_p_tbl_snmp_target_addr_master;
    tbl_snmp_target_addr_t *p_db_snmp_target_addr = NULL;

    /* 1. lookup entry exist */
    p_db_snmp_target_addr = tbl_snmp_target_addr_get_snmp_target_addr(p_snmp_target_addr_key);
    if (NULL == p_db_snmp_target_addr)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_SNMP_TARGET_ADDR, p_db_snmp_target_addr);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_SNMP_TARGET_ADDR, p_db_snmp_target_addr);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->snmp_target_addr_hash, (void*)p_db_snmp_target_addr);
    ctclib_slistnode_delete(p_master->snmp_target_addr_list, p_db_snmp_target_addr);

    /* 4. free db entry */
    XFREE(MEM_TBL_SNMP_TARGET_ADDR, p_db_snmp_target_addr);

    return PM_E_NONE;
}

int32
tbl_snmp_target_addr_set_snmp_target_addr_field_sync(tbl_snmp_target_addr_t *p_snmp_target_addr, tbl_snmp_target_addr_field_id_t field_id, uint32 sync)
{
    tbl_snmp_target_addr_t *p_db_snmp_target_addr = NULL;

    /* 1. lookup entry exist */
    p_db_snmp_target_addr = tbl_snmp_target_addr_get_snmp_target_addr(&p_snmp_target_addr->key);
    if (NULL == p_db_snmp_target_addr)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_SNMP_TARGET_ADDR_FLD_FLAGS:
        p_db_snmp_target_addr->flags = p_snmp_target_addr->flags;
        break;

    case TBL_SNMP_TARGET_ADDR_FLD_PARA_NAME:
        sal_memcpy(p_db_snmp_target_addr->para_name, p_snmp_target_addr->para_name, sizeof(p_snmp_target_addr->para_name));
        break;

    case TBL_SNMP_TARGET_ADDR_FLD_IP_ADDR:
        sal_memcpy(&p_db_snmp_target_addr->ip_addr, &p_snmp_target_addr->ip_addr, sizeof(p_snmp_target_addr->ip_addr));
        break;

    case TBL_SNMP_TARGET_ADDR_FLD_UPD_PORT:
        p_db_snmp_target_addr->upd_port = p_snmp_target_addr->upd_port;
        break;

    case TBL_SNMP_TARGET_ADDR_FLD_TIMEOUT:
        p_db_snmp_target_addr->timeout = p_snmp_target_addr->timeout;
        break;

    case TBL_SNMP_TARGET_ADDR_FLD_RETRIES:
        p_db_snmp_target_addr->retries = p_snmp_target_addr->retries;
        break;

    case TBL_SNMP_TARGET_ADDR_FLD_MGMT_IF:
        p_db_snmp_target_addr->mgmt_if = p_snmp_target_addr->mgmt_if;
        break;

    case TBL_SNMP_TARGET_ADDR_FLD_TAG_LIST:
        sal_memcpy(p_db_snmp_target_addr->tag_list, p_snmp_target_addr->tag_list, sizeof(p_snmp_target_addr->tag_list));
        break;

    case TBL_SNMP_TARGET_ADDR_FLD_MAX:
        sal_memcpy(p_db_snmp_target_addr, p_snmp_target_addr, sizeof(tbl_snmp_target_addr_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_SNMP_TARGET_ADDR, field_id, p_db_snmp_target_addr);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_SNMP_TARGET_ADDR, field_id, p_db_snmp_target_addr);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_snmp_target_addr_add_snmp_target_addr(tbl_snmp_target_addr_t *p_snmp_target_addr)
{
    return tbl_snmp_target_addr_add_snmp_target_addr_sync(p_snmp_target_addr, TRUE);
}

int32
tbl_snmp_target_addr_del_snmp_target_addr(tbl_snmp_target_addr_key_t *p_snmp_target_addr_key)
{
    return tbl_snmp_target_addr_del_snmp_target_addr_sync(p_snmp_target_addr_key, TRUE);
}

int32
tbl_snmp_target_addr_set_snmp_target_addr_field(tbl_snmp_target_addr_t *p_snmp_target_addr, tbl_snmp_target_addr_field_id_t field_id)
{
    return tbl_snmp_target_addr_set_snmp_target_addr_field_sync(p_snmp_target_addr, field_id, TRUE);
}

tbl_snmp_target_addr_t*
tbl_snmp_target_addr_get_snmp_target_addr(tbl_snmp_target_addr_key_t *p_snmp_target_addr_key)
{
    tbl_snmp_target_addr_master_t *p_master = _g_p_tbl_snmp_target_addr_master;
    tbl_snmp_target_addr_t lkp;

    sal_memcpy(&lkp.key, p_snmp_target_addr_key, sizeof(tbl_snmp_target_addr_key_t));
    return ctclib_hash_lookup(p_master->snmp_target_addr_hash, &lkp);
}

char*
tbl_snmp_target_addr_key_val2str(tbl_snmp_target_addr_t *p_snmp_target_addr, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_snmp_target_addr->key.taraddr_name);
    return str;
}

int32
tbl_snmp_target_addr_key_str2val(char *str, tbl_snmp_target_addr_t *p_snmp_target_addr)
{
    int32 ret = 0;

    sal_strcpy(p_snmp_target_addr->key.taraddr_name, str);
    return ret;
}

char*
tbl_snmp_target_addr_key_name_dump(tbl_snmp_target_addr_t *p_snmp_target_addr, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_TARGET_ADDR);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_snmp_target_addr->key.taraddr_name);
    return str;
}

char*
tbl_snmp_target_addr_key_value_dump(tbl_snmp_target_addr_t *p_snmp_target_addr, char *str)
{
    sal_sprintf(str, "%s", p_snmp_target_addr->key.taraddr_name);
    return str;
}

char*
tbl_snmp_target_addr_field_name_dump(tbl_snmp_target_addr_t *p_snmp_target_addr, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_TARGET_ADDR);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_SNMP_TARGET_ADDR_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_snmp_target_addr_field_value_dump(tbl_snmp_target_addr_t *p_snmp_target_addr, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_SNMP_TARGET_ADDR_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_snmp_target_addr->key.taraddr_name);
    }
    if (FLD_MATCH(TBL_SNMP_TARGET_ADDR_FLD_FLAGS, field_id))
    {
        sal_sprintf(str, "%u", p_snmp_target_addr->flags);
    }
    if (FLD_MATCH(TBL_SNMP_TARGET_ADDR_FLD_PARA_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_snmp_target_addr->para_name);
    }
    if (FLD_MATCH(TBL_SNMP_TARGET_ADDR_FLD_IP_ADDR, field_id))
    {
        cdb_addr_val2str(str, MAX_CMD_STR_LEN, &p_snmp_target_addr->ip_addr);
    }
    if (FLD_MATCH(TBL_SNMP_TARGET_ADDR_FLD_UPD_PORT, field_id))
    {
        sal_sprintf(str, "%u", p_snmp_target_addr->upd_port);
    }
    if (FLD_MATCH(TBL_SNMP_TARGET_ADDR_FLD_TIMEOUT, field_id))
    {
        sal_sprintf(str, "%u", p_snmp_target_addr->timeout);
    }
    if (FLD_MATCH(TBL_SNMP_TARGET_ADDR_FLD_RETRIES, field_id))
    {
        sal_sprintf(str, "%u", p_snmp_target_addr->retries);
    }
    if (FLD_MATCH(TBL_SNMP_TARGET_ADDR_FLD_MGMT_IF, field_id))
    {
        sal_sprintf(str, "%u", p_snmp_target_addr->mgmt_if);
    }
    if (FLD_MATCH(TBL_SNMP_TARGET_ADDR_FLD_TAG_LIST, field_id))
    {
        sal_sprintf(str, "%s", p_snmp_target_addr->tag_list);
    }
    return str;
}

char**
tbl_snmp_target_addr_table_dump(tbl_snmp_target_addr_t *p_snmp_target_addr, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_snmp_target_addr_key_name_dump(p_snmp_target_addr, buf));
    for(i=1; i<TBL_SNMP_TARGET_ADDR_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_snmp_target_addr_field_name_dump(p_snmp_target_addr, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_snmp_target_addr_field_value_dump(p_snmp_target_addr, i, buf));
    }
    return str;
}

int32
tbl_snmp_target_addr_field_value_parser(char *str, int32 field_id, tbl_snmp_target_addr_t *p_snmp_target_addr)
{
    if (FLD_MATCH(TBL_SNMP_TARGET_ADDR_FLD_KEY, field_id))
    {
        sal_strcpy(p_snmp_target_addr->key.taraddr_name, str);
    }
    if (FLD_MATCH(TBL_SNMP_TARGET_ADDR_FLD_FLAGS, field_id))
    {
        int32 ret;
        p_snmp_target_addr->flags = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SNMP_TARGET_ADDR_FLD_PARA_NAME, field_id))
    {
        sal_strcpy(p_snmp_target_addr->para_name, str);
    }
    if (FLD_MATCH(TBL_SNMP_TARGET_ADDR_FLD_IP_ADDR, field_id))
    {
        cdb_addr_str2val(&p_snmp_target_addr->ip_addr, str, 0);
    }
    if (FLD_MATCH(TBL_SNMP_TARGET_ADDR_FLD_UPD_PORT, field_id))
    {
        int32 ret;
        p_snmp_target_addr->upd_port = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SNMP_TARGET_ADDR_FLD_TIMEOUT, field_id))
    {
        int32 ret;
        p_snmp_target_addr->timeout = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SNMP_TARGET_ADDR_FLD_RETRIES, field_id))
    {
        int32 ret;
        p_snmp_target_addr->retries = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SNMP_TARGET_ADDR_FLD_MGMT_IF, field_id))
    {
        int32 ret;
        p_snmp_target_addr->mgmt_if = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SNMP_TARGET_ADDR_FLD_TAG_LIST, field_id))
    {
        sal_strcpy(p_snmp_target_addr->tag_list, str);
    }
    return PM_E_NONE;
}

int32
tbl_snmp_target_addr_table_parser(char** array, char* key_value,tbl_snmp_target_addr_t *p_snmp_target_addr)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_TARGET_ADDR);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_snmp_target_addr_key_str2val(key_value, p_snmp_target_addr));

    for(i=1; i<TBL_SNMP_TARGET_ADDR_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_snmp_target_addr_field_value_parser( array[j++], i, p_snmp_target_addr));
    }

    return PM_E_NONE;
}

int32
tbl_snmp_target_addr_dump_one(tbl_snmp_target_addr_t *p_snmp_target_addr, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_TARGET_ADDR);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_SNMP_TARGET_ADDR_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SNMP_TARGET_ADDR_FLD_KEY].name,
            p_snmp_target_addr->key.taraddr_name);
    }
    if (FLD_MATCH(TBL_SNMP_TARGET_ADDR_FLD_FLAGS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SNMP_TARGET_ADDR_FLD_FLAGS].name,
            p_snmp_target_addr->flags);
    }
    if (FLD_MATCH(TBL_SNMP_TARGET_ADDR_FLD_PARA_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SNMP_TARGET_ADDR_FLD_PARA_NAME].name,
            p_snmp_target_addr->para_name);
    }
    if (FLD_MATCH(TBL_SNMP_TARGET_ADDR_FLD_IP_ADDR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SNMP_TARGET_ADDR_FLD_IP_ADDR].name, 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_snmp_target_addr->ip_addr));
    }
    if (FLD_MATCH(TBL_SNMP_TARGET_ADDR_FLD_UPD_PORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SNMP_TARGET_ADDR_FLD_UPD_PORT].name,
            p_snmp_target_addr->upd_port);
    }
    if (FLD_MATCH(TBL_SNMP_TARGET_ADDR_FLD_TIMEOUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SNMP_TARGET_ADDR_FLD_TIMEOUT].name,
            p_snmp_target_addr->timeout);
    }
    if (FLD_MATCH(TBL_SNMP_TARGET_ADDR_FLD_RETRIES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SNMP_TARGET_ADDR_FLD_RETRIES].name,
            p_snmp_target_addr->retries);
    }
    if (FLD_MATCH(TBL_SNMP_TARGET_ADDR_FLD_MGMT_IF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SNMP_TARGET_ADDR_FLD_MGMT_IF].name,
            p_snmp_target_addr->mgmt_if);
    }
    if (FLD_MATCH(TBL_SNMP_TARGET_ADDR_FLD_TAG_LIST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SNMP_TARGET_ADDR_FLD_TAG_LIST].name,
            p_snmp_target_addr->tag_list);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_snmp_target_addr_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_snmp_target_addr_master_t *p_master = _g_p_tbl_snmp_target_addr_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_snmp_target_addr_t *p_db_snmp_target_addr = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->snmp_target_addr_list, p_db_snmp_target_addr, listnode, next)
    {
        rc |= fn(p_db_snmp_target_addr, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_snmp_target_addr_master_t*
tbl_snmp_target_addr_get_master()
{
    return _g_p_tbl_snmp_target_addr_master;
}

tbl_snmp_target_addr_master_t*
tbl_snmp_target_addr_init_snmp_target_addr()
{
    _g_p_tbl_snmp_target_addr_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_snmp_target_addr_master_t));
    _g_p_tbl_snmp_target_addr_master->snmp_target_addr_hash = ctclib_hash_create(_tbl_snmp_target_addr_hash_make, _tbl_snmp_target_addr_hash_cmp);
    _g_p_tbl_snmp_target_addr_master->snmp_target_addr_list = ctclib_slist_create(tbl_snmp_target_addr_cmp, NULL);
    return _g_p_tbl_snmp_target_addr_master;
}

int32
tbl_snmp_target_addr_num_get()
{
    tbl_snmp_target_addr_master_t *p_master = _g_p_tbl_snmp_target_addr_master;
    int32 count = 0;

    if (NULL == p_master)
    {
        return 0;
    }

    count = p_master->snmp_target_addr_list->count;
    return count;
}

tbl_snmp_target_addr_t *
tbl_snmp_target_addr_get_by_param_name(char *param_name)
{
    tbl_snmp_target_addr_master_t *p_master = _g_p_tbl_snmp_target_addr_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_snmp_target_addr_t *p_db_snmp_target_addr = NULL;

    if (NULL == p_master)
    {
        return NULL;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->snmp_target_addr_list, p_db_snmp_target_addr, listnode, next)
    {
        if (0 == sal_strcmp(p_db_snmp_target_addr->para_name, param_name))
        {
            return p_db_snmp_target_addr;
        }
    }
    return NULL;
}


