
#include "proto.h"
#include "gen/tbl_port_define.h"
#include "gen/tbl_port.h"
#include "cdb_data_cmp.h"

tbl_port_master_t *_g_p_tbl_port_master = NULL;

int32
tbl_port_add_port_sync(tbl_port_t *p_port, uint32 sync)
{
    tbl_port_master_t *p_master = _g_p_tbl_port_master;
    tbl_port_t *p_db_port = NULL;

    /* 1. lookup entry exist */
    if (tbl_port_get_port(&p_port->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_port = XCALLOC(MEM_TBL_PORT, sizeof(tbl_port_t));
    if (NULL == p_db_port)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_port, p_port, sizeof(tbl_port_t));

    /* 4. add to db */
    p_master->port_array[p_port->key.port_id] = p_db_port;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_PORT, p_db_port);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_PORT, p_db_port);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_port_del_port_sync(tbl_port_key_t *p_port_key, uint32 sync)
{
    tbl_port_master_t *p_master = _g_p_tbl_port_master;
    tbl_port_t *p_db_port = NULL;

    /* 1. lookup entry exist */
    p_db_port = tbl_port_get_port(p_port_key);
    if (NULL == p_db_port)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_PORT, p_db_port);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_PORT, p_db_port);
        #endif 
    }

    /* 3. delete from db */
    p_master->port_array[p_port_key->port_id] = NULL;

    /* 4. free db entry */
    XFREE(MEM_TBL_PORT, p_db_port);

    return PM_E_NONE;
}

int32
tbl_port_set_port_field_sync(tbl_port_t *p_port, tbl_port_field_id_t field_id, uint32 sync)
{
    tbl_port_t *p_db_port = NULL;

    /* 1. lookup entry exist */
    p_db_port = tbl_port_get_port(&p_port->key);
    if (NULL == p_db_port)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_PORT_FLD_SLOT_NO:
        p_db_port->slot_no = p_port->slot_no;
        break;

    case TBL_PORT_FLD_PANEL_PORT_NO:
        p_db_port->panel_port_no = p_port->panel_port_no;
        break;

    case TBL_PORT_FLD_PANEL_SUB_PORT_NO:
        p_db_port->panel_sub_port_no = p_port->panel_sub_port_no;
        break;

    case TBL_PORT_FLD_PHY_TYPE:
        p_db_port->phy_type = p_port->phy_type;
        break;

    case TBL_PORT_FLD_PHYINFO_FLAG:
        p_db_port->phyinfo_flag = p_port->phyinfo_flag;
        break;

    case TBL_PORT_FLD_SPEED_CAPBILITY:
        p_db_port->speed_capbility = p_port->speed_capbility;
        break;

    case TBL_PORT_FLD_LOGIC_PORT_ID:
        p_db_port->logic_port_id = p_port->logic_port_id;
        break;

    case TBL_PORT_FLD_MEDIA:
        p_db_port->media = p_port->media;
        break;

    case TBL_PORT_FLD_PORT_MEDIA_TYPE:
        p_db_port->port_media_type = p_port->port_media_type;
        break;

    case TBL_PORT_FLD_SUPPORT_MEDIA_SWITCH:
        p_db_port->support_media_switch = p_port->support_media_switch;
        break;

    case TBL_PORT_FLD_SPLIT_TYPE:
        p_db_port->split_type = p_port->split_type;
        break;

    case TBL_PORT_FLD_BYPASS:
        p_db_port->bypass = p_port->bypass;
        break;

    case TBL_PORT_FLD_BYPASS_SLOT:
        p_db_port->bypass_slot = p_port->bypass_slot;
        break;

    case TBL_PORT_FLD_TRAINING_ENABLE:
        p_db_port->training_enable = p_port->training_enable;
        break;

    case TBL_PORT_FLD_TRAINING_STATUS:
        p_db_port->training_status = p_port->training_status;
        break;

    case TBL_PORT_FLD_PRESENT_STATUS:
        p_db_port->present_status = p_port->present_status;
        break;

    case TBL_PORT_FLD_NAME:
        sal_memcpy(p_db_port->name, p_port->name, sizeof(p_port->name));
        break;

    case TBL_PORT_FLD_MAC:
        sal_memcpy(p_db_port->mac, p_port->mac, sizeof(p_port->mac));
        break;

    case TBL_PORT_FLD_MAX:
        sal_memcpy(p_db_port, p_port, sizeof(tbl_port_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_PORT, field_id, p_db_port);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_PORT, field_id, p_db_port);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_port_add_port(tbl_port_t *p_port)
{
    return tbl_port_add_port_sync(p_port, TRUE);
}

int32
tbl_port_del_port(tbl_port_key_t *p_port_key)
{
    return tbl_port_del_port_sync(p_port_key, TRUE);
}

int32
tbl_port_set_port_field(tbl_port_t *p_port, tbl_port_field_id_t field_id)
{
    return tbl_port_set_port_field_sync(p_port, field_id, TRUE);
}

tbl_port_t*
tbl_port_get_port(tbl_port_key_t *p_port_key)
{
    tbl_port_master_t *p_master = _g_p_tbl_port_master;

    if (GLB_CHSM_PORT_NUM_MAX <= p_port_key->port_id)
    {
        return NULL;
    }
    return (p_master->port_array[p_port_key->port_id]);
}

char*
tbl_port_key_val2str(tbl_port_t *p_port, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%d", p_port->key.port_id);
    return str;
}

int32
tbl_port_key_str2val(char *str, tbl_port_t *p_port)
{
    int32 ret = 0;

    p_port->key.port_id = cdb_int_str2val(str, &ret);
    return ret;
}

char*
tbl_port_key_name_dump(tbl_port_t *p_port, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_PORT);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
        "%d", p_node->name, p_port->key.port_id);
    return str;
}

