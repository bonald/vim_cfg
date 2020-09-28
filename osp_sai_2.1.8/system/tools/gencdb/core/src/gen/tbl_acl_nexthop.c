
#include "proto.h"
#include "gen/tbl_acl_nexthop_define.h"
#include "gen/tbl_acl_nexthop.h"
#include "cdb_data_cmp.h"

tbl_acl_nexthop_master_t *_g_p_tbl_acl_nexthop_master = NULL;

static uint32
_tbl_acl_nexthop_hash_make(tbl_acl_nexthop_t *p_acl_nexthop)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_acl_nexthop->key;
    for (index = 0; index < sizeof(p_acl_nexthop->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_acl_nexthop_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_acl_nexthop_t *p_acl_nexthop1 = (tbl_acl_nexthop_t*)p_arg1;
    tbl_acl_nexthop_t *p_acl_nexthop2 = (tbl_acl_nexthop_t*)p_arg2;

    if (0 == sal_memcmp(&p_acl_nexthop1->key, &p_acl_nexthop2->key, sizeof(tbl_acl_nexthop_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_acl_nexthop_add_acl_nexthop_sync(tbl_acl_nexthop_t *p_acl_nexthop, uint32 sync)
{
    tbl_acl_nexthop_master_t *p_master = _g_p_tbl_acl_nexthop_master;
    tbl_acl_nexthop_t *p_db_acl_nexthop = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_acl_nexthop_get_acl_nexthop(&p_acl_nexthop->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_acl_nexthop = XCALLOC(MEM_TBL_ACL_NEXTHOP, sizeof(tbl_acl_nexthop_t));
    if (NULL == p_db_acl_nexthop)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_acl_nexthop, p_acl_nexthop, sizeof(tbl_acl_nexthop_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->acl_nexthop_hash, (void*)p_db_acl_nexthop, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_ACL_NEXTHOP, p_db_acl_nexthop);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_ACL_NEXTHOP, p_db_acl_nexthop);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_acl_nexthop_del_acl_nexthop_sync(tbl_acl_nexthop_key_t *p_acl_nexthop_key, uint32 sync)
{
    tbl_acl_nexthop_master_t *p_master = _g_p_tbl_acl_nexthop_master;
    tbl_acl_nexthop_t *p_db_acl_nexthop = NULL;

    /* 1. lookup entry exist */
    p_db_acl_nexthop = tbl_acl_nexthop_get_acl_nexthop(p_acl_nexthop_key);
    if (NULL == p_db_acl_nexthop)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_ACL_NEXTHOP, p_db_acl_nexthop);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_ACL_NEXTHOP, p_db_acl_nexthop);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->acl_nexthop_hash, (void*)p_db_acl_nexthop);

    /* 4. free db entry */
    XFREE(MEM_TBL_ACL_NEXTHOP, p_db_acl_nexthop);

    return PM_E_NONE;
}

int32
tbl_acl_nexthop_set_acl_nexthop_field_sync(tbl_acl_nexthop_t *p_acl_nexthop, tbl_acl_nexthop_field_id_t field_id, uint32 sync)
{
    tbl_acl_nexthop_t *p_db_acl_nexthop = NULL;

    /* 1. lookup entry exist */
    p_db_acl_nexthop = tbl_acl_nexthop_get_acl_nexthop(&p_acl_nexthop->key);
    if (NULL == p_db_acl_nexthop)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_ACL_NEXTHOP_FLD_EDIT_FLAGS:
        p_db_acl_nexthop->edit_flags = p_acl_nexthop->edit_flags;
        break;

    case TBL_ACL_NEXTHOP_FLD_PORT:
        p_db_acl_nexthop->port = p_acl_nexthop->port;
        break;

    case TBL_ACL_NEXTHOP_FLD_SRC_MAC:
        sal_memcpy(p_db_acl_nexthop->src_mac, p_acl_nexthop->src_mac, sizeof(p_acl_nexthop->src_mac));
        break;

    case TBL_ACL_NEXTHOP_FLD_DST_MAC:
        sal_memcpy(p_db_acl_nexthop->dst_mac, p_acl_nexthop->dst_mac, sizeof(p_acl_nexthop->dst_mac));
        break;

    case TBL_ACL_NEXTHOP_FLD_VLAN:
        p_db_acl_nexthop->vlan = p_acl_nexthop->vlan;
        break;

    case TBL_ACL_NEXTHOP_FLD_SRC_IPV4:
        sal_memcpy(&p_db_acl_nexthop->src_ipv4, &p_acl_nexthop->src_ipv4, sizeof(p_acl_nexthop->src_ipv4));
        break;

    case TBL_ACL_NEXTHOP_FLD_DST_IPV4:
        sal_memcpy(&p_db_acl_nexthop->dst_ipv4, &p_acl_nexthop->dst_ipv4, sizeof(p_acl_nexthop->dst_ipv4));
        break;

    case TBL_ACL_NEXTHOP_FLD_DSCP:
        p_db_acl_nexthop->dscp = p_acl_nexthop->dscp;
        break;

    case TBL_ACL_NEXTHOP_FLD_L4_SRC_PORT:
        p_db_acl_nexthop->l4_src_port = p_acl_nexthop->l4_src_port;
        break;

    case TBL_ACL_NEXTHOP_FLD_L4_DST_PORT:
        p_db_acl_nexthop->l4_dst_port = p_acl_nexthop->l4_dst_port;
        break;

    case TBL_ACL_NEXTHOP_FLD_REF:
        p_db_acl_nexthop->ref = p_acl_nexthop->ref;
        break;

    case TBL_ACL_NEXTHOP_FLD_MAX:
        sal_memcpy(p_db_acl_nexthop, p_acl_nexthop, sizeof(tbl_acl_nexthop_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_ACL_NEXTHOP, field_id, p_db_acl_nexthop);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_ACL_NEXTHOP, field_id, p_db_acl_nexthop);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_acl_nexthop_add_acl_nexthop(tbl_acl_nexthop_t *p_acl_nexthop)
{
    return tbl_acl_nexthop_add_acl_nexthop_sync(p_acl_nexthop, TRUE);
}

int32
tbl_acl_nexthop_del_acl_nexthop(tbl_acl_nexthop_key_t *p_acl_nexthop_key)
{
    return tbl_acl_nexthop_del_acl_nexthop_sync(p_acl_nexthop_key, TRUE);
}

int32
tbl_acl_nexthop_set_acl_nexthop_field(tbl_acl_nexthop_t *p_acl_nexthop, tbl_acl_nexthop_field_id_t field_id)
{
    return tbl_acl_nexthop_set_acl_nexthop_field_sync(p_acl_nexthop, field_id, TRUE);
}

tbl_acl_nexthop_t*
tbl_acl_nexthop_get_acl_nexthop(tbl_acl_nexthop_key_t *p_acl_nexthop_key)
{
    tbl_acl_nexthop_master_t *p_master = _g_p_tbl_acl_nexthop_master;
    tbl_acl_nexthop_t lkp;

    sal_memcpy(&lkp.key, p_acl_nexthop_key, sizeof(tbl_acl_nexthop_key_t));
    return ctclib_hash_lookup(p_master->acl_nexthop_hash, &lkp);
}

char*
tbl_acl_nexthop_key_val2str(tbl_acl_nexthop_t *p_acl_nexthop, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_acl_nexthop->key.acl_nexthop_id);
    return str;
}

int32
tbl_acl_nexthop_key_str2val(char *str, tbl_acl_nexthop_t *p_acl_nexthop)
{
    int32 ret = 0;

    p_acl_nexthop->key.acl_nexthop_id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_acl_nexthop_key_name_dump(tbl_acl_nexthop_t *p_acl_nexthop, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_NEXTHOP);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_acl_nexthop->key.acl_nexthop_id);
    return str;
}

char*
tbl_acl_nexthop_key_value_dump(tbl_acl_nexthop_t *p_acl_nexthop, char *str)
{
    sal_sprintf(str, "%u", p_acl_nexthop->key.acl_nexthop_id);
    return str;
}

char*
tbl_acl_nexthop_field_name_dump(tbl_acl_nexthop_t *p_acl_nexthop, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_NEXTHOP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_ACL_NEXTHOP_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_acl_nexthop_field_value_dump(tbl_acl_nexthop_t *p_acl_nexthop, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_ACL_NEXTHOP_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_acl_nexthop->key.acl_nexthop_id);
    }
    if (FLD_MATCH(TBL_ACL_NEXTHOP_FLD_EDIT_FLAGS, field_id))
    {
        sal_sprintf(str, "%u", p_acl_nexthop->edit_flags);
    }
    if (FLD_MATCH(TBL_ACL_NEXTHOP_FLD_PORT, field_id))
    {
        sal_sprintf(str, "%u", p_acl_nexthop->port);
    }
    if (FLD_MATCH(TBL_ACL_NEXTHOP_FLD_SRC_MAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_acl_nexthop->src_mac);
    }
    if (FLD_MATCH(TBL_ACL_NEXTHOP_FLD_DST_MAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_acl_nexthop->dst_mac);
    }
    if (FLD_MATCH(TBL_ACL_NEXTHOP_FLD_VLAN, field_id))
    {
        sal_sprintf(str, "%u", p_acl_nexthop->vlan);
    }
    if (FLD_MATCH(TBL_ACL_NEXTHOP_FLD_SRC_IPV4, field_id))
    {
        cdb_addr_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_acl_nexthop->src_ipv4);
    }
    if (FLD_MATCH(TBL_ACL_NEXTHOP_FLD_DST_IPV4, field_id))
    {
        cdb_addr_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_acl_nexthop->dst_ipv4);
    }
    if (FLD_MATCH(TBL_ACL_NEXTHOP_FLD_DSCP, field_id))
    {
        sal_sprintf(str, "%u", p_acl_nexthop->dscp);
    }
    if (FLD_MATCH(TBL_ACL_NEXTHOP_FLD_L4_SRC_PORT, field_id))
    {
        sal_sprintf(str, "%u", p_acl_nexthop->l4_src_port);
    }
    if (FLD_MATCH(TBL_ACL_NEXTHOP_FLD_L4_DST_PORT, field_id))
    {
        sal_sprintf(str, "%u", p_acl_nexthop->l4_dst_port);
    }
    if (FLD_MATCH(TBL_ACL_NEXTHOP_FLD_REF, field_id))
    {
        sal_sprintf(str, "%u", p_acl_nexthop->ref);
    }
    return str;
}

