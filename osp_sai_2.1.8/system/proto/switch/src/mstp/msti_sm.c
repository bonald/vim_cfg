
#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_mstp_port_define.h"
#include "gen/tbl_mstp_port.h"
#include "gen/tbl_msti_port_define.h"
#include "gen/tbl_msti_port.h"
#include "gen/tbl_mstp_global_define.h"
#include "gen/tbl_mstp_global.h"
#include "gen/tbl_mstp_instance_define.h"
#include "gen/tbl_mstp_instance.h"
#include "gen/tbl_vlan_define.h"
#include "gen/tbl_vlan.h"
#include "switch.h"
#include "switch_api.h"
#include "mstp_define.h"
#include "mstp_pdu.h"
#include "mstp_api.h"
#include "mstp_sm.h"
#include "msti_sm.h"
#include "mstp.h"

#ifdef CONFIG_MSTP

static int32
_msti_sm_PST_DISCARDING(tbl_msti_port_t *p_db_port, char *msti_port_name);

int32 
msti_newTcWhile(tbl_msti_port_t *p_db_port);

int32
msti_sm_pim_process_event(tbl_msti_port_t *p_db_port, pim_event_t event);

static int32
_msti_sm_PRT_BLOCK_PORT(tbl_msti_port_t *p_db_port, char *msti_port_name);

static int32
_msti_sm_PRT_ALTERNATE_PORT(tbl_msti_port_t *p_db_port, char *msti_port_name);

static int32
_msti_sm_PRT_TRIGGER_EVENT_DESIGNATED(tbl_msti_port_t *p_db_port)
{
    tbl_mstp_port_t *p_mstp_port = NULL;

    if (ROLE_DESIGNATED != p_db_port->role)
    {
        return PM_E_NONE;
    }

    p_mstp_port = (tbl_mstp_port_t*)p_db_port->p_port;
    
    /* (!learning && !forwarding && !synced) || (agreed && !synced) || (operEdge && !synced) || (sync && synced) */
    if ((!p_db_port->learning && !p_db_port->forwarding && !p_db_port->synced)
     || (p_db_port->agreed && !p_db_port->synced)
     || (p_mstp_port->operEdge && !p_db_port->synced)
     || (p_db_port->sync && p_db_port->synced))
    {
        msti_sm_prt_process_event(p_db_port, PRT_EVT_DESIGNATED_SYNCED);
    }

    /* ((sync && !synced) || (reRoot && (rrWhile != 0)) || disputed) && !operEdge && (learn || forward) */
    if (((p_db_port->sync && !p_db_port->synced) || (p_db_port->reRoot && p_db_port->rrWhile_timer) || p_db_port->disputed)
     && (!p_db_port->rrWhile_timer || !p_db_port->reRoot)
     && !p_mstp_port->operEdge
     && (p_db_port->learn || p_db_port->forward))
    {
        msti_sm_prt_process_event(p_db_port, PRT_EVT_DESIGNATEDDISCARD);
    }

    /* ((fdWhile == 0) || agreed || operEdge) && ((rrWhile ==0) || !reRoot) && !sync && !learn */
    if ((!p_db_port->fdWhile_timer || p_db_port->agreed || p_mstp_port->operEdge) 
     && (!p_db_port->rrWhile_timer || !p_db_port->reRoot)
     && !p_db_port->sync
     && !p_db_port->learn
     /* modified by liwh for bug 36699/37625/37767, 2016-05-11 */
     && !p_db_port->oper_rootguard)
     /* liwh end */
    {
        msti_sm_prt_process_event(p_db_port, PRT_EVT_DESIGNATEDLEARN);
    }

    /* ((fdWhile == 0) || agreed || operEdge) && ((rrWhile ==0) || !reRoot) && !sync && (learn && !forward) */
    if ((!p_db_port->fdWhile_timer || p_db_port->agreed || p_mstp_port->operEdge) 
     && (!p_db_port->rrWhile_timer || !p_db_port->reRoot)
     && !p_db_port->sync 
     && (p_db_port->learn && !p_db_port->forward))
    {
        msti_sm_prt_process_event(p_db_port, PRT_EVT_DESIGNATEDFORWARD);
    }

    /* !forward && !agreed && !proposing && !operEdge */
    if (!p_db_port->forward
     && !p_db_port->agreed
     && !p_db_port->proposing
     && !p_mstp_port->operEdge)
    {
        msti_sm_prt_process_event(p_db_port, PRT_EVT_PROPOSING);
    }

    return PM_E_NONE;
}

static int32
_msti_sm_PRT_TRIGGER_EVENT_MASTERPORT(tbl_msti_port_t *p_db_port)
{
    tbl_mstp_port_t *p_mstp_port = NULL;
    
    if (ROLE_MASTERPORT != p_db_port->role)
    {
        return PM_E_NONE;
    }

    p_mstp_port = (tbl_mstp_port_t*)p_db_port->p_port;

    /* ((sync && !synced) || (reRoot && (rrWhile != 0)) || disputed) && !operEdge && (learn || forward) */
    if (((p_db_port->sync && !p_db_port->synced) || (p_db_port->reRoot && p_db_port->rrWhile_timer) || p_db_port->disputed)
     && (!p_db_port->rrWhile_timer || !p_db_port->reRoot)
     && !p_mstp_port->operEdge
     && (p_db_port->learn || p_db_port->forward))
    {
        msti_sm_prt_process_event(p_db_port, PRT_EVT_MASTERDISCARD);
    }

    /* (!learning && !forwarding && !synced) || (agreed && !synced) || (operEdge && !synced) || (sync && synced) */
    if ((!p_db_port->learning && !p_db_port->forwarding && !p_db_port->synced)
     || (p_db_port->agreed && !p_db_port->synced)
     || (p_mstp_port->operEdge && !p_db_port->synced)
     || (p_db_port->sync && p_db_port->synced))
    {
        msti_sm_prt_process_event(p_db_port, PRT_EVT_MASTER_SYNCED);
    }
    
    if ((!p_db_port->fdWhile_timer)
     && (p_mstp_port->rstpVersion)
     && (!p_db_port->learn))
    {
        msti_sm_prt_process_event(p_db_port, PRT_EVT_MASTERLEARN);
    }

    if ((!p_db_port->fdWhile_timer)
     && (p_mstp_port->rstpVersion)
     && (p_db_port->learn)
     && (!p_db_port->forward))
    {
        msti_sm_prt_process_event(p_db_port, PRT_EVT_MASTERFORWARD);
    }
    
    return PM_E_NONE;
}

static int32
_msti_sm_PRT_TRIGGER_EVENT_ROOTPORT(tbl_msti_port_t *p_db_port)
{
    tbl_mstp_port_t *p_mstp_port = NULL;
    uint32 reRooted = FALSE;    /* RSTPTODO */
    
    if (ROLE_ROOTPORT != p_db_port->role)
    {
        return PM_E_NONE;
    }

    p_mstp_port = (tbl_mstp_port_t*)p_db_port->p_port;
    
    if ((!p_db_port->fdWhile_timer || (reRooted && !p_db_port->rbWhile_timer))
     && (p_mstp_port->rstpVersion)
     && (!p_db_port->learn))
    {
        msti_sm_prt_process_event(p_db_port, PRT_EVT_ROOTLEARN);
    }

    if ((!p_db_port->fdWhile_timer || (reRooted && !p_db_port->rbWhile_timer))
     && (p_mstp_port->rstpVersion)
     && (p_db_port->learn)
     && (!p_db_port->forward))
    {
        msti_sm_prt_process_event(p_db_port, PRT_EVT_ROOTFORWARD);
    }
    
    return PM_E_NONE;
}

static int32
_msti_sm_PRT_TRIGGER_EVENT_ALTERNATE(tbl_msti_port_t *p_db_port)
{
    if (ROLE_ALTERNATE != p_db_port->role)
    {
        return PM_E_NONE;
    }

    return PM_E_NONE;
}

int32
msti_sm_PRT_TRIGGER_EVENT(tbl_msti_port_t *p_db_port)
{
    switch (p_db_port->role)
    {
    case ROLE_DESIGNATED:
        _msti_sm_PRT_TRIGGER_EVENT_DESIGNATED(p_db_port);
        break;
    case ROLE_MASTERPORT:
        _msti_sm_PRT_TRIGGER_EVENT_MASTERPORT(p_db_port);
        break;
    case ROLE_ROOTPORT:
        _msti_sm_PRT_TRIGGER_EVENT_ROOTPORT(p_db_port);
        break;
    case ROLE_ALTERNATE:
        _msti_sm_PRT_TRIGGER_EVENT_ALTERNATE(p_db_port);
        break;
    default:
        break;
    }
    return PM_E_NONE;
}

