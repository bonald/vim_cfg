
#include "proto.h"
#include "gen/tbl_ns_route_define.h"
#include "gen/tbl_ns_route.h"
#include "cdb_data_cmp.h"

tbl_ns_route_master_t *_g_p_tbl_ns_route_master = NULL;

static uint32
_tbl_ns_route_hash_make(tbl_ns_route_t *p_ns_route)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_ns_route->key;
    for (index = 0; index < sizeof(p_ns_route->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_ns_route_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_ns_route_t *p_ns_route1 = (tbl_ns_route_t*)p_arg1;
    tbl_ns_route_t *p_ns_route2 = (tbl_ns_route_t*)p_arg2;

    if (0 == sal_memcmp(&p_ns_route1->key, &p_ns_route2->key, sizeof(tbl_ns_route_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_ns_route_add_ns_route_sync(tbl_ns_route_t *p_ns_route, uint32 sync)
{
    tbl_ns_route_master_t *p_master = _g_p_tbl_ns_route_master;
    tbl_ns_route_t *p_db_ns_route = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_ns_route_get_ns_route(&p_ns_route->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_ns_route = XCALLOC(MEM_TBL_NS_ROUTE, sizeof(tbl_ns_route_t));
    if (NULL == p_db_ns_route)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_ns_route, p_ns_route, sizeof(tbl_ns_route_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->ns_route_hash, (void*)p_db_ns_route, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->ns_route_list, p_db_ns_route);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_NS_ROUTE, p_db_ns_route);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_NS_ROUTE, p_db_ns_route);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ns_route_del_ns_route_sync(tbl_ns_route_key_t *p_ns_route_key, uint32 sync)
{
    tbl_ns_route_master_t *p_master = _g_p_tbl_ns_route_master;
    tbl_ns_route_t *p_db_ns_route = NULL;

    /* 1. lookup entry exist */
    p_db_ns_route = tbl_ns_route_get_ns_route(p_ns_route_key);
    if (NULL == p_db_ns_route)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_NS_ROUTE, p_db_ns_route);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_NS_ROUTE, p_db_ns_route);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->ns_route_hash, (void*)p_db_ns_route);
    ctclib_slistnode_delete(p_master->ns_route_list, p_db_ns_route);

    /* 4. free db entry */
    XFREE(MEM_TBL_NS_ROUTE, p_db_ns_route);

    return PM_E_NONE;
}

int32
tbl_ns_route_set_ns_route_field_sync(tbl_ns_route_t *p_ns_route, tbl_ns_route_field_id_t field_id, uint32 sync)
{
    tbl_ns_route_t *p_db_ns_route = NULL;

    /* 1. lookup entry exist */
    p_db_ns_route = tbl_ns_route_get_ns_route(&p_ns_route->key);
    if (NULL == p_db_ns_route)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_NS_ROUTE_FLD_FLAG:
        p_db_ns_route->flag = p_ns_route->flag;
        break;

    case TBL_NS_ROUTE_FLD_IS_INBAND:
        p_db_ns_route->is_inband = p_ns_route->is_inband;
        break;

    case TBL_NS_ROUTE_FLD_IFNAME:
        sal_memcpy(p_db_ns_route->ifname, p_ns_route->ifname, sizeof(p_ns_route->ifname));
        break;

    case TBL_NS_ROUTE_FLD_CNT:
        p_db_ns_route->cnt = p_ns_route->cnt;
        break;

    case TBL_NS_ROUTE_FLD_MAX:
        sal_memcpy(p_db_ns_route, p_ns_route, sizeof(tbl_ns_route_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_NS_ROUTE, field_id, p_db_ns_route);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_NS_ROUTE, field_id, p_db_ns_route);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ns_route_add_ns_route(tbl_ns_route_t *p_ns_route)
{
    return tbl_ns_route_add_ns_route_sync(p_ns_route, TRUE);
}

int32
tbl_ns_route_del_ns_route(tbl_ns_route_key_t *p_ns_route_key)
{
    return tbl_ns_route_del_ns_route_sync(p_ns_route_key, TRUE);
}

int32
tbl_ns_route_set_ns_route_field(tbl_ns_route_t *p_ns_route, tbl_ns_route_field_id_t field_id)
{
    return tbl_ns_route_set_ns_route_field_sync(p_ns_route, field_id, TRUE);
}

tbl_ns_route_t*
tbl_ns_route_get_ns_route(tbl_ns_route_key_t *p_ns_route_key)
{
    tbl_ns_route_master_t *p_master = _g_p_tbl_ns_route_master;
    tbl_ns_route_t lkp;

    sal_memcpy(&lkp.key, p_ns_route_key, sizeof(tbl_ns_route_key_t));
    return ctclib_hash_lookup(p_master->ns_route_hash, &lkp);
}

char*
tbl_ns_route_key_val2str(tbl_ns_route_t *p_ns_route, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_ns_route_key_val2str(buf, MAX_CMD_STR_LEN, &p_ns_route->key));
    return str;
}

int32
tbl_ns_route_key_str2val(char *str, tbl_ns_route_t *p_ns_route)
{
    int32 ret = 0;

    ret = cdb_ns_route_key_str2val(str, &p_ns_route->key);
    return ret;
}

char*
tbl_ns_route_key_name_dump(tbl_ns_route_t *p_ns_route, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_NS_ROUTE);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_ns_route_key_val2str(buf, MAX_CMD_STR_LEN, &p_ns_route->key));
    return str;
}

