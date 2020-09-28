
#include "proto.h"
#include "switch.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_lag_global_define.h"
#include "gen/tbl_lag_global.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_mlag_define.h"
#include "gen/tbl_mlag.h"
#include "gen/tbl_mlag_peer_define.h"
#include "gen/tbl_mlag_peer.h"
#include "gen/tbl_mlag_port_define.h"
#include "gen/tbl_mlag_port.h"
#include "gen/tbl_errdisable_define.h"
#include "gen/tbl_errdisable.h"
#include "mlag.h"
#include "mlag_api.h"
#include "mlag_fsm.h"
#include "mlag_sock.h"
#include "mlag_encode.h"

static int32
mpf_action_invalid(tbl_mlag_peer_t *peer, mlag_event_t mpf_event);
static int32
mpf_action_idle(tbl_mlag_peer_t *peer, mlag_event_t mpf_event);
static int32
mpf_action_active(tbl_mlag_peer_t *peer, mlag_event_t mpf_event);
static int32
mpf_action_open_sent(tbl_mlag_peer_t *peer, mlag_event_t mpf_event);
static int32
mpf_action_open_cfm(tbl_mlag_peer_t *peer, mlag_event_t mpf_event);
static int32
mpf_action_established(tbl_mlag_peer_t *peer, mlag_event_t mpf_event);

mpf_act_func_t mpf_action_func[MPF_STATE_MAX] =
{
    mpf_action_invalid,
    mpf_action_idle,
    mpf_action_active,
    mpf_action_open_sent,
    mpf_action_open_cfm,
    mpf_action_established
};

/* Change MLAG Peer FSM State */
int32
mpf_change_state(tbl_mlag_peer_t *peer, mlag_state_t mpf_state)
{
    MLAG_DEBUG(MLAG_PROTO, "[FSM] State Change: %s(%u)->%s(%u)",
               MLAG_PEER_FSM_STATE_STR(peer->mpf_state),
               peer->mpf_state, 
               MLAG_PEER_FSM_STATE_STR(mpf_state),
               mpf_state);

    if (MPF_STATE_ESTABLISHED == peer->mpf_state)
    {
        logid_sys(LOG_MLAG_3_BREAKOFF);
    }
    /* Change to new state */
    peer->mpf_state = mpf_state;
    tbl_mlag_peer_set_mlag_peer_field(peer, TBL_MLAG_PEER_FLD_MPF_STATE);

    return PM_E_NONE;
}

/* MLAG Peer FSM generic timer-jitter generator */
uint32
mpf_timer_generate_jitter(uint32 time)
{
    uint32 jittered_time = 0;
    uint32 rand_val = 0;

    rand_val = 75 + (uint32)(25.0 * sal_rand()/RAND_MAX);

    jittered_time = (time * rand_val / 100);

    return (jittered_time > 1 ? jittered_time : 1);
}

int32
mpf_action_invalid(tbl_mlag_peer_t *peer, mlag_event_t mpf_event)
{
    int32 rc = PM_E_NONE;

    log_sys(M_MOD_INTERNAL, E_ERROR, "[FSM] State: %s invalid Event: %s", 
        MLAG_PEER_FSM_STATE_STR(peer->mpf_state), 
        MLAG_PEER_FSM_EVENT_STR(mpf_event));

    return rc;
}

/* MLAG Peer FSM auto-start timer */
void
mpf_timer_auto_start_fn(void *p_data)
{
    tbl_mlag_peer_t *peer = (tbl_mlag_peer_t*)p_data;

    MLAG_DEBUG(MLAG_TIMER, "[FSM] Auto-Start Timer Expired");

    peer->t_auto_start = NULL;
    mpn_sock_client_connect();

    return;
}

void
mpf_timer_keepalive_fn(void *p_data)
{
    tbl_mlag_peer_t *peer = (tbl_mlag_peer_t*)p_data;

    MLAG_DEBUG(MLAG_TIMER, "[FSM] Keepalive Timer Expired");

    peer->t_keepalive = NULL;
    mpf_process_event(MPF_EVENT_KEEPALIVE_EXP);

    return;
}

void
mpf_timer_holdtime_fn(void *p_data)
{
    tbl_mlag_peer_t *peer = (tbl_mlag_peer_t*)p_data;

    MLAG_DEBUG(MLAG_TIMER, "[FSM] Hold Timer Expired");

    peer->t_holdtime = NULL;
    mpf_process_event(MPF_EVENT_HOLD_EXP);

    return;
}

