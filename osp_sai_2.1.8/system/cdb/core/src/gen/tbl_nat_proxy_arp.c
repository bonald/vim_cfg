
#include "proto.h"
#include "gen/tbl_nat_proxy_arp_define.h"
#include "gen/tbl_nat_proxy_arp.h"
#include "cdb_data_cmp.h"

tbl_nat_proxy_arp_master_t *_g_p_tbl_nat_proxy_arp_master = NULL;

static uint32
_tbl_nat_proxy_arp_hash_make(tbl_nat_proxy_arp_t *p_nat_proxy_arp)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_nat_proxy_arp->key;
    for (index = 0; index < sizeof(p_nat_proxy_arp->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_nat_proxy_arp_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_nat_proxy_arp_t *p_nat_proxy_arp1 = (tbl_nat_proxy_arp_t*)p_arg1;
    tbl_nat_proxy_arp_t *p_nat_proxy_arp2 = (tbl_nat_proxy_arp_t*)p_arg2;

    if (0 == sal_memcmp(&p_nat_proxy_arp1->key, &p_nat_proxy_arp2->key, sizeof(tbl_nat_proxy_arp_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_nat_proxy_arp_add_nat_proxy_arp_sync(tbl_nat_proxy_arp_t *p_nat_proxy_arp, uint32 sync)
{
    tbl_nat_proxy_arp_master_t *p_master = _g_p_tbl_nat_proxy_arp_master;
    tbl_nat_proxy_arp_t *p_db_nat_proxy_arp = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_nat_proxy_arp_get_nat_proxy_arp(&p_nat_proxy_arp->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_nat_proxy_arp = XCALLOC(MEM_TBL_NAT_PROXY_ARP, sizeof(tbl_nat_proxy_arp_t));
    if (NULL == p_db_nat_proxy_arp)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_nat_proxy_arp, p_nat_proxy_arp, sizeof(tbl_nat_proxy_arp_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->nat_proxy_arp_hash, (void*)p_db_nat_proxy_arp, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->nat_proxy_arp_list, p_db_nat_proxy_arp);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_NAT_PROXY_ARP, p_db_nat_proxy_arp);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_NAT_PROXY_ARP, p_db_nat_proxy_arp);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_nat_proxy_arp_del_nat_proxy_arp_sync(tbl_nat_proxy_arp_key_t *p_nat_proxy_arp_key, uint32 sync)
{
    tbl_nat_proxy_arp_master_t *p_master = _g_p_tbl_nat_proxy_arp_master;
    tbl_nat_proxy_arp_t *p_db_nat_proxy_arp = NULL;

    /* 1. lookup entry exist */
    p_db_nat_proxy_arp = tbl_nat_proxy_arp_get_nat_proxy_arp(p_nat_proxy_arp_key);
    if (NULL == p_db_nat_proxy_arp)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_NAT_PROXY_ARP, p_db_nat_proxy_arp);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_NAT_PROXY_ARP, p_db_nat_proxy_arp);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->nat_proxy_arp_hash, (void*)p_db_nat_proxy_arp);
    ctclib_slistnode_delete(p_master->nat_proxy_arp_list, p_db_nat_proxy_arp);

    /* 4. free db entry */
    XFREE(MEM_TBL_NAT_PROXY_ARP, p_db_nat_proxy_arp);

    return PM_E_NONE;
}

int32
tbl_nat_proxy_arp_set_nat_proxy_arp_field_sync(tbl_nat_proxy_arp_t *p_nat_proxy_arp, tbl_nat_proxy_arp_field_id_t field_id, uint32 sync)
{
    tbl_nat_proxy_arp_t *p_db_nat_proxy_arp = NULL;

    /* 1. lookup entry exist */
    p_db_nat_proxy_arp = tbl_nat_proxy_arp_get_nat_proxy_arp(&p_nat_proxy_arp->key);
    if (NULL == p_db_nat_proxy_arp)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_NAT_PROXY_ARP_FLD_ENABLE:
        p_db_nat_proxy_arp->enable = p_nat_proxy_arp->enable;
        break;

    case TBL_NAT_PROXY_ARP_FLD_MAX:
        sal_memcpy(p_db_nat_proxy_arp, p_nat_proxy_arp, sizeof(tbl_nat_proxy_arp_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_NAT_PROXY_ARP, field_id, p_db_nat_proxy_arp);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_NAT_PROXY_ARP, field_id, p_db_nat_proxy_arp);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_nat_proxy_arp_add_nat_proxy_arp(tbl_nat_proxy_arp_t *p_nat_proxy_arp)
{
    return tbl_nat_proxy_arp_add_nat_proxy_arp_sync(p_nat_proxy_arp, TRUE);
}

int32
tbl_nat_proxy_arp_del_nat_proxy_arp(tbl_nat_proxy_arp_key_t *p_nat_proxy_arp_key)
{
    return tbl_nat_proxy_arp_del_nat_proxy_arp_sync(p_nat_proxy_arp_key, TRUE);
}

int32
tbl_nat_proxy_arp_set_nat_proxy_arp_field(tbl_nat_proxy_arp_t *p_nat_proxy_arp, tbl_nat_proxy_arp_field_id_t field_id)
{
    return tbl_nat_proxy_arp_set_nat_proxy_arp_field_sync(p_nat_proxy_arp, field_id, TRUE);
}

tbl_nat_proxy_arp_t*
tbl_nat_proxy_arp_get_nat_proxy_arp(tbl_nat_proxy_arp_key_t *p_nat_proxy_arp_key)
{
    tbl_nat_proxy_arp_master_t *p_master = _g_p_tbl_nat_proxy_arp_master;
    tbl_nat_proxy_arp_t lkp;

    sal_memcpy(&lkp.key, p_nat_proxy_arp_key, sizeof(tbl_nat_proxy_arp_key_t));
    return ctclib_hash_lookup(p_master->nat_proxy_arp_hash, &lkp);
}

char*
tbl_nat_proxy_arp_key_val2str(tbl_nat_proxy_arp_t *p_nat_proxy_arp, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_nat_proxy_arp_key_val2str(buf, MAX_CMD_STR_LEN, &p_nat_proxy_arp->key));
    return str;
}

int32
tbl_nat_proxy_arp_key_str2val(char *str, tbl_nat_proxy_arp_t *p_nat_proxy_arp)
{
    int32 ret = 0;

    ret = cdb_nat_proxy_arp_key_str2val(str, &p_nat_proxy_arp->key);
    return ret;
}

char*
tbl_nat_proxy_arp_key_name_dump(tbl_nat_proxy_arp_t *p_nat_proxy_arp, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_NAT_PROXY_ARP);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_nat_proxy_arp_key_val2str(buf, MAX_CMD_STR_LEN, &p_nat_proxy_arp->key));
    return str;
}

