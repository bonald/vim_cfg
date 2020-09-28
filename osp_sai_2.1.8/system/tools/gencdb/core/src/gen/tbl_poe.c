
#include "proto.h"
#include "gen/tbl_poe_define.h"
#include "gen/tbl_poe.h"
#include "cdb_data_cmp.h"

tbl_poe_master_t *_g_p_tbl_poe_master = NULL;

static uint32
_tbl_poe_hash_make(tbl_poe_t *p_poe)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_poe->key;
    for (index = 0; index < sizeof(p_poe->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_poe_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_poe_t *p_poe1 = (tbl_poe_t*)p_arg1;
    tbl_poe_t *p_poe2 = (tbl_poe_t*)p_arg2;

    if (0 == sal_memcmp(&p_poe1->key, &p_poe2->key, sizeof(tbl_poe_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_poe_add_poe_sync(tbl_poe_t *p_poe, uint32 sync)
{
    tbl_poe_master_t *p_master = _g_p_tbl_poe_master;
    tbl_poe_t *p_db_poe = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_poe_get_poe(&p_poe->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_poe = XCALLOC(MEM_TBL_POE, sizeof(tbl_poe_t));
    if (NULL == p_db_poe)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_poe, p_poe, sizeof(tbl_poe_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->poe_hash, (void*)p_db_poe, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->poe_list, p_db_poe);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_POE, p_db_poe);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_POE, p_db_poe);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_poe_del_poe_sync(tbl_poe_key_t *p_poe_key, uint32 sync)
{
    tbl_poe_master_t *p_master = _g_p_tbl_poe_master;
    tbl_poe_t *p_db_poe = NULL;

    /* 1. lookup entry exist */
    p_db_poe = tbl_poe_get_poe(p_poe_key);
    if (NULL == p_db_poe)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_POE, p_db_poe);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_POE, p_db_poe);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->poe_hash, (void*)p_db_poe);
    ctclib_slistnode_delete(p_master->poe_list, p_db_poe);

    /* 4. free db entry */
    XFREE(MEM_TBL_POE, p_db_poe);

    return PM_E_NONE;
}

int32
tbl_poe_set_poe_field_sync(tbl_poe_t *p_poe, tbl_poe_field_id_t field_id, uint32 sync)
{
    tbl_poe_t *p_db_poe = NULL;

    /* 1. lookup entry exist */
    p_db_poe = tbl_poe_get_poe(&p_poe->key);
    if (NULL == p_db_poe)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_POE_FLD_SLOT:
        p_db_poe->slot = p_poe->slot;
        break;

    case TBL_POE_FLD_GLB_ENABLE:
        p_db_poe->glb_enable = p_poe->glb_enable;
        break;

    case TBL_POE_FLD_GLB_POWER_LIMIT:
        p_db_poe->glb_power_limit = p_poe->glb_power_limit;
        break;

    case TBL_POE_FLD_GLB_STATUS:
        p_db_poe->glb_status = p_poe->glb_status;
        break;

    case TBL_POE_FLD_GLB_CUR_CONSUMP:
        p_db_poe->glb_cur_consump = p_poe->glb_cur_consump;
        break;

    case TBL_POE_FLD_GLB_AVER_CONSUMP:
        p_db_poe->glb_aver_consump = p_poe->glb_aver_consump;
        break;

    case TBL_POE_FLD_GLB_PEAK_CONSUMP:
        p_db_poe->glb_peak_consump = p_poe->glb_peak_consump;
        break;

    case TBL_POE_FLD_PSE_CHANNEL_NUM:
        p_db_poe->pse_channel_num = p_poe->pse_channel_num;
        break;

    case TBL_POE_FLD_CONST_POWER_LIMIT:
        p_db_poe->const_power_limit = p_poe->const_power_limit;
        break;

    case TBL_POE_FLD_PSE_CHANNEL_ID:
        sal_memcpy(p_db_poe->pse_channel_id, p_poe->pse_channel_id, sizeof(p_poe->pse_channel_id));
        break;

    case TBL_POE_FLD_PANEL_SLOT_NO:
        sal_memcpy(p_db_poe->panel_slot_no, p_poe->panel_slot_no, sizeof(p_poe->panel_slot_no));
        break;

    case TBL_POE_FLD_PANEL_PORT_NO:
        sal_memcpy(p_db_poe->panel_port_no, p_poe->panel_port_no, sizeof(p_poe->panel_port_no));
        break;

    case TBL_POE_FLD_PANEL_SUB_PORT_NO:
        sal_memcpy(p_db_poe->panel_sub_port_no, p_poe->panel_sub_port_no, sizeof(p_poe->panel_sub_port_no));
        break;

    case TBL_POE_FLD_ENABLE:
        sal_memcpy(p_db_poe->enable, p_poe->enable, sizeof(p_poe->enable));
        break;

    case TBL_POE_FLD_FORCE_POWER:
        sal_memcpy(p_db_poe->force_power, p_poe->force_power, sizeof(p_poe->force_power));
        break;

    case TBL_POE_FLD_MODE:
        sal_memcpy(p_db_poe->mode, p_poe->mode, sizeof(p_poe->mode));
        break;

    case TBL_POE_FLD_PRIORITY:
        sal_memcpy(p_db_poe->priority, p_poe->priority, sizeof(p_poe->priority));
        break;

    case TBL_POE_FLD_POWER_LIMIT:
        sal_memcpy(p_db_poe->power_limit, p_poe->power_limit, sizeof(p_poe->power_limit));
        break;

    case TBL_POE_FLD_CLASS_LEVEL:
        sal_memcpy(p_db_poe->class_level, p_poe->class_level, sizeof(p_poe->class_level));
        break;

    case TBL_POE_FLD_STATUS:
        sal_memcpy(p_db_poe->status, p_poe->status, sizeof(p_poe->status));
        break;

    case TBL_POE_FLD_CUR_CONSUMP:
        sal_memcpy(p_db_poe->cur_consump, p_poe->cur_consump, sizeof(p_poe->cur_consump));
        break;

    case TBL_POE_FLD_AVER_CONSUMP:
        sal_memcpy(p_db_poe->aver_consump, p_poe->aver_consump, sizeof(p_poe->aver_consump));
        break;

    case TBL_POE_FLD_PEAK_CONSUMP:
        sal_memcpy(p_db_poe->peak_consump, p_poe->peak_consump, sizeof(p_poe->peak_consump));
        break;

    case TBL_POE_FLD_CURRENT:
        sal_memcpy(p_db_poe->current, p_poe->current, sizeof(p_poe->current));
        break;

    case TBL_POE_FLD_VOLTAGE:
        sal_memcpy(p_db_poe->voltage, p_poe->voltage, sizeof(p_poe->voltage));
        break;

    case TBL_POE_FLD_TEMPER:
        sal_memcpy(p_db_poe->temper, p_poe->temper, sizeof(p_poe->temper));
        break;

    case TBL_POE_FLD_MAX:
        sal_memcpy(p_db_poe, p_poe, sizeof(tbl_poe_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_POE, field_id, p_db_poe);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_POE, field_id, p_db_poe);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_poe_add_poe(tbl_poe_t *p_poe)
{
    return tbl_poe_add_poe_sync(p_poe, TRUE);
}

int32
tbl_poe_del_poe(tbl_poe_key_t *p_poe_key)
{
    return tbl_poe_del_poe_sync(p_poe_key, TRUE);
}

int32
tbl_poe_set_poe_field(tbl_poe_t *p_poe, tbl_poe_field_id_t field_id)
{
    return tbl_poe_set_poe_field_sync(p_poe, field_id, TRUE);
}

tbl_poe_t*
tbl_poe_get_poe(tbl_poe_key_t *p_poe_key)
{
    tbl_poe_master_t *p_master = _g_p_tbl_poe_master;
    tbl_poe_t lkp;

    sal_memcpy(&lkp.key, p_poe_key, sizeof(tbl_poe_key_t));
    return ctclib_hash_lookup(p_master->poe_hash, &lkp);
}

char*
tbl_poe_key_val2str(tbl_poe_t *p_poe, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%d", p_poe->key.id);
    return str;
}

int32
tbl_poe_key_str2val(char *str, tbl_poe_t *p_poe)
{
    int32 ret = 0;

    p_poe->key.id = cdb_int_str2val(str, &ret);
    return ret;
}

char*
tbl_poe_key_name_dump(tbl_poe_t *p_poe, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_POE);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
        "%d", p_node->name, p_poe->key.id);
    return str;
}

char*
tbl_poe_key_value_dump(tbl_poe_t *p_poe, char *str)
{
    sal_sprintf(str, "%d", p_poe->key.id);
    return str;
}

char*
tbl_poe_field_name_dump(tbl_poe_t *p_poe, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_POE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_POE_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_poe_field_value_dump(tbl_poe_t *p_poe, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_POE_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%d", p_poe->key.id);
    }
    if (FLD_MATCH(TBL_POE_FLD_SLOT, field_id))
    {
        sal_sprintf(str, "%d", p_poe->slot);
    }
    if (FLD_MATCH(TBL_POE_FLD_GLB_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_poe->glb_enable);
    }
    if (FLD_MATCH(TBL_POE_FLD_GLB_POWER_LIMIT, field_id))
    {
        sal_sprintf(str, "%u", p_poe->glb_power_limit);
    }
    if (FLD_MATCH(TBL_POE_FLD_GLB_STATUS, field_id))
    {
        sal_sprintf(str, "%u", p_poe->glb_status);
    }
    if (FLD_MATCH(TBL_POE_FLD_GLB_CUR_CONSUMP, field_id))
    {
        sal_sprintf(str, "%u", p_poe->glb_cur_consump);
    }
    if (FLD_MATCH(TBL_POE_FLD_GLB_AVER_CONSUMP, field_id))
    {
        sal_sprintf(str, "%u", p_poe->glb_aver_consump);
    }
    if (FLD_MATCH(TBL_POE_FLD_GLB_PEAK_CONSUMP, field_id))
    {
        sal_sprintf(str, "%u", p_poe->glb_peak_consump);
    }
    if (FLD_MATCH(TBL_POE_FLD_PSE_CHANNEL_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_poe->pse_channel_num);
    }
    if (FLD_MATCH(TBL_POE_FLD_CONST_POWER_LIMIT, field_id))
    {
        sal_sprintf(str, "%u", p_poe->const_power_limit);
    }
    if (FLD_MATCH(TBL_POE_FLD_PSE_CHANNEL_ID, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_poe->pse_channel_id, sizeof(p_poe->pse_channel_id)/4);
    }
    if (FLD_MATCH(TBL_POE_FLD_PANEL_SLOT_NO, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_poe->panel_slot_no, sizeof(p_poe->panel_slot_no)/4);
    }
    if (FLD_MATCH(TBL_POE_FLD_PANEL_PORT_NO, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_poe->panel_port_no, sizeof(p_poe->panel_port_no)/4);
    }
    if (FLD_MATCH(TBL_POE_FLD_PANEL_SUB_PORT_NO, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_poe->panel_sub_port_no, sizeof(p_poe->panel_sub_port_no)/4);
    }
    if (FLD_MATCH(TBL_POE_FLD_ENABLE, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_poe->enable, sizeof(p_poe->enable)/4);
    }
    if (FLD_MATCH(TBL_POE_FLD_FORCE_POWER, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_poe->force_power, sizeof(p_poe->force_power)/4);
    }
    if (FLD_MATCH(TBL_POE_FLD_MODE, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_poe->mode, sizeof(p_poe->mode)/4);
    }
    if (FLD_MATCH(TBL_POE_FLD_PRIORITY, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_poe->priority, sizeof(p_poe->priority)/4);
    }
    if (FLD_MATCH(TBL_POE_FLD_POWER_LIMIT, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_poe->power_limit, sizeof(p_poe->power_limit)/4);
    }
    if (FLD_MATCH(TBL_POE_FLD_CLASS_LEVEL, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_poe->class_level, sizeof(p_poe->class_level)/4);
    }
    if (FLD_MATCH(TBL_POE_FLD_STATUS, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_poe->status, sizeof(p_poe->status)/4);
    }
    if (FLD_MATCH(TBL_POE_FLD_CUR_CONSUMP, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_poe->cur_consump, sizeof(p_poe->cur_consump)/4);
    }
    if (FLD_MATCH(TBL_POE_FLD_AVER_CONSUMP, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_poe->aver_consump, sizeof(p_poe->aver_consump)/4);
    }
    if (FLD_MATCH(TBL_POE_FLD_PEAK_CONSUMP, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_poe->peak_consump, sizeof(p_poe->peak_consump)/4);
    }
    if (FLD_MATCH(TBL_POE_FLD_CURRENT, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_poe->current, sizeof(p_poe->current)/4);
    }
    if (FLD_MATCH(TBL_POE_FLD_VOLTAGE, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_poe->voltage, sizeof(p_poe->voltage)/4);
    }
    if (FLD_MATCH(TBL_POE_FLD_TEMPER, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_poe->temper, sizeof(p_poe->temper)/4);
    }
    return str;
}

char**
tbl_poe_table_dump(tbl_poe_t *p_poe, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_poe_key_name_dump(p_poe, buf));
    for(i=1; i<TBL_POE_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_poe_field_name_dump(p_poe, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_poe_field_value_dump(p_poe, i, buf));
    }
    return str;
}

