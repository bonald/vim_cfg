
#include "proto.h"
#include "gen/tbl_ipsg_s_mac_define.h"
#include "gen/tbl_ipsg_s_mac.h"
#include "cdb_data_cmp.h"

tbl_ipsg_s_mac_master_t *_g_p_tbl_ipsg_s_mac_master = NULL;

static uint32
_tbl_ipsg_s_mac_hash_make(tbl_ipsg_s_mac_t *p_ipsg_s_mac)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_ipsg_s_mac->key;
    for (index = 0; index < sizeof(p_ipsg_s_mac->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_ipsg_s_mac_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_ipsg_s_mac_t *p_ipsg_s_mac1 = (tbl_ipsg_s_mac_t*)p_arg1;
    tbl_ipsg_s_mac_t *p_ipsg_s_mac2 = (tbl_ipsg_s_mac_t*)p_arg2;

    if (0 == sal_memcmp(&p_ipsg_s_mac1->key, &p_ipsg_s_mac2->key, sizeof(tbl_ipsg_s_mac_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_ipsg_s_mac_add_ipsg_s_mac_sync(tbl_ipsg_s_mac_t *p_ipsg_s_mac, uint32 sync)
{
    tbl_ipsg_s_mac_master_t *p_master = _g_p_tbl_ipsg_s_mac_master;
    tbl_ipsg_s_mac_t *p_db_ipsg_s_mac = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_ipsg_s_mac_get_ipsg_s_mac(&p_ipsg_s_mac->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_ipsg_s_mac = XCALLOC(MEM_TBL_IPSG_S_MAC, sizeof(tbl_ipsg_s_mac_t));
    if (NULL == p_db_ipsg_s_mac)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_ipsg_s_mac, p_ipsg_s_mac, sizeof(tbl_ipsg_s_mac_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->ipsg_s_mac_hash, (void*)p_db_ipsg_s_mac, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->ipsg_s_mac_list, p_db_ipsg_s_mac);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_IPSG_S_MAC, p_db_ipsg_s_mac);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_IPSG_S_MAC, p_db_ipsg_s_mac);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ipsg_s_mac_del_ipsg_s_mac_sync(tbl_ipsg_s_mac_key_t *p_ipsg_s_mac_key, uint32 sync)
{
    tbl_ipsg_s_mac_master_t *p_master = _g_p_tbl_ipsg_s_mac_master;
    tbl_ipsg_s_mac_t *p_db_ipsg_s_mac = NULL;

    /* 1. lookup entry exist */
    p_db_ipsg_s_mac = tbl_ipsg_s_mac_get_ipsg_s_mac(p_ipsg_s_mac_key);
    if (NULL == p_db_ipsg_s_mac)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_IPSG_S_MAC, p_db_ipsg_s_mac);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_IPSG_S_MAC, p_db_ipsg_s_mac);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->ipsg_s_mac_hash, (void*)p_db_ipsg_s_mac);
    ctclib_slistnode_delete(p_master->ipsg_s_mac_list, p_db_ipsg_s_mac);

    /* 4. free db entry */
    XFREE(MEM_TBL_IPSG_S_MAC, p_db_ipsg_s_mac);

    return PM_E_NONE;
}

int32
tbl_ipsg_s_mac_set_ipsg_s_mac_field_sync(tbl_ipsg_s_mac_t *p_ipsg_s_mac, tbl_ipsg_s_mac_field_id_t field_id, uint32 sync)
{
    tbl_ipsg_s_mac_t *p_db_ipsg_s_mac = NULL;

    /* 1. lookup entry exist */
    p_db_ipsg_s_mac = tbl_ipsg_s_mac_get_ipsg_s_mac(&p_ipsg_s_mac->key);
    if (NULL == p_db_ipsg_s_mac)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_IPSG_S_MAC_FLD_VID:
        p_db_ipsg_s_mac->vid = p_ipsg_s_mac->vid;
        break;

    case TBL_IPSG_S_MAC_FLD_IFINDEX:
        p_db_ipsg_s_mac->ifindex = p_ipsg_s_mac->ifindex;
        break;

    case TBL_IPSG_S_MAC_FLD_IFNAME:
        sal_memcpy(p_db_ipsg_s_mac->ifname, p_ipsg_s_mac->ifname, sizeof(p_ipsg_s_mac->ifname));
        break;

    case TBL_IPSG_S_MAC_FLD_MAC_ADDR:
        sal_memcpy(p_db_ipsg_s_mac->mac_addr, p_ipsg_s_mac->mac_addr, sizeof(p_ipsg_s_mac->mac_addr));
        break;

    case TBL_IPSG_S_MAC_FLD_IP_ADDR:
        sal_memcpy(&p_db_ipsg_s_mac->ip_addr, &p_ipsg_s_mac->ip_addr, sizeof(p_ipsg_s_mac->ip_addr));
        break;

    case TBL_IPSG_S_MAC_FLD_MAX:
        sal_memcpy(p_db_ipsg_s_mac, p_ipsg_s_mac, sizeof(tbl_ipsg_s_mac_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_IPSG_S_MAC, field_id, p_db_ipsg_s_mac);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_IPSG_S_MAC, field_id, p_db_ipsg_s_mac);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ipsg_s_mac_add_ipsg_s_mac(tbl_ipsg_s_mac_t *p_ipsg_s_mac)
{
    return tbl_ipsg_s_mac_add_ipsg_s_mac_sync(p_ipsg_s_mac, TRUE);
}

int32
tbl_ipsg_s_mac_del_ipsg_s_mac(tbl_ipsg_s_mac_key_t *p_ipsg_s_mac_key)
{
    return tbl_ipsg_s_mac_del_ipsg_s_mac_sync(p_ipsg_s_mac_key, TRUE);
}

int32
tbl_ipsg_s_mac_set_ipsg_s_mac_field(tbl_ipsg_s_mac_t *p_ipsg_s_mac, tbl_ipsg_s_mac_field_id_t field_id)
{
    return tbl_ipsg_s_mac_set_ipsg_s_mac_field_sync(p_ipsg_s_mac, field_id, TRUE);
}

tbl_ipsg_s_mac_t*
tbl_ipsg_s_mac_get_ipsg_s_mac(tbl_ipsg_s_mac_key_t *p_ipsg_s_mac_key)
{
    tbl_ipsg_s_mac_master_t *p_master = _g_p_tbl_ipsg_s_mac_master;
    tbl_ipsg_s_mac_t lkp;

    sal_memcpy(&lkp.key, p_ipsg_s_mac_key, sizeof(tbl_ipsg_s_mac_key_t));
    return ctclib_hash_lookup(p_master->ipsg_s_mac_hash, &lkp);
}

char*
tbl_ipsg_s_mac_key_val2str(tbl_ipsg_s_mac_t *p_ipsg_s_mac, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_ipsg_s_mac->key.mac));
    return str;
}

int32
tbl_ipsg_s_mac_key_str2val(char *str, tbl_ipsg_s_mac_t *p_ipsg_s_mac)
{
    int32 ret = 0;

    ret = cdb_mac_addr_str2val(p_ipsg_s_mac->key.mac, str);
    return ret;
}

char*
tbl_ipsg_s_mac_key_name_dump(tbl_ipsg_s_mac_t *p_ipsg_s_mac, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_IPSG_S_MAC);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_ipsg_s_mac->key.mac));
    return str;
}

