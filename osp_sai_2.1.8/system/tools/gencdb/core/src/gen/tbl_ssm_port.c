
#include "proto.h"
#include "gen/tbl_ssm_port_define.h"
#include "gen/tbl_ssm_port.h"
#include "cdb_data_cmp.h"

tbl_ssm_port_master_t *_g_p_tbl_ssm_port_master = NULL;

static uint32
_tbl_ssm_port_hash_make(tbl_ssm_port_t *p_ssm_port)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_ssm_port->key;
    for (index = 0; index < sizeof(p_ssm_port->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_ssm_port_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_ssm_port_t *p_ssm_port1 = (tbl_ssm_port_t*)p_arg1;
    tbl_ssm_port_t *p_ssm_port2 = (tbl_ssm_port_t*)p_arg2;

    if (0 == sal_memcmp(&p_ssm_port1->key, &p_ssm_port2->key, sizeof(tbl_ssm_port_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_ssm_port_add_ssm_port_sync(tbl_ssm_port_t *p_ssm_port, uint32 sync)
{
    tbl_ssm_port_master_t *p_master = _g_p_tbl_ssm_port_master;
    tbl_ssm_port_t *p_db_ssm_port = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_ssm_port_get_ssm_port(&p_ssm_port->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_ssm_port = XCALLOC(MEM_TBL_SSM_PORT, sizeof(tbl_ssm_port_t));
    if (NULL == p_db_ssm_port)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_ssm_port, p_ssm_port, sizeof(tbl_ssm_port_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->ssm_port_hash, (void*)p_db_ssm_port, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->ssm_port_list, p_db_ssm_port);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_SSM_PORT, p_db_ssm_port);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_SSM_PORT, p_db_ssm_port);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ssm_port_del_ssm_port_sync(tbl_ssm_port_key_t *p_ssm_port_key, uint32 sync)
{
    tbl_ssm_port_master_t *p_master = _g_p_tbl_ssm_port_master;
    tbl_ssm_port_t *p_db_ssm_port = NULL;

    /* 1. lookup entry exist */
    p_db_ssm_port = tbl_ssm_port_get_ssm_port(p_ssm_port_key);
    if (NULL == p_db_ssm_port)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_SSM_PORT, p_db_ssm_port);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_SSM_PORT, p_db_ssm_port);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->ssm_port_hash, (void*)p_db_ssm_port);
    ctclib_slistnode_delete(p_master->ssm_port_list, p_db_ssm_port);

    /* 4. free db entry */
    XFREE(MEM_TBL_SSM_PORT, p_db_ssm_port);

    return PM_E_NONE;
}

int32
tbl_ssm_port_set_ssm_port_field_sync(tbl_ssm_port_t *p_ssm_port, tbl_ssm_port_field_id_t field_id, uint32 sync)
{
    tbl_ssm_port_t *p_db_ssm_port = NULL;

    /* 1. lookup entry exist */
    p_db_ssm_port = tbl_ssm_port_get_ssm_port(&p_ssm_port->key);
    if (NULL == p_db_ssm_port)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_SSM_PORT_FLD_IFINDEX:
        p_db_ssm_port->ifindex = p_ssm_port->ifindex;
        break;

    case TBL_SSM_PORT_FLD_SYNCE_ENABLE:
        p_db_ssm_port->synce_enable = p_ssm_port->synce_enable;
        break;

    case TBL_SSM_PORT_FLD_SSM_PDU_DIRECT:
        p_db_ssm_port->ssm_pdu_direct = p_ssm_port->ssm_pdu_direct;
        break;

    case TBL_SSM_PORT_FLD_PRIORITY:
        p_db_ssm_port->priority = p_ssm_port->priority;
        break;

    case TBL_SSM_PORT_FLD_REPLACE_CLOCKID_EN:
        p_db_ssm_port->replace_clockid_en = p_ssm_port->replace_clockid_en;
        break;

    case TBL_SSM_PORT_FLD_RECEIVE_PDU_ENABLE:
        p_db_ssm_port->receive_pdu_enable = p_ssm_port->receive_pdu_enable;
        break;

    case TBL_SSM_PORT_FLD_SEND_QUALITY:
        p_db_ssm_port->send_quality = p_ssm_port->send_quality;
        break;

    case TBL_SSM_PORT_FLD_RECEIVE_QUALITY:
        p_db_ssm_port->receive_quality = p_ssm_port->receive_quality;
        break;

    case TBL_SSM_PORT_FLD_FORCE_TX_QUALITY:
        p_db_ssm_port->force_tx_quality = p_ssm_port->force_tx_quality;
        break;

    case TBL_SSM_PORT_FLD_FORCE_RX_QUALITY:
        p_db_ssm_port->force_rx_quality = p_ssm_port->force_rx_quality;
        break;

    case TBL_SSM_PORT_FLD_DNU_GROUP_ID:
        p_db_ssm_port->dnu_group_id = p_ssm_port->dnu_group_id;
        break;

    case TBL_SSM_PORT_FLD_HOP:
        p_db_ssm_port->hop = p_ssm_port->hop;
        break;

    case TBL_SSM_PORT_FLD_RECEIVE_CLOCK_ID:
        sal_memcpy(p_db_ssm_port->receive_clock_id, p_ssm_port->receive_clock_id, sizeof(p_ssm_port->receive_clock_id));
        break;

    case TBL_SSM_PORT_FLD_SSM_TIMEOUT_TIMER:
        p_db_ssm_port->ssm_timeout_timer = p_ssm_port->ssm_timeout_timer;
        break;

    case TBL_SSM_PORT_FLD_SSM_SEND_TIMER:
        p_db_ssm_port->ssm_send_timer = p_ssm_port->ssm_send_timer;
        break;

    case TBL_SSM_PORT_FLD_MAX:
        sal_memcpy(p_db_ssm_port, p_ssm_port, sizeof(tbl_ssm_port_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_SSM_PORT, field_id, p_db_ssm_port);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_SSM_PORT, field_id, p_db_ssm_port);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ssm_port_add_ssm_port(tbl_ssm_port_t *p_ssm_port)
{
    return tbl_ssm_port_add_ssm_port_sync(p_ssm_port, TRUE);
}

int32
tbl_ssm_port_del_ssm_port(tbl_ssm_port_key_t *p_ssm_port_key)
{
    return tbl_ssm_port_del_ssm_port_sync(p_ssm_port_key, TRUE);
}

int32
tbl_ssm_port_set_ssm_port_field(tbl_ssm_port_t *p_ssm_port, tbl_ssm_port_field_id_t field_id)
{
    return tbl_ssm_port_set_ssm_port_field_sync(p_ssm_port, field_id, TRUE);
}

tbl_ssm_port_t*
tbl_ssm_port_get_ssm_port(tbl_ssm_port_key_t *p_ssm_port_key)
{
    tbl_ssm_port_master_t *p_master = _g_p_tbl_ssm_port_master;
    tbl_ssm_port_t lkp;

    sal_memcpy(&lkp.key, p_ssm_port_key, sizeof(tbl_ssm_port_key_t));
    return ctclib_hash_lookup(p_master->ssm_port_hash, &lkp);
}

char*
tbl_ssm_port_key_val2str(tbl_ssm_port_t *p_ssm_port, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_ssm_port->key.name);
    return str;
}

int32
tbl_ssm_port_key_str2val(char *str, tbl_ssm_port_t *p_ssm_port)
{
    int32 ret = 0;

    sal_strcpy(p_ssm_port->key.name, str);
    return ret;
}

char*
tbl_ssm_port_key_name_dump(tbl_ssm_port_t *p_ssm_port, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SSM_PORT);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_ssm_port->key.name);
    return str;
}