char*
tbl_port_key_value_dump(tbl_port_t *p_port, char *str)
{
    sal_sprintf(str, "%d", p_port->key.port_id);
    return str;
}

char*
tbl_port_field_name_dump(tbl_port_t *p_port, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_PORT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_PORT_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_port_field_value_dump(tbl_port_t *p_port, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_PORT_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%d", p_port->key.port_id);
    }
    if (FLD_MATCH(TBL_PORT_FLD_SLOT_NO, field_id))
    {
        sal_sprintf(str, "%d", p_port->slot_no);
    }
    if (FLD_MATCH(TBL_PORT_FLD_PANEL_PORT_NO, field_id))
    {
        sal_sprintf(str, "%d", p_port->panel_port_no);
    }
    if (FLD_MATCH(TBL_PORT_FLD_PANEL_SUB_PORT_NO, field_id))
    {
        sal_sprintf(str, "%d", p_port->panel_sub_port_no);
    }
    if (FLD_MATCH(TBL_PORT_FLD_PHY_TYPE, field_id))
    {
        sal_sprintf(str, "%d", p_port->phy_type);
    }
    if (FLD_MATCH(TBL_PORT_FLD_PHYINFO_FLAG, field_id))
    {
        sal_sprintf(str, "%d", p_port->phyinfo_flag);
    }
    if (FLD_MATCH(TBL_PORT_FLD_SPEED_CAPBILITY, field_id))
    {
        sal_sprintf(str, "%d", p_port->speed_capbility);
    }
    if (FLD_MATCH(TBL_PORT_FLD_LOGIC_PORT_ID, field_id))
    {
        sal_sprintf(str, "%d", p_port->logic_port_id);
    }
    if (FLD_MATCH(TBL_PORT_FLD_MEDIA, field_id))
    {
        sal_sprintf(str, "%d", p_port->media);
    }
    if (FLD_MATCH(TBL_PORT_FLD_PORT_MEDIA_TYPE, field_id))
    {
        sal_sprintf(str, "%d", p_port->port_media_type);
    }
    if (FLD_MATCH(TBL_PORT_FLD_SUPPORT_MEDIA_SWITCH, field_id))
    {
        sal_sprintf(str, "%d", p_port->support_media_switch);
    }
    if (FLD_MATCH(TBL_PORT_FLD_SPLIT_TYPE, field_id))
    {
        sal_sprintf(str, "%d", p_port->split_type);
    }
    if (FLD_MATCH(TBL_PORT_FLD_BYPASS, field_id))
    {
        sal_sprintf(str, "%d", p_port->bypass);
    }
    if (FLD_MATCH(TBL_PORT_FLD_BYPASS_SLOT, field_id))
    {
        sal_sprintf(str, "%d", p_port->bypass_slot);
    }
    if (FLD_MATCH(TBL_PORT_FLD_TRAINING_ENABLE, field_id))
    {
        sal_sprintf(str, "%d", p_port->training_enable);
    }
    if (FLD_MATCH(TBL_PORT_FLD_TRAINING_STATUS, field_id))
    {
        sal_sprintf(str, "%d", p_port->training_status);
    }
    if (FLD_MATCH(TBL_PORT_FLD_PRESENT_STATUS, field_id))
    {
        sal_sprintf(str, "%d", p_port->present_status);
    }
    if (FLD_MATCH(TBL_PORT_FLD_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_port->name);
    }
    if (FLD_MATCH(TBL_PORT_FLD_MAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_port->mac);
    }
    return str;
}

