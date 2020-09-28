
#include "proto.h"
#include "gen/tbl_static_route_cfg_define.h"
#include "gen/tbl_static_route_cfg.h"
#include "cdb_data_cmp.h"

tbl_static_route_cfg_master_t *_g_p_tbl_static_route_cfg_master = NULL;

static uint32
_tbl_static_route_cfg_hash_make(tbl_static_route_cfg_t *p_rst_cfg)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_rst_cfg->key;
    for (index = 0; index < sizeof(p_rst_cfg->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_static_route_cfg_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_static_route_cfg_t *p_rst_cfg1 = (tbl_static_route_cfg_t*)p_arg1;
    tbl_static_route_cfg_t *p_rst_cfg2 = (tbl_static_route_cfg_t*)p_arg2;

    if (0 == sal_memcmp(&p_rst_cfg1->key, &p_rst_cfg2->key, sizeof(tbl_static_route_cfg_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_static_route_cfg_add_static_route_cfg_sync(tbl_static_route_cfg_t *p_rst_cfg, uint32 sync)
{
    tbl_static_route_cfg_master_t *p_master = _g_p_tbl_static_route_cfg_master;
    tbl_static_route_cfg_t *p_db_rst_cfg = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_static_route_cfg_get_static_route_cfg(&p_rst_cfg->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_rst_cfg = XCALLOC(MEM_TBL_STATIC_ROUTE_CFG, sizeof(tbl_static_route_cfg_t));
    if (NULL == p_db_rst_cfg)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_rst_cfg, p_rst_cfg, sizeof(tbl_static_route_cfg_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->rst_cfg_hash, (void*)p_db_rst_cfg, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->rst_cfg_list, p_db_rst_cfg);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_STATIC_ROUTE_CFG, p_db_rst_cfg);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_STATIC_ROUTE_CFG, p_db_rst_cfg);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_static_route_cfg_del_static_route_cfg_sync(tbl_static_route_cfg_key_t *p_rst_cfg_key, uint32 sync)
{
    tbl_static_route_cfg_master_t *p_master = _g_p_tbl_static_route_cfg_master;
    tbl_static_route_cfg_t *p_db_rst_cfg = NULL;

    /* 1. lookup entry exist */
    p_db_rst_cfg = tbl_static_route_cfg_get_static_route_cfg(p_rst_cfg_key);
    if (NULL == p_db_rst_cfg)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_STATIC_ROUTE_CFG, p_db_rst_cfg);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_STATIC_ROUTE_CFG, p_db_rst_cfg);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->rst_cfg_hash, (void*)p_db_rst_cfg);
    ctclib_slistnode_delete(p_master->rst_cfg_list, p_db_rst_cfg);

    /* 4. free db entry */
    XFREE(MEM_TBL_STATIC_ROUTE_CFG, p_db_rst_cfg);

    return PM_E_NONE;
}

int32
tbl_static_route_cfg_set_static_route_cfg_field_sync(tbl_static_route_cfg_t *p_rst_cfg, tbl_static_route_cfg_field_id_t field_id, uint32 sync)
{
    tbl_static_route_cfg_t *p_db_rst_cfg = NULL;

    /* 1. lookup entry exist */
    p_db_rst_cfg = tbl_static_route_cfg_get_static_route_cfg(&p_rst_cfg->key);
    if (NULL == p_db_rst_cfg)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_STATIC_ROUTE_CFG_FLD_DISTANCE:
        p_db_rst_cfg->distance = p_rst_cfg->distance;
        break;

    case TBL_STATIC_ROUTE_CFG_FLD_TRACK_ID:
        p_db_rst_cfg->track_id = p_rst_cfg->track_id;
        break;

    case TBL_STATIC_ROUTE_CFG_FLD_TRACK_STATUS:
        p_db_rst_cfg->track_status = p_rst_cfg->track_status;
        break;

    case TBL_STATIC_ROUTE_CFG_FLD_NEXTHOP_TYPE_V6:
        p_db_rst_cfg->nexthop_type_v6 = p_rst_cfg->nexthop_type_v6;
        break;

    case TBL_STATIC_ROUTE_CFG_FLD_DHCP_CLIENT:
        p_db_rst_cfg->dhcp_client = p_rst_cfg->dhcp_client;
        break;

    case TBL_STATIC_ROUTE_CFG_FLD_MAX:
        sal_memcpy(p_db_rst_cfg, p_rst_cfg, sizeof(tbl_static_route_cfg_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_STATIC_ROUTE_CFG, field_id, p_db_rst_cfg);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_STATIC_ROUTE_CFG, field_id, p_db_rst_cfg);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_static_route_cfg_add_static_route_cfg(tbl_static_route_cfg_t *p_rst_cfg)
{
    return tbl_static_route_cfg_add_static_route_cfg_sync(p_rst_cfg, TRUE);
}

int32
tbl_static_route_cfg_del_static_route_cfg(tbl_static_route_cfg_key_t *p_rst_cfg_key)
{
    return tbl_static_route_cfg_del_static_route_cfg_sync(p_rst_cfg_key, TRUE);
}

int32
tbl_static_route_cfg_set_static_route_cfg_field(tbl_static_route_cfg_t *p_rst_cfg, tbl_static_route_cfg_field_id_t field_id)
{
    return tbl_static_route_cfg_set_static_route_cfg_field_sync(p_rst_cfg, field_id, TRUE);
}

tbl_static_route_cfg_t*
tbl_static_route_cfg_get_static_route_cfg(tbl_static_route_cfg_key_t *p_rst_cfg_key)
{
    tbl_static_route_cfg_master_t *p_master = _g_p_tbl_static_route_cfg_master;
    tbl_static_route_cfg_t lkp;

    sal_memcpy(&lkp.key, p_rst_cfg_key, sizeof(tbl_static_route_cfg_key_t));
    return ctclib_hash_lookup(p_master->rst_cfg_hash, &lkp);
}

char*
tbl_static_route_cfg_key_val2str(tbl_static_route_cfg_t *p_rst_cfg, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_route_node_key_val2str(buf, MAX_CMD_STR_LEN, &p_rst_cfg->key));
    return str;
}

int32
tbl_static_route_cfg_key_str2val(char *str, tbl_static_route_cfg_t *p_rst_cfg)
{
    int32 ret = 0;

    ret = cdb_route_node_key_str2val(str, &p_rst_cfg->key);
    return ret;
}

char*
tbl_static_route_cfg_key_name_dump(tbl_static_route_cfg_t *p_rst_cfg, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_STATIC_ROUTE_CFG);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_route_node_key_val2str(buf, MAX_CMD_STR_LEN, &p_rst_cfg->key));
    return str;
}

