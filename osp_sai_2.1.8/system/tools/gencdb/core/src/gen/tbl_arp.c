
#include "proto.h"
#include "gen/tbl_arp_define.h"
#include "gen/tbl_arp.h"
#include "gen/ds_static_route_dup_ipv4_nh_define.h"
#include "gen/ds_static_route_dup_ipv4_nh.h"
#include "cdb_data_cmp.h"

tbl_arp_master_t *_g_p_tbl_arp_master = NULL;

static uint32
_tbl_arp_hash_make(tbl_arp_t *p_arp)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_arp->key;
    for (index = 0; index < sizeof(p_arp->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_arp_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_arp_t *p_arp1 = (tbl_arp_t*)p_arg1;
    tbl_arp_t *p_arp2 = (tbl_arp_t*)p_arg2;

    if (0 == sal_memcmp(&p_arp1->key, &p_arp2->key, sizeof(tbl_arp_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_arp_add_arp_sync(tbl_arp_t *p_arp, uint32 sync)
{
    tbl_arp_master_t *p_master = _g_p_tbl_arp_master;
    tbl_arp_t *p_db_arp = NULL;
    void *p_ret = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    if (tbl_arp_get_arp(&p_arp->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_arp = XCALLOC(MEM_TBL_ARP, sizeof(tbl_arp_t));
    if (NULL == p_db_arp)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_arp, p_arp, sizeof(tbl_arp_t));
    PM_E_RETURN(cdb_reference_list_init(&p_db_arp->static_route_dup_ipv4_nh, CDB_NODE_DS, DS_STATIC_ROUTE_DUP_IPV4_NH, ds_static_route_dup_ipv4_nh_cmp));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->arp_hash, (void*)p_db_arp, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->arp_list, p_db_arp);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_ARP, p_db_arp);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_ARP, p_db_arp);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_arp_del_arp_sync(tbl_arp_key_t *p_arp_key, uint32 sync)
{
    tbl_arp_master_t *p_master = _g_p_tbl_arp_master;
    tbl_arp_t *p_db_arp = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_arp = tbl_arp_get_arp(p_arp_key);
    if (NULL == p_db_arp)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_ARP, p_db_arp);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_ARP, p_db_arp);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->arp_hash, (void*)p_db_arp);
    ctclib_slistnode_delete(p_master->arp_list, p_db_arp);
    PM_E_RETURN(cdb_reference_list_deinit(&p_db_arp->static_route_dup_ipv4_nh));

    /* 4. free db entry */
    XFREE(MEM_TBL_ARP, p_db_arp);

    return PM_E_NONE;
}

int32
tbl_arp_set_arp_field_sync(tbl_arp_t *p_arp, tbl_arp_field_id_t field_id, uint32 sync)
{
    tbl_arp_t *p_db_arp = NULL;

    /* 1. lookup entry exist */
    p_db_arp = tbl_arp_get_arp(&p_arp->key);
    if (NULL == p_db_arp)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_ARP_FLD_TYPE:
        p_db_arp->type = p_arp->type;
        break;

    case TBL_ARP_FLD_IFNAME:
        sal_memcpy(p_db_arp->ifname, p_arp->ifname, sizeof(p_arp->ifname));
        break;

    case TBL_ARP_FLD_VRF_ID:
        p_db_arp->vrf_id = p_arp->vrf_id;
        break;

    case TBL_ARP_FLD_MAC_ADDR:
        sal_memcpy(p_db_arp->mac_addr, p_arp->mac_addr, sizeof(p_arp->mac_addr));
        break;

    case TBL_ARP_FLD_RIF_ID:
        p_db_arp->rif_id = p_arp->rif_id;
        break;

    case TBL_ARP_FLD_IFINDEX:
        p_db_arp->ifindex = p_arp->ifindex;
        break;

    case TBL_ARP_FLD_FLAGS:
        p_db_arp->flags = p_arp->flags;
        break;

    case TBL_ARP_FLD_STATUS:
        p_db_arp->status = p_arp->status;
        break;

    case TBL_ARP_FLD_ROUTE_REF:
        p_db_arp->route_ref = p_arp->route_ref;
        break;

    case TBL_ARP_FLD_UPTIME:
        sal_memcpy(&p_db_arp->uptime, &p_arp->uptime, sizeof(p_arp->uptime));
        break;

    case TBL_ARP_FLD_AGING_DELAY:
        p_db_arp->aging_delay = p_arp->aging_delay;
        break;

    case TBL_ARP_FLD_IS_GRATUITOUS:
        p_db_arp->is_gratuitous = p_arp->is_gratuitous;
        break;

    case TBL_ARP_FLD_IS_PROXY:
        p_db_arp->is_proxy = p_arp->is_proxy;
        break;

    case TBL_ARP_FLD_IS_ROUTER:
        p_db_arp->is_router = p_arp->is_router;
        break;

    case TBL_ARP_FLD_RETRY_COUNT:
        p_db_arp->retry_count = p_arp->retry_count;
        break;

    case TBL_ARP_FLD_IS_BFD_REF:
        p_db_arp->is_bfd_ref = p_arp->is_bfd_ref;
        break;

    case TBL_ARP_FLD_IS_STATIC_ROUTE_DUP:
        p_db_arp->is_static_route_dup = p_arp->is_static_route_dup;
        break;

    case TBL_ARP_FLD_RETRY_TIMER:
        p_db_arp->retry_timer = p_arp->retry_timer;
        break;

    case TBL_ARP_FLD_REFRESH_TIMER:
        p_db_arp->refresh_timer = p_arp->refresh_timer;
        break;

    case TBL_ARP_FLD_GRA_RSP_TIMER:
        p_db_arp->gra_rsp_timer = p_arp->gra_rsp_timer;
        break;

    case TBL_ARP_FLD_MAX:
        sal_memcpy(p_db_arp, p_arp, sizeof(tbl_arp_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_ARP, field_id, p_db_arp);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_ARP, field_id, p_db_arp);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_arp_add_arp(tbl_arp_t *p_arp)
{
    return tbl_arp_add_arp_sync(p_arp, TRUE);
}

int32
tbl_arp_del_arp(tbl_arp_key_t *p_arp_key)
{
    return tbl_arp_del_arp_sync(p_arp_key, TRUE);
}

int32
tbl_arp_set_arp_field(tbl_arp_t *p_arp, tbl_arp_field_id_t field_id)
{
    return tbl_arp_set_arp_field_sync(p_arp, field_id, TRUE);
}

tbl_arp_t*
tbl_arp_get_arp(tbl_arp_key_t *p_arp_key)
{
    tbl_arp_master_t *p_master = _g_p_tbl_arp_master;
    tbl_arp_t lkp;

    sal_memcpy(&lkp.key, p_arp_key, sizeof(tbl_arp_key_t));
    return ctclib_hash_lookup(p_master->arp_hash, &lkp);
}

char*
tbl_arp_key_val2str(tbl_arp_t *p_arp, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_arp->key.ip));
    return str;
}

