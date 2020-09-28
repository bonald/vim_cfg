
#include "proto.h"
#include "gen/tbl_fea_ptp_global_define.h"
#include "gen/tbl_fea_ptp_global.h"
#include "cdb_data_cmp.h"

tbl_fea_ptp_global_t *_g_p_tbl_fea_ptp_global = NULL;

int32
tbl_fea_ptp_global_set_fea_ptp_global_field_sync(tbl_fea_ptp_global_t *p_fea_ptp_global, tbl_fea_ptp_global_field_id_t field_id, uint32 sync)
{
    tbl_fea_ptp_global_t *p_db_fea_ptp_global = _g_p_tbl_fea_ptp_global;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_FEA_PTP_GLOBAL_FLD_GLOBAL_ENABLE:
        p_db_fea_ptp_global->global_enable = p_fea_ptp_global->global_enable;
        break;

    case TBL_FEA_PTP_GLOBAL_FLD_DEVICE_TYPE:
        p_db_fea_ptp_global->device_type = p_fea_ptp_global->device_type;
        break;

    case TBL_FEA_PTP_GLOBAL_FLD_RETRY_CNT:
        p_db_fea_ptp_global->retry_cnt = p_fea_ptp_global->retry_cnt;
        break;

    case TBL_FEA_PTP_GLOBAL_FLD_READY:
        p_db_fea_ptp_global->ready = p_fea_ptp_global->ready;
        break;

    case TBL_FEA_PTP_GLOBAL_FLD_BLOCKED:
        p_db_fea_ptp_global->blocked = p_fea_ptp_global->blocked;
        break;

    case TBL_FEA_PTP_GLOBAL_FLD_TYPE:
        p_db_fea_ptp_global->type = p_fea_ptp_global->type;
        break;

    case TBL_FEA_PTP_GLOBAL_FLD_TX_IFINDEX:
        p_db_fea_ptp_global->tx_ifindex = p_fea_ptp_global->tx_ifindex;
        break;

    case TBL_FEA_PTP_GLOBAL_FLD_TX_SEQ_ID:
        p_db_fea_ptp_global->tx_seq_id = p_fea_ptp_global->tx_seq_id;
        break;

    case TBL_FEA_PTP_GLOBAL_FLD_INTR_COUNT:
        p_db_fea_ptp_global->intr_count = p_fea_ptp_global->intr_count;
        break;

    case TBL_FEA_PTP_GLOBAL_FLD_ACL_UPDATE_TIME_RANGE:
        p_db_fea_ptp_global->acl_update_time_range = p_fea_ptp_global->acl_update_time_range;
        break;

    case TBL_FEA_PTP_GLOBAL_FLD_HSRV_PTP_TIMER:
        p_db_fea_ptp_global->hsrv_ptp_timer = p_fea_ptp_global->hsrv_ptp_timer;
        break;

    case TBL_FEA_PTP_GLOBAL_FLD_MAX:
        sal_memcpy(p_db_fea_ptp_global, p_fea_ptp_global, sizeof(tbl_fea_ptp_global_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_FEA_PTP_GLOBAL, field_id, p_db_fea_ptp_global);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_FEA_PTP_GLOBAL, field_id, p_db_fea_ptp_global);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_fea_ptp_global_set_fea_ptp_global_field(tbl_fea_ptp_global_t *p_fea_ptp_global, tbl_fea_ptp_global_field_id_t field_id)
{
    return tbl_fea_ptp_global_set_fea_ptp_global_field_sync(p_fea_ptp_global, field_id, TRUE);
}

tbl_fea_ptp_global_t*
tbl_fea_ptp_global_get_fea_ptp_global()
{
    return _g_p_tbl_fea_ptp_global;
}

int32
tbl_fea_ptp_global_key_str2val(char *str, tbl_fea_ptp_global_t *p_fea_ptp_global)
{
    return PM_E_NONE;
}

char*
tbl_fea_ptp_global_key_name_dump(tbl_fea_ptp_global_t *p_fea_ptp_global, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_PTP_GLOBAL);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_fea_ptp_global_key_value_dump(tbl_fea_ptp_global_t *p_fea_ptp_global, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_fea_ptp_global_field_name_dump(tbl_fea_ptp_global_t *p_fea_ptp_global, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_PTP_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_FEA_PTP_GLOBAL_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_fea_ptp_global_field_value_dump(tbl_fea_ptp_global_t *p_fea_ptp_global, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_FEA_PTP_GLOBAL_FLD_GLOBAL_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_fea_ptp_global->global_enable);
    }
    if (FLD_MATCH(TBL_FEA_PTP_GLOBAL_FLD_DEVICE_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_fea_ptp_global->device_type);
    }
    if (FLD_MATCH(TBL_FEA_PTP_GLOBAL_FLD_RETRY_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_fea_ptp_global->retry_cnt);
    }
    if (FLD_MATCH(TBL_FEA_PTP_GLOBAL_FLD_READY, field_id))
    {
        sal_sprintf(str, "%u", p_fea_ptp_global->ready);
    }
    if (FLD_MATCH(TBL_FEA_PTP_GLOBAL_FLD_BLOCKED, field_id))
    {
        sal_sprintf(str, "%u", p_fea_ptp_global->blocked);
    }
    if (FLD_MATCH(TBL_FEA_PTP_GLOBAL_FLD_TYPE, field_id))
    {
        sal_sprintf(str, "%u", p_fea_ptp_global->type);
    }
    if (FLD_MATCH(TBL_FEA_PTP_GLOBAL_FLD_TX_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_fea_ptp_global->tx_ifindex);
    }
    if (FLD_MATCH(TBL_FEA_PTP_GLOBAL_FLD_TX_SEQ_ID, field_id))
    {
        sal_sprintf(str, "%u", p_fea_ptp_global->tx_seq_id);
    }
    if (FLD_MATCH(TBL_FEA_PTP_GLOBAL_FLD_INTR_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_fea_ptp_global->intr_count);
    }
    if (FLD_MATCH(TBL_FEA_PTP_GLOBAL_FLD_ACL_UPDATE_TIME_RANGE, field_id))
    {
        sal_sprintf(str, "%u", p_fea_ptp_global->acl_update_time_range);
    }
    if (FLD_MATCH(TBL_FEA_PTP_GLOBAL_FLD_HSRV_PTP_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_fea_ptp_global->hsrv_ptp_timer);
    }
    return str;
}

char**
tbl_fea_ptp_global_table_dump(tbl_fea_ptp_global_t *p_fea_ptp_global, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_fea_ptp_global_key_name_dump(p_fea_ptp_global, buf));
    for(i=1; i<TBL_FEA_PTP_GLOBAL_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_fea_ptp_global_field_name_dump(p_fea_ptp_global, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_fea_ptp_global_field_value_dump(p_fea_ptp_global, i, buf));
    }
    return str;
}

