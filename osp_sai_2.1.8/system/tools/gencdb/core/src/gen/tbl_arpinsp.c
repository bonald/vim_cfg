
#include "proto.h"
#include "gen/tbl_arpinsp_define.h"
#include "gen/tbl_arpinsp.h"
#include "cdb_data_cmp.h"

tbl_arpinsp_master_t *_g_p_tbl_arpinsp_master = NULL;

int32
tbl_arpinsp_add_arpinsp_sync(tbl_arpinsp_t *p_arpinsp, uint32 sync)
{
    tbl_arpinsp_master_t *p_master = _g_p_tbl_arpinsp_master;
    tbl_arpinsp_t *p_db_arpinsp = NULL;

    /* 1. lookup entry exist */
    if (tbl_arpinsp_get_arpinsp(&p_arpinsp->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_arpinsp = XCALLOC(MEM_TBL_ARPINSP, sizeof(tbl_arpinsp_t));
    if (NULL == p_db_arpinsp)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_arpinsp, p_arpinsp, sizeof(tbl_arpinsp_t));

    /* 4. add to db */
    p_master->arpinsp_array[p_arpinsp->key.vlan_id] = p_db_arpinsp;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_ARPINSP, p_db_arpinsp);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_ARPINSP, p_db_arpinsp);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_arpinsp_del_arpinsp_sync(tbl_arpinsp_key_t *p_arpinsp_key, uint32 sync)
{
    tbl_arpinsp_master_t *p_master = _g_p_tbl_arpinsp_master;
    tbl_arpinsp_t *p_db_arpinsp = NULL;

    /* 1. lookup entry exist */
    p_db_arpinsp = tbl_arpinsp_get_arpinsp(p_arpinsp_key);
    if (NULL == p_db_arpinsp)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_ARPINSP, p_db_arpinsp);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_ARPINSP, p_db_arpinsp);
        #endif 
    }

    /* 3. delete from db */
    p_master->arpinsp_array[p_arpinsp_key->vlan_id] = NULL;

    /* 4. free db entry */
    XFREE(MEM_TBL_ARPINSP, p_db_arpinsp);

    return PM_E_NONE;
}

int32
tbl_arpinsp_set_arpinsp_field_sync(tbl_arpinsp_t *p_arpinsp, tbl_arpinsp_field_id_t field_id, uint32 sync)
{
    tbl_arpinsp_t *p_db_arpinsp = NULL;

    /* 1. lookup entry exist */
    p_db_arpinsp = tbl_arpinsp_get_arpinsp(&p_arpinsp->key);
    if (NULL == p_db_arpinsp)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_ARPINSP_FLD_ACLLOG_FILTER:
        p_db_arpinsp->acllog_filter = p_arpinsp->acllog_filter;
        break;

    case TBL_ARPINSP_FLD_DHCPLOG_FILTER:
        p_db_arpinsp->dhcplog_filter = p_arpinsp->dhcplog_filter;
        break;

    case TBL_ARPINSP_FLD_FILTER_STATE:
        p_db_arpinsp->filter_state = p_arpinsp->filter_state;
        break;

    case TBL_ARPINSP_FLD_ENABLE_STATE:
        p_db_arpinsp->enable_state = p_arpinsp->enable_state;
        break;

    case TBL_ARPINSP_FLD_FORWARDED:
        p_db_arpinsp->forwarded = p_arpinsp->forwarded;
        break;

    case TBL_ARPINSP_FLD_DROPPED:
        p_db_arpinsp->dropped = p_arpinsp->dropped;
        break;

    case TBL_ARPINSP_FLD_DHCP_DROPPED:
        p_db_arpinsp->dhcp_dropped = p_arpinsp->dhcp_dropped;
        break;

    case TBL_ARPINSP_FLD_ACL_DROPPED:
        p_db_arpinsp->acl_dropped = p_arpinsp->acl_dropped;
        break;

    case TBL_ARPINSP_FLD_DHCP_PERMIT:
        p_db_arpinsp->dhcp_permit = p_arpinsp->dhcp_permit;
        break;

    case TBL_ARPINSP_FLD_ACL_PERMIT:
        p_db_arpinsp->acl_permit = p_arpinsp->acl_permit;
        break;

    case TBL_ARPINSP_FLD_SRCMAC_FAILURE:
        p_db_arpinsp->srcmac_failure = p_arpinsp->srcmac_failure;
        break;

    case TBL_ARPINSP_FLD_DSTMAC_FAILURE:
        p_db_arpinsp->dstmac_failure = p_arpinsp->dstmac_failure;
        break;

    case TBL_ARPINSP_FLD_IP_FAILURE:
        p_db_arpinsp->ip_failure = p_arpinsp->ip_failure;
        break;

    case TBL_ARPINSP_FLD_INVALID_PROTOCOL:
        p_db_arpinsp->invalid_protocol = p_arpinsp->invalid_protocol;
        break;

    case TBL_ARPINSP_FLD_FILTER:
        sal_memcpy(p_db_arpinsp->filter, p_arpinsp->filter, sizeof(p_arpinsp->filter));
        break;

    case TBL_ARPINSP_FLD_MAX:
        sal_memcpy(p_db_arpinsp, p_arpinsp, sizeof(tbl_arpinsp_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_ARPINSP, field_id, p_db_arpinsp);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_ARPINSP, field_id, p_db_arpinsp);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_arpinsp_add_arpinsp(tbl_arpinsp_t *p_arpinsp)
{
    return tbl_arpinsp_add_arpinsp_sync(p_arpinsp, TRUE);
}

int32
tbl_arpinsp_del_arpinsp(tbl_arpinsp_key_t *p_arpinsp_key)
{
    return tbl_arpinsp_del_arpinsp_sync(p_arpinsp_key, TRUE);
}

int32
tbl_arpinsp_set_arpinsp_field(tbl_arpinsp_t *p_arpinsp, tbl_arpinsp_field_id_t field_id)
{
    return tbl_arpinsp_set_arpinsp_field_sync(p_arpinsp, field_id, TRUE);
}

tbl_arpinsp_t*
tbl_arpinsp_get_arpinsp(tbl_arpinsp_key_t *p_arpinsp_key)
{
    tbl_arpinsp_master_t *p_master = _g_p_tbl_arpinsp_master;

    if (GLB_MAX_VLAN_ID <= p_arpinsp_key->vlan_id)
    {
        return NULL;
    }
    return (p_master->arpinsp_array[p_arpinsp_key->vlan_id]);
}

char*
tbl_arpinsp_key_val2str(tbl_arpinsp_t *p_arpinsp, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_arpinsp->key.vlan_id);
    return str;
}

