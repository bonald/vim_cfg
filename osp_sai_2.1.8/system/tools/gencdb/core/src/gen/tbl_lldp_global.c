
#include "proto.h"
#include "gen/tbl_lldp_global_define.h"
#include "gen/tbl_lldp_global.h"
#include "cdb_data_cmp.h"

tbl_lldp_global_t *_g_p_tbl_lldp_global = NULL;

int32
tbl_lldp_global_set_lldp_global_field_sync(tbl_lldp_global_t *p_lldp_glb, tbl_lldp_global_field_id_t field_id, uint32 sync)
{
    tbl_lldp_global_t *p_db_lldp_glb = _g_p_tbl_lldp_global;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_LLDP_GLOBAL_FLD_ENABLE:
        p_db_lldp_glb->enable = p_lldp_glb->enable;
        break;

    case TBL_LLDP_GLOBAL_FLD_SYSTEM_MAC_EN:
        p_db_lldp_glb->system_mac_en = p_lldp_glb->system_mac_en;
        break;

    case TBL_LLDP_GLOBAL_FLD_CONFIG_FLAG:
        p_db_lldp_glb->config_flag = p_lldp_glb->config_flag;
        break;

    case TBL_LLDP_GLOBAL_FLD_SYSTEM_MAC:
        sal_memcpy(p_db_lldp_glb->system_mac, p_lldp_glb->system_mac, sizeof(p_lldp_glb->system_mac));
        break;

    case TBL_LLDP_GLOBAL_FLD_MANAGEMENT_IP:
        sal_memcpy(&p_db_lldp_glb->management_ip, &p_lldp_glb->management_ip, sizeof(p_lldp_glb->management_ip));
        break;

    case TBL_LLDP_GLOBAL_FLD_TX_HOLD:
        p_db_lldp_glb->tx_hold = p_lldp_glb->tx_hold;
        break;

    case TBL_LLDP_GLOBAL_FLD_TX_INTERVAL:
        p_db_lldp_glb->tx_interval = p_lldp_glb->tx_interval;
        break;

    case TBL_LLDP_GLOBAL_FLD_TX_DELAY:
        p_db_lldp_glb->tx_delay = p_lldp_glb->tx_delay;
        break;

    case TBL_LLDP_GLOBAL_FLD_REINIT_DELAY:
        p_db_lldp_glb->reinit_delay = p_lldp_glb->reinit_delay;
        break;

    case TBL_LLDP_GLOBAL_FLD_SYSTEM_CAPABILITY:
        p_db_lldp_glb->system_capability = p_lldp_glb->system_capability;
        break;

    case TBL_LLDP_GLOBAL_FLD_SYSTEM_NAME:
        sal_memcpy(p_db_lldp_glb->system_name, p_lldp_glb->system_name, sizeof(p_lldp_glb->system_name));
        break;

    case TBL_LLDP_GLOBAL_FLD_SYSTEM_DESC:
        sal_memcpy(p_db_lldp_glb->system_desc, p_lldp_glb->system_desc, sizeof(p_lldp_glb->system_desc));
        break;

    case TBL_LLDP_GLOBAL_FLD_SYSTEM_DESC_DEFAULT:
        sal_memcpy(p_db_lldp_glb->system_desc_default, p_lldp_glb->system_desc_default, sizeof(p_lldp_glb->system_desc_default));
        break;

    case TBL_LLDP_GLOBAL_FLD_MAX:
        sal_memcpy(p_db_lldp_glb, p_lldp_glb, sizeof(tbl_lldp_global_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_LLDP_GLOBAL, field_id, p_db_lldp_glb);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_LLDP_GLOBAL, field_id, p_db_lldp_glb);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_lldp_global_set_lldp_global_field(tbl_lldp_global_t *p_lldp_glb, tbl_lldp_global_field_id_t field_id)
{
    return tbl_lldp_global_set_lldp_global_field_sync(p_lldp_glb, field_id, TRUE);
}

tbl_lldp_global_t*
tbl_lldp_global_get_lldp_global()
{
    return _g_p_tbl_lldp_global;
}

int32
tbl_lldp_global_key_str2val(char *str, tbl_lldp_global_t *p_lldp_glb)
{
    return PM_E_NONE;
}

char*
tbl_lldp_global_key_name_dump(tbl_lldp_global_t *p_lldp_glb, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_LLDP_GLOBAL);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_lldp_global_key_value_dump(tbl_lldp_global_t *p_lldp_glb, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_lldp_global_field_name_dump(tbl_lldp_global_t *p_lldp_glb, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_LLDP_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_LLDP_GLOBAL_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_lldp_global_field_value_dump(tbl_lldp_global_t *p_lldp_glb, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_LLDP_GLOBAL_FLD_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_lldp_glb->enable);
    }
    if (FLD_MATCH(TBL_LLDP_GLOBAL_FLD_SYSTEM_MAC_EN, field_id))
    {
        sal_sprintf(str, "%u", p_lldp_glb->system_mac_en);
    }
    if (FLD_MATCH(TBL_LLDP_GLOBAL_FLD_CONFIG_FLAG, field_id))
    {
        sal_sprintf(str, "%u", p_lldp_glb->config_flag);
    }
    if (FLD_MATCH(TBL_LLDP_GLOBAL_FLD_SYSTEM_MAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_lldp_glb->system_mac);
    }
    if (FLD_MATCH(TBL_LLDP_GLOBAL_FLD_MANAGEMENT_IP, field_id))
    {
        cdb_addr_val2str(str, MAX_CMD_STR_LEN, &p_lldp_glb->management_ip);
    }
    if (FLD_MATCH(TBL_LLDP_GLOBAL_FLD_TX_HOLD, field_id))
    {
        sal_sprintf(str, "%u", p_lldp_glb->tx_hold);
    }
    if (FLD_MATCH(TBL_LLDP_GLOBAL_FLD_TX_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_lldp_glb->tx_interval);
    }
    if (FLD_MATCH(TBL_LLDP_GLOBAL_FLD_TX_DELAY, field_id))
    {
        sal_sprintf(str, "%u", p_lldp_glb->tx_delay);
    }
    if (FLD_MATCH(TBL_LLDP_GLOBAL_FLD_REINIT_DELAY, field_id))
    {
        sal_sprintf(str, "%u", p_lldp_glb->reinit_delay);
    }
    if (FLD_MATCH(TBL_LLDP_GLOBAL_FLD_SYSTEM_CAPABILITY, field_id))
    {
        sal_sprintf(str, "%u", p_lldp_glb->system_capability);
    }
    if (FLD_MATCH(TBL_LLDP_GLOBAL_FLD_SYSTEM_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_lldp_glb->system_name);
    }
    if (FLD_MATCH(TBL_LLDP_GLOBAL_FLD_SYSTEM_DESC, field_id))
    {
        sal_sprintf(str, "%s", p_lldp_glb->system_desc);
    }
    if (FLD_MATCH(TBL_LLDP_GLOBAL_FLD_SYSTEM_DESC_DEFAULT, field_id))
    {
        sal_sprintf(str, "%s", p_lldp_glb->system_desc_default);
    }
    return str;
}

char**
tbl_lldp_global_table_dump(tbl_lldp_global_t *p_lldp_glb, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_lldp_global_key_name_dump(p_lldp_glb, buf));
    for(i=1; i<TBL_LLDP_GLOBAL_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_lldp_global_field_name_dump(p_lldp_glb, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_lldp_global_field_value_dump(p_lldp_glb, i, buf));
    }
    return str;
}

