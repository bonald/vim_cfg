
#include "proto.h"
#include "gen/tbl_card_define.h"
#include "gen/tbl_card.h"
#include "cdb_data_cmp.h"

tbl_card_master_t *_g_p_tbl_card_master = NULL;

int32
tbl_card_add_card_sync(tbl_card_t *p_card, uint32 sync)
{
    tbl_card_master_t *p_master = _g_p_tbl_card_master;
    tbl_card_t *p_db_card = NULL;

    /* 1. lookup entry exist */
    if (tbl_card_get_card(&p_card->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_card = XCALLOC(MEM_TBL_CARD, sizeof(tbl_card_t));
    if (NULL == p_db_card)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_card, p_card, sizeof(tbl_card_t));

    /* 4. add to db */
    p_master->card_array[p_card->key.id] = p_db_card;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_CARD, p_db_card);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_CARD, p_db_card);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_card_del_card_sync(tbl_card_key_t *p_card_key, uint32 sync)
{
    tbl_card_master_t *p_master = _g_p_tbl_card_master;
    tbl_card_t *p_db_card = NULL;

    /* 1. lookup entry exist */
    p_db_card = tbl_card_get_card(p_card_key);
    if (NULL == p_db_card)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_CARD, p_db_card);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_CARD, p_db_card);
        #endif 
    }

    /* 3. delete from db */
    p_master->card_array[p_card_key->id] = NULL;

    /* 4. free db entry */
    XFREE(MEM_TBL_CARD, p_db_card);

    return PM_E_NONE;
}

