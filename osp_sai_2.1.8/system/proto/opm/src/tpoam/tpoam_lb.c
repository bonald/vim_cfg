#include "proto.h"

#include "ctclib_crc.h"

#include "gen/tbl_tpoam_exec_global_define.h"
#include "gen/tbl_tpoam_exec_global.h"
#include "gen/tbl_tpoam_global_define.h"
#include "gen/tbl_tpoam_global.h"
#include "gen/tbl_oam_session_define.h"
#include "gen/tbl_oam_session.h"
#include "gen/tbl_oam_session_upload_define.h"
#include "gen/tbl_oam_session_upload.h"

#include "gen/tbl_lsp_pe_define.h"
#include "gen/tbl_lsp_pe.h"
#include "gen/tbl_pw_tpe_define.h"
#include "gen/tbl_pw_tpe.h"
#include "gen/tbl_section_oam_define.h"
#include "gen/tbl_section_oam.h"
#include "gen/tbl_lsp_p_define.h"
#include "gen/tbl_lsp_p.h"

#include "tpoam_list.h"

#include "tpoam_sock.h"
#include "tpoam_pdu.h"
#include "tpoam_api.h"
#include "tpoam_error.h"
#include "tpoam_debug.h"
#include "tpoam_dm.h"
#include "tpoam_event.h"
#include "tpoam_lb.h"
#include "tpoam_cmd.h"

#define TPOAM_Y1731_PDU_LBM_START_TRANS_ID      0x00

#define IPV4_ADDR_STRLEN	16

#define TPOAM_LB_PKT_BUF_SIZE       9512

#define TPOAM_Y1731_PDU_IF_NUM_OF_PER_NODE_MIP  0
#define TPOAM_Y1731_PDU_MEP_ID_LEN              2
#define TPOAM_Y1731_PDU_MIP_ID_LEN              14      /* ICC + NODE_ID + IF_NUM = 6 + 4 + 4 */
#define TPOAM_Y1731_PDU_MEG_ID_48_LEN           48
#define TPOAM_Y1731_PDU_MEP_MIP_ID_LEN          24
#define TPOAM_Y1731_PDU_TARGET_MEP_MIP_ID_LEN   25
#define TPOAM_Y1731_PDU_REPLY_MEP_MIP_ID_LEN    25
#define TPOAM_Y1731_PDU_REQUEST_MEP_ID_LEN      53
#define TPOAM_Y1731_PDU_TEST_TLV_PATTERN_LEN    1
#define TPOAM_Y1731_PDU_TEST_TLV_CRC_LEN        4
#define TPOAM_Y1731_PDU_TEST_TLV_TYPE_LEN_LEN   3
#define TPOAM_Y1731_PDU_REQUEST_MEP_ID_RSVD_LEN 2
#define TPOAM_Y1731_PDU_REQUEST_MEP_ID_RESERVED 0


#define TPOAM_PDU_TLV_LEN_LEFT_CHECK(_LEN_)\
do {\
    if ((_LEN_) > (*size))\
    {\
        return TPOAM_E_LB_TLV_LEN_MORE_THAN_LEFT;\
    }\
}while(0)


int32
tpoam_lb_call_test_cb(tbl_oam_session_t* pst_oam, uint8 lb_sub_type)
{
    tbl_tpoam_exec_global_t *p_tpoam_exec_global = tbl_tpoam_exec_global_get_tpoam_exec_global();
    //TPOAM_DBG_LB_LBR("Oam session %u MEG %s test %s LBR, test type %u, reply CB", pst_oam->key.session_id, pst_oam->megid_str, 
    //    _tpoam_lb_sub_type_desc(lb_sub_type),
    //    p_tpoam_exec_global->test_type);
    
    if (TPOAM_EXEC_TYPE_LB_MEP == p_tpoam_exec_global->test_type)
    {
        _tpoam_cli_lb_mep_reply_cb((void *)pst_oam);
    }
    else if (TPOAM_EXEC_TYPE_LB_MIP == p_tpoam_exec_global->test_type)
    {
        _tpoam_cli_lb_mip_reply_cb((void *)pst_oam);
    }
    else if (TPOAM_EXEC_TYPE_LB_DISCOVERY == p_tpoam_exec_global->test_type)
    {
        _tpoam_cli_lb_discovery_reply_cb((void *)pst_oam);
    }
    else
    {
        TPOAM_DBG_LB_LBR("Oam session %u MEG %s test %s LBR, invalid test type %u", pst_oam->key.session_id, pst_oam->megid_str, 
        _tpoam_lb_sub_type_desc(lb_sub_type),
        p_tpoam_exec_global->test_type);
    }

    return TPOAM_E_NONE;
}


static int32
_tpoam_lb_zero_check(tbl_oam_session_t* pst_oam, uint32 is_lbm, uint8* pch_check_str, uint32 check_len)
{
    uint8 zero_str[TPOAM_Y1731_PDU_MEP_MIP_ID_LEN];
    
    sal_memset(&zero_str, 0, sizeof(zero_str));

    if (check_len > TPOAM_Y1731_PDU_MEP_MIP_ID_LEN)
    {
        TPOAM_ASSERT;
    }

    if (0 != sal_memcmp(zero_str, pch_check_str, check_len))
    {
        if (is_lbm)
        {
            TPOAM_LOG_USER(E_INFORMATIONAL, TPOAM_6_RX_LBM_ZERO_CHECK, pst_oam->megid_str);
        }
        else
        {
            TPOAM_LOG_USER(E_INFORMATIONAL, TPOAM_6_RX_LBR_ZERO_CHECK, pst_oam->megid_str);
        }
        return TPOAM_E_LB_ZERO_CHECK;
    }
    
    return TPOAM_E_NONE;
}


static int32
_tpoam_lb_deinit_lb_session(tbl_oam_session_t* pst_oam)
{
    tpoam_lb_t* pst_lb = &pst_oam->oam_lb;
    
    if (!pst_lb->active)
    {
        return TPOAM_E_NONE;
    }

    //if (l2_is_timer_running (pst_lb->p_t_lb_timer))
    {
        tpoam_util_cancel_timer(&pst_lb->p_t_lb_timer);
    }
    //if (pst_lb->p_lbm_data)
    {
        //XFREE(MEM_PM_TPOAM_LB_PKT_BUF, pst_lb->p_lbm_data);
        //pst_lb->p_lbm_data = NULL;
    }

    tpoam_free_skb(&(pst_lb->tx_skb));

    sal_gettimeofday(&pst_lb->lbm_stop);
    pst_lb->active = FALSE;
    
    return TPOAM_E_NONE;
}

static int32
_tpoam_lb_init_lb_session(tbl_oam_session_t* pst_oam, tpoam_lbm_para_t* pst_lbm_para)
{
    tpoam_lb_t* pst_lb = &pst_oam->oam_lb;
    tpoam_lb_reply_t* pst_entry = NULL;
    struct listnode* pst_next_node = NULL;
    struct listnode* pst_node = NULL;
    
    pst_lb->per_lb_status = TPOAM_PER_LB_STATUS_WAITING;
    pst_lb->lbm_tx_count = 0;
    pst_lb->lbr_rx_count = 0;
    pst_lb->lbr_out_order_count = 0;
    sal_memset(&pst_lb->discovery, 0, sizeof(pst_lb->discovery));
    sal_memset(&pst_lb->lbm_start, 0, sizeof(pst_lb->lbm_start));
    sal_memset(&pst_lb->lbm_stop, 0, sizeof(pst_lb->lbm_stop));
    sal_memcpy(&pst_lb->lbm_para, pst_lbm_para, sizeof(tpoam_lbm_para_t));
    pst_lb->discovery.curr_ttl = pst_lbm_para->ttl;

    LIST_LOOP_DEL(pst_lb->pst_lb_cache_list, pst_entry, pst_node, pst_next_node)
    {
        listnode_delete(pst_lb->pst_lb_cache_list, pst_entry);
        XFREE(MEM_PM_TPOAM_LB_REPLY_ENTRY, pst_entry);
    }

    if (pst_lb->p_lbm_data)
    {
        //TPOAM_ASSERT;
    }
    //pst_lb->p_lbm_data = XCALLOC(MEM_PM_TPOAM_LB_PKT_BUF, TPOAM_LB_PKT_BUF_SIZE);
    //if (!pst_lb->p_lbm_data)
    {
        //return TPOAM_E_MEMORY_ALLOC_FAIL;
    }

    sal_gettimeofday(&pst_lb->lbm_start);
    pst_lb->active = TRUE;
    return TPOAM_E_NONE;
}

