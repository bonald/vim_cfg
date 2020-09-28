
#include "ctc_error.h"
#include "ctc_debug.h"
#include "ctc_hash.h"
#include "ctc_linklist.h"

#include "ctc_qos.h"
#include "ctc_packet.h"
#include "sys_usw_diag.h"
#include "sys_usw_mchip.h"

extern int32
sys_usw_dump_db_register_cb(uint8 lchip, uint8 module, CTC_DUMP_MASTER_FUNC fn);

typedef struct sys_usw_diag_drop_pkt_node_s
{
    ctc_slistnode_t head;

    uint16 lport;
    uint16 reason;
    uint32 hash_seed;
    sal_time_t timestamp;
    uint32 len;
    uint8* buf;
}sys_usw_diag_drop_pkt_node_t;

typedef struct sys_usw_diag_drop_info_s
{
    /* key */
    uint16 lport;  /* 0xFFFF means system drop stats */
    uint16 reason;

    /* data */
    uint32 count;/*drop-count*/
}sys_usw_diag_drop_info_t;

typedef struct sys_usw_diag_s
{
    ctc_slist_t* drop_pkt_list;  /* refer to sys_usw_diag_drop_pkt_node_t */
    ctc_hash_t*  drop_hash;      /* node refer to sys_usw_diag_drop_info_t*/
    sal_mutex_t* diag_mutex;

    int32 (*drop_pkt_report_cb)(uint32 gport, uint16 reason, ctc_diag_drop_info_buffer_t* pkt_buf);
    uint32 drop_pkt_store_cnt;   /* determine the pkt_list count */
    uint32 drop_pkt_store_len;
    uint16 drop_pkt_hash_len;
    uint8  drop_pkt_overwrite;/* when set, if store drop packet count exceed drop_pkt_store_cnt, overwite the oldest buffer */
    uint8  rsv;
}sys_usw_diag_t;

sys_usw_diag_t* p_usw_diag_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

#define DIAG_LOCK \
    if (p_usw_diag_master[lchip]->diag_mutex) sal_mutex_lock(p_usw_diag_master[lchip]->diag_mutex)
#define DIAG_UNLOCK \
    if (p_usw_diag_master[lchip]->diag_mutex) sal_mutex_unlock(p_usw_diag_master[lchip]->diag_mutex)