int32
mpf_action_idle(tbl_mlag_peer_t *peer, mlag_event_t mpf_event)
{
    int32 rc = PM_E_NONE;

    switch (mpf_event)
    {
    case MPF_EVENT_MANUAL_START:
        peer->v_auto_start = MLAG_DEFAULT_AUTO_START;
        CTC_TASK_STOP_TIMER(peer->t_auto_start);
        peer->t_auto_start = ctc_task_add_timer(TASK_PRI_NORMAL, mpf_timer_auto_start_fn, 
            peer, peer->v_auto_start);
        /* server or client enter connect state */
        mpf_change_state(peer, MPF_STATE_ACTIVE);
        break;

    default:
        MLAG_DEBUG(MLAG_PROTO, "[FSM] State: %s ignoring Event: %s", 
            MLAG_PEER_FSM_STATE_STR(peer->mpf_state), 
            MLAG_PEER_FSM_EVENT_STR(mpf_event));
        break;
    }

    return rc;
}

int32
mpf_action_active(tbl_mlag_peer_t *peer, mlag_event_t mpf_event)
{
    int32 rc = PM_E_NONE;

    switch (mpf_event)
    {
    case MPF_EVENT_MANUAL_STOP:
        mlag_api_peer_session_close(peer);
        break;

    case MPF_EVENT_TCP_CONN_CFM:
        peer->v_holdtime = peer->holdtime;
        mlag_send_open(peer);
        mpf_change_state(peer, MPF_STATE_OPEN_SENT);
        break;

    default:
        MLAG_DEBUG(MLAG_PROTO, "[FSM] State: %s ignoring Event: %s", 
            MLAG_PEER_FSM_STATE_STR(peer->mpf_state), 
            MLAG_PEER_FSM_EVENT_STR(mpf_event));
        break;
    }

    return rc; 
}

int32
mpf_collision_detect_check(tbl_mlag_peer_t *peer)
{
    tbl_mlag_t *mlag = tbl_mlag_get_mlag();
    if (MLAG_ROLE_MASTER == mlag->role)
    {
        return PM_E_NONE;
    }

    return PM_E_FAIL;
}

int32
mpf_action_open_sent(tbl_mlag_peer_t *peer, mlag_event_t mpf_event)
{
    mlag_sock_connected_t connected;
    int32 rc = PM_E_NONE;

    switch (mpf_event)
    {
    case MPF_EVENT_MANUAL_STOP:     /* fall through */
    case MPF_EVENT_TCP_CONN_FAIL:
        mlag_api_peer_session_close(peer);
        break;

    case MPF_EVENT_OPEN_VALID:
        /* if has server connected, should close client connect timer */
        connected = mpn_sock_check_connected(peer);
        if (MLAG_SOCK_CONNECTED_SERVER == connected)
        {
            mpn_sock_client_disconnect();
        }
        rc = mpf_collision_detect_check(peer);
        mlag_send_keepalive(peer);
        if (peer->v_keepalive)
        {
            CTC_TASK_STOP_TIMER(peer->t_keepalive);
            peer->t_keepalive = ctc_task_add_timer(TASK_PRI_NORMAL, mpf_timer_keepalive_fn, 
                peer, peer->v_keepalive);
        }
        if (peer->v_holdtime)
        {
            CTC_TASK_STOP_TIMER(peer->t_holdtime);
            peer->t_holdtime = ctc_task_add_timer(TASK_PRI_NORMAL, mpf_timer_holdtime_fn, 
                peer, peer->v_holdtime);
        }
        mpf_change_state(peer, MPF_STATE_OPEN_CFM);
        break;

    default:
        MLAG_DEBUG(MLAG_PROTO, "[FSM] State: %s ignoring Event: %s", 
            MLAG_PEER_FSM_STATE_STR(peer->mpf_state), 
            MLAG_PEER_FSM_EVENT_STR(mpf_event));
        break;
    }

    return rc;   
}