int32
tbl_card_set_card_field_sync(tbl_card_t *p_card, tbl_card_field_id_t field_id, uint32 sync)
{
    tbl_card_t *p_db_card = NULL;

    /* 1. lookup entry exist */
    p_db_card = tbl_card_get_card(&p_card->key);
    if (NULL == p_db_card)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_CARD_FLD_SLOT:
        p_db_card->slot = p_card->slot;
        break;

    case TBL_CARD_FLD_PRODUCT_NAME:
        sal_memcpy(p_db_card->product_name, p_card->product_name, sizeof(p_card->product_name));
        break;

    case TBL_CARD_FLD_HARDWARE_NAME:
        sal_memcpy(p_db_card->hardware_name, p_card->hardware_name, sizeof(p_card->hardware_name));
        break;

    case TBL_CARD_FLD_COMPANY_NAME:
        sal_memcpy(p_db_card->company_name, p_card->company_name, sizeof(p_card->company_name));
        break;

    case TBL_CARD_FLD_PACKAGE_NAME:
        sal_memcpy(p_db_card->package_name, p_card->package_name, sizeof(p_card->package_name));
        break;

    case TBL_CARD_FLD_ENTERPRISE_OID:
        sal_memcpy(p_db_card->enterprise_oid, p_card->enterprise_oid, sizeof(p_card->enterprise_oid));
        break;

    case TBL_CARD_FLD_HARDWARE_TYPE:
        sal_memcpy(p_db_card->hardware_type, p_card->hardware_type, sizeof(p_card->hardware_type));
        break;

    case TBL_CARD_FLD_SERIAL_NO:
        sal_memcpy(p_db_card->serial_no, p_card->serial_no, sizeof(p_card->serial_no));
        break;

    case TBL_CARD_FLD_BOOTROM_VER:
        sal_memcpy(p_db_card->bootrom_ver, p_card->bootrom_ver, sizeof(p_card->bootrom_ver));
        break;

    case TBL_CARD_FLD_EPLD_VER:
        sal_memcpy(p_db_card->epld_ver, p_card->epld_ver, sizeof(p_card->epld_ver));
        break;

    case TBL_CARD_FLD_SW_VER:
        sal_memcpy(p_db_card->sw_ver, p_card->sw_ver, sizeof(p_card->sw_ver));
        break;

    case TBL_CARD_FLD_HARDWARE_VER:
        sal_memcpy(p_db_card->hardware_ver, p_card->hardware_ver, sizeof(p_card->hardware_ver));
        break;

    case TBL_CARD_FLD_BOARD_TYPE:
        p_db_card->board_type = p_card->board_type;
        break;

    case TBL_CARD_FLD_BOARD_SERIES:
        p_db_card->board_series = p_card->board_series;
        break;

    case TBL_CARD_FLD_FLASH_SIZE:
        p_db_card->flash_size = p_card->flash_size;
        break;

    case TBL_CARD_FLD_DRAM_SIZE:
        p_db_card->dram_size = p_card->dram_size;
        break;

    case TBL_CARD_FLD_PORT_NUM:
        p_db_card->port_num = p_card->port_num;
        break;

    case TBL_CARD_FLD_UPTIME_DAY:
        p_db_card->uptime_day = p_card->uptime_day;
        break;

    case TBL_CARD_FLD_UPTIME_HOUR:
        p_db_card->uptime_hour = p_card->uptime_hour;
        break;

    case TBL_CARD_FLD_UPTIME_MIN:
        p_db_card->uptime_min = p_card->uptime_min;
        break;

    case TBL_CARD_FLD_ATTACH_TIME:
        sal_memcpy(&p_db_card->attach_time, &p_card->attach_time, sizeof(p_card->attach_time));
        break;

    case TBL_CARD_FLD_ATTACH_COUNT:
        p_db_card->attach_count = p_card->attach_count;
        break;

    case TBL_CARD_FLD_TMPR_LOW:
        p_db_card->tmpr_low = p_card->tmpr_low;
        break;

    case TBL_CARD_FLD_TMPR_HIGH:
        p_db_card->tmpr_high = p_card->tmpr_high;
        break;

    case TBL_CARD_FLD_TMPR_CRITICAL:
        p_db_card->tmpr_critical = p_card->tmpr_critical;
        break;

    case TBL_CARD_FLD_STATUS:
        p_db_card->status = p_card->status;
        break;

    case TBL_CARD_FLD_EPLD_STATUS:
        p_db_card->epld_status = p_card->epld_status;
        break;

    case TBL_CARD_FLD_REBOOT:
        p_db_card->reboot = p_card->reboot;
        break;

    case TBL_CARD_FLD_UPDATE_PHY_FIRMWARE_PORT_IDX:
        p_db_card->update_phy_firmware_port_idx = p_card->update_phy_firmware_port_idx;
        break;

    case TBL_CARD_FLD_PLATFORM_TYPE:
        p_db_card->platform_type = p_card->platform_type;
        break;

    case TBL_CARD_FLD_BOOTUP_DIAG_LEVEL:
        p_db_card->bootup_diag_level = p_card->bootup_diag_level;
        break;

    case TBL_CARD_FLD_NEXT_BOOTUP_DIAG_LEVEL:
        p_db_card->next_bootup_diag_level = p_card->next_bootup_diag_level;
        break;

    case TBL_CARD_FLD_FAN_SPEED_BOTTOM:
        p_db_card->fan_speed_bottom = p_card->fan_speed_bottom;
        break;

    case TBL_CARD_FLD_FAN_SPEED_LOW:
        p_db_card->fan_speed_low = p_card->fan_speed_low;
        break;

    case TBL_CARD_FLD_FAN_SPEED_HIGH:
        p_db_card->fan_speed_high = p_card->fan_speed_high;
        break;

    case TBL_CARD_FLD_FAN_SPEED_FULL:
        p_db_card->fan_speed_full = p_card->fan_speed_full;
        break;

    case TBL_CARD_FLD_BOOTCMD:
        sal_memcpy(p_db_card->bootcmd, p_card->bootcmd, sizeof(p_card->bootcmd));
        break;

    case TBL_CARD_FLD_UPDATE_EPLD_NAME:
        sal_memcpy(p_db_card->update_epld_name, p_card->update_epld_name, sizeof(p_card->update_epld_name));
        break;

    case TBL_CARD_FLD_UPDATE_BOOTROM_NAME:
        sal_memcpy(p_db_card->update_bootrom_name, p_card->update_bootrom_name, sizeof(p_card->update_bootrom_name));
        break;

    case TBL_CARD_FLD_UPDATE_PHY_FIRMWARE_NAME:
        sal_memcpy(p_db_card->update_phy_firmware_name, p_card->update_phy_firmware_name, sizeof(p_card->update_phy_firmware_name));
        break;

    case TBL_CARD_FLD_GLB_POE_ENABLE:
        p_db_card->glb_poe_enable = p_card->glb_poe_enable;
        break;

    case TBL_CARD_FLD_GLB_POE_POWER_MAX:
        p_db_card->glb_poe_power_max = p_card->glb_poe_power_max;
        break;

    case TBL_CARD_FLD_MAX:
        sal_memcpy(p_db_card, p_card, sizeof(tbl_card_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_CARD, field_id, p_db_card);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_CARD, field_id, p_db_card);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_card_add_card(tbl_card_t *p_card)
{
    return tbl_card_add_card_sync(p_card, TRUE);
}

int32
tbl_card_del_card(tbl_card_key_t *p_card_key)
{
    return tbl_card_del_card_sync(p_card_key, TRUE);
}

int32
tbl_card_set_card_field(tbl_card_t *p_card, tbl_card_field_id_t field_id)
{
    return tbl_card_set_card_field_sync(p_card, field_id, TRUE);
}

tbl_card_t*
tbl_card_get_card(tbl_card_key_t *p_card_key)
{
    tbl_card_master_t *p_master = _g_p_tbl_card_master;

    if (GLB_SLOT_NUM_MAX <= p_card_key->id)
    {
        return NULL;
    }
    return (p_master->card_array[p_card_key->id]);
}

char*
tbl_card_key_val2str(tbl_card_t *p_card, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_card->key.id);
    return str;
}

