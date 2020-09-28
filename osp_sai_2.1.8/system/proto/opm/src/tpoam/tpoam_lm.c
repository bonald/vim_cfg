#include "proto.h"

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
#include "tpoam_error.h"
#include "tpoam_debug.h"
#include "tpoam_api.h"

uint32 glb_test_dual_lm_count = 0;

/* wait for last LMR */
#define TPOAM_LM_LAST_LMR_WAIT_TIME     2

#define TPOAM_CSF_RELAY_VLAN_DEFAULT 0

struct glb_tpoam_lm_single_counter_s
{
    uint32  TxFCf;
    uint32  RxFCf;
    uint32  TxFCb;
    uint32  RxFCl;

    uint32  lmm_cos_value;
};
typedef struct glb_tpoam_lm_single_counter_s glb_tpoam_lm_single_counter_t;


uint32 
_tpoam_lm_calculate(uint32 new_counter, uint32 old_counter)
{
    uint32 counter_diff = 0 ;

    if (new_counter >= old_counter)
    {
        counter_diff = new_counter - old_counter;
    }
    /* if old counter > new counter, the counter should be greater than uint32 */
    else
    {
        counter_diff = 0xFFFFFFFF - old_counter + new_counter + 1;
    }

    return counter_diff;
}

#ifndef dual_lm
void
tpoam_lm_clean_list_dual_lm(tbl_oam_session_t* pst_oam)
{
    struct listnode* pst_next_node = NULL;
    struct listnode* pst_node = NULL;
    tpoam_dual_lm_entry_t* pst_entry = NULL;
    tpoam_list_t         *oam_list = &pst_oam->oam_list;
    
    if (oam_list->pst_dual_lm_cache_list)
    {
        LIST_LOOP_DEL(oam_list->pst_dual_lm_cache_list, pst_entry, pst_node, pst_next_node)
        {
            listnode_delete(oam_list->pst_dual_lm_cache_list, pst_entry);
            XFREE(MEM_PM_TPOAM_LM_DUAL, pst_entry);
        }
    }

    return;
}

void
tpoam_lm_destroy_list_dual_lm(tbl_oam_session_t* pst_oam)
{
    tpoam_lm_clean_list_dual_lm(pst_oam);
    
    list_free(pst_oam->oam_list.pst_dual_lm_cache_list);
    pst_oam->oam_list.pst_dual_lm_cache_list = NULL;

    return;
}

void
tpoam_lm_get_test_data(hal_tpoam_lm_dual_entry_t *lm_dual_counter)
{
    glb_test_dual_lm_count++;
    if (glb_test_dual_lm_count >= 0 && glb_test_dual_lm_count <= 10)
    {
        return;
    }

    /*threshold is 30*/
    
    if (glb_test_dual_lm_count == 11)
    {
        /*near*/
        lm_dual_counter->TxFCf = 20;
        lm_dual_counter->RxFCl = 10;
        /*loss is 10*/
        /*ratio is 50*/

        /*far*/
        lm_dual_counter->TxFCb = 10;
        lm_dual_counter->RxFCb = 8;
        /*loss is 2*/
        /*ratio is 20*/
    }

    if (glb_test_dual_lm_count == 12)
    {
        /*near*/
        lm_dual_counter->TxFCf = 30;
        lm_dual_counter->RxFCl = 18;
        /*loss is 2*/
        /*ratio is 20*/

        /*far*/
        lm_dual_counter->TxFCb = 20;
        lm_dual_counter->RxFCb = 13;
        /*loss is 5*/
        /*ratio is 50*/
    }
    
    if (glb_test_dual_lm_count == 13)
    {
        /*near*/
        lm_dual_counter->TxFCf = 40;
        lm_dual_counter->RxFCl = 26;
        /*loss is 2*/
        /*ratio is 20*/

        /*far*/
        lm_dual_counter->TxFCb = 40;
        lm_dual_counter->RxFCb = 29;
        /*loss is 4*/
        /*ratio is 20*/
    }

    if (glb_test_dual_lm_count == 14)
    {
        /*near*/
        lm_dual_counter->TxFCf = 50;
        lm_dual_counter->RxFCl = 34;
        /*loss is 2*/
        /*ratio is 20*/

        /*far*/
        lm_dual_counter->TxFCb = 50;
        lm_dual_counter->RxFCb = 37;
        /*loss is 2*/
        /*ratio is 20*/
    }

    if (glb_test_dual_lm_count == 15)
    {
        /*near*/
        lm_dual_counter->TxFCf = 60;
        lm_dual_counter->RxFCl = 42;
        /*loss is 2*/
        /*ratio is 20*/

        /*far*/
        lm_dual_counter->TxFCb = 60;
        lm_dual_counter->RxFCb = 45;
        /*loss is 2*/
        /*ratio is 20*/
    }

    if (glb_test_dual_lm_count == 16)
    {
        /*near*/
        lm_dual_counter->TxFCf = 70;
        lm_dual_counter->RxFCl = 50;
        /*loss is 2*/
        /*ratio is 20*/

        /*far*/
        lm_dual_counter->TxFCb = 70;
        lm_dual_counter->RxFCb = 53;
        /*loss is 2*/
        /*ratio is 20*/
    }
    
    if (glb_test_dual_lm_count == 17)
    {
        /*near*/
        lm_dual_counter->TxFCf = 80;
        lm_dual_counter->RxFCl = 57;
        /*loss is 3*/
        /*ratio is 30*/

        /*far*/
        lm_dual_counter->TxFCb = 80;
        lm_dual_counter->RxFCb = 61;
        /*loss is 2*/
        /*ratio is 20*/
    }

    if (glb_test_dual_lm_count == 18)
    {
        /*near*/
        lm_dual_counter->TxFCf = 4294967290UL;
        lm_dual_counter->RxFCl = 4194967267UL;
        /*loss is 3*/
        /*ratio is 30*/

        /*far*/
        lm_dual_counter->TxFCb = 80;
        lm_dual_counter->RxFCb = 61;
        /*loss is 2*/
        /*ratio is 20*/
    }

    if (glb_test_dual_lm_count == 19)
    {
        /*near*/
        lm_dual_counter->TxFCf = 5;
        lm_dual_counter->RxFCl = 4194967274UL;
        /*loss is 3*/
        /*ratio is 30*/

        /*far*/
        lm_dual_counter->TxFCb = 80;
        lm_dual_counter->RxFCb = 61;
        /*loss is 2*/
        /*ratio is 20*/
    }

    return;
}

uint32
tpoam_lm_get_local_threshold(tbl_oam_session_t* pst_oam)
{
    tbl_tpoam_global_t *p_db_tpoam_glb = tbl_tpoam_global_get_tpoam_global();
    
    if (pst_oam->oam_lm.local_loss_threshold)
    {
        return pst_oam->oam_lm.local_loss_threshold;
    }
    else
    {
        return p_db_tpoam_glb->lm_local_threshold;
    }
}

uint32
tpoam_lm_get_remote_threshold(tbl_oam_session_t* pst_oam)
{
    tbl_tpoam_global_t *p_db_tpoam_glb = tbl_tpoam_global_get_tpoam_global();
    
    if (pst_oam->oam_lm.remote_loss_threshold)
    {
        return pst_oam->oam_lm.remote_loss_threshold;
    }
    else
    {
        return p_db_tpoam_glb->lm_remote_threshold;
    }
}