static int32
_tpoam_lb_encode_lbm(tbl_oam_session_t* pst_oam, int32* p_length)
{
    tpoam_lb_t* pst_lb = &pst_oam->oam_lb;
    uint8*  p_curr_data = pst_lb->p_lbm_data; 
    uint8   megid_48[TPOAM_Y1731_PDU_MEG_ID_48_LEN];
    //int32   pkt_size_left = TPOAM_LB_PKT_BUF_SIZE;
    int32*  size = NULL;
    uint8** pnt = NULL;
    uint8   level_version = 0;    
    uint8   icc_str[GLB_TPOAM_ICC_LEN+1];
    uint8*  p_crc_start = NULL;
    uint32  crc = 0;
    uint32  rand_data = 0;
    int32 tlv_data_length = 0;
    
    //size = &pkt_size_left;
    size = p_length;
    pnt = &p_curr_data;

    /*
       0                   1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |0 0 0 1|0 0 0 0|0 0 0 0 0 0 0 0|  Y.1731 Channel Type (0xXXXX) |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      | MEL | Version |    OpCode     |     Flags     |   TLV Offset  |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |               Transaction ID/Sequence Number                  |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                    Target MEP/MIP ID TLV                      |
      |                              ...                              |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |               [optional Requesting MEP ID TLV]                |
      |                              ...                              |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      | [other optional TLV starts here]                              |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |     End TLV     |
      +-+-+-+-+-+-+-+-+-+
                        Figure 2 LBM Packet Format
    */
    
    level_version = (((pst_oam->level & TPOAM_Y1731_PDU_MEL_MASK) << TPOAM_Y1731_PDU_MEL_SHIFT)
                    | (TPOAM_Y1731_PDU_VER & TPOAM_Y1731_PDU_VER_MASK));
    
    TLV_ENCODE_PUTC(level_version);
    TLV_ENCODE_PUTC(TPOAM_Y1731_PDU_LBM);
    TLV_ENCODE_PUTC(TPOAM_Y1731_PDU_FLAG_NONE);
    TLV_ENCODE_PUTC(TPOAM_Y1731_LBM_LBR_FIRST_TLV_OFFSET);
    TLV_ENCODE_PUTL(pst_lb->lbm_next_trans_id);

    /*
       0                   1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |  Type (0x21)  |          Length (25)          |   Sub-Type    |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                              ...                              |
      |      MEP/MIP Identifier (format is ID Sub-Type specific)      |
      |                              ...                              |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

                   Figure 4 Target MEP/MIP ID TLV format
    */
    
    TLV_ENCODE_PUTC(TPOAM_Y1731_PDU_TARGET_MEP_MIP_ID_TLV);
    TLV_ENCODE_PUTW(TPOAM_Y1731_PDU_TARGET_MEP_MIP_ID_LEN);
    TLV_ENCODE_PUTC(pst_lb->lbm_para.lb_sub_type);

    switch (pst_lb->lbm_para.lb_sub_type)
    {
    case TPOAM_Y1731_PDU_LB_SUB_TYPE_DISCOVERY_INGRESS_NODE:
        TLV_ENCODE_PUT_EMPTY(TPOAM_Y1731_PDU_MEP_MIP_ID_LEN);
        break;
    case TPOAM_Y1731_PDU_LB_SUB_TYPE_ICC_BASED_MEP_ID:
/*
       0                   1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |     Type      |          Length (25)          |Sub-Type (0x02)|
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |             MEP ID            |                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                               |
      |                              ...                              |
      |                          MUST be ZERO                         |
      |                              ...                              |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

    Figure 8 Target or Replying MEP/MIP ID TLV format (ICC-based MEP ID)
 */
        TLV_ENCODE_PUTW(pst_lb->lbm_para.rmep_id);
        TLV_ENCODE_PUT_EMPTY(TPOAM_Y1731_PDU_MEP_MIP_ID_LEN - TPOAM_Y1731_PDU_MEP_ID_LEN);
        break;
    case TPOAM_Y1731_PDU_LB_SUB_TYPE_ICC_BASED_MIP_ID:
/*
       0                   1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |     Type      |          Length (25)          |Sub-Type (0x03)|
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                    ITU-T Carrier Code (ICC)                   |
      |                               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                               |            Node-ID            |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |            Node-ID            |             IF-Num            |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |             IF-Num            |                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                               |
      |                              ...                              |
      |                          MUST be ZERO                         |
      |                              ...                              |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

    Figure 9 Target or Replying MEP/MIP ID TLV format (ICC-based MIP ID)
 */
        sal_memset(icc_str, 0, sizeof(icc_str));
        sal_memcpy(icc_str, pst_oam->megid_str, pst_oam->icclen);
        TLV_ENCODE_PUT(icc_str, GLB_TPOAM_ICC_LEN);
        TLV_ENCODE_PUTL(sal_hton32(pst_lb->lbm_para.node_id));
        TLV_ENCODE_PUTL(TPOAM_Y1731_PDU_IF_NUM_OF_PER_NODE_MIP);
        TLV_ENCODE_PUT_EMPTY(TPOAM_Y1731_PDU_MEP_MIP_ID_LEN - TPOAM_Y1731_PDU_MIP_ID_LEN);
        break;
    default:
        return TPOAM_E_LB_UNSUPPORT_LB_TYPE;
    }
    
    /* requesting optional TLV should be follow Target TLV */
    if (pst_lb->lbm_para.is_req_mep_id)
    {
/*
       0                   1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |  Type (0x23)  |          Length (53)          | Loopback Ind. |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |             MEP ID            |                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                               |
      |                                                               |
      |                             MEG ID                            |
      |                                                               |
      |                               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                               |       Reserved (0x0000)       |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

                   Figure 10Requesting MEP ID TLV format
 */

        TLV_ENCODE_PUTC(TPOAM_Y1731_PDU_REQUEST_MEP_ID_TLV);
        TLV_ENCODE_PUTW(TPOAM_Y1731_PDU_REQUEST_MEP_ID_LEN);
        TLV_ENCODE_PUTC(TPOAM_PDU_LB_INDICATION_LBM);
        TLV_ENCODE_PUTW(pst_oam->mepid);
        sal_memset(megid_48, 0, sizeof(megid_48));
        tpoam_api_build_megid(pst_oam->megid_str, megid_48);
        TLV_ENCODE_PUT(megid_48, TPOAM_Y1731_PDU_MEG_ID_48_LEN);
        TLV_ENCODE_PUTW(TPOAM_Y1731_PDU_REQUEST_MEP_ID_RESERVED);
    }

    /* ITU-T Y.1731 7.2.1.1 */
    if (TPOAM_Y1731_PDU_DATA_TLV == pst_lb->lbm_para.opt_tlv_type)
    {
        /* Data TLV format - 9.3.2 pg 35 of ITU-T Y.1731 */
        
        /* Type */
        TLV_ENCODE_PUTC(TPOAM_Y1731_PDU_DATA_TLV);

        /* Length */
        TLV_ENCODE_PUTW(pst_lb->lbm_para.opt_tlv_len);

        /* Data : only support 0 */
        TLV_ENCODE_PUT_EMPTY(pst_lb->lbm_para.opt_tlv_len);
    }
    else if (TPOAM_Y1731_PDU_TEST_TLV == pst_lb->lbm_para.opt_tlv_type)
    {
        /* Test TLV format - 9.3.2 pg 36 of ITU-T Y.1731 */
        p_crc_start = (*pnt);

        /* Type */
        TLV_ENCODE_PUTC(TPOAM_Y1731_PDU_TEST_TLV);

        /* Length */
        TLV_ENCODE_PUTW(pst_lb->lbm_para.opt_tlv_len);

        /* Pattern type */
        TLV_ENCODE_PUTC(pst_lb->lbm_para.test_tlv_mode);

        /* Test */
        switch (pst_lb->lbm_para.test_tlv_mode)
        {
        case TPOAM_TEST_MODE_0_NULL_WO_CRC:
            tlv_data_length = pst_lb->lbm_para.opt_tlv_len - TPOAM_Y1731_PDU_TEST_TLV_PATTERN_LEN;
            if (tlv_data_length <= 0)
            {
                return TPOAM_E_LB_TEST_TLV_LENGTH_CHECK;
            }
            TLV_ENCODE_PUT_EMPTY(tlv_data_length);
            break;
        case TPOAM_TEST_MODE_1_NULL_WI_CRC:
            tlv_data_length = pst_lb->lbm_para.opt_tlv_len
                - (TPOAM_Y1731_PDU_TEST_TLV_PATTERN_LEN+TPOAM_Y1731_PDU_TEST_TLV_CRC_LEN);
            if (tlv_data_length <= 0)
            {
                return TPOAM_E_LB_TEST_TLV_LENGTH_CHECK;
            }
            TLV_ENCODE_PUT_EMPTY(tlv_data_length);
            crc = ctclib_gen_crc32(0, (char*)p_crc_start, 
                pst_lb->lbm_para.opt_tlv_len + TPOAM_Y1731_PDU_TEST_TLV_TYPE_LEN_LEN - TPOAM_Y1731_PDU_TEST_TLV_CRC_LEN);
            TLV_ENCODE_PUTL(sal_hton32(crc));
            break;
        case TPOAM_TEST_MODE_2_PRBS_WO_CRC:
            tlv_data_length = pst_lb->lbm_para.opt_tlv_len - TPOAM_Y1731_PDU_TEST_TLV_PATTERN_LEN;
            if (tlv_data_length <= 0)
            {
                return TPOAM_E_LB_TEST_TLV_LENGTH_CHECK;
            }
            while (tlv_data_length >= 4)
            {
                rand_data = sal_rand();
                TLV_ENCODE_PUTL(rand_data);
                tlv_data_length -= 4;
            };
            if (tlv_data_length > 0)
            {
                rand_data = sal_rand();
                TLV_ENCODE_PUT(&rand_data, tlv_data_length);
            }
            break;
        case TPOAM_TEST_MODE_3_PRBS_WI_CRC:
            tlv_data_length = pst_lb->lbm_para.opt_tlv_len
                - (TPOAM_Y1731_PDU_TEST_TLV_PATTERN_LEN+TPOAM_Y1731_PDU_TEST_TLV_CRC_LEN);
            if (tlv_data_length <= 0)
            {
                return TPOAM_E_LB_TEST_TLV_LENGTH_CHECK;
            }
            while (tlv_data_length >= 4)
            {
                rand_data = sal_rand();
                TLV_ENCODE_PUTL(rand_data);
                tlv_data_length -= 4;
            };
            if (tlv_data_length > 0)
            {
                rand_data = sal_rand();
                TLV_ENCODE_PUT(&rand_data, tlv_data_length);
            }
            crc = ctclib_gen_crc32(0, (char*)p_crc_start, 
                pst_lb->lbm_para.opt_tlv_len + TPOAM_Y1731_PDU_TEST_TLV_TYPE_LEN_LEN - TPOAM_Y1731_PDU_TEST_TLV_CRC_LEN);
            TLV_ENCODE_PUTL(sal_hton32(crc));
            break;
        default:
            TPOAM_ASSERT;
        }
    }

    TLV_ENCODE_PUTC (TPOAM_Y1731_PDU_END_TLV);

    if (*p_length < 0)
    //if (pkt_size_left < 0)
    {
        TPOAM_ASSERT;
    }
    
    //pst_lb->payload_len = TPOAM_LB_PKT_BUF_SIZE - pkt_size_left;
    pst_lb->payload_len = TPOAM_LB_PKT_BUF_SIZE - *p_length;

    return TPOAM_E_NONE;
}

static int32
_tpoam_lb_encode_lbr(tbl_oam_session_t* pst_oam, tpoam_rx_pdu_t* pst_lbm, uint8* p_data, uint32 *len)
{
    int32   pkt_size_left = *len;
    int32*  size = NULL;
    uint8** pnt = NULL;
    uint8   icc_str[GLB_TPOAM_ICC_LEN + 1];
    tbl_tpoam_global_t* pst_master = tbl_tpoam_global_get_tpoam_global();
    
    size = &pkt_size_left;
    pnt = &p_data;

    /* encode from Y.1731 header (after ACH header) */
    sal_memcpy(p_data, pst_lbm->p_y1731_head, pst_lbm->y1731_len);
    p_data[1] = TPOAM_Y1731_PDU_LBR;    /* change opcode to LBR */

    size = &pkt_size_left;
    p_data += 8;       /* Y.1731 hdr(4) + Trans ID(4)*/
    TLV_ENCODE_PUTC(TPOAM_Y1731_PDU_REPLY_MEP_MIP_ID_TLV);
    TLV_ENCODE_PUTW(TPOAM_Y1731_PDU_REPLY_MEP_MIP_ID_LEN);
    switch (pst_lbm->lb_sub_type)
    {
    case TPOAM_Y1731_PDU_LB_SUB_TYPE_DISCOVERY_INGRESS_NODE:
        if (TPOAM_MP_TYPE_MEP == pst_oam->mp_type)
        {
            /* reply MEP */
            if (!pst_oam->mepid)
            {
                TPOAM_LOG_USER(E_INFORMATIONAL, TPOAM_6_RX_LBM_LMEP_NOT_EXIST, pst_oam->megid_str);
                return TPOAM_E_LMEP_NOT_EXIST;
            }
            TLV_ENCODE_PUTC(TPOAM_Y1731_PDU_LB_SUB_TYPE_ICC_BASED_MEP_ID);
            TLV_ENCODE_PUTW(pst_oam->mepid);
            TLV_ENCODE_PUT_EMPTY(TPOAM_Y1731_PDU_MEP_MIP_ID_LEN - TPOAM_Y1731_PDU_MEP_ID_LEN);
        }
        else
        {
            /* reply MIP */
            //if (!pst_oam->mip_en)
            {
                //TPOAM_LOG_USER(E_INFORMATIONAL, TPOAM_6_RX_LBM_MIP_NOT_EXIST, pst_oam->megid_str);                
                //return TPOAM_E_MIP_NOT_EXIST;
            }
            TLV_ENCODE_PUTC(TPOAM_Y1731_PDU_LB_SUB_TYPE_ICC_BASED_MIP_ID);
            sal_memset(icc_str, 0, sizeof(icc_str));
            sal_memcpy(icc_str, pst_oam->megid_str, pst_oam->icclen);
            TLV_ENCODE_PUT(icc_str, GLB_TPOAM_ICC_LEN);
            TLV_ENCODE_PUTL(sal_hton32(pst_master->node_id));
            TLV_ENCODE_PUTL(TPOAM_Y1731_PDU_IF_NUM_OF_PER_NODE_MIP);
            TLV_ENCODE_PUT_EMPTY(TPOAM_Y1731_PDU_MEP_MIP_ID_LEN - TPOAM_Y1731_PDU_MIP_ID_LEN);
        }
        break;
    case TPOAM_Y1731_PDU_LB_SUB_TYPE_ICC_BASED_MEP_ID:
        break;
    case TPOAM_Y1731_PDU_LB_SUB_TYPE_ICC_BASED_MIP_ID:
        break;
    default:
        return TPOAM_E_LB_UNSUPPORT_LB_TYPE;
    }

    *len = pkt_size_left;
    return TPOAM_E_NONE;
}

