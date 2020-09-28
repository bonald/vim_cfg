#include "proto.h"

#include "gen/tbl_tpoam_exec_global_define.h"
#include "gen/tbl_tpoam_exec_global.h"
#include "gen/tbl_tpoam_global_define.h"
#include "gen/tbl_tpoam_global.h"
#include "gen/tbl_oam_session_define.h"
#include "gen/tbl_oam_session.h"
#include "gen/tbl_oam_session_upload_define.h"
#include "gen/tbl_oam_session_upload.h"
#include "gen/tbl_aps_group_info_define.h"
#include "gen/tbl_aps_group_info.h"
#include "gen/tbl_opm_debug_define.h"
#include "gen/tbl_opm_debug.h"

#include "gen/tbl_lsp_pe_define.h"
#include "gen/tbl_lsp_pe.h"
#include "gen/tbl_pw_tpe_define.h"
#include "gen/tbl_pw_tpe.h"
#include "gen/tbl_section_oam_define.h"
#include "gen/tbl_section_oam.h"
#include "gen/tbl_lsp_p_define.h"
#include "gen/tbl_lsp_p.h"
#include "tpoam_sock.h"
#include "tpoam_pdu.h"
#include "tpoam_api.h"
#include "tpoam_error.h"
#include "tpoam_debug.h"
#include "tpoam_dm.h"
#include "tpoam_lb.h"
#include "g8131_pdu.h"

static inline uint8 *
_tpoam_pdu_pull(tpoam_rx_pdu_t* pst_pdu, uint32 len)
{
    pst_pdu->left_len -= len;
    return pst_pdu->p_data += len;
}