int32
tbl_card_key_str2val(char *str, tbl_card_t *p_card)
{
    int32 ret = 0;

    p_card->key.id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_card_key_name_dump(tbl_card_t *p_card, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CARD);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_card->key.id);
    return str;
}

char*
tbl_card_key_value_dump(tbl_card_t *p_card, char *str)
{
    sal_sprintf(str, "%u", p_card->key.id);
    return str;
}

char*
tbl_card_field_name_dump(tbl_card_t *p_card, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_CARD);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_CARD_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_card_field_value_dump(tbl_card_t *p_card, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_CARD_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_card->key.id);
    }
    if (FLD_MATCH(TBL_CARD_FLD_SLOT, field_id))
    {
        sal_sprintf(str, "%u", p_card->slot);
    }
    if (FLD_MATCH(TBL_CARD_FLD_PRODUCT_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_card->product_name);
    }
    if (FLD_MATCH(TBL_CARD_FLD_HARDWARE_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_card->hardware_name);
    }
    if (FLD_MATCH(TBL_CARD_FLD_COMPANY_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_card->company_name);
    }
    if (FLD_MATCH(TBL_CARD_FLD_PACKAGE_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_card->package_name);
    }
    if (FLD_MATCH(TBL_CARD_FLD_ENTERPRISE_OID, field_id))
    {
        sal_sprintf(str, "%s", p_card->enterprise_oid);
    }
    if (FLD_MATCH(TBL_CARD_FLD_HARDWARE_TYPE, field_id))
    {
        sal_sprintf(str, "%s", p_card->hardware_type);
    }
    if (FLD_MATCH(TBL_CARD_FLD_SERIAL_NO, field_id))
    {
        sal_sprintf(str, "%s", p_card->serial_no);
    }
    if (FLD_MATCH(TBL_CARD_FLD_BOOTROM_VER, field_id))
    {
        sal_sprintf(str, "%s", p_card->bootrom_ver);
    }
    if (FLD_MATCH(TBL_CARD_FLD_EPLD_VER, field_id))
    {
        sal_sprintf(str, "%s", p_card->epld_ver);
    }
    if (FLD_MATCH(TBL_CARD_FLD_SW_VER, field_id))
    {
        sal_sprintf(str, "%s", p_card->sw_ver);
    }
    if (FLD_MATCH(TBL_CARD_FLD_HARDWARE_VER, field_id))
    {
        sal_sprintf(str, "%s", p_card->hardware_ver);
    }
    if (FLD_MATCH(TBL_CARD_FLD_BOARD_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_card->board_type);
    }
    if (FLD_MATCH(TBL_CARD_FLD_BOARD_SERIES, field_id))
    {
        sal_sprintf(str, "%u", p_card->board_series);
    }
    if (FLD_MATCH(TBL_CARD_FLD_FLASH_SIZE, field_id))
    {
        sal_sprintf(str, "%d", p_card->flash_size);
    }
    if (FLD_MATCH(TBL_CARD_FLD_DRAM_SIZE, field_id))
    {
        sal_sprintf(str, "%d", p_card->dram_size);
    }
    if (FLD_MATCH(TBL_CARD_FLD_PORT_NUM, field_id))
    {
        sal_sprintf(str, "%d", p_card->port_num);
    }
    if (FLD_MATCH(TBL_CARD_FLD_UPTIME_DAY, field_id))
    {
        sal_sprintf(str, "%d", p_card->uptime_day);
    }
    if (FLD_MATCH(TBL_CARD_FLD_UPTIME_HOUR, field_id))
    {
        sal_sprintf(str, "%d", p_card->uptime_hour);
    }
    if (FLD_MATCH(TBL_CARD_FLD_UPTIME_MIN, field_id))
    {
        sal_sprintf(str, "%d", p_card->uptime_min);
    }
    if (FLD_MATCH(TBL_CARD_FLD_ATTACH_TIME, field_id))
    {
        cdb_sal_time_val2str(str, MAX_CMD_STR_LEN, &p_card->attach_time);
    }
    if (FLD_MATCH(TBL_CARD_FLD_ATTACH_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_card->attach_count);
    }
    if (FLD_MATCH(TBL_CARD_FLD_TMPR_LOW, field_id))
    {
        sal_sprintf(str, "%d", p_card->tmpr_low);
    }
    if (FLD_MATCH(TBL_CARD_FLD_TMPR_HIGH, field_id))
    {
        sal_sprintf(str, "%d", p_card->tmpr_high);
    }
    if (FLD_MATCH(TBL_CARD_FLD_TMPR_CRITICAL, field_id))
    {
        sal_sprintf(str, "%d", p_card->tmpr_critical);
    }
    if (FLD_MATCH(TBL_CARD_FLD_STATUS, field_id))
    {
        sal_sprintf(str, "%u", p_card->status);
    }
    if (FLD_MATCH(TBL_CARD_FLD_EPLD_STATUS, field_id))
    {
        sal_sprintf(str, "%u", p_card->epld_status);
    }
    if (FLD_MATCH(TBL_CARD_FLD_REBOOT, field_id))
    {
        sal_sprintf(str, "%u", p_card->reboot);
    }
    if (FLD_MATCH(TBL_CARD_FLD_UPDATE_PHY_FIRMWARE_PORT_IDX, field_id))
    {
        sal_sprintf(str, "%d", p_card->update_phy_firmware_port_idx);
    }
    if (FLD_MATCH(TBL_CARD_FLD_PLATFORM_TYPE, field_id))
    {
        sal_sprintf(str, "%d", p_card->platform_type);
    }
    if (FLD_MATCH(TBL_CARD_FLD_BOOTUP_DIAG_LEVEL, field_id))
    {
        sal_sprintf(str, "%d", p_card->bootup_diag_level);
    }
    if (FLD_MATCH(TBL_CARD_FLD_NEXT_BOOTUP_DIAG_LEVEL, field_id))
    {
        sal_sprintf(str, "%d", p_card->next_bootup_diag_level);
    }
    if (FLD_MATCH(TBL_CARD_FLD_FAN_SPEED_BOTTOM, field_id))
    {
        sal_sprintf(str, "%d", p_card->fan_speed_bottom);
    }
    if (FLD_MATCH(TBL_CARD_FLD_FAN_SPEED_LOW, field_id))
    {
        sal_sprintf(str, "%d", p_card->fan_speed_low);
    }
    if (FLD_MATCH(TBL_CARD_FLD_FAN_SPEED_HIGH, field_id))
    {
        sal_sprintf(str, "%d", p_card->fan_speed_high);
    }
    if (FLD_MATCH(TBL_CARD_FLD_FAN_SPEED_FULL, field_id))
    {
        sal_sprintf(str, "%d", p_card->fan_speed_full);
    }
    if (FLD_MATCH(TBL_CARD_FLD_BOOTCMD, field_id))
    {
        sal_sprintf(str, "%s", p_card->bootcmd);
    }
    if (FLD_MATCH(TBL_CARD_FLD_UPDATE_EPLD_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_card->update_epld_name);
    }
    if (FLD_MATCH(TBL_CARD_FLD_UPDATE_BOOTROM_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_card->update_bootrom_name);
    }
    if (FLD_MATCH(TBL_CARD_FLD_UPDATE_PHY_FIRMWARE_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_card->update_phy_firmware_name);
    }
    if (FLD_MATCH(TBL_CARD_FLD_GLB_POE_ENABLE, field_id))
    {
        sal_sprintf(str, "%d", p_card->glb_poe_enable);
    }
    if (FLD_MATCH(TBL_CARD_FLD_GLB_POE_POWER_MAX, field_id))
    {
        sal_sprintf(str, "%u", p_card->glb_poe_power_max);
    }
    return str;
}

