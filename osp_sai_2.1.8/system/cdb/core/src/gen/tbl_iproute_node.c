
#include "proto.h"
#include "gen/tbl_iproute_node_define.h"
#include "gen/tbl_iproute_node.h"
#include "cdb_data_cmp.h"

tbl_iproute_node_master_t *_g_p_tbl_iproute_node_master = NULL;

static uint32
_tbl_iproute_node_hash_make(tbl_iproute_node_t *p_rn)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_rn->key;
    for (index = 0; index < sizeof(p_rn->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_iproute_node_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_iproute_node_t *p_rn1 = (tbl_iproute_node_t*)p_arg1;
    tbl_iproute_node_t *p_rn2 = (tbl_iproute_node_t*)p_arg2;

    if (0 == sal_memcmp(&p_rn1->key, &p_rn2->key, sizeof(tbl_iproute_node_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_iproute_node_add_iproute_node_sync(tbl_iproute_node_t *p_rn, uint32 sync)
{
    tbl_iproute_node_master_t *p_master = _g_p_tbl_iproute_node_master;
    tbl_iproute_node_t *p_db_rn = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_iproute_node_get_iproute_node(&p_rn->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_rn = XCALLOC(MEM_TBL_IPROUTE_NODE, sizeof(tbl_iproute_node_t));
    if (NULL == p_db_rn)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_rn, p_rn, sizeof(tbl_iproute_node_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->rn_hash, (void*)p_db_rn, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->rn_list, p_db_rn);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_IPROUTE_NODE, p_db_rn);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_IPROUTE_NODE, p_db_rn);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_iproute_node_del_iproute_node_sync(tbl_iproute_node_key_t *p_rn_key, uint32 sync)
{
    tbl_iproute_node_master_t *p_master = _g_p_tbl_iproute_node_master;
    tbl_iproute_node_t *p_db_rn = NULL;

    /* 1. lookup entry exist */
    p_db_rn = tbl_iproute_node_get_iproute_node(p_rn_key);
    if (NULL == p_db_rn)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_IPROUTE_NODE, p_db_rn);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_IPROUTE_NODE, p_db_rn);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->rn_hash, (void*)p_db_rn);
    ctclib_slistnode_delete(p_master->rn_list, p_db_rn);

    /* 4. free db entry */
    XFREE(MEM_TBL_IPROUTE_NODE, p_db_rn);

    return PM_E_NONE;
}

int32
tbl_iproute_node_set_iproute_node_field_sync(tbl_iproute_node_t *p_rn, tbl_iproute_node_field_id_t field_id, uint32 sync)
{
    tbl_iproute_node_t *p_db_rn = NULL;

    /* 1. lookup entry exist */
    p_db_rn = tbl_iproute_node_get_iproute_node(&p_rn->key);
    if (NULL == p_db_rn)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_IPROUTE_NODE_FLD_ROUTE_TYPE:
        p_db_rn->route_type = p_rn->route_type;
        break;

    case TBL_IPROUTE_NODE_FLD_DISTANCE:
        p_db_rn->distance = p_rn->distance;
        break;

    case TBL_IPROUTE_NODE_FLD_MULTIPATH:
        p_db_rn->multipath = p_rn->multipath;
        break;

    case TBL_IPROUTE_NODE_FLD_NH_KEY:
        sal_memcpy(&p_db_rn->nh_key, &p_rn->nh_key, sizeof(p_rn->nh_key));
        break;

    case TBL_IPROUTE_NODE_FLD_NH_GROUP:
        sal_memcpy(&p_db_rn->nh_group, &p_rn->nh_group, sizeof(p_rn->nh_group));
        break;

    case TBL_IPROUTE_NODE_FLD_MAX:
        sal_memcpy(p_db_rn, p_rn, sizeof(tbl_iproute_node_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_IPROUTE_NODE, field_id, p_db_rn);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_IPROUTE_NODE, field_id, p_db_rn);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_iproute_node_add_iproute_node(tbl_iproute_node_t *p_rn)
{
    return tbl_iproute_node_add_iproute_node_sync(p_rn, TRUE);
}

int32
tbl_iproute_node_del_iproute_node(tbl_iproute_node_key_t *p_rn_key)
{
    return tbl_iproute_node_del_iproute_node_sync(p_rn_key, TRUE);
}

int32
tbl_iproute_node_set_iproute_node_field(tbl_iproute_node_t *p_rn, tbl_iproute_node_field_id_t field_id)
{
    return tbl_iproute_node_set_iproute_node_field_sync(p_rn, field_id, TRUE);
}

tbl_iproute_node_t*
tbl_iproute_node_get_iproute_node(tbl_iproute_node_key_t *p_rn_key)
{
    tbl_iproute_node_master_t *p_master = _g_p_tbl_iproute_node_master;
    tbl_iproute_node_t lkp;

    sal_memcpy(&lkp.key, p_rn_key, sizeof(tbl_iproute_node_key_t));
    return ctclib_hash_lookup(p_master->rn_hash, &lkp);
}

char*
tbl_iproute_node_key_val2str(tbl_iproute_node_t *p_rn, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_route_node_key_val2str(buf, MAX_CMD_STR_LEN, &p_rn->key));
    return str;
}

int32
tbl_iproute_node_key_str2val(char *str, tbl_iproute_node_t *p_rn)
{
    int32 ret = 0;

    ret = cdb_route_node_key_str2val(str, &p_rn->key);
    return ret;
}

char*
tbl_iproute_node_key_name_dump(tbl_iproute_node_t *p_rn, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_IPROUTE_NODE);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_route_node_key_val2str(buf, MAX_CMD_STR_LEN, &p_rn->key));
    return str;
}

