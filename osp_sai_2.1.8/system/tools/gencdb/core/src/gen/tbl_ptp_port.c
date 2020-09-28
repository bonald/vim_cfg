
#include "proto.h"
#include "gen/tbl_ptp_port_define.h"
#include "gen/tbl_ptp_port.h"
#include "cdb_data_cmp.h"

tbl_ptp_port_master_t *_g_p_tbl_ptp_port_master = NULL;

static uint32
_tbl_ptp_port_hash_make(tbl_ptp_port_t *p_port)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_port->key;
    for (index = 0; index < sizeof(p_port->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_ptp_port_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_ptp_port_t *p_port1 = (tbl_ptp_port_t*)p_arg1;
    tbl_ptp_port_t *p_port2 = (tbl_ptp_port_t*)p_arg2;

    if (0 == sal_memcmp(&p_port1->key, &p_port2->key, sizeof(tbl_ptp_port_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_ptp_port_add_ptp_port_sync(tbl_ptp_port_t *p_port, uint32 sync)
{
    tbl_ptp_port_master_t *p_master = _g_p_tbl_ptp_port_master;
    tbl_ptp_port_t *p_db_port = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_ptp_port_get_ptp_port(&p_port->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_port = XCALLOC(MEM_TBL_PTP_PORT, sizeof(tbl_ptp_port_t));
    if (NULL == p_db_port)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_port, p_port, sizeof(tbl_ptp_port_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->ptp_port_hash, (void*)p_db_port, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->ptp_port_list, p_db_port);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_PTP_PORT, p_db_port);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_PTP_PORT, p_db_port);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ptp_port_del_ptp_port_sync(tbl_ptp_port_key_t *p_port_key, uint32 sync)
{
    tbl_ptp_port_master_t *p_master = _g_p_tbl_ptp_port_master;
    tbl_ptp_port_t *p_db_port = NULL;

    /* 1. lookup entry exist */
    p_db_port = tbl_ptp_port_get_ptp_port(p_port_key);
    if (NULL == p_db_port)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_PTP_PORT, p_db_port);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_PTP_PORT, p_db_port);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->ptp_port_hash, (void*)p_db_port);
    ctclib_slistnode_delete(p_master->ptp_port_list, p_db_port);

    /* 4. free db entry */
    XFREE(MEM_TBL_PTP_PORT, p_db_port);

    return PM_E_NONE;
}

int32
tbl_ptp_port_set_ptp_port_field_sync(tbl_ptp_port_t *p_port, tbl_ptp_port_field_id_t field_id, uint32 sync)
{
    tbl_ptp_port_t *p_db_port = NULL;

    /* 1. lookup entry exist */
    p_db_port = tbl_ptp_port_get_ptp_port(&p_port->key);
    if (NULL == p_db_port)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_PTP_PORT_FLD_IFINDEX:
        p_db_port->ifindex = p_port->ifindex;
        break;

    case TBL_PTP_PORT_FLD_HSRV_PATH_DELAY:
        p_db_port->hsrv_path_delay = p_port->hsrv_path_delay;
        break;

    case TBL_PTP_PORT_FLD_PORT_DS:
        sal_memcpy(&p_db_port->port_ds, &p_port->port_ds, sizeof(p_port->port_ds));
        break;

    case TBL_PTP_PORT_FLD_PORT_ADDR:
        sal_memcpy(&p_db_port->port_addr, &p_port->port_addr, sizeof(p_port->port_addr));
        break;

    case TBL_PTP_PORT_FLD_ASYMMETRY_CORRECTION:
        sal_memcpy(&p_db_port->asymmetry_correction, &p_port->asymmetry_correction, sizeof(p_port->asymmetry_correction));
        break;

    case TBL_PTP_PORT_FLD_INGRESS_LATENCY:
        p_db_port->ingress_latency = p_port->ingress_latency;
        break;

    case TBL_PTP_PORT_FLD_EGRESS_LATENCY:
        p_db_port->egress_latency = p_port->egress_latency;
        break;

    case TBL_PTP_PORT_FLD_DUMP_FLAG:
        sal_memcpy(&p_db_port->dump_flag, &p_port->dump_flag, sizeof(p_port->dump_flag));
        break;

    case TBL_PTP_PORT_FLD_ENABLE:
        p_db_port->enable = p_port->enable;
        break;

    case TBL_PTP_PORT_FLD_LAG_ID:
        p_db_port->lag_id = p_port->lag_id;
        break;

    case TBL_PTP_PORT_FLD_LINK_UP:
        p_db_port->link_up = p_port->link_up;
        break;

    case TBL_PTP_PORT_FLD_DROP_ANNOUNCE:
        p_db_port->drop_announce = p_port->drop_announce;
        break;

    case TBL_PTP_PORT_FLD_PACKET_STATS:
        sal_memcpy(&p_db_port->packet_stats, &p_port->packet_stats, sizeof(p_port->packet_stats));
        break;

    case TBL_PTP_PORT_FLD_VLAN_INFO:
        sal_memcpy(&p_db_port->vlan_info, &p_port->vlan_info, sizeof(p_port->vlan_info));
        break;

    case TBL_PTP_PORT_FLD_RECOMMENDED_STATE:
        p_db_port->recommended_state = p_port->recommended_state;
        break;

    case TBL_PTP_PORT_FLD_BMC_CODE:
        p_db_port->bmc_code = p_port->bmc_code;
        break;

    case TBL_PTP_PORT_FLD_BMC_EXPIRES:
        p_db_port->bmc_expires = p_port->bmc_expires;
        break;

    case TBL_PTP_PORT_FLD_SLAVE_CHANGE_MASTER:
        p_db_port->slave_change_master = p_port->slave_change_master;
        break;

    case TBL_PTP_PORT_FLD_SYNCHRONIZATION_FAULT:
        p_db_port->synchronization_fault = p_port->synchronization_fault;
        break;

    case TBL_PTP_PORT_FLD_MASTER_CLOCK_SELECTED:
        p_db_port->master_clock_selected = p_port->master_clock_selected;
        break;

    case TBL_PTP_PORT_FLD_PTP_SEQ_ID:
        p_db_port->ptp_seq_id = p_port->ptp_seq_id;
        break;

    case TBL_PTP_PORT_FLD_UNCALIBRATED_UPDATE_OFFSET_TIMES:
        p_db_port->uncalibrated_update_offset_times = p_port->uncalibrated_update_offset_times;
        break;

    case TBL_PTP_PORT_FLD_UNCALIBRATED_UPDATE_LIMIT_TIMES:
        p_db_port->uncalibrated_update_limit_times = p_port->uncalibrated_update_limit_times;
        break;

    case TBL_PTP_PORT_FLD_P:
        sal_memcpy(&p_db_port->p, &p_port->p, sizeof(p_port->p));
        break;

    case TBL_PTP_PORT_FLD_C:
        sal_memcpy(&p_db_port->c, &p_port->c, sizeof(p_port->c));
        break;

    case TBL_PTP_PORT_FLD_PTM_ANNOUNCE_TIMER:
        p_db_port->ptm_announce_timer = p_port->ptm_announce_timer;
        break;

    case TBL_PTP_PORT_FLD_PTM_SYNC_TIMER:
        p_db_port->ptm_sync_timer = p_port->ptm_sync_timer;
        break;

    case TBL_PTP_PORT_FLD_PTM_DELAY_TIMER:
        p_db_port->ptm_delay_timer = p_port->ptm_delay_timer;
        break;

    case TBL_PTP_PORT_FLD_PTM_PDELAY_TIMER:
        p_db_port->ptm_pdelay_timer = p_port->ptm_pdelay_timer;
        break;

    case TBL_PTP_PORT_FLD_PTM_ANNOUNCE_RECEIPT_TIMER:
        p_db_port->ptm_announce_receipt_timer = p_port->ptm_announce_receipt_timer;
        break;

    case TBL_PTP_PORT_FLD_PTM_QUALIFICATION_TIMER:
        p_db_port->ptm_qualification_timer = p_port->ptm_qualification_timer;
        break;

    case TBL_PTP_PORT_FLD_P_TIMER_FAULTY_RECOVER_TIMER:
        p_db_port->p_timer_faulty_recover_timer = p_port->p_timer_faulty_recover_timer;
        break;

    case TBL_PTP_PORT_FLD_MSG_PROC_OPS:
        sal_memcpy(&p_db_port->msg_proc_ops, &p_port->msg_proc_ops, sizeof(p_port->msg_proc_ops));
        break;

    case TBL_PTP_PORT_FLD_MAX:
        sal_memcpy(p_db_port, p_port, sizeof(tbl_ptp_port_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_PTP_PORT, field_id, p_db_port);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_PTP_PORT, field_id, p_db_port);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ptp_port_add_ptp_port(tbl_ptp_port_t *p_port)
{
    return tbl_ptp_port_add_ptp_port_sync(p_port, TRUE);
}

int32
tbl_ptp_port_del_ptp_port(tbl_ptp_port_key_t *p_port_key)
{
    return tbl_ptp_port_del_ptp_port_sync(p_port_key, TRUE);
}

int32
tbl_ptp_port_set_ptp_port_field(tbl_ptp_port_t *p_port, tbl_ptp_port_field_id_t field_id)
{
    return tbl_ptp_port_set_ptp_port_field_sync(p_port, field_id, TRUE);
}

tbl_ptp_port_t*
tbl_ptp_port_get_ptp_port(tbl_ptp_port_key_t *p_port_key)
{
    tbl_ptp_port_master_t *p_master = _g_p_tbl_ptp_port_master;
    tbl_ptp_port_t lkp;

    sal_memcpy(&lkp.key, p_port_key, sizeof(tbl_ptp_port_key_t));
    return ctclib_hash_lookup(p_master->ptp_port_hash, &lkp);
}

char*
tbl_ptp_port_key_val2str(tbl_ptp_port_t *p_port, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_port->key.name);
    return str;
}

int32
tbl_ptp_port_key_str2val(char *str, tbl_ptp_port_t *p_port)
{
    int32 ret = 0;

    sal_strcpy(p_port->key.name, str);
    return ret;
}

char*
tbl_ptp_port_key_name_dump(tbl_ptp_port_t *p_port, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_PTP_PORT);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_port->key.name);
    return str;
}