int32
tbl_fea_ptp_global_field_value_parser(char *str, int32 field_id, tbl_fea_ptp_global_t *p_fea_ptp_global)
{
    if (FLD_MATCH(TBL_FEA_PTP_GLOBAL_FLD_GLOBAL_ENABLE, field_id))
    {
        int32 ret;
        p_fea_ptp_global->global_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_PTP_GLOBAL_FLD_DEVICE_TYPE, field_id))
    {
        int32 ret;
        p_fea_ptp_global->device_type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_PTP_GLOBAL_FLD_RETRY_CNT, field_id))
    {
        int32 ret;
        p_fea_ptp_global->retry_cnt = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_PTP_GLOBAL_FLD_READY, field_id))
    {
        int32 ret;
        p_fea_ptp_global->ready = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_PTP_GLOBAL_FLD_BLOCKED, field_id))
    {
        int32 ret;
        p_fea_ptp_global->blocked = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_PTP_GLOBAL_FLD_TYPE, field_id))
    {
        int32 ret;
        p_fea_ptp_global->type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_PTP_GLOBAL_FLD_TX_IFINDEX, field_id))
    {
        int32 ret;
        p_fea_ptp_global->tx_ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_PTP_GLOBAL_FLD_TX_SEQ_ID, field_id))
    {
        int32 ret;
        p_fea_ptp_global->tx_seq_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_PTP_GLOBAL_FLD_INTR_COUNT, field_id))
    {
        int32 ret;
        p_fea_ptp_global->intr_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_PTP_GLOBAL_FLD_ACL_UPDATE_TIME_RANGE, field_id))
    {
        int32 ret;
        p_fea_ptp_global->acl_update_time_range = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_PTP_GLOBAL_FLD_HSRV_PTP_TIMER, field_id))
    {
        ctc_task_str2val(str, p_fea_ptp_global->hsrv_ptp_timer);
    }
    return PM_E_NONE;
}