char**
tbl_port_table_dump(tbl_port_t *p_port, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_port_key_name_dump(p_port, buf));
    for(i=1; i<TBL_PORT_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_port_field_name_dump(p_port, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_port_field_value_dump(p_port, i, buf));
    }
    return str;
}

int32
tbl_port_field_value_parser(char *str, int32 field_id, tbl_port_t *p_port)
{
    if (FLD_MATCH(TBL_PORT_FLD_KEY, field_id))
    {
    }
    if (FLD_MATCH(TBL_PORT_FLD_SLOT_NO, field_id))
    {
    }
    if (FLD_MATCH(TBL_PORT_FLD_PANEL_PORT_NO, field_id))
    {
    }
    if (FLD_MATCH(TBL_PORT_FLD_PANEL_SUB_PORT_NO, field_id))
    {
    }
    if (FLD_MATCH(TBL_PORT_FLD_PHY_TYPE, field_id))
    {
    }
    if (FLD_MATCH(TBL_PORT_FLD_PHYINFO_FLAG, field_id))
    {
    }
    if (FLD_MATCH(TBL_PORT_FLD_SPEED_CAPBILITY, field_id))
    {
    }
    if (FLD_MATCH(TBL_PORT_FLD_LOGIC_PORT_ID, field_id))
    {
    }
    if (FLD_MATCH(TBL_PORT_FLD_MEDIA, field_id))
    {
    }
    if (FLD_MATCH(TBL_PORT_FLD_PORT_MEDIA_TYPE, field_id))
    {
    }
    if (FLD_MATCH(TBL_PORT_FLD_SUPPORT_MEDIA_SWITCH, field_id))
    {
    }
    if (FLD_MATCH(TBL_PORT_FLD_SPLIT_TYPE, field_id))
    {
    }
    if (FLD_MATCH(TBL_PORT_FLD_BYPASS, field_id))
    {
    }
    if (FLD_MATCH(TBL_PORT_FLD_BYPASS_SLOT, field_id))
    {
    }
    if (FLD_MATCH(TBL_PORT_FLD_TRAINING_ENABLE, field_id))
    {
    }
    if (FLD_MATCH(TBL_PORT_FLD_TRAINING_STATUS, field_id))
    {
    }
    if (FLD_MATCH(TBL_PORT_FLD_PRESENT_STATUS, field_id))
    {
    }
    if (FLD_MATCH(TBL_PORT_FLD_NAME, field_id))
    {
        sal_strcpy(p_port->name, str);
    }
    if (FLD_MATCH(TBL_PORT_FLD_MAC, field_id))
    {
        cdb_mac_addr_str2val(p_port->mac, str);
    }
    return PM_E_NONE;
}

int32
tbl_port_table_parser(char** array, char* key_value,tbl_port_t *p_port)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_PORT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_port_key_str2val(key_value, p_port));

    for(i=1; i<TBL_PORT_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_port_field_value_parser( array[j++], i, p_port));
    }

    return PM_E_NONE;
}

