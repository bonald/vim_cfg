
#include "proto.h"
#include "gen/tbl_fiber_define.h"
#include "gen/tbl_fiber.h"
#include "cdb_data_cmp.h"

tbl_fiber_master_t *_g_p_tbl_fiber_master = NULL;

int32
tbl_fiber_add_fiber_sync(tbl_fiber_t *p_fiber, uint32 sync)
{
    tbl_fiber_master_t *p_master = _g_p_tbl_fiber_master;
    tbl_fiber_t *p_db_fiber = NULL;

    /* 1. lookup entry exist */
    if (tbl_fiber_get_fiber(&p_fiber->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_fiber = XCALLOC(MEM_TBL_FIBER, sizeof(tbl_fiber_t));
    if (NULL == p_db_fiber)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_fiber, p_fiber, sizeof(tbl_fiber_t));

    /* 4. add to db */
    p_master->fiber[p_fiber->key.port_id] = p_db_fiber;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_FIBER, p_db_fiber);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_FIBER, p_db_fiber);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_fiber_del_fiber_sync(tbl_fiber_key_t *p_fiber_key, uint32 sync)
{
    tbl_fiber_master_t *p_master = _g_p_tbl_fiber_master;
    tbl_fiber_t *p_db_fiber = NULL;

    /* 1. lookup entry exist */
    p_db_fiber = tbl_fiber_get_fiber(p_fiber_key);
    if (NULL == p_db_fiber)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_FIBER, p_db_fiber);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_FIBER, p_db_fiber);
        #endif 
    }

    /* 3. delete from db */
    p_master->fiber[p_fiber_key->port_id] = NULL;

    /* 4. free db entry */
    XFREE(MEM_TBL_FIBER, p_db_fiber);

    return PM_E_NONE;
}

