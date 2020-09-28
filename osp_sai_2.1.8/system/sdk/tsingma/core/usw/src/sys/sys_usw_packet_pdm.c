/**
 @file sys_usw_inband.c

 @date 2018-12-14

 @version v5.5
*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "sal.h"
#include "ctc_error.h"
#include "ctc_debug.h"
#include "ctc_interrupt.h"
#include "ctc_packet.h"
#include "ctc_warmboot.h"

#include "sys_usw_common.h"
#include "sys_usw_port.h"
#include "sys_usw_chip.h"
#include "sys_usw_l3if.h"
#include "sys_usw_scl_api.h"
#include "sys_usw_vlan.h"
#include "sys_usw_vlan_mapping.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_internal_port.h"
#include "sys_usw_qos_api.h"
#include "sys_usw_chip.h"
#include "sys_usw_packet.h"
#include "sys_usw_l2_fdb.h"
#include "sys_usw_ftm.h"
#include "sys_usw_register.h"
#include "sys_usw_dmps.h"
#include "sys_usw_mac.h"
#include "sys_usw_dma.h"
#include "sys_usw_packet_priv.h"
#include "sys_usw_register.h"
#include "sys_usw_qos.h"

#include "sys_usw_wb_common.h"

#include "drv_api.h"

#define SYS_INBAND_POLICER_ID_BASE  100
#define SYS_INBAND_MANAGEMENT_PKT_LEN 53+8
#define SYS_INBAND_SVLAN_TPID 0x8101
#define SYS_INBAND_SVLAN_TPID_INDEX 3
#define SYS_INBAND_EGRESS_ACL_RPIORIYT 0
#define SYS_INBAND_INGRESS_SCL_ID 0
#define SYS_INBAND_WHOLE_PACKET_PID 0x1F
#define SYS_INBAND_PORT_ACL_ENTRY_BASE 0xFFFF0000
#define SYS_INBAND_PORT_ACL_GROUP 0xFFFF0000
#define SYS_INBAND_PORT_CPU_REASON_BASE 256
#define SYS_INBAND_MAG_PKT_INERVAL_TIMER 60*1000
#define SYS_INBAND_PORT_BUFFER_COUNT 32
#define SYS_INBAND_MAX_PORT 4



#define SYS_INBAND_LOCK(lchip) \
    if (p_usw_inband_master[lchip]->mutex) sal_mutex_lock(p_usw_inband_master[lchip]->mutex)
#define SYS_INBAND_UNLOCK(lchip) \
    if (p_usw_inband_master[lchip]->mutex) sal_mutex_unlock(p_usw_inband_master[lchip]->mutex)

struct sys_inband_pkt_rx_s
{
    uint16 packet_seqnum;
    uint8 mag_pkt[SYS_INBAND_MANAGEMENT_PKT_LEN];
    uint32 mag_pkt_bitmap;
};
typedef struct sys_inband_pkt_rx_s sys_inband_pkt_rx_t;

struct sys_inband_pkt_tx_s
{
    ctc_slistnode_t node;
    uint8 mag_pkt[SYS_INBAND_MANAGEMENT_PKT_LEN];
    uint16 pkt_len;
    uint16 offset;                /*packet tx offset*/
    uint32 dest_gport;
};
typedef struct sys_inband_pkt_tx_s sys_inband_pkt_tx_t;

struct sys_inband_pkt_buf_s
{
    uint16 packet_id;
    uint32 bitmap:27;
    uint32 rsv:5;
    uint8  buffer[54];
};
typedef struct sys_inband_pkt_buf_s sys_inband_pkt_buf_t;

struct sys_inband_master_s
{
    uint8 timer;
    sys_inband_pkt_rx_t pkt_rx[SYS_INBAND_MAX_PORT];
    uint32 inband_port[SYS_INBAND_MAX_PORT];
    uint64 pkt_rx_count[SYS_INBAND_MAX_PORT];
    uint64 pkt_tx_count[SYS_INBAND_MAX_PORT];
    ctc_slist_t* pkt_tx_list;
    uint16 pkt_tx_num[SYS_INBAND_MAX_PORT];
    sal_mutex_t* mutex;
    sal_sem_t* tx_sem;                  /* to notify async task to send packet*/
    sal_task_t* p_async_tx_task;        /* async task send packet */
    sys_inband_pkt_buf_t p_inband_buffer[SYS_INBAND_MAX_PORT][SYS_INBAND_PORT_BUFFER_COUNT];

};
typedef struct sys_inband_master_s sys_inband_master_t;

sys_inband_master_t* p_usw_inband_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

int32
sys_usw_inband_packet_tx_whole_mag_packet(uint8 lchip, uint32 dest_gport, uint8* data, uint16 len, uint16 seq_num)
{
    ctc_pkt_tx_t* p_pkt_tx = NULL;
    ctc_pkt_info_t* p_tx_info = NULL;
    ctc_pkt_skb_t* p_skb = NULL;
    int32 ret= 0;

    p_pkt_tx = (ctc_pkt_tx_t*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(ctc_pkt_tx_t));
    if (NULL == p_pkt_tx)
    {
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_pkt_tx, 0, sizeof(ctc_pkt_tx_t));
    p_tx_info = &(p_pkt_tx->tx_info);
    p_skb = &(p_pkt_tx->skb);
    p_pkt_tx->lchip = lchip;
    p_pkt_tx->mode = CTC_PKT_MODE_DMA;
    p_tx_info->flags |= CTC_PKT_FLAG_NH_OFFSET_BYPASS;
    p_tx_info->dest_gport = dest_gport;
    p_tx_info->ttl = 1;
    p_tx_info->is_critical = 1;
    sal_memmove(&data[22], &data[0], len);

    data[0] = 0xaa;
    data[1] = 0xbb;
    data[2] = 0xcc;
    data[3] = 0xdd;
    data[4] = 0xee;
    data[5] = 0xff;
    data[6] = 0x00;
    data[7] = 0x11;
    data[8] = 0x22;
    data[9] = 0x33;
    data[10] = 0x44;
    data[11] = 0x55;
    data[12] = (SYS_INBAND_SVLAN_TPID&0xFF00)>>8;
    data[13] = SYS_INBAND_SVLAN_TPID&0x00FF;
    data[14] = ((seq_num&0xF80)>>7);
    data[15] = (SYS_INBAND_WHOLE_PACKET_PID&0x1F)| ((seq_num&0x7F)<<5);
    data[16] = 0x81;
    data[17] = 0x00;
    data[18] = 0x00;
    data[19] = 0x00;
    data[20] = 0x08;
    data[21] = 0x00;
    p_skb->data = data;
    p_skb->len = len+14+8;


    CTC_ERROR_GOTO(sys_usw_packet_encap(lchip, p_pkt_tx), ret, roll_back_0);

    if (CTC_PKT_MODE_DMA == p_pkt_tx->mode)
    {

        CTC_ERROR_GOTO(sys_usw_dma_pkt_tx(p_pkt_tx), ret, roll_back_0);

    }

    return CTC_E_NONE;
roll_back_0:
    mem_free(p_pkt_tx);
    return ret;
}