static int32
_tpoam_lb_decode_lbm(tbl_oam_session_t* pst_oam, tpoam_rx_pdu_t* pst_pdu)
{

    tbl_tpoam_global_t *p_db_tpoam_glb = tbl_tpoam_global_get_tpoam_global();
    
    uint8** pnt = &pst_pdu->p_data;
    int32* size = &pst_pdu->left_len;
    uint8 tlv_type = 0;
    uint16 lb_mep_mip_len = 0;
    uint16 mep_id = 0;
    uint16 tlv_length = 0;
    int32 tlv_data_length = 0;
    uint8 test_tlv_mode = 0;
    uint8 icc_str[GLB_TPOAM_ICC_LEN+1];
    uint8 local_icc_str[GLB_TPOAM_ICC_LEN+1];
    uint8 meg_id_str[TPOAM_Y1731_PDU_MEG_ID_48_LEN];
    uint8 local_meg_id_str[TPOAM_Y1731_PDU_MEG_ID_48_LEN];
    uint8 mep_mip_str[TPOAM_Y1731_PDU_MEP_MIP_ID_LEN];
    uint8 test_tlv[TPOAM_LB_PKT_BUF_SIZE];
    uint32 node_id = 0;
    uint32 ifnum = 0;
    uint32 check_len = 0;
    uint32 crc_in_pkt = 0;
    uint32 crc = 0;
    uint32 i = 0;
//    uint8 loopback_ind = 0;
    uint8* pch_loopback_ind = NULL;
    uint8* p_crc_start = NULL; 
    addr_ipv4_t node_id_addr;
    addr_ipv4_t local_node_id_addr;
    char node_id_str[IPV4_ADDR_STRLEN+1];
    char local_node_id_str[IPV4_ADDR_STRLEN+1];
    
    /*
       0                   1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |0 0 0 1|0 0 0 0|0 0 0 0 0 0 0 0|  Y.1731 Channel Type (0xXXXX) |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      | MEL | Version |    OpCode     |     Flags     |   TLV Offset  |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |               Transaction ID/Sequence Number                  |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                    Target MEP/MIP ID TLV                      |
      |                              ...                              |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |               [optional Requesting MEP ID TLV]                |
      |                              ...                              |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      | [other optional TLV starts here]                              |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |     End TLV     |
      +-+-+-+-+-+-+-+-+-+
                        Figure 2 LBM Packet Format
    */
    TLV_DECODE_GETL(pst_pdu->lb_trans_id);

    /*
       0                   1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |  Type (0x21)  |          Length (25)          |   Sub-Type    |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                              ...                              |
      |      MEP/MIP Identifier (format is ID Sub-Type specific)      |
      |                              ...                              |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

                   Figure 4 Target MEP/MIP ID TLV format
    */

    TLV_DECODE_GETC(tlv_type);
    if (TPOAM_Y1731_PDU_TARGET_MEP_MIP_ID_TLV != tlv_type)
    {
        TPOAM_LOG_USER(E_INFORMATIONAL, TPOAM_6_RX_LBM_INVALID_TARGET_TLV_TYPE, pst_oam->megid_str,
            tlv_type, TPOAM_Y1731_PDU_TARGET_MEP_MIP_ID_TLV);
        return TPOAM_E_PDU_INVALID_TARGET_TLV_TYPE;
    }    
    TLV_DECODE_GETW(lb_mep_mip_len);
    if (TPOAM_Y1731_PDU_TARGET_MEP_MIP_ID_LEN != lb_mep_mip_len)
    {
        TPOAM_LOG_USER(E_INFORMATIONAL, TPOAM_6_RX_LBM_INVALID_TARGET_TLV_LEN, pst_oam->megid_str,
            lb_mep_mip_len, TPOAM_Y1731_PDU_TARGET_MEP_MIP_ID_LEN);
        return TPOAM_E_PDU_INVALID_TARGET_TLV_LEN;
    }  
    TLV_DECODE_GETC(pst_pdu->lb_sub_type);

    switch (pst_pdu->lb_sub_type)
    {
    case TPOAM_Y1731_PDU_LB_SUB_TYPE_DISCOVERY_INGRESS_NODE:
        if (1 != pst_pdu->label.ttl)
        {
            TPOAM_LOG_USER(E_INFORMATIONAL, TPOAM_6_RX_LBM_DISCOVERY_TTL_MISMATCH, 
                pst_oam->megid_str, pst_pdu->label.ttl);
            return TPOAM_E_LB_DISCOVERY_TTL_MISMATCH;
        }
        check_len = TPOAM_Y1731_PDU_MEP_MIP_ID_LEN;
        TPOAM_PDU_TLV_LEN_LEFT_CHECK(check_len);
        TLV_DECODE_GET(mep_mip_str, check_len);
        TPOAM_OP_ERR_RET_NO_LOG(_tpoam_lb_zero_check(pst_oam, TRUE, mep_mip_str, check_len));
        break;

    case TPOAM_Y1731_PDU_LB_SUB_TYPE_ICC_BASED_MEP_ID:
        /* LB MEP : should check MEP ID is self */
/*
       0                   1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |     Type      |          Length (25)          |Sub-Type (0x02)|
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |             MEP ID            |                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                               |
      |                              ...                              |
      |                          MUST be ZERO                         |
      |                              ...                              |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

    Figure 8 Target or Replying MEP/MIP ID TLV format (ICC-based MEP ID)
 */
        
        TLV_DECODE_GETW(mep_id);
        if (TPOAM_MP_TYPE_MEP != pst_oam->mp_type)
        {
            return TPOAM_E_SERVICE_NON_MEP;
        }
        if (!pst_oam->mepid)
        {
            TPOAM_LOG_USER(E_INFORMATIONAL, TPOAM_6_RX_LBM_LMEP_NOT_EXIST, pst_oam->megid_str);
            return TPOAM_E_LMEP_NOT_EXIST;
        }        
        if (mep_id != pst_oam->mepid)
        {
            TPOAM_LOG_USER(E_INFORMATIONAL, TPOAM_6_RX_LBM_MEP_ID_MISMATCH, pst_oam->megid_str, 
                mep_id, pst_oam->mepid);
            return TPOAM_E_LB_MEP_ID_MISMATCH;
        }
        
        check_len = TPOAM_Y1731_PDU_MEP_MIP_ID_LEN - TPOAM_Y1731_PDU_MEP_ID_LEN;
        TPOAM_PDU_TLV_LEN_LEFT_CHECK(check_len);
        TLV_DECODE_GET(mep_mip_str, check_len);
        TPOAM_OP_ERR_RET_NO_LOG(_tpoam_lb_zero_check(pst_oam, TRUE, mep_mip_str, check_len));
        break;

    case TPOAM_Y1731_PDU_LB_SUB_TYPE_ICC_BASED_MIP_ID:
/*
       0                   1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |     Type      |          Length (25)          |Sub-Type (0x03)|
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                    ITU-T Carrier Code (ICC)                   |
      |                               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                               |            Node-ID            |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |            Node-ID            |             IF-Num            |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |             IF-Num            |                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                               |
      |                              ...                              |
      |                          MUST be ZERO                         |
      |                              ...                              |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */        
        /* LB MIP */
        if (TPOAM_MP_TYPE_MIP != pst_oam->mp_type)
        {
            return TPOAM_E_SERVICE_NON_MIP;
        }
        sal_memset(local_icc_str, 0, sizeof(local_icc_str));
        sal_memcpy(local_icc_str, pst_oam->megid_str, pst_oam->icclen);
        sal_memset(icc_str, 0, sizeof(icc_str));
        TPOAM_PDU_TLV_LEN_LEFT_CHECK(GLB_TPOAM_ICC_LEN);
        TLV_DECODE_GET(icc_str, GLB_TPOAM_ICC_LEN);

        //if (!pst_oam->mip_en)
        {
            //TPOAM_LOG_USER(E_INFORMATIONAL, TPOAM_6_RX_LBM_MIP_NOT_EXIST, pst_oam->megid_str);
            //return TPOAM_E_MIP_NOT_EXIST;
        }

        if (sal_memcmp(local_icc_str, icc_str, GLB_TPOAM_ICC_LEN))
        {
            TPOAM_LOG_USER(E_INFORMATIONAL, TPOAM_6_RX_LBM_ICC_MISMATCH, 
                pst_oam->megid_str, icc_str, local_icc_str);
            return TPOAM_E_LB_ICC_MISMATCH;
        }
        TLV_DECODE_GETL(node_id);
        node_id = sal_ntoh32(node_id);
        if (p_db_tpoam_glb->node_id != node_id)
        {
            node_id_addr.s_addr = node_id;
            local_node_id_addr.s_addr = p_db_tpoam_glb->node_id;
            sal_memset(node_id_str, 0, sizeof(node_id_str));
            sal_memset(local_node_id_str, 0, sizeof(local_node_id_str));
            cdb_addr_ipv4_val2str(node_id_str, 32, &node_id_addr);
            cdb_addr_ipv4_val2str (local_node_id_str, 32, &local_node_id_addr);
            TPOAM_LOG_USER(E_INFORMATIONAL, TPOAM_6_RX_LBM_NODE_ID_MISMATCH, pst_oam->megid_str,
                node_id_str, local_node_id_str);
            return TPOAM_E_LB_NODE_ID_MISMATCH;
        }
        TLV_DECODE_GETL(ifnum);
        if (TPOAM_Y1731_PDU_IF_NUM_OF_PER_NODE_MIP != ifnum)
        {
            return TPOAM_E_PDU_UNSUPPORT_IF_NUM;
        }
        check_len = TPOAM_Y1731_PDU_MEP_MIP_ID_LEN - TPOAM_Y1731_PDU_MIP_ID_LEN;
        TPOAM_PDU_TLV_LEN_LEFT_CHECK(check_len);
        TLV_DECODE_GET(mep_mip_str, check_len);
        TPOAM_OP_ERR_RET_NO_LOG(_tpoam_lb_zero_check(pst_oam, TRUE, mep_mip_str, check_len));
        break;

    default:
        return TPOAM_E_LB_UNSUPPORT_LB_TYPE;
    }

    /* optional TLV */
    do {
        p_crc_start = (*pnt); 
        TLV_DECODE_GETC(tlv_type);
        switch (tlv_type)
        {
        case TPOAM_Y1731_PDU_REQUEST_MEP_ID_TLV:
            TLV_DECODE_GETW(tlv_length);
            pch_loopback_ind = (*pnt);
            //TLV_DECODE_GETC(loopback_ind);
            *pch_loopback_ind = TPOAM_PDU_LB_INDICATION_LBR;
            TLV_DECODE_GETW(mep_id);
            if (TPOAM_MP_TYPE_MEP == pst_oam->mp_type)
            {
                /* only MEP & discovery sub type need to check MEP ID */
                if (TPOAM_Y1731_PDU_LB_SUB_TYPE_ICC_BASED_MEP_ID == pst_pdu->lb_sub_type
                    || TPOAM_Y1731_PDU_LB_SUB_TYPE_DISCOVERY_INGRESS_NODE == pst_pdu->lb_sub_type)
                {
                    if (!pst_oam->mepid)
                    {
                        TPOAM_LOG_USER(E_INFORMATIONAL, TPOAM_6_RX_LBM_LMEP_NOT_EXIST, pst_oam->megid_str);                        
                        return TPOAM_E_LMEP_NOT_EXIST;
                    }
                    if (mep_id != pst_oam->rmepid)
                    {
                        TPOAM_LOG_USER(E_INFORMATIONAL, TPOAM_6_RX_LBM_MEP_ID_MISMATCH_IN_REQUEST_TLV, 
                            pst_oam->megid_str, mep_id, pst_oam->rmepid);
                        return TPOAM_E_LB_MEP_ID_MISMATCH_IN_REQUEST_TLV;
                    }
                }
            }
            TPOAM_PDU_TLV_LEN_LEFT_CHECK(TPOAM_Y1731_PDU_MEG_ID_48_LEN);
            TLV_DECODE_GET(meg_id_str, TPOAM_Y1731_PDU_MEG_ID_48_LEN);
            sal_memset(local_meg_id_str, 0, sizeof(local_meg_id_str));
            tpoam_api_build_megid(pst_oam->megid_str, local_meg_id_str);
            if (sal_memcmp(meg_id_str, local_meg_id_str, TPOAM_Y1731_PDU_MEG_ID_48_LEN))
            {
                TPOAM_LOG_USER(E_INFORMATIONAL, TPOAM_6_RX_LBM_MEG_ID_MISMATCH_IN_REQUEST_TLV, 
                    pst_oam->megid_str, &meg_id_str[3]);
                return TPOAM_E_LB_MEG_ID_MISMATCH_IN_REQUEST_TLV;
            }
            TLV_ENCODE_PUT_EMPTY(TPOAM_Y1731_PDU_REQUEST_MEP_ID_RSVD_LEN);
            break;
        case TPOAM_Y1731_PDU_DATA_TLV:
            /* Data TLV format - 9.3.2 pg 35 of ITU-T Y.1731 */
            /* Length */
            TLV_DECODE_GETW(tlv_length);
            /* Data : only support 0 */
            DECODE_GET_EMPTY(tlv_length);
            break;

        case TPOAM_Y1731_PDU_TEST_TLV:
            /* Length */
            TLV_DECODE_GETW(tlv_length);
            /* Pattern type */
            TLV_DECODE_GETC(test_tlv_mode);

            sal_memset(test_tlv, 0, sizeof(test_tlv));
            switch (test_tlv_mode)
            {
            case TPOAM_TEST_MODE_0_NULL_WO_CRC:
                tlv_data_length = tlv_length - TPOAM_Y1731_PDU_TEST_TLV_PATTERN_LEN;
                if (tlv_data_length <= 0)
                {
                    return TPOAM_E_LB_TEST_TLV_LENGTH_CHECK;
                }
                TPOAM_PDU_TLV_LEN_LEFT_CHECK(tlv_data_length);
                TLV_DECODE_GET(test_tlv, tlv_data_length);
                for (i = 0; i < tlv_data_length; i++)
                {
                    if (test_tlv[i] != 0)
                    {
                        return TPOAM_E_LB_TEST_TLV_NON_ZERO_CHECK;
                    }
                }
                break;
            case TPOAM_TEST_MODE_1_NULL_WI_CRC:
                tlv_data_length = tlv_length - (TPOAM_Y1731_PDU_TEST_TLV_PATTERN_LEN+TPOAM_Y1731_PDU_TEST_TLV_CRC_LEN);
                if (tlv_data_length <= 0)
                {
                    return TPOAM_E_LB_TEST_TLV_LENGTH_CHECK;
                }
                TPOAM_PDU_TLV_LEN_LEFT_CHECK(tlv_data_length);
                TLV_DECODE_GET(test_tlv, tlv_data_length);
                for (i = 0; i < tlv_data_length; i++)
                {
                    if (test_tlv[i] != 0)
                    {
                        return TPOAM_E_LB_TEST_TLV_NON_ZERO_CHECK;
                    }
                }
                TLV_DECODE_GETL(crc_in_pkt);
                crc_in_pkt = sal_ntoh32(crc_in_pkt);
                crc = ctclib_gen_crc32(0, (char*)p_crc_start, tlv_length + TPOAM_Y1731_PDU_TEST_TLV_TYPE_LEN_LEN - TPOAM_Y1731_PDU_TEST_TLV_CRC_LEN);
                if (crc != crc_in_pkt)
                {
                    TPOAM_DBG_LB_LBM("Oam session %u MEG %s RX LBM with NULL test TLV crc 0x%08X, but calculated crc 0x%08X", 
                        pst_oam->key.session_id, pst_oam->megid_str, crc_in_pkt, crc);
                    return TPOAM_E_LB_TEST_TLV_CRC_CHECK;
                }
                break;
            case TPOAM_TEST_MODE_2_PRBS_WO_CRC:
                tlv_data_length = tlv_length - TPOAM_Y1731_PDU_TEST_TLV_PATTERN_LEN;
                if (tlv_data_length <= 0)
                {
                    return TPOAM_E_LB_TEST_TLV_LENGTH_CHECK;
                }
                TPOAM_PDU_TLV_LEN_LEFT_CHECK(tlv_data_length);
                TLV_DECODE_GET(test_tlv, tlv_data_length);
                break;
            case TPOAM_TEST_MODE_3_PRBS_WI_CRC:
                tlv_data_length = tlv_length - (TPOAM_Y1731_PDU_TEST_TLV_PATTERN_LEN+TPOAM_Y1731_PDU_TEST_TLV_CRC_LEN);
                if (tlv_data_length <= 0)
                {
                    return TPOAM_E_LB_TEST_TLV_LENGTH_CHECK;
                }
                TPOAM_PDU_TLV_LEN_LEFT_CHECK(tlv_data_length);
                TLV_DECODE_GET(test_tlv, tlv_data_length);
                TLV_DECODE_GETL(crc_in_pkt);
                crc_in_pkt = sal_ntoh32(crc_in_pkt);
                crc = ctclib_gen_crc32(0, (char*)p_crc_start, tlv_length + TPOAM_Y1731_PDU_TEST_TLV_TYPE_LEN_LEN - TPOAM_Y1731_PDU_TEST_TLV_CRC_LEN);
                if (crc != crc_in_pkt)
                {
                    TPOAM_DBG_LB_LBM("Oam session %u MEG %s RX LBM with PRBS test TLV crc 0x%08X, but calculated crc 0x%08X", 
                        pst_oam->key.session_id, pst_oam->megid_str, crc_in_pkt, crc);
                    return TPOAM_E_LB_TEST_TLV_CRC_CHECK;
                }
                break;
            default:
                TPOAM_ASSERT;
            }
        
        case TPOAM_Y1731_PDU_END_TLV:
            break;            
        default:
            return TPOAM_E_PDU_DECODE_TLV_FAIL;
        }
    } while ((TPOAM_Y1731_PDU_END_TLV != tlv_type) && (0 < pst_pdu->left_len));

    return TPOAM_E_NONE;
}


