
#include "proto.h"
#include "gen/tbl_dynamic_dns_domain_define.h"
#include "gen/tbl_dynamic_dns_domain.h"
#include "cdb_data_cmp.h"

tbl_dynamic_dns_domain_master_t *_g_p_tbl_dynamic_dns_domain_master = NULL;

int32
tbl_dynamic_dns_domain_add_dynamic_dns_domain_sync(tbl_dynamic_dns_domain_t *p_dynamic_domain_cfg, uint32 sync)
{
    tbl_dynamic_dns_domain_master_t *p_master = _g_p_tbl_dynamic_dns_domain_master;
    tbl_dynamic_dns_domain_t *p_db_dynamic_domain_cfg = NULL;

    /* 1. lookup entry exist */
    if (tbl_dynamic_dns_domain_get_dynamic_dns_domain(&p_dynamic_domain_cfg->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_dynamic_domain_cfg = XCALLOC(MEM_TBL_DYNAMIC_DNS_DOMAIN, sizeof(tbl_dynamic_dns_domain_t));
    if (NULL == p_db_dynamic_domain_cfg)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_dynamic_domain_cfg, p_dynamic_domain_cfg, sizeof(tbl_dynamic_dns_domain_t));

    /* 4. add to db */
    ctclib_slistnode_insert(p_master->dynamic_domain_list, p_db_dynamic_domain_cfg);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_DYNAMIC_DNS_DOMAIN, p_db_dynamic_domain_cfg);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_DYNAMIC_DNS_DOMAIN, p_db_dynamic_domain_cfg);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_dynamic_dns_domain_del_dynamic_dns_domain_sync(tbl_dynamic_dns_domain_key_t *p_dynamic_domain_cfg_key, uint32 sync)
{
    tbl_dynamic_dns_domain_master_t *p_master = _g_p_tbl_dynamic_dns_domain_master;
    tbl_dynamic_dns_domain_t *p_db_dynamic_domain_cfg = NULL;

    /* 1. lookup entry exist */
    p_db_dynamic_domain_cfg = tbl_dynamic_dns_domain_get_dynamic_dns_domain(p_dynamic_domain_cfg_key);
    if (NULL == p_db_dynamic_domain_cfg)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_DYNAMIC_DNS_DOMAIN, p_db_dynamic_domain_cfg);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_DYNAMIC_DNS_DOMAIN, p_db_dynamic_domain_cfg);
        #endif 
    }

    /* 3. delete from db */
    ctclib_slistnode_delete(p_master->dynamic_domain_list, p_db_dynamic_domain_cfg);

    /* 4. free db entry */
    XFREE(MEM_TBL_DYNAMIC_DNS_DOMAIN, p_db_dynamic_domain_cfg);

    return PM_E_NONE;
}

int32
tbl_dynamic_dns_domain_set_dynamic_dns_domain_field_sync(tbl_dynamic_dns_domain_t *p_dynamic_domain_cfg, tbl_dynamic_dns_domain_field_id_t field_id, uint32 sync)
{
    tbl_dynamic_dns_domain_t *p_db_dynamic_domain_cfg = NULL;

    /* 1. lookup entry exist */
    p_db_dynamic_domain_cfg = tbl_dynamic_dns_domain_get_dynamic_dns_domain(&p_dynamic_domain_cfg->key);
    if (NULL == p_db_dynamic_domain_cfg)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_DYNAMIC_DNS_DOMAIN_FLD_MAX:
        sal_memcpy(p_db_dynamic_domain_cfg, p_dynamic_domain_cfg, sizeof(tbl_dynamic_dns_domain_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_DYNAMIC_DNS_DOMAIN, field_id, p_db_dynamic_domain_cfg);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_DYNAMIC_DNS_DOMAIN, field_id, p_db_dynamic_domain_cfg);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_dynamic_dns_domain_add_dynamic_dns_domain(tbl_dynamic_dns_domain_t *p_dynamic_domain_cfg)
{
    return tbl_dynamic_dns_domain_add_dynamic_dns_domain_sync(p_dynamic_domain_cfg, TRUE);
}

int32
tbl_dynamic_dns_domain_del_dynamic_dns_domain(tbl_dynamic_dns_domain_key_t *p_dynamic_domain_cfg_key)
{
    return tbl_dynamic_dns_domain_del_dynamic_dns_domain_sync(p_dynamic_domain_cfg_key, TRUE);
}

int32
tbl_dynamic_dns_domain_set_dynamic_dns_domain_field(tbl_dynamic_dns_domain_t *p_dynamic_domain_cfg, tbl_dynamic_dns_domain_field_id_t field_id)
{
    return tbl_dynamic_dns_domain_set_dynamic_dns_domain_field_sync(p_dynamic_domain_cfg, field_id, TRUE);
}

tbl_dynamic_dns_domain_t*
tbl_dynamic_dns_domain_get_dynamic_dns_domain(tbl_dynamic_dns_domain_key_t *p_dynamic_domain_cfg_key)
{
    tbl_dynamic_dns_domain_master_t *p_master = _g_p_tbl_dynamic_dns_domain_master;
    tbl_dynamic_dns_domain_t lkp;

    sal_memcpy(&lkp.key, p_dynamic_domain_cfg_key, sizeof(tbl_dynamic_dns_domain_key_t));
    return ctclib_slistdata_lookup(p_master->dynamic_domain_list, &lkp);
}

char*
tbl_dynamic_dns_domain_key_val2str(tbl_dynamic_dns_domain_t *p_dynamic_domain_cfg, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_dynamic_domain_cfg->key.name);
    return str;
}