int32 
tpoam_lm_add_dual_cache_node(hal_tpoam_lm_dual_entry_t *lm_dual_counter, tbl_oam_session_t* pst_oam, tpoam_dual_lm_entry_t *out_lm_dual_entry)
{
    tpoam_dual_lm_t *p_lm_dual = &pst_oam->oam_lm.lm_dual;
    tpoam_list_t         *oam_list = &pst_oam->oam_list;
    tpoam_dual_lm_entry_t *lm_dual_entry_new = NULL;
    struct listnode *prev_node = NULL;
    struct listnode *node = NULL;
    tpoam_dual_lm_entry_t *lm_dual_entry = NULL;
    sal_time_t  dwtime;
    uint32  TxFCf_diff = 0;
    uint32  TxFCb_diff = 0;
    uint32  RxFCb_diff = 0;
    uint32  RxFCl_diff = 0;
    uint64   frame_loss = 0;

    lm_dual_entry_new = XCALLOC (MEM_PM_TPOAM_LM_DUAL, sizeof (tpoam_dual_lm_entry_t));
    if (lm_dual_entry_new == NULL)
    {
        TPOAM_LOG_ERR("Unable to allocate the memory for tpoam_dual_lm_entry_t structure\n");
        return TPOAM_E_MEMORY_ALLOC_FAIL;
    }

    /* reserve counter and ccm cos */
    lm_dual_entry_new->TxFCf = lm_dual_counter->TxFCf;
    lm_dual_entry_new->TxFCb = lm_dual_counter->TxFCb;
    lm_dual_entry_new->RxFCb = lm_dual_counter->RxFCb;
    lm_dual_entry_new->RxFCl = lm_dual_counter->RxFCl;

    if (TPOAM_LM_COS_TYPE_ALL == p_lm_dual->cos_type)
    {
        lm_dual_entry_new->ccm_cos_value = TPOAM_LM_PRIORITY_FOR_ALL_COS;
    }
    else
    {
        lm_dual_entry_new->ccm_cos_value = lm_dual_counter->ccm_cos_value;
    }

    if (TPOAM_LM_PRIORITY_FOR_ALL_COS == lm_dual_entry_new->ccm_cos_value)
    {
        TPOAM_DBG_LM_COMM("Add dual lm node: \nsession id: %d, TxFCf %u, TxFCb %u, RxFCb %u, RxFCl %u, all-exp\n", 
            pst_oam->key.session_id,
            lm_dual_counter->TxFCf, lm_dual_counter->TxFCb,
            lm_dual_counter->RxFCb, lm_dual_counter->RxFCl);
    }
    else
    {
        TPOAM_DBG_LM_COMM("Add dual lm node: \nsession id: %d, TxFCf %u, TxFCb %u, RxFCb %u, RxFCl %u, exp %u\n",
            pst_oam->key.session_id,
            lm_dual_counter->TxFCf, lm_dual_counter->TxFCb,
            lm_dual_counter->RxFCb, lm_dual_counter->RxFCl, lm_dual_entry_new->ccm_cos_value);
    }

    /* reserve time */
    time(&dwtime); 
    lm_dual_entry_new->recv_time = (uint32)dwtime; 
    lm_dual_entry_new->signal_status = pst_oam->signal_status;

    if (NULL == oam_list->pst_dual_lm_cache_list)
    {
        oam_list->pst_dual_lm_cache_list = list_new();
    }
    
    /* find prev node with same cos for per cos, or tail node for other */
    if (TPOAM_LM_COS_TYPE_PER_COS != p_lm_dual->cos_type)
    {
        prev_node = oam_list->pst_dual_lm_cache_list->tail;
    }
    else
    {
        /*for per cos, get the same cos node*/
        node = oam_list->pst_dual_lm_cache_list->tail;
        while (node)
        {
            lm_dual_entry = node->data;
            if (lm_dual_entry->ccm_cos_value == lm_dual_entry_new->ccm_cos_value)
            {
                prev_node = node;
                break;
            }

            node = node->prev;
        }
    }
    
    /* calculate loss and loss ratio */
    if (prev_node)
    {
        lm_dual_entry = prev_node->data;

        if (TPOAM_LM_PRIORITY_FOR_ALL_COS == lm_dual_entry->ccm_cos_value)
        {
            TPOAM_DBG_LM_COMM("Old dual lm node: \nsession id: %d, TxFCf %u, TxFCb %u, RxFCb %u, RxFCl %u, all-exp\n", 
                pst_oam->key.session_id,
                lm_dual_entry->TxFCf, lm_dual_entry->TxFCb,
                lm_dual_entry->RxFCb, lm_dual_entry->RxFCl);
        }
        else
        {
            TPOAM_DBG_LM_COMM("Old dual lm node: \nsession id: %d, TxFCf %u, TxFCb %u, RxFCb %u, RxFCl %u, exp %u\n", 
                pst_oam->key.session_id,
                lm_dual_entry->TxFCf, lm_dual_entry->TxFCb,
                lm_dual_entry->RxFCb, lm_dual_entry->RxFCl, 
                lm_dual_entry->ccm_cos_value);
        }
        
        TxFCf_diff = _tpoam_lm_calculate(lm_dual_entry_new->TxFCf, lm_dual_entry->TxFCf);
        TxFCb_diff = _tpoam_lm_calculate(lm_dual_entry_new->TxFCb, lm_dual_entry->TxFCb);
        RxFCb_diff = _tpoam_lm_calculate(lm_dual_entry_new->RxFCb, lm_dual_entry->RxFCb);
        RxFCl_diff = _tpoam_lm_calculate(lm_dual_entry_new->RxFCl, lm_dual_entry->RxFCl);

        TPOAM_DBG_LM_COMM("Result: session id: %d, TxFCf %u, TxFCb %u, RxFCb %u, RxFCl %u\n",
            pst_oam->key.session_id,
            TxFCf_diff, TxFCb_diff, RxFCb_diff, RxFCl_diff);

        /* Frame Loss (far-end) = |TxFCb[tc] - TxFCb[tp]| - |RxFCb[tc] - RxFCb[tp]| */
        if (TxFCb_diff >= RxFCb_diff)
        {
            lm_dual_entry_new->lm_far_end_frame_loss = TxFCb_diff - RxFCb_diff;
        }
        else
        {
            /*modifyed by yangl for bug 52705*/
            //lm_dual_entry_new->lm_far_end_frame_loss = RxFCb_diff - TxFCb_diff;    
        }

        /*expend 1million*/
        frame_loss = lm_dual_entry_new->lm_far_end_frame_loss * TPOAM_LM_FRAME_LOSS_RATIO_BITS;

        if (TxFCb_diff)
        {
            lm_dual_entry_new->lm_far_end_frame_loss_ratio = frame_loss / TxFCb_diff;
        }

        /* Frame Loss (near-end) = |TxFCf[tc] - TxFCf[tp]| - |RxFCl[tc] - RxFCl[tp]| */
        if (TxFCf_diff >= RxFCl_diff)
        {
            lm_dual_entry_new->lm_near_end_frame_loss = TxFCf_diff - RxFCl_diff;
        }
        else
        {
            /*modifyed by yangl for bug 52705*/
            //lm_dual_entry_new->lm_near_end_frame_loss = RxFCl_diff - TxFCf_diff;    
        }
        
        frame_loss = lm_dual_entry_new->lm_near_end_frame_loss * TPOAM_LM_FRAME_LOSS_RATIO_BITS;

        if (TxFCf_diff)
        {
            lm_dual_entry_new->lm_near_end_frame_loss_ratio = frame_loss / TxFCf_diff;
        }
    }

    /*delete the oldest node*/
    if (oam_list->pst_dual_lm_cache_list->count == p_lm_dual->cache_size)
    {
        TPOAM_DBG_LM_COMM("Session id: %d, lm list count %u, is equal to lm cache size %u, delete the oldest node\n",
            pst_oam->key.session_id,
            oam_list->pst_dual_lm_cache_list->count, p_lm_dual->cache_size);
            
        node = oam_list->pst_dual_lm_cache_list->head;
        lm_dual_entry = node->data;
        listnode_delete(oam_list->pst_dual_lm_cache_list, node->data);
        XFREE (MEM_PM_TPOAM_LM_DUAL, lm_dual_entry);
    }

    if (TPOAM_LM_PRIORITY_FOR_ALL_COS == lm_dual_entry_new->ccm_cos_value)
    {
        TPOAM_DBG_LM_COMM("Session id: %d, add new dual lm entry %u/%u: all-exp\n",
            pst_oam->key.session_id,
            (oam_list->pst_dual_lm_cache_list->count) + 1, p_lm_dual->cache_size);
    }
    else
    {
        TPOAM_DBG_LM_COMM("Session id: %d, add new dual lm entry %u/%u: exp %u\n",
            pst_oam->key.session_id,
            (oam_list->pst_dual_lm_cache_list->count) + 1, p_lm_dual->cache_size, lm_dual_entry_new->ccm_cos_value);
    }

    TPOAM_DBG_LM_COMM("\nSession id: %d, lm_near_end_frame_loss %u\nlm_near_end_frame_loss_ratio %02d.%04d\n"
        "lm_far_end_frame_loss %u\nlm_far_end_frame_loss_ratio %02d.%04d\n",
        pst_oam->key.session_id,
        lm_dual_entry_new->lm_near_end_frame_loss, 
        lm_dual_entry_new->lm_near_end_frame_loss_ratio / TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS,
        lm_dual_entry_new->lm_near_end_frame_loss_ratio % TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS,
        lm_dual_entry_new->lm_far_end_frame_loss, 
        lm_dual_entry_new->lm_far_end_frame_loss_ratio / TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS,
        lm_dual_entry_new->lm_far_end_frame_loss_ratio % TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS);
        
     TPOAM_DBG_LM_COMM("\nSession id: %d, TxFCf %u, TxFCb %u, RxFCb %u, RxFCl %u\n",
        pst_oam->key.session_id,lm_dual_entry_new->TxFCf, 
        lm_dual_entry_new->TxFCb, lm_dual_entry_new->RxFCb, 
        lm_dual_entry_new->RxFCl);
        
    listnode_add (oam_list->pst_dual_lm_cache_list, lm_dual_entry_new);

    sal_memcpy(out_lm_dual_entry, lm_dual_entry_new, sizeof(tpoam_dual_lm_entry_t));

    return TPOAM_E_NONE;
}

