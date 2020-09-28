
#include "proto.h"
#include "gen/tbl_ns_route_ip_define.h"
#include "gen/tbl_ns_route_ip.h"
#include "cdb_data_cmp.h"

tbl_ns_route_ip_master_t *_g_p_tbl_ns_route_ip_master = NULL;

static uint32
_tbl_ns_route_ip_hash_make(tbl_ns_route_ip_t *p_ns_route_ip)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_ns_route_ip->key;
    for (index = 0; index < sizeof(p_ns_route_ip->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_ns_route_ip_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_ns_route_ip_t *p_ns_route_ip1 = (tbl_ns_route_ip_t*)p_arg1;
    tbl_ns_route_ip_t *p_ns_route_ip2 = (tbl_ns_route_ip_t*)p_arg2;

    if (0 == sal_memcmp(&p_ns_route_ip1->key, &p_ns_route_ip2->key, sizeof(tbl_ns_route_ip_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_ns_route_ip_add_ns_route_ip_sync(tbl_ns_route_ip_t *p_ns_route_ip, uint32 sync)
{
    tbl_ns_route_ip_master_t *p_master = _g_p_tbl_ns_route_ip_master;
    tbl_ns_route_ip_t *p_db_ns_route_ip = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_ns_route_ip_get_ns_route_ip(&p_ns_route_ip->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_ns_route_ip = XCALLOC(MEM_TBL_NS_ROUTE_IP, sizeof(tbl_ns_route_ip_t));
    if (NULL == p_db_ns_route_ip)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_ns_route_ip, p_ns_route_ip, sizeof(tbl_ns_route_ip_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->ns_route_ip_hash, (void*)p_db_ns_route_ip, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->ns_route_ip_list, p_db_ns_route_ip);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_NS_ROUTE_IP, p_db_ns_route_ip);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_NS_ROUTE_IP, p_db_ns_route_ip);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ns_route_ip_del_ns_route_ip_sync(tbl_ns_route_ip_key_t *p_ns_route_ip_key, uint32 sync)
{
    tbl_ns_route_ip_master_t *p_master = _g_p_tbl_ns_route_ip_master;
    tbl_ns_route_ip_t *p_db_ns_route_ip = NULL;

    /* 1. lookup entry exist */
    p_db_ns_route_ip = tbl_ns_route_ip_get_ns_route_ip(p_ns_route_ip_key);
    if (NULL == p_db_ns_route_ip)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_NS_ROUTE_IP, p_db_ns_route_ip);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_NS_ROUTE_IP, p_db_ns_route_ip);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->ns_route_ip_hash, (void*)p_db_ns_route_ip);
    ctclib_slistnode_delete(p_master->ns_route_ip_list, p_db_ns_route_ip);

    /* 4. free db entry */
    XFREE(MEM_TBL_NS_ROUTE_IP, p_db_ns_route_ip);

    return PM_E_NONE;
}

int32
tbl_ns_route_ip_set_ns_route_ip_field_sync(tbl_ns_route_ip_t *p_ns_route_ip, tbl_ns_route_ip_field_id_t field_id, uint32 sync)
{
    tbl_ns_route_ip_t *p_db_ns_route_ip = NULL;

    /* 1. lookup entry exist */
    p_db_ns_route_ip = tbl_ns_route_ip_get_ns_route_ip(&p_ns_route_ip->key);
    if (NULL == p_db_ns_route_ip)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_NS_ROUTE_IP_FLD_IS_INBAND:
        p_db_ns_route_ip->is_inband = p_ns_route_ip->is_inband;
        break;

    case TBL_NS_ROUTE_IP_FLD_CNT:
        p_db_ns_route_ip->cnt = p_ns_route_ip->cnt;
        break;

    case TBL_NS_ROUTE_IP_FLD_MAX:
        sal_memcpy(p_db_ns_route_ip, p_ns_route_ip, sizeof(tbl_ns_route_ip_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_NS_ROUTE_IP, field_id, p_db_ns_route_ip);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_NS_ROUTE_IP, field_id, p_db_ns_route_ip);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ns_route_ip_add_ns_route_ip(tbl_ns_route_ip_t *p_ns_route_ip)
{
    return tbl_ns_route_ip_add_ns_route_ip_sync(p_ns_route_ip, TRUE);
}

int32
tbl_ns_route_ip_del_ns_route_ip(tbl_ns_route_ip_key_t *p_ns_route_ip_key)
{
    return tbl_ns_route_ip_del_ns_route_ip_sync(p_ns_route_ip_key, TRUE);
}

int32
tbl_ns_route_ip_set_ns_route_ip_field(tbl_ns_route_ip_t *p_ns_route_ip, tbl_ns_route_ip_field_id_t field_id)
{
    return tbl_ns_route_ip_set_ns_route_ip_field_sync(p_ns_route_ip, field_id, TRUE);
}

tbl_ns_route_ip_t*
tbl_ns_route_ip_get_ns_route_ip(tbl_ns_route_ip_key_t *p_ns_route_ip_key)
{
    tbl_ns_route_ip_master_t *p_master = _g_p_tbl_ns_route_ip_master;
    tbl_ns_route_ip_t lkp;

    sal_memcpy(&lkp.key, p_ns_route_ip_key, sizeof(tbl_ns_route_ip_key_t));
    return ctclib_hash_lookup(p_master->ns_route_ip_hash, &lkp);
}

char*
tbl_ns_route_ip_key_val2str(tbl_ns_route_ip_t *p_ns_route_ip, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_ns_route_ip->key.ip));
    return str;
}

int32
tbl_ns_route_ip_key_str2val(char *str, tbl_ns_route_ip_t *p_ns_route_ip)
{
    int32 ret = 0;

    ret = cdb_addr_str2val(&p_ns_route_ip->key.ip, str, 0);
    return ret;
}

char*
tbl_ns_route_ip_key_name_dump(tbl_ns_route_ip_t *p_ns_route_ip, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_NS_ROUTE_IP);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_ns_route_ip->key.ip));
    return str;
}

