#include "proto.h"

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

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a,b)	((a) > (b) ? (b) : (a))

/* DM timestamp defines */
struct tpoam_timestamp_s
{
    uint32 sec;
    uint32 ns;
};
typedef struct tpoam_timestamp_s tpoam_timestamp_t;

static inline tpoam_interval_t
_tpoam_dm_sub_ts(tpoam_timestamp_t* pst_minuend, tpoam_timestamp_t* pst_sub)
{
    return ((((int64)pst_minuend->sec - (int64)pst_sub->sec) * TPOAM_NS_PER_SEC) + ((int64)pst_minuend->ns - (int64)pst_sub->ns));
}

void
_tpoam_dm_clean_list_dmm(tbl_oam_session_t* pst_oam)
{
    struct listnode* pst_next_node = NULL;
    struct listnode* pst_node = NULL;
    tpoam_dm_entry_t* pst_entry = NULL;
    
    if (pst_oam->oam_list.pst_dmm_cache_list)
    {
        LIST_LOOP_DEL(pst_oam->oam_list.pst_dmm_cache_list, pst_entry, pst_node, pst_next_node)
        {
            listnode_delete(pst_oam->oam_list.pst_dmm_cache_list, pst_entry);
            XFREE(MEM_PM_TPOAM_DM_ENTRY, pst_entry);
        }
    }

    return;
}

void
tpoam_dm_destroy_list_dmm(tbl_oam_session_t* pst_oam)
{
    _tpoam_dm_clean_list_dmm(pst_oam);
    
    list_free(pst_oam->oam_list.pst_dmm_cache_list);
    pst_oam->oam_list.pst_dmm_cache_list = NULL;

    return;
}

static int32
_tpoam_dm_init_dm_session(tbl_oam_session_t* pst_oam, tpoam_dm_para_t* pst_para)
{
    tpoam_dm_t* pst_dm = &pst_oam->oam_dm;

    _tpoam_dm_clean_list_dmm(pst_oam);
    
    sal_memcpy(&pst_dm->para, pst_para, sizeof(tpoam_dm_para_t));
    pst_dm->tx_count = 0;
    pst_dm->rx_count = 0;
    pst_dm->per_dm_status = TPOAM_PER_DMM_STATUS_WAITING;
    pst_oam->dm_active = TRUE;

    return TPOAM_E_NONE;
}


int32
tpoam_dm_deinit_dm_session(tbl_oam_session_t* pst_oam)
{
    if (!pst_oam->dm_active)
    {
        return TPOAM_E_NONE;
    }

    // TODO: timer_running
    //if (l2_is_timer_running (pst_oam->p_t_send_dm_timer))
    {
        tpoam_util_cancel_timer(&pst_oam->p_t_send_dm_timer);
    }

    pst_oam->dm_active = FALSE;
    return TPOAM_E_NONE;
}

static int32
_tpoam_dm_decode_dmm(tpoam_rx_pdu_t* pst_pdu)
{
    /* modified by yaom for 10G remote fault detection 20120926 */
    //tpoam_timestamp_t rx_ts_f;  /* RxTimeStampf */
    //uint64 rx_ts = 0;
    uint8** pnt = &pst_pdu->p_data;
    int32* size = &pst_pdu->left_len;

    /* convert timestamp */
    TLV_DECODE_SKIP(sizeof(uint64));
    
    #if 0
    TLV_DECODE_GETLL(rx_ts);
    rx_ts_f.sec = rx_ts / 1000000000ULL;
    rx_ts_f.ns = rx_ts % 1000000000ULL;
    
    TLV_DECODE_REWIND(sizeof(uint64));
    TLV_ENCODE_PUTL(rx_ts_f.sec);
    TLV_ENCODE_PUTL(rx_ts_f.ns);
    #endif
    
    return TPOAM_E_NONE;
}

uint32 glb_test_dmm_count = 0;
void
tpoam_dm_get_test_data(int64 *delay)
{
    glb_test_dmm_count++;
    if (glb_test_dmm_count >= 0 && glb_test_dmm_count <= 10)
    {
        return;
    }

    /*threshold is 200000, cmd is 200*/
    
    if (glb_test_dmm_count == 11)
    {
        *delay = 100000;
    }
    if (glb_test_dmm_count == 12)
    {
        *delay = 300000;
    }
    if (glb_test_dmm_count == 13)
    {
        *delay = 100000;
    }
    if (glb_test_dmm_count == 14)
    {
        *delay = 100000;
    }
    if (glb_test_dmm_count == 15)
    {
        *delay = 100000;
    }
    if (glb_test_dmm_count == 16)
    {
        *delay = 100000;
    }
    if (glb_test_dmm_count == 17)
    {
        *delay = 100000;
    }
    if (glb_test_dmm_count == 18)
    {
        *delay = 200000;
    }

    return;
}

static int32
_tpoam_dm_decode_dmr(tbl_oam_session_t* pst_oam, tpoam_rx_pdu_t* pst_pdu)
{
    tpoam_interval_t delay1 = 0;
    tpoam_interval_t delay2 = 0;
    tpoam_timestamp_t tx_ts_f;  /* TxTimeStampf */
    tpoam_timestamp_t rx_ts_f;  /* RxTimeStampf */
    tpoam_timestamp_t tx_ts_b;  /* TxTimeStampb */
    tpoam_timestamp_t rx_ts_b;  /* RxTimeStampb */

    /* Frame Delay = (RxTimeb ¨C TxTimeStampf) ¨C (TxTimeStampb ¨C RxTimeStampf) */
    uint8** pnt = &pst_pdu->p_data;
    int32* size = &pst_pdu->left_len;
    
    TLV_DECODE_GETL(tx_ts_f.sec);
    TLV_DECODE_GETL(tx_ts_f.ns);
    TLV_DECODE_GETL(rx_ts_f.sec);
    TLV_DECODE_GETL(rx_ts_f.ns);
    TLV_DECODE_GETL(tx_ts_b.sec);
    TLV_DECODE_GETL(tx_ts_b.ns);
    TLV_DECODE_GETL(rx_ts_b.sec);
    TLV_DECODE_GETL(rx_ts_b.ns);

    delay1 = _tpoam_dm_sub_ts(&rx_ts_b, &tx_ts_f);
    delay2 = _tpoam_dm_sub_ts(&tx_ts_b, &rx_ts_f);

    pst_pdu->delay = delay1 - delay2;
    
    //tpoam_dm_get_test_data(&pst_pdu->delay);

    TPOAM_DBG_DM_DMR("Oam session %u MEG %s RX DMR Txf (%d.%09d), Rxf (%d.%09d), Txb (%d.%09d), Rxb (%d.%09d) "
        "delay1 %"PRId64" delay2 %"PRId64" delay %"PRId64,
        pst_oam->key.session_id, pst_oam->megid_str, tx_ts_f.sec, tx_ts_f.ns, rx_ts_f.sec, rx_ts_f.ns, tx_ts_b.sec, tx_ts_b.ns, rx_ts_b.sec, rx_ts_b.ns,
        delay1, delay2, pst_pdu->delay);

    return TPOAM_E_NONE;
}

