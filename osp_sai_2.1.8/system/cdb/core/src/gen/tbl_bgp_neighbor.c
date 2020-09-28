
#include "proto.h"
#include "gen/tbl_bgp_neighbor_define.h"
#include "gen/tbl_bgp_neighbor.h"
#include "cdb_data_cmp.h"

tbl_bgp_neighbor_master_t *_g_p_tbl_bgp_neighbor_master = NULL;

static uint32
_tbl_bgp_neighbor_hash_make(tbl_bgp_neighbor_t *p_bgp_neighbor)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_bgp_neighbor->key;
    for (index = 0; index < sizeof(p_bgp_neighbor->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_bgp_neighbor_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_bgp_neighbor_t *p_bgp_neighbor1 = (tbl_bgp_neighbor_t*)p_arg1;
    tbl_bgp_neighbor_t *p_bgp_neighbor2 = (tbl_bgp_neighbor_t*)p_arg2;

    if (0 == sal_memcmp(&p_bgp_neighbor1->key, &p_bgp_neighbor2->key, sizeof(tbl_bgp_neighbor_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_bgp_neighbor_add_bgp_neighbor_sync(tbl_bgp_neighbor_t *p_bgp_neighbor, uint32 sync)
{
    tbl_bgp_neighbor_master_t *p_master = _g_p_tbl_bgp_neighbor_master;
    tbl_bgp_neighbor_t *p_db_bgp_neighbor = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_bgp_neighbor_get_bgp_neighbor(&p_bgp_neighbor->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_bgp_neighbor = XCALLOC(MEM_TBL_BGP_NEIGHBOR, sizeof(tbl_bgp_neighbor_t));
    if (NULL == p_db_bgp_neighbor)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_bgp_neighbor, p_bgp_neighbor, sizeof(tbl_bgp_neighbor_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->bgp_neighbor_hash, (void*)p_db_bgp_neighbor, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->bgp_neighbor_list, p_db_bgp_neighbor);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_BGP_NEIGHBOR, p_db_bgp_neighbor);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_BGP_NEIGHBOR, p_db_bgp_neighbor);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_bgp_neighbor_del_bgp_neighbor_sync(tbl_bgp_neighbor_key_t *p_bgp_neighbor_key, uint32 sync)
{
    tbl_bgp_neighbor_master_t *p_master = _g_p_tbl_bgp_neighbor_master;
    tbl_bgp_neighbor_t *p_db_bgp_neighbor = NULL;

    /* 1. lookup entry exist */
    p_db_bgp_neighbor = tbl_bgp_neighbor_get_bgp_neighbor(p_bgp_neighbor_key);
    if (NULL == p_db_bgp_neighbor)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_BGP_NEIGHBOR, p_db_bgp_neighbor);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_BGP_NEIGHBOR, p_db_bgp_neighbor);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->bgp_neighbor_hash, (void*)p_db_bgp_neighbor);
    ctclib_slistnode_delete(p_master->bgp_neighbor_list, p_db_bgp_neighbor);

    /* 4. free db entry */
    XFREE(MEM_TBL_BGP_NEIGHBOR, p_db_bgp_neighbor);

    return PM_E_NONE;
}

int32
tbl_bgp_neighbor_set_bgp_neighbor_field_sync(tbl_bgp_neighbor_t *p_bgp_neighbor, tbl_bgp_neighbor_field_id_t field_id, uint32 sync)
{
    tbl_bgp_neighbor_t *p_db_bgp_neighbor = NULL;

    /* 1. lookup entry exist */
    p_db_bgp_neighbor = tbl_bgp_neighbor_get_bgp_neighbor(&p_bgp_neighbor->key);
    if (NULL == p_db_bgp_neighbor)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_BGP_NEIGHBOR_FLD_REMOTE_AS:
        p_db_bgp_neighbor->remote_as = p_bgp_neighbor->remote_as;
        break;

    case TBL_BGP_NEIGHBOR_FLD_EBGP_MULTIHOP:
        p_db_bgp_neighbor->ebgp_multihop = p_bgp_neighbor->ebgp_multihop;
        break;

    case TBL_BGP_NEIGHBOR_FLD_PASSWORD:
        sal_memcpy(p_db_bgp_neighbor->password, p_bgp_neighbor->password, sizeof(p_bgp_neighbor->password));
        break;

    case TBL_BGP_NEIGHBOR_FLD_INTF_NAME:
        sal_memcpy(p_db_bgp_neighbor->intf_name, p_bgp_neighbor->intf_name, sizeof(p_bgp_neighbor->intf_name));
        break;

    case TBL_BGP_NEIGHBOR_FLD_INTF_ADDR:
        sal_memcpy(&p_db_bgp_neighbor->intf_addr, &p_bgp_neighbor->intf_addr, sizeof(p_bgp_neighbor->intf_addr));
        break;

    case TBL_BGP_NEIGHBOR_FLD_SHUTDOWN:
        p_db_bgp_neighbor->shutdown = p_bgp_neighbor->shutdown;
        break;

    case TBL_BGP_NEIGHBOR_FLD_NEXT_HOP_SELF:
        p_db_bgp_neighbor->next_hop_self = p_bgp_neighbor->next_hop_self;
        break;

    case TBL_BGP_NEIGHBOR_FLD_MAX:
        sal_memcpy(p_db_bgp_neighbor, p_bgp_neighbor, sizeof(tbl_bgp_neighbor_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_BGP_NEIGHBOR, field_id, p_db_bgp_neighbor);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_BGP_NEIGHBOR, field_id, p_db_bgp_neighbor);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_bgp_neighbor_add_bgp_neighbor(tbl_bgp_neighbor_t *p_bgp_neighbor)
{
    return tbl_bgp_neighbor_add_bgp_neighbor_sync(p_bgp_neighbor, TRUE);
}

int32
tbl_bgp_neighbor_del_bgp_neighbor(tbl_bgp_neighbor_key_t *p_bgp_neighbor_key)
{
    return tbl_bgp_neighbor_del_bgp_neighbor_sync(p_bgp_neighbor_key, TRUE);
}

int32
tbl_bgp_neighbor_set_bgp_neighbor_field(tbl_bgp_neighbor_t *p_bgp_neighbor, tbl_bgp_neighbor_field_id_t field_id)
{
    return tbl_bgp_neighbor_set_bgp_neighbor_field_sync(p_bgp_neighbor, field_id, TRUE);
}

tbl_bgp_neighbor_t*
tbl_bgp_neighbor_get_bgp_neighbor(tbl_bgp_neighbor_key_t *p_bgp_neighbor_key)
{
    tbl_bgp_neighbor_master_t *p_master = _g_p_tbl_bgp_neighbor_master;
    tbl_bgp_neighbor_t lkp;

    sal_memcpy(&lkp.key, p_bgp_neighbor_key, sizeof(tbl_bgp_neighbor_key_t));
    return ctclib_hash_lookup(p_master->bgp_neighbor_hash, &lkp);
}

char*
tbl_bgp_neighbor_key_val2str(tbl_bgp_neighbor_t *p_bgp_neighbor, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_bgp_neighbor->key));
    return str;
}

int32
tbl_bgp_neighbor_key_str2val(char *str, tbl_bgp_neighbor_t *p_bgp_neighbor)
{
    int32 ret = 0;

    ret = cdb_addr_str2val(&p_bgp_neighbor->key, str, 0);
    return ret;
}

char*
tbl_bgp_neighbor_key_name_dump(tbl_bgp_neighbor_t *p_bgp_neighbor, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_BGP_NEIGHBOR);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_bgp_neighbor->key));
    return str;
}