int32
tbl_fiber_set_fiber_field_sync(tbl_fiber_t *p_fiber, tbl_fiber_field_id_t field_id, uint32 sync)
{
    tbl_fiber_t *p_db_fiber = NULL;

    /* 1. lookup entry exist */
    p_db_fiber = tbl_fiber_get_fiber(&p_fiber->key);
    if (NULL == p_db_fiber)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_FIBER_FLD_INTERFACE_NAME:
        sal_memcpy(p_db_fiber->interface_name, p_fiber->interface_name, sizeof(p_fiber->interface_name));
        break;

    case TBL_FIBER_FLD_FIBER_TYPE_NAME:
        sal_memcpy(p_db_fiber->fiber_type_name, p_fiber->fiber_type_name, sizeof(p_fiber->fiber_type_name));
        break;

    case TBL_FIBER_FLD_NAME:
        sal_memcpy(p_db_fiber->name, p_fiber->name, sizeof(p_fiber->name));
        break;

    case TBL_FIBER_FLD_OUI:
        sal_memcpy(p_db_fiber->oui, p_fiber->oui, sizeof(p_fiber->oui));
        break;

    case TBL_FIBER_FLD_PN:
        sal_memcpy(p_db_fiber->pn, p_fiber->pn, sizeof(p_fiber->pn));
        break;

    case TBL_FIBER_FLD_REV:
        sal_memcpy(p_db_fiber->rev, p_fiber->rev, sizeof(p_fiber->rev));
        break;

    case TBL_FIBER_FLD_SN:
        sal_memcpy(p_db_fiber->sn, p_fiber->sn, sizeof(p_fiber->sn));
        break;

    case TBL_FIBER_FLD_COMPLIANCE_CODE:
        sal_memcpy(p_db_fiber->compliance_code, p_fiber->compliance_code, sizeof(p_fiber->compliance_code));
        break;

    case TBL_FIBER_FLD_SLOT:
        p_db_fiber->slot = p_fiber->slot;
        break;

    case TBL_FIBER_FLD_TMPR:
        sal_memcpy(p_db_fiber->tmpr, p_fiber->tmpr, sizeof(p_fiber->tmpr));
        break;

    case TBL_FIBER_FLD_VOLTAGE:
        sal_memcpy(p_db_fiber->voltage, p_fiber->voltage, sizeof(p_fiber->voltage));
        break;

    case TBL_FIBER_FLD_BIAS:
        sal_memcpy(p_db_fiber->bias, p_fiber->bias, sizeof(p_fiber->bias));
        break;

    case TBL_FIBER_FLD_TX_PWR:
        sal_memcpy(p_db_fiber->tx_pwr, p_fiber->tx_pwr, sizeof(p_fiber->tx_pwr));
        break;

    case TBL_FIBER_FLD_RX_PWR:
        sal_memcpy(p_db_fiber->rx_pwr, p_fiber->rx_pwr, sizeof(p_fiber->rx_pwr));
        break;

    case TBL_FIBER_FLD_TMPR2:
        sal_memcpy(p_db_fiber->tmpr2, p_fiber->tmpr2, sizeof(p_fiber->tmpr2));
        break;

    case TBL_FIBER_FLD_VOLTAGE2:
        sal_memcpy(p_db_fiber->voltage2, p_fiber->voltage2, sizeof(p_fiber->voltage2));
        break;

    case TBL_FIBER_FLD_BIAS2:
        sal_memcpy(p_db_fiber->bias2, p_fiber->bias2, sizeof(p_fiber->bias2));
        break;

    case TBL_FIBER_FLD_TX_PWR2:
        sal_memcpy(p_db_fiber->tx_pwr2, p_fiber->tx_pwr2, sizeof(p_fiber->tx_pwr2));
        break;

    case TBL_FIBER_FLD_RX_PWR2:
        sal_memcpy(p_db_fiber->rx_pwr2, p_fiber->rx_pwr2, sizeof(p_fiber->rx_pwr2));
        break;

    case TBL_FIBER_FLD_ALARM_FLAG:
        sal_memcpy(p_db_fiber->alarm_flag, p_fiber->alarm_flag, sizeof(p_fiber->alarm_flag));
        break;

    case TBL_FIBER_FLD_WARN_FLAG:
        sal_memcpy(p_db_fiber->warn_flag, p_fiber->warn_flag, sizeof(p_fiber->warn_flag));
        break;

    case TBL_FIBER_FLD_UPDATE_CNT:
        p_db_fiber->update_cnt = p_fiber->update_cnt;
        break;

    case TBL_FIBER_FLD_CHANNEL_NUM:
        p_db_fiber->channel_num = p_fiber->channel_num;
        break;

    case TBL_FIBER_FLD_CHANNEL_IDX:
        p_db_fiber->channel_idx = p_fiber->channel_idx;
        break;

    case TBL_FIBER_FLD_IS_DETAIL:
        p_db_fiber->is_detail = p_fiber->is_detail;
        break;

    case TBL_FIBER_FLD_FIBER_TYPE:
        p_db_fiber->fiber_type = p_fiber->fiber_type;
        break;

    case TBL_FIBER_FLD_FIBER_FLAG:
        p_db_fiber->fiber_flag = p_fiber->fiber_flag;
        break;

    case TBL_FIBER_FLD_IS_COPPER:
        p_db_fiber->is_copper = p_fiber->is_copper;
        break;

    case TBL_FIBER_FLD_SPEED:
        p_db_fiber->speed = p_fiber->speed;
        break;

    case TBL_FIBER_FLD_DDM_SUPPORT:
        p_db_fiber->ddm_support = p_fiber->ddm_support;
        break;

    case TBL_FIBER_FLD_EXTERNALLY_CALIBRATED:
        p_db_fiber->externally_calibrated = p_fiber->externally_calibrated;
        break;

    case TBL_FIBER_FLD_WAVELENGTH:
        sal_memcpy(p_db_fiber->wavelength, p_fiber->wavelength, sizeof(p_fiber->wavelength));
        break;

    case TBL_FIBER_FLD_LENGTH:
        sal_memcpy(p_db_fiber->length, p_fiber->length, sizeof(p_fiber->length));
        break;

    case TBL_FIBER_FLD_MAX:
        sal_memcpy(p_db_fiber, p_fiber, sizeof(tbl_fiber_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_FIBER, field_id, p_db_fiber);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_FIBER, field_id, p_db_fiber);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_fiber_add_fiber(tbl_fiber_t *p_fiber)
{
    return tbl_fiber_add_fiber_sync(p_fiber, TRUE);
}

int32
tbl_fiber_del_fiber(tbl_fiber_key_t *p_fiber_key)
{
    return tbl_fiber_del_fiber_sync(p_fiber_key, TRUE);
}

int32
tbl_fiber_set_fiber_field(tbl_fiber_t *p_fiber, tbl_fiber_field_id_t field_id)
{
    return tbl_fiber_set_fiber_field_sync(p_fiber, field_id, TRUE);
}

tbl_fiber_t*
tbl_fiber_get_fiber(tbl_fiber_key_t *p_fiber_key)
{
    tbl_fiber_master_t *p_master = _g_p_tbl_fiber_master;

    if (GLB_CHSM_PORT_NUM_MAX <= p_fiber_key->port_id)
    {
        return NULL;
    }
    return (p_master->fiber[p_fiber_key->port_id]);
}

char*
tbl_fiber_key_val2str(tbl_fiber_t *p_fiber, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%d", p_fiber->key.port_id);
    return str;
}

int32
tbl_fiber_key_str2val(char *str, tbl_fiber_t *p_fiber)
{
    int32 ret = 0;

    p_fiber->key.port_id = cdb_int_str2val(str, &ret);
    return ret;
}

char*
tbl_fiber_key_name_dump(tbl_fiber_t *p_fiber, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_FIBER);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
        "%d", p_node->name, p_fiber->key.port_id);
    return str;
}