static int32
_tpoam_dm_encode_dmr(tbl_oam_session_t* pst_oam, tpoam_rx_pdu_t* pst_lbm, uint8* p_data, uint32 len)
{
    //int32   pkt_size_left = len;
    //int32*  size = NULL;
    //uint8** pnt = NULL;
    
    //size = &pkt_size_left;
    //pnt = &p_data;

    /* encode from Y.1731 header (after ACH header) */
    sal_memcpy(p_data, pst_lbm->p_y1731_head, pst_lbm->y1731_len);
    p_data[1] = TPOAM_Y1731_PDU_DMR;    /* change opcode to DMR */

    return TPOAM_E_NONE;
}

#ifdef support_1dm
static int32
_tpoam_dm_decode_1dm(tbl_oam_session_t* pst_oam, tpoam_rx_pdu_t* pst_pdu)
{
    tpoam_timestamp_t tx_ts_f;  /* TxTimeStampf */
    tpoam_timestamp_t rx_ts_f;  /* RxTimeStampf */
    uint8** pnt = &pst_pdu->p_data;
    int32* size = &pst_pdu->left_len;
    /* modified by yaom for 10G remote fault detection 20120926 */
    uint64 rx_ts = 0;
    

    TLV_DECODE_GETL(tx_ts_f.sec);
    TLV_DECODE_GETL(tx_ts_f.ns);

    #if 0
    TLV_DECODE_GETLL(rx_ts);

    rx_ts_f.sec = rx_ts / 1000000000ULL;
    rx_ts_f.ns = rx_ts % 1000000000ULL;
    #endif
    TLV_DECODE_GETL(rx_ts_f.sec);
    TLV_DECODE_GETL(rx_ts_f.ns);

    TPOAM_DBG_DM_1DMRX("Oam session %u MEG %s RX 1DM Txf (%d.%09d), Rxf (%d.%09d)", 
        pst_oam->key.session_id, pst_oam->megid_str, tx_ts_f.sec, tx_ts_f.ns, rx_ts_f.sec, rx_ts_f.ns);
    
    pst_pdu->delay = _tpoam_dm_sub_ts(&rx_ts_f, &tx_ts_f);
    return TPOAM_E_NONE;
}


static int32
_tpoam_dm_encode_1dm(tbl_oam_session_t* pst_oam, uint8* p_data, int32* p_length)
{
    uint8*  p_curr_data = p_data; 
    int32*  size = NULL;
    uint8** pnt = NULL;
    uint8   level_version = 0;
    
    size = p_length;
    pnt = &p_curr_data;

    /*
       0                   1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |0 0 0 1|0 0 0 0|0 0 0 0 0 0 0 0|  Y.1731 Channel Type (0xXXXX) |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      | MEL | Version |OpCode (1DM=45)|    Flags(0)   | TLV Offset(16)|
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                          TxTimeStampf                         |
      |                                                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |           Reserved for 1DM receiving equipment(0)             |
      |                        (for RxTimeStampf)                     |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |   End TLV (0) |
      +-+-+-+-+-+-+-+-+
    */

    level_version = (((pst_oam->level & TPOAM_Y1731_PDU_MEL_MASK) << TPOAM_Y1731_PDU_MEL_SHIFT)
                    | (TPOAM_Y1731_PDU_VER & TPOAM_Y1731_PDU_VER_MASK));
    
    TLV_ENCODE_PUTC(level_version);
    TLV_ENCODE_PUTC(TPOAM_Y1731_PDU_1DM);
    TLV_ENCODE_PUTC(0);
    TLV_ENCODE_PUTC(TPOAM_Y1731_1DM_FIRST_TLV_OFFSET);
    TLV_ENCODE_PUT_EMPTY(TPOAM_Y1731_1DM_FIRST_TLV_OFFSET);     /* will be updated by HW */
    TLV_ENCODE_PUTC(TPOAM_Y1731_PDU_END_TLV);
    return TPOAM_E_NONE;
}

static int32
_tpoam_dm_handle_1dm(tbl_oam_session_t* pst_oam, tpoam_rx_pdu_t* pst_pdu)
{
    tpoam_rx1dm_t* pst_1dm = &pst_oam->oam_dm.rx1dm;
    tpoam_dm_entry_t* pst_entry = NULL;
    tpoam_dm_entry_t* pst_oldest_entry = NULL;    
    struct listnode* pst_oldest_node = NULL;
    uint32 event_bmp = 0;
    
    pst_entry = XCALLOC(MEM_PM_TPOAM_1DM_ENTRY, sizeof(tpoam_dm_entry_t));
    if (!pst_entry)
    {
        return TPOAM_E_MEMORY_ALLOC_FAIL;
    }
    
    pst_entry->delay = pst_pdu->delay;
    pst_entry->exp = pst_pdu->label.exp;
    sal_time(&pst_entry->time);

    if (LISTCOUNT(pst_1dm->pst_rx1dm_cache_list) < pst_1dm->max_count)
    {
        listnode_add(pst_1dm->pst_rx1dm_cache_list, pst_entry);
    }
    else if (LISTCOUNT(pst_1dm->pst_rx1dm_cache_list) == pst_1dm->max_count)
    {
        GLB_SET_FLAG(event_bmp, GLB_TPOAM_EVT_1DM_CACHE_FULL);
        TPOAM_OP_ERR_NO_RET(tpoam_event_set_oam_status(pst_oam, event_bmp, GLB_TPOAM_EVT_1DM_CACHE_FULL, NULL));
        
        /* delete oldest entry */
        pst_oldest_node = LISTHEAD(pst_1dm->pst_rx1dm_cache_list);
        pst_oldest_entry = GETDATA(pst_oldest_node);
        XFREE(MEM_PM_TPOAM_1DM_ENTRY, pst_oldest_entry);
        list_delete_node(pst_1dm->pst_rx1dm_cache_list, pst_oldest_node);

        /* add new entry */
        listnode_add(pst_1dm->pst_rx1dm_cache_list, pst_entry);
    }
    else
    {
        TPOAM_ASSERT;
    }
    return TPOAM_E_NONE;
}