static int32
_tpoam_pdu_check_pdu(tpoam_rx_pdu_t* pst_pdu)
{
    if (TPOAM_DEF_VERSION != pst_pdu->hdr.ver)
    {
        TPOAM_LOG_USER(E_INFORMATIONAL, TPOAM_6_RX_PDU_INVALID_VERSION, pst_pdu->hdr.ver, TPOAM_DEF_VERSION);
        return TPOAM_E_PDU_INVALID_VERSION;
    }

    switch (pst_pdu->hdr.opcode)
    {
    case TPOAM_Y1731_PDU_CCM:
        if (pst_pdu->hdr.tlv_offset != TPOAM_Y1731_CCM_FIRST_TLV_OFFSET)
        {
            TPOAM_LOG_USER(E_INFORMATIONAL, TPOAM_6_RX_PDU_INVALID_FIRST_TLV_OFFSET, 
                pst_pdu->hdr.tlv_offset, TPOAM_Y1731_CCM_FIRST_TLV_OFFSET);
            return TPOAM_E_PDU_INVALID_FIRST_TLV_OFFSET;
        }
        break;

    case TPOAM_Y1731_PDU_LBR:
    case TPOAM_Y1731_PDU_LBM:
        if (pst_pdu->hdr.tlv_offset != TPOAM_Y1731_LBM_LBR_FIRST_TLV_OFFSET)
        {
            TPOAM_LOG_USER(E_INFORMATIONAL, TPOAM_6_RX_PDU_INVALID_FIRST_TLV_OFFSET, 
                pst_pdu->hdr.tlv_offset, TPOAM_Y1731_LBM_LBR_FIRST_TLV_OFFSET);
            return TPOAM_E_PDU_INVALID_FIRST_TLV_OFFSET;
        }
        if (0 != pst_pdu->hdr.flags)
        {
            TPOAM_LOG_USER(E_INFORMATIONAL, TPOAM_6_RX_PDU_INVALID_FLAGS, pst_pdu->hdr.flags, 0);
            return TPOAM_E_PDU_INVALID_FLAGS;
        }
        break;
    case TPOAM_Y1731_PDU_APS:
        if (pst_pdu->hdr.tlv_offset != TPOAM_Y1731_APS_FIRST_TLV_OFFSET)
        {
            TPOAM_LOG_USER(E_INFORMATIONAL, TPOAM_6_RX_PDU_INVALID_FIRST_TLV_OFFSET, 
                pst_pdu->hdr.tlv_offset, TPOAM_Y1731_APS_FIRST_TLV_OFFSET);
            return TPOAM_E_PDU_INVALID_FIRST_TLV_OFFSET;
        }
        break;

    case TPOAM_Y1731_PDU_1DM:
        if (pst_pdu->hdr.tlv_offset != TPOAM_Y1731_1DM_FIRST_TLV_OFFSET)
        {
            TPOAM_LOG_USER(E_INFORMATIONAL, TPOAM_6_RX_PDU_INVALID_FIRST_TLV_OFFSET, 
                pst_pdu->hdr.tlv_offset, TPOAM_Y1731_1DM_FIRST_TLV_OFFSET);
            return TPOAM_E_PDU_INVALID_FIRST_TLV_OFFSET;
        }
        if (0 != pst_pdu->hdr.flags)
        {
            TPOAM_LOG_USER(E_INFORMATIONAL, TPOAM_6_RX_PDU_INVALID_FLAGS, pst_pdu->hdr.flags, 0);
            return TPOAM_E_PDU_INVALID_FLAGS;
        }
        break;

    case TPOAM_Y1731_PDU_DMM:
    case TPOAM_Y1731_PDU_DMR:
        if (pst_pdu->hdr.tlv_offset != TPOAM_Y1731_DMM_DMR_FIRST_TLV_OFFSET)
        {
            TPOAM_LOG_USER(E_INFORMATIONAL, TPOAM_6_RX_PDU_INVALID_FIRST_TLV_OFFSET, 
                pst_pdu->hdr.tlv_offset, TPOAM_Y1731_DMM_DMR_FIRST_TLV_OFFSET);
            return TPOAM_E_PDU_INVALID_FIRST_TLV_OFFSET;
        }
        if (0 != pst_pdu->hdr.flags)
        {
            TPOAM_LOG_USER(E_INFORMATIONAL, TPOAM_6_RX_PDU_INVALID_FLAGS, pst_pdu->hdr.flags, 0);
            return TPOAM_E_PDU_INVALID_FLAGS;
        }
        break;

    case TPOAM_Y1731_PDU_LMM:
    case TPOAM_Y1731_PDU_LMR:
        if (pst_pdu->hdr.tlv_offset != TPOAM_Y1731_LMM_LMR_FIRST_TLV_OFFSET)
        {
            TPOAM_LOG_USER(E_INFORMATIONAL, TPOAM_6_RX_PDU_INVALID_FIRST_TLV_OFFSET, 
                pst_pdu->hdr.tlv_offset, TPOAM_Y1731_LMM_LMR_FIRST_TLV_OFFSET);
            return TPOAM_E_PDU_INVALID_FIRST_TLV_OFFSET;
        }
        if (0 != pst_pdu->hdr.flags)
        {
            TPOAM_LOG_USER(E_INFORMATIONAL, TPOAM_6_RX_PDU_INVALID_FLAGS, pst_pdu->hdr.flags, 0);
            return TPOAM_E_PDU_INVALID_FLAGS;
        }
        break;

    case TPOAM_Y1731_PDU_CSF:
        break;
    case TPOAM_PW_MAC_WITHDRAW:
        break;

    default:
        return TPOAM_E_PDU_INVALID_OPCODE;
    }
    return TPOAM_E_NONE;
}