int32
tbl_poe_field_value_parser(char *str, int32 field_id, tbl_poe_t *p_poe)
{
    if (FLD_MATCH(TBL_POE_FLD_KEY, field_id))
    {
    }
    if (FLD_MATCH(TBL_POE_FLD_SLOT, field_id))
    {
    }
    if (FLD_MATCH(TBL_POE_FLD_GLB_ENABLE, field_id))
    {
        int32 ret;
        p_poe->glb_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_POE_FLD_GLB_POWER_LIMIT, field_id))
    {
        int32 ret;
        p_poe->glb_power_limit = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_POE_FLD_GLB_STATUS, field_id))
    {
        int32 ret;
        p_poe->glb_status = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_POE_FLD_GLB_CUR_CONSUMP, field_id))
    {
        int32 ret;
        p_poe->glb_cur_consump = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_POE_FLD_GLB_AVER_CONSUMP, field_id))
    {
        int32 ret;
        p_poe->glb_aver_consump = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_POE_FLD_GLB_PEAK_CONSUMP, field_id))
    {
        int32 ret;
        p_poe->glb_peak_consump = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_POE_FLD_PSE_CHANNEL_NUM, field_id))
    {
        int32 ret;
        p_poe->pse_channel_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_POE_FLD_CONST_POWER_LIMIT, field_id))
    {
        int32 ret;
        p_poe->const_power_limit = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_POE_FLD_PSE_CHANNEL_ID, field_id))
    {
        cdb_uint32_array_str2val(str, p_poe->pse_channel_id);
    }
    if (FLD_MATCH(TBL_POE_FLD_PANEL_SLOT_NO, field_id))
    {
        cdb_uint32_array_str2val(str, p_poe->panel_slot_no);
    }
    if (FLD_MATCH(TBL_POE_FLD_PANEL_PORT_NO, field_id))
    {
        cdb_uint32_array_str2val(str, p_poe->panel_port_no);
    }
    if (FLD_MATCH(TBL_POE_FLD_PANEL_SUB_PORT_NO, field_id))
    {
        cdb_uint32_array_str2val(str, p_poe->panel_sub_port_no);
    }
    if (FLD_MATCH(TBL_POE_FLD_ENABLE, field_id))
    {
        cdb_uint32_array_str2val(str, p_poe->enable);
    }
    if (FLD_MATCH(TBL_POE_FLD_FORCE_POWER, field_id))
    {
        cdb_uint32_array_str2val(str, p_poe->force_power);
    }
    if (FLD_MATCH(TBL_POE_FLD_MODE, field_id))
    {
        cdb_uint32_array_str2val(str, p_poe->mode);
    }
    if (FLD_MATCH(TBL_POE_FLD_PRIORITY, field_id))
    {
        cdb_uint32_array_str2val(str, p_poe->priority);
    }
    if (FLD_MATCH(TBL_POE_FLD_POWER_LIMIT, field_id))
    {
        cdb_uint32_array_str2val(str, p_poe->power_limit);
    }
    if (FLD_MATCH(TBL_POE_FLD_CLASS_LEVEL, field_id))
    {
        cdb_uint32_array_str2val(str, p_poe->class_level);
    }
    if (FLD_MATCH(TBL_POE_FLD_STATUS, field_id))
    {
        cdb_uint32_array_str2val(str, p_poe->status);
    }
    if (FLD_MATCH(TBL_POE_FLD_CUR_CONSUMP, field_id))
    {
        cdb_uint32_array_str2val(str, p_poe->cur_consump);
    }
    if (FLD_MATCH(TBL_POE_FLD_AVER_CONSUMP, field_id))
    {
        cdb_uint32_array_str2val(str, p_poe->aver_consump);
    }
    if (FLD_MATCH(TBL_POE_FLD_PEAK_CONSUMP, field_id))
    {
        cdb_uint32_array_str2val(str, p_poe->peak_consump);
    }
    if (FLD_MATCH(TBL_POE_FLD_CURRENT, field_id))
    {
        cdb_uint32_array_str2val(str, p_poe->current);
    }
    if (FLD_MATCH(TBL_POE_FLD_VOLTAGE, field_id))
    {
        cdb_uint32_array_str2val(str, p_poe->voltage);
    }
    if (FLD_MATCH(TBL_POE_FLD_TEMPER, field_id))
    {
        cdb_uint32_array_str2val(str, p_poe->temper);
    }
    return PM_E_NONE;
}