char*
tbl_ptp_port_key_value_dump(tbl_ptp_port_t *p_port, char *str)
{
    sal_sprintf(str, "%s", p_port->key.name);
    return str;
}

char*
tbl_ptp_port_field_name_dump(tbl_ptp_port_t *p_port, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_PTP_PORT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_PTP_PORT_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_ptp_port_field_value_dump(tbl_ptp_port_t *p_port, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_PTP_PORT_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_port->key.name);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_port->ifindex);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_HSRV_PATH_DELAY, field_id))
    {
        sal_sprintf(str, "%"PRId64, p_port->hsrv_path_delay);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_PORT_DS, field_id))
    {
        cdb_ptp_port_val2str(str, MAX_CMD_STR_LEN, &p_port->port_ds);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_PORT_ADDR, field_id))
    {
        cdb_glb_ptp_port_address_val2str(str, MAX_CMD_STR_LEN, &p_port->port_addr);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_ASYMMETRY_CORRECTION, field_id))
    {
        cdb_glb_ptp_timeinterval_val2str(str, MAX_CMD_STR_LEN, &p_port->asymmetry_correction);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_INGRESS_LATENCY, field_id))
    {
        sal_sprintf(str, "%d", p_port->ingress_latency);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_EGRESS_LATENCY, field_id))
    {
        sal_sprintf(str, "%d", p_port->egress_latency);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_DUMP_FLAG, field_id))
    {
        cdb_ptp_dump_flag_val2str(str, MAX_CMD_STR_LEN, &p_port->dump_flag);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_port->enable);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_LAG_ID, field_id))
    {
        sal_sprintf(str, "%u", p_port->lag_id);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_LINK_UP, field_id))
    {
        sal_sprintf(str, "%u", p_port->link_up);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_DROP_ANNOUNCE, field_id))
    {
        sal_sprintf(str, "%u", p_port->drop_announce);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_PACKET_STATS, field_id))
    {
        cdb_ptp_port_stats_val2str(str, MAX_CMD_STR_LEN, &p_port->packet_stats);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_VLAN_INFO, field_id))
    {
        cdb_ptp_vlan_info_val2str(str, MAX_CMD_STR_LEN, &p_port->vlan_info);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_RECOMMENDED_STATE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(ptp_state_strs, PTP_STATE_MAX, p_port->recommended_state));
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_BMC_CODE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(ptp_bmc_code_strs, PTP_BMC_CODE_MAX, p_port->bmc_code));
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_BMC_EXPIRES, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(ptp_bmc_expires_strs, PTP_BMC_EXPIRES_MAX, p_port->bmc_expires));
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_SLAVE_CHANGE_MASTER, field_id))
    {
        sal_sprintf(str, "%u", p_port->slave_change_master);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_SYNCHRONIZATION_FAULT, field_id))
    {
        sal_sprintf(str, "%u", p_port->synchronization_fault);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_MASTER_CLOCK_SELECTED, field_id))
    {
        sal_sprintf(str, "%u", p_port->master_clock_selected);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_PTP_SEQ_ID, field_id))
    {
        sal_sprintf(str, "%u", p_port->ptp_seq_id);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_UNCALIBRATED_UPDATE_OFFSET_TIMES, field_id))
    {
        sal_sprintf(str, "%u", p_port->uncalibrated_update_offset_times);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_UNCALIBRATED_UPDATE_LIMIT_TIMES, field_id))
    {
        sal_sprintf(str, "%u", p_port->uncalibrated_update_limit_times);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_P, field_id))
    {
        cdb_ptp_port_p_val2str(str, MAX_CMD_STR_LEN, &p_port->p);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_C, field_id))
    {
        cdb_ptp_port_c_val2str(str, MAX_CMD_STR_LEN, &p_port->c);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_PTM_ANNOUNCE_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_port->ptm_announce_timer);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_PTM_SYNC_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_port->ptm_sync_timer);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_PTM_DELAY_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_port->ptm_delay_timer);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_PTM_PDELAY_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_port->ptm_pdelay_timer);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_PTM_ANNOUNCE_RECEIPT_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_port->ptm_announce_receipt_timer);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_PTM_QUALIFICATION_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_port->ptm_qualification_timer);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_P_TIMER_FAULTY_RECOVER_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_port->p_timer_faulty_recover_timer);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_MSG_PROC_OPS, field_id))
    {
        cdb_ptp_msg_proc_ops_val2str(str, MAX_CMD_STR_LEN, &p_port->msg_proc_ops);
    }
    return str;
}

