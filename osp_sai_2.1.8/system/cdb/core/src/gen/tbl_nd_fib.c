
#include "proto.h"
#include "gen/tbl_nd_fib_define.h"
#include "gen/tbl_nd_fib.h"
#include "cdb_data_cmp.h"

tbl_nd_fib_master_t *_g_p_tbl_nd_fib_master = NULL;

static uint32
_tbl_nd_fib_hash_make(tbl_nd_fib_t *p_nd_fib)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_nd_fib->key;
    for (index = 0; index < sizeof(p_nd_fib->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_nd_fib_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_nd_fib_t *p_nd_fib1 = (tbl_nd_fib_t*)p_arg1;
    tbl_nd_fib_t *p_nd_fib2 = (tbl_nd_fib_t*)p_arg2;

    if (0 == sal_memcmp(&p_nd_fib1->key, &p_nd_fib2->key, sizeof(tbl_nd_fib_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_nd_fib_add_nd_fib_sync(tbl_nd_fib_t *p_nd_fib, uint32 sync)
{
    tbl_nd_fib_master_t *p_master = _g_p_tbl_nd_fib_master;
    tbl_nd_fib_t *p_db_nd_fib = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_nd_fib_get_nd_fib(&p_nd_fib->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_nd_fib = XCALLOC(MEM_TBL_ND_FIB, sizeof(tbl_nd_fib_t));
    if (NULL == p_db_nd_fib)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_nd_fib, p_nd_fib, sizeof(tbl_nd_fib_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->neighbor_hash, (void*)p_db_nd_fib, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->neighbor_list, p_db_nd_fib);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_ND_FIB, p_db_nd_fib);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_ND_FIB, p_db_nd_fib);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_nd_fib_del_nd_fib_sync(tbl_nd_fib_key_t *p_nd_fib_key, uint32 sync)
{
    tbl_nd_fib_master_t *p_master = _g_p_tbl_nd_fib_master;
    tbl_nd_fib_t *p_db_nd_fib = NULL;

    /* 1. lookup entry exist */
    p_db_nd_fib = tbl_nd_fib_get_nd_fib(p_nd_fib_key);
    if (NULL == p_db_nd_fib)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_ND_FIB, p_db_nd_fib);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_ND_FIB, p_db_nd_fib);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->neighbor_hash, (void*)p_db_nd_fib);
    ctclib_slistnode_delete(p_master->neighbor_list, p_db_nd_fib);

    /* 4. free db entry */
    XFREE(MEM_TBL_ND_FIB, p_db_nd_fib);

    return PM_E_NONE;
}

int32
tbl_nd_fib_set_nd_fib_field_sync(tbl_nd_fib_t *p_nd_fib, tbl_nd_fib_field_id_t field_id, uint32 sync)
{
    tbl_nd_fib_t *p_db_nd_fib = NULL;

    /* 1. lookup entry exist */
    p_db_nd_fib = tbl_nd_fib_get_nd_fib(&p_nd_fib->key);
    if (NULL == p_db_nd_fib)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_ND_FIB_FLD_IFINDEX:
        p_db_nd_fib->ifindex = p_nd_fib->ifindex;
        break;

    case TBL_ND_FIB_FLD_IFNAME:
        sal_memcpy(p_db_nd_fib->ifname, p_nd_fib->ifname, sizeof(p_nd_fib->ifname));
        break;

    case TBL_ND_FIB_FLD_MAC_ADDR:
        sal_memcpy(p_db_nd_fib->mac_addr, p_nd_fib->mac_addr, sizeof(p_nd_fib->mac_addr));
        break;

    case TBL_ND_FIB_FLD_STATE:
        p_db_nd_fib->state = p_nd_fib->state;
        break;

    case TBL_ND_FIB_FLD_PERMANENT:
        CDB_FLAG_CHECK_SET(p_db_nd_fib, p_nd_fib, flags, GLB_ND_FLAG_PERMANENT)
        break;

    case TBL_ND_FIB_FLD_DYNAMIC:
        CDB_FLAG_CHECK_SET(p_db_nd_fib, p_nd_fib, flags, GLB_ND_FLAG_DYNAMIC)
        break;

    case TBL_ND_FIB_FLD_ACTIVE:
        CDB_FLAG_CHECK_SET(p_db_nd_fib, p_nd_fib, flags, GLB_ND_FLAG_ACTIVE)
        break;

    case TBL_ND_FIB_FLD_ROUTER:
        CDB_FLAG_CHECK_SET(p_db_nd_fib, p_nd_fib, flags, GLB_ND_FLAG_ROUTER)
        break;

    case TBL_ND_FIB_FLD_FDB_AGE:
        CDB_FLAG_CHECK_SET(p_db_nd_fib, p_nd_fib, flags, GLB_ND_FLAG_FDB_AGE)
        break;

    case TBL_ND_FIB_FLD_UPTIME:
        sal_memcpy(&p_db_nd_fib->uptime, &p_nd_fib->uptime, sizeof(p_nd_fib->uptime));
        break;

    case TBL_ND_FIB_FLD_PROBES:
        p_db_nd_fib->probes = p_nd_fib->probes;
        break;

    case TBL_ND_FIB_FLD_T_TIMER:
        p_db_nd_fib->t_timer = p_nd_fib->t_timer;
        break;

    case TBL_ND_FIB_FLD_IPACKETS:
        p_db_nd_fib->ipackets = p_nd_fib->ipackets;
        break;

    case TBL_ND_FIB_FLD_OPACKETS:
        p_db_nd_fib->opackets = p_nd_fib->opackets;
        break;

    case TBL_ND_FIB_FLD_ROUTE_REF:
        p_db_nd_fib->route_ref = p_nd_fib->route_ref;
        break;

    case TBL_ND_FIB_FLD_IS_STATIC_ROUTE_DUP:
        p_db_nd_fib->is_static_route_dup = p_nd_fib->is_static_route_dup;
        break;

    case TBL_ND_FIB_FLD_STAIC_ROUTE_DUP_IPV6_NH:
        sal_memcpy(&p_db_nd_fib->staic_route_dup_ipv6_nh, &p_nd_fib->staic_route_dup_ipv6_nh, sizeof(p_nd_fib->staic_route_dup_ipv6_nh));
        break;

    case TBL_ND_FIB_FLD_MAX:
        sal_memcpy(p_db_nd_fib, p_nd_fib, sizeof(tbl_nd_fib_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_ND_FIB, field_id, p_db_nd_fib);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_ND_FIB, field_id, p_db_nd_fib);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_nd_fib_add_nd_fib(tbl_nd_fib_t *p_nd_fib)
{
    return tbl_nd_fib_add_nd_fib_sync(p_nd_fib, TRUE);
}

int32
tbl_nd_fib_del_nd_fib(tbl_nd_fib_key_t *p_nd_fib_key)
{
    return tbl_nd_fib_del_nd_fib_sync(p_nd_fib_key, TRUE);
}

int32
tbl_nd_fib_set_nd_fib_field(tbl_nd_fib_t *p_nd_fib, tbl_nd_fib_field_id_t field_id)
{
    return tbl_nd_fib_set_nd_fib_field_sync(p_nd_fib, field_id, TRUE);
}

tbl_nd_fib_t*
tbl_nd_fib_get_nd_fib(tbl_nd_fib_key_t *p_nd_fib_key)
{
    tbl_nd_fib_master_t *p_master = _g_p_tbl_nd_fib_master;
    tbl_nd_fib_t lkp;

    sal_memcpy(&lkp.key, p_nd_fib_key, sizeof(tbl_nd_fib_key_t));
    return ctclib_hash_lookup(p_master->neighbor_hash, &lkp);
}

char*
tbl_nd_fib_key_val2str(tbl_nd_fib_t *p_nd_fib, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_nd_prefix_key_val2str(buf, MAX_CMD_STR_LEN, &p_nd_fib->key));
    return str;
}

int32
tbl_nd_fib_key_str2val(char *str, tbl_nd_fib_t *p_nd_fib)
{
    int32 ret = 0;

    ret = cdb_nd_prefix_key_str2val(str, &p_nd_fib->key);
    return ret;
}

char*
tbl_nd_fib_key_name_dump(tbl_nd_fib_t *p_nd_fib, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ND_FIB);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_nd_prefix_key_val2str(buf, MAX_CMD_STR_LEN, &p_nd_fib->key));
    return str;
}