char*
tbl_fiber_key_value_dump(tbl_fiber_t *p_fiber, char *str)
{
    sal_sprintf(str, "%d", p_fiber->key.port_id);
    return str;
}

char*
tbl_fiber_field_name_dump(tbl_fiber_t *p_fiber, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_FIBER);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_FIBER_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_fiber_field_value_dump(tbl_fiber_t *p_fiber, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_FIBER_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%d", p_fiber->key.port_id);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_INTERFACE_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_fiber->interface_name);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_FIBER_TYPE_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_fiber->fiber_type_name);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_fiber->name);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_OUI, field_id))
    {
        cdb_uint8_array_val2str(str, MAX_CMD_STR_LEN, p_fiber->oui, sizeof(p_fiber->oui));
    }
    if (FLD_MATCH(TBL_FIBER_FLD_PN, field_id))
    {
        sal_sprintf(str, "%s", p_fiber->pn);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_REV, field_id))
    {
        sal_sprintf(str, "%s", p_fiber->rev);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_SN, field_id))
    {
        sal_sprintf(str, "%s", p_fiber->sn);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_COMPLIANCE_CODE, field_id))
    {
        cdb_uint8_array_val2str(str, MAX_CMD_STR_LEN, p_fiber->compliance_code, sizeof(p_fiber->compliance_code));
    }
    if (FLD_MATCH(TBL_FIBER_FLD_SLOT, field_id))
    {
        sal_sprintf(str, "%d", p_fiber->slot);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_TMPR, field_id))
    {
        cdb_double_array_val2str(str, MAX_CMD_STR_LEN, p_fiber->tmpr, sizeof(p_fiber->tmpr)/sizeof(double));
    }
    if (FLD_MATCH(TBL_FIBER_FLD_VOLTAGE, field_id))
    {
        cdb_double_array_val2str(str, MAX_CMD_STR_LEN, p_fiber->voltage, sizeof(p_fiber->voltage)/sizeof(double));
    }
    if (FLD_MATCH(TBL_FIBER_FLD_BIAS, field_id))
    {
        cdb_double_array_val2str(str, MAX_CMD_STR_LEN, p_fiber->bias, sizeof(p_fiber->bias)/sizeof(double));
    }
    if (FLD_MATCH(TBL_FIBER_FLD_TX_PWR, field_id))
    {
        cdb_double_array_val2str(str, MAX_CMD_STR_LEN, p_fiber->tx_pwr, sizeof(p_fiber->tx_pwr)/sizeof(double));
    }
    if (FLD_MATCH(TBL_FIBER_FLD_RX_PWR, field_id))
    {
        cdb_double_array_val2str(str, MAX_CMD_STR_LEN, p_fiber->rx_pwr, sizeof(p_fiber->rx_pwr)/sizeof(double));
    }
    if (FLD_MATCH(TBL_FIBER_FLD_TMPR2, field_id))
    {
        cdb_double_array_val2str(str, MAX_CMD_STR_LEN, p_fiber->tmpr2, sizeof(p_fiber->tmpr2)/sizeof(double));
    }
    if (FLD_MATCH(TBL_FIBER_FLD_VOLTAGE2, field_id))
    {
        cdb_double_array_val2str(str, MAX_CMD_STR_LEN, p_fiber->voltage2, sizeof(p_fiber->voltage2)/sizeof(double));
    }
    if (FLD_MATCH(TBL_FIBER_FLD_BIAS2, field_id))
    {
        cdb_double_array_val2str(str, MAX_CMD_STR_LEN, p_fiber->bias2, sizeof(p_fiber->bias2)/sizeof(double));
    }
    if (FLD_MATCH(TBL_FIBER_FLD_TX_PWR2, field_id))
    {
        cdb_double_array_val2str(str, MAX_CMD_STR_LEN, p_fiber->tx_pwr2, sizeof(p_fiber->tx_pwr2)/sizeof(double));
    }
    if (FLD_MATCH(TBL_FIBER_FLD_RX_PWR2, field_id))
    {
        cdb_double_array_val2str(str, MAX_CMD_STR_LEN, p_fiber->rx_pwr2, sizeof(p_fiber->rx_pwr2)/sizeof(double));
    }
    if (FLD_MATCH(TBL_FIBER_FLD_ALARM_FLAG, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_fiber->alarm_flag, sizeof(p_fiber->alarm_flag)/4);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_WARN_FLAG, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_fiber->warn_flag, sizeof(p_fiber->warn_flag)/4);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_UPDATE_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_fiber->update_cnt);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_CHANNEL_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_fiber->channel_num);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_CHANNEL_IDX, field_id))
    {
        sal_sprintf(str, "%u", p_fiber->channel_idx);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_IS_DETAIL, field_id))
    {
        sal_sprintf(str, "%u", p_fiber->is_detail);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_FIBER_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_fiber->fiber_type);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_FIBER_FLAG, field_id))
    {
        sal_sprintf(str, "%u", p_fiber->fiber_flag);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_IS_COPPER, field_id))
    {
        sal_sprintf(str, "%u", p_fiber->is_copper);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_SPEED, field_id))
    {
        sal_sprintf(str, "%u", p_fiber->speed);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_DDM_SUPPORT, field_id))
    {
        sal_sprintf(str, "%u", p_fiber->ddm_support);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_EXTERNALLY_CALIBRATED, field_id))
    {
        sal_sprintf(str, "%u", p_fiber->externally_calibrated);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_WAVELENGTH, field_id))
    {
        cdb_uint8_array_val2str(str, MAX_CMD_STR_LEN, p_fiber->wavelength, sizeof(p_fiber->wavelength));
    }
    if (FLD_MATCH(TBL_FIBER_FLD_LENGTH, field_id))
    {
        cdb_uint8_array_val2str(str, MAX_CMD_STR_LEN, p_fiber->length, sizeof(p_fiber->length));
    }
    return str;
}