int32
tpoam_lm_event_upload(tbl_oam_session_t* pst_oam, tpoam_dual_lm_entry_t *p_lm_dual_entry)
{
    tbl_oam_session_upload_t * pst_oam_upload;
    uint32                      local_loss_threshold_tmp = 0;
    uint32                      remote_loss_threshold_tmp = 0;

    local_loss_threshold_tmp = tpoam_lm_get_local_threshold(pst_oam);
    remote_loss_threshold_tmp = tpoam_lm_get_remote_threshold(pst_oam);

    /*modified by chenc for bug 51665, 2019-04-15*/
    //if (local_loss_threshold_tmp == 0 && remote_loss_threshold_tmp == 0)
    //{
    //    TPOAM_DBG_LM_COMM("Upload lm event, session id %u, lm threshold had not configured\n", 
    //            pst_oam->key.session_id);
    //    return TPOAM_E_NONE;
    //}

    TPOAM_DBG_LM_COMM("Upload lm event, session id %u, local threshold %.3f, lm_near_end_frame_loss_ratio %02d.%04d\n"
        "remote_loss_threshold %.3f, lm_far_end_frame_loss_ratio %02d.%04d\n", 
        pst_oam->key.session_id, (float)local_loss_threshold_tmp/TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS_1000, 
        p_lm_dual_entry->lm_near_end_frame_loss_ratio / TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS,
        p_lm_dual_entry->lm_near_end_frame_loss_ratio % TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS,
        (float)remote_loss_threshold_tmp/TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS_1000, 
        p_lm_dual_entry->lm_far_end_frame_loss_ratio / TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS,
        p_lm_dual_entry->lm_far_end_frame_loss_ratio % TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS);

    /*modified by chenc for bug 51665, 2019-04-15*/
    if (p_lm_dual_entry->lm_far_end_frame_loss_ratio > remote_loss_threshold_tmp * TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS_10 ||
        p_lm_dual_entry->lm_near_end_frame_loss_ratio > local_loss_threshold_tmp * TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS_10)
    //if ((remote_loss_threshold_tmp != 0 && 
    //    p_lm_dual_entry->lm_far_end_frame_loss_ratio >= remote_loss_threshold_tmp * TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS_10) ||
    //    (local_loss_threshold_tmp != 0 &&
    //    p_lm_dual_entry->lm_near_end_frame_loss_ratio >= local_loss_threshold_tmp * TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS_10))
    {
        pst_oam->oam_lm.normal_packet_cnt_continue = 0;
        if (pst_oam->oam_lm.is_event_occur)
        {
            TPOAM_DBG_LM_COMM("Upload lm event, session id %u, event had occur\n", 
                pst_oam->key.session_id);
            return TPOAM_E_NONE;
        }
        
        pst_oam->oam_lm.is_event_occur = TRUE;
        
        pst_oam_upload = tbl_oam_session_upload_get_oam_session_upload((tbl_oam_session_upload_key_t *)&(pst_oam->key));
        if (pst_oam_upload)
        {
            pst_oam_upload->upload_lm_event.bit_flag = GLB_TPOAM_UPLOAD_EVENT_SET;
            pst_oam_upload->upload_lm_event.index++;
            pst_oam_upload->upload_lm_event.lm_far_end_frame_loss = p_lm_dual_entry->lm_far_end_frame_loss;
            pst_oam_upload->upload_lm_event.lm_far_end_frame_loss_ratio = p_lm_dual_entry->lm_far_end_frame_loss_ratio;
            pst_oam_upload->upload_lm_event.lm_near_end_frame_loss = p_lm_dual_entry->lm_near_end_frame_loss;
            pst_oam_upload->upload_lm_event.lm_near_end_frame_loss_ratio = p_lm_dual_entry->lm_near_end_frame_loss_ratio;

            logid_sys(LOG_OAM_4_TPOAM_LM_EVENT, tpoam_api_event_action_desc(pst_oam_upload->upload_lm_event.bit_flag),
                    pst_oam->key.session_id, 
                    pst_oam_upload->upload_lm_event.lm_near_end_frame_loss,
                    pst_oam_upload->upload_lm_event.lm_near_end_frame_loss_ratio / TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS,
                    pst_oam_upload->upload_lm_event.lm_near_end_frame_loss_ratio % TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS,
                    pst_oam_upload->upload_lm_event.lm_far_end_frame_loss,
                    pst_oam_upload->upload_lm_event.lm_far_end_frame_loss_ratio / TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS,
                    pst_oam_upload->upload_lm_event.lm_far_end_frame_loss_ratio % TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS);
                    
            tbl_oam_session_upload_set_oam_session_upload_field(pst_oam_upload, TBL_OAM_SESSION_UPLOAD_FLD_UPLOAD_LM_EVENT);
            TPOAM_DBG_LM_COMM("Upload lm event set, session id %u, bit_flag %hhu, index %hhu, \n"
                "lm_near_end_frame_loss %u, lm_near_end_frame_loss_ratio %02d.%04d\n"
                "lm_far_end_frame_loss %u, lm_far_end_frame_loss_ratio %02d.%04d, \n", 
                pst_oam->key.session_id,
                pst_oam_upload->upload_lm_event.bit_flag,
                pst_oam_upload->upload_lm_event.index,
                pst_oam_upload->upload_lm_event.lm_near_end_frame_loss,
                pst_oam_upload->upload_lm_event.lm_near_end_frame_loss_ratio / TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS,
                pst_oam_upload->upload_lm_event.lm_near_end_frame_loss_ratio % TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS,
                pst_oam_upload->upload_lm_event.lm_far_end_frame_loss,
                pst_oam_upload->upload_lm_event.lm_far_end_frame_loss_ratio / TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS, 
                pst_oam_upload->upload_lm_event.lm_far_end_frame_loss_ratio % TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS);
        }
        else
        {
            TPOAM_LOG_ERR("Upload lm event set, but not find the tbl %u, error\n", pst_oam->key.session_id);
            return TPOAM_E_FAIL;
        }
    }
    else
    {
        pst_oam->oam_lm.normal_packet_cnt_continue++;
        if (pst_oam->oam_lm.is_event_occur && pst_oam->oam_lm.normal_packet_cnt_continue >= 3)
        {
            pst_oam->oam_lm.is_event_occur = FALSE;
            
            pst_oam_upload = tbl_oam_session_upload_get_oam_session_upload((tbl_oam_session_upload_key_t *)&(pst_oam->key));
            if (pst_oam_upload)
            {
                pst_oam_upload->upload_lm_event.bit_flag = GLB_TPOAM_UPLOAD_EVENT_CLEAR;
                pst_oam_upload->upload_lm_event.index++;
                pst_oam_upload->upload_lm_event.lm_far_end_frame_loss = p_lm_dual_entry->lm_far_end_frame_loss;
                pst_oam_upload->upload_lm_event.lm_far_end_frame_loss_ratio = p_lm_dual_entry->lm_far_end_frame_loss_ratio;
                pst_oam_upload->upload_lm_event.lm_near_end_frame_loss = p_lm_dual_entry->lm_near_end_frame_loss;
                pst_oam_upload->upload_lm_event.lm_near_end_frame_loss_ratio = p_lm_dual_entry->lm_near_end_frame_loss_ratio;

                logid_sys(LOG_OAM_4_TPOAM_LM_EVENT, tpoam_api_event_action_desc(pst_oam_upload->upload_lm_event.bit_flag),
                    pst_oam->key.session_id, 
                    pst_oam_upload->upload_lm_event.lm_near_end_frame_loss,
                    pst_oam_upload->upload_lm_event.lm_near_end_frame_loss_ratio / TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS,
                    pst_oam_upload->upload_lm_event.lm_near_end_frame_loss_ratio % TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS,
                    pst_oam_upload->upload_lm_event.lm_far_end_frame_loss,
                    pst_oam_upload->upload_lm_event.lm_far_end_frame_loss_ratio / TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS,
                    pst_oam_upload->upload_lm_event.lm_far_end_frame_loss_ratio % TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS);
                    
                tbl_oam_session_upload_set_oam_session_upload_field(pst_oam_upload, TBL_OAM_SESSION_UPLOAD_FLD_UPLOAD_LM_EVENT);
                TPOAM_DBG_LM_COMM("Upload lm event clear, session id %u, bit_flag %hhu, index %hhu, \n"
                "lm_near_end_frame_loss %u, lm_near_end_frame_loss_ratio %02d.%04d\n"
                "lm_far_end_frame_loss %u, lm_far_end_frame_loss_ratio %02d.%04d\n", 
                pst_oam->key.session_id,
                pst_oam_upload->upload_lm_event.bit_flag,
                pst_oam_upload->upload_lm_event.index,
                pst_oam_upload->upload_lm_event.lm_near_end_frame_loss,
                pst_oam_upload->upload_lm_event.lm_near_end_frame_loss_ratio / TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS,
                pst_oam_upload->upload_lm_event.lm_near_end_frame_loss_ratio % TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS,
                pst_oam_upload->upload_lm_event.lm_far_end_frame_loss,
                pst_oam_upload->upload_lm_event.lm_far_end_frame_loss_ratio / TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS, 
                pst_oam_upload->upload_lm_event.lm_far_end_frame_loss_ratio % TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS);
            }
            else
            {
                TPOAM_LOG_ERR("Upload lm event clear, but not find the tbl %u, error\n", pst_oam->key.session_id);
                return TPOAM_E_FAIL;
            }
        }
    }
    
    return TPOAM_E_NONE;
}