static int32
_tpoam_lb_tx_lbr(tbl_oam_session_t* pst_oam, tpoam_rx_pdu_t* pst_pdu)
{
    tbl_tpoam_global_t *p_db_tpoam_glb = tbl_tpoam_global_get_tpoam_global();
    uint8 lbr_data[TPOAM_LB_PKT_BUF_SIZE];
    uint32 direction = 0;
    int32 rc = 0;
    uint32 pkt_size = 0;

    tbl_lsp_p_t *pst_lsp_p = NULL;

    uint32 lb_data_left = TPOAM_LB_PKT_BUF_SIZE;
    tpoam_sk_buff_t *tx_skb;
    tx_skb = tpoam_alloc_skb(TPOAM_LB_PKT_BUF_SIZE);
    if (!tx_skb)
    {
        TPOAM_LOG_ERR("Oam session %u, cannot allocate memory for send LBR packet", pst_oam->key.session_id);
        goto TPOAM_FINALLY;
    }
    
    sal_memset(lbr_data, 0, sizeof(lbr_data));
    
    rc = _tpoam_lb_encode_lbr(pst_oam, pst_pdu, tx_skb->data, &lb_data_left);
    if (rc != 0)
    {
        goto TPOAM_FINALLY;
    }

    if (TPOAM_MP_TYPE_MIP == pst_oam->mp_type)
    {
        pst_lsp_p = tbl_lsp_p_get_lsp_p_by_name(pst_oam->service_name);
        if (NULL == pst_lsp_p)
        {
            TPOAM_LOG_ERR("Oam session %u, cannot get LSP_P %s", pst_oam->key.session_id, pst_oam->service_name);
            return TPOAM_E_FAIL;
        }
        else
        {
            if (pst_pdu->label.label == pst_lsp_p->east_inlabel)
            {
                pst_pdu->direction = TPOAM_DIR_EAST;
            }
            else if (pst_pdu->label.label == pst_lsp_p->west_inlabel)
            {
                pst_pdu->direction = TPOAM_DIR_WEST;
            }
            else
            {
                TPOAM_DBG_LB_LBR("Oam session %u, no match inlabel %u", pst_oam->key.session_id, pst_pdu->label.label);
                return TPOAM_E_FAIL;
            }
        }
    }
    
    if (TPOAM_DIR_EAST == pst_pdu->direction || TPOAM_DIR_WEST == pst_pdu->direction)
    {
        direction = pst_pdu->direction;
    }
    else
    {
        TPOAM_ASSERT;
    }

    pkt_size = TPOAM_LB_PKT_BUF_SIZE - lb_data_left;
    tx_skb->len = pkt_size;

    TPOAM_DBG_LB_LBR("Oam session %u MEG %s TX %s LBR, TransID %d, exp %d, ttl %d, payload length %d, pkt size %u", 
        pst_oam->key.session_id, pst_oam->megid_str, 
        _tpoam_lb_sub_type_desc(pst_pdu->lb_sub_type), 
        pst_pdu->lb_trans_id, pst_pdu->label.exp, pst_pdu->label.ttl, pst_pdu->y1731_len, pkt_size);

    tx_skb->len = pst_pdu->y1731_len;
    
    //TPOAM_OP_ERR_RET(tpoam_sock_tx(pst_oam, direction, pst_pdu->y1731_len, lbr_data, 
    //    pst_pdu->label.exp, TPOAM_SOCK_TTL_DEF, TPOAM_SOCK_DM_FLAG_UNSET, FALSE, TRUE));
    rc = tpoam_sock_tx_new(pst_oam, direction, pkt_size, tx_skb,
        pst_pdu->label.exp, TPOAM_SOCK_TTL_DEF, TPOAM_SOCK_DM_FLAG_UNSET, FALSE, TRUE, TPOAM_Y1731_PDU_LBR);
    if (rc != 0)
    {
        goto TPOAM_FINALLY;
    }
    
    tpoam_api_stats_add_tx(&pst_oam->oam_stats.tx, TPOAM_Y1731_PDU_LBR);
    tpoam_api_stats_add_tx(&p_db_tpoam_glb->oam_glb_stats.tx, TPOAM_Y1731_PDU_LBR);        

 TPOAM_FINALLY:

    tpoam_free_skb(&tx_skb);
    return TPOAM_E_NONE;
}