char**
tbl_acl_nexthop_table_dump(tbl_acl_nexthop_t *p_acl_nexthop, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_acl_nexthop_key_name_dump(p_acl_nexthop, buf));
    for(i=1; i<TBL_ACL_NEXTHOP_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_acl_nexthop_field_name_dump(p_acl_nexthop, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_acl_nexthop_field_value_dump(p_acl_nexthop, i, buf));
    }
    return str;
}

int32
tbl_acl_nexthop_field_value_parser(char *str, int32 field_id, tbl_acl_nexthop_t *p_acl_nexthop)
{
    if (FLD_MATCH(TBL_ACL_NEXTHOP_FLD_KEY, field_id))
    {
        int32 ret;
        p_acl_nexthop->key.acl_nexthop_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_NEXTHOP_FLD_EDIT_FLAGS, field_id))
    {
        int32 ret;
        p_acl_nexthop->edit_flags = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_NEXTHOP_FLD_PORT, field_id))
    {
        int32 ret;
        p_acl_nexthop->port = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_NEXTHOP_FLD_SRC_MAC, field_id))
    {
        cdb_mac_addr_str2val(p_acl_nexthop->src_mac, str);
    }
    if (FLD_MATCH(TBL_ACL_NEXTHOP_FLD_DST_MAC, field_id))
    {
        cdb_mac_addr_str2val(p_acl_nexthop->dst_mac, str);
    }
    if (FLD_MATCH(TBL_ACL_NEXTHOP_FLD_VLAN, field_id))
    {
        int32 ret;
        p_acl_nexthop->vlan = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_NEXTHOP_FLD_SRC_IPV4, field_id))
    {
        cdb_addr_ipv4_str2val(&p_acl_nexthop->src_ipv4, str, 0);
    }
    if (FLD_MATCH(TBL_ACL_NEXTHOP_FLD_DST_IPV4, field_id))
    {
        cdb_addr_ipv4_str2val(&p_acl_nexthop->dst_ipv4, str, 0);
    }
    if (FLD_MATCH(TBL_ACL_NEXTHOP_FLD_DSCP, field_id))
    {
        int32 ret;
        p_acl_nexthop->dscp = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_NEXTHOP_FLD_L4_SRC_PORT, field_id))
    {
        int32 ret;
        p_acl_nexthop->l4_src_port = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_NEXTHOP_FLD_L4_DST_PORT, field_id))
    {
        int32 ret;
        p_acl_nexthop->l4_dst_port = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACL_NEXTHOP_FLD_REF, field_id))
    {
        int32 ret;
        p_acl_nexthop->ref = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_acl_nexthop_table_parser(char** array, char* key_value,tbl_acl_nexthop_t *p_acl_nexthop)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_NEXTHOP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_acl_nexthop_key_str2val(key_value, p_acl_nexthop));

    for(i=1; i<TBL_ACL_NEXTHOP_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_acl_nexthop_field_value_parser( array[j++], i, p_acl_nexthop));
    }

    return PM_E_NONE;
}