int32
tbl_arp_key_str2val(char *str, tbl_arp_t *p_arp)
{
    int32 ret = 0;

    ret = cdb_addr_ipv4_str2val(&p_arp->key.ip, str, 0);
    return ret;
}

char*
tbl_arp_key_name_dump(tbl_arp_t *p_arp, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ARP);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_arp->key.ip));
    return str;
}

char*
tbl_arp_key_value_dump(tbl_arp_t *p_arp, char *str)
{

    cdb_addr_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_arp->key.ip);
    return str;
}

char*
tbl_arp_field_name_dump(tbl_arp_t *p_arp, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_ARP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_ARP_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_arp_field_value_dump(tbl_arp_t *p_arp, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_ARP_FLD_KEY, field_id))
    {
        cdb_addr_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_arp->key.ip);
    }
    if (FLD_MATCH(TBL_ARP_FLD_TYPE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(arp_type_strs, ARP_TYPE_MAX, p_arp->type));
    }
    if (FLD_MATCH(TBL_ARP_FLD_IFNAME, field_id))
    {
        sal_sprintf(str, "%s", p_arp->ifname);
    }
    if (FLD_MATCH(TBL_ARP_FLD_VRF_ID, field_id))
    {
        sal_sprintf(str, "%u", p_arp->vrf_id);
    }
    if (FLD_MATCH(TBL_ARP_FLD_MAC_ADDR, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_arp->mac_addr);
    }
    if (FLD_MATCH(TBL_ARP_FLD_RIF_ID, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_arp->rif_id);
    }
    if (FLD_MATCH(TBL_ARP_FLD_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_arp->ifindex);
    }
    if (FLD_MATCH(TBL_ARP_FLD_FLAGS, field_id))
    {
        sal_sprintf(str, "%u", p_arp->flags);
    }
    if (FLD_MATCH(TBL_ARP_FLD_STATUS, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(as_strs, AS_MAX, p_arp->status));
    }
    if (FLD_MATCH(TBL_ARP_FLD_ROUTE_REF, field_id))
    {
        sal_sprintf(str, "%u", p_arp->route_ref);
    }
    if (FLD_MATCH(TBL_ARP_FLD_UPTIME, field_id))
    {
        cdb_sal_time_val2str(str, MAX_CMD_STR_LEN, &p_arp->uptime);
    }
    if (FLD_MATCH(TBL_ARP_FLD_AGING_DELAY, field_id))
    {
        sal_sprintf(str, "%u", p_arp->aging_delay);
    }
    if (FLD_MATCH(TBL_ARP_FLD_IS_GRATUITOUS, field_id))
    {
        sal_sprintf(str, "%u", p_arp->is_gratuitous);
    }
    if (FLD_MATCH(TBL_ARP_FLD_IS_PROXY, field_id))
    {
        sal_sprintf(str, "%u", p_arp->is_proxy);
    }
    if (FLD_MATCH(TBL_ARP_FLD_IS_ROUTER, field_id))
    {
        sal_sprintf(str, "%u", p_arp->is_router);
    }
    if (FLD_MATCH(TBL_ARP_FLD_RETRY_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_arp->retry_count);
    }
    if (FLD_MATCH(TBL_ARP_FLD_IS_BFD_REF, field_id))
    {
        sal_sprintf(str, "%u", p_arp->is_bfd_ref);
    }
    if (FLD_MATCH(TBL_ARP_FLD_IS_STATIC_ROUTE_DUP, field_id))
    {
        sal_sprintf(str, "%u", p_arp->is_static_route_dup);
    }
    if (FLD_MATCH(TBL_ARP_FLD_STATIC_ROUTE_DUP_IPV4_NH, field_id))
    {
    }
    if (FLD_MATCH(TBL_ARP_FLD_RETRY_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_arp->retry_timer);
    }
    if (FLD_MATCH(TBL_ARP_FLD_REFRESH_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_arp->refresh_timer);
    }
    if (FLD_MATCH(TBL_ARP_FLD_GRA_RSP_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_arp->gra_rsp_timer);
    }
    return str;
}