int32
tbl_dynamic_dns_domain_key_str2val(char *str, tbl_dynamic_dns_domain_t *p_dynamic_domain_cfg)
{
    int32 ret = 0;

    sal_strcpy(p_dynamic_domain_cfg->key.name, str);
    return ret;
}

char*
tbl_dynamic_dns_domain_key_name_dump(tbl_dynamic_dns_domain_t *p_dynamic_domain_cfg, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_DYNAMIC_DNS_DOMAIN);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_dynamic_domain_cfg->key.name);
    return str;
}

char*
tbl_dynamic_dns_domain_key_value_dump(tbl_dynamic_dns_domain_t *p_dynamic_domain_cfg, char *str)
{
    sal_sprintf(str, "%s", p_dynamic_domain_cfg->key.name);
    return str;
}

char*
tbl_dynamic_dns_domain_field_name_dump(tbl_dynamic_dns_domain_t *p_dynamic_domain_cfg, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_DYNAMIC_DNS_DOMAIN);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_DYNAMIC_DNS_DOMAIN_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_dynamic_dns_domain_field_value_dump(tbl_dynamic_dns_domain_t *p_dynamic_domain_cfg, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_DYNAMIC_DNS_DOMAIN_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_dynamic_domain_cfg->key.name);
    }
    return str;
}

char**
tbl_dynamic_dns_domain_table_dump(tbl_dynamic_dns_domain_t *p_dynamic_domain_cfg, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_dynamic_dns_domain_key_name_dump(p_dynamic_domain_cfg, buf));
    for(i=1; i<TBL_DYNAMIC_DNS_DOMAIN_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_dynamic_dns_domain_field_name_dump(p_dynamic_domain_cfg, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_dynamic_dns_domain_field_value_dump(p_dynamic_domain_cfg, i, buf));
    }
    return str;
}

int32
tbl_dynamic_dns_domain_field_value_parser(char *str, int32 field_id, tbl_dynamic_dns_domain_t *p_dynamic_domain_cfg)
{
    if (FLD_MATCH(TBL_DYNAMIC_DNS_DOMAIN_FLD_KEY, field_id))
    {
        sal_strcpy(p_dynamic_domain_cfg->key.name, str);
    }
    return PM_E_NONE;
}

int32
tbl_dynamic_dns_domain_table_parser(char** array, char* key_value,tbl_dynamic_dns_domain_t *p_dynamic_domain_cfg)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_DYNAMIC_DNS_DOMAIN);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_dynamic_dns_domain_key_str2val(key_value, p_dynamic_domain_cfg));

    for(i=1; i<TBL_DYNAMIC_DNS_DOMAIN_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_dynamic_dns_domain_field_value_parser( array[j++], i, p_dynamic_domain_cfg));
    }

    return PM_E_NONE;
}

int32
tbl_dynamic_dns_domain_dump_one(tbl_dynamic_dns_domain_t *p_dynamic_domain_cfg, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_DYNAMIC_DNS_DOMAIN);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_DYNAMIC_DNS_DOMAIN_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DYNAMIC_DNS_DOMAIN_FLD_KEY].name,
            p_dynamic_domain_cfg->key.name);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_dynamic_dns_domain_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_dynamic_dns_domain_master_t *p_master = _g_p_tbl_dynamic_dns_domain_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_dynamic_dns_domain_t *p_db_dynamic_domain_cfg = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->dynamic_domain_list, p_db_dynamic_domain_cfg, listnode, next)
    {
        rc |= fn(p_db_dynamic_domain_cfg, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_dynamic_dns_domain_master_t*
tbl_dynamic_dns_domain_get_master()
{
    return _g_p_tbl_dynamic_dns_domain_master;
}

tbl_dynamic_dns_domain_master_t*
tbl_dynamic_dns_domain_init_dynamic_dns_domain()
{
    _g_p_tbl_dynamic_dns_domain_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_dynamic_dns_domain_master_t));
    _g_p_tbl_dynamic_dns_domain_master->dynamic_domain_list = ctclib_slist_create(tbl_dynamic_dns_domain_cmp, NULL);
    return _g_p_tbl_dynamic_dns_domain_master;
}