char*
tbl_ssm_port_key_value_dump(tbl_ssm_port_t *p_ssm_port, char *str)
{
    sal_sprintf(str, "%s", p_ssm_port->key.name);
    return str;
}

char*
tbl_ssm_port_field_name_dump(tbl_ssm_port_t *p_ssm_port, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_SSM_PORT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_SSM_PORT_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_ssm_port_field_value_dump(tbl_ssm_port_t *p_ssm_port, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_SSM_PORT_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_ssm_port->key.name);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_ssm_port->ifindex);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_SYNCE_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_ssm_port->synce_enable);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_SSM_PDU_DIRECT, field_id))
    {
        sal_sprintf(str, "%u", p_ssm_port->ssm_pdu_direct);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_PRIORITY, field_id))
    {
        sal_sprintf(str, "%u", p_ssm_port->priority);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_REPLACE_CLOCKID_EN, field_id))
    {
        sal_sprintf(str, "%u", p_ssm_port->replace_clockid_en);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_RECEIVE_PDU_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_ssm_port->receive_pdu_enable);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_SEND_QUALITY, field_id))
    {
        sal_sprintf(str, "%u", p_ssm_port->send_quality);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_RECEIVE_QUALITY, field_id))
    {
        sal_sprintf(str, "%u", p_ssm_port->receive_quality);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_FORCE_TX_QUALITY, field_id))
    {
        sal_sprintf(str, "%u", p_ssm_port->force_tx_quality);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_FORCE_RX_QUALITY, field_id))
    {
        sal_sprintf(str, "%u", p_ssm_port->force_rx_quality);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_DNU_GROUP_ID, field_id))
    {
        sal_sprintf(str, "%u", p_ssm_port->dnu_group_id);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_HOP, field_id))
    {
        sal_sprintf(str, "%u", p_ssm_port->hop);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_RECEIVE_CLOCK_ID, field_id))
    {
        cdb_ssm_clock_id_val2str(str, MAX_CMD_STR_LEN, p_ssm_port->receive_clock_id);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_SSM_TIMEOUT_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_ssm_port->ssm_timeout_timer);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_SSM_SEND_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_ssm_port->ssm_send_timer);
    }
    return str;
}