char**
tbl_arp_table_dump(tbl_arp_t *p_arp, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_arp_key_name_dump(p_arp, buf));
    for(i=1; i<TBL_ARP_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_arp_field_name_dump(p_arp, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_arp_field_value_dump(p_arp, i, buf));
    }
    return str;
}

int32
tbl_arp_field_value_parser(char *str, int32 field_id, tbl_arp_t *p_arp)
{
    if (FLD_MATCH(TBL_ARP_FLD_KEY, field_id))
    {
        cdb_addr_ipv4_str2val(&p_arp->key.ip, str, 0);
    }
    if (FLD_MATCH(TBL_ARP_FLD_TYPE, field_id))
    {
        cdb_enum_str2val(arp_type_strs, ARP_TYPE_MAX, str);
    }
    if (FLD_MATCH(TBL_ARP_FLD_IFNAME, field_id))
    {
        sal_strcpy(p_arp->ifname, str);
    }
    if (FLD_MATCH(TBL_ARP_FLD_VRF_ID, field_id))
    {
        int32 ret;
        p_arp->vrf_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ARP_FLD_MAC_ADDR, field_id))
    {
        cdb_mac_addr_str2val(p_arp->mac_addr, str);
    }
    if (FLD_MATCH(TBL_ARP_FLD_RIF_ID, field_id))
    {
        int32 ret;
        p_arp->rif_id = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ARP_FLD_IFINDEX, field_id))
    {
        int32 ret;
        p_arp->ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ARP_FLD_FLAGS, field_id))
    {
        int32 ret;
        p_arp->flags = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ARP_FLD_STATUS, field_id))
    {
        cdb_enum_str2val(as_strs, AS_MAX, str);
    }
    if (FLD_MATCH(TBL_ARP_FLD_ROUTE_REF, field_id))
    {
        int32 ret;
        p_arp->route_ref = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ARP_FLD_UPTIME, field_id))
    {
        cdb_sal_time_str2val(str, &p_arp->uptime);
    }
    if (FLD_MATCH(TBL_ARP_FLD_AGING_DELAY, field_id))
    {
        int32 ret;
        p_arp->aging_delay = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ARP_FLD_IS_GRATUITOUS, field_id))
    {
        int32 ret;
        p_arp->is_gratuitous = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ARP_FLD_IS_PROXY, field_id))
    {
        int32 ret;
        p_arp->is_proxy = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ARP_FLD_IS_ROUTER, field_id))
    {
        int32 ret;
        p_arp->is_router = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ARP_FLD_RETRY_COUNT, field_id))
    {
        int32 ret;
        p_arp->retry_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ARP_FLD_IS_BFD_REF, field_id))
    {
        int32 ret;
        p_arp->is_bfd_ref = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ARP_FLD_IS_STATIC_ROUTE_DUP, field_id))
    {
        int32 ret;
        p_arp->is_static_route_dup = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ARP_FLD_STATIC_ROUTE_DUP_IPV4_NH, field_id))
    {
    }
    if (FLD_MATCH(TBL_ARP_FLD_RETRY_TIMER, field_id))
    {
        ctc_task_str2val(str, p_arp->retry_timer);
    }
    if (FLD_MATCH(TBL_ARP_FLD_REFRESH_TIMER, field_id))
    {
        ctc_task_str2val(str, p_arp->refresh_timer);
    }
    if (FLD_MATCH(TBL_ARP_FLD_GRA_RSP_TIMER, field_id))
    {
        ctc_task_str2val(str, p_arp->gra_rsp_timer);
    }
    return PM_E_NONE;
}