char*
tbl_nd_fib_key_value_dump(tbl_nd_fib_t *p_nd_fib, char *str)
{

    cdb_nd_prefix_key_val2str(str, MAX_CMD_STR_LEN, &p_nd_fib->key);
    return str;
}

char*
tbl_nd_fib_field_name_dump(tbl_nd_fib_t *p_nd_fib, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_ND_FIB);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_ND_FIB_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_nd_fib_field_value_dump(tbl_nd_fib_t *p_nd_fib, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_ND_FIB_FLD_KEY, field_id))
    {
        cdb_nd_prefix_key_val2str(str, MAX_CMD_STR_LEN, &p_nd_fib->key);
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_nd_fib->ifindex);
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_IFNAME, field_id))
    {
        sal_sprintf(str, "%s", p_nd_fib->ifname);
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_MAC_ADDR, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_nd_fib->mac_addr);
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_STATE, field_id))
    {
        sal_sprintf(str, "%u", p_nd_fib->state);
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_PERMANENT, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_nd_fib->flags, GLB_ND_FLAG_PERMANENT));
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_DYNAMIC, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_nd_fib->flags, GLB_ND_FLAG_DYNAMIC));
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_ACTIVE, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_nd_fib->flags, GLB_ND_FLAG_ACTIVE));
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_ROUTER, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_nd_fib->flags, GLB_ND_FLAG_ROUTER));
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_FDB_AGE, field_id))
    {
        sal_sprintf(str, "%u",
            GLB_FLAG_ISSET(p_nd_fib->flags, GLB_ND_FLAG_FDB_AGE));
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_UPTIME, field_id))
    {
        cdb_sal_time_val2str(str, MAX_CMD_STR_LEN, &p_nd_fib->uptime);
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_PROBES, field_id))
    {
        sal_sprintf(str, "%u", p_nd_fib->probes);
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_T_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_nd_fib->t_timer);
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_IPACKETS, field_id))
    {
        sal_sprintf(str, "%u", p_nd_fib->ipackets);
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_OPACKETS, field_id))
    {
        sal_sprintf(str, "%u", p_nd_fib->opackets);
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_ROUTE_REF, field_id))
    {
        sal_sprintf(str, "%u", p_nd_fib->route_ref);
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_IS_STATIC_ROUTE_DUP, field_id))
    {
        sal_sprintf(str, "%u", p_nd_fib->is_static_route_dup);
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_STAIC_ROUTE_DUP_IPV6_NH, field_id))
    {
        cdb_addr_ipv6_val2str(str, MAX_CMD_STR_LEN, &p_nd_fib->staic_route_dup_ipv6_nh);
    }
    return str;
}