int32
tpoam_lb_rx_lbm(tbl_oam_session_t* pst_oam, tpoam_rx_pdu_t* pst_pdu)
{
    /* 5.2. OAM Loopback (MT-LBM/LBR) procedures */
    int32 ret = TPOAM_E_NONE;
    
    if (TPOAM_Y1731_PDU_LBM != pst_pdu->hdr.opcode)
    {
        return TPOAM_E_PDU_INVALID_OPCODE;
    }

    ret = _tpoam_lb_decode_lbm(pst_oam, pst_pdu);
    if (ret < 0)
    {
        return ret;
    }

    TPOAM_DBG_LB_LBM("Oam session %u MEG %s RX %s LBM, TransID %d, exp %d, ttl %d, payload length %d", 
        pst_oam->key.session_id, pst_oam->megid_str, 
        _tpoam_lb_sub_type_desc(pst_pdu->lb_sub_type), 
        pst_pdu->lb_trans_id, pst_pdu->label.exp, pst_pdu->label.ttl, pst_pdu->y1731_len);

    TPOAM_OP_ERR_RET(_tpoam_lb_tx_lbr(pst_oam, pst_pdu));
    return TPOAM_E_NONE;
}

static int32
_tpoam_lb_decode_lbr(tbl_oam_session_t* pst_oam, tpoam_rx_pdu_t* pst_pdu)
{
    uint16 mep_id = 0;
    uint16 tlv_length = 0;
    int32 tlv_data_length = 0;
    uint8 test_tlv_mode = 0;
    uint8 meg_id_str[TPOAM_Y1731_PDU_MEG_ID_48_LEN];
    uint8 local_meg_id_str[TPOAM_Y1731_PDU_MEG_ID_48_LEN];
    uint8 test_tlv[TPOAM_LB_PKT_BUF_SIZE];
    uint32 crc_in_pkt = 0;
    uint32 crc = 0;
    uint32 i = 0;
    //uint8 loopback_ind = 0;
    uint8* pch_loopback_ind = NULL;
    uint8* p_crc_start = NULL; 
    struct sal_in4_addr node_id_addr;
    struct sal_in4_addr send_node_id_addr;
    char node_id_str[IPV4_ADDR_STRLEN+1];
    char send_node_id_str[IPV4_ADDR_STRLEN+1];
    tpoam_lb_t* pst_lb = &pst_oam->oam_lb;
    uint8** pnt = &pst_pdu->p_data;
    int32* size = &pst_pdu->left_len;
    
    /*
       0                   1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |0 0 0 1|0 0 0 0|0 0 0 0 0 0 0 0|  Y.1731 Channel Type (0xXXXX) |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      | MEL | Version |    OpCode     |     Flags     |   TLV Offset  |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |               Transaction ID/Sequence Number                  |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                    Target MEP/MIP ID TLV                      |
      |                              ...                              |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |               [optional Requesting MEP ID TLV]                |
      |                              ...                              |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      | [other optional TLV starts here]                              |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |     End TLV     |
      +-+-+-+-+-+-+-+-+-+
                        Figure 2 LBM Packet Format
    */
    TLV_DECODE_GETL(pst_pdu->lb_trans_id);

    /*
       0                   1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |  Type (0x21)  |          Length (25)          |   Sub-Type    |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                              ...                              |
      |      MEP/MIP Identifier (format is ID Sub-Type specific)      |
      |                              ...                              |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

                   Figure 4 Target MEP/MIP ID TLV format
    */
    uint8 tlv_type = 0;
    uint16 lb_mep_mip_len = 0;
    uint8 icc_str[GLB_TPOAM_ICC_LEN+1];
    uint8 local_icc_str[GLB_TPOAM_ICC_LEN+1];
    uint8 mep_mip_str[TPOAM_Y1731_PDU_MEP_MIP_ID_LEN];
    uint32 ifnum = 0;
    uint32 lbr_node_id = 0;
    uint32 lbr_mep_id = 0;
    uint32 check_len = 0;

    TLV_DECODE_GETC(tlv_type);
    if (TPOAM_Y1731_PDU_REPLY_MEP_MIP_ID_TLV != tlv_type)
    {
        TPOAM_LOG_USER(E_INFORMATIONAL, TPOAM_6_RX_LBR_INVALID_REPLY_TLV_TYPE, pst_oam->megid_str,
            tlv_type, TPOAM_Y1731_PDU_REPLY_MEP_MIP_ID_TLV);
        return TPOAM_E_PDU_INVALID_REPLY_TLV_TYPE;
    }
    TLV_DECODE_GETW(lb_mep_mip_len);
    if (TPOAM_Y1731_PDU_REPLY_MEP_MIP_ID_LEN != lb_mep_mip_len)
    {
        TPOAM_LOG_USER(E_INFORMATIONAL, TPOAM_6_RX_LBR_INVALID_REPLY_TLV_LEN, pst_oam->megid_str,
            lb_mep_mip_len, TPOAM_Y1731_PDU_REPLY_MEP_MIP_ID_LEN);
        return TPOAM_E_PDU_INVALID_REPLY_TLV_LEN;
    }      
    TLV_DECODE_GETC(pst_pdu->lb_sub_type);

    switch (pst_pdu->lb_sub_type)
    {
    case TPOAM_Y1731_PDU_LB_SUB_TYPE_DISCOVERY_INGRESS_NODE:
        /* LBR should not send discovery */
        return TPOAM_E_PDU_LBR_WITH_DISCOVERY;

    case TPOAM_Y1731_PDU_LB_SUB_TYPE_ICC_BASED_MEP_ID:
        /* LB MEP : should check MEP ID is self */
/*
       0                   1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |     Type      |          Length (25)          |Sub-Type (0x02)|
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |             MEP ID            |                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                               |
      |                              ...                              |
      |                          MUST be ZERO                         |
      |                              ...                              |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

    Figure 8 Target or Replying MEP/MIP ID TLV format (ICC-based MEP ID)
 */
        TLV_DECODE_GETW(lbr_mep_id);
        if (TPOAM_LB_DISCOVERY != pst_lb->lbm_para.lb_sub_type)
        {
            if (lbr_mep_id != pst_lb->lbm_para.rmep_id)
            {
                /* The Target MEP/MIP-ID and Originator MEP-ID fields are is set to
                 * the value which is copied from the last received MT-LBM PDU;
                 */
                TPOAM_LOG_USER(E_INFORMATIONAL, TPOAM_6_RX_LBR_MEPID_MISMATCH, 
                        pst_oam->megid_str, lbr_mep_id, pst_lb->lbm_para.rmep_id);
                return TPOAM_E_PDU_LBR_MEPID_MISMATCH;
            }
        }
        check_len = TPOAM_Y1731_PDU_MEP_MIP_ID_LEN - TPOAM_Y1731_PDU_MEP_ID_LEN;
        TPOAM_PDU_TLV_LEN_LEFT_CHECK(check_len);
        TLV_DECODE_GET(mep_mip_str, check_len);
        TPOAM_OP_ERR_RET_NO_LOG(_tpoam_lb_zero_check(pst_oam, FALSE, mep_mip_str, check_len));
        /* store the returned MEP info */
        if (TPOAM_LB_DISCOVERY == pst_lb->lbm_para.lb_sub_type)
        {
            pst_lb->discovery.reply.ttl = pst_lb->discovery.curr_ttl;
            pst_lb->discovery.reply.lb_sub_type = TPOAM_LB_MEP;
            pst_lb->discovery.reply.mep_id = lbr_mep_id;
        }
        break;

    case TPOAM_Y1731_PDU_LB_SUB_TYPE_ICC_BASED_MIP_ID:
/*
       0                   1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |     Type      |          Length (25)          |Sub-Type (0x03)|
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                    ITU-T Carrier Code (ICC)                   |
      |                               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                               |            Node-ID            |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |            Node-ID            |             IF-Num            |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |             IF-Num            |                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                               |
      |                              ...                              |
      |                          MUST be ZERO                         |
      |                              ...                              |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */        
        /* LB MIP */
        sal_memset(local_icc_str, 0, sizeof(local_icc_str));
        sal_memcpy(local_icc_str, pst_oam->megid_str, pst_oam->icclen);
        sal_memset(icc_str, 0, sizeof(icc_str));
        TPOAM_PDU_TLV_LEN_LEFT_CHECK(GLB_TPOAM_ICC_LEN);
        TLV_DECODE_GET(icc_str, GLB_TPOAM_ICC_LEN);

        if (TPOAM_LB_DISCOVERY != pst_lb->lbm_para.lb_sub_type)
        {
            /* The Target MEP/MIP-ID and Originator MEP-ID fields are is set to
             * the value which is copied from the last received MT-LBM PDU;
             */
            if (sal_memcmp(local_icc_str, icc_str, GLB_TPOAM_ICC_LEN))
            {
                TPOAM_LOG_USER(E_INFORMATIONAL, TPOAM_6_RX_LBR_ICC_MISMATCH, 
                    pst_oam->megid_str, icc_str, local_icc_str);
                return TPOAM_E_LB_ICC_MISMATCH;
            }
        }
        TLV_DECODE_GETL(lbr_node_id);
        lbr_node_id = sal_ntoh32(lbr_node_id);
        if (TPOAM_LB_DISCOVERY != pst_lb->lbm_para.lb_sub_type)
        {        
            if (lbr_node_id != pst_lb->lbm_para.node_id)
            {
                node_id_addr.s_addr = lbr_node_id;
                send_node_id_addr.s_addr = pst_lb->lbm_para.node_id;
                sal_memset(node_id_str, 0, sizeof(node_id_str));
                sal_memset(send_node_id_str, 0, sizeof(send_node_id_str));
                cdb_addr_ipv4_val2str(node_id_str, 32, &node_id_addr);
                cdb_addr_ipv4_val2str (send_node_id_str, 32, &send_node_id_addr);        
                TPOAM_LOG_USER(E_INFORMATIONAL, TPOAM_6_RX_LBR_NODE_ID_MISMATCH, pst_oam->megid_str,
                    node_id_str, send_node_id_str);
                return TPOAM_E_LB_NODE_ID_MISMATCH;
            }
        }
        TLV_DECODE_GETL(ifnum);
        check_len = TPOAM_Y1731_PDU_MEP_MIP_ID_LEN - TPOAM_Y1731_PDU_MIP_ID_LEN;
        TPOAM_PDU_TLV_LEN_LEFT_CHECK(check_len);
        TLV_DECODE_GET(mep_mip_str, check_len);
        TPOAM_OP_ERR_RET_NO_LOG(_tpoam_lb_zero_check(pst_oam, FALSE, mep_mip_str, check_len));
        
        /* store the returned MIP info */
        if (TPOAM_LB_DISCOVERY == pst_lb->lbm_para.lb_sub_type)
        {
            pst_lb->discovery.reply.ttl = pst_lb->discovery.curr_ttl;
            pst_lb->discovery.reply.lb_sub_type = TPOAM_LB_MIP;            
            sal_memset(pst_lb->discovery.reply.icc, 0, sizeof(pst_lb->discovery.reply.icc));
            sal_memcpy(pst_lb->discovery.reply.icc, icc_str, GLB_TPOAM_ICC_LEN);
            pst_lb->discovery.reply.node_id = lbr_node_id;
            pst_lb->discovery.reply.if_num = ifnum;
        }
        break;

    default:
        return TPOAM_E_LB_UNSUPPORT_LB_TYPE;
    }

    /* optional TLV */
    do {
        p_crc_start = (*pnt); 
        TLV_DECODE_GETC(tlv_type);
        switch (tlv_type)
        {
        case TPOAM_Y1731_PDU_REQUEST_MEP_ID_TLV:
            TLV_DECODE_GETW(tlv_length);
            pch_loopback_ind = (*pnt);
            //TLV_DECODE_GETC(loopback_ind);
            *pch_loopback_ind = TPOAM_PDU_LB_INDICATION_LBR;
            TLV_DECODE_GETW(mep_id);
            /* only MEP & discovery sub type need to check MEP ID */
            if (TPOAM_Y1731_PDU_LB_SUB_TYPE_ICC_BASED_MEP_ID == pst_pdu->lb_sub_type
                || TPOAM_Y1731_PDU_LB_SUB_TYPE_DISCOVERY_INGRESS_NODE == pst_pdu->lb_sub_type)
            {
                if (!pst_oam->mepid)
                {
                    return TPOAM_E_LMEP_NOT_EXIST;
                }
                if (mep_id != pst_oam->mepid)
                {
                    TPOAM_LOG_USER(E_INFORMATIONAL, TPOAM_6_RX_LBR_MEPID_MISMATCH_IN_REQUEST_TLV, 
                        pst_oam->megid_str, mep_id, pst_oam->mepid);
                    return TPOAM_E_LB_MEP_ID_MISMATCH_IN_REQUEST_TLV;
                }
            }
            TPOAM_PDU_TLV_LEN_LEFT_CHECK(TPOAM_Y1731_PDU_MEG_ID_48_LEN);
            TLV_DECODE_GET(meg_id_str, TPOAM_Y1731_PDU_MEG_ID_48_LEN);
            sal_memset(local_meg_id_str, 0, sizeof(local_meg_id_str));
            tpoam_api_build_megid(pst_oam->megid_str, local_meg_id_str);
            if (sal_memcmp(meg_id_str, local_meg_id_str, TPOAM_Y1731_PDU_MEG_ID_48_LEN))
            {
                TPOAM_LOG_USER(E_INFORMATIONAL, TPOAM_6_RX_LBR_MEGID_MISMATCH_IN_REQUEST_TLV, 
                    pst_oam->megid_str, &meg_id_str[3]);
                return TPOAM_E_LB_MEG_ID_MISMATCH_IN_REQUEST_TLV;
            }
            TLV_ENCODE_PUT_EMPTY(TPOAM_Y1731_PDU_REQUEST_MEP_ID_RSVD_LEN);
            break;
        case TPOAM_Y1731_PDU_DATA_TLV:
            /* Data TLV format - 9.3.2 pg 35 of ITU-T Y.1731 */
            /* Length */
            TLV_DECODE_GETW(tlv_length);
            /* Data : only support 0 */
            DECODE_GET_EMPTY(tlv_length);
            break;

        case TPOAM_Y1731_PDU_TEST_TLV:
            /* Length */
            TLV_DECODE_GETW(tlv_length);
            /* Pattern type */
            TLV_DECODE_GETC(test_tlv_mode);

            sal_memset(test_tlv, 0, sizeof(test_tlv));
            switch (test_tlv_mode)
            {
            case TPOAM_TEST_MODE_0_NULL_WO_CRC:
                tlv_data_length = tlv_length - TPOAM_Y1731_PDU_TEST_TLV_PATTERN_LEN;
                if (tlv_data_length <= 0)
                {
                    return TPOAM_E_LB_TEST_TLV_LENGTH_CHECK;
                }
                TPOAM_PDU_TLV_LEN_LEFT_CHECK(tlv_data_length);
                TLV_DECODE_GET(test_tlv, tlv_data_length);
                for (i = 0; i < tlv_data_length; i++)
                {
                    if (test_tlv[i] != 0)
                    {
                        return TPOAM_E_LB_TEST_TLV_NON_ZERO_CHECK;
                    }
                }
                break;
            case TPOAM_TEST_MODE_1_NULL_WI_CRC:
                tlv_data_length = tlv_length - (TPOAM_Y1731_PDU_TEST_TLV_PATTERN_LEN+TPOAM_Y1731_PDU_TEST_TLV_CRC_LEN);
                if (tlv_data_length <= 0)
                {
                    return TPOAM_E_LB_TEST_TLV_LENGTH_CHECK;
                }
                TPOAM_PDU_TLV_LEN_LEFT_CHECK(tlv_data_length);
                TLV_DECODE_GET(test_tlv, tlv_data_length);
                for (i = 0; i < tlv_data_length; i++)
                {
                    if (test_tlv[i] != 0)
                    {
                        return TPOAM_E_LB_TEST_TLV_NON_ZERO_CHECK;
                    }
                }
                TLV_DECODE_GETL(crc_in_pkt);
                crc_in_pkt = sal_ntoh32(crc_in_pkt);
                crc = ctclib_gen_crc32(0, (char*)p_crc_start, tlv_length + TPOAM_Y1731_PDU_TEST_TLV_TYPE_LEN_LEN - TPOAM_Y1731_PDU_TEST_TLV_CRC_LEN);
                if (crc != crc_in_pkt)
                {
                    TPOAM_DBG_LB_LBR("Oam session %u MEG %s RX LBR with NULL test TLV crc 0x%08X, but calculated crc 0x%08X", 
                        pst_oam->key.session_id, pst_oam->megid_str, crc_in_pkt, crc);
                    return TPOAM_E_LB_TEST_TLV_CRC_CHECK;
                }
                break;
            case TPOAM_TEST_MODE_2_PRBS_WO_CRC:
                tlv_data_length = tlv_length - TPOAM_Y1731_PDU_TEST_TLV_PATTERN_LEN;
                if (tlv_data_length <= 0)
                {
                    return TPOAM_E_LB_TEST_TLV_LENGTH_CHECK;
                }
                TPOAM_PDU_TLV_LEN_LEFT_CHECK(tlv_data_length);
                TLV_DECODE_GET(test_tlv, tlv_data_length);
                break;
            case TPOAM_TEST_MODE_3_PRBS_WI_CRC:
                tlv_data_length = tlv_length - (TPOAM_Y1731_PDU_TEST_TLV_PATTERN_LEN+TPOAM_Y1731_PDU_TEST_TLV_CRC_LEN);
                if (tlv_data_length <= 0)
                {
                    return TPOAM_E_LB_TEST_TLV_LENGTH_CHECK;
                }
                TPOAM_PDU_TLV_LEN_LEFT_CHECK(tlv_data_length);
                TLV_DECODE_GET(test_tlv, tlv_data_length);
                TLV_DECODE_GETL(crc_in_pkt);
                crc_in_pkt = sal_ntoh32(crc_in_pkt);
                crc = ctclib_gen_crc32(0, (char*)p_crc_start, tlv_length + TPOAM_Y1731_PDU_TEST_TLV_TYPE_LEN_LEN - TPOAM_Y1731_PDU_TEST_TLV_CRC_LEN);
                if (crc != crc_in_pkt)
                {
                    TPOAM_DBG_LB_LBR("Oam session %u MEG %s RX LBR with PRBS test TLV crc 0x%08X, but calculated crc 0x%08X", 
                        pst_oam->key.session_id, pst_oam->megid_str, crc_in_pkt, crc);
                    return TPOAM_E_LB_TEST_TLV_CRC_CHECK;
                }
                break;
            default:
                TPOAM_ASSERT;
            }
        
        case TPOAM_Y1731_PDU_END_TLV:
            break;            
        default:
            return TPOAM_E_PDU_DECODE_TLV_FAIL;
        }
    } while ((TPOAM_Y1731_PDU_END_TLV != tlv_type) && (0 < pst_pdu->left_len));

    if (TPOAM_Y1731_PDU_END_TLV != tlv_type)
    {
        TPOAM_LOG_USER(E_INFORMATIONAL, TPOAM_6_RX_LBR_NO_END_TLV, pst_oam->megid_str);    
        return TPOAM_E_LB_NO_END_TLV;
    }

    return TPOAM_E_NONE;
}