/* compare priority of bpdu and port */
int32
msti_comparePriority_msg_port(msti_bpdu_t *bpdu, tbl_msti_port_t *p_db_port)
{
    int32 root_cmp = 0;
    int32 rpc_cmp = 0;
    int32 designated_cmp = 0;
    int32 designated_port_cmp = 0;
    
    root_cmp = sal_memcmp(&bpdu->reg_root_id, &p_db_port->port_rootId, sizeof(mstp_brg_id_t));
    rpc_cmp = bpdu->internal_rpc - p_db_port->port_rootPathCost;
    designated_cmp = sal_memcmp(&bpdu->bridge_id, &p_db_port->port_designatedBridgeId, sizeof(mstp_brg_id_t));
    designated_port_cmp = bpdu->port_id - p_db_port->port_designatedPortId;

    if ((root_cmp < 0)
     || ((root_cmp == 0) && (rpc_cmp < 0))
     || ((root_cmp == 0) && (rpc_cmp == 0) && (designated_cmp < 0))
     || ((root_cmp == 0) && (rpc_cmp == 0) && (designated_cmp == 0) && (designated_port_cmp < 0)))
    {
        return -1;
    }
    else if ((root_cmp == 0) && (rpc_cmp == 0) && (designated_cmp == 0) && (designated_port_cmp == 0))
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

/* compare timer of bpdu and port */
int32 
msti_compareTimer_msg_port(msti_bpdu_t *bpdu, tbl_msti_port_t *p_db_port)
{
    bpdu_t *cist_bpdu = (bpdu_t*)bpdu->cist;

    if (cist_bpdu->messageAge == p_db_port->port_messageAge
     && cist_bpdu->maxAge == p_db_port->port_maxAge
     && cist_bpdu->fwdDelay == p_db_port->port_fwdDelay
     && cist_bpdu->helloTime == p_db_port->port_helloTime)
    {
        return 0;
    }

    return 1;
}

int32 
msti_getBridgePriority(msti_port_priority_vector_t *best_priority, tbl_mstp_instance_t *p_db_inst)
{
    sal_memcpy(&best_priority->regionRootId, &p_db_inst->bridge_id, sizeof(mstp_brg_id_t));
    best_priority->intRootPathCost = 0;
    sal_memcpy(&best_priority->designatedBridgeId, &p_db_inst->bridge_id, sizeof(mstp_brg_id_t));
    best_priority->designatedPortId = 0;
    best_priority->bridgePortId = 0;

    return PM_E_NONE;
}

int32 
msti_compareBestPortPriority(msti_port_priority_vector_t *best_priority, tbl_msti_port_t *p_db_port)
{
    int32 rootId_cmp = 0;
    int32 rootPathCost_cmp = 0;
    int32 designatedBridgeId_cmp = 0;
    int32 designatedPortId_cmp = 0;
    int32 bridgePortId_cmp = 0;
    
    rootId_cmp = sal_memcmp(&best_priority->regionRootId, &p_db_port->port_rootId, sizeof(mstp_brg_id_t));
    if (rootId_cmp != 0)
    {
        return rootId_cmp;
    }

    rootPathCost_cmp = best_priority->intRootPathCost - (p_db_port->port_rootPathCost + p_db_port->portPathCost);
    if (rootPathCost_cmp != 0)
    {
        return rootPathCost_cmp;
    }

    designatedBridgeId_cmp = sal_memcmp(&best_priority->designatedBridgeId, &p_db_port->port_designatedBridgeId, sizeof(mstp_brg_id_t));
    if (designatedBridgeId_cmp != 0)
    {
        return designatedBridgeId_cmp;
    }

    designatedPortId_cmp = best_priority->designatedPortId - p_db_port->port_designatedPortId;
    if (designatedPortId_cmp != 0)
    {
        return designatedPortId_cmp;
    }

    bridgePortId_cmp = best_priority->bridgePortId - p_db_port->port_bridgePortId;

    return bridgePortId_cmp;
}

int32 
msti_updateBestPortPriority(msti_port_priority_vector_t *best_priority, tbl_msti_port_t *p_db_port)
{
    sal_memcpy(&best_priority->regionRootId, &p_db_port->port_rootId, sizeof(mstp_brg_id_t));
    best_priority->intRootPathCost = (p_db_port->port_rootPathCost + p_db_port->portPathCost);
    sal_memcpy(&best_priority->designatedBridgeId, &p_db_port->port_designatedBridgeId, sizeof(mstp_brg_id_t));
    best_priority->designatedPortId = p_db_port->port_designatedPortId;
    best_priority->bridgePortId = p_db_port->port_bridgePortId;

    return PM_E_NONE;
}

/* compare priority of port and port designated */
int32
msti_comparePriority_port_designated(tbl_msti_port_t *p_db_port)
{
    int32 root_cmp = 0;
    int32 rpc_cmp = 0;
    int32 designated_cmp = 0;
    int32 designated_port_cmp = 0;
    
    root_cmp = sal_memcmp(&p_db_port->port_rootId, &p_db_port->designated_regionRootId, sizeof(mstp_brg_id_t));
    rpc_cmp = p_db_port->port_rootPathCost - p_db_port->designated_intRootPathCost;
    designated_cmp = sal_memcmp(&p_db_port->port_designatedBridgeId, &p_db_port->designated_designatedBridgeId, sizeof(mstp_brg_id_t));
    designated_port_cmp = p_db_port->port_designatedPortId - p_db_port->designated_designatedPortId;

    if ((root_cmp < 0)
     || ((root_cmp == 0) && (rpc_cmp < 0))
     || ((root_cmp == 0) && (rpc_cmp == 0) && (designated_cmp < 0))
     || ((root_cmp == 0) && (rpc_cmp == 0) && (designated_cmp == 0) && (designated_port_cmp < 0)))
    {
        return -1;
    }
    else if ((root_cmp == 0) && (rpc_cmp == 0) && (designated_cmp == 0) && (designated_port_cmp == 0))
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

/* compare timer of port and rootport */
int32
msti_compareTimer_port_root(tbl_mstp_instance_t *p_db_inst, tbl_msti_port_t *p_db_port, tbl_msti_port_t *p_db_rootport)
{
    if (NULL == p_db_rootport)
    {
        if (p_db_port->port_messageAge == p_db_inst->root_messageAge
         && p_db_port->port_maxAge == p_db_inst->root_maxAge
         && p_db_port->port_fwdDelay == p_db_inst->root_fwdDelay
         && p_db_port->port_helloTime == p_db_inst->root_helloTime)
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }
    else
    {
        if (p_db_port->port_messageAge == p_db_rootport->port_messageAge
         && p_db_port->port_maxAge == p_db_rootport->port_maxAge
         && p_db_port->port_fwdDelay == p_db_rootport->port_fwdDelay
         && p_db_port->port_helloTime == p_db_rootport->port_helloTime)
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }
}

/*
17.21.25 updtRolesTree()
*/
int32 
msti_updtRolesTree(tbl_mstp_instance_t *p_db_inst)
{
    msti_port_priority_vector_t best_priority;
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    tbl_msti_port_t *p_db_port = NULL;
    tbl_msti_port_t *p_db_rootport = NULL;
    tbl_mstp_port_t *p_db_mstp_rootport = NULL;
    tbl_mstp_port_t *p_mstp_port = NULL;

    ctclib_slistnode_t *listnode = NULL;
    int32 port_priority_cmp = 0;
    uint32 is_self_root = TRUE;
    uint32 is_rootport_change = FALSE;
    char msti_port_name[IFNAME_EXT_SIZE];

    (void)is_rootport_change;
    /*
    a) The root path priority vector for each Port that has a port priority vector (portPriority plus portId;
    17.19.19, 17.19.21), recorded from a received message and not aged out (infoIs == Received)
    */

    /* best_priority is inited with bridge priority vector */
    msti_getBridgePriority(&best_priority, p_db_inst);

    /* calculate best_priority by compare all port's priority */
    CTCLIB_SLIST_LOOP(p_db_inst->port_list.obj_list, p_db_port, listnode)
    {
        p_mstp_port = (tbl_mstp_port_t*)p_db_port->p_port;
        if (p_mstp_port->portEnabled && (MSTP_INFO_RECEIVED == p_db_port->infoIs))
        {
            /* whose DesignatedBridgeID Bridge Address component is not equal to that component of 
               the Bridge¡¯s own bridge priority vector */
            if (0 == sal_memcmp(p_db_port->port_designatedBridgeId.address, p_db_inst->bridge_id.address, MAC_ADDR_LEN))
            {
                continue;
            }
            
            port_priority_cmp = msti_compareBestPortPriority(&best_priority, p_db_port);
            if (port_priority_cmp < 0)
            {
                continue;
            }
            else if (port_priority_cmp == 0)
            {
                SAL_ASSERT(0);
            }
            else
            {
                /* modified by liwh for bug 37514, 2016-05-17 */
                /* Ports with Restricted Role not considered for root port selection */
                if (p_db_port->restricted_role)
                {
                    continue;    
                }
                /* liwh end */
                
                p_db_rootport = p_db_port;
                msti_updateBestPortPriority(&best_priority, p_db_rootport);
                is_self_root = FALSE;
            }
        }
    }

    if (is_self_root)
    {
        /* clear root port */
        p_db_inst->root_port = NULL;
        p_db_inst->root_portId = 0;
        p_db_inst->hopCount = p_db_glb->bridge_maxHops;
        MSTP_DEBUG(STP_PROTO_MSTI, "[msti updtRolesTree] Instance %u reselect as root bridge", p_db_inst->key.instance);
    }
    else
    {
        p_db_mstp_rootport = (tbl_mstp_port_t*)p_db_rootport->p_port;
        if (p_db_mstp_rootport->rcvdInternal)
        {
            p_db_inst->hopCount = p_db_rootport->hopCount;
        }
        else
        {
            p_db_inst->hopCount = p_db_glb->bridge_maxHops;
        }
        if (p_db_rootport != p_db_inst->root_port)
        {
            is_rootport_change = TRUE;
            p_db_inst->root_port = p_db_rootport;
            p_db_inst->root_portId = p_db_rootport->portId;
        }
        else
        {
            /* Added by kcao for bug 37739, need update root_portId for port-priority maybe changes */
            if (p_db_inst->root_portId != p_db_rootport->portId)
            {
                p_db_inst->root_portId = p_db_rootport->portId;
            }
        }
        MSTP_MSTI_PORT_STR(msti_port_name, p_db_rootport);
        MSTP_DEBUG(STP_PROTO_MSTI, "[msti updtRolesTree] msti_port %s reselect as root port", msti_port_name);
    }

    /*
    b) The Bridge¡¯s root priority vector (rootPriority plus rootPortId; 17.18.6, 17.18.5), chosen as the best
    of the set of priority vectors comprising the Bridge¡¯s own bridge priority vector (BridgePriority;
    17.18.3) and all the calculated root path priority vectors whose DesignatedBridgeID Bridge Address
    component is not equal to that component of the Bridge¡¯s own bridge priority vector (see 17.6)

    root priority vector = {B : 0 : B : 0 : 0} if B is better than RD , or
                         = {RD : RPCD + PPCPB : D : PD : PB } if B is worse than RD    
    */
    if (is_self_root)
    {
        sal_memcpy(&p_db_inst->root_rootId, &p_db_inst->bridge_id, sizeof(mstp_brg_id_t));
        p_db_inst->root_rootPathCost = 0;
        sal_memcpy(&p_db_inst->root_designatedBridgeId, &p_db_inst->bridge_id, sizeof(mstp_brg_id_t));
        p_db_inst->root_designatedPortId = 0;
        p_db_inst->root_bridgePortId = 0;
    }
    else
    {
        sal_memcpy(&p_db_inst->root_rootId, &p_db_rootport->port_rootId, sizeof(mstp_brg_id_t));
        p_db_inst->root_rootPathCost = p_db_rootport->port_rootPathCost + p_db_rootport->portPathCost;
        sal_memcpy(&p_db_inst->root_designatedBridgeId, &p_db_rootport->port_designatedBridgeId, sizeof(mstp_brg_id_t));
        p_db_inst->root_designatedPortId = p_db_rootport->port_designatedPortId;
        p_db_inst->root_bridgePortId = p_db_rootport->portId;
    }
    
    /*
    c) The Bridge¡¯s rootTimes (17.18.7) parameter, set equal to:
    */
    if (is_self_root)
    {
        /* 1) BridgeTimes (17.18.4), if the chosen root priority vector is the bridge priority vector, otherwise
        */
        p_db_inst->root_messageAge   = 0;
        p_db_inst->root_fwdDelay     = p_db_glb->bridge_fwdDelay;
        p_db_inst->root_helloTime    = p_db_glb->bridge_helloTime;
        p_db_inst->root_maxAge       = p_db_glb->bridge_maxAge;
    }
    else
    {
        /* 2) portTimes (17.19.22) for the port associated with the selected root priority vector, with the
           Message Age component incremented by 1 second and rounded to the nearest whole second. */
        p_db_inst->root_messageAge   = p_db_rootport->port_messageAge;
        p_db_inst->root_fwdDelay     = p_db_rootport->port_fwdDelay;
        p_db_inst->root_helloTime    = p_db_rootport->port_helloTime;
        p_db_inst->root_maxAge       = p_db_rootport->port_maxAge;
        p_db_inst->root_messageAge += 1;
    }

    /*
    d) The first four components of the designated priority vector (designatedPriority, 17.19.4) for each port.
    designated priority vector = {B : 0 : B : QB : QB } if B is better than RD, or
                               = {RD : RPCD + PPCPB : B : QB: QB }if B is worse than RD
    */
    CTCLIB_SLIST_LOOP(p_db_inst->port_list.obj_list, p_db_port, listnode)
    {
        sal_memcpy(&p_db_port->designated_regionRootId, &p_db_inst->root_rootId, sizeof(mstp_brg_id_t));
        p_db_port->designated_intRootPathCost = p_db_inst->root_rootPathCost;
        sal_memcpy(&p_db_port->designated_designatedBridgeId, &p_db_inst->bridge_id, sizeof(mstp_brg_id_t));
        p_db_port->designated_designatedPortId = p_db_port->portId;
        p_db_port->designated_bridgePortId = p_db_port->portId;
        p_db_port->hopCount = p_db_inst->hopCount;
    }
    
    /*
    e) The designatedTimes (17.19.5) for each Port, set equal to the value of rootTimes, except for the
    Hello Time component, which is set equal to BridgeTimes¡¯ Hello Time
    */
    CTCLIB_SLIST_LOOP(p_db_inst->port_list.obj_list, p_db_port, listnode)
    {
        p_db_port->designated_messageAge   = p_db_inst->root_messageAge;
        p_db_port->designated_fwdDelay     = p_db_inst->root_fwdDelay;
        p_db_port->designated_helloTime    = p_db_inst->root_helloTime; // bridge_helloTime;
        p_db_port->designated_maxAge       = p_db_inst->root_maxAge;
    }
    
    CTCLIB_SLIST_LOOP(p_db_inst->port_list.obj_list, p_db_port, listnode)
    {
        /* Deleted by kcao for bug 38829 2016-07-11 */
#if 0
        /* for self root, update all port to designated */
        if (is_self_root)
        {
            if (ROLE_DESIGNATED != p_db_port->role)
            {
                p_db_port->infoIs = MSTP_INFO_MINE;
            }
        }
#endif
        p_mstp_port = (tbl_mstp_port_t*)p_db_port->p_port;
        if (!p_mstp_port->rcvdInternal && (MSTP_INFO_RECEIVED == p_mstp_port->infoIs))
        {
            if (ROLE_ALTERNATE == p_mstp_port->selectedRole)
            {
                p_db_port->selectedRole = ROLE_ALTERNATE;
            }
            if (ROLE_ROOTPORT == p_mstp_port->selectedRole)
            {
                p_db_port->selectedRole = ROLE_MASTERPORT;
            }
        }
        else
        {

            if (MSTP_INFO_DISABLED == p_db_port->infoIs)
            {
                /*
                f) If the Port is Disabled (infoIs = Disabled), selectedRole is set to DisabledPort. Otherwise:
                */
                p_db_port->selectedRole = ROLE_DISABLED;
            }
            else if (MSTP_INFO_AGED == p_db_port->infoIs)
            {
                /*
                g) If the port priority vector information was aged (infoIs = Aged), updtInfo is set and selectedRole is
                set to DesignatedPort.
                */
                p_db_port->updtInfo = TRUE;
                p_db_port->selectedRole = ROLE_DESIGNATED;

                /* reset agreed */
                p_db_port->agree = FALSE;
                p_db_port->agreed = FALSE;
            }
            else if (MSTP_INFO_MINE == p_db_port->infoIs)
            {
                /*
                h) If the port priority vector was derived from another port on the Bridge or from the Bridge itself as
                the Root Bridge (infoIs = Mine), selectedRole is set to DesignatedPort. Additionally, updtInfo is set
                if the port priority vector differs from the designated priority vector or the Port¡¯s associated timer
                parameters differ from those for the Root Port.
                */
                p_db_port->selectedRole = ROLE_DESIGNATED;
                if (msti_comparePriority_port_designated(p_db_port) 
                 || msti_compareTimer_port_root(p_db_inst, p_db_port, p_db_rootport))
                {
                    p_db_port->updtInfo = TRUE;
                }

                /* reset agreed */
                p_db_port->agree = FALSE;
                p_db_port->agreed = FALSE;
            }
            else if (MSTP_INFO_RECEIVED == p_db_port->infoIs)
            {
                if (p_db_inst->root_portId != 0 && p_db_inst->root_portId == p_db_port->portId)
                {
                    /*
                    i) If the port priority vector was received in a Configuration Message and is not aged (infoIs ==
                    Received), and the root priority vector is now derived from it, selectedRole is set to RootPort and
                    updtInfo is reset.
                    */
                    p_db_port->updtInfo = FALSE;
                    p_db_port->selectedRole = ROLE_ROOTPORT;
                }
                else
                {
                    if (msti_comparePriority_port_designated(p_db_port) <= 0)
                    {
                        /*
                        j) If the port priority vector was received in a Configuration Message and is not aged
                        (infoIs = Received), the root priority vector is not now derived from it, the designated priority vector
                        is not higher than the port priority vector, and the designated bridge and designated port components
                        of the port priority vector do not reflect another port on this bridge, selectedRole is set to
                        AlternatePort and updtInfo is reset.
                        */
                        if (0 != sal_memcmp(&p_db_port->port_designatedBridgeId, &p_db_inst->bridge_id, sizeof(mstp_brg_id_t)))
                        {
                            p_db_port->updtInfo = FALSE;
                            p_db_port->selectedRole = ROLE_ALTERNATE;
                        }
                        /*
                        k) If the port priority vector was received in a Configuration Message and is not aged
                        (infoIs = Received), the root priority vector is not now derived from it, the designated priority vector
                        is not higher than the port priority vector, and the designated bridge and designated port components
                        of the port priority vector reflect another port on this bridge, selectedRole is set to BackupPort and
                        updtInfo is reset.
                        */
                        else
                        {
                            p_db_port->updtInfo = FALSE;
                            p_db_port->selectedRole = ROLE_BACKUP;
                        }
                    }
                    else
                    {
                        /*
                        l) If the port priority vector was received in a Configuration Message and is not aged (infoIs ==
                        Received), the root priority vector is not now derived from it, the designated priority vector is higher
                        than the port priority vector, selectedRole is set to DesignatedPort and updtInfo is set.
                        */
                        p_db_port->updtInfo = TRUE;
                        p_db_port->selectedRole = ROLE_DESIGNATED;

                        /* reset agreed */
                        p_db_port->agree = FALSE;
                        p_db_port->agreed = FALSE;             
                    }
                }
            }

        }
    }

    return PM_E_NONE;
}

static uint32 
_msti_betterOrSameInfoRcvd(tbl_msti_port_t *p_db_port)
{
    msti_bpdu_t *bpdu = (msti_bpdu_t*)p_db_port->bpdu;
    int32 result = 0;

    result = msti_comparePriority_msg_port(bpdu, p_db_port);
    if (result <= 0)
    {
        return TRUE;
    }
  
    return FALSE;
}

static uint32 
_msti_betterOrSameInfoMine(tbl_msti_port_t *p_db_port)
{
    int32 root_cmp = 0;
    int32 rpc_cmp = 0;
    int32 designated_cmp = 0;
    int32 designated_port_cmp = 0;
    
    root_cmp = sal_memcmp(&p_db_port->designated_regionRootId, &p_db_port->port_rootId, sizeof(mstp_brg_id_t));
    rpc_cmp = p_db_port->designated_intRootPathCost - p_db_port->port_rootPathCost;
    designated_cmp = sal_memcmp(&p_db_port->designated_designatedBridgeId, &p_db_port->port_designatedBridgeId, sizeof(mstp_brg_id_t));
    designated_port_cmp = p_db_port->designated_designatedPortId - p_db_port->port_designatedPortId;

    if ((root_cmp < 0)
     || ((root_cmp == 0) && (rpc_cmp < 0))
     || ((root_cmp == 0) && (rpc_cmp == 0) && (designated_cmp < 0))
     || ((root_cmp == 0) && (rpc_cmp == 0) && (designated_cmp == 0) && (designated_port_cmp < 0))
     || ((root_cmp == 0) && (rpc_cmp == 0) && (designated_cmp == 0) && (designated_port_cmp == 0)))
    {
        return TRUE;
    }

    return FALSE;
}

uint32 
msti_betterOrSameInfo(tbl_msti_port_t *p_db_port, mstp_info_t newInfoIs)
{
    if (newInfoIs == MSTP_INFO_RECEIVED && newInfoIs == p_db_port->infoIs)
    {
        return _msti_betterOrSameInfoRcvd(p_db_port);
    }
    else if (newInfoIs == MSTP_INFO_MINE && newInfoIs == p_db_port->infoIs)
    {
        return _msti_betterOrSameInfoMine(p_db_port);
    }
    
    return FALSE;
}

/* 17.21.9 recordAgreement() */
int32 
msti_recordAgreement(tbl_msti_port_t *p_db_port)
{
    tbl_mstp_port_t *p_mstp_port = NULL;
    msti_bpdu_t *bpdu = (msti_bpdu_t*)p_db_port->bpdu;

    p_mstp_port = (tbl_mstp_port_t*)p_db_port->p_port;
    if (p_mstp_port->rstpVersion && p_mstp_port->operP2PMac)
    {
        if (bpdu->agreement)
        {
            p_db_port->agreed = TRUE;
            p_db_port->proposing = FALSE;

            _msti_sm_PRT_TRIGGER_EVENT_DESIGNATED(p_db_port);
        }
        else
        {
            p_db_port->agreed = FALSE;
        }
    }

    return PM_E_NONE;
}

/* 17.21.10 recordDispute()
If an RST BPDU with the learning flag set has been received:
a) The agreed flag is set; and
b) The proposing flag is cleared.
*/
int32 
msti_recordDispute(tbl_msti_port_t *p_db_port)
{
    msti_bpdu_t *bpdu = (msti_bpdu_t*)p_db_port->bpdu;

    if (bpdu->learning)
    {
        p_db_port->disputed = TRUE;
        p_db_port->agreed = FALSE;
    }

    return PM_E_NONE;
}

/*
17.21.11 recordProposal()
If the received Configuration Message conveys a Designated Port Role, and has the Proposal flag is set, the
proposed flag is set. Otherwise, the proposed flag is not changed.
*/
int32
msti_recordProposal(tbl_msti_port_t *p_db_port)
{
    msti_bpdu_t *bpdu = (msti_bpdu_t*)p_db_port->bpdu;
    
    if (bpdu->proposal && ROLE_DESIGNATED == bpdu->port_role)
    {
        p_db_port->proposed = TRUE;
    }
    
    return PM_E_NONE;
}

int32
msti_recordHopCount(tbl_msti_port_t *p_db_port)
{
    tbl_mstp_instance_key_t inst_key;
    tbl_mstp_instance_t *p_db_inst = NULL;
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    tbl_mstp_port_t *p_mstp_port = (tbl_mstp_port_t*)p_db_port->p_port;
    msti_bpdu_t *bpdu = p_db_port->bpdu;
    uint32 new_hopCount = 0;

    if (p_mstp_port->rcvdInternal)
    {
        if (bpdu->hops - 1 >= 0)
        {
            new_hopCount = bpdu->hops - 1;
        }
        else
        {
            new_hopCount = 0;
        }
    }
    else
    {
        new_hopCount = p_db_glb->bridge_maxHops;
    }

    if (new_hopCount != p_db_port->hopCount)
    {
        p_db_port->hopCount = new_hopCount;
        inst_key.instance = p_db_port->key.instance;
        p_db_inst = tbl_mstp_instance_get_mstp_instance(&inst_key);
        if (p_db_inst)
        {
            if (p_db_port == p_db_inst->root_port)
            {
                mstp_api_msti_reselect(p_db_inst);
            }
        }
    }

    return PM_E_NONE;
}

/*
17.21.12 recordPriority()
Sets the components of the portPriority variable to the values of the corresponding msgPriority components.
*/
int32
msti_recordPriority(tbl_msti_port_t *p_db_port)
{
    msti_bpdu_t *bpdu = (msti_bpdu_t*)p_db_port->bpdu;

    sal_memcpy(&p_db_port->port_rootId, &bpdu->reg_root_id, sizeof(mstp_brg_id_t));
    p_db_port->port_rootPathCost = bpdu->internal_rpc;
    sal_memcpy(&p_db_port->port_designatedBridgeId, &bpdu->bridge_id, sizeof(mstp_brg_id_t));
    p_db_port->port_designatedPortId = bpdu->port_id;
    p_db_port->port_bridgePortId = p_db_port->portId;

    return PM_E_NONE;
}

/*
17.21.13 recordTimes()
Sets portTimes¡¯ Message Age, MaxAge, and Forward Delay to the received values held in the messageTimes
parameter. and portTimes¡¯ Hello time to messageTimes¡¯ HelloTime if that is greater than the minimum
specified in the Compatibility Range column of Table 17-1, and to that minimum otherwise.
*/
int32
msti_recordTimes(tbl_msti_port_t *p_db_port)
{
    msti_bpdu_t *bpdu = (msti_bpdu_t*)p_db_port->bpdu;
    bpdu_t *cist_bpdu = (bpdu_t*)bpdu->cist;

    p_db_port->port_messageAge  = cist_bpdu->messageAge;
    p_db_port->port_maxAge      = cist_bpdu->maxAge;
    p_db_port->port_fwdDelay    = cist_bpdu->fwdDelay;
    p_db_port->port_helloTime   = cist_bpdu->helloTime;

    return PM_E_NONE;
}

#define _17_28__13_33__PRS_Port_Role_Selection_Machine_

/* PRS State (Port Role Selection Machine)
   802.1D.2004  P.174  17.28 Port Role Selection state machine
   802.1Q.2005  P.183  13.33 Port Role Selection state machine
*/
extern uint32 mstp_sm_prs_state_table[PRS_EVENT_MAX][PRS_STATE_MAX];

static int32
_msti_sm_PRS_TRIGGER_EVENT(tbl_mstp_instance_t *p_db_inst)
{
    tbl_mstp_port_t *p_mstp_port = NULL;
    tbl_msti_port_t *p_db_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PRS] Instance %u enter TRIGGER_EVENT", p_db_inst->key.instance);

    CTCLIB_SLIST_LOOP(p_db_inst->port_list.obj_list, p_db_port, listnode)
    {
        p_mstp_port = (tbl_mstp_port_t*)p_db_port->p_port;
        if (p_mstp_port->portEnabled)
        {
            if (p_db_port->selected && p_db_port->updtInfo)
            {
                msti_sm_pim_process_event(p_db_port, PIM_EVT_SELECTEDUPDTINFO);
            }
        }
    }

    CTCLIB_SLIST_LOOP(p_db_inst->port_list.obj_list, p_db_port, listnode)
    {
        p_mstp_port = (tbl_mstp_port_t*)p_db_port->p_port;
        if (p_mstp_port->portEnabled)
        {
            if (p_db_port->selectedRole == p_db_port->role)
            {
                continue;
            }

            if ((ROLE_DESIGNATED == p_db_port->role) || (ROLE_ROOTPORT == p_db_port->role))
            {
                if ((ROLE_DESIGNATED == p_db_port->selectedRole) || (ROLE_ROOTPORT == p_db_port->selectedRole))
                {

                }
                else
                {
                    if (TCM_ACTIVE == p_db_port->tcm_state)
                    {
                        msti_sm_tcm_process_event(p_db_port, TCM_EVT_ROLENOTROOTDESIGNATEDPORT);
                    }
                }
            }

            switch (p_db_port->selectedRole)
            {
            case ROLE_DISABLED:
                msti_sm_prt_process_event(p_db_port, PRT_EVT_ROLE_DISABLED);
                break;
            case ROLE_ROOTPORT:
                msti_sm_prt_process_event(p_db_port, PRT_EVT_ROLE_ROOTPORT);
                break;
            case ROLE_MASTERPORT:
                msti_sm_prt_process_event(p_db_port, PRT_EVT_ROLE_MASTERPORT);
                break;
            case ROLE_DESIGNATED:
                /* When first ROLE_DISABLED -> PRT_DESIGNATED_PORT, should enter PRT_DESIGNATED_DISCARD state */
                if (ROLE_DISABLED == p_db_port->role && !p_mstp_port->operEdge)
                {
                    p_db_port->disputed = TRUE;
                    p_db_port->learn = TRUE;
                    p_db_port->forward = TRUE;
                }
                msti_sm_prt_process_event(p_db_port, PRT_EVT_ROLE_DESIGNATED);
                break;
            case ROLE_ALTERNATE:
                msti_sm_prt_process_event(p_db_port, PRT_EVT_ROLE_ALTERNATE);
                break;
            case ROLE_BACKUP:
                msti_sm_prt_process_event(p_db_port, PRT_EVT_ROLE_BACKUP);
                break;
            default:
                break;
            }
        }
    }

    return PM_E_NONE;
}

/*
17.21.24 updtRoleDisabledTree()
Sets selectedRole to DisabledPort for all Ports of the Bridge.
*/
int32 
msti_updtRoleDisabledTree(tbl_mstp_instance_t *p_db_inst)
{
    tbl_msti_port_t *p_db_port = NULL;
    ctclib_slistnode_t *listnode = NULL;

    CTCLIB_SLIST_LOOP(p_db_inst->port_list.obj_list, p_db_port, listnode)
    {
        if (ROLE_DISABLED != p_db_port->selectedRole)
        {
            p_db_port->selectedRole = ROLE_DISABLED;
        }
    }

    return PM_E_NONE;
}

/*
17.21.2 clearReselectTree()
Clears reselect for all Ports of the Bridge.
*/
int32
msti_clearReselectTree(tbl_mstp_instance_t *p_db_inst)
{
    tbl_msti_port_t *p_db_port = NULL;
    ctclib_slistnode_t *listnode = NULL;

    CTCLIB_SLIST_LOOP(p_db_inst->port_list.obj_list, p_db_port, listnode)
    {
        if (p_db_port->reselect)
        {
            p_db_port->reselect = FALSE;
        }
    }

    return PM_E_NONE;
}

/*
17.21.16 setSelectedTree()
Sets the selected variable TRUE for all Ports of the Bridge if reselect is FALSE for all Ports. If reselect is
TRUE for any Port, this procedure takes no action.
*/
int32 
msti_setSelectedTree(tbl_mstp_instance_t *p_db_inst)
{
    tbl_mstp_port_t *p_mstp_port = NULL;
    tbl_msti_port_t *p_db_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    uint32 has_reselect = FALSE;
    
    CTCLIB_SLIST_LOOP(p_db_inst->port_list.obj_list, p_db_port, listnode)
    {
        p_mstp_port = (tbl_mstp_port_t*)p_db_port->p_port;
        if (p_mstp_port->portEnabled && p_db_port->reselect)
        {
            has_reselect = TRUE;
            break;
        }
    }

    if (!has_reselect)
    {
        CTCLIB_SLIST_LOOP(p_db_inst->port_list.obj_list, p_db_port, listnode)
        {
            p_mstp_port = (tbl_mstp_port_t*)p_db_port->p_port;
            if (p_mstp_port->portEnabled)
            {
                p_db_port->selected = TRUE;
            }
        }
    }

    return PM_E_NONE;
}

/*
updtRoleDisabledTree();
*/
static int32
_msti_sm_PRS_INIT_BRIDGE(tbl_mstp_instance_t *p_db_inst)
{
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PRS] Instance :%u enter INIT_BRIDGE", p_db_inst->key.instance);
    
    msti_updtRoleDisabledTree(p_db_inst);

    p_db_inst->prs_state = PRS_INIT_BRIDGE;
    
    return PM_E_NONE;
}

