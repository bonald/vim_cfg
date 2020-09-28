
#include "proto.h"
#include "gen/tbl_bgp_define.h"
#include "gen/tbl_bgp.h"
#include "cdb_data_cmp.h"

tbl_bgp_master_t *_g_p_tbl_bgp_master = NULL;

static uint32
_tbl_bgp_hash_make(tbl_bgp_t *p_bgp)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_bgp->key;
    for (index = 0; index < sizeof(p_bgp->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_bgp_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_bgp_t *p_bgp1 = (tbl_bgp_t*)p_arg1;
    tbl_bgp_t *p_bgp2 = (tbl_bgp_t*)p_arg2;

    if (0 == sal_memcmp(&p_bgp1->key, &p_bgp2->key, sizeof(tbl_bgp_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_bgp_add_bgp_sync(tbl_bgp_t *p_bgp, uint32 sync)
{
    tbl_bgp_master_t *p_master = _g_p_tbl_bgp_master;
    tbl_bgp_t *p_db_bgp = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_bgp_get_bgp(&p_bgp->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_bgp = XCALLOC(MEM_TBL_BGP, sizeof(tbl_bgp_t));
    if (NULL == p_db_bgp)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_bgp, p_bgp, sizeof(tbl_bgp_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->bgp_hash, (void*)p_db_bgp, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->bgp_list, p_db_bgp);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_BGP, p_db_bgp);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_BGP, p_db_bgp);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_bgp_del_bgp_sync(tbl_bgp_key_t *p_bgp_key, uint32 sync)
{
    tbl_bgp_master_t *p_master = _g_p_tbl_bgp_master;
    tbl_bgp_t *p_db_bgp = NULL;

    /* 1. lookup entry exist */
    p_db_bgp = tbl_bgp_get_bgp(p_bgp_key);
    if (NULL == p_db_bgp)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_BGP, p_db_bgp);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_BGP, p_db_bgp);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->bgp_hash, (void*)p_db_bgp);
    ctclib_slistnode_delete(p_master->bgp_list, p_db_bgp);

    /* 4. free db entry */
    XFREE(MEM_TBL_BGP, p_db_bgp);

    return PM_E_NONE;
}

int32
tbl_bgp_set_bgp_field_sync(tbl_bgp_t *p_bgp, tbl_bgp_field_id_t field_id, uint32 sync)
{
    tbl_bgp_t *p_db_bgp = NULL;

    /* 1. lookup entry exist */
    p_db_bgp = tbl_bgp_get_bgp(&p_bgp->key);
    if (NULL == p_db_bgp)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_BGP_FLD_ROUTER_ID:
        sal_memcpy(&p_db_bgp->router_id, &p_bgp->router_id, sizeof(p_bgp->router_id));
        break;

    case TBL_BGP_FLD_REDISTRIBUTE:
        p_db_bgp->redistribute = p_bgp->redistribute;
        break;

    case TBL_BGP_FLD_RED_METRIC:
        sal_memcpy(p_db_bgp->red_metric, p_bgp->red_metric, sizeof(p_bgp->red_metric));
        break;

    case TBL_BGP_FLD_MAX:
        sal_memcpy(p_db_bgp, p_bgp, sizeof(tbl_bgp_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_BGP, field_id, p_db_bgp);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_BGP, field_id, p_db_bgp);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_bgp_add_bgp(tbl_bgp_t *p_bgp)
{
    return tbl_bgp_add_bgp_sync(p_bgp, TRUE);
}

int32
tbl_bgp_del_bgp(tbl_bgp_key_t *p_bgp_key)
{
    return tbl_bgp_del_bgp_sync(p_bgp_key, TRUE);
}

int32
tbl_bgp_set_bgp_field(tbl_bgp_t *p_bgp, tbl_bgp_field_id_t field_id)
{
    return tbl_bgp_set_bgp_field_sync(p_bgp, field_id, TRUE);
}

tbl_bgp_t*
tbl_bgp_get_bgp(tbl_bgp_key_t *p_bgp_key)
{
    tbl_bgp_master_t *p_master = _g_p_tbl_bgp_master;
    tbl_bgp_t lkp;

    sal_memcpy(&lkp.key, p_bgp_key, sizeof(tbl_bgp_key_t));
    return ctclib_hash_lookup(p_master->bgp_hash, &lkp);
}

char*
tbl_bgp_key_val2str(tbl_bgp_t *p_bgp, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_bgp->key.as_id);
    return str;
}

int32
tbl_bgp_key_str2val(char *str, tbl_bgp_t *p_bgp)
{
    int32 ret = 0;

    p_bgp->key.as_id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_bgp_key_name_dump(tbl_bgp_t *p_bgp, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_BGP);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_bgp->key.as_id);
    return str;
}

char*
tbl_bgp_key_value_dump(tbl_bgp_t *p_bgp, char *str)
{
    sal_sprintf(str, "%u", p_bgp->key.as_id);
    return str;
}

char*
tbl_bgp_field_name_dump(tbl_bgp_t *p_bgp, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_BGP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_BGP_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_bgp_field_value_dump(tbl_bgp_t *p_bgp, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_BGP_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_bgp->key.as_id);
    }
    if (FLD_MATCH(TBL_BGP_FLD_ROUTER_ID, field_id))
    {
        cdb_addr_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_bgp->router_id);
    }
    if (FLD_MATCH(TBL_BGP_FLD_REDISTRIBUTE, field_id))
    {
        sal_sprintf(str, "%u", p_bgp->redistribute);
    }
    if (FLD_MATCH(TBL_BGP_FLD_RED_METRIC, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_bgp->red_metric, sizeof(p_bgp->red_metric)/4);
    }
    return str;
}

