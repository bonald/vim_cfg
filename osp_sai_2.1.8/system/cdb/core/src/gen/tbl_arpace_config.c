
#include "proto.h"
#include "gen/tbl_arpace_config_define.h"
#include "gen/tbl_arpace_config.h"
#include "cdb_data_cmp.h"

tbl_arpace_config_master_t *_g_p_tbl_arpace_config_master = NULL;

static uint32
_tbl_arpace_config_hash_make(tbl_arpace_config_t *p_arpace_config)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_arpace_config->key;
    for (index = 0; index < sizeof(p_arpace_config->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_arpace_config_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_arpace_config_t *p_arpace_config1 = (tbl_arpace_config_t*)p_arg1;
    tbl_arpace_config_t *p_arpace_config2 = (tbl_arpace_config_t*)p_arg2;

    if (0 == sal_memcmp(&p_arpace_config1->key, &p_arpace_config2->key, sizeof(tbl_arpace_config_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_arpace_config_add_arpace_config_sync(tbl_arpace_config_t *p_arpace_config, uint32 sync)
{
    tbl_arpace_config_master_t *p_master = _g_p_tbl_arpace_config_master;
    tbl_arpace_config_t *p_db_arpace_config = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_arpace_config_get_arpace_config(&p_arpace_config->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_arpace_config = XCALLOC(MEM_TBL_ARPACE_CONFIG, sizeof(tbl_arpace_config_t));
    if (NULL == p_db_arpace_config)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_arpace_config, p_arpace_config, sizeof(tbl_arpace_config_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->arpace_config_hash, (void*)p_db_arpace_config, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->arpace_config_list, p_db_arpace_config);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_ARPACE_CONFIG, p_db_arpace_config);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_ARPACE_CONFIG, p_db_arpace_config);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_arpace_config_del_arpace_config_sync(tbl_arpace_config_key_t *p_arpace_config_key, uint32 sync)
{
    tbl_arpace_config_master_t *p_master = _g_p_tbl_arpace_config_master;
    tbl_arpace_config_t *p_db_arpace_config = NULL;

    /* 1. lookup entry exist */
    p_db_arpace_config = tbl_arpace_config_get_arpace_config(p_arpace_config_key);
    if (NULL == p_db_arpace_config)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_ARPACE_CONFIG, p_db_arpace_config);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_ARPACE_CONFIG, p_db_arpace_config);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->arpace_config_hash, (void*)p_db_arpace_config);
    ctclib_slistnode_delete(p_master->arpace_config_list, p_db_arpace_config);

    /* 4. free db entry */
    XFREE(MEM_TBL_ARPACE_CONFIG, p_db_arpace_config);

    return PM_E_NONE;
}

int32
tbl_arpace_config_set_arpace_config_field_sync(tbl_arpace_config_t *p_arpace_config, tbl_arpace_config_field_id_t field_id, uint32 sync)
{
    tbl_arpace_config_t *p_db_arpace_config = NULL;

    /* 1. lookup entry exist */
    p_db_arpace_config = tbl_arpace_config_get_arpace_config(&p_arpace_config->key);
    if (NULL == p_db_arpace_config)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_ARPACE_CONFIG_FLD_ARP_TYPE:
        p_db_arpace_config->arp_type = p_arpace_config->arp_type;
        break;

    case TBL_ARPACE_CONFIG_FLD_SRC_MAC:
        sal_memcpy(p_db_arpace_config->src_mac, p_arpace_config->src_mac, sizeof(p_arpace_config->src_mac));
        break;

    case TBL_ARPACE_CONFIG_FLD_SRC_MAC_MASK:
        sal_memcpy(p_db_arpace_config->src_mac_mask, p_arpace_config->src_mac_mask, sizeof(p_arpace_config->src_mac_mask));
        break;

    case TBL_ARPACE_CONFIG_FLD_SRC_IP:
        sal_memcpy(&p_db_arpace_config->src_ip, &p_arpace_config->src_ip, sizeof(p_arpace_config->src_ip));
        break;

    case TBL_ARPACE_CONFIG_FLD_SRC_IP_MASK:
        sal_memcpy(&p_db_arpace_config->src_ip_mask, &p_arpace_config->src_ip_mask, sizeof(p_arpace_config->src_ip_mask));
        break;

    case TBL_ARPACE_CONFIG_FLD_ARP_DENY:
        p_db_arpace_config->arp_deny = p_arpace_config->arp_deny;
        break;

    case TBL_ARPACE_CONFIG_FLD_ARP_LOG:
        p_db_arpace_config->arp_log = p_arpace_config->arp_log;
        break;

    case TBL_ARPACE_CONFIG_FLD_MAX:
        sal_memcpy(p_db_arpace_config, p_arpace_config, sizeof(tbl_arpace_config_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_ARPACE_CONFIG, field_id, p_db_arpace_config);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_ARPACE_CONFIG, field_id, p_db_arpace_config);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_arpace_config_add_arpace_config(tbl_arpace_config_t *p_arpace_config)
{
    return tbl_arpace_config_add_arpace_config_sync(p_arpace_config, TRUE);
}

int32
tbl_arpace_config_del_arpace_config(tbl_arpace_config_key_t *p_arpace_config_key)
{
    return tbl_arpace_config_del_arpace_config_sync(p_arpace_config_key, TRUE);
}

int32
tbl_arpace_config_set_arpace_config_field(tbl_arpace_config_t *p_arpace_config, tbl_arpace_config_field_id_t field_id)
{
    return tbl_arpace_config_set_arpace_config_field_sync(p_arpace_config, field_id, TRUE);
}

tbl_arpace_config_t*
tbl_arpace_config_get_arpace_config(tbl_arpace_config_key_t *p_arpace_config_key)
{
    tbl_arpace_config_master_t *p_master = _g_p_tbl_arpace_config_master;
    tbl_arpace_config_t lkp;

    sal_memcpy(&lkp.key, p_arpace_config_key, sizeof(tbl_arpace_config_key_t));
    return ctclib_hash_lookup(p_master->arpace_config_hash, &lkp);
}

char*
tbl_arpace_config_key_val2str(tbl_arpace_config_t *p_arpace_config, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_arpace_config_key_val2str(buf, MAX_CMD_STR_LEN, &p_arpace_config->key));
    return str;
}

int32
tbl_arpace_config_key_str2val(char *str, tbl_arpace_config_t *p_arpace_config)
{
    int32 ret = 0;

    ret = cdb_arpace_config_key_str2val(str, &p_arpace_config->key);
    return ret;
}

char*
tbl_arpace_config_key_name_dump(tbl_arpace_config_t *p_arpace_config, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ARPACE_CONFIG);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_arpace_config_key_val2str(buf, MAX_CMD_STR_LEN, &p_arpace_config->key));
    return str;
}