int32
tpoam_dm_rx_1dm(tbl_oam_session_t* pst_oam, tpoam_rx_pdu_t* pst_pdu)
{
    int32 ret = TPOAM_E_NONE;
    
    if (TPOAM_Y1731_PDU_1DM != pst_pdu->hdr.opcode)
    {
        return TPOAM_E_PDU_INVALID_OPCODE;
    }

    ret = _tpoam_dm_decode_1dm(pst_oam, pst_pdu);
    if (ret < 0)
    {
        return ret;
    }    
    TPOAM_OP_ERR_RET(_tpoam_dm_handle_1dm(pst_oam, pst_pdu));
    return TPOAM_E_NONE;
}

void
_tpoam_dm_send_1dm_cb(void *p_data)
{
    tbl_oam_session_t* pst_oam = p_data ? (tbl_oam_session_t*)p_data : NULL;
    tpoam_dm_t* pst_dm = &pst_oam->oam_dm;
    int32 tics = 0;
    int32 ret = TPOAM_E_NONE;

    pst_oam->p_t_send_dm_timer = NULL;

    ret =_tpoam_dm_tx(pst_oam);
    if (ret < 0)
    {
        tpoam_dm_deinit_dm_session(pst_oam);
        return;
    }    
    pst_dm->tx_count++;

    TPOAM_DBG_DM_1DMTX("Oam session %u MEG %s TX 1DM, current count %d, total count %d", 
        pst_oam->key.session_id, pst_oam->megid_str, pst_dm->tx_count, pst_dm->para.repeat_cnt);
    if (pst_dm->tx_count >= pst_dm->para.repeat_cnt)
    {
        tpoam_dm_deinit_dm_session(pst_oam);
        return;
    }
    ret = tpoam_api_dm_interval_to_ms(pst_dm->para.interval, &tics);
    if (ret < 0)
    {
        TPOAM_ASSERT;
    }
    pst_oam->p_t_send_dm_timer = tpoam_util_add_timer_ms(_tpoam_dm_send_1dm_cb, pst_oam, tics);   
    return;
}


int32
tpoam_dm_clear_1dm(FILE *fp, tbl_oam_session_t* pst_oam, tpoam_lm_dual_enable_input_t* bb)
{
    struct listnode* pst_next_node = NULL;
    struct listnode* pst_node = NULL;
    tpoam_dm_entry_t* pst_entry = NULL;
    tpoam_rx1dm_t* pst_1dm = &pst_oam->oam_dm.rx1dm;
    uint32 event_bmp = 0;

    LIST_LOOP_DEL(pst_1dm->pst_rx1dm_cache_list, pst_entry, pst_node, pst_next_node)
    {
        listnode_delete(pst_1dm->pst_rx1dm_cache_list, pst_entry);
        XFREE(MEM_PM_TPOAM_1DM_ENTRY, pst_entry);
    }
    
    GLB_UNSET_FLAG(event_bmp, GLB_TPOAM_EVT_1DM_CACHE_FULL);
    TPOAM_OP_ERR_NO_RET(tpoam_event_set_oam_status(pst_oam, event_bmp, GLB_TPOAM_EVT_1DM_CACHE_FULL, NULL));
    return TPOAM_E_NONE;    
}

static int32
_tpoam_dm_deinit_1dm(tbl_oam_session_t* pst_oam)
{
    struct listnode* pst_next_node = NULL;
    struct listnode* pst_node = NULL;
    tpoam_dm_entry_t* pst_entry = NULL;
    tpoam_rx1dm_t* pst_1dm = &pst_oam->oam_dm.rx1dm;

    /* deinit 1DM */
    if (pst_1dm->pst_rx1dm_cache_list)
    {
        LIST_LOOP_DEL(pst_1dm->pst_rx1dm_cache_list, pst_entry, pst_node, pst_next_node)
        {
            listnode_delete(pst_1dm->pst_rx1dm_cache_list, pst_entry);
            XFREE(MEM_PM_TPOAM_1DM_ENTRY, pst_entry);
        }
        list_free(pst_1dm->pst_rx1dm_cache_list);
    }
    pst_1dm->pst_rx1dm_cache_list = NULL;
    
    return TPOAM_E_NONE;
}
#endif

static int32
_tpoam_dm_encode_dmm(tbl_oam_session_t* pst_oam, uint8* p_data, int32* p_length)
{
    uint8*  p_curr_data = p_data; 
    int32*  size = NULL;
    uint8** pnt = NULL;
    uint8   level_version = 0;
    
    size = p_length;
    pnt = &p_curr_data;

    /*
       0                   1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |0 0 0 1|0 0 0 0|0 0 0 0 0 0 0 0|  Y.1731 Channel Type (0xXXXX) |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      | MEL | Version |OpCode (DMM=47)|    Flags(0)   | TLV Offset(32)|
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                         TxTimeStampf                          |
      |                                                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |           Reserved for DMM receiving equipment(0)             |
      |                         (for RxTimeStampf)                    |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                         Reserved for DMR(0)                   |
      |                         (for TxTimeStampb)                    |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |           Reserved for DMR receiving equipment(0)             |
      |                                                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |   End TLV (0) |
      +-+-+-+-+-+-+-+-+
    */

    level_version = (((pst_oam->level & TPOAM_Y1731_PDU_MEL_MASK) << TPOAM_Y1731_PDU_MEL_SHIFT)
                    | (TPOAM_Y1731_PDU_VER & TPOAM_Y1731_PDU_VER_MASK));
    
    TLV_ENCODE_PUTC(level_version);
    TLV_ENCODE_PUTC(TPOAM_Y1731_PDU_DMM);
    TLV_ENCODE_PUTC(0);
    TLV_ENCODE_PUTC(TPOAM_Y1731_DMM_DMR_FIRST_TLV_OFFSET);
    TLV_ENCODE_PUT_EMPTY(TPOAM_Y1731_DMM_DMR_FIRST_TLV_OFFSET);     /* will be updated by HW */
    TLV_ENCODE_PUTC(TPOAM_Y1731_PDU_END_TLV);
    return TPOAM_E_NONE;
}


uint32
tpoam_get_delay_threshold(tbl_oam_session_t* pst_oam)
{
    tbl_tpoam_global_t *p_db_tpoam_glb = tbl_tpoam_global_get_tpoam_global();
    
    if (pst_oam->oam_dm.delay_threshold)
    {
        return pst_oam->oam_dm.delay_threshold;
    }
    else
    {
        return p_db_tpoam_glb->dm_delay_threshold;
    }
}