static int32
_tpoam_lb_tx_lbm(tbl_oam_session_t* pst_oam)
{
    tbl_tpoam_global_t *p_db_tpoam_glb = tbl_tpoam_global_get_tpoam_global();
    uint8 ttl = 0;
    uint32 rc = 0;
    uint32 pkt_size = 0;

    tpoam_sk_buff_t *tx_skb;
    tx_skb = tpoam_alloc_skb(TPOAM_LB_PKT_BUF_SIZE);
    if (!tx_skb)
    {
        TPOAM_LOG_ERR("Oam session %u, cannot allocate memory for send LBM packet", pst_oam->key.session_id);
        goto TPOAM_FINALLY;
    }

    pst_oam->oam_lb.tx_skb = tx_skb;
    pst_oam->oam_lb.p_lbm_data = tx_skb->data;
    int32 lb_data_left = TPOAM_LB_PKT_BUF_SIZE;
    rc = _tpoam_lb_encode_lbm(pst_oam, &lb_data_left);
    if (rc != 0)
    {
        goto TPOAM_FINALLY;
    }

    pkt_size = TPOAM_LB_PKT_BUF_SIZE - lb_data_left;
    tx_skb->len = pkt_size;

    ttl = (TPOAM_LB_DISCOVERY == pst_oam->oam_lb.lbm_para.lb_sub_type) ? pst_oam->oam_lb.discovery.curr_ttl : pst_oam->oam_lb.lbm_para.ttl;    

    TPOAM_DBG_LB_LBM("Oam session %u MEG %s TX %s LBM, TransID %d, tx count %u, exp %d, ttl %d, payload length %d", 
        pst_oam->key.session_id, 
        pst_oam->megid_str, 
        _tpoam_lb_sub_type_desc(pst_oam->oam_lb.lbm_para.lb_sub_type), 
        pst_oam->oam_lb.lbm_next_trans_id, 
        pst_oam->oam_lb.lbm_tx_count,
        pst_oam->oam_lb.lbm_para.exp, 
        ttl, 
        pst_oam->oam_lb.payload_len);
    
    //rc = tpoam_sock_tx_new(pst_oam, TPOAM_SOCK_DIRECTION_PE, pst_oam->oam_lb.payload_len, pst_oam->oam_lb.p_lbm_data,
    //    pst_oam->oam_lb.lbm_para.exp, ttl, TPOAM_SOCK_DM_FLAG_UNSET, FALSE, TRUE);
    rc = tpoam_sock_tx_new(pst_oam, TPOAM_SOCK_DIRECTION_PE, pst_oam->oam_lb.payload_len, tx_skb,
        pst_oam->oam_lb.lbm_para.exp, ttl, TPOAM_SOCK_DM_FLAG_UNSET, FALSE, TRUE, TPOAM_Y1731_PDU_LBM);
    if (rc != 0)
    {
        goto TPOAM_FINALLY;
    }

    tpoam_api_stats_add_tx(&pst_oam->oam_stats.tx, TPOAM_Y1731_PDU_LBM);
    tpoam_api_stats_add_tx(&p_db_tpoam_glb->oam_glb_stats.tx, TPOAM_Y1731_PDU_LBM);     
    
    return TPOAM_E_NONE;
    
TPOAM_FINALLY:

    tpoam_free_skb(&(pst_oam->oam_lb.tx_skb));

    return TPOAM_E_NONE;
}