char*
tbl_arpace_config_key_value_dump(tbl_arpace_config_t *p_arpace_config, char *str)
{

    cdb_arpace_config_key_val2str(str, MAX_CMD_STR_LEN, &p_arpace_config->key);
    return str;
}

char*
tbl_arpace_config_field_name_dump(tbl_arpace_config_t *p_arpace_config, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_ARPACE_CONFIG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_ARPACE_CONFIG_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_arpace_config_field_value_dump(tbl_arpace_config_t *p_arpace_config, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_ARPACE_CONFIG_FLD_KEY, field_id))
    {
        cdb_arpace_config_key_val2str(str, MAX_CMD_STR_LEN, &p_arpace_config->key);
    }
    if (FLD_MATCH(TBL_ARPACE_CONFIG_FLD_ARP_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_arpace_config->arp_type);
    }
    if (FLD_MATCH(TBL_ARPACE_CONFIG_FLD_SRC_MAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_arpace_config->src_mac);
    }
    if (FLD_MATCH(TBL_ARPACE_CONFIG_FLD_SRC_MAC_MASK, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_arpace_config->src_mac_mask);
    }
    if (FLD_MATCH(TBL_ARPACE_CONFIG_FLD_SRC_IP, field_id))
    {
        cdb_addr_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_arpace_config->src_ip);
    }
    if (FLD_MATCH(TBL_ARPACE_CONFIG_FLD_SRC_IP_MASK, field_id))
    {
        cdb_addr_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_arpace_config->src_ip_mask);
    }
    if (FLD_MATCH(TBL_ARPACE_CONFIG_FLD_ARP_DENY, field_id))
    {
        sal_sprintf(str, "%u", p_arpace_config->arp_deny);
    }
    if (FLD_MATCH(TBL_ARPACE_CONFIG_FLD_ARP_LOG, field_id))
    {
        sal_sprintf(str, "%u", p_arpace_config->arp_log);
    }
    return str;
}

char**
tbl_arpace_config_table_dump(tbl_arpace_config_t *p_arpace_config, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_arpace_config_key_name_dump(p_arpace_config, buf));
    for(i=1; i<TBL_ARPACE_CONFIG_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_arpace_config_field_name_dump(p_arpace_config, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_arpace_config_field_value_dump(p_arpace_config, i, buf));
    }
    return str;
}