void
_tpoam_dm_caculate_average_delay(tbl_oam_session_t* pst_oam, tpoam_interval_t *p_avg_delay, tpoam_interval_t *p_avg_variation)
{
    struct list* pst_cache_list = NULL;
    struct listnode* pst_node = NULL;
    tpoam_dm_entry_t* pst_entry = NULL;
    uint32 index = 0;
    tpoam_interval_t avg_variation = 0;
    tpoam_interval_t avg_delay = 0;
    tpoam_interval_t max_delay = 0;
    tpoam_interval_t min_delay = 0;
    tpoam_interval_t variation = 0;
    tpoam_interval_t local_delay = 0;
    tpoam_interval_t avg_delay_tmp = 0;
    tpoam_interval_t avg_variation_tmp = 0;

    uint32 delay_threshold = 0;

    if (NULL == pst_oam->oam_list.pst_dmm_cache_list)
    {
        TPOAM_LOG_ERR("Oam session %u, dual dm cache list is NULL", pst_oam->key.session_id);
        return;
    }
    
    pst_cache_list = pst_oam->oam_list.pst_dmm_cache_list;

    if (0 == LISTCOUNT(pst_cache_list))
    {
        TPOAM_DBG_DM_COMM("Oam session %u, dual dm cache list count is 0", pst_oam->key.session_id);
        return;
    }
    
    LIST_LOOP(pst_cache_list, pst_entry, pst_node)
    {
        index++;
        if (1 == index)
        {
            variation = 0;
            min_delay = pst_entry->delay;
            max_delay = pst_entry->delay;
        }
        else
        {
            variation = MAX(pst_entry->delay, local_delay) - MIN(pst_entry->delay, local_delay);
        }
        local_delay = pst_entry->delay;
        avg_delay += pst_entry->delay;
        avg_variation += variation;
        if (pst_entry->delay > max_delay)
        {
            max_delay = pst_entry->delay;
        }
        if (pst_entry->delay < min_delay)
        {
            min_delay = pst_entry->delay;
        }
    }

    avg_delay = avg_delay/index;
    avg_variation = (index > 1) ? avg_variation/(index - 1) : 0;
    
    TPOAM_DBG_DM_COMM("Oam session %u, average delay %"PRIu64, pst_oam->key.session_id, avg_delay);
    
    delay_threshold = tpoam_get_delay_threshold(pst_oam);

    /*ns to us*/
    #ifdef dm_ns_to_us
        avg_delay_tmp = avg_delay/1000;
        avg_variation_tmp = avg_variation/1000;
    #else
        avg_delay_tmp = avg_delay;
        avg_variation_tmp = avg_variation;
    #endif

    /*if delay threshold is not configured, not print log*/
    if (0 != delay_threshold && avg_delay_tmp > delay_threshold) 
    {
        TPOAM_LOG_EMERG("Oam session %u, average delay %"PRIu64" exceed configured dm threshold %u!", 
            pst_oam->key.session_id, avg_delay_tmp, delay_threshold);
    }

    *p_avg_delay = avg_delay_tmp;
    *p_avg_variation= avg_variation_tmp;
    
    return;
}

int32 
tpoam_dm_check_delay(tpoam_interval_t delay)
{
    if (delay >= GLB_MAX_UINT32_VALUE)
    {
        return TPOAM_E_FAIL;
    }
    else
    {
        return TPOAM_E_NONE;
    }
}

int32
_tpoam_dm_calculate_for_timer_upload(tbl_oam_session_t* pst_oam)
{
    tbl_tpoam_global_t *p_db_tpoam_glb = tbl_tpoam_global_get_tpoam_global();
    tpoam_interval_t avg_variation = 0;
    tpoam_interval_t avg_delay = 0;

    /*default dm_calc_interval is 10*/
    if (pst_oam->oam_dm.packet_cnt_for_upload >= p_db_tpoam_glb->dm_calc_interval)
    {
        pst_oam->oam_dm.packet_cnt_for_upload = 0;
        _tpoam_dm_caculate_average_delay(pst_oam, &avg_delay, &avg_variation);

        if (TPOAM_E_NONE != tpoam_dm_check_delay(avg_delay))
        {
            TPOAM_DBG_DM_COMM("Upload dm on timer failed for delay is bigger than 0xFFFFFFFF, oam session %u, delay %"PRId64"\n", 
                pst_oam->key.session_id, avg_delay);
            return TPOAM_E_FAIL;
        }
        
        /*add to db*/
        tbl_oam_session_upload_t * pst_oam_upload;
        pst_oam_upload = tbl_oam_session_upload_get_oam_session_upload((tbl_oam_session_upload_key_t *)&(pst_oam->key));
        if (pst_oam_upload)
        {
            pst_oam_upload->upload_dm.index++;
            pst_oam_upload->upload_dm.delay = avg_delay;
            pst_oam_upload->upload_dm.variation = avg_variation;
            
            /*sync msg maybe many*/
            tbl_oam_session_upload_set_oam_session_upload_field(pst_oam_upload, TBL_OAM_SESSION_UPLOAD_FLD_UPLOAD_DM);
            TPOAM_DBG_DM_COMM("Upload dm on timer, oam session %u, index %u, delay %u\n", 
                pst_oam->key.session_id, pst_oam_upload->upload_dm.index, pst_oam_upload->upload_dm.delay);
        }
        else
        {
            /*clean list*/
            _tpoam_dm_clean_list_dmm(pst_oam);
            TPOAM_LOG_ERR("Upload dm on timer, but not find the tbl %u, delay %"PRId64", error\n", 
                pst_oam->key.session_id, avg_delay);
            return TPOAM_E_FAIL;
        }

        /*clean list*/
        _tpoam_dm_clean_list_dmm(pst_oam);
    }
    else
    {
        TPOAM_DBG_DM_COMM("Oam session %u current packet count for upload is %u, global dm_calc_interval is %u", 
            pst_oam->key.session_id, pst_oam->oam_dm.packet_cnt_for_upload, p_db_tpoam_glb->dm_calc_interval);
    }

    return TPOAM_E_NONE;
}

