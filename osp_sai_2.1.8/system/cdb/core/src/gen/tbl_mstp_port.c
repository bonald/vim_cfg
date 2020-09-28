
#include "proto.h"
#include "gen/tbl_mstp_port_define.h"
#include "gen/tbl_mstp_port.h"
#include "gen/tbl_msti_port_define.h"
#include "gen/tbl_msti_port.h"
#include "cdb_data_cmp.h"

tbl_mstp_port_master_t *_g_p_tbl_mstp_port_master = NULL;

#include "mstp_define.h"

static uint32
_tbl_mstp_port_hash_make(tbl_mstp_port_t *p_mstp_port)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_mstp_port->key;
    for (index = 0; index < sizeof(p_mstp_port->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_mstp_port_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_mstp_port_t *p_mstp_port1 = (tbl_mstp_port_t*)p_arg1;
    tbl_mstp_port_t *p_mstp_port2 = (tbl_mstp_port_t*)p_arg2;

    if (0 == sal_memcmp(&p_mstp_port1->key, &p_mstp_port2->key, sizeof(tbl_mstp_port_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_mstp_port_add_mstp_port_sync(tbl_mstp_port_t *p_mstp_port, uint32 sync)
{
    tbl_mstp_port_master_t *p_master = _g_p_tbl_mstp_port_master;
    tbl_mstp_port_t *p_db_mstp_port = NULL;
    void *p_ret = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    if (tbl_mstp_port_get_mstp_port(&p_mstp_port->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_mstp_port = XCALLOC(MEM_TBL_MSTP_PORT, sizeof(tbl_mstp_port_t));
    if (NULL == p_db_mstp_port)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_mstp_port, p_mstp_port, sizeof(tbl_mstp_port_t));
    PM_E_RETURN(cdb_reference_list_init(&p_db_mstp_port->msti_list, CDB_NODE_TBL, TBL_MSTI_PORT, tbl_msti_port_cmp));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->port_hash, (void*)p_db_mstp_port, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->port_list, p_db_mstp_port);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_MSTP_PORT, p_db_mstp_port);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_MSTP_PORT, p_db_mstp_port);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_mstp_port_del_mstp_port_sync(tbl_mstp_port_key_t *p_mstp_port_key, uint32 sync)
{
    tbl_mstp_port_master_t *p_master = _g_p_tbl_mstp_port_master;
    tbl_mstp_port_t *p_db_mstp_port = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_mstp_port = tbl_mstp_port_get_mstp_port(p_mstp_port_key);
    if (NULL == p_db_mstp_port)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_MSTP_PORT, p_db_mstp_port);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_MSTP_PORT, p_db_mstp_port);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->port_hash, (void*)p_db_mstp_port);
    ctclib_slistnode_delete(p_master->port_list, p_db_mstp_port);
    PM_E_RETURN(cdb_reference_list_deinit(&p_db_mstp_port->msti_list));

    /* 4. free db entry */
    XFREE(MEM_TBL_MSTP_PORT, p_db_mstp_port);

    return PM_E_NONE;
}

int32
tbl_mstp_port_set_mstp_port_field_sync(tbl_mstp_port_t *p_mstp_port, tbl_mstp_port_field_id_t field_id, uint32 sync)
{
    tbl_mstp_port_t *p_db_mstp_port = NULL;

    /* 1. lookup entry exist */
    p_db_mstp_port = tbl_mstp_port_get_mstp_port(&p_mstp_port->key);
    if (NULL == p_db_mstp_port)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_MSTP_PORT_FLD_IFINDEX:
        p_db_mstp_port->ifindex = p_mstp_port->ifindex;
        break;

    case TBL_MSTP_PORT_FLD_ADMINPORTPATHCOST:
        p_db_mstp_port->adminPortPathCost = p_mstp_port->adminPortPathCost;
        break;

    case TBL_MSTP_PORT_FLD_PORTPATHCOST:
        p_db_mstp_port->portPathCost = p_mstp_port->portPathCost;
        break;

    case TBL_MSTP_PORT_FLD_FORWARD_TRANSITIONS:
        p_db_mstp_port->forward_transitions = p_mstp_port->forward_transitions;
        break;

    case TBL_MSTP_PORT_FLD_TX_BPDU_COUNT:
        p_db_mstp_port->tx_bpdu_count = p_mstp_port->tx_bpdu_count;
        break;

    case TBL_MSTP_PORT_FLD_TX_ERROR_COUNT:
        p_db_mstp_port->tx_error_count = p_mstp_port->tx_error_count;
        break;

    case TBL_MSTP_PORT_FLD_RX_BPDU_COUNT:
        p_db_mstp_port->rx_bpdu_count = p_mstp_port->rx_bpdu_count;
        break;

    case TBL_MSTP_PORT_FLD_RX_ERROR_COUNT:
        p_db_mstp_port->rx_error_count = p_mstp_port->rx_error_count;
        break;

    case TBL_MSTP_PORT_FLD_PORT_ENABLE:
        p_db_mstp_port->port_enable = p_mstp_port->port_enable;
        break;

    case TBL_MSTP_PORT_FLD_BPDU_LOOP_COUNT:
        p_db_mstp_port->bpdu_loop_count = p_mstp_port->bpdu_loop_count;
        break;

    case TBL_MSTP_PORT_FLD_RESTRICTED_ROLE:
        p_db_mstp_port->restricted_role = p_mstp_port->restricted_role;
        break;

    case TBL_MSTP_PORT_FLD_RESTRICTED_TCN:
        p_db_mstp_port->restricted_tcn = p_mstp_port->restricted_tcn;
        break;

    case TBL_MSTP_PORT_FLD_ADMIN_BPDUGUARD:
        p_db_mstp_port->admin_bpduguard = p_mstp_port->admin_bpduguard;
        break;

    case TBL_MSTP_PORT_FLD_OPER_BPDUGUARD:
        p_db_mstp_port->oper_bpduguard = p_mstp_port->oper_bpduguard;
        break;

    case TBL_MSTP_PORT_FLD_ADMIN_BPDUFILTER:
        p_db_mstp_port->admin_bpdufilter = p_mstp_port->admin_bpdufilter;
        break;

    case TBL_MSTP_PORT_FLD_OPER_BPDUFILTER:
        p_db_mstp_port->oper_bpdufilter = p_mstp_port->oper_bpdufilter;
        break;

    case TBL_MSTP_PORT_FLD_ADMIN_ROOTGUARD:
        p_db_mstp_port->admin_rootguard = p_mstp_port->admin_rootguard;
        break;

    case TBL_MSTP_PORT_FLD_OPER_ROOTGUARD:
        p_db_mstp_port->oper_rootguard = p_mstp_port->oper_rootguard;
        break;

    case TBL_MSTP_PORT_FLD_ADMIN_LOOPGUARD:
        p_db_mstp_port->admin_loopguard = p_mstp_port->admin_loopguard;
        break;

    case TBL_MSTP_PORT_FLD_OPER_LOOPGUARD:
        p_db_mstp_port->oper_loopguard = p_mstp_port->oper_loopguard;
        break;

    case TBL_MSTP_PORT_FLD_CISCO_CFG_FORMAT_ID:
        p_db_mstp_port->cisco_cfg_format_id = p_mstp_port->cisco_cfg_format_id;
        break;

    case TBL_MSTP_PORT_FLD_ADMINP2PMAC:
        p_db_mstp_port->adminP2PMac = p_mstp_port->adminP2PMac;
        break;

    case TBL_MSTP_PORT_FLD_OPERP2PMAC:
        p_db_mstp_port->operP2PMac = p_mstp_port->operP2PMac;
        break;

    case TBL_MSTP_PORT_FLD_AGREE:
        p_db_mstp_port->agree = p_mstp_port->agree;
        break;

    case TBL_MSTP_PORT_FLD_AGREED:
        p_db_mstp_port->agreed = p_mstp_port->agreed;
        break;

    case TBL_MSTP_PORT_FLD_ADMINFORCEVERSION:
        p_db_mstp_port->adminForceVersion = p_mstp_port->adminForceVersion;
        break;

    case TBL_MSTP_PORT_FLD_FORCEVERSION:
        p_db_mstp_port->forceVersion = p_mstp_port->forceVersion;
        break;

    case TBL_MSTP_PORT_FLD_DESIGNATED_ROOTID:
        sal_memcpy(&p_db_mstp_port->designated_rootId, &p_mstp_port->designated_rootId, sizeof(p_mstp_port->designated_rootId));
        break;

    case TBL_MSTP_PORT_FLD_DESIGNATED_ROOTPATHCOST:
        p_db_mstp_port->designated_rootPathCost = p_mstp_port->designated_rootPathCost;
        break;

    case TBL_MSTP_PORT_FLD_DESIGNATED_REGIONROOTID:
        sal_memcpy(&p_db_mstp_port->designated_regionRootId, &p_mstp_port->designated_regionRootId, sizeof(p_mstp_port->designated_regionRootId));
        break;

    case TBL_MSTP_PORT_FLD_DESIGNATED_INTROOTPATHCOST:
        p_db_mstp_port->designated_intRootPathCost = p_mstp_port->designated_intRootPathCost;
        break;

    case TBL_MSTP_PORT_FLD_DESIGNATED_DESIGNATEDBRIDGEID:
        sal_memcpy(&p_db_mstp_port->designated_designatedBridgeId, &p_mstp_port->designated_designatedBridgeId, sizeof(p_mstp_port->designated_designatedBridgeId));
        break;

    case TBL_MSTP_PORT_FLD_DESIGNATED_DESIGNATEDPORTID:
        p_db_mstp_port->designated_designatedPortId = p_mstp_port->designated_designatedPortId;
        break;

    case TBL_MSTP_PORT_FLD_DESIGNATED_BRIDGEPORTID:
        p_db_mstp_port->designated_bridgePortId = p_mstp_port->designated_bridgePortId;
        break;

    case TBL_MSTP_PORT_FLD_DESIGNATED_MESSAGEAGE:
        p_db_mstp_port->designated_messageAge = p_mstp_port->designated_messageAge;
        break;

    case TBL_MSTP_PORT_FLD_DESIGNATED_FWDDELAY:
        p_db_mstp_port->designated_fwdDelay = p_mstp_port->designated_fwdDelay;
        break;

    case TBL_MSTP_PORT_FLD_DESIGNATED_HELLOTIME:
        p_db_mstp_port->designated_helloTime = p_mstp_port->designated_helloTime;
        break;

    case TBL_MSTP_PORT_FLD_DESIGNATED_MAXAGE:
        p_db_mstp_port->designated_maxAge = p_mstp_port->designated_maxAge;
        break;

    case TBL_MSTP_PORT_FLD_DISPUTED:
        p_db_mstp_port->disputed = p_mstp_port->disputed;
        break;

    case TBL_MSTP_PORT_FLD_FDBFLUSH:
        p_db_mstp_port->fdbFlush = p_mstp_port->fdbFlush;
        break;

    case TBL_MSTP_PORT_FLD_FORWARD:
        p_db_mstp_port->forward = p_mstp_port->forward;
        break;

    case TBL_MSTP_PORT_FLD_FORWARDING:
        p_db_mstp_port->forwarding = p_mstp_port->forwarding;
        break;

    case TBL_MSTP_PORT_FLD_INFOIS:
        p_db_mstp_port->infoIs = p_mstp_port->infoIs;
        break;

    case TBL_MSTP_PORT_FLD_LEARN:
        p_db_mstp_port->learn = p_mstp_port->learn;
        break;

    case TBL_MSTP_PORT_FLD_LEARNING:
        p_db_mstp_port->learning = p_mstp_port->learning;
        break;

    case TBL_MSTP_PORT_FLD_MCHECK:
        p_db_mstp_port->mcheck = p_mstp_port->mcheck;
        break;

    case TBL_MSTP_PORT_FLD_NEWINFO:
        p_db_mstp_port->newInfo = p_mstp_port->newInfo;
        break;

    case TBL_MSTP_PORT_FLD_NEWINFOMSTI:
        p_db_mstp_port->newInfoMsti = p_mstp_port->newInfoMsti;
        break;

    case TBL_MSTP_PORT_FLD_ADMINEDGE:
        p_db_mstp_port->adminEdge = p_mstp_port->adminEdge;
        break;

    case TBL_MSTP_PORT_FLD_OPEREDGE:
        p_db_mstp_port->operEdge = p_mstp_port->operEdge;
        break;

    case TBL_MSTP_PORT_FLD_PORTENABLED:
        p_db_mstp_port->portEnabled = p_mstp_port->portEnabled;
        break;

    case TBL_MSTP_PORT_FLD_PORTID:
        p_db_mstp_port->portId = p_mstp_port->portId;
        break;

    case TBL_MSTP_PORT_FLD_PRIORITY:
        p_db_mstp_port->priority = p_mstp_port->priority;
        break;

    case TBL_MSTP_PORT_FLD_PORT_ROOTID:
        sal_memcpy(&p_db_mstp_port->port_rootId, &p_mstp_port->port_rootId, sizeof(p_mstp_port->port_rootId));
        break;

    case TBL_MSTP_PORT_FLD_PORT_ROOTPATHCOST:
        p_db_mstp_port->port_rootPathCost = p_mstp_port->port_rootPathCost;
        break;

    case TBL_MSTP_PORT_FLD_PORT_REGIONROOTID:
        sal_memcpy(&p_db_mstp_port->port_regionRootId, &p_mstp_port->port_regionRootId, sizeof(p_mstp_port->port_regionRootId));
        break;

    case TBL_MSTP_PORT_FLD_PORT_INTROOTPATHCOST:
        p_db_mstp_port->port_intRootPathCost = p_mstp_port->port_intRootPathCost;
        break;

    case TBL_MSTP_PORT_FLD_PORT_DESIGNATEDBRIDGEID:
        sal_memcpy(&p_db_mstp_port->port_designatedBridgeId, &p_mstp_port->port_designatedBridgeId, sizeof(p_mstp_port->port_designatedBridgeId));
        break;

    case TBL_MSTP_PORT_FLD_PORT_DESIGNATEDPORTID:
        p_db_mstp_port->port_designatedPortId = p_mstp_port->port_designatedPortId;
        break;

    case TBL_MSTP_PORT_FLD_PORT_BRIDGEPORTID:
        p_db_mstp_port->port_bridgePortId = p_mstp_port->port_bridgePortId;
        break;

    case TBL_MSTP_PORT_FLD_PORT_MESSAGEAGE:
        p_db_mstp_port->port_messageAge = p_mstp_port->port_messageAge;
        break;

    case TBL_MSTP_PORT_FLD_PORT_FWDDELAY:
        p_db_mstp_port->port_fwdDelay = p_mstp_port->port_fwdDelay;
        break;

    case TBL_MSTP_PORT_FLD_PORT_HELLOTIME:
        p_db_mstp_port->port_helloTime = p_mstp_port->port_helloTime;
        break;

    case TBL_MSTP_PORT_FLD_PORT_MAXAGE:
        p_db_mstp_port->port_maxAge = p_mstp_port->port_maxAge;
        break;

    case TBL_MSTP_PORT_FLD_PROPOSED:
        p_db_mstp_port->proposed = p_mstp_port->proposed;
        break;

    case TBL_MSTP_PORT_FLD_PROPOSING:
        p_db_mstp_port->proposing = p_mstp_port->proposing;
        break;

    case TBL_MSTP_PORT_FLD_RCVDBPDU:
        p_db_mstp_port->rcvdBpdu = p_mstp_port->rcvdBpdu;
        break;

    case TBL_MSTP_PORT_FLD_RCVDINFO:
        p_db_mstp_port->rcvdInfo = p_mstp_port->rcvdInfo;
        break;

    case TBL_MSTP_PORT_FLD_RCVDINFOUPDATETIMERS:
        p_db_mstp_port->rcvdInfoUpdateTimers = p_mstp_port->rcvdInfoUpdateTimers;
        break;

    case TBL_MSTP_PORT_FLD_RCVDMSG:
        p_db_mstp_port->rcvdMsg = p_mstp_port->rcvdMsg;
        break;

    case TBL_MSTP_PORT_FLD_RCVDMSTP:
        p_db_mstp_port->rcvdMSTP = p_mstp_port->rcvdMSTP;
        break;

    case TBL_MSTP_PORT_FLD_RCVDRSTP:
        p_db_mstp_port->rcvdRSTP = p_mstp_port->rcvdRSTP;
        break;

    case TBL_MSTP_PORT_FLD_RCVDSTP:
        p_db_mstp_port->rcvdSTP = p_mstp_port->rcvdSTP;
        break;

    case TBL_MSTP_PORT_FLD_RCVDTC:
        p_db_mstp_port->rcvdTc = p_mstp_port->rcvdTc;
        break;

    case TBL_MSTP_PORT_FLD_RCVDTCACK:
        p_db_mstp_port->rcvdTcAck = p_mstp_port->rcvdTcAck;
        break;

    case TBL_MSTP_PORT_FLD_RCVDTCN:
        p_db_mstp_port->rcvdTcn = p_mstp_port->rcvdTcn;
        break;

    case TBL_MSTP_PORT_FLD_REROOT:
        p_db_mstp_port->reRoot = p_mstp_port->reRoot;
        break;

    case TBL_MSTP_PORT_FLD_RESELECT:
        p_db_mstp_port->reselect = p_mstp_port->reselect;
        break;

    case TBL_MSTP_PORT_FLD_ROLE:
        p_db_mstp_port->role = p_mstp_port->role;
        break;

    case TBL_MSTP_PORT_FLD_SELECTED:
        p_db_mstp_port->selected = p_mstp_port->selected;
        break;

    case TBL_MSTP_PORT_FLD_SELECTEDROLE:
        p_db_mstp_port->selectedRole = p_mstp_port->selectedRole;
        break;

    case TBL_MSTP_PORT_FLD_SENDRSTP:
        p_db_mstp_port->sendRstp = p_mstp_port->sendRstp;
        break;

    case TBL_MSTP_PORT_FLD_SYNC:
        p_db_mstp_port->sync = p_mstp_port->sync;
        break;

    case TBL_MSTP_PORT_FLD_SYNCED:
        p_db_mstp_port->synced = p_mstp_port->synced;
        break;

    case TBL_MSTP_PORT_FLD_TCACK:
        p_db_mstp_port->tcAck = p_mstp_port->tcAck;
        break;

    case TBL_MSTP_PORT_FLD_TCPROP:
        p_db_mstp_port->tcProp = p_mstp_port->tcProp;
        break;

    case TBL_MSTP_PORT_FLD_TXCOUNT:
        p_db_mstp_port->txCount = p_mstp_port->txCount;
        break;

    case TBL_MSTP_PORT_FLD_UPDTINFO:
        p_db_mstp_port->updtInfo = p_mstp_port->updtInfo;
        break;

    case TBL_MSTP_PORT_FLD_REROOTED:
        p_db_mstp_port->reRooted = p_mstp_port->reRooted;
        break;

    case TBL_MSTP_PORT_FLD_RSTPVERSION:
        p_db_mstp_port->rstpVersion = p_mstp_port->rstpVersion;
        break;

    case TBL_MSTP_PORT_FLD_STPVERSION:
        p_db_mstp_port->stpVersion = p_mstp_port->stpVersion;
        break;

    case TBL_MSTP_PORT_FLD_RCVDINTERNAL:
        p_db_mstp_port->rcvdInternal = p_mstp_port->rcvdInternal;
        break;

    case TBL_MSTP_PORT_FLD_RCVDINTERNALCHANGE:
        p_db_mstp_port->rcvdInternalChange = p_mstp_port->rcvdInternalChange;
        break;

    case TBL_MSTP_PORT_FLD_HOPCOUNT:
        p_db_mstp_port->hopCount = p_mstp_port->hopCount;
        break;

    case TBL_MSTP_PORT_FLD_PRX_STATE:
        p_db_mstp_port->prx_state = p_mstp_port->prx_state;
        break;

    case TBL_MSTP_PORT_FLD_PPM_STATE:
        p_db_mstp_port->ppm_state = p_mstp_port->ppm_state;
        break;

    case TBL_MSTP_PORT_FLD_BDM_STATE:
        p_db_mstp_port->bdm_state = p_mstp_port->bdm_state;
        break;

    case TBL_MSTP_PORT_FLD_PTX_STATE:
        p_db_mstp_port->ptx_state = p_mstp_port->ptx_state;
        break;

    case TBL_MSTP_PORT_FLD_PIM_STATE:
        p_db_mstp_port->pim_state = p_mstp_port->pim_state;
        break;

    case TBL_MSTP_PORT_FLD_PRT_STATE:
        p_db_mstp_port->prt_state = p_mstp_port->prt_state;
        break;

    case TBL_MSTP_PORT_FLD_PST_STATE:
        p_db_mstp_port->pst_state = p_mstp_port->pst_state;
        break;

    case TBL_MSTP_PORT_FLD_TCM_STATE:
        p_db_mstp_port->tcm_state = p_mstp_port->tcm_state;
        break;

    case TBL_MSTP_PORT_FLD_FEA_STATE:
        p_db_mstp_port->fea_state = p_mstp_port->fea_state;
        break;

    case TBL_MSTP_PORT_FLD_BPDU:
        p_db_mstp_port->bpdu = p_mstp_port->bpdu;
        break;

    case TBL_MSTP_PORT_FLD_PORT_TIMER:
        p_db_mstp_port->port_timer = p_mstp_port->port_timer;
        break;

    case TBL_MSTP_PORT_FLD_EDGEDELAYWHILE_TIMER:
        p_db_mstp_port->edgeDelayWhile_timer = p_mstp_port->edgeDelayWhile_timer;
        break;

    case TBL_MSTP_PORT_FLD_FDWHILE_TIMER:
        p_db_mstp_port->fdWhile_timer = p_mstp_port->fdWhile_timer;
        break;

    case TBL_MSTP_PORT_FLD_HELLOWHEN_TIMER:
        p_db_mstp_port->helloWhen_timer = p_mstp_port->helloWhen_timer;
        break;

    case TBL_MSTP_PORT_FLD_MDELAYWHILE_TIMER:
        p_db_mstp_port->mdelayWhile_timer = p_mstp_port->mdelayWhile_timer;
        break;

    case TBL_MSTP_PORT_FLD_RBWHILE_TIMER:
        p_db_mstp_port->rbWhile_timer = p_mstp_port->rbWhile_timer;
        break;

    case TBL_MSTP_PORT_FLD_RCVDINFOWHILE_TIMER:
        p_db_mstp_port->rcvdInfoWhile_timer = p_mstp_port->rcvdInfoWhile_timer;
        break;

    case TBL_MSTP_PORT_FLD_RRWHILE_TIMER:
        p_db_mstp_port->rrWhile_timer = p_mstp_port->rrWhile_timer;
        break;

    case TBL_MSTP_PORT_FLD_TCWHILE_TIMER:
        p_db_mstp_port->tcWhile_timer = p_mstp_port->tcWhile_timer;
        break;

    case TBL_MSTP_PORT_FLD_HOLD_TIMER:
        p_db_mstp_port->hold_timer = p_mstp_port->hold_timer;
        break;

    case TBL_MSTP_PORT_FLD_MAX:
        sal_memcpy(p_db_mstp_port, p_mstp_port, sizeof(tbl_mstp_port_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_MSTP_PORT, field_id, p_db_mstp_port);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_MSTP_PORT, field_id, p_db_mstp_port);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_mstp_port_add_mstp_port(tbl_mstp_port_t *p_mstp_port)
{
    return tbl_mstp_port_add_mstp_port_sync(p_mstp_port, TRUE);
}

int32
tbl_mstp_port_del_mstp_port(tbl_mstp_port_key_t *p_mstp_port_key)
{
    return tbl_mstp_port_del_mstp_port_sync(p_mstp_port_key, TRUE);
}

int32
tbl_mstp_port_set_mstp_port_field(tbl_mstp_port_t *p_mstp_port, tbl_mstp_port_field_id_t field_id)
{
    return tbl_mstp_port_set_mstp_port_field_sync(p_mstp_port, field_id, TRUE);
}

tbl_mstp_port_t*
tbl_mstp_port_get_mstp_port(tbl_mstp_port_key_t *p_mstp_port_key)
{
    tbl_mstp_port_master_t *p_master = _g_p_tbl_mstp_port_master;
    tbl_mstp_port_t lkp;

    sal_memcpy(&lkp.key, p_mstp_port_key, sizeof(tbl_mstp_port_key_t));
    return ctclib_hash_lookup(p_master->port_hash, &lkp);
}

char*
tbl_mstp_port_key_val2str(tbl_mstp_port_t *p_mstp_port, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_mstp_port->key.name);
    return str;
}

int32
tbl_mstp_port_key_str2val(char *str, tbl_mstp_port_t *p_mstp_port)
{
    int32 ret = 0;

    sal_strcpy(p_mstp_port->key.name, str);
    return ret;
}

char*
tbl_mstp_port_key_name_dump(tbl_mstp_port_t *p_mstp_port, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_MSTP_PORT);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_mstp_port->key.name);
    return str;
}