char*
tbl_static_route_cfg_key_value_dump(tbl_static_route_cfg_t *p_rst_cfg, char *str)
{

    cdb_route_node_key_val2str(str, MAX_CMD_STR_LEN, &p_rst_cfg->key);
    return str;
}

char*
tbl_static_route_cfg_field_name_dump(tbl_static_route_cfg_t *p_rst_cfg, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_STATIC_ROUTE_CFG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_STATIC_ROUTE_CFG_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_static_route_cfg_field_value_dump(tbl_static_route_cfg_t *p_rst_cfg, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_STATIC_ROUTE_CFG_FLD_KEY, field_id))
    {
        cdb_route_node_key_val2str(str, MAX_CMD_STR_LEN, &p_rst_cfg->key);
    }
    if (FLD_MATCH(TBL_STATIC_ROUTE_CFG_FLD_DISTANCE, field_id))
    {
        sal_sprintf(str, "%u", p_rst_cfg->distance);
    }
    if (FLD_MATCH(TBL_STATIC_ROUTE_CFG_FLD_TRACK_ID, field_id))
    {
        sal_sprintf(str, "%u", p_rst_cfg->track_id);
    }
    if (FLD_MATCH(TBL_STATIC_ROUTE_CFG_FLD_TRACK_STATUS, field_id))
    {
        sal_sprintf(str, "%u", p_rst_cfg->track_status);
    }
    if (FLD_MATCH(TBL_STATIC_ROUTE_CFG_FLD_NEXTHOP_TYPE_V6, field_id))
    {
        sal_sprintf(str, "%u", p_rst_cfg->nexthop_type_v6);
    }
    if (FLD_MATCH(TBL_STATIC_ROUTE_CFG_FLD_DHCP_CLIENT, field_id))
    {
        sal_sprintf(str, "%u", p_rst_cfg->dhcp_client);
    }
    return str;
}

char**
tbl_static_route_cfg_table_dump(tbl_static_route_cfg_t *p_rst_cfg, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_static_route_cfg_key_name_dump(p_rst_cfg, buf));
    for(i=1; i<TBL_STATIC_ROUTE_CFG_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_static_route_cfg_field_name_dump(p_rst_cfg, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_static_route_cfg_field_value_dump(p_rst_cfg, i, buf));
    }
    return str;
}