static int32
_sys_usw_inband_packet_dump(uint8 lchip, uint8* data, uint32 len)
{
    uint32 cnt = 0;
    char line[256] = {'\0'};
    char tmp[32] = {'\0'};

    if (0 == len)
    {
        return CTC_E_NONE;
    }

    for (cnt = 0; cnt < len; cnt++)
    {
        if ((cnt % 16) == 0)
        {
            if (cnt != 0)
            {
                SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%s", line);
            }

            sal_memset(line, 0, sizeof(line));
            if (cnt == 0)
            {
                sal_sprintf(tmp, "0x%04x:  ", cnt);
            }
            else
            {
                sal_sprintf(tmp, "\n0x%04x:  ", cnt);
            }
            sal_strcat(line, tmp);
        }

        sal_sprintf(tmp, "%02x", data[cnt]);
        sal_strcat(line, tmp);

        if ((cnt % 2) == 1)
        {
            sal_strcat(line, " ");
        }
    }

    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "%s", line);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "\n");

    return CTC_E_NONE;
}

static int32
_sys_usw_inband_get_port_index(uint8 lchip, uint32 gport, uint8* p_index)
{
    uint8 index = 0;

    *p_index = 0xFF;
    for (index=0; index< SYS_INBAND_MAX_PORT; index++)
    {
        if (p_usw_inband_master[lchip]->inband_port[index] == gport)
        {
            *p_index = index;
            break;
        }
    }

    return CTC_E_NONE;
}

int32
_sys_usw_inband_latency_init(uint8 lchip, bool enable)
{
    uint32 cmd = 0;
    uint32 field_val = 0;
    IpeLatencyCtl_m ipe_lcy_ctl;
    EpeLatencyCtl_m epe_lcy_ctl;

    sal_memset(&ipe_lcy_ctl, 0, sizeof(IpeLatencyCtl_m));
    sal_memset(&epe_lcy_ctl, 0, sizeof(EpeLatencyCtl_m));

    /*write IpeLatencyCtl*/
    field_val = (enable)?1:0;
    SetIpeLatencyCtl(V,  ecmpDisable_f, &ipe_lcy_ctl, field_val);
    SetIpeLatencyCtl(V,  lagEngineDisable_f, &ipe_lcy_ctl, field_val);
    SetIpeLatencyCtl(V,  ppTableDisable_f, &ipe_lcy_ctl, field_val);
    cmd = DRV_IOW(IpeLatencyCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd , &ipe_lcy_ctl));

    /*write EpeLatencyCtl */
    field_val = (enable)?1:0;
    SetEpeLatencyCtl(V, dsVlanDisable_f, &epe_lcy_ctl, field_val);
    SetEpeLatencyCtl(V, stackingHeaderDisable_f, &epe_lcy_ctl, field_val);
    cmd = DRV_IOW(EpeLatencyCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd , &epe_lcy_ctl));

    return CTC_E_NONE;
}