static int32 
_tpoam_pdu_decode_pdu_label(tpoam_rx_pdu_t* pst_pdu)
{
    uint32 service_type = 0;
    uint32 current_label = 0;
    uint32 next_label = 0;
    uint32 service_label = 0;
    uint32 label_num = 0;
    bool end_parser = FALSE;
    bool sbit = FALSE;
    
    current_label = sal_ntohl(*(uint32 *)pst_pdu->p_data);
    sbit = GLB_IS_BIT_SET(current_label, 8);
    _tpoam_pdu_pull(pst_pdu, TPOAM_MPLS_LABEL_BYTE_WIDTH);
    next_label = sal_ntohl(*(uint32 *)pst_pdu->p_data);
    label_num = 1;
    
    while (!end_parser)
    {
        if (GLB_MPLS_TP_ALERT_LABEL == TPOAM_GET_LABEL_ID(current_label))
        {
            /* section */
            end_parser = TRUE;
            service_label = current_label;
            service_type = GLB_OAM_TYPE_MPLS_SECTION;
        }
        else if ((!sbit) && (GLB_MPLS_TP_ALERT_LABEL == TPOAM_GET_LABEL_ID(next_label)))
        {
            /* lsp */
            end_parser = TRUE;
            service_label = current_label;
            service_type = GLB_OAM_TYPE_MPLS_LSP;
            _tpoam_pdu_pull(pst_pdu, TPOAM_MPLS_LABEL_BYTE_WIDTH);
        }
        else if (sbit && ((next_label >> 28) == 0x0001))
        {
            /* pw */
            end_parser = TRUE;
            service_label = current_label;
            /* modified by huwx for bug 20931, 2012.10.17 */
            if ((next_label & 0xFFFF) == GLB_MPLS_OAM_PW_MAC_WD_TYPE)
            {
                // not support
                //service_type = GLB_OAM_TYPE_PW_MAC_WD;
            }
            else
            {
                service_type = GLB_OAM_TYPE_MPLS_PW;
            }
        }
        else 
        {
            label_num++;
            pst_pdu->outer_label.label = TPOAM_GET_LABEL_ID(current_label);
            pst_pdu->outer_label.exp = TPOAM_GET_LABEL_EXP(current_label);
            pst_pdu->outer_label.s = TPOAM_GET_LABEL_S(current_label);
            pst_pdu->outer_label.ttl = TPOAM_GET_LABEL_TTL(current_label);
            current_label = next_label;
            end_parser = sbit || (label_num > 4);
            _tpoam_pdu_pull(pst_pdu, TPOAM_MPLS_LABEL_BYTE_WIDTH);
            next_label = sal_ntohl(*(uint32 *)pst_pdu->p_data);
            sbit = GLB_IS_BIT_SET(current_label, 8);
        }
    }

    pst_pdu->service_type = service_type;
    pst_pdu->label.label = TPOAM_GET_LABEL_ID(service_label);
    pst_pdu->label.exp = TPOAM_GET_LABEL_EXP(service_label);
    pst_pdu->label.s = TPOAM_GET_LABEL_S(service_label);
    pst_pdu->label.ttl = TPOAM_GET_LABEL_TTL(service_label);
    pst_pdu->label_num = label_num;

#if 0
    if (label_num > 1)
    {
        TPOAM_DBG_PACKET_RX("%s RX label 0x%08X (label %d, exp %d, s %d, ttl %d), outlabel (label %d, exp %d, s %d, ttl %d)", 
            tpoam_api_service_type_desc(service_type, TPOAM_PW_TYPE_MAX), 
            service_label, 
            pst_pdu->label.label, pst_pdu->label.exp, pst_pdu->label.s, pst_pdu->label.ttl, 
            pst_pdu->outer_label.label, pst_pdu->outer_label.exp, pst_pdu->outer_label.s, pst_pdu->outer_label.ttl);
    }
    else
    {
        TPOAM_DBG_PACKET_RX("%s RX label 0x%08X (label %d, exp %d, s %d, ttl %d)", 
            tpoam_api_service_type_desc(service_type, TPOAM_PW_TYPE_MAX), 
            service_label,
            pst_pdu->label.label, pst_pdu->label.exp, pst_pdu->label.s, pst_pdu->label.ttl);
    }
#endif

    return TPOAM_E_NONE;
}


