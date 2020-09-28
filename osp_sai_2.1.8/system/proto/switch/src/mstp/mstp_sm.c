
#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_mstp_port_define.h"
#include "gen/tbl_mstp_port.h"
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

int32
_mstp_sm_PRT_DESIGNATED_DISCARD(tbl_mstp_port_t *p_db_port, char *ifname_ext);
static int32
_mstp_sm_PRT_DESIGNATED_PORT(tbl_mstp_port_t *p_db_port, char *ifname_ext);
static int32
_mstp_sm_PPM_SENSING(tbl_mstp_port_t *p_db_port, char *ifname_ext);
static int32
_mstp_sm_PRT_BLOCK_PORT(tbl_mstp_port_t *p_db_port, char *ifname_ext);
static int32
_mstp_sm_PRT_ALTERNATE_PORT(tbl_mstp_port_t *p_db_port, char *ifname_ext);
static int32
_mstp_sm_PST_DISCARDING(tbl_mstp_port_t *p_db_port, char *ifname_ext);

static int32
_mstp_sm_PRT_TRIGGER_EVENT_DESIGNATED(tbl_mstp_port_t *p_db_port)
{
    tbl_mstp_global_t *p_mstp_glb = tbl_mstp_global_get_mstp_global();
    
    if (ROLE_DESIGNATED != p_db_port->role)
    {
        return PM_E_NONE;
    }

    if (MSTP_TYPE_STP == p_mstp_glb->type)
    {
        if (PST_DISCARDING == p_db_port->pst_state)
        {
            mstp_sm_pst_process_event(p_db_port, PST_EVT_LISTEN);
        }
    }

    /* (!learning && !forwarding && !synced) || (agreed && !synced) || (operEdge && !synced) || (sync && synced) */
    if ((!p_db_port->learning && !p_db_port->forwarding && !p_db_port->synced)
     || (p_db_port->agreed && !p_db_port->synced)
     || (p_db_port->operEdge && !p_db_port->synced)
     || (p_db_port->sync && p_db_port->synced))
    {
        mstp_sm_prt_process_event(p_db_port, PRT_EVT_DESIGNATED_SYNCED);
    }
    
    /* ((sync && !synced) || (reRoot && (rrWhile != 0)) || disputed) && !operEdge && (learn || forward) */
    if (((p_db_port->sync && !p_db_port->synced) || (p_db_port->reRoot && p_db_port->rrWhile_timer) || p_db_port->disputed)
     && (!p_db_port->rrWhile_timer || !p_db_port->reRoot)
     && !p_db_port->operEdge
     && (p_db_port->learn || p_db_port->forward))
    {
        mstp_sm_prt_process_event(p_db_port, PRT_EVT_DESIGNATEDDISCARD);
    }

    /* ((fdWhile == 0) || agreed || operEdge) && ((rrWhile ==0) || !reRoot) && !sync && !learn */
    if ((!p_db_port->fdWhile_timer || p_db_port->agreed || p_db_port->operEdge) 
     && (!p_db_port->rrWhile_timer || !p_db_port->reRoot)
     && !p_db_port->sync
     && !p_db_port->learn
     /* modified by liwh for bug 36699/37625/37767, 2016-05-11 */
     && !p_db_port->oper_rootguard)
     /* liwh end */
    {
        mstp_sm_prt_process_event(p_db_port, PRT_EVT_DESIGNATEDLEARN);
    }

    /* ((fdWhile == 0) || agreed || operEdge) && ((rrWhile ==0) || !reRoot) && !sync && (learn && !forward) */
    if ((!p_db_port->fdWhile_timer || p_db_port->agreed || p_db_port->operEdge) 
     && (!p_db_port->rrWhile_timer || !p_db_port->reRoot)
     && !p_db_port->sync 
     && (p_db_port->learn && !p_db_port->forward))
    {
        mstp_sm_prt_process_event(p_db_port, PRT_EVT_DESIGNATEDFORWARD);
    }

    /* !forward && !agreed && !proposing && !operEdge */
    if (!p_db_port->forward
     && !p_db_port->agreed
     && !p_db_port->proposing
     && !p_db_port->operEdge)
    {
        mstp_sm_prt_process_event(p_db_port, PRT_EVT_PROPOSING);
    }

    return PM_E_NONE;
}

static int32
_mstp_sm_PRT_TRIGGER_EVENT_ROOTPORT(tbl_mstp_port_t *p_db_port)
{
    tbl_mstp_global_t *p_mstp_glb = tbl_mstp_global_get_mstp_global();
    uint32 reRooted = FALSE;    /* RSTPTODO */
    
    if (ROLE_ROOTPORT != p_db_port->role)
    {
        return PM_E_NONE;
    }

    if (MSTP_TYPE_STP == p_mstp_glb->type)
    {
        if (PST_DISCARDING == p_db_port->pst_state)
        {
            mstp_sm_pst_process_event(p_db_port, PST_EVT_LISTEN);
        }

        if ((!p_db_port->fdWhile_timer || (reRooted && !p_db_port->rbWhile_timer))
         && (!p_db_port->learn))
        {
            mstp_sm_prt_process_event(p_db_port, PRT_EVT_ROOTLEARN);
        }

        if ((!p_db_port->fdWhile_timer || (reRooted && !p_db_port->rbWhile_timer))
         && (p_db_port->learn)
         && (!p_db_port->forward))
        {
            mstp_sm_prt_process_event(p_db_port, PRT_EVT_ROOTFORWARD);
        }
    }
    else
    {
        if ((!p_db_port->fdWhile_timer || (reRooted && !p_db_port->rbWhile_timer))
         && (p_db_port->rstpVersion)
         && (!p_db_port->learn))
        {
            mstp_sm_prt_process_event(p_db_port, PRT_EVT_ROOTLEARN);
        }

        if ((!p_db_port->fdWhile_timer || (reRooted && !p_db_port->rbWhile_timer))
         && (p_db_port->rstpVersion)
         && (p_db_port->learn)
         && (!p_db_port->forward))
        {
            mstp_sm_prt_process_event(p_db_port, PRT_EVT_ROOTFORWARD);
        }
    }
    
    if (MSTP_TYPE_MSTP == p_mstp_glb->type)
    {
        /* MSTP inter-region need to update it's MSTI states */
        if (!p_db_port->rcvdInternal && (MSTP_INFO_RECEIVED == p_db_port->infoIs))
        {
            mstp_api_port_reselect_all_msti(p_db_port);
        }
    }
    
    return PM_E_NONE;
}

static int32
_mstp_sm_PRT_TRIGGER_EVENT_ALTERNATE(tbl_mstp_port_t *p_db_port)
{
    tbl_mstp_global_t *p_mstp_glb = tbl_mstp_global_get_mstp_global();
    
    if (ROLE_ALTERNATE != p_db_port->role)
    {
        return PM_E_NONE;
    }

    if (MSTP_TYPE_STP == p_mstp_glb->type)
    {
        if (PST_LISTENING == p_db_port->pst_state)
        {
            mstp_sm_pst_process_event(p_db_port, PST_EVT_NOTLEARN);
        }
    }

    if (MSTP_TYPE_MSTP == p_mstp_glb->type)
    {
        /* MSTP inter-region need to update it's MSTI states */
        if (!p_db_port->rcvdInternal && (MSTP_INFO_RECEIVED == p_db_port->infoIs))
        {
            mstp_api_port_reselect_all_msti(p_db_port);
        }
    }
    
    return PM_E_NONE;
}

int32
mstp_sm_PRT_TRIGGER_EVENT(tbl_mstp_port_t *p_db_port)
{
    switch (p_db_port->role)
    {
    case ROLE_DESIGNATED:
        _mstp_sm_PRT_TRIGGER_EVENT_DESIGNATED(p_db_port);
        break;
    case ROLE_ROOTPORT:
        _mstp_sm_PRT_TRIGGER_EVENT_ROOTPORT(p_db_port);
        break;
    case ROLE_ALTERNATE:
        _mstp_sm_PRT_TRIGGER_EVENT_ALTERNATE(p_db_port);
        break;
    default:
        break;
    }
    return PM_E_NONE;
}

int32
mstp_sm_rx_bpdu(tbl_mstp_port_t *p_db_port)
{
    tbl_msti_port_t *p_db_msti_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    bpdu_t *p_bpdu = (bpdu_t*)p_db_port->bpdu;
    char ifname_ext[IFNAME_EXT_SIZE];
    char msti_port_name[IFNAME_EXT_SIZE];
    
    IFNAME_ETH2FULL(p_db_port->key.name, ifname_ext);
    
    /* Added by kcao for bug 37707, process TCN especially */
    if (BPDU_TYPE_TCN == p_bpdu->type)
    {
        p_db_port->rcvdTcn = TRUE;
        mstp_sm_tcm_process_event(p_db_port, TCM_EVT_RCVDTCN);
        return PM_E_NONE;
    }

    if (PRX_DISCARD == p_db_port->prx_state)
    {
        mstp_sm_prx_process_event(p_db_port, PRX_EVT_RCVDBPDU_PORT_ENABLED);
    }
    else if (PRX_RECEIVE == p_db_port->prx_state)
    {
        /* modified by liwh for bug 37762, 2016-06-21 */
        /* Update port loopguard operational . */
        if (p_db_port->admin_loopguard && p_db_port->oper_loopguard)
        {  
            MSTP_DEBUG(STP_PROTO_CIST, "mstp_cist_process_message: "
            "receive a bpdu on a loop guard enabled "
            "port %s .", ifname_ext);

            _mstp_sm_PRT_DESIGNATED_DISCARD(p_db_port, ifname_ext);
            p_db_port->oper_loopguard = FALSE;
            CTCLIB_SLIST_LOOP(p_db_port->msti_list.obj_list, p_db_msti_port, listnode)
            {
                if (p_db_msti_port->oper_loopguard)
                {
                    MSTP_MSTI_PORT_STR(msti_port_name, p_db_msti_port);
                    msti_sm_PRT_DESIGNATED_DISCARD(p_db_msti_port, msti_port_name);
                    p_db_msti_port->oper_loopguard = FALSE;
                }
            }
            mstp_api_reselect();
            mstp_api_port_reselect_all_msti(p_db_port);
        }
        /* liwh end */
        
        if (!p_db_port->rcvdMsg)
        {
            mstp_sm_prx_process_event(p_db_port, PRX_EVT_RCVDBPDU_PORT_ENABLED_NOT_RCVDMSG);
        }
    }

    return PM_E_NONE;
}

uint32
mstp_sm_has_topo_change()
{
    tbl_mstp_port_master_t *p_master = tbl_mstp_port_get_master();
    tbl_mstp_port_t *p_item_port = NULL;
    ctclib_slistnode_t *listnode = NULL;

    CTCLIB_SLIST_LOOP(p_master->port_list, p_item_port, listnode)
    {
        if (p_item_port->tcWhile_timer)
        {
            return TRUE;
        }
    }

    return FALSE;
}

uint32
mstp_sm_update_topo_change()
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    tbl_mstp_port_master_t *p_master = tbl_mstp_port_get_master();
    tbl_mstp_port_t *p_item_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    uint32 new_topology_change_detected = FALSE;

    CTCLIB_SLIST_LOOP(p_master->port_list, p_item_port, listnode)
    {
        if (p_item_port->tcWhile_timer)
        {
            new_topology_change_detected = TRUE;
        }
    }

    if (p_db_glb->topology_change_detected != new_topology_change_detected)
    {
        
        if (new_topology_change_detected)
        {
            /* TC start, change FDB ageing time */
            swth_api_mstp_topology_change_cb(new_topology_change_detected, p_db_glb->root_fwdDelay);
            MSTP_DEBUG(STP_PROTO_CIST, "Topo-change start, set FDB ageing time to %u seconds", p_db_glb->root_fwdDelay);
        }
        else
        {
            /* TC stop, resume FDB ageing time */
            swth_api_mstp_topology_change_cb(new_topology_change_detected, 0);
            MSTP_DEBUG(STP_PROTO_CIST, "Topo-change stop, resume FDB ageing time");
        }
        
        p_db_glb->topology_change_detected = new_topology_change_detected;
    }

    return new_topology_change_detected;
}

/* PTI (Port TImer Machine)
   802.1D.2004  P.170  17.22 Port Timers state machine
   802.1Q.2005  P.179  13.27 The Port Timers state machine
*/
static void
_mstp_sm_pti_port_timer_fn(void *p_data)
{
    tbl_mstp_port_t* p_db_port = (tbl_mstp_port_t*)p_data;
    
    p_db_port->port_timer = NULL;

    /* Other timer dec is not in port timer */
    GLB_DEC(p_db_port->txCount);
    p_db_port->port_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _mstp_sm_pti_port_timer_fn, p_db_port, MSTP_PORT_TIMER_INTERVAL);

    return;
}

static void
_mstp_sm_helloWhen_timer_fn(void *p_data)
{
    tbl_mstp_port_t *p_db_port = (tbl_mstp_port_t*)p_data;
    char ifname_ext[IFNAME_EXT_SIZE];

    IFNAME_ETH2FULL(p_db_port->key.name, ifname_ext);
    MSTP_DEBUG(STP_TIMER, "Timer helloWhen timeout on interface %s", ifname_ext);

    /* txCount should be cleared every helloTime */
    p_db_port->txCount = 0;
    p_db_port->helloWhen_timer = NULL;
    mstp_sm_ptx_process_event(p_db_port, PTX_EVT_HELLOWHENZERO);

    return;
}

void
_mstp_sm_rrWhile_timer_fn(void *p_data)
{
    tbl_mstp_port_t *p_db_port = (tbl_mstp_port_t*)p_data;

    p_db_port->rrWhile_timer = NULL;

    return;
}

static void
_mstp_sm_fdWhile_timer_fn(void *p_data)
{
    tbl_mstp_port_t *p_db_port = (tbl_mstp_port_t*)p_data;
    char ifname_ext[IFNAME_EXT_SIZE];

    IFNAME_ETH2FULL(p_db_port->key.name, ifname_ext);
    MSTP_DEBUG(STP_TIMER, "Timer fdWhile timeout on interface %s", ifname_ext);

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

    mstp_sm_PRT_TRIGGER_EVENT(p_db_port);

    return;
}

static void
_mstp_sm_rbWhile_timer_fn(void *p_data)
{
    tbl_mstp_port_t *p_db_port = (tbl_mstp_port_t*)p_data;
    char ifname_ext[IFNAME_EXT_SIZE];

    IFNAME_ETH2FULL(p_db_port->key.name, ifname_ext);
    MSTP_DEBUG(STP_TIMER, "Timer rbWhile timeout on interface %s", ifname_ext);

    p_db_port->rbWhile_timer = NULL;

    return;
}

static void
_mstp_sm_tcWhile_timer_fn(void *p_data)
{
    tbl_mstp_port_t *p_db_port = (tbl_mstp_port_t*)p_data;
    char ifname_ext[IFNAME_EXT_SIZE];

    IFNAME_ETH2FULL(p_db_port->key.name, ifname_ext);
    MSTP_DEBUG(STP_TIMER, "Timer tcWhile timeout on interface %s", ifname_ext);

    p_db_port->tcWhile_timer = NULL;
    
    mstp_sm_update_topo_change();

    return;
}

static void
_mstp_sm_edgeDelayWhile_timer_fn(void *p_data)
{
    tbl_mstp_port_t* p_db_port = (tbl_mstp_port_t*)p_data;
    char ifname_ext[IFNAME_EXT_SIZE];

    IFNAME_ETH2FULL(p_db_port->key.name, ifname_ext);
    MSTP_DEBUG(STP_TIMER, "Timer edgeDelayWhile timeout on interface %s", ifname_ext);

    if (p_db_port->adminEdge)
    {
        if (!p_db_port->operEdge)
        {
            mstp_sm_bdm_process_event(p_db_port, BDM_EVT_EDGEDELAY_TIMEOUT_EDGE);
        }
    }

    p_db_port->edgeDelayWhile_timer = NULL;
    // MSTP_TODO
    return;
}

static void
_mstp_sm_mdelayWhile_timer_fn(void *p_data)
{
    tbl_mstp_port_t* p_db_port = (tbl_mstp_port_t*)p_data;
    char ifname_ext[IFNAME_EXT_SIZE];

    IFNAME_ETH2FULL(p_db_port->key.name, ifname_ext);
    MSTP_DEBUG(STP_TIMER, "Timer mdelayWhile timeout on interface %s", ifname_ext);

    p_db_port->mdelayWhile_timer = NULL;
    
    if (PPM_CHECKING_RSTP == p_db_port->ppm_state
     || PPM_SELECTING_STP == p_db_port->ppm_state)
    {
        _mstp_sm_PPM_SENSING(p_db_port, ifname_ext);
    }

    return;
}

/* 17.21.22 updtBPDUVersion()
   Sets rcvdSTP TRUE if the BPDU received is a version 0 or version 1 TCN or a Config BPDU. Sets
   rcvdRSTP TRUE if the received BPDU is an RST BPDU.
*/ 
int32
updtBPDUVersion(tbl_mstp_port_t *p_db_port)
{
    bpdu_t *p_bpdu = (bpdu_t*)p_db_port->bpdu;

    if (NULL == p_bpdu)
    {
        return PM_E_NONE;
    }

    if (BPDU_TYPE_CONFIG == p_bpdu->type || BPDU_TYPE_TCN == p_bpdu->type)
    {
        p_db_port->rcvdSTP = TRUE;
        p_db_port->rcvdRSTP = FALSE;
        p_db_port->rcvdMSTP = FALSE;
    }
    else if (BPDU_TYPE_RST == p_bpdu->type)
    {
        p_db_port->rcvdSTP = FALSE;
        p_db_port->rcvdRSTP = TRUE;
        if (BPDU_VERSION_MSTP == p_bpdu->version)
        {
            p_db_port->rcvdMSTP = TRUE;
        }
        else
        {
            p_db_port->rcvdMSTP = FALSE;
        }
    }
    
    return PM_E_NONE;
}

#define _17_22__13_27__PTI_Port_TImer_Machine_
/* PTI is sample, need not state machine */
#if 0
/* PTI (Port TImer Machine)
   802.1D.2004  P.170  17.22 Port Timers state machine
   802.1Q.2005  P.179  13.27 The Port Timers state machine
*/
uint32 mstp_sm_pti_state_table[PTI_EVENT_MAX][PTI_STATE_MAX] = 
{
  /* PTI_TICK           PTI_ONE_SECOND */
    {PTI_ONE_SECOND,    PTI_ONE_SECOND},    /* PTI_EVT_BEGIN */
    {PTI_STATE_MAX,     PTI_TICK},          /* PTI_EVT_TICKTRUE */
};

int32
mstp_sm_pti_process_event(tbl_mstp_port_t *p_db_port, pti_event_t event)
{
    pti_state_t next_state = 0;

    if (event >= PTI_EVENT_MAX)
    {
        return PM_E_FAIL;
    }

    next_state =  mstp_sm_pti_state_table[event][p_db_port->pti_state];
    MSTP_DEBUG(STP_PROTO_CIST, "[%s] Interface %s recv event: %s, state: %s -> %s", 
            "PTI SM",
            ifname_ext,
            cdb_enum_val2str(pti_event_strs, PTI_EVENT_MAX, event),
            cdb_enum_val2str(pti_state_strs, PTI_STATE_MAX, p_db_port->pti_state),
            cdb_enum_val2str(pti_state_strs, PTI_STATE_MAX, next_state));

    switch (next_state)
    {
    case PTI_TICK:
        _mstp_sm_pti_tick(p_db_port);
        break;
    case PTI_ONE_SECOND:
        _mstp_sm_pti_port_timer_fn(p_db_port);
        break;
    default:
        break;
    }

    return PM_E_NONE;
}
#endif

#define _17_23__13_28__PRX_Port_Receive_Machine_

/* PRX (Port Receive Machine)
   802.1D.2004  P.170  17.23 Port Receive state machine
   802.1Q.2005  P.179  13.28 Port Receive state machine
*/
uint32 mstp_sm_prx_state_table[PRX_EVENT_MAX][PRX_STATE_MAX] = 
{
  /* PRX_DISCARD        PRX_RECEIVE */
    {PRX_DISCARD,       PRX_DISCARD},       /* PRX_EVT_BEGIN */
    {PRX_RECEIVE,       PRX_STATE_MAX},     /* PRX_EVT_RCVDBPDU_PORT_ENABLED */
    {PRX_STATE_MAX,     PRX_RECEIVE},       /* PRX_EVT_RCVDBPDU_PORT_ENABLED_NOT_RCVDMSG */
};

static int32
_mstp_sm_PRX_DISCARD(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[PRX] %s enter PRX_DISCARD", ifname_ext);
    
    p_db_port->rcvdBpdu = p_db_port->rcvdMSTP = p_db_port->rcvdRSTP = p_db_port->rcvdSTP = FALSE;
    p_db_port->rcvdMsg = FALSE;

    CTC_TASK_STOP_TIMER(p_db_port->edgeDelayWhile_timer);
    p_db_port->edgeDelayWhile_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _mstp_sm_edgeDelayWhile_timer_fn, p_db_port, 
        MSTP_TIMER_DEF_MIGRATE_TIME);

    p_db_port->prx_state = PRX_DISCARD;

    return PM_E_NONE;
}

static int32
mstp_sm_msti_setRcvdMsgs(tbl_mstp_port_t *p_db_port)
{
    tbl_msti_port_t *p_db_msti_port = NULL;
    bpdu_t *bpdu = p_db_port->bpdu;
    msti_bpdu_t *msti_bpdu = NULL;
    uint32 i = 0;
    
    if (!p_db_port->rcvdInternal)
    {
        /* Added by kcao for bug 42488, do root guard process when MSTI in different region */
        msti_sm_do_rootguard_process(p_db_port);
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
        p_db_msti_port->rcvdMsg = TRUE;
        p_db_msti_port->bpdu = msti_bpdu;
        msti_sm_pim_process_event(p_db_msti_port, PIM_EVT_RCVDMSGNOTUPDTINFO);
        p_db_msti_port->bpdu = NULL;
    }
    
    return PM_E_NONE;
}