char*
tbl_mstp_port_key_value_dump(tbl_mstp_port_t *p_mstp_port, char *str)
{
    sal_sprintf(str, "%s", p_mstp_port->key.name);
    return str;
}

char*
tbl_mstp_port_field_name_dump(tbl_mstp_port_t *p_mstp_port, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_MSTP_PORT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_MSTP_PORT_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_mstp_port_field_value_dump(tbl_mstp_port_t *p_mstp_port, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_mstp_port->key.name);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_IFINDEX, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->ifindex);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_ADMINPORTPATHCOST, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->adminPortPathCost);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORTPATHCOST, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->portPathCost);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_FORWARD_TRANSITIONS, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->forward_transitions);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_TX_BPDU_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->tx_bpdu_count);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_TX_ERROR_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->tx_error_count);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RX_BPDU_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->rx_bpdu_count);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RX_ERROR_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->rx_error_count);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORT_ENABLE, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->port_enable);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_BPDU_LOOP_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->bpdu_loop_count);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RESTRICTED_ROLE, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->restricted_role);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RESTRICTED_TCN, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->restricted_tcn);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_ADMIN_BPDUGUARD, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(mstp_edgeport_strs, MSTP_EDGEPORT_MAX, p_mstp_port->admin_bpduguard));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_OPER_BPDUGUARD, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->oper_bpduguard);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_ADMIN_BPDUFILTER, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(mstp_edgeport_strs, MSTP_EDGEPORT_MAX, p_mstp_port->admin_bpdufilter));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_OPER_BPDUFILTER, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->oper_bpdufilter);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_ADMIN_ROOTGUARD, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->admin_rootguard);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_OPER_ROOTGUARD, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->oper_rootguard);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_ADMIN_LOOPGUARD, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->admin_loopguard);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_OPER_LOOPGUARD, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->oper_loopguard);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_CISCO_CFG_FORMAT_ID, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->cisco_cfg_format_id);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_ADMINP2PMAC, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(mstp_link_strs, MSTP_LINK_MAX, p_mstp_port->adminP2PMac));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_OPERP2PMAC, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->operP2PMac);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_AGREE, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->agree);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_AGREED, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->agreed);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_ADMINFORCEVERSION, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->adminForceVersion);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_FORCEVERSION, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->forceVersion);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_DESIGNATED_ROOTID, field_id))
    {
        cdb_mstp_brg_id_val2str(str, MAX_CMD_STR_LEN, &p_mstp_port->designated_rootId);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_DESIGNATED_ROOTPATHCOST, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->designated_rootPathCost);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_DESIGNATED_REGIONROOTID, field_id))
    {
        cdb_mstp_brg_id_val2str(str, MAX_CMD_STR_LEN, &p_mstp_port->designated_regionRootId);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_DESIGNATED_INTROOTPATHCOST, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->designated_intRootPathCost);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_DESIGNATED_DESIGNATEDBRIDGEID, field_id))
    {
        cdb_mstp_brg_id_val2str(str, MAX_CMD_STR_LEN, &p_mstp_port->designated_designatedBridgeId);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_DESIGNATED_DESIGNATEDPORTID, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->designated_designatedPortId);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_DESIGNATED_BRIDGEPORTID, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->designated_bridgePortId);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_DESIGNATED_MESSAGEAGE, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->designated_messageAge);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_DESIGNATED_FWDDELAY, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->designated_fwdDelay);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_DESIGNATED_HELLOTIME, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->designated_helloTime);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_DESIGNATED_MAXAGE, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->designated_maxAge);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_DISPUTED, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->disputed);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_FDBFLUSH, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->fdbFlush);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_FORWARD, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->forward);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_FORWARDING, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->forwarding);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_INFOIS, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(mstp_info_strs, MSTP_INFO_MAX, p_mstp_port->infoIs));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_LEARN, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->learn);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_LEARNING, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->learning);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_MCHECK, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->mcheck);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_NEWINFO, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->newInfo);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_NEWINFOMSTI, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->newInfoMsti);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_ADMINEDGE, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->adminEdge);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_OPEREDGE, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->operEdge);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORTENABLED, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->portEnabled);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORTID, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->portId);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PRIORITY, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->priority);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORT_ROOTID, field_id))
    {
        cdb_mstp_brg_id_val2str(str, MAX_CMD_STR_LEN, &p_mstp_port->port_rootId);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORT_ROOTPATHCOST, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->port_rootPathCost);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORT_REGIONROOTID, field_id))
    {
        cdb_mstp_brg_id_val2str(str, MAX_CMD_STR_LEN, &p_mstp_port->port_regionRootId);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORT_INTROOTPATHCOST, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->port_intRootPathCost);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORT_DESIGNATEDBRIDGEID, field_id))
    {
        cdb_mstp_brg_id_val2str(str, MAX_CMD_STR_LEN, &p_mstp_port->port_designatedBridgeId);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORT_DESIGNATEDPORTID, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->port_designatedPortId);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORT_BRIDGEPORTID, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->port_bridgePortId);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORT_MESSAGEAGE, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->port_messageAge);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORT_FWDDELAY, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->port_fwdDelay);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORT_HELLOTIME, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->port_helloTime);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORT_MAXAGE, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->port_maxAge);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PROPOSED, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->proposed);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PROPOSING, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->proposing);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RCVDBPDU, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->rcvdBpdu);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RCVDINFO, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(mstp_rcvd_info_strs, MSTP_RCVD_INFO_MAX, p_mstp_port->rcvdInfo));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RCVDINFOUPDATETIMERS, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->rcvdInfoUpdateTimers);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RCVDMSG, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->rcvdMsg);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RCVDMSTP, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->rcvdMSTP);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RCVDRSTP, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->rcvdRSTP);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RCVDSTP, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->rcvdSTP);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RCVDTC, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->rcvdTc);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RCVDTCACK, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->rcvdTcAck);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RCVDTCN, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->rcvdTcn);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_REROOT, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->reRoot);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RESELECT, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->reselect);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_ROLE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(mstp_port_role_strs, MSTP_PORT_ROLE_MAX, p_mstp_port->role));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_SELECTED, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->selected);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_SELECTEDROLE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(mstp_port_role_strs, MSTP_PORT_ROLE_MAX, p_mstp_port->selectedRole));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_SENDRSTP, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->sendRstp);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_SYNC, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->sync);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_SYNCED, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->synced);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_TCACK, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->tcAck);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_TCPROP, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->tcProp);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_TXCOUNT, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->txCount);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_UPDTINFO, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->updtInfo);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_REROOTED, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->reRooted);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RSTPVERSION, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->rstpVersion);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_STPVERSION, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->stpVersion);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RCVDINTERNAL, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->rcvdInternal);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RCVDINTERNALCHANGE, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->rcvdInternalChange);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_HOPCOUNT, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->hopCount);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PRX_STATE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(prx_state_strs, PRX_STATE_MAX, p_mstp_port->prx_state));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PPM_STATE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(ppm_state_strs, PPM_STATE_MAX, p_mstp_port->ppm_state));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_BDM_STATE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(bdm_state_strs, BDM_STATE_MAX, p_mstp_port->bdm_state));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PTX_STATE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(ptx_state_strs, PTX_STATE_MAX, p_mstp_port->ptx_state));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PIM_STATE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(pim_state_strs, PIM_STATE_MAX, p_mstp_port->pim_state));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PRT_STATE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(prt_state_strs, PRT_STATE_MAX, p_mstp_port->prt_state));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PST_STATE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(pst_state_strs, PST_STATE_MAX, p_mstp_port->pst_state));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_TCM_STATE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(tcm_state_strs, TCM_STATE_MAX, p_mstp_port->tcm_state));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_FEA_STATE, field_id))
    {
        sal_sprintf(str, "%u", p_mstp_port->fea_state);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_BPDU, field_id))
    {
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORT_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_mstp_port->port_timer);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_EDGEDELAYWHILE_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_mstp_port->edgeDelayWhile_timer);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_FDWHILE_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_mstp_port->fdWhile_timer);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_HELLOWHEN_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_mstp_port->helloWhen_timer);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_MDELAYWHILE_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_mstp_port->mdelayWhile_timer);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RBWHILE_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_mstp_port->rbWhile_timer);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RCVDINFOWHILE_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_mstp_port->rcvdInfoWhile_timer);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RRWHILE_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_mstp_port->rrWhile_timer);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_TCWHILE_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_mstp_port->tcWhile_timer);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_HOLD_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_mstp_port->hold_timer);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_MSTI_LIST, field_id))
    {
    }
    return str;
}

