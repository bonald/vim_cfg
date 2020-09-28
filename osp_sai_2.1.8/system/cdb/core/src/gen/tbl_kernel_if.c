
#include "proto.h"
#include "gen/tbl_kernel_if_define.h"
#include "gen/tbl_kernel_if.h"
#include "cdb_data_cmp.h"

tbl_kernel_if_master_t *_g_p_tbl_kernel_if_master = NULL;

static uint32
_tbl_kernel_if_hash_make(tbl_kernel_if_t *p_kernel_if)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_kernel_if->key;
    for (index = 0; index < sizeof(p_kernel_if->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_kernel_if_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_kernel_if_t *p_kernel_if1 = (tbl_kernel_if_t*)p_arg1;
    tbl_kernel_if_t *p_kernel_if2 = (tbl_kernel_if_t*)p_arg2;

    if (0 == sal_memcmp(&p_kernel_if1->key, &p_kernel_if2->key, sizeof(tbl_kernel_if_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_kernel_if_add_kernel_if_sync(tbl_kernel_if_t *p_kernel_if, uint32 sync)
{
    tbl_kernel_if_master_t *p_master = _g_p_tbl_kernel_if_master;
    tbl_kernel_if_t *p_db_kernel_if = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_kernel_if_get_kernel_if(&p_kernel_if->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_kernel_if = XCALLOC(MEM_TBL_KERNEL_IF, sizeof(tbl_kernel_if_t));
    if (NULL == p_db_kernel_if)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_kernel_if, p_kernel_if, sizeof(tbl_kernel_if_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->kernel_if_hash, (void*)p_db_kernel_if, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->kernel_if_list, p_db_kernel_if);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_KERNEL_IF, p_db_kernel_if);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_KERNEL_IF, p_db_kernel_if);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_kernel_if_del_kernel_if_sync(tbl_kernel_if_key_t *p_kernel_if_key, uint32 sync)
{
    tbl_kernel_if_master_t *p_master = _g_p_tbl_kernel_if_master;
    tbl_kernel_if_t *p_db_kernel_if = NULL;

    /* 1. lookup entry exist */
    p_db_kernel_if = tbl_kernel_if_get_kernel_if(p_kernel_if_key);
    if (NULL == p_db_kernel_if)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_KERNEL_IF, p_db_kernel_if);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_KERNEL_IF, p_db_kernel_if);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->kernel_if_hash, (void*)p_db_kernel_if);
    ctclib_slistnode_delete(p_master->kernel_if_list, p_db_kernel_if);

    /* 4. free db entry */
    XFREE(MEM_TBL_KERNEL_IF, p_db_kernel_if);

    return PM_E_NONE;
}

int32
tbl_kernel_if_set_kernel_if_field_sync(tbl_kernel_if_t *p_kernel_if, tbl_kernel_if_field_id_t field_id, uint32 sync)
{
    tbl_kernel_if_t *p_db_kernel_if = NULL;

    /* 1. lookup entry exist */
    p_db_kernel_if = tbl_kernel_if_get_kernel_if(&p_kernel_if->key);
    if (NULL == p_db_kernel_if)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_KERNEL_IF_FLD_NAME:
        sal_memcpy(p_db_kernel_if->name, p_kernel_if->name, sizeof(p_kernel_if->name));
        break;

    case TBL_KERNEL_IF_FLD_IP_ADDR:
        sal_memcpy(&p_db_kernel_if->ip_addr, &p_kernel_if->ip_addr, sizeof(p_kernel_if->ip_addr));
        break;

    case TBL_KERNEL_IF_FLD_MASKLEN:
        p_db_kernel_if->masklen = p_kernel_if->masklen;
        break;

    case TBL_KERNEL_IF_FLD_IP_ADDR_PREFIX_LENGTH_V6:
        sal_memcpy(&p_db_kernel_if->ip_addr_prefix_length_v6, &p_kernel_if->ip_addr_prefix_length_v6, sizeof(p_kernel_if->ip_addr_prefix_length_v6));
        break;

    case TBL_KERNEL_IF_FLD_VRF_ID:
        p_db_kernel_if->vrf_id = p_kernel_if->vrf_id;
        break;

    case TBL_KERNEL_IF_FLD_MAX:
        sal_memcpy(p_db_kernel_if, p_kernel_if, sizeof(tbl_kernel_if_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_KERNEL_IF, field_id, p_db_kernel_if);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_KERNEL_IF, field_id, p_db_kernel_if);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_kernel_if_add_kernel_if(tbl_kernel_if_t *p_kernel_if)
{
    return tbl_kernel_if_add_kernel_if_sync(p_kernel_if, TRUE);
}

int32
tbl_kernel_if_del_kernel_if(tbl_kernel_if_key_t *p_kernel_if_key)
{
    return tbl_kernel_if_del_kernel_if_sync(p_kernel_if_key, TRUE);
}

int32
tbl_kernel_if_set_kernel_if_field(tbl_kernel_if_t *p_kernel_if, tbl_kernel_if_field_id_t field_id)
{
    return tbl_kernel_if_set_kernel_if_field_sync(p_kernel_if, field_id, TRUE);
}

tbl_kernel_if_t*
tbl_kernel_if_get_kernel_if(tbl_kernel_if_key_t *p_kernel_if_key)
{
    tbl_kernel_if_master_t *p_master = _g_p_tbl_kernel_if_master;
    tbl_kernel_if_t lkp;

    sal_memcpy(&lkp.key, p_kernel_if_key, sizeof(tbl_kernel_if_key_t));
    return ctclib_hash_lookup(p_master->kernel_if_hash, &lkp);
}

char*
tbl_kernel_if_key_val2str(tbl_kernel_if_t *p_kernel_if, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_kernel_if->key.ifindex);
    return str;
}

int32
tbl_kernel_if_key_str2val(char *str, tbl_kernel_if_t *p_kernel_if)
{
    int32 ret = 0;

    p_kernel_if->key.ifindex = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_kernel_if_key_name_dump(tbl_kernel_if_t *p_kernel_if, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_KERNEL_IF);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_kernel_if->key.ifindex);
    return str;
}

char*
tbl_kernel_if_key_value_dump(tbl_kernel_if_t *p_kernel_if, char *str)
{
    sal_sprintf(str, "%u", p_kernel_if->key.ifindex);
    return str;
}

char*
tbl_kernel_if_field_name_dump(tbl_kernel_if_t *p_kernel_if, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_KERNEL_IF);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_KERNEL_IF_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_kernel_if_field_value_dump(tbl_kernel_if_t *p_kernel_if, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_KERNEL_IF_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_kernel_if->key.ifindex);
    }
    if (FLD_MATCH(TBL_KERNEL_IF_FLD_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_kernel_if->name);
    }
    if (FLD_MATCH(TBL_KERNEL_IF_FLD_IP_ADDR, field_id))
    {
        cdb_addr_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_kernel_if->ip_addr);
    }
    if (FLD_MATCH(TBL_KERNEL_IF_FLD_MASKLEN, field_id))
    {
        sal_sprintf(str, "%u", p_kernel_if->masklen);
    }
    if (FLD_MATCH(TBL_KERNEL_IF_FLD_IP_ADDR_PREFIX_LENGTH_V6, field_id))
    {
        cdb_kernel_if_ipv6_addr_val2str(str, MAX_CMD_STR_LEN, &p_kernel_if->ip_addr_prefix_length_v6);
    }
    if (FLD_MATCH(TBL_KERNEL_IF_FLD_VRF_ID, field_id))
    {
        sal_sprintf(str, "%u", p_kernel_if->vrf_id);
    }
    return str;
}