char**
tbl_fiber_table_dump(tbl_fiber_t *p_fiber, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_fiber_key_name_dump(p_fiber, buf));
    for(i=1; i<TBL_FIBER_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_fiber_field_name_dump(p_fiber, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_fiber_field_value_dump(p_fiber, i, buf));
    }
    return str;
}

int32
tbl_fiber_field_value_parser(char *str, int32 field_id, tbl_fiber_t *p_fiber)
{
    if (FLD_MATCH(TBL_FIBER_FLD_KEY, field_id))
    {
    }
    if (FLD_MATCH(TBL_FIBER_FLD_INTERFACE_NAME, field_id))
    {
        sal_strcpy(p_fiber->interface_name, str);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_FIBER_TYPE_NAME, field_id))
    {
        sal_strcpy(p_fiber->fiber_type_name, str);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_NAME, field_id))
    {
        sal_strcpy(p_fiber->name, str);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_OUI, field_id))
    {
        cdb_uint8_array_str2val(str, p_fiber->oui);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_PN, field_id))
    {
        sal_strcpy(p_fiber->pn, str);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_REV, field_id))
    {
        sal_strcpy(p_fiber->rev, str);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_SN, field_id))
    {
        sal_strcpy(p_fiber->sn, str);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_COMPLIANCE_CODE, field_id))
    {
        cdb_uint8_array_str2val(str, p_fiber->compliance_code);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_SLOT, field_id))
    {
    }
    if (FLD_MATCH(TBL_FIBER_FLD_TMPR, field_id))
    {
        cdb_double_array_str2val(str, p_fiber->tmpr);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_VOLTAGE, field_id))
    {
        cdb_double_array_str2val(str, p_fiber->voltage);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_BIAS, field_id))
    {
        cdb_double_array_str2val(str, p_fiber->bias);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_TX_PWR, field_id))
    {
        cdb_double_array_str2val(str, p_fiber->tx_pwr);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_RX_PWR, field_id))
    {
        cdb_double_array_str2val(str, p_fiber->rx_pwr);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_TMPR2, field_id))
    {
        cdb_double_array_str2val(str, p_fiber->tmpr2);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_VOLTAGE2, field_id))
    {
        cdb_double_array_str2val(str, p_fiber->voltage2);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_BIAS2, field_id))
    {
        cdb_double_array_str2val(str, p_fiber->bias2);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_TX_PWR2, field_id))
    {
        cdb_double_array_str2val(str, p_fiber->tx_pwr2);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_RX_PWR2, field_id))
    {
        cdb_double_array_str2val(str, p_fiber->rx_pwr2);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_ALARM_FLAG, field_id))
    {
        cdb_uint32_array_str2val(str, p_fiber->alarm_flag);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_WARN_FLAG, field_id))
    {
        cdb_uint32_array_str2val(str, p_fiber->warn_flag);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_UPDATE_CNT, field_id))
    {
        int32 ret;
        p_fiber->update_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_CHANNEL_NUM, field_id))
    {
        int32 ret;
        p_fiber->channel_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_CHANNEL_IDX, field_id))
    {
        int32 ret;
        p_fiber->channel_idx = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_IS_DETAIL, field_id))
    {
        int32 ret;
        p_fiber->is_detail = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_FIBER_TYPE, field_id))
    {
        int32 ret;
        p_fiber->fiber_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_FIBER_FLAG, field_id))
    {
        int32 ret;
        p_fiber->fiber_flag = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_IS_COPPER, field_id))
    {
        int32 ret;
        p_fiber->is_copper = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_SPEED, field_id))
    {
        int32 ret;
        p_fiber->speed = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_DDM_SUPPORT, field_id))
    {
        int32 ret;
        p_fiber->ddm_support = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_EXTERNALLY_CALIBRATED, field_id))
    {
        int32 ret;
        p_fiber->externally_calibrated = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_WAVELENGTH, field_id))
    {
        cdb_uint8_array_str2val(str, p_fiber->wavelength);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_LENGTH, field_id))
    {
        cdb_uint8_array_str2val(str, p_fiber->length);
    }
    return PM_E_NONE;
}

