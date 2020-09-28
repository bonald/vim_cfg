
#include "proto.h"
#include "gen/tbl_msti_port_define.h"
#include "gen/tbl_msti_port.h"
#include "cdb_data_cmp.h"

tbl_msti_port_master_t *_g_p_tbl_msti_port_master = NULL;

#include "mstp_define.h"

static uint32
_tbl_msti_port_hash_make(tbl_msti_port_t *p_msti_port)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_msti_port->key;
    for (index = 0; index < sizeof(p_msti_port->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_msti_port_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_msti_port_t *p_msti_port1 = (tbl_msti_port_t*)p_arg1;
    tbl_msti_port_t *p_msti_port2 = (tbl_msti_port_t*)p_arg2;

    if (0 == sal_memcmp(&p_msti_port1->key, &p_msti_port2->key, sizeof(tbl_msti_port_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_msti_port_add_msti_port_sync(tbl_msti_port_t *p_msti_port, uint32 sync)
{
    tbl_msti_port_master_t *p_master = _g_p_tbl_msti_port_master;
    tbl_msti_port_t *p_db_msti_port = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_msti_port_get_msti_port(&p_msti_port->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_msti_port = XCALLOC(MEM_TBL_MSTI_PORT, sizeof(tbl_msti_port_t));
    if (NULL == p_db_msti_port)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_msti_port, p_msti_port, sizeof(tbl_msti_port_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->port_msti_hash, (void*)p_db_msti_port, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->port_msti_list, p_db_msti_port);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_MSTI_PORT, p_db_msti_port);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_MSTI_PORT, p_db_msti_port);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_msti_port_del_msti_port_sync(tbl_msti_port_key_t *p_msti_port_key, uint32 sync)
{
    tbl_msti_port_master_t *p_master = _g_p_tbl_msti_port_master;
    tbl_msti_port_t *p_db_msti_port = NULL;

    /* 1. lookup entry exist */
    p_db_msti_port = tbl_msti_port_get_msti_port(p_msti_port_key);
    if (NULL == p_db_msti_port)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_MSTI_PORT, p_db_msti_port);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_MSTI_PORT, p_db_msti_port);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->port_msti_hash, (void*)p_db_msti_port);
    ctclib_slistnode_delete(p_master->port_msti_list, p_db_msti_port);

    /* 4. free db entry */
    XFREE(MEM_TBL_MSTI_PORT, p_db_msti_port);

    return PM_E_NONE;
}

int32
tbl_msti_port_set_msti_port_field_sync(tbl_msti_port_t *p_msti_port, tbl_msti_port_field_id_t field_id, uint32 sync)
{
    tbl_msti_port_t *p_db_msti_port = NULL;

    /* 1. lookup entry exist */
    p_db_msti_port = tbl_msti_port_get_msti_port(&p_msti_port->key);
    if (NULL == p_db_msti_port)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_MSTI_PORT_FLD_IFINDEX:
        p_db_msti_port->ifindex = p_msti_port->ifindex;
        break;

    case TBL_MSTI_PORT_FLD_ADMINPORTPATHCOST:
        p_db_msti_port->adminPortPathCost = p_msti_port->adminPortPathCost;
        break;

    case TBL_MSTI_PORT_FLD_PORTPATHCOST:
        p_db_msti_port->portPathCost = p_msti_port->portPathCost;
        break;

    case TBL_MSTI_PORT_FLD_FORWARD_TRANSITIONS:
        p_db_msti_port->forward_transitions = p_msti_port->forward_transitions;
        break;

    case TBL_MSTI_PORT_FLD_NEWINFOMSTI:
        p_db_msti_port->newInfoMsti = p_msti_port->newInfoMsti;
        break;

    case TBL_MSTI_PORT_FLD_BPDU_LOOP_COUNT:
        p_db_msti_port->bpdu_loop_count = p_msti_port->bpdu_loop_count;
        break;

    case TBL_MSTI_PORT_FLD_RESTRICTED_ROLE:
        p_db_msti_port->restricted_role = p_msti_port->restricted_role;
        break;

    case TBL_MSTI_PORT_FLD_RESTRICTED_TCN:
        p_db_msti_port->restricted_tcn = p_msti_port->restricted_tcn;
        break;

    case TBL_MSTI_PORT_FLD_OPER_ROOTGUARD:
        p_db_msti_port->oper_rootguard = p_msti_port->oper_rootguard;
        break;

    case TBL_MSTI_PORT_FLD_OPER_LOOPGUARD:
        p_db_msti_port->oper_loopguard = p_msti_port->oper_loopguard;
        break;

    case TBL_MSTI_PORT_FLD_AGREE:
        p_db_msti_port->agree = p_msti_port->agree;
        break;

    case TBL_MSTI_PORT_FLD_AGREED:
        p_db_msti_port->agreed = p_msti_port->agreed;
        break;

    case TBL_MSTI_PORT_FLD_ADMINFORCEVERSION:
        p_db_msti_port->adminForceVersion = p_msti_port->adminForceVersion;
        break;

    case TBL_MSTI_PORT_FLD_DESIGNATED_REGIONROOTID:
        sal_memcpy(&p_db_msti_port->designated_regionRootId, &p_msti_port->designated_regionRootId, sizeof(p_msti_port->designated_regionRootId));
        break;

    case TBL_MSTI_PORT_FLD_DESIGNATED_INTROOTPATHCOST:
        p_db_msti_port->designated_intRootPathCost = p_msti_port->designated_intRootPathCost;
        break;

    case TBL_MSTI_PORT_FLD_DESIGNATED_DESIGNATEDBRIDGEID:
        sal_memcpy(&p_db_msti_port->designated_designatedBridgeId, &p_msti_port->designated_designatedBridgeId, sizeof(p_msti_port->designated_designatedBridgeId));
        break;

    case TBL_MSTI_PORT_FLD_DESIGNATED_DESIGNATEDPORTID:
        p_db_msti_port->designated_designatedPortId = p_msti_port->designated_designatedPortId;
        break;

    case TBL_MSTI_PORT_FLD_DESIGNATED_BRIDGEPORTID:
        p_db_msti_port->designated_bridgePortId = p_msti_port->designated_bridgePortId;
        break;

    case TBL_MSTI_PORT_FLD_DESIGNATED_MESSAGEAGE:
        p_db_msti_port->designated_messageAge = p_msti_port->designated_messageAge;
        break;

    case TBL_MSTI_PORT_FLD_DESIGNATED_FWDDELAY:
        p_db_msti_port->designated_fwdDelay = p_msti_port->designated_fwdDelay;
        break;

    case TBL_MSTI_PORT_FLD_DESIGNATED_HELLOTIME:
        p_db_msti_port->designated_helloTime = p_msti_port->designated_helloTime;
        break;

    case TBL_MSTI_PORT_FLD_DESIGNATED_MAXAGE:
        p_db_msti_port->designated_maxAge = p_msti_port->designated_maxAge;
        break;

    case TBL_MSTI_PORT_FLD_DISPUTED:
        p_db_msti_port->disputed = p_msti_port->disputed;
        break;

    case TBL_MSTI_PORT_FLD_FDBFLUSH:
        p_db_msti_port->fdbFlush = p_msti_port->fdbFlush;
        break;

    case TBL_MSTI_PORT_FLD_FORWARD:
        p_db_msti_port->forward = p_msti_port->forward;
        break;

    case TBL_MSTI_PORT_FLD_FORWARDING:
        p_db_msti_port->forwarding = p_msti_port->forwarding;
        break;

    case TBL_MSTI_PORT_FLD_INFOIS:
        p_db_msti_port->infoIs = p_msti_port->infoIs;
        break;

    case TBL_MSTI_PORT_FLD_LEARN:
        p_db_msti_port->learn = p_msti_port->learn;
        break;

    case TBL_MSTI_PORT_FLD_LEARNING:
        p_db_msti_port->learning = p_msti_port->learning;
        break;

    case TBL_MSTI_PORT_FLD_NEWINFO:
        p_db_msti_port->newInfo = p_msti_port->newInfo;
        break;

    case TBL_MSTI_PORT_FLD_PORTID:
        p_db_msti_port->portId = p_msti_port->portId;
        break;

    case TBL_MSTI_PORT_FLD_PRIORITY:
        p_db_msti_port->priority = p_msti_port->priority;
        break;

    case TBL_MSTI_PORT_FLD_PORT_ROOTID:
        sal_memcpy(&p_db_msti_port->port_rootId, &p_msti_port->port_rootId, sizeof(p_msti_port->port_rootId));
        break;

    case TBL_MSTI_PORT_FLD_PORT_ROOTPATHCOST:
        p_db_msti_port->port_rootPathCost = p_msti_port->port_rootPathCost;
        break;

    case TBL_MSTI_PORT_FLD_PORT_DESIGNATEDBRIDGEID:
        sal_memcpy(&p_db_msti_port->port_designatedBridgeId, &p_msti_port->port_designatedBridgeId, sizeof(p_msti_port->port_designatedBridgeId));
        break;

    case TBL_MSTI_PORT_FLD_PORT_DESIGNATEDPORTID:
        p_db_msti_port->port_designatedPortId = p_msti_port->port_designatedPortId;
        break;

    case TBL_MSTI_PORT_FLD_PORT_BRIDGEPORTID:
        p_db_msti_port->port_bridgePortId = p_msti_port->port_bridgePortId;
        break;

    case TBL_MSTI_PORT_FLD_PORT_MESSAGEAGE:
        p_db_msti_port->port_messageAge = p_msti_port->port_messageAge;
        break;

    case TBL_MSTI_PORT_FLD_PORT_FWDDELAY:
        p_db_msti_port->port_fwdDelay = p_msti_port->port_fwdDelay;
        break;

    case TBL_MSTI_PORT_FLD_PORT_HELLOTIME:
        p_db_msti_port->port_helloTime = p_msti_port->port_helloTime;
        break;

    case TBL_MSTI_PORT_FLD_PORT_MAXAGE:
        p_db_msti_port->port_maxAge = p_msti_port->port_maxAge;
        break;

    case TBL_MSTI_PORT_FLD_PROPOSED:
        p_db_msti_port->proposed = p_msti_port->proposed;
        break;

    case TBL_MSTI_PORT_FLD_PROPOSING:
        p_db_msti_port->proposing = p_msti_port->proposing;
        break;

    case TBL_MSTI_PORT_FLD_RCVDBPDU:
        p_db_msti_port->rcvdBpdu = p_msti_port->rcvdBpdu;
        break;

    case TBL_MSTI_PORT_FLD_RCVDINFO:
        p_db_msti_port->rcvdInfo = p_msti_port->rcvdInfo;
        break;

    case TBL_MSTI_PORT_FLD_RCVDMSG:
        p_db_msti_port->rcvdMsg = p_msti_port->rcvdMsg;
        break;

    case TBL_MSTI_PORT_FLD_RCVDRSTP:
        p_db_msti_port->rcvdRSTP = p_msti_port->rcvdRSTP;
        break;

    case TBL_MSTI_PORT_FLD_RCVDSTP:
        p_db_msti_port->rcvdSTP = p_msti_port->rcvdSTP;
        break;

    case TBL_MSTI_PORT_FLD_RCVDTC:
        p_db_msti_port->rcvdTc = p_msti_port->rcvdTc;
        break;

    case TBL_MSTI_PORT_FLD_RCVDTCACK:
        p_db_msti_port->rcvdTcAck = p_msti_port->rcvdTcAck;
        break;

    case TBL_MSTI_PORT_FLD_RCVDTCN:
        p_db_msti_port->rcvdTcn = p_msti_port->rcvdTcn;
        break;

    case TBL_MSTI_PORT_FLD_REROOT:
        p_db_msti_port->reRoot = p_msti_port->reRoot;
        break;

    case TBL_MSTI_PORT_FLD_RESELECT:
        p_db_msti_port->reselect = p_msti_port->reselect;
        break;

    case TBL_MSTI_PORT_FLD_ROLE:
        p_db_msti_port->role = p_msti_port->role;
        break;

    case TBL_MSTI_PORT_FLD_SELECTED:
        p_db_msti_port->selected = p_msti_port->selected;
        break;

    case TBL_MSTI_PORT_FLD_SELECTEDROLE:
        p_db_msti_port->selectedRole = p_msti_port->selectedRole;
        break;

    case TBL_MSTI_PORT_FLD_SYNC:
        p_db_msti_port->sync = p_msti_port->sync;
        break;

    case TBL_MSTI_PORT_FLD_SYNCED:
        p_db_msti_port->synced = p_msti_port->synced;
        break;

    case TBL_MSTI_PORT_FLD_TCACK:
        p_db_msti_port->tcAck = p_msti_port->tcAck;
        break;

    case TBL_MSTI_PORT_FLD_TCPROP:
        p_db_msti_port->tcProp = p_msti_port->tcProp;
        break;

    case TBL_MSTI_PORT_FLD_TXCOUNT:
        p_db_msti_port->txCount = p_msti_port->txCount;
        break;

    case TBL_MSTI_PORT_FLD_UPDTINFO:
        p_db_msti_port->updtInfo = p_msti_port->updtInfo;
        break;

    case TBL_MSTI_PORT_FLD_REROOTED:
        p_db_msti_port->reRooted = p_msti_port->reRooted;
        break;

    case TBL_MSTI_PORT_FLD_HOPCOUNT:
        p_db_msti_port->hopCount = p_msti_port->hopCount;
        break;

    case TBL_MSTI_PORT_FLD_PPM_STATE:
        p_db_msti_port->ppm_state = p_msti_port->ppm_state;
        break;

    case TBL_MSTI_PORT_FLD_PIM_STATE:
        p_db_msti_port->pim_state = p_msti_port->pim_state;
        break;

    case TBL_MSTI_PORT_FLD_PRT_STATE:
        p_db_msti_port->prt_state = p_msti_port->prt_state;
        break;

    case TBL_MSTI_PORT_FLD_PST_STATE:
        p_db_msti_port->pst_state = p_msti_port->pst_state;
        break;

    case TBL_MSTI_PORT_FLD_TCM_STATE:
        p_db_msti_port->tcm_state = p_msti_port->tcm_state;
        break;

    case TBL_MSTI_PORT_FLD_FEA_STATE:
        p_db_msti_port->fea_state = p_msti_port->fea_state;
        break;

    case TBL_MSTI_PORT_FLD_BPDU:
        p_db_msti_port->bpdu = p_msti_port->bpdu;
        break;

    case TBL_MSTI_PORT_FLD_FDWHILE_TIMER:
        p_db_msti_port->fdWhile_timer = p_msti_port->fdWhile_timer;
        break;

    case TBL_MSTI_PORT_FLD_MDELAYWHILE_TIMER:
        p_db_msti_port->mdelayWhile_timer = p_msti_port->mdelayWhile_timer;
        break;

    case TBL_MSTI_PORT_FLD_RBWHILE_TIMER:
        p_db_msti_port->rbWhile_timer = p_msti_port->rbWhile_timer;
        break;

    case TBL_MSTI_PORT_FLD_RCVDINFOWHILE_TIMER:
        p_db_msti_port->rcvdInfoWhile_timer = p_msti_port->rcvdInfoWhile_timer;
        break;

    case TBL_MSTI_PORT_FLD_RRWHILE_TIMER:
        p_db_msti_port->rrWhile_timer = p_msti_port->rrWhile_timer;
        break;

    case TBL_MSTI_PORT_FLD_TCWHILE_TIMER:
        p_db_msti_port->tcWhile_timer = p_msti_port->tcWhile_timer;
        break;

    case TBL_MSTI_PORT_FLD_HOLD_TIMER:
        p_db_msti_port->hold_timer = p_msti_port->hold_timer;
        break;

    case TBL_MSTI_PORT_FLD_MAX:
        sal_memcpy(p_db_msti_port, p_msti_port, sizeof(tbl_msti_port_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_MSTI_PORT, field_id, p_db_msti_port);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_MSTI_PORT, field_id, p_db_msti_port);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_msti_port_add_msti_port(tbl_msti_port_t *p_msti_port)
{
    return tbl_msti_port_add_msti_port_sync(p_msti_port, TRUE);
}

int32
tbl_msti_port_del_msti_port(tbl_msti_port_key_t *p_msti_port_key)
{
    return tbl_msti_port_del_msti_port_sync(p_msti_port_key, TRUE);
}

int32
tbl_msti_port_set_msti_port_field(tbl_msti_port_t *p_msti_port, tbl_msti_port_field_id_t field_id)
{
    return tbl_msti_port_set_msti_port_field_sync(p_msti_port, field_id, TRUE);
}

tbl_msti_port_t*
tbl_msti_port_get_msti_port(tbl_msti_port_key_t *p_msti_port_key)
{
    tbl_msti_port_master_t *p_master = _g_p_tbl_msti_port_master;
    tbl_msti_port_t lkp;

    sal_memcpy(&lkp.key, p_msti_port_key, sizeof(tbl_msti_port_key_t));
    return ctclib_hash_lookup(p_master->port_msti_hash, &lkp);
}

char*
tbl_msti_port_key_val2str(tbl_msti_port_t *p_msti_port, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    char buf[MAX_CMD_STR_LEN];

    sal_snprintf(str, str_len, "%s", 
            cdb_msti_port_key_val2str(buf, MAX_CMD_STR_LEN, &p_msti_port->key));
    return str;
}

int32
tbl_msti_port_key_str2val(char *str, tbl_msti_port_t *p_msti_port)
{
    int32 ret = 0;

    ret = cdb_msti_port_key_str2val(str, &p_msti_port->key);
    return ret;
}

char*
tbl_msti_port_key_name_dump(tbl_msti_port_t *p_msti_port, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_MSTI_PORT);
    char buf[MAX_CMD_STR_LEN];

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
            cdb_msti_port_key_val2str(buf, MAX_CMD_STR_LEN, &p_msti_port->key));
    return str;
}

char*
tbl_msti_port_key_value_dump(tbl_msti_port_t *p_msti_port, char *str)
{

    cdb_msti_port_key_val2str(str, MAX_CMD_STR_LEN, &p_msti_port->key);
    return str;
}

char*
tbl_msti_port_field_name_dump(tbl_msti_port_t *p_msti_port, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_MSTI_PORT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_MSTI_PORT_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_msti_port_field_value_dump(tbl_msti_port_t *p_msti_port, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_KEY, field_id))
    {
        cdb_msti_port_key_val2str(str, MAX_CMD_STR_LEN, &p_msti_port->key);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->ifindex);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_ADMINPORTPATHCOST, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->adminPortPathCost);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PORTPATHCOST, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->portPathCost);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_FORWARD_TRANSITIONS, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->forward_transitions);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_NEWINFOMSTI, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->newInfoMsti);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_BPDU_LOOP_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->bpdu_loop_count);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RESTRICTED_ROLE, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->restricted_role);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RESTRICTED_TCN, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->restricted_tcn);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_OPER_ROOTGUARD, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->oper_rootguard);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_OPER_LOOPGUARD, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->oper_loopguard);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_AGREE, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->agree);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_AGREED, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->agreed);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_ADMINFORCEVERSION, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->adminForceVersion);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_DESIGNATED_REGIONROOTID, field_id))
    {
        cdb_mstp_brg_id_val2str(str, MAX_CMD_STR_LEN, &p_msti_port->designated_regionRootId);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_DESIGNATED_INTROOTPATHCOST, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->designated_intRootPathCost);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_DESIGNATED_DESIGNATEDBRIDGEID, field_id))
    {
        cdb_mstp_brg_id_val2str(str, MAX_CMD_STR_LEN, &p_msti_port->designated_designatedBridgeId);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_DESIGNATED_DESIGNATEDPORTID, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->designated_designatedPortId);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_DESIGNATED_BRIDGEPORTID, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->designated_bridgePortId);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_DESIGNATED_MESSAGEAGE, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->designated_messageAge);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_DESIGNATED_FWDDELAY, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->designated_fwdDelay);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_DESIGNATED_HELLOTIME, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->designated_helloTime);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_DESIGNATED_MAXAGE, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->designated_maxAge);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_DISPUTED, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->disputed);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_FDBFLUSH, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->fdbFlush);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_FORWARD, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->forward);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_FORWARDING, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->forwarding);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_INFOIS, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(mstp_info_strs, MSTP_INFO_MAX, p_msti_port->infoIs));
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_LEARN, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->learn);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_LEARNING, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->learning);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_NEWINFO, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->newInfo);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PORTID, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->portId);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PRIORITY, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->priority);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PORT_ROOTID, field_id))
    {
        cdb_mstp_brg_id_val2str(str, MAX_CMD_STR_LEN, &p_msti_port->port_rootId);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PORT_ROOTPATHCOST, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->port_rootPathCost);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PORT_DESIGNATEDBRIDGEID, field_id))
    {
        cdb_mstp_brg_id_val2str(str, MAX_CMD_STR_LEN, &p_msti_port->port_designatedBridgeId);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PORT_DESIGNATEDPORTID, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->port_designatedPortId);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PORT_BRIDGEPORTID, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->port_bridgePortId);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PORT_MESSAGEAGE, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->port_messageAge);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PORT_FWDDELAY, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->port_fwdDelay);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PORT_HELLOTIME, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->port_helloTime);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PORT_MAXAGE, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->port_maxAge);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PROPOSED, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->proposed);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PROPOSING, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->proposing);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RCVDBPDU, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->rcvdBpdu);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RCVDINFO, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(mstp_rcvd_info_strs, MSTP_RCVD_INFO_MAX, p_msti_port->rcvdInfo));
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RCVDMSG, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->rcvdMsg);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RCVDRSTP, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->rcvdRSTP);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RCVDSTP, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->rcvdSTP);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RCVDTC, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->rcvdTc);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RCVDTCACK, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->rcvdTcAck);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RCVDTCN, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->rcvdTcn);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_REROOT, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->reRoot);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RESELECT, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->reselect);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_ROLE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(mstp_port_role_strs, MSTP_PORT_ROLE_MAX, p_msti_port->role));
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_SELECTED, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->selected);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_SELECTEDROLE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(mstp_port_role_strs, MSTP_PORT_ROLE_MAX, p_msti_port->selectedRole));
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_SYNC, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->sync);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_SYNCED, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->synced);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_TCACK, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->tcAck);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_TCPROP, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->tcProp);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_TXCOUNT, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->txCount);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_UPDTINFO, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->updtInfo);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_REROOTED, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->reRooted);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_HOPCOUNT, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->hopCount);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PPM_STATE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(ppm_state_strs, PPM_STATE_MAX, p_msti_port->ppm_state));
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PIM_STATE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(pim_state_strs, PIM_STATE_MAX, p_msti_port->pim_state));
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PRT_STATE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(prt_state_strs, PRT_STATE_MAX, p_msti_port->prt_state));
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PST_STATE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(pst_state_strs, PST_STATE_MAX, p_msti_port->pst_state));
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_TCM_STATE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(tcm_state_strs, TCM_STATE_MAX, p_msti_port->tcm_state));
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_FEA_STATE, field_id))
    {
        sal_sprintf(str, "%u", p_msti_port->fea_state);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_BPDU, field_id))
    {
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_FDWHILE_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_msti_port->fdWhile_timer);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_MDELAYWHILE_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_msti_port->mdelayWhile_timer);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RBWHILE_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_msti_port->rbWhile_timer);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RCVDINFOWHILE_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_msti_port->rcvdInfoWhile_timer);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RRWHILE_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_msti_port->rrWhile_timer);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_TCWHILE_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_msti_port->tcWhile_timer);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_HOLD_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_msti_port->hold_timer);
    }
    return str;
}