char*
tbl_bgp_neighbor_key_value_dump(tbl_bgp_neighbor_t *p_bgp_neighbor, char *str)
{

    cdb_addr_val2str(str, MAX_CMD_STR_LEN, &p_bgp_neighbor->key);
    return str;
}

char*
tbl_bgp_neighbor_field_name_dump(tbl_bgp_neighbor_t *p_bgp_neighbor, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_BGP_NEIGHBOR);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_BGP_NEIGHBOR_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_bgp_neighbor_field_value_dump(tbl_bgp_neighbor_t *p_bgp_neighbor, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_BGP_NEIGHBOR_FLD_KEY, field_id))
    {
        cdb_addr_val2str(str, MAX_CMD_STR_LEN, &p_bgp_neighbor->key);
    }
    if (FLD_MATCH(TBL_BGP_NEIGHBOR_FLD_REMOTE_AS, field_id))
    {
        sal_sprintf(str, "%u", p_bgp_neighbor->remote_as);
    }
    if (FLD_MATCH(TBL_BGP_NEIGHBOR_FLD_EBGP_MULTIHOP, field_id))
    {
        sal_sprintf(str, "%u", p_bgp_neighbor->ebgp_multihop);
    }
    if (FLD_MATCH(TBL_BGP_NEIGHBOR_FLD_PASSWORD, field_id))
    {
        sal_sprintf(str, "%s", p_bgp_neighbor->password);
    }
    if (FLD_MATCH(TBL_BGP_NEIGHBOR_FLD_INTF_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_bgp_neighbor->intf_name);
    }
    if (FLD_MATCH(TBL_BGP_NEIGHBOR_FLD_INTF_ADDR, field_id))
    {
        cdb_addr_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_bgp_neighbor->intf_addr);
    }
    if (FLD_MATCH(TBL_BGP_NEIGHBOR_FLD_SHUTDOWN, field_id))
    {
        sal_sprintf(str, "%u", p_bgp_neighbor->shutdown);
    }
    if (FLD_MATCH(TBL_BGP_NEIGHBOR_FLD_NEXT_HOP_SELF, field_id))
    {
        sal_sprintf(str, "%u", p_bgp_neighbor->next_hop_self);
    }
    return str;
}