char**
tbl_kernel_if_table_dump(tbl_kernel_if_t *p_kernel_if, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_kernel_if_key_name_dump(p_kernel_if, buf));
    for(i=1; i<TBL_KERNEL_IF_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_kernel_if_field_name_dump(p_kernel_if, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_kernel_if_field_value_dump(p_kernel_if, i, buf));
    }
    return str;
}

int32
tbl_kernel_if_field_value_parser(char *str, int32 field_id, tbl_kernel_if_t *p_kernel_if)
{
    if (FLD_MATCH(TBL_KERNEL_IF_FLD_KEY, field_id))
    {
        int32 ret;
        p_kernel_if->key.ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_KERNEL_IF_FLD_NAME, field_id))
    {
        sal_strcpy(p_kernel_if->name, str);
    }
    if (FLD_MATCH(TBL_KERNEL_IF_FLD_IP_ADDR, field_id))
    {
        cdb_addr_ipv4_str2val(&p_kernel_if->ip_addr, str, 0);
    }
    if (FLD_MATCH(TBL_KERNEL_IF_FLD_MASKLEN, field_id))
    {
        int32 ret;
        p_kernel_if->masklen = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_KERNEL_IF_FLD_IP_ADDR_PREFIX_LENGTH_V6, field_id))
    {
        cdb_kernel_if_ipv6_addr_str2val(str, &p_kernel_if->ip_addr_prefix_length_v6);
    }
    if (FLD_MATCH(TBL_KERNEL_IF_FLD_VRF_ID, field_id))
    {
        int32 ret;
        p_kernel_if->vrf_id = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_kernel_if_table_parser(char** array, char* key_value,tbl_kernel_if_t *p_kernel_if)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_KERNEL_IF);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_kernel_if_key_str2val(key_value, p_kernel_if));

    for(i=1; i<TBL_KERNEL_IF_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_kernel_if_field_value_parser( array[j++], i, p_kernel_if));
    }

    return PM_E_NONE;
}