/*
clearReselectTree();
updtRolesTree();
setSelectedTree();
*/
static int32
_msti_sm_PRS_ROLE_SELECTION(tbl_mstp_instance_t *p_db_inst)
{
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PRS] Instance :%u enter ROLE_SELECTION", p_db_inst->key.instance);
    
    msti_clearReselectTree(p_db_inst);
    msti_updtRolesTree(p_db_inst);
    msti_setSelectedTree(p_db_inst);

    p_db_inst->prs_state = PRS_ROLE_SELECTION;
    
    _msti_sm_PRS_TRIGGER_EVENT(p_db_inst);

    return PM_E_NONE;
}

/* 
 Figure 17-17   Port Role Selection machine
*/
int32
msti_sm_prs_process_event(tbl_mstp_instance_t *p_db_inst, prs_event_t event)
{
    prs_state_t next_state = 0;

    if (event >= PRS_EVENT_MAX)
    {
        return PM_E_FAIL;
    }

    next_state =  mstp_sm_prs_state_table[event][p_db_inst->prs_state];
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti %s] Instance :%u recv event: %s, state: %s -> %s", 
            "PRS",
            p_db_inst->key.instance,
            cdb_enum_val2str(prs_event_strs, PRS_EVENT_MAX, event),
            cdb_enum_val2str(prs_state_strs, PRS_STATE_MAX, p_db_inst->prs_state),
            cdb_enum_val2str(prs_state_strs, PRS_STATE_MAX, next_state));

    switch (next_state)
    {
    case PRS_INIT_BRIDGE:
        _msti_sm_PRS_INIT_BRIDGE(p_db_inst);
        _msti_sm_PRS_ROLE_SELECTION(p_db_inst);
        break;
    case PRS_ROLE_SELECTION:
        _msti_sm_PRS_ROLE_SELECTION(p_db_inst);
        break;
    default:
        break;
    }

    return PM_E_NONE;
}

#define _17_27__13_32__PIM_Port_Information_Machine_

/* PIM State (Port Information Machine)
   802.1D.2004  P.173  17.27 Port Information state machine
   802.1Q.2005  P.182  13.32 Port Information state machine
*/
extern uint32 mstp_sm_pim_state_table[PIM_EVENT_MAX][PIM_STATE_MAX];

/*
(infoIs == Received) && (rcvdInfoWhile == 0) && !updtInfo && !rcvdMsg
*/
static void
_msti_sm_rcvdInfoWhile_timer_fn(void *p_data)
{
    tbl_msti_port_t *p_db_port = (tbl_msti_port_t*)p_data;
    tbl_mstp_port_t *p_mstp_port = (tbl_mstp_port_t*)p_db_port->p_port;
    char msti_port_name[IFNAME_EXT_SIZE];

    MSTP_MSTI_PORT_STR(msti_port_name, p_db_port);
    MSTP_DEBUG(STP_TIMER, "msti Timer rcvdInfoWhile timeout on msti_port %s", msti_port_name);

    p_db_port->rcvdInfoWhile_timer = NULL;

    /* modified by liwh for bug 37762, 2016-06-21 */
    if (p_mstp_port->admin_loopguard)
    {
        if (ROLE_ROOTPORT == p_db_port->role || ROLE_ALTERNATE == p_db_port->role || ROLE_BACKUP == p_db_port->role)      
        {          
            MSTP_DEBUG(STP_PROTO_MSTI, "_msti_sm_rcvdInfoWhile_timer_fn: could not receive a bpdu on a loop guard enabled "          
                "msti_port %s", msti_port_name);

            p_db_port->oper_loopguard = TRUE;
            _msti_sm_PST_DISCARDING(p_db_port, msti_port_name);
        }
    }
    /* liwh end */

    /* modified by liwh for bug 37722, 2016-08-16 */
    msti_newTcWhile(p_db_port);
    /* liwh end */
    
    if ((MSTP_INFO_RECEIVED == p_db_port->infoIs)
     && (!p_db_port->updtInfo)
     && (!p_db_port->rcvdMsg))
    {
        msti_sm_pim_process_event(p_db_port, PIM_EVT_RECVINFOWHILEZERO);
    }

    return;
}

/*
17.21.23 updtRcvdInfoWhile()
Updates rcvdInfoWhile (17.17.6). The value assigned to rcvdInfoWhile is the three times the Hello Time, if
Message Age, incremented by 1 second and rounded to the nearest whole second, does not exceed Max Age,
and is zero otherwise. The values of Message Age, Max Age, and Hello Time used in this calculation are
taken from portTimes (17.19.22).
*/
int32
msti_updtRcvdInfoWhile(tbl_msti_port_t *p_db_port)
{
    msti_bpdu_t *bpdu = (msti_bpdu_t*)p_db_port->bpdu;
    bpdu_t *cist_bpdu = (bpdu_t*)bpdu->cist;

    CTC_TASK_STOP_TIMER(p_db_port->rcvdInfoWhile_timer);
    if (cist_bpdu->messageAge + 1 >= p_db_port->port_maxAge)
    {
        p_db_port->rcvdInfoWhile_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _msti_sm_rcvdInfoWhile_timer_fn, p_db_port, 
            0);
    }
    else
    {
        p_db_port->rcvdInfoWhile_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _msti_sm_rcvdInfoWhile_timer_fn, p_db_port, 
            3 * p_db_port->port_helloTime);
    }

    return PM_E_NONE;
}

/* 17.21.17 setTcFlags() */
int32
msti_setTcFlags(tbl_msti_port_t *p_db_port)
{
    msti_bpdu_t *bpdu = (msti_bpdu_t*)p_db_port->bpdu;
    
    //if (bpdu->topology_change_ack)
    if (0)  // No topo_change_ack for MSTI
    {
        p_db_port->rcvdTcAck = TRUE;
        msti_sm_tcm_process_event(p_db_port, TCM_EVT_RCVDTCACK);
    }
    else if (bpdu->topology_change)
    {
        p_db_port->rcvdTc = TRUE;
        msti_sm_tcm_process_event(p_db_port, TCM_EVT_RCVDTC);
    }
    
    return PM_E_NONE;
}

