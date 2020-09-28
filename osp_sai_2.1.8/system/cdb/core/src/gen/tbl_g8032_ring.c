
#include "proto.h"
#include "gen/tbl_g8032_ring_define.h"
#include "gen/tbl_g8032_ring.h"
#include "cdb_data_cmp.h"

tbl_g8032_ring_master_t *_g_p_tbl_g8032_ring_master = NULL;

static uint32
_tbl_g8032_ring_hash_make(tbl_g8032_ring_t *p_g8032_ring)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_g8032_ring->key;
    for (index = 0; index < sizeof(p_g8032_ring->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_g8032_ring_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_g8032_ring_t *p_g8032_ring1 = (tbl_g8032_ring_t*)p_arg1;
    tbl_g8032_ring_t *p_g8032_ring2 = (tbl_g8032_ring_t*)p_arg2;

    if (0 == sal_memcmp(&p_g8032_ring1->key, &p_g8032_ring2->key, sizeof(tbl_g8032_ring_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_g8032_ring_add_g8032_ring_sync(tbl_g8032_ring_t *p_g8032_ring, uint32 sync)
{
    tbl_g8032_ring_master_t *p_master = _g_p_tbl_g8032_ring_master;
    tbl_g8032_ring_t *p_db_g8032_ring = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_g8032_ring_get_g8032_ring(&p_g8032_ring->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_g8032_ring = XCALLOC(MEM_TBL_G8032_RING, sizeof(tbl_g8032_ring_t));
    if (NULL == p_db_g8032_ring)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_g8032_ring, p_g8032_ring, sizeof(tbl_g8032_ring_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->g8032_ring_hash, (void*)p_db_g8032_ring, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->g8032_ring_list, p_db_g8032_ring);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_G8032_RING, p_db_g8032_ring);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_G8032_RING, p_db_g8032_ring);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_g8032_ring_del_g8032_ring_sync(tbl_g8032_ring_key_t *p_g8032_ring_key, uint32 sync)
{
    tbl_g8032_ring_master_t *p_master = _g_p_tbl_g8032_ring_master;
    tbl_g8032_ring_t *p_db_g8032_ring = NULL;

    /* 1. lookup entry exist */
    p_db_g8032_ring = tbl_g8032_ring_get_g8032_ring(p_g8032_ring_key);
    if (NULL == p_db_g8032_ring)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_G8032_RING, p_db_g8032_ring);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_G8032_RING, p_db_g8032_ring);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->g8032_ring_hash, (void*)p_db_g8032_ring);
    ctclib_slistnode_delete(p_master->g8032_ring_list, p_db_g8032_ring);

    /* 4. free db entry */
    XFREE(MEM_TBL_G8032_RING, p_db_g8032_ring);

    return PM_E_NONE;
}

int32
tbl_g8032_ring_set_g8032_ring_field_sync(tbl_g8032_ring_t *p_g8032_ring, tbl_g8032_ring_field_id_t field_id, uint32 sync)
{
    tbl_g8032_ring_t *p_db_g8032_ring = NULL;

    /* 1. lookup entry exist */
    p_db_g8032_ring = tbl_g8032_ring_get_g8032_ring(&p_g8032_ring->key);
    if (NULL == p_db_g8032_ring)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_G8032_RING_FLD_RPL_OWNER:
        p_db_g8032_ring->rpl_owner = p_g8032_ring->rpl_owner;
        break;

    case TBL_G8032_RING_FLD_RPL_NEIGHBOR:
        p_db_g8032_ring->rpl_neighbor = p_g8032_ring->rpl_neighbor;
        break;

    case TBL_G8032_RING_FLD_RPL_NEXT_NEIGHBOR:
        p_db_g8032_ring->rpl_next_neighbor = p_g8032_ring->rpl_next_neighbor;
        break;

    case TBL_G8032_RING_FLD_RING_LEVEL_ID:
        p_db_g8032_ring->ring_level_id = p_g8032_ring->ring_level_id;
        break;

    case TBL_G8032_RING_FLD_PROTECTION_EN:
        p_db_g8032_ring->protection_en = p_g8032_ring->protection_en;
        break;

    case TBL_G8032_RING_FLD_CONFIG_SYNC:
        p_db_g8032_ring->config_sync = p_g8032_ring->config_sync;
        break;

    case TBL_G8032_RING_FLD_APS_PROTECTION_EN:
        p_db_g8032_ring->aps_protection_en = p_g8032_ring->aps_protection_en;
        break;

    case TBL_G8032_RING_FLD_PROTECTION_TIME:
        p_db_g8032_ring->protection_time = p_g8032_ring->protection_time;
        break;

    case TBL_G8032_RING_FLD_MODE_REVERTIVE:
        p_db_g8032_ring->mode_revertive = p_g8032_ring->mode_revertive;
        break;

    case TBL_G8032_RING_FLD_VIRTUAL_CHANNEL:
        p_db_g8032_ring->virtual_channel = p_g8032_ring->virtual_channel;
        break;

    case TBL_G8032_RING_FLD_G8032_MAJOR_RING_ID:
        p_db_g8032_ring->g8032_major_ring_id = p_g8032_ring->g8032_major_ring_id;
        break;

    case TBL_G8032_RING_FLD_IS_SUB_RING:
        p_db_g8032_ring->is_sub_ring = p_g8032_ring->is_sub_ring;
        break;

    case TBL_G8032_RING_FLD_INSTANCE_COUNT:
        p_db_g8032_ring->instance_count = p_g8032_ring->instance_count;
        break;

    case TBL_G8032_RING_FLD_INSTANCE:
        sal_memcpy(p_db_g8032_ring->instance, p_g8032_ring->instance, sizeof(p_g8032_ring->instance));
        break;

    case TBL_G8032_RING_FLD_SUB_RING_ID:
        sal_memcpy(p_db_g8032_ring->sub_ring_id, p_g8032_ring->sub_ring_id, sizeof(p_g8032_ring->sub_ring_id));
        break;

    case TBL_G8032_RING_FLD_IS_LOCAL_FS:
        p_db_g8032_ring->is_local_fs = p_g8032_ring->is_local_fs;
        break;

    case TBL_G8032_RING_FLD_IS_LOCAL_MS:
        p_db_g8032_ring->is_local_ms = p_g8032_ring->is_local_ms;
        break;

    case TBL_G8032_RING_FLD_EAST_PORT_IFINDEX:
        p_db_g8032_ring->east_port_ifindex = p_g8032_ring->east_port_ifindex;
        break;

    case TBL_G8032_RING_FLD_EAST_PORT_IS_RPL:
        p_db_g8032_ring->east_port_is_rpl = p_g8032_ring->east_port_is_rpl;
        break;

    case TBL_G8032_RING_FLD_EAST_PORT_IS_RPL_NEIGHBOR:
        p_db_g8032_ring->east_port_is_rpl_neighbor = p_g8032_ring->east_port_is_rpl_neighbor;
        break;

    case TBL_G8032_RING_FLD_EAST_PORT_IS_RPL_NEXT_NEIGHBOR:
        p_db_g8032_ring->east_port_is_rpl_next_neighbor = p_g8032_ring->east_port_is_rpl_next_neighbor;
        break;

    case TBL_G8032_RING_FLD_EAST_PORT_IS_FORCE_SWITCH:
        p_db_g8032_ring->east_port_is_force_switch = p_g8032_ring->east_port_is_force_switch;
        break;

    case TBL_G8032_RING_FLD_EAST_PORT_IS_MANUAL_SWITCH:
        p_db_g8032_ring->east_port_is_manual_switch = p_g8032_ring->east_port_is_manual_switch;
        break;

    case TBL_G8032_RING_FLD_EAST_PORT_IS_BLOCKED:
        p_db_g8032_ring->east_port_is_blocked = p_g8032_ring->east_port_is_blocked;
        break;

    case TBL_G8032_RING_FLD_EAST_PORT_FLUSH_FDB:
        p_db_g8032_ring->east_port_flush_fdb = p_g8032_ring->east_port_flush_fdb;
        break;

    case TBL_G8032_RING_FLD_EAST_PORT_IS_FAIL:
        p_db_g8032_ring->east_port_is_fail = p_g8032_ring->east_port_is_fail;
        break;

    case TBL_G8032_RING_FLD_EAST_PORT_REC_NODE_ID:
        sal_memcpy(p_db_g8032_ring->east_port_rec_node_id, p_g8032_ring->east_port_rec_node_id, sizeof(p_g8032_ring->east_port_rec_node_id));
        break;

    case TBL_G8032_RING_FLD_EAST_PORT_REC_BPR:
        p_db_g8032_ring->east_port_rec_bpr = p_g8032_ring->east_port_rec_bpr;
        break;

    case TBL_G8032_RING_FLD_WEST_PORT_IFINDEX:
        p_db_g8032_ring->west_port_ifindex = p_g8032_ring->west_port_ifindex;
        break;

    case TBL_G8032_RING_FLD_WEST_PORT_IS_RPL:
        p_db_g8032_ring->west_port_is_rpl = p_g8032_ring->west_port_is_rpl;
        break;

    case TBL_G8032_RING_FLD_WEST_PORT_IS_RPL_NEIGHBOR:
        p_db_g8032_ring->west_port_is_rpl_neighbor = p_g8032_ring->west_port_is_rpl_neighbor;
        break;

    case TBL_G8032_RING_FLD_WEST_PORT_IS_RPL_NEXT_NEIGHBOR:
        p_db_g8032_ring->west_port_is_rpl_next_neighbor = p_g8032_ring->west_port_is_rpl_next_neighbor;
        break;

    case TBL_G8032_RING_FLD_WEST_PORT_IS_FORCE_SWITCH:
        p_db_g8032_ring->west_port_is_force_switch = p_g8032_ring->west_port_is_force_switch;
        break;

    case TBL_G8032_RING_FLD_WEST_PORT_IS_MANUAL_SWITCH:
        p_db_g8032_ring->west_port_is_manual_switch = p_g8032_ring->west_port_is_manual_switch;
        break;

    case TBL_G8032_RING_FLD_WEST_PORT_IS_BLOCKED:
        p_db_g8032_ring->west_port_is_blocked = p_g8032_ring->west_port_is_blocked;
        break;

    case TBL_G8032_RING_FLD_WEST_PORT_FLUSH_FDB:
        p_db_g8032_ring->west_port_flush_fdb = p_g8032_ring->west_port_flush_fdb;
        break;

    case TBL_G8032_RING_FLD_WEST_PORT_IS_FAIL:
        p_db_g8032_ring->west_port_is_fail = p_g8032_ring->west_port_is_fail;
        break;

    case TBL_G8032_RING_FLD_WEST_PORT_REC_NODE_ID:
        sal_memcpy(p_db_g8032_ring->west_port_rec_node_id, p_g8032_ring->west_port_rec_node_id, sizeof(p_g8032_ring->west_port_rec_node_id));
        break;

    case TBL_G8032_RING_FLD_WEST_PORT_REC_BPR:
        p_db_g8032_ring->west_port_rec_bpr = p_g8032_ring->west_port_rec_bpr;
        break;

    case TBL_G8032_RING_FLD_CONTROL_VLAN:
        p_db_g8032_ring->control_vlan = p_g8032_ring->control_vlan;
        break;

    case TBL_G8032_RING_FLD_IS_SUB_CONTROL_VLAN:
        p_db_g8032_ring->is_sub_control_vlan = p_g8032_ring->is_sub_control_vlan;
        break;

    case TBL_G8032_RING_FLD_IS_FWDTOCPU:
        p_db_g8032_ring->is_fwdtocpu = p_g8032_ring->is_fwdtocpu;
        break;

    case TBL_G8032_RING_FLD_RAPS_MEL:
        p_db_g8032_ring->raps_mel = p_g8032_ring->raps_mel;
        break;

    case TBL_G8032_RING_FLD_RAPS_DNF:
        p_db_g8032_ring->raps_dnf = p_g8032_ring->raps_dnf;
        break;

    case TBL_G8032_RING_FLD_PROTECTION_TRIGGER:
        p_db_g8032_ring->protection_trigger = p_g8032_ring->protection_trigger;
        break;

    case TBL_G8032_RING_FLD_VLAN:
        sal_memcpy(p_db_g8032_ring->vlan, p_g8032_ring->vlan, sizeof(p_g8032_ring->vlan));
        break;

    case TBL_G8032_RING_FLD_PRIMARY_VLAN:
        p_db_g8032_ring->primary_vlan = p_g8032_ring->primary_vlan;
        break;

    case TBL_G8032_RING_FLD_CURRENT_NODE_STATE:
        p_db_g8032_ring->current_node_state = p_g8032_ring->current_node_state;
        break;

    case TBL_G8032_RING_FLD_PRIORITY_REQUEST:
        p_db_g8032_ring->priority_request = p_g8032_ring->priority_request;
        break;

    case TBL_G8032_RING_FLD_HOLD_OFF_EXPIRED:
        p_db_g8032_ring->hold_off_expired = p_g8032_ring->hold_off_expired;
        break;

    case TBL_G8032_RING_FLD_WTR_TIMEOUT_VAL:
        p_db_g8032_ring->wtr_timeout_val = p_g8032_ring->wtr_timeout_val;
        break;

    case TBL_G8032_RING_FLD_WTB_TIMEOUT_VAL:
        p_db_g8032_ring->wtb_timeout_val = p_g8032_ring->wtb_timeout_val;
        break;

    case TBL_G8032_RING_FLD_GUARD_TIMEOUT_VAL:
        p_db_g8032_ring->guard_timeout_val = p_g8032_ring->guard_timeout_val;
        break;

    case TBL_G8032_RING_FLD_HOLD_OFF_TIMEOUT_VAL:
        p_db_g8032_ring->hold_off_timeout_val = p_g8032_ring->hold_off_timeout_val;
        break;

    case TBL_G8032_RING_FLD_FLUSH_FDB_TIMEOUT_VAL:
        p_db_g8032_ring->flush_fdb_timeout_val = p_g8032_ring->flush_fdb_timeout_val;
        break;

    case TBL_G8032_RING_FLD_CFM_MD_NAME:
        sal_memcpy(p_db_g8032_ring->cfm_md_name, p_g8032_ring->cfm_md_name, sizeof(p_g8032_ring->cfm_md_name));
        break;

    case TBL_G8032_RING_FLD_CFM_MA_NAME:
        sal_memcpy(p_db_g8032_ring->cfm_ma_name, p_g8032_ring->cfm_ma_name, sizeof(p_g8032_ring->cfm_ma_name));
        break;

    case TBL_G8032_RING_FLD_LEVEL:
        p_db_g8032_ring->level = p_g8032_ring->level;
        break;

    case TBL_G8032_RING_FLD_VLAN_ID:
        p_db_g8032_ring->vlan_id = p_g8032_ring->vlan_id;
        break;

    case TBL_G8032_RING_FLD_EAST_INTERFACE_CFM_STATE:
        p_db_g8032_ring->east_interface_cfm_state = p_g8032_ring->east_interface_cfm_state;
        break;

    case TBL_G8032_RING_FLD_WEST_INTERFACE_CFM_STATE:
        p_db_g8032_ring->west_interface_cfm_state = p_g8032_ring->west_interface_cfm_state;
        break;

    case TBL_G8032_RING_FLD_P_RAPS_TX_WHILE:
        p_db_g8032_ring->p_raps_tx_while = p_g8032_ring->p_raps_tx_while;
        break;

    case TBL_G8032_RING_FLD_P_WTR_WHILE:
        p_db_g8032_ring->p_wtr_while = p_g8032_ring->p_wtr_while;
        break;

    case TBL_G8032_RING_FLD_P_WTB_WHILE:
        p_db_g8032_ring->p_wtb_while = p_g8032_ring->p_wtb_while;
        break;

    case TBL_G8032_RING_FLD_P_HOLD_OFF_WHILE:
        p_db_g8032_ring->p_hold_off_while = p_g8032_ring->p_hold_off_while;
        break;

    case TBL_G8032_RING_FLD_P_GUARD_WHILE:
        p_db_g8032_ring->p_guard_while = p_g8032_ring->p_guard_while;
        break;

    case TBL_G8032_RING_FLD_P_FLUSH_FDB_WHILE:
        p_db_g8032_ring->p_flush_fdb_while = p_g8032_ring->p_flush_fdb_while;
        break;

    case TBL_G8032_RING_FLD_P_TX_RAPS_NR_WHILE:
        p_db_g8032_ring->p_tx_raps_nr_while = p_g8032_ring->p_tx_raps_nr_while;
        break;

    case TBL_G8032_RING_FLD_P_TX_RAPS_SF_WHILE:
        p_db_g8032_ring->p_tx_raps_sf_while = p_g8032_ring->p_tx_raps_sf_while;
        break;

    case TBL_G8032_RING_FLD_P_TX_RAPS_FS_WHILE:
        p_db_g8032_ring->p_tx_raps_fs_while = p_g8032_ring->p_tx_raps_fs_while;
        break;

    case TBL_G8032_RING_FLD_P_TX_RAPS_MS_WHILE:
        p_db_g8032_ring->p_tx_raps_ms_while = p_g8032_ring->p_tx_raps_ms_while;
        break;

    case TBL_G8032_RING_FLD_P_TX_RAPS_NR_NB_WHILE:
        p_db_g8032_ring->p_tx_raps_nr_nb_while = p_g8032_ring->p_tx_raps_nr_nb_while;
        break;

    case TBL_G8032_RING_FLD_MAX:
        sal_memcpy(p_db_g8032_ring, p_g8032_ring, sizeof(tbl_g8032_ring_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_G8032_RING, field_id, p_db_g8032_ring);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_G8032_RING, field_id, p_db_g8032_ring);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_g8032_ring_add_g8032_ring(tbl_g8032_ring_t *p_g8032_ring)
{
    return tbl_g8032_ring_add_g8032_ring_sync(p_g8032_ring, TRUE);
}

int32
tbl_g8032_ring_del_g8032_ring(tbl_g8032_ring_key_t *p_g8032_ring_key)
{
    return tbl_g8032_ring_del_g8032_ring_sync(p_g8032_ring_key, TRUE);
}

int32
tbl_g8032_ring_set_g8032_ring_field(tbl_g8032_ring_t *p_g8032_ring, tbl_g8032_ring_field_id_t field_id)
{
    return tbl_g8032_ring_set_g8032_ring_field_sync(p_g8032_ring, field_id, TRUE);
}

tbl_g8032_ring_t*
tbl_g8032_ring_get_g8032_ring(tbl_g8032_ring_key_t *p_g8032_ring_key)
{
    tbl_g8032_ring_master_t *p_master = _g_p_tbl_g8032_ring_master;
    tbl_g8032_ring_t lkp;

    sal_memcpy(&lkp.key, p_g8032_ring_key, sizeof(tbl_g8032_ring_key_t));
    return ctclib_hash_lookup(p_master->g8032_ring_hash, &lkp);
}

char*
tbl_g8032_ring_key_val2str(tbl_g8032_ring_t *p_g8032_ring, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_g8032_ring->key.ring_id);
    return str;
}

int32
tbl_g8032_ring_key_str2val(char *str, tbl_g8032_ring_t *p_g8032_ring)
{
    int32 ret = 0;

    p_g8032_ring->key.ring_id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_g8032_ring_key_name_dump(tbl_g8032_ring_t *p_g8032_ring, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_G8032_RING);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_g8032_ring->key.ring_id);
    return str;
}

char*
tbl_g8032_ring_key_value_dump(tbl_g8032_ring_t *p_g8032_ring, char *str)
{
    sal_sprintf(str, "%u", p_g8032_ring->key.ring_id);
    return str;
}

char*
tbl_g8032_ring_field_name_dump(tbl_g8032_ring_t *p_g8032_ring, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_G8032_RING);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_G8032_RING_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_g8032_ring_field_value_dump(tbl_g8032_ring_t *p_g8032_ring, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_G8032_RING_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->key.ring_id);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_RPL_OWNER, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->rpl_owner);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_RPL_NEIGHBOR, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->rpl_neighbor);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_RPL_NEXT_NEIGHBOR, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->rpl_next_neighbor);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_RING_LEVEL_ID, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->ring_level_id);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_PROTECTION_EN, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->protection_en);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_CONFIG_SYNC, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->config_sync);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_APS_PROTECTION_EN, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->aps_protection_en);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_PROTECTION_TIME, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->protection_time);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_MODE_REVERTIVE, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->mode_revertive);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_VIRTUAL_CHANNEL, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->virtual_channel);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_G8032_MAJOR_RING_ID, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->g8032_major_ring_id);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_IS_SUB_RING, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->is_sub_ring);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_INSTANCE_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->instance_count);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_INSTANCE, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_g8032_ring->instance, sizeof(p_g8032_ring->instance)/4);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_SUB_RING_ID, field_id))
    {
        cdb_uint32_array_val2str(str, MAX_CMD_STR_LEN, p_g8032_ring->sub_ring_id, sizeof(p_g8032_ring->sub_ring_id)/4);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_IS_LOCAL_FS, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->is_local_fs);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_IS_LOCAL_MS, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->is_local_ms);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_EAST_PORT_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->east_port_ifindex);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_EAST_PORT_IS_RPL, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->east_port_is_rpl);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_EAST_PORT_IS_RPL_NEIGHBOR, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->east_port_is_rpl_neighbor);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_EAST_PORT_IS_RPL_NEXT_NEIGHBOR, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->east_port_is_rpl_next_neighbor);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_EAST_PORT_IS_FORCE_SWITCH, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->east_port_is_force_switch);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_EAST_PORT_IS_MANUAL_SWITCH, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->east_port_is_manual_switch);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_EAST_PORT_IS_BLOCKED, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->east_port_is_blocked);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_EAST_PORT_FLUSH_FDB, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->east_port_flush_fdb);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_EAST_PORT_IS_FAIL, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->east_port_is_fail);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_EAST_PORT_REC_NODE_ID, field_id))
    {
        cdb_uint8_array_val2str(str, MAX_CMD_STR_LEN, p_g8032_ring->east_port_rec_node_id, sizeof(p_g8032_ring->east_port_rec_node_id));
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_EAST_PORT_REC_BPR, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->east_port_rec_bpr);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WEST_PORT_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->west_port_ifindex);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WEST_PORT_IS_RPL, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->west_port_is_rpl);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WEST_PORT_IS_RPL_NEIGHBOR, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->west_port_is_rpl_neighbor);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WEST_PORT_IS_RPL_NEXT_NEIGHBOR, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->west_port_is_rpl_next_neighbor);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WEST_PORT_IS_FORCE_SWITCH, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->west_port_is_force_switch);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WEST_PORT_IS_MANUAL_SWITCH, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->west_port_is_manual_switch);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WEST_PORT_IS_BLOCKED, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->west_port_is_blocked);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WEST_PORT_FLUSH_FDB, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->west_port_flush_fdb);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WEST_PORT_IS_FAIL, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->west_port_is_fail);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WEST_PORT_REC_NODE_ID, field_id))
    {
        cdb_uint8_array_val2str(str, MAX_CMD_STR_LEN, p_g8032_ring->west_port_rec_node_id, sizeof(p_g8032_ring->west_port_rec_node_id));
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WEST_PORT_REC_BPR, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->west_port_rec_bpr);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_CONTROL_VLAN, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->control_vlan);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_IS_SUB_CONTROL_VLAN, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->is_sub_control_vlan);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_IS_FWDTOCPU, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->is_fwdtocpu);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_RAPS_MEL, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->raps_mel);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_RAPS_DNF, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->raps_dnf);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_PROTECTION_TRIGGER, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->protection_trigger);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_VLAN, field_id))
    {
        cdb_bitmap_val2str(str, MAX_CMD_STR_LEN, p_g8032_ring->vlan, sizeof(p_g8032_ring->vlan)/4);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_PRIMARY_VLAN, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->primary_vlan);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_CURRENT_NODE_STATE, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->current_node_state);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_PRIORITY_REQUEST, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->priority_request);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_HOLD_OFF_EXPIRED, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->hold_off_expired);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WTR_TIMEOUT_VAL, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->wtr_timeout_val);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WTB_TIMEOUT_VAL, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->wtb_timeout_val);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_GUARD_TIMEOUT_VAL, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->guard_timeout_val);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_HOLD_OFF_TIMEOUT_VAL, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->hold_off_timeout_val);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_FLUSH_FDB_TIMEOUT_VAL, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->flush_fdb_timeout_val);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_CFM_MD_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_g8032_ring->cfm_md_name);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_CFM_MA_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_g8032_ring->cfm_ma_name);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_LEVEL, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->level);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_VLAN_ID, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->vlan_id);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_EAST_INTERFACE_CFM_STATE, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->east_interface_cfm_state);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WEST_INTERFACE_CFM_STATE, field_id))
    {
        sal_sprintf(str, "%u", p_g8032_ring->west_interface_cfm_state);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_P_RAPS_TX_WHILE, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_g8032_ring->p_raps_tx_while);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_P_WTR_WHILE, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_g8032_ring->p_wtr_while);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_P_WTB_WHILE, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_g8032_ring->p_wtb_while);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_P_HOLD_OFF_WHILE, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_g8032_ring->p_hold_off_while);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_P_GUARD_WHILE, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_g8032_ring->p_guard_while);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_P_FLUSH_FDB_WHILE, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_g8032_ring->p_flush_fdb_while);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_P_TX_RAPS_NR_WHILE, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_g8032_ring->p_tx_raps_nr_while);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_P_TX_RAPS_SF_WHILE, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_g8032_ring->p_tx_raps_sf_while);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_P_TX_RAPS_FS_WHILE, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_g8032_ring->p_tx_raps_fs_while);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_P_TX_RAPS_MS_WHILE, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_g8032_ring->p_tx_raps_ms_while);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_P_TX_RAPS_NR_NB_WHILE, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_g8032_ring->p_tx_raps_nr_nb_while);
    }
    return str;
}