static int32
_sys_usw_inband_async_tx_pkt(uint8 lchip)
{

    int32 ret = CTC_E_NONE;
    ctc_slist_t* tx_list = NULL;
    sys_inband_pkt_tx_t* p_tx_inband = NULL;
    ctc_scl_entry_t scl_entry;
    ctc_field_port_t field_port;
    ctc_field_key_t field_key;
    ctc_scl_field_action_t field_action;
    ctc_scl_vlan_edit_t vlan_edit;
    uint8 pid = 0;
    uint8 index = 0;
    uint16 seq_num = 0;
    uint16 data = 0;
    uint16 loop_timer = 0;
    ctc_mac_stats_t mac_stats_st;
    ctc_mac_stats_t mac_stats_ed;

    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_INBAND_LOCK(lchip);
    tx_list = p_usw_inband_master[lchip]->pkt_tx_list;

    /* send new pkt */
    p_tx_inband = (sys_inband_pkt_tx_t*)CTC_SLISTHEAD(tx_list);
    if(!p_tx_inband)
    {
        SYS_INBAND_UNLOCK(lchip);
        return CTC_E_NONE;
    }
    ctc_slist_delete_node(tx_list, (ctc_slistnode_t*)p_tx_inband);
    SYS_INBAND_UNLOCK(lchip);

    sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));

    scl_entry.key_type = CTC_SCL_KEY_HASH_PORT;
    scl_entry.action_type = CTC_SCL_ACTION_EGRESS;
    scl_entry.mode = 1;
    CTC_ERROR_GOTO(sys_usw_scl_add_entry(lchip, CTC_SCL_GROUP_ID_HASH_PORT, &scl_entry, 1), ret, roll_back_0);

    /* KEY */
    sal_memset(&field_port, 0, sizeof(ctc_field_port_t));
    field_key.type = CTC_FIELD_KEY_DST_GPORT;
    field_key.ext_data = &field_port;
    field_port.type  = CTC_FIELD_PORT_TYPE_GPORT;
    field_port.gport = p_tx_inband->dest_gport;
    CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, roll_back_0);

    field_key.type = SYS_SCL_FIELD_KEY_COMMON;
    field_key.data = 1;
    CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, roll_back_0);
    field_key.type = CTC_FIELD_KEY_HASH_VALID;
    field_key.data = 1;
    CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, roll_back_0);
    _sys_usw_inband_get_port_index(lchip, p_tx_inband->dest_gport, &index);
    if (index == 0xFF)
    {
        ret = CTC_E_NOT_EXIST;
        goto roll_back_0;
    }

    while(loop_timer < SYS_INBAND_MAG_PKT_INERVAL_TIMER)
    {

        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "send packet index %d packet %d %d\n", p_tx_inband->offset, p_tx_inband->mag_pkt[p_tx_inband->offset], p_tx_inband->mag_pkt[p_tx_inband->offset+1]);

        /* ACTION */
        sal_memset(&vlan_edit, 0 ,sizeof(ctc_scl_vlan_edit_t));
        vlan_edit.stag_op = CTC_VLAN_TAG_OP_ADD;
        vlan_edit.ctag_op = CTC_VLAN_TAG_OP_ADD;
        vlan_edit.svid_sl = CTC_VLAN_TAG_SL_NEW;
        vlan_edit.cvid_sl = CTC_VLAN_TAG_SL_NEW;
        vlan_edit.scos_sl = CTC_VLAN_TAG_SL_NEW;
        vlan_edit.ccos_sl = CTC_VLAN_TAG_SL_NEW;
        vlan_edit.scfi_sl = CTC_VLAN_TAG_SL_NEW;
        vlan_edit.ccfi_sl = CTC_VLAN_TAG_SL_NEW;
        pid = p_tx_inband->offset/2+1;
        seq_num = p_usw_inband_master[lchip]->pkt_tx_num[index];
        data = sal_htons(*(uint16*)(p_tx_inband->mag_pkt+p_tx_inband->offset));
        vlan_edit.svid_new = ((pid&0x1F)|((seq_num&0x7F)<<5));
        vlan_edit.cvid_new = (data&0xFFF);
        vlan_edit.scos_new = ((seq_num>>8)&0x7);
        vlan_edit.ccos_new = ((data>>13)&0x7);
        vlan_edit.scfi_new = ((seq_num>>7)&0x1);
        vlan_edit.ccfi_new = ((data>>12)&0x01);
        vlan_edit.tpid_index = 3;

        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "send packet, port index %u, seq num is %u\n", index, p_usw_inband_master[lchip]->pkt_tx_num[index]);

        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_VLAN_EDIT;
        field_action.ext_data = &vlan_edit;
        CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip, scl_entry.entry_id, &field_action), ret, roll_back_0);
        if (0 == p_tx_inband->offset)
        {
            CTC_ERROR_GOTO(sys_usw_scl_install_entry(lchip, scl_entry.entry_id, 1), ret, roll_back_0);
        }

        sal_memset(&mac_stats_st, 0, sizeof(ctc_mac_stats_t));
        mac_stats_st.stats_mode = CTC_STATS_MODE_PLUS;
        sys_usw_mac_stats_get_tx_stats(lchip, p_tx_inband->dest_gport, &mac_stats_st);
        if (p_usw_inband_master[lchip]->timer >= 10)
        {
            sal_task_sleep(p_usw_inband_master[lchip]->timer-5);
        }
        sal_memset(&mac_stats_ed, 0, sizeof(ctc_mac_stats_t));
        mac_stats_ed.stats_mode = CTC_STATS_MODE_PLUS;
        sys_usw_mac_stats_get_tx_stats(lchip, p_tx_inband->dest_gport, &mac_stats_ed);

        if (mac_stats_ed.u.stats_plus.stats.tx_stats_plus.all_pkts != mac_stats_st.u.stats_plus.stats.tx_stats_plus.all_pkts)
        {
            p_tx_inband->offset += 2;
            loop_timer += p_usw_inband_master[lchip]->timer;
            if (p_tx_inband->offset >= p_tx_inband->pkt_len)
            {
                p_tx_inband->offset = 0;
                if (p_usw_inband_master[lchip]->timer < 10)
                {
                    break;
                }
            }
        }
        else
        {
            ret = sys_usw_inband_packet_tx_whole_mag_packet(lchip, p_tx_inband->dest_gport, p_tx_inband->mag_pkt, p_tx_inband->pkt_len, seq_num);
            if (ret < 0)
            {
                goto roll_back_1;
            }
            break;
        }
    }

    p_usw_inband_master[lchip]->pkt_tx_count[index]++;
    p_usw_inband_master[lchip]->pkt_tx_num[index]++;
    if (p_usw_inband_master[lchip]->pkt_tx_num[index] > 0x7FF)
    {
        p_usw_inband_master[lchip]->pkt_tx_num[index] = 0;
    }

roll_back_1:
    sys_usw_scl_uninstall_entry(lchip, scl_entry.entry_id, 1);

roll_back_0:
    sys_usw_scl_remove_entry(lchip, scl_entry.entry_id, 1);

    mem_free(p_tx_inband);
    return ret;
}


/**
@brief inband packet tx thread for async
*/
static void
_sys_usw_inband_pkt_tx_thread(void *param)
{
    int32 ret = 0;
    uint8 lchip = (uintptr)param;
    uint32 count = 0;

    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    while (1)
    {
        ret = sal_sem_take(p_usw_inband_master[lchip]->tx_sem, SAL_SEM_FOREVER);
        if (0 != ret)
        {
            continue;
        }

        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "recevie sem to send management packet\n");

        SYS_LCHIP_CHECK_ACTIVE_IN_THREAD(lchip);

        SYS_INBAND_LOCK(lchip);
        count = p_usw_inband_master[lchip]->pkt_tx_list->count;
        SYS_INBAND_UNLOCK(lchip);
        while(count > 0)
        {
           _sys_usw_inband_async_tx_pkt(lchip);
           SYS_INBAND_LOCK(lchip);
           count = p_usw_inband_master[lchip]->pkt_tx_list->count;
           SYS_INBAND_UNLOCK(lchip);
        }

        SYS_INBAND_UNLOCK(lchip);

    }

    return;
}

#define __________API_________ ""

int32
sys_usw_inband_packet_rx_count(uint8 lchip, uint8 index, uint8 count)
{
    CTC_MAX_VALUE_CHECK(index, SYS_INBAND_MAX_PORT-1);

    p_usw_inband_master[lchip]->pkt_rx_count[index] += count;

    return CTC_E_NONE;
}