/*added by yejl for bug 52508, 2019-06-24*/
int32
_tpoam_dm_event_clear(tbl_oam_session_t* pst_oam, tbl_oam_session_upload_t * pst_oam_upload)
{
    tpoam_dm_entry_t* pst_entry_tmp = NULL;    
    struct listnode* pst_node_tmp = NULL;
    
    tpoam_dm_t* pst_dm = &pst_oam->oam_dm;
    tpoam_list_t *oam_list = &pst_oam->oam_list;
    if (NULL == pst_dm || NULL == oam_list)
    {
        return TPOAM_E_NONE;
    }
    if (NULL == oam_list->pst_dmm_cache_list)
    {
        return TPOAM_E_NONE;
    }
    pst_node_tmp = LISTTAIL(oam_list->pst_dmm_cache_list);
    pst_entry_tmp = GETDATA(pst_node_tmp);
    
    pst_dm->is_event_occur = FALSE;
    pst_oam_upload->upload_dm_event.bit_flag = GLB_TPOAM_UPLOAD_EVENT_CLEAR;

    if (pst_entry_tmp)
    {
        pst_oam_upload->upload_dm_event.delay = pst_entry_tmp->delay;
        if (TPOAM_E_NONE != tpoam_dm_check_delay(pst_entry_tmp->delay))
        {
            pst_oam_upload->upload_dm_event.delay = 0;
        }
    }

    logid_sys(LOG_OAM_4_TPOAM_DM_EVENT, tpoam_api_event_action_desc(pst_oam_upload->upload_dm_event.bit_flag),
        pst_oam->key.session_id, 
        pst_oam_upload->upload_dm_event.delay,
        pst_oam_upload->upload_dm_event.variation);
    
    /*sync msg maybe many*/
    tbl_oam_session_upload_set_oam_session_upload_field(pst_oam_upload, TBL_OAM_SESSION_UPLOAD_FLD_UPLOAD_DM_EVENT);
    TPOAM_DBG_DM_COMM("Upload dm event clear, oam session %u, index %u, bit flag %u, delay %u\n", 
    pst_oam->key.session_id, pst_oam_upload->upload_dm_event.index, pst_oam_upload->upload_dm_event.bit_flag, pst_oam_upload->upload_dm_event.delay);

    return TPOAM_E_NONE;
}
/*ended by yejl*/

int32
_tpoam_dm_calculate_for_event_upload(tbl_oam_session_t* pst_oam, int64 delay)
{
    uint32 delay_threshold = 0;
    uint32 delay_tmp = 0;
    tbl_oam_session_upload_t * pst_oam_upload;
    tpoam_dm_t* pst_dm = &pst_oam->oam_dm;
    
    delay_threshold = tpoam_get_delay_threshold(pst_oam);
    if (0 == delay_threshold)
    {
        TPOAM_DBG_DM_COMM("Upload dm event, oam session %u, delay threshold had not configured\n", 
                pst_oam->key.session_id);
        return TPOAM_E_NONE;
    }

    /*ns to us*/
    #ifdef dm_ns_to_us
        delay_tmp = delay/1000;
    #else
        delay_tmp = delay;
    #endif

    TPOAM_DBG_DM_COMM("Oam session %u, delay %u, delay threshold %u", pst_oam->key.session_id, delay_tmp, delay_threshold);
    if (delay_tmp >= delay_threshold)
    {
        pst_dm->normal_packet_cnt_continue = 0;
        
        /*suppress continue event*/
        if (pst_dm->is_event_occur)
        {
            TPOAM_DBG_DM_COMM("Oam session %u, suppress dm event", pst_oam->key.session_id);
            return TPOAM_E_NONE;
        }
        
        pst_dm->is_event_occur = TRUE;

        /*add to db*/
        pst_oam_upload = tbl_oam_session_upload_get_oam_session_upload((tbl_oam_session_upload_key_t *)&(pst_oam->key));
        if (pst_oam_upload)
        {
            //pst_oam_upload->upload_dm_event.index++;
            pst_oam_upload->upload_dm_event.bit_flag = GLB_TPOAM_UPLOAD_EVENT_SET;
            pst_oam_upload->upload_dm_event.delay = delay_tmp;

            /*if delay is big than 0xffffffff, set delay to 0 for upload*/
            if (TPOAM_E_NONE != tpoam_dm_check_delay(delay_tmp))
            {
                pst_oam_upload->upload_dm_event.delay = 0;
            }
            
            logid_sys(LOG_OAM_4_TPOAM_DM_EVENT, tpoam_api_event_action_desc(pst_oam_upload->upload_dm_event.bit_flag),
                pst_oam->key.session_id, 
                pst_oam_upload->upload_dm_event.delay,
                pst_oam_upload->upload_dm_event.variation);
        
            /*sync msg maybe many*/
            tbl_oam_session_upload_set_oam_session_upload_field(pst_oam_upload, TBL_OAM_SESSION_UPLOAD_FLD_UPLOAD_DM_EVENT);
            TPOAM_DBG_DM_COMM("Upload dm event set, oam session %u, bit flag %u, delay %u\n", 
                pst_oam->key.session_id, pst_oam_upload->upload_dm_event.bit_flag, pst_oam_upload->upload_dm_event.delay);
        }
        else
        {
            TPOAM_LOG_ERR("Upload dm event set, but not find the tbl %u, error\n", pst_oam->key.session_id);
            return TPOAM_E_FAIL;
        }
    }
    else
    {
        pst_dm->normal_packet_cnt_continue++;
        if (pst_dm->is_event_occur && 3 <= pst_dm->normal_packet_cnt_continue)
        {
            /*event clear*/
            pst_dm->is_event_occur = FALSE;
            pst_oam_upload = tbl_oam_session_upload_get_oam_session_upload((tbl_oam_session_upload_key_t *)&(pst_oam->key));
            if (pst_oam_upload)
            {
                //pst_oam_upload->upload_dm_event.index++;
                pst_oam_upload->upload_dm_event.bit_flag = GLB_TPOAM_UPLOAD_EVENT_CLEAR;
                pst_oam_upload->upload_dm_event.delay = delay_tmp;

                /*if delay is big than 0xffffffff, set delay to 0 for upload*/
                if (TPOAM_E_NONE != tpoam_dm_check_delay(delay_tmp))
                {
                    pst_oam_upload->upload_dm_event.delay = 0;
                }

                logid_sys(LOG_OAM_4_TPOAM_DM_EVENT, tpoam_api_event_action_desc(pst_oam_upload->upload_dm_event.bit_flag),
                    pst_oam->key.session_id, 
                    pst_oam_upload->upload_dm_event.delay,
                    pst_oam_upload->upload_dm_event.variation);
                
                /*sync msg maybe many*/
                tbl_oam_session_upload_set_oam_session_upload_field(pst_oam_upload, TBL_OAM_SESSION_UPLOAD_FLD_UPLOAD_DM_EVENT);
                TPOAM_DBG_DM_COMM("Upload dm event clear, oam session %u, index %u, bit flag %u, delay %u\n", 
                pst_oam->key.session_id, pst_oam_upload->upload_dm_event.index, pst_oam_upload->upload_dm_event.bit_flag, pst_oam_upload->upload_dm_event.delay);
            }
            else
            {
                TPOAM_LOG_ERR("Upload dm event clear, but not find the tbl %u, error\n", pst_oam->key.session_id);
                return TPOAM_E_FAIL;
            }
        }
        else
        {
            TPOAM_DBG_DM_COMM("Is event occur %u, normal packet count continue %u", pst_dm->is_event_occur, pst_dm->normal_packet_cnt_continue);
        }
    }

    return TPOAM_E_NONE;
}