static int32
_tpoam_lb_discovery_add_cache(tbl_oam_session_t* pst_oam, tpoam_lb_reply_t* pst_reply)
{
    tpoam_lb_t* pst_lb = &pst_oam->oam_lb;
    tpoam_lb_reply_t* pst_entry = NULL;
    
    if (NULL == pst_lb->pst_lb_cache_list)
    {
        pst_lb->pst_lb_cache_list = list_new();
    }
    
    pst_entry = XCALLOC(MEM_PM_TPOAM_LB_REPLY_ENTRY, sizeof(tpoam_lb_reply_t));
    if (!pst_entry)
    {
        return TPOAM_E_MEMORY_ALLOC_FAIL;
    }

    sal_memcpy(pst_entry, pst_reply, sizeof(tpoam_lb_reply_t));
    listnode_add(pst_lb->pst_lb_cache_list, pst_entry);

    return TPOAM_E_NONE;
}


static void
_tpoam_lb_tx_lbm_cb(void *p_data)
{
    tbl_oam_session_t* pst_oam = p_data ? (tbl_oam_session_t*)p_data : NULL;
    tpoam_lb_t* pst_lb = &pst_oam->oam_lb;
    tpoam_lb_reply_t lb_reply;
    int32 ret = TPOAM_E_NONE;
    
    pst_lb->per_lb_status = TPOAM_PER_LB_STATUS_TO;
    sal_memset(&lb_reply, 0, sizeof(lb_reply));
    lb_reply.ttl = pst_lb->discovery.curr_ttl;
    lb_reply.lb_sub_type = TPOAM_LB_DISCOVERY;  /* means timeout */
    _tpoam_lb_discovery_add_cache(pst_oam, &lb_reply);
    
    pst_lb->p_t_lb_timer = NULL;
    switch (pst_lb->lbm_para.lb_sub_type)
    {
    case TPOAM_LB_DISCOVERY:
        pst_lb->discovery.timeout_ttl = pst_lb->discovery.curr_ttl;
        if (pst_lb->discovery.curr_ttl >= pst_lb->lbm_para.ttl_end)
        {
            _tpoam_lb_deinit_lb_session(pst_oam);
            TPOAM_DBG_LB_LBR("Oam session %u MEG %s, discovery ttl %u test end", pst_oam->key.session_id, pst_oam->megid_str, pst_lb->discovery.curr_ttl);
            tpoam_lb_call_test_cb(pst_oam, pst_lb->lbm_para.lb_sub_type);
            return;
        }
        else
        {
            pst_lb->discovery.curr_ttl++;
        }
        break;
    case TPOAM_LB_MEP:  /* same as MIP branch */
    case TPOAM_LB_MIP:  
        if (pst_lb->lbm_tx_count >= pst_lb->lbm_para.repeat_cnt)
        {
            _tpoam_lb_deinit_lb_session(pst_oam);
            TPOAM_DBG_LB_LBR("Oam session %u MEG %s, MIP or MEP count %u test end", pst_oam->key.session_id, pst_oam->megid_str, pst_lb->lbm_tx_count);
            tpoam_lb_call_test_cb(pst_oam, pst_lb->lbm_para.lb_sub_type);
            return;
        }
        break;
    default:
        break;
    }

    TPOAM_DBG_LB_LBR("Oam session %u MEG %s, TX LBM but not receive LBR when timeout", pst_oam->key.session_id, pst_oam->megid_str);
    tpoam_lb_call_test_cb(pst_oam, pst_lb->lbm_para.lb_sub_type);
    
    pst_lb->lbm_next_trans_id++;
    ret = _tpoam_lb_tx_lbm(pst_oam);
    if (ret < 0)
    {
        _tpoam_lb_deinit_lb_session(pst_oam);
        return;
    }
    pst_lb->lbm_tx_count++;
    pst_lb->p_t_lb_timer = tpoam_util_add_timer_ms(_tpoam_lb_tx_lbm_cb, pst_oam,
                         (pst_lb->lbm_para.time_out * 1000));
    return;
}


static int32
_tpoam_lb_handle_lbr(tbl_oam_session_t* pst_oam, tpoam_rx_pdu_t* pst_pdu)
{
    tpoam_lb_t* pst_lb = &pst_oam->oam_lb;
    int32 ret = TPOAM_E_NONE;
    
    if (!pst_lb->active)
    {
        return TPOAM_E_NONE;
    }
    
    if (TPOAM_LB_DISCOVERY == pst_lb->lbm_para.lb_sub_type)
    {
        if (pst_lb->lbm_next_trans_id != pst_pdu->lb_trans_id)
        {
            pst_lb->lbr_out_order_count++;
            TPOAM_LOG_USER(E_INFORMATIONAL, TPOAM_6_RX_LBR_TRANS_ID_MISMATCH, 
                pst_oam->megid_str, pst_pdu->lb_trans_id, pst_lb->lbm_next_trans_id);                
            return TPOAM_E_LB_TRANS_ID_MISMATCH;
        }    
        pst_lb->lbr_rx_count++;
        pst_lb->per_lb_status = TPOAM_PER_LB_STATUS_RX_LBR;
        _tpoam_lb_discovery_add_cache(pst_oam, &pst_lb->discovery.reply);

        //if (l2_is_timer_running (pst_lb->p_t_lb_timer))
        {
            tpoam_util_cancel_timer(&pst_lb->p_t_lb_timer);
        }
        pst_lb->p_t_lb_timer = NULL;
        
        if (pst_lb->discovery.curr_ttl >= pst_lb->lbm_para.ttl_end || TPOAM_LB_MEP == pst_lb->discovery.reply.lb_sub_type)
        {
            TPOAM_OP_ERR_RET(_tpoam_lb_deinit_lb_session(pst_oam));
            return TPOAM_E_NONE;
        }
        else
        {
            pst_lb->discovery.curr_ttl++;
            pst_lb->lbm_next_trans_id++;
            ret = _tpoam_lb_tx_lbm(pst_oam);
            if (ret < 0)
            {
                _tpoam_lb_deinit_lb_session(pst_oam);
                return ret;
            }
            pst_lb->lbm_tx_count++;
            pst_lb->p_t_lb_timer = tpoam_util_add_timer_ms(_tpoam_lb_tx_lbm_cb, pst_oam,
                         (pst_lb->lbm_para.time_out * 1000));
        }
        
        return TPOAM_E_NONE;
    }
    else
    {
        if (pst_lb->lbm_next_trans_id != pst_pdu->lb_trans_id)
        {
            pst_lb->lbr_out_order_count++;
            TPOAM_LOG_USER(E_INFORMATIONAL, TPOAM_6_RX_LBR_TRANS_ID_MISMATCH, 
                pst_oam->megid_str, pst_pdu->lb_trans_id, pst_lb->lbm_next_trans_id);              
            return TPOAM_E_LB_TRANS_ID_MISMATCH;
        }
        pst_lb->lbr_rx_count++;
        pst_lb->per_lb_status = TPOAM_PER_LB_STATUS_RX_LBR;

        //if (l2_is_timer_running (pst_lb->p_t_lb_timer))
        {
            tpoam_util_cancel_timer(&pst_lb->p_t_lb_timer);
        }
        pst_lb->p_t_lb_timer = NULL;
        if (pst_lb->lbm_tx_count >= pst_lb->lbm_para.repeat_cnt)
        {
            TPOAM_OP_ERR_RET(_tpoam_lb_deinit_lb_session(pst_oam));
            return TPOAM_E_NONE;
        }
        else
        {
            pst_lb->lbm_next_trans_id++;
            ret = _tpoam_lb_tx_lbm(pst_oam);
            if (ret < 0)
            {
                _tpoam_lb_deinit_lb_session(pst_oam);
                return ret;
            }
            pst_lb->lbm_tx_count++;
            pst_lb->p_t_lb_timer = tpoam_util_add_timer_ms(_tpoam_lb_tx_lbm_cb, pst_oam,
                         (pst_lb->lbm_para.time_out * 1000));
        }
    }
    
    return TPOAM_E_NONE;
}