int32
sys_usw_inband_alloc_mgt_pkt(uint8 lchip, uint32 dest_gport, uint8* data, uint16 len)
{
    sys_inband_pkt_tx_t* p_tx_inband = NULL;

    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(data);
    if (len> (SYS_INBAND_MANAGEMENT_PKT_LEN-8))
    {
        return CTC_E_INVALID_PARAM;
    }

    p_tx_inband = (sys_inband_pkt_tx_t*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(sys_inband_pkt_tx_t));
    if (NULL == p_tx_inband)
    {
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_tx_inband, 0, sizeof(sys_inband_pkt_tx_t));

    p_tx_inband->dest_gport = dest_gport;
    sal_memcpy(p_tx_inband->mag_pkt, data, len);
    p_tx_inband->pkt_len = len;

    SYS_INBAND_LOCK(lchip);
    ctc_slist_add_tail(p_usw_inband_master[lchip]->pkt_tx_list, &p_tx_inband->node);
    SYS_INBAND_UNLOCK(lchip);
    sal_sem_give(p_usw_inband_master[lchip]->tx_sem);

    return CTC_E_NONE;
}

int32
sys_usw_inband_packet_tx(uint8 lchip, void* p_tx_data)
{
    uint32 len = 0;
    uint8 header_len = 0;
    uint8 is_skb_buf = 0;
    uint8* p_data = NULL;
    ctc_pkt_tx_t* p_pkt_tx = (ctc_pkt_tx_t*)p_tx_data;

    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    if (!CTC_FLAG_ISSET(p_pkt_tx->tx_info.flags, CTC_PKT_FLAG_NH_OFFSET_BYPASS))
    {
        return CTC_E_INVALID_PARAM;
    }

    is_skb_buf = (((p_pkt_tx->skb.head == NULL) ||
                   (p_pkt_tx->skb.tail == NULL) ||
                   (p_pkt_tx->skb.end == NULL)) &&
                   (p_pkt_tx->skb.data != NULL) ? 0:1);
    header_len = (p_pkt_tx->mode == CTC_PKT_MODE_ETH)?(SYS_USW_PKT_HEADER_LEN+SYS_USW_PKT_CPUMAC_LEN):SYS_USW_PKT_HEADER_LEN;

    /*p_pkt_tx->skb.len ,to ETH include the header_len,to DMA exclude the header_len*/
    if (p_pkt_tx->mode == CTC_PKT_MODE_ETH)
    {
        return CTC_E_INVALID_PARAM;
    }
    else
    {
        if (p_pkt_tx->skb.len < SYS_PKT_BUF_PKT_LEN)
        {
            len = p_pkt_tx->skb.len;
        }
        else
        {
            len = SYS_PKT_BUF_PKT_LEN;
        }
    }

    if(CTC_PKT_MODE_DMA == p_pkt_tx->mode)
    {
        if(is_skb_buf)
        {
            p_data = p_pkt_tx->skb.data+header_len;
        }
        else
        {
            p_data = p_pkt_tx->skb.data;
        }
    }
    else
    {
        p_data = p_pkt_tx->skb.data+header_len;
    }

    CTC_ERROR_RETURN(sys_usw_inband_alloc_mgt_pkt(lchip, p_pkt_tx->tx_info.dest_gport, p_data, len));

    return CTC_E_NONE;
}

int32
sys_usw_inband_packet_rx(uint8 lchip, void* p_rx_data)
{
    uint16 packet_id = 0;
    uint16 stag = 0;
    uint8* p_data = NULL;
    ctc_pkt_buf_t*  p_buf = NULL;
    sys_inband_pkt_buf_t* p_packet_buffer = NULL;
    uint8  pid = 0;
    uint8  need_process = 0;
    uint16 tpid = 0;
    uint8 port_index= 0;
    int32 ret = 0;
    ctc_pkt_rx_t* p_pkt_rx = (ctc_pkt_rx_t*)p_rx_data;
    uint8  gchip = 0;

    if(p_pkt_rx == NULL)
    {
        return CTC_E_NONE;
    }


    p_buf = p_pkt_rx->pkt_buf;
    /*packet format MACDA||MACSA||TPID|| SVLAN ||TPID|| CVLAN ||*/
    if(!p_buf || !p_buf->data || (p_buf->len < 20))
    {
        return CTC_E_UNEXPECT;
    }

    p_data = (uint8*)p_buf->data+40;
    tpid = sal_ntohs(*(uint16*)(p_data+12));

    if(SYS_INBAND_SVLAN_TPID == tpid)
    {
        stag = sal_ntohs(*((uint16*)(p_data+14)));
        packet_id = (stag&0xFFE0) >> 5;
        pid = stag & 0x1F;

        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "receive packet, port index %u, seq num is %u, pid is %u\n", port_index, packet_id, pid);
        sys_usw_get_gchip_id(lchip, &gchip);
        _sys_usw_inband_get_port_index(lchip, CTC_MAP_LPORT_TO_GPORT(gchip, p_pkt_rx->rx_info.lport), &port_index);

        p_packet_buffer = &(p_usw_inband_master[lchip]->p_inband_buffer[port_index][packet_id%SYS_INBAND_PORT_BUFFER_COUNT]);
        /*packet receive done*/
        if(packet_id == p_packet_buffer->packet_id && 0x7FFFFFF == p_packet_buffer->bitmap)
        {
            return CTC_E_NONE;
        }

        /*whole packet*/
        if(pid == 0x1F)
        {
            need_process = 1;
            sys_usw_inband_packet_rx_count(lchip, port_index, 1);
            p_buf->len = 53+40;
            p_pkt_rx->pkt_len = 53+40;
            sal_memmove(p_data,p_data+22, 53);

            p_packet_buffer->bitmap = 0xFFFFFF;
            p_packet_buffer->packet_id = packet_id;
            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "receive whole packet seqnum %d\n", packet_id);
            _sys_usw_inband_packet_dump(lchip,p_data, 53);
        }
        else
        {

            if (packet_id != p_packet_buffer->packet_id)
            {
                if(p_packet_buffer->bitmap && p_packet_buffer->bitmap != 0x7FFFFFF)
                {
                    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "packet %u not receive done, packet id is %u ,received bitmap 0x%x\n",\
                        p_packet_buffer->packet_id, packet_id, p_packet_buffer->bitmap);
                }
                p_packet_buffer->bitmap = 0;
                p_packet_buffer->packet_id = packet_id;
            }
            pid--;
            if(!CTC_IS_BIT_SET(p_packet_buffer->bitmap, pid))
            {
                sal_memcpy(&(p_packet_buffer->buffer[pid*2]), p_data+18, 2);
                CTC_BIT_SET(p_packet_buffer->bitmap, pid);
                if(0x7FFFFFF == p_packet_buffer->bitmap)
                {
                    need_process = 1;
                    sys_usw_inband_packet_rx_count(lchip, port_index, 1);
                    p_buf->len = 53+40;
                    sal_memcpy(p_data, p_packet_buffer->buffer, 53);
                    p_pkt_rx->pkt_len = 53+40;
                    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "receive whole pid packet seqnum %d\n", packet_id);
                    _sys_usw_inband_packet_dump(lchip,p_data, 53);
                }
            }
        }
    }
    ret = need_process ? -1 : 0;

    return ret;
}