char*
tbl_ipsg_s_mac_key_value_dump(tbl_ipsg_s_mac_t *p_ipsg_s_mac, char *str)
{

    cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_ipsg_s_mac->key.mac);
    return str;
}

char*
tbl_ipsg_s_mac_field_name_dump(tbl_ipsg_s_mac_t *p_ipsg_s_mac, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_IPSG_S_MAC);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_IPSG_S_MAC_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_ipsg_s_mac_field_value_dump(tbl_ipsg_s_mac_t *p_ipsg_s_mac, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_IPSG_S_MAC_FLD_KEY, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_ipsg_s_mac->key.mac);
    }
    if (FLD_MATCH(TBL_IPSG_S_MAC_FLD_VID, field_id))
    {
        sal_sprintf(str, "%u", p_ipsg_s_mac->vid);
    }
    if (FLD_MATCH(TBL_IPSG_S_MAC_FLD_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_ipsg_s_mac->ifindex);
    }
    if (FLD_MATCH(TBL_IPSG_S_MAC_FLD_IFNAME, field_id))
    {
        sal_sprintf(str, "%s", p_ipsg_s_mac->ifname);
    }
    if (FLD_MATCH(TBL_IPSG_S_MAC_FLD_MAC_ADDR, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_ipsg_s_mac->mac_addr);
    }
    if (FLD_MATCH(TBL_IPSG_S_MAC_FLD_IP_ADDR, field_id))
    {
        cdb_prefix_val2str(str, MAX_CMD_STR_LEN, &p_ipsg_s_mac->ip_addr);
    }
    return str;
}

char**
tbl_ipsg_s_mac_table_dump(tbl_ipsg_s_mac_t *p_ipsg_s_mac, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_ipsg_s_mac_key_name_dump(p_ipsg_s_mac, buf));
    for(i=1; i<TBL_IPSG_S_MAC_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_ipsg_s_mac_field_name_dump(p_ipsg_s_mac, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_ipsg_s_mac_field_value_dump(p_ipsg_s_mac, i, buf));
    }
    return str;
}