char**
tbl_ssm_port_table_dump(tbl_ssm_port_t *p_ssm_port, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_ssm_port_key_name_dump(p_ssm_port, buf));
    for(i=1; i<TBL_SSM_PORT_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_ssm_port_field_name_dump(p_ssm_port, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_ssm_port_field_value_dump(p_ssm_port, i, buf));
    }
    return str;
}

int32
tbl_ssm_port_field_value_parser(char *str, int32 field_id, tbl_ssm_port_t *p_ssm_port)
{
    if (FLD_MATCH(TBL_SSM_PORT_FLD_KEY, field_id))
    {
        sal_strcpy(p_ssm_port->key.name, str);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_IFINDEX, field_id))
    {
        int32 ret;
        p_ssm_port->ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_SYNCE_ENABLE, field_id))
    {
        int32 ret;
        p_ssm_port->synce_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_SSM_PDU_DIRECT, field_id))
    {
        int32 ret;
        p_ssm_port->ssm_pdu_direct = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_PRIORITY, field_id))
    {
        int32 ret;
        p_ssm_port->priority = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_REPLACE_CLOCKID_EN, field_id))
    {
        int32 ret;
        p_ssm_port->replace_clockid_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_RECEIVE_PDU_ENABLE, field_id))
    {
        int32 ret;
        p_ssm_port->receive_pdu_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_SEND_QUALITY, field_id))
    {
        int32 ret;
        p_ssm_port->send_quality = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_RECEIVE_QUALITY, field_id))
    {
        int32 ret;
        p_ssm_port->receive_quality = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_FORCE_TX_QUALITY, field_id))
    {
        int32 ret;
        p_ssm_port->force_tx_quality = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_FORCE_RX_QUALITY, field_id))
    {
        int32 ret;
        p_ssm_port->force_rx_quality = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_DNU_GROUP_ID, field_id))
    {
        int32 ret;
        p_ssm_port->dnu_group_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_HOP, field_id))
    {
        int32 ret;
        p_ssm_port->hop = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_RECEIVE_CLOCK_ID, field_id))
    {
        cdb_ssm_clock_id_str2val(str, p_ssm_port->receive_clock_id);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_SSM_TIMEOUT_TIMER, field_id))
    {
        ctc_task_str2val(str, p_ssm_port->ssm_timeout_timer);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_SSM_SEND_TIMER, field_id))
    {
        ctc_task_str2val(str, p_ssm_port->ssm_send_timer);
    }
    return PM_E_NONE;
}

