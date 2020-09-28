#include "proto.h"
#include "gen/tbl_aps_group_info_define.h"
#include "gen/tbl_aps_group_info.h"
#include "gen/tbl_lsp_aps_group_define.h"
#include "gen/tbl_lsp_aps_group.h"
#include "gen/tbl_pw_aps_group_define.h"
#include "gen/tbl_pw_aps_group.h"
#include "gen/tbl_tpoam_exec_global_define.h"
#include "gen/tbl_tpoam_exec_global.h"
#include "gen/tbl_tpoam_global_define.h"
#include "gen/tbl_tpoam_global.h"
#include "gen/tbl_oam_session_define.h"
#include "gen/tbl_oam_session.h"
#include "gen/tbl_lsp_pe_define.h"
#include "gen/tbl_lsp_pe.h"
#include "gen/tbl_lsp_p_define.h"
#include "gen/tbl_lsp_p.h"
#include "gen/tbl_pw_tpe_define.h"
#include "gen/tbl_pw_tpe.h"
#include "glb_g8131_define.h"
#include "tpoam_sock.h"
#include "tpoam_pdu.h"
#include "tpoam_error.h"
//#include "tpoam_api.h"
#include "glb_tp_oam.h"
#include "g8131_api.h"
#include "g8131_fsm.h"
#include "g8131_pdu.h"
#include "fei.h"
#include "fei_g8131.h"

extern int32
tpoam_api_stats_add_tx(tpoam_stats_tx_item_t* pst_tx_stats, uint8 opcode);

static int32
g8131_map_fsm_state_to_request_state(uint32 fsm_state, uint32 *request_state)
{
    if (fsm_state >= G8131_FSM_STATE_MAX)
    {
        return PM_E_FAIL;
    }

    switch (fsm_state)
    {
    case G8131_FSM_STATE_NR_W:
        *request_state = G8131_REQ_STATE_NR;
          break;
    case G8131_FSM_STATE_NR_P:
        *request_state = G8131_REQ_STATE_NR;
          break;
    case G8131_FSM_STATE_LO:
        *request_state = G8131_REQ_STATE_LO;
          break;
    case G8131_FSM_STATE_FS:
        *request_state = G8131_REQ_STATE_FS;
          break;
    case G8131_FSM_STATE_SF_W:
        *request_state = G8131_REQ_STATE_SF_W;
          break;
    case G8131_FSM_STATE_SF_P:
        *request_state = G8131_REQ_STATE_SF_P;
          break;
    case G8131_FSM_STATE_SD_W:
        *request_state = G8131_REQ_STATE_SD;
          break;
    case G8131_FSM_STATE_SD_P:
        *request_state = G8131_REQ_STATE_SD;
          break;
    case G8131_FSM_STATE_MS_P:
        *request_state = G8131_REQ_STATE_MS;
          break;
    case G8131_FSM_STATE_MS_W:
        *request_state = G8131_REQ_STATE_MS;
          break;
    case G8131_FSM_STATE_WTR:
        *request_state = G8131_REQ_STATE_WTR;
          break;
    case G8131_FSM_STATE_DNR:
        *request_state = G8131_REQ_STATE_DNR;
          break;
    case G8131_FSM_STATE_EXER_W:
        *request_state = G8131_REQ_STATE_EXER;
          break;
    case G8131_FSM_STATE_EXER_P:
        *request_state = G8131_REQ_STATE_EXER;
          break;
    case G8131_FSM_STATE_RR_W:
        *request_state = G8131_REQ_STATE_RR;
          break;
    case G8131_FSM_STATE_RR_P:
        *request_state = G8131_REQ_STATE_RR;
          break;
    default:
        return PM_E_FAIL;
    }

    return PM_E_NONE;
}

static int32
g8131_get_bridged_signal(uint32 bridge_permanent, uint32 requested_signal)
{
    if (bridge_permanent)
    {
        return G8131_NORMAL_TRAFFIC_SIGNAL;
    }
    else
    {
        return requested_signal;
    }
}