int32
tbl_port_dump_one(tbl_port_t *p_port, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_PORT);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_PORT_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_PORT_FLD_KEY].name,
            p_port->key.port_id);
    }
    if (FLD_MATCH(TBL_PORT_FLD_SLOT_NO, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_PORT_FLD_SLOT_NO].name,
            p_port->slot_no);
    }
    if (FLD_MATCH(TBL_PORT_FLD_PANEL_PORT_NO, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_PORT_FLD_PANEL_PORT_NO].name,
            p_port->panel_port_no);
    }
    if (FLD_MATCH(TBL_PORT_FLD_PANEL_SUB_PORT_NO, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_PORT_FLD_PANEL_SUB_PORT_NO].name,
            p_port->panel_sub_port_no);
    }
    if (FLD_MATCH(TBL_PORT_FLD_PHY_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_PORT_FLD_PHY_TYPE].name,
            p_port->phy_type);
    }
    if (FLD_MATCH(TBL_PORT_FLD_PHYINFO_FLAG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_PORT_FLD_PHYINFO_FLAG].name,
            p_port->phyinfo_flag);
    }
    if (FLD_MATCH(TBL_PORT_FLD_SPEED_CAPBILITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_PORT_FLD_SPEED_CAPBILITY].name,
            p_port->speed_capbility);
    }
    if (FLD_MATCH(TBL_PORT_FLD_LOGIC_PORT_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_PORT_FLD_LOGIC_PORT_ID].name,
            p_port->logic_port_id);
    }
    if (FLD_MATCH(TBL_PORT_FLD_MEDIA, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_PORT_FLD_MEDIA].name,
            p_port->media);
    }
    if (FLD_MATCH(TBL_PORT_FLD_PORT_MEDIA_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_PORT_FLD_PORT_MEDIA_TYPE].name,
            p_port->port_media_type);
    }
    if (FLD_MATCH(TBL_PORT_FLD_SUPPORT_MEDIA_SWITCH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_PORT_FLD_SUPPORT_MEDIA_SWITCH].name,
            p_port->support_media_switch);
    }
    if (FLD_MATCH(TBL_PORT_FLD_SPLIT_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_PORT_FLD_SPLIT_TYPE].name,
            p_port->split_type);
    }
    if (FLD_MATCH(TBL_PORT_FLD_BYPASS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_PORT_FLD_BYPASS].name,
            p_port->bypass);
    }
    if (FLD_MATCH(TBL_PORT_FLD_BYPASS_SLOT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_PORT_FLD_BYPASS_SLOT].name,
            p_port->bypass_slot);
    }
    if (FLD_MATCH(TBL_PORT_FLD_TRAINING_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_PORT_FLD_TRAINING_ENABLE].name,
            p_port->training_enable);
    }
    if (FLD_MATCH(TBL_PORT_FLD_TRAINING_STATUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_PORT_FLD_TRAINING_STATUS].name,
            p_port->training_status);
    }
    if (FLD_MATCH(TBL_PORT_FLD_PRESENT_STATUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_PORT_FLD_PRESENT_STATUS].name,
            p_port->present_status);
    }
    if (FLD_MATCH(TBL_PORT_FLD_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PORT_FLD_NAME].name,
            p_port->name);
    }
    if (FLD_MATCH(TBL_PORT_FLD_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PORT_FLD_MAC].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_port->mac));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_port_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_port_master_t *p_master = _g_p_tbl_port_master;
    tbl_port_t *p_db_port = NULL;
    uint32 port_id = 0;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    for (port_id = 0; port_id < GLB_CHSM_PORT_NUM_MAX; port_id++)
    {
        p_db_port = p_master->port_array[port_id];
        if (NULL == p_db_port)
        {
            continue;
        }
        rc |= fn(p_db_port, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_port_master_t*
tbl_port_get_master()
{
    return _g_p_tbl_port_master;
}

tbl_port_master_t*
tbl_port_init_port()
{
    _g_p_tbl_port_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_port_master_t));
    return _g_p_tbl_port_master;
}

tbl_port_t*
tbl_port_get_port_by_slot_port(uint8 slot_no, uint8 port_no, uint8 sub_port_no)
{
    tbl_port_master_t *p_master = _g_p_tbl_port_master;
    tbl_port_t *p_db_port = NULL;
    uint32 port_idx = 0;

    for (port_idx = 0; port_idx < GLB_CARD_PORT_NUM_MAX; port_idx++)
    {
        p_db_port = p_master->port_array[port_idx];
        if (NULL == p_db_port)
        {
            continue;
        }

        if (p_db_port->slot_no == slot_no
         && p_db_port->panel_port_no == port_no
         && p_db_port->panel_sub_port_no == sub_port_no)
        {
            return p_db_port;
        }
    }

    return NULL;
}