int32
tbl_ipsg_s_mac_field_value_parser(char *str, int32 field_id, tbl_ipsg_s_mac_t *p_ipsg_s_mac)
{
    if (FLD_MATCH(TBL_IPSG_S_MAC_FLD_KEY, field_id))
    {
        cdb_mac_addr_str2val(p_ipsg_s_mac->key.mac, str);
    }
    if (FLD_MATCH(TBL_IPSG_S_MAC_FLD_VID, field_id))
    {
        int32 ret;
        p_ipsg_s_mac->vid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSG_S_MAC_FLD_IFINDEX, field_id))
    {
        int32 ret;
        p_ipsg_s_mac->ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_IPSG_S_MAC_FLD_IFNAME, field_id))
    {
        sal_strcpy(p_ipsg_s_mac->ifname, str);
    }
    if (FLD_MATCH(TBL_IPSG_S_MAC_FLD_MAC_ADDR, field_id))
    {
        cdb_mac_addr_str2val(p_ipsg_s_mac->mac_addr, str);
    }
    if (FLD_MATCH(TBL_IPSG_S_MAC_FLD_IP_ADDR, field_id))
    {
        cdb_prefix_str2val_wo_masklen(str, &p_ipsg_s_mac->ip_addr);
    }
    return PM_E_NONE;
}

int32
tbl_ipsg_s_mac_table_parser(char** array, char* key_value,tbl_ipsg_s_mac_t *p_ipsg_s_mac)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_IPSG_S_MAC);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_ipsg_s_mac_key_str2val(key_value, p_ipsg_s_mac));

    for(i=1; i<TBL_IPSG_S_MAC_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_ipsg_s_mac_field_value_parser( array[j++], i, p_ipsg_s_mac));
    }

    return PM_E_NONE;
}

int32
tbl_ipsg_s_mac_dump_one(tbl_ipsg_s_mac_t *p_ipsg_s_mac, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_IPSG_S_MAC);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_IPSG_S_MAC_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IPSG_S_MAC_FLD_KEY].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_ipsg_s_mac->key.mac));
    }
    if (FLD_MATCH(TBL_IPSG_S_MAC_FLD_VID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSG_S_MAC_FLD_VID].name,
            p_ipsg_s_mac->vid);
    }
    if (FLD_MATCH(TBL_IPSG_S_MAC_FLD_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_IPSG_S_MAC_FLD_IFINDEX].name,
            p_ipsg_s_mac->ifindex);
    }
    if (FLD_MATCH(TBL_IPSG_S_MAC_FLD_IFNAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IPSG_S_MAC_FLD_IFNAME].name,
            p_ipsg_s_mac->ifname);
    }
    if (FLD_MATCH(TBL_IPSG_S_MAC_FLD_MAC_ADDR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IPSG_S_MAC_FLD_MAC_ADDR].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_ipsg_s_mac->mac_addr));
    }
    if (FLD_MATCH(TBL_IPSG_S_MAC_FLD_IP_ADDR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_IPSG_S_MAC_FLD_IP_ADDR].name, 
            cdb_prefix_val2str(buf, MAX_CMD_STR_LEN, &p_ipsg_s_mac->ip_addr));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_ipsg_s_mac_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_ipsg_s_mac_master_t *p_master = _g_p_tbl_ipsg_s_mac_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_ipsg_s_mac_t *p_db_ipsg_s_mac = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->ipsg_s_mac_list, p_db_ipsg_s_mac, listnode, next)
    {
        rc |= fn(p_db_ipsg_s_mac, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_ipsg_s_mac_master_t*
tbl_ipsg_s_mac_get_master()
{
    return _g_p_tbl_ipsg_s_mac_master;
}

tbl_ipsg_s_mac_master_t*
tbl_ipsg_s_mac_init_ipsg_s_mac()
{
    _g_p_tbl_ipsg_s_mac_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_ipsg_s_mac_master_t));
    _g_p_tbl_ipsg_s_mac_master->ipsg_s_mac_hash = ctclib_hash_create(_tbl_ipsg_s_mac_hash_make, _tbl_ipsg_s_mac_hash_cmp);
    _g_p_tbl_ipsg_s_mac_master->ipsg_s_mac_list = ctclib_slist_create(tbl_ipsg_s_mac_cmp, NULL);
    return _g_p_tbl_ipsg_s_mac_master;
}


tbl_ipsg_s_mac_t*
tbl_ipsg_s_mac_get_ipsg_s_mac_by_macaddr(mac_addr_t mac)
{
    tbl_ipsg_s_mac_key_t ipsg_s_mac_key;

    sal_memset(&ipsg_s_mac_key, 0, sizeof(ipsg_s_mac_key));
    sal_memcpy(ipsg_s_mac_key.mac, mac, sizeof(mac_addr_t));
    return tbl_ipsg_s_mac_get_ipsg_s_mac(&ipsg_s_mac_key);
}