char**
tbl_g8032_ring_table_dump(tbl_g8032_ring_t *p_g8032_ring, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_g8032_ring_key_name_dump(p_g8032_ring, buf));
    for(i=1; i<TBL_G8032_RING_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_g8032_ring_field_name_dump(p_g8032_ring, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_g8032_ring_field_value_dump(p_g8032_ring, i, buf));
    }
    return str;
}

int32
tbl_g8032_ring_field_value_parser(char *str, int32 field_id, tbl_g8032_ring_t *p_g8032_ring)
{
    if (FLD_MATCH(TBL_G8032_RING_FLD_KEY, field_id))
    {
        int32 ret;
        p_g8032_ring->key.ring_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_RPL_OWNER, field_id))
    {
        int32 ret;
        p_g8032_ring->rpl_owner = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_RPL_NEIGHBOR, field_id))
    {
        int32 ret;
        p_g8032_ring->rpl_neighbor = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_RPL_NEXT_NEIGHBOR, field_id))
    {
        int32 ret;
        p_g8032_ring->rpl_next_neighbor = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_RING_LEVEL_ID, field_id))
    {
        int32 ret;
        p_g8032_ring->ring_level_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_PROTECTION_EN, field_id))
    {
        int32 ret;
        p_g8032_ring->protection_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_CONFIG_SYNC, field_id))
    {
        int32 ret;
        p_g8032_ring->config_sync = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_APS_PROTECTION_EN, field_id))
    {
        int32 ret;
        p_g8032_ring->aps_protection_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_PROTECTION_TIME, field_id))
    {
        int32 ret;
        p_g8032_ring->protection_time = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_MODE_REVERTIVE, field_id))
    {
        int32 ret;
        p_g8032_ring->mode_revertive = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_VIRTUAL_CHANNEL, field_id))
    {
        int32 ret;
        p_g8032_ring->virtual_channel = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_G8032_MAJOR_RING_ID, field_id))
    {
        int32 ret;
        p_g8032_ring->g8032_major_ring_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_IS_SUB_RING, field_id))
    {
        int32 ret;
        p_g8032_ring->is_sub_ring = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_INSTANCE_COUNT, field_id))
    {
        int32 ret;
        p_g8032_ring->instance_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_INSTANCE, field_id))
    {
        cdb_uint32_array_str2val(str, p_g8032_ring->instance);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_SUB_RING_ID, field_id))
    {
        cdb_uint32_array_str2val(str, p_g8032_ring->sub_ring_id);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_IS_LOCAL_FS, field_id))
    {
        int32 ret;
        p_g8032_ring->is_local_fs = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_IS_LOCAL_MS, field_id))
    {
        int32 ret;
        p_g8032_ring->is_local_ms = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_EAST_PORT_IFINDEX, field_id))
    {
        int32 ret;
        p_g8032_ring->east_port_ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_EAST_PORT_IS_RPL, field_id))
    {
        int32 ret;
        p_g8032_ring->east_port_is_rpl = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_EAST_PORT_IS_RPL_NEIGHBOR, field_id))
    {
        int32 ret;
        p_g8032_ring->east_port_is_rpl_neighbor = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_EAST_PORT_IS_RPL_NEXT_NEIGHBOR, field_id))
    {
        int32 ret;
        p_g8032_ring->east_port_is_rpl_next_neighbor = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_EAST_PORT_IS_FORCE_SWITCH, field_id))
    {
        int32 ret;
        p_g8032_ring->east_port_is_force_switch = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_EAST_PORT_IS_MANUAL_SWITCH, field_id))
    {
        int32 ret;
        p_g8032_ring->east_port_is_manual_switch = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_EAST_PORT_IS_BLOCKED, field_id))
    {
        int32 ret;
        p_g8032_ring->east_port_is_blocked = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_EAST_PORT_FLUSH_FDB, field_id))
    {
        int32 ret;
        p_g8032_ring->east_port_flush_fdb = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_EAST_PORT_IS_FAIL, field_id))
    {
        int32 ret;
        p_g8032_ring->east_port_is_fail = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_EAST_PORT_REC_NODE_ID, field_id))
    {
        cdb_uint8_array_str2val(str, p_g8032_ring->east_port_rec_node_id);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_EAST_PORT_REC_BPR, field_id))
    {
        int32 ret;
        p_g8032_ring->east_port_rec_bpr = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WEST_PORT_IFINDEX, field_id))
    {
        int32 ret;
        p_g8032_ring->west_port_ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WEST_PORT_IS_RPL, field_id))
    {
        int32 ret;
        p_g8032_ring->west_port_is_rpl = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WEST_PORT_IS_RPL_NEIGHBOR, field_id))
    {
        int32 ret;
        p_g8032_ring->west_port_is_rpl_neighbor = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WEST_PORT_IS_RPL_NEXT_NEIGHBOR, field_id))
    {
        int32 ret;
        p_g8032_ring->west_port_is_rpl_next_neighbor = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WEST_PORT_IS_FORCE_SWITCH, field_id))
    {
        int32 ret;
        p_g8032_ring->west_port_is_force_switch = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WEST_PORT_IS_MANUAL_SWITCH, field_id))
    {
        int32 ret;
        p_g8032_ring->west_port_is_manual_switch = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WEST_PORT_IS_BLOCKED, field_id))
    {
        int32 ret;
        p_g8032_ring->west_port_is_blocked = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WEST_PORT_FLUSH_FDB, field_id))
    {
        int32 ret;
        p_g8032_ring->west_port_flush_fdb = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WEST_PORT_IS_FAIL, field_id))
    {
        int32 ret;
        p_g8032_ring->west_port_is_fail = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WEST_PORT_REC_NODE_ID, field_id))
    {
        cdb_uint8_array_str2val(str, p_g8032_ring->west_port_rec_node_id);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WEST_PORT_REC_BPR, field_id))
    {
        int32 ret;
        p_g8032_ring->west_port_rec_bpr = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_CONTROL_VLAN, field_id))
    {
        int32 ret;
        p_g8032_ring->control_vlan = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_IS_SUB_CONTROL_VLAN, field_id))
    {
        int32 ret;
        p_g8032_ring->is_sub_control_vlan = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_IS_FWDTOCPU, field_id))
    {
        int32 ret;
        p_g8032_ring->is_fwdtocpu = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_RAPS_MEL, field_id))
    {
        int32 ret;
        p_g8032_ring->raps_mel = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_RAPS_DNF, field_id))
    {
        int32 ret;
        p_g8032_ring->raps_dnf = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_PROTECTION_TRIGGER, field_id))
    {
        int32 ret;
        p_g8032_ring->protection_trigger = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_VLAN, field_id))
    {
        char err[MAX_CMD_STR_LEN];
        cdb_bitmap_str2val(str, 0, GLB_UINT16_BMP_WORD_MAX, p_g8032_ring->vlan, sizeof(p_g8032_ring->vlan)/4, err);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_PRIMARY_VLAN, field_id))
    {
        int32 ret;
        p_g8032_ring->primary_vlan = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_CURRENT_NODE_STATE, field_id))
    {
        int32 ret;
        p_g8032_ring->current_node_state = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_PRIORITY_REQUEST, field_id))
    {
        int32 ret;
        p_g8032_ring->priority_request = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_HOLD_OFF_EXPIRED, field_id))
    {
        int32 ret;
        p_g8032_ring->hold_off_expired = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WTR_TIMEOUT_VAL, field_id))
    {
        int32 ret;
        p_g8032_ring->wtr_timeout_val = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WTB_TIMEOUT_VAL, field_id))
    {
        int32 ret;
        p_g8032_ring->wtb_timeout_val = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_GUARD_TIMEOUT_VAL, field_id))
    {
        int32 ret;
        p_g8032_ring->guard_timeout_val = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_HOLD_OFF_TIMEOUT_VAL, field_id))
    {
        int32 ret;
        p_g8032_ring->hold_off_timeout_val = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_FLUSH_FDB_TIMEOUT_VAL, field_id))
    {
        int32 ret;
        p_g8032_ring->flush_fdb_timeout_val = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_CFM_MD_NAME, field_id))
    {
        sal_strcpy(p_g8032_ring->cfm_md_name, str);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_CFM_MA_NAME, field_id))
    {
        sal_strcpy(p_g8032_ring->cfm_ma_name, str);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_LEVEL, field_id))
    {
        int32 ret;
        p_g8032_ring->level = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_VLAN_ID, field_id))
    {
        int32 ret;
        p_g8032_ring->vlan_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_EAST_INTERFACE_CFM_STATE, field_id))
    {
        int32 ret;
        p_g8032_ring->east_interface_cfm_state = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WEST_INTERFACE_CFM_STATE, field_id))
    {
        int32 ret;
        p_g8032_ring->west_interface_cfm_state = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_P_RAPS_TX_WHILE, field_id))
    {
        ctc_task_str2val(str, p_g8032_ring->p_raps_tx_while);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_P_WTR_WHILE, field_id))
    {
        ctc_task_str2val(str, p_g8032_ring->p_wtr_while);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_P_WTB_WHILE, field_id))
    {
        ctc_task_str2val(str, p_g8032_ring->p_wtb_while);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_P_HOLD_OFF_WHILE, field_id))
    {
        ctc_task_str2val(str, p_g8032_ring->p_hold_off_while);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_P_GUARD_WHILE, field_id))
    {
        ctc_task_str2val(str, p_g8032_ring->p_guard_while);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_P_FLUSH_FDB_WHILE, field_id))
    {
        ctc_task_str2val(str, p_g8032_ring->p_flush_fdb_while);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_P_TX_RAPS_NR_WHILE, field_id))
    {
        ctc_task_str2val(str, p_g8032_ring->p_tx_raps_nr_while);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_P_TX_RAPS_SF_WHILE, field_id))
    {
        ctc_task_str2val(str, p_g8032_ring->p_tx_raps_sf_while);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_P_TX_RAPS_FS_WHILE, field_id))
    {
        ctc_task_str2val(str, p_g8032_ring->p_tx_raps_fs_while);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_P_TX_RAPS_MS_WHILE, field_id))
    {
        ctc_task_str2val(str, p_g8032_ring->p_tx_raps_ms_while);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_P_TX_RAPS_NR_NB_WHILE, field_id))
    {
        ctc_task_str2val(str, p_g8032_ring->p_tx_raps_nr_nb_while);
    }
    return PM_E_NONE;
}