/*
17.21.8 rcvInfo()
*/
mstp_rcvd_info_t 
msti_rcvInfo(tbl_msti_port_t *p_db_port)
{
    msti_bpdu_t *bpdu = (msti_bpdu_t*)p_db_port->bpdu;
    bpdu_t *cist_bpdu = (bpdu_t*)bpdu->cist;
    int32 priority_result = 0;
    int32 timer_result = 0;

    priority_result = msti_comparePriority_msg_port(bpdu, p_db_port);
    timer_result = msti_compareTimer_msg_port(bpdu, p_db_port);

    if (((BPDU_VERSION_RSTP <= cist_bpdu->version) && (ROLE_DESIGNATED == bpdu->port_role || ROLE_ROOTPORT == bpdu->port_role|| ROLE_ALTERNATE == bpdu->port_role || ROLE_MASTERPORT == bpdu->port_role))
     || (BPDU_VERSION_STP == cist_bpdu->version))
    {
        if (priority_result < 0)
        {
            /*
            a) The received message conveys a Designated Port Role, and
               1) The message priority is superior (17.6) to the Port¡¯s port priority vector
            */
            return MSTP_SUPERIOR_DESIGNATED;
        }
        else if ((priority_result == 0)
            && (timer_result != 0))
        {
            /*
            a) The received message conveys a Designated Port Role, and
                2) The message priority vector is the same as the Port¡¯s port priority vector, and any of the
                received timer parameter values (msgTimes¡ª17.19.15) differ from those already held for the
                Port (portTimes¡ª17.19.22).
            */
            return MSTP_SUPERIOR_DESIGNATED;
        }
        else if ((priority_result == 0)
            && (timer_result == 0))
        {
            /*
            b) The received message conveys Designated Port Role, and a message priority vector and timer
            parameters that are the same as the Port¡¯s port priority vector or timer values.
            */
            return MSTP_REPEATED_DESIGNATED;
        }
        else if (priority_result > 0)
        {
            /* modified by liwh for bug 53744, 2019-09-26 */
            if (ROLE_ROOTPORT != bpdu->port_role)
            /* liwh end */
            {
                /*
                c) The received message conveys a Designated Port Role, and a message priority vector that is worse
                than the Port¡¯s port priority vector.
                */
                return MSTP_INFERIOR_DESIGNATED;
            }
            /* modified by liwh for bug 53744, 2019-09-26 */
            else
            {
                return MSTP_INFERIOR_ROOTALTERNATE;
            }
            /* liwh end */
        }
    }
    else if (priority_result >= 0)
    {
        /* d) The received message conveys a Root Port, Alternate Port, or Backup Port Role and a message
           priority that is the same as or worse than the port priority vector. 
         */
        return MSTP_INFERIOR_ROOTALTERNATE;
    }
    else
    {
        /* Otherwise, returns OtherInfo. */
        return MSTP_OTHER;
    }

    return MSTP_OTHER;
}

static int32
_msti_sm_PIM_TRIGGER_EVENT(tbl_msti_port_t *p_db_port)
{
    if ((PRT_ROOT_PORT == p_db_port->prt_state) || (PRT_ALTERNATE_PORT == p_db_port->prt_state))
    {
        if (p_db_port->proposed && !p_db_port->agree)
        {
            msti_sm_prt_process_event(p_db_port, PRT_EVT_PROPOSED_NOTAGREE);
        }
    }
    
    return PM_E_NONE;
}

/*
rcvdMsg = FALSE;
proposing = proposed = agree = agreed = FALSE;
rcvdInfoWhile = 0;
infoIs = Disabled; reselect = TRUE; selected = FALSE;
*/
static int32
_msti_sm_PIM_DISABLED(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PIM] %s enter DISABLED", msti_port_name);
    
    p_db_port->rcvdMsg = FALSE;
    p_db_port->proposing = p_db_port->proposed = p_db_port->agree = p_db_port->agreed = FALSE;
    CTC_TASK_STOP_TIMER(p_db_port->rcvdInfoWhile_timer);
    p_db_port->infoIs = MSTP_INFO_DISABLED; p_db_port->reselect = TRUE; p_db_port->selected = FALSE;

    p_db_port->pim_state = PIM_DISABLED;

    return PM_E_NONE;
}

/*
infoIs = Aged;
reselect = TRUE; selected = FALSE;
*/
static int32
_msti_sm_PIM_AGED(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    tbl_mstp_instance_key_t inst_key;
    tbl_mstp_instance_t *p_db_inst = NULL;
    
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PIM] %s enter AGED", msti_port_name);
    
    p_db_port->infoIs = MSTP_INFO_AGED;
    p_db_port->reselect = TRUE; p_db_port->selected = FALSE;

    p_db_port->pim_state = PIM_AGED;

    inst_key.instance = p_db_port->key.instance;
    p_db_inst = tbl_mstp_instance_get_mstp_instance(&inst_key);
    if (p_db_inst)
    {
        mstp_api_msti_reselect(p_db_inst);
    }
    
    return PM_E_NONE;
}

/*
*/
static int32
_msti_sm_PIM_CURRENT(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PIM] %s enter CURRENT", msti_port_name);
    
    p_db_port->pim_state = PIM_CURRENT;

    return PM_E_NONE;
}

/*
proposing = proposed = FALSE;
agreed = agreed && betterorsameInfo();
synced = synced && agreed;
PortPriority = DesignatedPriority; PortTimes = DesignatedTimes;
updtInfo = FALSE; infoIs = Mine; newInfo = TRUE;
*/
static int32
_msti_sm_PIM_UPDATE(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PIM] %s enter UPDATE", msti_port_name);
    
    p_db_port->proposing = p_db_port->proposed = FALSE;

    p_db_port->agreed = p_db_port->agreed && msti_betterOrSameInfo(p_db_port, MSTP_INFO_MINE);
    p_db_port->synced = p_db_port->synced && p_db_port->agreed;

    /* PortPriority = DesignatedPriority; */
    p_db_port->port_rootId              = p_db_port->designated_regionRootId;
    p_db_port->port_rootPathCost        = p_db_port->designated_intRootPathCost;
    p_db_port->port_designatedBridgeId  = p_db_port->designated_designatedBridgeId;
    p_db_port->port_designatedPortId    = p_db_port->designated_designatedPortId;
    p_db_port->port_bridgePortId        = p_db_port->designated_bridgePortId;

    /* PortTimes = DesignatedTimes; */
    p_db_port->port_messageAge = p_db_port->designated_messageAge;
    p_db_port->port_fwdDelay = p_db_port->designated_fwdDelay;
    p_db_port->port_helloTime = p_db_port->designated_helloTime;
    p_db_port->port_maxAge = p_db_port->designated_maxAge;
    
    p_db_port->updtInfo = FALSE;
    p_db_port->infoIs = MSTP_INFO_MINE;
    p_db_port->newInfo = TRUE;
        
    p_db_port->pim_state = PIM_UPDATE;

    _msti_sm_PIM_CURRENT(p_db_port, msti_port_name);

    return PM_E_NONE;
}

/*
rcvdMsg = FALSE
*/
static int32
_msti_sm_PIM_OTHER(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PIM] %s enter OTHER", msti_port_name);
    
    p_db_port->rcvdMsg = FALSE;

    p_db_port->pim_state = PIM_OTHER;

    _msti_sm_PIM_CURRENT(p_db_port, msti_port_name);
    return PM_E_NONE;
}

/*
recordAgreement(); setTcFlags();
rcvdMsg = FALSE;
*/
static int32
_msti_sm_PIM_NOT_DESIGNATED(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PIM] %s enter NOT_DESIGNATED", msti_port_name);
    
    msti_recordAgreement(p_db_port); 
    msti_setTcFlags(p_db_port);
    p_db_port->rcvdMsg = FALSE;

    p_db_port->pim_state = PIM_NOT_DESIGNATED;

    _msti_sm_PIM_CURRENT(p_db_port, msti_port_name);
    return PM_E_NONE;
}

static int32
_msti_sm_update_all_port_rootId(tbl_msti_port_t *p_db_rootport)
{
    tbl_mstp_instance_key_t inst_key;
    tbl_mstp_instance_t *p_db_inst = NULL;
    tbl_msti_port_t *p_db_port = NULL;
    tbl_mstp_port_t *p_mstp_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    msti_bpdu_t *bpdu = (msti_bpdu_t*)p_db_rootport->bpdu;

    inst_key.instance = p_db_rootport->key.instance;
    p_db_inst = tbl_mstp_instance_get_mstp_instance(&inst_key);
    if (NULL == p_db_inst)
    {
        return PM_E_FAIL;
    }

    if (0 == sal_memcmp(bpdu->reg_root_id.address, p_db_rootport->port_rootId.address, MAC_ADDR_LEN))
    {
        /* if rootId.address not changes, but priority changes */
        CTCLIB_SLIST_LOOP(p_db_inst->port_list.obj_list, p_db_port, listnode)
        {
            p_mstp_port = (tbl_mstp_port_t*)p_db_port->p_port;
            if (p_mstp_port->portEnabled)
            {
                if (p_db_port == p_db_rootport)
                {
                    continue;
                }
                
                if (0 == sal_memcmp(&p_db_port->port_rootId, &p_db_rootport->port_rootId, sizeof(mstp_brg_id_t)))
                {
                    sal_memcpy(&p_db_port->port_rootId, &bpdu->reg_root_id, sizeof(mstp_brg_id_t));
                }
            }
        }
    }
    else
    {
        /* Modified by kcao for bug 37798, for DUT1 - DUT2 has multiple Alternate port, should update infoIs to MINE */

        CTCLIB_SLIST_LOOP(p_db_inst->port_list.obj_list, p_db_port, listnode)
        {
            p_mstp_port = (tbl_mstp_port_t*)p_db_port->p_port;
            if (p_mstp_port->portEnabled)
            {
                if (p_db_port == p_db_rootport)
                {
                    continue;
                }

                if (MSTP_INFO_RECEIVED == p_db_port->infoIs)
                {
                    p_db_port->infoIs = MSTP_INFO_MINE;
                }
            }
        }
    }

    return PM_E_NONE;
}

static int32
_msti_sm_clear_rootId_for_rx_inferior_bpdu_from_root_port(tbl_msti_port_t *p_db_port)
{
    tbl_mstp_instance_key_t inst_key;
    tbl_mstp_instance_t *p_db_inst = NULL;
    msti_bpdu_t *bpdu = (msti_bpdu_t*)p_db_port->bpdu;
    int32 root_cmp = 0;

    inst_key.instance = p_db_port->key.instance;
    p_db_inst = tbl_mstp_instance_get_mstp_instance(&inst_key);
    if (NULL == p_db_inst)
    {
        return PM_E_FAIL;
    }

    if (p_db_inst->root_port == p_db_port)
    {
        root_cmp = sal_memcmp(&bpdu->reg_root_id, &p_db_inst->root_rootId, sizeof(mstp_brg_id_t));
        if (root_cmp > 0)
        {
            /* for RX inferior BPDU from root port, we need update other port's rootId.address same to rootId.address */
            _msti_sm_update_all_port_rootId(p_db_port);

            /* for RX inferior BPDU from root port, we think need to clear old rootId immediately, but not by aged */
            msti_recordPriority(p_db_port);
            msti_recordHopCount(p_db_port);
            mstp_api_msti_reselect(p_db_inst);
        }
    }
    else if (ROLE_ALTERNATE == p_db_port->role)
    {
        p_db_port->selectedRole = ROLE_DESIGNATED;
        msti_sm_prt_process_event(p_db_port, PRT_EVT_ROLE_DESIGNATED);
        msti_sm_prt_process_event(p_db_port, PRT_EVT_PROPOSING);
    }

    return PM_E_NONE;
}

/*
recordDispute();
rcvdMsg = FALSE;
*/
static int32
_msti_sm_PIM_INFERIOR_DESIGNATED(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PIM] %s enter INFERIOR_DESIGNATED", msti_port_name);

    /* modified by liwh for bug 36699/37625/37767, 2016-05-11 */
    if (p_db_port->oper_rootguard)
    {
        p_db_port->oper_rootguard = FALSE;
        p_db_port->prt_state = PRT_DESIGNATED_PORT;
    }
    /* liwh end */
    
    msti_recordDispute(p_db_port);
    p_db_port->rcvdMsg = FALSE;
    p_db_port->pim_state = PIM_INFERIOR_DESIGNATED;

    _msti_sm_PIM_CURRENT(p_db_port, msti_port_name);

    /* Added by kcao for bug 37666 2016-06-16 */
    _msti_sm_clear_rootId_for_rx_inferior_bpdu_from_root_port(p_db_port);

    return PM_E_NONE;
}

/*
recordProposal(); setTcFlags();
updtRcvdInfoWhile();
rcvdMsg = FALSE;
*/
static int32
_msti_sm_PIM_REPEATED_DESIGNATED(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PIM] %s enter REPEATED_DESIGNATED", msti_port_name);
    
    msti_recordProposal(p_db_port);
    msti_setTcFlags(p_db_port);
    msti_updtRcvdInfoWhile(p_db_port);
    msti_recordHopCount(p_db_port);
    p_db_port->rcvdMsg = FALSE;
    p_db_port->pim_state = PIM_REPEATED_DESIGNATED;

    _msti_sm_PIM_CURRENT(p_db_port, msti_port_name);
    return PM_E_NONE;
}

/*
agreed = proposing = FALSE;
recordProposal(); setTcFlags();
agree = agree && betterorsameInfo();
recordPriority(); recordTimes(); updtRcvdInfoWhile();
infoIs = Received; reselect = TRUE; selected = FALSE;
rcvdMsg = FALSE;
*/
static int32
_msti_sm_PIM_SUPERIOR_DESIGNATED(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    tbl_mstp_port_t *p_mstp_port = (tbl_mstp_port_t*)p_db_port->p_port;
    tbl_mstp_instance_key_t inst_key;
    tbl_mstp_instance_t *p_db_inst = NULL;
    
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PIM] %s enter SUPERIOR_DESIGNATED", msti_port_name);

    /* modified by liwh for bug 36699/37625/37767, 2016-05-11 */
    if ((p_mstp_port->admin_rootguard))
    {
        p_db_port->rcvdMsg = FALSE;
        p_db_port->oper_rootguard = TRUE;
        msti_sm_PRT_DESIGNATED_DISCARD(p_db_port, msti_port_name);

        _msti_sm_PIM_CURRENT(p_db_port, msti_port_name);

        return PM_E_NONE;
    }
    /* liwh end */
    
    p_db_port->agreed = p_db_port->proposing = FALSE;
        
    msti_recordProposal(p_db_port);
    msti_setTcFlags(p_db_port);
    p_db_port->agree = p_db_port->agree && msti_betterOrSameInfo(p_db_port, MSTP_INFO_RECEIVED);
    
    msti_recordPriority(p_db_port);
    msti_recordHopCount(p_db_port);
    msti_recordTimes(p_db_port); 
    msti_updtRcvdInfoWhile(p_db_port);
    
    p_db_port->infoIs = MSTP_INFO_RECEIVED; 
    p_db_port->reselect = TRUE; 
    p_db_port->selected = FALSE;
    
    p_db_port->rcvdMsg = FALSE;
    p_db_port->pim_state = PIM_SUPERIOR_DESIGNATED;

    _msti_sm_PIM_CURRENT(p_db_port, msti_port_name);

    inst_key.instance = p_db_port->key.instance;
    p_db_inst = tbl_mstp_instance_get_mstp_instance(&inst_key);
    if (p_db_inst)
    {
        mstp_api_msti_reselect(p_db_inst);
    }
    
    return PM_E_NONE;
}

/* Added by kcao for bug 42217, check  */
int32
msti_sm_check_hops(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    tbl_mstp_port_t *p_mstp_port = (tbl_mstp_port_t*)p_db_port->p_port;
    bpdu_t *p_bpdu = (bpdu_t*)p_mstp_port->bpdu;
    msti_bpdu_t *p_msti_bpdu = (msti_bpdu_t*)p_db_port->bpdu;
    
    /* Max-hops is in same MST region */
    if (p_mstp_port->rcvdInternal)
    {
        if (p_bpdu && p_msti_bpdu)
        {
            if ((BPDU_VERSION_MSTP == p_bpdu->version) && (0 == p_msti_bpdu->hops))
            {
                p_db_port->rcvdMsg = FALSE;
                _msti_sm_PIM_CURRENT(p_db_port, msti_port_name);
                return PM_E_FAIL;
            }
        }
    }

    return PM_E_NONE;
}