int32
mpf_action_open_cfm(tbl_mlag_peer_t *peer, mlag_event_t mpf_event)
{
    mlag_sock_connected_t connected;
    char buf[CMD_BUF_64];
    int32 rc = PM_E_NONE;

    switch (mpf_event)
    {
    case MPF_EVENT_MANUAL_STOP:     /* fall through */
    case MPF_EVENT_HOLD_EXP:        /* fall through */
    case MPF_EVENT_TCP_CONN_FAIL:
        mlag_api_peer_session_close(peer);
        if (MPF_EVENT_HOLD_EXP == mpf_event)
        {
            mlag_api_restart_socket(peer);
        }
        break;

    case MPF_EVENT_TCP_CONN_CFM:
        connected = mpn_sock_check_connected(peer);
        if (MLAG_SOCK_CONNECTED_BOTH == connected)
        {
            /* if has both connected, should check collision and close master's client connection */
            rc = mpf_collision_detect_check(peer);
#if 0
            log_sys(M_MOD_INTERNAL, E_ERROR, "[FSM] is %s, and connected is %u",
                (PM_E_NONE == rc) ? "master" : "slave", connected);
#endif
            if (PM_E_NONE == rc)
            {
                /* disconnect but not deinit */
                mpn_sock_client_disconnect();
            }
        }
        break;

    case MPF_EVENT_KEEPALIVE_VALID:
        if (peer->v_holdtime)
        {
            CTC_TASK_STOP_TIMER(peer->t_holdtime);
            peer->t_holdtime = ctc_task_add_timer(TASK_PRI_NORMAL, mpf_timer_holdtime_fn, 
                peer, peer->v_holdtime);
        }
        peer->established++;
        peer->uptime = ctc_time_boottime_sec(NULL);
        mpf_change_state (peer, MPF_STATE_ESTABLISHED);
        mlag_api_sync_peer_conf_all_ports(peer);
        mlag_api_sync_peer_fdb(peer);
        mlag_send_syspri(peer);
        mlag_api_established_reload_recovery();
        logid_sys(LOG_MLAG_4_ESTABLISH, cdb_addr_val2str(buf, CMD_BUF_64, &peer->peer_addr));
        break;

    case MPF_EVENT_KEEPALIVE_EXP:
        mlag_send_keepalive(peer);
        if (peer->v_holdtime && peer->v_keepalive)
        {
            CTC_TASK_STOP_TIMER(peer->t_keepalive);
            peer->t_keepalive = ctc_task_add_timer(TASK_PRI_NORMAL, mpf_timer_keepalive_fn, 
                peer, peer->v_keepalive);
        }
        break;
      
    default:
        MLAG_DEBUG(MLAG_PROTO, "[FSM] State: %s ignoring Event: %s", 
            MLAG_PEER_FSM_STATE_STR(peer->mpf_state), 
            MLAG_PEER_FSM_EVENT_STR(mpf_event));
        break;
    }

    return rc;   
}

void
mpf_rx_disconnect_fn(void *p_data)
{
    tbl_mlag_peer_t *peer = tbl_mlag_peer_get_mlag_peer();
    mlag_api_restart_socket(peer);
    return;
}
            
int32
mpf_action_established(tbl_mlag_peer_t *peer, mlag_event_t mpf_event)
{
    int32 rc = PM_E_NONE;

    switch (mpf_event)
    {
    case MPF_EVENT_MANUAL_STOP:     /* fall through */
    case MPF_EVENT_HOLD_EXP:        /* fall through */
    case MPF_EVENT_SOCK_TX_FAIL:    /* fall through */
    case MPF_EVENT_TCP_CONN_FAIL:   /* fall through */
    case MPF_EVENT_RX_DISCONNECT:
        peer->dropped++;
        /* should before peer_routemac changes */
        mlag_api_peer_fdb_flush();
        mlag_api_flush_fdb_sync_in();
        mlag_api_clear_peer_conf();
        mlag_api_peer_session_close(peer);
        if (MPF_EVENT_HOLD_EXP == mpf_event)
        {
            mlag_api_restart_socket(peer);
        }
        if (MPF_EVENT_RX_DISCONNECT == mpf_event)
        {
            ctc_task_add_timer(TASK_PRI_NORMAL, mpf_rx_disconnect_fn, NULL, 0);
        }
        break;

    case MPF_EVENT_KEEPALIVE_EXP:
        mlag_send_keepalive(peer);
        if (peer->v_holdtime && peer->v_keepalive)
        {
            CTC_TASK_STOP_TIMER(peer->t_keepalive);
            peer->t_keepalive = ctc_task_add_timer(TASK_PRI_NORMAL, mpf_timer_keepalive_fn, 
                peer, peer->v_keepalive);
        }
        break;

    case MPF_EVENT_KEEPALIVE_VALID:
        if (peer->v_holdtime)
        {
            CTC_TASK_STOP_TIMER(peer->t_holdtime);
            peer->t_holdtime = ctc_task_add_timer(TASK_PRI_NORMAL, mpf_timer_holdtime_fn, 
                peer, peer->v_holdtime);
        }
        break;

    default:
        MLAG_DEBUG(MLAG_PROTO, "[FSM] State: %s ignoring Event: %s", 
            MLAG_PEER_FSM_STATE_STR(peer->mpf_state), 
            MLAG_PEER_FSM_EVENT_STR(mpf_event));
        break;
    }

    return rc;  
}

int32
mpf_process_event(mlag_event_t mpf_event)
{
    tbl_mlag_peer_t *peer = tbl_mlag_peer_get_mlag_peer();
    int32 rc = PM_E_NONE;
    
    MLAG_DEBUG(MLAG_PROTO, "[FSM] State: %s Event: %s", 
        MLAG_PEER_FSM_STATE_STR(peer->mpf_state), 
        MLAG_PEER_FSM_EVENT_STR(mpf_event));

    if (peer->mpf_state >= MPF_STATE_MAX)
    {
        SAL_ASSERT(0);
    }

    if (mpf_action_func[peer->mpf_state])
    {
        rc = mpf_action_func[peer->mpf_state](peer, mpf_event);
    }
    else
    {
        rc = PM_E_FAIL;
    }

    return rc;
}