int32
g8131_pdu_map_request_state_to_fsm_event(uint32 request_state, uint32 requested_signal, uint32 *fsm_event)
{
    if (request_state >= G8131_REQ_STATE_MAX)
    {
        return PM_E_FAIL;
    }

    switch (request_state)
    {
    case G8131_REQ_STATE_NR:
        if(G8131_REQUEST_SIGNAL_NULL == requested_signal)
            *fsm_event = G8131_FSM_R_EVENT_NR_NULL;
        else
            *fsm_event = G8131_FSM_R_EVENT_NR_NORMAL;
        break;
    case G8131_REQ_STATE_DNR:
        *fsm_event = G8131_FSM_R_EVENT_DNR;
        break;
    case G8131_REQ_STATE_RR:
        if(G8131_REQUEST_SIGNAL_NULL == requested_signal)
            *fsm_event = G8131_FSM_R_EVENT_RR_W;
        else
            *fsm_event = G8131_FSM_R_EVENT_RR_P;
        break;
    case G8131_REQ_STATE_EXER:
        if(G8131_REQUEST_SIGNAL_NULL == requested_signal)
            *fsm_event = G8131_FSM_R_EVENT_EXER_W;
        else
            *fsm_event = G8131_FSM_R_EVENT_EXER_P;
        break;
    case G8131_REQ_STATE_WTR:
        *fsm_event = G8131_FSM_R_EVENT_WTR;
        break;
    case G8131_REQ_STATE_MS:
        if(G8131_REQUEST_SIGNAL_NULL == requested_signal)
            *fsm_event = G8131_FSM_R_EVENT_MS_W;
        else
            *fsm_event = G8131_FSM_R_EVENT_MS_P;
        break;
    case G8131_REQ_STATE_SD:
        if(G8131_REQUEST_SIGNAL_NULL == requested_signal)
            *fsm_event = G8131_FSM_R_EVENT_SD_P;
        else
            *fsm_event = G8131_FSM_R_EVENT_SD_W;
        break;
    case G8131_REQ_STATE_SF_W:
        *fsm_event = G8131_FSM_R_EVENT_SF_W;
        break;
   case G8131_REQ_STATE_FS:
        *fsm_event = G8131_FSM_R_EVENT_FS;
        break;
    case G8131_REQ_STATE_SF_P:
        *fsm_event = G8131_FSM_R_EVENT_SF_P;
        break;
    case G8131_REQ_STATE_LO:
        *fsm_event = G8131_FSM_R_EVENT_LO;
        break;

    default:
        return PM_E_FAIL;
    }

    return PM_E_NONE;
}

int32
validate_g8131_aps_pdu(tbl_aps_group_info_t* p_db_ag_info, g8131_aps_pdu_t *pdu)
{
    if (!p_db_ag_info || !pdu)
    {
        return G8131_INVALID_PTR;
    }

    if (pdu->A != p_db_ag_info->aps_channel)
    {
         /* If the "A" bit mismatches, the side expecting APS will fall
            back to 1 + 1 unidirectional switching without APS communication */
         return G8131_A_BIT_MISMATCH_ERROR;
    }

    if (pdu->B != !p_db_ag_info->bridge_permanent)
    {
        /* If the "B" bit mismatches, the selector is released since
           1:1 and 1+1 are incompatible*/

       return G8131_B_BIT_MISMATCH_ERROR;
    }

    if (pdu->D != p_db_ag_info->direction)
    {
        /* If the "D" bit mismatches, the bidirectional side will fallback
           to unidirectional switching */
        return G8131_D_BIT_MISMATCH_ERROR;
    }

    if (pdu->T!= p_db_ag_info->bridge_type)
    {
        /* If the "T" bit mismatches, the side using a broadcast bridge will 
           fall back to using a selector bridge. */
        return G8131_T_BIT_MISMATCH_ERROR;
    }

    if (pdu->req_state ==  G8131_REQ_STATE_EXER ||
        pdu->req_state == G8131_REQ_STATE_DNR )
    {
        return G8131_HAND_OVER_REQ_TO_EXER_DNR_LOGIC;
    }

    return G8131_HAND_OVER_REQ_TO_FAR_END_FSM;
}