int32
sys_usw_inband_get_enable(uint8 lchip, uint32* p_value)
{
    LCHIP_CHECK(lchip);

    if (p_usw_inband_master[lchip])
    {
        *p_value = 1;
    }
    else
    {
        *p_value = 0;
    }

    return CTC_E_NONE;
}

int32
sys_usw_port_set_inband_en(uint8 lchip, uint32 gport, uint32 enable)
{
    uint8 index = 0;
    uint8 find_index = 0;
    bool is_find = 0;
    ctc_port_scl_property_t scl_prop;
    ctc_acl_property_t acl_prop;
    ctc_scl_field_action_t scl_field_action;
    ctc_scl_vlan_edit_t scl_vlan_edit;
    sys_scl_default_action_t def_action;
    ctc_acl_entry_t  acl_entry;
    ctc_qos_queue_cfg_t queue_cfg;
    uint8  offset = 0;
    uint32 entry_id;
    uint16 cpu_reason_id;
    ctc_field_key_t field_key;
    ctc_field_port_t field_port;
    ctc_field_port_t field_port_mask;
    ctc_acl_field_action_t  acl_field_action;
    ctc_acl_vlan_edit_t acl_vlan_edit;
    ctc_acl_to_cpu_t acl_cpu;
    ctc_qos_policer_t policer;
    int32 ret = CTC_E_NONE;

    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set inband enable gport:0x%04X enable %d!\n", gport, enable);


    LCHIP_CHECK(lchip);
    if (p_usw_inband_master[lchip] == NULL)
    {
        return CTC_E_NOT_INIT;
    }

    sal_memset(&scl_prop, 0, sizeof(ctc_port_scl_property_t));
    sal_memset(&acl_prop, 0, sizeof(ctc_acl_property_t));
    sal_memset(&scl_vlan_edit, 0, sizeof(ctc_scl_vlan_edit_t));
    sal_memset(&scl_field_action, 0, sizeof(ctc_scl_field_action_t));
    sal_memset(&def_action, 0, sizeof(sys_scl_default_action_t));

    _sys_usw_inband_get_port_index(lchip, gport, &index);
    if (0xFF != index)
    {
        is_find = TRUE;
    }
    if (enable && is_find)
    {
        return CTC_E_EXIST;
    }

    if (!enable && !is_find)
    {
        return CTC_E_NOT_EXIST;
    }

    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "inband port find %d index %d!\n", is_find, index);

    if (enable)
    {
        for (index=0; index<SYS_INBAND_MAX_PORT; index++)
        {
            if (p_usw_inband_master[lchip]->inband_port[index] == 0xFFFFFFFF)
            {
                break;
            }
        }

        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "inband find empty index %d!\n", index);

        if (SYS_INBAND_MAX_PORT == index)
        {
            return CTC_E_NO_RESOURCE;
        }
    }
    else
    {
        find_index = index;
        p_usw_inband_master[lchip]->inband_port[find_index] = 0xFFFFFFFF;
        goto roll_back_6;
    }
    find_index = index;

    CTC_ERROR_RETURN(sys_usw_mac_set_property(lchip, gport, CTC_PORT_PROP_PREAMBLE, 4));
    CTC_ERROR_GOTO(sys_usw_mac_set_property(lchip, gport, CTC_PORT_PROP_MAC_TX_IPG, 8), ret, roll_back_0);

    /*enable egress scl*/
    scl_prop.scl_id = SYS_INBAND_INGRESS_SCL_ID;
    scl_prop.direction = CTC_EGRESS;
    scl_prop.hash_type = CTC_PORT_EGS_SCL_HASH_TYPE_PORT;
    scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
    scl_prop.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_DISABLE;
    CTC_ERROR_GOTO(sys_usw_port_set_scl_property(lchip, gport, &scl_prop), ret, roll_back_1);

    /*enable ingress acl*/
    acl_prop.acl_priority = SYS_INBAND_EGRESS_ACL_RPIORIYT;
    acl_prop.acl_en = 1;
    acl_prop.direction= CTC_INGRESS;
    acl_prop.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_L2;
    CTC_ERROR_GOTO(sys_usw_port_set_acl_property(lchip, gport, &acl_prop), ret, roll_back_2);

    CTC_ERROR_GOTO(sys_usw_port_set_direction_property(lchip, gport, CTC_PORT_DIR_PROP_STAG_TPID_INDEX, CTC_INGRESS, SYS_INBAND_SVLAN_TPID_INDEX), ret, roll_back_3);
    CTC_ERROR_GOTO(sys_usw_port_set_direction_property(lchip, gport, CTC_PORT_DIR_PROP_STAG_TPID_INDEX, CTC_EGRESS, SYS_INBAND_SVLAN_TPID_INDEX), ret, roll_back_4);

    /*default to insert double vlan*/
    def_action.lport = CTC_MAP_GPORT_TO_LPORT(gport);
    def_action.field_action = &scl_field_action;
    def_action.action_type = SYS_SCL_ACTION_EGRESS;
    def_action.scl_id = SYS_INBAND_INGRESS_SCL_ID;

    scl_field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
    scl_field_action.data0 = 1;
    CTC_ERROR_GOTO(sys_usw_scl_set_default_action(lchip, &def_action), ret, roll_back_5);

    sal_memset(&scl_field_action, 0, sizeof(ctc_scl_field_action_t));
    scl_vlan_edit.stag_op = CTC_VLAN_TAG_OP_ADD;
    scl_vlan_edit.svid_sl = CTC_VLAN_TAG_SL_NEW;
    scl_vlan_edit.svid_new = 0;
    scl_vlan_edit.ctag_op = CTC_VLAN_TAG_OP_ADD;
    scl_vlan_edit.cvid_sl = CTC_VLAN_TAG_SL_NEW;
    scl_vlan_edit.cvid_new = 0;
    scl_field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_VLAN_EDIT;
    scl_field_action.ext_data = (void*)&scl_vlan_edit;
    CTC_ERROR_GOTO(sys_usw_scl_set_default_action(lchip, &def_action), ret, roll_back_5);

    sal_memset(&scl_field_action, 0, sizeof(ctc_scl_field_action_t));
    scl_field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
    scl_field_action.data0 = 0;
    CTC_ERROR_GOTO(sys_usw_scl_set_default_action(lchip, &def_action), ret, roll_back_5);

    sal_memset(&queue_cfg, 0, sizeof(queue_cfg));
    sal_memset(&acl_entry, 0, sizeof(acl_entry));
    sal_memset(&field_port, 0, sizeof(field_port));
    sal_memset(&field_port_mask, 0xFF, sizeof(field_port_mask));
    for(index=0; index < 28; index++)
    {
        offset = (index==27)?0x1F:(index+1);
        entry_id = SYS_INBAND_PORT_ACL_ENTRY_BASE+find_index*28+index;

        acl_entry.entry_id = entry_id;
        acl_entry.key_type = CTC_ACL_KEY_MAC;
        acl_entry.mode = 1;
        CTC_ERROR_GOTO(sys_usw_acl_add_entry(lchip, SYS_INBAND_PORT_ACL_GROUP, &acl_entry), ret, roll_back_6);

        field_port.gport = gport;
        field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
        field_key.type = CTC_FIELD_KEY_PORT;
        field_key.ext_data = (void*)&field_port;
        field_key.ext_mask = (void*)&field_port_mask;
        CTC_ERROR_GOTO(sys_usw_acl_add_key_field(lchip, entry_id, &field_key), ret, roll_back_6);

        sal_memset(&field_key, 0, sizeof(field_key));
        field_key.type = CTC_FIELD_KEY_SVLAN_ID;
        field_key.data = offset&0x1F;
        field_key.mask = 0x1F;
        CTC_ERROR_GOTO(sys_usw_acl_add_key_field(lchip, entry_id, &field_key), ret, roll_back_6);

        sal_memset(&acl_vlan_edit, 0, sizeof(acl_vlan_edit));

        acl_vlan_edit.stag_op = CTC_ACL_VLAN_TAG_OP_DEL;
        acl_vlan_edit.ctag_op = CTC_ACL_VLAN_TAG_OP_DEL;
        acl_field_action.type = CTC_ACL_FIELD_ACTION_VLAN_EDIT;
        acl_field_action.ext_data = &acl_vlan_edit;
        CTC_ERROR_GOTO(sys_usw_acl_add_action_field(lchip, entry_id, &acl_field_action), ret, roll_back_6);

        if(index == 27)
        {
            sal_memset(&acl_field_action, 0, sizeof(acl_field_action));
            acl_field_action.type = CTC_ACL_FIELD_ACTION_DISCARD;
            acl_field_action.data0  = 0;
            CTC_ERROR_GOTO(sys_usw_acl_add_action_field(lchip, entry_id, &acl_field_action), ret, roll_back_6);
        }

        {

            if (find_index < 2)
            {
                cpu_reason_id = find_index*28+CTC_PKT_CPU_REASON_L2_PDU+index;
            }
            else if(find_index == 2)
            {
                cpu_reason_id = (index < 9) ? (find_index*28+CTC_PKT_CPU_REASON_L2_PDU+index) : \
                    ((find_index-2)*28+SYS_INBAND_PORT_CPU_REASON_BASE+index);
            }
            else
            {
                cpu_reason_id = (find_index-2)*28+SYS_INBAND_PORT_CPU_REASON_BASE+index;
            }

            /*cpu reason queue map*/
            queue_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_REASON_MAP;
            queue_cfg.value.reason_map.cpu_reason = cpu_reason_id;
            queue_cfg.value.reason_map.queue_id = (find_index*28+index)%8;
            queue_cfg.value.reason_map.reason_group = (find_index*28+index)/8;
            CTC_ERROR_GOTO(sys_usw_qos_set_queue(lchip, &queue_cfg), ret, roll_back_6);

            sys_usw_cpu_reason_set_dest(lchip,  cpu_reason_id, CTC_PKT_CPU_REASON_TO_LOCAL_CPU, 0);


                sal_memset(&policer, 0, sizeof(policer));

                policer.id.policer_id = SYS_INBAND_POLICER_ID_BASE+index;
                policer.type = CTC_QOS_POLICER_TYPE_COPP;
                policer.enable = 1;
                policer.pps_en = 1;
                policer.policer.policer_mode = CTC_QOS_POLICER_MODE_STBM;
                policer.policer.cir = 60;
                policer.policer.pir = 60;
                policer.policer.pbs = 64;
                policer.policer.cbs = 64;
                policer.policer.drop_color = CTC_QOS_COLOR_RED;
                CTC_ERROR_GOTO(sys_usw_qos_set_policer(lchip, &policer), ret, roll_back_6);

                acl_field_action.type = CTC_ACL_FIELD_ACTION_COPP;
                acl_field_action.data0 = SYS_INBAND_POLICER_ID_BASE+index;
                CTC_ERROR_GOTO(sys_usw_acl_add_action_field(lchip, entry_id, &acl_field_action), ret, roll_back_6);

            sal_memset(&acl_cpu, 0, sizeof(acl_cpu));
            sal_memset(&acl_field_action, 0, sizeof(acl_field_action));
            acl_cpu.mode = CTC_ACL_TO_CPU_MODE_TO_CPU_COVER;
            acl_cpu.cpu_reason_id = cpu_reason_id;
            acl_field_action.type = CTC_ACL_FIELD_ACTION_CP_TO_CPU;
            acl_field_action.ext_data = &acl_cpu;
            CTC_ERROR_GOTO(sys_usw_acl_add_action_field(lchip, entry_id, &acl_field_action), ret, roll_back_6);
        }
        CTC_ERROR_GOTO(sys_usw_acl_install_entry(lchip, entry_id), ret, roll_back_6);
    }

    p_usw_inband_master[lchip]->inband_port[find_index] = gport;

    return CTC_E_NONE;

