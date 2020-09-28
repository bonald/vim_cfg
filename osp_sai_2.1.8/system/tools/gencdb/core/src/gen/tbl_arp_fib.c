
#include "proto.h"
#include "gen/tbl_arp_fib_define.h"
#include "gen/tbl_arp_fib.h"
#include "cdb_data_cmp.h"

tbl_arp_fib_master_t *_g_p_tbl_arp_fib_master = NULL;

static uint32
_tbl_arp_fib_hash_make(tbl_arp_fib_t *p_arp_fib)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_arp_fib->key;
    for (index = 0; index < sizeof(p_arp_fib->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_arp_fib_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_arp_fib_t *p_arp_fib1 = (tbl_arp_fib_t*)p_arg1;
    tbl_arp_fib_t *p_arp_fib2 = (tbl_arp_fib_t*)p_arg2;

    if (0 == sal_memcmp(&p_arp_fib1->key, &p_arp_fib2->key, sizeof(tbl_arp_fib_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_arp_fib_add_arp_fib_sync(tbl_arp_fib_t *p_arp_fib, uint32 sync)
{
    tbl_arp_fib_master_t *p_master = _g_p_tbl_arp_fib_master;
    tbl_arp_fib_t *p_db_arp_fib = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_arp_fib_get_arp_fib(&p_arp_fib->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_arp_fib = XCALLOC(MEM_TBL_ARP_FIB, sizeof(tbl_arp_fib_t));
    if (NULL == p_db_arp_fib)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_arp_fib, p_arp_fib, sizeof(tbl_arp_fib_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->arp_fib_hash, (void*)p_db_arp_fib, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_ARP_FIB, p_db_arp_fib);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_ARP_FIB, p_db_arp_fib);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_arp_fib_del_arp_fib_sync(tbl_arp_fib_key_t *p_arp_fib_key, uint32 sync)
{
    tbl_arp_fib_master_t *p_master = _g_p_tbl_arp_fib_master;
    tbl_arp_fib_t *p_db_arp_fib = NULL;

    /* 1. lookup entry exist */
    p_db_arp_fib = tbl_arp_fib_get_arp_fib(p_arp_fib_key);
    if (NULL == p_db_arp_fib)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_ARP_FIB, p_db_arp_fib);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_ARP_FIB, p_db_arp_fib);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->arp_fib_hash, (void*)p_db_arp_fib);

    /* 4. free db entry */
    XFREE(MEM_TBL_ARP_FIB, p_db_arp_fib);

    return PM_E_NONE;
}

int32
tbl_arp_fib_set_arp_fib_field_sync(tbl_arp_fib_t *p_arp_fib, tbl_arp_fib_field_id_t field_id, uint32 sync)
{
    tbl_arp_fib_t *p_db_arp_fib = NULL;

    /* 1. lookup entry exist */
    p_db_arp_fib = tbl_arp_fib_get_arp_fib(&p_arp_fib->key);
    if (NULL == p_db_arp_fib)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_ARP_FIB_FLD_IFINDEX:
        p_db_arp_fib->ifindex = p_arp_fib->ifindex;
        break;

    case TBL_ARP_FIB_FLD_TYPE:
        p_db_arp_fib->type = p_arp_fib->type;
        break;

    case TBL_ARP_FIB_FLD_VRF_ID:
        p_db_arp_fib->vrf_id = p_arp_fib->vrf_id;
        break;

    case TBL_ARP_FIB_FLD_IFNAME:
        sal_memcpy(p_db_arp_fib->ifname, p_arp_fib->ifname, sizeof(p_arp_fib->ifname));
        break;

    case TBL_ARP_FIB_FLD_FLAGS:
        p_db_arp_fib->flags = p_arp_fib->flags;
        break;

    case TBL_ARP_FIB_FLD_MAC_ADDR:
        sal_memcpy(p_db_arp_fib->mac_addr, p_arp_fib->mac_addr, sizeof(p_arp_fib->mac_addr));
        break;

    case TBL_ARP_FIB_FLD_IS_ROUTER:
        p_db_arp_fib->is_router = p_arp_fib->is_router;
        break;

    case TBL_ARP_FIB_FLD_ROUTE_REF:
        p_db_arp_fib->route_ref = p_arp_fib->route_ref;
        break;

    case TBL_ARP_FIB_FLD_MAX:
        sal_memcpy(p_db_arp_fib, p_arp_fib, sizeof(tbl_arp_fib_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_ARP_FIB, field_id, p_db_arp_fib);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_ARP_FIB, field_id, p_db_arp_fib);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_arp_fib_add_arp_fib(tbl_arp_fib_t *p_arp_fib)
{
    return tbl_arp_fib_add_arp_fib_sync(p_arp_fib, TRUE);
}

int32
tbl_arp_fib_del_arp_fib(tbl_arp_fib_key_t *p_arp_fib_key)
{
    return tbl_arp_fib_del_arp_fib_sync(p_arp_fib_key, TRUE);
}

int32
tbl_arp_fib_set_arp_fib_field(tbl_arp_fib_t *p_arp_fib, tbl_arp_fib_field_id_t field_id)
{
    return tbl_arp_fib_set_arp_fib_field_sync(p_arp_fib, field_id, TRUE);
}

tbl_arp_fib_t*
tbl_arp_fib_get_arp_fib(tbl_arp_fib_key_t *p_arp_fib_key)
{
    tbl_arp_fib_master_t *p_master = _g_p_tbl_arp_fib_master;
    tbl_arp_fib_t lkp;

    sal_memcpy(&lkp.key, p_arp_fib_key, sizeof(tbl_arp_fib_key_t));
    return ctclib_hash_lookup(p_master->arp_fib_hash, &lkp);
}

char*
tbl_arp_fib_key_val2str(tbl_arp_fib_t *p_arp_fib, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_arp_fib->key.ip));
    return str;
}

int32
tbl_arp_fib_key_str2val(char *str, tbl_arp_fib_t *p_arp_fib)
{
    int32 ret = 0;

    ret = cdb_addr_ipv4_str2val(&p_arp_fib->key.ip, str, 0);
    return ret;
}

char*
tbl_arp_fib_key_name_dump(tbl_arp_fib_t *p_arp_fib, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ARP_FIB);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_arp_fib->key.ip));
    return str;
}

