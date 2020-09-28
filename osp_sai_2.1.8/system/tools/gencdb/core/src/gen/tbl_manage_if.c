
#include "proto.h"
#include "gen/tbl_manage_if_define.h"
#include "gen/tbl_manage_if.h"
#include "cdb_data_cmp.h"

tbl_manage_if_t *_g_p_tbl_manage_if = NULL;

int32
tbl_manage_if_set_manage_if_field_sync(tbl_manage_if_t *p_mng_if, tbl_manage_if_field_id_t field_id, uint32 sync)
{
    tbl_manage_if_t *p_db_mng_if = _g_p_tbl_manage_if;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_MANAGE_IF_FLD_ADDR:
        sal_memcpy(&p_db_mng_if->addr, &p_mng_if->addr, sizeof(p_mng_if->addr));
        break;

    case TBL_MANAGE_IF_FLD_GATEWAY:
        sal_memcpy(&p_db_mng_if->gateway, &p_mng_if->gateway, sizeof(p_mng_if->gateway));
        break;

    case TBL_MANAGE_IF_FLD_DHCLIENT_ENABLE:
        p_db_mng_if->dhclient_enable = p_mng_if->dhclient_enable;
        break;

    case TBL_MANAGE_IF_FLD_DHCLIENT_ADDR:
        sal_memcpy(&p_db_mng_if->dhclient_addr, &p_mng_if->dhclient_addr, sizeof(p_mng_if->dhclient_addr));
        break;

    case TBL_MANAGE_IF_FLD_DHCLIENT_GATEWAY:
        sal_memcpy(&p_db_mng_if->dhclient_gateway, &p_mng_if->dhclient_gateway, sizeof(p_mng_if->dhclient_gateway));
        break;

    case TBL_MANAGE_IF_FLD_ADDR_V6:
        sal_memcpy(&p_db_mng_if->addr_v6, &p_mng_if->addr_v6, sizeof(p_mng_if->addr_v6));
        break;

    case TBL_MANAGE_IF_FLD_GATEWAY_V6:
        sal_memcpy(&p_db_mng_if->gateway_v6, &p_mng_if->gateway_v6, sizeof(p_mng_if->gateway_v6));
        break;

    case TBL_MANAGE_IF_FLD_LINK_LOCAL_ADDR_STR:
        sal_memcpy(p_db_mng_if->link_local_addr_str, p_mng_if->link_local_addr_str, sizeof(p_mng_if->link_local_addr_str));
        break;

    case TBL_MANAGE_IF_FLD_DHCLIENTV6_ENABLE:
        p_db_mng_if->dhclientv6_enable = p_mng_if->dhclientv6_enable;
        break;

    case TBL_MANAGE_IF_FLD_DHCLIENTV6_ADDR:
        sal_memcpy(&p_db_mng_if->dhclientv6_addr, &p_mng_if->dhclientv6_addr, sizeof(p_mng_if->dhclientv6_addr));
        break;

    case TBL_MANAGE_IF_FLD_DHCLIENTV6_GATEWAY:
        sal_memcpy(&p_db_mng_if->dhclientv6_gateway, &p_mng_if->dhclientv6_gateway, sizeof(p_mng_if->dhclientv6_gateway));
        break;

    case TBL_MANAGE_IF_FLD_MAX:
        sal_memcpy(p_db_mng_if, p_mng_if, sizeof(tbl_manage_if_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_MANAGE_IF, field_id, p_db_mng_if);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_MANAGE_IF, field_id, p_db_mng_if);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_manage_if_set_manage_if_field(tbl_manage_if_t *p_mng_if, tbl_manage_if_field_id_t field_id)
{
    return tbl_manage_if_set_manage_if_field_sync(p_mng_if, field_id, TRUE);
}

tbl_manage_if_t*
tbl_manage_if_get_manage_if()
{
    return _g_p_tbl_manage_if;
}

int32
tbl_manage_if_key_str2val(char *str, tbl_manage_if_t *p_mng_if)
{
    return PM_E_NONE;
}

char*
tbl_manage_if_key_name_dump(tbl_manage_if_t *p_mng_if, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_MANAGE_IF);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_manage_if_key_value_dump(tbl_manage_if_t *p_mng_if, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_manage_if_field_name_dump(tbl_manage_if_t *p_mng_if, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_MANAGE_IF);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_MANAGE_IF_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_manage_if_field_value_dump(tbl_manage_if_t *p_mng_if, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_MANAGE_IF_FLD_ADDR, field_id))
    {
        cdb_prefix_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_mng_if->addr);
    }
    if (FLD_MATCH(TBL_MANAGE_IF_FLD_GATEWAY, field_id))
    {
        cdb_addr_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_mng_if->gateway);
    }
    if (FLD_MATCH(TBL_MANAGE_IF_FLD_DHCLIENT_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_mng_if->dhclient_enable);
    }
    if (FLD_MATCH(TBL_MANAGE_IF_FLD_DHCLIENT_ADDR, field_id))
    {
        cdb_prefix_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_mng_if->dhclient_addr);
    }
    if (FLD_MATCH(TBL_MANAGE_IF_FLD_DHCLIENT_GATEWAY, field_id))
    {
        cdb_addr_ipv4_val2str(str, MAX_CMD_STR_LEN, &p_mng_if->dhclient_gateway);
    }
    if (FLD_MATCH(TBL_MANAGE_IF_FLD_ADDR_V6, field_id))
    {
        cdb_prefix_val2str(str, MAX_CMD_STR_LEN, &p_mng_if->addr_v6);
    }
    if (FLD_MATCH(TBL_MANAGE_IF_FLD_GATEWAY_V6, field_id))
    {
        cdb_prefix_val2str(str, MAX_CMD_STR_LEN, &p_mng_if->gateway_v6);
    }
    if (FLD_MATCH(TBL_MANAGE_IF_FLD_LINK_LOCAL_ADDR_STR, field_id))
    {
        sal_sprintf(str, "%s", p_mng_if->link_local_addr_str);
    }
    if (FLD_MATCH(TBL_MANAGE_IF_FLD_DHCLIENTV6_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_mng_if->dhclientv6_enable);
    }
    if (FLD_MATCH(TBL_MANAGE_IF_FLD_DHCLIENTV6_ADDR, field_id))
    {
        cdb_prefix_val2str(str, MAX_CMD_STR_LEN, &p_mng_if->dhclientv6_addr);
    }
    if (FLD_MATCH(TBL_MANAGE_IF_FLD_DHCLIENTV6_GATEWAY, field_id))
    {
        cdb_prefix_val2str(str, MAX_CMD_STR_LEN, &p_mng_if->dhclientv6_gateway);
    }
    return str;
}