int32
tpoam_lm_dual_counters_handle_hsrv_msg(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    tbl_tpoam_global_t* pst_master = tbl_tpoam_global_get_tpoam_global();
    
    if (!pst_master->lm_enable_global)
    {
        return PM_E_NONE;
    }
    
    tpoam_dual_lm_entry_t lm_dual_entry = {0};
    hal_tpoam_lm_dual_t* pst_lm = NULL;
    tbl_oam_session_t* pst_oam = NULL;
    uint32 i = 0;
    
    pst_lm = (hal_tpoam_lm_dual_t*)(p_msg->data);

    for (i = 0; i < pst_lm->count; i++)
    {
        pst_oam = tbl_oam_session_get_oam_session_by_session_id(pst_lm->lm_data[i].oam_session_id);
        if (NULL == pst_oam)
        {
            TPOAM_DBG_LM_COMM("Not found oam session by session id %u", pst_lm->lm_data[i].oam_session_id);
            continue;
        }

        if (!pst_oam->mepid)
        {
            continue;
        }
        
        if ((TPOAM_LM_TYPE_DUAL == pst_oam->oam_lm.lm_type) /*&& pst_oam->oam_lm.lm_dual*/)
        {
            /*for test*/
            //tpoam_lm_get_test_data(&pst_lm->lm_data[i]);

            /*fix bug 47695 by chenc*/
            if (!GLB_FLAG_ISSET(pst_oam->oam_status, GLB_TPOAM_EVT_UNEXP_PERIOD))    
            {
                tpoam_lm_add_dual_cache_node(&pst_lm->lm_data[i], pst_oam, &lm_dual_entry);
                tpoam_lm_event_upload(pst_oam, &lm_dual_entry);
            }
        }
    }

    return PM_E_NONE;
}

int32 
tpoam_lm_util_dual_lm_calculate_loss_ratio(uint32 session_id, tpoam_dual_lm_entry_t *lm_dual_entry_new, tpoam_dual_lm_entry_t *lm_dual_entry_old, tpoam_dual_lm_entry_t *out_lm_dual_entry)
{
    uint32  TxFCf_diff = 0;
    uint32  TxFCb_diff = 0;
    uint32  RxFCb_diff = 0;
    uint32  RxFCl_diff = 0;
    uint64   frame_loss = 0;

    /* calculate loss and loss ratio */
    if (lm_dual_entry_old && lm_dual_entry_new)
    {
        TPOAM_DBG_LM_COMM("Calc loss ratio for oam session id %u: \n"
            "new :TxFCf %u, TxFCb %u, RxFCb %u, RxFCl %u,\n "
            "old:TxFCf %u, TxFCb %u, RxFCb %u, RxFCl %u\n", 
            session_id, lm_dual_entry_new->TxFCf, lm_dual_entry_new->TxFCb,
            lm_dual_entry_new->RxFCb, lm_dual_entry_new->RxFCl,
            lm_dual_entry_old->TxFCf, lm_dual_entry_old->TxFCb,
            lm_dual_entry_old->RxFCb, lm_dual_entry_old->RxFCl);
    
        TxFCf_diff = _tpoam_lm_calculate(lm_dual_entry_new->TxFCf, lm_dual_entry_old->TxFCf);
        TxFCb_diff = _tpoam_lm_calculate(lm_dual_entry_new->TxFCb, lm_dual_entry_old->TxFCb);
        RxFCb_diff = _tpoam_lm_calculate(lm_dual_entry_new->RxFCb, lm_dual_entry_old->RxFCb);
        RxFCl_diff = _tpoam_lm_calculate(lm_dual_entry_new->RxFCl, lm_dual_entry_old->RxFCl);

        /* Frame Loss (far-end) = |TxFCb[tc] - TxFCb[tp]| - |RxFCb[tc] - RxFCb[tp]| */
        if (TxFCb_diff >= RxFCb_diff)
        {
            out_lm_dual_entry->lm_far_end_frame_loss = TxFCb_diff - RxFCb_diff;
        }
        else
        {
            /*modified by chenc for bug 52392, 2019-06-12*/
            return PM_E_FAIL;
            //out_lm_dual_entry->lm_far_end_frame_loss = RxFCb_diff - TxFCb_diff;    
        }

        /*expend 1million*/
        frame_loss = out_lm_dual_entry->lm_far_end_frame_loss * TPOAM_LM_FRAME_LOSS_RATIO_BITS;

        if (TxFCb_diff)
        {
            out_lm_dual_entry->lm_far_end_frame_loss_ratio = frame_loss / TxFCb_diff;
        }

        /* Frame Loss (near-end) = |TxFCf[tc] - TxFCf[tp]| - |RxFCl[tc] - RxFCl[tp]| */
        if (TxFCf_diff >= RxFCl_diff)
        {
            out_lm_dual_entry->lm_near_end_frame_loss = TxFCf_diff - RxFCl_diff;
        }
        else
        {
            /*modified by chenc for bug 52392, 2019-06-12*/
            return PM_E_FAIL;
            //out_lm_dual_entry->lm_near_end_frame_loss = RxFCl_diff - TxFCf_diff;    
        }
        
        frame_loss = out_lm_dual_entry->lm_near_end_frame_loss * TPOAM_LM_FRAME_LOSS_RATIO_BITS;

        if (TxFCf_diff)
        {
            out_lm_dual_entry->lm_near_end_frame_loss_ratio = frame_loss / TxFCf_diff;
        }

        if (TPOAM_LM_PRIORITY_FOR_ALL_COS == out_lm_dual_entry->ccm_cos_value)
        {
            TPOAM_DBG_LM_COMM("Result of oam session id %d: all-exp, \nlm_near_end_frame_loss %u, lm_near_end_frame_loss_ratio %02d.%04d\n"
                ", lm_far_end_frame_loss %u, lm_far_end_frame_loss_ratio %02d.%04d, \nTxFCb %u, TxFCf %u, RxFCb %u, RxFCl %u\n",
                session_id,
                out_lm_dual_entry->lm_near_end_frame_loss, 
                out_lm_dual_entry->lm_near_end_frame_loss_ratio / TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS, 
                out_lm_dual_entry->lm_near_end_frame_loss_ratio % TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS, 
                out_lm_dual_entry->lm_far_end_frame_loss, 
                out_lm_dual_entry->lm_far_end_frame_loss_ratio / TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS, 
                out_lm_dual_entry->lm_far_end_frame_loss_ratio % TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS, 
                out_lm_dual_entry->TxFCb, out_lm_dual_entry->TxFCf, out_lm_dual_entry->RxFCb, out_lm_dual_entry->RxFCl);
        }
        else
        {
            TPOAM_DBG_LM_COMM("Result oam session id %d: exp %u,  \nlm_near_end_frame_loss %u, lm_near_end_frame_loss_ratio %02d.%04d\n"
                ", lm_far_end_frame_loss %u, lm_far_end_frame_loss_ratio %02d.%04d, \nTxFCb %u, TxFCf %u, RxFCb %u, RxFCl %u\n",
                session_id,
                out_lm_dual_entry->ccm_cos_value,
                out_lm_dual_entry->lm_near_end_frame_loss, 
                out_lm_dual_entry->lm_near_end_frame_loss_ratio / TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS, 
                out_lm_dual_entry->lm_near_end_frame_loss_ratio % TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS, 
                out_lm_dual_entry->lm_far_end_frame_loss, 
                out_lm_dual_entry->lm_far_end_frame_loss_ratio / TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS, 
                out_lm_dual_entry->lm_far_end_frame_loss_ratio % TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS, 
                out_lm_dual_entry->TxFCb, out_lm_dual_entry->TxFCf, out_lm_dual_entry->RxFCb, out_lm_dual_entry->RxFCl);
        }
    }

    return TPOAM_E_NONE;
}