int32
g8131_pdu_aps_encode(tbl_oam_session_t* p_db_oam_session, tbl_aps_group_info_t * p_db_ag_info, uint8* p_data, int32* p_length)
{
    uint32 level_version = 0;
    uint32 request_state = 0;
    uint32 req_protocol = 0;
    uint32 bridged_signal = 0;
    uint32 bridge_type = 0;

    uint8*  p_curr_data = p_data;
    int32*  size = NULL;
    uint8** ppnt = NULL;
    size = p_length;
    ppnt = &p_curr_data;

     /*
       0                   1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |0 0 0 1|0 0 0 0|0 0 0 0 0 0 0 0|  Y.1731 Channel Type (0xXXXX) |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      | MEL | Version |OpCode (APS=39)|     Flags     | TLV Offset(4) |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |Req  | A B D R |Request Signal     |Bridged Signal |Reserved   |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |   End TLV (0) |
      +-+-+-+-+-+-+-+-+
    */

    level_version = ((p_db_oam_session->level & TPOAM_Y1731_PDU_MEL_MASK)<< TPOAM_Y1731_PDU_MEL_SHIFT)
                    | (TPOAM_Y1731_PDU_VER & TPOAM_Y1731_PDU_VER_MASK);
    G8131_ENCODE_PUTC(level_version);
    G8131_ENCODE_PUTC(TPOAM_Y1731_PDU_APS);
    G8131_ENCODE_PUTC(G8131_INVALID_VAL);
    G8131_ENCODE_PUTC(G8131_APS_FIRST_TLV_OFFSET);

    g8131_map_fsm_state_to_request_state(p_db_ag_info->current_state, &request_state);
    /* encode requested stated */
    req_protocol = request_state;
    req_protocol = (req_protocol << 4 ) & 0xf0;

    /* encode A -- APS or NO-APS */
    if (p_db_ag_info->aps_channel)
    {
        GLB_SET_FLAG(req_protocol, G8131_APS_CHANNEL_MASK);
    }
    
    /* encode B  permenant bridge/ no permenant bridge*/
    if (!p_db_ag_info->bridge_permanent)
    {
        GLB_SET_FLAG(req_protocol, G8131_NO_PERMANENT_BR);
    }
    
    /* encode D  Uni/Bi */
    if (p_db_ag_info->direction)
    {
        GLB_SET_FLAG(req_protocol, G8131_DIRECTION_BI);
    }
    
    /* encode R  Revertive/Non-Revertive */
    if (p_db_ag_info->is_revertive)
    {
        GLB_SET_FLAG(req_protocol, G8131_REVERTIVE);
    }

    /* ecoding Req/State ABDR */
    G8131_ENCODE_PUTC(req_protocol);

    /* encoding Requested Signal */
    G8131_ENCODE_PUTC(p_db_ag_info->requested_signal);

    /* encoding Bridged Signal */
    bridged_signal = g8131_get_bridged_signal(p_db_ag_info->bridge_permanent, p_db_ag_info->requested_signal);
    G8131_ENCODE_PUTC(bridged_signal);

    /* Encoding T bit and reserved field */
    bridge_type = p_db_ag_info->bridge_type;
    G8131_ENCODE_PUTC((bridge_type << 7) & 0x80);

    /* encoding end tlv */
    G8131_ENCODE_PUTC(TPOAM_Y1731_PDU_END_TLV);
    
    return PM_E_NONE;
}

int32
g8131_pdu_aps_decode(uint8 *p_buf, uint32 len, g8131_aps_pdu_t *pdu)
{
    int32   len_left = len;
    int32*  size = NULL;
    uint8** ppnt = NULL;
    size = &len_left;
    ppnt = &p_buf;

    uint32  reqstate_abdr = 0;

    G8131_DECODE_GETC(reqstate_abdr);

    /* extracting incoming event */
    pdu->req_state = (reqstate_abdr >> 4);
    
    /* extracting A. denotes APS channel or NO-APS Channel  */
    pdu->A = GLB_FLAG_ISSET(reqstate_abdr, G8131_APS_CHANNEL_MASK);

    /* extracting B. denotes permenant bridge or no-permenant bridge */
    pdu->B = GLB_FLAG_ISSET(reqstate_abdr, G8131_NO_PERMANENT_BR);

    /* extracting D. denotes uni-directional or bi-directional */
    pdu->D = FLAG_ISSET(reqstate_abdr, G8131_DIRECTION_BI);

    /* extracting R. denotes revertive channel or non-revertive channel*/
    pdu->R = FLAG_ISSET(reqstate_abdr, G8131_REVERTIVE);

    G8131_DECODE_GETC(pdu->req_signal);

    G8131_DECODE_GETC(pdu->bridged_signal);

    G8131_DECODE_GETC(pdu->reserved);

    pdu->T = (pdu->reserved >> 7);

    return PM_E_NONE;
}