char**
tbl_ptp_port_table_dump(tbl_ptp_port_t *p_port, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_ptp_port_key_name_dump(p_port, buf));
    for(i=1; i<TBL_PTP_PORT_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_ptp_port_field_name_dump(p_port, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_ptp_port_field_value_dump(p_port, i, buf));
    }
    return str;
}

int32
tbl_ptp_port_field_value_parser(char *str, int32 field_id, tbl_ptp_port_t *p_port)
{
    if (FLD_MATCH(TBL_PTP_PORT_FLD_KEY, field_id))
    {
        sal_strcpy(p_port->key.name, str);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_IFINDEX, field_id))
    {
        int32 ret;
        p_port->ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_HSRV_PATH_DELAY, field_id))
    {
        int32 ret;
        p_port->hsrv_path_delay = cdb_int64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_PORT_DS, field_id))
    {
        cdb_ptp_port_str2val(str, &p_port->port_ds);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_PORT_ADDR, field_id))
    {
        cdb_glb_ptp_port_address_str2val(str, &p_port->port_addr);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_ASYMMETRY_CORRECTION, field_id))
    {
        cdb_glb_ptp_timeinterval_str2val(str, &p_port->asymmetry_correction);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_INGRESS_LATENCY, field_id))
    {
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_EGRESS_LATENCY, field_id))
    {
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_DUMP_FLAG, field_id))
    {
        cdb_ptp_dump_flag_str2val(str, &p_port->dump_flag);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_ENABLE, field_id))
    {
        int32 ret;
        p_port->enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_LAG_ID, field_id))
    {
        int32 ret;
        p_port->lag_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_LINK_UP, field_id))
    {
        int32 ret;
        p_port->link_up = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_DROP_ANNOUNCE, field_id))
    {
        int32 ret;
        p_port->drop_announce = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_PACKET_STATS, field_id))
    {
        cdb_ptp_port_stats_str2val(str, &p_port->packet_stats);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_VLAN_INFO, field_id))
    {
        cdb_ptp_vlan_info_str2val(str, &p_port->vlan_info);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_RECOMMENDED_STATE, field_id))
    {
        cdb_enum_str2val(ptp_state_strs, PTP_STATE_MAX, str);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_BMC_CODE, field_id))
    {
        cdb_enum_str2val(ptp_bmc_code_strs, PTP_BMC_CODE_MAX, str);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_BMC_EXPIRES, field_id))
    {
        cdb_enum_str2val(ptp_bmc_expires_strs, PTP_BMC_EXPIRES_MAX, str);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_SLAVE_CHANGE_MASTER, field_id))
    {
        int32 ret;
        p_port->slave_change_master = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_SYNCHRONIZATION_FAULT, field_id))
    {
        int32 ret;
        p_port->synchronization_fault = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_MASTER_CLOCK_SELECTED, field_id))
    {
        int32 ret;
        p_port->master_clock_selected = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_PTP_SEQ_ID, field_id))
    {
        int32 ret;
        p_port->ptp_seq_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_UNCALIBRATED_UPDATE_OFFSET_TIMES, field_id))
    {
        int32 ret;
        p_port->uncalibrated_update_offset_times = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_UNCALIBRATED_UPDATE_LIMIT_TIMES, field_id))
    {
        int32 ret;
        p_port->uncalibrated_update_limit_times = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_P, field_id))
    {
        cdb_ptp_port_p_str2val(str, &p_port->p);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_C, field_id))
    {
        cdb_ptp_port_c_str2val(str, &p_port->c);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_PTM_ANNOUNCE_TIMER, field_id))
    {
        ctc_task_str2val(str, p_port->ptm_announce_timer);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_PTM_SYNC_TIMER, field_id))
    {
        ctc_task_str2val(str, p_port->ptm_sync_timer);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_PTM_DELAY_TIMER, field_id))
    {
        ctc_task_str2val(str, p_port->ptm_delay_timer);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_PTM_PDELAY_TIMER, field_id))
    {
        ctc_task_str2val(str, p_port->ptm_pdelay_timer);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_PTM_ANNOUNCE_RECEIPT_TIMER, field_id))
    {
        ctc_task_str2val(str, p_port->ptm_announce_receipt_timer);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_PTM_QUALIFICATION_TIMER, field_id))
    {
        ctc_task_str2val(str, p_port->ptm_qualification_timer);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_P_TIMER_FAULTY_RECOVER_TIMER, field_id))
    {
        ctc_task_str2val(str, p_port->p_timer_faulty_recover_timer);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_MSG_PROC_OPS, field_id))
    {
        cdb_ptp_msg_proc_ops_str2val(str, &p_port->msg_proc_ops);
    }
    return PM_E_NONE;
}