int32
tbl_arpinsp_key_str2val(char *str, tbl_arpinsp_t *p_arpinsp)
{
    int32 ret = 0;

    p_arpinsp->key.vlan_id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_arpinsp_key_name_dump(tbl_arpinsp_t *p_arpinsp, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ARPINSP);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_arpinsp->key.vlan_id);
    return str;
}

char*
tbl_arpinsp_key_value_dump(tbl_arpinsp_t *p_arpinsp, char *str)
{
    sal_sprintf(str, "%u", p_arpinsp->key.vlan_id);
    return str;
}

char*
tbl_arpinsp_field_name_dump(tbl_arpinsp_t *p_arpinsp, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_ARPINSP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_ARPINSP_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_arpinsp_field_value_dump(tbl_arpinsp_t *p_arpinsp, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_ARPINSP_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_arpinsp->key.vlan_id);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_ACLLOG_FILTER, field_id))
    {
        sal_sprintf(str, "%u", p_arpinsp->acllog_filter);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_DHCPLOG_FILTER, field_id))
    {
        sal_sprintf(str, "%u", p_arpinsp->dhcplog_filter);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_FILTER_STATE, field_id))
    {
        sal_sprintf(str, "%u", p_arpinsp->filter_state);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_ENABLE_STATE, field_id))
    {
        sal_sprintf(str, "%u", p_arpinsp->enable_state);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_FORWARDED, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_arpinsp->forwarded);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_DROPPED, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_arpinsp->dropped);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_DHCP_DROPPED, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_arpinsp->dhcp_dropped);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_ACL_DROPPED, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_arpinsp->acl_dropped);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_DHCP_PERMIT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_arpinsp->dhcp_permit);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_ACL_PERMIT, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_arpinsp->acl_permit);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_SRCMAC_FAILURE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_arpinsp->srcmac_failure);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_DSTMAC_FAILURE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_arpinsp->dstmac_failure);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_IP_FAILURE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_arpinsp->ip_failure);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_INVALID_PROTOCOL, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_arpinsp->invalid_protocol);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_FILTER, field_id))
    {
        sal_sprintf(str, "%s", p_arpinsp->filter);
    }
    return str;
}

char**
tbl_arpinsp_table_dump(tbl_arpinsp_t *p_arpinsp, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_arpinsp_key_name_dump(p_arpinsp, buf));
    for(i=1; i<TBL_ARPINSP_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_arpinsp_field_name_dump(p_arpinsp, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_arpinsp_field_value_dump(p_arpinsp, i, buf));
    }
    return str;
}