static int32
_mstp_sm_PRX_RECEIVE_TRIGGER_EVENT(tbl_mstp_port_t *p_db_port)
{
    if (PIM_CURRENT == p_db_port->pim_state)
    {
        if (!p_db_port->updtInfo)
        {
            mstp_sm_pim_process_event(p_db_port, PIM_EVT_RCVDMSGNOTUPDTINFO);
            mstp_sm_msti_setRcvdMsgs(p_db_port);
        }
    }
    else if (PIM_DISABLED == p_db_port->pim_state)
    {
        mstp_sm_pim_process_event(p_db_port, PIM_EVT_RCVDMSG);
    }

    if (PPM_SENSING == p_db_port->ppm_state)
    {
        if (p_db_port->sendRstp && p_db_port->rcvdSTP)
        {
            mstp_sm_ppm_process_event(p_db_port, PPM_EVT_SENDRSTP_RCVDSTP);
        }

        if (p_db_port->rstpVersion && !p_db_port->sendRstp && p_db_port->rcvdRSTP)
        {
            mstp_sm_ppm_process_event(p_db_port, PPM_EVT_RSTPVERSION_NOTSENDRSTP_RCVDRSTP);
        }
    }

    return PM_E_NONE;
}

uint32
fromSameRegion(tbl_mstp_port_t *p_db_port)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    bpdu_t *bpdu = (bpdu_t*)p_db_port->bpdu;

    p_db_port->rcvdInternal = FALSE;

    if (MSTP_TYPE_MSTP != p_db_glb->type)
    {
        return FALSE;
    }

    if (p_db_port->rcvdRSTP && (BPDU_VERSION_MSTP == bpdu->version))
    {
        if (!sal_memcmp(bpdu->mstp_cfg.region_name, p_db_glb->mstp_region_name, MSTP_REGION_NAME_LEN)
          && !sal_memcmp(bpdu->mstp_cfg.digest, p_db_glb->mstp_digest.data, MSTP_DIGEST_LEN)
          && (bpdu->mstp_cfg.revision_level == p_db_glb->mstp_revision_level)
          && (bpdu->mstp_cfg.format_id == 0))
        {
            return TRUE;
        }
    }

    return FALSE;
}

/*
updtBPDUVersion();
operEdge = rcvdBpdu = FALSE; rcvdMsg = TRUE;
edgeDelayWhile = MigrateTime;
*/
static int32
_mstp_sm_PRX_RECEIVE(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{    
    uint32 old_rcvdInternal = p_db_port->rcvdInternal;
        
    MSTP_DEBUG(STP_PROTO_CIST, "[PRX] %s enter PRX_RECEIVE", ifname_ext);
    
    updtBPDUVersion(p_db_port);
    p_db_port->rcvdInternal = fromSameRegion(p_db_port);

    if (old_rcvdInternal != p_db_port->rcvdInternal)
    {
        p_db_port->rcvdInternalChange = TRUE;
    }
    
    if (p_db_port->operEdge)
    {
        mstp_sm_bdm_process_event(p_db_port, BDM_EVT_RECV_BPDU);
    }
    p_db_port->rcvdBpdu = FALSE;
    p_db_port->rcvdMsg = TRUE;

    CTC_TASK_STOP_TIMER(p_db_port->edgeDelayWhile_timer);
    p_db_port->edgeDelayWhile_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _mstp_sm_edgeDelayWhile_timer_fn, p_db_port, 
        MSTP_TIMER_DEF_MIGRATE_TIME);
    
    p_db_port->prx_state = PRX_RECEIVE;

    _mstp_sm_PRX_RECEIVE_TRIGGER_EVENT(p_db_port);

    /* if rcvdInternal change, need reselect */
    if (p_db_port->rcvdInternalChange)
    {
        mstp_api_port_reselect_all_msti(p_db_port);
    }
    p_db_port->rcvdInternalChange = FALSE;
    p_db_port->rcvdMsg = FALSE;

    return PM_E_NONE;
}

/*
17.23 Port Receive state machine
The Port Receive state machine shall implement the function specified by the state diagram in Figure 17-14,
the definitions in 17.16, and the variable declarations and procedures specified in 17.17 through 17.21. It
receives each valid BPDU (9.3.4), setting rcvdMsg to communicate the BPDU¡¯s arrival to the Port
Information Machine (17.27), and using the updtBPDUversion procedure (17.21.22) to set either rcvdRSTP
(17.19.28) or rcvdSTP (17.19.29) to communicate the BPDU¡¯s arrival and type to the Port Protocol
Migration machine (17.24).
*/
int32
mstp_sm_prx_process_event(tbl_mstp_port_t *p_db_port, prx_event_t event)
{
    prx_state_t next_state = 0;
    char ifname_ext[IFNAME_EXT_SIZE];

    IFNAME_ETH2FULL(p_db_port->key.name, ifname_ext);

    if (event >= PRX_EVENT_MAX)
    {
        return PM_E_FAIL;
    }

    next_state =  mstp_sm_prx_state_table[event][p_db_port->prx_state];
    MSTP_DEBUG(STP_PROTO_CIST, "[%s] Interface %s recv event: %s, state: %s -> %s", 
            "PRX",
            ifname_ext,
            cdb_enum_val2str(prx_event_strs, PRX_EVENT_MAX, event),
            cdb_enum_val2str(prx_state_strs, PRX_STATE_MAX, p_db_port->prx_state),
            cdb_enum_val2str(prx_state_strs, PRX_STATE_MAX, next_state));

    switch (next_state)
    {
    case PRX_DISCARD:
        _mstp_sm_PRX_DISCARD(p_db_port, ifname_ext);
        break;
    case PRX_RECEIVE:
        _mstp_sm_PRX_RECEIVE(p_db_port, ifname_ext);
        break;
    default:
        break;
    }

    return PM_E_NONE;
}

#define _17_24__13_29__PPM_Port_Protocol_Machine_

/* PPM State (Port Protocol Migration)
   802.1D.2004  P.171  17.24 Port Protocol Migration state machine
   802.1Q.2005  P.180  13.29 Port Protocol Migration state machine
*/
uint32 mstp_sm_ppm_state_table[PPM_EVENT_MAX][PPM_STATE_MAX] = 
{
  /* PPM_CHECKING_RSTP  PPM_SELECTING_STP   PPM_SENSING */
    {PPM_CHECKING_RSTP, PPM_CHECKING_RSTP,  PPM_CHECKING_RSTP},     /* PPM_EVT_BEGIN */
    {PPM_STATE_MAX,     PPM_SENSING,        PPM_CHECKING_RSTP},     /* PPM_EVT_MCHECK */
    {PPM_STATE_MAX,     PPM_STATE_MAX,      PPM_SELECTING_STP},     /* PPM_EVT_SENDRSTP_RCVDSTP */
    {PPM_STATE_MAX,     PPM_STATE_MAX,      PPM_CHECKING_RSTP},     /* PPM_EVT_RSTPVERSION_NOTSENDRSTP_RCVDRSTP */
};

static int32
_mstp_sm_PPM_TRIGGER_EVENT(tbl_mstp_port_t *p_db_port)
{
    if (PPM_SENSING == p_db_port->ppm_state)
    {        
        if (p_db_port->mcheck)
        {
            mstp_sm_ppm_process_event(p_db_port, PPM_EVT_MCHECK);
        }
    }
    else if (PPM_SELECTING_STP == p_db_port->ppm_state)
    {
        if (p_db_port->mcheck)
        {
            mstp_sm_ppm_process_event(p_db_port, PPM_EVT_MCHECK);
        }
    }
    
    return PM_E_NONE;
}

/*
mcheck = FALSE;
sendRSTP = (rstpVersion);
mdelayWhile = Migrate Time;
*/
static int32
_mstp_sm_PPM_CHECKING_RSTP(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[PPM] %s enter CHECKING_RSTP", ifname_ext);

    p_db_port->mcheck = FALSE;
    p_db_port->sendRstp = p_db_port->rstpVersion;
    CTC_TASK_STOP_TIMER(p_db_port->mdelayWhile_timer);
    p_db_port->mdelayWhile_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _mstp_sm_mdelayWhile_timer_fn, p_db_port, 
        MSTP_TIMER_DEF_MIGRATE_TIME);

    p_db_port->ppm_state = PPM_CHECKING_RSTP;

    return PM_E_NONE;
}

/*
sendRSTP = FALSE;
mdelayWhile = Migrate Time;
*/
static int32
_mstp_sm_PPM_SELECTING_STP(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[PPM] %s enter SELECTING_STP", ifname_ext);
    
    p_db_port->sendRstp = FALSE;
    CTC_TASK_STOP_TIMER(p_db_port->mdelayWhile_timer);
    p_db_port->mdelayWhile_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _mstp_sm_mdelayWhile_timer_fn, p_db_port, 
        MSTP_TIMER_DEF_MIGRATE_TIME);

    p_db_port->ppm_state = PPM_SELECTING_STP;

    return PM_E_NONE;
}

int32
mstp_sm_port_do_mcheck(tbl_mstp_port_t *p_db_port)
{
    p_db_port->rcvdMSTP = p_db_port->rcvdRSTP = p_db_port->rcvdSTP = FALSE;
    
    return PM_E_NONE;
}

/*
rcvdRSTP = rcvdSTP = FALSE;
*/
static int32
_mstp_sm_PPM_SENSING(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[PPM] %s enter PPM_SENSING", ifname_ext);
    
    mstp_sm_port_do_mcheck(p_db_port);
    
    p_db_port->ppm_state = PPM_SENSING;

    return PM_E_NONE;
}

/* 
 Figure 17-15  Port protocol migration state machine
*/
int32
mstp_sm_ppm_process_event(tbl_mstp_port_t *p_db_port, ppm_event_t event)
{
    ppm_state_t next_state = 0;
    char ifname_ext[IFNAME_EXT_SIZE];

    if (event >= PPM_EVENT_MAX)
    {
        return PM_E_FAIL;
    }

    IFNAME_ETH2FULL(p_db_port->key.name, ifname_ext);
    next_state =  mstp_sm_ppm_state_table[event][p_db_port->ppm_state];
    MSTP_DEBUG(STP_PROTO_CIST, "[%s] Interface %s recv event: %s, state: %s -> %s", 
            "PPM",
            ifname_ext,
            cdb_enum_val2str(ppm_event_strs, PPM_EVENT_MAX, event),
            cdb_enum_val2str(ppm_state_strs, PPM_STATE_MAX, p_db_port->ppm_state),
            cdb_enum_val2str(ppm_state_strs, PPM_STATE_MAX, next_state));

    switch (next_state)
    {
    case PPM_CHECKING_RSTP:
        _mstp_sm_PPM_CHECKING_RSTP(p_db_port, ifname_ext);
        break;
    case PPM_SELECTING_STP:
        _mstp_sm_PPM_SELECTING_STP(p_db_port, ifname_ext);
        break;
    case PPM_SENSING:
        _mstp_sm_PPM_SENSING(p_db_port, ifname_ext);
        break;
    default:
        break;
    }

    _mstp_sm_PPM_TRIGGER_EVENT(p_db_port);

    return PM_E_NONE;
}

#define _17_25__13_30__BDM_Bridge_Detection_Machine_

/* BDM State (Bridge Detection Machine)
   802.1D.2004  P.171  17.25 Bridge Detection state machine
   802.1Q.2005  P.180  13.30 Bridge Detection state machine
*/
uint32 mstp_sm_bdm_state_table[BDM_EVENT_MAX][BDM_STATE_MAX] = 
{
  /* BDM_NOT_EDGE       BDM_EDGE */
    {BDM_EDGE,          BDM_STATE_MAX},     /* BDM_EVT_BEGIN_ADMINEDGE */
    {BDM_NOT_EDGE,      BDM_NOT_EDGE},      /* BDM_EVT_BEGIN_NOTADMINEDGE */
    {BDM_EDGE,          BDM_EDGE},          /* BDM_EVT_EDGEDELAY_TIMEOUT_EDGE */
    {BDM_NOT_EDGE,      BDM_NOT_EDGE},      /* BDM_EVT_RECV_BPDU */
};

int32
mstp_sm_BDM_EDGE_update(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    /* disabled port should not enter DESIGNATED port */
    if (FALSE == p_db_port->portEnabled)
    {
        return PM_E_NONE;
    }

    if (p_db_port->operEdge)
    {
        /* modified by liwh for bug 37747, 2016-09-06 
           if port is not designated, for example, rootport/alternate
           they shouldn't turn to designated */
        if (ROLE_DESIGNATED == p_db_port->role)
        /* liwh end */
        {
            if (FALSE == p_db_port->learn)
            {
                mstp_sm_pst_process_event(p_db_port, PST_EVT_LEARN);
                p_db_port->learn = TRUE;
            }
    
            if (FALSE == p_db_port->forward)
            {
                mstp_sm_pst_process_event(p_db_port, PST_EVT_FORWARD);
                p_db_port->forward = TRUE;
            }
            CTC_TASK_STOP_TIMER(p_db_port->fdWhile_timer);
            p_db_port->prt_state = PRT_DESIGNATED_FORWARD;
    
            _mstp_sm_PRT_DESIGNATED_PORT(p_db_port, ifname_ext);
        }

        if (TCM_ACTIVE == p_db_port->tcm_state)
        {
            mstp_sm_tcm_process_event(p_db_port, TCM_EVT_OPEREDGE);
        }
    }
    /* modified by liwh for bug 37747, 2016-09-06 
       if port is not designated, for example, rootport/alternate
           they shouldn't turn to designated */
    /*else
    {
        _mstp_sm_PRT_DESIGNATED_DISCARD(p_db_port, ifname_ext);
        _mstp_sm_PRT_DESIGNATED_PORT(p_db_port, ifname_ext);
    }*/
    /* liwh end */

    return PM_E_NONE;
}

/*
operEdge = TRUE;
*/
static int32
_mstp_sm_BDM_EDGE(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[BDM] %s enter EDGE", ifname_ext);
    
    p_db_port->operEdge = TRUE;
    p_db_port->bdm_state = BDM_EDGE;

    mstp_sm_BDM_EDGE_update(p_db_port, ifname_ext);
    return PM_E_NONE;
}

/*
operEdge = FALSE;
*/
static int32
_mstp_sm_BDM_NOT_EDGE(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[BDM] %s enter NOT_EDGE", ifname_ext);

    if (p_db_port->operEdge && p_db_port->forward 
     && (ROLE_ROOTPORT == p_db_port->role || ROLE_DESIGNATED == p_db_port->role))
    {
        mstp_sm_tcm_process_event(p_db_port, TCM_EVT_FORWARDNOTOPEREDGE);
    }

    p_db_port->operEdge = FALSE;
    p_db_port->bdm_state = BDM_NOT_EDGE;
    
    mstp_sm_BDM_EDGE_update(p_db_port, ifname_ext);
    return PM_E_NONE;
}

/* 
 Figure 17-16  Bridge Detection state machine
*/
int32
mstp_sm_bdm_process_event(tbl_mstp_port_t *p_db_port, bdm_event_t event)
{
    bdm_state_t next_state = 0;
    char ifname_ext[IFNAME_EXT_SIZE];
    
    if (event >= BDM_EVENT_MAX)
    {
        return PM_E_FAIL;
    }

    IFNAME_ETH2FULL(p_db_port->key.name, ifname_ext);
    next_state =  mstp_sm_bdm_state_table[event][p_db_port->bdm_state];
    MSTP_DEBUG(STP_PROTO_CIST, "[%s] Interface %s recv event: %s, state: %s -> %s", 
            "BDM",
            ifname_ext,
            cdb_enum_val2str(bdm_event_strs, BDM_EVENT_MAX, event),
            cdb_enum_val2str(bdm_state_strs, BDM_STATE_MAX, p_db_port->bdm_state),
            cdb_enum_val2str(bdm_state_strs, BDM_STATE_MAX, next_state));

    switch (next_state)
    {
    case BDM_EDGE:
        _mstp_sm_BDM_EDGE(p_db_port, ifname_ext);
        break;
    case BDM_NOT_EDGE:
        _mstp_sm_BDM_NOT_EDGE(p_db_port, ifname_ext);
        break;
    default:
        break;
    }

    return PM_E_NONE;
}

#define _17_26__13_31__PTX_Port_Transmit_Machine_

/* PTX State (Port Transmit)
   802.1D.2004  P.172  17.26 Port Transmit state machine
   802.1Q.2005  P.180  13.31 Port Transmit state machine
*/
uint32 mstp_sm_ptx_state_table[PTX_EVENT_MAX][PTX_STATE_MAX] = 
{
  /* PTX_TRANSMIT_INIT  PTX_TRANSMIT_PERIODIC PTX_IDLE                PTX_TRANSMIT_RSTP  PTX_TRANSMIT_TCN   PTX_TRANSMIT_CONFIG */
    {PTX_TRANSMIT_INIT, PTX_TRANSMIT_INIT,    PTX_TRANSMIT_INIT,      PTX_TRANSMIT_INIT, PTX_TRANSMIT_INIT, PTX_TRANSMIT_INIT}, /* PTX_EVT_BEGIN */
    {PTX_STATE_MAX,     PTX_STATE_MAX,        PTX_TRANSMIT_PERIODIC,  PTX_STATE_MAX,     PTX_STATE_MAX,     PTX_STATE_MAX},     /* PTX_EVT_HELLOWHENZERO */
    {PTX_STATE_MAX,     PTX_STATE_MAX,        PTX_TRANSMIT_RSTP,      PTX_STATE_MAX,     PTX_STATE_MAX,     PTX_STATE_MAX},     /* PTX_EVT_SENDRSTP */
    {PTX_STATE_MAX,     PTX_STATE_MAX,        PTX_TRANSMIT_TCN,       PTX_STATE_MAX,     PTX_STATE_MAX,     PTX_STATE_MAX},     /* PTX_EVT_SENDTCN */
    {PTX_STATE_MAX,     PTX_STATE_MAX,        PTX_TRANSMIT_CONFIG,    PTX_STATE_MAX,     PTX_STATE_MAX,     PTX_STATE_MAX},     /* PTX_EVT_SENDCONFIG */
};

int32
mstp_sm_send_bpdu(tbl_mstp_port_t *p_db_port)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    char ifname_ext[IFNAME_EXT_SIZE];
    mstp_bpdu_role_t bpdu_role = BPDU_ROLE_UNKNOWN;

    if (p_db_port->txCount <= p_db_glb->txHoldCount)
    {
        if (p_db_port->sendRstp)
        {
            bpdu_role = mstp_pdu_get_bpdu_role(p_db_port);
            /* Added by kcao 2016-01-03 for bug 42382, should not sent BPDU with unknown role */
            if (BPDU_ROLE_UNKNOWN != bpdu_role)
            {
                mstp_sm_ptx_process_event(p_db_port, PTX_EVT_SENDRSTP);
            }
        }
        else
        {
            if (ROLE_ROOTPORT == p_db_port->role)
            {
                mstp_sm_ptx_process_event(p_db_port, PTX_EVT_SENDTCN);
            }
            else if (ROLE_DESIGNATED == p_db_port->role)
            {
                mstp_sm_ptx_process_event(p_db_port, PTX_EVT_SENDCONFIG);
            }
        }
    }
    else
    {
        IFNAME_ETH2FULL(p_db_port->key.name, ifname_ext);
        MSTP_DEBUG(STP_PROTO_CIST, "Interface %s txCount %u exceeds txHoldCount %u", ifname_ext, p_db_port->txCount, p_db_glb->txHoldCount);
    }

    return PM_E_NONE;
}

/*
helloWhen = HelloTime;
*/
static int32
_mstp_sm_PTX_IDLE(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[PTX] %s enter IDLE", ifname_ext);
    
    CTC_TASK_STOP_TIMER(p_db_port->helloWhen_timer);
    p_db_port->helloWhen_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _mstp_sm_helloWhen_timer_fn, p_db_port, 
        p_db_port->port_helloTime);

    p_db_port->ptx_state = PTX_IDLE;

    /* Added by kcao to fix send BPDU with flag byte is 0x00 */
    if (ROLE_DISABLED == p_db_port->role)
    {
        return PM_E_NONE;
    }

    if ((p_db_port->newInfo || p_db_port->newInfoMsti))
    {
        mstp_sm_send_bpdu(p_db_port);
    }

    return PM_E_NONE;
}

/*
newInfo = newInfoMsti = TRUE;
txCount = 0;
*/
static int32
_mstp_sm_PTX_TRANSMIT_INIT(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[PTX] %s enter TRANSMIT_INIT", ifname_ext);
    
    p_db_port->newInfo = TRUE;
    p_db_port->newInfoMsti = TRUE;
    p_db_port->txCount = 0;
    
    p_db_port->ptx_state = PTX_TRANSMIT_INIT;
    return PM_E_NONE;
}

/* 13.25.9 mstiDesignatedOrTCpropagatingRootPort
 * a) DesignatedPort; or
 * b) RootPort, and the instance for the given MSTI and Port of the tcWhile timer is not zero.
 */