/*loss from first to current*/
static int
_tpoam_lm_caculate_loss_more_interator(void *obj, tbl_iter_args_t *pargs)
{
    tbl_oam_session_t * pst_oam = (tbl_oam_session_t *)obj;
    tpoam_dual_lm_entry_t* pst_entry_newest = NULL;    
    tpoam_dual_lm_entry_t* pst_entry_oldest = NULL;
    tpoam_dual_lm_entry_t dual_lm_entry_result = {0};  
    struct listnode* pst_node_newest = NULL;
    struct listnode* pst_node_oldest = NULL;
    int32 rc = PM_E_NONE;

    /*not enable means not set to fea*/
    if (!pst_oam->lm_enable)
    {
        TPOAM_DBG_LM_COMM("Oam session %u, dual lm not enable, return", pst_oam->key.session_id);
        tpoam_lm_clean_list_dual_lm(pst_oam);
        return TPOAM_E_NONE;
    }

    /*fix bug 47662 by chenc*/
    if (TPOAM_STATUS_OK != pst_oam->signal_status)
    {
        TPOAM_DBG_LM_COMM("Oam session %u, status %s, return", pst_oam->key.session_id, tpoam_api_signal_status_desc(pst_oam->signal_status));
        tpoam_lm_clean_list_dual_lm(pst_oam);
        return TPOAM_E_NONE;
    }

    /*not bind oam flow means not set to fea and asic*/
    if (!pst_oam->bind_oam_flow_id)
    {
        TPOAM_DBG_LM_COMM("Oam session %u, dual lm not bind oam flow, return", pst_oam->key.session_id);
        tpoam_lm_clean_list_dual_lm(pst_oam);
        return TPOAM_E_NONE;
    }
    
    if (NULL == pst_oam->oam_list.pst_dual_lm_cache_list)
    {
        TPOAM_DBG_LM_COMM("Oam session %u, dual lm cache list is NULL", pst_oam->key.session_id);
        tpoam_lm_clean_list_dual_lm(pst_oam);
        return TPOAM_E_FAIL;
    }

    if (LISTCOUNT(pst_oam->oam_list.pst_dual_lm_cache_list) <= 1)
    {
        TPOAM_DBG_LM_COMM("Oam session %u, list count is %u, return",
            pst_oam->key.session_id, LISTCOUNT(pst_oam->oam_list.pst_dual_lm_cache_list));
        tpoam_lm_clean_list_dual_lm(pst_oam);
        return TPOAM_E_NONE;
    }

    /*get the newest entry*/
    pst_node_newest = LISTTAIL(pst_oam->oam_list.pst_dual_lm_cache_list);
    if (NULL == pst_node_newest)
    {
        TPOAM_DBG_LM_COMM("Oam session %u, dual lm cache list tail is NULL", pst_oam->key.session_id);
        tpoam_lm_clean_list_dual_lm(pst_oam);
        return TPOAM_E_NONE;
    }
    
    pst_entry_newest = GETDATA(pst_node_newest);
    
    /*get the oldest entry*/
    pst_node_oldest = LISTHEAD(pst_oam->oam_list.pst_dual_lm_cache_list);
    if (NULL == pst_node_oldest)
    {
        TPOAM_DBG_LM_COMM("Oam session %u, dual lm cache list head is NULL", pst_oam->key.session_id);
        tpoam_lm_clean_list_dual_lm(pst_oam);
        return TPOAM_E_NONE;
    }

    if (pst_node_newest == pst_node_oldest)
    {
        TPOAM_DBG_LM_COMM("Oam session %u, newest entry is the same with oldest entry", pst_oam->key.session_id);
        tpoam_lm_clean_list_dual_lm(pst_oam);
        return TPOAM_E_NONE;
    }
    
    pst_entry_oldest = GETDATA(pst_node_oldest);

    if (NULL == pst_entry_newest || NULL == pst_entry_oldest)
    {
        TPOAM_DBG_LM_COMM("Oam session %u, new entry or old entry is NULL, %p %p", 
            pst_oam->key.session_id, pst_entry_newest, pst_entry_oldest);
        tpoam_lm_clean_list_dual_lm(pst_oam);
        return TPOAM_E_NONE;
    }
    
    rc = tpoam_lm_util_dual_lm_calculate_loss_ratio(pst_oam->key.session_id, pst_entry_newest, pst_entry_oldest, &dual_lm_entry_result);

    if (TPOAM_STATUS_OK == pst_oam->signal_status && PM_E_NONE == rc)
    {
        /*add to db*/
        tbl_oam_session_upload_t * pst_oam_upload;
        pst_oam_upload = tbl_oam_session_upload_get_oam_session_upload((tbl_oam_session_upload_key_t *)&(pst_oam->key));
        if (pst_oam_upload)
        {
            pst_oam_upload->upload_lm.index++;
            pst_oam_upload->upload_lm.lm_far_end_frame_loss = dual_lm_entry_result.lm_far_end_frame_loss;
            pst_oam_upload->upload_lm.lm_far_end_frame_loss_ratio = dual_lm_entry_result.lm_far_end_frame_loss_ratio;
            pst_oam_upload->upload_lm.lm_near_end_frame_loss = dual_lm_entry_result.lm_near_end_frame_loss;
            pst_oam_upload->upload_lm.lm_near_end_frame_loss_ratio = dual_lm_entry_result.lm_near_end_frame_loss_ratio;
            
            /*sync msg maybe many*/
            tbl_oam_session_upload_set_oam_session_upload_field(pst_oam_upload, TBL_OAM_SESSION_UPLOAD_FLD_UPLOAD_LM);
            TPOAM_DBG_LM_COMM("Upload lm on timer, session id %u, index %hhu, \n"
                    "lm_near_end_frame_loss %u, lm_near_end_frame_loss_ratio %02d.%04d\n"
                    "lm_far_end_frame_loss %u, lm_far_end_frame_loss_ratio %02d.%04d, \n", 
                    pst_oam->key.session_id,
                    pst_oam_upload->upload_lm.index,
                    pst_oam_upload->upload_lm.lm_near_end_frame_loss,
                    pst_oam_upload->upload_lm.lm_near_end_frame_loss_ratio / TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS,
                    pst_oam_upload->upload_lm.lm_near_end_frame_loss_ratio % TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS,
                    pst_oam_upload->upload_lm.lm_far_end_frame_loss,
                    pst_oam_upload->upload_lm.lm_far_end_frame_loss_ratio / TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS, 
                    pst_oam_upload->upload_lm.lm_far_end_frame_loss_ratio % TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS);
        }
        else
        {
            TPOAM_DBG_LM_COMM("Upload lm on timer, but not find the tbl %u, error\n", pst_oam->key.session_id);
            tpoam_lm_clean_list_dual_lm(pst_oam);
            return TPOAM_E_FAIL;
        }
    }
    else
    {
        TPOAM_DBG_LM_COMM("Not upload lm on timer for status %s, session id %u,\n"
            "lm_near_end_frame_loss %u, lm_near_end_frame_loss_ratio %02d.%04d\n"
            "lm_far_end_frame_loss %u, lm_far_end_frame_loss_ratio %02d.%04d, rc %d\n", 
            tpoam_api_signal_status_desc(pst_oam->signal_status),
            pst_oam->key.session_id,
            dual_lm_entry_result.lm_near_end_frame_loss,
            dual_lm_entry_result.lm_near_end_frame_loss_ratio / TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS,
            dual_lm_entry_result.lm_near_end_frame_loss_ratio % TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS,
            dual_lm_entry_result.lm_far_end_frame_loss,
            dual_lm_entry_result.lm_far_end_frame_loss_ratio / TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS, 
            dual_lm_entry_result.lm_far_end_frame_loss_ratio % TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS, rc);
    }

    tpoam_lm_clean_list_dual_lm(pst_oam);
    return PM_E_NONE;
}

void
_tpoam_lm_calculate_cb(void *p_data)
{
    tbl_tpoam_global_t* pst_master = tbl_tpoam_global_get_tpoam_global();

    pst_master->lm_calc_interval_timer = NULL;

    if (pst_master->lm_enable_global)
    {
        tbl_oam_session_iterate(_tpoam_lm_caculate_loss_more_interator, NULL);
    }
 
    pst_master->lm_calc_interval_timer = tpoam_util_add_timer_ms(_tpoam_lm_calculate_cb, NULL, pst_master->lm_calc_interval * 1000);   
    return;
}

int32
tpoam_lm_start_calc()
{
    tbl_tpoam_global_t* pst_master = tbl_tpoam_global_get_tpoam_global();
    pst_master->lm_calc_interval_timer = pst_master->lm_calc_interval_timer ? pst_master->lm_calc_interval_timer : 
        tpoam_util_add_timer_ms(_tpoam_lm_calculate_cb, NULL, pst_master->lm_calc_interval * 1000);

    return TPOAM_E_NONE;
}