#define SYS_DIAG_DBG_OUT(level, FMT, ...) \
    CTC_DEBUG_OUT(diag, diag, DIAG_SYS, level, FMT, ## __VA_ARGS__)

#define SYS_USW_DIAG_INIT_CHECK(lchip)  \
    do{                                 \
        LCHIP_CHECK(lchip);         \
        SYS_LCHIP_CHECK_ACTIVE(lchip); \
        if (p_usw_diag_master[lchip] == NULL)    \
        { SYS_DIAG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");\
          return CTC_E_NOT_INIT; } \
      }while(0)


extern int32
sys_usw_qos_set_queue(uint8 lchip, ctc_qos_queue_cfg_t* p_que_cfg);

extern int32
sys_usw_qos_set_shape(uint8 lchip, ctc_qos_shape_t* p_shape);

extern int32
sys_usw_packet_tx(uint8 lchip, ctc_pkt_tx_t* p_pkt_tx);

extern int32
sys_usw_packet_register_drop_rx_cb(uint8 lchip, CTC_PKT_RX_CALLBACK drop_rx_cb);

int32
sys_usw_diag_drop_pkt_sync(ctc_pkt_rx_t* p_pkt_rx);

uint32
_sys_usw_diag_drop_hash_make(sys_usw_diag_drop_info_t* node)
{
    return ctc_hash_caculate(sizeof(node->lport) + sizeof(node->reason), &(node->lport));
}

bool
_sys_usw_diag_drop_hash_cmp(sys_usw_diag_drop_info_t* stored_node, sys_usw_diag_drop_info_t* lkup_node)
{
    if (!stored_node || !lkup_node)
    {
        return FALSE;
    }
    if ((stored_node->lport == lkup_node->lport)
        && (stored_node->reason == lkup_node->reason))
    {
        return TRUE;
    }
    return FALSE;
}

STATIC int32
_sys_usw_diag_free_drop_hash_data(void* node_data, void* user_data)
{
    sys_usw_diag_drop_info_t* info = (sys_usw_diag_drop_info_t*)node_data;
    if (info)
    {
        mem_free(info);
    }
    return 1;
}

STATIC sys_usw_diag_drop_info_t*
_sys_usw_diag_drop_hash_lkup(uint8 lchip, uint16 lport, uint16 reason)
{
    sys_usw_diag_drop_info_t info;
    sal_memset(&info, 0, sizeof(sys_usw_diag_drop_info_t));
    info.lport = lport;
    info.reason = reason;
    return ctc_hash_lookup(p_usw_diag_master[lchip]->drop_hash, &info);
}

STATIC int32
_sys_usw_diag_drop_hash_add(uint8 lchip, sys_usw_diag_drop_info_t* info)
{
    sys_usw_diag_drop_info_t* node = NULL;

    node = ctc_hash_lookup(p_usw_diag_master[lchip]->drop_hash, info);
    if (NULL == node)
    {
        node = mem_malloc(MEM_DIAG_MODULE, sizeof(sys_usw_diag_drop_info_t));
        if (NULL == node)
        {
            SYS_DIAG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Add Drop Hash Info Fail. No Memory!\n");
            return CTC_E_NO_MEMORY;
        }
        sal_memcpy(node, info,sizeof(sys_usw_diag_drop_info_t));
        if (NULL == ctc_hash_insert(p_usw_diag_master[lchip]->drop_hash, node))
        {
            mem_free(node);
            SYS_DIAG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Insert Drop Hash Info Fail. No Memory!\n");
            return CTC_E_NO_MEMORY;
        }
    }
    else
    {
        sal_memcpy(node, info,sizeof(sys_usw_diag_drop_info_t));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_diag_drop_hash_remove(uint8 lchip, sys_usw_diag_drop_info_t* info)
{
    sys_usw_diag_drop_info_t* info_out = ctc_hash_remove(p_usw_diag_master[lchip]->drop_hash, (void*)info);
    if (NULL == info_out)
    {
        SYS_DIAG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Remove Drop Hash Info Fail. Entry not exist!\n");
        return CTC_E_NOT_EXIST;
    }
    mem_free(info);
    return CTC_E_NONE;
}

int32
sys_usw_diag_drop_hash_update_count(uint8 lchip, uint16 lport, uint16 reason, uint32 count)
{
    sys_usw_diag_drop_info_t info_temp;
    sys_usw_diag_drop_info_t* info = NULL;
    if (count == 0)
    {
        return CTC_E_NONE;
    }
    info = _sys_usw_diag_drop_hash_lkup(lchip, lport, reason);
    if (info != NULL)
    {
        sal_memcpy(&info_temp, info, sizeof(sys_usw_diag_drop_info_t));
        info_temp.count += count;
    }
    else
    {
        sal_memset(&info_temp, 0, sizeof(sys_usw_diag_drop_info_t));
        info_temp.lport = lport;
        info_temp.reason = reason;
        info_temp.count = count;
    }
    CTC_ERROR_RETURN(_sys_usw_diag_drop_hash_add(lchip, &info_temp));
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_diag_drop_pkt_clear_buf(uint8 lchip)
{
    ctc_slistnode_t* node = NULL;
    ctc_slistnode_t* next_node = NULL;

    CTC_SLIST_LOOP_DEL(p_usw_diag_master[lchip]->drop_pkt_list, node, next_node)
    {
        ctc_slist_delete_node(p_usw_diag_master[lchip]->drop_pkt_list, node);
        mem_free(((sys_usw_diag_drop_pkt_node_t*)node)->buf);
        mem_free(node);
    }
    return CTC_E_NONE;
}
STATIC int32
_sys_usw_diag_pkt_trace_watch_key_check(uint8 lchip, ctc_diag_pkt_trace_t* p_trace)
{
    if ((p_trace->mode != CTC_DIAG_TRACE_MODE_USER)
        && (p_trace->mode != CTC_DIAG_TRACE_MODE_NETWORK))
    {
        SYS_DIAG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Packet Trace Mode Error! \n");
        return CTC_E_INVALID_PARAM;
    }

    if (p_trace->watch_point >= CTC_DIAG_TRACE_POINT_MAX)
    {
        SYS_DIAG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Watch Point Error! \n");
        return CTC_E_INVALID_PARAM;
    }

    if (CTC_BMP_ISSET(p_trace->watch_key.key_mask_bmp, CTC_DIAG_WATCH_KEY_MASK_CHANNEL)
        && (p_trace->watch_key.channel >= MCHIP_CAP(SYS_CAP_CHANNEL_NUM)))
    {
        SYS_DIAG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Watch Key Channel Invalid! \n");
        return CTC_E_INVALID_PARAM;
    }
    if (CTC_BMP_ISSET(p_trace->watch_key.key_mask_bmp, CTC_DIAG_WATCH_KEY_MASK_SRC_LPORT)
        && (p_trace->watch_key.src_lport >= MCHIP_CAP(SYS_CAP_SPEC_MAX_PHY_PORT_NUM)))
    {
        SYS_DIAG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Watch Key SrcLport Invalid! \n");
        return CTC_E_INVALID_PARAM;
    }

    if (CTC_BMP_ISSET(p_trace->watch_key.key_mask_bmp, CTC_DIAG_WATCH_KEY_MASK_MC_GROUP)
        && CTC_BMP_ISSET(p_trace->watch_key.key_mask_bmp, CTC_DIAG_WATCH_KEY_MASK_DST_GPORT))
    {
        SYS_DIAG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Watch Key DstGport & Mcast Group Should not Coexist! \n");
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_diag_trigger_pkt_trace(uint8 lchip, ctc_diag_pkt_trace_t* p_trace)
{
    int32 ret = CTC_E_NONE;
    ctc_pkt_tx_t*  pkt_tx = NULL;

    if (MCHIP_API(lchip)->diag_set_dbg_session)
    {
        CTC_ERROR_RETURN(MCHIP_API(lchip)->diag_set_dbg_session(lchip, p_trace));
    }

    if (p_trace->mode == CTC_DIAG_TRACE_MODE_USER)
    {
        /* Packet Tx */
        pkt_tx = mem_malloc(MEM_DIAG_MODULE, sizeof(ctc_pkt_tx_t));
        if (NULL == pkt_tx)
        {
            SYS_DIAG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "No memory! \n");
            return CTC_E_NO_MEMORY;
        }
        SYS_DIAG_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Packet Trace Mode: User! Len: %d Src_port:0x%.4x\n", p_trace->pkt.user.len, p_trace->pkt.user.src_port);
        sal_memset(pkt_tx, 0, sizeof(ctc_pkt_tx_t));
        pkt_tx->lchip = lchip;
        pkt_tx->mode = CTC_PKT_MODE_DMA;
        pkt_tx->skb.data = p_trace->pkt.user.pkt_buf;
        pkt_tx->skb.len = p_trace->pkt.user.len;
        pkt_tx->tx_info.flags = CTC_PKT_FLAG_SRC_PORT_VALID | CTC_PKT_FLAG_INGRESS_MODE | CTC_PKT_FLAG_DIAG_PKT;
        pkt_tx->tx_info.src_port = p_trace->pkt.user.src_port;
        pkt_tx->tx_info.dest_gport = p_trace->pkt.user.src_port;
        ret = sys_usw_packet_tx(lchip, pkt_tx);
        mem_free(pkt_tx);
        return ret;
    }
    else if (MCHIP_API(lchip)->diag_set_dbg_pkt)
    {
        /* Network Key */
        SYS_DIAG_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Packet Trace Mode: Network!\n");
        CTC_ERROR_RETURN(MCHIP_API(lchip)->diag_set_dbg_pkt(lchip, p_trace));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_diag_drop_hash_traverse_cb(sys_usw_diag_drop_info_t* info, ctc_diag_drop_t* p_drop)
{
    CTC_PTR_VALID_CHECK(info);
    CTC_PTR_VALID_CHECK(p_drop);

    if (p_drop->oper_type == CTC_DIAG_DROP_OPER_TYPE_GET_PORT_BMP)
    {
        CTC_BMP_SET(p_drop->u.port_bmp, info->lport);
    }
    else if (p_drop->oper_type == CTC_DIAG_DROP_OPER_TYPE_GET_REASON_BMP)
    {
        if (p_drop->lport == info->lport)
        {
            CTC_BMP_SET(p_drop->u.reason_bmp, info->reason);
        }
    }
    return 0;
}

STATIC int32
_sys_usw_diag_get_drop_info(uint8 lchip, ctc_diag_drop_t* p_drop)
{
    uint32 cp_len = 0;
    uint32 loop = 0;
    uint32 clear_cnt = 0;
    sys_usw_diag_drop_pkt_node_t* pkt_node = NULL;
    ctc_slistnode_t* node = NULL;
    ctc_slistnode_t* next_node = NULL;
    sys_usw_diag_drop_info_t* info = NULL;

    SYS_DIAG_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Oper_type:%d buffer_clear:%d\n", p_drop->oper_type, p_drop->with_clear);

    if (p_drop->oper_type > CTC_DIAG_DROP_OPER_TYPE_GET_DETAIL_INFO)
    {
        SYS_DIAG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Oper_type Invalid! \n");
        return CTC_E_INTR_INVALID_PARAM;
    }
    if (p_drop->oper_type != CTC_DIAG_DROP_OPER_TYPE_GET_PORT_BMP)
    {
        if (p_drop->lport >= MCHIP_CAP(SYS_CAP_SPEC_MAX_PHY_PORT_NUM))
        {
            SYS_DIAG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Lport Invalid! \n");
            return CTC_E_INVALID_CONFIG;
        }
    }
    switch (p_drop->oper_type)
    {
        case CTC_DIAG_DROP_OPER_TYPE_GET_PORT_BMP:
        case CTC_DIAG_DROP_OPER_TYPE_GET_REASON_BMP:
            if (MCHIP_API(lchip)->diag_get_drop_info)
            {
                CTC_ERROR_RETURN(MCHIP_API(lchip)->diag_get_drop_info(lchip, (void*)p_drop));
            }
            ctc_hash_traverse(p_usw_diag_master[lchip]->drop_hash, (hash_traversal_fn)_sys_usw_diag_drop_hash_traverse_cb, p_drop);
            break;
        case CTC_DIAG_DROP_OPER_TYPE_GET_DETAIL_INFO:
            if (MCHIP_API(lchip)->diag_get_drop_info)
            {
                CTC_ERROR_RETURN(MCHIP_API(lchip)->diag_get_drop_info(lchip, (void*)p_drop));
            }
            info = _sys_usw_diag_drop_hash_lkup(lchip, p_drop->lport, p_drop->reason);
            if (NULL == info)
            {
                SYS_DIAG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Buffer is not exist! lport=%d reason=%d \n", p_drop->lport, p_drop->reason);
                return CTC_E_NOT_EXIST;
            }
            p_drop->u.info.real_buffer_count = 0;
            p_drop->u.info.count = info->count;

            CTC_SLIST_LOOP_DEL(p_usw_diag_master[lchip]->drop_pkt_list, node, next_node)
            {
                pkt_node =  _ctc_container_of(node, sys_usw_diag_drop_pkt_node_t, head);
                if ((pkt_node->lport != p_drop->lport) || (pkt_node->reason != p_drop->reason))
                {
                    continue;
                }
                if ((0 == p_drop->u.info.buffer_count)
                    || (NULL == p_drop->u.info.buffer)
                    || (loop >= p_drop->u.info.buffer_count))
                {
                    loop++;
                    continue;
                }

                p_drop->u.info.buffer[loop].real_len = pkt_node->len;
                p_drop->u.info.buffer[loop].real_len = pkt_node->len;

                cp_len = (pkt_node->len > p_drop->u.info.buffer[loop].len) ?
                                            p_drop->u.info.buffer[loop].len : pkt_node->len;
                sal_memcpy(&p_drop->u.info.buffer[loop].timestamp, &pkt_node->timestamp, sizeof(sal_time_t));
                sal_memcpy(p_drop->u.info.buffer[loop].pkt_buf, pkt_node->buf, cp_len * sizeof(uint8));
                if (p_drop->with_clear
                    && (p_drop->u.info.buffer[loop].len >= pkt_node->len))
                {
                    ctc_slist_delete_node(p_usw_diag_master[lchip]->drop_pkt_list, node);
                    mem_free(pkt_node->buf);
                    mem_free(pkt_node);
                    clear_cnt++;
                }
                loop++;
            }
            p_drop->u.info.real_buffer_count = loop;
            if (p_drop->with_clear && (clear_cnt == loop))
            {
                /* clear hash data */
                _sys_usw_diag_drop_hash_remove(lchip, info);
            }
            break;
        default:
            break;
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_diag_drop_pkt_sync(uint8 lchip, ctc_pkt_rx_t* p_pkt_rx)
{
    uint16 lport = 0;
    uint16 reason = 0;
    uint32 hash_seed = 0;
    uint32 pkt_len = 0;
    uint32 hdr_len = 0;
    uint32 store_len = 0;
    uint32 hash_calc_len = 0;
    ctc_slistnode_t*       node = NULL;
    sys_usw_diag_drop_pkt_node_t* pkt_node = NULL;

    if (!p_pkt_rx->buf_count)
    {
        return CTC_E_NONE;
    }

    lport = p_pkt_rx->rx_info.lport;
    if (p_pkt_rx->rx_info.ttl & 0x40)
    {
        /*EPE*/
        reason = (p_pkt_rx->rx_info.ttl & 0x3F)  + SYS_DIAG_DROP_REASON_BASE_EPE;
    }
    else
    {
        /*IPE*/
        reason = p_pkt_rx->rx_info.ttl;
    }
    hdr_len = p_pkt_rx->eth_hdr_len + p_pkt_rx->pkt_hdr_len + p_pkt_rx->stk_hdr_len;
    pkt_len = p_pkt_rx->pkt_buf[0].len - hdr_len - 4; /*without CRC Length (4 bytes)*/
    store_len = (pkt_len > p_usw_diag_master[lchip]->drop_pkt_store_len) ? p_usw_diag_master[lchip]->drop_pkt_store_len : pkt_len;

    /* Packet Report */
    if (p_usw_diag_master[lchip]->drop_pkt_report_cb)
    {
        uint8 gchip = 0;
        int32 ret = CTC_E_NONE;
        ctc_diag_drop_info_buffer_t pkt_buf;
        sal_memset(&pkt_buf, 0, sizeof(pkt_buf));
        sys_usw_get_gchip_id(lchip, &gchip);
        pkt_buf.len = store_len;
        pkt_buf.real_len = store_len;
        pkt_buf.pkt_buf = p_pkt_rx->pkt_buf[0].data + hdr_len;
        sal_time(&pkt_buf.timestamp);
        ret = p_usw_diag_master[lchip]->drop_pkt_report_cb(CTC_MAP_LPORT_TO_GPORT(gchip, lport), reason, &pkt_buf);
        if (ret < 0)
        {
            SYS_DIAG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Drop Packet Report Error, error=%d!\n", ret);
        }
    }
    if (NULL == p_usw_diag_master[lchip]->drop_pkt_list)
    {
        return CTC_E_NONE;
    }

    /* hash key : packet */
    hash_calc_len = (store_len > p_usw_diag_master[lchip]->drop_pkt_hash_len) ? p_usw_diag_master[lchip]->drop_pkt_hash_len : store_len;
    hash_seed = ctc_hash_caculate(hash_calc_len, (void*)(p_pkt_rx->pkt_buf[0].data + hdr_len));
    CTC_SLIST_LOOP(p_usw_diag_master[lchip]->drop_pkt_list, node)
    {
        pkt_node = (sys_usw_diag_drop_pkt_node_t*)node;
        if ((hash_seed == pkt_node->hash_seed)
            && (store_len == pkt_node->len)
            && (lport == pkt_node->lport)
            && (reason == pkt_node->reason))
        {
            /* means find the same packet */
            return CTC_E_NONE;
        }
    }
    if ((p_usw_diag_master[lchip]->drop_pkt_list->count >= p_usw_diag_master[lchip]->drop_pkt_store_cnt)
        && (!p_usw_diag_master[lchip]->drop_pkt_overwrite))
    {
        /* buffer exceed, and would not cover the oldest buffer */
        return CTC_E_NONE;
    }
    pkt_node = mem_malloc(MEM_DIAG_MODULE, sizeof(sys_usw_diag_drop_pkt_node_t));
    if (NULL == pkt_node)
    {
        SYS_DIAG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Drop Packet Sync Error, No Memory!\n");
        return CTC_E_NO_MEMORY;
    }
    sal_memset(pkt_node, 0, sizeof(sys_usw_diag_drop_pkt_node_t));
    pkt_node->hash_seed = hash_seed;
    pkt_node->lport = lport;
    pkt_node->reason = reason;
    pkt_node->len = store_len;
    sal_time(&pkt_node->timestamp);
    pkt_node->buf = mem_malloc(MEM_DIAG_MODULE, store_len);
    if (NULL == pkt_node->buf)
    {
        mem_free(pkt_node);
        SYS_DIAG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Drop Packet Sync Error, No Memory!\n");
        return CTC_E_NO_MEMORY;
    }
    sal_memcpy(pkt_node->buf, p_pkt_rx->pkt_buf[0].data + hdr_len, store_len);
    if (p_usw_diag_master[lchip]->drop_pkt_list->count >= p_usw_diag_master[lchip]->drop_pkt_store_cnt)
    {
        sys_usw_diag_drop_pkt_node_t* node_temp = NULL;
        node = p_usw_diag_master[lchip]->drop_pkt_list->head;
        node_temp = (sys_usw_diag_drop_pkt_node_t*)node;
        /* buffer exceed, and cover the oldest buffer */
        ctc_slist_delete_node(p_usw_diag_master[lchip]->drop_pkt_list, node);
        /* free hash data */
        mem_free(node_temp->buf);
        mem_free(node_temp);
    }
    ctc_slist_add_tail(p_usw_diag_master[lchip]->drop_pkt_list, &pkt_node->head);
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_diag_drop_pkt_redirect_tocpu_en(uint8 lchip, uint8 enable)
{
    ctc_qos_shape_t shape;
    ctc_qos_queue_cfg_t que_cfg;
    uint32 value = 0;
    uint32 cmd = 0;

    sal_memset(&que_cfg, 0, sizeof(ctc_qos_queue_cfg_t));
    sal_memset(&shape, 0, sizeof(ctc_qos_shape_t));
    if (enable)
    {
        CTC_ERROR_RETURN(sys_usw_packet_register_drop_rx_cb(lchip, sys_usw_diag_drop_pkt_sync));
        /*IPE*/
        value = 0x1c;/* all discard*/
        cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_discardPacketLogType_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        value = 1;
        cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_discardPacketLogEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        /*EPE*/
        value = 1;/* all discard, Tsingma = 1 */
        cmd = DRV_IOW(EpeHeaderEditCtl_t, EpeHeaderEditCtl_discardPacketLogType_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        value = 1;
        cmd = DRV_IOW(EpeHeaderEditCtl_t, EpeHeaderEditCtl_discardPacketLogEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        value = 1;
        cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_dropPktLogWithDiscardType_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        cmd = DRV_IOW(EpeHeaderEditCtl_t, EpeHeaderEditCtl_dropPktLogWithDiscardType_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        cmd = DRV_IOR(EpeHeaderEditCtl_t, EpeHeaderEditCtl_logOnDiscard_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        /* Set discardPacketLogEn */
        CTC_BIT_SET(value, 8);
        cmd = DRV_IOW(EpeHeaderEditCtl_t, EpeHeaderEditCtl_logOnDiscard_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

        /*CTC_PKT_CPU_REASON_DIAG_DISCARD_PKT dest-to Local-CPU*/
        que_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_REASON_DEST;
        que_cfg.value.reason_dest.cpu_reason = CTC_PKT_CPU_REASON_DIAG_DISCARD_PKT;
        que_cfg.value.reason_dest.dest_type = CTC_PKT_CPU_REASON_TO_LOCAL_CPU;
        CTC_ERROR_RETURN(sys_usw_qos_set_queue(lchip, &que_cfg));

        /* Shape */
        shape.type = CTC_QOS_SHAPE_QUEUE;
        shape.shape.queue_shape.enable = 1;
        shape.shape.queue_shape.queue.queue_type = CTC_QUEUE_TYPE_EXCP_CPU;
        shape.shape.queue_shape.pir = 1000;/* 1M */
        shape.shape.queue_shape.pbs = 0xFFFFFFFF;
        shape.shape.queue_shape.queue.cpu_reason = CTC_PKT_CPU_REASON_DIAG_DISCARD_PKT;
        CTC_ERROR_RETURN(sys_usw_qos_set_shape(lchip, &shape));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_packet_register_drop_rx_cb(lchip, NULL));
        /*IPE*/
        cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_discardPacketLogType_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_discardPacketLogEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        /*EPE*/
        cmd = DRV_IOW(EpeHeaderEditCtl_t, EpeHeaderEditCtl_discardPacketLogType_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        cmd = DRV_IOW(EpeHeaderEditCtl_t, EpeHeaderEditCtl_discardPacketLogEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_dropPktLogWithDiscardType_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        cmd = DRV_IOW(EpeHeaderEditCtl_t, EpeHeaderEditCtl_dropPktLogWithDiscardType_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        cmd = DRV_IOR(EpeHeaderEditCtl_t, EpeHeaderEditCtl_logOnDiscard_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        /* Set discardPacketLogEn */
        CTC_BIT_UNSET(value, 8);
        cmd = DRV_IOW(EpeHeaderEditCtl_t, EpeHeaderEditCtl_logOnDiscard_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

        shape.type = CTC_QOS_SHAPE_QUEUE;
        shape.shape.queue_shape.enable = 0;
        shape.shape.queue_shape.queue.queue_type = CTC_QUEUE_TYPE_EXCP_CPU;
        shape.shape.queue_shape.queue.cpu_reason = CTC_PKT_CPU_REASON_DIAG_DISCARD_PKT;
        CTC_ERROR_RETURN(sys_usw_qos_set_shape(lchip, &shape));

        que_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_REASON_DEST;
        que_cfg.value.reason_dest.cpu_reason = CTC_PKT_CPU_REASON_DIAG_DISCARD_PKT;
        que_cfg.value.reason_dest.dest_type = CTC_PKT_CPU_REASON_TO_DROP;
        CTC_ERROR_RETURN(sys_usw_qos_set_queue(lchip, &que_cfg));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_diag_drop_pkt_set_config(uint8 lchip, ctc_diag_drop_pkt_config_t* p_cfg)
{
    if (p_cfg->flags & CTC_DIAG_DROP_PKT_HASH_CALC_LEN)
    {
        p_usw_diag_master[lchip]->drop_pkt_hash_len = p_cfg->hash_calc_len;
    }
    if (p_cfg->flags & CTC_DIAG_DROP_PKT_STORE_LEN)
    {
        p_usw_diag_master[lchip]->drop_pkt_store_len = p_cfg->storage_len;
    }
    if (p_cfg->flags & CTC_DIAG_DROP_PKT_STORE_CNT)
    {
        p_usw_diag_master[lchip]->drop_pkt_store_cnt = p_cfg->storage_cnt;
    }
    if (p_cfg->flags & CTC_DIAG_DROP_PKT_OVERWRITE)
    {
        p_usw_diag_master[lchip]->drop_pkt_overwrite = p_cfg->overwrite ? 1 : 0;
    }
    if (p_cfg->flags & CTC_DIAG_DROP_PKT_CLEAR)
    {
        _sys_usw_diag_drop_pkt_clear_buf(lchip);
    }

    if (p_cfg->flags & CTC_DIAG_DROP_PKT_STORE_EN)
    {
        if (p_cfg->storage_en)
        {
            if (p_usw_diag_master[lchip]->drop_pkt_list)
            {
                return CTC_E_NONE;
            }
            p_usw_diag_master[lchip]->drop_pkt_list = ctc_slist_new();
            if (NULL == p_usw_diag_master[lchip]->drop_pkt_list)
            {
                SYS_DIAG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "No Memory! \n");
                return CTC_E_NO_MEMORY;
            }
            if (p_usw_diag_master[lchip]->drop_pkt_report_cb == NULL)
            {
                int32 ret = 0;
                ret = _sys_usw_diag_drop_pkt_redirect_tocpu_en(lchip, 1);
                if (ret < 0)
                {
                    ctc_slist_free(p_usw_diag_master[lchip]->drop_pkt_list);
                }
            }
        }
        else
        {
            if (p_usw_diag_master[lchip]->drop_pkt_list == NULL)
            {
                return CTC_E_NONE;
            }
            _sys_usw_diag_drop_pkt_clear_buf(lchip);
            ctc_slist_free(p_usw_diag_master[lchip]->drop_pkt_list);
            p_usw_diag_master[lchip]->drop_pkt_list = NULL;
            if (p_usw_diag_master[lchip]->drop_pkt_report_cb == NULL)
            {
                CTC_ERROR_RETURN(_sys_usw_diag_drop_pkt_redirect_tocpu_en(lchip, 0));
            }
        }
    }

    if (p_cfg->flags & CTC_DIAG_DROP_PKT_REPORT_CALLBACK)
    {
        p_usw_diag_master[lchip]->drop_pkt_report_cb = p_cfg->report_cb;
        if (p_usw_diag_master[lchip]->drop_pkt_list == NULL)
        {
            CTC_ERROR_RETURN(_sys_usw_diag_drop_pkt_redirect_tocpu_en(lchip, p_cfg->report_cb ? 1 : 0));
        }
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_diag_drop_pkt_get_config(uint8 lchip, ctc_diag_drop_pkt_config_t* p_cfg)
{
    p_cfg->hash_calc_len = p_usw_diag_master[lchip]->drop_pkt_hash_len;
    CTC_SET_FLAG(p_cfg->flags, CTC_DIAG_DROP_PKT_HASH_CALC_LEN);
    p_cfg->storage_len = p_usw_diag_master[lchip]->drop_pkt_store_len;
    CTC_SET_FLAG(p_cfg->flags, CTC_DIAG_DROP_PKT_HASH_CALC_LEN);
    p_cfg->storage_cnt = p_usw_diag_master[lchip]->drop_pkt_store_cnt;
    CTC_SET_FLAG(p_cfg->flags, CTC_DIAG_DROP_PKT_STORE_CNT);
    p_cfg->overwrite = p_usw_diag_master[lchip]->drop_pkt_overwrite ? TRUE : FALSE;
    CTC_SET_FLAG(p_cfg->flags, CTC_DIAG_DROP_PKT_OVERWRITE);
    p_cfg->storage_en = p_usw_diag_master[lchip]->drop_pkt_list ? TRUE : FALSE;
    CTC_SET_FLAG(p_cfg->flags, CTC_DIAG_DROP_PKT_STORE_EN);
    p_cfg->report_cb = p_usw_diag_master[lchip]->drop_pkt_report_cb;
    CTC_SET_FLAG(p_cfg->flags, CTC_DIAG_DROP_PKT_REPORT_CALLBACK);
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_diag_set_property(uint8 lchip,ctc_diag_property_t prop, void* p_value)
{
    switch (prop)
    {
        case CTC_DIAG_PROP_DROP_PKT_CONFIG:
            CTC_ERROR_RETURN(_sys_usw_diag_drop_pkt_set_config(lchip, (ctc_diag_drop_pkt_config_t*)p_value));
        default:
            break;
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_diag_get_property(uint8 lchip,ctc_diag_property_t prop, void* p_value)
{
    switch (prop)
    {
        case CTC_DIAG_PROP_DROP_PKT_CONFIG:
            CTC_ERROR_RETURN(_sys_usw_diag_drop_pkt_get_config(lchip, (ctc_diag_drop_pkt_config_t*)p_value));
        default:
            break;
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_diag_drop_hash_traverse_dump(sys_usw_diag_drop_info_t* info, sys_traverse_t*  p_para)
{
    CTC_PTR_VALID_CHECK(p_para)
    CTC_PTR_VALID_CHECK(info);
    SYS_DUMP_DB_LOG((sal_file_t)p_para->data, "%.4d   %.4d    %10d\n", info->lport, info->reason, info->count);
    return 0;
}

STATIC int32
_sys_usw_diag_dump_db(uint8 lchip, sal_file_t p_f,ctc_global_dump_db_t* p_dump_param)
{
    sys_traverse_t  para;

    SYS_USW_DIAG_INIT_CHECK(lchip);
    DIAG_LOCK;
    SYS_DUMP_DB_LOG(p_f, "%s\n", "# DIAG");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "{");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "Master config:");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%-36s: %s\n","Storage drop packet", p_usw_diag_master[lchip]->drop_pkt_list ? "Enable" : "Disable");
    SYS_DUMP_DB_LOG(p_f, "%-36s: %s\n","Report drop packet", p_usw_diag_master[lchip]->drop_pkt_report_cb ? "Enable" : "Disable");
    SYS_DUMP_DB_LOG(p_f, "%-36s: %s\n","Cover exceed", p_usw_diag_master[lchip]->drop_pkt_overwrite ? "Enable" : "Disable");
    SYS_DUMP_DB_LOG(p_f, "%-36s: %u\n","Hash calc length", p_usw_diag_master[lchip]->drop_pkt_hash_len);
    SYS_DUMP_DB_LOG(p_f, "%-36s: %u\n","Storage packet count", p_usw_diag_master[lchip]->drop_pkt_store_cnt);
    SYS_DUMP_DB_LOG(p_f, "%-36s: %u\n","Storage packet length", p_usw_diag_master[lchip]->drop_pkt_store_len);
    SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------------------");

    if (p_usw_diag_master[lchip]->drop_hash->count)
    {
        SYS_DUMP_DB_LOG(p_f, "\n");
        SYS_DUMP_DB_LOG(p_f, "%s\n", "##Drop Statistics");
        SYS_DUMP_DB_LOG(p_f, "%-5s  %-6s  %-10s\n", "Lport", "Reason", "Statistics");
        SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------");
        sal_memset(&para, 0, sizeof(sys_traverse_t));
        para.data = (void*)p_f;
        ctc_hash_traverse(p_usw_diag_master[lchip]->drop_hash, (hash_traversal_fn)_sys_usw_diag_drop_hash_traverse_dump, &para);
        SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------");
    }
    if (p_usw_diag_master[lchip]->drop_pkt_list && p_usw_diag_master[lchip]->drop_pkt_list->count)
    {
        uint32 loop = 0;
        uint32 index = 0;
        char* p_tm = NULL;
        ctc_slistnode_t* node = NULL;
        sys_usw_diag_drop_pkt_node_t* pkt_node = NULL;
        SYS_DUMP_DB_LOG(p_f, "\n");
        SYS_DUMP_DB_LOG(p_f, "%s\n", "##Drop Packet");
        SYS_DUMP_DB_LOG(p_f, "%s\n", "------------------------------------------------------");
        CTC_SLIST_LOOP(p_usw_diag_master[lchip]->drop_pkt_list, node)
        {
            pkt_node = (sys_usw_diag_drop_pkt_node_t*)node;
            SYS_DUMP_DB_LOG(p_f,"%-20s:%d\n", "Index", loop);
            SYS_DUMP_DB_LOG(p_f,"%-20s:%d\n", "Lport", pkt_node->lport);
            SYS_DUMP_DB_LOG(p_f,"%-20s:%d\n", "Reason", pkt_node->reason);
            p_tm = sal_ctime(&pkt_node->timestamp);
            SYS_DUMP_DB_LOG(p_f,"%-20s:0x%x\n", "Hashseed", pkt_node->hash_seed);
            SYS_DUMP_DB_LOG(p_f,"%-20s:%s", "Timestamp", p_tm);
            SYS_DUMP_DB_LOG(p_f, "%s\n", "-------------------------------------------------");
            for (index = 0; index < pkt_node->len; index++)
            {
                if (index % 16 == 0)
                {
                    SYS_DUMP_DB_LOG(p_f, "0x%.4x:  ", index / 16 * 16);
                }
                SYS_DUMP_DB_LOG(p_f, "%.2x", pkt_node->buf[index]);
                if (index % 2)
                {
                    SYS_DUMP_DB_LOG(p_f," ");
                }
                if ((index + 1) % 16 == 0)
                {
                    SYS_DUMP_DB_LOG(p_f,"\n");
                }
            }
            SYS_DUMP_DB_LOG(p_f, "\n\n");
            loop++;
        }
    }
    DIAG_UNLOCK;
    SYS_DUMP_DB_LOG(p_f, "%s\n", "}");
    return CTC_E_NONE;
}


#define ______INTERNAL_API______

int32
sys_usw_diag_drop_pkt_sync(ctc_pkt_rx_t* p_pkt_rx)
{
    int32 ret = CTC_E_NONE;
    uint8 lchip = 0;
    CTC_PTR_VALID_CHECK(p_pkt_rx);
    lchip = p_pkt_rx->lchip;
    SYS_USW_DIAG_INIT_CHECK(lchip);
    DIAG_LOCK;
    ret = _sys_usw_diag_drop_pkt_sync(lchip, p_pkt_rx);
    DIAG_UNLOCK;
    return ret;
}


#define ______SYS_API______

int32
sys_usw_diag_trigger_pkt_trace(uint8 lchip, ctc_diag_pkt_trace_t* p_trace)
{
    int32 ret = CTC_E_NONE;
    SYS_USW_DIAG_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_trace);
    CTC_ERROR_RETURN(_sys_usw_diag_pkt_trace_watch_key_check(lchip, p_trace));
    DIAG_LOCK;
    ret = _sys_usw_diag_trigger_pkt_trace(lchip, p_trace);
    DIAG_UNLOCK;
    return ret;
}

int32
sys_usw_diag_get_pkt_trace(uint8 lchip, ctc_diag_pkt_trace_result_t* p_rslt)
{
    int32 ret = CTC_E_NONE;

    SYS_USW_DIAG_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_rslt);

    DIAG_LOCK;
    if (MCHIP_API(lchip)->diag_get_pkt_trace)
    {
        ret = MCHIP_API(lchip)->diag_get_pkt_trace(lchip, (void*)p_rslt);
    }
    DIAG_UNLOCK;
    return ret;
}

int32
sys_usw_diag_get_drop_info(uint8 lchip, ctc_diag_drop_t* p_drop)
{
    int32 ret = CTC_E_NONE;
    SYS_USW_DIAG_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_drop);

    DIAG_LOCK;
    ret = _sys_usw_diag_get_drop_info(lchip, p_drop);
    DIAG_UNLOCK;
    return ret;
}


int32
sys_usw_diag_set_property(uint8 lchip, ctc_diag_property_t prop, void* p_value)
{
    int32 ret = CTC_E_NONE;
    SYS_USW_DIAG_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_value);
    if (prop >= CTC_DIAG_PROP_MAX)
    {
        return CTC_E_INVALID_PARAM;
    }
    DIAG_LOCK;
    ret = _sys_usw_diag_set_property(lchip, prop, p_value);
    DIAG_UNLOCK;
    return ret;
}

int32
sys_usw_diag_get_property(uint8 lchip, ctc_diag_property_t prop, void* p_value)
{
    int32 ret = CTC_E_NONE;
    SYS_USW_DIAG_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_value);
    if (prop >= CTC_DIAG_PROP_MAX)
    {
        return CTC_E_INVALID_PARAM;
    }
    DIAG_LOCK;
    ret = _sys_usw_diag_get_property(lchip, prop, p_value);
    DIAG_UNLOCK;
    return ret;
}

int32
sys_usw_diag_init(uint8 lchip, void* init_cfg)
{
    int32 ret = CTC_E_NONE;

    if (p_usw_diag_master[lchip])
    {
        return CTC_E_NONE;
    }
    p_usw_diag_master[lchip] = (sys_usw_diag_t*)mem_malloc(MEM_DIAG_MODULE, sizeof(sys_usw_diag_t));
    if ( NULL == p_usw_diag_master[lchip] )
    {
        SYS_DIAG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Diag Init Error. No Memory! \n");
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_usw_diag_master[lchip], 0, sizeof(sys_usw_diag_t));

    ret = sal_mutex_create(&(p_usw_diag_master[lchip]->diag_mutex));
    if (ret || !(p_usw_diag_master[lchip]->diag_mutex))
    {
        SYS_DIAG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No resource in ASIC. Mutex Create Failed! \n");
        ret = CTC_E_NO_RESOURCE;
        goto error_back;
    }
    p_usw_diag_master[lchip]->drop_hash = ctc_hash_create(64, 16,
                                        (hash_key_fn)_sys_usw_diag_drop_hash_make,
                                        (hash_cmp_fn)_sys_usw_diag_drop_hash_cmp);
    if (NULL == p_usw_diag_master[lchip]->drop_hash)
    {
        SYS_DIAG_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Diag Init Error. No Memory! \n");
        ret = CTC_E_NO_MEMORY;
        goto error_back;
    }

    p_usw_diag_master[lchip]->drop_pkt_overwrite = 1;
    p_usw_diag_master[lchip]->drop_pkt_hash_len = 32;
    p_usw_diag_master[lchip]->drop_pkt_store_cnt = 128;
    p_usw_diag_master[lchip]->drop_pkt_store_len = 256;
    CTC_ERROR_RETURN(sys_usw_dump_db_register_cb(lchip, CTC_FEATURE_DIAG, _sys_usw_diag_dump_db));
    return CTC_E_NONE;

error_back:
    sys_usw_diag_deinit(lchip);
    return ret;
}

int32
sys_usw_diag_deinit(uint8 lchip)
{
    LCHIP_CHECK(lchip);
    if (NULL == p_usw_diag_master[lchip])
    {
        return CTC_E_NONE;
    }

    if (p_usw_diag_master[lchip]->drop_hash)
    {
        ctc_hash_traverse_remove(p_usw_diag_master[lchip]->drop_hash, (hash_traversal_fn)_sys_usw_diag_free_drop_hash_data, NULL);
        ctc_hash_free(p_usw_diag_master[lchip]->drop_hash);
    }
    if (p_usw_diag_master[lchip]->diag_mutex)
    {
        sal_mutex_destroy(p_usw_diag_master[lchip]->diag_mutex);
    }
    _sys_usw_diag_drop_pkt_clear_buf(lchip);
    ctc_slist_free(p_usw_diag_master[lchip]->drop_pkt_list);
    mem_free(p_usw_diag_master[lchip]);
    return CTC_E_NONE;
}