int32
tbl_poe_table_parser(char** array, char* key_value,tbl_poe_t *p_poe)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_POE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_poe_key_str2val(key_value, p_poe));

    for(i=1; i<TBL_POE_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_poe_field_value_parser( array[j++], i, p_poe));
    }

    return PM_E_NONE;
}

int32
tbl_poe_dump_one(tbl_poe_t *p_poe, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_POE);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_POE_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_POE_FLD_KEY].name,
            p_poe->key.id);
    }
    if (FLD_MATCH(TBL_POE_FLD_SLOT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_POE_FLD_SLOT].name,
            p_poe->slot);
    }
    if (FLD_MATCH(TBL_POE_FLD_GLB_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_POE_FLD_GLB_ENABLE].name,
            p_poe->glb_enable);
    }
    if (FLD_MATCH(TBL_POE_FLD_GLB_POWER_LIMIT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_POE_FLD_GLB_POWER_LIMIT].name,
            p_poe->glb_power_limit);
    }
    if (FLD_MATCH(TBL_POE_FLD_GLB_STATUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_POE_FLD_GLB_STATUS].name,
            p_poe->glb_status);
    }
    if (FLD_MATCH(TBL_POE_FLD_GLB_CUR_CONSUMP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_POE_FLD_GLB_CUR_CONSUMP].name,
            p_poe->glb_cur_consump);
    }
    if (FLD_MATCH(TBL_POE_FLD_GLB_AVER_CONSUMP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_POE_FLD_GLB_AVER_CONSUMP].name,
            p_poe->glb_aver_consump);
    }
    if (FLD_MATCH(TBL_POE_FLD_GLB_PEAK_CONSUMP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_POE_FLD_GLB_PEAK_CONSUMP].name,
            p_poe->glb_peak_consump);
    }
    if (FLD_MATCH(TBL_POE_FLD_PSE_CHANNEL_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_POE_FLD_PSE_CHANNEL_NUM].name,
            p_poe->pse_channel_num);
    }
    if (FLD_MATCH(TBL_POE_FLD_CONST_POWER_LIMIT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_POE_FLD_CONST_POWER_LIMIT].name,
            p_poe->const_power_limit);
    }
    if (FLD_MATCH(TBL_POE_FLD_PSE_CHANNEL_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_POE_FLD_PSE_CHANNEL_ID].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_poe->pse_channel_id, sizeof(p_poe->pse_channel_id)/4));
    }
    if (FLD_MATCH(TBL_POE_FLD_PANEL_SLOT_NO, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_POE_FLD_PANEL_SLOT_NO].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_poe->panel_slot_no, sizeof(p_poe->panel_slot_no)/4));
    }
    if (FLD_MATCH(TBL_POE_FLD_PANEL_PORT_NO, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_POE_FLD_PANEL_PORT_NO].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_poe->panel_port_no, sizeof(p_poe->panel_port_no)/4));
    }
    if (FLD_MATCH(TBL_POE_FLD_PANEL_SUB_PORT_NO, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_POE_FLD_PANEL_SUB_PORT_NO].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_poe->panel_sub_port_no, sizeof(p_poe->panel_sub_port_no)/4));
    }
    if (FLD_MATCH(TBL_POE_FLD_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_POE_FLD_ENABLE].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_poe->enable, sizeof(p_poe->enable)/4));
    }
    if (FLD_MATCH(TBL_POE_FLD_FORCE_POWER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_POE_FLD_FORCE_POWER].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_poe->force_power, sizeof(p_poe->force_power)/4));
    }
    if (FLD_MATCH(TBL_POE_FLD_MODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_POE_FLD_MODE].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_poe->mode, sizeof(p_poe->mode)/4));
    }
    if (FLD_MATCH(TBL_POE_FLD_PRIORITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_POE_FLD_PRIORITY].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_poe->priority, sizeof(p_poe->priority)/4));
    }
    if (FLD_MATCH(TBL_POE_FLD_POWER_LIMIT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_POE_FLD_POWER_LIMIT].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_poe->power_limit, sizeof(p_poe->power_limit)/4));
    }
    if (FLD_MATCH(TBL_POE_FLD_CLASS_LEVEL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_POE_FLD_CLASS_LEVEL].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_poe->class_level, sizeof(p_poe->class_level)/4));
    }
    if (FLD_MATCH(TBL_POE_FLD_STATUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_POE_FLD_STATUS].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_poe->status, sizeof(p_poe->status)/4));
    }
    if (FLD_MATCH(TBL_POE_FLD_CUR_CONSUMP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_POE_FLD_CUR_CONSUMP].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_poe->cur_consump, sizeof(p_poe->cur_consump)/4));
    }
    if (FLD_MATCH(TBL_POE_FLD_AVER_CONSUMP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_POE_FLD_AVER_CONSUMP].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_poe->aver_consump, sizeof(p_poe->aver_consump)/4));
    }
    if (FLD_MATCH(TBL_POE_FLD_PEAK_CONSUMP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_POE_FLD_PEAK_CONSUMP].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_poe->peak_consump, sizeof(p_poe->peak_consump)/4));
    }
    if (FLD_MATCH(TBL_POE_FLD_CURRENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_POE_FLD_CURRENT].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_poe->current, sizeof(p_poe->current)/4));
    }
    if (FLD_MATCH(TBL_POE_FLD_VOLTAGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_POE_FLD_VOLTAGE].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_poe->voltage, sizeof(p_poe->voltage)/4));
    }
    if (FLD_MATCH(TBL_POE_FLD_TEMPER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_POE_FLD_TEMPER].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_poe->temper, sizeof(p_poe->temper)/4));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_poe_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_poe_master_t *p_master = _g_p_tbl_poe_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_poe_t *p_db_poe = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->poe_list, p_db_poe, listnode, next)
    {
        rc |= fn(p_db_poe, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_poe_master_t*
tbl_poe_get_master()
{
    return _g_p_tbl_poe_master;
}

tbl_poe_master_t*
tbl_poe_init_poe()
{
    _g_p_tbl_poe_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_poe_master_t));
    _g_p_tbl_poe_master->poe_hash = ctclib_hash_create(_tbl_poe_hash_make, _tbl_poe_hash_cmp);
    _g_p_tbl_poe_master->poe_list = ctclib_slist_create(tbl_poe_cmp, NULL);
    return _g_p_tbl_poe_master;
}