int32
tpoam_lm_stop_calc()
{
    tbl_tpoam_global_t* pst_master = tbl_tpoam_global_get_tpoam_global();
    tbl_oam_session_master_t *p_oam_session_master =  tbl_oam_session_get_master();
    
    if (p_oam_session_master && 0 == p_oam_session_master->oam_session_hash->count)
    {
        tpoam_util_cancel_timer(&pst_master->lm_calc_interval_timer);
    }
    
    return TPOAM_E_NONE;
}
#endif

#ifdef single_lm
static int32
_tpoam_lm_decode_lmm(tpoam_rx_pdu_t* pst_pdu)
{
    uint8** pnt = &pst_pdu->p_data;
    int32* size = &pst_pdu->left_len;

    TLV_DECODE_SKIP(sizeof(uint32));
    
    TLV_ENCODE_PUTL(pst_pdu->RxFcl);
    return TPOAM_E_NONE;
}


static int32
_tpoam_lm_encode_lmr(tbl_oam_session_t* pst_oam, tpoam_rx_pdu_t* pst_lbm, uint8* p_data, uint32 len)
{
    //int32   pkt_size_left = len;
    //int32*  size = NULL;
    //uint8** pnt = NULL;
    
    //size = &pkt_size_left;
    //pnt = &p_data;

    /* encode from Y.1731 header (after ACH header) */
   sal_memcpy(p_data, pst_lbm->p_y1731_head, pst_lbm->y1731_len);
    p_data[1] = TPOAM_Y1731_PDU_LMR;    /* change opcode to LMR */

    return TPOAM_E_NONE;
}

static int32
_tpoam_lm_tx_lmr(tbl_oam_session_t* pst_oam, tpoam_rx_pdu_t* pst_pdu)
{
    uint8 lm_data[TPOAM_PKT_BUF_SIZE];
    int32 ret = TPOAM_E_NONE;

    tbl_tpoam_global_t *p_db_tpoam_glb = tbl_tpoam_global_get_tpoam_global();
    
    sal_memset(lm_data, 0, sizeof(lm_data));
    ret = _tpoam_lm_encode_lmr(pst_oam, pst_pdu, lm_data, TPOAM_PKT_BUF_SIZE);
    if (ret < 0)
    {
        return ret;
    }
    
    ret = tpoam_sock_tx(pst_oam, TPOAM_SOCK_DIRECTION_PE, pst_pdu->y1731_len, lm_data, 
        pst_pdu->label.exp, TPOAM_SOCK_TTL_DEF, TPOAM_SOCK_LM_FLAG_SET, FALSE, FALSE);   
    if (ret < 0)
    {
        return ret;
    }
    tpoam_api_stats_add_tx(&pst_oam->oam_stats.tx, TPOAM_Y1731_PDU_LMR);
    tpoam_api_stats_add_tx(&p_db_tpoam_glb->oam_glb_stats.tx, TPOAM_Y1731_PDU_LMR);        
    
    return TPOAM_E_NONE;
}


int32
tpoam_lm_rx_lmm(tbl_oam_session_t* pst_oam, tpoam_rx_pdu_t* pst_pdu)
{
    int32 ret = TPOAM_E_NONE;

    if ((pst_oam->oam_lm.lm_type != TPOAM_LM_TYPE_SINGLE) /*|| !pst_oam->oam_lm.lm_single*/)
    {
        TPOAM_DBG_LM_LMR("Oam session %u MEG %s receives LMM when single LM disabled, ignore LMM", 
            pst_oam->key.session_id, pst_oam->megid_str);
        return TPOAM_E_NONE;
    }
    
    if (TPOAM_Y1731_PDU_LMM != pst_pdu->hdr.opcode)
    {
        return TPOAM_E_PDU_INVALID_OPCODE;
    }

    ret = _tpoam_lm_decode_lmm(pst_pdu);
    if (ret < 0)
    {
        return ret;
    }
    TPOAM_OP_ERR_RET(_tpoam_lm_tx_lmr(pst_oam, pst_pdu));
    return TPOAM_E_NONE;
}


static int32
_tpoam_lm_decode_lmr(tbl_oam_session_t* pst_oam, tpoam_rx_pdu_t* pst_pdu)
{
    uint32 TxFcf, RxFcf, TxFcb; 
    uint8** pnt = &pst_pdu->p_data;
    int32* size = &pst_pdu->left_len;
    
    
    TLV_DECODE_GETL(TxFcf);
    TLV_DECODE_GETL(RxFcf);
    TLV_DECODE_GETL(TxFcb);
    
    pst_pdu->TxFcf = TxFcf;
    pst_pdu->RxFcf = RxFcf;
    pst_pdu->TxFcb = TxFcb;
    return TPOAM_E_NONE;
}

int32 tpoam_lm_add_single_cache_node(glb_tpoam_lm_single_counter_t *lm_single_counter, tpoam_single_lm_t* lm_single)
{
    tpoam_single_lm_entry_t *lm_single_entry_new = NULL;
    struct listnode *prev_node = NULL;
    struct listnode *node = NULL;
    tpoam_single_lm_entry_t *lm_single_entry = NULL;
    sal_time_t  dwtime;
    uint32  TxFCf_diff = 0;
    uint32  TxFCb_diff = 0;
    uint32  RxFCf_diff = 0;
    uint32  RxFCl_diff = 0;
    uint64     frame_loss = 0;

    lm_single_entry_new = XCALLOC (MEM_PM_TPOAM_DM_SINGLE_ENTRY, sizeof (tpoam_single_lm_entry_t));
    if (lm_single_entry_new == NULL)
    {
        TPOAM_DBG_LM_LMM("Unable to allocate the memory for tpoam_single_lm_entry_t structure");
        return TPOAM_E_MEMORY_ALLOC_FAIL;
    }

    /* reserve counter and ccm cos */
    lm_single_entry_new->TxFCf = lm_single_counter->TxFCf;
    lm_single_entry_new->TxFCb = lm_single_counter->TxFCb;
    lm_single_entry_new->RxFCf = lm_single_counter->RxFCf;
    lm_single_entry_new->RxFCl = lm_single_counter->RxFCl;

    if (TPOAM_LM_PRIORITY_FOR_ALL_COS == lm_single_counter->lmm_cos_value)
    {
        TPOAM_DBG_LM_LMM("Add lm single node: TxFCf %u, TxFCb %u, RxFCf %u, RxFCl %u, all-exp, "
            "for mep_id %d\n", 
            lm_single_counter->TxFCf, lm_single_counter->TxFCb, lm_single_counter->RxFCf, 
            lm_single_counter->RxFCl, 0/*(uint32)lm_single->mep->mep_id*/);
    }
    else
    {
        TPOAM_DBG_LM_LMM("Add lm single node: TxFCf %u, TxFCb %u, RxFCf %u, RxFCl %u, exp %u, "
            "for mep_id %d\n", 
            lm_single_counter->TxFCf, lm_single_counter->TxFCb, lm_single_counter->RxFCf, 
            lm_single_counter->RxFCl, lm_single_counter->lmm_cos_value, 0/*(uint32)lm_single->mep->mep_id*/);
    }

    if (TPOAM_LM_COS_TYPE_ALL == lm_single->cos_type)
    {
        lm_single_entry_new->lmm_cos_value = TPOAM_LM_PRIORITY_FOR_ALL_COS;
    }
    else
    {
        lm_single_entry_new->lmm_cos_value = lm_single_counter->lmm_cos_value;
    }

    /* reserve time */
    time(&dwtime); 
    lm_single_entry_new->recv_time = (uint32)dwtime; 

    /* find prev node with same cos for per cos, or tail node for other */
    if (TPOAM_LM_COS_TYPE_PER_COS != lm_single->cos_type)
    {
        prev_node = lm_single->lm_cache_list->tail;
    }
    else
    {
        node = lm_single->lm_cache_list->tail;
        while (node)
        {
            lm_single_entry = node->data;
            if (lm_single_entry->lmm_cos_value == lm_single_entry_new->lmm_cos_value)
            {
                prev_node = node;
                break;
            }

            node = node->prev;
        }
    }
    
    /* calculate loss and loss ratio */
    if (prev_node)
    {
        lm_single_entry = prev_node->data;
        
        TxFCf_diff = _tpoam_lm_calculate(lm_single_entry_new->TxFCf, lm_single_entry->TxFCf);
        TxFCb_diff = _tpoam_lm_calculate(lm_single_entry_new->TxFCb, lm_single_entry->TxFCb);
        RxFCf_diff = _tpoam_lm_calculate(lm_single_entry_new->RxFCf, lm_single_entry->RxFCf);
        RxFCl_diff = _tpoam_lm_calculate(lm_single_entry_new->RxFCl, lm_single_entry->RxFCl);

        /* Frame Loss (far-end) = |TxFCf[tc] 每 TxFCf[tp]| 每 |RxFCf[tc] 每 RxFCf[tp]| */
        if (TxFCf_diff >= RxFCf_diff)
        {
            lm_single_entry_new->lm_far_end_frame_loss = TxFCf_diff - RxFCf_diff;
        }
        else
        {
            lm_single_entry_new->lm_far_end_frame_loss = RxFCf_diff - TxFCf_diff;    
        }

        frame_loss = lm_single_entry_new->lm_far_end_frame_loss * TPOAM_LM_FRAME_LOSS_RATIO_BITS;

        if (TxFCf_diff)
        {
            lm_single_entry_new->lm_far_end_frame_loss_ratio = frame_loss / TxFCf_diff;
        }
        
        /* Frame Loss (near-end) = |TxFCb[tc] 每 TxFCb[tp]| 每 |RxFCl[tc] 每 RxFCl[tp]| */
        if (TxFCb_diff >= RxFCl_diff)
        {
            lm_single_entry_new->lm_near_end_frame_loss = TxFCb_diff - RxFCl_diff;
        }
        else
        {
            lm_single_entry_new->lm_near_end_frame_loss = RxFCl_diff - TxFCb_diff;    
        }

        frame_loss = lm_single_entry_new->lm_near_end_frame_loss * TPOAM_LM_FRAME_LOSS_RATIO_BITS;

        if (TxFCb_diff)
        {
            lm_single_entry_new->lm_near_end_frame_loss_ratio = frame_loss / TxFCb_diff;
        }

        /* modified by yaom for proactive single lm 20140416 */
        if (lm_single->is_proactive)
        {
            /* local loss */
            if (0 != lm_single->local_loss_threshold)
            {
                if (lm_single_entry_new->lm_near_end_frame_loss_ratio >= 
                    TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS*lm_single->local_loss_threshold)
                {
                    if (!lm_single->local_loss_alarm)
                    {
                        lm_single->local_loss_alarm = TRUE;
                        /* send alarm occur trap */
                        // TODO: trap
                        //nsm_mpls_tp_oam_single_lm_trap(lm_single, lm_single_entry_new, TRUE, TRUE);
                    }
                }

                if (lm_single_entry_new->lm_near_end_frame_loss_ratio < 
                    TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS*lm_single->local_loss_threshold)
                {
                    if (lm_single->local_loss_alarm)
                    {
                        lm_single->local_loss_alarm = FALSE;
                        /* send alarm clear trap */
                        // TODO: trap
                        //mpls_tp_oam_single_lm_trap(lm_single, lm_single_entry_new, TRUE, FALSE);
                    }
                }
            }

            /* remote loss*/
            if (0 != lm_single->remote_loss_threshold)
            {
                if (lm_single_entry_new->lm_far_end_frame_loss_ratio >=
                    TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS*lm_single->remote_loss_threshold)
                {
                    if (!lm_single->remote_loss_alarm)
                    {
                        lm_single->remote_loss_alarm = TRUE;
                        /* send alarm occur trap */
                        // TODO: trap
                        //nsm_mpls_tp_oam_single_lm_trap(lm_single, lm_single_entry_new, FALSE, TRUE);
                    }
                }

                if (lm_single_entry_new->lm_far_end_frame_loss_ratio <
                    TPOAM_LM_FRAME_LOSS_RATIO_LOW_BITS*lm_single->remote_loss_threshold)
                {
                    if (lm_single->remote_loss_alarm)
                    {
                        lm_single->remote_loss_alarm = FALSE;
                        /* send alarm clear trap */
                        // TODO: trap
                        //nsm_mpls_tp_oam_single_lm_trap(lm_single, lm_single_entry_new, FALSE, FALSE);
                    }
                }
            }
        }
    }

    if (lm_single->lm_cache_list->count == lm_single->cache_size)
    {
        node = lm_single->lm_cache_list->head;
        listnode_delete(lm_single->lm_cache_list, node->data);
    }
    listnode_add (lm_single->lm_cache_list, lm_single_entry_new);

    return TPOAM_E_NONE;
}