static int32
_tpoam_dm_handle_dmr(tbl_oam_session_t* pst_oam, tpoam_rx_pdu_t* pst_pdu)
{
    tpoam_dm_t* pst_dm = &pst_oam->oam_dm;
    tpoam_list_t         *oam_list = &pst_oam->oam_list;
    tpoam_dm_entry_t* pst_entry = NULL;
    tpoam_dm_entry_t* pst_oldest_entry = NULL;    
    struct listnode* pst_oldest_node = NULL;

    tpoam_dm_entry_t* pst_entry_tmp = NULL;    
    struct listnode* pst_node_tmp = NULL;
    
    pst_dm->rx_count++;
    pst_oam->oam_dm.packet_cnt_for_upload++;
    pst_dm->per_dm_status = TPOAM_PER_DMM_STATUS_RX_DMR;
    
    pst_entry = XCALLOC(MEM_PM_TPOAM_DM_ENTRY, sizeof(tpoam_dm_entry_t));
    if (!pst_entry)
    {
        TPOAM_LOG_ERR("Oam session %u, alloc dm entry for dual dm failed", pst_oam->key.session_id);
        return TPOAM_E_MEMORY_ALLOC_FAIL;
    }
    
    pst_entry->delay = pst_pdu->delay;
    pst_entry->exp = pst_pdu->label.exp;
    sal_time(&pst_entry->time);

    if (NULL == oam_list->pst_dmm_cache_list)
    {
        oam_list->pst_dmm_cache_list = list_new();
    }

    /*calc variation*/
    if (0 == LISTCOUNT(oam_list->pst_dmm_cache_list))
    {
        pst_entry->variation = 0;
    }
    else
    {
        pst_node_tmp = LISTTAIL(oam_list->pst_dmm_cache_list);
        pst_entry_tmp = GETDATA(pst_node_tmp);
        pst_entry->variation = MAX(pst_entry->delay, pst_entry_tmp->delay) - MIN(pst_entry->delay, pst_entry_tmp->delay);
    }

    TPOAM_DBG_DM_COMM("Add new dm entry %u/%u: session id %u, delay %"PRIu64" variation %"PRIu64" exp %hhu\n",
        LISTCOUNT(oam_list->pst_dmm_cache_list) + 1, GLB_TPOAM_DM_CACHE_SIZE, 
        pst_oam->key.session_id, pst_entry->delay, pst_entry->variation, pst_entry->exp);
        
    if (LISTCOUNT(oam_list->pst_dmm_cache_list) < GLB_TPOAM_DM_CACHE_SIZE)
    {
        listnode_add(oam_list->pst_dmm_cache_list, pst_entry);
        //if (pst_dm->rx_count >= pst_dm->para.repeat_cnt)
        //{
        //    tpoam_dm_deinit_dm_session(pst_oam);
        //    return TPOAM_E_NONE;
        //}
    }
    else if (LISTCOUNT(oam_list->pst_dmm_cache_list) == GLB_TPOAM_DM_CACHE_SIZE)
    {
        TPOAM_DBG_DM_COMM("Dm list count %u, is more than cache size %u, delete the oldest node\n", 
            LISTCOUNT(oam_list->pst_dmm_cache_list), GLB_TPOAM_DM_CACHE_SIZE);
            
        /* delete oldest entry */
        pst_oldest_node = LISTHEAD(oam_list->pst_dmm_cache_list);
        pst_oldest_entry = GETDATA(pst_oldest_node);
        XFREE(MEM_PM_TPOAM_DM_ENTRY, pst_oldest_entry);
        list_delete_node(oam_list->pst_dmm_cache_list, pst_oldest_node);
        
        /* add new entry */
        listnode_add(oam_list->pst_dmm_cache_list, pst_entry);
    }
    else
    {
        TPOAM_ASSERT;
    }

    /*for on timer upload*/
    _tpoam_dm_calculate_for_timer_upload(pst_oam);

    /*for on event upload*/
    _tpoam_dm_calculate_for_event_upload(pst_oam, pst_pdu->delay);

    return TPOAM_E_NONE;
}

static int32
_tpoam_dm_tx_new(tbl_oam_session_t* pst_oam)
{
    tpoam_sk_buff_t *tx_skb;
    int32 rc = 0;

    tx_skb = tpoam_alloc_skb(TPOAM_PKT_BUF_SIZE);
    if (!tx_skb)
    {
        TPOAM_LOG_ERR("Cannot allocate memory for issue message, session id %u", pst_oam->key.session_id);
        goto TPOAM_FINALLY;
    }
    
    tbl_tpoam_global_t *p_db_tpoam_glb = tbl_tpoam_global_get_tpoam_global();
    tpoam_dm_t* pst_dm = &pst_oam->oam_dm;
    uint8 *dm_data = tx_skb->data;
    int32 dm_data_left = TPOAM_PKT_BUF_SIZE;
    uint32 pkt_size = 0;
    uint8 opcode = 0;
    
    if (TPOAM_DM_TYPE_1DM == pst_dm->para.type)
    {
        //TPOAM_OP_ERR_RET(_tpoam_dm_encode_1dm(pst_oam, dm_data, &dm_data_left));
    }
    else if (TPOAM_DM_TYPE_DMM == pst_dm->para.type)
    {
        rc = _tpoam_dm_encode_dmm(pst_oam, dm_data, &dm_data_left);
        if (0 != rc)
        {
            goto TPOAM_FINALLY;
        }
    }
    else
    {
        TPOAM_LOG_ERR("Oam session %u, invalid dm type %u", pst_oam->key.session_id, pst_dm->para.type);
        goto TPOAM_FINALLY;
    }

    pkt_size = TPOAM_PKT_BUF_SIZE - dm_data_left;
    tx_skb->len = pkt_size;

    rc = tpoam_sock_tx_new(pst_oam, TPOAM_SOCK_DIRECTION_PE, pkt_size, tx_skb, 
        pst_dm->para.exp, TPOAM_SOCK_TTL_DEF, TPOAM_SOCK_DM_FLAG_SET, FALSE, FALSE, TPOAM_Y1731_PDU_DMM);
    if (rc < 0)
    {
        TPOAM_DBG_DM_DMM("Send packet failed %s on oam session %u\n", tpoam_err_desc(rc), pst_oam->key.session_id);
        tpoam_free_skb(&tx_skb);
        return TPOAM_E_FAIL;
    }
    
    opcode = (TPOAM_DM_TYPE_1DM == pst_dm->para.type) ? TPOAM_Y1731_PDU_1DM : TPOAM_Y1731_PDU_DMM;
    tpoam_api_stats_add_tx(&pst_oam->oam_stats.tx, opcode);
    tpoam_api_stats_add_tx(&p_db_tpoam_glb->oam_glb_stats.tx, opcode);        

TPOAM_FINALLY:

    tpoam_free_skb(&tx_skb);
    
    return TPOAM_E_NONE;
}