char**
tbl_msti_port_table_dump(tbl_msti_port_t *p_msti_port, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_msti_port_key_name_dump(p_msti_port, buf));
    for(i=1; i<TBL_MSTI_PORT_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_msti_port_field_name_dump(p_msti_port, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_msti_port_field_value_dump(p_msti_port, i, buf));
    }
    return str;
}

int32
tbl_msti_port_field_value_parser(char *str, int32 field_id, tbl_msti_port_t *p_msti_port)
{
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_KEY, field_id))
    {
        cdb_msti_port_key_str2val(str, &p_msti_port->key);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_IFINDEX, field_id))
    {
        int32 ret;
        p_msti_port->ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_ADMINPORTPATHCOST, field_id))
    {
        int32 ret;
        p_msti_port->adminPortPathCost = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PORTPATHCOST, field_id))
    {
        int32 ret;
        p_msti_port->portPathCost = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_FORWARD_TRANSITIONS, field_id))
    {
        int32 ret;
        p_msti_port->forward_transitions = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_NEWINFOMSTI, field_id))
    {
        int32 ret;
        p_msti_port->newInfoMsti = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_BPDU_LOOP_COUNT, field_id))
    {
        int32 ret;
        p_msti_port->bpdu_loop_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RESTRICTED_ROLE, field_id))
    {
        int32 ret;
        p_msti_port->restricted_role = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RESTRICTED_TCN, field_id))
    {
        int32 ret;
        p_msti_port->restricted_tcn = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_OPER_ROOTGUARD, field_id))
    {
        int32 ret;
        p_msti_port->oper_rootguard = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_OPER_LOOPGUARD, field_id))
    {
        int32 ret;
        p_msti_port->oper_loopguard = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_AGREE, field_id))
    {
        int32 ret;
        p_msti_port->agree = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_AGREED, field_id))
    {
        int32 ret;
        p_msti_port->agreed = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_ADMINFORCEVERSION, field_id))
    {
        int32 ret;
        p_msti_port->adminForceVersion = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_DESIGNATED_REGIONROOTID, field_id))
    {
        cdb_mstp_brg_id_str2val(str, &p_msti_port->designated_regionRootId);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_DESIGNATED_INTROOTPATHCOST, field_id))
    {
        int32 ret;
        p_msti_port->designated_intRootPathCost = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_DESIGNATED_DESIGNATEDBRIDGEID, field_id))
    {
        cdb_mstp_brg_id_str2val(str, &p_msti_port->designated_designatedBridgeId);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_DESIGNATED_DESIGNATEDPORTID, field_id))
    {
        int32 ret;
        p_msti_port->designated_designatedPortId = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_DESIGNATED_BRIDGEPORTID, field_id))
    {
        int32 ret;
        p_msti_port->designated_bridgePortId = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_DESIGNATED_MESSAGEAGE, field_id))
    {
        int32 ret;
        p_msti_port->designated_messageAge = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_DESIGNATED_FWDDELAY, field_id))
    {
        int32 ret;
        p_msti_port->designated_fwdDelay = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_DESIGNATED_HELLOTIME, field_id))
    {
        int32 ret;
        p_msti_port->designated_helloTime = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_DESIGNATED_MAXAGE, field_id))
    {
        int32 ret;
        p_msti_port->designated_maxAge = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_DISPUTED, field_id))
    {
        int32 ret;
        p_msti_port->disputed = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_FDBFLUSH, field_id))
    {
        int32 ret;
        p_msti_port->fdbFlush = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_FORWARD, field_id))
    {
        int32 ret;
        p_msti_port->forward = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_FORWARDING, field_id))
    {
        int32 ret;
        p_msti_port->forwarding = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_INFOIS, field_id))
    {
        cdb_enum_str2val(mstp_info_strs, MSTP_INFO_MAX, str);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_LEARN, field_id))
    {
        int32 ret;
        p_msti_port->learn = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_LEARNING, field_id))
    {
        int32 ret;
        p_msti_port->learning = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_NEWINFO, field_id))
    {
        int32 ret;
        p_msti_port->newInfo = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PORTID, field_id))
    {
        int32 ret;
        p_msti_port->portId = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PRIORITY, field_id))
    {
        int32 ret;
        p_msti_port->priority = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PORT_ROOTID, field_id))
    {
        cdb_mstp_brg_id_str2val(str, &p_msti_port->port_rootId);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PORT_ROOTPATHCOST, field_id))
    {
        int32 ret;
        p_msti_port->port_rootPathCost = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PORT_DESIGNATEDBRIDGEID, field_id))
    {
        cdb_mstp_brg_id_str2val(str, &p_msti_port->port_designatedBridgeId);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PORT_DESIGNATEDPORTID, field_id))
    {
        int32 ret;
        p_msti_port->port_designatedPortId = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PORT_BRIDGEPORTID, field_id))
    {
        int32 ret;
        p_msti_port->port_bridgePortId = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PORT_MESSAGEAGE, field_id))
    {
        int32 ret;
        p_msti_port->port_messageAge = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PORT_FWDDELAY, field_id))
    {
        int32 ret;
        p_msti_port->port_fwdDelay = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PORT_HELLOTIME, field_id))
    {
        int32 ret;
        p_msti_port->port_helloTime = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PORT_MAXAGE, field_id))
    {
        int32 ret;
        p_msti_port->port_maxAge = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PROPOSED, field_id))
    {
        int32 ret;
        p_msti_port->proposed = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PROPOSING, field_id))
    {
        int32 ret;
        p_msti_port->proposing = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RCVDBPDU, field_id))
    {
        int32 ret;
        p_msti_port->rcvdBpdu = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RCVDINFO, field_id))
    {
        cdb_enum_str2val(mstp_rcvd_info_strs, MSTP_RCVD_INFO_MAX, str);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RCVDMSG, field_id))
    {
        int32 ret;
        p_msti_port->rcvdMsg = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RCVDRSTP, field_id))
    {
        int32 ret;
        p_msti_port->rcvdRSTP = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RCVDSTP, field_id))
    {
        int32 ret;
        p_msti_port->rcvdSTP = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RCVDTC, field_id))
    {
        int32 ret;
        p_msti_port->rcvdTc = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RCVDTCACK, field_id))
    {
        int32 ret;
        p_msti_port->rcvdTcAck = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RCVDTCN, field_id))
    {
        int32 ret;
        p_msti_port->rcvdTcn = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_REROOT, field_id))
    {
        int32 ret;
        p_msti_port->reRoot = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RESELECT, field_id))
    {
        int32 ret;
        p_msti_port->reselect = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_ROLE, field_id))
    {
        cdb_enum_str2val(mstp_port_role_strs, MSTP_PORT_ROLE_MAX, str);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_SELECTED, field_id))
    {
        int32 ret;
        p_msti_port->selected = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_SELECTEDROLE, field_id))
    {
        cdb_enum_str2val(mstp_port_role_strs, MSTP_PORT_ROLE_MAX, str);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_SYNC, field_id))
    {
        int32 ret;
        p_msti_port->sync = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_SYNCED, field_id))
    {
        int32 ret;
        p_msti_port->synced = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_TCACK, field_id))
    {
        int32 ret;
        p_msti_port->tcAck = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_TCPROP, field_id))
    {
        int32 ret;
        p_msti_port->tcProp = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_TXCOUNT, field_id))
    {
        int32 ret;
        p_msti_port->txCount = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_UPDTINFO, field_id))
    {
        int32 ret;
        p_msti_port->updtInfo = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_REROOTED, field_id))
    {
        int32 ret;
        p_msti_port->reRooted = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_HOPCOUNT, field_id))
    {
        int32 ret;
        p_msti_port->hopCount = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PPM_STATE, field_id))
    {
        cdb_enum_str2val(ppm_state_strs, PPM_STATE_MAX, str);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PIM_STATE, field_id))
    {
        cdb_enum_str2val(pim_state_strs, PIM_STATE_MAX, str);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PRT_STATE, field_id))
    {
        cdb_enum_str2val(prt_state_strs, PRT_STATE_MAX, str);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PST_STATE, field_id))
    {
        cdb_enum_str2val(pst_state_strs, PST_STATE_MAX, str);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_TCM_STATE, field_id))
    {
        cdb_enum_str2val(tcm_state_strs, TCM_STATE_MAX, str);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_FEA_STATE, field_id))
    {
        int32 ret;
        p_msti_port->fea_state = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_BPDU, field_id))
    {
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_FDWHILE_TIMER, field_id))
    {
        ctc_task_str2val(str, p_msti_port->fdWhile_timer);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_MDELAYWHILE_TIMER, field_id))
    {
        ctc_task_str2val(str, p_msti_port->mdelayWhile_timer);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RBWHILE_TIMER, field_id))
    {
        ctc_task_str2val(str, p_msti_port->rbWhile_timer);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RCVDINFOWHILE_TIMER, field_id))
    {
        ctc_task_str2val(str, p_msti_port->rcvdInfoWhile_timer);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RRWHILE_TIMER, field_id))
    {
        ctc_task_str2val(str, p_msti_port->rrWhile_timer);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_TCWHILE_TIMER, field_id))
    {
        ctc_task_str2val(str, p_msti_port->tcWhile_timer);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_HOLD_TIMER, field_id))
    {
        ctc_task_str2val(str, p_msti_port->hold_timer);
    }
    return PM_E_NONE;
}