char*
tbl_arp_fib_key_value_dump(tbl_arp_fib_t *p_arp_fib, char *str)
{

    cdb_addr_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_arp_fib->key.ip);
    return str;
}

char*
tbl_arp_fib_field_name_dump(tbl_arp_fib_t *p_arp_fib, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_ARP_FIB);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_ARP_FIB_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_arp_fib_field_value_dump(tbl_arp_fib_t *p_arp_fib, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_ARP_FIB_FLD_KEY, field_id))
    {
        cdb_addr_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_arp_fib->key.ip);
    }
    if (FLD_MATCH(TBL_ARP_FIB_FLD_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_arp_fib->ifindex);
    }
    if (FLD_MATCH(TBL_ARP_FIB_FLD_TYPE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(arp_type_strs, ARP_TYPE_MAX, p_arp_fib->type));
    }
    if (FLD_MATCH(TBL_ARP_FIB_FLD_VRF_ID, field_id))
    {
        sal_sprintf(str, "%u", p_arp_fib->vrf_id);
    }
    if (FLD_MATCH(TBL_ARP_FIB_FLD_IFNAME, field_id))
    {
        sal_sprintf(str, "%s", p_arp_fib->ifname);
    }
    if (FLD_MATCH(TBL_ARP_FIB_FLD_FLAGS, field_id))
    {
        sal_sprintf(str, "%u", p_arp_fib->flags);
    }
    if (FLD_MATCH(TBL_ARP_FIB_FLD_MAC_ADDR, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_arp_fib->mac_addr);
    }
    if (FLD_MATCH(TBL_ARP_FIB_FLD_IS_ROUTER, field_id))
    {
        sal_sprintf(str, "%u", p_arp_fib->is_router);
    }
    if (FLD_MATCH(TBL_ARP_FIB_FLD_ROUTE_REF, field_id))
    {
        sal_sprintf(str, "%u", p_arp_fib->route_ref);
    }
    return str;
}

char**
tbl_arp_fib_table_dump(tbl_arp_fib_t *p_arp_fib, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_arp_fib_key_name_dump(p_arp_fib, buf));
    for(i=1; i<TBL_ARP_FIB_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_arp_fib_field_name_dump(p_arp_fib, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_arp_fib_field_value_dump(p_arp_fib, i, buf));
    }
    return str;
}