int32
tbl_kernel_if_dump_one(tbl_kernel_if_t *p_kernel_if, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_KERNEL_IF);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_KERNEL_IF_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_KERNEL_IF_FLD_KEY].name,
            p_kernel_if->key.ifindex);
    }
    if (FLD_MATCH(TBL_KERNEL_IF_FLD_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_KERNEL_IF_FLD_NAME].name,
            p_kernel_if->name);
    }
    if (FLD_MATCH(TBL_KERNEL_IF_FLD_IP_ADDR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_KERNEL_IF_FLD_IP_ADDR].name, 
            cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_kernel_if->ip_addr));
    }
    if (FLD_MATCH(TBL_KERNEL_IF_FLD_MASKLEN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_KERNEL_IF_FLD_MASKLEN].name,
            p_kernel_if->masklen);
    }
    if (FLD_MATCH(TBL_KERNEL_IF_FLD_IP_ADDR_PREFIX_LENGTH_V6, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_KERNEL_IF_FLD_IP_ADDR_PREFIX_LENGTH_V6].name, 
            cdb_kernel_if_ipv6_addr_val2str(buf, MAX_CMD_STR_LEN, &p_kernel_if->ip_addr_prefix_length_v6));
    }
    if (FLD_MATCH(TBL_KERNEL_IF_FLD_VRF_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_KERNEL_IF_FLD_VRF_ID].name,
            p_kernel_if->vrf_id);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_kernel_if_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_kernel_if_master_t *p_master = _g_p_tbl_kernel_if_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_kernel_if_t *p_db_kernel_if = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->kernel_if_list, p_db_kernel_if, listnode, next)
    {
        rc |= fn(p_db_kernel_if, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_kernel_if_master_t*
tbl_kernel_if_get_master()
{
    return _g_p_tbl_kernel_if_master;
}

tbl_kernel_if_master_t*
tbl_kernel_if_init_kernel_if()
{
    _g_p_tbl_kernel_if_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_kernel_if_master_t));
    _g_p_tbl_kernel_if_master->kernel_if_hash = ctclib_hash_create(_tbl_kernel_if_hash_make, _tbl_kernel_if_hash_cmp);
    _g_p_tbl_kernel_if_master->kernel_if_list = ctclib_slist_create(tbl_kernel_if_cmp, NULL);
    return _g_p_tbl_kernel_if_master;
}


tbl_kernel_if_t *
tbl_kernel_if_get_by_name(char *name)
{
    ctclib_slistnode_t *listnode    = NULL;    
    tbl_kernel_if_t *p_db_kernel_if = NULL;
    tbl_kernel_if_master_t *p_master = _g_p_tbl_kernel_if_master;

    CTCLIB_SLIST_LOOP(p_master->kernel_if_list, p_db_kernel_if, listnode)
    {
        if(0 == sal_strcmp(p_db_kernel_if->name, name))
        {
             return p_db_kernel_if;
        }
    }
    
    return NULL;
}

tbl_kernel_if_t *
tbl_kernel_if_check_by_list_loop(addr_ipv4_t *prefix)
{
    ctclib_slistnode_t *listnode    = NULL;    
    tbl_kernel_if_t *p_db_kernel_if = NULL;
    tbl_kernel_if_master_t *p_master = _g_p_tbl_kernel_if_master;
    addr_ipv4_t ip_addr_check;    

    sal_memset(&ip_addr_check, 0, sizeof(ip_addr_check));
    CTCLIB_SLIST_LOOP(p_master->kernel_if_list, p_db_kernel_if, listnode)
    {
        if (NULL != p_db_kernel_if)
        {
            ip_addr_check.s_addr = sal_htonl(p_db_kernel_if->ip_addr.s_addr);
            if (ip_addr_check.s_addr && (0 == sal_memcmp(&ip_addr_check, prefix, sizeof(addr_ipv4_t))))
            {
                return p_db_kernel_if;
            }                
        }
    }
    
    return NULL;
}

tbl_kernel_if_t *
tbl_kernel_if_check_by_ipmask_list_loop(addr_ipv4_t *ip_mask)
{
    ctclib_slistnode_t *listnode    = NULL;    
    tbl_kernel_if_t *p_db_kernel_if = NULL;
    tbl_kernel_if_master_t *p_master = _g_p_tbl_kernel_if_master;
    addr_ipv4_t ip_addr_check;
    uint32      mask = 0;
    
    sal_memset(&ip_addr_check, 0, sizeof(ip_addr_check));
    CTCLIB_SLIST_LOOP(p_master->kernel_if_list, p_db_kernel_if, listnode)
    {
        if (NULL != p_db_kernel_if)
        {
            ip_addr_check.s_addr = sal_htonl(p_db_kernel_if->ip_addr.s_addr);
            IPV4_LEN_TO_MASK(mask, p_db_kernel_if->masklen);
            ip_addr_check.s_addr = ip_addr_check.s_addr & mask;
            if (ip_addr_check.s_addr && (0 == sal_memcmp(&ip_addr_check, ip_mask, sizeof(addr_ipv4_t))))
            {
                return p_db_kernel_if;
            }                
        }
    }
    
    return NULL;
}