int32
tbl_acl_nexthop_dump_one(tbl_acl_nexthop_t *p_acl_nexthop, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACL_NEXTHOP);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_ACL_NEXTHOP_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_NEXTHOP_FLD_KEY].name,
            p_acl_nexthop->key.acl_nexthop_id);
    }
    if (FLD_MATCH(TBL_ACL_NEXTHOP_FLD_EDIT_FLAGS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_NEXTHOP_FLD_EDIT_FLAGS].name,
            p_acl_nexthop->edit_flags);
    }
    if (FLD_MATCH(TBL_ACL_NEXTHOP_FLD_PORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_NEXTHOP_FLD_PORT].name,
            p_acl_nexthop->port);
    }
    if (FLD_MATCH(TBL_ACL_NEXTHOP_FLD_SRC_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_NEXTHOP_FLD_SRC_MAC].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_acl_nexthop->src_mac));
    }
    if (FLD_MATCH(TBL_ACL_NEXTHOP_FLD_DST_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_NEXTHOP_FLD_DST_MAC].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_acl_nexthop->dst_mac));
    }
    if (FLD_MATCH(TBL_ACL_NEXTHOP_FLD_VLAN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_NEXTHOP_FLD_VLAN].name,
            p_acl_nexthop->vlan);
    }
    if (FLD_MATCH(TBL_ACL_NEXTHOP_FLD_SRC_IPV4, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_NEXTHOP_FLD_SRC_IPV4].name, 
            cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_acl_nexthop->src_ipv4));
    }
    if (FLD_MATCH(TBL_ACL_NEXTHOP_FLD_DST_IPV4, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACL_NEXTHOP_FLD_DST_IPV4].name, 
            cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_acl_nexthop->dst_ipv4));
    }
    if (FLD_MATCH(TBL_ACL_NEXTHOP_FLD_DSCP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_NEXTHOP_FLD_DSCP].name,
            p_acl_nexthop->dscp);
    }
    if (FLD_MATCH(TBL_ACL_NEXTHOP_FLD_L4_SRC_PORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_NEXTHOP_FLD_L4_SRC_PORT].name,
            p_acl_nexthop->l4_src_port);
    }
    if (FLD_MATCH(TBL_ACL_NEXTHOP_FLD_L4_DST_PORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_NEXTHOP_FLD_L4_DST_PORT].name,
            p_acl_nexthop->l4_dst_port);
    }
    if (FLD_MATCH(TBL_ACL_NEXTHOP_FLD_REF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACL_NEXTHOP_FLD_REF].name,
            p_acl_nexthop->ref);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_acl_nexthop_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_acl_nexthop_master_t *p_master = _g_p_tbl_acl_nexthop_master;
    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    ctclib_hash_iterate2(p_master->acl_nexthop_hash, tbl_hash_iter_adpt_fn, fn, pargs);
    return PM_E_NONE;
}

tbl_acl_nexthop_master_t*
tbl_acl_nexthop_get_master()
{
    return _g_p_tbl_acl_nexthop_master;
}

tbl_acl_nexthop_master_t*
tbl_acl_nexthop_init_acl_nexthop()
{
    _g_p_tbl_acl_nexthop_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_acl_nexthop_master_t));
    _g_p_tbl_acl_nexthop_master->acl_nexthop_hash = ctclib_hash_create(_tbl_acl_nexthop_hash_make, _tbl_acl_nexthop_hash_cmp);
    return _g_p_tbl_acl_nexthop_master;
}