/*
rcvdInfo = rcvInfo();
*/
static int32
_msti_sm_PIM_RECEIVE(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    int32 rc = PM_E_NONE;

    p_db_port->rcvdInfo = msti_rcvInfo(p_db_port);
    p_db_port->pim_state = PIM_RECEIVE;

    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PIM] %s enter PIM_RECEIVE, rcvdInfo %u", msti_port_name, p_db_port->rcvdInfo);
    
    switch (p_db_port->rcvdInfo)
    {
    case MSTP_SUPERIOR_DESIGNATED:
        rc = msti_sm_check_hops(p_db_port, msti_port_name);
        if (rc < 0)
        {
            return PM_E_NONE;
        }
#ifdef MSTP_TODO
        /* root guard check */
        if (p_db_port->rootGuard &&       /* not backup port */
            rstpPort.bpdu.compare_bridge_mac(_rstp_bridge.get_bridge_identifier().get_mac_addr()) == false){
            PimRootGuardAction(rstpPort);
        }
        else
#endif
        {
            _msti_sm_PIM_SUPERIOR_DESIGNATED(p_db_port, msti_port_name);
        }
        break;
    case MSTP_REPEATED_DESIGNATED:
        rc = msti_sm_check_hops(p_db_port, msti_port_name);
        if (rc < 0)
        {
            return PM_E_NONE;
        }
        _msti_sm_PIM_REPEATED_DESIGNATED(p_db_port, msti_port_name);
        break;
    case MSTP_INFERIOR_DESIGNATED:
        _msti_sm_PIM_INFERIOR_DESIGNATED(p_db_port, msti_port_name);
        break;
    case MSTP_INFERIOR_ROOTALTERNATE:
        _msti_sm_PIM_NOT_DESIGNATED(p_db_port, msti_port_name);
        break;
    case MSTP_OTHER:
        _msti_sm_PIM_OTHER(p_db_port, msti_port_name);
        break;

    default:
        break;
    }
    
    return PM_E_NONE;
}

/*
  Figure 17-18  Port Information state machine
*/
int32
msti_sm_pim_process_event(tbl_msti_port_t *p_db_port, pim_event_t event)
{
    pim_state_t next_state = 0;
    char msti_port_name[IFNAME_EXT_SIZE];

    if (event >= PIM_EVENT_MAX)
    {
        return PM_E_FAIL;
    }

    MSTP_MSTI_PORT_STR(msti_port_name, p_db_port);
    next_state =  mstp_sm_pim_state_table[event][p_db_port->pim_state];
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti %s] msti_port %s recv event: %s, state: %s -> %s", 
            "PIM",
            msti_port_name,
            cdb_enum_val2str(pim_event_strs, PIM_EVENT_MAX, event),
            cdb_enum_val2str(pim_state_strs, PIM_STATE_MAX, p_db_port->pim_state),
            cdb_enum_val2str(pim_state_strs, PIM_STATE_MAX, next_state));

    switch (next_state)
    {
    case PIM_DISABLED:
        _msti_sm_PIM_DISABLED(p_db_port, msti_port_name);
        break;
    case PIM_AGED:
        _msti_sm_PIM_AGED(p_db_port, msti_port_name);
        break;
    case PIM_UPDATE:
        _msti_sm_PIM_UPDATE(p_db_port, msti_port_name);
        break;
    case PIM_CURRENT:
        _msti_sm_PIM_CURRENT(p_db_port, msti_port_name);
        break;
    /* below 5 states are direct transferred from PIM_RECEIVE */
#if 0
    case PIM_OTHER:
        _msti_sm_PIM_OTHER(p_db_port, msti_port_name);
        break;
    case PIM_NOT_DESIGNATED:
        _msti_sm_PIM_NOT_DESIGNATED(p_db_port, msti_port_name);
        break;
    case PIM_INFERIOR_DESIGNATED:
        _msti_sm_PIM_INFERIOR_DESIGNATED(p_db_port, msti_port_name);
        break;
    case PIM_REPEATED_DESIGNATED:
        _msti_sm_PIM_REPEATED_DESIGNATED(p_db_port, msti_port_name);
        break;
    case PIM_SUPERIOR_DESIGNATED:
        _msti_sm_PIM_SUPERIOR_DESIGNATED(p_db_port, msti_port_name);
        break;
#endif
    case PIM_RECEIVE:
        _msti_sm_PIM_RECEIVE(p_db_port, msti_port_name);
        break;
    default:
        break;
    }

    _msti_sm_PIM_TRIGGER_EVENT(p_db_port);

    return PM_E_NONE;
}

#define _17_29__13_34__PRT_Port_Role_Transitions_Machine_

/* PRT State (Port Role Transitions)
   802.1D.2004  P.174  17.29 Port Role Transitions state machine
   802.1Q.2005  P.183  13.34 Port Role Transitions state machine
*/
extern uint32 mstp_sm_prt_state_table[PRT_EVENT_MAX][PRT_STATE_MAX];

/*
role =DisabledPort;
learn= forward = FALSE;
synced = FALSE;
sync = reRoot = TRUE;
rrWhile = FwdDelay;
fdWhile = MaxAge;
rbWhile = 0;
*/
static int32
_msti_sm_PRT_INIT_PORT(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PRT] %s enter INIT_PORT", msti_port_name);

    p_db_port->role = ROLE_DISABLED;
    // learn= forward = FALSE;
    p_db_port->synced = FALSE;
    p_db_port->sync = p_db_port->reRoot = TRUE;
    // rrWhile = FwdDelay;
    // fdWhile = MaxAge;
    // rbWhile = 0;

    p_db_port->prt_state = PRT_INIT_PORT;
    
    return PM_E_NONE;
}

/*
DISABLE_PORT
role = selectedRole;
learn= forward = FALSE;

DISABLED_PORT
fdWhile = MaxAge;
synced = TRUE; rrWhile = 0;
sync = reRoot = FALSE;
*/
static int32
_msti_sm_PRT_DISABLED_PORT(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PRT] %s enter DISABLED_PORT", msti_port_name);

    /* DISABLE_PORT */
    p_db_port->role = ROLE_DISABLED;
    // learn= forward = FALSE;

    /* DISABLED_PORT */
    // fdWhile = MaxAge;
    p_db_port->synced = TRUE;
    // rrWhile = 0;
    p_db_port->sync = p_db_port->reRoot = FALSE;

    p_db_port->prt_state = PRT_DISABLED_PORT;
    
    return PM_E_NONE;
}

/*
17.20.3 allSynced
TRUE if and only if, for all Ports for the given Tree, selected is true and the port¡¯s role is the same as its
selectedRole and either
a) synced is true; or
b) The port is the Root Port.
*/
int32
msti_isAllSynced(tbl_mstp_instance_t *p_db_inst)
{
    tbl_mstp_port_t *p_mstp_port = NULL;
    tbl_msti_port_t *p_db_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    
    CTCLIB_SLIST_LOOP(p_db_inst->port_list.obj_list, p_db_port, listnode)
    {
        p_mstp_port = (tbl_mstp_port_t*)p_db_port->p_port;
        if (p_mstp_port->portEnabled)
        {
            if (ROLE_ROOTPORT == p_db_port->role)
            {
                continue;
            }

            if (!p_db_port->synced)
            {
                return FALSE;
            }
        }
    }

    return TRUE;
}

int32
msti_setAgree(tbl_mstp_instance_t *p_db_inst, int32 allSynced)
{
    tbl_mstp_port_t *p_mstp_port = NULL;
    tbl_msti_port_t *p_db_port = NULL;
    ctclib_slistnode_t *listnode = NULL;

    if (!allSynced)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP(p_db_inst->port_list.obj_list, p_db_port, listnode)
    {
        p_mstp_port = (tbl_mstp_port_t*)p_db_port->p_port;
        if (p_mstp_port->portEnabled)
        {
            if (ROLE_ROOTPORT == p_db_port->role)
            {
                if (!p_db_port->agree)
                {
                    msti_sm_prt_process_event(p_db_port, PRT_EVT_ALLSYNCED_NOTAGREE);
                }
            }
            else if (ROLE_ALTERNATE == p_db_port->role)
            {
                if (!p_db_port->agree)
                {
                    msti_sm_prt_process_event(p_db_port, PRT_EVT_ALLSYNCED_NOTAGREE);
                }
            }
        }
    }

    return TRUE;
}

int32
msti_updateAllSynced(tbl_mstp_instance_t *p_db_inst)
{
    int32 allSynced = FALSE;
    
    allSynced = msti_isAllSynced(p_db_inst);
    msti_setAgree(p_db_inst, allSynced);

    return PM_E_NONE;
}

/*
17.21.14 setSyncTree()
Sets sync TRUE for all Ports of the Bridge.
*/
int32
msti_setSyncTree(uint16 instance)
{
    tbl_mstp_port_t *p_mstp_port = NULL;
    tbl_mstp_instance_key_t inst_key;
    tbl_mstp_instance_t *p_db_inst = NULL;
    tbl_msti_port_t *p_db_port = NULL;
    ctclib_slistnode_t *listnode = NULL;

    inst_key.instance = instance;
    p_db_inst = tbl_mstp_instance_get_mstp_instance(&inst_key);
    if (NULL == p_db_inst)
    {
        return PM_E_FAIL;
    }
    
    CTCLIB_SLIST_LOOP(p_db_inst->port_list.obj_list, p_db_port, listnode)
    {
        p_mstp_port = (tbl_mstp_port_t*)p_db_port->p_port;
        if (p_mstp_port->portEnabled)
        {
            p_db_port->sync = TRUE;
            p_db_port->synced = FALSE;
            p_db_port->agreed = FALSE;

            switch (p_db_port->role)
            {
            case ROLE_ALTERNATE:
                p_db_port->synced = TRUE;
                p_db_port->sync = FALSE;
                break;

            case ROLE_BACKUP:
                p_db_port->synced = TRUE;
                p_db_port->sync = FALSE;
                break;
    
            case ROLE_DISABLED:
                p_db_port->synced = TRUE;
                p_db_port->sync = FALSE;
                break;

            case ROLE_DESIGNATED:
                /* ((sync && !synced) || (reRoot && (rrWhile != 0)) || disputed) && !operEdge && (learn || forward) */
                if (((p_db_port->sync && !p_db_port->synced) || (p_db_port->reRoot && p_db_port->rrWhile_timer) || p_db_port->disputed)
                 && (!p_db_port->rrWhile_timer || !p_db_port->reRoot)
                 && !p_mstp_port->operEdge
                 && (p_db_port->learn || p_db_port->forward))
                {
                    msti_sm_prt_process_event(p_db_port, PRT_EVT_DESIGNATEDDISCARD);
                }
                
                /*
                    (!learning && !forwarding && !synced) ||
                    (agreed && !synced) ||
                    (operEdge && !synced) ||
                    (sync && synced)
                 */
                if (!p_db_port->learning && !p_db_port->forwarding && !p_db_port->synced)
                {
                    p_db_port->synced = TRUE;
                    p_db_port->sync = FALSE;
                }
                else if (p_db_port->agreed && !p_db_port->synced)
                {
                    p_db_port->synced = TRUE;
                    p_db_port->sync = FALSE;
                }
                else if (p_mstp_port->operEdge && !p_db_port->synced)
                {
                    p_db_port->synced = TRUE;
                    p_db_port->sync = FALSE;
                }
                else if (p_db_port->sync && p_db_port->synced)
                {
                    p_db_port->synced = TRUE;
                    p_db_port->sync = FALSE;
                }
                break;

            default:
                break;
            }
        }
    }

    msti_updateAllSynced(p_db_inst);
    
    return PM_E_NONE;
}

/*
17.21.15 setReRootTree()
Sets reRoot TRUE for all Ports of the Bridge.
*/
int32
msti_setReRootTree(tbl_mstp_instance_t *p_db_inst)
{
    tbl_mstp_port_t *p_mstp_port = NULL;
    tbl_msti_port_t *p_db_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    
    CTCLIB_SLIST_LOOP(p_db_inst->port_list.obj_list, p_db_port, listnode)
    {
        p_mstp_port = (tbl_mstp_port_t*)p_db_port->p_port;
        if (p_mstp_port->portEnabled)
        {
            p_db_port->reRoot = TRUE;
            break;
        }
    }

    return PM_E_NONE;
}

/*
setSyncTree();
proposed = FALSE;
*/
static int32
_msti_sm_PRT_ROOT_PROPOSED(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PRT] %s enter ROOT_PROPOSED", msti_port_name);

    msti_setSyncTree(p_db_port->key.instance);
    p_db_port->proposed = FALSE;

    p_db_port->prt_state = PRT_ROOT_PROPOSED;
    return PM_E_NONE;
}

/*
proposed = sync = FALSE;
agree = TRUE;
newInfo = TRUE;
*/
static int32
_msti_sm_PRT_ROOT_AGREED(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    tbl_mstp_port_t *p_mstp_port = (tbl_mstp_port_t*)p_db_port->p_port;
    
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PRT] %s enter ROOT_AGREED", msti_port_name);
    
    p_db_port->proposed = p_db_port->sync = FALSE;
    p_db_port->agree = TRUE;
    p_db_port->newInfo = TRUE;

    p_db_port->prt_state = PRT_ROOT_AGREED;
    
    if (p_mstp_port->sendRstp)
    {
        mstp_sm_send_bpdu(p_mstp_port);
    }

    return PM_E_NONE;
}

/*
setReRootTree();
*/
static int32
_msti_sm_PRT_REROOT(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    tbl_mstp_instance_key_t inst_key;
    tbl_mstp_instance_t *p_db_inst = NULL;
    
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PRT] %s enter REROOT", msti_port_name);

    inst_key.instance = p_db_port->key.instance;
    p_db_inst = tbl_mstp_instance_get_mstp_instance(&inst_key);
    if (p_db_inst)
    {
        msti_setReRootTree(p_db_inst);
    }

    p_db_port->prt_state = PRT_REROOT;
    
    return PM_E_NONE;
}

void
_msti_sm_delay_rootLearnForward_timer_fn(void *p_data)
{
    tbl_msti_port_key_t *p_port_key = (tbl_msti_port_key_t*)p_data;
    tbl_msti_port_t *p_db_port = NULL;

    p_db_port = tbl_msti_port_get_msti_port(p_port_key);
    XFREE(MEM_PM_MSTP_TMP, p_port_key);
    if (NULL == p_db_port)
    {
        return;
    }

    if (ROLE_ROOTPORT == p_db_port->role)
    {
        if (!p_db_port->learn)
        {
            msti_sm_prt_process_event(p_db_port, PRT_EVT_ROOTLEARN);
        }
        
        if (!p_db_port->forward)
        {
            msti_sm_prt_process_event(p_db_port, PRT_EVT_ROOTFORWARD);
        }
    }
    else if (ROLE_MASTERPORT == p_db_port->role)
    {
        if (!p_db_port->learn)
        {
            msti_sm_prt_process_event(p_db_port, PRT_EVT_MASTERLEARN);
        }
        
        if (!p_db_port->forward)
        {
            msti_sm_prt_process_event(p_db_port, PRT_EVT_MASTERFORWARD);
        }
    }

    return;
}

void
_msti_sm_rrWhile_timer_fn(void *p_data)
{
    tbl_msti_port_t *p_db_port = (tbl_msti_port_t*)p_data;

    p_db_port->rrWhile_timer = NULL;

    return;
}

/*
role = RootPort;
rrWhile = FwdDelay;
*/
static int32
_msti_sm_PRT_ROOT_PORT(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    tbl_mstp_global_t *p_mstp_glb = tbl_mstp_global_get_mstp_global();
    uint8 old_role = 0;
    tbl_msti_port_key_t *p_port_key = NULL;

    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PRT] %s enter ROOT_PORT", msti_port_name);

    /* modified by liwh for bug 37514, 2016-05-17 */
    if (p_db_port->restricted_role)
    {
        _msti_sm_PRT_BLOCK_PORT(p_db_port, msti_port_name);
        _msti_sm_PRT_ALTERNATE_PORT(p_db_port, msti_port_name);

        return PM_E_NONE;
    }
    /* liwh end */

    old_role = p_db_port->role;
    p_db_port->role = ROLE_ROOTPORT;

    CTC_TASK_STOP_TIMER(p_db_port->rrWhile_timer);
    p_db_port->rrWhile_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _msti_sm_rrWhile_timer_fn, p_db_port, 
        p_db_port->port_fwdDelay);

    p_db_port->prt_state = PRT_ROOT_PORT;

    if (ROLE_ALTERNATE == old_role || ROLE_DESIGNATED == old_role)
    {
        if (MSTP_TYPE_STP != p_mstp_glb->type)
        {
            /* Modified by kcao for bug 39730, alternate -> rootport should switch to forward after a short timer,
             * otherwise, will result in instant loop
             */
            p_port_key = XCALLOC(MEM_PM_MSTP_TMP, sizeof(tbl_msti_port_key_t));
            if (p_port_key)
            {
                sal_memcpy(p_port_key, &p_db_port->key, sizeof(tbl_msti_port_key_t));
                ctc_task_add_timer(TASK_PRI_NORMAL, _msti_sm_delay_rootLearnForward_timer_fn, p_port_key, 0);
            }
        }
    }

    return PM_E_NONE;
}