int32
tbl_fiber_table_parser(char** array, char* key_value,tbl_fiber_t *p_fiber)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_FIBER);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_fiber_key_str2val(key_value, p_fiber));

    for(i=1; i<TBL_FIBER_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_fiber_field_value_parser( array[j++], i, p_fiber));
    }

    return PM_E_NONE;
}

int32
tbl_fiber_dump_one(tbl_fiber_t *p_fiber, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_FIBER);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_FIBER_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_FIBER_FLD_KEY].name,
            p_fiber->key.port_id);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_INTERFACE_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_FIBER_FLD_INTERFACE_NAME].name,
            p_fiber->interface_name);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_FIBER_TYPE_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_FIBER_FLD_FIBER_TYPE_NAME].name,
            p_fiber->fiber_type_name);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_FIBER_FLD_NAME].name,
            p_fiber->name);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_OUI, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_FIBER_FLD_OUI].name, 
            cdb_uint8_array_val2str(buf, MAX_CMD_STR_LEN, p_fiber->oui, sizeof(p_fiber->oui)));
    }
    if (FLD_MATCH(TBL_FIBER_FLD_PN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_FIBER_FLD_PN].name,
            p_fiber->pn);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_REV, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_FIBER_FLD_REV].name,
            p_fiber->rev);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_SN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_FIBER_FLD_SN].name,
            p_fiber->sn);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_COMPLIANCE_CODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_FIBER_FLD_COMPLIANCE_CODE].name, 
            cdb_uint8_array_val2str(buf, MAX_CMD_STR_LEN, p_fiber->compliance_code, sizeof(p_fiber->compliance_code)));
    }
    if (FLD_MATCH(TBL_FIBER_FLD_SLOT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_FIBER_FLD_SLOT].name,
            p_fiber->slot);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_TMPR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_FIBER_FLD_TMPR].name, 
            cdb_double_array_val2str(buf, MAX_CMD_STR_LEN, p_fiber->tmpr, sizeof(p_fiber->tmpr)/sizeof(double)));
    }
    if (FLD_MATCH(TBL_FIBER_FLD_VOLTAGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_FIBER_FLD_VOLTAGE].name, 
            cdb_double_array_val2str(buf, MAX_CMD_STR_LEN, p_fiber->voltage, sizeof(p_fiber->voltage)/sizeof(double)));
    }
    if (FLD_MATCH(TBL_FIBER_FLD_BIAS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_FIBER_FLD_BIAS].name, 
            cdb_double_array_val2str(buf, MAX_CMD_STR_LEN, p_fiber->bias, sizeof(p_fiber->bias)/sizeof(double)));
    }
    if (FLD_MATCH(TBL_FIBER_FLD_TX_PWR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_FIBER_FLD_TX_PWR].name, 
            cdb_double_array_val2str(buf, MAX_CMD_STR_LEN, p_fiber->tx_pwr, sizeof(p_fiber->tx_pwr)/sizeof(double)));
    }
    if (FLD_MATCH(TBL_FIBER_FLD_RX_PWR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_FIBER_FLD_RX_PWR].name, 
            cdb_double_array_val2str(buf, MAX_CMD_STR_LEN, p_fiber->rx_pwr, sizeof(p_fiber->rx_pwr)/sizeof(double)));
    }
    if (FLD_MATCH(TBL_FIBER_FLD_TMPR2, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_FIBER_FLD_TMPR2].name, 
            cdb_double_array_val2str(buf, MAX_CMD_STR_LEN, p_fiber->tmpr2, sizeof(p_fiber->tmpr2)/sizeof(double)));
    }
    if (FLD_MATCH(TBL_FIBER_FLD_VOLTAGE2, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_FIBER_FLD_VOLTAGE2].name, 
            cdb_double_array_val2str(buf, MAX_CMD_STR_LEN, p_fiber->voltage2, sizeof(p_fiber->voltage2)/sizeof(double)));
    }
    if (FLD_MATCH(TBL_FIBER_FLD_BIAS2, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_FIBER_FLD_BIAS2].name, 
            cdb_double_array_val2str(buf, MAX_CMD_STR_LEN, p_fiber->bias2, sizeof(p_fiber->bias2)/sizeof(double)));
    }
    if (FLD_MATCH(TBL_FIBER_FLD_TX_PWR2, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_FIBER_FLD_TX_PWR2].name, 
            cdb_double_array_val2str(buf, MAX_CMD_STR_LEN, p_fiber->tx_pwr2, sizeof(p_fiber->tx_pwr2)/sizeof(double)));
    }
    if (FLD_MATCH(TBL_FIBER_FLD_RX_PWR2, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_FIBER_FLD_RX_PWR2].name, 
            cdb_double_array_val2str(buf, MAX_CMD_STR_LEN, p_fiber->rx_pwr2, sizeof(p_fiber->rx_pwr2)/sizeof(double)));
    }
    if (FLD_MATCH(TBL_FIBER_FLD_ALARM_FLAG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_FIBER_FLD_ALARM_FLAG].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_fiber->alarm_flag, sizeof(p_fiber->alarm_flag)/4));
    }
    if (FLD_MATCH(TBL_FIBER_FLD_WARN_FLAG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_FIBER_FLD_WARN_FLAG].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_fiber->warn_flag, sizeof(p_fiber->warn_flag)/4));
    }
    if (FLD_MATCH(TBL_FIBER_FLD_UPDATE_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FIBER_FLD_UPDATE_CNT].name,
            p_fiber->update_cnt);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_CHANNEL_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FIBER_FLD_CHANNEL_NUM].name,
            p_fiber->channel_num);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_CHANNEL_IDX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FIBER_FLD_CHANNEL_IDX].name,
            p_fiber->channel_idx);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_IS_DETAIL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FIBER_FLD_IS_DETAIL].name,
            p_fiber->is_detail);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_FIBER_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FIBER_FLD_FIBER_TYPE].name,
            p_fiber->fiber_type);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_FIBER_FLAG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FIBER_FLD_FIBER_FLAG].name,
            p_fiber->fiber_flag);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_IS_COPPER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FIBER_FLD_IS_COPPER].name,
            p_fiber->is_copper);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_SPEED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FIBER_FLD_SPEED].name,
            p_fiber->speed);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_DDM_SUPPORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FIBER_FLD_DDM_SUPPORT].name,
            p_fiber->ddm_support);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_EXTERNALLY_CALIBRATED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FIBER_FLD_EXTERNALLY_CALIBRATED].name,
            p_fiber->externally_calibrated);
    }
    if (FLD_MATCH(TBL_FIBER_FLD_WAVELENGTH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_FIBER_FLD_WAVELENGTH].name, 
            cdb_uint8_array_val2str(buf, MAX_CMD_STR_LEN, p_fiber->wavelength, sizeof(p_fiber->wavelength)));
    }
    if (FLD_MATCH(TBL_FIBER_FLD_LENGTH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_FIBER_FLD_LENGTH].name, 
            cdb_uint8_array_val2str(buf, MAX_CMD_STR_LEN, p_fiber->length, sizeof(p_fiber->length)));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_fiber_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_fiber_master_t *p_master = _g_p_tbl_fiber_master;
    tbl_fiber_t *p_db_fiber = NULL;
    uint32 port_id = 0;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    for (port_id = 0; port_id < GLB_CHSM_PORT_NUM_MAX; port_id++)
    {
        p_db_fiber = p_master->fiber[port_id];
        if (NULL == p_db_fiber)
        {
            continue;
        }
        rc |= fn(p_db_fiber, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_fiber_master_t*
tbl_fiber_get_master()
{
    return _g_p_tbl_fiber_master;
}

tbl_fiber_master_t*
tbl_fiber_init_fiber()
{
    _g_p_tbl_fiber_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_fiber_master_t));
    return _g_p_tbl_fiber_master;
}