int32
tbl_static_route_cfg_field_value_parser(char *str, int32 field_id, tbl_static_route_cfg_t *p_rst_cfg)
{
    if (FLD_MATCH(TBL_STATIC_ROUTE_CFG_FLD_KEY, field_id))
    {
        cdb_route_node_key_str2val(str, &p_rst_cfg->key);
    }
    if (FLD_MATCH(TBL_STATIC_ROUTE_CFG_FLD_DISTANCE, field_id))
    {
        int32 ret;
        p_rst_cfg->distance = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_STATIC_ROUTE_CFG_FLD_TRACK_ID, field_id))
    {
        int32 ret;
        p_rst_cfg->track_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_STATIC_ROUTE_CFG_FLD_TRACK_STATUS, field_id))
    {
        int32 ret;
        p_rst_cfg->track_status = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_STATIC_ROUTE_CFG_FLD_NEXTHOP_TYPE_V6, field_id))
    {
        int32 ret;
        p_rst_cfg->nexthop_type_v6 = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_STATIC_ROUTE_CFG_FLD_DHCP_CLIENT, field_id))
    {
        int32 ret;
        p_rst_cfg->dhcp_client = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_static_route_cfg_table_parser(char** array, char* key_value,tbl_static_route_cfg_t *p_rst_cfg)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_STATIC_ROUTE_CFG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_static_route_cfg_key_str2val(key_value, p_rst_cfg));

    for(i=1; i<TBL_STATIC_ROUTE_CFG_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_static_route_cfg_field_value_parser( array[j++], i, p_rst_cfg));
    }

    return PM_E_NONE;
}

int32
tbl_static_route_cfg_dump_one(tbl_static_route_cfg_t *p_rst_cfg, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_STATIC_ROUTE_CFG);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_STATIC_ROUTE_CFG_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_STATIC_ROUTE_CFG_FLD_KEY].name, 
            cdb_route_node_key_val2str(buf, MAX_CMD_STR_LEN, &p_rst_cfg->key));
    }
    if (FLD_MATCH(TBL_STATIC_ROUTE_CFG_FLD_DISTANCE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_STATIC_ROUTE_CFG_FLD_DISTANCE].name,
            p_rst_cfg->distance);
    }
    if (FLD_MATCH(TBL_STATIC_ROUTE_CFG_FLD_TRACK_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_STATIC_ROUTE_CFG_FLD_TRACK_ID].name,
            p_rst_cfg->track_id);
    }
    if (FLD_MATCH(TBL_STATIC_ROUTE_CFG_FLD_TRACK_STATUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_STATIC_ROUTE_CFG_FLD_TRACK_STATUS].name,
            p_rst_cfg->track_status);
    }
    if (FLD_MATCH(TBL_STATIC_ROUTE_CFG_FLD_NEXTHOP_TYPE_V6, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_STATIC_ROUTE_CFG_FLD_NEXTHOP_TYPE_V6].name,
            p_rst_cfg->nexthop_type_v6);
    }
    if (FLD_MATCH(TBL_STATIC_ROUTE_CFG_FLD_DHCP_CLIENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_STATIC_ROUTE_CFG_FLD_DHCP_CLIENT].name,
            p_rst_cfg->dhcp_client);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_static_route_cfg_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_static_route_cfg_master_t *p_master = _g_p_tbl_static_route_cfg_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_static_route_cfg_t *p_db_rst_cfg = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->rst_cfg_list, p_db_rst_cfg, listnode, next)
    {
        rc |= fn(p_db_rst_cfg, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_static_route_cfg_master_t*
tbl_static_route_cfg_get_master()
{
    return _g_p_tbl_static_route_cfg_master;
}

tbl_static_route_cfg_master_t*
tbl_static_route_cfg_init_static_route_cfg()
{
    _g_p_tbl_static_route_cfg_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_static_route_cfg_master_t));
    _g_p_tbl_static_route_cfg_master->rst_cfg_hash = ctclib_hash_create(_tbl_static_route_cfg_hash_make, _tbl_static_route_cfg_hash_cmp);
    _g_p_tbl_static_route_cfg_master->rst_cfg_list = ctclib_slist_create(tbl_static_route_cfg_cmp, NULL);
    return _g_p_tbl_static_route_cfg_master;
}