/*
reRoot = FALSE;
*/
static int32
_msti_sm_PRT_REROOTED(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PRT] %s enter REROOTED", msti_port_name);
    
    p_db_port->reRoot = FALSE;
    
    p_db_port->prt_state = PRT_REROOT;
    return PM_E_NONE;
}

static void
_msti_sm_fdWhile_timer_fn(void *p_data)
{
    tbl_msti_port_t *p_db_port = (tbl_msti_port_t*)p_data;
    char msti_port_name[IFNAME_EXT_SIZE];

    MSTP_MSTI_PORT_STR(msti_port_name, p_db_port);
    MSTP_DEBUG(STP_TIMER, "msti Timer fdWhile timeout on interface %s", msti_port_name);

    p_db_port->fdWhile_timer = NULL;

    /* modified by liwh for bug 36699/37625/37767, 2016-05-11 */
    if (p_db_port->oper_rootguard)
    {
        p_db_port->oper_rootguard = FALSE;
        p_db_port->prt_state = PRT_DESIGNATED_PORT;
    }
    /* liwh end */

    /* modified by liwh for bug 37762, 2016-06-21 
       when loop gurad, port role should be designated, state should be discarding */
    if (p_db_port->oper_loopguard)
    {
        return;    
    }
    /* liwn end */

    msti_sm_PRT_TRIGGER_EVENT(p_db_port);

    return;
}

/*
fdWhile= forwardDelay;
learn = TRUE;
*/
static int32
_msti_sm_PRT_ROOT_LEARN(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    tbl_mstp_port_t *p_mstp_port = (tbl_mstp_port_t*)p_db_port->p_port;
    uint32 forwardDelay = 0;

    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PRT] %s enter ROOT_LEARN", msti_port_name);
    
    /* 
    17.20.5 forwardDelay
    Returns the value of HelloTime if sendRSTP is TRUE, and the value of FwdDelay otherwise.
    */
    forwardDelay = (p_mstp_port->sendRstp) ? p_db_port->port_helloTime : p_db_port->port_fwdDelay;
    CTC_TASK_STOP_TIMER(p_db_port->fdWhile_timer);
    p_db_port->fdWhile_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _msti_sm_fdWhile_timer_fn, p_db_port, 
        forwardDelay);

    if (!p_db_port->learn)
    {
        msti_sm_pst_process_event(p_db_port, PST_EVT_LEARN);
    }
    p_db_port->learn = TRUE;
    if (TCM_INACTIVE == p_db_port->tcm_state && !p_db_port->fdbFlush)
    {
        msti_sm_tcm_process_event(p_db_port, TCM_EVT_LEARNNOTFDBFLUSH);
    }
    p_db_port->sync = FALSE;    /* when port enter LEARN state, should clear sync */

    p_db_port->prt_state = PRT_ROOT_LEARN;
    return PM_E_NONE;
}

/*
fdWhile = 0;
forward = TRUE;
*/
static int32
_msti_sm_PRT_ROOT_FORWARD(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    tbl_mstp_port_t *p_mstp_port = NULL;
    p_mstp_port = (tbl_mstp_port_t*)p_db_port->p_port;

    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PRT] %s enter ROOT_FORWARD", msti_port_name);
    
    CTC_TASK_STOP_TIMER(p_db_port->fdWhile_timer);
    
    if (!p_db_port->forward)
    {
        msti_sm_pst_process_event(p_db_port, PST_EVT_FORWARD);
    }
    p_db_port->forward = TRUE;
    if (!p_mstp_port->operEdge)
    {
        msti_sm_tcm_process_event(p_db_port, TCM_EVT_FORWARDNOTOPEREDGE);
    }

    p_db_port->prt_state = PRT_ROOT_FORWARD;
    return PM_E_NONE;
}

/*
setSyncTree();
proposed = FALSE;
*/
static int32
_msti_sm_PRT_MASTER_PROPOSED(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PRT] %s enter MASTER_PROPOSED", msti_port_name);

    msti_setSyncTree(p_db_port->key.instance);
    p_db_port->proposed = FALSE;

    p_db_port->prt_state = PRT_MASTER_PROPOSED;
    return PM_E_NONE;
}

/*
proposed = sync = FALSE;
agree = TRUE;
*/
static int32
_msti_sm_PRT_MASTER_AGREED(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    tbl_mstp_port_t *p_mstp_port = (tbl_mstp_port_t*)p_db_port->p_port;
    
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PRT] %s enter MASTER_AGREED", msti_port_name);
    
    p_db_port->proposed = p_db_port->sync = FALSE;
    p_db_port->agree = TRUE;

    p_db_port->prt_state = PRT_MASTER_AGREED;
    
    if (p_mstp_port->sendRstp)
    {
        mstp_sm_send_bpdu(p_mstp_port);
    }

    return PM_E_NONE;
}

/*
rrWhile = 0;
synced = TRUE;
sync = FALSE;
*/
static int32
_msti_sm_PRT_MASTER_SYNCED(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PRT] %s enter MASTER_SYNCED", msti_port_name);

    p_db_port->synced = TRUE;
    p_db_port->sync = FALSE;

    p_db_port->prt_state = PRT_MASTER_SYNCED;    
    return PM_E_NONE;
}

/*
reRoot = FALSE;
*/
static int32
_msti_sm_PRT_MASTER_RETIRED(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PRT] %s enter MASTER_RETIRED", msti_port_name);

    p_db_port->reRoot = FALSE;

    p_db_port->prt_state = PRT_MASTER_RETIRED;    
    return PM_E_NONE;
}

/*
fdWhile = 0;
forward = TRUE;
*/
static int32
_msti_sm_PRT_MASTER_FORWARD(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    tbl_mstp_port_t *p_mstp_port = (tbl_mstp_port_t*)p_db_port->p_port;
    
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PRT] %s enter MASTER_FORWARD", msti_port_name);
    
    CTC_TASK_STOP_TIMER(p_db_port->fdWhile_timer);
    
    if (!p_db_port->forward)
    {
        msti_sm_pst_process_event(p_db_port, PST_EVT_FORWARD);
    }
    p_db_port->forward = TRUE;
    if (!p_mstp_port->operEdge)
    {
        msti_sm_tcm_process_event(p_db_port, TCM_EVT_FORWARDNOTOPEREDGE);
    }

    p_db_port->prt_state = PRT_MASTER_FORWARD;
    return PM_E_NONE;
}

/*
fdWhile= forwardDelay;
learn = TRUE;
*/
static int32
_msti_sm_PRT_MASTER_LEARN(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    tbl_mstp_port_t *p_mstp_port = (tbl_mstp_port_t*)p_db_port->p_port;
    uint32 forwardDelay = 0;

    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PRT] %s enter MASTER_LEARN", msti_port_name);
    
    /* 
    17.20.5 forwardDelay
    Returns the value of HelloTime if sendRSTP is TRUE, and the value of FwdDelay otherwise.
    */
    forwardDelay = (p_mstp_port->sendRstp) ? p_mstp_port->port_helloTime : p_mstp_port->port_fwdDelay;
    CTC_TASK_STOP_TIMER(p_db_port->fdWhile_timer);
    p_db_port->fdWhile_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _msti_sm_fdWhile_timer_fn, p_db_port, 
        forwardDelay);

    if (!p_db_port->learn)
    {
        msti_sm_pst_process_event(p_db_port, PST_EVT_LEARN);
    }
    p_db_port->learn = TRUE;
    if (TCM_INACTIVE == p_db_port->tcm_state && !p_db_port->fdbFlush)
    {
        msti_sm_tcm_process_event(p_db_port, TCM_EVT_LEARNNOTFDBFLUSH);
    }
    
    p_db_port->prt_state = PRT_MASTER_LEARN;
    return PM_E_NONE;
}

/*
learn = forward = disputed = FALSE;
fdWhile= forwardDelay;
*/
int32
_msti_sm_PRT_MASTER_DISCARD(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    tbl_mstp_port_t *p_mstp_port = (tbl_mstp_port_t*)p_db_port->p_port;
    uint32 forwardDelay = 0;
    
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PRT] %s enter MASTER_DISCARD", msti_port_name);
    /* 
    17.20.5 forwardDelay
    Returns the value of HelloTime if sendRSTP is TRUE, and the value of FwdDelay otherwise.
    */
    forwardDelay = (p_mstp_port->sendRstp) ? p_mstp_port->port_helloTime : p_mstp_port->port_fwdDelay;

    if (p_db_port->forward)
    {
        msti_sm_pst_process_event(p_db_port, PST_EVT_NOTFORWARD);
    }
    if (p_db_port->learn)
    {
        msti_sm_pst_process_event(p_db_port, PST_EVT_NOTLEARN);
    }
    p_db_port->learn = FALSE;
    p_db_port->forward = FALSE;
    p_db_port->disputed = FALSE;
    CTC_TASK_STOP_TIMER(p_db_port->fdWhile_timer);
    p_db_port->fdWhile_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _msti_sm_fdWhile_timer_fn, p_db_port, 
        forwardDelay);

    p_db_port->prt_state = PRT_MASTER_DISCARD;
    
    mstp_api_flush_fdb(p_mstp_port);

    return PM_E_NONE;
}

/*
role = RootPort;
rrWhile = FwdDelay;
*/
static int32
_msti_sm_PRT_MASTER_PORT(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    tbl_mstp_global_t *p_mstp_glb = tbl_mstp_global_get_mstp_global();
    uint8 old_role = 0;
    tbl_msti_port_key_t *p_port_key = NULL;

    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PRT] %s enter MASTER_PORT", msti_port_name);

    /* modified by liwh for bug 37514, 2016-05-17 */
    if (p_db_port->restricted_role)
    {
        _msti_sm_PRT_BLOCK_PORT(p_db_port, msti_port_name);
        _msti_sm_PRT_ALTERNATE_PORT(p_db_port, msti_port_name);
        return PM_E_NONE;
    }
    /* liwh end */

    old_role = p_db_port->role;
    p_db_port->role = ROLE_MASTERPORT;
   
    CTC_TASK_STOP_TIMER(p_db_port->rrWhile_timer);
    p_db_port->rrWhile_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _msti_sm_rrWhile_timer_fn, p_db_port, 
        p_db_port->port_fwdDelay);

    p_db_port->prt_state = PRT_MASTER_PORT;

    if (ROLE_ALTERNATE == old_role || ROLE_DESIGNATED == old_role)
    {
        if (MSTP_TYPE_STP != p_mstp_glb->type)
        {
            /* Modified by kcao for bug 39730, alternate -> rootport should switch to forward after a short timer,
             * otherwise, will result in instant loop
             */
            p_port_key = XCALLOC(MEM_PM_MSTP_TMP, sizeof(tbl_msti_port_key_t));
            if (p_port_key)
            {
                sal_memcpy(p_port_key, &p_db_port->key, sizeof(tbl_msti_port_key_t));
                ctc_task_add_timer(TASK_PRI_NORMAL, _msti_sm_delay_rootLearnForward_timer_fn, p_port_key, 0);
            }
        }
        else
        {
        }
    }

    return PM_E_NONE;
}

/*
proposing = TRUE;
edgeDelayWhile = EdgeDelay;
newInfo = TRUE;
*/
static int32
_msti_sm_PRT_DESIGNATED_PROPOSE(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    tbl_mstp_port_t *p_mstp_port = (tbl_mstp_port_t*)p_db_port->p_port;
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PRT] %s enter DESIGNATED_PROPOSE", msti_port_name);

    p_db_port->proposing = TRUE;
    p_db_port->newInfo = TRUE;
    p_db_port->prt_state = PRT_DESIGNATED_PROPOSE;

    /* send propose immediately */
    if (p_mstp_port->sendRstp)
    {
        mstp_sm_send_bpdu(p_mstp_port);
    }

    return PM_E_NONE;
}

/*
rrWhile = 0; synced = TRUE;
sync = FALSE;
*/
static int32
_msti_sm_PRT_DESIGNATED_SYNCED(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PRT] %s enter DESIGNATED_SYNCED", msti_port_name);
    
    CTC_TASK_STOP_TIMER(p_db_port->rrWhile_timer);
    p_db_port->synced = TRUE;
    p_db_port->sync = FALSE;

    p_db_port->prt_state = PRT_DESIGNATED_SYNCED;
    return PM_E_NONE;
}

/*
reRoot = FALSE;
*/
static int32
_msti_sm_PRT_DESIGNATED_RETIRED(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PRT] %s enter DESIGNATED_RETIRED", msti_port_name);
    
    p_db_port->reRoot = FALSE;

    p_db_port->prt_state = PRT_DESIGNATED_RETIRED;
    return PM_E_NONE;
}

/*
role = DesignatedPort;
*/
static int32
_msti_sm_PRT_DESIGNATED_PORT(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PRT] %s enter DESIGNATED_PORT", msti_port_name);
    p_db_port->role = ROLE_DESIGNATED;
    p_db_port->prt_state = PRT_DESIGNATED_PORT;
    
    return PM_E_NONE;
}

/*
learn = forward = disputed = FALSE;
fdWhile= forwardDelay;
*/
int32
msti_sm_PRT_DESIGNATED_DISCARD(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    tbl_mstp_port_t *p_mstp_port = (tbl_mstp_port_t*)p_db_port->p_port;
    uint32 forwardDelay = 0;
    
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PRT] %s enter DESIGNATED_DISCARD", msti_port_name);
    /* 
    17.20.5 forwardDelay
    Returns the value of HelloTime if sendRSTP is TRUE, and the value of FwdDelay otherwise.
    */
    forwardDelay = (p_mstp_port->sendRstp) ? p_db_port->port_helloTime : p_db_port->port_fwdDelay;

    if (p_db_port->forward)
    {
        msti_sm_pst_process_event(p_db_port, PST_EVT_NOTFORWARD);
    }
    if (p_db_port->learn)
    {
        msti_sm_pst_process_event(p_db_port, PST_EVT_NOTLEARN);
    }
    p_db_port->learn = FALSE;
    p_db_port->forward = FALSE;
    p_db_port->disputed = FALSE;
    CTC_TASK_STOP_TIMER(p_db_port->fdWhile_timer);
    p_db_port->fdWhile_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _msti_sm_fdWhile_timer_fn, p_db_port, 
        forwardDelay);

    p_db_port->prt_state = PRT_DESIGNATED_DISCARD;
    
    mstp_api_flush_fdb(p_mstp_port);

    return PM_E_NONE;
}

/*
learn = TRUE;
fdWhile= forwardDelay;
*/
static int32
_msti_sm_PRT_DESIGNATED_LEARN(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    tbl_mstp_port_t *p_mstp_port = (tbl_mstp_port_t*)p_db_port->p_port;
    uint32 forwardDelay = 0;

    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PRT] %s enter DESIGNATED_LEARN", msti_port_name);
    
    /* 
    17.20.5 forwardDelay
    Returns the value of HelloTime if sendRSTP is TRUE, and the value of FwdDelay otherwise.
    */
    forwardDelay = (p_mstp_port->sendRstp) ? p_db_port->port_helloTime : p_db_port->port_fwdDelay;

    if (!p_db_port->learn)
    {
        msti_sm_pst_process_event(p_db_port, PST_EVT_LEARN);
    }
    p_db_port->learn = TRUE;
    if (TCM_INACTIVE == p_db_port->tcm_state && !p_db_port->fdbFlush)
    {
        msti_sm_tcm_process_event(p_db_port, TCM_EVT_LEARNNOTFDBFLUSH);
    }
    
    CTC_TASK_STOP_TIMER(p_db_port->fdWhile_timer);
    p_db_port->fdWhile_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _msti_sm_fdWhile_timer_fn, p_db_port, 
        forwardDelay);

    p_db_port->prt_state = PRT_DESIGNATED_LEARN;
    return PM_E_NONE;
}