int32
tbl_ptp_port_table_parser(char** array, char* key_value,tbl_ptp_port_t *p_port)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_PTP_PORT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_ptp_port_key_str2val(key_value, p_port));

    for(i=1; i<TBL_PTP_PORT_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_ptp_port_field_value_parser( array[j++], i, p_port));
    }

    return PM_E_NONE;
}

int32
tbl_ptp_port_dump_one(tbl_ptp_port_t *p_port, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_PTP_PORT);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_PTP_PORT_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PTP_PORT_FLD_KEY].name,
            p_port->key.name);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PTP_PORT_FLD_IFINDEX].name,
            p_port->ifindex);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_HSRV_PATH_DELAY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRId64, p_tbl_info->field[TBL_PTP_PORT_FLD_HSRV_PATH_DELAY].name,
            p_port->hsrv_path_delay);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_PORT_DS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PTP_PORT_FLD_PORT_DS].name, 
            cdb_ptp_port_val2str(buf, MAX_CMD_STR_LEN, &p_port->port_ds));
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_PORT_ADDR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PTP_PORT_FLD_PORT_ADDR].name, 
            cdb_glb_ptp_port_address_val2str(buf, MAX_CMD_STR_LEN, &p_port->port_addr));
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_ASYMMETRY_CORRECTION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PTP_PORT_FLD_ASYMMETRY_CORRECTION].name, 
            cdb_glb_ptp_timeinterval_val2str(buf, MAX_CMD_STR_LEN, &p_port->asymmetry_correction));
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_INGRESS_LATENCY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_PTP_PORT_FLD_INGRESS_LATENCY].name,
            p_port->ingress_latency);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_EGRESS_LATENCY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_PTP_PORT_FLD_EGRESS_LATENCY].name,
            p_port->egress_latency);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_DUMP_FLAG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PTP_PORT_FLD_DUMP_FLAG].name, 
            cdb_ptp_dump_flag_val2str(buf, MAX_CMD_STR_LEN, &p_port->dump_flag));
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PTP_PORT_FLD_ENABLE].name,
            p_port->enable);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_LAG_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PTP_PORT_FLD_LAG_ID].name,
            p_port->lag_id);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_LINK_UP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PTP_PORT_FLD_LINK_UP].name,
            p_port->link_up);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_DROP_ANNOUNCE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PTP_PORT_FLD_DROP_ANNOUNCE].name,
            p_port->drop_announce);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_PACKET_STATS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PTP_PORT_FLD_PACKET_STATS].name, 
            cdb_ptp_port_stats_val2str(buf, MAX_CMD_STR_LEN, &p_port->packet_stats));
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_VLAN_INFO, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PTP_PORT_FLD_VLAN_INFO].name, 
            cdb_ptp_vlan_info_val2str(buf, MAX_CMD_STR_LEN, &p_port->vlan_info));
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_RECOMMENDED_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PTP_PORT_FLD_RECOMMENDED_STATE].name, 
            cdb_enum_val2str(ptp_state_strs, PTP_STATE_MAX, p_port->recommended_state));
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_BMC_CODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PTP_PORT_FLD_BMC_CODE].name, 
            cdb_enum_val2str(ptp_bmc_code_strs, PTP_BMC_CODE_MAX, p_port->bmc_code));
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_BMC_EXPIRES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PTP_PORT_FLD_BMC_EXPIRES].name, 
            cdb_enum_val2str(ptp_bmc_expires_strs, PTP_BMC_EXPIRES_MAX, p_port->bmc_expires));
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_SLAVE_CHANGE_MASTER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PTP_PORT_FLD_SLAVE_CHANGE_MASTER].name,
            p_port->slave_change_master);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_SYNCHRONIZATION_FAULT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PTP_PORT_FLD_SYNCHRONIZATION_FAULT].name,
            p_port->synchronization_fault);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_MASTER_CLOCK_SELECTED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PTP_PORT_FLD_MASTER_CLOCK_SELECTED].name,
            p_port->master_clock_selected);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_PTP_SEQ_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PTP_PORT_FLD_PTP_SEQ_ID].name,
            p_port->ptp_seq_id);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_UNCALIBRATED_UPDATE_OFFSET_TIMES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PTP_PORT_FLD_UNCALIBRATED_UPDATE_OFFSET_TIMES].name,
            p_port->uncalibrated_update_offset_times);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_UNCALIBRATED_UPDATE_LIMIT_TIMES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_PTP_PORT_FLD_UNCALIBRATED_UPDATE_LIMIT_TIMES].name,
            p_port->uncalibrated_update_limit_times);
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_P, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PTP_PORT_FLD_P].name, 
            cdb_ptp_port_p_val2str(buf, MAX_CMD_STR_LEN, &p_port->p));
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_C, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PTP_PORT_FLD_C].name, 
            cdb_ptp_port_c_val2str(buf, MAX_CMD_STR_LEN, &p_port->c));
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_PTM_ANNOUNCE_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PTP_PORT_FLD_PTM_ANNOUNCE_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_port->ptm_announce_timer));
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_PTM_SYNC_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PTP_PORT_FLD_PTM_SYNC_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_port->ptm_sync_timer));
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_PTM_DELAY_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PTP_PORT_FLD_PTM_DELAY_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_port->ptm_delay_timer));
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_PTM_PDELAY_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PTP_PORT_FLD_PTM_PDELAY_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_port->ptm_pdelay_timer));
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_PTM_ANNOUNCE_RECEIPT_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PTP_PORT_FLD_PTM_ANNOUNCE_RECEIPT_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_port->ptm_announce_receipt_timer));
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_PTM_QUALIFICATION_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PTP_PORT_FLD_PTM_QUALIFICATION_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_port->ptm_qualification_timer));
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_P_TIMER_FAULTY_RECOVER_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PTP_PORT_FLD_P_TIMER_FAULTY_RECOVER_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_port->p_timer_faulty_recover_timer));
    }
    if (FLD_MATCH(TBL_PTP_PORT_FLD_MSG_PROC_OPS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_PTP_PORT_FLD_MSG_PROC_OPS].name, 
            cdb_ptp_msg_proc_ops_val2str(buf, MAX_CMD_STR_LEN, &p_port->msg_proc_ops));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_ptp_port_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_ptp_port_master_t *p_master = _g_p_tbl_ptp_port_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_ptp_port_t *p_db_port = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->ptp_port_list, p_db_port, listnode, next)
    {
        rc |= fn(p_db_port, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_ptp_port_master_t*
tbl_ptp_port_get_master()
{
    return _g_p_tbl_ptp_port_master;
}

tbl_ptp_port_master_t*
tbl_ptp_port_init_ptp_port()
{
    _g_p_tbl_ptp_port_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_ptp_port_master_t));
    _g_p_tbl_ptp_port_master->ptp_port_hash = ctclib_hash_create(_tbl_ptp_port_hash_make, _tbl_ptp_port_hash_cmp);
    _g_p_tbl_ptp_port_master->ptp_port_list = ctclib_slist_create(tbl_ptp_port_cmp, NULL);
    return _g_p_tbl_ptp_port_master;
}

tbl_ptp_port_t*
tbl_ptp_port_get_ptp_port_by_name(char* ifname)
{
    tbl_ptp_port_key_t ptp_port_key;

    sal_memset(&ptp_port_key, 0, sizeof(ptp_port_key));
    sal_memcpy(ptp_port_key.name, ifname, IFNAME_SIZE);
    
    return tbl_ptp_port_get_ptp_port(&ptp_port_key);
}