int32
tbl_arpinsp_field_value_parser(char *str, int32 field_id, tbl_arpinsp_t *p_arpinsp)
{
    if (FLD_MATCH(TBL_ARPINSP_FLD_KEY, field_id))
    {
        int32 ret;
        p_arpinsp->key.vlan_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_ACLLOG_FILTER, field_id))
    {
        int32 ret;
        p_arpinsp->acllog_filter = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_DHCPLOG_FILTER, field_id))
    {
        int32 ret;
        p_arpinsp->dhcplog_filter = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_FILTER_STATE, field_id))
    {
        int32 ret;
        p_arpinsp->filter_state = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_ENABLE_STATE, field_id))
    {
        int32 ret;
        p_arpinsp->enable_state = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_FORWARDED, field_id))
    {
        int32 ret;
        p_arpinsp->forwarded = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_DROPPED, field_id))
    {
        int32 ret;
        p_arpinsp->dropped = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_DHCP_DROPPED, field_id))
    {
        int32 ret;
        p_arpinsp->dhcp_dropped = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_ACL_DROPPED, field_id))
    {
        int32 ret;
        p_arpinsp->acl_dropped = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_DHCP_PERMIT, field_id))
    {
        int32 ret;
        p_arpinsp->dhcp_permit = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_ACL_PERMIT, field_id))
    {
        int32 ret;
        p_arpinsp->acl_permit = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_SRCMAC_FAILURE, field_id))
    {
        int32 ret;
        p_arpinsp->srcmac_failure = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_DSTMAC_FAILURE, field_id))
    {
        int32 ret;
        p_arpinsp->dstmac_failure = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_IP_FAILURE, field_id))
    {
        int32 ret;
        p_arpinsp->ip_failure = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_INVALID_PROTOCOL, field_id))
    {
        int32 ret;
        p_arpinsp->invalid_protocol = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_FILTER, field_id))
    {
        sal_strcpy(p_arpinsp->filter, str);
    }
    return PM_E_NONE;
}

int32
tbl_arpinsp_table_parser(char** array, char* key_value,tbl_arpinsp_t *p_arpinsp)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_ARPINSP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_arpinsp_key_str2val(key_value, p_arpinsp));

    for(i=1; i<TBL_ARPINSP_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_arpinsp_field_value_parser( array[j++], i, p_arpinsp));
    }

    return PM_E_NONE;
}

int32
tbl_arpinsp_dump_one(tbl_arpinsp_t *p_arpinsp, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ARPINSP);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_ARPINSP_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ARPINSP_FLD_KEY].name,
            p_arpinsp->key.vlan_id);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_ACLLOG_FILTER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ARPINSP_FLD_ACLLOG_FILTER].name,
            p_arpinsp->acllog_filter);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_DHCPLOG_FILTER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ARPINSP_FLD_DHCPLOG_FILTER].name,
            p_arpinsp->dhcplog_filter);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_FILTER_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ARPINSP_FLD_FILTER_STATE].name,
            p_arpinsp->filter_state);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_ENABLE_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ARPINSP_FLD_ENABLE_STATE].name,
            p_arpinsp->enable_state);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_FORWARDED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ARPINSP_FLD_FORWARDED].name,
            p_arpinsp->forwarded);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_DROPPED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ARPINSP_FLD_DROPPED].name,
            p_arpinsp->dropped);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_DHCP_DROPPED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ARPINSP_FLD_DHCP_DROPPED].name,
            p_arpinsp->dhcp_dropped);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_ACL_DROPPED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ARPINSP_FLD_ACL_DROPPED].name,
            p_arpinsp->acl_dropped);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_DHCP_PERMIT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ARPINSP_FLD_DHCP_PERMIT].name,
            p_arpinsp->dhcp_permit);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_ACL_PERMIT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ARPINSP_FLD_ACL_PERMIT].name,
            p_arpinsp->acl_permit);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_SRCMAC_FAILURE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ARPINSP_FLD_SRCMAC_FAILURE].name,
            p_arpinsp->srcmac_failure);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_DSTMAC_FAILURE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ARPINSP_FLD_DSTMAC_FAILURE].name,
            p_arpinsp->dstmac_failure);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_IP_FAILURE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ARPINSP_FLD_IP_FAILURE].name,
            p_arpinsp->ip_failure);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_INVALID_PROTOCOL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_ARPINSP_FLD_INVALID_PROTOCOL].name,
            p_arpinsp->invalid_protocol);
    }
    if (FLD_MATCH(TBL_ARPINSP_FLD_FILTER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ARPINSP_FLD_FILTER].name,
            p_arpinsp->filter);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_arpinsp_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_arpinsp_master_t *p_master = _g_p_tbl_arpinsp_master;
    tbl_arpinsp_t *p_db_arpinsp = NULL;
    uint32 vlan_id = 0;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    for (vlan_id = 0; vlan_id < GLB_MAX_VLAN_ID; vlan_id++)
    {
        p_db_arpinsp = p_master->arpinsp_array[vlan_id];
        if (NULL == p_db_arpinsp)
        {
            continue;
        }
        rc |= fn(p_db_arpinsp, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_arpinsp_master_t*
tbl_arpinsp_get_master()
{
    return _g_p_tbl_arpinsp_master;
}

tbl_arpinsp_master_t*
tbl_arpinsp_init_arpinsp()
{
    _g_p_tbl_arpinsp_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_arpinsp_master_t));
    return _g_p_tbl_arpinsp_master;
}