int32
g8131_pdu_aps_tx(tbl_aps_group_info_t * p_db_ag_info)
{
    tpoam_sk_buff_t *tx_skb;

    tx_skb = tpoam_alloc_skb(TPOAM_PKT_BUF_SIZE);
    if (!tx_skb)
    {
        G8131_TX_DEBUG("Cannot allocate memory for issue message on aps group %u", p_db_ag_info->key.aps_group_id);
        goto TPOAM_FINALLY;
    }
    
    tbl_tpoam_global_t *p_db_tpoam_glb = tbl_tpoam_global_get_tpoam_global();
    tbl_oam_session_t* p_db_oam_session = NULL;
    int32 pkt_size = 0;
    uint8 *g8131_aps_data = tx_skb->data;
    int32 g8131_aps_data_left = TPOAM_PKT_BUF_SIZE;
    int32 rc = PM_E_NONE;

    if (G8131_ACTIVE_NONE == p_db_ag_info->active_path)
    {
        goto TPOAM_FINALLY;
    }
    
    p_db_oam_session = g8131_get_oam_by_aps_group(p_db_ag_info->protection_name, p_db_ag_info->aps_group_type);
    if (NULL == p_db_oam_session)
    {
        G8131_TX_DEBUG("Tx fail: can not find the oam session by aps group %u", p_db_ag_info->key.aps_group_id);
        goto TPOAM_FINALLY;
    }

    g8131_pdu_aps_encode(p_db_oam_session, p_db_ag_info, g8131_aps_data, &g8131_aps_data_left);
    pkt_size = TPOAM_PKT_BUF_SIZE - g8131_aps_data_left;
    tx_skb->len = pkt_size;
    
    rc = tpoam_sock_tx_new(p_db_oam_session, TPOAM_SOCK_DIRECTION_PE, pkt_size, tx_skb,
            TPOAM_EXP_DEF, TPOAM_SOCK_TTL_DEF, TPOAM_SOCK_DM_FLAG_UNSET, FALSE, FALSE, TPOAM_Y1731_PDU_APS);
    if (TPOAM_E_INTF_DOWN == rc)
    {
        goto TPOAM_FINALLY;
    }
    else if (rc < 0)
    {
        /**/
        G8131_TX_DEBUG("Tx fail: Tpoam sock tx fail on oam session %u", p_db_oam_session->key.session_id);
        tpoam_free_skb(&tx_skb);
        return PM_E_FAIL;
    }
    tpoam_api_stats_add_tx(&p_db_oam_session->oam_stats.tx, TPOAM_Y1731_PDU_APS);
    tpoam_api_stats_add_tx(&p_db_tpoam_glb->oam_glb_stats.tx, TPOAM_Y1731_PDU_APS); 
    /*Added by yejl for bug 47678, 2018-06-19*/
    G8131_TX_DEBUG("APS group %u: TX success! State: %s  Event: %s\n", 
        p_db_ag_info->key.aps_group_id, 
        G8131_FSM_STATE_STR(p_db_ag_info->current_state), 
        G8131_FSM_EVENT_STR(p_db_ag_info->current_event));

TPOAM_FINALLY:

    tpoam_free_skb(&tx_skb);
    
    return PM_E_NONE;
}