int32
tbl_arpace_config_field_value_parser(char *str, int32 field_id, tbl_arpace_config_t *p_arpace_config)
{
    if (FLD_MATCH(TBL_ARPACE_CONFIG_FLD_KEY, field_id))
    {
        cdb_arpace_config_key_str2val(str, &p_arpace_config->key);
    }
    if (FLD_MATCH(TBL_ARPACE_CONFIG_FLD_ARP_TYPE, field_id))
    {
        int32 ret;
        p_arpace_config->arp_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ARPACE_CONFIG_FLD_SRC_MAC, field_id))
    {
        cdb_mac_addr_str2val(p_arpace_config->src_mac, str);
    }
    if (FLD_MATCH(TBL_ARPACE_CONFIG_FLD_SRC_MAC_MASK, field_id))
    {
        cdb_mac_addr_str2val(p_arpace_config->src_mac_mask, str);
    }
    if (FLD_MATCH(TBL_ARPACE_CONFIG_FLD_SRC_IP, field_id))
    {
        cdb_addr_ipv4_str2val(&p_arpace_config->src_ip, str, 0);
    }
    if (FLD_MATCH(TBL_ARPACE_CONFIG_FLD_SRC_IP_MASK, field_id))
    {
        cdb_addr_ipv4_str2val(&p_arpace_config->src_ip_mask, str, 0);
    }
    if (FLD_MATCH(TBL_ARPACE_CONFIG_FLD_ARP_DENY, field_id))
    {
        int32 ret;
        p_arpace_config->arp_deny = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ARPACE_CONFIG_FLD_ARP_LOG, field_id))
    {
        int32 ret;
        p_arpace_config->arp_log = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_arpace_config_table_parser(char** array, char* key_value,tbl_arpace_config_t *p_arpace_config)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_ARPACE_CONFIG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_arpace_config_key_str2val(key_value, p_arpace_config));

    for(i=1; i<TBL_ARPACE_CONFIG_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_arpace_config_field_value_parser( array[j++], i, p_arpace_config));
    }

    return PM_E_NONE;
}

int32
tbl_arpace_config_dump_one(tbl_arpace_config_t *p_arpace_config, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ARPACE_CONFIG);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_ARPACE_CONFIG_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ARPACE_CONFIG_FLD_KEY].name, 
            cdb_arpace_config_key_val2str(buf, MAX_CMD_STR_LEN, &p_arpace_config->key));
    }
    if (FLD_MATCH(TBL_ARPACE_CONFIG_FLD_ARP_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ARPACE_CONFIG_FLD_ARP_TYPE].name,
            p_arpace_config->arp_type);
    }
    if (FLD_MATCH(TBL_ARPACE_CONFIG_FLD_SRC_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ARPACE_CONFIG_FLD_SRC_MAC].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_arpace_config->src_mac));
    }
    if (FLD_MATCH(TBL_ARPACE_CONFIG_FLD_SRC_MAC_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ARPACE_CONFIG_FLD_SRC_MAC_MASK].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_arpace_config->src_mac_mask));
    }
    if (FLD_MATCH(TBL_ARPACE_CONFIG_FLD_SRC_IP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ARPACE_CONFIG_FLD_SRC_IP].name, 
            cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_arpace_config->src_ip));
    }
    if (FLD_MATCH(TBL_ARPACE_CONFIG_FLD_SRC_IP_MASK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ARPACE_CONFIG_FLD_SRC_IP_MASK].name, 
            cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_arpace_config->src_ip_mask));
    }
    if (FLD_MATCH(TBL_ARPACE_CONFIG_FLD_ARP_DENY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ARPACE_CONFIG_FLD_ARP_DENY].name,
            p_arpace_config->arp_deny);
    }
    if (FLD_MATCH(TBL_ARPACE_CONFIG_FLD_ARP_LOG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ARPACE_CONFIG_FLD_ARP_LOG].name,
            p_arpace_config->arp_log);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_arpace_config_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_arpace_config_master_t *p_master = _g_p_tbl_arpace_config_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_arpace_config_t *p_db_arpace_config = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->arpace_config_list, p_db_arpace_config, listnode, next)
    {
        rc |= fn(p_db_arpace_config, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_arpace_config_master_t*
tbl_arpace_config_get_master()
{
    return _g_p_tbl_arpace_config_master;
}

tbl_arpace_config_master_t*
tbl_arpace_config_init_arpace_config()
{
    _g_p_tbl_arpace_config_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_arpace_config_master_t));
    _g_p_tbl_arpace_config_master->arpace_config_hash = ctclib_hash_create(_tbl_arpace_config_hash_make, _tbl_arpace_config_hash_cmp);
    _g_p_tbl_arpace_config_master->arpace_config_list = ctclib_slist_create(tbl_arpace_config_cmp, NULL);
    return _g_p_tbl_arpace_config_master;
}