int32
tbl_arp_table_parser(char** array, char* key_value,tbl_arp_t *p_arp)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_ARP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_arp_key_str2val(key_value, p_arp));

    for(i=1; i<TBL_ARP_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_arp_field_value_parser( array[j++], i, p_arp));
    }

    return PM_E_NONE;
}

int32
tbl_arp_dump_one(tbl_arp_t *p_arp, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ARP);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    ds_static_route_dup_ipv4_nh_t *p_db_static_route_dup_ipv4_nh = NULL;
    ctclib_slistnode_t *listnode = NULL;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_ARP_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ARP_FLD_KEY].name, 
            cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_arp->key.ip));
    }
    if (FLD_MATCH(TBL_ARP_FLD_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ARP_FLD_TYPE].name, 
            cdb_enum_val2str(arp_type_strs, ARP_TYPE_MAX, p_arp->type));
    }
    if (FLD_MATCH(TBL_ARP_FLD_IFNAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ARP_FLD_IFNAME].name,
            p_arp->ifname);
    }
    if (FLD_MATCH(TBL_ARP_FLD_VRF_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ARP_FLD_VRF_ID].name,
            p_arp->vrf_id);
    }
    if (FLD_MATCH(TBL_ARP_FLD_MAC_ADDR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ARP_FLD_MAC_ADDR].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_arp->mac_addr));
    }
    if (FLD_MATCH(TBL_ARP_FLD_RIF_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ARP_FLD_RIF_ID].name,
            p_arp->rif_id);
    }
    if (FLD_MATCH(TBL_ARP_FLD_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ARP_FLD_IFINDEX].name,
            p_arp->ifindex);
    }
    if (FLD_MATCH(TBL_ARP_FLD_FLAGS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ARP_FLD_FLAGS].name,
            p_arp->flags);
    }
    if (FLD_MATCH(TBL_ARP_FLD_STATUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ARP_FLD_STATUS].name, 
            cdb_enum_val2str(as_strs, AS_MAX, p_arp->status));
    }
    if (FLD_MATCH(TBL_ARP_FLD_ROUTE_REF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ARP_FLD_ROUTE_REF].name,
            p_arp->route_ref);
    }
    if (FLD_MATCH(TBL_ARP_FLD_UPTIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ARP_FLD_UPTIME].name, 
            cdb_sal_time_val2str(buf, MAX_CMD_STR_LEN, &p_arp->uptime));
    }
    if (FLD_MATCH(TBL_ARP_FLD_AGING_DELAY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ARP_FLD_AGING_DELAY].name,
            p_arp->aging_delay);
    }
    if (FLD_MATCH(TBL_ARP_FLD_IS_GRATUITOUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ARP_FLD_IS_GRATUITOUS].name,
            p_arp->is_gratuitous);
    }
    if (FLD_MATCH(TBL_ARP_FLD_IS_PROXY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ARP_FLD_IS_PROXY].name,
            p_arp->is_proxy);
    }
    if (FLD_MATCH(TBL_ARP_FLD_IS_ROUTER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ARP_FLD_IS_ROUTER].name,
            p_arp->is_router);
    }
    if (FLD_MATCH(TBL_ARP_FLD_RETRY_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ARP_FLD_RETRY_COUNT].name,
            p_arp->retry_count);
    }
    if (FLD_MATCH(TBL_ARP_FLD_IS_BFD_REF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ARP_FLD_IS_BFD_REF].name,
            p_arp->is_bfd_ref);
    }
    if (FLD_MATCH(TBL_ARP_FLD_IS_STATIC_ROUTE_DUP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ARP_FLD_IS_STATIC_ROUTE_DUP].name,
            p_arp->is_static_route_dup);
    }
    if (FLD_MATCH(TBL_ARP_FLD_STATIC_ROUTE_DUP_IPV4_NH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"{", p_tbl_info->field[TBL_ARP_FLD_STATIC_ROUTE_DUP_IPV4_NH].name);
        if (p_arp->static_route_dup_ipv4_nh.obj_list)
        {
            p_field->field_id[1] = FLD_ID_ALL;
            CTCLIB_SLIST_LOOP(p_arp->static_route_dup_ipv4_nh.obj_list, p_db_static_route_dup_ipv4_nh, listnode)
            {
                ds_static_route_dup_ipv4_nh_dump_one(p_db_static_route_dup_ipv4_nh, pargs);
                sal_fprintf(fp, "/,");
            }
        }
        sal_fprintf(fp, "}");
    }
    if (FLD_MATCH(TBL_ARP_FLD_RETRY_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ARP_FLD_RETRY_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_arp->retry_timer));
    }
    if (FLD_MATCH(TBL_ARP_FLD_REFRESH_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ARP_FLD_REFRESH_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_arp->refresh_timer));
    }
    if (FLD_MATCH(TBL_ARP_FLD_GRA_RSP_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ARP_FLD_GRA_RSP_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_arp->gra_rsp_timer));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_arp_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_arp_master_t *p_master = _g_p_tbl_arp_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_arp_t *p_db_arp = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->arp_list, p_db_arp, listnode, next)
    {
        rc |= fn(p_db_arp, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_arp_master_t*
tbl_arp_get_master()
{
    return _g_p_tbl_arp_master;
}

tbl_arp_master_t*
tbl_arp_init_arp()
{
    _g_p_tbl_arp_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_arp_master_t));
    _g_p_tbl_arp_master->arp_hash = ctclib_hash_create(_tbl_arp_hash_make, _tbl_arp_hash_cmp);
    _g_p_tbl_arp_master->arp_list = ctclib_slist_create(tbl_arp_cmp, NULL);
    return _g_p_tbl_arp_master;
}