uint32
mstiDesignatedOrTCpropagatingRootPort(tbl_mstp_port_t *p_db_port)
{
    tbl_msti_port_t *p_db_msti_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    
    CTCLIB_SLIST_LOOP(p_db_port->msti_list.obj_list, p_db_msti_port, listnode)
    {
        if (ROLE_DESIGNATED == p_db_msti_port->role)
        {
            return TRUE;
        }
        else if (ROLE_ROOTPORT == p_db_msti_port->role)
        {
            if (p_db_msti_port->tcWhile_timer)
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

/*
newInfo = newInfo || (DesignatedPort ||
(RootPort && (tcWhile !=0)));
*/
static int32
_mstp_sm_PTX_TRANSMIT_PERIODIC(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    uint32 state = FALSE;
    
    MSTP_DEBUG(STP_PROTO_CIST, "[PTX] %s enter TRANSMIT_PERIODIC", ifname_ext);

    p_db_port->newInfo = p_db_port->newInfo ||
        (ROLE_DESIGNATED == p_db_port->role || (ROLE_ROOTPORT == p_db_port->role && (p_db_port->tcWhile_timer)));

    state = mstiDesignatedOrTCpropagatingRootPort(p_db_port);
    p_db_port->newInfoMsti = p_db_port->newInfoMsti || state;

    p_db_port->ptx_state = PTX_TRANSMIT_PERIODIC;

    return PM_E_NONE;
}

/*
newInfo = FALSE; txConfig(); txCount +=1;
tcAck = FALSE;
*/
static int32
_mstp_sm_PTX_TRANSMIT_CONFIG(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    int32 rc = PM_E_NONE;
    
    MSTP_DEBUG(STP_PROTO_CIST, "[PTX] %s enter TRANSMIT_CONFIG", ifname_ext);
    
    p_db_port->newInfo = FALSE;
    p_db_port->newInfoMsti = FALSE;
    rc = mstp_pdu_tx_bpdu(p_db_port, FALSE);
    if (PM_E_NONE == rc)
    {
        p_db_port->txCount++;
        p_db_port->tx_bpdu_count++;
    }
    p_db_port->tcAck = FALSE;
    p_db_port->ptx_state = PTX_TRANSMIT_CONFIG;    
    return PM_E_NONE;
}

/*
newInfo = FALSE; txTcn(); txCount +=1;
*/
static int32
_mstp_sm_PTX_TRANSMIT_TCN(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    int32 rc = PM_E_NONE;
    
    MSTP_DEBUG(STP_PROTO_CIST, "[PTX] %s enter TRANSMIT_TCN", ifname_ext);
    
    p_db_port->newInfo = FALSE;
    p_db_port->newInfoMsti = FALSE;
    rc = mstp_pdu_tx_tcn(p_db_port);
    if (PM_E_NONE == rc)
    {
        p_db_port->txCount++;
        p_db_port->tx_bpdu_count++;
    }
    p_db_port->ptx_state = PTX_TRANSMIT_TCN;
    return PM_E_NONE;
}

/*
newInfo = FALSE; txRstp(); txCount +=1;
tcAck = FALSE;
*/
static int32
_mstp_sm_PTX_TRANSMIT_RSTP(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    int32 rc = PM_E_NONE;
    
    MSTP_DEBUG(STP_PROTO_CIST, "[PTX] %s enter TRANSMIT_RSTP", ifname_ext);
    
    p_db_port->newInfo = FALSE;
    p_db_port->newInfoMsti = FALSE;
    rc = mstp_pdu_tx_bpdu(p_db_port, TRUE);
    if (PM_E_NONE == rc)
    {
        p_db_port->txCount++;
        p_db_port->tx_bpdu_count++;
    }
    p_db_port->tcAck = FALSE;
    p_db_port->ptx_state = PTX_TRANSMIT_RSTP;
    return PM_E_NONE;
}

/* 
 Figure 17-17   Port Transmit state machine
*/
int32
mstp_sm_ptx_process_event(tbl_mstp_port_t *p_db_port, ptx_event_t event)
{
    ptx_state_t next_state = 0;
    char ifname_ext[IFNAME_EXT_SIZE];

    if (event >= PTX_EVENT_MAX)
    {
        return PM_E_FAIL;
    }

    IFNAME_ETH2FULL(p_db_port->key.name, ifname_ext);
    next_state =  mstp_sm_ptx_state_table[event][p_db_port->ptx_state];
    MSTP_DEBUG(STP_PROTO_CIST, "[%s] Interface %s recv event: %s, state: %s -> %s", 
            "PTX",
            ifname_ext,
            cdb_enum_val2str(ptx_event_strs, PTX_EVENT_MAX, event),
            cdb_enum_val2str(ptx_state_strs, PTX_STATE_MAX, p_db_port->ptx_state),
            cdb_enum_val2str(ptx_state_strs, PTX_STATE_MAX, next_state));

    switch (next_state)
    {
    case PTX_TRANSMIT_INIT:
        _mstp_sm_PTX_TRANSMIT_INIT(p_db_port, ifname_ext);
        _mstp_sm_PTX_IDLE(p_db_port, ifname_ext);
        break;
    case PTX_TRANSMIT_PERIODIC:
        _mstp_sm_PTX_TRANSMIT_PERIODIC(p_db_port, ifname_ext);
        _mstp_sm_PTX_IDLE(p_db_port, ifname_ext);
        break;
    case PTX_IDLE:
        _mstp_sm_PTX_IDLE(p_db_port, ifname_ext);
        break;
    case PTX_TRANSMIT_RSTP:
        _mstp_sm_PTX_TRANSMIT_RSTP(p_db_port, ifname_ext);
        _mstp_sm_PTX_IDLE(p_db_port, ifname_ext);
        break;
    case PTX_TRANSMIT_TCN:
        _mstp_sm_PTX_TRANSMIT_TCN(p_db_port, ifname_ext);
        _mstp_sm_PTX_IDLE(p_db_port, ifname_ext);
        break;
    case PTX_TRANSMIT_CONFIG:
        _mstp_sm_PTX_TRANSMIT_CONFIG(p_db_port, ifname_ext);
        _mstp_sm_PTX_IDLE(p_db_port, ifname_ext);
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
uint32 mstp_sm_pim_state_table[PIM_EVENT_MAX][PIM_STATE_MAX] = 
{
  /* PIM_DISABLED    PIM_AGED        PIM_UPDATE      PIM_CURRENT             PIM_OTHER       PIM_NOT_DESIGNATED PIM_INFERIOR_DESIGNATED PIM_REPEATED_DESIGNATED PIM_SUPERIOR_DESIGNATED PIM_RECEIVE */
    {PIM_DISABLED,   PIM_DISABLED,   PIM_DISABLED,   PIM_DISABLED,           PIM_DISABLED,   PIM_DISABLED,      PIM_DISABLED,           PIM_DISABLED,           PIM_DISABLED,           PIM_DISABLED },
    {PIM_DISABLED,   PIM_STATE_MAX,  PIM_STATE_MAX,  PIM_STATE_MAX,          PIM_STATE_MAX,  PIM_STATE_MAX,     PIM_STATE_MAX,          PIM_STATE_MAX,          PIM_STATE_MAX,          PIM_STATE_MAX },
    {PIM_STATE_MAX,  PIM_UPDATE,     PIM_UPDATE,     PIM_UPDATE,             PIM_UPDATE,     PIM_STATE_MAX,     PIM_STATE_MAX,          PIM_STATE_MAX,          PIM_STATE_MAX,          PIM_STATE_MAX },
    {PIM_STATE_MAX,  PIM_STATE_MAX,  PIM_AGED,       PIM_AGED,               PIM_AGED,       PIM_STATE_MAX,     PIM_STATE_MAX,          PIM_STATE_MAX,          PIM_STATE_MAX,          PIM_STATE_MAX },
    {PIM_AGED,       PIM_STATE_MAX,  PIM_STATE_MAX,  PIM_STATE_MAX,          PIM_STATE_MAX,  PIM_STATE_MAX,     PIM_STATE_MAX,          PIM_STATE_MAX,          PIM_STATE_MAX,          PIM_STATE_MAX },
    {PIM_STATE_MAX,  PIM_RECEIVE,    PIM_RECEIVE,    PIM_RECEIVE,            PIM_RECEIVE,    PIM_RECEIVE,       PIM_RECEIVE,            PIM_RECEIVE,            PIM_RECEIVE,            PIM_STATE_MAX },
};

/* compare priority of port and port designated */
int32
comparePriority_port_designated(tbl_mstp_port_t *p_db_port)
{
    int32 root_cmp = 0;
    int32 rpc_cmp = 0;
    int32 reg_root_cmp = 0;
    int32 int_rpc_cmp = 0;
    int32 designated_cmp = 0;
    int32 designated_port_cmp = 0;
    bpdu_t *p_bpdu = p_db_port->bpdu;
    uint32 bpdu_version = p_db_port->forceVersion;

    /* Added by kcao 2016-12-27 for bug 42334, fix bpdu_version 2 problem */
    if (p_bpdu)
    {
        bpdu_version = p_bpdu->version;
    }
    else
    {
        bpdu_version = p_db_port->forceVersion;
    }
    
    root_cmp = sal_memcmp(&p_db_port->port_rootId, &p_db_port->designated_rootId, sizeof(mstp_brg_id_t));

    if ((BPDU_VERSION_MSTP == p_db_port->forceVersion) && (BPDU_VERSION_MSTP == bpdu_version))
    {
        if (p_db_port->rcvdInternal)
        {
            rpc_cmp = p_db_port->port_rootPathCost - p_db_port->designated_rootPathCost;
        }
        else
        {
            rpc_cmp = p_db_port->port_rootPathCost - p_db_port->designated_rootPathCost;
        }
        reg_root_cmp = sal_memcmp(&p_db_port->port_regionRootId, &p_db_port->designated_regionRootId, sizeof(mstp_brg_id_t));
        int_rpc_cmp = p_db_port->port_intRootPathCost - p_db_port->designated_intRootPathCost;
    }
    else
    {
        rpc_cmp = p_db_port->port_rootPathCost - p_db_port->designated_rootPathCost;
        reg_root_cmp = 0;
        int_rpc_cmp = 0;
    }    
    designated_cmp = sal_memcmp(&p_db_port->port_designatedBridgeId, &p_db_port->designated_designatedBridgeId, sizeof(mstp_brg_id_t));
    designated_port_cmp = p_db_port->port_designatedPortId - p_db_port->designated_designatedPortId;

    if ((root_cmp < 0)
     || ((root_cmp == 0) && (rpc_cmp < 0))
     || ((root_cmp == 0) && (rpc_cmp == 0) && (reg_root_cmp < 0))
     || ((root_cmp == 0) && (rpc_cmp == 0) && (reg_root_cmp == 0) && (int_rpc_cmp < 0))
     || ((root_cmp == 0) && (rpc_cmp == 0) && (reg_root_cmp == 0) && (int_rpc_cmp == 0) && (designated_cmp < 0))
     || ((root_cmp == 0) && (rpc_cmp == 0) && (reg_root_cmp == 0) && (int_rpc_cmp == 0) && (designated_cmp == 0) && (designated_port_cmp < 0)))
    {
        return -1;
    }
    else if ((root_cmp == 0) && (rpc_cmp == 0) && (reg_root_cmp == 0) && (int_rpc_cmp == 0) && (designated_cmp == 0) && (designated_port_cmp == 0))
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
compareTimer_port_root(tbl_mstp_port_t *p_db_port, tbl_mstp_port_t *p_db_rootport)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    
    if (NULL == p_db_rootport)
    {
        if (p_db_port->port_messageAge == p_db_glb->root_messageAge
         && p_db_port->port_maxAge == p_db_glb->root_maxAge
         && p_db_port->port_fwdDelay == p_db_glb->root_fwdDelay
         && p_db_port->port_helloTime == p_db_glb->root_helloTime)
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

/* compare priority of bpdu and port */
int32
comparePriority_msg_port(bpdu_t *bpdu, tbl_mstp_port_t *p_db_port)
{
    int32 root_cmp = 0;
    int32 rpc_cmp = 0;
    int32 regRoot_cmp = 0;
    int32 intRpc_cmp = 0;
    int32 designated_cmp = 0;
    int32 designated_port_cmp = 0;
    
    root_cmp = sal_memcmp(&bpdu->root_id, &p_db_port->port_rootId, sizeof(mstp_brg_id_t));
    rpc_cmp = bpdu->external_rpc - p_db_port->port_rootPathCost;
    if ((BPDU_VERSION_MSTP == p_db_port->forceVersion) && (BPDU_VERSION_MSTP == bpdu->version) && (p_db_port->rcvdInternal))
    {
        regRoot_cmp = sal_memcmp(&bpdu->reg_root_id, &p_db_port->port_regionRootId, sizeof(mstp_brg_id_t));
        intRpc_cmp = bpdu->internal_rpc - p_db_port->port_intRootPathCost;
    }
    else
    {
        regRoot_cmp = 0;
        intRpc_cmp = 0;
    }
    designated_cmp = sal_memcmp(&bpdu->bridge_id, &p_db_port->port_designatedBridgeId, sizeof(mstp_brg_id_t));
    designated_port_cmp = bpdu->port_id - p_db_port->port_designatedPortId;

    if ((root_cmp < 0)
     || ((root_cmp == 0) && (rpc_cmp < 0))
     || ((root_cmp == 0) && (rpc_cmp == 0) && (regRoot_cmp < 0))
     || ((root_cmp == 0) && (rpc_cmp == 0) && (regRoot_cmp == 0) && (intRpc_cmp < 0))
     || ((root_cmp == 0) && (rpc_cmp == 0) && (regRoot_cmp == 0) && (intRpc_cmp == 0) && (designated_cmp < 0))
     || ((root_cmp == 0) && (rpc_cmp == 0) && (regRoot_cmp == 0) && (intRpc_cmp == 0) && (designated_cmp == 0) && (designated_port_cmp < 0)))
    {
        return -1;
    }
    else if ((root_cmp == 0) && (rpc_cmp == 0) && (regRoot_cmp == 0) && (intRpc_cmp == 0) && (designated_cmp == 0) && (designated_port_cmp == 0))
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
compareTimer_msg_port(bpdu_t *bpdu, tbl_mstp_port_t *p_db_port)
{
    if (bpdu->messageAge == p_db_port->port_messageAge
     && bpdu->maxAge == p_db_port->port_maxAge
     && bpdu->fwdDelay == p_db_port->port_fwdDelay
     && bpdu->helloTime == p_db_port->port_helloTime)
    {
        return 0;
    }

    return 1;
}

static uint32 
_betterOrSameInfoRcvd(tbl_mstp_port_t *p_db_port)
{
    bpdu_t *bpdu = (bpdu_t*)p_db_port->bpdu;
    int32 result = 0;

    result = comparePriority_msg_port(bpdu, p_db_port);
    if (result <= 0)
    {
        return TRUE;
    }
  
    return FALSE;
}

static uint32 
_betterOrSameInfoMine(tbl_mstp_port_t *p_db_port)
{
    int32 root_cmp = 0;
    int32 rpc_cmp = 0;
    int32 designated_cmp = 0;
    int32 designated_port_cmp = 0;
    
    root_cmp = sal_memcmp(&p_db_port->designated_rootId, &p_db_port->port_rootId, sizeof(mstp_brg_id_t));
    rpc_cmp = p_db_port->designated_rootPathCost - p_db_port->port_rootPathCost;
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
betterOrSameInfo(tbl_mstp_port_t *p_db_port, mstp_info_t newInfoIs)
{
    if (newInfoIs == MSTP_INFO_RECEIVED && newInfoIs == p_db_port->infoIs)
    {
        return _betterOrSameInfoRcvd(p_db_port);
    }
    else if (newInfoIs == MSTP_INFO_MINE && newInfoIs == p_db_port->infoIs)
    {
        return _betterOrSameInfoMine(p_db_port);
    }
    
    return FALSE;
}

/* 17.21.9 recordAgreement() */
int32 
recordAgreement(tbl_mstp_port_t *p_db_port)
{
    bpdu_t *bpdu = (bpdu_t*)p_db_port->bpdu;
    
    if (p_db_port->rstpVersion && p_db_port->operP2PMac)
    {
        if (bpdu->agreement)
        {
            p_db_port->agreed = TRUE;
            p_db_port->proposing = FALSE;

            _mstp_sm_PRT_TRIGGER_EVENT_DESIGNATED(p_db_port);
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
recordDispute(tbl_mstp_port_t *p_db_port)
{
    bpdu_t *bpdu = (bpdu_t*)p_db_port->bpdu;

    if (bpdu->learning)
    {
        /* Deleted by kcao 2019-02-22 for bug 51102, disputed is TRUE will trigger DESIGNATED_LEARNING to DESIGNATED_DISCARDING */
        //p_db_port->disputed = TRUE;
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
recordProposal(tbl_mstp_port_t *p_db_port)
{
    bpdu_t *bpdu = (bpdu_t*)p_db_port->bpdu;
    
    if (bpdu->proposal && ROLE_DESIGNATED == bpdu->port_role)
    {
        p_db_port->proposed = TRUE;
    }
    
    return PM_E_NONE;
}

/*
17.21.12 recordPriority()
Sets the components of the portPriority variable to the values of the corresponding msgPriority components.
17.19.14/13.24.9 msgPriority
For the CIST and a given Port, the CIST Root Identifier, External Root Path Cost, Regional Root Identifier,
Internal Root Path Cost, Designated Bridge Identifier, and Designated Port Identifier components of the
CIST message priority vector conveyed in a received BPDU, as defined in 13.10.
For a given MSTI and Port, the Regional Root Identifier, Internal Root Path Cost, Designated Bridge
Identifier, and Designated Port Identifier components of the MSTI message priority vector, as defined in
13.11 and conveyed in a received BPDU for this MSTI.
refer mstp_cist_record_priority_vector()
*/
int32
recordPriority(tbl_mstp_port_t *p_db_port)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    bpdu_t *bpdu = p_db_port->bpdu;

    sal_memcpy(&p_db_port->port_rootId, &bpdu->root_id, sizeof(mstp_brg_id_t));
    p_db_port->port_rootPathCost = bpdu->external_rpc;

    if (p_db_port->rcvdInternal)
    {
        sal_memcpy(&p_db_port->port_regionRootId, &bpdu->reg_root_id, sizeof(mstp_brg_id_t));
        p_db_port->port_intRootPathCost = bpdu->internal_rpc;
    }
    else
    {
        sal_memcpy(&p_db_port->port_regionRootId, &p_db_glb->bridge_id, sizeof(mstp_brg_id_t));
        p_db_port->port_intRootPathCost = 0;
    }
    sal_memcpy(&p_db_port->port_designatedBridgeId, &bpdu->bridge_id, sizeof(mstp_brg_id_t));
    p_db_port->port_designatedPortId = bpdu->port_id;
    p_db_port->port_bridgePortId = p_db_port->portId;

    return PM_E_NONE;
}

int32
recordHopCount(tbl_mstp_port_t *p_db_port)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    bpdu_t *bpdu = p_db_port->bpdu;
    uint32 new_hopCount = 0;

    if (p_db_port->rcvdInternal)
    {
        if (bpdu->mstp_hops - 1 >= 0)
        {
            new_hopCount = bpdu->mstp_hops - 1;
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
        if (p_db_port == p_db_glb->root_port)
        {
            mstp_api_reselect();
            mstp_api_port_reselect_all_msti(p_db_port);
        }
    }

    return PM_E_NONE;
}

/*
17.21.13 recordTimes()
Sets portTimes¡¯ Message Age, MaxAge, and Forward Delay to the received values held in the messageTimes
parameter. and portTimes¡¯ Hello time to messageTimes¡¯ HelloTime if that is greater than the minimum
specified in the Compatibility Range column of Table 17-1, and to that minimum otherwise.
*/
int32
recordTimes(tbl_mstp_port_t *p_db_port)
{
    bpdu_t *bpdu = (bpdu_t*)p_db_port->bpdu;

    p_db_port->port_messageAge  = bpdu->messageAge;
    p_db_port->port_maxAge      = bpdu->maxAge;
    p_db_port->port_fwdDelay    = bpdu->fwdDelay;
    p_db_port->port_helloTime   = bpdu->helloTime;

    return PM_E_NONE;
}

/*
(infoIs == Received) && (rcvdInfoWhile == 0) && !updtInfo && !rcvdMsg
*/
static void
_mstp_sm_rcvdInfoWhile_timer_fn(void *p_data)
{
    tbl_mstp_port_t *p_db_port = (tbl_mstp_port_t*)p_data;
    char ifname_ext[IFNAME_EXT_SIZE];

    IFNAME_ETH2FULL(p_db_port->key.name, ifname_ext);
    MSTP_DEBUG(STP_TIMER, "Timer rcvdInfoWhile timeout on interface %s", ifname_ext);

    p_db_port->rcvdInfoWhile_timer = NULL;

    /* modified by liwh for bug 37762, 2016-06-21 */
    if (p_db_port->admin_loopguard)  
    {   
        if (ROLE_ROOTPORT == p_db_port->role || ROLE_ALTERNATE == p_db_port->role || ROLE_BACKUP == p_db_port->role)      
        {          
            MSTP_DEBUG(STP_PROTO_CIST, "_mstp_sm_rcvdInfoWhile_timer_fn: could not receive a bpdu on a loop guard enabled "          
                "port %s", ifname_ext);

            p_db_port->oper_loopguard = TRUE;  
            _mstp_sm_PST_DISCARDING(p_db_port, ifname_ext);
        }
    }
    /* liwh end */

    /* modified by liwh for bug 37722, 2016-08-16 */
    newTcWhile(p_db_port);
    /* liwh end */

    if ((MSTP_INFO_RECEIVED == p_db_port->infoIs)
     && (!p_db_port->updtInfo)
     && (!p_db_port->rcvdMsg))
    {
        mstp_sm_pim_process_event(p_db_port, PIM_EVT_RECVINFOWHILEZERO);
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
updtRcvdInfoWhile(tbl_mstp_port_t *p_db_port)
{
    bpdu_t *bpdu = (bpdu_t*)p_db_port->bpdu;

    CTC_TASK_STOP_TIMER(p_db_port->rcvdInfoWhile_timer);
    if (bpdu->messageAge + 1 >= p_db_port->port_maxAge)
    {
        p_db_port->rcvdInfoWhile_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _mstp_sm_rcvdInfoWhile_timer_fn, p_db_port, 
            0);
    }
    else
    {
        p_db_port->rcvdInfoWhile_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _mstp_sm_rcvdInfoWhile_timer_fn, p_db_port, 
            3 * p_db_port->port_helloTime);
    }

    return PM_E_NONE;
}

/* 17.21.17 setTcFlags() */
int32
setTcFlags(tbl_mstp_port_t *p_db_port)
{
    bpdu_t *bpdu = (bpdu_t*)p_db_port->bpdu;
    uint32 has_tc = FALSE;

    p_db_port->rcvdTc = p_db_port->rcvdTcn = FALSE;

    if (BPDU_TYPE_TCN == bpdu->type)
    {
        p_db_port->rcvdTcn = TRUE;
        mstp_sm_tcm_process_event(p_db_port, TCM_EVT_RCVDTCN);
        return PM_E_NONE;
    }
    
    if (BPDU_TYPE_CONFIG == bpdu->type || BPDU_TYPE_RST == bpdu->type)
    {
        if (bpdu->topology_change_ack)
        {
            p_db_port->rcvdTcAck = TRUE;
            mstp_sm_tcm_process_event(p_db_port, TCM_EVT_RCVDTCACK);
        }
        else if (bpdu->topology_change)
        {
            p_db_port->rcvdTc = TRUE;
            mstp_sm_tcm_process_event(p_db_port, TCM_EVT_RCVDTC);
            has_tc = TRUE;
        }
    }

    if (has_tc)
    {
        if (ROLE_ROOTPORT != bpdu->port_role)
        {
            if (ROLE_DESIGNATED == p_db_port->selectedRole)
            {
                if (comparePriority_port_designated(p_db_port) <= 0)
                {
                    mstp_api_reselect();
                }
            }
        }
    }

    return PM_E_NONE;
}

/*
17.21.8 rcvInfo()
*/
mstp_rcvd_info_t 
rcvInfo(tbl_mstp_port_t *p_db_port)
{
    bpdu_t *bpdu = (bpdu_t*)p_db_port->bpdu;
    int32 priority_result = 0;
    int32 timer_result = 0;

    /* if rcvdInternal change, need to update, for case mstp_func_58_xxx */
    if (p_db_port->rcvdInternalChange)
    {
        return MSTP_SUPERIOR_DESIGNATED;
    }
    
    priority_result = comparePriority_msg_port(bpdu, p_db_port);
    timer_result = compareTimer_msg_port(bpdu, p_db_port);
    (void)timer_result;

    /* Added by kcao 2019-02-21 for bug 51052, 51054, 51057 */
    if (((BPDU_VERSION_RSTP <= bpdu->version) && (ROLE_DESIGNATED == bpdu->port_role || ROLE_ROOTPORT == bpdu->port_role|| ROLE_ALTERNATE == bpdu->port_role || ROLE_MASTERPORT == bpdu->port_role))
     || (BPDU_VERSION_STP == bpdu->version))
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
            p_db_port->rcvdInfoUpdateTimers = TRUE;
            return MSTP_REPEATED_DESIGNATED;
        }
        else if ((priority_result == 0)
            && (timer_result == 0))
        {
            if (ROLE_ROOTPORT == bpdu->port_role)
            {
                return MSTP_INFERIOR_DESIGNATED;
            }
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
_mstp_sm_PIM_TRIGGER_EVENT(tbl_mstp_port_t *p_db_port)
{
    if ((PRT_ROOT_PORT == p_db_port->prt_state) || (PRT_ALTERNATE_PORT == p_db_port->prt_state))
    {
        if (p_db_port->proposed && !p_db_port->agree)
        {
            mstp_sm_prt_process_event(p_db_port, PRT_EVT_PROPOSED_NOTAGREE);
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
_mstp_sm_PIM_DISABLED(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[PIM] %s enter DISABLED", ifname_ext);
    
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
_mstp_sm_PIM_AGED(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[PIM] %s enter AGED", ifname_ext);
    
    p_db_port->infoIs = MSTP_INFO_AGED;
    p_db_port->reselect = TRUE; p_db_port->selected = FALSE;
        
    p_db_port->pim_state = PIM_AGED;

    mstp_api_reselect();
    mstp_api_port_reselect_all_msti(p_db_port);

    return PM_E_NONE;
}

/*
*/
static int32
_mstp_sm_PIM_CURRENT(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[PIM] %s enter CURRENT", ifname_ext);
    
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
_mstp_sm_PIM_UPDATE(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[PIM] %s enter UPDATE", ifname_ext);
    
    p_db_port->proposing = p_db_port->proposed = FALSE;

    p_db_port->agreed = p_db_port->agreed && betterOrSameInfo(p_db_port, MSTP_INFO_MINE);
    p_db_port->synced = p_db_port->synced && p_db_port->agreed;

    /* PortPriority = DesignatedPriority; */
    p_db_port->port_rootId              = p_db_port->designated_rootId;
    p_db_port->port_rootPathCost        = p_db_port->designated_rootPathCost;
    p_db_port->port_regionRootId        = p_db_port->designated_regionRootId;
    p_db_port->port_intRootPathCost     = p_db_port->designated_intRootPathCost;
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

    _mstp_sm_PIM_CURRENT(p_db_port, ifname_ext);

    return PM_E_NONE;
}

/*
rcvdMsg = FALSE
*/
static int32
_mstp_sm_PIM_OTHER(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[PIM] %s enter OTHER", ifname_ext);

    /* Added by kcao for bug 42399, other also need to handle TC */
    setTcFlags(p_db_port);
    p_db_port->rcvdMsg = FALSE;

    p_db_port->pim_state = PIM_OTHER;

    _mstp_sm_PIM_CURRENT(p_db_port, ifname_ext);
    return PM_E_NONE;
}

/*
recordAgreement(); setTcFlags();
rcvdMsg = FALSE;
*/
static int32
_mstp_sm_PIM_NOT_DESIGNATED(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[PIM] %s enter NOT_DESIGNATED", ifname_ext);
    
    recordAgreement(p_db_port); 
    setTcFlags(p_db_port);
    p_db_port->rcvdMsg = FALSE;

    p_db_port->pim_state = PIM_NOT_DESIGNATED;

    _mstp_sm_PIM_CURRENT(p_db_port, ifname_ext);
    return PM_E_NONE;
}

static int32
_mstp_sm_update_all_port_rootId(tbl_mstp_port_t *p_db_rootport)
{
    tbl_mstp_port_master_t *p_master = tbl_mstp_port_get_master();
    tbl_mstp_port_t *p_db_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    bpdu_t *bpdu = p_db_rootport->bpdu;

    if (0 == sal_memcmp(bpdu->root_id.address, p_db_rootport->port_rootId.address, MAC_ADDR_LEN))
    {
        /* if rootId.address not changes, but priority changes */
        CTCLIB_SLIST_LOOP(p_master->port_list, p_db_port, listnode)
        {
            if (p_db_port->portEnabled)
            {
                if (p_db_port == p_db_rootport)
                {
                    continue;
                }
                
                if (0 == sal_memcmp(&p_db_port->port_rootId, &p_db_rootport->port_rootId, sizeof(mstp_brg_id_t)))
                {
                    sal_memcpy(&p_db_port->port_rootId, &bpdu->root_id, sizeof(mstp_brg_id_t));
#if 0
                    log_sys(M_MOD_MSTP, E_ERROR, "BBBBB Update %s's port_rootId from root port %s", p_db_port->key.name, p_db_rootport->key.name);
#endif
                }
            }
        }
    }
    else
    {
        /* Modified by kcao for bug 37798, for DUT1 - DUT2 has multiple Alternate port, should update infoIs to MINE */
        CTCLIB_SLIST_LOOP(p_master->port_list, p_db_port, listnode)
        {
            if (p_db_port->portEnabled)
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
_mstp_sm_clear_rootId_for_rx_inferior_bpdu_from_root_port(tbl_mstp_port_t *p_db_port)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    bpdu_t *bpdu = p_db_port->bpdu;
    int32 root_cmp = 0;

    if (p_db_glb->root_port == p_db_port)
    {
        root_cmp = sal_memcmp(&bpdu->root_id, &p_db_glb->root_rootId, sizeof(mstp_brg_id_t));
        if (root_cmp > 0)
        {
            /* for RX inferior BPDU from root port, we need update other port's rootId.address same to rootId.address */
            _mstp_sm_update_all_port_rootId(p_db_port);

            /* for RX inferior BPDU from root port, we think need to clear old rootId immediately, but not by aged */
            recordPriority(p_db_port);
            recordHopCount(p_db_port);
#if 0
            log_sys(M_MOD_MSTP, E_ERROR, "BBBBB Rx inferior BPDU root_cmp %u new (%04X-%02X%02X.%02X%02X.%02X%02X) old (%04X-%02X%02X.%02X%02X.%02X%02X) from root port %s, reselect", 
                root_cmp,
                bpdu->root_id.priority,
                bpdu->root_id.address[0], bpdu->root_id.address[1], bpdu->root_id.address[2], 
                bpdu->root_id.address[3], bpdu->root_id.address[4], bpdu->root_id.address[5], 
                p_db_glb->root_rootId.priority,
                p_db_glb->root_rootId.address[0], p_db_glb->root_rootId.address[1], p_db_glb->root_rootId.address[2], 
                p_db_glb->root_rootId.address[3], p_db_glb->root_rootId.address[4], p_db_glb->root_rootId.address[5], 
                p_db_port->key.name);
#endif
            mstp_api_reselect();
            mstp_api_port_reselect_all_msti(p_db_port);
        }
    }
    else if (ROLE_ALTERNATE == p_db_port->role)
    {
        /* Added for bug 45523, ROLE_ALTERNATE to ROLE_DESIGNATED, refer to MSG_TYPE_INFERIOR_DESIGNATED */
        p_db_port->selectedRole = ROLE_DESIGNATED;
        mstp_sm_prt_process_event(p_db_port, PRT_EVT_ROLE_DESIGNATED);
        mstp_sm_prt_process_event(p_db_port, PRT_EVT_PROPOSING);
    }

    return PM_E_NONE;
}

/*
recordDispute();
rcvdMsg = FALSE;
*/
static int32
_mstp_sm_PIM_INFERIOR_DESIGNATED(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    bpdu_t *bpdu = (bpdu_t*)p_db_port->bpdu;
    
    MSTP_DEBUG(STP_PROTO_CIST, "[PIM] %s enter INFERIOR_DESIGNATED", ifname_ext);

    /* modified by liwh for bug 36699/37625/37767, 2016-05-11 */
    if (p_db_port->oper_rootguard)
    {
        p_db_port->oper_rootguard = FALSE;
        p_db_port->prt_state = PRT_DESIGNATED_PORT;
    }
    /* liwh end */

    /* Added by kcao 2019-02-21 for bug 51045, if port link down trigger other ports to send TC */
    if (bpdu->topology_change)
    {
        mstp_api_flush_fdb(p_db_port);
    }
    
    recordDispute(p_db_port);
    p_db_port->rcvdMsg = FALSE;
    p_db_port->pim_state = PIM_INFERIOR_DESIGNATED;

    _mstp_sm_PIM_CURRENT(p_db_port, ifname_ext);

    /* Added by kcao for bug 37666 2016-06-16 */
    _mstp_sm_clear_rootId_for_rx_inferior_bpdu_from_root_port(p_db_port);

    return PM_E_NONE;
}

/*
recordProposal(); setTcFlags();
updtRcvdInfoWhile();
rcvdMsg = FALSE;
*/
static int32
_mstp_sm_PIM_REPEATED_DESIGNATED(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[PIM] %s enter REPEATED_DESIGNATED", ifname_ext);
    
    recordProposal(p_db_port);
    setTcFlags(p_db_port);
    updtRcvdInfoWhile(p_db_port);
    recordHopCount(p_db_port);
    p_db_port->rcvdMsg = FALSE;
    p_db_port->pim_state = PIM_REPEATED_DESIGNATED;

    /* Added by kcao 2019-02-26 for bug 51127, update times if changed */
    if (p_db_port->rcvdInfoUpdateTimers)
    {
        recordTimes(p_db_port);
        mstp_api_reselect();
        mstp_api_port_reselect_all_msti(p_db_port);
        p_db_port->rcvdInfoUpdateTimers = FALSE;
    }
    
    _mstp_sm_PIM_CURRENT(p_db_port, ifname_ext);
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
_mstp_sm_PIM_SUPERIOR_DESIGNATED(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    bpdu_t *bpdu = (bpdu_t*)p_db_port->bpdu;
    
    MSTP_DEBUG(STP_PROTO_CIST, "[PIM] %s enter SUPERIOR_DESIGNATED", ifname_ext);

    /* modified by liwh for bug 36699/37625/37767, 2016-05-11 */
    if ((p_db_port->admin_rootguard) && (BPDU_TYPE_TCN != bpdu->type))
    {
        p_db_port->rcvdMsg = FALSE;
        p_db_port->oper_rootguard = TRUE;
        _mstp_sm_PRT_DESIGNATED_DISCARD(p_db_port, ifname_ext);

        _mstp_sm_PIM_CURRENT(p_db_port, ifname_ext);

        return PM_E_NONE;
    }
    /* liwh end */
    
    p_db_port->agreed = p_db_port->proposing = FALSE;
        
    recordProposal(p_db_port);
    setTcFlags(p_db_port);
    p_db_port->agree = p_db_port->agree && betterOrSameInfo(p_db_port, MSTP_INFO_RECEIVED);
    
    recordPriority(p_db_port);
    recordHopCount(p_db_port);
    recordTimes(p_db_port); 
    updtRcvdInfoWhile(p_db_port);
    
    p_db_port->infoIs = MSTP_INFO_RECEIVED; 
    p_db_port->reselect = TRUE; 
    p_db_port->selected = FALSE;
    
    p_db_port->rcvdMsg = FALSE;
    p_db_port->pim_state = PIM_SUPERIOR_DESIGNATED;

    _mstp_sm_PIM_CURRENT(p_db_port, ifname_ext);

    mstp_api_reselect();
    mstp_api_port_reselect_all_msti(p_db_port);
    
    return PM_E_NONE;
}

/* Added by kcao for bug 42217, check  */
int32
mstp_sm_check_hops(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    bpdu_t *p_bpdu = (bpdu_t*)p_db_port->bpdu;
    
    /* Max-hops is in same MST region */
    if (p_db_port->rcvdInternal)
    {
        if (p_bpdu)
        {
            if ((BPDU_VERSION_MSTP == p_bpdu->version) && (0 == p_bpdu->mstp_hops))
            {
                p_db_port->rcvdMsg = FALSE;
                _mstp_sm_PIM_CURRENT(p_db_port, ifname_ext);
                MSTP_DEBUG(STP_PROTO_CIST, "[PIM] %s drops MSTP BPDU for max-hops is zero", ifname_ext);
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
_mstp_sm_PIM_RECEIVE(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    p_db_port->rcvdInfo = rcvInfo(p_db_port);
    p_db_port->pim_state = PIM_RECEIVE;
    int32 rc = PM_E_NONE;

    MSTP_DEBUG(STP_PROTO_CIST, "[PIM] %s enter PIM_RECEIVE, rcvdInfo %u", ifname_ext, p_db_port->rcvdInfo);
//    log_sys(M_MOD_MSTP, E_ERROR, "MSTPDEBUG [PIM] %s enter PIM_RECEIVE, rcvdInfo %u", ifname_ext, p_db_port->rcvdInfo);
    
    switch (p_db_port->rcvdInfo)
    {
    case MSTP_SUPERIOR_DESIGNATED:
        rc = mstp_sm_check_hops(p_db_port, ifname_ext);
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
            _mstp_sm_PIM_SUPERIOR_DESIGNATED(p_db_port, ifname_ext);
        }
        break;
    case MSTP_REPEATED_DESIGNATED:
        rc = mstp_sm_check_hops(p_db_port, ifname_ext);
        if (rc < 0)
        {
            return PM_E_NONE;
        }
        _mstp_sm_PIM_REPEATED_DESIGNATED(p_db_port, ifname_ext);
        break;
    case MSTP_INFERIOR_DESIGNATED:
        _mstp_sm_PIM_INFERIOR_DESIGNATED(p_db_port, ifname_ext);
        break;
    case MSTP_INFERIOR_ROOTALTERNATE:
        _mstp_sm_PIM_NOT_DESIGNATED(p_db_port, ifname_ext);
        break;
    case MSTP_OTHER:
        _mstp_sm_PIM_OTHER(p_db_port, ifname_ext);
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
mstp_sm_pim_process_event(tbl_mstp_port_t *p_db_port, pim_event_t event)
{
    pim_state_t next_state = 0;
    char ifname_ext[IFNAME_EXT_SIZE];

    if (event >= PIM_EVENT_MAX)
    {
        return PM_E_FAIL;
    }

    IFNAME_ETH2FULL(p_db_port->key.name, ifname_ext);
    next_state =  mstp_sm_pim_state_table[event][p_db_port->pim_state];
    MSTP_DEBUG(STP_PROTO_CIST, "[%s] Interface %s recv event: %s, state: %s -> %s", 
            "PIM",
            ifname_ext,
            cdb_enum_val2str(pim_event_strs, PIM_EVENT_MAX, event),
            cdb_enum_val2str(pim_state_strs, PIM_STATE_MAX, p_db_port->pim_state),
            cdb_enum_val2str(pim_state_strs, PIM_STATE_MAX, next_state));

    switch (next_state)
    {
    case PIM_DISABLED:
        _mstp_sm_PIM_DISABLED(p_db_port, ifname_ext);
        break;
    case PIM_AGED:
        _mstp_sm_PIM_AGED(p_db_port, ifname_ext);
        break;
    case PIM_UPDATE:
        _mstp_sm_PIM_UPDATE(p_db_port, ifname_ext);
        break;
    case PIM_CURRENT:
        _mstp_sm_PIM_CURRENT(p_db_port, ifname_ext);
        break;
    /* below 5 states are direct transferred from PIM_RECEIVE */
#if 0
    case PIM_OTHER:
        _mstp_sm_PIM_OTHER(p_db_port, ifname_ext);
        break;
    case PIM_NOT_DESIGNATED:
        _mstp_sm_PIM_NOT_DESIGNATED(p_db_port, ifname_ext);
        break;
    case PIM_INFERIOR_DESIGNATED:
        _mstp_sm_PIM_INFERIOR_DESIGNATED(p_db_port, ifname_ext);
        break;
    case PIM_REPEATED_DESIGNATED:
        _mstp_sm_PIM_REPEATED_DESIGNATED(p_db_port, ifname_ext);
        break;
    case PIM_SUPERIOR_DESIGNATED:
        _mstp_sm_PIM_SUPERIOR_DESIGNATED(p_db_port, ifname_ext);
        break;
#endif
    case PIM_RECEIVE:
        _mstp_sm_PIM_RECEIVE(p_db_port, ifname_ext);
        break;
    default:
        break;
    }

    _mstp_sm_PIM_TRIGGER_EVENT(p_db_port);

    return PM_E_NONE;
}

#define _17_28__13_33__PIM_Port_Role_Selection_Machine_

/* PRS State (Port Role Selection Machine)
   802.1D.2004  P.174  17.28 Port Role Selection state machine
   802.1Q.2005  P.183  13.33 Port Role Selection state machine
*/
uint32 mstp_sm_prs_state_table[PRS_EVENT_MAX][PRS_STATE_MAX] = 
{
  /* PRS_INIT_BRIDGE        PRS_ROLE_SELECTION */
    {PRS_INIT_BRIDGE,       PRS_INIT_BRIDGE},       /* PRS_EVT_BEGIN */
    {PRS_STATE_MAX,         PRS_ROLE_SELECTION},    /* PRS_EVT_RESELECT */
};

/*
17.21.24 updtRoleDisabledTree()
Sets selectedRole to DisabledPort for all Ports of the Bridge.
*/
int32 
updtRoleDisabledTree()
{
    tbl_mstp_port_master_t *p_master = tbl_mstp_port_get_master();
    tbl_mstp_port_t *p_db_port = NULL;
    ctclib_slistnode_t *listnode = NULL;

    CTCLIB_SLIST_LOOP(p_master->port_list, p_db_port, listnode)
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
clearReselectTree()
{
    tbl_mstp_port_master_t *p_master = tbl_mstp_port_get_master();
    tbl_mstp_port_t *p_db_port = NULL;
    ctclib_slistnode_t *listnode = NULL;

    CTCLIB_SLIST_LOOP(p_master->port_list, p_db_port, listnode)
    {
        if (p_db_port->reselect)
        {
            p_db_port->reselect = FALSE;
        }
    }

    return PM_E_NONE;
}

int32 
compareBestPortPriority(mstp_port_priority_vector_t *best_priority, tbl_mstp_port_t *p_db_port, tbl_mstp_port_t *p_db_rootport)
{
    int32 rootId_cmp = 0;
    int32 rootPathCost_cmp = 0;
    int32 regRootId_cmp = 0;
    int32 intRootPathCost_cmp = 0;
    int32 designatedBridgeId_cmp = 0;
    int32 designatedPortId_cmp = 0;
    int32 bridgePortId_cmp = 0;
    uint32 external_rpc_incr = 0;
    uint32 internal_rpc_incr = 0;
    
    if (p_db_port->rcvdInternal)
    {
        external_rpc_incr = 0;
        internal_rpc_incr = p_db_port->portPathCost;
    }
    else
    {
        external_rpc_incr = p_db_port->portPathCost;
        internal_rpc_incr = 0;
    }
    
    rootId_cmp = sal_memcmp(&best_priority->rootId, &p_db_port->port_rootId, sizeof(mstp_brg_id_t));
    if (rootId_cmp != 0)
    {
        return rootId_cmp;
    }

    rootPathCost_cmp = best_priority->rootPathCost - (p_db_port->port_rootPathCost + external_rpc_incr);
    if (rootPathCost_cmp != 0)
    {
        return rootPathCost_cmp;
    }

    if ((BPDU_VERSION_MSTP == p_db_port->forceVersion))
    {
        regRootId_cmp = sal_memcmp(&best_priority->regionRootId, &p_db_port->port_regionRootId, sizeof(mstp_brg_id_t));
        if (regRootId_cmp != 0)
        {
            return regRootId_cmp;
        }

        intRootPathCost_cmp = best_priority->intRootPathCost - (p_db_port->port_intRootPathCost + internal_rpc_incr);
        if (intRootPathCost_cmp != 0)
        {
            return intRootPathCost_cmp;
        }
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
updateBestPortPriority(mstp_port_priority_vector_t *port_priority, tbl_mstp_port_t *p_db_port)
{
    uint32 external_rpc_incr = 0;
    uint32 internal_rpc_incr = 0;

    if (p_db_port->rcvdInternal)
    {
        external_rpc_incr = 0;
        internal_rpc_incr = p_db_port->portPathCost;
    }
    else
    {
        external_rpc_incr = p_db_port->portPathCost;
        internal_rpc_incr = 0;
    }
    sal_memcpy(&port_priority->rootId, &p_db_port->port_rootId, sizeof(mstp_brg_id_t));
    port_priority->rootPathCost = (p_db_port->port_rootPathCost + external_rpc_incr);
    sal_memcpy(&port_priority->regionRootId, &p_db_port->port_regionRootId, sizeof(mstp_brg_id_t));
    port_priority->intRootPathCost = (p_db_port->port_intRootPathCost + internal_rpc_incr);
    sal_memcpy(&port_priority->designatedBridgeId, &p_db_port->port_designatedBridgeId, sizeof(mstp_brg_id_t));
    port_priority->designatedPortId = p_db_port->port_designatedPortId;
    port_priority->bridgePortId = p_db_port->port_bridgePortId;

    return PM_E_NONE;
}

/* 17.18.3/13.23.3 BridgePriority */
int32 
getBridgePriority(mstp_port_priority_vector_t *bridge_priority, tbl_mstp_global_t *p_db_glb)
{
    sal_memcpy(&bridge_priority->rootId, &p_db_glb->bridge_id, sizeof(mstp_brg_id_t));
    bridge_priority->rootPathCost = 0;
    sal_memcpy(&bridge_priority->regionRootId, &p_db_glb->bridge_id, sizeof(mstp_brg_id_t));
    bridge_priority->intRootPathCost = 0;
    sal_memcpy(&bridge_priority->designatedBridgeId, &p_db_glb->bridge_id, sizeof(mstp_brg_id_t));
    bridge_priority->designatedPortId = 0;
    bridge_priority->bridgePortId = 0;

    return PM_E_NONE;
}

/*
17.21.25 updtRolesTree()
*/
int32 
updtRolesTree(uint32 *tx_bpdu)
{
    mstp_port_priority_vector_t bridge_priority;
    mstp_port_priority_vector_t *p_best_priority;
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    tbl_mstp_port_master_t *p_master = tbl_mstp_port_get_master();
    tbl_mstp_port_t *p_db_port = NULL;
    tbl_mstp_port_t *p_db_rootport = NULL;
    ctclib_slistnode_t *listnode = NULL;
    int32 port_priority_cmp = 0;
    uint32 old_is_self_root = FALSE;
    uint32 is_self_root = TRUE;
    uint32 is_rootport_change = FALSE;
    uint32 external_rpc_incr = 0;
    uint32 internal_rpc_incr = 0;
    char ifname_ext[IFNAME_EXT_SIZE];

    old_is_self_root = (p_db_glb->root_port) ?  FALSE : TRUE;

    /*
    a) The root path priority vector for each Port that has a port priority vector (portPriority plus portId;
    17.19.19, 17.19.21), recorded from a received message and not aged out (infoIs == Received)
    */

    /* best_priority is inited with bridge priority vector */
    getBridgePriority(&bridge_priority, p_db_glb);
    p_best_priority = &bridge_priority;

    /* calculate best_priority by compare all port's priority */
    CTCLIB_SLIST_LOOP(p_master->port_list, p_db_port, listnode)
    {
        if (p_db_port->portEnabled && (MSTP_INFO_RECEIVED == p_db_port->infoIs))
        {
            /* whose DesignatedBridgeID Bridge Address component is not equal to that component of 
               the Bridge¡¯s own bridge priority vector */
            if (0 == sal_memcmp(p_db_port->port_designatedBridgeId.address, p_db_glb->bridge_id.address, MAC_ADDR_LEN))
            {
                continue;
            }
            
            port_priority_cmp = compareBestPortPriority(p_best_priority, p_db_port, p_db_rootport);
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
                updateBestPortPriority(p_best_priority, p_db_rootport);
                is_self_root = FALSE;
            }
        }
    }

    if (is_self_root)
    {
        /* clear root port */
        p_db_glb->root_port = NULL;
        p_db_glb->root_portId = 0;
        tbl_mstp_global_set_mstp_global_field(p_db_glb, TBL_MSTP_GLOBAL_FLD_ROOT_PORTID);
        MSTP_DEBUG(STP_PROTO_CIST, "[updtRolesTree] reselect as root bridge");
    }
    else
    {
        if (p_db_rootport != p_db_glb->root_port)
        {
            is_rootport_change = TRUE;
            p_db_glb->root_port = p_db_rootport;
            p_db_glb->root_portId = p_db_rootport->portId;
            tbl_mstp_global_set_mstp_global_field(p_db_glb, TBL_MSTP_GLOBAL_FLD_ROOT_PORTID);
        }
        else
        {
            /* Added by kcao for bug 37739, need update root_portId for port-priority maybe changes */
            if (p_db_glb->root_portId != p_db_rootport->portId)
            {
                p_db_glb->root_portId = p_db_rootport->portId;
                tbl_mstp_global_set_mstp_global_field(p_db_glb, TBL_MSTP_GLOBAL_FLD_ROOT_PORTID);
            }
        }
        IFNAME_ETH2FULL(p_db_rootport->key.name, ifname_ext);
        MSTP_DEBUG(STP_PROTO_CIST, "[updtRolesTree] port %s reselect as root port", ifname_ext);
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
        sal_memcpy(&p_db_glb->root_rootId, &p_db_glb->bridge_id, sizeof(mstp_brg_id_t));
        p_db_glb->root_rootPathCost = 0;
        sal_memcpy(&p_db_glb->root_regionRootId, &p_db_glb->bridge_id, sizeof(mstp_brg_id_t));
        p_db_glb->root_intRootPathCost = 0;
        sal_memcpy(&p_db_glb->root_designatedBridgeId, &p_db_glb->bridge_id, sizeof(mstp_brg_id_t));
        p_db_glb->root_designatedPortId = 0;
        p_db_glb->root_bridgePortId = 0;
        p_db_glb->hopCount = p_db_glb->bridge_maxHops;
    }
    else
    {
        sal_memcpy(&p_db_glb->root_rootId, &p_db_rootport->port_rootId, sizeof(mstp_brg_id_t));
        sal_memcpy(&p_db_glb->root_regionRootId, &p_db_rootport->port_regionRootId, sizeof(mstp_brg_id_t));
        if (p_db_rootport->rcvdInternal)
        {
            external_rpc_incr = 0;
            internal_rpc_incr = p_db_rootport->portPathCost;
            p_db_glb->hopCount = p_db_rootport->hopCount;
        }
        else
        {
            external_rpc_incr = p_db_rootport->portPathCost;
            internal_rpc_incr = 0;
            p_db_glb->hopCount = p_db_glb->bridge_maxHops;
        }
        p_db_glb->root_rootPathCost = p_db_rootport->port_rootPathCost + external_rpc_incr;
        p_db_glb->root_intRootPathCost = p_db_rootport->port_intRootPathCost + internal_rpc_incr;
        sal_memcpy(&p_db_glb->root_designatedBridgeId, &p_db_rootport->port_designatedBridgeId, sizeof(mstp_brg_id_t));
        p_db_glb->root_designatedPortId = p_db_rootport->port_designatedPortId;
        p_db_glb->root_bridgePortId = p_db_rootport->portId;
    }
    
    /*
    c) The Bridge¡¯s rootTimes (17.18.7) parameter, set equal to:
    */
    if (is_self_root)
    {
        /* 1) BridgeTimes (17.18.4), if the chosen root priority vector is the bridge priority vector, otherwise
        */
        p_db_glb->root_messageAge   = 0;
        p_db_glb->root_fwdDelay     = p_db_glb->bridge_fwdDelay;
        p_db_glb->root_helloTime    = p_db_glb->bridge_helloTime;
        p_db_glb->root_maxAge       = p_db_glb->bridge_maxAge;
    }
    else
    {
        /* 2) portTimes (17.19.22) for the port associated with the selected root priority vector, with the
           Message Age component incremented by 1 second and rounded to the nearest whole second. */
        p_db_glb->root_messageAge   = p_db_rootport->port_messageAge;
        p_db_glb->root_fwdDelay     = p_db_rootport->port_fwdDelay;
        p_db_glb->root_helloTime    = p_db_rootport->port_helloTime;
        p_db_glb->root_maxAge       = p_db_rootport->port_maxAge;
        p_db_glb->root_messageAge += 1;
    }

    /*
    d) The first four components of the designated priority vector (designatedPriority, 17.19.4) for each port.
    designated priority vector = {B : 0 : B : QB : QB } if B is better than RD, or
                               = {RD : RPCD + PPCPB : B : QB: QB }if B is worse than RD
    */
    CTCLIB_SLIST_LOOP(p_master->port_list, p_db_port, listnode)
    {
        sal_memcpy(&p_db_port->designated_rootId, &p_db_glb->root_rootId, sizeof(mstp_brg_id_t));
        sal_memcpy(&p_db_port->designated_regionRootId, &p_db_glb->root_regionRootId, sizeof(mstp_brg_id_t));
        if (p_db_port != p_db_rootport)
        {
            p_db_port->designated_rootPathCost = p_db_glb->root_rootPathCost;
            p_db_port->designated_intRootPathCost = p_db_glb->root_intRootPathCost;
            /* Added by kcao 2019-02-21 for bug 51043, update port messageAge */
            p_db_port->port_messageAge = p_db_glb->root_messageAge;
        }
        else
        {
            p_db_port->designated_rootPathCost = p_db_port->port_rootPathCost;
            p_db_port->designated_intRootPathCost = p_db_port->port_intRootPathCost;
        }
        sal_memcpy(&p_db_port->designated_designatedBridgeId, &p_db_glb->bridge_id, sizeof(mstp_brg_id_t));
        p_db_port->designated_designatedPortId = p_db_port->portId;
        p_db_port->designated_bridgePortId = p_db_port->portId;
        p_db_port->hopCount = p_db_glb->hopCount;
    }
    
    /*
    e) The designatedTimes (17.19.5) for each Port, set equal to the value of rootTimes, except for the
    Hello Time component, which is set equal to BridgeTimes¡¯ Hello Time
    */
    CTCLIB_SLIST_LOOP(p_master->port_list, p_db_port, listnode)
    {
        p_db_port->designated_messageAge   = p_db_glb->root_messageAge;
        p_db_port->designated_fwdDelay     = p_db_glb->root_fwdDelay;
        p_db_port->designated_helloTime    = p_db_glb->root_helloTime; // bridge_helloTime;
        p_db_port->designated_maxAge       = p_db_glb->root_maxAge;
    }
    
    CTCLIB_SLIST_LOOP(p_master->port_list, p_db_port, listnode)
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
            if (comparePriority_port_designated(p_db_port) 
             || compareTimer_port_root(p_db_port, p_db_rootport))
            {
                p_db_port->updtInfo = TRUE;
            }

            /* reset agreed */
            p_db_port->agree = FALSE;
            p_db_port->agreed = FALSE;
        }
        else if (MSTP_INFO_RECEIVED == p_db_port->infoIs)
        {
            if (p_db_glb->root_portId != 0 && p_db_glb->root_portId == p_db_port->portId)
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
                if (comparePriority_port_designated(p_db_port) <= 0)
                {
                    /*
                    j) If the port priority vector was received in a Configuration Message and is not aged
                    (infoIs = Received), the root priority vector is not now derived from it, the designated priority vector
                    is not higher than the port priority vector, and the designated bridge and designated port components
                    of the port priority vector do not reflect another port on this bridge, selectedRole is set to
                    AlternatePort and updtInfo is reset.
                    */
                    if (0 != sal_memcmp(&p_db_port->port_designatedBridgeId, &p_db_glb->bridge_id, sizeof(mstp_brg_id_t)))
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

    /* Added for bug 45523, speed up root changes notification */
    if (old_is_self_root != is_self_root)
    {
        /* Added by kcao 2019-02-26 for bug 51125, TX bpdu should after _mstp_sm_PRS_TRIGGER_EVENT() */
        *tx_bpdu = TRUE;
    }

    (void)is_rootport_change;
    return PM_E_NONE;
}

/*
17.21.16 setSelectedTree()
Sets the selected variable TRUE for all Ports of the Bridge if reselect is FALSE for all Ports. If reselect is
TRUE for any Port, this procedure takes no action.
*/
int32 
setSelectedTree()
{
    tbl_mstp_port_master_t *p_master = tbl_mstp_port_get_master();
    tbl_mstp_port_t *p_db_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    uint32 has_reselect = FALSE;
    
    CTCLIB_SLIST_LOOP(p_master->port_list, p_db_port, listnode)
    {
        if (p_db_port->portEnabled && p_db_port->reselect)
        {
            has_reselect = TRUE;
            break;
        }
    }

    if (!has_reselect)
    {
        CTCLIB_SLIST_LOOP(p_master->port_list, p_db_port, listnode)
        {
            if (p_db_port->portEnabled)
            {
                p_db_port->selected = TRUE;
            }
        }
    }
        
    return PM_E_NONE;
}

static int32
_mstp_sm_PRS_TRIGGER_EVENT()
{
    tbl_mstp_port_master_t *p_master = tbl_mstp_port_get_master();
    tbl_mstp_port_t *p_db_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    
    MSTP_DEBUG(STP_PROTO_CIST, "[PRS] enter TRIGGER_EVENT");

    CTCLIB_SLIST_LOOP(p_master->port_list, p_db_port, listnode)
    {
        if (p_db_port->portEnabled)
        {
            if (p_db_port->selected && p_db_port->updtInfo)
            {
                mstp_sm_pim_process_event(p_db_port, PIM_EVT_SELECTEDUPDTINFO);
            }
        }
    }

    CTCLIB_SLIST_LOOP(p_master->port_list, p_db_port, listnode)
    {
        if (p_db_port->portEnabled)
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
                        mstp_sm_tcm_process_event(p_db_port, TCM_EVT_ROLENOTROOTDESIGNATEDPORT);
                    }
                }
            }

            switch (p_db_port->selectedRole)
            {
            case ROLE_DISABLED:
                mstp_sm_prt_process_event(p_db_port, PRT_EVT_ROLE_DISABLED);
                break;
            case ROLE_ROOTPORT:
                mstp_sm_prt_process_event(p_db_port, PRT_EVT_ROLE_ROOTPORT);
                break;
            case ROLE_DESIGNATED:
                /* When first ROLE_DISABLED -> PRT_DESIGNATED_PORT, should enter PRT_DESIGNATED_DISCARD state */
                if (ROLE_DISABLED == p_db_port->role && !p_db_port->operEdge)
                {
                    p_db_port->disputed = TRUE;
                    p_db_port->learn = TRUE;
                    p_db_port->forward = TRUE;
                }
                mstp_sm_prt_process_event(p_db_port, PRT_EVT_ROLE_DESIGNATED);
                break;
            case ROLE_ALTERNATE:
                mstp_sm_prt_process_event(p_db_port, PRT_EVT_ROLE_ALTERNATE);
                break;
            case ROLE_BACKUP:
                mstp_sm_prt_process_event(p_db_port, PRT_EVT_ROLE_BACKUP);
                break;
            default:
                break;
            }
        }
    }

    return PM_E_NONE;
}

/*
updtRoleDisabledTree();
*/
static int32
_mstp_sm_PRS_INIT_BRIDGE(tbl_mstp_global_t *p_db_glb)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[PRS] enter INIT_BRIDGE");
    
    updtRoleDisabledTree();

    p_db_glb->prs_state = PRS_INIT_BRIDGE;
    
    return PM_E_NONE;
}

/*
clearReselectTree();
updtRolesTree();
setSelectedTree();
*/
static int32
_mstp_sm_PRS_ROLE_SELECTION(tbl_mstp_global_t *p_db_glb)
{
    uint32 tx_bpdu = FALSE;
    MSTP_DEBUG(STP_PROTO_CIST, "[PRS] enter ROLE_SELECTION");
    
    clearReselectTree();
    updtRolesTree(&tx_bpdu);
    setSelectedTree();

    p_db_glb->prs_state = PRS_ROLE_SELECTION;
    
    _mstp_sm_PRS_TRIGGER_EVENT();
    if (tx_bpdu)
    {
        mstp_api_all_port_tx_pdu();
    }
    
    return PM_E_NONE;
}

/* 
 Figure 17-17   Port Role Selection machine
*/
int32
mstp_sm_prs_process_event(tbl_mstp_global_t *p_db_glb, prs_event_t event)
{
    prs_state_t next_state = 0;

    if (event >= PRS_EVENT_MAX)
    {
        return PM_E_FAIL;
    }

    next_state =  mstp_sm_prs_state_table[event][p_db_glb->prs_state];
    MSTP_DEBUG(STP_PROTO_CIST, "[%s] Bridge recv event: %s, state: %s -> %s", 
            "PRS",
            cdb_enum_val2str(prs_event_strs, PRS_EVENT_MAX, event),
            cdb_enum_val2str(prs_state_strs, PRS_STATE_MAX, p_db_glb->prs_state),
            cdb_enum_val2str(prs_state_strs, PRS_STATE_MAX, next_state));

    switch (next_state)
    {
    case PRS_INIT_BRIDGE:
        _mstp_sm_PRS_INIT_BRIDGE(p_db_glb);
        _mstp_sm_PRS_ROLE_SELECTION(p_db_glb);
        break;
    case PRS_ROLE_SELECTION:
        _mstp_sm_PRS_ROLE_SELECTION(p_db_glb);
        break;
    default:
        break;
    }

    return PM_E_NONE;
}

#define _17_29__13_34__PRT_Port_Role_Transitions_Machine_

/* PRT State (Port Role Transitions)
   802.1D.2004  P.174  17.29 Port Role Transitions state machine
   802.1Q.2005  P.183  13.34 Port Role Transitions state machine
*/
uint32 mstp_sm_prt_state_table[PRT_EVENT_MAX][PRT_STATE_MAX] = 
{
  /* PRT_INIT_PORT          PRT_DISABLED_PORT    PRT_MASTER_PROPOSED    PRT_MASTER_AGREED    PRT_MASTER_SYNCED    PRT_MASTER_RETIRED    PRT_MASTER_FORWARD    PRT_MASTER_LEARN      PRT_MASTER_DISCARD    PRT_MASTER_PORT       PRT_ROOT_PROPOSED    PRT_ROOT_AGREED      PRT_REROOT              PRT_ROOT_PORT           PRT_REROOTED            PRT_ROOT_LEARN          PRT_ROOT_FORWARD        PRT_DESIGNATED_PROPOSE  PRT_DESIGNATED_SYNCED   PRT_DESIGNATED_RETIRED  PRT_DESIGNATED_PORT     PRT_DESIGNATED_DISCARD  PRT_DESIGNATED_LEARN    PRT_DESIGNATED_FORWARD    PRT_ALTERNATE_PROPOSED    PRT_ALTERNATE_AGREED    PRT_ALTERNATE_PORT      PRT_BACKUP_PORT         PRT_BLOCK_PORT */
    {PRT_INIT_PORT,         PRT_INIT_PORT,       PRT_INIT_PORT,         PRT_INIT_PORT,       PRT_INIT_PORT,       PRT_INIT_PORT,        PRT_INIT_PORT,        PRT_INIT_PORT,        PRT_INIT_PORT,        PRT_INIT_PORT,        PRT_INIT_PORT,       PRT_INIT_PORT,       PRT_INIT_PORT,          PRT_INIT_PORT,          PRT_INIT_PORT,          PRT_INIT_PORT,          PRT_INIT_PORT,          PRT_INIT_PORT,          PRT_INIT_PORT,          PRT_INIT_PORT,          PRT_INIT_PORT,          PRT_INIT_PORT,          PRT_INIT_PORT,          PRT_INIT_PORT,            PRT_INIT_PORT,            PRT_INIT_PORT,          PRT_INIT_PORT,          PRT_INIT_PORT,          PRT_INIT_PORT},        /* PRT_EVT_BEGIN */
    {PRT_DISABLED_PORT,     PRT_STATE_MAX,       PRT_DISABLED_PORT,     PRT_DISABLED_PORT,   PRT_DISABLED_PORT,   PRT_DISABLED_PORT,    PRT_DISABLED_PORT,    PRT_DISABLED_PORT,    PRT_DISABLED_PORT,    PRT_DISABLED_PORT,    PRT_DISABLED_PORT,   PRT_DISABLED_PORT,   PRT_DISABLED_PORT,      PRT_DISABLED_PORT,      PRT_DISABLED_PORT,      PRT_DISABLED_PORT,      PRT_DISABLED_PORT,      PRT_DISABLED_PORT,      PRT_DISABLED_PORT,      PRT_DISABLED_PORT,      PRT_DISABLED_PORT,      PRT_DISABLED_PORT,      PRT_DISABLED_PORT,      PRT_DISABLED_PORT,        PRT_DISABLED_PORT,        PRT_DISABLED_PORT,      PRT_DISABLED_PORT,      PRT_DISABLED_PORT,      PRT_DISABLED_PORT},    /* PRT_EVT_ROLE_DISABLED */
    {PRT_MASTER_PORT,       PRT_MASTER_PORT,     PRT_MASTER_PORT,       PRT_MASTER_PORT,     PRT_MASTER_PORT,     PRT_MASTER_PORT,      PRT_MASTER_PORT,      PRT_MASTER_PORT,      PRT_MASTER_PORT,      PRT_MASTER_PORT,      PRT_MASTER_PORT,     PRT_MASTER_PORT,     PRT_MASTER_PORT,        PRT_MASTER_PORT,        PRT_MASTER_PORT,        PRT_MASTER_PORT,        PRT_MASTER_PORT,        PRT_MASTER_PORT,        PRT_MASTER_PORT,        PRT_MASTER_PORT,        PRT_MASTER_PORT,        PRT_MASTER_PORT,        PRT_MASTER_PORT,        PRT_MASTER_PORT,          PRT_MASTER_PORT,          PRT_MASTER_PORT,        PRT_MASTER_PORT,        PRT_MASTER_PORT,        PRT_MASTER_PORT},      /* PRT_EVT_ROLE_MASTERPORT */
    {PRT_ROOT_PORT,         PRT_ROOT_PORT,       PRT_ROOT_PORT,         PRT_ROOT_PORT,       PRT_ROOT_PORT,       PRT_ROOT_PORT,        PRT_ROOT_PORT,        PRT_ROOT_PORT,        PRT_ROOT_PORT,        PRT_ROOT_PORT,        PRT_ROOT_PORT,       PRT_ROOT_PORT,       PRT_ROOT_PORT,          PRT_ROOT_PORT,          PRT_ROOT_PORT,          PRT_ROOT_PORT,          PRT_ROOT_PORT,          PRT_ROOT_PORT,          PRT_ROOT_PORT,          PRT_ROOT_PORT,          PRT_ROOT_PORT,          PRT_ROOT_PORT,          PRT_ROOT_PORT,          PRT_ROOT_PORT,            PRT_ROOT_PORT,            PRT_ROOT_PORT,          PRT_ROOT_PORT,          PRT_ROOT_PORT,          PRT_ROOT_PORT},        /* PRT_EVT_ROLE_ROOTPORT */
    {PRT_DESIGNATED_PORT,   PRT_DESIGNATED_PORT, PRT_DESIGNATED_PORT,   PRT_DESIGNATED_PORT, PRT_DESIGNATED_PORT, PRT_DESIGNATED_PORT,  PRT_DESIGNATED_PORT,  PRT_DESIGNATED_PORT,  PRT_DESIGNATED_PORT,  PRT_DESIGNATED_PORT,  PRT_DESIGNATED_PORT, PRT_DESIGNATED_PORT, PRT_DESIGNATED_PORT,    PRT_DESIGNATED_PORT,    PRT_DESIGNATED_PORT,    PRT_DESIGNATED_PORT,    PRT_DESIGNATED_PORT,    PRT_DESIGNATED_PORT,    PRT_DESIGNATED_PORT,    PRT_DESIGNATED_PORT,    PRT_DESIGNATED_PORT,    PRT_DESIGNATED_PORT,    PRT_DESIGNATED_PORT,    PRT_DESIGNATED_PORT,      PRT_DESIGNATED_PORT,      PRT_DESIGNATED_PORT,    PRT_DESIGNATED_PORT,    PRT_DESIGNATED_PORT,    PRT_DESIGNATED_PORT},  /* PRT_EVT_ROLE_DESIGNATED */
    {PRT_ALTERNATE_PORT,    PRT_ALTERNATE_PORT,  PRT_ALTERNATE_PORT,    PRT_ALTERNATE_PORT,  PRT_ALTERNATE_PORT,  PRT_ALTERNATE_PORT,   PRT_ALTERNATE_PORT,   PRT_ALTERNATE_PORT,   PRT_ALTERNATE_PORT,   PRT_ALTERNATE_PORT,   PRT_ALTERNATE_PORT,  PRT_ALTERNATE_PORT,  PRT_ALTERNATE_PORT,     PRT_ALTERNATE_PORT,     PRT_ALTERNATE_PORT,     PRT_ALTERNATE_PORT,     PRT_ALTERNATE_PORT,     PRT_ALTERNATE_PORT,     PRT_ALTERNATE_PORT,     PRT_ALTERNATE_PORT,     PRT_ALTERNATE_PORT,     PRT_ALTERNATE_PORT,     PRT_ALTERNATE_PORT,     PRT_ALTERNATE_PORT,       PRT_ALTERNATE_PORT,       PRT_ALTERNATE_PORT,     PRT_ALTERNATE_PORT,     PRT_ALTERNATE_PORT,     PRT_ALTERNATE_PORT},   /* PRT_EVT_ROLE_ALTERNATE */
    {PRT_BACKUP_PORT,       PRT_BACKUP_PORT,     PRT_BACKUP_PORT,       PRT_BACKUP_PORT,     PRT_BACKUP_PORT,     PRT_BACKUP_PORT,      PRT_BACKUP_PORT,      PRT_BACKUP_PORT,      PRT_BACKUP_PORT,      PRT_BACKUP_PORT,      PRT_BACKUP_PORT,     PRT_BACKUP_PORT,     PRT_BACKUP_PORT,        PRT_BACKUP_PORT,        PRT_BACKUP_PORT,        PRT_BACKUP_PORT,        PRT_BACKUP_PORT,        PRT_BACKUP_PORT,        PRT_BACKUP_PORT,        PRT_BACKUP_PORT,        PRT_BACKUP_PORT,        PRT_BACKUP_PORT,        PRT_BACKUP_PORT,        PRT_BACKUP_PORT,          PRT_BACKUP_PORT,          PRT_BACKUP_PORT,        PRT_BACKUP_PORT,        PRT_BACKUP_PORT,        PRT_BACKUP_PORT},      /* PRT_EVT_ROLE_BACKUP */
    {PRT_STATE_MAX,         PRT_STATE_MAX,       PRT_STATE_MAX,         PRT_STATE_MAX,       PRT_STATE_MAX,       PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,       PRT_STATE_MAX,       PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_DESIGNATED_SYNCED,  PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,            PRT_STATE_MAX,            PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX},        /* PRT_EVT_DESIGNATED_SYNCED */
    {PRT_STATE_MAX,         PRT_STATE_MAX,       PRT_STATE_MAX,         PRT_STATE_MAX,       PRT_STATE_MAX,       PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,       PRT_STATE_MAX,       PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_DESIGNATED_DISCARD, PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,            PRT_STATE_MAX,            PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX},        /* PRT_EVT_DESIGNATEDDISCARD */
    {PRT_STATE_MAX,         PRT_STATE_MAX,       PRT_STATE_MAX,         PRT_STATE_MAX,       PRT_STATE_MAX,       PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,       PRT_STATE_MAX,       PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_DESIGNATED_LEARN,   PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,            PRT_STATE_MAX,            PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX},        /* PRT_EVT_DESIGNATEDLEARN */
    {PRT_STATE_MAX,         PRT_STATE_MAX,       PRT_STATE_MAX,         PRT_STATE_MAX,       PRT_STATE_MAX,       PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,       PRT_STATE_MAX,       PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_DESIGNATED_FORWARD, PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,            PRT_STATE_MAX,            PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX},        /* PRT_EVT_DESIGNATEDFORWARD */
    {PRT_STATE_MAX,         PRT_STATE_MAX,       PRT_STATE_MAX,         PRT_STATE_MAX,       PRT_STATE_MAX,       PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_MASTER_SYNCED,    PRT_STATE_MAX,       PRT_STATE_MAX,       PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,            PRT_STATE_MAX,            PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX},        /* PRT_EVT_MASTER_SYNCED */
    {PRT_STATE_MAX,         PRT_STATE_MAX,       PRT_STATE_MAX,         PRT_STATE_MAX,       PRT_STATE_MAX,       PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_MASTER_DISCARD,   PRT_STATE_MAX,       PRT_STATE_MAX,       PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,            PRT_STATE_MAX,            PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX},        /* PRT_EVT_MASTERDISCARD */
    {PRT_STATE_MAX,         PRT_STATE_MAX,       PRT_STATE_MAX,         PRT_STATE_MAX,       PRT_STATE_MAX,       PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_MASTER_LEARN,     PRT_STATE_MAX,       PRT_STATE_MAX,       PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,            PRT_STATE_MAX,            PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX},        /* PRT_EVT_MASTERLEARN */
    {PRT_STATE_MAX,         PRT_STATE_MAX,       PRT_STATE_MAX,         PRT_STATE_MAX,       PRT_STATE_MAX,       PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_MASTER_FORWARD,   PRT_STATE_MAX,       PRT_STATE_MAX,       PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,            PRT_STATE_MAX,            PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX},        /* PRT_EVT_MASTERFORWARD */
    {PRT_STATE_MAX,         PRT_STATE_MAX,       PRT_STATE_MAX,         PRT_STATE_MAX,       PRT_STATE_MAX,       PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_ROOT_LEARN,      PRT_ROOT_LEARN,      PRT_ROOT_LEARN,         PRT_ROOT_LEARN,         PRT_ROOT_LEARN,         PRT_ROOT_LEARN,         PRT_ROOT_LEARN,         PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_DESIGNATED_LEARN,   PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,            PRT_STATE_MAX,            PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX},        /* PRT_EVT_ROOTLEARN */
    {PRT_STATE_MAX,         PRT_STATE_MAX,       PRT_STATE_MAX,         PRT_STATE_MAX,       PRT_STATE_MAX,       PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_ROOT_FORWARD,    PRT_ROOT_FORWARD,    PRT_ROOT_FORWARD,       PRT_ROOT_FORWARD,       PRT_ROOT_FORWARD,       PRT_ROOT_FORWARD,       PRT_ROOT_FORWARD,       PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_DESIGNATED_FORWARD, PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,            PRT_STATE_MAX,            PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX},        /* PRT_EVT_ROOTFORWARD */
    {PRT_STATE_MAX,         PRT_STATE_MAX,       PRT_STATE_MAX,         PRT_STATE_MAX,       PRT_STATE_MAX,       PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_ROOT_FORWARD,    PRT_ROOT_FORWARD,    PRT_ROOT_FORWARD,       PRT_ROOT_FORWARD,       PRT_ROOT_FORWARD,       PRT_ROOT_FORWARD,       PRT_ROOT_FORWARD,       PRT_STATE_MAX,          PRT_DESIGNATED_PROPOSE, PRT_DESIGNATED_PROPOSE, PRT_DESIGNATED_PROPOSE, PRT_DESIGNATED_PROPOSE, PRT_DESIGNATED_PROPOSE, PRT_DESIGNATED_PROPOSE,   PRT_STATE_MAX,            PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX},        /* PRT_EVT_PROPOSING */
    {PRT_STATE_MAX,         PRT_STATE_MAX,       PRT_STATE_MAX,         PRT_STATE_MAX,       PRT_STATE_MAX,       PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,       PRT_STATE_MAX,       PRT_STATE_MAX,          PRT_ROOT_PROPOSED,      PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,            PRT_STATE_MAX,            PRT_STATE_MAX,          PRT_ALTERNATE_PROPOSED, PRT_STATE_MAX,          PRT_STATE_MAX},        /* PRT_EVT_PROPOSED_NOTAGREE */
    {PRT_STATE_MAX,         PRT_STATE_MAX,       PRT_STATE_MAX,         PRT_STATE_MAX,       PRT_STATE_MAX,       PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_STATE_MAX,        PRT_ROOT_AGREED,     PRT_ROOT_AGREED,     PRT_ROOT_AGREED,        PRT_ROOT_AGREED,        PRT_ROOT_AGREED,        PRT_ROOT_AGREED,        PRT_ROOT_AGREED,        PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,          PRT_STATE_MAX,            PRT_ALTERNATE_AGREED,     PRT_ALTERNATE_AGREED,   PRT_ALTERNATE_AGREED,   PRT_STATE_MAX,          PRT_STATE_MAX},        /* PRT_EVT_ALLSYNCED_NOTAGREE */
};

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
_mstp_sm_PRT_INIT_PORT(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[PRT] %s enter INIT_PORT", ifname_ext);

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
_mstp_sm_PRT_DISABLED_PORT(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[PRT] %s enter DISABLED_PORT", ifname_ext);

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
isAllSynced()
{
    tbl_mstp_port_master_t *p_master = tbl_mstp_port_get_master();
    tbl_mstp_port_t *p_db_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    
    CTCLIB_SLIST_LOOP(p_master->port_list, p_db_port, listnode)
    {
        if (p_db_port->portEnabled)
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
setAgree(int32 allSynced)
{
    tbl_mstp_port_master_t *p_master = tbl_mstp_port_get_master();
    tbl_mstp_port_t *p_db_port = NULL;
    ctclib_slistnode_t *listnode = NULL;

    if (!allSynced)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP(p_master->port_list, p_db_port, listnode)
    {
        if (p_db_port->portEnabled)
        {
            if (ROLE_ROOTPORT == p_db_port->role)
            {
                if (!p_db_port->agree)
                {
                    mstp_sm_prt_process_event(p_db_port, PRT_EVT_ALLSYNCED_NOTAGREE);
                }
            }
            else if (ROLE_ALTERNATE == p_db_port->role)
            {
                if (!p_db_port->agree)
                {
                    mstp_sm_prt_process_event(p_db_port, PRT_EVT_ALLSYNCED_NOTAGREE);
                }
            }
        }
    }

    return TRUE;
}

int32
updateAllSynced()
{
    int32 allSynced = FALSE;
    
    allSynced = isAllSynced();
    setAgree(allSynced);

    return PM_E_NONE;
}

/*
17.21.14 setSyncTree()
Sets sync TRUE for all Ports of the Bridge.
*/
int32
setSyncTree()
{
    tbl_mstp_port_master_t *p_master = tbl_mstp_port_get_master();
    tbl_mstp_port_t *p_db_port = NULL;
    ctclib_slistnode_t *listnode = NULL;

    CTCLIB_SLIST_LOOP(p_master->port_list, p_db_port, listnode)
    {
        if (p_db_port->portEnabled)
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
                 && !p_db_port->operEdge
                 && (p_db_port->learn || p_db_port->forward))
                {
                    mstp_sm_prt_process_event(p_db_port, PRT_EVT_DESIGNATEDDISCARD);
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
                else if (p_db_port->operEdge && !p_db_port->synced)
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

    updateAllSynced();
    
    return PM_E_NONE;
}

/*
17.21.15 setReRootTree()
Sets reRoot TRUE for all Ports of the Bridge.
*/
int32
setReRootTree()
{
    tbl_mstp_port_master_t *p_master = tbl_mstp_port_get_master();
    tbl_mstp_port_t *p_db_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    
    CTCLIB_SLIST_LOOP(p_master->port_list, p_db_port, listnode)
    {
        if (p_db_port->portEnabled)
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
_mstp_sm_PRT_ROOT_PROPOSED(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[PRT] %s enter ROOT_PROPOSED", ifname_ext);

    setSyncTree();
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
_mstp_sm_PRT_ROOT_AGREED(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[PRT] %s enter ROOT_AGREED", ifname_ext);
    
    p_db_port->proposed = p_db_port->sync = FALSE;
    p_db_port->agree = TRUE;
    p_db_port->newInfo = TRUE;

    p_db_port->prt_state = PRT_ROOT_AGREED;
    
    if (p_db_port->sendRstp)
    {
        mstp_sm_send_bpdu(p_db_port);
    }

    return PM_E_NONE;
}

/*
setReRootTree();
*/
static int32
_mstp_sm_PRT_REROOT(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[PRT] %s enter REROOT", ifname_ext);

    setReRootTree();

    p_db_port->prt_state = PRT_REROOT;    
    return PM_E_NONE;
}

void
_mstp_sm_delay_rootLearnForward_timer_fn(void *p_data)
{
    tbl_mstp_port_key_t *p_port_key = (tbl_mstp_port_key_t*)p_data;
    tbl_mstp_port_t *p_db_port = NULL;

    p_db_port = tbl_mstp_port_get_mstp_port(p_port_key);
    XFREE(MEM_PM_MSTP_TMP, p_port_key);
    if (NULL == p_db_port)
    {
        return;
    }

    if (ROLE_ROOTPORT == p_db_port->role)
    {
        if (!p_db_port->learn)
        {
            mstp_sm_prt_process_event(p_db_port, PRT_EVT_ROOTLEARN);
        }
        
        if (!p_db_port->forward)
        {
            mstp_sm_prt_process_event(p_db_port, PRT_EVT_ROOTFORWARD);
        }
    }

    return;
}

/*
role = RootPort;
rrWhile = FwdDelay;
*/
static int32
_mstp_sm_PRT_ROOT_PORT(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    tbl_mstp_global_t *p_mstp_glb = tbl_mstp_global_get_mstp_global();
    uint8 old_role = 0;
    tbl_mstp_port_key_t *p_port_key = NULL;

    MSTP_DEBUG(STP_PROTO_CIST, "[PRT] %s enter ROOT_PORT", ifname_ext);

    /* modified by liwh for bug 37514, 2016-05-17 */
    if (p_db_port->restricted_role)
    {
        _mstp_sm_PRT_BLOCK_PORT(p_db_port, ifname_ext);
        _mstp_sm_PRT_ALTERNATE_PORT(p_db_port, ifname_ext);
        return PM_E_NONE;
    }
    /* liwh end */

    old_role = p_db_port->role;
    p_db_port->role = ROLE_ROOTPORT;
    
    CTC_TASK_STOP_TIMER(p_db_port->rrWhile_timer);
    p_db_port->rrWhile_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _mstp_sm_rrWhile_timer_fn, p_db_port, 
        p_db_port->port_fwdDelay);

    p_db_port->prt_state = PRT_ROOT_PORT;

    if (ROLE_ALTERNATE == old_role || ROLE_DESIGNATED == old_role)
    {
        if (MSTP_TYPE_STP != p_mstp_glb->type)
        {
            /* Modified by kcao for bug 39730, alternate -> rootport should switch to forward after a short timer,
             * otherwise, will result in instant loop
             */
            p_port_key = XCALLOC(MEM_PM_MSTP_TMP, sizeof(tbl_mstp_port_key_t));
            if (p_port_key)
            {
                sal_memcpy(p_port_key, &p_db_port->key, sizeof(tbl_mstp_port_key_t));
                ctc_task_add_timer(TASK_PRI_NORMAL, _mstp_sm_delay_rootLearnForward_timer_fn, p_port_key, 0);
            }
        }
        else
        {
            if (PST_DISCARDING == p_db_port->pst_state)
            {
                mstp_sm_pst_process_event(p_db_port, PST_EVT_LISTEN);
            }
        }
    }

    return PM_E_NONE;
}

/*
reRoot = FALSE;
*/
static int32
_mstp_sm_PRT_REROOTED(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[PRT] %s enter REROOTED", ifname_ext);
    
    p_db_port->reRoot = FALSE;
    
    p_db_port->prt_state = PRT_REROOT;
    return PM_E_NONE;
}

/*
fdWhile= forwardDelay;
learn = TRUE;
*/
static int32
_mstp_sm_PRT_ROOT_LEARN(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    uint32 forwardDelay = 0;

    MSTP_DEBUG(STP_PROTO_CIST, "[PRT] %s enter ROOT_LEARN", ifname_ext);
    
    /* 
    17.20.5 forwardDelay
    Returns the value of HelloTime if sendRSTP is TRUE, and the value of FwdDelay otherwise.
    */
    forwardDelay = (p_db_port->sendRstp) ? p_db_port->port_helloTime : p_db_port->port_fwdDelay;
    CTC_TASK_STOP_TIMER(p_db_port->fdWhile_timer);
    p_db_port->fdWhile_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _mstp_sm_fdWhile_timer_fn, p_db_port, 
        forwardDelay);

    if (!p_db_port->learn)
    {
        mstp_sm_pst_process_event(p_db_port, PST_EVT_LEARN);
    }
    p_db_port->learn = TRUE;
    if (TCM_INACTIVE == p_db_port->tcm_state && !p_db_port->fdbFlush)
    {
        mstp_sm_tcm_process_event(p_db_port, TCM_EVT_LEARNNOTFDBFLUSH);
    }
    
    p_db_port->prt_state = PRT_ROOT_LEARN;
    return PM_E_NONE;
}

/*
fdWhile = 0;
forward = TRUE;
*/
static int32
_mstp_sm_PRT_ROOT_FORWARD(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[PRT] %s enter ROOT_FORWARD", ifname_ext);
    
    CTC_TASK_STOP_TIMER(p_db_port->fdWhile_timer);
    
    if (!p_db_port->forward)
    {
        mstp_sm_pst_process_event(p_db_port, PST_EVT_FORWARD);
    }
    p_db_port->forward = TRUE;
    if (!p_db_port->operEdge)
    {
        mstp_sm_tcm_process_event(p_db_port, TCM_EVT_FORWARDNOTOPEREDGE);
    }

    p_db_port->prt_state = PRT_ROOT_FORWARD;
    return PM_E_NONE;
}

/*
setSyncTree();
proposed = FALSE;
*/
static int32
_mstp_sm_PRT_MASTER_PROPOSED(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[PRT] %s enter MASTER_PROPOSED", ifname_ext);

    setSyncTree();
    p_db_port->proposed = FALSE;

    p_db_port->prt_state = PRT_MASTER_PROPOSED;
    return PM_E_NONE;
}

/*
proposed = sync = FALSE;
agree = TRUE;
*/
static int32
_mstp_sm_PRT_MASTER_AGREED(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[PRT] %s enter MASTER_AGREED", ifname_ext);
    
    p_db_port->proposed = p_db_port->sync = FALSE;
    p_db_port->agree = TRUE;

    p_db_port->prt_state = PRT_MASTER_AGREED;
    
    if (p_db_port->sendRstp)
    {
        mstp_sm_send_bpdu(p_db_port);
    }

    return PM_E_NONE;
}

/*
rrWhile = 0;
synced = TRUE;
sync = FALSE;
*/
static int32
_mstp_sm_PRT_MASTER_SYNCED(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[PRT] %s enter MASTER_SYNCED", ifname_ext);

    p_db_port->synced = TRUE;
    p_db_port->sync = FALSE;

    p_db_port->prt_state = PRT_MASTER_SYNCED;    
    return PM_E_NONE;
}

/*
reRoot = FALSE;
*/
static int32
_mstp_sm_PRT_MASTER_RETIRED(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[PRT] %s enter MASTER_RETIRED", ifname_ext);

    p_db_port->reRoot = FALSE;

    p_db_port->prt_state = PRT_MASTER_RETIRED;    
    return PM_E_NONE;
}

/*
fdWhile = 0;
forward = TRUE;
*/
static int32
_mstp_sm_PRT_MASTER_FORWARD(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[PRT] %s enter MASTER_FORWARD", ifname_ext);
    
    CTC_TASK_STOP_TIMER(p_db_port->fdWhile_timer);
    
    if (!p_db_port->forward)
    {
        mstp_sm_pst_process_event(p_db_port, PST_EVT_FORWARD);
    }
    p_db_port->forward = TRUE;
    if (!p_db_port->operEdge)
    {
        mstp_sm_tcm_process_event(p_db_port, TCM_EVT_FORWARDNOTOPEREDGE);
    }

    p_db_port->prt_state = PRT_MASTER_FORWARD;
    return PM_E_NONE;
}

/*
fdWhile= forwardDelay;
learn = TRUE;
*/
static int32
_mstp_sm_PRT_MASTER_LEARN(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    uint32 forwardDelay = 0;

    MSTP_DEBUG(STP_PROTO_CIST, "[PRT] %s enter MASTER_LEARN", ifname_ext);
    
    /* 
    17.20.5 forwardDelay
    Returns the value of HelloTime if sendRSTP is TRUE, and the value of FwdDelay otherwise.
    */
    forwardDelay = (p_db_port->sendRstp) ? p_db_port->port_helloTime : p_db_port->port_fwdDelay;
    CTC_TASK_STOP_TIMER(p_db_port->fdWhile_timer);
    p_db_port->fdWhile_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _mstp_sm_fdWhile_timer_fn, p_db_port, 
        forwardDelay);

    if (!p_db_port->learn)
    {
        mstp_sm_pst_process_event(p_db_port, PST_EVT_LEARN);
    }
    p_db_port->learn = TRUE;
    if (TCM_INACTIVE == p_db_port->tcm_state && !p_db_port->fdbFlush)
    {
        mstp_sm_tcm_process_event(p_db_port, TCM_EVT_LEARNNOTFDBFLUSH);
    }
    
    p_db_port->prt_state = PRT_MASTER_LEARN;
    return PM_E_NONE;
}

/*
learn = forward = disputed = FALSE;
fdWhile= forwardDelay;
*/
int32
_mstp_sm_PRT_MASTER_DISCARD(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    uint32 forwardDelay = 0;
    
    MSTP_DEBUG(STP_PROTO_CIST, "[PRT] %s enter MASTER_DISCARD", ifname_ext);
    /* 
    17.20.5 forwardDelay
    Returns the value of HelloTime if sendRSTP is TRUE, and the value of FwdDelay otherwise.
    */
    forwardDelay = (p_db_port->sendRstp) ? p_db_port->port_helloTime : p_db_port->port_fwdDelay;

    if (p_db_port->forward)
    {
        mstp_sm_pst_process_event(p_db_port, PST_EVT_NOTFORWARD);
    }
    if (p_db_port->learn)
    {
        mstp_sm_pst_process_event(p_db_port, PST_EVT_NOTLEARN);
    }
    p_db_port->learn = FALSE;
    p_db_port->forward = FALSE;
    p_db_port->disputed = FALSE;
    CTC_TASK_STOP_TIMER(p_db_port->fdWhile_timer);
    p_db_port->fdWhile_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _mstp_sm_fdWhile_timer_fn, p_db_port, 
        forwardDelay);

    p_db_port->prt_state = PRT_MASTER_DISCARD;
    
    mstp_api_flush_fdb(p_db_port);

    return PM_E_NONE;
}

/*
role = RootPort;
rrWhile = FwdDelay;
*/
static int32
_mstp_sm_PRT_MASTER_PORT(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    tbl_mstp_global_t *p_mstp_glb = tbl_mstp_global_get_mstp_global();
    uint8 old_role = 0;
    tbl_mstp_port_key_t *p_port_key = NULL;

    MSTP_DEBUG(STP_PROTO_CIST, "[PRT] %s enter MASTER_PORT", ifname_ext);

    /* modified by liwh for bug 37514, 2016-05-17 */
    if (p_db_port->restricted_role)
    {
        _mstp_sm_PRT_BLOCK_PORT(p_db_port, ifname_ext);
        _mstp_sm_PRT_ALTERNATE_PORT(p_db_port, ifname_ext);
        return PM_E_NONE;
    }
    /* liwh end */

    old_role = p_db_port->role;
    p_db_port->role = ROLE_MASTERPORT;
    
    CTC_TASK_STOP_TIMER(p_db_port->rrWhile_timer);
    p_db_port->rrWhile_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _mstp_sm_rrWhile_timer_fn, p_db_port, 
        p_db_port->port_fwdDelay);

    p_db_port->prt_state = PRT_MASTER_PORT;

    if (ROLE_ALTERNATE == old_role || ROLE_DESIGNATED == old_role)
    {
        if (MSTP_TYPE_STP != p_mstp_glb->type)
        {
            /* Modified by kcao for bug 39730, alternate -> rootport should switch to forward after a short timer,
             * otherwise, will result in instant loop
             */
            p_port_key = XCALLOC(MEM_PM_MSTP_TMP, sizeof(tbl_mstp_port_key_t));
            if (p_port_key)
            {
                sal_memcpy(p_port_key, &p_db_port->key, sizeof(tbl_mstp_port_key_t));
                ctc_task_add_timer(TASK_PRI_NORMAL, _mstp_sm_delay_rootLearnForward_timer_fn, p_port_key, 0);
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
_mstp_sm_PRT_DESIGNATED_PROPOSE(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    uint32 EdgeDelay = 0;

    MSTP_DEBUG(STP_PROTO_CIST, "[PRT] %s enter DESIGNATED_PROPOSE", ifname_ext);
    
    /*
    17.20.4 EdgeDelay
    Returns the value of MigrateTime if operPointToPointMAC is TRUE, and the value of MaxAge otherwise.
    */
    EdgeDelay = (p_db_port->operP2PMac) ? MSTP_TIMER_DEF_MIGRATE_TIME : p_db_port->port_maxAge;

    p_db_port->proposing = TRUE;
    CTC_TASK_STOP_TIMER(p_db_port->edgeDelayWhile_timer);
    p_db_port->edgeDelayWhile_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _mstp_sm_edgeDelayWhile_timer_fn, p_db_port, 
        EdgeDelay);
    p_db_port->newInfo = TRUE;

    p_db_port->prt_state = PRT_DESIGNATED_PROPOSE;

    /* send propose immediately */
    if (p_db_port->sendRstp)
    {
        mstp_sm_send_bpdu(p_db_port);
    }

    return PM_E_NONE;
}

/*
rrWhile = 0; synced = TRUE;
sync = FALSE;
*/
static int32
_mstp_sm_PRT_DESIGNATED_SYNCED(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[PRT] %s enter DESIGNATED_SYNCED", ifname_ext);
    
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
_mstp_sm_PRT_DESIGNATED_RETIRED(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[PRT] %s enter DESIGNATED_RETIRED", ifname_ext);
    
    p_db_port->reRoot = FALSE;

    p_db_port->prt_state = PRT_DESIGNATED_RETIRED;
    return PM_E_NONE;
}

/*
role = DesignatedPort;
*/
static int32
_mstp_sm_PRT_DESIGNATED_PORT(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[PRT] %s enter DESIGNATED_PORT", ifname_ext);
    p_db_port->role = ROLE_DESIGNATED;
    p_db_port->prt_state = PRT_DESIGNATED_PORT;
    
    return PM_E_NONE;
}

/*
learn = forward = disputed = FALSE;
fdWhile= forwardDelay;
*/
int32
_mstp_sm_PRT_DESIGNATED_DISCARD(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    uint32 forwardDelay = 0;
    
    MSTP_DEBUG(STP_PROTO_CIST, "[PRT] %s enter DESIGNATED_DISCARD", ifname_ext);
    /* 
    17.20.5 forwardDelay
    Returns the value of HelloTime if sendRSTP is TRUE, and the value of FwdDelay otherwise.
    */
    forwardDelay = (p_db_port->sendRstp) ? p_db_port->port_helloTime : p_db_port->port_fwdDelay;

    if (p_db_port->forward)
    {
        mstp_sm_pst_process_event(p_db_port, PST_EVT_NOTFORWARD);
    }
    if (p_db_port->learn)
    {
        mstp_sm_pst_process_event(p_db_port, PST_EVT_NOTLEARN);
    }
    p_db_port->learn = FALSE;
    p_db_port->forward = FALSE;
    p_db_port->disputed = FALSE;
    CTC_TASK_STOP_TIMER(p_db_port->fdWhile_timer);
    p_db_port->fdWhile_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _mstp_sm_fdWhile_timer_fn, p_db_port, 
        forwardDelay);

    p_db_port->prt_state = PRT_DESIGNATED_DISCARD;
    
    mstp_api_flush_fdb(p_db_port);

    return PM_E_NONE;
}

/*
learn = TRUE;
fdWhile= forwardDelay;
*/
static int32
_mstp_sm_PRT_DESIGNATED_LEARN(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    uint32 forwardDelay = 0;

    MSTP_DEBUG(STP_PROTO_CIST, "[PRT] %s enter DESIGNATED_LEARN", ifname_ext);
    
    /* 
    17.20.5 forwardDelay
    Returns the value of HelloTime if sendRSTP is TRUE, and the value of FwdDelay otherwise.
    */
    forwardDelay = (p_db_port->sendRstp) ? p_db_port->port_helloTime : p_db_port->port_fwdDelay;

    if (!p_db_port->learn)
    {
        mstp_sm_pst_process_event(p_db_port, PST_EVT_LEARN);
    }
    p_db_port->learn = TRUE;
    if (TCM_INACTIVE == p_db_port->tcm_state && !p_db_port->fdbFlush)
    {
        mstp_sm_tcm_process_event(p_db_port, TCM_EVT_LEARNNOTFDBFLUSH);
    }
    
    CTC_TASK_STOP_TIMER(p_db_port->fdWhile_timer);
    p_db_port->fdWhile_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _mstp_sm_fdWhile_timer_fn, p_db_port, 
        forwardDelay);

    p_db_port->prt_state = PRT_DESIGNATED_LEARN;
    return PM_E_NONE;
}

/*
forward = TRUE; fdWhile = 0;
agreed = sendRstp;
*/
static int32
_mstp_sm_PRT_DESIGNATED_FORWARD(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[PRT] %s enter DESIGNATED_FORWARD", ifname_ext);
    
    if (!p_db_port->forward)
    {
        /* if port to forward, clear proposing */
        p_db_port->proposing = FALSE;
        mstp_sm_pst_process_event(p_db_port, PST_EVT_FORWARD);
    }
    p_db_port->forward = TRUE;
    if (!p_db_port->operEdge)
    {
        mstp_sm_tcm_process_event(p_db_port, TCM_EVT_FORWARDNOTOPEREDGE);
    }

    CTC_TASK_STOP_TIMER(p_db_port->fdWhile_timer);
    p_db_port->agreed = p_db_port->sendRstp;

    p_db_port->prt_state = PRT_DESIGNATED_FORWARD;

    return PM_E_NONE;
}

/*
setSyncTree();
proposed = FALSE;
*/
static int32
_mstp_sm_PRT_ALTERNATE_PROPOSED(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[PRT] %s enter ALTERNATE_PROPOSED", ifname_ext);

    /* For switch eth-0-1 <--> eth-0-2 loopback, the Designated port cannot enter forwarding state, and switch between discard and learning */
    if (ROLE_BACKUP != p_db_port->role)
    {
        setSyncTree();
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
_mstp_sm_PRT_ALTERNATE_AGREED(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[PRT] %s enter ALTERNATE_AGREED", ifname_ext);
    
    p_db_port->proposed = FALSE;
    p_db_port->agree = TRUE;
    p_db_port->newInfo = TRUE;

    p_db_port->prt_state = PRT_ALTERNATE_AGREED;

    if (p_db_port->sendRstp)
    {
        mstp_sm_send_bpdu(p_db_port);
    }

    return PM_E_NONE;
}

/*
fdWhile = FwdDelay;synced = TRUE; rrWhile = 0; sync = reRoot = FALSE;
*/
static int32
_mstp_sm_PRT_ALTERNATE_PORT(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[PRT] %s enter ALTERNATE_PORT", ifname_ext);
    
    CTC_TASK_STOP_TIMER(p_db_port->fdWhile_timer);
    p_db_port->fdWhile_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _mstp_sm_fdWhile_timer_fn, p_db_port, 
        p_db_port->port_fwdDelay);
    p_db_port->synced = TRUE;
    CTC_TASK_STOP_TIMER(p_db_port->rrWhile_timer);
    p_db_port->sync = FALSE;
    p_db_port->reRoot = FALSE;
    
    p_db_port->prt_state = PRT_ALTERNATE_PORT;
    return PM_E_NONE;
}

/*
rbWhile = 2*HelloTime;
*/
static int32
_mstp_sm_PRT_BACKUP_PORT(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[PRT] %s enter BACKUP_PORT", ifname_ext);
    
    CTC_TASK_STOP_TIMER(p_db_port->rbWhile_timer);
    p_db_port->rbWhile_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _mstp_sm_rbWhile_timer_fn, p_db_port, 
        2 * p_db_port->port_helloTime);

    p_db_port->prt_state = PRT_BACKUP_PORT;
    return PM_E_NONE;
}

/*
role = selectedRole;
learn = forward = FALSE;
*/
static int32
_mstp_sm_PRT_BLOCK_PORT(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[PRT] %s enter BLOCK_PORT", ifname_ext);
    
    if (p_db_port->role == p_db_port->selectedRole)
    {
        return PM_E_NONE;
    }

    if (p_db_port->forward)
    {
        mstp_sm_pst_process_event(p_db_port, PST_EVT_NOTFORWARD);
    }
    if (p_db_port->learn)
    {
        mstp_sm_pst_process_event(p_db_port, PST_EVT_NOTLEARN);
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

    mstp_api_flush_fdb(p_db_port);

    return PM_E_NONE;
}

/*
  Figure 17-24  Port State Transition state machine
*/
int32
mstp_sm_prt_process_event(tbl_mstp_port_t *p_db_port, prt_event_t event)
{
    prt_state_t next_state = 0;
    char ifname_ext[IFNAME_EXT_SIZE];

    if (event >= PRT_EVENT_MAX)
    {
        return PM_E_FAIL;
    }

    IFNAME_ETH2FULL(p_db_port->key.name, ifname_ext);
    next_state = mstp_sm_prt_state_table[event][p_db_port->prt_state];
    MSTP_DEBUG(STP_PROTO_CIST, "[%s] Interface %s recv event: %s, state: %s -> %s", 
            "PRT",
            ifname_ext,
            cdb_enum_val2str(prt_event_strs, PRT_EVENT_MAX, event),
            cdb_enum_val2str(prt_state_strs, PRT_STATE_MAX, p_db_port->prt_state),
            cdb_enum_val2str(prt_state_strs, PRT_STATE_MAX, next_state));
   
    switch (next_state)
    {
    case PRT_INIT_PORT:
        _mstp_sm_PRT_INIT_PORT(p_db_port, ifname_ext);
        _mstp_sm_PRT_DISABLED_PORT(p_db_port, ifname_ext);
        break;
    case PRT_DISABLED_PORT:
        _mstp_sm_PRT_DISABLED_PORT(p_db_port, ifname_ext);
        break;
    case PRT_ROOT_PROPOSED:
        _mstp_sm_PRT_ROOT_PROPOSED(p_db_port, ifname_ext);
        _mstp_sm_PRT_ROOT_PORT(p_db_port, ifname_ext);
        break;
    case PRT_ROOT_AGREED:
        _mstp_sm_PRT_ROOT_AGREED(p_db_port, ifname_ext);
        _mstp_sm_PRT_ROOT_PORT(p_db_port, ifname_ext);
        break;
    case PRT_REROOT:
        _mstp_sm_PRT_REROOT(p_db_port, ifname_ext);
        _mstp_sm_PRT_ROOT_PORT(p_db_port, ifname_ext);
        break;
    case PRT_ROOT_PORT:
        _mstp_sm_PRT_ROOT_PORT(p_db_port, ifname_ext);
        break;
    case PRT_REROOTED:
        _mstp_sm_PRT_REROOTED(p_db_port, ifname_ext);
        _mstp_sm_PRT_ROOT_PORT(p_db_port, ifname_ext);
        break;
    case PRT_ROOT_LEARN:
        _mstp_sm_PRT_ROOT_LEARN(p_db_port, ifname_ext);
        _mstp_sm_PRT_ROOT_PORT(p_db_port, ifname_ext);
        break;
    case PRT_ROOT_FORWARD:
        _mstp_sm_PRT_ROOT_FORWARD(p_db_port, ifname_ext);
        _mstp_sm_PRT_ROOT_PORT(p_db_port, ifname_ext);
        break;
    case PRT_MASTER_PROPOSED:
        _mstp_sm_PRT_MASTER_PROPOSED(p_db_port, ifname_ext);
        _mstp_sm_PRT_MASTER_PORT(p_db_port, ifname_ext);
        break;
    case PRT_MASTER_AGREED:
        _mstp_sm_PRT_MASTER_AGREED(p_db_port, ifname_ext);
        _mstp_sm_PRT_MASTER_PORT(p_db_port, ifname_ext);
        break;
    case PRT_MASTER_SYNCED:
        _mstp_sm_PRT_MASTER_SYNCED(p_db_port, ifname_ext);
        _mstp_sm_PRT_MASTER_PORT(p_db_port, ifname_ext);
        break;
    case PRT_MASTER_RETIRED:
        _mstp_sm_PRT_MASTER_RETIRED(p_db_port, ifname_ext);
        _mstp_sm_PRT_MASTER_PORT(p_db_port, ifname_ext);
        break;
    case PRT_MASTER_FORWARD:
        _mstp_sm_PRT_MASTER_FORWARD(p_db_port, ifname_ext);
        _mstp_sm_PRT_MASTER_PORT(p_db_port, ifname_ext);
        break;
    case PRT_MASTER_LEARN:
        _mstp_sm_PRT_MASTER_LEARN(p_db_port, ifname_ext);
        _mstp_sm_PRT_MASTER_PORT(p_db_port, ifname_ext);
        break;
    case PRT_MASTER_DISCARD:
        _mstp_sm_PRT_MASTER_DISCARD(p_db_port, ifname_ext);
        _mstp_sm_PRT_MASTER_PORT(p_db_port, ifname_ext);
        break;
    case PRT_MASTER_PORT:
        _mstp_sm_PRT_MASTER_PORT(p_db_port, ifname_ext);
        break;
    case PRT_DESIGNATED_PROPOSE:
        _mstp_sm_PRT_DESIGNATED_PROPOSE(p_db_port, ifname_ext);
        _mstp_sm_PRT_DESIGNATED_PORT(p_db_port, ifname_ext);
        break;
    case PRT_DESIGNATED_SYNCED:
        _mstp_sm_PRT_DESIGNATED_SYNCED(p_db_port, ifname_ext);
        _mstp_sm_PRT_DESIGNATED_PORT(p_db_port, ifname_ext);
        break;
    case PRT_DESIGNATED_RETIRED:
        _mstp_sm_PRT_DESIGNATED_RETIRED(p_db_port, ifname_ext);
        _mstp_sm_PRT_DESIGNATED_PORT(p_db_port, ifname_ext);
        break;
    case PRT_DESIGNATED_PORT:
        _mstp_sm_PRT_DESIGNATED_PORT(p_db_port, ifname_ext);
        break;
    case PRT_DESIGNATED_DISCARD:
        _mstp_sm_PRT_DESIGNATED_DISCARD(p_db_port, ifname_ext);
        _mstp_sm_PRT_DESIGNATED_PORT(p_db_port, ifname_ext);
        break;
    case PRT_DESIGNATED_LEARN:
        _mstp_sm_PRT_DESIGNATED_LEARN(p_db_port, ifname_ext);
        _mstp_sm_PRT_DESIGNATED_PORT(p_db_port, ifname_ext);
        break;
    case PRT_DESIGNATED_FORWARD:
        _mstp_sm_PRT_DESIGNATED_FORWARD(p_db_port, ifname_ext);
        _mstp_sm_PRT_DESIGNATED_PORT(p_db_port, ifname_ext);
        break;
    case PRT_ALTERNATE_PROPOSED:
        _mstp_sm_PRT_ALTERNATE_PROPOSED(p_db_port, ifname_ext);
        _mstp_sm_PRT_ALTERNATE_PORT(p_db_port, ifname_ext);
        break;
    case PRT_ALTERNATE_AGREED:
        _mstp_sm_PRT_ALTERNATE_AGREED(p_db_port, ifname_ext);
        _mstp_sm_PRT_ALTERNATE_PORT(p_db_port, ifname_ext);
        break;
    case PRT_ALTERNATE_PORT:
        _mstp_sm_PRT_BLOCK_PORT(p_db_port, ifname_ext);
        _mstp_sm_PRT_ALTERNATE_PORT(p_db_port, ifname_ext);
        break;
    case PRT_BACKUP_PORT:
        _mstp_sm_PRT_BLOCK_PORT(p_db_port, ifname_ext);
        _mstp_sm_PRT_BACKUP_PORT(p_db_port, ifname_ext);
        break;
    case PRT_BLOCK_PORT:
        _mstp_sm_PRT_BLOCK_PORT(p_db_port, ifname_ext);
        break;
    default:
        /* modified by liwh for bug 36699/37625/37767, 2016-05-11 
           when there is no process, should return;
           otherwise, dead loop will happen */
        return PM_E_NONE;
        /* liwh end */
    }

    mstp_sm_PRT_TRIGGER_EVENT(p_db_port);
    return PM_E_NONE;
}

#define _17_30__13_35__PST_Port_State_Transitions_Machine_

/* PST State (Port State Transition)
   802.1D.2004  P.177  17.30 Port State Transition state machine
   802.1Q.2005  P.187  13.35 Port State Transition state machine
*/
uint32 mstp_sm_pst_state_table[PST_EVENT_MAX][PST_STATE_MAX] = 
{
  /* PST_DISCARDING     PST_LISTENING       PST_LEARNING        PST_FORWARDING */
    {PST_DISCARDING,    PST_DISCARDING,     PST_DISCARDING,     PST_DISCARDING},    /* PST_EVT_BEGIN */
    {PST_LISTENING,     PST_STATE_MAX,      PST_STATE_MAX,      PST_STATE_MAX},     /* PST_EVT_LISTEN */
    {PST_LEARNING,      PST_LEARNING,       PST_STATE_MAX,      PST_STATE_MAX},     /* PST_EVT_LEARN */
    {PST_STATE_MAX,     PST_DISCARDING,     PST_DISCARDING,     PST_STATE_MAX},     /* PST_EVT_NOTLEARN */
    {PST_STATE_MAX,     PST_STATE_MAX,      PST_FORWARDING,     PST_STATE_MAX},     /* PST_EVT_FORWARD */
    {PST_STATE_MAX,     PST_STATE_MAX,      PST_STATE_MAX,      PST_DISCARDING},    /* PST_EVT_NOTFORWARD */
};

/*
disableLearning(); learning = FALSE;
disableForwarding(); forwarding = FALSE;
*/
static int32
_mstp_sm_PST_DISCARDING(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[PST] %s enter DISCARDING", ifname_ext);
        
    p_db_port->learning = FALSE;
    p_db_port->forwarding = FALSE;
    
    mstp_api_set_port_fei_state(p_db_port, GLB_BR_PORT_STATE_BLOCKING);

    p_db_port->pst_state = PST_DISCARDING;

    return PM_E_NONE;
}

/*
only for STP
*/
static int32
_mstp_sm_PST_LISTENING(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[PST] %s enter LISTENING", ifname_ext);

    if (NULL == p_db_port->fdWhile_timer)
    {
        p_db_port->fdWhile_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _mstp_sm_fdWhile_timer_fn, p_db_port, 
            p_db_port->port_fwdDelay);
    }
        
    p_db_port->pst_state = PST_LISTENING;

    return PM_E_NONE;
}

/*
enableLearning(); learning = TRUE;
*/
static int32
_mstp_sm_PST_LEARNING(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[PST] %s enter LEARNING", ifname_ext);
    
    p_db_port->learning = TRUE;
    
    mstp_api_set_port_fei_state(p_db_port, GLB_BR_PORT_STATE_LEARNING);

    p_db_port->pst_state = PST_LEARNING;

    return PM_E_NONE;
}

/*
enableForwarding(); forwarding = TRUE;
*/
static int32
_mstp_sm_PST_FORWARDING(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[PST] %s enter FORWARDING", ifname_ext);
    
    p_db_port->forwarding = TRUE;

    mstp_api_set_port_fei_state(p_db_port, GLB_BR_PORT_STATE_FORWARDING);

    p_db_port->pst_state = PST_FORWARDING;

    return PM_E_NONE;
}

/*
  Figure 17-24  Port State Transition state machine
*/
int32
mstp_sm_pst_process_event(tbl_mstp_port_t *p_db_port, pst_event_t event)
{
    pst_state_t next_state = 0;
    char ifname_ext[IFNAME_EXT_SIZE];

    if (event >= PST_EVENT_MAX)
    {
        return PM_E_FAIL;
    }

    IFNAME_ETH2FULL(p_db_port->key.name, ifname_ext);
    next_state =  mstp_sm_pst_state_table[event][p_db_port->pst_state];
    MSTP_DEBUG(STP_PROTO_CIST, "[%s] Interface %s recv event: %s, state: %s -> %s", 
            "PST",
            ifname_ext,
            cdb_enum_val2str(pst_event_strs, PST_EVENT_MAX, event),
            cdb_enum_val2str(pst_state_strs, PST_STATE_MAX, p_db_port->pst_state),
            cdb_enum_val2str(pst_state_strs, PST_STATE_MAX, next_state));

    /* modified by liwh for bug 37762, 2016-06-21 */
    if (p_db_port->admin_loopguard && p_db_port->oper_loopguard
        && (PST_DISCARDING != next_state))
    {
        return PM_E_NONE;
    }
    /* liwh end */
    
    switch (next_state)
    {
    case PST_DISCARDING:
        _mstp_sm_PST_DISCARDING(p_db_port, ifname_ext);
        break;
    case PST_EVT_LISTEN:
        _mstp_sm_PST_LISTENING(p_db_port, ifname_ext);
        break;
    case PST_LEARNING:
        _mstp_sm_PST_LEARNING(p_db_port, ifname_ext);
        break;
    case PST_FORWARDING:
        _mstp_sm_PST_FORWARDING(p_db_port, ifname_ext);
        break;
    default:
        break;
    }

    return PM_E_NONE;
}

#define _17_31__13_36__TCM_Topology_Change_Machine_

/* TCM State (Topology Change Machine)
   802.1D.2004  P.178  17.31 Topology Change state machine
   802.1Q.2005  P.187  13.36 Topology Change state machine
*/
uint32 mstp_sm_tcm_state_table[TCM_EVENT_MAX][TCM_STATE_MAX] = 
{
  /* TCM_INACTIVE    TCM_DETECTED      TCM_ACTIVE        TCM_ACKNOWLEDGED  TCM_PROPAGATING  TCM_NOTIFIED_TC  TCM_NOTIFIED_TCN */
    {TCM_INACTIVE,   TCM_INACTIVE,     TCM_INACTIVE,     TCM_INACTIVE,     TCM_INACTIVE,    TCM_INACTIVE,     TCM_INACTIVE},    /* TCM_EVT_BEGIN */
    {TCM_STATE_MAX,  TCM_NOTIFIED_TC,  TCM_NOTIFIED_TC,  TCM_NOTIFIED_TC,  TCM_NOTIFIED_TC, TCM_NOTIFIED_TC,  TCM_NOTIFIED_TC}, /* TCM_EVT_RCVDTC */
    {TCM_STATE_MAX,  TCM_NOTIFIED_TCN, TCM_NOTIFIED_TCN, TCM_NOTIFIED_TCN, TCM_NOTIFIED_TCN,TCM_NOTIFIED_TCN, TCM_NOTIFIED_TCN},/* TCM_EVT_RCVDTCN */
    {TCM_ACKNOWLEDGED,TCM_ACKNOWLEDGED, TCM_ACKNOWLEDGED, TCM_ACKNOWLEDGED, TCM_ACKNOWLEDGED,TCM_ACKNOWLEDGED, TCM_ACKNOWLEDGED},/* TCM_EVT_RCVDTCACK */
    {TCM_STATE_MAX,  TCM_STATE_MAX,    TCM_STATE_MAX,    TCM_STATE_MAX,    TCM_STATE_MAX,   TCM_STATE_MAX,    TCM_STATE_MAX},   /* TCM_EVT_TCPROP */
    {TCM_STATE_MAX,  TCM_STATE_MAX,    TCM_PROPAGATING,  TCM_STATE_MAX,    TCM_STATE_MAX,   TCM_STATE_MAX,    TCM_STATE_MAX},   /* TCM_EVT_TCPROPNOTOPEREDGE */
    {TCM_DETECTED,   TCM_STATE_MAX,    TCM_STATE_MAX,    TCM_STATE_MAX,    TCM_STATE_MAX,   TCM_STATE_MAX,    TCM_STATE_MAX},   /* TCM_EVT_FORWARDNOTOPEREDGE */
    {TCM_STATE_MAX,  TCM_STATE_MAX,    TCM_INACTIVE,     TCM_INACTIVE,     TCM_INACTIVE,    TCM_INACTIVE,     TCM_INACTIVE},    /* TCM_EVT_OPEREDGE */
    {TCM_STATE_MAX,  TCM_STATE_MAX,    TCM_INACTIVE,     TCM_INACTIVE,     TCM_INACTIVE,    TCM_INACTIVE,     TCM_INACTIVE},    /* TCM_EVT_ROLENOTROOTDESIGNATEDPORT */
    {TCM_INACTIVE,   TCM_STATE_MAX,    TCM_STATE_MAX,    TCM_STATE_MAX,    TCM_STATE_MAX,   TCM_STATE_MAX,    TCM_STATE_MAX},   /* TCM_EVT_LEARNNOTFDBFLUSH */
};

/*
rcvdTc = rcvdTcn = rcvdTcAck = FALSE;
rcvdTc = tcProp = FALSE;
*/
static int32
_mstp_sm_TCM_LEARNING(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[TCM] %s enter LEARNING", ifname_ext);
    
    p_db_port->rcvdTc = p_db_port->rcvdTcn = p_db_port->rcvdTcAck = FALSE;
    p_db_port->rcvdTc = p_db_port->tcProp = FALSE;

    return PM_E_NONE;
}

/*
fdbFlush = TRUE;
tcWhile = 0; tcAck = FALSE;
*/
static int32
_mstp_sm_TCM_INACTIVE(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[TCM] %s enter INACTIVE", ifname_ext);
#if 0
    p_db_port->fdbFlush = TRUE;
    mstp_api_flush_fdb(p_db_port);
    p_db_port->fdbFlush = FALSE;
#endif

    CTC_TASK_STOP_TIMER(p_db_port->tcWhile_timer);
    p_db_port->tcAck = FALSE;

    p_db_port->tcm_state = TCM_INACTIVE;

    _mstp_sm_TCM_LEARNING(p_db_port, ifname_ext);
    
    return PM_E_NONE;
}

/*
17.21.7 newTcWhile()
*/
int32 
newTcWhile(tbl_mstp_port_t *p_db_port)
{
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

    if (p_db_port->sendRstp)
    {
        tcWhile_delay = p_db_port->designated_helloTime + 1;
        p_db_port->newInfo = TRUE;       
    }
    else 
    {
        tcWhile_delay = p_db_glb->root_maxAge + p_db_glb->root_fwdDelay;
    }

    p_db_port->tcWhile_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _mstp_sm_tcWhile_timer_fn, p_db_port, 
        tcWhile_delay);

    mstp_sm_update_topo_change();
    
    return PM_E_NONE;
}

/*
17.21.18 setTcPropTree()
*/
int32 
setTcPropTree(tbl_mstp_port_t *p_db_port)
{
    tbl_mstp_port_master_t *p_master = tbl_mstp_port_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_mstp_port_t *p_iter_port = NULL;

    CTCLIB_SLIST_LOOP(p_master->port_list, p_iter_port, listnode)
    {
        if (p_db_port == p_iter_port)
        {
            continue;
        }
        if (!p_iter_port->portEnabled)
        {
            continue;
        }
        if (!p_iter_port->tcProp && !p_iter_port->operEdge)
        {
            p_iter_port->tcProp = TRUE;
            if (p_iter_port->tcProp && (TCM_ACTIVE == p_iter_port->tcm_state))
            {
                mstp_sm_tcm_process_event(p_iter_port, TCM_EVT_TCPROPNOTOPEREDGE);
            }
        }
    }
    
    return PM_E_NONE;
}

static int32
_mstp_sm_TCM_TRIGGER_EVENT()
{
    return PM_E_NONE;
}

/*
newTcWhile(); setTcPropTree();
newInfo = TRUE;
*/
static int32
_mstp_sm_TCM_DETECTED(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[TCM] %s enter DETECTED", ifname_ext);
    
    newTcWhile(p_db_port);
    setTcPropTree(p_db_port);
    p_db_port->newInfo = TRUE;
    p_db_port->tcm_state = TCM_DETECTED;

    return PM_E_NONE;
}

/*
*/
static int32
_mstp_sm_TCM_ACTIVE(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[TCM] %s enter ACTIVE", ifname_ext);
    
    p_db_port->tcm_state = TCM_ACTIVE;

    if ((p_db_port->tcProp && !p_db_port->operEdge))
    {
        mstp_sm_tcm_process_event(p_db_port, TCM_EVT_TCPROPNOTOPEREDGE);
    }
    return PM_E_NONE;
}

/*
tcWhile = 0; rcvdTcAck = FALSE;
*/
static int32
_mstp_sm_TCM_ACKNOWLEDGED(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[TCM] %s enter ACKNOWLEDGED", ifname_ext);
    
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
_mstp_sm_TCM_PROPAGATING(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[TCM] %s enter PROPAGATING", ifname_ext);
    
    newTcWhile(p_db_port);
    p_db_port->fdbFlush = TRUE;
    mstp_api_flush_fdb(p_db_port);
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
_mstp_sm_TCM_NOTIFIED_TC(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[TCM] %s enter NOTIFIED_TC", ifname_ext);
    
    p_db_port->rcvdTcn = p_db_port->rcvdTc = FALSE;
    if (ROLE_DESIGNATED == p_db_port->role)
    {
        p_db_port->tcAck = TRUE;
    }
    setTcPropTree(p_db_port);
    p_db_port->tcm_state = TCM_NOTIFIED_TC;

    return PM_E_NONE;
}

/*
newTcWhile();
*/
static int32
_mstp_sm_TCM_NOTIFIED_TCN(tbl_mstp_port_t *p_db_port, char *ifname_ext)
{
    MSTP_DEBUG(STP_PROTO_CIST, "[TCM] %s enter NOTIFIED_TCN", ifname_ext);

    /* Modified by kcao 2017-01-10 for bug 42414, should ack for TCN */
    if (ROLE_DESIGNATED == p_db_port->role)
    {
        p_db_port->tcAck = TRUE;
    }
    newTcWhile(p_db_port);
    p_db_port->tcm_state = TCM_NOTIFIED_TCN;

    return PM_E_NONE;
}

/* 
  Figure 17-25  Topology Change state machine
*/
int32
mstp_sm_tcm_process_event(tbl_mstp_port_t *p_db_port, tcm_event_t event)
{
    tcm_state_t next_state = 0;
    char ifname_ext[IFNAME_EXT_SIZE];

    if (event >= TCM_EVENT_MAX)
    {
        return PM_E_FAIL;
    }

    IFNAME_ETH2FULL(p_db_port->key.name, ifname_ext);
    next_state =  mstp_sm_tcm_state_table[event][p_db_port->tcm_state];
    MSTP_DEBUG(STP_PROTO_CIST, "[%s] Interface %s recv event: %s, state: %s -> %s", 
            "TCM",
            ifname_ext,
            cdb_enum_val2str(tcm_event_strs, TCM_EVENT_MAX, event),
            cdb_enum_val2str(tcm_state_strs, TCM_STATE_MAX, p_db_port->tcm_state),
            cdb_enum_val2str(tcm_state_strs, TCM_STATE_MAX, next_state));

    switch (next_state)
    {
    case TCM_INACTIVE:
        _mstp_sm_TCM_INACTIVE(p_db_port, ifname_ext);
        _mstp_sm_TCM_LEARNING(p_db_port, ifname_ext);
        break;
    case TCM_DETECTED:
        _mstp_sm_TCM_DETECTED(p_db_port, ifname_ext);
        _mstp_sm_TCM_ACTIVE(p_db_port, ifname_ext);
        break;
    case TCM_ACTIVE:
        _mstp_sm_TCM_ACTIVE(p_db_port, ifname_ext);
        break;
    case TCM_ACKNOWLEDGED:
        _mstp_sm_TCM_ACKNOWLEDGED(p_db_port, ifname_ext);
        _mstp_sm_TCM_ACTIVE(p_db_port, ifname_ext);
        break;
    case TCM_PROPAGATING:
        /* modified by liwh for bug 37514, 2016-05-17 */
        if (!p_db_port->restricted_tcn)
        /* liwh end */
        {   
            _mstp_sm_TCM_PROPAGATING(p_db_port, ifname_ext);
        }
        else
        {
            /* Added by kcao for bug 39442, if restricted_tcn, should clear tcProp flag, otherwise, will deadloop */    
            p_db_port->tcProp = FALSE;
        }
        _mstp_sm_TCM_ACTIVE(p_db_port, ifname_ext);
        break;
    case TCM_NOTIFIED_TC:
        _mstp_sm_TCM_NOTIFIED_TC(p_db_port, ifname_ext);
        _mstp_sm_TCM_ACTIVE(p_db_port, ifname_ext);
        break;
    case TCM_NOTIFIED_TCN:
        _mstp_sm_TCM_NOTIFIED_TCN(p_db_port, ifname_ext);
        _mstp_sm_TCM_ACTIVE(p_db_port, ifname_ext);
        break;
    default:
        break;
    }

    _mstp_sm_TCM_TRIGGER_EVENT();
    
    return PM_E_NONE;
}

int32
mstp_sm_port_start_pti(tbl_mstp_port_t *p_db_port)
{
    CTC_TASK_STOP_TIMER(p_db_port->port_timer);    
    p_db_port->port_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _mstp_sm_pti_port_timer_fn, p_db_port, MSTP_PORT_TIMER_INTERVAL);

    return PM_E_NONE;
}

int32
mstp_sm_port_stop_pti(tbl_mstp_port_t *p_db_port)
{
    CTC_TASK_STOP_TIMER(p_db_port->port_timer);
    return PM_E_NONE;
}

int32
mstp_sm_port_start_prx(tbl_mstp_port_t *p_db_port)
{
    mstp_sm_prx_process_event(p_db_port, PRX_EVT_BEGIN);
    return PM_E_NONE;
}

int32
mstp_sm_port_stop_prx(tbl_mstp_port_t *p_db_port)
{
    CTC_TASK_STOP_TIMER(p_db_port->edgeDelayWhile_timer);
    p_db_port->prx_state = PRX_EVT_BEGIN;
    return PM_E_NONE;
}

int32
mstp_sm_port_start_ppm(tbl_mstp_port_t *p_db_port)
{
    mstp_sm_ppm_process_event(p_db_port, PPM_EVT_BEGIN);
    return PM_E_NONE;
}

int32
mstp_sm_port_stop_ppm(tbl_mstp_port_t *p_db_port)
{
    CTC_TASK_STOP_TIMER(p_db_port->mdelayWhile_timer);
    p_db_port->ppm_state = PPM_CHECKING_RSTP;
    return PM_E_NONE;
}

int32
mstp_sm_port_start_bdm(tbl_mstp_port_t *p_db_port)
{
    p_db_port->bdm_state = BDM_NOT_EDGE;
    if (p_db_port->adminEdge)
    {
        mstp_sm_bdm_process_event(p_db_port, BDM_EVT_BEGIN_ADMINEDGE);
    }
    else
    {
        mstp_sm_bdm_process_event(p_db_port, BDM_EVT_BEGIN_NOTADMINEDGE);
    }

    return PM_E_NONE;
}

int32
mstp_sm_port_stop_bdm(tbl_mstp_port_t *p_db_port)
{
    p_db_port->bdm_state = BDM_NOT_EDGE;
    return PM_E_NONE;
}

int32
mstp_sm_port_start_ptx(tbl_mstp_port_t *p_db_port)
{
    mstp_sm_ptx_process_event(p_db_port, PTX_EVT_BEGIN);
    return PM_E_NONE;
}

int32
mstp_sm_port_stop_ptx(tbl_mstp_port_t *p_db_port)
{
    CTC_TASK_STOP_TIMER(p_db_port->helloWhen_timer);
    p_db_port->ptx_state = PTX_TRANSMIT_INIT;
    return PM_E_NONE;
}

int32
mstp_sm_port_start_pim(tbl_mstp_port_t *p_db_port)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    tbl_msti_port_t *p_db_msti_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    
    mstp_sm_pim_process_event(p_db_port, PIM_EVT_BEGIN);
    mstp_sm_pim_process_event(p_db_port, PIM_EVT_PORTENABLED);

    if (MSTP_TYPE_MSTP == p_db_glb->type)
    {
        CTCLIB_SLIST_LOOP(p_db_port->msti_list.obj_list, p_db_msti_port, listnode)
        {
            msti_sm_port_start_pim(p_db_msti_port);
        }
    }

    return PM_E_NONE;
}

int32
mstp_sm_port_stop_pim(tbl_mstp_port_t *p_db_port)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    tbl_msti_port_t *p_db_msti_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    
    p_db_port->pim_state = PIM_DISABLED;

    if (MSTP_TYPE_MSTP == p_db_glb->type)
    {
        CTCLIB_SLIST_LOOP(p_db_port->msti_list.obj_list, p_db_msti_port, listnode)
        {
            msti_sm_port_stop_pim(p_db_msti_port);
        }
    }
    
    return PM_E_NONE;
}

int32
mstp_sm_port_start_prs(tbl_mstp_global_t *p_db_glb)
{
    mstp_sm_prs_process_event(p_db_glb, PRS_EVT_BEGIN);
    return PM_E_NONE;
}

int32
mstp_sm_port_stop_prs(tbl_mstp_global_t *p_db_glb)
{
    p_db_glb->prs_state = PRS_INIT_BRIDGE;
    return PM_E_NONE;
}

int32
mstp_sm_port_start_prt(tbl_mstp_port_t *p_db_port)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    tbl_msti_port_t *p_db_msti_port = NULL;
    ctclib_slistnode_t *listnode = NULL;

    mstp_sm_prt_process_event(p_db_port, PRT_EVT_BEGIN);

    if (MSTP_TYPE_MSTP == p_db_glb->type)
    {
        CTCLIB_SLIST_LOOP(p_db_port->msti_list.obj_list, p_db_msti_port, listnode)
        {
            msti_sm_port_start_prt(p_db_msti_port);
        }
    }
    
    return PM_E_NONE;
}

int32
mstp_sm_port_stop_prt(tbl_mstp_port_t *p_db_port)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    tbl_msti_port_t *p_db_msti_port = NULL;
    ctclib_slistnode_t *listnode = NULL;

    CTC_TASK_STOP_TIMER(p_db_port->fdWhile_timer);
    CTC_TASK_STOP_TIMER(p_db_port->rrWhile_timer);
    CTC_TASK_STOP_TIMER(p_db_port->rbWhile_timer);
    p_db_port->prt_state = PRT_INIT_PORT;

    if (MSTP_TYPE_MSTP == p_db_glb->type)
    {
        CTCLIB_SLIST_LOOP(p_db_port->msti_list.obj_list, p_db_msti_port, listnode)
        {
            msti_sm_port_stop_prt(p_db_msti_port);
        }
    }
    
    return PM_E_NONE;
}

int32
mstp_sm_port_start_tcm(tbl_mstp_port_t *p_db_port)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    tbl_msti_port_t *p_db_msti_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    
    mstp_sm_tcm_process_event(p_db_port, TCM_EVT_BEGIN);

    if (MSTP_TYPE_MSTP == p_db_glb->type)
    {
        CTCLIB_SLIST_LOOP(p_db_port->msti_list.obj_list, p_db_msti_port, listnode)
        {
            msti_sm_port_start_tcm(p_db_msti_port);
        }
    }
    
    return PM_E_NONE;
}

int32
mstp_sm_port_stop_tcm(tbl_mstp_port_t *p_db_port)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    tbl_msti_port_t *p_db_msti_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    
    CTC_TASK_STOP_TIMER(p_db_port->tcWhile_timer);
    p_db_port->tcm_state = TCM_INACTIVE;

    if (MSTP_TYPE_MSTP == p_db_glb->type)
    {
        CTCLIB_SLIST_LOOP(p_db_port->msti_list.obj_list, p_db_msti_port, listnode)
        {
            msti_sm_port_stop_tcm(p_db_msti_port);
        }
    }

    return PM_E_NONE;
}

int32
mstp_sm_port_start_pst(tbl_mstp_port_t *p_db_port)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    tbl_msti_port_t *p_db_msti_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    
    mstp_sm_pst_process_event(p_db_port, PST_EVT_BEGIN);
    
    if (MSTP_TYPE_MSTP == p_db_glb->type)
    {
        CTCLIB_SLIST_LOOP(p_db_port->msti_list.obj_list, p_db_msti_port, listnode)
        {
            msti_sm_port_start_pst(p_db_msti_port);
        }
    }
    
    return PM_E_NONE;
}

int32
mstp_sm_port_stop_pst(tbl_mstp_port_t *p_db_port)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    tbl_msti_port_t *p_db_msti_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    
    p_db_port->pst_state = PST_DISCARDING;

    if (MSTP_TYPE_MSTP == p_db_glb->type)
    {
        CTCLIB_SLIST_LOOP(p_db_port->msti_list.obj_list, p_db_msti_port, listnode)
        {
            msti_sm_port_stop_pst(p_db_msti_port);
        }
    }

    return PM_E_NONE;
}

int32
mstp_sm_init()
{
    uint32 tx_bpdu = FALSE;
    /* update Bridge root priority vector */
    updtRolesTree(&tx_bpdu);
    if (tx_bpdu)
    {
        mstp_api_all_port_tx_pdu();
    }
    return PM_E_NONE;
}

/* modified by liwh for bug 37514, 2016-05-17 */
int32
mstp_sm_role_selection(void)
{
    tbl_mstp_global_t *p_db_glb = NULL;
    
    p_db_glb = tbl_mstp_global_get_mstp_global();
    if (!p_db_glb)
    {
        return PM_E_NONE;
    }

    _mstp_sm_PRS_ROLE_SELECTION(p_db_glb);

    return PM_E_NONE;
}
/* liwh end */

#endif /* CONFIG_MSTP */