char**
tbl_bgp_neighbor_table_dump(tbl_bgp_neighbor_t *p_bgp_neighbor, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_bgp_neighbor_key_name_dump(p_bgp_neighbor, buf));
    for(i=1; i<TBL_BGP_NEIGHBOR_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_bgp_neighbor_field_name_dump(p_bgp_neighbor, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_bgp_neighbor_field_value_dump(p_bgp_neighbor, i, buf));
    }
    return str;
}

int32
tbl_bgp_neighbor_field_value_parser(char *str, int32 field_id, tbl_bgp_neighbor_t *p_bgp_neighbor)
{
    if (FLD_MATCH(TBL_BGP_NEIGHBOR_FLD_KEY, field_id))
    {
        cdb_addr_str2val(&p_bgp_neighbor->key, str, 0);
    }
    if (FLD_MATCH(TBL_BGP_NEIGHBOR_FLD_REMOTE_AS, field_id))
    {
        int32 ret;
        p_bgp_neighbor->remote_as = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BGP_NEIGHBOR_FLD_EBGP_MULTIHOP, field_id))
    {
        int32 ret;
        p_bgp_neighbor->ebgp_multihop = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BGP_NEIGHBOR_FLD_PASSWORD, field_id))
    {
        sal_strcpy(p_bgp_neighbor->password, str);
    }
    if (FLD_MATCH(TBL_BGP_NEIGHBOR_FLD_INTF_NAME, field_id))
    {
        sal_strcpy(p_bgp_neighbor->intf_name, str);
    }
    if (FLD_MATCH(TBL_BGP_NEIGHBOR_FLD_INTF_ADDR, field_id))
    {
        cdb_addr_ipv4_str2val(&p_bgp_neighbor->intf_addr, str, 0);
    }
    if (FLD_MATCH(TBL_BGP_NEIGHBOR_FLD_SHUTDOWN, field_id))
    {
        int32 ret;
        p_bgp_neighbor->shutdown = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BGP_NEIGHBOR_FLD_NEXT_HOP_SELF, field_id))
    {
        int32 ret;
        p_bgp_neighbor->next_hop_self = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_bgp_neighbor_table_parser(char** array, char* key_value,tbl_bgp_neighbor_t *p_bgp_neighbor)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_BGP_NEIGHBOR);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_bgp_neighbor_key_str2val(key_value, p_bgp_neighbor));

    for(i=1; i<TBL_BGP_NEIGHBOR_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_bgp_neighbor_field_value_parser( array[j++], i, p_bgp_neighbor));
    }

    return PM_E_NONE;
}