roll_back_6:
    for(index=0; index < 28; index++)
    {
        entry_id = SYS_INBAND_PORT_ACL_ENTRY_BASE+find_index*28+index;
        sys_usw_acl_uninstall_entry(lchip, entry_id);
        sys_usw_acl_remove_entry(lchip, entry_id);
    }
roll_back_5:
    sys_usw_port_set_direction_property(lchip, gport, CTC_PORT_DIR_PROP_STAG_TPID_INDEX, CTC_EGRESS, 0);

roll_back_4:
    sys_usw_port_set_direction_property(lchip, gport, CTC_PORT_DIR_PROP_STAG_TPID_INDEX, CTC_INGRESS, 0);

roll_back_3:
    acl_prop.acl_priority = SYS_INBAND_EGRESS_ACL_RPIORIYT;
    acl_prop.acl_en = 0;
    acl_prop.direction= CTC_INGRESS;
    acl_prop.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_L2;
    sys_usw_port_set_acl_property(lchip, gport, &acl_prop);

roll_back_2:
    scl_prop.scl_id = SYS_INBAND_INGRESS_SCL_ID;
    scl_prop.direction = CTC_EGRESS;
    scl_prop.hash_type = CTC_PORT_EGS_SCL_HASH_TYPE_DISABLE;
    scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
    scl_prop.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_DISABLE;
    sys_usw_port_set_scl_property(lchip, gport, &scl_prop);