char**
tbl_nd_fib_table_dump(tbl_nd_fib_t *p_nd_fib, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_nd_fib_key_name_dump(p_nd_fib, buf));
    for(i=1; i<TBL_ND_FIB_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_nd_fib_field_name_dump(p_nd_fib, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_nd_fib_field_value_dump(p_nd_fib, i, buf));
    }
    return str;
}

int32
tbl_nd_fib_field_value_parser(char *str, int32 field_id, tbl_nd_fib_t *p_nd_fib)
{
    if (FLD_MATCH(TBL_ND_FIB_FLD_KEY, field_id))
    {
        cdb_nd_prefix_key_str2val(str, &p_nd_fib->key);
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_IFINDEX, field_id))
    {
        int32 ret;
        p_nd_fib->ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_IFNAME, field_id))
    {
        sal_strcpy(p_nd_fib->ifname, str);
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_MAC_ADDR, field_id))
    {
        cdb_mac_addr_str2val(p_nd_fib->mac_addr, str);
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_STATE, field_id))
    {
        int32 ret;
        p_nd_fib->state = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_PERMANENT, field_id))
    {
        GLB_SET_FLAG(p_nd_fib->flags, GLB_ND_FLAG_PERMANENT);
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_DYNAMIC, field_id))
    {
        GLB_SET_FLAG(p_nd_fib->flags, GLB_ND_FLAG_DYNAMIC);
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_ACTIVE, field_id))
    {
        GLB_SET_FLAG(p_nd_fib->flags, GLB_ND_FLAG_ACTIVE);
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_ROUTER, field_id))
    {
        GLB_SET_FLAG(p_nd_fib->flags, GLB_ND_FLAG_ROUTER);
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_FDB_AGE, field_id))
    {
        GLB_SET_FLAG(p_nd_fib->flags, GLB_ND_FLAG_FDB_AGE);
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_UPTIME, field_id))
    {
        cdb_sal_time_str2val(str, &p_nd_fib->uptime);
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_PROBES, field_id))
    {
        int32 ret;
        p_nd_fib->probes = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_T_TIMER, field_id))
    {
        ctc_task_str2val(str, p_nd_fib->t_timer);
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_IPACKETS, field_id))
    {
        int32 ret;
        p_nd_fib->ipackets = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_OPACKETS, field_id))
    {
        int32 ret;
        p_nd_fib->opackets = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_ROUTE_REF, field_id))
    {
        int32 ret;
        p_nd_fib->route_ref = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_IS_STATIC_ROUTE_DUP, field_id))
    {
        int32 ret;
        p_nd_fib->is_static_route_dup = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_STAIC_ROUTE_DUP_IPV6_NH, field_id))
    {
        cdb_addr_ipv6_str2val(str, &p_nd_fib->staic_route_dup_ipv6_nh);
    }
    return PM_E_NONE;
}

