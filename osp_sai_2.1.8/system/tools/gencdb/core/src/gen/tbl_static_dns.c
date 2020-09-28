
#include "proto.h"
#include "gen/tbl_static_dns_define.h"
#include "gen/tbl_static_dns.h"
#include "cdb_data_cmp.h"

tbl_static_dns_master_t *_g_p_tbl_static_dns_master = NULL;

int32
tbl_static_dns_add_static_dns_sync(tbl_static_dns_t *p_static_cfg, uint32 sync)
{
    tbl_static_dns_master_t *p_master = _g_p_tbl_static_dns_master;
    tbl_static_dns_t *p_db_static_cfg = NULL;

    /* 1. lookup entry exist */
    if (tbl_static_dns_get_static_dns(&p_static_cfg->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_static_cfg = XCALLOC(MEM_TBL_STATIC_DNS, sizeof(tbl_static_dns_t));
    if (NULL == p_db_static_cfg)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_static_cfg, p_static_cfg, sizeof(tbl_static_dns_t));

    /* 4. add to db */
    ctclib_slistnode_insert(p_master->static_list, p_db_static_cfg);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_STATIC_DNS, p_db_static_cfg);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_STATIC_DNS, p_db_static_cfg);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_static_dns_del_static_dns_sync(tbl_static_dns_key_t *p_static_cfg_key, uint32 sync)
{
    tbl_static_dns_master_t *p_master = _g_p_tbl_static_dns_master;
    tbl_static_dns_t *p_db_static_cfg = NULL;

    /* 1. lookup entry exist */
    p_db_static_cfg = tbl_static_dns_get_static_dns(p_static_cfg_key);
    if (NULL == p_db_static_cfg)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_STATIC_DNS, p_db_static_cfg);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_STATIC_DNS, p_db_static_cfg);
        #endif 
    }

    /* 3. delete from db */
    ctclib_slistnode_delete(p_master->static_list, p_db_static_cfg);

    /* 4. free db entry */
    XFREE(MEM_TBL_STATIC_DNS, p_db_static_cfg);

    return PM_E_NONE;
}

int32
tbl_static_dns_set_static_dns_field_sync(tbl_static_dns_t *p_static_cfg, tbl_static_dns_field_id_t field_id, uint32 sync)
{
    tbl_static_dns_t *p_db_static_cfg = NULL;

    /* 1. lookup entry exist */
    p_db_static_cfg = tbl_static_dns_get_static_dns(&p_static_cfg->key);
    if (NULL == p_db_static_cfg)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_STATIC_DNS_FLD_IP_ADDR:
        sal_memcpy(&p_db_static_cfg->ip_addr, &p_static_cfg->ip_addr, sizeof(p_static_cfg->ip_addr));
        break;

    case TBL_STATIC_DNS_FLD_MAX:
        sal_memcpy(p_db_static_cfg, p_static_cfg, sizeof(tbl_static_dns_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_STATIC_DNS, field_id, p_db_static_cfg);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_STATIC_DNS, field_id, p_db_static_cfg);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_static_dns_add_static_dns(tbl_static_dns_t *p_static_cfg)
{
    return tbl_static_dns_add_static_dns_sync(p_static_cfg, TRUE);
}

int32
tbl_static_dns_del_static_dns(tbl_static_dns_key_t *p_static_cfg_key)
{
    return tbl_static_dns_del_static_dns_sync(p_static_cfg_key, TRUE);
}

int32
tbl_static_dns_set_static_dns_field(tbl_static_dns_t *p_static_cfg, tbl_static_dns_field_id_t field_id)
{
    return tbl_static_dns_set_static_dns_field_sync(p_static_cfg, field_id, TRUE);
}

tbl_static_dns_t*
tbl_static_dns_get_static_dns(tbl_static_dns_key_t *p_static_cfg_key)
{
    tbl_static_dns_master_t *p_master = _g_p_tbl_static_dns_master;
    tbl_static_dns_t lkp;

    sal_memcpy(&lkp.key, p_static_cfg_key, sizeof(tbl_static_dns_key_t));
    return ctclib_slistdata_lookup(p_master->static_list, &lkp);
}

char*
tbl_static_dns_key_val2str(tbl_static_dns_t *p_static_cfg, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_static_cfg->key.name);
    return str;
}