roll_back_1:
    sys_usw_mac_set_property(lchip, gport, CTC_PORT_PROP_MAC_TX_IPG, 12);

roll_back_0:
    sys_usw_mac_set_property(lchip, gport, CTC_PORT_PROP_PREAMBLE, 8);

    return ret;
}


int32
sys_usw_inband_timer(uint32 lchip, uint32 timer)
{
    uint8 task_destroy = 0;
    int32 ret = CTC_E_NONE;
    ctc_acl_group_info_t acl_group_info;
    char buffer[SAL_TASK_MAX_NAME_LEN]={0};

    if (p_usw_inband_master[lchip] == NULL)
    {
        return CTC_E_NOT_EXIST;
    }

    if (timer && (timer > 100))
    {
        return CTC_E_NOT_SUPPORT;
    }

    task_destroy = timer ? 0 : 1;

    if (task_destroy && p_usw_inband_master[lchip]->timer)/*destroy*/
    {
        goto roll_back_6;
    }
    else if ((!task_destroy) && (0 == p_usw_inband_master[lchip]->timer))/*create*/
    {
        /* create async tx thread*/
        ret = sal_sem_create(&p_usw_inband_master[lchip]->tx_sem, 0);
        if (ret < 0)
        {
            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Create sem fail \n");
            ret = CTC_E_NO_RESOURCE;
            goto roll_back_0;
        }

        sal_sprintf(buffer, "inband_async_tx-%d", lchip);
        ret = sal_task_create(&p_usw_inband_master[lchip]->p_async_tx_task, buffer,
                              SAL_DEF_TASK_STACK_SIZE, SAL_TASK_PRIO_DEF, _sys_usw_inband_pkt_tx_thread, (void*)(uintptr)lchip);
        if (ret < 0)
        {
            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Create inband async task fail \n");
            ret = CTC_E_NOT_INIT;
            goto roll_back_1;
        }
        p_usw_inband_master[lchip]->timer = timer; /*PID packet send timer*/
    }
    else if ((!task_destroy) && (0 != p_usw_inband_master[lchip]->timer))
    {
        p_usw_inband_master[lchip]->timer =  timer; /*PID packet send timer*/
        return CTC_E_NONE;
    }
    else
    {
        return CTC_E_INVALID_CONFIG;
    }

    sal_memset(&acl_group_info, 0, sizeof(ctc_acl_group_info_t));
    acl_group_info.dir = CTC_INGRESS;
    acl_group_info.lchip = lchip;
    acl_group_info.type = CTC_ACL_GROUP_TYPE_NONE;
    CTC_ERROR_GOTO(sys_usw_acl_create_group(lchip, SYS_INBAND_PORT_ACL_GROUP, &acl_group_info), ret, roll_back_1);

    /*register cb*/
    CTC_ERROR_GOTO(sys_usw_register_api_cb(lchip, SYS_REGISTER_CB_TYPE_INBAND_RX, sys_usw_inband_packet_rx), ret, roll_back_2);
    CTC_ERROR_GOTO(sys_usw_register_api_cb(lchip, SYS_REGISTER_CB_TYPE_INBAND_TX, sys_usw_inband_packet_tx), ret, roll_back_3);
    CTC_ERROR_GOTO(sys_usw_register_api_cb(lchip, SYS_REGISTER_CB_TYPE_INBAND_PORT_EN, sys_usw_port_set_inband_en), ret, roll_back_4);

    CTC_ERROR_GOTO(_sys_usw_inband_latency_init(lchip, TRUE), ret, roll_back_5);

    CTC_ERROR_GOTO(sys_usw_parser_set_tpid(lchip, CTC_PARSER_L2_TPID_SVLAN_TPID_0+SYS_INBAND_SVLAN_TPID_INDEX, SYS_INBAND_SVLAN_TPID), ret, roll_back_6);


    return ret;

roll_back_6:
    _sys_usw_inband_latency_init(lchip, FALSE);
roll_back_5:
    sys_usw_register_api_cb(lchip, SYS_REGISTER_CB_TYPE_INBAND_PORT_EN, NULL);
roll_back_4:
    sys_usw_register_api_cb(lchip, SYS_REGISTER_CB_TYPE_INBAND_TX, NULL);
roll_back_3:
    sys_usw_register_api_cb(lchip, SYS_REGISTER_CB_TYPE_INBAND_RX, NULL);
roll_back_2:
    sys_usw_acl_destroy_group(lchip, SYS_INBAND_PORT_ACL_GROUP);
roll_back_1:
    if (p_usw_inband_master[lchip]->p_async_tx_task)
    {
        sal_task_destroy(p_usw_inband_master[lchip]->p_async_tx_task);
        p_usw_inband_master[lchip]->p_async_tx_task = 0;
    }
roll_back_0:
    if (p_usw_inband_master[lchip]->tx_sem)
    {
        sal_sem_destroy(p_usw_inband_master[lchip]->tx_sem);
        p_usw_inband_master[lchip]->tx_sem = 0;
    }
    p_usw_inband_master[lchip]->timer = 0;

    return ret;

}