char**
tbl_manage_if_table_dump(tbl_manage_if_t *p_mng_if, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_manage_if_key_name_dump(p_mng_if, buf));
    for(i=1; i<TBL_MANAGE_IF_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_manage_if_field_name_dump(p_mng_if, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_manage_if_field_value_dump(p_mng_if, i, buf));
    }
    return str;
}

int32
tbl_manage_if_field_value_parser(char *str, int32 field_id, tbl_manage_if_t *p_mng_if)
{
    if (FLD_MATCH(TBL_MANAGE_IF_FLD_ADDR, field_id))
    {
        cdb_prefix_ipv4_str2val_wo_masklen(str, &p_mng_if->addr);
    }
    if (FLD_MATCH(TBL_MANAGE_IF_FLD_GATEWAY, field_id))
    {
        cdb_addr_ipv4_str2val(&p_mng_if->gateway, str, 0);
    }
    if (FLD_MATCH(TBL_MANAGE_IF_FLD_DHCLIENT_ENABLE, field_id))
    {
        int32 ret;
        p_mng_if->dhclient_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MANAGE_IF_FLD_DHCLIENT_ADDR, field_id))
    {
        cdb_prefix_ipv4_str2val_wo_masklen(str, &p_mng_if->dhclient_addr);
    }
    if (FLD_MATCH(TBL_MANAGE_IF_FLD_DHCLIENT_GATEWAY, field_id))
    {
        cdb_addr_ipv4_str2val(&p_mng_if->dhclient_gateway, str, 0);
    }
    if (FLD_MATCH(TBL_MANAGE_IF_FLD_ADDR_V6, field_id))
    {
        cdb_prefix_str2val_wo_masklen(str, &p_mng_if->addr_v6);
    }
    if (FLD_MATCH(TBL_MANAGE_IF_FLD_GATEWAY_V6, field_id))
    {
        cdb_prefix_str2val_wo_masklen(str, &p_mng_if->gateway_v6);
    }
    if (FLD_MATCH(TBL_MANAGE_IF_FLD_LINK_LOCAL_ADDR_STR, field_id))
    {
        sal_strcpy(p_mng_if->link_local_addr_str, str);
    }
    if (FLD_MATCH(TBL_MANAGE_IF_FLD_DHCLIENTV6_ENABLE, field_id))
    {
        int32 ret;
        p_mng_if->dhclientv6_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MANAGE_IF_FLD_DHCLIENTV6_ADDR, field_id))
    {
        cdb_prefix_str2val_wo_masklen(str, &p_mng_if->dhclientv6_addr);
    }
    if (FLD_MATCH(TBL_MANAGE_IF_FLD_DHCLIENTV6_GATEWAY, field_id))
    {
        cdb_prefix_str2val_wo_masklen(str, &p_mng_if->dhclientv6_gateway);
    }
    return PM_E_NONE;
}