int32
tbl_ssm_port_table_parser(char** array, char* key_value,tbl_ssm_port_t *p_ssm_port)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_SSM_PORT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_ssm_port_key_str2val(key_value, p_ssm_port));

    for(i=1; i<TBL_SSM_PORT_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_ssm_port_field_value_parser( array[j++], i, p_ssm_port));
    }

    return PM_E_NONE;
}

int32
tbl_ssm_port_dump_one(tbl_ssm_port_t *p_ssm_port, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SSM_PORT);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_SSM_PORT_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SSM_PORT_FLD_KEY].name,
            p_ssm_port->key.name);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSM_PORT_FLD_IFINDEX].name,
            p_ssm_port->ifindex);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_SYNCE_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSM_PORT_FLD_SYNCE_ENABLE].name,
            p_ssm_port->synce_enable);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_SSM_PDU_DIRECT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSM_PORT_FLD_SSM_PDU_DIRECT].name,
            p_ssm_port->ssm_pdu_direct);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_PRIORITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSM_PORT_FLD_PRIORITY].name,
            p_ssm_port->priority);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_REPLACE_CLOCKID_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSM_PORT_FLD_REPLACE_CLOCKID_EN].name,
            p_ssm_port->replace_clockid_en);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_RECEIVE_PDU_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSM_PORT_FLD_RECEIVE_PDU_ENABLE].name,
            p_ssm_port->receive_pdu_enable);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_SEND_QUALITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSM_PORT_FLD_SEND_QUALITY].name,
            p_ssm_port->send_quality);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_RECEIVE_QUALITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSM_PORT_FLD_RECEIVE_QUALITY].name,
            p_ssm_port->receive_quality);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_FORCE_TX_QUALITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSM_PORT_FLD_FORCE_TX_QUALITY].name,
            p_ssm_port->force_tx_quality);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_FORCE_RX_QUALITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSM_PORT_FLD_FORCE_RX_QUALITY].name,
            p_ssm_port->force_rx_quality);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_DNU_GROUP_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSM_PORT_FLD_DNU_GROUP_ID].name,
            p_ssm_port->dnu_group_id);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_HOP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SSM_PORT_FLD_HOP].name,
            p_ssm_port->hop);
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_RECEIVE_CLOCK_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SSM_PORT_FLD_RECEIVE_CLOCK_ID].name, 
            cdb_ssm_clock_id_val2str(buf, MAX_CMD_STR_LEN, p_ssm_port->receive_clock_id));
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_SSM_TIMEOUT_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SSM_PORT_FLD_SSM_TIMEOUT_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_ssm_port->ssm_timeout_timer));
    }
    if (FLD_MATCH(TBL_SSM_PORT_FLD_SSM_SEND_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SSM_PORT_FLD_SSM_SEND_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_ssm_port->ssm_send_timer));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_ssm_port_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_ssm_port_master_t *p_master = _g_p_tbl_ssm_port_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_ssm_port_t *p_db_ssm_port = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->ssm_port_list, p_db_ssm_port, listnode, next)
    {
        rc |= fn(p_db_ssm_port, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_ssm_port_master_t*
tbl_ssm_port_get_master()
{
    return _g_p_tbl_ssm_port_master;
}

tbl_ssm_port_master_t*
tbl_ssm_port_init_ssm_port()
{
    _g_p_tbl_ssm_port_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_ssm_port_master_t));
    _g_p_tbl_ssm_port_master->ssm_port_hash = ctclib_hash_create(_tbl_ssm_port_hash_make, _tbl_ssm_port_hash_cmp);
    _g_p_tbl_ssm_port_master->ssm_port_list = ctclib_slist_create(tbl_ssm_port_cmp, NULL);
    return _g_p_tbl_ssm_port_master;
}