char*
tbl_iproute_node_key_value_dump(tbl_iproute_node_t *p_rn, char *str)
{

    cdb_route_node_key_val2str(str, MAX_CMD_STR_LEN, &p_rn->key);
    return str;
}

char*
tbl_iproute_node_field_name_dump(tbl_iproute_node_t *p_rn, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_IPROUTE_NODE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_IPROUTE_NODE_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_iproute_node_field_value_dump(tbl_iproute_node_t *p_rn, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_IPROUTE_NODE_FLD_KEY, field_id))
    {
        cdb_route_node_key_val2str(str, MAX_CMD_STR_LEN, &p_rn->key);
    }
    if (FLD_MATCH(TBL_IPROUTE_NODE_FLD_ROUTE_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_rn->route_type);
    }
    if (FLD_MATCH(TBL_IPROUTE_NODE_FLD_DISTANCE, field_id))
    {
        sal_sprintf(str, "%u", p_rn->distance);
    }
    if (FLD_MATCH(TBL_IPROUTE_NODE_FLD_MULTIPATH, field_id))
    {
        sal_sprintf(str, "%u", p_rn->multipath);
    }
    if (FLD_MATCH(TBL_IPROUTE_NODE_FLD_NH_KEY, field_id))
    {
        cdb_nexthop_key_val2str(str, MAX_CMD_STR_LEN, &p_rn->nh_key);
    }
    if (FLD_MATCH(TBL_IPROUTE_NODE_FLD_NH_GROUP, field_id))
    {
        cdb_nexthop_ecmp_val2str(str, MAX_CMD_STR_LEN, &p_rn->nh_group);
    }
    return str;
}

char**
tbl_iproute_node_table_dump(tbl_iproute_node_t *p_rn, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_iproute_node_key_name_dump(p_rn, buf));
    for(i=1; i<TBL_IPROUTE_NODE_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_iproute_node_field_name_dump(p_rn, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_iproute_node_field_value_dump(p_rn, i, buf));
    }
    return str;
}

int32
tbl_iproute_node_field_value_parser(char *str, int32 field_id, tbl_iproute_node_t *p_rn)
{
    if (FLD_MATCH(TBL_IPROUTE_NODE_FLD_KEY, field_id))
    {
        cdb_route_node_key_str2val(str, &p_rn->key);
    }
    if (FLD_MATCH(TBL_IPROUTE_NODE_FLD_ROUTE_TYPE, field_id))
    {
        int32 ret;
        p_rn->route_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPROUTE_NODE_FLD_DISTANCE, field_id))
    {
        int32 ret;
        p_rn->distance = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPROUTE_NODE_FLD_MULTIPATH, field_id))
    {
        int32 ret;
        p_rn->multipath = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPROUTE_NODE_FLD_NH_KEY, field_id))
    {
        cdb_nexthop_key_str2val(str, &p_rn->nh_key);
    }
    if (FLD_MATCH(TBL_IPROUTE_NODE_FLD_NH_GROUP, field_id))
    {
        cdb_nexthop_ecmp_str2val(str, &p_rn->nh_group);
    }
    return PM_E_NONE;
}

int32
tbl_iproute_node_table_parser(char** array, char* key_value,tbl_iproute_node_t *p_rn)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_IPROUTE_NODE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_iproute_node_key_str2val(key_value, p_rn));

    for(i=1; i<TBL_IPROUTE_NODE_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_iproute_node_field_value_parser( array[j++], i, p_rn));
    }

    return PM_E_NONE;
}

int32
tbl_iproute_node_dump_one(tbl_iproute_node_t *p_rn, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_IPROUTE_NODE);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_IPROUTE_NODE_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IPROUTE_NODE_FLD_KEY].name, 
            cdb_route_node_key_val2str(buf, MAX_CMD_STR_LEN, &p_rn->key));
    }
    if (FLD_MATCH(TBL_IPROUTE_NODE_FLD_ROUTE_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPROUTE_NODE_FLD_ROUTE_TYPE].name,
            p_rn->route_type);
    }
    if (FLD_MATCH(TBL_IPROUTE_NODE_FLD_DISTANCE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPROUTE_NODE_FLD_DISTANCE].name,
            p_rn->distance);
    }
    if (FLD_MATCH(TBL_IPROUTE_NODE_FLD_MULTIPATH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPROUTE_NODE_FLD_MULTIPATH].name,
            p_rn->multipath);
    }
    if (FLD_MATCH(TBL_IPROUTE_NODE_FLD_NH_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IPROUTE_NODE_FLD_NH_KEY].name, 
            cdb_nexthop_key_val2str(buf, MAX_CMD_STR_LEN, &p_rn->nh_key));
    }
    if (FLD_MATCH(TBL_IPROUTE_NODE_FLD_NH_GROUP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IPROUTE_NODE_FLD_NH_GROUP].name, 
            cdb_nexthop_ecmp_val2str(buf, MAX_CMD_STR_LEN, &p_rn->nh_group));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_iproute_node_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_iproute_node_master_t *p_master = _g_p_tbl_iproute_node_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_iproute_node_t *p_db_rn = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->rn_list, p_db_rn, listnode, next)
    {
        rc |= fn(p_db_rn, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_iproute_node_master_t*
tbl_iproute_node_get_master()
{
    return _g_p_tbl_iproute_node_master;
}

tbl_iproute_node_master_t*
tbl_iproute_node_init_iproute_node()
{
    _g_p_tbl_iproute_node_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_iproute_node_master_t));
    _g_p_tbl_iproute_node_master->rn_hash = ctclib_hash_create(_tbl_iproute_node_hash_make, _tbl_iproute_node_hash_cmp);
    _g_p_tbl_iproute_node_master->rn_list = ctclib_slist_create(tbl_iproute_node_cmp, NULL);
    return _g_p_tbl_iproute_node_master;
}