char*
tbl_nat_proxy_arp_key_value_dump(tbl_nat_proxy_arp_t *p_nat_proxy_arp, char *str)
{

    cdb_nat_proxy_arp_key_val2str(str, MAX_CMD_STR_LEN, &p_nat_proxy_arp->key);
    return str;
}

char*
tbl_nat_proxy_arp_field_name_dump(tbl_nat_proxy_arp_t *p_nat_proxy_arp, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_NAT_PROXY_ARP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_NAT_PROXY_ARP_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_nat_proxy_arp_field_value_dump(tbl_nat_proxy_arp_t *p_nat_proxy_arp, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_NAT_PROXY_ARP_FLD_KEY, field_id))
    {
        cdb_nat_proxy_arp_key_val2str(str, MAX_CMD_STR_LEN, &p_nat_proxy_arp->key);
    }
    if (FLD_MATCH(TBL_NAT_PROXY_ARP_FLD_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_nat_proxy_arp->enable);
    }
    return str;
}

char**
tbl_nat_proxy_arp_table_dump(tbl_nat_proxy_arp_t *p_nat_proxy_arp, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_nat_proxy_arp_key_name_dump(p_nat_proxy_arp, buf));
    for(i=1; i<TBL_NAT_PROXY_ARP_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_nat_proxy_arp_field_name_dump(p_nat_proxy_arp, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_nat_proxy_arp_field_value_dump(p_nat_proxy_arp, i, buf));
    }
    return str;
}

int32
tbl_nat_proxy_arp_field_value_parser(char *str, int32 field_id, tbl_nat_proxy_arp_t *p_nat_proxy_arp)
{
    if (FLD_MATCH(TBL_NAT_PROXY_ARP_FLD_KEY, field_id))
    {
        cdb_nat_proxy_arp_key_str2val(str, &p_nat_proxy_arp->key);
    }
    if (FLD_MATCH(TBL_NAT_PROXY_ARP_FLD_ENABLE, field_id))
    {
        int32 ret;
        p_nat_proxy_arp->enable = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_nat_proxy_arp_table_parser(char** array, char* key_value,tbl_nat_proxy_arp_t *p_nat_proxy_arp)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_NAT_PROXY_ARP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_nat_proxy_arp_key_str2val(key_value, p_nat_proxy_arp));

    for(i=1; i<TBL_NAT_PROXY_ARP_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_nat_proxy_arp_field_value_parser( array[j++], i, p_nat_proxy_arp));
    }

    return PM_E_NONE;
}

int32
tbl_nat_proxy_arp_dump_one(tbl_nat_proxy_arp_t *p_nat_proxy_arp, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_NAT_PROXY_ARP);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_NAT_PROXY_ARP_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_NAT_PROXY_ARP_FLD_KEY].name, 
            cdb_nat_proxy_arp_key_val2str(buf, MAX_CMD_STR_LEN, &p_nat_proxy_arp->key));
    }
    if (FLD_MATCH(TBL_NAT_PROXY_ARP_FLD_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NAT_PROXY_ARP_FLD_ENABLE].name,
            p_nat_proxy_arp->enable);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_nat_proxy_arp_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_nat_proxy_arp_master_t *p_master = _g_p_tbl_nat_proxy_arp_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_nat_proxy_arp_t *p_db_nat_proxy_arp = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->nat_proxy_arp_list, p_db_nat_proxy_arp, listnode, next)
    {
        rc |= fn(p_db_nat_proxy_arp, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_nat_proxy_arp_master_t*
tbl_nat_proxy_arp_get_master()
{
    return _g_p_tbl_nat_proxy_arp_master;
}

tbl_nat_proxy_arp_master_t*
tbl_nat_proxy_arp_init_nat_proxy_arp()
{
    _g_p_tbl_nat_proxy_arp_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_nat_proxy_arp_master_t));
    _g_p_tbl_nat_proxy_arp_master->nat_proxy_arp_hash = ctclib_hash_create(_tbl_nat_proxy_arp_hash_make, _tbl_nat_proxy_arp_hash_cmp);
    _g_p_tbl_nat_proxy_arp_master->nat_proxy_arp_list = ctclib_slist_create(tbl_nat_proxy_arp_cmp, NULL);
    return _g_p_tbl_nat_proxy_arp_master;
}