int32
tbl_static_dns_key_str2val(char *str, tbl_static_dns_t *p_static_cfg)
{
    int32 ret = 0;

    sal_strcpy(p_static_cfg->key.name, str);
    return ret;
}

char*
tbl_static_dns_key_name_dump(tbl_static_dns_t *p_static_cfg, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_STATIC_DNS);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_static_cfg->key.name);
    return str;
}

char*
tbl_static_dns_key_value_dump(tbl_static_dns_t *p_static_cfg, char *str)
{
    sal_sprintf(str, "%s", p_static_cfg->key.name);
    return str;
}

char*
tbl_static_dns_field_name_dump(tbl_static_dns_t *p_static_cfg, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_STATIC_DNS);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_STATIC_DNS_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_static_dns_field_value_dump(tbl_static_dns_t *p_static_cfg, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_STATIC_DNS_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_static_cfg->key.name);
    }
    if (FLD_MATCH(TBL_STATIC_DNS_FLD_IP_ADDR, field_id))
    {
        cdb_addr_val2str(str, MAX_CMD_STR_LEN, &p_static_cfg->ip_addr);
    }
    return str;
}

char**
tbl_static_dns_table_dump(tbl_static_dns_t *p_static_cfg, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_static_dns_key_name_dump(p_static_cfg, buf));
    for(i=1; i<TBL_STATIC_DNS_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_static_dns_field_name_dump(p_static_cfg, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_static_dns_field_value_dump(p_static_cfg, i, buf));
    }
    return str;
}

int32
tbl_static_dns_field_value_parser(char *str, int32 field_id, tbl_static_dns_t *p_static_cfg)
{
    if (FLD_MATCH(TBL_STATIC_DNS_FLD_KEY, field_id))
    {
        sal_strcpy(p_static_cfg->key.name, str);
    }
    if (FLD_MATCH(TBL_STATIC_DNS_FLD_IP_ADDR, field_id))
    {
        cdb_addr_str2val(&p_static_cfg->ip_addr, str, 0);
    }
    return PM_E_NONE;
}

int32
tbl_static_dns_table_parser(char** array, char* key_value,tbl_static_dns_t *p_static_cfg)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_STATIC_DNS);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_static_dns_key_str2val(key_value, p_static_cfg));

    for(i=1; i<TBL_STATIC_DNS_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_static_dns_field_value_parser( array[j++], i, p_static_cfg));
    }

    return PM_E_NONE;
}

int32
tbl_static_dns_dump_one(tbl_static_dns_t *p_static_cfg, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_STATIC_DNS);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_STATIC_DNS_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_STATIC_DNS_FLD_KEY].name,
            p_static_cfg->key.name);
    }
    if (FLD_MATCH(TBL_STATIC_DNS_FLD_IP_ADDR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_STATIC_DNS_FLD_IP_ADDR].name, 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_static_cfg->ip_addr));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_static_dns_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_static_dns_master_t *p_master = _g_p_tbl_static_dns_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_static_dns_t *p_db_static_cfg = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->static_list, p_db_static_cfg, listnode, next)
    {
        rc |= fn(p_db_static_cfg, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_static_dns_master_t*
tbl_static_dns_get_master()
{
    return _g_p_tbl_static_dns_master;
}

tbl_static_dns_master_t*
tbl_static_dns_init_static_dns()
{
    _g_p_tbl_static_dns_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_static_dns_master_t));
    _g_p_tbl_static_dns_master->static_list = ctclib_slist_create(tbl_static_dns_cmp, NULL);
    return _g_p_tbl_static_dns_master;
}