char**
tbl_mstp_port_table_dump(tbl_mstp_port_t *p_mstp_port, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_mstp_port_key_name_dump(p_mstp_port, buf));
    for(i=1; i<TBL_MSTP_PORT_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_mstp_port_field_name_dump(p_mstp_port, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_mstp_port_field_value_dump(p_mstp_port, i, buf));
    }
    return str;
}

int32
tbl_mstp_port_field_value_parser(char *str, int32 field_id, tbl_mstp_port_t *p_mstp_port)
{
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_KEY, field_id))
    {
        sal_strcpy(p_mstp_port->key.name, str);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_IFINDEX, field_id))
    {
        int32 ret;
        p_mstp_port->ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_ADMINPORTPATHCOST, field_id))
    {
        int32 ret;
        p_mstp_port->adminPortPathCost = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORTPATHCOST, field_id))
    {
        int32 ret;
        p_mstp_port->portPathCost = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_FORWARD_TRANSITIONS, field_id))
    {
        int32 ret;
        p_mstp_port->forward_transitions = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_TX_BPDU_COUNT, field_id))
    {
        int32 ret;
        p_mstp_port->tx_bpdu_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_TX_ERROR_COUNT, field_id))
    {
        int32 ret;
        p_mstp_port->tx_error_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RX_BPDU_COUNT, field_id))
    {
        int32 ret;
        p_mstp_port->rx_bpdu_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RX_ERROR_COUNT, field_id))
    {
        int32 ret;
        p_mstp_port->rx_error_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORT_ENABLE, field_id))
    {
        int32 ret;
        p_mstp_port->port_enable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_BPDU_LOOP_COUNT, field_id))
    {
        int32 ret;
        p_mstp_port->bpdu_loop_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RESTRICTED_ROLE, field_id))
    {
        int32 ret;
        p_mstp_port->restricted_role = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RESTRICTED_TCN, field_id))
    {
        int32 ret;
        p_mstp_port->restricted_tcn = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_ADMIN_BPDUGUARD, field_id))
    {
        cdb_enum_str2val(mstp_edgeport_strs, MSTP_EDGEPORT_MAX, str);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_OPER_BPDUGUARD, field_id))
    {
        int32 ret;
        p_mstp_port->oper_bpduguard = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_ADMIN_BPDUFILTER, field_id))
    {
        cdb_enum_str2val(mstp_edgeport_strs, MSTP_EDGEPORT_MAX, str);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_OPER_BPDUFILTER, field_id))
    {
        int32 ret;
        p_mstp_port->oper_bpdufilter = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_ADMIN_ROOTGUARD, field_id))
    {
        int32 ret;
        p_mstp_port->admin_rootguard = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_OPER_ROOTGUARD, field_id))
    {
        int32 ret;
        p_mstp_port->oper_rootguard = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_ADMIN_LOOPGUARD, field_id))
    {
        int32 ret;
        p_mstp_port->admin_loopguard = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_OPER_LOOPGUARD, field_id))
    {
        int32 ret;
        p_mstp_port->oper_loopguard = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_CISCO_CFG_FORMAT_ID, field_id))
    {
        int32 ret;
        p_mstp_port->cisco_cfg_format_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_ADMINP2PMAC, field_id))
    {
        cdb_enum_str2val(mstp_link_strs, MSTP_LINK_MAX, str);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_OPERP2PMAC, field_id))
    {
        int32 ret;
        p_mstp_port->operP2PMac = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_AGREE, field_id))
    {
        int32 ret;
        p_mstp_port->agree = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_AGREED, field_id))
    {
        int32 ret;
        p_mstp_port->agreed = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_ADMINFORCEVERSION, field_id))
    {
        int32 ret;
        p_mstp_port->adminForceVersion = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_FORCEVERSION, field_id))
    {
        int32 ret;
        p_mstp_port->forceVersion = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_DESIGNATED_ROOTID, field_id))
    {
        cdb_mstp_brg_id_str2val(str, &p_mstp_port->designated_rootId);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_DESIGNATED_ROOTPATHCOST, field_id))
    {
        int32 ret;
        p_mstp_port->designated_rootPathCost = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_DESIGNATED_REGIONROOTID, field_id))
    {
        cdb_mstp_brg_id_str2val(str, &p_mstp_port->designated_regionRootId);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_DESIGNATED_INTROOTPATHCOST, field_id))
    {
        int32 ret;
        p_mstp_port->designated_intRootPathCost = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_DESIGNATED_DESIGNATEDBRIDGEID, field_id))
    {
        cdb_mstp_brg_id_str2val(str, &p_mstp_port->designated_designatedBridgeId);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_DESIGNATED_DESIGNATEDPORTID, field_id))
    {
        int32 ret;
        p_mstp_port->designated_designatedPortId = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_DESIGNATED_BRIDGEPORTID, field_id))
    {
        int32 ret;
        p_mstp_port->designated_bridgePortId = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_DESIGNATED_MESSAGEAGE, field_id))
    {
        int32 ret;
        p_mstp_port->designated_messageAge = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_DESIGNATED_FWDDELAY, field_id))
    {
        int32 ret;
        p_mstp_port->designated_fwdDelay = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_DESIGNATED_HELLOTIME, field_id))
    {
        int32 ret;
        p_mstp_port->designated_helloTime = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_DESIGNATED_MAXAGE, field_id))
    {
        int32 ret;
        p_mstp_port->designated_maxAge = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_DISPUTED, field_id))
    {
        int32 ret;
        p_mstp_port->disputed = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_FDBFLUSH, field_id))
    {
        int32 ret;
        p_mstp_port->fdbFlush = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_FORWARD, field_id))
    {
        int32 ret;
        p_mstp_port->forward = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_FORWARDING, field_id))
    {
        int32 ret;
        p_mstp_port->forwarding = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_INFOIS, field_id))
    {
        cdb_enum_str2val(mstp_info_strs, MSTP_INFO_MAX, str);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_LEARN, field_id))
    {
        int32 ret;
        p_mstp_port->learn = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_LEARNING, field_id))
    {
        int32 ret;
        p_mstp_port->learning = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_MCHECK, field_id))
    {
        int32 ret;
        p_mstp_port->mcheck = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_NEWINFO, field_id))
    {
        int32 ret;
        p_mstp_port->newInfo = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_NEWINFOMSTI, field_id))
    {
        int32 ret;
        p_mstp_port->newInfoMsti = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_ADMINEDGE, field_id))
    {
        int32 ret;
        p_mstp_port->adminEdge = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_OPEREDGE, field_id))
    {
        int32 ret;
        p_mstp_port->operEdge = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORTENABLED, field_id))
    {
        int32 ret;
        p_mstp_port->portEnabled = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORTID, field_id))
    {
        int32 ret;
        p_mstp_port->portId = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PRIORITY, field_id))
    {
        int32 ret;
        p_mstp_port->priority = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORT_ROOTID, field_id))
    {
        cdb_mstp_brg_id_str2val(str, &p_mstp_port->port_rootId);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORT_ROOTPATHCOST, field_id))
    {
        int32 ret;
        p_mstp_port->port_rootPathCost = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORT_REGIONROOTID, field_id))
    {
        cdb_mstp_brg_id_str2val(str, &p_mstp_port->port_regionRootId);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORT_INTROOTPATHCOST, field_id))
    {
        int32 ret;
        p_mstp_port->port_intRootPathCost = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORT_DESIGNATEDBRIDGEID, field_id))
    {
        cdb_mstp_brg_id_str2val(str, &p_mstp_port->port_designatedBridgeId);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORT_DESIGNATEDPORTID, field_id))
    {
        int32 ret;
        p_mstp_port->port_designatedPortId = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORT_BRIDGEPORTID, field_id))
    {
        int32 ret;
        p_mstp_port->port_bridgePortId = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORT_MESSAGEAGE, field_id))
    {
        int32 ret;
        p_mstp_port->port_messageAge = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORT_FWDDELAY, field_id))
    {
        int32 ret;
        p_mstp_port->port_fwdDelay = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORT_HELLOTIME, field_id))
    {
        int32 ret;
        p_mstp_port->port_helloTime = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORT_MAXAGE, field_id))
    {
        int32 ret;
        p_mstp_port->port_maxAge = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PROPOSED, field_id))
    {
        int32 ret;
        p_mstp_port->proposed = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PROPOSING, field_id))
    {
        int32 ret;
        p_mstp_port->proposing = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RCVDBPDU, field_id))
    {
        int32 ret;
        p_mstp_port->rcvdBpdu = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RCVDINFO, field_id))
    {
        cdb_enum_str2val(mstp_rcvd_info_strs, MSTP_RCVD_INFO_MAX, str);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RCVDINFOUPDATETIMERS, field_id))
    {
        int32 ret;
        p_mstp_port->rcvdInfoUpdateTimers = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RCVDMSG, field_id))
    {
        int32 ret;
        p_mstp_port->rcvdMsg = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RCVDMSTP, field_id))
    {
        int32 ret;
        p_mstp_port->rcvdMSTP = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RCVDRSTP, field_id))
    {
        int32 ret;
        p_mstp_port->rcvdRSTP = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RCVDSTP, field_id))
    {
        int32 ret;
        p_mstp_port->rcvdSTP = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RCVDTC, field_id))
    {
        int32 ret;
        p_mstp_port->rcvdTc = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RCVDTCACK, field_id))
    {
        int32 ret;
        p_mstp_port->rcvdTcAck = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RCVDTCN, field_id))
    {
        int32 ret;
        p_mstp_port->rcvdTcn = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_REROOT, field_id))
    {
        int32 ret;
        p_mstp_port->reRoot = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RESELECT, field_id))
    {
        int32 ret;
        p_mstp_port->reselect = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_ROLE, field_id))
    {
        cdb_enum_str2val(mstp_port_role_strs, MSTP_PORT_ROLE_MAX, str);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_SELECTED, field_id))
    {
        int32 ret;
        p_mstp_port->selected = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_SELECTEDROLE, field_id))
    {
        cdb_enum_str2val(mstp_port_role_strs, MSTP_PORT_ROLE_MAX, str);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_SENDRSTP, field_id))
    {
        int32 ret;
        p_mstp_port->sendRstp = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_SYNC, field_id))
    {
        int32 ret;
        p_mstp_port->sync = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_SYNCED, field_id))
    {
        int32 ret;
        p_mstp_port->synced = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_TCACK, field_id))
    {
        int32 ret;
        p_mstp_port->tcAck = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_TCPROP, field_id))
    {
        int32 ret;
        p_mstp_port->tcProp = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_TXCOUNT, field_id))
    {
        int32 ret;
        p_mstp_port->txCount = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_UPDTINFO, field_id))
    {
        int32 ret;
        p_mstp_port->updtInfo = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_REROOTED, field_id))
    {
        int32 ret;
        p_mstp_port->reRooted = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RSTPVERSION, field_id))
    {
        int32 ret;
        p_mstp_port->rstpVersion = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_STPVERSION, field_id))
    {
        int32 ret;
        p_mstp_port->stpVersion = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RCVDINTERNAL, field_id))
    {
        int32 ret;
        p_mstp_port->rcvdInternal = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RCVDINTERNALCHANGE, field_id))
    {
        int32 ret;
        p_mstp_port->rcvdInternalChange = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_HOPCOUNT, field_id))
    {
        int32 ret;
        p_mstp_port->hopCount = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PRX_STATE, field_id))
    {
        cdb_enum_str2val(prx_state_strs, PRX_STATE_MAX, str);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PPM_STATE, field_id))
    {
        cdb_enum_str2val(ppm_state_strs, PPM_STATE_MAX, str);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_BDM_STATE, field_id))
    {
        cdb_enum_str2val(bdm_state_strs, BDM_STATE_MAX, str);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PTX_STATE, field_id))
    {
        cdb_enum_str2val(ptx_state_strs, PTX_STATE_MAX, str);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PIM_STATE, field_id))
    {
        cdb_enum_str2val(pim_state_strs, PIM_STATE_MAX, str);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PRT_STATE, field_id))
    {
        cdb_enum_str2val(prt_state_strs, PRT_STATE_MAX, str);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PST_STATE, field_id))
    {
        cdb_enum_str2val(pst_state_strs, PST_STATE_MAX, str);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_TCM_STATE, field_id))
    {
        cdb_enum_str2val(tcm_state_strs, TCM_STATE_MAX, str);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_FEA_STATE, field_id))
    {
        int32 ret;
        p_mstp_port->fea_state = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_BPDU, field_id))
    {
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORT_TIMER, field_id))
    {
        ctc_task_str2val(str, p_mstp_port->port_timer);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_EDGEDELAYWHILE_TIMER, field_id))
    {
        ctc_task_str2val(str, p_mstp_port->edgeDelayWhile_timer);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_FDWHILE_TIMER, field_id))
    {
        ctc_task_str2val(str, p_mstp_port->fdWhile_timer);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_HELLOWHEN_TIMER, field_id))
    {
        ctc_task_str2val(str, p_mstp_port->helloWhen_timer);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_MDELAYWHILE_TIMER, field_id))
    {
        ctc_task_str2val(str, p_mstp_port->mdelayWhile_timer);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RBWHILE_TIMER, field_id))
    {
        ctc_task_str2val(str, p_mstp_port->rbWhile_timer);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RCVDINFOWHILE_TIMER, field_id))
    {
        ctc_task_str2val(str, p_mstp_port->rcvdInfoWhile_timer);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RRWHILE_TIMER, field_id))
    {
        ctc_task_str2val(str, p_mstp_port->rrWhile_timer);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_TCWHILE_TIMER, field_id))
    {
        ctc_task_str2val(str, p_mstp_port->tcWhile_timer);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_HOLD_TIMER, field_id))
    {
        ctc_task_str2val(str, p_mstp_port->hold_timer);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_MSTI_LIST, field_id))
    {
    }
    return PM_E_NONE;
}