char*
tbl_ns_route_key_value_dump(tbl_ns_route_t *p_ns_route, char *str)
{

    cdb_ns_route_key_val2str(str, MAX_CMD_STR_LEN, &p_ns_route->key);
    return str;
}

char*
tbl_ns_route_field_name_dump(tbl_ns_route_t *p_ns_route, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_NS_ROUTE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_NS_ROUTE_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_ns_route_field_value_dump(tbl_ns_route_t *p_ns_route, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_NS_ROUTE_FLD_KEY, field_id))
    {
        cdb_ns_route_key_val2str(str, MAX_CMD_STR_LEN, &p_ns_route->key);
    }
    if (FLD_MATCH(TBL_NS_ROUTE_FLD_FLAG, field_id))
    {
        sal_sprintf(str, "%u", p_ns_route->flag);
    }
    if (FLD_MATCH(TBL_NS_ROUTE_FLD_IS_INBAND, field_id))
    {
        sal_sprintf(str, "%u", p_ns_route->is_inband);
    }
    if (FLD_MATCH(TBL_NS_ROUTE_FLD_IFNAME, field_id))
    {
        sal_sprintf(str, "%s", p_ns_route->ifname);
    }
    if (FLD_MATCH(TBL_NS_ROUTE_FLD_CNT, field_id))
    {
        sal_sprintf(str, "%d", p_ns_route->cnt);
    }
    return str;
}

char**
tbl_ns_route_table_dump(tbl_ns_route_t *p_ns_route, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_ns_route_key_name_dump(p_ns_route, buf));
    for(i=1; i<TBL_NS_ROUTE_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_ns_route_field_name_dump(p_ns_route, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_ns_route_field_value_dump(p_ns_route, i, buf));
    }
    return str;
}

int32
tbl_ns_route_field_value_parser(char *str, int32 field_id, tbl_ns_route_t *p_ns_route)
{
    if (FLD_MATCH(TBL_NS_ROUTE_FLD_KEY, field_id))
    {
        cdb_ns_route_key_str2val(str, &p_ns_route->key);
    }
    if (FLD_MATCH(TBL_NS_ROUTE_FLD_FLAG, field_id))
    {
        int32 ret;
        p_ns_route->flag = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NS_ROUTE_FLD_IS_INBAND, field_id))
    {
        int32 ret;
        p_ns_route->is_inband = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NS_ROUTE_FLD_IFNAME, field_id))
    {
        sal_strcpy(p_ns_route->ifname, str);
    }
    if (FLD_MATCH(TBL_NS_ROUTE_FLD_CNT, field_id))
    {
    }
    return PM_E_NONE;
}

int32
tbl_ns_route_table_parser(char** array, char* key_value,tbl_ns_route_t *p_ns_route)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_NS_ROUTE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_ns_route_key_str2val(key_value, p_ns_route));

    for(i=1; i<TBL_NS_ROUTE_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_ns_route_field_value_parser( array[j++], i, p_ns_route));
    }

    return PM_E_NONE;
}

int32
tbl_ns_route_dump_one(tbl_ns_route_t *p_ns_route, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_NS_ROUTE);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_NS_ROUTE_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_NS_ROUTE_FLD_KEY].name, 
            cdb_ns_route_key_val2str(buf, MAX_CMD_STR_LEN, &p_ns_route->key));
    }
    if (FLD_MATCH(TBL_NS_ROUTE_FLD_FLAG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NS_ROUTE_FLD_FLAG].name,
            p_ns_route->flag);
    }
    if (FLD_MATCH(TBL_NS_ROUTE_FLD_IS_INBAND, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NS_ROUTE_FLD_IS_INBAND].name,
            p_ns_route->is_inband);
    }
    if (FLD_MATCH(TBL_NS_ROUTE_FLD_IFNAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_NS_ROUTE_FLD_IFNAME].name,
            p_ns_route->ifname);
    }
    if (FLD_MATCH(TBL_NS_ROUTE_FLD_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_NS_ROUTE_FLD_CNT].name,
            p_ns_route->cnt);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_ns_route_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_ns_route_master_t *p_master = _g_p_tbl_ns_route_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_ns_route_t *p_db_ns_route = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->ns_route_list, p_db_ns_route, listnode, next)
    {
        rc |= fn(p_db_ns_route, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_ns_route_master_t*
tbl_ns_route_get_master()
{
    return _g_p_tbl_ns_route_master;
}

tbl_ns_route_master_t*
tbl_ns_route_init_ns_route()
{
    _g_p_tbl_ns_route_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_ns_route_master_t));
    _g_p_tbl_ns_route_master->ns_route_hash = ctclib_hash_create(_tbl_ns_route_hash_make, _tbl_ns_route_hash_cmp);
    _g_p_tbl_ns_route_master->ns_route_list = ctclib_slist_create(tbl_ns_route_cmp, NULL);
    return _g_p_tbl_ns_route_master;
}