int32
tpoam_dm_rx_dmr(tbl_oam_session_t* pst_oam, tpoam_rx_pdu_t* pst_pdu)
{
    //tpoam_dm_t* pst_dm = &pst_oam->oam_dm;
    int32 ret = TPOAM_E_NONE;

    if (!pst_oam->dm_active)
    {
        TPOAM_DBG_DM_DMR("Oam session %u MEG %s receives DMR when DMM inactive, ignore DMR", pst_oam->key.session_id, pst_oam->megid_str);
        return TPOAM_E_NONE;
    }
    
    if (TPOAM_Y1731_PDU_DMR != pst_pdu->hdr.opcode)
    {
        return TPOAM_E_PDU_INVALID_OPCODE;
    }

    ret = _tpoam_dm_decode_dmr(pst_oam, pst_pdu);
    if (ret < 0)
    {
        return ret;
    }
    TPOAM_OP_ERR_RET(_tpoam_dm_handle_dmr(pst_oam, pst_pdu));
    return TPOAM_E_NONE;
}

static int32
_tpoam_dm_tx_dmr_new(tbl_oam_session_t* pst_oam, tpoam_rx_pdu_t* pst_pdu)
{
    tpoam_sk_buff_t *tx_skb;

    tx_skb = tpoam_alloc_skb(TPOAM_PKT_BUF_SIZE);
    if (!tx_skb)
    {
        TPOAM_LOG_ERR("Cannot allocate memory for send packet");
        goto TPOAM_FINALLY;
    }
    
    uint8 *dm_data = tx_skb->data;
    int32 ret = TPOAM_E_NONE;
    
    ret = _tpoam_dm_encode_dmr(pst_oam, pst_pdu, dm_data, TPOAM_PKT_BUF_SIZE);
    if (ret < 0)
    {
        goto TPOAM_FINALLY;
    }

    tx_skb->len = pst_pdu->y1731_len;
    
    ret = tpoam_sock_tx_new(pst_oam, TPOAM_SOCK_DIRECTION_PE, pst_pdu->y1731_len, tx_skb, 
        pst_pdu->label.exp, TPOAM_SOCK_TTL_DEF, TPOAM_SOCK_DM_FLAG_SET, TRUE, FALSE, TPOAM_Y1731_PDU_DMR);   
    if (ret < 0)
    {
        TPOAM_LOG_ERR("Send packet failed %s\n", tpoam_err_desc(ret));
        tpoam_free_skb(&tx_skb);
        return TPOAM_E_FAIL;
    }

    tbl_tpoam_global_t *p_db_tpoam_glb = tbl_tpoam_global_get_tpoam_global();
    tpoam_api_stats_add_tx(&pst_oam->oam_stats.tx, TPOAM_Y1731_PDU_DMR);
    tpoam_api_stats_add_tx(&p_db_tpoam_glb->oam_glb_stats.tx, TPOAM_Y1731_PDU_DMR);        

TPOAM_FINALLY:

    tpoam_free_skb(&tx_skb);
    
    return TPOAM_E_NONE;
}

int32
tpoam_dm_rx_dmm(tbl_oam_session_t* pst_oam, tpoam_rx_pdu_t* pst_pdu)
{
    int32 ret = TPOAM_E_NONE;
    
    if (TPOAM_Y1731_PDU_DMM != pst_pdu->hdr.opcode)
    {
        return TPOAM_E_PDU_INVALID_OPCODE;
    }

    ret = _tpoam_dm_decode_dmm(pst_pdu);
    if (ret < 0)
    {
        return ret;
    }
    TPOAM_OP_ERR_RET(_tpoam_dm_tx_dmr_new(pst_oam, pst_pdu));
    return TPOAM_E_NONE;
}

void
_tpoam_dm_send_dmm_cb(void *p_data)
{
    tbl_tpoam_global_t *p_db_tpoam_glb = tbl_tpoam_global_get_tpoam_global();
    tbl_oam_session_t* pst_oam = p_data ? (tbl_oam_session_t*)p_data : NULL;
    tpoam_dm_t* pst_dm = &pst_oam->oam_dm;
    int32 tics = 0;
    int32 ret = TPOAM_E_NONE;

    pst_oam->p_t_send_dm_timer = NULL;

    if (p_db_tpoam_glb->dm_enable_global)
    {
        ret = _tpoam_dm_tx_new(pst_oam);
        if (ret < 0)
        {
            TPOAM_DBG_DM_DMM("Oam session %u MEG %s TX DMM, send failed %d", 
                pst_oam->key.session_id, pst_oam->megid_str, ret);
        }
        else
        {
            pst_dm->tx_count++;
            TPOAM_DBG_DM_DMM("Oam session %u MEG %s TX DMM", 
                pst_oam->key.session_id, pst_oam->megid_str);
        }
    }

    ret = tpoam_api_dm_interval_to_ms(pst_dm->para.interval, &tics);
    if (ret < 0)
    {
        TPOAM_ASSERT;
    }
 
    pst_oam->p_t_send_dm_timer = tpoam_util_add_timer_ms(_tpoam_dm_send_dmm_cb, pst_oam, tics);   
    return;
}

void
_tpoam_dm_send_dmm_cb_on_demand(void *p_data)
{
    tbl_oam_session_t* pst_oam = p_data ? (tbl_oam_session_t*)p_data : NULL;
    tpoam_dm_t* pst_dm = &pst_oam->oam_dm;
    int32 tics = 0;
    int32 ret = TPOAM_E_NONE;

    pst_oam->p_t_send_dm_timer = NULL;

    if (pst_dm->tx_count >= pst_dm->para.repeat_cnt)
    {
        tpoam_dm_deinit_dm_session(pst_oam);
        return;
    }

    ret = _tpoam_dm_tx_new(pst_oam);
    if (ret < 0)
    {
        tpoam_dm_deinit_dm_session(pst_oam);
        return;
    }
    pst_dm->tx_count++;

    TPOAM_DBG_DM_DMM("Oam session %u MEG %s TX DMM, current count %d, total count %d", 
        pst_oam->key.session_id, pst_oam->megid_str, pst_dm->tx_count, pst_dm->para.repeat_cnt);
    if (pst_dm->tx_count >= pst_dm->para.repeat_cnt)
    {
        /* wait 2s for last DMR */
        tics = (TPOAM_DM_LAST_DMR_WAIT_TIME * 1000);
    }
    else
    {
        ret = tpoam_api_dm_interval_to_ms(pst_dm->para.interval, &tics);
        if (ret < 0)
        {
            TPOAM_ASSERT;
        }
    }
    pst_oam->p_t_send_dm_timer = tpoam_util_add_timer_ms(_tpoam_dm_send_dmm_cb_on_demand, pst_oam, tics);   
    return;
}