int32
tbl_mstp_port_table_parser(char** array, char* key_value,tbl_mstp_port_t *p_mstp_port)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_MSTP_PORT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_mstp_port_key_str2val(key_value, p_mstp_port));

    for(i=1; i<TBL_MSTP_PORT_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_mstp_port_field_value_parser( array[j++], i, p_mstp_port));
    }

    return PM_E_NONE;
}

int32
tbl_mstp_port_dump_one(tbl_mstp_port_t *p_mstp_port, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_MSTP_PORT);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_MSTP_PORT_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_PORT_FLD_KEY].name,
            p_mstp_port->key.name);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_IFINDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_IFINDEX].name,
            p_mstp_port->ifindex);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_ADMINPORTPATHCOST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_ADMINPORTPATHCOST].name,
            p_mstp_port->adminPortPathCost);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORTPATHCOST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_PORTPATHCOST].name,
            p_mstp_port->portPathCost);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_FORWARD_TRANSITIONS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_FORWARD_TRANSITIONS].name,
            p_mstp_port->forward_transitions);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_TX_BPDU_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_TX_BPDU_COUNT].name,
            p_mstp_port->tx_bpdu_count);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_TX_ERROR_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_TX_ERROR_COUNT].name,
            p_mstp_port->tx_error_count);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RX_BPDU_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_RX_BPDU_COUNT].name,
            p_mstp_port->rx_bpdu_count);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RX_ERROR_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_RX_ERROR_COUNT].name,
            p_mstp_port->rx_error_count);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORT_ENABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_PORT_ENABLE].name,
            p_mstp_port->port_enable);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_BPDU_LOOP_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_BPDU_LOOP_COUNT].name,
            p_mstp_port->bpdu_loop_count);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RESTRICTED_ROLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_RESTRICTED_ROLE].name,
            p_mstp_port->restricted_role);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RESTRICTED_TCN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_RESTRICTED_TCN].name,
            p_mstp_port->restricted_tcn);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_ADMIN_BPDUGUARD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_PORT_FLD_ADMIN_BPDUGUARD].name, 
            cdb_enum_val2str(mstp_edgeport_strs, MSTP_EDGEPORT_MAX, p_mstp_port->admin_bpduguard));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_OPER_BPDUGUARD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_OPER_BPDUGUARD].name,
            p_mstp_port->oper_bpduguard);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_ADMIN_BPDUFILTER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_PORT_FLD_ADMIN_BPDUFILTER].name, 
            cdb_enum_val2str(mstp_edgeport_strs, MSTP_EDGEPORT_MAX, p_mstp_port->admin_bpdufilter));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_OPER_BPDUFILTER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_OPER_BPDUFILTER].name,
            p_mstp_port->oper_bpdufilter);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_ADMIN_ROOTGUARD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_ADMIN_ROOTGUARD].name,
            p_mstp_port->admin_rootguard);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_OPER_ROOTGUARD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_OPER_ROOTGUARD].name,
            p_mstp_port->oper_rootguard);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_ADMIN_LOOPGUARD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_ADMIN_LOOPGUARD].name,
            p_mstp_port->admin_loopguard);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_OPER_LOOPGUARD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_OPER_LOOPGUARD].name,
            p_mstp_port->oper_loopguard);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_CISCO_CFG_FORMAT_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_CISCO_CFG_FORMAT_ID].name,
            p_mstp_port->cisco_cfg_format_id);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_ADMINP2PMAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_PORT_FLD_ADMINP2PMAC].name, 
            cdb_enum_val2str(mstp_link_strs, MSTP_LINK_MAX, p_mstp_port->adminP2PMac));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_OPERP2PMAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_OPERP2PMAC].name,
            p_mstp_port->operP2PMac);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_AGREE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_AGREE].name,
            p_mstp_port->agree);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_AGREED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_AGREED].name,
            p_mstp_port->agreed);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_ADMINFORCEVERSION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_ADMINFORCEVERSION].name,
            p_mstp_port->adminForceVersion);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_FORCEVERSION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_FORCEVERSION].name,
            p_mstp_port->forceVersion);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_DESIGNATED_ROOTID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_PORT_FLD_DESIGNATED_ROOTID].name, 
            cdb_mstp_brg_id_val2str(buf, MAX_CMD_STR_LEN, &p_mstp_port->designated_rootId));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_DESIGNATED_ROOTPATHCOST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_DESIGNATED_ROOTPATHCOST].name,
            p_mstp_port->designated_rootPathCost);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_DESIGNATED_REGIONROOTID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_PORT_FLD_DESIGNATED_REGIONROOTID].name, 
            cdb_mstp_brg_id_val2str(buf, MAX_CMD_STR_LEN, &p_mstp_port->designated_regionRootId));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_DESIGNATED_INTROOTPATHCOST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_DESIGNATED_INTROOTPATHCOST].name,
            p_mstp_port->designated_intRootPathCost);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_DESIGNATED_DESIGNATEDBRIDGEID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_PORT_FLD_DESIGNATED_DESIGNATEDBRIDGEID].name, 
            cdb_mstp_brg_id_val2str(buf, MAX_CMD_STR_LEN, &p_mstp_port->designated_designatedBridgeId));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_DESIGNATED_DESIGNATEDPORTID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_DESIGNATED_DESIGNATEDPORTID].name,
            p_mstp_port->designated_designatedPortId);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_DESIGNATED_BRIDGEPORTID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_DESIGNATED_BRIDGEPORTID].name,
            p_mstp_port->designated_bridgePortId);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_DESIGNATED_MESSAGEAGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_DESIGNATED_MESSAGEAGE].name,
            p_mstp_port->designated_messageAge);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_DESIGNATED_FWDDELAY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_DESIGNATED_FWDDELAY].name,
            p_mstp_port->designated_fwdDelay);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_DESIGNATED_HELLOTIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_DESIGNATED_HELLOTIME].name,
            p_mstp_port->designated_helloTime);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_DESIGNATED_MAXAGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_DESIGNATED_MAXAGE].name,
            p_mstp_port->designated_maxAge);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_DISPUTED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_DISPUTED].name,
            p_mstp_port->disputed);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_FDBFLUSH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_FDBFLUSH].name,
            p_mstp_port->fdbFlush);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_FORWARD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_FORWARD].name,
            p_mstp_port->forward);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_FORWARDING, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_FORWARDING].name,
            p_mstp_port->forwarding);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_INFOIS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_PORT_FLD_INFOIS].name, 
            cdb_enum_val2str(mstp_info_strs, MSTP_INFO_MAX, p_mstp_port->infoIs));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_LEARN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_LEARN].name,
            p_mstp_port->learn);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_LEARNING, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_LEARNING].name,
            p_mstp_port->learning);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_MCHECK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_MCHECK].name,
            p_mstp_port->mcheck);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_NEWINFO, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_NEWINFO].name,
            p_mstp_port->newInfo);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_NEWINFOMSTI, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_NEWINFOMSTI].name,
            p_mstp_port->newInfoMsti);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_ADMINEDGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_ADMINEDGE].name,
            p_mstp_port->adminEdge);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_OPEREDGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_OPEREDGE].name,
            p_mstp_port->operEdge);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORTENABLED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_PORTENABLED].name,
            p_mstp_port->portEnabled);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORTID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_PORTID].name,
            p_mstp_port->portId);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PRIORITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_PRIORITY].name,
            p_mstp_port->priority);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORT_ROOTID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_PORT_FLD_PORT_ROOTID].name, 
            cdb_mstp_brg_id_val2str(buf, MAX_CMD_STR_LEN, &p_mstp_port->port_rootId));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORT_ROOTPATHCOST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_PORT_ROOTPATHCOST].name,
            p_mstp_port->port_rootPathCost);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORT_REGIONROOTID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_PORT_FLD_PORT_REGIONROOTID].name, 
            cdb_mstp_brg_id_val2str(buf, MAX_CMD_STR_LEN, &p_mstp_port->port_regionRootId));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORT_INTROOTPATHCOST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_PORT_INTROOTPATHCOST].name,
            p_mstp_port->port_intRootPathCost);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORT_DESIGNATEDBRIDGEID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_PORT_FLD_PORT_DESIGNATEDBRIDGEID].name, 
            cdb_mstp_brg_id_val2str(buf, MAX_CMD_STR_LEN, &p_mstp_port->port_designatedBridgeId));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORT_DESIGNATEDPORTID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_PORT_DESIGNATEDPORTID].name,
            p_mstp_port->port_designatedPortId);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORT_BRIDGEPORTID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_PORT_BRIDGEPORTID].name,
            p_mstp_port->port_bridgePortId);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORT_MESSAGEAGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_PORT_MESSAGEAGE].name,
            p_mstp_port->port_messageAge);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORT_FWDDELAY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_PORT_FWDDELAY].name,
            p_mstp_port->port_fwdDelay);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORT_HELLOTIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_PORT_HELLOTIME].name,
            p_mstp_port->port_helloTime);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORT_MAXAGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_PORT_MAXAGE].name,
            p_mstp_port->port_maxAge);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PROPOSED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_PROPOSED].name,
            p_mstp_port->proposed);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PROPOSING, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_PROPOSING].name,
            p_mstp_port->proposing);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RCVDBPDU, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_RCVDBPDU].name,
            p_mstp_port->rcvdBpdu);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RCVDINFO, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_PORT_FLD_RCVDINFO].name, 
            cdb_enum_val2str(mstp_rcvd_info_strs, MSTP_RCVD_INFO_MAX, p_mstp_port->rcvdInfo));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RCVDINFOUPDATETIMERS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_RCVDINFOUPDATETIMERS].name,
            p_mstp_port->rcvdInfoUpdateTimers);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RCVDMSG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_RCVDMSG].name,
            p_mstp_port->rcvdMsg);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RCVDMSTP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_RCVDMSTP].name,
            p_mstp_port->rcvdMSTP);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RCVDRSTP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_RCVDRSTP].name,
            p_mstp_port->rcvdRSTP);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RCVDSTP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_RCVDSTP].name,
            p_mstp_port->rcvdSTP);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RCVDTC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_RCVDTC].name,
            p_mstp_port->rcvdTc);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RCVDTCACK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_RCVDTCACK].name,
            p_mstp_port->rcvdTcAck);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RCVDTCN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_RCVDTCN].name,
            p_mstp_port->rcvdTcn);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_REROOT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_REROOT].name,
            p_mstp_port->reRoot);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RESELECT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_RESELECT].name,
            p_mstp_port->reselect);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_ROLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_PORT_FLD_ROLE].name, 
            cdb_enum_val2str(mstp_port_role_strs, MSTP_PORT_ROLE_MAX, p_mstp_port->role));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_SELECTED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_SELECTED].name,
            p_mstp_port->selected);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_SELECTEDROLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_PORT_FLD_SELECTEDROLE].name, 
            cdb_enum_val2str(mstp_port_role_strs, MSTP_PORT_ROLE_MAX, p_mstp_port->selectedRole));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_SENDRSTP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_SENDRSTP].name,
            p_mstp_port->sendRstp);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_SYNC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_SYNC].name,
            p_mstp_port->sync);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_SYNCED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_SYNCED].name,
            p_mstp_port->synced);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_TCACK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_TCACK].name,
            p_mstp_port->tcAck);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_TCPROP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_TCPROP].name,
            p_mstp_port->tcProp);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_TXCOUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_TXCOUNT].name,
            p_mstp_port->txCount);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_UPDTINFO, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_UPDTINFO].name,
            p_mstp_port->updtInfo);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_REROOTED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_REROOTED].name,
            p_mstp_port->reRooted);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RSTPVERSION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_RSTPVERSION].name,
            p_mstp_port->rstpVersion);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_STPVERSION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_STPVERSION].name,
            p_mstp_port->stpVersion);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RCVDINTERNAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_RCVDINTERNAL].name,
            p_mstp_port->rcvdInternal);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RCVDINTERNALCHANGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_RCVDINTERNALCHANGE].name,
            p_mstp_port->rcvdInternalChange);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_HOPCOUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_HOPCOUNT].name,
            p_mstp_port->hopCount);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PRX_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_PORT_FLD_PRX_STATE].name, 
            cdb_enum_val2str(prx_state_strs, PRX_STATE_MAX, p_mstp_port->prx_state));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PPM_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_PORT_FLD_PPM_STATE].name, 
            cdb_enum_val2str(ppm_state_strs, PPM_STATE_MAX, p_mstp_port->ppm_state));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_BDM_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_PORT_FLD_BDM_STATE].name, 
            cdb_enum_val2str(bdm_state_strs, BDM_STATE_MAX, p_mstp_port->bdm_state));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PTX_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_PORT_FLD_PTX_STATE].name, 
            cdb_enum_val2str(ptx_state_strs, PTX_STATE_MAX, p_mstp_port->ptx_state));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PIM_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_PORT_FLD_PIM_STATE].name, 
            cdb_enum_val2str(pim_state_strs, PIM_STATE_MAX, p_mstp_port->pim_state));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PRT_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_PORT_FLD_PRT_STATE].name, 
            cdb_enum_val2str(prt_state_strs, PRT_STATE_MAX, p_mstp_port->prt_state));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PST_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_PORT_FLD_PST_STATE].name, 
            cdb_enum_val2str(pst_state_strs, PST_STATE_MAX, p_mstp_port->pst_state));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_TCM_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_PORT_FLD_TCM_STATE].name, 
            cdb_enum_val2str(tcm_state_strs, TCM_STATE_MAX, p_mstp_port->tcm_state));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_FEA_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MSTP_PORT_FLD_FEA_STATE].name,
            p_mstp_port->fea_state);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_BPDU, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"",
            p_tbl_info->field[TBL_MSTP_PORT_FLD_BPDU].name);
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_PORT_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_PORT_FLD_PORT_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_mstp_port->port_timer));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_EDGEDELAYWHILE_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_PORT_FLD_EDGEDELAYWHILE_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_mstp_port->edgeDelayWhile_timer));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_FDWHILE_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_PORT_FLD_FDWHILE_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_mstp_port->fdWhile_timer));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_HELLOWHEN_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_PORT_FLD_HELLOWHEN_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_mstp_port->helloWhen_timer));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_MDELAYWHILE_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_PORT_FLD_MDELAYWHILE_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_mstp_port->mdelayWhile_timer));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RBWHILE_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_PORT_FLD_RBWHILE_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_mstp_port->rbWhile_timer));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RCVDINFOWHILE_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_PORT_FLD_RCVDINFOWHILE_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_mstp_port->rcvdInfoWhile_timer));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_RRWHILE_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_PORT_FLD_RRWHILE_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_mstp_port->rrWhile_timer));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_TCWHILE_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_PORT_FLD_TCWHILE_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_mstp_port->tcWhile_timer));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_HOLD_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_MSTP_PORT_FLD_HOLD_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_mstp_port->hold_timer));
    }
    if (FLD_MATCH(TBL_MSTP_PORT_FLD_MSTI_LIST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s",
            p_node->tbl_info->field[TBL_MSTP_PORT_FLD_MSTI_LIST].name,
            cdb_reference_list_val2str(buf, MAX_CMD_STR_LEN, &p_mstp_port->msti_list));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_mstp_port_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_mstp_port_master_t *p_master = _g_p_tbl_mstp_port_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_mstp_port_t *p_db_mstp_port = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->port_list, p_db_mstp_port, listnode, next)
    {
        rc |= fn(p_db_mstp_port, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_mstp_port_master_t*
tbl_mstp_port_get_master()
{
    return _g_p_tbl_mstp_port_master;
}

tbl_mstp_port_master_t*
tbl_mstp_port_init_mstp_port()
{
    _g_p_tbl_mstp_port_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_mstp_port_master_t));
    _g_p_tbl_mstp_port_master->port_hash = ctclib_hash_create(_tbl_mstp_port_hash_make, _tbl_mstp_port_hash_cmp);
    _g_p_tbl_mstp_port_master->port_list = ctclib_slist_create(tbl_mstp_port_cmp, NULL);
    return _g_p_tbl_mstp_port_master;
}

int32
tbl_mstp_port_set_msti_list_sync(tbl_mstp_port_t *p_mstp_port, uint32 add, void *p_object, uint32 sync)
{
    cdb_sync_refer_hdr_t refer;
    tbl_mstp_port_t *p_db_mstp_port = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup entry exist */
    p_db_mstp_port = tbl_mstp_port_get_mstp_port(&p_mstp_port->key);
    if (NULL == p_db_mstp_port)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    if (add)
    {
        PM_E_RETURN(cdb_reference_list_add_object(&p_db_mstp_port->msti_list, p_object));
    }
    else
    {
        PM_E_RETURN(cdb_reference_list_del_object(&p_db_mstp_port->msti_list, p_object));
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_build_refer_tbl(TBL_MSTI_PORT, add, &refer);
        cdb_sync_tbl_set_refer(TBL_MSTP_PORT, TBL_MSTP_PORT_FLD_MSTI_LIST, p_mstp_port, &refer, p_object);
    }

    return PM_E_NONE;
}

int32
tbl_mstp_port_set_msti_list(tbl_mstp_port_t *p_mstp_port, uint32 add, void *p_object)
{
    return tbl_mstp_port_set_msti_list_sync(p_mstp_port, add, p_object, TRUE);
}