/*
forward = TRUE; fdWhile = 0;
agreed = sendRstp;
*/
static int32
_msti_sm_PRT_DESIGNATED_FORWARD(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    tbl_mstp_port_t *p_mstp_port = (tbl_mstp_port_t*)p_db_port->p_port;
    
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PRT] %s enter DESIGNATED_FORWARD", msti_port_name);
    
    if (!p_db_port->forward)
    {
        /* if port to forward, clear proposing */
        p_db_port->proposing = FALSE;
        msti_sm_pst_process_event(p_db_port, PST_EVT_FORWARD);
    }
    p_db_port->forward = TRUE;
    if (!p_mstp_port->operEdge)
    {
        msti_sm_tcm_process_event(p_db_port, TCM_EVT_FORWARDNOTOPEREDGE);
    }

    CTC_TASK_STOP_TIMER(p_db_port->fdWhile_timer);
    p_db_port->agreed = p_mstp_port->sendRstp;

    p_db_port->prt_state = PRT_DESIGNATED_FORWARD;

    return PM_E_NONE;
}

/*
setSyncTree();
proposed = FALSE;
*/
static int32
_msti_sm_PRT_ALTERNATE_PROPOSED(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PRT] %s enter ALTERNATE_PROPOSED", msti_port_name);

    /* For switch eth-0-1 <--> eth-0-2 loopback, the Designated port cannot enter forwarding state, and switch between discard and learning */
    if (ROLE_BACKUP != p_db_port->role)
    {
        msti_setSyncTree(p_db_port->key.instance);
        p_db_port->proposed = FALSE;
    }

    p_db_port->prt_state = PRT_ALTERNATE_PROPOSED;
    return PM_E_NONE;
}

/*
proposed = FALSE;
agree = TRUE;
newInfo = TRUE;
*/
static int32
_msti_sm_PRT_ALTERNATE_AGREED(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    tbl_mstp_port_t *p_mstp_port = (tbl_mstp_port_t*)p_db_port->p_port;

    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PRT] %s enter ALTERNATE_AGREED", msti_port_name);
    
    p_db_port->proposed = FALSE;
    p_db_port->agree = TRUE;
    p_db_port->newInfo = TRUE;

    p_db_port->prt_state = PRT_ALTERNATE_AGREED;

    if (p_mstp_port->sendRstp)
    {
        mstp_sm_send_bpdu(p_mstp_port);
    }

    return PM_E_NONE;
}

/*
fdWhile = FwdDelay;synced = TRUE; rrWhile = 0; sync = reRoot = FALSE;
*/
static int32
_msti_sm_PRT_ALTERNATE_PORT(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PRT] %s enter ALTERNATE_PORT", msti_port_name);
    
    CTC_TASK_STOP_TIMER(p_db_port->fdWhile_timer);
    p_db_port->fdWhile_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _msti_sm_fdWhile_timer_fn, p_db_port, 
        p_db_port->port_fwdDelay);
    p_db_port->synced = TRUE;
    CTC_TASK_STOP_TIMER(p_db_port->rrWhile_timer);
    p_db_port->sync = FALSE;
    p_db_port->reRoot = FALSE;
    
    p_db_port->prt_state = PRT_ALTERNATE_PORT;
    return PM_E_NONE;
}

static void
_msti_sm_rbWhile_timer_fn(void *p_data)
{
    tbl_msti_port_t *p_db_port = (tbl_msti_port_t*)p_data;
    char msti_port_name[IFNAME_EXT_SIZE];

    MSTP_MSTI_PORT_STR(msti_port_name, p_db_port);
    MSTP_DEBUG(STP_TIMER, "msti Timer rbWhile timeout on interface %s", msti_port_name);

    p_db_port->rbWhile_timer = NULL;

    return;
}

/*
rbWhile = 2*HelloTime;
*/
static int32
_msti_sm_PRT_BACKUP_PORT(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PRT] %s enter BACKUP_PORT", msti_port_name);
    
    CTC_TASK_STOP_TIMER(p_db_port->rbWhile_timer);
    p_db_port->rbWhile_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _msti_sm_rbWhile_timer_fn, p_db_port, 
        2 * p_db_port->port_helloTime);

    p_db_port->prt_state = PRT_BACKUP_PORT;
    return PM_E_NONE;
}

/*
role = selectedRole;
learn = forward = FALSE;
*/
static int32
_msti_sm_PRT_BLOCK_PORT(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    tbl_mstp_port_t *p_mstp_port = (tbl_mstp_port_t*)p_db_port->p_port;

    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PRT] %s enter BLOCK_PORT", msti_port_name);
    
    if (p_db_port->role == p_db_port->selectedRole)
    {
        return PM_E_NONE;
    }

    if (p_db_port->forward)
    {
        msti_sm_pst_process_event(p_db_port, PST_EVT_NOTFORWARD);
    }
    if (p_db_port->learn)
    {
        msti_sm_pst_process_event(p_db_port, PST_EVT_NOTLEARN);
    }

    /* Added by kcao for stp backup port is stay in listening state */
    if (PST_LISTENING == p_db_port->pst_state)
    {
        p_db_port->pst_state = PST_DISCARDING;
    }
    
    p_db_port->role = p_db_port->selectedRole;
    p_db_port->learn = FALSE;
    p_db_port->forward = FALSE;

    p_db_port->prt_state = PRT_BLOCK_PORT;

    mstp_api_flush_fdb(p_mstp_port);

    return PM_E_NONE;
}


/*
  Figure 17-24  Port State Transition state machine
*/
int32
msti_sm_prt_process_event(tbl_msti_port_t *p_db_port, prt_event_t event)
{
    prt_state_t next_state = 0;
    char msti_port_name[IFNAME_EXT_SIZE];

    if (event >= PRT_EVENT_MAX)
    {
        return PM_E_FAIL;
    }

    MSTP_MSTI_PORT_STR(msti_port_name, p_db_port);
    next_state = mstp_sm_prt_state_table[event][p_db_port->prt_state];
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti %s] msti_port %s recv event: %s, state: %s -> %s", 
            "PRT",
            msti_port_name,
            cdb_enum_val2str(prt_event_strs, PRT_EVENT_MAX, event),
            cdb_enum_val2str(prt_state_strs, PRT_STATE_MAX, p_db_port->prt_state),
            cdb_enum_val2str(prt_state_strs, PRT_STATE_MAX, next_state));
   
    switch (next_state)
    {
    case PRT_INIT_PORT:
        _msti_sm_PRT_INIT_PORT(p_db_port, msti_port_name);
        _msti_sm_PRT_DISABLED_PORT(p_db_port, msti_port_name);
        break;
    case PRT_DISABLED_PORT:
        _msti_sm_PRT_DISABLED_PORT(p_db_port, msti_port_name);
        break;
    case PRT_ROOT_PROPOSED:
        _msti_sm_PRT_ROOT_PROPOSED(p_db_port, msti_port_name);
        _msti_sm_PRT_ROOT_PORT(p_db_port, msti_port_name);
        break;
    case PRT_ROOT_AGREED:
        _msti_sm_PRT_ROOT_AGREED(p_db_port, msti_port_name);
        _msti_sm_PRT_ROOT_PORT(p_db_port, msti_port_name);
        break;
    case PRT_REROOT:
        _msti_sm_PRT_REROOT(p_db_port, msti_port_name);
        _msti_sm_PRT_ROOT_PORT(p_db_port, msti_port_name);
        break;
    case PRT_ROOT_PORT:
        _msti_sm_PRT_ROOT_PORT(p_db_port, msti_port_name);
        break;
    case PRT_REROOTED:
        _msti_sm_PRT_REROOTED(p_db_port, msti_port_name);
        _msti_sm_PRT_ROOT_PORT(p_db_port, msti_port_name);
        break;
    case PRT_ROOT_LEARN:
        _msti_sm_PRT_ROOT_LEARN(p_db_port, msti_port_name);
        _msti_sm_PRT_ROOT_PORT(p_db_port, msti_port_name);
        break;
    case PRT_ROOT_FORWARD:
        _msti_sm_PRT_ROOT_FORWARD(p_db_port, msti_port_name);
        _msti_sm_PRT_ROOT_PORT(p_db_port, msti_port_name);
        break;
    case PRT_MASTER_PROPOSED:
        _msti_sm_PRT_MASTER_PROPOSED(p_db_port, msti_port_name);
        _msti_sm_PRT_MASTER_PORT(p_db_port, msti_port_name);
        break;
    case PRT_MASTER_AGREED:
        _msti_sm_PRT_MASTER_AGREED(p_db_port, msti_port_name);
        _msti_sm_PRT_MASTER_PORT(p_db_port, msti_port_name);
        break;
    case PRT_MASTER_SYNCED:
        _msti_sm_PRT_MASTER_SYNCED(p_db_port, msti_port_name);
        _msti_sm_PRT_MASTER_PORT(p_db_port, msti_port_name);
        break;
    case PRT_MASTER_RETIRED:
        _msti_sm_PRT_MASTER_RETIRED(p_db_port, msti_port_name);
        _msti_sm_PRT_MASTER_PORT(p_db_port, msti_port_name);
        break;
    case PRT_MASTER_FORWARD:
        _msti_sm_PRT_MASTER_FORWARD(p_db_port, msti_port_name);
        _msti_sm_PRT_MASTER_PORT(p_db_port, msti_port_name);
        break;
    case PRT_MASTER_LEARN:
        _msti_sm_PRT_MASTER_LEARN(p_db_port, msti_port_name);
        _msti_sm_PRT_MASTER_PORT(p_db_port, msti_port_name);
        break;
    case PRT_MASTER_DISCARD:
        _msti_sm_PRT_MASTER_DISCARD(p_db_port, msti_port_name);
        _msti_sm_PRT_MASTER_PORT(p_db_port, msti_port_name);
        break;
    case PRT_MASTER_PORT:
        _msti_sm_PRT_MASTER_PORT(p_db_port, msti_port_name);
        break;
    case PRT_DESIGNATED_PROPOSE:
        _msti_sm_PRT_DESIGNATED_PROPOSE(p_db_port, msti_port_name);
        _msti_sm_PRT_DESIGNATED_PORT(p_db_port, msti_port_name);
        break;
    case PRT_DESIGNATED_SYNCED:
        _msti_sm_PRT_DESIGNATED_SYNCED(p_db_port, msti_port_name);
        _msti_sm_PRT_DESIGNATED_PORT(p_db_port, msti_port_name);
        break;
    case PRT_DESIGNATED_RETIRED:
        _msti_sm_PRT_DESIGNATED_RETIRED(p_db_port, msti_port_name);
        _msti_sm_PRT_DESIGNATED_PORT(p_db_port, msti_port_name);
        break;
    case PRT_DESIGNATED_PORT:
        _msti_sm_PRT_DESIGNATED_PORT(p_db_port, msti_port_name);
        break;
    case PRT_DESIGNATED_DISCARD:
        msti_sm_PRT_DESIGNATED_DISCARD(p_db_port, msti_port_name);
        _msti_sm_PRT_DESIGNATED_PORT(p_db_port, msti_port_name);
        break;
    case PRT_DESIGNATED_LEARN:
        _msti_sm_PRT_DESIGNATED_LEARN(p_db_port, msti_port_name);
        _msti_sm_PRT_DESIGNATED_PORT(p_db_port, msti_port_name);
        break;
    case PRT_DESIGNATED_FORWARD:
        _msti_sm_PRT_DESIGNATED_FORWARD(p_db_port, msti_port_name);
        _msti_sm_PRT_DESIGNATED_PORT(p_db_port, msti_port_name);
        break;
    case PRT_ALTERNATE_PROPOSED:
        _msti_sm_PRT_ALTERNATE_PROPOSED(p_db_port, msti_port_name);
        _msti_sm_PRT_ALTERNATE_PORT(p_db_port, msti_port_name);
        break;
    case PRT_ALTERNATE_AGREED:
        _msti_sm_PRT_ALTERNATE_AGREED(p_db_port, msti_port_name);
        _msti_sm_PRT_ALTERNATE_PORT(p_db_port, msti_port_name);
        break;
    case PRT_ALTERNATE_PORT:
        _msti_sm_PRT_BLOCK_PORT(p_db_port, msti_port_name);
        _msti_sm_PRT_ALTERNATE_PORT(p_db_port, msti_port_name);
        break;
    case PRT_BACKUP_PORT:
        _msti_sm_PRT_BLOCK_PORT(p_db_port, msti_port_name);
        _msti_sm_PRT_BACKUP_PORT(p_db_port, msti_port_name);
        break;
    case PRT_BLOCK_PORT:
        _msti_sm_PRT_BLOCK_PORT(p_db_port, msti_port_name);
        break;
    default:
        /* modified by liwh for bug 36699/37625/37767, 2016-05-11 
           when there is no process, should return;
           otherwise, dead loop will happen */
        return PM_E_NONE;
        /* liwh end */
    }

    msti_sm_PRT_TRIGGER_EVENT(p_db_port);

    return PM_E_NONE;
}

/* Added by kcao for bug 42488, do root guard process when MSTI in different region */
int32
msti_sm_do_rootguard_process(tbl_mstp_port_t *p_db_port)
{
    tbl_msti_port_t *p_db_msti_port = NULL;
    bpdu_t *bpdu = p_db_port->bpdu;
    msti_bpdu_t *msti_bpdu = NULL;
    uint32 i = 0;
    uint8 rcvdInfo = 0;
    char msti_port_name[IFNAME_EXT_SIZE];

    if (!p_db_port->admin_rootguard)
    {
        return PM_E_NONE;
    }

    for (i = 0; i < bpdu->msti_num; i++)
    {
        msti_bpdu = &bpdu->msti[i];
        p_db_msti_port = mstp_api_get_msti_port(p_db_port, msti_bpdu->instance_id);
        if (NULL == p_db_msti_port)
        {
            continue;
        }
        
        p_db_msti_port->bpdu = msti_bpdu;
        rcvdInfo = msti_rcvInfo(p_db_msti_port);
        if (MSTP_SUPERIOR_DESIGNATED == rcvdInfo)
        {
            if (p_db_port->admin_rootguard)
            {
                p_db_msti_port->rcvdMsg = FALSE;
                p_db_msti_port->oper_rootguard = TRUE;    
                MSTP_MSTI_PORT_STR(msti_port_name, p_db_msti_port);
                msti_sm_PRT_DESIGNATED_DISCARD(p_db_msti_port, msti_port_name);
                _msti_sm_PIM_CURRENT(p_db_msti_port, msti_port_name);
            }
        }
        p_db_msti_port->bpdu = NULL;
    }
    
    return PM_E_NONE;
}

#define _17_30__13_35__PST_Port_State_Transitions_Machine_

extern uint32 mstp_sm_pst_state_table[PST_EVENT_MAX][PST_STATE_MAX];

/*
disableLearning(); learning = FALSE;
disableForwarding(); forwarding = FALSE;
*/
static int32
_msti_sm_PST_DISCARDING(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PST] %s enter DISCARDING", msti_port_name);
        
    p_db_port->learning = FALSE;
    p_db_port->forwarding = FALSE;
    
    mstp_api_set_msti_fei_state(p_db_port, GLB_BR_PORT_STATE_BLOCKING);

    p_db_port->pst_state = PST_DISCARDING;

    return PM_E_NONE;
}

/*
only for STP
*/
static int32
_msti_sm_PST_LISTENING(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PST] %s enter LISTENING", msti_port_name);

    if (NULL == p_db_port->fdWhile_timer)
    {
        p_db_port->fdWhile_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _msti_sm_fdWhile_timer_fn, p_db_port, 
            p_db_port->port_fwdDelay);
    }
        
    p_db_port->pst_state = PST_LISTENING;

    return PM_E_NONE;
}

/*
enableLearning(); learning = TRUE;
*/
static int32
_msti_sm_PST_LEARNING(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PST] %s enter LEARNING", msti_port_name);
    
    p_db_port->learning = TRUE;
    
    mstp_api_set_msti_fei_state(p_db_port, GLB_BR_PORT_STATE_LEARNING);

    p_db_port->pst_state = PST_LEARNING;

    return PM_E_NONE;
}

/*
enableForwarding(); forwarding = TRUE;
*/
static int32
_msti_sm_PST_FORWARDING(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti PST] %s enter FORWARDING", msti_port_name);
    
    p_db_port->forwarding = TRUE;

    mstp_api_set_msti_fei_state(p_db_port, GLB_BR_PORT_STATE_FORWARDING);

    p_db_port->pst_state = PST_FORWARDING;

    return PM_E_NONE;
}