int32
tbl_lldp_global_field_value_parser(char *str, int32 field_id, tbl_lldp_global_t *p_lldp_glb)
{
    if (FLD_MATCH(TBL_LLDP_GLOBAL_FLD_ENABLE, field_id))
    {
        int32 ret;
        p_lldp_glb->enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LLDP_GLOBAL_FLD_SYSTEM_MAC_EN, field_id))
    {
        int32 ret;
        p_lldp_glb->system_mac_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LLDP_GLOBAL_FLD_CONFIG_FLAG, field_id))
    {
        int32 ret;
        p_lldp_glb->config_flag = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LLDP_GLOBAL_FLD_SYSTEM_MAC, field_id))
    {
        cdb_mac_addr_str2val(p_lldp_glb->system_mac, str);
    }
    if (FLD_MATCH(TBL_LLDP_GLOBAL_FLD_MANAGEMENT_IP, field_id))
    {
        cdb_addr_str2val(&p_lldp_glb->management_ip, str, 0);
    }
    if (FLD_MATCH(TBL_LLDP_GLOBAL_FLD_TX_HOLD, field_id))
    {
        int32 ret;
        p_lldp_glb->tx_hold = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LLDP_GLOBAL_FLD_TX_INTERVAL, field_id))
    {
        int32 ret;
        p_lldp_glb->tx_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LLDP_GLOBAL_FLD_TX_DELAY, field_id))
    {
        int32 ret;
        p_lldp_glb->tx_delay = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LLDP_GLOBAL_FLD_REINIT_DELAY, field_id))
    {
        int32 ret;
        p_lldp_glb->reinit_delay = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LLDP_GLOBAL_FLD_SYSTEM_CAPABILITY, field_id))
    {
        int32 ret;
        p_lldp_glb->system_capability = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LLDP_GLOBAL_FLD_SYSTEM_NAME, field_id))
    {
        sal_strcpy(p_lldp_glb->system_name, str);
    }
    if (FLD_MATCH(TBL_LLDP_GLOBAL_FLD_SYSTEM_DESC, field_id))
    {
        sal_strcpy(p_lldp_glb->system_desc, str);
    }
    if (FLD_MATCH(TBL_LLDP_GLOBAL_FLD_SYSTEM_DESC_DEFAULT, field_id))
    {
        sal_strcpy(p_lldp_glb->system_desc_default, str);
    }
    return PM_E_NONE;
}