int32
g8131_pdu_aps_rx(tbl_oam_session_t * p_oam_session, tpoam_rx_pdu_t* pst_pdu)
{
    int32 rc = PM_E_NONE;
    tbl_aps_group_info_t* p_db_ag_info = NULL;  
    g8131_aps_pdu_t aps_pdu;
    uint32 fsm_event = 0;

    /*If the oam type is not lsp nor pw, return*/
    if ((p_oam_session->oam_type != GLB_OAM_TYPE_MPLS_LSP)&&
        (p_oam_session->oam_type != GLB_OAM_TYPE_MPLS_PW))
    {
        G8131_RX_DEBUG("RX fail because oam type is not lsp oam nor pw oam");
        return PM_E_NONE;
    }
    
    /*If the aps_group_info is NULL, return*/
    p_db_ag_info = g8131_get_aps_group_by_oam(p_oam_session);
    if (NULL == p_db_ag_info)
    {
        G8131_RX_DEBUG("RX fail because can not get aps group info by oam session %u", p_oam_session->key.session_id);
        return PM_E_NONE;
    }

    /*If not working nor protection, return*/
    if (G8131_ACTIVE_NONE == p_db_ag_info->active_path)
    {
        return PM_E_NONE;
    }

    //in uni-directional case, APS pdu should not be received on head end 
    if (G8131_UNIDIRECTIONAL_SWITCHING == p_db_ag_info->direction)
    {
        G8131_RX_DEBUG("APS group %u rx fail because protection direction is uni-directional", p_db_ag_info->key.aps_group_id);
        return PM_E_NONE;
    }

    /*If oam service name equal aps group woring name, return*/
    if (0 == sal_strcmp(p_db_ag_info->working_name, p_oam_session->service_name))
    {
        G8131_RX_DEBUG("APS group %u rx fail because pdu received on working interface", p_db_ag_info->key.aps_group_id);
        p_db_ag_info->dfop_reason = G8131_APS_PDU_RCVD_ON_WORKING_INTERFACE;
        g8131_dfop(p_db_ag_info, p_db_ag_info->dfop_reason);
        return PM_E_NONE;
    }

    /*decode aps pdu*/
    sal_memset(&aps_pdu, 0, sizeof(aps_pdu));
    rc = g8131_pdu_aps_decode(pst_pdu->p_data, pst_pdu->left_len, &aps_pdu);
    if (PM_E_NONE != rc)
    {
        G8131_RX_DEBUG("APS group %u rx fail because decode fail", p_db_ag_info->key.aps_group_id);
        return PM_E_INVALID_PARAM;
    }

    /*map the request state to fsm event*/
    rc = g8131_pdu_map_request_state_to_fsm_event(aps_pdu.req_state, aps_pdu.req_signal, &fsm_event);
    if (PM_E_NONE != rc)
    {
        G8131_RX_DEBUG("APS group %u rx fail because invalid request state", p_db_ag_info->key.aps_group_id);
        p_db_ag_info->dfop_reason = G8131_INVALID_REQ_STATE_IN_PDU;
        g8131_dfop(p_db_ag_info, p_db_ag_info->dfop_reason);
        return PM_E_INVALID_PARAM;
    }

    if (G8131_A_BIT_MISMATCH_ERROR == p_db_ag_info->dfop_reason
        || G8131_B_BIT_MISMATCH_ERROR == p_db_ag_info->dfop_reason
        || G8131_D_BIT_MISMATCH_ERROR == p_db_ag_info->dfop_reason
        || G8131_R_BIT_MISMATCH_ERROR == p_db_ag_info->dfop_reason
        || G8131_T_BIT_MISMATCH_ERROR == p_db_ag_info->dfop_reason
        || G8131_INVALID_REQ_STATE_IN_PDU == p_db_ag_info->dfop_reason)
    {
        p_db_ag_info->dfop_reason = G8131_SUCCESS;
    }

    switch (validate_g8131_aps_pdu (p_db_ag_info, &aps_pdu))
    {
        case G8131_HAND_OVER_REQ_TO_EXER_DNR_LOGIC:
        {
            G8131_RX_DEBUG("APS group %u received EXER/DNR\n", p_db_ag_info->key.aps_group_id);
            g8131_fsm_execute(p_db_ag_info, fsm_event);
            break;
        }
        case G8131_HAND_OVER_REQ_TO_FAR_END_FSM:
        {
            G8131_RX_DEBUG("APS group %u: RX success! Request State: %s  FSM Event: %s\n", 
                            p_db_ag_info->key.aps_group_id, 
                            G8131_REQUEST_STATE_STR(aps_pdu.req_state),
                            G8131_FSM_EVENT_STR(fsm_event));
            g8131_fsm_execute(p_db_ag_info, fsm_event);
            break;
        }
        case G8131_A_BIT_MISMATCH_ERROR:
        {
            G8131_RX_DEBUG("APS group %u rx fail because A_BIT_MISMATCH\n", p_db_ag_info->key.aps_group_id);
            p_db_ag_info->dfop_reason = G8131_A_BIT_MISMATCH_ERROR;
            g8131_dfop (p_db_ag_info, p_db_ag_info->dfop_reason);
            break;
        }
        case G8131_B_BIT_MISMATCH_ERROR:
        {
            G8131_RX_DEBUG("APS group %u rx fail because B_BIT_MISMATCH\n", p_db_ag_info->key.aps_group_id);
            p_db_ag_info->dfop_reason = G8131_B_BIT_MISMATCH_ERROR;
            g8131_dfop(p_db_ag_info, p_db_ag_info->dfop_reason);
            break;
        }
        case G8131_D_BIT_MISMATCH_ERROR:
        {
            G8131_RX_DEBUG("APS group %u rx fail because D_BIT_MISMATCH\n", p_db_ag_info->key.aps_group_id);
            p_db_ag_info->dfop_reason = G8131_D_BIT_MISMATCH_ERROR;
            g8131_dfop(p_db_ag_info, p_db_ag_info->dfop_reason);
            break;
        }
        case G8131_R_BIT_MISMATCH_ERROR:
        {
            G8131_RX_DEBUG("APS group %u rx fail because R_BIT_MISMATCH\n", p_db_ag_info->key.aps_group_id);
            p_db_ag_info->dfop_reason = G8131_R_BIT_MISMATCH_ERROR;
            g8131_dfop (p_db_ag_info, p_db_ag_info->dfop_reason);
            break;
        }
        case G8131_T_BIT_MISMATCH_ERROR:
        {
            G8131_RX_DEBUG("APS group %u rx fail because T_BIT_MISMATCH\n", p_db_ag_info->key.aps_group_id);
            p_db_ag_info->dfop_reason = G8131_T_BIT_MISMATCH_ERROR;
            g8131_dfop (p_db_ag_info, p_db_ag_info->dfop_reason);
            g8131_fsm_execute(p_db_ag_info, fsm_event);
            break;
        }
        default:
            break;
    }
    return PM_E_NONE;
}