int32
tbl_bgp_neighbor_dump_one(tbl_bgp_neighbor_t *p_bgp_neighbor, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_BGP_NEIGHBOR);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_BGP_NEIGHBOR_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_BGP_NEIGHBOR_FLD_KEY].name, 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_bgp_neighbor->key));
    }
    if (FLD_MATCH(TBL_BGP_NEIGHBOR_FLD_REMOTE_AS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BGP_NEIGHBOR_FLD_REMOTE_AS].name,
            p_bgp_neighbor->remote_as);
    }
    if (FLD_MATCH(TBL_BGP_NEIGHBOR_FLD_EBGP_MULTIHOP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BGP_NEIGHBOR_FLD_EBGP_MULTIHOP].name,
            p_bgp_neighbor->ebgp_multihop);
    }
    if (FLD_MATCH(TBL_BGP_NEIGHBOR_FLD_PASSWORD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_BGP_NEIGHBOR_FLD_PASSWORD].name,
            p_bgp_neighbor->password);
    }
    if (FLD_MATCH(TBL_BGP_NEIGHBOR_FLD_INTF_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_BGP_NEIGHBOR_FLD_INTF_NAME].name,
            p_bgp_neighbor->intf_name);
    }
    if (FLD_MATCH(TBL_BGP_NEIGHBOR_FLD_INTF_ADDR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_BGP_NEIGHBOR_FLD_INTF_ADDR].name, 
            cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_bgp_neighbor->intf_addr));
    }
    if (FLD_MATCH(TBL_BGP_NEIGHBOR_FLD_SHUTDOWN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BGP_NEIGHBOR_FLD_SHUTDOWN].name,
            p_bgp_neighbor->shutdown);
    }
    if (FLD_MATCH(TBL_BGP_NEIGHBOR_FLD_NEXT_HOP_SELF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BGP_NEIGHBOR_FLD_NEXT_HOP_SELF].name,
            p_bgp_neighbor->next_hop_self);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_bgp_neighbor_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_bgp_neighbor_master_t *p_master = _g_p_tbl_bgp_neighbor_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_bgp_neighbor_t *p_db_bgp_neighbor = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->bgp_neighbor_list, p_db_bgp_neighbor, listnode, next)
    {
        rc |= fn(p_db_bgp_neighbor, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_bgp_neighbor_master_t*
tbl_bgp_neighbor_get_master()
{
    return _g_p_tbl_bgp_neighbor_master;
}

tbl_bgp_neighbor_master_t*
tbl_bgp_neighbor_init_bgp_neighbor()
{
    _g_p_tbl_bgp_neighbor_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_bgp_neighbor_master_t));
    _g_p_tbl_bgp_neighbor_master->bgp_neighbor_hash = ctclib_hash_create(_tbl_bgp_neighbor_hash_make, _tbl_bgp_neighbor_hash_cmp);
    _g_p_tbl_bgp_neighbor_master->bgp_neighbor_list = ctclib_slist_create(tbl_bgp_neighbor_cmp, NULL);
    return _g_p_tbl_bgp_neighbor_master;
}