int32
tbl_lldp_global_table_parser(char** array, char* key_value,tbl_lldp_global_t *p_lldp_glb)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_LLDP_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_lldp_global_key_str2val(key_value, p_lldp_glb));

    for(i=1; i<TBL_LLDP_GLOBAL_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_lldp_global_field_value_parser( array[j++], i, p_lldp_glb));
    }

    return PM_E_NONE;
}

int32
tbl_lldp_global_dump_one(tbl_lldp_global_t *p_lldp_glb, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_LLDP_GLOBAL);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_LLDP_GLOBAL_FLD_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LLDP_GLOBAL_FLD_ENABLE].name,
            p_lldp_glb->enable);
    }
    if (FLD_MATCH(TBL_LLDP_GLOBAL_FLD_SYSTEM_MAC_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LLDP_GLOBAL_FLD_SYSTEM_MAC_EN].name,
            p_lldp_glb->system_mac_en);
    }
    if (FLD_MATCH(TBL_LLDP_GLOBAL_FLD_CONFIG_FLAG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LLDP_GLOBAL_FLD_CONFIG_FLAG].name,
            p_lldp_glb->config_flag);
    }
    if (FLD_MATCH(TBL_LLDP_GLOBAL_FLD_SYSTEM_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_LLDP_GLOBAL_FLD_SYSTEM_MAC].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_lldp_glb->system_mac));
    }
    if (FLD_MATCH(TBL_LLDP_GLOBAL_FLD_MANAGEMENT_IP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_LLDP_GLOBAL_FLD_MANAGEMENT_IP].name, 
            cdb_addr_val2str(buf, MAX_CMD_STR_LEN, &p_lldp_glb->management_ip));
    }
    if (FLD_MATCH(TBL_LLDP_GLOBAL_FLD_TX_HOLD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LLDP_GLOBAL_FLD_TX_HOLD].name,
            p_lldp_glb->tx_hold);
    }
    if (FLD_MATCH(TBL_LLDP_GLOBAL_FLD_TX_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LLDP_GLOBAL_FLD_TX_INTERVAL].name,
            p_lldp_glb->tx_interval);
    }
    if (FLD_MATCH(TBL_LLDP_GLOBAL_FLD_TX_DELAY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LLDP_GLOBAL_FLD_TX_DELAY].name,
            p_lldp_glb->tx_delay);
    }
    if (FLD_MATCH(TBL_LLDP_GLOBAL_FLD_REINIT_DELAY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LLDP_GLOBAL_FLD_REINIT_DELAY].name,
            p_lldp_glb->reinit_delay);
    }
    if (FLD_MATCH(TBL_LLDP_GLOBAL_FLD_SYSTEM_CAPABILITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LLDP_GLOBAL_FLD_SYSTEM_CAPABILITY].name,
            p_lldp_glb->system_capability);
    }
    if (FLD_MATCH(TBL_LLDP_GLOBAL_FLD_SYSTEM_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_LLDP_GLOBAL_FLD_SYSTEM_NAME].name,
            p_lldp_glb->system_name);
    }
    if (FLD_MATCH(TBL_LLDP_GLOBAL_FLD_SYSTEM_DESC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_LLDP_GLOBAL_FLD_SYSTEM_DESC].name,
            p_lldp_glb->system_desc);
    }
    if (FLD_MATCH(TBL_LLDP_GLOBAL_FLD_SYSTEM_DESC_DEFAULT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_LLDP_GLOBAL_FLD_SYSTEM_DESC_DEFAULT].name,
            p_lldp_glb->system_desc_default);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_lldp_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_lldp_global_t *p_db_lldp_glb = _g_p_tbl_lldp_global;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_lldp_glb)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_lldp_glb, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_lldp_global_t*
tbl_lldp_global_init_lldp_global()
{
    _g_p_tbl_lldp_global = XCALLOC(MEM_TBL_LLDP_GLOBAL, sizeof(tbl_lldp_global_t));
    return _g_p_tbl_lldp_global;
}