char**
tbl_bgp_table_dump(tbl_bgp_t *p_bgp, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_bgp_key_name_dump(p_bgp, buf));
    for(i=1; i<TBL_BGP_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_bgp_field_name_dump(p_bgp, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_bgp_field_value_dump(p_bgp, i, buf));
    }
    return str;
}

int32
tbl_bgp_field_value_parser(char *str, int32 field_id, tbl_bgp_t *p_bgp)
{
    if (FLD_MATCH(TBL_BGP_FLD_KEY, field_id))
    {
        int32 ret;
        p_bgp->key.as_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BGP_FLD_ROUTER_ID, field_id))
    {
        cdb_addr_ipv4_str2val(&p_bgp->router_id, str, 0);
    }
    if (FLD_MATCH(TBL_BGP_FLD_REDISTRIBUTE, field_id))
    {
        int32 ret;
        p_bgp->redistribute = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BGP_FLD_RED_METRIC, field_id))
    {
        cdb_uint32_array_str2val(str, p_bgp->red_metric);
    }
    return PM_E_NONE;
}

int32
tbl_bgp_table_parser(char** array, char* key_value,tbl_bgp_t *p_bgp)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_BGP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_bgp_key_str2val(key_value, p_bgp));

    for(i=1; i<TBL_BGP_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_bgp_field_value_parser( array[j++], i, p_bgp));
    }

    return PM_E_NONE;
}

int32
tbl_bgp_dump_one(tbl_bgp_t *p_bgp, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_BGP);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_BGP_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BGP_FLD_KEY].name,
            p_bgp->key.as_id);
    }
    if (FLD_MATCH(TBL_BGP_FLD_ROUTER_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_BGP_FLD_ROUTER_ID].name, 
            cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_bgp->router_id));
    }
    if (FLD_MATCH(TBL_BGP_FLD_REDISTRIBUTE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BGP_FLD_REDISTRIBUTE].name,
            p_bgp->redistribute);
    }
    if (FLD_MATCH(TBL_BGP_FLD_RED_METRIC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_BGP_FLD_RED_METRIC].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_bgp->red_metric, sizeof(p_bgp->red_metric)/4));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_bgp_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_bgp_master_t *p_master = _g_p_tbl_bgp_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_bgp_t *p_db_bgp = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->bgp_list, p_db_bgp, listnode, next)
    {
        rc |= fn(p_db_bgp, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_bgp_master_t*
tbl_bgp_get_master()
{
    return _g_p_tbl_bgp_master;
}

tbl_bgp_master_t*
tbl_bgp_init_bgp()
{
    _g_p_tbl_bgp_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_bgp_master_t));
    _g_p_tbl_bgp_master->bgp_hash = ctclib_hash_create(_tbl_bgp_hash_make, _tbl_bgp_hash_cmp);
    _g_p_tbl_bgp_master->bgp_list = ctclib_slist_create(tbl_bgp_cmp, NULL);
    return _g_p_tbl_bgp_master;
}