void
tpoam_dm_para_init(tpoam_dm_para_t* pst_para)
{
    if (NULL != pst_para)
    {
        pst_para->type = TPOAM_DM_TYPE_DMM;
        pst_para->interval = TPOAM_DM_INTERVAL_1S;
        pst_para->exp = TPOAM_EXP_DEF;
    }
    else
    {
        TPOAM_ASSERT;
    }
}

int32
tpoam_dm_start(tbl_oam_session_t* pst_oam, tpoam_dm_para_t* pst_para, uint32 on_demand)
{
    tbl_tpoam_global_t *p_db_tpoam_glb = tbl_tpoam_global_get_tpoam_global();
    uint32 status;
    tpoam_dm_t* pst_dm = &pst_oam->oam_dm;
    int32 tics = 0;
    int32 ret = TPOAM_E_NONE;

    if (pst_oam->dm_active)
    {
        if (TPOAM_DM_TYPE_1DM == pst_dm->para.type)
        {
            return TPOAM_E_DM_1DM_IS_RUNNING;
        }
        else
        {
            return TPOAM_E_DM_DMM_IS_RUNNING;
        }
    }

    /* 1. check has lmep? */
    if (!pst_oam->mepid)
    {
        return TPOAM_E_LMEP_NOT_EXIST;
    }
        
    /* 3. check service is up */
    status = TPOAM_STATUS_DOWN;
    ret = tpoam_api_service_get_status(pst_oam, TPOAM_SOCK_DIRECTION_PE, &status);
    if (ret < 0)
    {
        return ret;
    }
    
    if (TPOAM_STATUS_DOWN == status)
    {
        if (on_demand)
        {
            return TPOAM_E_SERVICE_DOWN;
        }
    }

    ret = tpoam_dm_deinit_dm_session(pst_oam);
    if (ret < 0)
    {
        return ret;
    }
    
    ret = _tpoam_dm_init_dm_session(pst_oam, pst_para);
    if (ret < 0)
    {
        return ret;
    }

    if (p_db_tpoam_glb->dm_enable_global)
    {
        if (on_demand)
        {
            /*added by chenc for bug 47987*/
            //sleep(3);
            ret = _tpoam_dm_tx_new(pst_oam);
            if (ret < 0)
            {
                if (on_demand)
                {
                    tpoam_dm_deinit_dm_session(pst_oam);
                    return ret;
                }
            }
            else
            {
                pst_dm->tx_count++;
            } 
        }
    }

    if (!on_demand) 
    {
        pst_dm->para.interval = pst_para->interval;
    }

    ret = tpoam_api_dm_interval_to_ms(pst_dm->para.interval, &tics);
    if (ret < 0)
    {
        tpoam_dm_deinit_dm_session(pst_oam);
        return ret;
    }
    
    if (TPOAM_DM_TYPE_1DM == pst_dm->para.type)
    {
        TPOAM_DBG_DM_1DMTX("Oam session %u MEG %s TX 1DM, current count %d, total count %d", 
            pst_oam->key.session_id, pst_oam->megid_str, pst_dm->tx_count, pst_dm->para.repeat_cnt);
        if (pst_dm->tx_count < pst_dm->para.repeat_cnt)
        {
            //pst_oam->p_t_send_dm_timer = tpoam_util_add_timer_ms(_tpoam_dm_send_1dm_cb, pst_oam, tics);
        }
        else
        {
            /* for repeat_cnt = 1, deinit session at once */
            tpoam_dm_deinit_dm_session(pst_oam);
        }
    }
    else if (TPOAM_DM_TYPE_DMM == pst_dm->para.type)
    {
        if (on_demand) 
        {
            if (pst_dm->tx_count >= pst_dm->para.repeat_cnt)
            {
                /* wait 2s for last DMR */
                tics = (TPOAM_DM_LAST_DMR_WAIT_TIME * 1000);
            }
            
            TPOAM_DBG_DM_DMM("Oam session %u MEG %s start TX DMM, current count %d, total count %d on demand ", 
                pst_oam->key.session_id, pst_oam->megid_str, pst_dm->tx_count, pst_dm->para.repeat_cnt);        
            pst_oam->p_t_send_dm_timer = tpoam_util_add_timer_ms(_tpoam_dm_send_dmm_cb_on_demand, pst_oam, tics);
        }
        else
        {
            TPOAM_DBG_DM_DMM("Oam session %u MEG %s start TX DMM", 
                pst_oam->key.session_id, pst_oam->megid_str);
            pst_oam->p_t_send_dm_timer = tpoam_util_add_timer_ms(_tpoam_dm_send_dmm_cb, pst_oam, 2000);
        }
    }
    else
    {
        TPOAM_ASSERT;
    }
    
    return TPOAM_E_NONE;
}

int32
tpoam_dm_stop_on_demand(tbl_oam_session_t* pst_oam)
{
    TPOAM_OP_ERR_RET(tpoam_dm_deinit_dm_session(pst_oam));
    TPOAM_OP_ERR_RET(tpoam_api_exec_deinit(pst_oam));
    return PM_E_NONE;
}

int32
tpoam_dm_stop(tbl_oam_session_t* pst_oam)
{
    tpoam_util_cancel_timer(&pst_oam->p_t_send_dm_timer);
    return PM_E_NONE;
}

static int32
_tpoam_dm_deinit_dm(tbl_oam_session_t* pst_oam)
{
    /* deinit DM if has */    
    TPOAM_OP_ERR_RET(tpoam_dm_stop(pst_oam));

    /* deinit DM */
    tpoam_dm_destroy_list_dmm(pst_oam);
    
    return TPOAM_E_NONE;
}

int32
tpoam_dm_init(tbl_oam_session_t* pst_oam)
{
    //tpoam_dm_t* pst_dm = &pst_oam->oam_dm;
    //tpoam_rx1dm_t* pst_1dm = &pst_oam->oam_dm.rx1dm;

    pst_oam->dm_active = FALSE;
    if (NULL == pst_oam->oam_list.pst_dmm_cache_list)
    {
        pst_oam->oam_list.pst_dmm_cache_list = list_new();
    }

    //pst_1dm->pst_rx1dm_cache_list = list_new();
    //pst_1dm->max_count = TPOAM_1DM_CACHE_SIZE_DEF;
    return TPOAM_E_NONE;
}

int32
tpoam_dm_deinit(tbl_oam_session_t* pst_oam)
{
    //TPOAM_OP_ERR_RET(_tpoam_dm_deinit_1dm(pst_oam));
    TPOAM_OP_ERR_RET(_tpoam_dm_deinit_dm(pst_oam));
    
    return TPOAM_E_NONE;
}