int32
tbl_nd_fib_table_parser(char** array, char* key_value,tbl_nd_fib_t *p_nd_fib)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_ND_FIB);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_nd_fib_key_str2val(key_value, p_nd_fib));

    for(i=1; i<TBL_ND_FIB_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_nd_fib_field_value_parser( array[j++], i, p_nd_fib));
    }

    return PM_E_NONE;
}

int32
tbl_nd_fib_dump_one(tbl_nd_fib_t *p_nd_fib, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ND_FIB);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_ND_FIB_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ND_FIB_FLD_KEY].name, 
            cdb_nd_prefix_key_val2str(buf, MAX_CMD_STR_LEN, &p_nd_fib->key));
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ND_FIB_FLD_IFINDEX].name,
            p_nd_fib->ifindex);
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_IFNAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ND_FIB_FLD_IFNAME].name,
            p_nd_fib->ifname);
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_MAC_ADDR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ND_FIB_FLD_MAC_ADDR].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_nd_fib->mac_addr));
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ND_FIB_FLD_STATE].name,
            p_nd_fib->state);
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_PERMANENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ND_FIB_FLD_PERMANENT].name,
            GLB_FLAG_ISSET(p_nd_fib->flags, GLB_ND_FLAG_PERMANENT));
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_DYNAMIC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ND_FIB_FLD_DYNAMIC].name,
            GLB_FLAG_ISSET(p_nd_fib->flags, GLB_ND_FLAG_DYNAMIC));
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_ACTIVE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ND_FIB_FLD_ACTIVE].name,
            GLB_FLAG_ISSET(p_nd_fib->flags, GLB_ND_FLAG_ACTIVE));
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_ROUTER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ND_FIB_FLD_ROUTER].name,
            GLB_FLAG_ISSET(p_nd_fib->flags, GLB_ND_FLAG_ROUTER));
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_FDB_AGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ND_FIB_FLD_FDB_AGE].name,
            GLB_FLAG_ISSET(p_nd_fib->flags, GLB_ND_FLAG_FDB_AGE));
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_UPTIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ND_FIB_FLD_UPTIME].name, 
            cdb_sal_time_val2str(buf, MAX_CMD_STR_LEN, &p_nd_fib->uptime));
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_PROBES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ND_FIB_FLD_PROBES].name,
            p_nd_fib->probes);
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_T_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ND_FIB_FLD_T_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_nd_fib->t_timer));
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_IPACKETS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ND_FIB_FLD_IPACKETS].name,
            p_nd_fib->ipackets);
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_OPACKETS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ND_FIB_FLD_OPACKETS].name,
            p_nd_fib->opackets);
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_ROUTE_REF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ND_FIB_FLD_ROUTE_REF].name,
            p_nd_fib->route_ref);
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_IS_STATIC_ROUTE_DUP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ND_FIB_FLD_IS_STATIC_ROUTE_DUP].name,
            p_nd_fib->is_static_route_dup);
    }
    if (FLD_MATCH(TBL_ND_FIB_FLD_STAIC_ROUTE_DUP_IPV6_NH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ND_FIB_FLD_STAIC_ROUTE_DUP_IPV6_NH].name, 
            cdb_addr_ipv6_val2str(buf, MAX_CMD_STR_LEN, &p_nd_fib->staic_route_dup_ipv6_nh));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_nd_fib_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_nd_fib_master_t *p_master = _g_p_tbl_nd_fib_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_nd_fib_t *p_db_nd_fib = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->neighbor_list, p_db_nd_fib, listnode, next)
    {
        rc |= fn(p_db_nd_fib, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_nd_fib_master_t*
tbl_nd_fib_get_master()
{
    return _g_p_tbl_nd_fib_master;
}

tbl_nd_fib_master_t*
tbl_nd_fib_init_nd_fib()
{
    _g_p_tbl_nd_fib_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_nd_fib_master_t));
    _g_p_tbl_nd_fib_master->neighbor_hash = ctclib_hash_create(_tbl_nd_fib_hash_make, _tbl_nd_fib_hash_cmp);
    _g_p_tbl_nd_fib_master->neighbor_list = ctclib_slist_create(tbl_nd_fib_cmp, NULL);
    return _g_p_tbl_nd_fib_master;
}