/* decode ACH + Y.1731 common */
static int32 
_tpoam_pdu_decode_pdu_common(tpoam_rx_pdu_t* pst_pdu, uint32 oam_session_id, uint32 pw_type, uint32 lsp_type)
{
    tbl_tpoam_global_t *p_db_tpoam_glb = tbl_tpoam_global_get_tpoam_global();
    
    /* decode ACH */
    /*
       0                   1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |0 0 0 1|0 0 0 0|0 0 0 0 0 0 0 0|  Y.1731 Channel Type (0xXXXX) |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      | MEL | Version |    OpCode     |     Flags     |   TLV Offset  |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     */
    int32 ret = TPOAM_E_NONE;
    uint32 ach = 0;
    uint8 mel_and_ver = 0;
    uint8** pnt = &pst_pdu->p_data;
    int32* size = &pst_pdu->left_len;

    TLV_DECODE_GETL(ach);
    pst_pdu->channel_type = (ach & 0xFFFF);
    if (pst_pdu->channel_type != p_db_tpoam_glb->channel_type)
    {
        if (GLB_MPLS_OAM_ACH_CHAN_TYPE_MCC == pst_pdu->channel_type)
        {

        }
        else if (GLB_MPLS_OAM_ACH_CHAN_TYPE_SCC == pst_pdu->channel_type)
        {

        }
        else
        {

        }
        TPOAM_LOG_USER(E_INFORMATIONAL, TPOAM_6_RX_PDU_UNSUPPORT_CHANNEL_TYPE, pst_pdu->channel_type);
        return TPOAM_E_PDU_UNSUPPORT_CHANNEL_TYPE;
    }

    pst_pdu->p_y1731_head = pst_pdu->p_data;
    pst_pdu->y1731_len = pst_pdu->left_len;
    
    TLV_DECODE_GETC(mel_and_ver);
    pst_pdu->hdr.mel = (mel_and_ver >> 5) & 0x7;
    pst_pdu->hdr.ver = (mel_and_ver) & 0x1F;
    TLV_DECODE_GETC(pst_pdu->hdr.opcode);
    TLV_DECODE_GETC(pst_pdu->hdr.flags);
    TLV_DECODE_GETC(pst_pdu->hdr.tlv_offset);

    if (2 == pst_pdu->label_num || 3 == pst_pdu->label_num)
    {
        TPOAM_DBG_PACKET_RX("Oam session %u %s RX %s %u labels (label %d, exp %d, s %d, ttl %d), outlabel (label %d, exp %d, s %d, ttl %d)",
            oam_session_id,
            tpoam_api_service_type_for_display_desc(pst_pdu->service_type, pw_type, lsp_type), 
            tpoam_api_pkt_type_desc(pst_pdu->hdr.opcode),
            pst_pdu->label_num,
            pst_pdu->label.label, pst_pdu->label.exp, pst_pdu->label.s, pst_pdu->label.ttl, 
            pst_pdu->outer_label.label, pst_pdu->outer_label.exp, pst_pdu->outer_label.s, pst_pdu->outer_label.ttl);
    }
    else if (1 == pst_pdu->label_num)
    {
        TPOAM_DBG_PACKET_RX("Oam session %u %s RX %s %u label (label %d, exp %d, s %d, ttl %d)",
            oam_session_id,
            tpoam_api_service_type_for_display_desc(pst_pdu->service_type, pw_type, lsp_type), 
            tpoam_api_pkt_type_desc(pst_pdu->hdr.opcode),
            pst_pdu->label_num,
            pst_pdu->label.label, pst_pdu->label.exp, pst_pdu->label.s, pst_pdu->label.ttl);
    }
            
    tpoam_api_stats_add_rx(&p_db_tpoam_glb->oam_glb_stats.rx, pst_pdu->hdr.opcode);
    ret = _tpoam_pdu_check_pdu(pst_pdu);
    if (ret < 0)
    {
        tpoam_api_stats_add_err(&p_db_tpoam_glb->oam_glb_stats.err, pst_pdu->hdr.opcode);
        return ret;
    }
    
    return TPOAM_E_NONE;
}