char**
tbl_card_table_dump(tbl_card_t *p_card, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_card_key_name_dump(p_card, buf));
    for(i=1; i<TBL_CARD_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_card_field_name_dump(p_card, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_card_field_value_dump(p_card, i, buf));
    }
    return str;
}

int32
tbl_card_field_value_parser(char *str, int32 field_id, tbl_card_t *p_card)
{
    if (FLD_MATCH(TBL_CARD_FLD_KEY, field_id))
    {
        int32 ret;
        p_card->key.id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CARD_FLD_SLOT, field_id))
    {
        int32 ret;
        p_card->slot = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CARD_FLD_PRODUCT_NAME, field_id))
    {
        sal_strcpy(p_card->product_name, str);
    }
    if (FLD_MATCH(TBL_CARD_FLD_HARDWARE_NAME, field_id))
    {
        sal_strcpy(p_card->hardware_name, str);
    }
    if (FLD_MATCH(TBL_CARD_FLD_COMPANY_NAME, field_id))
    {
        sal_strcpy(p_card->company_name, str);
    }
    if (FLD_MATCH(TBL_CARD_FLD_PACKAGE_NAME, field_id))
    {
        sal_strcpy(p_card->package_name, str);
    }
    if (FLD_MATCH(TBL_CARD_FLD_ENTERPRISE_OID, field_id))
    {
        sal_strcpy(p_card->enterprise_oid, str);
    }
    if (FLD_MATCH(TBL_CARD_FLD_HARDWARE_TYPE, field_id))
    {
        sal_strcpy(p_card->hardware_type, str);
    }
    if (FLD_MATCH(TBL_CARD_FLD_SERIAL_NO, field_id))
    {
        sal_strcpy(p_card->serial_no, str);
    }
    if (FLD_MATCH(TBL_CARD_FLD_BOOTROM_VER, field_id))
    {
        sal_strcpy(p_card->bootrom_ver, str);
    }
    if (FLD_MATCH(TBL_CARD_FLD_EPLD_VER, field_id))
    {
        sal_strcpy(p_card->epld_ver, str);
    }
    if (FLD_MATCH(TBL_CARD_FLD_SW_VER, field_id))
    {
        sal_strcpy(p_card->sw_ver, str);
    }
    if (FLD_MATCH(TBL_CARD_FLD_HARDWARE_VER, field_id))
    {
        sal_strcpy(p_card->hardware_ver, str);
    }
    if (FLD_MATCH(TBL_CARD_FLD_BOARD_TYPE, field_id))
    {
        int32 ret;
        p_card->board_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CARD_FLD_BOARD_SERIES, field_id))
    {
        int32 ret;
        p_card->board_series = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CARD_FLD_FLASH_SIZE, field_id))
    {
    }
    if (FLD_MATCH(TBL_CARD_FLD_DRAM_SIZE, field_id))
    {
    }
    if (FLD_MATCH(TBL_CARD_FLD_PORT_NUM, field_id))
    {
    }
    if (FLD_MATCH(TBL_CARD_FLD_UPTIME_DAY, field_id))
    {
    }
    if (FLD_MATCH(TBL_CARD_FLD_UPTIME_HOUR, field_id))
    {
    }
    if (FLD_MATCH(TBL_CARD_FLD_UPTIME_MIN, field_id))
    {
    }
    if (FLD_MATCH(TBL_CARD_FLD_ATTACH_TIME, field_id))
    {
        cdb_sal_time_str2val(str, &p_card->attach_time);
    }
    if (FLD_MATCH(TBL_CARD_FLD_ATTACH_COUNT, field_id))
    {
        int32 ret;
        p_card->attach_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CARD_FLD_TMPR_LOW, field_id))
    {
    }
    if (FLD_MATCH(TBL_CARD_FLD_TMPR_HIGH, field_id))
    {
    }
    if (FLD_MATCH(TBL_CARD_FLD_TMPR_CRITICAL, field_id))
    {
    }
    if (FLD_MATCH(TBL_CARD_FLD_STATUS, field_id))
    {
        int32 ret;
        p_card->status = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CARD_FLD_EPLD_STATUS, field_id))
    {
        int32 ret;
        p_card->epld_status = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CARD_FLD_REBOOT, field_id))
    {
        int32 ret;
        p_card->reboot = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CARD_FLD_UPDATE_PHY_FIRMWARE_PORT_IDX, field_id))
    {
    }
    if (FLD_MATCH(TBL_CARD_FLD_PLATFORM_TYPE, field_id))
    {
    }
    if (FLD_MATCH(TBL_CARD_FLD_BOOTUP_DIAG_LEVEL, field_id))
    {
    }
    if (FLD_MATCH(TBL_CARD_FLD_NEXT_BOOTUP_DIAG_LEVEL, field_id))
    {
    }
    if (FLD_MATCH(TBL_CARD_FLD_FAN_SPEED_BOTTOM, field_id))
    {
    }
    if (FLD_MATCH(TBL_CARD_FLD_FAN_SPEED_LOW, field_id))
    {
    }
    if (FLD_MATCH(TBL_CARD_FLD_FAN_SPEED_HIGH, field_id))
    {
    }
    if (FLD_MATCH(TBL_CARD_FLD_FAN_SPEED_FULL, field_id))
    {
    }
    if (FLD_MATCH(TBL_CARD_FLD_BOOTCMD, field_id))
    {
        sal_strcpy(p_card->bootcmd, str);
    }
    if (FLD_MATCH(TBL_CARD_FLD_UPDATE_EPLD_NAME, field_id))
    {
        sal_strcpy(p_card->update_epld_name, str);
    }
    if (FLD_MATCH(TBL_CARD_FLD_UPDATE_BOOTROM_NAME, field_id))
    {
        sal_strcpy(p_card->update_bootrom_name, str);
    }
    if (FLD_MATCH(TBL_CARD_FLD_UPDATE_PHY_FIRMWARE_NAME, field_id))
    {
        sal_strcpy(p_card->update_phy_firmware_name, str);
    }
    if (FLD_MATCH(TBL_CARD_FLD_GLB_POE_ENABLE, field_id))
    {
    }
    if (FLD_MATCH(TBL_CARD_FLD_GLB_POE_POWER_MAX, field_id))
    {
        int32 ret;
        p_card->glb_poe_power_max = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_card_table_parser(char** array, char* key_value,tbl_card_t *p_card)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_CARD);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_card_key_str2val(key_value, p_card));

    for(i=1; i<TBL_CARD_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_card_field_value_parser( array[j++], i, p_card));
    }

    return PM_E_NONE;
}