/*
  Figure 17-24  Port State Transition state machine
*/
int32
msti_sm_pst_process_event(tbl_msti_port_t *p_db_port, pst_event_t event)
{
    tbl_mstp_port_t *p_mstp_port = (tbl_mstp_port_t*)p_db_port->p_port;
    pst_state_t next_state = 0;
    char msti_port_name[IFNAME_EXT_SIZE];

    if (event >= PST_EVENT_MAX)
    {
        return PM_E_FAIL;
    }

    MSTP_MSTI_PORT_STR(msti_port_name, p_db_port);
    next_state =  mstp_sm_pst_state_table[event][p_db_port->pst_state];
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti %s] msti_port %s recv event: %s, state: %s -> %s", 
            "PST",
            msti_port_name,
            cdb_enum_val2str(pst_event_strs, PST_EVENT_MAX, event),
            cdb_enum_val2str(pst_state_strs, PST_STATE_MAX, p_db_port->pst_state),
            cdb_enum_val2str(pst_state_strs, PST_STATE_MAX, next_state));

    /* modified by liwh for bug 37762, 2016-06-21 */
    if (p_mstp_port->admin_loopguard && p_db_port->oper_loopguard
        && (PST_DISCARDING != next_state))
    {
        return PM_E_NONE;
    }
    /* liwh end */
    
    switch (next_state)
    {
    case PST_DISCARDING:
        _msti_sm_PST_DISCARDING(p_db_port, msti_port_name);
        break;
    case PST_EVT_LISTEN:
        _msti_sm_PST_LISTENING(p_db_port, msti_port_name);
        break;
    case PST_LEARNING:
        _msti_sm_PST_LEARNING(p_db_port, msti_port_name);
        break;
    case PST_FORWARDING:
        _msti_sm_PST_FORWARDING(p_db_port, msti_port_name);
        break;
    default:
        break;
    }

    return PM_E_NONE;
}

#define _17_31__13_36__TCM_Topology_Change_Machine_

extern uint32 mstp_sm_tcm_state_table[TCM_EVENT_MAX][TCM_STATE_MAX];

/*
rcvdTc = rcvdTcn = rcvdTcAck = FALSE;
rcvdTc = tcProp = FALSE;
*/
static int32
_msti_sm_TCM_LEARNING(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti TCM] %s enter LEARNING", msti_port_name);
    
    p_db_port->rcvdTc = p_db_port->rcvdTcn = p_db_port->rcvdTcAck = FALSE;
    p_db_port->rcvdTc = p_db_port->tcProp = FALSE;

    return PM_E_NONE;
}

/*
fdbFlush = TRUE;
tcWhile = 0; tcAck = FALSE;
*/
static int32
_msti_sm_TCM_INACTIVE(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti TCM] %s enter INACTIVE", msti_port_name);
#if 0
    p_db_port->fdbFlush = TRUE;
    mstp_api_flush_fdb(p_db_port);
    p_db_port->fdbFlush = FALSE;
#endif

    CTC_TASK_STOP_TIMER(p_db_port->tcWhile_timer);
    p_db_port->tcAck = FALSE;

    p_db_port->tcm_state = TCM_INACTIVE;

    _msti_sm_TCM_LEARNING(p_db_port, msti_port_name);
    
    return PM_E_NONE;
}

static void
_msti_sm_tcWhile_timer_fn(void *p_data)
{
    tbl_msti_port_t *p_db_port = (tbl_msti_port_t*)p_data;
    char msti_port_name[IFNAME_EXT_SIZE];

    MSTP_MSTI_PORT_STR(msti_port_name, p_db_port);
    MSTP_DEBUG(STP_TIMER, "msti Timer tcWhile timeout on interface %s", msti_port_name);

    p_db_port->tcWhile_timer = NULL;
    
    mstp_sm_update_topo_change();

    return;
}

/*
17.21.7 newTcWhile()
*/
int32 
msti_newTcWhile(tbl_msti_port_t *p_db_port)
{
    tbl_mstp_port_t *p_mstp_port = (tbl_mstp_port_t*)p_db_port->p_port;
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    uint32 tcWhile_delay = 0;
    uint32 has_topo_change = FALSE;
    
    if (p_db_port->tcWhile_timer)
    {
        return PM_E_NONE;
    }

    /* check if this is a new top change */
    has_topo_change = mstp_sm_has_topo_change();

    if (!has_topo_change)
    {
        p_db_glb->last_tc_time = ctc_time_boottime_sec(NULL);
        p_db_glb->num_topo_changes++;
        tbl_mstp_global_set_mstp_global_field(p_db_glb, TBL_MSTP_GLOBAL_FLD_LAST_TC_TIME);
        tbl_mstp_global_set_mstp_global_field(p_db_glb, TBL_MSTP_GLOBAL_FLD_NUM_TOPO_CHANGES);
    }

    if (p_mstp_port->sendRstp)
    {
        tcWhile_delay = p_db_port->designated_helloTime + 1;
        p_db_port->newInfo = TRUE;       
    }
    else 
    {
        tcWhile_delay = p_db_glb->root_maxAge + p_db_glb->root_fwdDelay;
    }

    p_db_port->tcWhile_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _msti_sm_tcWhile_timer_fn, p_db_port, 
        tcWhile_delay);

    mstp_sm_update_topo_change();
    
    return PM_E_NONE;
}

/*
17.21.18 setTcPropTree()
*/
int32 
msti_setTcPropTree(tbl_msti_port_t *p_db_port)
{
    tbl_mstp_instance_key_t inst_key;
    tbl_mstp_instance_t *p_db_inst = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_msti_port_t *p_iter_port = NULL;
    tbl_mstp_port_t *p_iter_mstp_port = NULL;

    inst_key.instance = p_db_port->key.instance;
    p_db_inst = tbl_mstp_instance_get_mstp_instance(&inst_key);
    if (NULL == p_db_inst)
    {
        return PM_E_NONE;
    }
    
    CTCLIB_SLIST_LOOP(p_db_inst->port_list.obj_list, p_iter_port, listnode)
    {
        if (p_db_port == p_iter_port)
        {
            continue;
        }
        p_iter_mstp_port = (tbl_mstp_port_t*)p_iter_port->p_port;
        if (!p_iter_mstp_port->portEnabled)
        {
            continue;
        }
        if (!p_iter_port->tcProp && !p_iter_mstp_port->operEdge)
        {
            p_iter_port->tcProp = TRUE;
            if (p_iter_port->tcProp && (TCM_ACTIVE == p_iter_port->tcm_state))
            {
                msti_sm_tcm_process_event(p_iter_port, TCM_EVT_TCPROPNOTOPEREDGE);
            }
        }
    }
    
    return PM_E_NONE;
}

/*
newTcWhile(); setTcPropTree();
newInfo = TRUE;
*/
static int32
_msti_sm_TCM_DETECTED(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti TCM] %s enter DETECTED", msti_port_name);
    
    msti_newTcWhile(p_db_port);
    msti_setTcPropTree(p_db_port);
    p_db_port->newInfo = TRUE;
    p_db_port->tcm_state = TCM_DETECTED;

    return PM_E_NONE;
}

/*
*/
static int32
_msti_sm_TCM_ACTIVE(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    tbl_mstp_port_t *p_mstp_port = (tbl_mstp_port_t*)p_db_port->p_port;
    
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti TCM] %s enter ACTIVE", msti_port_name);
    
    p_db_port->tcm_state = TCM_ACTIVE;

    if ((p_db_port->tcProp && !p_mstp_port->operEdge))
    {
        msti_sm_tcm_process_event(p_db_port, TCM_EVT_TCPROPNOTOPEREDGE);
    }
    return PM_E_NONE;
}

/*
tcWhile = 0; rcvdTcAck = FALSE;
*/
static int32
_msti_sm_TCM_ACKNOWLEDGED(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti TCM] %s enter ACKNOWLEDGED", msti_port_name);
    
    CTC_TASK_STOP_TIMER(p_db_port->tcWhile_timer);
    p_db_port->rcvdTcAck = FALSE;
    p_db_port->tcm_state = TCM_ACKNOWLEDGED;

    mstp_sm_update_topo_change();
    return PM_E_NONE;
}

/*
newTcWhile(); fdbFlush = TRUE;
tcProp = FALSE;
*/
static int32
_msti_sm_TCM_PROPAGATING(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    tbl_mstp_port_t *p_mstp_port = (tbl_mstp_port_t*)p_db_port->p_port;

    MSTP_DEBUG(STP_PROTO_MSTI, "[msti TCM] %s enter PROPAGATING", msti_port_name);
    
    msti_newTcWhile(p_db_port);
    p_db_port->fdbFlush = TRUE;
    mstp_api_flush_fdb(p_mstp_port);
    p_db_port->fdbFlush = FALSE;
    p_db_port->tcProp = FALSE;
    p_db_port->tcm_state = TCM_PROPAGATING;

    return PM_E_NONE;
}

/*
rcvdTcn = rcvdTc = FALSE;
if (role == DesignatedPort) tcAck = TRUE;
setTcPropTree();
*/
static int32
_msti_sm_TCM_NOTIFIED_TC(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti TCM] %s enter NOTIFIED_TC", msti_port_name);
    
    p_db_port->rcvdTcn = p_db_port->rcvdTc = FALSE;
    if (ROLE_DESIGNATED == p_db_port->role)
    {
        p_db_port->tcAck = TRUE;
    }
    msti_setTcPropTree(p_db_port);
    p_db_port->tcm_state = TCM_NOTIFIED_TC;

    return PM_E_NONE;
}

/*
newTcWhile();
*/
static int32
_msti_sm_TCM_NOTIFIED_TCN(tbl_msti_port_t *p_db_port, char *msti_port_name)
{
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti TCM] %s enter NOTIFIED_TCN", msti_port_name);
    
    msti_newTcWhile(p_db_port);
    p_db_port->tcm_state = TCM_NOTIFIED_TCN;

    return PM_E_NONE;
}

static int32
_msti_sm_TCM_TRIGGER_EVENT()
{
    return PM_E_NONE;
}

/* 
  Figure 17-25  Topology Change state machine
*/
int32
msti_sm_tcm_process_event(tbl_msti_port_t *p_db_port, tcm_event_t event)
{
    tcm_state_t next_state = 0;
    char msti_port_name[IFNAME_EXT_SIZE];

    if (event >= TCM_EVENT_MAX)
    {
        return PM_E_FAIL;
    }

    MSTP_MSTI_PORT_STR(msti_port_name, p_db_port);
    next_state =  mstp_sm_tcm_state_table[event][p_db_port->tcm_state];
    MSTP_DEBUG(STP_PROTO_MSTI, "[msti %s] msti_port %s recv event: %s, state: %s -> %s", 
            "TCM",
            msti_port_name,
            cdb_enum_val2str(tcm_event_strs, TCM_EVENT_MAX, event),
            cdb_enum_val2str(tcm_state_strs, TCM_STATE_MAX, p_db_port->tcm_state),
            cdb_enum_val2str(tcm_state_strs, TCM_STATE_MAX, next_state));

    switch (next_state)
    {
    case TCM_INACTIVE:
        _msti_sm_TCM_INACTIVE(p_db_port, msti_port_name);
        _msti_sm_TCM_LEARNING(p_db_port, msti_port_name);
        break;
    case TCM_DETECTED:
        _msti_sm_TCM_DETECTED(p_db_port, msti_port_name);
        _msti_sm_TCM_ACTIVE(p_db_port, msti_port_name);
        break;
    case TCM_ACTIVE:
        _msti_sm_TCM_ACTIVE(p_db_port, msti_port_name);
        break;
    case TCM_ACKNOWLEDGED:
        _msti_sm_TCM_ACKNOWLEDGED(p_db_port, msti_port_name);
        _msti_sm_TCM_ACTIVE(p_db_port, msti_port_name);
        break;
    case TCM_PROPAGATING:
        /* modified by liwh for bug 37514, 2016-05-17 */
        if (!p_db_port->restricted_tcn)
        /* liwh end */
        {   
            _msti_sm_TCM_PROPAGATING(p_db_port, msti_port_name);
        }
        else
        {
            /* Added by kcao for bug 39442, if restricted_tcn, should clear tcProp flag, otherwise, will deadloop */    
            p_db_port->tcProp = FALSE;
        }
        _msti_sm_TCM_ACTIVE(p_db_port, msti_port_name);
        break;
    case TCM_NOTIFIED_TC:
        _msti_sm_TCM_NOTIFIED_TC(p_db_port, msti_port_name);
        _msti_sm_TCM_ACTIVE(p_db_port, msti_port_name);
        break;
    case TCM_NOTIFIED_TCN:
        _msti_sm_TCM_NOTIFIED_TCN(p_db_port, msti_port_name);
        _msti_sm_TCM_ACTIVE(p_db_port, msti_port_name);
        break;
    default:
        break;
    }

    _msti_sm_TCM_TRIGGER_EVENT();

    return PM_E_NONE;
}

int32
msti_sm_port_start_prs(tbl_mstp_instance_t *p_db_inst)
{
    msti_sm_prs_process_event(p_db_inst, PRS_EVT_BEGIN);
    return PM_E_NONE;
}

int32
msti_sm_port_stop_prs(tbl_mstp_instance_t *p_db_inst)
{
    p_db_inst->prs_state = PRS_INIT_BRIDGE;
    return PM_E_NONE;
}

int32
msti_sm_port_start_pst(tbl_msti_port_t *p_db_port)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    
    if (MSTP_TYPE_MSTP == p_db_glb->type)
    {
        msti_sm_pst_process_event(p_db_port, PST_EVT_BEGIN);
    }
    
    return PM_E_NONE;
}

int32
msti_sm_port_stop_pst(tbl_msti_port_t *p_db_port)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    
    if (MSTP_TYPE_MSTP == p_db_glb->type)
    {
        p_db_port->pst_state = PST_DISCARDING;
    }

    return PM_E_NONE;
}

int32
msti_sm_port_start_pim(tbl_msti_port_t *p_db_port)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    
    if (MSTP_TYPE_MSTP == p_db_glb->type)
    {
        msti_sm_pim_process_event(p_db_port, PIM_EVT_BEGIN);
        msti_sm_pim_process_event(p_db_port, PIM_EVT_PORTENABLED);
    }
    
    return PM_E_NONE;
}

int32
msti_sm_port_stop_pim(tbl_msti_port_t *p_db_port)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    
    if (MSTP_TYPE_MSTP == p_db_glb->type)
    {
        p_db_port->pim_state = PIM_DISABLED;
    }
    
    return PM_E_NONE;
}

int32
msti_sm_port_start_prt(tbl_msti_port_t *p_db_port)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    
    if (MSTP_TYPE_MSTP == p_db_glb->type)
    {
        msti_sm_prt_process_event(p_db_port, PRT_EVT_BEGIN);
    }
    
    return PM_E_NONE;
}

int32
msti_sm_port_stop_prt(tbl_msti_port_t *p_db_port)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    
    if (MSTP_TYPE_MSTP == p_db_glb->type)
    {
        CTC_TASK_STOP_TIMER(p_db_port->fdWhile_timer);
        CTC_TASK_STOP_TIMER(p_db_port->rrWhile_timer);
        CTC_TASK_STOP_TIMER(p_db_port->rbWhile_timer);
        p_db_port->prt_state = PRT_INIT_PORT;
    }
    
    return PM_E_NONE;
}

int32
msti_sm_port_start_tcm(tbl_msti_port_t *p_db_port)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    
    if (MSTP_TYPE_MSTP == p_db_glb->type)
    {
        msti_sm_tcm_process_event(p_db_port, TCM_EVT_BEGIN);
    }
    
    return PM_E_NONE;
}

int32
msti_sm_port_stop_tcm(tbl_msti_port_t *p_db_port)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    
    if (MSTP_TYPE_MSTP == p_db_glb->type)
    {
        CTC_TASK_STOP_TIMER(p_db_port->tcWhile_timer);
        p_db_port->tcm_state = TCM_INACTIVE;
    }
    
    return PM_E_NONE;
}

#endif /* !CONFIG_MSTP */