int32
tbl_msti_port_table_parser(char** array, char* key_value,tbl_msti_port_t *p_msti_port)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_MSTI_PORT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_msti_port_key_str2val(key_value, p_msti_port));

    for(i=1; i<TBL_MSTI_PORT_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_msti_port_field_value_parser( array[j++], i, p_msti_port));
    }

    return PM_E_NONE;
}

int32
tbl_msti_port_dump_one(tbl_msti_port_t *p_msti_port, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_MSTI_PORT);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_MSTI_PORT_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTI_PORT_FLD_KEY].name, 
            cdb_msti_port_key_val2str(buf, MAX_CMD_STR_LEN, &p_msti_port->key));
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_IFINDEX].name,
            p_msti_port->ifindex);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_ADMINPORTPATHCOST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_ADMINPORTPATHCOST].name,
            p_msti_port->adminPortPathCost);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PORTPATHCOST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_PORTPATHCOST].name,
            p_msti_port->portPathCost);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_FORWARD_TRANSITIONS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_FORWARD_TRANSITIONS].name,
            p_msti_port->forward_transitions);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_NEWINFOMSTI, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_NEWINFOMSTI].name,
            p_msti_port->newInfoMsti);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_BPDU_LOOP_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_BPDU_LOOP_COUNT].name,
            p_msti_port->bpdu_loop_count);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RESTRICTED_ROLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_RESTRICTED_ROLE].name,
            p_msti_port->restricted_role);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RESTRICTED_TCN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_RESTRICTED_TCN].name,
            p_msti_port->restricted_tcn);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_OPER_ROOTGUARD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_OPER_ROOTGUARD].name,
            p_msti_port->oper_rootguard);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_OPER_LOOPGUARD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_OPER_LOOPGUARD].name,
            p_msti_port->oper_loopguard);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_AGREE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_AGREE].name,
            p_msti_port->agree);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_AGREED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_AGREED].name,
            p_msti_port->agreed);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_ADMINFORCEVERSION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_ADMINFORCEVERSION].name,
            p_msti_port->adminForceVersion);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_DESIGNATED_REGIONROOTID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTI_PORT_FLD_DESIGNATED_REGIONROOTID].name, 
            cdb_mstp_brg_id_val2str(buf, MAX_CMD_STR_LEN, &p_msti_port->designated_regionRootId));
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_DESIGNATED_INTROOTPATHCOST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_DESIGNATED_INTROOTPATHCOST].name,
            p_msti_port->designated_intRootPathCost);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_DESIGNATED_DESIGNATEDBRIDGEID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTI_PORT_FLD_DESIGNATED_DESIGNATEDBRIDGEID].name, 
            cdb_mstp_brg_id_val2str(buf, MAX_CMD_STR_LEN, &p_msti_port->designated_designatedBridgeId));
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_DESIGNATED_DESIGNATEDPORTID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_DESIGNATED_DESIGNATEDPORTID].name,
            p_msti_port->designated_designatedPortId);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_DESIGNATED_BRIDGEPORTID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_DESIGNATED_BRIDGEPORTID].name,
            p_msti_port->designated_bridgePortId);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_DESIGNATED_MESSAGEAGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_DESIGNATED_MESSAGEAGE].name,
            p_msti_port->designated_messageAge);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_DESIGNATED_FWDDELAY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_DESIGNATED_FWDDELAY].name,
            p_msti_port->designated_fwdDelay);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_DESIGNATED_HELLOTIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_DESIGNATED_HELLOTIME].name,
            p_msti_port->designated_helloTime);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_DESIGNATED_MAXAGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_DESIGNATED_MAXAGE].name,
            p_msti_port->designated_maxAge);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_DISPUTED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_DISPUTED].name,
            p_msti_port->disputed);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_FDBFLUSH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_FDBFLUSH].name,
            p_msti_port->fdbFlush);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_FORWARD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_FORWARD].name,
            p_msti_port->forward);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_FORWARDING, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_FORWARDING].name,
            p_msti_port->forwarding);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_INFOIS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTI_PORT_FLD_INFOIS].name, 
            cdb_enum_val2str(mstp_info_strs, MSTP_INFO_MAX, p_msti_port->infoIs));
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_LEARN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_LEARN].name,
            p_msti_port->learn);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_LEARNING, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_LEARNING].name,
            p_msti_port->learning);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_NEWINFO, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_NEWINFO].name,
            p_msti_port->newInfo);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PORTID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_PORTID].name,
            p_msti_port->portId);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PRIORITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_PRIORITY].name,
            p_msti_port->priority);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PORT_ROOTID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTI_PORT_FLD_PORT_ROOTID].name, 
            cdb_mstp_brg_id_val2str(buf, MAX_CMD_STR_LEN, &p_msti_port->port_rootId));
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PORT_ROOTPATHCOST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_PORT_ROOTPATHCOST].name,
            p_msti_port->port_rootPathCost);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PORT_DESIGNATEDBRIDGEID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTI_PORT_FLD_PORT_DESIGNATEDBRIDGEID].name, 
            cdb_mstp_brg_id_val2str(buf, MAX_CMD_STR_LEN, &p_msti_port->port_designatedBridgeId));
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PORT_DESIGNATEDPORTID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_PORT_DESIGNATEDPORTID].name,
            p_msti_port->port_designatedPortId);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PORT_BRIDGEPORTID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_PORT_BRIDGEPORTID].name,
            p_msti_port->port_bridgePortId);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PORT_MESSAGEAGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_PORT_MESSAGEAGE].name,
            p_msti_port->port_messageAge);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PORT_FWDDELAY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_PORT_FWDDELAY].name,
            p_msti_port->port_fwdDelay);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PORT_HELLOTIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_PORT_HELLOTIME].name,
            p_msti_port->port_helloTime);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PORT_MAXAGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_PORT_MAXAGE].name,
            p_msti_port->port_maxAge);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PROPOSED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_PROPOSED].name,
            p_msti_port->proposed);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PROPOSING, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_PROPOSING].name,
            p_msti_port->proposing);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RCVDBPDU, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_RCVDBPDU].name,
            p_msti_port->rcvdBpdu);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RCVDINFO, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTI_PORT_FLD_RCVDINFO].name, 
            cdb_enum_val2str(mstp_rcvd_info_strs, MSTP_RCVD_INFO_MAX, p_msti_port->rcvdInfo));
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RCVDMSG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_RCVDMSG].name,
            p_msti_port->rcvdMsg);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RCVDRSTP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_RCVDRSTP].name,
            p_msti_port->rcvdRSTP);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RCVDSTP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_RCVDSTP].name,
            p_msti_port->rcvdSTP);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RCVDTC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_RCVDTC].name,
            p_msti_port->rcvdTc);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RCVDTCACK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_RCVDTCACK].name,
            p_msti_port->rcvdTcAck);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RCVDTCN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_RCVDTCN].name,
            p_msti_port->rcvdTcn);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_REROOT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_REROOT].name,
            p_msti_port->reRoot);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RESELECT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_RESELECT].name,
            p_msti_port->reselect);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_ROLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTI_PORT_FLD_ROLE].name, 
            cdb_enum_val2str(mstp_port_role_strs, MSTP_PORT_ROLE_MAX, p_msti_port->role));
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_SELECTED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_SELECTED].name,
            p_msti_port->selected);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_SELECTEDROLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTI_PORT_FLD_SELECTEDROLE].name, 
            cdb_enum_val2str(mstp_port_role_strs, MSTP_PORT_ROLE_MAX, p_msti_port->selectedRole));
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_SYNC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_SYNC].name,
            p_msti_port->sync);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_SYNCED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_SYNCED].name,
            p_msti_port->synced);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_TCACK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_TCACK].name,
            p_msti_port->tcAck);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_TCPROP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_TCPROP].name,
            p_msti_port->tcProp);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_TXCOUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_TXCOUNT].name,
            p_msti_port->txCount);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_UPDTINFO, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_UPDTINFO].name,
            p_msti_port->updtInfo);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_REROOTED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_REROOTED].name,
            p_msti_port->reRooted);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_HOPCOUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_HOPCOUNT].name,
            p_msti_port->hopCount);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PPM_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTI_PORT_FLD_PPM_STATE].name, 
            cdb_enum_val2str(ppm_state_strs, PPM_STATE_MAX, p_msti_port->ppm_state));
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PIM_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTI_PORT_FLD_PIM_STATE].name, 
            cdb_enum_val2str(pim_state_strs, PIM_STATE_MAX, p_msti_port->pim_state));
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PRT_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTI_PORT_FLD_PRT_STATE].name, 
            cdb_enum_val2str(prt_state_strs, PRT_STATE_MAX, p_msti_port->prt_state));
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_PST_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTI_PORT_FLD_PST_STATE].name, 
            cdb_enum_val2str(pst_state_strs, PST_STATE_MAX, p_msti_port->pst_state));
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_TCM_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTI_PORT_FLD_TCM_STATE].name, 
            cdb_enum_val2str(tcm_state_strs, TCM_STATE_MAX, p_msti_port->tcm_state));
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_FEA_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTI_PORT_FLD_FEA_STATE].name,
            p_msti_port->fea_state);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_BPDU, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"",
            p_tbl_info->field[TBL_MSTI_PORT_FLD_BPDU].name);
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_FDWHILE_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTI_PORT_FLD_FDWHILE_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_msti_port->fdWhile_timer));
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_MDELAYWHILE_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTI_PORT_FLD_MDELAYWHILE_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_msti_port->mdelayWhile_timer));
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RBWHILE_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTI_PORT_FLD_RBWHILE_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_msti_port->rbWhile_timer));
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RCVDINFOWHILE_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTI_PORT_FLD_RCVDINFOWHILE_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_msti_port->rcvdInfoWhile_timer));
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_RRWHILE_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTI_PORT_FLD_RRWHILE_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_msti_port->rrWhile_timer));
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_TCWHILE_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTI_PORT_FLD_TCWHILE_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_msti_port->tcWhile_timer));
    }
    if (FLD_MATCH(TBL_MSTI_PORT_FLD_HOLD_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTI_PORT_FLD_HOLD_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_msti_port->hold_timer));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_msti_port_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_msti_port_master_t *p_master = _g_p_tbl_msti_port_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_msti_port_t *p_db_msti_port = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->port_msti_list, p_db_msti_port, listnode, next)
    {
        rc |= fn(p_db_msti_port, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_msti_port_master_t*
tbl_msti_port_get_master()
{
    return _g_p_tbl_msti_port_master;
}

tbl_msti_port_master_t*
tbl_msti_port_init_msti_port()
{
    _g_p_tbl_msti_port_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_msti_port_master_t));
    _g_p_tbl_msti_port_master->port_msti_hash = ctclib_hash_create(_tbl_msti_port_hash_make, _tbl_msti_port_hash_cmp);
    _g_p_tbl_msti_port_master->port_msti_list = ctclib_slist_create(tbl_msti_port_cmp, NULL);
    return _g_p_tbl_msti_port_master;
}