int32
sys_usw_inband_init(uint8 lchip)
{
    int32 ret = 0;

    LCHIP_CHECK(lchip);

    if (p_usw_inband_master[lchip] != NULL)
    {
        return CTC_E_NONE;
    }



    /*alloc&init DB and mutex*/
    p_usw_inband_master[lchip] = (sys_inband_master_t*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(sys_inband_master_t));
    if (NULL == p_usw_inband_master[lchip])
    {
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_usw_inband_master[lchip], 0, sizeof(sys_inband_master_t));
    sal_memset(p_usw_inband_master[lchip]->inband_port, 0xFF, sizeof(uint32)*SYS_INBAND_MAX_PORT);


    ret = sal_mutex_create(&(p_usw_inband_master[lchip]->mutex));
    if (ret || !(p_usw_inband_master[lchip]->mutex))
    {
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Create mutex fail\n");
        ret =  CTC_E_NO_MEMORY;
        goto roll_back_0;
    }

    p_usw_inband_master[lchip]->pkt_tx_list = ctc_slist_new();
    if(NULL == p_usw_inband_master[lchip]->pkt_tx_list)
    {
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        ret =  CTC_E_NO_MEMORY;
        goto roll_back_1;
    }



    CTC_ERROR_GOTO(sys_usw_register_api_cb(lchip, SYS_REGISTER_CB_TYPE_INBAND_TIMER, sys_usw_inband_timer), ret, roll_back_2);

    return CTC_E_NONE;



roll_back_2:
    if (p_usw_inband_master[lchip]->pkt_tx_list)
    {
        ctc_slist_free(p_usw_inband_master[lchip]->pkt_tx_list);
    }

roll_back_1:
    if (p_usw_inband_master[lchip]->mutex)
    {
        sal_mutex_destroy(p_usw_inband_master[lchip]->mutex);
    }

roll_back_0:
    if (p_usw_inband_master[lchip])
    {
        mem_free(p_usw_inband_master[lchip]);
    }

    return ret;
}

int32
sys_usw_inband_deinit(uint8 lchip)
{

    if (p_usw_inband_master[lchip] == NULL)
    {
        return CTC_E_NONE;
    }


    sys_usw_register_api_cb(lchip, SYS_REGISTER_CB_TYPE_INBAND_TIMER, NULL);
    sys_usw_register_api_cb(lchip, SYS_REGISTER_CB_TYPE_INBAND_PORT_EN, NULL);
    sys_usw_register_api_cb(lchip, SYS_REGISTER_CB_TYPE_INBAND_TX, NULL);
    sys_usw_register_api_cb(lchip, SYS_REGISTER_CB_TYPE_INBAND_RX, NULL);

    _sys_usw_inband_latency_init(lchip, FALSE);

    sal_sem_give(p_usw_inband_master[lchip]->tx_sem);
    if (p_usw_inband_master[lchip]->p_async_tx_task)
    {
        sal_task_destroy(p_usw_inband_master[lchip]->p_async_tx_task);
    }
    if (p_usw_inband_master[lchip]->tx_sem)
    {
        sal_sem_destroy(p_usw_inband_master[lchip]->tx_sem);
    }

    if (p_usw_inband_master[lchip]->timer)
    {
        sys_usw_acl_destroy_group(lchip, SYS_INBAND_PORT_ACL_GROUP);
    }

    if (p_usw_inband_master[lchip]->pkt_tx_list)
    {
        ctc_slist_free(p_usw_inband_master[lchip]->pkt_tx_list);
    }


    if (p_usw_inband_master[lchip]->mutex)
    {
        sal_mutex_destroy(p_usw_inband_master[lchip]->mutex);
    }


    if (p_usw_inband_master[lchip])
    {
        mem_free(p_usw_inband_master[lchip]);
    }

    return CTC_E_NONE;
}


int32
sys_usw_inband_show_status(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    uint8 index_st = 0;
    uint8 index_ed = SYS_INBAND_MAX_PORT;
    uint8 index = 0;
    char port_str[100] = {0};
    uint8 port_cnt    = 0;

    if (p_usw_inband_master[lchip] == NULL)
    {
        return CTC_E_NOT_INIT;
    }

    for (index=index_st; index<index_ed; index++)
    {
        if (p_usw_inband_master[lchip]->inband_port[index] != 0xFFFFFFFF)
        {
            sal_sprintf((port_str + (7*port_cnt)), "0x%.4x ", p_usw_inband_master[lchip]->inband_port[index]);
            port_cnt++;
        }

        if ((index == (index_ed -1)) &&  port_cnt == 0)
        {
            sal_sprintf(port_str, "-");
        }
    }

    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------global config---------------------------------------\n");
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  Inband Timer: %d(ms)\n", p_usw_inband_master[lchip]->timer);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  Inband Gport: %s\n", port_str);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
    if (0 == port_cnt)
    {
        return CTC_E_NONE;
    }
    for (index=index_st; index<index_ed; index++)
    {
        if (index == index_st)
        {
            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------port stats------------------------------------------\n");
        }
        if (p_usw_inband_master[lchip]->inband_port[index] != 0xFFFFFFFF)
        {
            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  Gport 0x%.4x:\n", p_usw_inband_master[lchip]->inband_port[index]);
            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  Packet tx 0x%"PRIx64"\n", p_usw_inband_master[lchip]->pkt_tx_count[index]);
            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  Packet rx 0x%"PRIx64"\n", p_usw_inband_master[lchip]->pkt_rx_count[index]);
            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
        }
        if (index == (index_ed-1))
        {
            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "--------------------------------------------------------------------------------\n");
        }
    }

    return ret;
}