static int32 
_tpoam_pdu_handle_pdu(tpoam_rx_pdu_t* pst_pdu, uint32 oam_session_id)
{
    tbl_tpoam_global_t *p_db_tpoam_glb = tbl_tpoam_global_get_tpoam_global();

    int32 ret = TPOAM_E_NONE;
    tbl_oam_session_t* pst_oam = NULL;

    pst_oam = tbl_oam_session_get_oam_session_by_session_id(oam_session_id);
    if (!pst_oam)
    {
        tpoam_api_stats_add_err(&p_db_tpoam_glb->oam_glb_stats.err, pst_pdu->hdr.opcode);
        return TPOAM_E_SERVICE_NON_OAM;
    }

    if (pst_oam->level != pst_pdu->hdr.mel)
    {
        TPOAM_LOG_USER(E_INFORMATIONAL, TPOAM_6_RX_PDU_INVALID_MEL, pst_pdu->hdr.mel, pst_oam->level);
        return TPOAM_E_PDU_INVALID_MEL;
    }
    
    switch (pst_pdu->hdr.opcode)
    {
    case TPOAM_Y1731_PDU_LBM:      
        ret = tpoam_lb_rx_lbm(pst_oam, pst_pdu);
        break;

    case TPOAM_Y1731_PDU_LBR:
        ret = tpoam_lb_rx_lbr(pst_oam, pst_pdu);
        break;

    case TPOAM_Y1731_PDU_APS:
        ret =  g8131_pdu_aps_rx(pst_oam, pst_pdu);
        break;
        
    case TPOAM_Y1731_PDU_1DM:
        //ret = tpoam_dm_rx_1dm(pst_oam, pst_pdu);
        break;

    case TPOAM_Y1731_PDU_DMR:
        ret = tpoam_dm_rx_dmr(pst_oam, pst_pdu);
        break;

    case TPOAM_Y1731_PDU_DMM:
        ret = tpoam_dm_rx_dmm(pst_oam, pst_pdu);
        break;

    case TPOAM_Y1731_PDU_LMR:
        //ret = tpoam_lm_rx_lmr(pst_oam, pst_pdu);
        break;

    case TPOAM_Y1731_PDU_LMM:
        //ret = tpoam_lm_rx_lmm(pst_oam, pst_pdu);
        break;

    case TPOAM_Y1731_PDU_CSF:
        // TODO:
        //ret = nsm_tpoam_csf_client_rx_csf(pst_oam, pst_pdu);
        break;

    case TPOAM_Y1731_PDU_CCM:
        ret = TPOAM_E_PDU_INVALID_OPCODE;

    default:
        ret = TPOAM_E_PDU_INVALID_OPCODE;
    }

    tpoam_api_stats_add_rx(&pst_oam->oam_stats.rx, pst_pdu->hdr.opcode);
    if (ret < 0)
    {
        tpoam_api_stats_add_err(&p_db_tpoam_glb->oam_glb_stats.err, pst_pdu->hdr.opcode);
        tpoam_api_stats_add_err(&pst_oam->oam_stats.err, pst_pdu->hdr.opcode);
        return ret;
    }
    
    return TPOAM_E_NONE;
}

int32 
tpoam_pdu_rx_pdu (tpoam_rx_pdu_t* pst_pdu, uint32 had_paser_label, uint32 oam_session_id, uint32 pw_type, uint32 lsp_type)
{
    if (!had_paser_label)
    {
        TPOAM_OP_ERR_RET(_tpoam_pdu_decode_pdu_label(pst_pdu));
    }

    TPOAM_OP_ERR_RET(_tpoam_pdu_decode_pdu_common(pst_pdu, oam_session_id, pw_type, lsp_type));
    return _tpoam_pdu_handle_pdu(pst_pdu, oam_session_id);
}