int32
tbl_manage_if_table_parser(char** array, char* key_value,tbl_manage_if_t *p_mng_if)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_MANAGE_IF);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_manage_if_key_str2val(key_value, p_mng_if));

    for(i=1; i<TBL_MANAGE_IF_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_manage_if_field_value_parser( array[j++], i, p_mng_if));
    }

    return PM_E_NONE;
}

int32
tbl_manage_if_dump_one(tbl_manage_if_t *p_mng_if, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_MANAGE_IF);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_MANAGE_IF_FLD_ADDR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MANAGE_IF_FLD_ADDR].name, 
            cdb_prefix_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_mng_if->addr));
    }
    if (FLD_MATCH(TBL_MANAGE_IF_FLD_GATEWAY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MANAGE_IF_FLD_GATEWAY].name, 
            cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_mng_if->gateway));
    }
    if (FLD_MATCH(TBL_MANAGE_IF_FLD_DHCLIENT_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MANAGE_IF_FLD_DHCLIENT_ENABLE].name,
            p_mng_if->dhclient_enable);
    }
    if (FLD_MATCH(TBL_MANAGE_IF_FLD_DHCLIENT_ADDR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MANAGE_IF_FLD_DHCLIENT_ADDR].name, 
            cdb_prefix_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_mng_if->dhclient_addr));
    }
    if (FLD_MATCH(TBL_MANAGE_IF_FLD_DHCLIENT_GATEWAY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MANAGE_IF_FLD_DHCLIENT_GATEWAY].name, 
            cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_mng_if->dhclient_gateway));
    }
    if (FLD_MATCH(TBL_MANAGE_IF_FLD_ADDR_V6, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MANAGE_IF_FLD_ADDR_V6].name, 
            cdb_prefix_val2str(buf, MAX_CMD_STR_LEN, &p_mng_if->addr_v6));
    }
    if (FLD_MATCH(TBL_MANAGE_IF_FLD_GATEWAY_V6, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MANAGE_IF_FLD_GATEWAY_V6].name, 
            cdb_prefix_val2str(buf, MAX_CMD_STR_LEN, &p_mng_if->gateway_v6));
    }
    if (FLD_MATCH(TBL_MANAGE_IF_FLD_LINK_LOCAL_ADDR_STR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MANAGE_IF_FLD_LINK_LOCAL_ADDR_STR].name,
            p_mng_if->link_local_addr_str);
    }
    if (FLD_MATCH(TBL_MANAGE_IF_FLD_DHCLIENTV6_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MANAGE_IF_FLD_DHCLIENTV6_ENABLE].name,
            p_mng_if->dhclientv6_enable);
    }
    if (FLD_MATCH(TBL_MANAGE_IF_FLD_DHCLIENTV6_ADDR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MANAGE_IF_FLD_DHCLIENTV6_ADDR].name, 
            cdb_prefix_val2str(buf, MAX_CMD_STR_LEN, &p_mng_if->dhclientv6_addr));
    }
    if (FLD_MATCH(TBL_MANAGE_IF_FLD_DHCLIENTV6_GATEWAY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MANAGE_IF_FLD_DHCLIENTV6_GATEWAY].name, 
            cdb_prefix_val2str(buf, MAX_CMD_STR_LEN, &p_mng_if->dhclientv6_gateway));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_manage_if_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_manage_if_t *p_db_mng_if = _g_p_tbl_manage_if;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_mng_if)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_mng_if, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_manage_if_t*
tbl_manage_if_init_manage_if()
{
    _g_p_tbl_manage_if = XCALLOC(MEM_TBL_MANAGE_IF, sizeof(tbl_manage_if_t));
    return _g_p_tbl_manage_if;
}