int32
tbl_card_dump_one(tbl_card_t *p_card, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CARD);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_CARD_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CARD_FLD_KEY].name,
            p_card->key.id);
    }
    if (FLD_MATCH(TBL_CARD_FLD_SLOT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CARD_FLD_SLOT].name,
            p_card->slot);
    }
    if (FLD_MATCH(TBL_CARD_FLD_PRODUCT_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CARD_FLD_PRODUCT_NAME].name,
            p_card->product_name);
    }
    if (FLD_MATCH(TBL_CARD_FLD_HARDWARE_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CARD_FLD_HARDWARE_NAME].name,
            p_card->hardware_name);
    }
    if (FLD_MATCH(TBL_CARD_FLD_COMPANY_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CARD_FLD_COMPANY_NAME].name,
            p_card->company_name);
    }
    if (FLD_MATCH(TBL_CARD_FLD_PACKAGE_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CARD_FLD_PACKAGE_NAME].name,
            p_card->package_name);
    }
    if (FLD_MATCH(TBL_CARD_FLD_ENTERPRISE_OID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CARD_FLD_ENTERPRISE_OID].name,
            p_card->enterprise_oid);
    }
    if (FLD_MATCH(TBL_CARD_FLD_HARDWARE_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CARD_FLD_HARDWARE_TYPE].name,
            p_card->hardware_type);
    }
    if (FLD_MATCH(TBL_CARD_FLD_SERIAL_NO, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CARD_FLD_SERIAL_NO].name,
            p_card->serial_no);
    }
    if (FLD_MATCH(TBL_CARD_FLD_BOOTROM_VER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CARD_FLD_BOOTROM_VER].name,
            p_card->bootrom_ver);
    }
    if (FLD_MATCH(TBL_CARD_FLD_EPLD_VER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CARD_FLD_EPLD_VER].name,
            p_card->epld_ver);
    }
    if (FLD_MATCH(TBL_CARD_FLD_SW_VER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CARD_FLD_SW_VER].name,
            p_card->sw_ver);
    }
    if (FLD_MATCH(TBL_CARD_FLD_HARDWARE_VER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CARD_FLD_HARDWARE_VER].name,
            p_card->hardware_ver);
    }
    if (FLD_MATCH(TBL_CARD_FLD_BOARD_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CARD_FLD_BOARD_TYPE].name,
            p_card->board_type);
    }
    if (FLD_MATCH(TBL_CARD_FLD_BOARD_SERIES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CARD_FLD_BOARD_SERIES].name,
            p_card->board_series);
    }
    if (FLD_MATCH(TBL_CARD_FLD_FLASH_SIZE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_CARD_FLD_FLASH_SIZE].name,
            p_card->flash_size);
    }
    if (FLD_MATCH(TBL_CARD_FLD_DRAM_SIZE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_CARD_FLD_DRAM_SIZE].name,
            p_card->dram_size);
    }
    if (FLD_MATCH(TBL_CARD_FLD_PORT_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_CARD_FLD_PORT_NUM].name,
            p_card->port_num);
    }
    if (FLD_MATCH(TBL_CARD_FLD_UPTIME_DAY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_CARD_FLD_UPTIME_DAY].name,
            p_card->uptime_day);
    }
    if (FLD_MATCH(TBL_CARD_FLD_UPTIME_HOUR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_CARD_FLD_UPTIME_HOUR].name,
            p_card->uptime_hour);
    }
    if (FLD_MATCH(TBL_CARD_FLD_UPTIME_MIN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_CARD_FLD_UPTIME_MIN].name,
            p_card->uptime_min);
    }
    if (FLD_MATCH(TBL_CARD_FLD_ATTACH_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CARD_FLD_ATTACH_TIME].name, 
            cdb_sal_time_val2str(buf, MAX_CMD_STR_LEN, &p_card->attach_time));
    }
    if (FLD_MATCH(TBL_CARD_FLD_ATTACH_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CARD_FLD_ATTACH_COUNT].name,
            p_card->attach_count);
    }
    if (FLD_MATCH(TBL_CARD_FLD_TMPR_LOW, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_CARD_FLD_TMPR_LOW].name,
            p_card->tmpr_low);
    }
    if (FLD_MATCH(TBL_CARD_FLD_TMPR_HIGH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_CARD_FLD_TMPR_HIGH].name,
            p_card->tmpr_high);
    }
    if (FLD_MATCH(TBL_CARD_FLD_TMPR_CRITICAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_CARD_FLD_TMPR_CRITICAL].name,
            p_card->tmpr_critical);
    }
    if (FLD_MATCH(TBL_CARD_FLD_STATUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CARD_FLD_STATUS].name,
            p_card->status);
    }
    if (FLD_MATCH(TBL_CARD_FLD_EPLD_STATUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CARD_FLD_EPLD_STATUS].name,
            p_card->epld_status);
    }
    if (FLD_MATCH(TBL_CARD_FLD_REBOOT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CARD_FLD_REBOOT].name,
            p_card->reboot);
    }
    if (FLD_MATCH(TBL_CARD_FLD_UPDATE_PHY_FIRMWARE_PORT_IDX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_CARD_FLD_UPDATE_PHY_FIRMWARE_PORT_IDX].name,
            p_card->update_phy_firmware_port_idx);
    }
    if (FLD_MATCH(TBL_CARD_FLD_PLATFORM_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_CARD_FLD_PLATFORM_TYPE].name,
            p_card->platform_type);
    }
    if (FLD_MATCH(TBL_CARD_FLD_BOOTUP_DIAG_LEVEL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_CARD_FLD_BOOTUP_DIAG_LEVEL].name,
            p_card->bootup_diag_level);
    }
    if (FLD_MATCH(TBL_CARD_FLD_NEXT_BOOTUP_DIAG_LEVEL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_CARD_FLD_NEXT_BOOTUP_DIAG_LEVEL].name,
            p_card->next_bootup_diag_level);
    }
    if (FLD_MATCH(TBL_CARD_FLD_FAN_SPEED_BOTTOM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_CARD_FLD_FAN_SPEED_BOTTOM].name,
            p_card->fan_speed_bottom);
    }
    if (FLD_MATCH(TBL_CARD_FLD_FAN_SPEED_LOW, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_CARD_FLD_FAN_SPEED_LOW].name,
            p_card->fan_speed_low);
    }
    if (FLD_MATCH(TBL_CARD_FLD_FAN_SPEED_HIGH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_CARD_FLD_FAN_SPEED_HIGH].name,
            p_card->fan_speed_high);
    }
    if (FLD_MATCH(TBL_CARD_FLD_FAN_SPEED_FULL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_CARD_FLD_FAN_SPEED_FULL].name,
            p_card->fan_speed_full);
    }
    if (FLD_MATCH(TBL_CARD_FLD_BOOTCMD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CARD_FLD_BOOTCMD].name,
            p_card->bootcmd);
    }
    if (FLD_MATCH(TBL_CARD_FLD_UPDATE_EPLD_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CARD_FLD_UPDATE_EPLD_NAME].name,
            p_card->update_epld_name);
    }
    if (FLD_MATCH(TBL_CARD_FLD_UPDATE_BOOTROM_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CARD_FLD_UPDATE_BOOTROM_NAME].name,
            p_card->update_bootrom_name);
    }
    if (FLD_MATCH(TBL_CARD_FLD_UPDATE_PHY_FIRMWARE_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CARD_FLD_UPDATE_PHY_FIRMWARE_NAME].name,
            p_card->update_phy_firmware_name);
    }
    if (FLD_MATCH(TBL_CARD_FLD_GLB_POE_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_CARD_FLD_GLB_POE_ENABLE].name,
            p_card->glb_poe_enable);
    }
    if (FLD_MATCH(TBL_CARD_FLD_GLB_POE_POWER_MAX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CARD_FLD_GLB_POE_POWER_MAX].name,
            p_card->glb_poe_power_max);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_card_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_card_master_t *p_master = _g_p_tbl_card_master;
    tbl_card_t *p_db_card = NULL;
    uint32 id = 0;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    for (id = 0; id < GLB_SLOT_NUM_MAX; id++)
    {
        p_db_card = p_master->card_array[id];
        if (NULL == p_db_card)
        {
            continue;
        }
        rc |= fn(p_db_card, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_card_master_t*
tbl_card_get_master()
{
    return _g_p_tbl_card_master;
}

tbl_card_master_t*
tbl_card_init_card()
{
    _g_p_tbl_card_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_card_master_t));
    return _g_p_tbl_card_master;
}

tbl_card_t*
tbl_card_get_default_card()
{
    tbl_card_master_t *p_master = _g_p_tbl_card_master;

    return (p_master->card_array[0]);
}

