
#include "proto.h"
#include "gen/tbl_dynamic_dns_server_define.h"
#include "gen/tbl_dynamic_dns_server.h"
#include "cdb_data_cmp.h"

tbl_dynamic_dns_server_master_t *_g_p_tbl_dynamic_dns_server_master = NULL;

int32
tbl_dynamic_dns_server_add_dynamic_dns_server_sync(tbl_dynamic_dns_server_t *p_dynamic_server_cfg, uint32 sync)
{
    tbl_dynamic_dns_server_master_t *p_master = _g_p_tbl_dynamic_dns_server_master;
    tbl_dynamic_dns_server_t *p_db_dynamic_server_cfg = NULL;

    /* 1. lookup entry exist */
    if (tbl_dynamic_dns_server_get_dynamic_dns_server(&p_dynamic_server_cfg->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_dynamic_server_cfg = XCALLOC(MEM_TBL_DYNAMIC_DNS_SERVER, sizeof(tbl_dynamic_dns_server_t));
    if (NULL == p_db_dynamic_server_cfg)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_dynamic_server_cfg, p_dynamic_server_cfg, sizeof(tbl_dynamic_dns_server_t));

    /* 4. add to db */
    ctclib_slistnode_insert(p_master->dynamic_server_list, p_db_dynamic_server_cfg);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_DYNAMIC_DNS_SERVER, p_db_dynamic_server_cfg);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_DYNAMIC_DNS_SERVER, p_db_dynamic_server_cfg);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_dynamic_dns_server_del_dynamic_dns_server_sync(tbl_dynamic_dns_server_key_t *p_dynamic_server_cfg_key, uint32 sync)
{
    tbl_dynamic_dns_server_master_t *p_master = _g_p_tbl_dynamic_dns_server_master;
    tbl_dynamic_dns_server_t *p_db_dynamic_server_cfg = NULL;

    /* 1. lookup entry exist */
    p_db_dynamic_server_cfg = tbl_dynamic_dns_server_get_dynamic_dns_server(p_dynamic_server_cfg_key);
    if (NULL == p_db_dynamic_server_cfg)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_DYNAMIC_DNS_SERVER, p_db_dynamic_server_cfg);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_DYNAMIC_DNS_SERVER, p_db_dynamic_server_cfg);
        #endif 
    }

    /* 3. delete from db */
    ctclib_slistnode_delete(p_master->dynamic_server_list, p_db_dynamic_server_cfg);

    /* 4. free db entry */
    XFREE(MEM_TBL_DYNAMIC_DNS_SERVER, p_db_dynamic_server_cfg);

    return PM_E_NONE;
}

int32
tbl_dynamic_dns_server_set_dynamic_dns_server_field_sync(tbl_dynamic_dns_server_t *p_dynamic_server_cfg, tbl_dynamic_dns_server_field_id_t field_id, uint32 sync)
{
    tbl_dynamic_dns_server_t *p_db_dynamic_server_cfg = NULL;

    /* 1. lookup entry exist */
    p_db_dynamic_server_cfg = tbl_dynamic_dns_server_get_dynamic_dns_server(&p_dynamic_server_cfg->key);
    if (NULL == p_db_dynamic_server_cfg)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_DYNAMIC_DNS_SERVER_FLD_MAX:
        sal_memcpy(p_db_dynamic_server_cfg, p_dynamic_server_cfg, sizeof(tbl_dynamic_dns_server_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_DYNAMIC_DNS_SERVER, field_id, p_db_dynamic_server_cfg);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_DYNAMIC_DNS_SERVER, field_id, p_db_dynamic_server_cfg);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_dynamic_dns_server_add_dynamic_dns_server(tbl_dynamic_dns_server_t *p_dynamic_server_cfg)
{
    return tbl_dynamic_dns_server_add_dynamic_dns_server_sync(p_dynamic_server_cfg, TRUE);
}

int32
tbl_dynamic_dns_server_del_dynamic_dns_server(tbl_dynamic_dns_server_key_t *p_dynamic_server_cfg_key)
{
    return tbl_dynamic_dns_server_del_dynamic_dns_server_sync(p_dynamic_server_cfg_key, TRUE);
}

int32
tbl_dynamic_dns_server_set_dynamic_dns_server_field(tbl_dynamic_dns_server_t *p_dynamic_server_cfg, tbl_dynamic_dns_server_field_id_t field_id)
{
    return tbl_dynamic_dns_server_set_dynamic_dns_server_field_sync(p_dynamic_server_cfg, field_id, TRUE);
}

tbl_dynamic_dns_server_t*
tbl_dynamic_dns_server_get_dynamic_dns_server(tbl_dynamic_dns_server_key_t *p_dynamic_server_cfg_key)
{
    tbl_dynamic_dns_server_master_t *p_master = _g_p_tbl_dynamic_dns_server_master;
    tbl_dynamic_dns_server_t lkp;

    sal_memcpy(&lkp.key, p_dynamic_server_cfg_key, sizeof(tbl_dynamic_dns_server_key_t));
    return ctclib_slistdata_lookup(p_master->dynamic_server_list, &lkp);
}

char*
tbl_dynamic_dns_server_key_val2str(tbl_dynamic_dns_server_t *p_dynamic_server_cfg, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_dynamic_server_cfg->key.ip_addr));
    return str;
}