void
g8131_pdu_aps_tx_timer_handler(void *p_data)
{
    tbl_aps_group_info_t * p_db_ag_info = NULL;
    
    p_db_ag_info = (tbl_aps_group_info_t*)p_data;
    if (!p_db_ag_info)
    {
        return;
    }
    p_db_ag_info->aps_tx_timer = NULL;

    g8131_pdu_aps_tx(p_db_ag_info);

    /* start timer for new interval */
    p_db_ag_info->aps_tx_timer = 
            ctc_task_add_timer(TASK_PRI_NORMAL, g8131_pdu_aps_tx_timer_handler, p_db_ag_info, G8131_DEFAULT_APS_TX_INTERVAL);
    if (!p_db_ag_info->aps_tx_timer)
    {
        return;
    }
    return;
}

void
g8131_pdu_sf_hold_off_timer_handler(void *p_data)
{
    tbl_aps_group_info_t * p_db_ag_info = NULL;
    
    p_db_ag_info = (tbl_aps_group_info_t*)p_data;
    if (!p_db_ag_info)
    {
        return;
    }
    p_db_ag_info->sf_hold_off_timer = NULL;

    if (g8131_is_sf_or_sfp_reasserted_by_oam(p_db_ag_info->working_name, p_db_ag_info->aps_group_type))
    {
        g8131_fsm_execute(p_db_ag_info, G8131_FSM_L_EVENT_SF_W);
    }   

    GLB_UNSET_FLAG(p_db_ag_info->hold_off_expired, G8131_IN_SF_HOLD_OFF_STATE);
    return;
}

void 
g8131_pdu_sfp_hold_off_timer_handler (void *p_data)
{
    tbl_aps_group_info_t * p_db_ag_info = NULL;
    
    p_db_ag_info = (tbl_aps_group_info_t*)p_data;
    if (!p_db_ag_info)
    {
        return;
    }
    p_db_ag_info->sfp_hold_off_timer = NULL;

    if (g8131_is_sf_or_sfp_reasserted_by_oam(p_db_ag_info->protection_name, p_db_ag_info->aps_group_type))
    {
        g8131_fsm_execute(p_db_ag_info, G8131_FSM_L_EVENT_SF_P);
    }   

    GLB_UNSET_FLAG(p_db_ag_info->hold_off_expired, G8131_IN_SFP_HOLD_OFF_STATE);
    return;
}

void 
g8131_pdu_sd_hold_off_timer_handler (void *p_data)
{
    tbl_aps_group_info_t * p_db_ag_info = NULL;
    
    p_db_ag_info = (tbl_aps_group_info_t*)p_data;
    if (!p_db_ag_info)
    {
        return;
    }
    p_db_ag_info->sd_hold_off_timer = NULL;

    if (g8131_is_sd_or_sdp_reasserted_by_oam(p_db_ag_info->working_name, p_db_ag_info->aps_group_type))
    {
        g8131_fsm_execute(p_db_ag_info, G8131_FSM_L_EVENT_SD_W);
    }   

    GLB_UNSET_FLAG(p_db_ag_info->hold_off_expired, G8131_IN_SD_HOLD_OFF_STATE);
    return;
}