int32
tbl_fea_ptp_global_table_parser(char** array, char* key_value,tbl_fea_ptp_global_t *p_fea_ptp_global)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_PTP_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_fea_ptp_global_key_str2val(key_value, p_fea_ptp_global));

    for(i=1; i<TBL_FEA_PTP_GLOBAL_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_fea_ptp_global_field_value_parser( array[j++], i, p_fea_ptp_global));
    }

    return PM_E_NONE;
}

int32
tbl_fea_ptp_global_dump_one(tbl_fea_ptp_global_t *p_fea_ptp_global, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_PTP_GLOBAL);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_FEA_PTP_GLOBAL_FLD_GLOBAL_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_PTP_GLOBAL_FLD_GLOBAL_ENABLE].name,
            p_fea_ptp_global->global_enable);
    }
    if (FLD_MATCH(TBL_FEA_PTP_GLOBAL_FLD_DEVICE_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_PTP_GLOBAL_FLD_DEVICE_TYPE].name,
            p_fea_ptp_global->device_type);
    }
    if (FLD_MATCH(TBL_FEA_PTP_GLOBAL_FLD_RETRY_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_PTP_GLOBAL_FLD_RETRY_CNT].name,
            p_fea_ptp_global->retry_cnt);
    }
    if (FLD_MATCH(TBL_FEA_PTP_GLOBAL_FLD_READY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_PTP_GLOBAL_FLD_READY].name,
            p_fea_ptp_global->ready);
    }
    if (FLD_MATCH(TBL_FEA_PTP_GLOBAL_FLD_BLOCKED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_PTP_GLOBAL_FLD_BLOCKED].name,
            p_fea_ptp_global->blocked);
    }
    if (FLD_MATCH(TBL_FEA_PTP_GLOBAL_FLD_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_PTP_GLOBAL_FLD_TYPE].name,
            p_fea_ptp_global->type);
    }
    if (FLD_MATCH(TBL_FEA_PTP_GLOBAL_FLD_TX_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_PTP_GLOBAL_FLD_TX_IFINDEX].name,
            p_fea_ptp_global->tx_ifindex);
    }
    if (FLD_MATCH(TBL_FEA_PTP_GLOBAL_FLD_TX_SEQ_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_PTP_GLOBAL_FLD_TX_SEQ_ID].name,
            p_fea_ptp_global->tx_seq_id);
    }
    if (FLD_MATCH(TBL_FEA_PTP_GLOBAL_FLD_INTR_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_PTP_GLOBAL_FLD_INTR_COUNT].name,
            p_fea_ptp_global->intr_count);
    }
    if (FLD_MATCH(TBL_FEA_PTP_GLOBAL_FLD_ACL_UPDATE_TIME_RANGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_PTP_GLOBAL_FLD_ACL_UPDATE_TIME_RANGE].name,
            p_fea_ptp_global->acl_update_time_range);
    }
    if (FLD_MATCH(TBL_FEA_PTP_GLOBAL_FLD_HSRV_PTP_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_FEA_PTP_GLOBAL_FLD_HSRV_PTP_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_fea_ptp_global->hsrv_ptp_timer));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_fea_ptp_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_fea_ptp_global_t *p_db_fea_ptp_global = _g_p_tbl_fea_ptp_global;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_fea_ptp_global)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_fea_ptp_global, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_fea_ptp_global_t*
tbl_fea_ptp_global_init_fea_ptp_global()
{
    _g_p_tbl_fea_ptp_global = XCALLOC(MEM_TBL_FEA_PTP_GLOBAL, sizeof(tbl_fea_ptp_global_t));
    return _g_p_tbl_fea_ptp_global;
}