int32
tbl_g8032_ring_table_parser(char** array, char* key_value,tbl_g8032_ring_t *p_g8032_ring)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_G8032_RING);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_g8032_ring_key_str2val(key_value, p_g8032_ring));

    for(i=1; i<TBL_G8032_RING_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_g8032_ring_field_value_parser( array[j++], i, p_g8032_ring));
    }

    return PM_E_NONE;
}

int32
tbl_g8032_ring_dump_one(tbl_g8032_ring_t *p_g8032_ring, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_G8032_RING);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_G8032_RING_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_KEY].name,
            p_g8032_ring->key.ring_id);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_RPL_OWNER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_RPL_OWNER].name,
            p_g8032_ring->rpl_owner);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_RPL_NEIGHBOR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_RPL_NEIGHBOR].name,
            p_g8032_ring->rpl_neighbor);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_RPL_NEXT_NEIGHBOR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_RPL_NEXT_NEIGHBOR].name,
            p_g8032_ring->rpl_next_neighbor);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_RING_LEVEL_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_RING_LEVEL_ID].name,
            p_g8032_ring->ring_level_id);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_PROTECTION_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_PROTECTION_EN].name,
            p_g8032_ring->protection_en);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_CONFIG_SYNC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_CONFIG_SYNC].name,
            p_g8032_ring->config_sync);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_APS_PROTECTION_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_APS_PROTECTION_EN].name,
            p_g8032_ring->aps_protection_en);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_PROTECTION_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_PROTECTION_TIME].name,
            p_g8032_ring->protection_time);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_MODE_REVERTIVE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_MODE_REVERTIVE].name,
            p_g8032_ring->mode_revertive);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_VIRTUAL_CHANNEL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_VIRTUAL_CHANNEL].name,
            p_g8032_ring->virtual_channel);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_G8032_MAJOR_RING_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_G8032_MAJOR_RING_ID].name,
            p_g8032_ring->g8032_major_ring_id);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_IS_SUB_RING, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_IS_SUB_RING].name,
            p_g8032_ring->is_sub_ring);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_INSTANCE_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_INSTANCE_COUNT].name,
            p_g8032_ring->instance_count);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_INSTANCE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_G8032_RING_FLD_INSTANCE].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_g8032_ring->instance, sizeof(p_g8032_ring->instance)/4));
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_SUB_RING_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_G8032_RING_FLD_SUB_RING_ID].name, 
            cdb_uint32_array_val2str(buf, MAX_CMD_STR_LEN, p_g8032_ring->sub_ring_id, sizeof(p_g8032_ring->sub_ring_id)/4));
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_IS_LOCAL_FS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_IS_LOCAL_FS].name,
            p_g8032_ring->is_local_fs);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_IS_LOCAL_MS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_IS_LOCAL_MS].name,
            p_g8032_ring->is_local_ms);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_EAST_PORT_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_EAST_PORT_IFINDEX].name,
            p_g8032_ring->east_port_ifindex);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_EAST_PORT_IS_RPL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_EAST_PORT_IS_RPL].name,
            p_g8032_ring->east_port_is_rpl);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_EAST_PORT_IS_RPL_NEIGHBOR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_EAST_PORT_IS_RPL_NEIGHBOR].name,
            p_g8032_ring->east_port_is_rpl_neighbor);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_EAST_PORT_IS_RPL_NEXT_NEIGHBOR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_EAST_PORT_IS_RPL_NEXT_NEIGHBOR].name,
            p_g8032_ring->east_port_is_rpl_next_neighbor);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_EAST_PORT_IS_FORCE_SWITCH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_EAST_PORT_IS_FORCE_SWITCH].name,
            p_g8032_ring->east_port_is_force_switch);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_EAST_PORT_IS_MANUAL_SWITCH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_EAST_PORT_IS_MANUAL_SWITCH].name,
            p_g8032_ring->east_port_is_manual_switch);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_EAST_PORT_IS_BLOCKED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_EAST_PORT_IS_BLOCKED].name,
            p_g8032_ring->east_port_is_blocked);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_EAST_PORT_FLUSH_FDB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_EAST_PORT_FLUSH_FDB].name,
            p_g8032_ring->east_port_flush_fdb);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_EAST_PORT_IS_FAIL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_EAST_PORT_IS_FAIL].name,
            p_g8032_ring->east_port_is_fail);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_EAST_PORT_REC_NODE_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_G8032_RING_FLD_EAST_PORT_REC_NODE_ID].name, 
            cdb_uint8_array_val2str(buf, MAX_CMD_STR_LEN, p_g8032_ring->east_port_rec_node_id, sizeof(p_g8032_ring->east_port_rec_node_id)));
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_EAST_PORT_REC_BPR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_EAST_PORT_REC_BPR].name,
            p_g8032_ring->east_port_rec_bpr);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WEST_PORT_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_WEST_PORT_IFINDEX].name,
            p_g8032_ring->west_port_ifindex);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WEST_PORT_IS_RPL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_WEST_PORT_IS_RPL].name,
            p_g8032_ring->west_port_is_rpl);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WEST_PORT_IS_RPL_NEIGHBOR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_WEST_PORT_IS_RPL_NEIGHBOR].name,
            p_g8032_ring->west_port_is_rpl_neighbor);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WEST_PORT_IS_RPL_NEXT_NEIGHBOR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_WEST_PORT_IS_RPL_NEXT_NEIGHBOR].name,
            p_g8032_ring->west_port_is_rpl_next_neighbor);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WEST_PORT_IS_FORCE_SWITCH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_WEST_PORT_IS_FORCE_SWITCH].name,
            p_g8032_ring->west_port_is_force_switch);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WEST_PORT_IS_MANUAL_SWITCH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_WEST_PORT_IS_MANUAL_SWITCH].name,
            p_g8032_ring->west_port_is_manual_switch);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WEST_PORT_IS_BLOCKED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_WEST_PORT_IS_BLOCKED].name,
            p_g8032_ring->west_port_is_blocked);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WEST_PORT_FLUSH_FDB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_WEST_PORT_FLUSH_FDB].name,
            p_g8032_ring->west_port_flush_fdb);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WEST_PORT_IS_FAIL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_WEST_PORT_IS_FAIL].name,
            p_g8032_ring->west_port_is_fail);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WEST_PORT_REC_NODE_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_G8032_RING_FLD_WEST_PORT_REC_NODE_ID].name, 
            cdb_uint8_array_val2str(buf, MAX_CMD_STR_LEN, p_g8032_ring->west_port_rec_node_id, sizeof(p_g8032_ring->west_port_rec_node_id)));
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WEST_PORT_REC_BPR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_WEST_PORT_REC_BPR].name,
            p_g8032_ring->west_port_rec_bpr);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_CONTROL_VLAN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_CONTROL_VLAN].name,
            p_g8032_ring->control_vlan);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_IS_SUB_CONTROL_VLAN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_IS_SUB_CONTROL_VLAN].name,
            p_g8032_ring->is_sub_control_vlan);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_IS_FWDTOCPU, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_IS_FWDTOCPU].name,
            p_g8032_ring->is_fwdtocpu);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_RAPS_MEL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_RAPS_MEL].name,
            p_g8032_ring->raps_mel);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_RAPS_DNF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_RAPS_DNF].name,
            p_g8032_ring->raps_dnf);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_PROTECTION_TRIGGER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_PROTECTION_TRIGGER].name,
            p_g8032_ring->protection_trigger);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_VLAN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_G8032_RING_FLD_VLAN].name, 
            cdb_bitmap_val2str(buf, MAX_CMD_STR_LEN, p_g8032_ring->vlan, sizeof(p_g8032_ring->vlan)/4));
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_PRIMARY_VLAN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_PRIMARY_VLAN].name,
            p_g8032_ring->primary_vlan);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_CURRENT_NODE_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_CURRENT_NODE_STATE].name,
            p_g8032_ring->current_node_state);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_PRIORITY_REQUEST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_PRIORITY_REQUEST].name,
            p_g8032_ring->priority_request);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_HOLD_OFF_EXPIRED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_HOLD_OFF_EXPIRED].name,
            p_g8032_ring->hold_off_expired);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WTR_TIMEOUT_VAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_WTR_TIMEOUT_VAL].name,
            p_g8032_ring->wtr_timeout_val);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WTB_TIMEOUT_VAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_WTB_TIMEOUT_VAL].name,
            p_g8032_ring->wtb_timeout_val);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_GUARD_TIMEOUT_VAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_GUARD_TIMEOUT_VAL].name,
            p_g8032_ring->guard_timeout_val);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_HOLD_OFF_TIMEOUT_VAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_HOLD_OFF_TIMEOUT_VAL].name,
            p_g8032_ring->hold_off_timeout_val);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_FLUSH_FDB_TIMEOUT_VAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_FLUSH_FDB_TIMEOUT_VAL].name,
            p_g8032_ring->flush_fdb_timeout_val);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_CFM_MD_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_G8032_RING_FLD_CFM_MD_NAME].name,
            p_g8032_ring->cfm_md_name);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_CFM_MA_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_G8032_RING_FLD_CFM_MA_NAME].name,
            p_g8032_ring->cfm_ma_name);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_LEVEL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_LEVEL].name,
            p_g8032_ring->level);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_VLAN_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_VLAN_ID].name,
            p_g8032_ring->vlan_id);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_EAST_INTERFACE_CFM_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_EAST_INTERFACE_CFM_STATE].name,
            p_g8032_ring->east_interface_cfm_state);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_WEST_INTERFACE_CFM_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_G8032_RING_FLD_WEST_INTERFACE_CFM_STATE].name,
            p_g8032_ring->west_interface_cfm_state);
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_P_RAPS_TX_WHILE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_G8032_RING_FLD_P_RAPS_TX_WHILE].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_g8032_ring->p_raps_tx_while));
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_P_WTR_WHILE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_G8032_RING_FLD_P_WTR_WHILE].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_g8032_ring->p_wtr_while));
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_P_WTB_WHILE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_G8032_RING_FLD_P_WTB_WHILE].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_g8032_ring->p_wtb_while));
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_P_HOLD_OFF_WHILE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_G8032_RING_FLD_P_HOLD_OFF_WHILE].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_g8032_ring->p_hold_off_while));
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_P_GUARD_WHILE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_G8032_RING_FLD_P_GUARD_WHILE].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_g8032_ring->p_guard_while));
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_P_FLUSH_FDB_WHILE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_G8032_RING_FLD_P_FLUSH_FDB_WHILE].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_g8032_ring->p_flush_fdb_while));
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_P_TX_RAPS_NR_WHILE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_G8032_RING_FLD_P_TX_RAPS_NR_WHILE].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_g8032_ring->p_tx_raps_nr_while));
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_P_TX_RAPS_SF_WHILE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_G8032_RING_FLD_P_TX_RAPS_SF_WHILE].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_g8032_ring->p_tx_raps_sf_while));
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_P_TX_RAPS_FS_WHILE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_G8032_RING_FLD_P_TX_RAPS_FS_WHILE].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_g8032_ring->p_tx_raps_fs_while));
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_P_TX_RAPS_MS_WHILE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_G8032_RING_FLD_P_TX_RAPS_MS_WHILE].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_g8032_ring->p_tx_raps_ms_while));
    }
    if (FLD_MATCH(TBL_G8032_RING_FLD_P_TX_RAPS_NR_NB_WHILE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_G8032_RING_FLD_P_TX_RAPS_NR_NB_WHILE].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_g8032_ring->p_tx_raps_nr_nb_while));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_g8032_ring_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_g8032_ring_master_t *p_master = _g_p_tbl_g8032_ring_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_g8032_ring_t *p_db_g8032_ring = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->g8032_ring_list, p_db_g8032_ring, listnode, next)
    {
        rc |= fn(p_db_g8032_ring, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_g8032_ring_master_t*
tbl_g8032_ring_get_master()
{
    return _g_p_tbl_g8032_ring_master;
}

tbl_g8032_ring_master_t*
tbl_g8032_ring_init_g8032_ring()
{
    _g_p_tbl_g8032_ring_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_g8032_ring_master_t));
    _g_p_tbl_g8032_ring_master->g8032_ring_hash = ctclib_hash_create(_tbl_g8032_ring_hash_make, _tbl_g8032_ring_hash_cmp);
    _g_p_tbl_g8032_ring_master->g8032_ring_list = ctclib_slist_create(tbl_g8032_ring_cmp, NULL);
    return _g_p_tbl_g8032_ring_master;
}