static int32
_tpoam_lm_handle_lmr(tbl_oam_session_t* pst_oam, tpoam_rx_pdu_t* pst_pdu)
{
    tpoam_single_lm_t* pst_lm_single = NULL;
    glb_tpoam_lm_single_counter_t lm_single_counter;

    if (!pst_oam->mepid /*|| !pst_oam->oam_lm.lm_single*/)
    {
        return TPOAM_E_NONE;
    }

    pst_lm_single = &pst_oam->oam_lm.lm_single;
    
    pst_lm_single->rx_count++;
    pst_lm_single->per_lm_status = NSM_TPOAM_PER_LMM_STATUS_RX_LMR;
    
    lm_single_counter.RxFCf = pst_pdu->RxFcf;
    lm_single_counter.TxFCf = pst_pdu->TxFcf;
    lm_single_counter.TxFCb = pst_pdu->TxFcb;
    lm_single_counter.RxFCl = pst_pdu->RxFcl;
    lm_single_counter.lmm_cos_value = pst_pdu->label.exp;
    
    tpoam_lm_add_single_cache_node(&lm_single_counter, pst_lm_single);
    return TPOAM_E_NONE;
}


int32
tpoam_lm_rx_lmr(tbl_oam_session_t* pst_oam, tpoam_rx_pdu_t* pst_pdu)
{
    tpoam_single_lm_t* pst_lm_single = &pst_oam->oam_lm.lm_single;
    int32 ret = TPOAM_E_NONE;

    if (!pst_lm_single || !pst_lm_single->active)
    {
        TPOAM_DBG_LM_LMR("Oam session %u MEG %s receives LMR when LMM inactive, ignore LMR", 
            pst_oam->key.session_id, pst_oam->megid_str);
        return TPOAM_E_NONE;
    }
    
    if (TPOAM_Y1731_PDU_LMR != pst_pdu->hdr.opcode)
    {
        return TPOAM_E_PDU_INVALID_OPCODE;
    }

    ret = _tpoam_lm_decode_lmr(pst_oam, pst_pdu);
    if (ret < 0)
    {
        return ret;
    }
    TPOAM_OP_ERR_RET(_tpoam_lm_handle_lmr(pst_oam, pst_pdu));
    return TPOAM_E_NONE;
}

static int32
_tpoam_lm_encode_lmm(tbl_oam_session_t* pst_oam, uint8* p_data, int32* p_length)
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
      | MEL | Version |OpCode (LMM=43)|    Flags(0)   | TLV Offset(12)|
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                          TxFcf                                |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                          RxFcf                                |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                          TxFcb                                |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |   End TLV (0) |
      +-+-+-+-+-+-+-+-+
    */

    level_version = (((pst_oam->level & TPOAM_Y1731_PDU_MEL_MASK) << TPOAM_Y1731_PDU_MEL_SHIFT)
                    | (TPOAM_Y1731_PDU_VER & TPOAM_Y1731_PDU_VER_MASK));
    
    TLV_ENCODE_PUTC(level_version);
    TLV_ENCODE_PUTC(TPOAM_Y1731_PDU_LMM);
    TLV_ENCODE_PUTC(0);
    TLV_ENCODE_PUTC(TPOAM_Y1731_LMM_LMR_FIRST_TLV_OFFSET);
    TLV_ENCODE_PUT_EMPTY(TPOAM_Y1731_LMM_LMR_FIRST_TLV_OFFSET);     /* will be updated by HW */
    TLV_ENCODE_PUTC(TPOAM_Y1731_PDU_END_TLV);
    return TPOAM_E_NONE;
}

static int32
_tpoam_lm_tx(tbl_oam_session_t* pst_oam)
{
    tbl_tpoam_global_t *p_db_tpoam_glb = tbl_tpoam_global_get_tpoam_global();
     
    //tpoam_lm_t* pst_lm = &pst_oam->oam_lm;
    //tpoam_single_lm_t* pst_lm_single = &(pst_lm->lm_single);/*maybe error*/
    uint8 lm_data[TPOAM_PKT_BUF_SIZE];
    int32 lm_data_left = TPOAM_PKT_BUF_SIZE;
//    uint32 pkt_size = 0;

    TPOAM_OP_ERR_RET(_tpoam_lm_encode_lmm(pst_oam, lm_data, &lm_data_left));

    //pkt_size = TPOAM_PKT_BUF_SIZE - lm_data_left;

    // TODO:
    //TPOAM_OP_ERR_RET(tpoam_sock_tx(pst_oam, TPOAM_SOCK_DIRECTION_PE, pkt_size, lm_data, 
    //    pst_lm_single->cos_value, TPOAM_SOCK_TTL_DEF, TPOAM_SOCK_LM_FLAG_SET, FALSE, FALSE));
    
    tpoam_api_stats_add_tx(&pst_oam->oam_stats.tx, TPOAM_Y1731_PDU_LMM);
    tpoam_api_stats_add_tx(&p_db_tpoam_glb->oam_glb_stats.tx, TPOAM_Y1731_PDU_LMM);        

    return TPOAM_E_NONE;
}