char*
tbl_ns_route_ip_key_value_dump(tbl_ns_route_ip_t *p_ns_route_ip, char *str)
{

    cdb_addr_val2str(str, MAX_CMD_STR_LEN, &p_ns_route_ip->key.ip);
    return str;
}

char*
tbl_ns_route_ip_field_name_dump(tbl_ns_route_ip_t *p_ns_route_ip, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_NS_ROUTE_IP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_NS_ROUTE_IP_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_ns_route_ip_field_value_dump(tbl_ns_route_ip_t *p_ns_route_ip, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_NS_ROUTE_IP_FLD_KEY, field_id))
    {
        cdb_addr_val2str(str, MAX_CMD_STR_LEN, &p_ns_route_ip->key.ip);
    }
    if (FLD_MATCH(TBL_NS_ROUTE_IP_FLD_IS_INBAND, field_id))
    {
        sal_sprintf(str, "%u", p_ns_route_ip->is_inband);
    }
    if (FLD_MATCH(TBL_NS_ROUTE_IP_FLD_CNT, field_id))
    {
        sal_sprintf(str, "%d", p_ns_route_ip->cnt);
    }
    return str;
}

char**
tbl_ns_route_ip_table_dump(tbl_ns_route_ip_t *p_ns_route_ip, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_ns_route_ip_key_name_dump(p_ns_route_ip, buf));
    for(i=1; i<TBL_NS_ROUTE_IP_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_ns_route_ip_field_name_dump(p_ns_route_ip, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_ns_route_ip_field_value_dump(p_ns_route_ip, i, buf));
    }
    return str;
}

int32
tbl_ns_route_ip_field_value_parser(char *str, int32 field_id, tbl_ns_route_ip_t *p_ns_route_ip)
{
    if (FLD_MATCH(TBL_NS_ROUTE_IP_FLD_KEY, field_id))
    {
        cdb_addr_str2val(&p_ns_route_ip->key.ip, str, 0);
    }
    if (FLD_MATCH(TBL_NS_ROUTE_IP_FLD_IS_INBAND, field_id))
    {
        int32 ret;
        p_ns_route_ip->is_inband = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NS_ROUTE_IP_FLD_CNT, field_id))
    {
    }
    return PM_E_NONE;
}

int32
tbl_ns_route_ip_table_parser(char** array, char* key_value,tbl_ns_route_ip_t *p_ns_route_ip)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_NS_ROUTE_IP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_ns_route_ip_key_str2val(key_value, p_ns_route_ip));

    for(i=1; i<TBL_NS_ROUTE_IP_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_ns_route_ip_field_value_parser( array[j++], i, p_ns_route_ip));
    }

    return PM_E_NONE;
}

int32
tbl_ns_route_ip_dump_one(tbl_ns_route_ip_t *p_ns_route_ip, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_NS_ROUTE_IP);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_NS_ROUTE_IP_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_NS_ROUTE_IP_FLD_KEY].name, 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_ns_route_ip->key.ip));
    }
    if (FLD_MATCH(TBL_NS_ROUTE_IP_FLD_IS_INBAND, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NS_ROUTE_IP_FLD_IS_INBAND].name,
            p_ns_route_ip->is_inband);
    }
    if (FLD_MATCH(TBL_NS_ROUTE_IP_FLD_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_NS_ROUTE_IP_FLD_CNT].name,
            p_ns_route_ip->cnt);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_ns_route_ip_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_ns_route_ip_master_t *p_master = _g_p_tbl_ns_route_ip_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_ns_route_ip_t *p_db_ns_route_ip = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->ns_route_ip_list, p_db_ns_route_ip, listnode, next)
    {
        rc |= fn(p_db_ns_route_ip, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_ns_route_ip_master_t*
tbl_ns_route_ip_get_master()
{
    return _g_p_tbl_ns_route_ip_master;
}

tbl_ns_route_ip_master_t*
tbl_ns_route_ip_init_ns_route_ip()
{
    _g_p_tbl_ns_route_ip_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_ns_route_ip_master_t));
    _g_p_tbl_ns_route_ip_master->ns_route_ip_hash = ctclib_hash_create(_tbl_ns_route_ip_hash_make, _tbl_ns_route_ip_hash_cmp);
    _g_p_tbl_ns_route_ip_master->ns_route_ip_list = ctclib_slist_create(tbl_ns_route_ip_cmp, NULL);
    return _g_p_tbl_ns_route_ip_master;
}