int32
tbl_arp_fib_field_value_parser(char *str, int32 field_id, tbl_arp_fib_t *p_arp_fib)
{
    if (FLD_MATCH(TBL_ARP_FIB_FLD_KEY, field_id))
    {
        cdb_addr_ipv4_str2val(&p_arp_fib->key.ip, str, 0);
    }
    if (FLD_MATCH(TBL_ARP_FIB_FLD_IFINDEX, field_id))
    {
        int32 ret;
        p_arp_fib->ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ARP_FIB_FLD_TYPE, field_id))
    {
        cdb_enum_str2val(arp_type_strs, ARP_TYPE_MAX, str);
    }
    if (FLD_MATCH(TBL_ARP_FIB_FLD_VRF_ID, field_id))
    {
        int32 ret;
        p_arp_fib->vrf_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ARP_FIB_FLD_IFNAME, field_id))
    {
        sal_strcpy(p_arp_fib->ifname, str);
    }
    if (FLD_MATCH(TBL_ARP_FIB_FLD_FLAGS, field_id))
    {
        int32 ret;
        p_arp_fib->flags = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ARP_FIB_FLD_MAC_ADDR, field_id))
    {
        cdb_mac_addr_str2val(p_arp_fib->mac_addr, str);
    }
    if (FLD_MATCH(TBL_ARP_FIB_FLD_IS_ROUTER, field_id))
    {
        int32 ret;
        p_arp_fib->is_router = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ARP_FIB_FLD_ROUTE_REF, field_id))
    {
        int32 ret;
        p_arp_fib->route_ref = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_arp_fib_table_parser(char** array, char* key_value,tbl_arp_fib_t *p_arp_fib)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_ARP_FIB);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_arp_fib_key_str2val(key_value, p_arp_fib));

    for(i=1; i<TBL_ARP_FIB_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_arp_fib_field_value_parser( array[j++], i, p_arp_fib));
    }

    return PM_E_NONE;
}

int32
tbl_arp_fib_dump_one(tbl_arp_fib_t *p_arp_fib, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ARP_FIB);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_ARP_FIB_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ARP_FIB_FLD_KEY].name, 
            cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_arp_fib->key.ip));
    }
    if (FLD_MATCH(TBL_ARP_FIB_FLD_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ARP_FIB_FLD_IFINDEX].name,
            p_arp_fib->ifindex);
    }
    if (FLD_MATCH(TBL_ARP_FIB_FLD_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ARP_FIB_FLD_TYPE].name, 
            cdb_enum_val2str(arp_type_strs, ARP_TYPE_MAX, p_arp_fib->type));
    }
    if (FLD_MATCH(TBL_ARP_FIB_FLD_VRF_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ARP_FIB_FLD_VRF_ID].name,
            p_arp_fib->vrf_id);
    }
    if (FLD_MATCH(TBL_ARP_FIB_FLD_IFNAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ARP_FIB_FLD_IFNAME].name,
            p_arp_fib->ifname);
    }
    if (FLD_MATCH(TBL_ARP_FIB_FLD_FLAGS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ARP_FIB_FLD_FLAGS].name,
            p_arp_fib->flags);
    }
    if (FLD_MATCH(TBL_ARP_FIB_FLD_MAC_ADDR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ARP_FIB_FLD_MAC_ADDR].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_arp_fib->mac_addr));
    }
    if (FLD_MATCH(TBL_ARP_FIB_FLD_IS_ROUTER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ARP_FIB_FLD_IS_ROUTER].name,
            p_arp_fib->is_router);
    }
    if (FLD_MATCH(TBL_ARP_FIB_FLD_ROUTE_REF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ARP_FIB_FLD_ROUTE_REF].name,
            p_arp_fib->route_ref);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_arp_fib_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_arp_fib_master_t *p_master = _g_p_tbl_arp_fib_master;
    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    ctclib_hash_iterate2(p_master->arp_fib_hash, tbl_hash_iter_adpt_fn, fn, pargs);
    return PM_E_NONE;
}

tbl_arp_fib_master_t*
tbl_arp_fib_get_master()
{
    return _g_p_tbl_arp_fib_master;
}

tbl_arp_fib_master_t*
tbl_arp_fib_init_arp_fib()
{
    _g_p_tbl_arp_fib_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_arp_fib_master_t));
    _g_p_tbl_arp_fib_master->arp_fib_hash = ctclib_hash_create(_tbl_arp_fib_hash_make, _tbl_arp_fib_hash_cmp);
    return _g_p_tbl_arp_fib_master;
}