/* modified by yaom for proactive single lm 20140416 */
void
tpoam_lm_send_proactive_lmm(void* p_data)
{
    tbl_oam_session_t* pst_oam = p_data ? (tbl_oam_session_t*)p_data : NULL;
    tpoam_single_lm_t* pst_lm_single = &(pst_oam->oam_lm.lm_single);
    int32 tics = 0;
    int32 ret = TPOAM_E_NONE;
    uint32 status;
//    uint32 lck_by_srv = FALSE;

    //TPOAM_DBG_LM_FUNC(LM_LMM);

    if (!pst_lm_single->active)
    {
        pst_lm_single->active = TRUE;
    }

    pst_lm_single->p_t_send_timer = NULL;

    ret = tpoam_api_lm_interval_to_ms(pst_lm_single->interval, &tics);
    if (ret < 0)
    {
        return;
    }

    pst_lm_single->p_t_send_timer = tpoam_util_add_timer_ms(tpoam_lm_send_proactive_lmm, pst_oam, tics);
    
    /* 3. check service is up */
    status = TPOAM_STATUS_DOWN;
    ret = tpoam_api_service_get_status(pst_oam, TPOAM_SOCK_DIRECTION_PE, &status);
    if (ret < 0)
    {
        return;
    }
    if (TPOAM_STATUS_DOWN == status)
    {
        return;
    }

    if (TPOAM_LM_COS_TYPE_PER_COS == pst_lm_single->cos_type)
    {
        if (pst_lm_single->cos_value < TPOAM_PRIORITY_MAX)
        {
            pst_lm_single->cos_value++;
        }
        else
        {
            pst_lm_single->cos_value = TPOAM_PRIORITY_MIN;
        }
    }

    ret = _tpoam_lm_tx(pst_oam);
    if (ret < 0)
    {
        return;
    }
    pst_lm_single->tx_count++;

    TPOAM_DBG_LM_LMM("Oam session %u MEG %s TX LMM, current count %d", 
                         pst_oam->key.session_id, pst_oam->megid_str, pst_lm_single->tx_count);
    return;
}

static int32
_tpoam_lm_deinit_lm_session(tbl_oam_session_t* pst_oam)
{
    tpoam_single_lm_t* pst_lm_single = &pst_oam->oam_lm.lm_single;

    if (!pst_lm_single || (TPOAM_LM_TYPE_SINGLE != pst_oam->oam_lm.lm_type))
    {
        return TPOAM_E_NONE;
    }
    
    if (!pst_lm_single->active)
    {
        return TPOAM_E_NONE;
    }

    //if (l2_is_timer_running (pst_lm_single->p_t_send_timer))
    {
        tpoam_util_cancel_timer(&pst_lm_single->p_t_send_timer);
    }

    pst_lm_single->active = FALSE;
    return TPOAM_E_NONE;
}

void
_tpoam_lm_send_lmm_cb(void *p_data)
{
    tbl_oam_session_t* pst_oam = p_data ? (tbl_oam_session_t*)p_data : NULL;
    tpoam_single_lm_t* pst_lm_single = &pst_oam->oam_lm.lm_single;
    int32 tics = 0;
    int32 ret = TPOAM_E_NONE;

    pst_lm_single->p_t_send_timer = NULL;

    if (pst_lm_single->tx_count >= pst_lm_single->para.repeat_cnt)
    {
        _tpoam_lm_deinit_lm_session(pst_oam);
        return;
    }

    if (TPOAM_LM_COS_TYPE_PER_COS == pst_lm_single->cos_type)
    {
        if (pst_lm_single->cos_value < TPOAM_PRIORITY_MAX)
        {
            pst_lm_single->cos_value++;
        }
        else
        {
            pst_lm_single->cos_value = TPOAM_PRIORITY_MIN;
        }
    }

    ret = _tpoam_lm_tx(pst_oam);
    if (ret < 0)
    {
        _tpoam_lm_deinit_lm_session(pst_oam);
        return;
    }
    pst_lm_single->tx_count++;

    TPOAM_DBG_LM_LMM("Oam session %u MEG %s TX LMM, current count %d, total count %d", 
        pst_oam->key.session_id, pst_oam->megid_str, pst_lm_single->tx_count, pst_lm_single->para.repeat_cnt);
    if (pst_lm_single->tx_count >= pst_lm_single->para.repeat_cnt)
    {
        /* wait 2s for last DMR */
        tics = TPOAM_LM_LAST_LMR_WAIT_TIME;
    }
    else
    {
        ret = tpoam_api_lm_interval_to_ms(pst_lm_single->para.interval, &tics);
        if (ret < 0)
        {
            TPOAM_ASSERT;
        }
    }
    pst_lm_single->p_t_send_timer = tpoam_util_add_timer_ms(_tpoam_lm_send_lmm_cb, pst_oam, tics);   
    return;
}

static int32
_tpoam_lm_init_lm_session(tbl_oam_session_t* pst_oam, tpoam_lmm_para_t* pst_para)
{
    tpoam_single_lm_t* pst_lm_single = &pst_oam->oam_lm.lm_single;

    sal_memcpy(&pst_lm_single->para, pst_para, sizeof(tpoam_lmm_para_t));
    pst_lm_single->cache_size = pst_para->repeat_cnt;
    pst_lm_single->tx_count = 0;
    pst_lm_single->rx_count = 0;
    pst_lm_single->per_lm_status = NSM_TPOAM_PER_LMM_STATUS_WAITING;
    pst_lm_single->active = TRUE;

    return TPOAM_E_NONE;
}

int32
tpoam_lm_start(tbl_oam_session_t* pst_oam, tpoam_lmm_para_t* pst_para)
{
    uint32 status;
    tpoam_single_lm_t* pst_lm_single = &pst_oam->oam_lm.lm_single;
    //uint32 lck_by_srv = FALSE;
    int32 tics = 0;
    int32 ret = TPOAM_E_NONE;

    /* 1. check has lmep? */
    if (!pst_oam->mepid)
    {
        return TPOAM_E_LMEP_NOT_EXIST;
    }
    #if 0
    if (!pst_oam->oam_lm.lm_single)
    {
        return TPOAM_E_LM_SINGLE_NOT_ENABLED;
    }
    #endif
    /* modified by yaom for proactive single lm 20140416 */
    if (pst_lm_single->is_proactive)
    {
        return TPOAM_E_LM_PROACTIVE_LMM_IS_RUNNING;
    }

    if (pst_lm_single->active)
    {
        return TPOAM_E_LM_LMM_IS_RUNNING;
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
        return TPOAM_E_SERVICE_DOWN;
    }

    ret = _tpoam_lm_deinit_lm_session(pst_oam);
    if (ret < 0)
    {
        return ret;
    }
    
    ret = _tpoam_lm_init_lm_session(pst_oam, pst_para);
    if (ret < 0)
    {
        return ret;
    }

    if (TPOAM_LM_COS_TYPE_PER_COS == pst_lm_single->cos_type)
    {
        pst_lm_single->cos_value = TPOAM_PRIORITY_MIN;
    }

    ret = _tpoam_lm_tx(pst_oam);
    if (ret < 0)
    {
        _tpoam_lm_deinit_lm_session(pst_oam);
        return ret;
    }
    pst_lm_single->tx_count++;
    
    ret = tpoam_api_lm_interval_to_ms(pst_lm_single->para.interval, &tics);
    if (ret < 0)
    {
        _tpoam_lm_deinit_lm_session(pst_oam);
        return ret;
    }
    
    if (pst_lm_single->tx_count >= pst_lm_single->para.repeat_cnt)
    {
        /* wait 2s for last DMR */
        tics = TPOAM_LM_LAST_LMR_WAIT_TIME;
    }
    TPOAM_DBG_LM_LMM("Oam session %u MEG %s TX LMM, current count %d, total count %d", 
        pst_oam->key.session_id, pst_oam->megid_str, pst_lm_single->tx_count, pst_lm_single->para.repeat_cnt);
    
    pst_lm_single->p_t_send_timer = tpoam_util_add_timer_ms(_tpoam_lm_send_lmm_cb, pst_oam, tics);
    return TPOAM_E_NONE;
}


int32
tpoam_lm_stop(tbl_oam_session_t* pst_oam)
{
    TPOAM_OP_ERR_RET(_tpoam_lm_deinit_lm_session(pst_oam));
    TPOAM_OP_ERR_RET(tpoam_api_exec_deinit(pst_oam));
    
    return TPOAM_E_NONE;
}

int32
tpoam_lm_init(tbl_oam_session_t* pst_oam)
{
    return TPOAM_E_NONE;
}

int32
tpoam_lm_deinit(tbl_oam_session_t* pst_oam)
{
    TPOAM_OP_ERR_RET(tpoam_lm_stop(pst_oam));    
    return TPOAM_E_NONE;
}
#endif