void 
g8131_pdu_sdp_hold_off_timer_handler (void *p_data)
{
    tbl_aps_group_info_t * p_db_ag_info = NULL;
    
    p_db_ag_info = (tbl_aps_group_info_t*)p_data;
    if (!p_db_ag_info)
    {
        return;
    }
    p_db_ag_info->sdp_hold_off_timer = NULL;

    if (g8131_is_sd_or_sdp_reasserted_by_oam(p_db_ag_info->protection_name, p_db_ag_info->aps_group_type))
    {
        g8131_fsm_execute(p_db_ag_info, G8131_FSM_L_EVENT_SD_P);
    }   

    GLB_UNSET_FLAG(p_db_ag_info->hold_off_expired, G8131_IN_SDP_HOLD_OFF_STATE);
    return;
}

void 
g8131_pdu_wtr_timer_handler (void *p_data)
{
    tbl_aps_group_info_t * p_db_ag_info = NULL;
    
    p_db_ag_info = (tbl_aps_group_info_t*)p_data;
    if (!p_db_ag_info)
    {
        return;
    }
    p_db_ag_info->wtr_timer = NULL;
    g8131_fsm_execute(p_db_ag_info, G8131_FSM_L_EVENT_WTR);
    
    return;
}

void 
g8131_pdu_out_state_change_timer_handler (void *p_data)
{
    tbl_aps_group_info_t * p_db_ag_info = NULL;
    
    p_db_ag_info = (tbl_aps_group_info_t*)p_data;
    if (!p_db_ag_info)
    {
        return;
    }
    p_db_ag_info->sc_out_aps_timer = NULL;

    if (PM_E_NONE != g8131_pdu_aps_tx(p_db_ag_info))
    {
        p_db_ag_info->sc_aps_out_pdu_count = 0;
        return;
    }
    p_db_ag_info->sc_aps_out_pdu_count++;

    if (p_db_ag_info->sc_aps_out_pdu_count < G8131_NUM_OF_APS_FRAMES_AFTER_STATE_CHANGE)
    {
        p_db_ag_info->sc_out_aps_timer = p_db_ag_info->sc_out_aps_timer ? p_db_ag_info->sc_out_aps_timer:
        ctc_task_add_timer(TASK_PRI_NORMAL, g8131_pdu_out_state_change_timer_handler, p_db_ag_info, 0);
    }
    else
    {
        p_db_ag_info->sc_aps_out_pdu_count = 0;
    }
    
    return;
}

void 
g8131_dfop(tbl_aps_group_info_t * p_db_ag_info, uint32 defect)
{
    G8131_EVENT_DEBUG("APS group %u: Protection Group entered into dFop state \n", p_db_ag_info->key.aps_group_id);
    switch (defect)
    {
    case G8131_A_BIT_MISMATCH_ERROR:
        p_db_ag_info->dfop_reason = G8131_A_BIT_MISMATCH_ERROR;
        break;
    case G8131_B_BIT_MISMATCH_ERROR:
        p_db_ag_info->dfop_reason = G8131_B_BIT_MISMATCH_ERROR;
        break;
    case G8131_D_BIT_MISMATCH_ERROR:
        p_db_ag_info->dfop_reason = G8131_D_BIT_MISMATCH_ERROR;
        break;
    case G8131_R_BIT_MISMATCH_ERROR:
        p_db_ag_info->dfop_reason = G8131_R_BIT_MISMATCH_ERROR;
        break;
    case G8131_T_BIT_MISMATCH_ERROR:
        p_db_ag_info->dfop_reason = G8131_T_BIT_MISMATCH_ERROR;
        break;
    case G8131_APS_PDU_RCVD_ON_WORKING_INTERFACE:
        p_db_ag_info->dfop_reason = G8131_APS_PDU_RCVD_ON_WORKING_INTERFACE;
        break;
    case G8131_INVALID_REQ_STATE_IN_PDU:
        p_db_ag_info->dfop_reason = G8131_INVALID_REQ_STATE_IN_PDU;
        break;
    default:
        break;
    }
    return;
}