int32
tpoam_lb_rx_lbr(tbl_oam_session_t* pst_oam, tpoam_rx_pdu_t* pst_pdu)
{
    /* 5.2. OAM Loopback (MT-LBM/LBR) procedures */
    int32 ret = TPOAM_E_NONE;

    if (!pst_oam->oam_lb.active)
    {
        TPOAM_DBG_LB_LBR("Oam session %u MEG %s RX LBR when LB is inactive, ignore LBR", pst_oam->key.session_id, pst_oam->megid_str);
        return TPOAM_E_NONE;
    }
    
    if (TPOAM_Y1731_PDU_LBR != pst_pdu->hdr.opcode)
    {
        return TPOAM_E_PDU_INVALID_OPCODE;
    }

    ret = _tpoam_lb_decode_lbr(pst_oam, pst_pdu);
    if (ret < 0)
    {
        return ret;
    }

    TPOAM_DBG_LB_LBR("Oam session %u MEG %s RX %s LBR, TransID %d, exp %d, ttl %d, payload length %d", pst_oam->key.session_id, pst_oam->megid_str, 
        _tpoam_lb_sub_type_desc(pst_pdu->lb_sub_type), 
        pst_pdu->lb_trans_id, pst_pdu->label.exp, pst_pdu->label.ttl, pst_pdu->y1731_len);
    
    TPOAM_OP_ERR_RET(_tpoam_lb_handle_lbr(pst_oam, pst_pdu));

    tpoam_lb_call_test_cb(pst_oam, pst_pdu->lb_sub_type);
    
    return TPOAM_E_NONE;
}


int32
tpoam_lb_stop_lb(tbl_oam_session_t* pst_oam)
{
    /* modified by liwh for bug 20741, 2012-11-12 */
    if (pst_oam && pst_oam->oam_lb.active)
    {
        TPOAM_DBG_LB_LBM("Oam session %u MEG %s stop LBM", pst_oam->key.session_id, pst_oam->megid_str);
    }
    
    TPOAM_OP_ERR_RET(_tpoam_lb_deinit_lb_session(pst_oam));
    TPOAM_OP_ERR_RET(tpoam_api_exec_deinit(pst_oam));
    return TPOAM_E_NONE;
}

int32
tpoam_lb_start_lb_check(tbl_oam_session_t* pst_oam, tpoam_lbm_para_t* pst_lbm_para)
{
    uint32 status;
    tpoam_lb_t* pst_lb = &pst_oam->oam_lb;
    //int32 ret = TPOAM_E_NONE;

    TPOAM_DBG_LB_LBM("Oam session %u MEG %s start %s LBM, timeout %d, repeat %d, RMEP %d, ttl %d, ttl end %d", 
        pst_oam->key.session_id,
        pst_oam->megid_str, _tpoam_lb_sub_type_desc(pst_lbm_para->lb_sub_type), pst_lbm_para->time_out, 
        pst_lbm_para->repeat_cnt, pst_lbm_para->rmep_id, pst_lbm_para->ttl, pst_lbm_para->ttl_end);
    
    if (pst_lb->active)
    {
        return TPOAM_E_LB_IS_RUNNING;
    }

    if (pst_lb->p_t_lb_timer)
    {
        TPOAM_ASSERT;
    }

    /* 1. check has lmep? */
    if (!pst_oam->mepid)
    {
        return TPOAM_E_LMEP_NOT_EXIST;
    }

    /* 2. check service is lcked by server service */
    #ifdef support_lck
    uint32 lck_by_srv = FALSE;
    TPOAM_OP_ERR_RET(tpoam_api_get_lck_by_srv_service(pst_oam, NSM_TPOAM_DIR_MAX, &lck_by_srv));
    if (lck_by_srv)
    {
        return TPOAM_E_LCKED_BY_SRV_SERVICE;
    }
    #endif

    /* 3. check service is up */
    status = TPOAM_STATUS_DOWN;
    TPOAM_OP_ERR_RET(tpoam_api_service_get_status(pst_oam, TPOAM_SOCK_DIRECTION_PE, &status));

    if (TPOAM_STATUS_DOWN == status)
    {
        return TPOAM_E_SERVICE_DOWN;
    }

    return TPOAM_E_NONE;
}

int32
tpoam_lb_start_lb_send(tbl_oam_session_t* pst_oam, tpoam_lbm_para_t* pst_lbm_para)
{
    tpoam_lb_t* pst_lb = &pst_oam->oam_lb;
    int32 ret = TPOAM_E_NONE;
    
    TPOAM_OP_ERR_RET(_tpoam_lb_deinit_lb_session(pst_oam));
    TPOAM_OP_ERR_RET(_tpoam_lb_init_lb_session(pst_oam, pst_lbm_para));

    pst_lb->lbm_next_trans_id++;
    ret = _tpoam_lb_tx_lbm(pst_oam);
    if (ret < 0)
    {   
        _tpoam_lb_deinit_lb_session(pst_oam);
        return ret;
    }
    pst_oam->oam_lb.lbm_tx_count++;

    /*timeout is configured or default?*/
    pst_lb->p_t_lb_timer = tpoam_util_add_timer_ms(_tpoam_lb_tx_lbm_cb, pst_oam,
                         pst_lb->lbm_para.time_out * 1000);
    
    return TPOAM_E_NONE;
}

int32
tpoam_lb_start_lb(tbl_oam_session_t* pst_oam, tpoam_lbm_para_t* pst_lbm_para)
{
    uint32 status;
    tpoam_lb_t* pst_lb = &pst_oam->oam_lb;
    int32 ret = TPOAM_E_NONE;

    TPOAM_DBG_LB_LBM("Oam session %u MEG %s start %s LBM, timeout %d, repeat %d, RMEP %d, ttl %d, ttl end %d", 
        pst_oam->key.session_id,
        pst_oam->megid_str, _tpoam_lb_sub_type_desc(pst_lbm_para->lb_sub_type), pst_lbm_para->time_out, 
        pst_lbm_para->repeat_cnt, pst_lbm_para->rmep_id, pst_lbm_para->ttl, pst_lbm_para->ttl_end);
    
    if (pst_lb->active)
    {
        return TPOAM_E_LB_IS_RUNNING;
    }

    if (pst_lb->p_t_lb_timer)
    {
        TPOAM_ASSERT;
    }

    /* 1. check has lmep? */
    if (!pst_oam->mepid)
    {
        return TPOAM_E_LMEP_NOT_EXIST;
    }

    /* 2. check service is lcked by server service */
    #ifdef support_lck
    uint32 lck_by_srv = FALSE;
    TPOAM_OP_ERR_RET(tpoam_api_get_lck_by_srv_service(pst_oam, NSM_TPOAM_DIR_MAX, &lck_by_srv));
    if (lck_by_srv)
    {
        return TPOAM_E_LCKED_BY_SRV_SERVICE;
    }
    #endif

    /* 3. check service is up */
    status = TPOAM_STATUS_DOWN;
    TPOAM_OP_ERR_RET(tpoam_api_service_get_status(pst_oam, TPOAM_SOCK_DIRECTION_PE, &status));

    if (TPOAM_STATUS_DOWN == status)
    {
        return TPOAM_E_SERVICE_DOWN;
    }

    TPOAM_OP_ERR_RET(_tpoam_lb_deinit_lb_session(pst_oam));
    TPOAM_OP_ERR_RET(_tpoam_lb_init_lb_session(pst_oam, pst_lbm_para));

    pst_lb->lbm_next_trans_id++;
    ret = _tpoam_lb_tx_lbm(pst_oam);
    if (ret < 0)
    {   
        _tpoam_lb_deinit_lb_session(pst_oam);
        return ret;
    }
    pst_oam->oam_lb.lbm_tx_count++;

    /*timeout is configured or default?*/
    pst_lb->p_t_lb_timer = tpoam_util_add_timer_ms(_tpoam_lb_tx_lbm_cb, pst_oam,
                         pst_lb->lbm_para.time_out * 1000);

    return TPOAM_E_NONE;
}

int32
tpoam_lb_init(tbl_oam_session_t* pst_oam)
{
    tpoam_lb_t* pst_lb = &pst_oam->oam_lb;
    pst_lb->lbm_next_trans_id = TPOAM_Y1731_PDU_LBM_START_TRANS_ID;
    if (NULL == pst_lb->pst_lb_cache_list)
    {
        pst_lb->pst_lb_cache_list = list_new();
    }
    
    return TPOAM_E_NONE;
}

int32
tpoam_lb_deinit(tbl_oam_session_t* pst_oam)
{
    tpoam_lb_t* pst_lb = &pst_oam->oam_lb;
    tpoam_lb_reply_t* pst_entry = NULL;
    struct listnode* pst_next_node = NULL;
    struct listnode* pst_node = NULL;
    TPOAM_OP_ERR_RET(tpoam_lb_stop_lb(pst_oam));

    /* release other datas */
    if (pst_lb->pst_lb_cache_list)
    {
        LIST_LOOP_DEL(pst_lb->pst_lb_cache_list, pst_entry, pst_node, pst_next_node)
        {
            listnode_delete(pst_lb->pst_lb_cache_list, pst_entry);
            XFREE(MEM_PM_TPOAM_LB_REPLY_ENTRY, pst_entry);
        }
        list_free(pst_lb->pst_lb_cache_list);
    }
    pst_lb->pst_lb_cache_list = NULL;
    
    return TPOAM_E_NONE;
}

extern cfg_result_t g_lb_result_str;
void
tpoam_lb_clean(tbl_oam_session_t* pst_oam)
{
    tbl_tpoam_exec_global_t *p_tpoam_exec_global = tbl_tpoam_exec_global_get_tpoam_exec_global();
    char lb_result_str[MAX_ON_DEMAND_CMD_RET_LEN] = {0};
    int len = 0;
    cfg_result_t result;

    if (TPOAM_EXEC_TYPE_NONE != p_tpoam_exec_global->test_type && pst_oam->key.session_id == p_tpoam_exec_global->oam_session_id)
    {
        tpoam_util_cancel_timer(&p_tpoam_exec_global->p_t_send_lb_msg_timer);
        
        if (sal_strlen(g_lb_result_str.desc) != 0)
        {
            g_lb_result_str.rc = PM_E_NONE;
            ipc_pm_continue_send_to_ccs(p_tpoam_exec_global->sock, p_tpoam_exec_global->pid, &g_lb_result_str);
            sal_memset(&g_lb_result_str, 0x00, sizeof(g_lb_result_str));
        }
        else
        {
            sal_memset(&result, 0, sizeof(result));    
            
            /*output result summary*/
            sal_sprintf(lb_result_str + len,  "\n");
            len = sal_strlen(lb_result_str);

            sal_memset(&result, 0, sizeof(result));
            CFG_CONTINUE_RESULT(&result, PM_E_NONE, "%s", lb_result_str);
            ipc_pm_continue_send_to_ccs(p_tpoam_exec_global->sock, p_tpoam_exec_global->pid, &result);
        }
        p_tpoam_exec_global->test_type = TPOAM_EXEC_TYPE_NONE;
        p_tpoam_exec_global->oam_session_id = 0;
        
        tpoam_api_exec_deinit(pst_oam);
        tpoam_lb_deinit(pst_oam);
    }

    return;
}