int32
tbl_dynamic_dns_server_key_str2val(char *str, tbl_dynamic_dns_server_t *p_dynamic_server_cfg)
{
    int32 ret = 0;

    ret = cdb_addr_str2val(&p_dynamic_server_cfg->key.ip_addr, str, 0);
    return ret;
}

char*
tbl_dynamic_dns_server_key_name_dump(tbl_dynamic_dns_server_t *p_dynamic_server_cfg, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_DYNAMIC_DNS_SERVER);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_dynamic_server_cfg->key.ip_addr));
    return str;
}

char*
tbl_dynamic_dns_server_key_value_dump(tbl_dynamic_dns_server_t *p_dynamic_server_cfg, char *str)
{

    cdb_addr_val2str(str, MAX_CMD_STR_LEN, &p_dynamic_server_cfg->key.ip_addr);
    return str;
}

char*
tbl_dynamic_dns_server_field_name_dump(tbl_dynamic_dns_server_t *p_dynamic_server_cfg, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_DYNAMIC_DNS_SERVER);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_DYNAMIC_DNS_SERVER_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_dynamic_dns_server_field_value_dump(tbl_dynamic_dns_server_t *p_dynamic_server_cfg, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_DYNAMIC_DNS_SERVER_FLD_KEY, field_id))
    {
        cdb_addr_val2str(str, MAX_CMD_STR_LEN, &p_dynamic_server_cfg->key.ip_addr);
    }
    return str;
}

char**
tbl_dynamic_dns_server_table_dump(tbl_dynamic_dns_server_t *p_dynamic_server_cfg, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_dynamic_dns_server_key_name_dump(p_dynamic_server_cfg, buf));
    for(i=1; i<TBL_DYNAMIC_DNS_SERVER_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_dynamic_dns_server_field_name_dump(p_dynamic_server_cfg, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_dynamic_dns_server_field_value_dump(p_dynamic_server_cfg, i, buf));
    }
    return str;
}

int32
tbl_dynamic_dns_server_field_value_parser(char *str, int32 field_id, tbl_dynamic_dns_server_t *p_dynamic_server_cfg)
{
    if (FLD_MATCH(TBL_DYNAMIC_DNS_SERVER_FLD_KEY, field_id))
    {
        cdb_addr_str2val(&p_dynamic_server_cfg->key.ip_addr, str, 0);
    }
    return PM_E_NONE;
}

int32
tbl_dynamic_dns_server_table_parser(char** array, char* key_value,tbl_dynamic_dns_server_t *p_dynamic_server_cfg)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_DYNAMIC_DNS_SERVER);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_dynamic_dns_server_key_str2val(key_value, p_dynamic_server_cfg));

    for(i=1; i<TBL_DYNAMIC_DNS_SERVER_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_dynamic_dns_server_field_value_parser( array[j++], i, p_dynamic_server_cfg));
    }

    return PM_E_NONE;
}

int32
tbl_dynamic_dns_server_dump_one(tbl_dynamic_dns_server_t *p_dynamic_server_cfg, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_DYNAMIC_DNS_SERVER);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_DYNAMIC_DNS_SERVER_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DYNAMIC_DNS_SERVER_FLD_KEY].name, 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_dynamic_server_cfg->key.ip_addr));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_dynamic_dns_server_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_dynamic_dns_server_master_t *p_master = _g_p_tbl_dynamic_dns_server_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_dynamic_dns_server_t *p_db_dynamic_server_cfg = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->dynamic_server_list, p_db_dynamic_server_cfg, listnode, next)
    {
        rc |= fn(p_db_dynamic_server_cfg, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_dynamic_dns_server_master_t*
tbl_dynamic_dns_server_get_master()
{
    return _g_p_tbl_dynamic_dns_server_master;
}

tbl_dynamic_dns_server_master_t*
tbl_dynamic_dns_server_init_dynamic_dns_server()
{
    _g_p_tbl_dynamic_dns_server_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_dynamic_dns_server_master_t));
    _g_p_tbl_dynamic_dns_server_master->dynamic_server_list = ctclib_slist_create(tbl_dynamic_dns_server_cmp, NULL);
    return _g_p_tbl_dynamic_dns_server_master;
}

