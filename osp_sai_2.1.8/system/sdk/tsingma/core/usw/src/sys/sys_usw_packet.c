/**
 @file sys_usw_packet.c

 @author  Copyright (C) 2012 Centec Networks Inc.  All rights reserved.

 @date 2012-11-25

 @version v2.0

 This file define sys functions

*/

/****************************************************************************
*
* Header Files
*
****************************************************************************/
#include "sal.h"
#include "dal.h"
#include "ctc_error.h"
#include "ctc_macro.h"
#include "ctc_debug.h"
#include "ctc_packet.h"
#include "ctc_parser.h"
#include "ctc_oam.h"
#include "ctc_qos.h"
#include "ctc_crc.h"
#include "ctc_vlan.h"
#include "ctc_nexthop.h"
#include "ctc_usw_packet.h"
#include "ctc_usw_nexthop.h"
#include "sys_usw_common.h"
#include "sys_usw_packet.h"
#include "sys_usw_packet_priv.h"
#include "sys_usw_dma.h"
#include "sys_usw_chip.h"
#include "sys_usw_vlan.h"
#include "sys_usw_port.h"
#include "sys_usw_dmps.h"
#include "sys_usw_qos_api.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_register.h"
#include "sys_usw_dma_priv.h"
#include "sys_usw_wb_common.h"
#include "drv_api.h"
#include "usw/include/drv_chip_ctrl.h"
#include "ctc_hash.h"

/****************************************************************************
*
* Defines and Macros
*
*****************************************************************************/
#define SYS_PACKET_ENCAP_LOOP_NHPTR(lport,ttl_valid) ((((lport & 0x80)>>7) << 16) |(ttl_valid<<7)| (lport & 0x7F))
#define SYS_PACKET_BASIC_STACKING_HEADER_LEN 32

/****************************************************************************
*
* Global and Declaration
*
*****************************************************************************/
sys_pkt_master_t* p_usw_pkt_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

enum sys_pkt_ctag_action_e
{
    SYS_PKT_CTAG_ACTION_NONE,                  /* 0 */
    SYS_PKT_CTAG_ACTION_MODIFY,                /* 1 */
    SYS_PKT_CTAG_ACTION_ADD,                   /* 2 */
    SYS_PKT_CTAG_ACTION_DELETE,                /* 3 */
};
typedef enum sys_pkt_ctag_action_e sys_pkt_ctag_action_t;

/****************************************************************************
*
* Lock
*
*****************************************************************************/
#ifndef PACKET_TX_USE_SPINLOCK
#define SYS_PACKET_LOCK(lchip) \
    if(p_usw_pkt_master[lchip]->mutex) sal_mutex_lock(p_usw_pkt_master[lchip]->mutex)

#define SYS_PACKET_UNLOCK(lchip) \
    if(p_usw_pkt_master[lchip]->mutex) sal_mutex_unlock(p_usw_pkt_master[lchip]->mutex)
#else
#define SYS_PACKET_LOCK(lchip) \
    if(p_usw_pkt_master[lchip]->mutex) sal_spinlock_lock((sal_spinlock_t*)p_usw_pkt_master[lchip]->mutex)

#define SYS_PACKET_UNLOCK(lchip) \
    if(p_usw_pkt_master[lchip]->mutex) sal_spinlock_unlock((sal_spinlock_t*)p_usw_pkt_master[lchip]->mutex)
#endif

#define SYS_PACKET_RX_LOCK(lchip) \
    if(p_usw_pkt_master[lchip]->mutex_rx) sal_mutex_lock(p_usw_pkt_master[lchip]->mutex_rx)

#define SYS_PACKET_RX_UNLOCK(lchip) \
    if(p_usw_pkt_master[lchip]->mutex_rx) sal_mutex_unlock(p_usw_pkt_master[lchip]->mutex_rx)

#define SYS_PACKET_INIT_CHECK(lchip) \
    if (p_usw_pkt_master[lchip] == NULL)\
    {\
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");\
        return CTC_E_NOT_INIT;\
    }

#define CTC_ERROR_RETURN_PACKET_UNLOCK(lchip, op) \
    do \
    { \
        int32 rv = (op); \
        if (rv < 0) \
        { \
            sal_mutex_unlock(p_usw_pkt_master[lchip]->mutex); \
            return rv; \
        } \
    } while (0)

#define CTC_ERROR_RETURN_PACKET_RX_UNLOCK(lchip, op) \
    do \
    { \
        int32 rv = (op); \
        if (rv < 0) \
        { \
            sal_mutex_unlock(p_usw_pkt_master[lchip]->mutex_rx); \
            return rv; \
        } \
    } while (0)

#define CTC_RETURN_PACKET_UNLOCK(lchip, op) \
    do \
    { \
        sal_mutex_unlock(p_usw_pkt_master[lchip]->mutex); \
        return (op); \
    } while (0)

extern char*
sys_usw_reason_2Str(uint16 reason_id);

extern dal_op_t g_dal_op;
/****************************************************************************
*
* Function
*
*****************************************************************************/
#define ROT(x, k) (((x) << (k)) | ((x) >> (32 - (k))))
#define MIX(a, b, c) \
    do \
    { \
        a -= c;  a ^= ROT(c, 4);  c += b; \
        b -= a;  b ^= ROT(a, 6);  a += c; \
        c -= b;  c ^= ROT(b, 8);  b += a; \
        a -= c;  a ^= ROT(c, 16);  c += b; \
        b -= a;  b ^= ROT(a, 19);  a += c; \
        c -= b;  c ^= ROT(b, 4);  b += a; \
    } while (0)

static uint32
_sys_usw_netif_hash_make(ctc_pkt_netif_t* p_netif)
{
    uint32 a, b, c;

    /* Set up the internal state */
    a = b = c = 0xdeadbeef + (((uint32)96) << 2);

    a += p_netif->gport;
    b += p_netif->vlan;
    MIX(a, b, c);

    return (c & SYS_PKT_NETIF_HASH_SIZE);
}

static bool
_sys_usw_netif_hash_cmp(ctc_pkt_netif_t* p_netif1, ctc_pkt_netif_t* p_netif2)
{
    if (p_netif1->type != p_netif2->type)
    {
        return FALSE;
    }

    if ((p_netif1->type == CTC_PKT_NETIF_T_PORT) &&
        (p_netif1->gport == p_netif2->gport))
    {
        return TRUE;
    }

    if ((p_netif1->type == CTC_PKT_NETIF_T_VLAN) &&
        (p_netif1->vlan == p_netif2->vlan))
    {
        return TRUE;
    }

    return FALSE;
}

static int32
_sys_usw_packet_dump(uint8 lchip, uint8* data, uint32 len)
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

    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%s", line);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");

    return CTC_E_NONE;
}

static int32
_sys_usw_packet_dump_rx_header(uint8 lchip, ctc_pkt_rx_t  *p_rx_info)
{
    char* p_str_tmp = NULL;
    char str[40] = {0};
    char* str_op_type[] = {"NORMAL", "LMTX", "E2ILOOP", "DMTX", "C2C", "PTP", "NAT", "OAM"};

    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------\n");
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Packet Header Raw Info(Length : %d): \n", SYS_USW_PKT_HEADER_LEN);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------\n");

    p_str_tmp = sys_usw_reason_2Str(p_rx_info->rx_info.reason);
    sal_sprintf((char*)&str, "%s%s%d%s", p_str_tmp, "(ID:", p_rx_info->rx_info.reason, ")");

    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %s\n", "cpu reason", (char*)&str);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "queue id", p_rx_info->rx_info.dest_gport);

    /*source*/
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: 0x%.04x\n", "src port", p_rx_info->rx_info.src_port);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: 0x%.04x\n", "stacking src port", p_rx_info->rx_info.stacking_src_port);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "src svid", p_rx_info->rx_info.src_svid);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "src cvid", p_rx_info->rx_info.src_cvid);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "src cos", p_rx_info->rx_info.src_cos);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "stag op", p_rx_info->rx_info.stag_op);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "ctag op", p_rx_info->rx_info.ctag_op);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "vrf", p_rx_info->rx_info.vrfid);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "fid", p_rx_info->rx_info.fid);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "ttl", p_rx_info->rx_info.ttl);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "priority", p_rx_info->rx_info.priority);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "color", p_rx_info->rx_info.color);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "hash value", p_rx_info->rx_info.hash);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "critical packet", p_rx_info->rx_info.is_critical);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "packet type", p_rx_info->rx_info.packet_type);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "src chip", p_rx_info->rx_info.src_chip);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "payload offset", p_rx_info->rx_info.payload_offset);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "buffer log victim packet",CTC_FLAG_ISSET(p_rx_info->rx_info.flags, CTC_PKT_FLAG_BUFFER_VICTIM_PKT));
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %s\n", "operation type", str_op_type[p_rx_info->rx_info.oper_type]);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "logic src port", p_rx_info->rx_info.logic_src_port);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "meta data", p_rx_info->rx_info.meta_data);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "cid", p_rx_info->rx_info.cid);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "lport", p_rx_info->rx_info.lport);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "mac unknown", CTC_FLAG_ISSET(p_rx_info->rx_info.flags, CTC_PKT_FLAG_UNKOWN_MACDA));
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "stk_hdr_len", p_rx_info->stk_hdr_len);

    if (p_rx_info->rx_info.oper_type == CTC_PKT_OPER_OAM)
    {
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "oam info:\n");
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "oam type", p_rx_info->rx_info.oam.type);
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "mep index", p_rx_info->rx_info.oam.mep_index);
    }

    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "timestamp info:\n");
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %u\n", "seconds", p_rx_info->rx_info.ptp.ts.seconds);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %u\n", "nanoseconds", p_rx_info->rx_info.ptp.ts.nanoseconds);

    if (CTC_PKT_CPU_REASON_DOT1AE_REACH_PN_THRD == p_rx_info->rx_info.reason)
    {
        if(CTC_FLAG_ISSET(p_rx_info->rx_info.flags, CTC_PKT_FLAG_ENCRYPTED))
        {
            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %s\n", "dot1ae type", "ENCRYPTED" );
        }
        else
        {
            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %s\n", "dot1ae type", "DECRYPTED" );
        }

    }
    if ((p_rx_info->pkt_buf) && (p_rx_info->pkt_buf[0].data))
    {
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
        CTC_ERROR_RETURN(_sys_usw_packet_dump(lchip, (p_rx_info->pkt_buf[0].data), SYS_USW_PKT_HEADER_LEN));
    }

    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------\n");

    return CTC_E_NONE;
}

static int32
_sys_usw_packet_dump_tx_header(uint8 lchip, ctc_pkt_tx_t  *p_tx_info, uint8 is_skb_buf, uint8 dump_mode)
{
    char* str_op_type[] = {"NORMAL", "LMTX", "E2ILOOP", "DMTX", "C2C", "PTP", "NAT", "OAM"};
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------\n");
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Packet Header Raw Info(Length : %d): \n", SYS_USW_PKT_HEADER_LEN);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------\n");

    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: 0x%x\n", "dest gport", p_tx_info->tx_info.dest_gport);

    /*source*/
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: 0x%.04x\n", "src port", p_tx_info->tx_info.src_port);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "src svid", p_tx_info->tx_info.src_svid);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "src cvid", p_tx_info->tx_info.src_cvid);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "src cos", p_tx_info->tx_info.src_cos);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "stag op", p_tx_info->tx_info.stag_op);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "ctag op", p_tx_info->tx_info.ctag_op);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "fid", p_tx_info->tx_info.fid);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "ttl", p_tx_info->tx_info.ttl);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %s\n", "operation type", str_op_type[p_tx_info->tx_info.oper_type]);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "priority", p_tx_info->tx_info.priority);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "dest group id", p_tx_info->tx_info.dest_group_id);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "color", p_tx_info->tx_info.color);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "critical packet", p_tx_info->tx_info.is_critical);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "nexthop offset", p_tx_info->tx_info.nh_offset);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "hash ", p_tx_info->tx_info.hash);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "vlan domain", p_tx_info->tx_info.vlan_domain);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "svlan tpid", p_tx_info->tx_info.svlan_tpid);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %u\n", "nexthop id", p_tx_info->tx_info.nhid);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "buffer log victim packet",CTC_FLAG_ISSET(p_tx_info->tx_info.flags, CTC_PKT_FLAG_BUFFER_VICTIM_PKT));

    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "MCAST", CTC_FLAG_ISSET(p_tx_info->tx_info.flags, CTC_PKT_FLAG_MCAST));
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "NH_OFFSET_BYPASS", CTC_FLAG_ISSET(p_tx_info->tx_info.flags, CTC_PKT_FLAG_NH_OFFSET_BYPASS));
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "NH_OFFSET_IS_8W", CTC_FLAG_ISSET(p_tx_info->tx_info.flags, CTC_PKT_FLAG_NH_OFFSET_IS_8W));
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "INGRESS_MODE", CTC_FLAG_ISSET(p_tx_info->tx_info.flags, CTC_PKT_FLAG_INGRESS_MODE));
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "CANCEL_DOT1AE", CTC_FLAG_ISSET(p_tx_info->tx_info.flags, CTC_PKT_FLAG_CANCEL_DOT1AE));
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "ASSIGN_DEST_PORT", CTC_FLAG_ISSET(p_tx_info->tx_info.flags, CTC_PKT_FLAG_ASSIGN_DEST_PORT));

    if (p_tx_info->tx_info.oper_type == CTC_PKT_OPER_OAM)
    {
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "oam info:\n");
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "oam type", p_tx_info->tx_info.oam.type);
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "mep index", p_tx_info->tx_info.oam.mep_index);
    }

    if (p_tx_info->tx_info.oper_type == CTC_PKT_OPER_PTP)
    {
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ptp info:\n");
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "seconds", p_tx_info->tx_info.ptp.ts.seconds);
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d\n", "nanoseconds", p_tx_info->tx_info.ptp.ts.nanoseconds);
    }

    if (dump_mode)
    {
        if (p_tx_info->skb.data)
        {
            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
            if (CTC_PKT_MODE_ETH == p_tx_info->mode)
            {
                CTC_ERROR_RETURN(_sys_usw_packet_dump(lchip, p_tx_info->skb.data+SYS_USW_PKT_CPUMAC_LEN, SYS_USW_PKT_HEADER_LEN));
            }
            else
            {
                if (!is_skb_buf)
                {
                    CTC_ERROR_RETURN(_sys_usw_packet_dump(lchip, (p_tx_info->skb.buf + CTC_PKT_HDR_ROOM - SYS_USW_PKT_HEADER_LEN), SYS_USW_PKT_HEADER_LEN));
                }
                else
                {
                    CTC_ERROR_RETURN(_sys_usw_packet_dump(lchip, p_tx_info->skb.data, SYS_USW_PKT_HEADER_LEN));
                }
            }
        }
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------\n");
    }


    return CTC_E_NONE;
}

static int32
_sys_usw_packet_bit62_to_ts(uint8 lchip, uint32 ns_only_format, uint64 ts_61_0, ctc_packet_ts_t* p_ts)
{
    if (ns_only_format)
    {
        /* [61:0] nano seconds */
        p_ts->seconds = ts_61_0 / 1000000000ULL;
        p_ts->nanoseconds = ts_61_0 % 1000000000ULL;
    }
    else
    {
        /* [61:30] seconds + [29:0] nano seconds */
        p_ts->seconds = (ts_61_0 >> 30) & 0xFFFFFFFFULL;
        p_ts->nanoseconds = ts_61_0 & 0x3FFFFFFFULL;
    }

    return CTC_E_NONE;
}

static int32
_sys_usw_packet_ts_to_bit62(uint8 lchip, uint32 ns_only_format, ctc_packet_ts_t* p_ts, uint64* p_ts_61_0)
{
    if (ns_only_format)
    {
        /* [61:0] nano seconds */
        *p_ts_61_0 = p_ts->seconds * 1000000000ULL + p_ts->nanoseconds;
    }
    else
    {
        /* [61:30] seconds + [29:0] nano seconds */
        *p_ts_61_0 = ((uint64)(p_ts->seconds) << 30) | ((uint64)p_ts->nanoseconds);
    }

    return CTC_E_NONE;
}

static int32
_sys_usw_packet_recover_timestamp(uint8 lchip, uint32 ts_36_5, uint64* p_ts_61_0)
{
    uint32 cmd = 0;
    uint32 rc_s = 0;
    uint32 rc_ns = 0;
    uint64 rc_36_5 = 0;
    uint64 rc_61_0 = 0;
    uint64 delta_36_5 = 0;
    uint64 delta_36_0 = 0;
    uint8 delta_s = 0;
    TsEngineRefRc_m ptp_ref_frc;

    /*read frc time, 32bit s + 30bit ns*/
    sal_memset(&ptp_ref_frc, 0, sizeof(ptp_ref_frc));
    cmd = DRV_IOR(TsEngineRefRc_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ptp_ref_frc));

    GetTsEngineRefRc(A, rcSecond_f, &ptp_ref_frc, &rc_s);
    GetTsEngineRefRc(A, rcNs_f, &ptp_ref_frc, &rc_ns);

    rc_61_0 = ((uint64)rc_s) * 1000000000ULL + rc_ns;
    rc_36_5 = (rc_61_0 >> 5) & 0xFFFFFFFF;

    if (rc_36_5 >= ts_36_5)
    {
        delta_36_5 = rc_36_5 - ts_36_5;
    }
    else
    {
        delta_36_5 = 0x100000000ULL + rc_36_5 - ts_36_5;
    }

    delta_s = (delta_36_5 << 5) / 1000000000ULL;
    if (delta_s > rc_s)
    {
        return CTC_E_INVALID_CONFIG;
    }

    rc_s = rc_s - delta_s;
    delta_36_0 = (delta_36_5 << 5) - ((uint64)delta_s) * 1000000000ULL;

    if (rc_ns >= (delta_36_0 & 0x3FFFFFFF))
    {
        rc_ns = rc_ns - (delta_36_0 & 0x3FFFFFFF);
    }
    else
    {
        rc_ns = 1000000000ULL + rc_ns - (delta_36_0 & 0x3FFFFFFF);
        if (1 > rc_s)
        {
            return CTC_E_INVALID_CONFIG;
        }
        rc_s = rc_s -1;
    }

    *p_ts_61_0 = ((uint64)rc_s) * 1000000000ULL + rc_ns;

    return CTC_E_NONE;
}

static int32
_sys_usw_packet_rx_dump(uint8 lchip, ctc_pkt_rx_t* p_pkt_rx)
{
    uint32 len = 0;
    uint8 header_len = 0;

    if (CTC_BMP_ISSET(p_usw_pkt_master[lchip]->rx_reason_bm, p_pkt_rx->rx_info.reason))
    {
        header_len = (p_pkt_rx->mode == CTC_PKT_MODE_ETH)?(SYS_USW_PKT_HEADER_LEN+SYS_USW_PKT_CPUMAC_LEN):SYS_USW_PKT_HEADER_LEN;
        if (p_pkt_rx->pkt_len < header_len)
        {
            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Packet length is too small!!!Pkt len: %d\n",p_pkt_rx->pkt_len);
            return CTC_E_INVALID_PARAM;
        }

        if (p_usw_pkt_master[lchip]->rx_header_en[p_pkt_rx->rx_info.reason])
        {
            CTC_ERROR_RETURN(_sys_usw_packet_dump_rx_header(lchip, p_pkt_rx));
        }

        if (p_pkt_rx->pkt_len - header_len < SYS_PKT_BUF_PKT_LEN)
        {
            len = p_pkt_rx->pkt_len- header_len;
        }
        else
        {
            len = SYS_PKT_BUF_PKT_LEN;
        }

        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n-----------------------------------------------\n");
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Packet Info(Length : %d):\n", (p_pkt_rx->pkt_len-header_len));
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------\n");
        CTC_ERROR_RETURN(_sys_usw_packet_dump(lchip, (p_pkt_rx->pkt_buf[0].data+header_len), len));
    }

    return 0;
}
static int32
_sys_usw_packet_tx_dump(uint8 lchip, ctc_pkt_tx_t* p_pkt_tx)
{
    uint32 len = 0;
    uint8 header_len = 0;
    uint8 is_skb_buf = 0;

    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    is_skb_buf = (((p_pkt_tx->skb.head == NULL) ||
                   (p_pkt_tx->skb.tail == NULL) ||
                   (p_pkt_tx->skb.end == NULL)) &&
                   (p_pkt_tx->skb.data != NULL) ? 0:1);

    if(p_usw_pkt_master[lchip]->tx_dump_enable)
    {
        CTC_ERROR_RETURN(_sys_usw_packet_dump_tx_header(lchip, p_pkt_tx,is_skb_buf,1));

        header_len = (p_pkt_tx->mode == CTC_PKT_MODE_ETH)?(SYS_USW_PKT_HEADER_LEN+SYS_USW_PKT_CPUMAC_LEN):SYS_USW_PKT_HEADER_LEN;

        /*p_pkt_tx->skb.len ,to ETH include the header_len,to DMA exclude the header_len*/
        if (p_pkt_tx->mode == CTC_PKT_MODE_ETH)
        {
            if (p_pkt_tx->skb.len - header_len < SYS_PKT_BUF_PKT_LEN)
            {
                len = p_pkt_tx->skb.len - header_len;
            }
            else
            {
                len = SYS_PKT_BUF_PKT_LEN;
            }
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

        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n-----------------------------------------------\n");
        if (p_pkt_tx->mode == CTC_PKT_MODE_ETH)
        {
            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Packet Info(Length : %d):\n", (p_pkt_tx->skb.len - header_len));
        }
        else
        {
            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Packet Info(Length : %d):\n", p_pkt_tx->skb.len);
        }
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------\n");

        if(CTC_PKT_MODE_DMA == p_pkt_tx->mode)
        {
            if(is_skb_buf)
            {
                CTC_ERROR_RETURN(_sys_usw_packet_dump(lchip, (p_pkt_tx->skb.data+header_len), len));
            }
            else
            {
                CTC_ERROR_RETURN(_sys_usw_packet_dump(lchip, p_pkt_tx->skb.data, len));
            }
        }
        else
        {
            CTC_ERROR_RETURN(_sys_usw_packet_dump(lchip, (p_pkt_tx->skb.data+header_len), len));
        }

    }
    return 0;
}

STATIC int32
_sys_usw_packet_get_svlan_tpid_index(uint8 lchip, uint16 svlan_tpid, uint8* svlan_tpid_index, sys_pkt_tx_hdr_info_t* p_tx_hdr_info)
{
    int32 ret = CTC_E_NONE;
    CTC_PTR_VALID_CHECK(p_tx_hdr_info);

    if ((0 == svlan_tpid) || (svlan_tpid == p_tx_hdr_info->tp_id[0]))
    {
        *svlan_tpid_index = 0;
    }
    else if (svlan_tpid == p_tx_hdr_info->tp_id[1])
    {
        *svlan_tpid_index = 1;
    }
    else if (svlan_tpid == p_tx_hdr_info->tp_id[2])
    {
        *svlan_tpid_index = 2;
    }
    else if (svlan_tpid == p_tx_hdr_info->tp_id[3])
    {
        *svlan_tpid_index = 3;
    }
    else
    {
        ret = CTC_E_INVALID_PARAM;
    }

    return ret;
}

STATIC int32
_sys_usw_packet_init_svlan_tpid_index(uint8 lchip)
{
    uint32 cmd = 0;
    ParserEthernetCtl_m parser_ethernet_ctl;
    int32 ret = CTC_E_NONE;

    sal_memset(&parser_ethernet_ctl, 0, sizeof(ParserEthernetCtl_m));
    cmd = DRV_IOR(ParserEthernetCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &parser_ethernet_ctl));

    p_usw_pkt_master[lchip]->tx_hdr_info.tp_id[0] = GetParserEthernetCtl(V, array_0_svlanTpid_f, &parser_ethernet_ctl);
    p_usw_pkt_master[lchip]->tx_hdr_info.tp_id[1] = GetParserEthernetCtl(V, array_1_svlanTpid_f, &parser_ethernet_ctl);
    p_usw_pkt_master[lchip]->tx_hdr_info.tp_id[2] = GetParserEthernetCtl(V, array_2_svlanTpid_f, &parser_ethernet_ctl);
    p_usw_pkt_master[lchip]->tx_hdr_info.tp_id[3] = GetParserEthernetCtl(V, array_3_svlanTpid_f, &parser_ethernet_ctl);

    return ret;
}

int32
sys_usw_packet_tx_set_property(uint8 lchip, uint16 type, uint32 value1, uint32 value2)
{
    int32 ret = CTC_E_NONE;
    SYS_PACKET_INIT_CHECK(lchip);

    SYS_PACKET_LOCK(lchip);
    switch(type)
    {
        case SYS_PKT_TX_TYPE_TPID:
            p_usw_pkt_master[lchip]->tx_hdr_info.tp_id[value1] = value2;
            break;
        case SYS_PKT_TX_TYPE_C2C_SUB_QUEUE_ID:
            p_usw_pkt_master[lchip]->tx_hdr_info.c2c_sub_queue_id = value1;
            break;
        case SYS_PKT_TX_TYPE_FWD_CPU_SUB_QUEUE_ID:
            p_usw_pkt_master[lchip]->tx_hdr_info.fwd_cpu_sub_queue_id = value1;
            break;
        case SYS_PKT_TX_TYPE_PTP_ADJUST_OFFSET:
            p_usw_pkt_master[lchip]->tx_hdr_info.offset_ns = value1;
            p_usw_pkt_master[lchip]->tx_hdr_info.offset_s = value2;
            break;
        case SYS_PKT_TX_TYPE_VLAN_PTR:
            p_usw_pkt_master[lchip]->tx_hdr_info.vlanptr[value1] = value2;
            break;
        case SYS_PKT_TX_TYPE_RSV_NH:
            p_usw_pkt_master[lchip]->tx_hdr_info.rsv_nh[value1] = value2;
            break;
        default:
            ret = CTC_E_INVALID_PARAM;
            break;

    }
    SYS_PACKET_UNLOCK(lchip);

    return ret;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!WARNING!!!!!!!!!!!!!!!!!!!!!!!!!!!
 Note that you should not add other module APIs in this function.
 If you need to add it, save it to the packet module database.*/
int32
_sys_usw_packet_txinfo_to_rawhdr(uint8 lchip, ctc_pkt_info_t* p_tx_info, uint32* p_raw_hdr_net, uint8 is_skb_buf, uint8 mode, uint8* data)
{
    uint32 bheader[SYS_USW_PKT_HEADER_LEN/4] = {0};
    uint32* p_raw_hdr              = bheader;
    uint32 dest_map                = 0;
    uint16 vlan_ptr                = 0;
    uint8* pkt_data                = NULL;
    uint8 gchip                    = 0;
    uint32 lport                   = 0;
    uint8 hash                     = 0;
    uint8 src_gchip                = 0;
    uint8 packet_type              = 0;
    uint8 priority                 = 0;
    uint8 color                    = 0;
    uint32 src_port                = 0;
    uint32 next_hop_ptr            = 0;
    uint32 ts_36_5 = 0;
    uint32 ts_4_0 = 0;
    uint32 ts_61_37 = 0;
    uint64 ts_61_0 = 0;
    uint32 offset_ns = 0;
    uint32 offset_s = 0;
    uint64 offset = 0;
    uint8  svlan_tpid_index = 0;
    uint8  chan_id = 0xFF;
    sys_nh_info_dsnh_t      sys_nh_info;
    /*uint32 cmd = 0;*/
    uint32 gport = 0;
    /*TsEngineOffsetAdj_m ptp_offset_adjust;*/
    drv_work_platform_type_t platform_type = MAX_WORK_PLATFORM;
    bool* p_protect_en = NULL;
    bool protect_en = FALSE;
    uint32 mep_index = 0;
    uint8 sub_queue_id = 0;

    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    sal_memset(p_raw_hdr, 0, SYS_USW_PKT_HEADER_LEN);
    if (p_tx_info->priority >= 16)
    {
        return CTC_E_INVALID_PARAM;
    }

    drv_get_platform_type(lchip, &platform_type);
    /* Must be inited */
    SetMsPacketHeader(V , packetType_f     , p_raw_hdr , p_tx_info->packet_type);
    /* Support offload data tunnel cryption in hw */
    SetMsPacketHeader(V , packetOffset_f   , p_raw_hdr , p_tx_info->payload_offset);
    SetMsPacketHeader(V , fromCpuOrOam_f   , p_raw_hdr , TRUE);
    SetMsPacketHeader(V , operationType_f  , p_raw_hdr , p_tx_info->oper_type);
    SetMsPacketHeader(V , criticalPacket_f , p_raw_hdr , p_tx_info->is_critical);
    SetMsPacketHeader(V , srcStagCos_f      , p_raw_hdr , p_tx_info->src_cos);
    SetMsPacketHeader(V , ttl_f            , p_raw_hdr , p_tx_info->ttl);
    SetMsPacketHeader(V , macKnown_f       , p_raw_hdr , TRUE);
    SetMsPacketHeader(V , fid_f, p_raw_hdr , p_tx_info->fid);
    CTC_ERROR_RETURN(_sys_usw_packet_get_svlan_tpid_index(lchip, p_tx_info->svlan_tpid, &svlan_tpid_index, &p_usw_pkt_master[lchip]->tx_hdr_info));
    SetMsPacketHeader(V , svlanTpidIndex_f, p_raw_hdr , svlan_tpid_index);

    SetMsPacketHeader(V , u3_other_fromCpu_f, p_raw_hdr , TRUE);
    SetMsPacketHeader(V, logicSrcPort_f, p_raw_hdr , p_tx_info->logic_src_port);
    SetMsPacketHeader(V, logicPortType_f, p_raw_hdr , p_tx_info->logic_port_type);

    if (CTC_FLAG_ISSET(p_tx_info->flags, CTC_PKT_FLAG_DIAG_PKT))
    {
        SetMsPacketHeader(V, isDebuggedPkt_f, p_raw_hdr , 1);
    }

    /*don't eccrypt specified pkt*/
    if (CTC_FLAG_ISSET(p_tx_info->flags, CTC_PKT_FLAG_CANCEL_DOT1AE))
    {
        SetMsPacketHeader(V, u1_normal_noDot1AeEncrypt_f, p_raw_hdr, TRUE);
    }

    /*Set oamuse fid lookup */
    if (CTC_FLAG_ISSET(p_tx_info->flags, CTC_PKT_FLAG_OAM_USE_FID))
    {
        SetMsPacketHeader(V, oamUseFid_f, p_raw_hdr, TRUE);
    }

   /*Set Vlan domain */
    if (p_tx_info->vlan_domain == CTC_PORT_VLAN_DOMAIN_SVLAN)
    {
        SetMsPacketHeader(V, outerVlanIsCVlan_f, p_raw_hdr, FALSE);
        /*sys_usw_vlan_get_vlan_ptr(lchip, p_tx_info->src_svid, &vlan_ptr);*/
        vlan_ptr = p_usw_pkt_master[lchip]->tx_hdr_info.vlanptr[p_tx_info->src_svid];
        SetMsPacketHeader(V, srcVlanPtr_f, p_raw_hdr, vlan_ptr);
    }
    else  if (p_tx_info->vlan_domain == CTC_PORT_VLAN_DOMAIN_CVLAN)
    {
        SetMsPacketHeader(V, outerVlanIsCVlan_f, p_raw_hdr, TRUE);
        /*sys_usw_vlan_get_vlan_ptr(lchip, p_tx_info->src_cvid, &vlan_ptr);*/
        vlan_ptr = p_usw_pkt_master[lchip]->tx_hdr_info.vlanptr[p_tx_info->src_cvid];
        SetMsPacketHeader(V, srcVlanPtr_f, p_raw_hdr, vlan_ptr);
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    /* Next_hop_ptr and DestMap*/
    if (CTC_FLAG_ISSET(p_tx_info->flags, CTC_PKT_FLAG_BPE_TRANSPARENT))
    {
        /* Bypass NH is 8W  and egress edit mode*/
        SetMsPacketHeader(V, bypassIngressEdit_f, p_raw_hdr, TRUE);
        SetMsPacketHeader(V, nextHopExt_f, p_raw_hdr, TRUE);
        /*CTC_ERROR_RETURN(sys_usw_nh_get_resolved_offset(lchip, SYS_NH_RES_OFFSET_TYPE_BPE_TRANSPARENT_NH, &next_hop_ptr));*/
        next_hop_ptr = p_usw_pkt_master[lchip]->tx_hdr_info.rsv_nh[SYS_NH_RES_OFFSET_TYPE_BPE_TRANSPARENT_NH];
    }
    else if (CTC_FLAG_ISSET(p_tx_info->flags, CTC_PKT_FLAG_NH_OFFSET_BYPASS))
    {
        /* Bypass NH is 8W  and egress edit mode*/
        SetMsPacketHeader(V, bypassIngressEdit_f, p_raw_hdr, TRUE);
        SetMsPacketHeader(V, nextHopExt_f, p_raw_hdr, TRUE);
        /*CTC_ERROR_RETURN(sys_usw_nh_get_resolved_offset(lchip, SYS_NH_RES_OFFSET_TYPE_BYPASS_NH, &next_hop_ptr));*/
        next_hop_ptr = p_usw_pkt_master[lchip]->tx_hdr_info.rsv_nh[SYS_NH_RES_OFFSET_TYPE_BYPASS_NH];
    }
    /* SYSTEM MODIFIED by liwh for merge bug 100413 & 100427 to support 1br */
    else if (CTC_FLAG_ISSET(p_tx_info->flags, CTC_PKT_FLAG_NH_OFFSET_BYPASS_ECID))
    {
        /* GG Bypass NH is 8W  and egress edit mode*/
        SetMsPacketHeader(V, bypassIngressEdit_f, p_raw_hdr, TRUE);
        SetMsPacketHeader(V, nextHopExt_f, p_raw_hdr, TRUE);
        CTC_ERROR_RETURN(sys_usw_nh_get_resolved_offset(lchip, SYS_NH_RES_OFFSET_TYPE_BYPASS_ECID_NH, &next_hop_ptr));
    }
    /* liwh end */
    else
    {
        if (CTC_FLAG_ISSET(p_tx_info->flags, CTC_PKT_FLAG_NH_OFFSET_VALID))
        {
            next_hop_ptr = p_tx_info->nh_offset;
            if (CTC_FLAG_ISSET(p_tx_info->flags, CTC_PKT_FLAG_NH_OFFSET_IS_8W))
            {
                SetMsPacketHeader(V, nextHopExt_f, p_raw_hdr, TRUE);
            }
        }
        else
        {
            /* GG Bridge NH is 4W */
            SetMsPacketHeader(V, nextHopExt_f, p_raw_hdr, FALSE);
            /*CTC_ERROR_RETURN(sys_usw_nh_get_resolved_offset(lchip, SYS_NH_RES_OFFSET_TYPE_BRIDGE_NH, &next_hop_ptr));*/
            next_hop_ptr = p_usw_pkt_master[lchip]->tx_hdr_info.rsv_nh[SYS_NH_RES_OFFSET_TYPE_BRIDGE_NH];
        }
    }

    if (CTC_FLAG_ISSET(p_tx_info->flags, CTC_PKT_FLAG_MCAST))
    {
        dest_map = SYS_ENCODE_MCAST_IPE_DESTMAP(p_tx_info->dest_group_id);
        next_hop_ptr = 0;
    }
    else
    {
        if (CTC_IS_LINKAGG_PORT(p_tx_info->dest_gport))
        {
            dest_map = SYS_ENCODE_DESTMAP(CTC_LINKAGG_CHIPID, p_tx_info->dest_gport&0xFF);
        }
        else
        {
            gchip = CTC_MAP_GPORT_TO_GCHIP(p_tx_info->dest_gport);
            if (CTC_IS_CPU_PORT(p_tx_info->dest_gport))
            {
                /*CTC_ERROR_RETURN(sys_usw_get_sub_queue_id_by_cpu_reason(lchip, CTC_PKT_CPU_REASON_FWD_CPU, &sub_queue_id));*/
                sub_queue_id = p_usw_pkt_master[lchip]->tx_hdr_info.fwd_cpu_sub_queue_id;
                dest_map = SYS_ENCODE_EXCP_DESTMAP(gchip, sub_queue_id);
            }
            else
            {
                dest_map = SYS_ENCODE_DESTMAP(gchip, SYS_MAP_CTC_GPORT_TO_DRV_LPORT(p_tx_info->dest_gport));
            }
        }
    }

    if (CTC_FLAG_ISSET(p_tx_info->flags, CTC_PKT_FLAG_NHID_VALID))
    {
        sal_memset(&sys_nh_info, 0, sizeof(sys_nh_info_dsnh_t));
        if(CTC_E_NONE == sys_usw_nh_get_nhinfo(lchip, p_tx_info->nhid, &sys_nh_info, 1))
        {
            next_hop_ptr = sys_nh_info.dsnh_offset;
            SetMsPacketHeader(V, nextHopExt_f, p_raw_hdr, sys_nh_info.nexthop_ext);
            dest_map = sys_nh_info.dest_map;
            if(sys_nh_info.aps_en)
            {
                /*if (CTC_FLAG_ISSET(p_tx_info->flags, CTC_PKT_FLAG_APS_PATH_EN))
                {
                    mep_index = p_tx_info->aps_p_path ? 0x3 : 0x1;
                    protect_en = p_tx_info->aps_p_path;
                    p_protect_en = &protect_en;

                    if (!sys_nh_info.mep_on_tunnel)
                    {
                        mep_index = 0;
                    }
                }*/

                 /* system modified by liwh & lil for bug 46591, 2018-05-15 */
                if (CTC_FLAG_ISSET(p_tx_info->flags, CTC_PKT_FLAG_APS_W_PATH))
                {
                    mep_index |= 0x1;
                    p_protect_en = &protect_en;
                }
                else if (CTC_FLAG_ISSET(p_tx_info->flags, CTC_PKT_FLAG_APS_P_PATH))
                {
                    mep_index |= 0x3;
                    protect_en = TRUE;
                    p_protect_en = &protect_en;
                }

                if (!sys_nh_info.mep_on_tunnel)
                {
                    mep_index = 0;
                }
                /*ended*/

                CTC_ERROR_RETURN(sys_usw_nh_get_aps_working_path(lchip, p_tx_info->nhid, &gport, &next_hop_ptr, p_protect_en));
                dest_map = SYS_ENCODE_DESTMAP(SYS_MAP_CTC_GPORT_TO_GCHIP(gport), SYS_MAP_CTC_GPORT_TO_DRV_LPORT(gport));
            }
        }

        if (sys_nh_info.dsnh_offset & SYS_NH_DSNH_BY_PASS_FLAG)
        {
            next_hop_ptr = sys_nh_info.dsnh_offset & (~SYS_NH_DSNH_BY_PASS_FLAG);
            SetMsPacketHeader(V, bypassIngressEdit_f, p_raw_hdr, 1);
        }
    }

    SetMsPacketHeader(V, nextHopPtr_f, p_raw_hdr, next_hop_ptr);
    p_tx_info->nh_offset = next_hop_ptr;

    if (CTC_FLAG_ISSET(p_tx_info->flags, CTC_PKT_FLAG_ASSIGN_DEST_PORT))
    {
        uint8 gchip = 0;

        sys_usw_get_gchip_id(lchip, &gchip);
        chan_id = SYS_GET_CHANNEL_ID(lchip, CTC_MAP_LPORT_TO_GPORT(gchip, p_tx_info->lport));
        if(0xff != chan_id)
        {
            SetMsPacketHeader(V, u3_other_destChannelIdFromCpu_f, p_raw_hdr, chan_id);
            SetMsPacketHeader(V, u3_other_destChannelIdValidFromCpu_f, p_raw_hdr, TRUE);
        }
    }

    /* SrcPort */
    sys_usw_get_gchip_id(lchip, &src_gchip);
    if (CTC_FLAG_ISSET(p_tx_info->flags, CTC_PKT_FLAG_SRC_PORT_VALID))
    {
        src_port = p_tx_info->src_port;
        if(CTC_IS_LINKAGG_PORT(p_tx_info->src_port))
        {
            SetMsPacketHeader(V, fromLag_f, p_raw_hdr, 1);
            src_port = CTC_MAP_LPORT_TO_GPORT(src_gchip, CTC_MAP_GPORT_TO_LPORT(src_port));
        }
    }
    else
    {
        src_port = CTC_MAP_LPORT_TO_GPORT(src_gchip, SYS_RSV_PORT_OAM_CPU_ID);
    }
    src_port = SYS_MAP_CTC_GPORT_TO_DRV_GPORT(src_port);
    SetMsPacketHeader(V, sourcePort_f, p_raw_hdr, src_port);

    /* Svid */
    if (CTC_FLAG_ISSET(p_tx_info->flags, CTC_PKT_FLAG_SRC_SVID_VALID))
    {
        SetMsPacketHeader(V, srcSvlanId_f, p_raw_hdr, p_tx_info->src_svid);
        SetMsPacketHeader(V, stagAction_f, p_raw_hdr, CTC_VLAN_TAG_OP_ADD);
        SetMsPacketHeader(V, svlanTagOperationValid_f, p_raw_hdr, TRUE);
    }

    /* Cvid */
    if (CTC_FLAG_ISSET(p_tx_info->flags, CTC_PKT_FLAG_SRC_CVID_VALID))
    {
        SetMsPacketHeader(V, u1_normal_srcCvlanId_f, p_raw_hdr, p_tx_info->src_cvid);
        SetMsPacketHeader(V, u1_normal_cTagAction_f, p_raw_hdr, CTC_VLAN_TAG_OP_ADD);
        SetMsPacketHeader(V, u1_normal_cvlanTagOperationValid_f, p_raw_hdr, TRUE);
    }

    /* Linkagg Hash*/
    if (CTC_FLAG_ISSET(p_tx_info->flags, CTC_PKT_FLAG_HASH_VALID))
    {
        hash = p_tx_info->hash;
    }
    else
    {
        pkt_data = is_skb_buf ?((uint8*)p_raw_hdr_net + SYS_USW_PKT_HEADER_LEN):data;
        hash = ctc_crc_calculate_crc8(pkt_data, 12, 0);
    }
    SetMsPacketHeader(V, headerHash_f, p_raw_hdr, hash);


    /* PHB priority + color */
    priority = (CTC_FLAG_ISSET(p_tx_info->flags, CTC_PKT_FLAG_PRIORITY)) ? p_tx_info->priority : 15;
    color = (CTC_FLAG_ISSET(p_tx_info->flags, CTC_PKT_FLAG_COLOR)) ? p_tx_info->color : CTC_QOS_COLOR_GREEN;
    SetMsPacketHeader(V, color_f, p_raw_hdr, color);

    /* Iloop */
    if (CTC_FLAG_ISSET(p_tx_info->flags, CTC_PKT_FLAG_INGRESS_MODE))
    {
        gchip = CTC_MAP_GPORT_TO_GCHIP(p_tx_info->dest_gport);
        lport = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(p_tx_info->dest_gport);
        /* dest_map is ILOOP */
        dest_map = SYS_ENCODE_DESTMAP(gchip, SYS_RSV_PORT_ILOOP_ID);
        dest_map = dest_map | (lport&0x100); /*sliceId*/
        SetMsPacketHeader(V, destMap_f, p_raw_hdr, dest_map);

        /* nexthop_ptr is lport */
        next_hop_ptr = SYS_PACKET_ENCAP_LOOP_NHPTR(lport, 0);
        SetMsPacketHeader(V, nextHopPtr_f, p_raw_hdr, next_hop_ptr);
    }

    /* OAM Operation*/
    if (CTC_PKT_OPER_OAM == p_tx_info->oper_type)
    {
        SetMsPacketHeader(V, u1_oam_mepIndex_f, p_raw_hdr, mep_index);

        SetMsPacketHeader(V, u1_oam_oamType_f, p_raw_hdr, p_tx_info->oam.type);
        SetMsPacketHeader(V, u1_oam_rxOam_f, p_raw_hdr, FALSE);
         /*SetMsPacketHeader(V, u1_oam_entropyLabelExist_f, p_raw_hdr, FALSE);*/
         /*SetMsPacketHeader(V, u1_oam_isEloopPkt_f, p_raw_hdr, FALSE);*/

        if (CTC_FLAG_ISSET(p_tx_info->oam.flags, CTC_PKT_OAM_FLAG_IS_DM))
        {
            SetMsPacketHeader(V, u1_oam_dmEn_f, p_raw_hdr, TRUE);
            SetMsPacketHeader(V, u1_dmtx_dmOffset_f, p_raw_hdr, p_tx_info->oam.dm_ts_offset);
        }
        else
        {
            SetMsPacketHeader(V, u1_oam_dmEn_f, p_raw_hdr, FALSE);
        }

        if (CTC_FLAG_ISSET(p_tx_info->oam.flags, CTC_PKT_OAM_FLAG_LINK_OAM))
        {
            SetMsPacketHeader(V, u1_oam_linkOam_f, p_raw_hdr, TRUE);
        }

        /* TPOAM Y.1731 Section need not append 13 label */
        if (CTC_OAM_TYPE_ACH == p_tx_info->oam.type)
        {
            SetMsPacketHeader(V, u2_oam_mipEn_f, p_raw_hdr, TRUE);
            /* only ACH encapsulation need to change packet_type */
            if (CTC_FLAG_ISSET(p_tx_info->oam.flags, CTC_PKT_OAM_FLAG_HAS_MPLS_GAL))
            {
                SetMsPacketHeader(V, u1_oam_galExist_f, p_raw_hdr, TRUE);
                packet_type = CTC_PARSER_PKT_TYPE_MPLS;
            }
            else
            {
                packet_type = CTC_PARSER_PKT_TYPE_RESERVED;
            }
            SetMsPacketHeader(V, packetType_f, p_raw_hdr, packet_type);
        }

        /* set for UP MEP */
        if (CTC_FLAG_ISSET(p_tx_info->oam.flags, CTC_PKT_OAM_FLAG_IS_UP_MEP))
        {
            SetMsPacketHeader(V, u1_oam_isUp_f, p_raw_hdr, TRUE);
            gchip = CTC_MAP_GPORT_TO_GCHIP(p_tx_info->dest_gport);
            lport = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(p_tx_info->dest_gport);
            /* dest_map is ILOOP */
            dest_map = SYS_ENCODE_DESTMAP(gchip, SYS_RSV_PORT_ILOOP_ID);
            dest_map = dest_map | (lport&0x100);/*SliceId*/
            SetMsPacketHeader(V, destMap_f, p_raw_hdr, dest_map);

            next_hop_ptr = SYS_PACKET_ENCAP_LOOP_NHPTR(lport, (p_tx_info->ttl != 0));
            SetMsPacketHeader(V, nextHopPtr_f, p_raw_hdr, next_hop_ptr);

            /* src vlan_ptr is vlan_ptr by vid */
            /*sys_usw_vlan_get_vlan_ptr(lchip, p_tx_info->oam.vid, &vlan_ptr);*/
            vlan_ptr = p_usw_pkt_master[lchip]->tx_hdr_info.vlanptr[p_tx_info->oam.vid];
            SetMsPacketHeader(V, srcVlanPtr_f, p_raw_hdr, vlan_ptr);

        }

        /* set bypass MIP port */
        if (CTC_FLAG_ISSET(p_tx_info->oam.flags, CTC_PKT_OAM_FLAG_TX_MIP_TUNNEL))
        {
            /* bypass OAM packet to MIP configured port; otherwise, packet will send to CPU again */
            SetMsPacketHeader(V, u1_oam_oamType_f, p_raw_hdr, CTC_OAM_TYPE_NONE);
            SetMsPacketHeader(V, oamTunnelEn_f, p_raw_hdr, TRUE);
        }
    }
    else if (CTC_PKT_OPER_PTP == p_tx_info->oper_type)
    {
        /* 7. get PTP */
        /*  DM Timestamp Bits Mapping
            Timestamp               Field Name          BIT Width   BIT Base
            dmTimestamp[36:5]       timestamp_36_5           32          5
            dmTimestamp[4:0]      u1_ptp_timestamp_4_0  5          0
            dmTimestamp[61:37]      u1_ptp_timestamp_61_37  25          37
         */
        _sys_usw_packet_ts_to_bit62(lchip, TRUE, &p_tx_info->ptp.ts, &ts_61_0);

        if ((CTC_PTP_CORRECTION == p_tx_info->ptp.oper) && (!(CTC_FLAG_ISSET(p_tx_info->flags, CTC_PKT_FLAG_PTP_TS_PRECISION))))
        {
            /*sal_memset(&ptp_offset_adjust, 0, sizeof(ptp_offset_adjust));
            cmd = DRV_IOR(TsEngineOffsetAdj_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ptp_offset_adjust));

            GetTsEngineOffsetAdj(A, offsetNs_f, &ptp_offset_adjust, &offset_ns);
            GetTsEngineOffsetAdj(A, offsetSecond_f, &ptp_offset_adjust, &offset_s);*/
            offset_ns = p_usw_pkt_master[lchip]->tx_hdr_info.offset_ns;
            offset_s = p_usw_pkt_master[lchip]->tx_hdr_info.offset_s;
            offset = offset_s * 1000000000ULL + offset_ns;
            if (ts_61_0 >= offset)
            {
                ts_61_0 = ts_61_0 -offset;
            }
            else
            {
                return CTC_E_INVALID_PARAM;
            }
        }

        ts_4_0 = ts_61_0 & 0x1FULL;
        ts_36_5 = (ts_61_0 >> 5) & 0xFFFFFFFFULL;
        ts_61_37 = (ts_61_0 >> 37) & 0x01FFFFFFULL;
        SetMsPacketHeader(V, timestamp_36_5_f, p_raw_hdr, ts_36_5);
        SetMsPacketHeader(V, u1_ptp_timestamp_4_0_f, p_raw_hdr, ts_4_0);
        SetMsPacketHeader(V, u1_ptp_timestamp_61_37_f, p_raw_hdr, ts_61_37);

        switch (p_tx_info->ptp.oper)
        {
            case CTC_PTP_CAPTURE_ONLY:
            {
                /* Duet2 do not support  CTC_PTP_CAPTURE_ONLY */
                return CTC_E_INVALID_PARAM;
            }
            case CTC_PTP_REPLACE_ONLY:
            {
                SetMsPacketHeader(V, u1_ptp_ptpReplaceTimestamp_f, p_raw_hdr, TRUE);
                SetMsPacketHeader(V, u1_ptp_ptpOffset_f, p_raw_hdr, p_tx_info->ptp.ts_offset);
                break;
            }
            case CTC_PTP_NULL_OPERATION:
            {
                /* do nothing */
                break;
            }
            case CTC_PTP_CORRECTION:
            {
                /*SetMsPacketHeader(V, u1_ptp_ptpReplaceTimestamp_f, p_raw_hdr, TRUE);*/
                SetMsPacketHeader(V, u1_ptp_ptpUpdateResidenceTime_f, p_raw_hdr, TRUE);
                SetMsPacketHeader(V, u1_ptp_ptpOffset_f, p_raw_hdr, p_tx_info->ptp.ts_offset);
                break;
            }
            default:
            {
                return CTC_E_INVALID_PARAM;
            }
        }
    }
    else if (CTC_PKT_OPER_C2C == p_tx_info->oper_type)
    {
        uint8  sub_queue_id = 0;
        uint8  grp_id       = 0;
        SetMsPacketHeader(V, logicSrcPort_f, p_raw_hdr, 0); /*logic port mean c2c extend group id*/
        SetMsPacketHeader(V, bypassIngressEdit_f, p_raw_hdr, 1);
        if (CTC_FLAG_ISSET(p_tx_info->flags, CTC_PKT_FLAG_MCAST))
        {
            SetMsPacketHeader(V, u1_c2c_c2cCheckDisable_f, p_raw_hdr, 1);
        }

        if (!CTC_FLAG_ISSET(p_tx_info->flags, CTC_PKT_FLAG_MCAST) &&
            !CTC_FLAG_ISSET(p_tx_info->flags, CTC_PKT_FLAG_INGRESS_MODE))
        {
            if (!sys_usw_chip_is_local(lchip, SYS_DECODE_DESTMAP_GCHIP(dest_map)) ||
                 CTC_FLAG_ISSET(p_tx_info->flags, CTC_PKT_FLAG_ASSIGN_DEST_PORT))
            {
                /*CTC_ERROR_RETURN(sys_usw_get_sub_queue_id_by_cpu_reason(lchip, CTC_PKT_CPU_REASON_C2C_PKT, &sub_queue_id));*/
                sub_queue_id = p_usw_pkt_master[lchip]->tx_hdr_info.c2c_sub_queue_id;
                grp_id = (sub_queue_id / MCHIP_CAP(SYS_CAP_REASON_GRP_QUEUE_NUM)) + (priority/8);
                /*to cpu based on prio*/
                dest_map = SYS_ENCODE_EXCP_DESTMAP_GRP(gchip, grp_id);
                dest_map |= 0x10;
            }
            else
            {
                SetMsPacketHeader(V, u1_c2c_neighborDiscoverPacket_f, p_raw_hdr, 1);
                SetMsPacketHeader(V, nextHopPtr_f, p_raw_hdr,
                                  CTC_PKT_CPU_REASON_BUILD_NHPTR(CTC_PKT_CPU_REASON_C2C_PKT, 0));
                SetMsPacketHeader(V, bypassIngressEdit_f, p_raw_hdr, TRUE);
            }
        }
    }

    SetMsPacketHeader(V, prio_f, p_raw_hdr, priority);
    SetMsPacketHeader(V, destMap_f, p_raw_hdr, dest_map);

    if(CTC_PKT_MODE_ETH == mode)
    {
        /*6bytes MACDA + 6 bytes MACSA + 4 bytes VLAN Tag + 2 bytes EtherType + 40 bytes bridge header*/
        SetMsPacketHeader(V, packetOffset_f, p_raw_hdr, 58);
    }

    if (platform_type == HW_PLATFORM)
    {
        CTC_ERROR_RETURN(sys_usw_dword_reverse_copy(p_raw_hdr_net, p_raw_hdr, SYS_USW_PKT_HEADER_LEN/4));
        CTC_ERROR_RETURN(sys_usw_swap32((uint32*)p_raw_hdr_net, SYS_USW_PKT_HEADER_LEN / 4, FALSE))
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_byte_reverse_copy((uint8*)p_raw_hdr_net, (uint8*)p_raw_hdr, SYS_USW_PKT_HEADER_LEN));
    }

    return CTC_E_NONE;
}


static int32
_sys_usw_packet_rawhdr_to_rxinfo(uint8 lchip, uint32* p_raw_hdr_net, ctc_pkt_rx_t* p_pkt_rx)
{
    uint32 bheader[SYS_USW_PKT_HEADER_LEN/4] = {0};
    uint32* p_raw_hdr = bheader;
    uint32 dest_map                = 0;
    uint32 ts_36_5 = 0;
    uint32 ts_4_0 = 0;
    uint32 ts_61_37 = 0;
    uint64 ts_61_0 = 0;
    uint32 src_port                = 0;
    uint32 next_hop_ptr            = 0;
    ctc_pkt_info_t* p_rx_info      = NULL;
    uint32 value = 0;
    uint8 gchip_id = 0;
    uint32 offset_ns = 0;
    uint32 offset_s = 0;
    uint32 cmd = 0;
    uint16 vlan_ptr = 0;
    uint16 lport = 0;

    TsEngineOffsetAdj_m ptp_offset_adjust;

    p_rx_info = &p_pkt_rx->rx_info;

    CTC_ERROR_RETURN(sys_usw_dword_reverse_copy((uint32*)bheader, (uint32*)p_raw_hdr_net, SYS_USW_PKT_HEADER_LEN/4));
    CTC_ERROR_RETURN(sys_usw_swap32((uint32*)p_raw_hdr, SYS_USW_PKT_HEADER_LEN / 4, FALSE));

    sal_memset(p_rx_info, 0, sizeof(ctc_pkt_info_t));

    /* Must be inited */
    p_rx_info->packet_type    = GetMsPacketHeader(V, packetType_f, p_raw_hdr);
    p_rx_info->oper_type      = GetMsPacketHeader(V, operationType_f, p_raw_hdr);
    p_rx_info->priority       = GetMsPacketHeader(V, prio_f, p_raw_hdr);
    p_rx_info->color          = GetMsPacketHeader(V, color_f, p_raw_hdr);
    p_rx_info->src_cos        = GetMsPacketHeader(V, srcStagCos_f, p_raw_hdr);
    p_rx_info->fid          = GetMsPacketHeader(V, fid_f, p_raw_hdr);
    p_rx_info->payload_offset = GetMsPacketHeader(V, packetOffset_f, p_raw_hdr);
    p_rx_info->ttl            = GetMsPacketHeader(V, ttl_f, p_raw_hdr);
    p_rx_info->is_critical    = GetMsPacketHeader(V, criticalPacket_f, p_raw_hdr);
    p_rx_info->logic_src_port  = GetMsPacketHeader(V, logicSrcPort_f, p_raw_hdr);
    p_rx_info->lport            = GetMsPacketHeader(V, localPhyPort_f, p_raw_hdr);

    if (!GetMsPacketHeader(V, macKnown_f, p_raw_hdr))
    {
        CTC_SET_FLAG(p_rx_info->flags, CTC_PKT_FLAG_UNKOWN_MACDA);
    }

    if (CTC_PKT_OPER_NORMAL == p_rx_info->oper_type)
    {
        if (GetMsPacketHeader(V, u1_normal_metadataType_f, p_raw_hdr) == 0x01)
        {
            /*meta*/
            p_rx_info->meta_data= GetMsPacketHeader(V, u1_normal_metadata_f, p_raw_hdr);
            CTC_SET_FLAG(p_rx_info->flags, CTC_PKT_FLAG_METADATA_VALID);
        }
        else if (GetMsPacketHeader(V, u1_normal_metadataType_f, p_raw_hdr) == 0)
        {
            /*vrfid*/
            p_rx_info->vrfid= GetMsPacketHeader(V, u1_normal_metadata_f, p_raw_hdr);
            CTC_SET_FLAG(p_rx_info->flags, CTC_PKT_FLAG_VRFID_VALID);
        }
    }
    src_port = GetMsPacketHeader(V, sourcePort_f, p_raw_hdr);
    p_rx_info->src_chip = SYS_DRV_GPORT_TO_GCHIP(src_port);
    if (GetMsPacketHeader(V, fromLag_f, p_raw_hdr))
    {
        p_rx_info->src_port = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(CTC_LINKAGG_CHIPID, SYS_DRV_GPORT_TO_LPORT(src_port));
    }
    else
    {
        p_rx_info->src_port = SYS_MAP_DRV_GPORT_TO_CTC_GPORT(src_port);
    }

    gchip_id = p_rx_info->src_chip;
    if(sys_usw_chip_is_local(lchip, gchip_id) && !CTC_IS_LINKAGG_PORT(p_rx_info->src_port))
    {
        sys_usw_port_get_local_phy_port(lchip, p_rx_info->src_port, &value);
        if (SYS_INVALID_LOCAL_PHY_PORT != value)
        {
            p_rx_info->src_port = CTC_MAP_LPORT_TO_GPORT(p_rx_info->src_chip, value);
        }
    }
    if (SYS_DRV_GPORT_TO_LPORT(src_port) >= MCHIP_CAP(SYS_CAP_SPEC_MAX_PHY_PORT_NUM))
    {
        CTC_SET_FLAG(p_rx_info->flags, CTC_PKT_FLAG_INTERNAl_PORT);
    }

    /* RX Reason from next_hop_ptr */
    next_hop_ptr = GetMsPacketHeader(V, nextHopPtr_f, p_raw_hdr);
    p_rx_info->reason = CTC_PKT_CPU_REASON_GET_BY_NHPTR(next_hop_ptr);
    if ((p_rx_info->reason == CTC_PKT_CPU_REASON_DIAG_DISCARD_PKT)
        && (next_hop_ptr & 0x1))
    {
        /* EPE Discard */
        cmd = DRV_IOR(EpeHeaderEditCtl_t, EpeHeaderEditCtl_dropPktLogWithDiscardType_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        if (value)
        {
            /*ttl means discard type, set bit6 means EPE discard type*/
            p_rx_info->ttl |= 0x40;
        }
    }
    /*EFM src port*/
    if (SYS_DRV_GPORT_TO_LPORT(src_port) >= MCHIP_CAP(SYS_CAP_SPEC_MAX_PHY_PORT_NUM)
        && (sys_usw_chip_is_local(lchip, gchip_id) && !CTC_IS_LINKAGG_PORT(p_rx_info->src_port)))
    {
        lport = SYS_DRV_GPORT_TO_LPORT(src_port);
        sys_usw_port_lport_convert(lchip, SYS_DRV_GPORT_TO_LPORT(src_port), &lport);
        p_rx_info->src_port = CTC_MAP_LPORT_TO_GPORT(gchip_id, lport);
    }
    if (GetMsPacketHeader(V, operationType_f, p_raw_hdr) == 2)
    {
        uint32 cmd = 0;
        uint32 decap_nh = 0;
        uint32 ttl_limit = 0;

        cmd = DRV_IOR(IpeFwdCtl_t, IpeFwdCtl_capwapDecapNextHopPtr_f);
        DRV_IOCTL(lchip, 0, cmd, &decap_nh);
        cmd = DRV_IOR(IpeTunnelDecapCtl_t, IpeTunnelDecapCtl_ipTtlLimit_f);
        DRV_IOCTL(lchip, 0, cmd, &ttl_limit);

        if ((next_hop_ptr == decap_nh) && (p_rx_info->ttl < ttl_limit))
        {
            /*cpu reason overwrite, need recover*/
            p_rx_info->reason = CTC_PKT_CPU_REASON_IP_TTL_CHECK_FAIL;
        }
    }

    /* SubQueId */
    dest_map = GetMsPacketHeader(V, destMap_f, p_raw_hdr);
    p_rx_info->dest_gport = dest_map & 0xFF;

    /* Source svlan */
    p_rx_info->src_svid = GetMsPacketHeader(V, srcSvlanId_f, p_raw_hdr);
    vlan_ptr = GetMsPacketHeader(V, srcVlanPtr_f, p_raw_hdr);
    if((p_rx_info->src_svid == 0) && (vlan_ptr < 4096))
    {
        p_rx_info->src_svid = vlan_ptr;
    }

    if (p_rx_info->src_svid)
    {
        CTC_SET_FLAG(p_rx_info->flags, CTC_PKT_FLAG_SRC_SVID_VALID);
    }

    /* Source cvlan */
    if (CTC_PKT_OPER_NORMAL == p_rx_info->oper_type)
    {
        p_rx_info->src_cvid = GetMsPacketHeader(V, u1_normal_srcCvlanId_f, p_raw_hdr);
        if (p_rx_info->src_cvid)
        {
            CTC_SET_FLAG(p_rx_info->flags, CTC_PKT_FLAG_SRC_CVID_VALID);
        }
        CTC_SET_FLAG(p_rx_info->flags, CTC_PKT_FLAG_ENCRYPTED);
    }

    if (GetMsPacketHeader(V, svlanTagOperationValid_f, p_raw_hdr))
    {
        switch (GetMsPacketHeader(V, stagAction_f, p_raw_hdr))
        {
            case SYS_PKT_CTAG_ACTION_MODIFY:
                p_rx_info->stag_op = CTC_VLAN_TAG_OP_REP;
                break;
            case SYS_PKT_CTAG_ACTION_ADD:
                p_rx_info->stag_op = CTC_VLAN_TAG_OP_ADD;
                break;
            case SYS_PKT_CTAG_ACTION_DELETE:
                p_rx_info->stag_op = CTC_VLAN_TAG_OP_DEL;
                break;
        }
    }

    if (CTC_PKT_OPER_NORMAL == p_rx_info->oper_type)
    {
        if (GetMsPacketHeader(V, u1_normal_cvlanTagOperationValid_f, p_raw_hdr))
        {
            switch (GetMsPacketHeader(V, u1_normal_cTagAction_f, p_raw_hdr))
            {
                case SYS_PKT_CTAG_ACTION_MODIFY:
                    p_rx_info->ctag_op = CTC_VLAN_TAG_OP_REP;
                    break;
                case SYS_PKT_CTAG_ACTION_ADD:
                    p_rx_info->ctag_op = CTC_VLAN_TAG_OP_ADD;
                    break;
                case SYS_PKT_CTAG_ACTION_DELETE:
                    p_rx_info->ctag_op = CTC_VLAN_TAG_OP_DEL;
                    break;
            }
        }
    }

    if (GetMsPacketHeader(V, u3_other_i2eSrcCidValid_f, p_raw_hdr)
        && (CTC_PKT_OPER_OAM != p_rx_info->oper_type) && (1 != p_rx_info->oper_type))
    {
        p_rx_info->cid = GetMsPacketHeader(V, u3_other_i2eSrcCid_f, p_raw_hdr);
    }

    /* Timestamp */
    ts_36_5 = GetMsPacketHeader(V, timestamp_36_5_f, p_raw_hdr);

    if (CTC_PKT_CPU_REASON_PTP == p_rx_info->reason)
    {
        ts_4_0 = GetMsPacketHeader(V, u1_ptp_timestamp_4_0_f, p_raw_hdr);
        ts_61_37 = GetMsPacketHeader(V, u1_ptp_timestamp_61_37_f, p_raw_hdr);

        ts_61_0 = (((uint64)ts_4_0) << 0) | (((uint64)ts_36_5) << 5) | (((uint64)ts_61_37) << 37);
    }
    else
    {
        CTC_ERROR_RETURN(_sys_usw_packet_recover_timestamp(lchip, ts_36_5, &ts_61_0));
    }

    sal_memset(&ptp_offset_adjust, 0, sizeof(ptp_offset_adjust));
    cmd = DRV_IOR(TsEngineOffsetAdj_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ptp_offset_adjust));

    GetTsEngineOffsetAdj(A, offsetNs_f, &ptp_offset_adjust, &offset_ns);
    GetTsEngineOffsetAdj(A, offsetSecond_f, &ptp_offset_adjust, &offset_s);

    ts_61_0 = ts_61_0 + offset_s * 1000000000ULL + offset_ns;
    _sys_usw_packet_bit62_to_ts(lchip, TRUE, ts_61_0, &p_rx_info->ptp.ts);


    /* OAM */
    if (CTC_PKT_OPER_OAM == p_rx_info->oper_type)
    {
        p_rx_info->oam.type = GetMsPacketHeader(V, u1_oam_oamType_f, p_raw_hdr);
        p_rx_info->oam.mep_index = GetMsPacketHeader(V, u1_oam_mepIndex_f, p_raw_hdr);

        if (GetMsPacketHeader(V, u1_oam_isUp_f, p_raw_hdr))
        {
            CTC_SET_FLAG(p_rx_info->oam.flags, CTC_PKT_OAM_FLAG_IS_UP_MEP);
        }

        if (GetMsPacketHeader(V, u1_oam_lmReceivedPacket_f, p_raw_hdr))
        {
            CTC_SET_FLAG(p_rx_info->oam.flags, CTC_PKT_OAM_FLAG_IS_LM);
        }

        if (GetMsPacketHeader(V, u1_oam_linkOam_f, p_raw_hdr))
        {
            CTC_SET_FLAG(p_rx_info->oam.flags, CTC_PKT_OAM_FLAG_LINK_OAM);
        }

        if (GetMsPacketHeader(V, u1_oam_dmEn_f, p_raw_hdr))
        {
            CTC_SET_FLAG(p_rx_info->oam.flags, CTC_PKT_OAM_FLAG_IS_DM);
        }

        if (GetMsPacketHeader(V, u1_oam_galExist_f, p_raw_hdr))
        {
            CTC_SET_FLAG(p_rx_info->oam.flags, CTC_PKT_OAM_FLAG_HAS_MPLS_GAL);
        }

        if (GetMsPacketHeader(V, u2_oam_mipEn_f, p_raw_hdr))
        {
            CTC_SET_FLAG(p_rx_info->oam.flags, CTC_PKT_OAM_FLAG_IS_MIP);
        }

        /* get timestamp offset in bytes for DM */
        if (CTC_FLAG_ISSET(p_rx_info->oam.flags, CTC_PKT_OAM_FLAG_IS_DM))
        {
            /*  DM Timestamp Bits Mapping
                Timestamp               Field Name          BIT Width   BIT Base
                dmTimestamp[37:0]       timestamp           38          0
                dmTimestamp[61:38]      u3_other_timestamp  24          38
             */
 #if 0
            GetMsPacketHeader(A, timestamp_f, p_raw_hdr, &ts_37_0_tmp);
            ts_37_0 = ts_37_0_tmp[1];
            ts_37_0 <<= 32;
            ts_37_0 |= ts_37_0_tmp[0];

            ts_61_38 = GetMsPacketHeader(V, u3_other_timestamp_f, p_raw_hdr);

            ts_61_0 = ((uint64)(ts_37_0) << 0)
                    | ((uint64)(ts_61_38) << 38);
            ns_only_format = FALSE;
            _sys_usw_packet_bit62_to_ts(lchip, ns_only_format, ts_61_0, &p_rx_info->oam.dm_ts);
#endif
        }

        if (CTC_FLAG_ISSET(p_rx_info->oam.flags, CTC_PKT_OAM_FLAG_IS_LM))
        {
            p_rx_info->oam.lm_fcl = GetMsPacketHeader(V, u3_oam_rxtxFcl_f, p_raw_hdr);
            p_rx_info->oam.lm_fcl = (p_rx_info->oam.lm_fcl << 8) + GetMsPacketHeader(V, u1_oam_rxtxFcl_f, p_raw_hdr);
        }

        /* get svid from vlan_ptr for Up MEP/Up MIP */
        if (CTC_FLAG_ISSET(p_rx_info->oam.flags, CTC_PKT_OAM_FLAG_IS_UP_MEP))
        {
            CTC_SET_FLAG(p_rx_info->flags, CTC_PKT_FLAG_SRC_SVID_VALID);
            p_rx_info->src_svid = GetMsPacketHeader(V, srcVlanPtr_f, p_raw_hdr);
        }
    }
    else if (CTC_PKT_OPER_C2C == p_rx_info->oper_type)
    {
        uint8  sub_queue_id = 0;
        /* Stacking*/
        p_rx_info->reason       = CTC_PKT_CPU_REASON_C2C_PKT;
        p_rx_info->stacking_src_port     = GetMsPacketHeader(V, u1_c2c_stackingSrcPort_f, p_raw_hdr);
        p_rx_info->stacking_src_port     = SYS_MAP_DRV_GPORT_TO_CTC_GPORT(p_rx_info->stacking_src_port);

        if (CTC_FLAG_ISSET(p_rx_info->flags, CTC_PKT_FLAG_MCAST))
        {
            CTC_UNSET_FLAG(p_rx_info->flags, CTC_PKT_FLAG_MCAST);
            p_rx_info->dest_group_id = 0;
        }
        p_rx_info->vrfid = 0;

        if (GetMsPacketHeader(V, u1_c2c_neighborDiscoverPacket_f, p_raw_hdr))
        {
            p_rx_info->dest_gport = (dest_map & 0xF) * MCHIP_CAP(SYS_CAP_REASON_GRP_QUEUE_NUM) + (p_rx_info->priority/2);
        }
        else
        {
           p_rx_info->dest_gport = (dest_map & 0xF) * MCHIP_CAP(SYS_CAP_REASON_GRP_QUEUE_NUM) + (p_rx_info->priority%8);
        }

        CTC_ERROR_RETURN(sys_usw_get_sub_queue_id_by_cpu_reason(lchip, CTC_PKT_CPU_REASON_C2C_PKT, &sub_queue_id));
        if(DRV_IS_DUET2(lchip))
        {
            p_rx_info->priority = (p_rx_info->dest_gport > sub_queue_id) ? (p_rx_info->dest_gport - sub_queue_id) : 0;
        }
    }
    else if(2 == p_rx_info->oper_type)  /* OPERATIONTYPE_E2ILOOP */
    {
        if(GetMsPacketHeader(V, u1_e2iLoopOut_needDot1AeEncrypt_f, p_raw_hdr)
            || GetMsPacketHeader(V, u1_e2iLoopIn_needCapwapDecap_f, p_raw_hdr))
        {
             CTC_UNSET_FLAG(p_rx_info->flags, CTC_PKT_FLAG_ENCRYPTED);
        }
    }

    if (p_rx_info->reason == CTC_PKT_CPU_REASON_MONITOR_BUFFER_LOG)
    {
        if (GetMsPacketHeader(V, srcStagCfi_f, p_raw_hdr))
        {
            CTC_SET_FLAG(p_rx_info->flags, CTC_PKT_FLAG_BUFFER_VICTIM_PKT);
        }
    }

    if (GetMsPacketHeader(V, fromFabric_f, p_raw_hdr))
    {
        uint8 legacy_en = 0;
		uint16 cloud_hdr_len = 0;
		uint16 extHeaderLen = 0;
		uint16 stk_hdr_len = 0;
        uint32 stk_basic[4] = {0};
		uint32 gport = 0;
		uint8  packet_rx_header_en = 0;
		uint16 local_phy_port =0;
        uint8 gchip_id = 0;
		CFHeaderBasic_m* p_stk_header = NULL;
        bool inner_pkt_hdr_en = FALSE;
        uint8 oper_type = 0;

        local_phy_port = GetMsPacketHeader(V, localPhyPort_f, p_raw_hdr);
		sys_usw_get_gchip_id(lchip, &gchip_id);
		gport = CTC_MAP_LPORT_TO_GPORT(gchip_id, local_phy_port);

        if (REGISTER_API(lchip)->stacking_get_stkhdr_len)
        {
            CTC_ERROR_RETURN(REGISTER_API(lchip)->stacking_get_stkhdr_len(lchip, gport, &packet_rx_header_en, &legacy_en, &cloud_hdr_len));
        }

        if (packet_rx_header_en)
        {
            if (!legacy_en)
            {
                /*Using CFlex header, get from stacking module only means Cloud Header Length*/
                p_stk_header = (CFHeaderBasic_m*)(p_pkt_rx->pkt_buf[0].data + SYS_USW_PKT_HEADER_LEN + p_pkt_rx->eth_hdr_len + cloud_hdr_len);
                CTC_ERROR_RETURN(sys_usw_dword_reverse_copy((uint32*)stk_basic, (uint32*)p_stk_header, 4));
                CTC_ERROR_RETURN(sys_usw_swap32((uint32*)stk_basic, 4, FALSE));
                extHeaderLen = GetCFHeaderBasic(V, extHeaderLen_f, (uint32*)stk_basic);
                stk_hdr_len = (16 + (extHeaderLen << 3));
            }
            else
            {
                stk_hdr_len = SYS_PACKET_BASIC_STACKING_HEADER_LEN;
            }

            p_pkt_rx->stk_hdr_len = cloud_hdr_len + stk_hdr_len;
        }
        CTC_ERROR_RETURN(sys_usw_global_ctl_get(lchip, CTC_GLOBAL_STK_WITH_IGS_PKT_HDR_EN, &inner_pkt_hdr_en))
        if (inner_pkt_hdr_en)
        {
            p_raw_hdr_net = (uint32*)(p_pkt_rx->pkt_buf[0].data + SYS_USW_PKT_HEADER_LEN + p_pkt_rx->eth_hdr_len + p_pkt_rx->stk_hdr_len);
            CTC_ERROR_RETURN(sys_usw_dword_reverse_copy((uint32*)bheader, (uint32*)p_raw_hdr_net, SYS_USW_PKT_HEADER_LEN/4));
            CTC_ERROR_RETURN(sys_usw_swap32((uint32*)p_raw_hdr, SYS_USW_PKT_HEADER_LEN / 4, FALSE));
            oper_type  = GetMsPacketHeader(V, operationType_f, p_raw_hdr);
            if ((CTC_PKT_OPER_NORMAL == oper_type) && (GetMsPacketHeader(V, u1_normal_metadataType_f, p_raw_hdr) == 0))
            {
                /*vrfid*/
                p_rx_info->vrfid= GetMsPacketHeader(V, u1_normal_metadata_f, p_raw_hdr);
                CTC_SET_FLAG(p_rx_info->flags, CTC_PKT_FLAG_VRFID_VALID);
            }
            p_pkt_rx->stk_hdr_len += SYS_USW_PKT_HEADER_LEN;
        }
    }
    else
    {
        uint8 legacy_en = 0;
		uint16 cloud_hdr_len = 0;
		uint16 extHeaderLen = 0;
		uint16 stk_hdr_len = 0;
		uint8  packet_rx_header_en = 0;
        uint32 stk_basic[4] = {0};
		CFHeaderBasic_m* p_stk_header = NULL;
        if (REGISTER_API(lchip)->stacking_get_stkhdr_len)
        {
            CTC_ERROR_RETURN(REGISTER_API(lchip)->stacking_get_stkhdr_len(lchip, p_rx_info->src_port, &packet_rx_header_en, &legacy_en, &cloud_hdr_len));
        }

        if (packet_rx_header_en)
        {
            if (!legacy_en)
            {
                /*Using CFlex header, get from stacking module only means Cloud Header Length*/
                p_stk_header = (CFHeaderBasic_m*)(p_pkt_rx->pkt_buf[0].data + SYS_USW_PKT_HEADER_LEN + p_pkt_rx->eth_hdr_len + cloud_hdr_len);
                CTC_ERROR_RETURN(sys_usw_dword_reverse_copy((uint32*)stk_basic, (uint32*)p_stk_header, 4));
                CTC_ERROR_RETURN(sys_usw_swap32((uint32*)stk_basic, 4, FALSE));
                extHeaderLen = GetCFHeaderBasic(V, extHeaderLen_f, (uint32*)stk_basic);
                stk_hdr_len = (16 + (extHeaderLen << 3));
            }
            else
            {
                stk_hdr_len = SYS_PACKET_BASIC_STACKING_HEADER_LEN;
            }
            p_pkt_rx->stk_hdr_len = cloud_hdr_len + stk_hdr_len;
        }
    }

    return CTC_E_NONE;
}

static int32
_sys_usw_packet_tx_param_check(uint8 lchip, ctc_pkt_tx_t* p_pkt_tx)
{
    uint32 p_nhid;
    sys_nh_info_dsnh_t nh_info;
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_pkt_tx);
    CTC_PTR_VALID_CHECK(p_pkt_tx->skb.data);
    SYS_DMA_PKT_LEN_CHECK(p_pkt_tx->skb.len);
    SYS_GLOBAL_PORT_CHECK(p_pkt_tx->tx_info.src_port);
    SYS_GLOBAL_PORT_CHECK(p_pkt_tx->tx_info.dest_gport);
    if (CTC_FLAG_ISSET(p_pkt_tx->tx_info.flags, CTC_PKT_FLAG_SRC_SVID_VALID))
    {
        CTC_VLAN_RANGE_CHECK(p_pkt_tx->tx_info.src_svid);
    }
    if(CTC_FLAG_ISSET(p_pkt_tx->tx_info.flags, CTC_PKT_FLAG_SRC_CVID_VALID))
    {
        CTC_VLAN_RANGE_CHECK(p_pkt_tx->tx_info.src_cvid);
    }
    if (CTC_FLAG_ISSET(p_pkt_tx->tx_info.flags, CTC_PKT_FLAG_ASSIGN_DEST_PORT))
    {
        uint8 gchip = 0;
        uint8 chan_id = 0;

        sys_usw_get_gchip_id(lchip, &gchip);
        chan_id = SYS_GET_CHANNEL_ID(lchip, CTC_MAP_LPORT_TO_GPORT(gchip, p_pkt_tx->tx_info.lport));
        if (0xff == chan_id)
        {
            return CTC_E_INVALID_PARAM;
        }
    }
    CTC_MAX_VALUE_CHECK(p_pkt_tx->tx_info.priority,15);
    CTC_MAX_VALUE_CHECK(p_pkt_tx->tx_info.color, CTC_QOS_COLOR_GREEN);
    CTC_COS_RANGE_CHECK(p_pkt_tx->tx_info.src_cos);
    //CTC_MIN_VALUE_CHECK(p_pkt_tx->tx_info.ttl, 1);
    CTC_MAX_VALUE_CHECK(p_pkt_tx->tx_info.vlan_domain,CTC_PORT_VLAN_DOMAIN_CVLAN);
    CTC_FID_RANGE_CHECK(p_pkt_tx->tx_info.fid);
    if(p_pkt_tx->tx_info.oper_type == CTC_PKT_OPER_OAM)
    {
        if (CTC_FLAG_ISSET(p_pkt_tx->tx_info.oam.flags, CTC_PKT_OAM_FLAG_IS_UP_MEP))
        {
            if (CTC_OAM_TYPE_ETH != p_pkt_tx->tx_info.oam.type)
            {
                /* only Eth OAM support UP MEP */
                return CTC_E_INVALID_PARAM;
            }
            CTC_VLAN_RANGE_CHECK(p_pkt_tx->tx_info.oam.vid);
        }

        CTC_MAX_VALUE_CHECK(p_pkt_tx->tx_info.oam.type ,CTC_OAM_TYPE_TRILL_BFD_ECHO);

        if ((CTC_FLAG_ISSET(p_pkt_tx->tx_info.oam.flags, CTC_PKT_OAM_FLAG_IS_DM)) &&
         ((1 ==(p_pkt_tx->tx_info.oam.dm_ts_offset % 2))
         || (p_pkt_tx->tx_info.oam.dm_ts_offset > p_pkt_tx->skb.len - 8)
         || (p_pkt_tx->tx_info.oam.dm_ts_offset > MCHIP_CAP(SYS_CAP_NPM_MAX_TS_OFFSET))))
        {
            return CTC_E_INVALID_PARAM;
        }

    }
    if(p_pkt_tx->tx_info.oper_type == CTC_PKT_OPER_PTP)
    {
        CTC_MAX_VALUE_CHECK(p_pkt_tx->tx_info.ptp.oper,CTC_PTP_CORRECTION);
    }
    if (CTC_FLAG_ISSET(p_pkt_tx->tx_info.flags, CTC_PKT_FLAG_MCAST))
    {
        if(p_pkt_tx->tx_info.oper_type == CTC_PKT_OPER_C2C)
        {
            if(CTC_E_IN_USE != sys_usw_nh_check_glb_met_offset(lchip, p_pkt_tx->tx_info.dest_group_id, 1, TRUE))
            {
                SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Mcast group not created\n");
                return CTC_E_INVALID_PARAM;
            }
        }
        else
        {
            if(CTC_E_NOT_EXIST == sys_usw_nh_get_mcast_nh(lchip, p_pkt_tx->tx_info.dest_group_id, &p_nhid))
            {
                return CTC_E_INVALID_PARAM;
            }
        }
    }

    if(CTC_FLAG_ISSET(p_pkt_tx->tx_info.flags, CTC_PKT_FLAG_NHID_VALID))
    {
        if(0 > sys_usw_nh_get_nhinfo(lchip, p_pkt_tx->tx_info.nhid, &nh_info, 0))
        {
            return CTC_E_INVALID_PARAM;
        }
    }

    if (p_pkt_tx->mode > CTC_PKT_MODE_ETH)
    {
        return CTC_E_INVALID_PARAM;
    }

    if(CTC_FLAG_ISSET(p_pkt_tx->tx_info.flags, CTC_PKT_FLAG_SESSION_ID_EN))
    {
        if ((p_pkt_tx->mode == CTC_PKT_MODE_ETH) || (p_pkt_tx->tx_info.session_id >= SYS_PKT_MAX_TX_SESSION))
        {
            return CTC_E_INVALID_PARAM;
        }
    }
    return CTC_E_NONE;
}

static int32
_sys_usw_packet_register_tx_cb(uint8 lchip, void* tx_cb)
{
    LCHIP_CHECK(lchip);
    if (NULL == p_usw_pkt_master[lchip])
    {
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
			return CTC_E_NOT_INIT;

    }

     p_usw_pkt_master[lchip]->cfg.dma_tx_cb = (CTC_PKT_TX_CALLBACK)tx_cb;

    return CTC_E_NONE;
}

static int32
_sys_usw_packet_decap(uint8 lchip, ctc_pkt_rx_t* p_pkt_rx)
{
    ds_t raw_hdr;
    ds_t* p_raw_hdr = &raw_hdr;
    uint16 eth_hdr_len = 0;
    uint16 pkt_hdr_len = 0;
    uint16 stk_hdr_len = 0;

    CTC_PTR_VALID_CHECK(p_pkt_rx);
    LCHIP_CHECK(lchip);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "len %d, buf_count %d \n", p_pkt_rx->pkt_len,
                     p_pkt_rx->buf_count);

    /* 1. check packet length */
    /* ethernet has 20 Bytes L2 header */
    if (CTC_PKT_MODE_ETH == p_pkt_rx->mode)
    {
        eth_hdr_len = SYS_USW_PKT_CPUMAC_LEN;
    }
    else
    {
        eth_hdr_len = 0;
    }

    pkt_hdr_len = SYS_USW_PKT_HEADER_LEN;

    if (p_pkt_rx->pkt_len < (eth_hdr_len + pkt_hdr_len))
    {
        return DRV_E_WRONG_SIZE;
    }

    if (p_pkt_rx->pkt_buf[0].len < (eth_hdr_len + pkt_hdr_len))
    {
        return DRV_E_WRONG_SIZE;
    }

    p_pkt_rx->eth_hdr_len = eth_hdr_len;
    p_pkt_rx->pkt_hdr_len = pkt_hdr_len;
    p_pkt_rx->stk_hdr_len = stk_hdr_len;
    /* 2. decode raw header */
    sal_memcpy(p_raw_hdr, p_pkt_rx->pkt_buf[0].data + eth_hdr_len, SYS_USW_PKT_HEADER_LEN);

    /* 3. convert raw header to rx_info */
    CTC_ERROR_RETURN(_sys_usw_packet_rawhdr_to_rxinfo(lchip, (uint32*)p_raw_hdr, p_pkt_rx));

    /* add stats */
    p_usw_pkt_master[lchip]->stats.decap++;

    return CTC_E_NONE;
}

static int32
_sys_usw_packet_encap(uint8 lchip, ctc_pkt_tx_t* p_pkt_tx)
{
    ds_t* p_raw_hdr = NULL;
    sys_usw_cpumac_header_t* p_cpumac_hdr = NULL;
    uint32 cmd = 0;
    IpeHeaderAdjustCtl_m ipe_header_adjust_ctl;
    uint16 tpid = 0;
    uint8 is_skb_buf = 0;

    CTC_PTR_VALID_CHECK(p_pkt_tx);
    LCHIP_CHECK(lchip);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    is_skb_buf = (((p_pkt_tx->skb.head == NULL) ||
                   (p_pkt_tx->skb.tail == NULL) ||
                   (p_pkt_tx->skb.end == NULL)) &&
                   (p_pkt_tx->skb.data != NULL) ? 0:1);

    /* 1. encode packet header */
    if(!is_skb_buf)
    {
        if(CTC_PKT_MODE_ETH == p_pkt_tx->mode)
        {
            sal_memcpy(p_pkt_tx->skb.buf + SYS_USW_PKT_CPUMAC_LEN + SYS_USW_PKT_HEADER_LEN, p_pkt_tx->skb.data, p_pkt_tx->skb.len);
            p_pkt_tx->skb.data = p_pkt_tx->skb.buf + SYS_USW_PKT_CPUMAC_LEN + SYS_USW_PKT_HEADER_LEN;
            p_raw_hdr = (ds_t*)ctc_packet_skb_push(&p_pkt_tx->skb, SYS_USW_PKT_HEADER_LEN);
        }
        else
        {
            p_raw_hdr = (ds_t*)(p_pkt_tx->skb.buf + CTC_PKT_HDR_ROOM - SYS_USW_PKT_HEADER_LEN);

            /*as a flag that bridge header is in skb.buf to DMA*/
            p_pkt_tx->skb.tail = p_pkt_tx->skb.data;
        }
    }
    else
    {
        p_raw_hdr = (ds_t*)ctc_packet_skb_push(&p_pkt_tx->skb, SYS_USW_PKT_HEADER_LEN);
        if(CTC_PKT_MODE_DMA == p_pkt_tx->mode)
        {
            /*all skb.len exclude header len*/
            p_pkt_tx->skb.len -= SYS_USW_PKT_HEADER_LEN;
        }
    }


    if (p_pkt_tx->callback)
    {
        return CTC_E_NONE;
    }

    CTC_ERROR_RETURN(_sys_usw_packet_txinfo_to_rawhdr(lchip, &p_pkt_tx->tx_info, (uint32*)p_raw_hdr, is_skb_buf, p_pkt_tx->mode, p_pkt_tx->skb.data));

    if (CTC_PKT_MODE_ETH == p_pkt_tx->mode)
    {
        cmd = DRV_IOR(IpeHeaderAdjustCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_header_adjust_ctl));

        tpid = GetIpeHeaderAdjustCtl(V, headerTpid_f, &ipe_header_adjust_ctl);

        /* 2. encode CPUMAC Header */
        p_cpumac_hdr = (sys_usw_cpumac_header_t*)ctc_packet_skb_push(&p_pkt_tx->skb, SYS_USW_PKT_CPUMAC_LEN);
        p_cpumac_hdr->macda[0] = p_usw_pkt_master[lchip]->cpu_mac_sa[0];
        p_cpumac_hdr->macda[1] = p_usw_pkt_master[lchip]->cpu_mac_sa[1];
        p_cpumac_hdr->macda[2] = p_usw_pkt_master[lchip]->cpu_mac_sa[2];
        p_cpumac_hdr->macda[3] = p_usw_pkt_master[lchip]->cpu_mac_sa[3];
        p_cpumac_hdr->macda[4] = p_usw_pkt_master[lchip]->cpu_mac_sa[4];
        p_cpumac_hdr->macda[5] = p_usw_pkt_master[lchip]->cpu_mac_sa[5];
        p_cpumac_hdr->macsa[0] = p_usw_pkt_master[lchip]->cpu_mac_da[0];
        p_cpumac_hdr->macsa[1] = p_usw_pkt_master[lchip]->cpu_mac_da[1];
        p_cpumac_hdr->macsa[2] = p_usw_pkt_master[lchip]->cpu_mac_da[2];
        p_cpumac_hdr->macsa[3] = p_usw_pkt_master[lchip]->cpu_mac_da[3];
        p_cpumac_hdr->macsa[4] = p_usw_pkt_master[lchip]->cpu_mac_da[4];
        p_cpumac_hdr->macsa[5] = p_usw_pkt_master[lchip]->cpu_mac_da[5];
        p_cpumac_hdr->vlan_tpid = sal_htons(tpid);
        p_cpumac_hdr->vlan_vid = sal_htons(0x23);
        p_cpumac_hdr->type = sal_htons(0x5A5A);
    }

    /* add stats */
    p_usw_pkt_master[lchip]->stats.encap++;

    return CTC_E_NONE;

}

static int32
_sys_usw_packet_tx(uint8 lchip, ctc_pkt_tx_t* p_pkt_tx)
{
    int32 ret =  CTC_E_NONE;
    uint8 is_skb_buf = 0;

    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    ret = _sys_usw_packet_tx_param_check(lchip, p_pkt_tx);
    if(ret < 0)
    {
        return ret;
    }

    is_skb_buf = (((p_pkt_tx->skb.head == NULL) ||
                   (p_pkt_tx->skb.tail == NULL) ||
                   (p_pkt_tx->skb.end == NULL)) &&
                   (p_pkt_tx->skb.data != NULL) ? 0:1);


    p_pkt_tx->lchip = lchip;

    CTC_ERROR_RETURN(_sys_usw_packet_encap(lchip, p_pkt_tx));

     _sys_usw_packet_tx_dump(lchip, p_pkt_tx);

    if (REGISTER_API(lchip)->inband_tx)
    {
        CTC_ERROR_RETURN(REGISTER_API(lchip)->inband_tx(lchip, p_pkt_tx));
        return CTC_E_NONE;
    }


    /*tx buf dump data store*/
    if ((p_usw_pkt_master[lchip]->cursor[CTC_EGRESS] < SYS_PKT_BUF_MAX) && (p_usw_pkt_master[lchip]->pkt_buf_en[CTC_EGRESS]== 1))
    {
        sys_pkt_tx_buf_t* p_tx_buf = NULL;
        uint8   buffer_pos = 0;
        uint8   is_pkt_found = 0;
        uint32 total_pkt_len = (p_pkt_tx->mode == CTC_PKT_MODE_DMA )? (p_pkt_tx->skb.len +SYS_USW_PKT_HEADER_LEN) : p_pkt_tx->skb.len;
        uint32 pkt_data_len = (p_pkt_tx->mode == CTC_PKT_MODE_DMA )? (total_pkt_len - SYS_USW_PKT_HEADER_LEN) :(total_pkt_len - SYS_USW_PKT_HEADER_LEN -SYS_USW_PKT_CPUMAC_LEN);
        uint8  header_len = 0;

        p_tx_buf = (sys_pkt_tx_buf_t*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(sys_pkt_tx_buf_t));
        if (p_tx_buf == NULL)
        {
            return CTC_E_NO_MEMORY;
        }
        sal_memset(p_tx_buf, 0, sizeof(sys_pkt_tx_buf_t));
        p_tx_buf->mode  = p_pkt_tx->mode;
        p_tx_buf->pkt_len = total_pkt_len;


        sal_time(&p_tx_buf->tm);
        sal_memcpy(&p_tx_buf->tx_info,&(p_pkt_tx->tx_info),sizeof(ctc_pkt_info_t));

        total_pkt_len = (total_pkt_len >SYS_PKT_BUF_PKT_LEN) ?SYS_PKT_BUF_PKT_LEN:total_pkt_len;

        if (p_pkt_tx->mode == CTC_PKT_MODE_DMA )
        {
            if(is_skb_buf)
            {
               sal_memcpy(&p_tx_buf->pkt_data[0],p_pkt_tx->skb.data, total_pkt_len);
            }
            else
            {
               sal_memcpy(&p_tx_buf->pkt_data[0],&p_pkt_tx->skb.buf[CTC_PKT_HDR_ROOM - SYS_USW_PKT_HEADER_LEN], SYS_USW_PKT_HEADER_LEN);
               /*copy userdata*/
               sal_memcpy(&p_tx_buf->pkt_data[SYS_USW_PKT_HEADER_LEN],p_pkt_tx->skb.data, (total_pkt_len-SYS_USW_PKT_HEADER_LEN));
            }
        }
        else
        {

            sal_memcpy(&p_tx_buf->pkt_data[0],&p_pkt_tx->skb.data[0], total_pkt_len);

        }

        header_len = (p_pkt_tx->mode == CTC_PKT_MODE_DMA )? SYS_USW_PKT_HEADER_LEN:(SYS_USW_PKT_HEADER_LEN + SYS_USW_PKT_CPUMAC_LEN);
        p_tx_buf->hash_seed = ctc_hash_caculate(pkt_data_len, &p_tx_buf->pkt_data[header_len]);

        for (buffer_pos = 0; buffer_pos < SYS_PKT_BUF_MAX; buffer_pos++)
        {
            if ((p_tx_buf->hash_seed == p_usw_pkt_master[lchip]->tx_buf[buffer_pos].hash_seed)
                && (p_tx_buf->pkt_len ==  p_usw_pkt_master[lchip]->tx_buf[buffer_pos].pkt_len))
            {
                is_pkt_found = 1;
                break;
            }
        }

        /*update buf data*/
        if (!is_pkt_found )
        {
            buffer_pos = p_usw_pkt_master[lchip]->cursor[CTC_EGRESS];
            if(buffer_pos < SYS_PKT_BUF_MAX)
            {
                p_usw_pkt_master[lchip]->cursor[CTC_EGRESS]++;
            }
        }
        sal_memcpy(&p_usw_pkt_master[lchip]->tx_buf[buffer_pos] ,p_tx_buf , sizeof(sys_pkt_tx_buf_t));

        mem_free(p_tx_buf);

    }

    if (CTC_PKT_MODE_ETH == p_pkt_tx->mode)
    {
        if (p_usw_pkt_master[lchip]->cfg.socket_tx_cb )
        {
            CTC_ERROR_RETURN(p_usw_pkt_master[lchip]->cfg.socket_tx_cb(p_pkt_tx));
        }
        else
        {
            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Do not support Socket Tx!\n");
            return CTC_E_NOT_SUPPORT;
        }
    }
    else if (CTC_PKT_MODE_DMA == p_pkt_tx->mode)
    {
        if (p_usw_pkt_master[lchip]->cfg.dma_tx_cb )
        {
            CTC_ERROR_RETURN(p_usw_pkt_master[lchip]->cfg.dma_tx_cb(p_pkt_tx));
        }
        else
        {
            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Do not support Dma Tx!\n");
            return CTC_E_NOT_SUPPORT;
        }
    }

    if (CTC_FLAG_ISSET(p_pkt_tx->tx_info.flags, CTC_PKT_FLAG_SESSION_ID_EN))
    {
        CTC_BMP_SET(p_usw_pkt_master[lchip]->session_bm, p_pkt_tx->tx_info.session_id);
    }

    if (CTC_FLAG_ISSET(p_pkt_tx->tx_info.flags, CTC_PKT_FLAG_MCAST))
    {
         p_usw_pkt_master[lchip]->stats.mc_tx++;
    }
    else
    {
         p_usw_pkt_master[lchip]->stats.uc_tx++;
    }

    return CTC_E_NONE;
}

static int32
_sys_usw_packet_rx(uint8 lchip, ctc_pkt_rx_t* p_pkt_rx)
{
    int32 ret = 0;
    uint8 lchip_master = 0;
    LCHIP_CHECK(lchip);

    lchip_master = sys_usw_chip_get_rchain_en()?0:lchip;
    ret = (_sys_usw_packet_decap(lchip, p_pkt_rx));
    if (ret < 0)
    {
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "packet decap Error!ret:%d\n", ret);
        /* return ret;*/
    }

    if (p_pkt_rx->rx_info.reason < CTC_PKT_CPU_REASON_MAX_COUNT)
    {
        p_usw_pkt_master[lchip_master]->stats.rx[p_pkt_rx->rx_info.reason]++;
    }

    _sys_usw_packet_rx_dump(lchip_master, p_pkt_rx);

    if (REGISTER_API(lchip_master)->inband_rx)
    {
        ret = (REGISTER_API(lchip_master)->inband_rx(lchip_master, p_pkt_rx));
        if (!ret)
        {
            return CTC_E_NONE;
        }
    }

    if ((p_pkt_rx->rx_info.reason - CTC_PKT_CPU_REASON_OAM) == CTC_OAM_EXCP_LEARNING_BFD_TO_CPU
        || (CTC_OAM_EXCP_BFD_TIMER_NEGOTIATION == (p_pkt_rx->rx_info.reason - CTC_PKT_CPU_REASON_OAM)))
    {
        if (p_usw_pkt_master[lchip_master]->oam_rx_cb)
        {
            p_usw_pkt_master[lchip_master]->oam_rx_cb(lchip_master, p_pkt_rx);
        }
    }

    if ((p_usw_pkt_master[lchip_master]->cursor[CTC_INGRESS]  < SYS_PKT_BUF_MAX)
        && (p_usw_pkt_master[lchip_master]->pkt_buf_en[CTC_INGRESS]== 1)
        && ((!p_usw_pkt_master[lchip_master]->rx_buf_reason_id) || (p_usw_pkt_master[lchip_master]->rx_buf_reason_id == p_pkt_rx->rx_info.reason)))
    {
        sys_pkt_buf_t* p_pkt_rx_buf = NULL;
        uint8   buffer_pos = 0;
        uint8   is_pkt_found = 0;

        p_pkt_rx_buf = (sys_pkt_buf_t*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(sys_pkt_buf_t));
        if (p_pkt_rx_buf)
        {
            p_pkt_rx_buf->mode = p_pkt_rx->mode;
            p_pkt_rx_buf->pkt_len = p_pkt_rx->pkt_len;

            sal_time(&p_pkt_rx_buf->tm);

            if (p_pkt_rx->pkt_buf->len <= SYS_PKT_BUF_PKT_LEN)
            {
                sal_memcpy(p_pkt_rx_buf->pkt_data, p_pkt_rx->pkt_buf->data, p_pkt_rx->pkt_buf->len);
            }
            else
            {
                sal_memcpy(p_pkt_rx_buf->pkt_data, p_pkt_rx->pkt_buf->data, SYS_PKT_BUF_PKT_LEN);
            }

            p_pkt_rx_buf->hash_seed = ctc_hash_caculate(p_pkt_rx_buf->pkt_len, &p_pkt_rx_buf->pkt_data[0]);
            p_pkt_rx_buf->seconds = p_pkt_rx->rx_info.ptp.ts.seconds;
            p_pkt_rx_buf->nanoseconds = p_pkt_rx->rx_info.ptp.ts.nanoseconds;

            for (buffer_pos = 0; buffer_pos < SYS_PKT_BUF_MAX; buffer_pos++)
            {
                if ((p_pkt_rx_buf->hash_seed == p_usw_pkt_master[lchip_master]->rx_buf[buffer_pos].hash_seed)
                    && (p_pkt_rx_buf->pkt_len ==  p_usw_pkt_master[lchip_master]->rx_buf[buffer_pos].pkt_len))
                {
                    is_pkt_found = 1;
                    break;
                }
            }

            /*update buf data*/
            if (!is_pkt_found )
            {
                buffer_pos = p_usw_pkt_master[lchip_master]->cursor[CTC_INGRESS];
                if(buffer_pos < SYS_PKT_BUF_MAX)
                {
                    p_usw_pkt_master[lchip_master]->cursor[CTC_INGRESS]++;
                }
            }
            sal_memcpy(&p_usw_pkt_master[lchip_master]->rx_buf[buffer_pos] ,p_pkt_rx_buf , sizeof(sys_pkt_buf_t));

            mem_free(p_pkt_rx_buf);
        }
    }

    if (p_pkt_rx->rx_info.reason == CTC_PKT_CPU_REASON_DIAG_DISCARD_PKT
        && p_usw_pkt_master[lchip_master]->drop_rx_cb)
    {
        p_usw_pkt_master[lchip_master]->drop_rx_cb(p_pkt_rx);
    }
    else if (p_usw_pkt_master[lchip_master]->cfg.rx_cb)
    {
        p_usw_pkt_master[lchip_master]->cfg.rx_cb(p_pkt_rx);
    }



    return CTC_E_NONE;
}

static int32
_sys_usw_packet_tx_common_dump(uint8 lchip,uint8 buf_id ,uint8 flag, uint8* p_index, uint8 header_len)
{
    char* p_time_str = NULL;
    uint32 len  = 0;
    char str_time[64] = {0};

    if (p_usw_pkt_master[lchip]->tx_buf[buf_id].pkt_len == 0)
    {
        return CTC_E_NONE;
    }
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Packet No.%d, Pkt len: %d\n", (*p_index)++, p_usw_pkt_master[lchip]->tx_buf[buf_id].pkt_len);

    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------\n");
    p_time_str = sal_ctime(&(p_usw_pkt_master[lchip]->tx_buf[buf_id].tm));
    sal_strncpy(str_time, p_time_str, (sal_strlen(p_time_str)-1));
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Timestamp:%s\n", str_time);

    if (CTC_IS_BIT_SET(flag, 1))
    {
        ctc_pkt_tx_t* p_pkt_tx = NULL;
        ctc_pkt_info_t* p_pkt_info = NULL;

        p_pkt_tx = (ctc_pkt_tx_t*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(ctc_pkt_tx_t));
        if (NULL == p_pkt_tx)
        {
            return CTC_E_NO_MEMORY;
        }
        sal_memset(p_pkt_tx, 0, sizeof(ctc_pkt_tx_t));
        p_pkt_info = &(p_usw_pkt_master[lchip]->tx_buf[buf_id].tx_info);
        sal_memcpy(&(p_pkt_tx->tx_info),p_pkt_info ,sizeof(ctc_pkt_info_t));
        _sys_usw_packet_dump_tx_header(lchip, p_pkt_tx,1,0);
        mem_free(p_pkt_tx);

        if (p_usw_pkt_master[lchip]->tx_buf[buf_id].mode == CTC_PKT_MODE_ETH)
        {
            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------\n");
            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Packet CPU Header\n");
            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------\n");

            _sys_usw_packet_dump(lchip, p_usw_pkt_master[lchip]->tx_buf[buf_id].pkt_data, SYS_USW_PKT_CPUMAC_LEN);

            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------\n");
            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Packet Bridge Header\n");
            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------\n");

            _sys_usw_packet_dump(lchip, p_usw_pkt_master[lchip]->tx_buf[buf_id].pkt_data+SYS_USW_PKT_CPUMAC_LEN, SYS_USW_PKT_HEADER_LEN);
        }
        else
        {
            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------\n");
            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Packet Bridge Header\n");
            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------\n");

            _sys_usw_packet_dump(lchip, p_usw_pkt_master[lchip]->tx_buf[buf_id].pkt_data, SYS_USW_PKT_HEADER_LEN);
        }

    }

    if (CTC_IS_BIT_SET(flag, 0))
    {
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------\n");
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Packet Info(Length : %d):\n", (p_usw_pkt_master[lchip]->tx_buf[buf_id].pkt_len-header_len));
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------\n");

        /*print packet*/
        len = (p_usw_pkt_master[lchip]->tx_buf[buf_id].pkt_len <= SYS_PKT_BUF_PKT_LEN)?
        (p_usw_pkt_master[lchip]->tx_buf[buf_id].pkt_len-header_len): (SYS_PKT_BUF_PKT_LEN-header_len);
        _sys_usw_packet_dump(lchip, p_usw_pkt_master[lchip]->tx_buf[buf_id].pkt_data+header_len, len);
    }


    return CTC_E_NONE;
}


static int32
_sys_usw_packet_rx_common_dump(uint8 lchip,uint8 buf_id ,uint8 flag, uint8* p_index, uint8 header_len)
{
    char* p_time_str = NULL;
    uint32 len  = 0;
    char str_time[64] = {0};

    if (p_usw_pkt_master[lchip]->rx_buf[buf_id].pkt_len == 0)
    {
        return CTC_E_NONE;
    }
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Packet No.%d, Pkt len: %d\n", (*p_index)++, p_usw_pkt_master[lchip]->rx_buf[buf_id].pkt_len);

    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------\n");
    p_time_str = sal_ctime(&(p_usw_pkt_master[lchip]->rx_buf[buf_id].tm));
    sal_strncpy(str_time, p_time_str, (sal_strlen(p_time_str)-1));
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Timestamp:%s\n", str_time);

    if (CTC_IS_BIT_SET(flag, 1))
    {
        ctc_pkt_rx_t pkt_rx;
        ds_t raw_hdr;
        ctc_pkt_buf_t pkt_data_buf;

        sal_memset(&pkt_rx, 0, sizeof(pkt_rx));
        sal_memset(raw_hdr, 0, sizeof(raw_hdr));
        sal_memset(&pkt_data_buf, 0, sizeof(pkt_data_buf));
        if (p_usw_pkt_master[lchip]->rx_buf[buf_id].mode == CTC_PKT_MODE_ETH)
        {
            sal_memcpy(raw_hdr, p_usw_pkt_master[lchip]->rx_buf[buf_id].pkt_data+SYS_USW_PKT_CPUMAC_LEN, SYS_USW_PKT_HEADER_LEN);
        }
        else
        {
            sal_memcpy(raw_hdr, p_usw_pkt_master[lchip]->rx_buf[buf_id].pkt_data, SYS_USW_PKT_HEADER_LEN);
        }

        /*for get stacking len in _sys_usw_packet_rawhdr_to_rxinfo*/
        pkt_rx.pkt_buf = &pkt_data_buf;
        pkt_data_buf.data = p_usw_pkt_master[lchip]->rx_buf[buf_id].pkt_data;

        /* convert raw header to rx_info */
        _sys_usw_packet_rawhdr_to_rxinfo(lchip, (uint32*)&raw_hdr, &pkt_rx);

        /* none ptp packet timestamp dump should not change */
        pkt_rx.rx_info.ptp.ts.seconds = p_usw_pkt_master[lchip]->rx_buf[buf_id].seconds;
        pkt_rx.rx_info.ptp.ts.nanoseconds = p_usw_pkt_master[lchip]->rx_buf[buf_id].nanoseconds;
        _sys_usw_packet_dump_rx_header(lchip, &pkt_rx);

        if (p_usw_pkt_master[lchip]->rx_buf[buf_id].mode == CTC_PKT_MODE_ETH)
        {
            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------\n");
            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Packet CPU Header\n");
            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------\n");

            _sys_usw_packet_dump(lchip, p_usw_pkt_master[lchip]->rx_buf[buf_id].pkt_data, SYS_USW_PKT_CPUMAC_LEN);

            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------\n");
            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Packet Bridge Header\n");
            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------\n");

            _sys_usw_packet_dump(lchip, p_usw_pkt_master[lchip]->rx_buf[buf_id].pkt_data+SYS_USW_PKT_CPUMAC_LEN, SYS_USW_PKT_HEADER_LEN);
        }
        else
        {
            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------\n");
            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Packet Bridge Header\n");
            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------\n");

            _sys_usw_packet_dump(lchip, p_usw_pkt_master[lchip]->rx_buf[buf_id].pkt_data, SYS_USW_PKT_HEADER_LEN);
        }

    }

    if (CTC_IS_BIT_SET(flag, 0))
    {
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------\n");
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Packet Info(Length : %d):\n", (p_usw_pkt_master[lchip]->rx_buf[buf_id].pkt_len-header_len));
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------\n");

        /*print packet*/
        len = (p_usw_pkt_master[lchip]->rx_buf[buf_id].pkt_len <= SYS_PKT_BUF_PKT_LEN)?
        (p_usw_pkt_master[lchip]->rx_buf[buf_id].pkt_len-header_len): (SYS_PKT_BUF_PKT_LEN-header_len);
        _sys_usw_packet_dump(lchip, p_usw_pkt_master[lchip]->rx_buf[buf_id].pkt_data+header_len, len);
    }


    return CTC_E_NONE;
}

static int32
_sys_usw_packet_timer_param_check(uint8 lchip, ctc_pkt_tx_timer_t* p_pkt_timer)
{
    Pcie0IpStatus_m pcei_status;
    uint32 cmd = 0;
    uint8 pcie_gen = 0;
    uint8 lane_num = 0;
    uint32 max_trpt = 0;
    uint32 real_rate = 0;

    if (p_pkt_timer->session_num > SYS_PKT_MAX_TX_SESSION || !p_pkt_timer->session_num || !p_pkt_timer->interval)
    {
        return CTC_E_INVALID_PARAM;
    }

    SYS_DMA_PKT_LEN_CHECK(p_pkt_timer->pkt_len);

    cmd = DRV_IOR(Pcie0IpStatus_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcei_status));
    pcie_gen = GetPcie0IpStatus(V, pcie0DlCurLinkSpeed_f, &pcei_status);
    lane_num = GetPcie0IpStatus(V, pcie0DlNegLinkWidth_f, &pcei_status);


    if ((pcie_gen == 2) && lane_num)
    {
        /*gen2: maxrate = 16G*92.8% x 80% x len/(len+24) x 92.8%*/
        max_trpt = 5511*(p_pkt_timer->pkt_len+40)*lane_num/(p_pkt_timer->pkt_len+40+24);
        real_rate = (p_pkt_timer->pkt_len+40)*p_pkt_timer->session_num*8/p_pkt_timer->interval;
        if (max_trpt*1000 < real_rate)
        {
            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Exceed pcie Gen2 trpt, max_trpt:%uK, cfg_rate:%uK\n", max_trpt*1000, real_rate);
            return CTC_E_INVALID_PARAM;
        }

    }
    else if ((pcie_gen == 3) && lane_num)
    {
        /*gen3: maxrate = 16G*92.8% x 98% x len/(len+24) x 92.8%*/
        max_trpt = 6751*(p_pkt_timer->pkt_len+40)*lane_num/(p_pkt_timer->pkt_len+40+24);
        real_rate = (p_pkt_timer->pkt_len+40)*p_pkt_timer->session_num*8/p_pkt_timer->interval;
        if (max_trpt*1000 < real_rate)
        {
            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Exceed pcie Gen3 trpt, max_trpt:%uK, cfg_rate:%uK\n", max_trpt*1000, real_rate);
            return CTC_E_INVALID_PARAM;
        }
    }

    return CTC_E_NONE;
}


int32
sys_usw_packet_dump_db(uint8 lchip, sal_file_t dump_db_fp,ctc_global_dump_db_t* p_dump_param)
{
    int32 ret = CTC_E_NONE;
    uint32 loop = 0;

    SYS_PACKET_INIT_CHECK(lchip);
    SYS_PACKET_LOCK(lchip);

    SYS_DUMP_DB_LOG(dump_db_fp, "\n");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "# Packet");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "{");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "Master config:");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "----------------------------------------------------------------------------------------------------------------------");
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%-48s\n","cpu_mac_sa", sys_output_mac(p_usw_pkt_master[lchip]->cpu_mac_sa));
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%-48s\n","cpu_mac_da", sys_output_mac(p_usw_pkt_master[lchip]->cpu_mac_da));
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","tx_dump_enable",p_usw_pkt_master[lchip]->tx_dump_enable);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","is_skb_buf",p_usw_pkt_master[lchip]->is_skb_buf);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","pkt_buf_en[CTC_INGRESS]",p_usw_pkt_master[lchip]->pkt_buf_en[CTC_INGRESS]);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","pkt_buf_en[CTC_EGRESS]",p_usw_pkt_master[lchip]->pkt_buf_en[CTC_EGRESS]);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","rx_buf_reason_id",p_usw_pkt_master[lchip]->rx_buf_reason_id);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:0x","session_bm");
    for (loop = 0; loop < STS_PKT_SESSION_BLOCK_NUM; loop++)
    {
        SYS_DUMP_DB_LOG(dump_db_fp, "%x",p_usw_pkt_master[lchip]->session_bm[loop]);
    }
    SYS_DUMP_DB_LOG(dump_db_fp, "\n");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "----------------------------------------------------------------------------------------------------------------------\n");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "}");
    SYS_PACKET_UNLOCK(lchip);

    return ret;
}

#define ___________InnerAPI_____________

int32
sys_usw_packet_register_drop_rx_cb(uint8 lchip, CTC_PKT_RX_CALLBACK drop_rx_cb)
{
    SYS_PACKET_INIT_CHECK(lchip);
    SYS_PACKET_LOCK(lchip);
    p_usw_pkt_master[lchip]->drop_rx_cb = drop_rx_cb;
    SYS_PACKET_UNLOCK(lchip);
    return CTC_E_NONE;
}


int32
sys_usw_packet_buffer_clear(uint8 lchip, uint8 bitmap)
{
    LCHIP_CHECK(lchip);
    SYS_PACKET_INIT_CHECK(lchip);

    /*rx clear*/
    SYS_PACKET_RX_LOCK(lchip);
    if (CTC_IS_BIT_SET(bitmap, 0))
    {
        sal_memset(&p_usw_pkt_master[lchip]->rx_buf, 0, sizeof(p_usw_pkt_master[lchip]->rx_buf));
        (p_usw_pkt_master[lchip]->cursor[CTC_INGRESS]) = 0;
    }
    SYS_PACKET_RX_UNLOCK(lchip);

    /*tx clear*/
    SYS_PACKET_LOCK(lchip);
    if (CTC_IS_BIT_SET(bitmap, 1))
    {
        sal_memset(&p_usw_pkt_master[lchip]->tx_buf, 0, sizeof(p_usw_pkt_master[lchip]->tx_buf));
        (p_usw_pkt_master[lchip]->cursor[CTC_EGRESS]) = 0;
    }
    SYS_PACKET_UNLOCK(lchip);

    return CTC_E_NONE;
}

/**
 @brief Display packet
*/
int32
sys_usw_packet_stats_dump(uint8 lchip)
{
    uint16 idx = 0;
    uint64 rx = 0;
    char* p_str_tmp = NULL;
    char str[40] = {0};
    uint32 reason_cnt = 0;

    LCHIP_CHECK(lchip);
    SYS_PACKET_INIT_CHECK(lchip);

   /*dump tx stats*/
    SYS_PACKET_LOCK(lchip);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Packet Tx Statistics:\n");
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "------------------------------\n");
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %"PRIu64" \n", "Total TX Count", (p_usw_pkt_master[lchip]->stats.uc_tx+p_usw_pkt_master[lchip]->stats.mc_tx));

    if ((p_usw_pkt_master[lchip]->stats.uc_tx) || (p_usw_pkt_master[lchip]->stats.mc_tx))
    {
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "--%-18s: %"PRIu64" \n", "Uc Count", p_usw_pkt_master[lchip]->stats.uc_tx);
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "--%-18s: %"PRIu64" \n", "Mc Count", p_usw_pkt_master[lchip]->stats.mc_tx);
    }


    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
    SYS_PACKET_UNLOCK(lchip);



    /*dump rx stats*/
    SYS_PACKET_RX_LOCK(lchip);
    for (idx  = 0; idx  < CTC_PKT_CPU_REASON_MAX_COUNT; idx ++)
    {
        rx += p_usw_pkt_master[lchip]->stats.rx[idx];
    }

    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Packet Rx Statistics:\n");
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "------------------------------\n");
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %"PRIu64" \n", "Total RX Count", rx);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s\n", "Detail RX Count");

    if (rx)
    {
        for (idx  = 0; idx  < CTC_PKT_CPU_REASON_MAX_COUNT; idx ++)
        {
            reason_cnt = p_usw_pkt_master[lchip]->stats.rx[idx];

            if (reason_cnt)
            {
                p_str_tmp = sys_usw_reason_2Str(idx);
                sal_sprintf((char*)&str, "%s%s%d%s", p_str_tmp, "(ID:", idx, ")");

                SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s: %d \n", (char*)&str, reason_cnt);
            }
        }
    }
    SYS_PACKET_RX_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_packet_stats_clear(uint8 lchip)
{
    LCHIP_CHECK(lchip);
    SYS_PACKET_INIT_CHECK(lchip);

    /*clear tx stats*/
    SYS_PACKET_LOCK(lchip);
    p_usw_pkt_master[lchip]->stats.encap = 0;
    p_usw_pkt_master[lchip]->stats.mc_tx= 0;
    p_usw_pkt_master[lchip]->stats.uc_tx= 0;
    SYS_PACKET_UNLOCK(lchip);

    /*clear rx stats*/
    SYS_PACKET_RX_LOCK(lchip);
    sal_memset(p_usw_pkt_master[lchip]->stats.rx, 0, (CTC_PKT_CPU_REASON_MAX_COUNT*sizeof(uint64)));
    p_usw_pkt_master[lchip]->stats.decap = 0;
    SYS_PACKET_RX_UNLOCK(lchip);

    return CTC_E_NONE;
}


int32
sys_usw_packet_set_rx_reason_log(uint8 lchip, uint16 reason_id, uint8 enable, uint8 is_all, uint8 is_detail)
{
    uint16 index = 0;
    LCHIP_CHECK(lchip);
    SYS_PACKET_INIT_CHECK(lchip);

    if ((reason_id >= CTC_PKT_CPU_REASON_MAX_COUNT) && !is_all)
    {
        return CTC_E_INVALID_PARAM;
    }
    SYS_PACKET_RX_LOCK(lchip);
    if (enable)
    {
        if (is_all)
        {
            for (index = 0; index < CTC_PKT_CPU_REASON_MAX_COUNT; index++)
            {
                CTC_BMP_SET(p_usw_pkt_master[lchip]->rx_reason_bm, index);
                p_usw_pkt_master[lchip]->rx_header_en[index] = is_detail;
            }
        }
        else
        {
            CTC_BMP_SET(p_usw_pkt_master[lchip]->rx_reason_bm, reason_id);
            p_usw_pkt_master[lchip]->rx_header_en[reason_id] = is_detail;
        }
    }
    else
    {
        if (is_all)
        {
            for (index = 0; index < CTC_PKT_CPU_REASON_MAX_COUNT; index++)
            {
                CTC_BMP_UNSET(p_usw_pkt_master[lchip]->rx_reason_bm, index);
                p_usw_pkt_master[lchip]->rx_header_en[index] = is_detail;
            }
        }
        else
        {
            CTC_BMP_UNSET(p_usw_pkt_master[lchip]->rx_reason_bm, reason_id);
            p_usw_pkt_master[lchip]->rx_header_en[reason_id] = is_detail;
        }
    }
    SYS_PACKET_RX_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_packet_register_internal_rx_cb(uint8 lchip, SYS_PKT_RX_CALLBACK oam_rx_cb)
{
    LCHIP_CHECK(lchip);
    SYS_PACKET_INIT_CHECK(lchip);

    SYS_PACKET_RX_LOCK(lchip);
    p_usw_pkt_master[lchip]->oam_rx_cb = oam_rx_cb;
    SYS_PACKET_RX_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_packet_rx_buffer_dump(uint8 lchip, uint8 start , uint8 end, uint8 flag, uint8 is_brief)
{
    uint8 header_len = 0;
    uint8 idx    = 0;
    uint8 cursor = 0;
    uint8 loop = 0;
    sys_pkt_buf_t* p_pkt_rx = NULL;
    char str_time[64] = {0};

    LCHIP_CHECK(lchip);
    SYS_PACKET_INIT_CHECK(lchip);
    SYS_PACKET_RX_LOCK(lchip);

    cursor = p_usw_pkt_master[lchip]->cursor[CTC_INGRESS];
    /*Brief overview */
    if (is_brief)
    {
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8s  %-8s  %-8s %-6s    %-10s  %-10s    %-32s\n",
                                                 "PKT-NO.","Pkt-len","srcPort"," lPort", "CPU reason","Pkt-type","Timestamp");
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "------------------------------------------------------------------------------------------\n");
        for (loop = 0;loop < cursor ; loop++)
        {
            ctc_pkt_rx_t pkt_rx;
            ds_t raw_hdr;
            ctc_pkt_info_t* p_rx_info = NULL;
            ctc_pkt_buf_t  pkt_data_buf;

            p_pkt_rx = &p_usw_pkt_master[lchip]->rx_buf[loop];
            sal_memset(&pkt_rx, 0, sizeof(pkt_rx));
            sal_memset(raw_hdr, 0, sizeof(raw_hdr));
            sal_memset(&pkt_data_buf, 0, sizeof(pkt_data_buf));
            if (p_pkt_rx->mode == CTC_PKT_MODE_ETH)
            {
                sal_memcpy(raw_hdr, p_pkt_rx->pkt_data+SYS_USW_PKT_CPUMAC_LEN, SYS_USW_PKT_HEADER_LEN);
            }
            else
            {
                sal_memcpy(raw_hdr, p_pkt_rx->pkt_data, SYS_USW_PKT_HEADER_LEN);
            }

            /*for get stacking len in _sys_usw_packet_rawhdr_to_rxinfo*/
            pkt_rx.pkt_buf = &pkt_data_buf;
            pkt_data_buf.data = p_usw_pkt_master[lchip]->rx_buf[loop].pkt_data;

            _sys_usw_packet_rawhdr_to_rxinfo(lchip, (uint32*)&raw_hdr, &pkt_rx);
            p_rx_info = &(pkt_rx.rx_info);

            header_len = p_pkt_rx->mode?(SYS_USW_PKT_HEADER_LEN+SYS_USW_PKT_CPUMAC_LEN):SYS_USW_PKT_HEADER_LEN;
            sal_strncpy(str_time, sal_ctime(&p_pkt_rx->tm), (sal_strlen(sal_ctime(&p_pkt_rx->tm))-1));
            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8d  %-8d  0x%.4x    0x%.5x  %-10d  %-11s   %-32s\n",
                loop ,p_pkt_rx->pkt_len -header_len ,p_rx_info->src_port ,p_rx_info->lport ,p_rx_info->reason ,sys_usw_get_pkt_type_desc(p_rx_info->packet_type) ,str_time );

        }
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "------------------------------------------------------------------------------------------\n");
    }
    else
    {
        for (;start <= end && start <= cursor ; start++)
        {
            CTC_BIT_SET(flag, 0);
            header_len = p_usw_pkt_master[lchip]->rx_buf[start].mode?(SYS_USW_PKT_HEADER_LEN+SYS_USW_PKT_CPUMAC_LEN):SYS_USW_PKT_HEADER_LEN;

            _sys_usw_packet_rx_common_dump(lchip, start, flag, &idx, header_len);
        }
    }

    SYS_PACKET_RX_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_packet_rx(uint8 lchip, ctc_pkt_rx_t* p_pkt_rx)
{
    LCHIP_CHECK(lchip);
    SYS_PACKET_INIT_CHECK(lchip);

    SYS_PACKET_RX_LOCK(lchip);
    CTC_ERROR_RETURN_PACKET_RX_UNLOCK(lchip, _sys_usw_packet_rx(lchip, p_pkt_rx));
    SYS_PACKET_RX_UNLOCK(lchip);
    return CTC_E_NONE;
}


int32
sys_usw_packet_tx_buffer_dump(uint8 lchip, uint8 start , uint8 end, uint8 flag, uint8 is_brief)
{
    uint8 header_len = 0;
    uint8 idx    = 0;
    uint8 cursor = 0;
    uint8 loop = 0;
    sys_pkt_tx_buf_t* p_pkt_tx = NULL;
    char str_time[64] = {0};

    LCHIP_CHECK(lchip);
    SYS_PACKET_INIT_CHECK(lchip);

    SYS_PACKET_LOCK(lchip);

    cursor = p_usw_pkt_master[lchip]->cursor[CTC_EGRESS];
    /*Brief overview*/
    if (is_brief)
    {
        char* p_temp = NULL;
        uint32 dest_id = 0;

        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8s  %-8s  %-12s  %-20s %-12s  %-32s\n",
                                                 "PKT-NO.","Pkt-len","Ucast/Mcast","Dest-port/Group-id","Dsnh-offset","Timestamp");
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "------------------------------------------------------------------------------------------------\n");
        for (loop = 0;loop <cursor ; loop++)
        {
            p_pkt_tx = &p_usw_pkt_master[lchip]->tx_buf[loop];
            header_len = p_pkt_tx->mode?(SYS_USW_PKT_HEADER_LEN+SYS_USW_PKT_CPUMAC_LEN):SYS_USW_PKT_HEADER_LEN;
            sal_strncpy(str_time, sal_ctime(&p_pkt_tx->tm), (sal_strlen(sal_ctime(&p_pkt_tx->tm))-1));
            p_temp = CTC_FLAG_ISSET(p_pkt_tx->tx_info.flags, CTC_PKT_FLAG_MCAST) ? "Mcast":"Ucast";
            dest_id = CTC_FLAG_ISSET(p_pkt_tx->tx_info.flags, CTC_PKT_FLAG_MCAST) ?  p_pkt_tx->tx_info.dest_group_id:p_pkt_tx->tx_info.dest_gport;
            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8d  %-8d  %-12s  %-20d %-12d  %-32s\n",
                                                     loop ,p_pkt_tx->pkt_len -header_len ,p_temp ,dest_id ,p_pkt_tx->tx_info.nh_offset, str_time);

        }
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "------------------------------------------------------------------------------------------------\n");
    }
    else
    {
        for (;start <= end && start <= cursor ; start++)
        {
            CTC_BIT_SET(flag, 0);
            header_len = p_usw_pkt_master[lchip]->tx_buf[start].mode?(SYS_USW_PKT_HEADER_LEN+SYS_USW_PKT_CPUMAC_LEN):SYS_USW_PKT_HEADER_LEN;

            _sys_usw_packet_tx_common_dump(lchip, start, flag, &idx, header_len);
        }
    }

    SYS_PACKET_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
sys_usw_packet_tx_dump_enable(uint8 lchip, uint8 enable)
{
    LCHIP_CHECK(lchip);
    SYS_PACKET_INIT_CHECK(lchip);

    SYS_PACKET_LOCK(lchip);
    if (enable)
    {
        p_usw_pkt_master[lchip]->tx_dump_enable = 1;
    }
    else
    {
        p_usw_pkt_master[lchip]->tx_dump_enable = 0;
    }
    SYS_PACKET_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_packet_tx_discard_pkt(uint8 lchip, uint8 buf_id, ctc_pkt_tx_t* p_pkt_tx)
{
    uint8* p_data = NULL;
    uint32 pkt_len = 0;
    ctc_pkt_skb_t* p_skb = &(p_pkt_tx->skb);
    uint32  header_len = 0;
    sys_pkt_buf_t* p_rx_buf = NULL;

    LCHIP_CHECK(lchip);
    SYS_PACKET_INIT_CHECK(lchip);

    if (buf_id >= SYS_PKT_BUF_MAX )
    {
        return CTC_E_INVALID_PARAM;
    }

    SYS_PACKET_RX_LOCK(lchip);

    p_rx_buf = &(p_usw_pkt_master[lchip]->rx_buf[buf_id]);
    header_len = p_rx_buf->mode?(SYS_USW_PKT_HEADER_LEN+SYS_USW_PKT_CPUMAC_LEN):SYS_USW_PKT_HEADER_LEN;
    pkt_len    = (p_rx_buf->pkt_len < header_len)?0:(p_rx_buf->pkt_len - header_len);
    if (!pkt_len)
    {
        SYS_PACKET_RX_UNLOCK(lchip);
        return CTC_E_INVALID_PARAM;
    }

    ctc_packet_skb_init(p_skb);
    p_data = ctc_packet_skb_put(p_skb, pkt_len);
    sal_memcpy(p_data,p_rx_buf->pkt_data + header_len, pkt_len);

    SYS_PACKET_RX_UNLOCK(lchip);

    SYS_PACKET_LOCK(lchip);
    CTC_ERROR_RETURN_PACKET_UNLOCK(lchip, _sys_usw_packet_tx(lchip, p_pkt_tx));
    SYS_PACKET_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
sys_usw_packet_pkt_buf_en(uint8 lchip, uint8 dir,uint16 reason_id,uint8 enable)
{
    LCHIP_CHECK(lchip);
    SYS_PACKET_INIT_CHECK(lchip);

    if (reason_id >= CTC_PKT_CPU_REASON_MAX_COUNT)
    {
        return CTC_E_INVALID_PARAM;
    }

    if (dir == CTC_EGRESS) /*tx direction*/
    {
        SYS_PACKET_LOCK(lchip);

        p_usw_pkt_master[lchip]->pkt_buf_en[CTC_EGRESS] = enable?1:0;

        SYS_PACKET_UNLOCK(lchip);
    }
    else if (dir == CTC_INGRESS) /*rx direction*/
    {
        SYS_PACKET_RX_LOCK(lchip);

        if (enable <= 1)
        {
            p_usw_pkt_master[lchip]->pkt_buf_en[CTC_INGRESS] = enable?1:0;
        }
        p_usw_pkt_master[lchip]->rx_buf_reason_id = reason_id;

        SYS_PACKET_RX_UNLOCK(lchip);
    }

    return CTC_E_NONE;
}


int32
sys_usw_packet_get_rx_buffer(uint8 lchip,
                                      uint16 lport,
                                      int32 (*filter)(uint8 *packet, uint16 *match, uint8 **got_value),
                                      void** p_pkt_test_rx,
                                      uint8* p_match_num)
{
    uint8 cursor = 0;
    uint8 loop = 0;
    sys_pkt_buf_t* p_pkt_rx = NULL;
    sys_pkt_buf_t** p_pkt_test_rx_buf = NULL;
    uint16 match = 0;
    uint8* value = 0;

    p_pkt_test_rx_buf = (sys_pkt_buf_t**)p_pkt_test_rx;
    LCHIP_CHECK(lchip);
    SYS_PACKET_INIT_CHECK(lchip);
    SYS_PACKET_RX_LOCK(lchip);
    cursor = p_usw_pkt_master[lchip]->cursor[CTC_INGRESS];
    for (loop = 0; loop < cursor && loop < 0xFF; loop++)
    {
        ctc_pkt_rx_t pkt_rx;
        ds_t raw_hdr;
        ctc_pkt_info_t* p_rx_info = NULL;

        p_pkt_rx = &p_usw_pkt_master[lchip]->rx_buf[loop];
        sal_memset(&pkt_rx, 0, sizeof(pkt_rx));
        sal_memset(raw_hdr, 0, sizeof(raw_hdr));
        sal_memcpy(raw_hdr, p_pkt_rx->pkt_data, SYS_USW_PKT_HEADER_LEN);
        _sys_usw_packet_rawhdr_to_rxinfo(lchip, (uint32*)&raw_hdr, &pkt_rx);
        p_rx_info = &(pkt_rx.rx_info);
        (*filter)(p_pkt_rx->pkt_data + 40, &match, &value);

        if (lport == p_rx_info->lport && match && p_rx_info->reason == CTC_PKT_CPU_REASON_MIRRORED_TO_CPU)
        {
            sal_memcpy((*p_pkt_test_rx_buf) + (*p_match_num), p_pkt_rx, sizeof(sys_pkt_buf_t));
            (*p_match_num)++;
        }
    }

    SYS_PACKET_RX_UNLOCK(lchip);
    return CTC_E_NONE;
}


#define __________OuterAPI__________
int32
sys_usw_packet_tx_performance_test(uint8 lchip, ctc_pkt_tx_t* p_pkt_tx)
{
    int32 ret = 0;
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    LCHIP_CHECK(lchip);
    SYS_PACKET_INIT_CHECK(lchip);

    CTC_PTR_VALID_CHECK(p_pkt_tx);
    p_pkt_tx->mode = CTC_PKT_MODE_DMA;
    p_pkt_tx->tx_info.flags = CTC_PKT_FLAG_NH_OFFSET_BYPASS;
    SYS_PACKET_LOCK(lchip);
    CTC_ERROR_GOTO(_sys_usw_packet_tx_param_check(lchip, p_pkt_tx), ret, error_end);

    CTC_ERROR_GOTO(_sys_usw_packet_encap(lchip, p_pkt_tx), ret, error_end);

    p_pkt_tx->tx_info.flags = 0xFFFFFFFF;
    if (p_usw_pkt_master[lchip]->cfg.dma_tx_cb )
    {
        CTC_ERROR_GOTO(p_usw_pkt_master[lchip]->cfg.dma_tx_cb(p_pkt_tx), ret, error_end);
    }
    else
    {
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Do not support Dma Tx!\n");
        SYS_PACKET_UNLOCK(lchip);
        return CTC_E_NOT_SUPPORT;
    }
error_end:
    SYS_PACKET_UNLOCK(lchip);
    return ret;
}
int32
sys_usw_packet_set_tx_timer(uint8 lchip, ctc_pkt_tx_timer_t* p_pkt_timer)
{
    LCHIP_CHECK(lchip);
    if (!p_usw_pkt_master[lchip])
    {
        return CTC_E_NOT_INIT;
    }

    switch (p_pkt_timer->timer_state)
    {
        case 0:
            /*pending state, cfg packet tx timer parameter*/
            CTC_ERROR_RETURN(_sys_usw_packet_timer_param_check(lchip, p_pkt_timer));
            CTC_ERROR_RETURN(sys_usw_dma_set_packet_timer_cfg(lchip, p_pkt_timer->session_num, p_pkt_timer->interval, p_pkt_timer->pkt_len, 0));
            CTC_ERROR_RETURN(sys_usw_dma_set_chan_en(lchip, SYS_DMA_PKT_TX_TIMER_CHAN_ID, 0));
            break;

        case 1:
            /*start tx, enable timer*/
            CTC_ERROR_RETURN(sys_usw_dma_set_pkt_timer(lchip, p_pkt_timer->interval, 1));
            break;

        case 2:
            /*timer stop*/
            CTC_ERROR_RETURN(sys_usw_dma_set_pkt_timer(lchip, p_pkt_timer->interval, 0));
            break;

        case 3:
            /*destroy timer*/
            CTC_ERROR_RETURN(sys_usw_dma_set_pkt_timer(lchip, p_pkt_timer->interval, 0));
            CTC_ERROR_RETURN(sys_usw_dma_set_packet_timer_cfg(lchip, p_pkt_timer->session_num, p_pkt_timer->interval, p_pkt_timer->pkt_len, 1));
            sal_memset(p_usw_pkt_master[lchip]->session_bm, 0, sizeof(p_usw_pkt_master[lchip]->session_bm));

            break;

        default:
            return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

void*
sys_usw_packet_tx_alloc(uint8 lchip, uint32 size)
{
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    return sys_usw_dma_tx_alloc(lchip, size);
}

void
sys_usw_packet_tx_free(uint8 lchip, void* addr)
{
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    sys_usw_dma_tx_free(lchip, addr);
    return;
}

int32
sys_usw_packet_tx(uint8 lchip, ctc_pkt_tx_t* p_pkt_tx)
{
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    LCHIP_CHECK(lchip);
    SYS_PACKET_INIT_CHECK(lchip);

    CTC_ERROR_RETURN(_sys_usw_packet_tx(lchip, p_pkt_tx));

    return CTC_E_NONE;
}

int32
sys_usw_packet_tx_array(uint8 lchip, ctc_pkt_tx_t **p_pkt_array, uint32 count, ctc_pkt_callback all_done_cb, void *cookie)
{
    int32 ret = CTC_E_NONE;
    uint32 pkt_idx = 0;

    LCHIP_CHECK(lchip);
    SYS_PACKET_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_pkt_array);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    SYS_PACKET_LOCK(lchip);
    for(pkt_idx = 0; pkt_idx < count; pkt_idx++)
    {
        if(!p_pkt_array[pkt_idx])
        {
            SYS_PACKET_UNLOCK(lchip);
            return CTC_E_INVALID_PARAM;
        }
        if(p_pkt_array[pkt_idx]->callback)
        {
            SYS_PACKET_UNLOCK(lchip);
            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "tx array not support async mode, packet->callback must be NULL\n");
            return CTC_E_INVALID_PARAM;
        }
        ret = _sys_usw_packet_tx(lchip, p_pkt_array[pkt_idx]);
        if(ret < 0)
        {
            SYS_PACKET_UNLOCK(lchip);
            SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "tx packet error, current pkt index:%d!\n",pkt_idx);
            return ret;
        }
    }

    (all_done_cb)(*p_pkt_array ,cookie);
    SYS_PACKET_UNLOCK(lchip);
    return ret;
}

int32
sys_usw_packet_encap(uint8 lchip, ctc_pkt_tx_t* p_pkt_tx)
{
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    LCHIP_CHECK(lchip);
    SYS_PACKET_INIT_CHECK(lchip);

    SYS_PACKET_LOCK(lchip);
    CTC_ERROR_RETURN_PACKET_UNLOCK(lchip, _sys_usw_packet_encap(lchip, p_pkt_tx));
    SYS_PACKET_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_packet_decap(uint8 lchip, ctc_pkt_rx_t* p_pkt_rx)
{
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    LCHIP_CHECK(lchip);
    SYS_PACKET_INIT_CHECK(lchip);

    SYS_PACKET_RX_LOCK(lchip);
    CTC_ERROR_RETURN_PACKET_RX_UNLOCK(lchip, _sys_usw_packet_decap(lchip, p_pkt_rx));
    SYS_PACKET_RX_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_packet_create_netif(uint8 lchip, ctc_pkt_netif_t* p_netif)
{
    uint8 gchip = 0;
    int32 ret = CTC_E_NONE;
    dal_netif_t dal_netif = {0};
    ctc_pkt_netif_t *p_new_netif = NULL;

    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    CTC_MAX_VALUE_CHECK(p_netif->type, CTC_PKT_NETIF_T_VLAN);
    SYS_GLOBAL_PORT_CHECK(p_netif->gport);
    if ((p_netif->type == CTC_PKT_NETIF_T_VLAN) || p_netif->vlan)
    {
        CTC_VLAN_RANGE_CHECK(p_netif->vlan);
    }

    if (!g_dal_op.handle_netif)
    {
        return CTC_E_NOT_SUPPORT;
    }

    p_new_netif = ctc_hash_lookup(p_usw_pkt_master[lchip]->netif_hash, p_netif);
    if (p_new_netif)
    {
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Netif exist!\n");
        return CTC_E_EXIST;
    }

    sys_usw_get_gchip_id(lchip, &gchip);
    dal_netif.op_type = DAL_OP_CREATE;
    dal_netif.lchip = lchip;
    dal_netif.type = p_netif->type;
    dal_netif.vlan = p_netif->vlan;
    dal_netif.gport = (p_netif->type == CTC_PKT_NETIF_T_PORT) ? p_netif->gport : (gchip << CTC_LOCAL_PORT_LENGTH);
    sal_memcpy(dal_netif.mac, p_netif->mac, 6);
    sal_memcpy(dal_netif.name, p_netif->name, CTC_PKT_MAX_NETIF_NAME_LEN);

    ret = g_dal_op.handle_netif(lchip, &dal_netif);
    if (ret)
    {
        return CTC_E_INVALID_CONFIG;
    }

    p_new_netif = (ctc_pkt_netif_t*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(ctc_pkt_netif_t));
    if (!p_new_netif)
    {
        return CTC_E_NO_MEMORY;
    }
    sal_memcpy(p_new_netif, p_netif, sizeof(ctc_pkt_netif_t));
    sal_memcpy(p_new_netif->name, dal_netif.name, CTC_PKT_MAX_NETIF_NAME_LEN);
    p_new_netif->vlan = (p_netif->type == CTC_PKT_NETIF_T_PORT) ? 0 : p_netif->vlan;
    p_new_netif->gport = (p_netif->type == CTC_PKT_NETIF_T_PORT) ? p_netif->gport : 0;

    ctc_hash_insert(p_usw_pkt_master[lchip]->netif_hash, p_new_netif);

    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Create netif Name: %s\tType: %s\tGport: 0x%x\tVlan: %d\tMac: %s\n",
        p_new_netif->name, ((p_new_netif->type == CTC_PKT_NETIF_T_PORT) ? "port" : "vlan"), p_new_netif->gport, p_new_netif->vlan, sys_output_mac(p_new_netif->mac));

    return CTC_E_NONE;
}

int32
sys_usw_packet_destory_netif(uint8 lchip, ctc_pkt_netif_t* p_netif)
{
    int32 ret = CTC_E_NONE;
    dal_netif_t dal_netif = {0};
    ctc_pkt_netif_t* p_del_netif = NULL;

    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    CTC_MAX_VALUE_CHECK(p_netif->type, CTC_PKT_NETIF_T_VLAN);
    SYS_GLOBAL_PORT_CHECK(p_netif->gport);
    if ((p_netif->type == CTC_PKT_NETIF_T_VLAN) || p_netif->vlan)
    {
        CTC_VLAN_RANGE_CHECK(p_netif->vlan);
    }

    if (!g_dal_op.handle_netif)
    {
        return CTC_E_NOT_SUPPORT;
    }

    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Remove netif Type: %s\tGport: 0x%x\tvlan: %d\n",
        ((p_netif->type == CTC_PKT_NETIF_T_PORT) ? "port" : "vlan"), p_netif->gport, p_netif->vlan);

    dal_netif.op_type = DAL_OP_DESTORY;
    dal_netif.type = p_netif->type;
    dal_netif.gport = p_netif->gport;
    dal_netif.vlan = p_netif->vlan;

    ret = g_dal_op.handle_netif(lchip, &dal_netif);
    if (ret)
    {
        return CTC_E_INVALID_CONFIG;
    }

    p_del_netif = ctc_hash_remove(p_usw_pkt_master[lchip]->netif_hash, p_netif);
    if (p_del_netif)
    {
        mem_free(p_del_netif);
    }

    return CTC_E_NONE;
}

int32
sys_usw_packet_get_netif(uint8 lchip, ctc_pkt_netif_t* p_netif)
{
    int32 ret = CTC_E_NONE;
    dal_netif_t dal_netif = {0};
    ctc_pkt_netif_t* p_get_netif = NULL;

    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    CTC_MAX_VALUE_CHECK(p_netif->type, CTC_PKT_NETIF_T_VLAN);
    SYS_GLOBAL_PORT_CHECK(p_netif->gport);
    if ((p_netif->type == CTC_PKT_NETIF_T_VLAN) || p_netif->vlan)
    {
        CTC_VLAN_RANGE_CHECK(p_netif->vlan);
    }

    if (!g_dal_op.handle_netif)
    {
        return CTC_E_NOT_SUPPORT;
    }

    dal_netif.op_type = DAL_OP_GET;
    dal_netif.type = p_netif->type;
    dal_netif.gport = p_netif->gport;
    dal_netif.vlan = p_netif->vlan;

    ret = g_dal_op.handle_netif(lchip, &dal_netif);
    if (ret)
    {
        return CTC_E_NOT_EXIST;
    }

    p_get_netif = ctc_hash_lookup(p_usw_pkt_master[lchip]->netif_hash, p_netif);
    if (!p_get_netif)
    {
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not found \n");
        return CTC_E_NOT_EXIST;
    }
    sal_memcpy(p_netif, p_get_netif, sizeof(ctc_pkt_netif_t));

    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get netif Name: %s\tType: %s\tGport: 0x%x\tVlan: %d\tMac: %s\n",
        p_netif->name, ((p_netif->type == CTC_PKT_NETIF_T_PORT) ? "port" : "vlan"), p_netif->gport, p_netif->vlan, sys_output_mac(p_netif->mac));

    return CTC_E_NONE;
}

int32
_sys_usw_packet_show_netif_info(ctc_pkt_netif_t* p_netif, void* data)
{
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%8s\t%s\t0x%08x\t%s\n",
        p_netif->name, ((p_netif->type == CTC_PKT_NETIF_T_PORT) ? "port" : "vlan"),
        ((p_netif->type == CTC_PKT_NETIF_T_PORT) ? p_netif->gport : p_netif->vlan), sys_output_mac(p_netif->mac));

    return CTC_E_NONE;
}

int32
sys_usw_packet_netif_show(uint8 lchip)
{
    if (!g_dal_op.handle_netif)
    {
        return CTC_E_NOT_SUPPORT;
    }
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Netif count: %d\n", p_usw_pkt_master[lchip]->netif_hash->count);
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%8s\t%s\t%10s\t%14s\n", "Name", "Type", "Gport/Vlan", "Mac");
    SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "--------------------------------------------------------------\n");
    ctc_hash_traverse_through(p_usw_pkt_master[lchip]->netif_hash, (hash_traversal_fn)_sys_usw_packet_show_netif_info, NULL);

    return CTC_E_NONE;
}

int32
_sys_usw_packet_wb_sync_netif_func(ctc_pkt_netif_t* p_netif, void *user_data)
{
    uint16 max_entry_cnt = 0;
    sys_wb_pkt_netif_t  *p_wb_netif;
    sys_traverse_t *data = (sys_traverse_t *)user_data;
    ctc_wb_data_t *wb_data = (ctc_wb_data_t *)(data->data);

    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (wb_data->key_len + wb_data->data_len);

    p_wb_netif = (sys_wb_pkt_netif_t *)wb_data->buffer + wb_data->valid_cnt;

    p_wb_netif->type = p_netif->type;
    p_wb_netif->rsv = 0;
    p_wb_netif->vlan = p_netif->vlan;
    p_wb_netif->gport = p_netif->gport;
    sal_memcpy(p_wb_netif->mac, p_netif->mac, sizeof(mac_addr_t));
    sal_memcpy(p_wb_netif->name, p_netif->name, CTC_PKT_MAX_NETIF_NAME_LEN);

    if (++wb_data->valid_cnt == max_entry_cnt)
    {
        CTC_ERROR_RETURN(ctc_wb_add_entry(wb_data));
        wb_data->valid_cnt = 0;
    }

    return CTC_E_NONE;
}


int32
sys_usw_packet_wb_sync(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    ctc_wb_data_t wb_data;
    sys_traverse_t pkt_hash_data;
    sys_wb_pkt_master_t  *p_wb_pkt_master;

    /*syncup packet matser*/
    wb_data.buffer = mem_malloc(MEM_SYSTEM_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_data.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_pkt_master_t, CTC_FEATURE_PACKET, SYS_WB_APPID_PACKET_SUBID_MASTER);

    p_wb_pkt_master = (sys_wb_pkt_master_t  *)wb_data.buffer;

    p_wb_pkt_master->lchip = lchip;
    p_wb_pkt_master->version = SYS_WB_VERSION_PACKET;

    wb_data.valid_cnt = 1;
    CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);

    /*syncup netif*/
    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_pkt_netif_t, CTC_FEATURE_PACKET, SYS_WB_APPID_PACKET_SUBID_NETIF);
    pkt_hash_data.data = &wb_data;
    pkt_hash_data.value1 = lchip;

    CTC_ERROR_GOTO(ctc_hash_traverse(p_usw_pkt_master[lchip]->netif_hash, (hash_traversal_fn) _sys_usw_packet_wb_sync_netif_func, (void *)&pkt_hash_data), ret, done);
    if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
        wb_data.valid_cnt = 0;
    }

done:
    if (wb_data.buffer)
    {
        mem_free(wb_data.buffer);
    }

    return ret;
}

int32
sys_usw_packet_wb_restore(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    uint16 entry_cnt = 0;
    ctc_wb_query_t wb_query;
    sys_wb_pkt_master_t  wb_pkt_master = {0};
    sys_wb_pkt_netif_t  wb_netif;
    ctc_pkt_netif_t  *p_netif;

    sal_memset(&wb_query, 0, sizeof(ctc_wb_query_t));
    wb_query.buffer = mem_malloc(MEM_SYSTEM_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_query.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    CTC_WB_INIT_QUERY_T((&wb_query),sys_wb_pkt_master_t, CTC_FEATURE_PACKET, SYS_WB_APPID_PACKET_SUBID_MASTER);

    CTC_ERROR_GOTO(ctc_wb_query_entry(&wb_query), ret, done);

    /*restore  packet master*/
    if (wb_query.valid_cnt != 1 || wb_query.is_end != 1)
    {
        CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "query packet master error! valid_cnt: %d, is_end: %d.\n", wb_query.valid_cnt, wb_query.is_end);
        ret = CTC_E_INVALID_CONFIG;
        goto done;
    }

    sal_memcpy((uint8*)&wb_pkt_master, (uint8*)wb_query.buffer, wb_query.key_len + wb_query.data_len);

    if (CTC_WB_VERSION_CHECK(SYS_WB_VERSION_PACKET, wb_pkt_master.version))
    {
        ret = CTC_E_VERSION_MISMATCH;
        goto done;
    }

    /*restore  netif*/
    CTC_WB_INIT_QUERY_T((&wb_query),sys_wb_pkt_netif_t, CTC_FEATURE_PACKET, SYS_WB_APPID_PACKET_SUBID_NETIF);

    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
        sal_memcpy((uint8*)&wb_netif, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);
        entry_cnt++;

        p_netif = mem_malloc(MEM_SYSTEM_MODULE,  sizeof(ctc_pkt_netif_t));
        if (NULL == p_netif)
        {
            ret = CTC_E_NO_MEMORY;

            goto done;
        }
        sal_memset(p_netif, 0, sizeof(ctc_pkt_netif_t));

        p_netif->type = wb_netif.type;
        p_netif->vlan = wb_netif.vlan;
        p_netif->gport = wb_netif.gport;
        sal_memcpy(p_netif->mac, wb_netif.mac, sizeof(mac_addr_t));
        sal_memcpy(p_netif->name, wb_netif.name, CTC_PKT_MAX_NETIF_NAME_LEN);

        sys_usw_packet_create_netif(lchip, p_netif);
    CTC_WB_QUERY_ENTRY_END((&wb_query));

done:
    if (wb_query.key)
    {
        mem_free(wb_query.key);
    }

    if (wb_query.buffer)
    {
        mem_free(wb_query.buffer);
    }

    return ret;
}

int32
sys_usw_packet_init(uint8 lchip, void* p_global_cfg)
{
    ctc_pkt_global_cfg_t* p_pkt_cfg = (ctc_pkt_global_cfg_t*)p_global_cfg;
    mac_addr_t mac_sa = {0xFE, 0xFD, 0x0, 0x0, 0x0, 0x1};
    mac_addr_t mac_da = {0xFE, 0xFD, 0x0, 0x0, 0x0, 0x0};
    uint32 ds_nh_offset = 0;
    uint8 sub_queue_id = 0;
    int32 ret = CTC_E_NONE;

    LCHIP_CHECK(lchip);
    /* 2. allocate interrupt master */
    if (p_usw_pkt_master[lchip])
    {
        return CTC_E_NONE;
    }

    p_usw_pkt_master[lchip] = (sys_pkt_master_t*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(sys_pkt_master_t));
    if (NULL == p_usw_pkt_master[lchip])
    {
        SYS_PACKET_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;

    }

    sal_memset(p_usw_pkt_master[lchip], 0, sizeof(sys_pkt_master_t));
    sal_memcpy(&p_usw_pkt_master[lchip]->cfg, p_pkt_cfg, sizeof(ctc_pkt_global_cfg_t));

    sal_memcpy(p_usw_pkt_master[lchip]->cpu_mac_sa, mac_sa, sizeof(mac_sa));
    sal_memcpy(p_usw_pkt_master[lchip]->cpu_mac_da, mac_da, sizeof(mac_da));

#ifndef PACKET_TX_USE_SPINLOCK
    sal_mutex_create(&p_usw_pkt_master[lchip]->mutex);
#else
    sal_spinlock_create((sal_spinlock_t**)&p_usw_pkt_master[lchip]->mutex);
#endif
    if (NULL == p_usw_pkt_master[lchip]->mutex)
    {
        ret = CTC_E_NO_MEMORY;
        goto roll_back_0;
    }

    sal_mutex_create(&p_usw_pkt_master[lchip]->mutex_rx);
    if (NULL == p_usw_pkt_master[lchip]->mutex_rx)
    {
        ret = CTC_E_NO_MEMORY;
        goto roll_back_1;
    }

    p_usw_pkt_master[lchip]->pkt_buf_en[CTC_INGRESS] = 1;
    p_usw_pkt_master[lchip]->pkt_buf_en[CTC_EGRESS] = 1;
    p_usw_pkt_master[lchip]->rx_buf_reason_id = 0;

    sys_usw_get_chip_cpumac(lchip, p_usw_pkt_master[lchip]->cpu_mac_sa, p_usw_pkt_master[lchip]->cpu_mac_da);
    _sys_usw_packet_register_tx_cb(lchip, sys_usw_dma_pkt_tx);
    _sys_usw_packet_init_svlan_tpid_index(lchip);
    CTC_ERROR_GOTO(sys_usw_nh_get_resolved_offset(lchip, SYS_NH_RES_OFFSET_TYPE_BRIDGE_NH,  &ds_nh_offset), ret, roll_back_2);
    CTC_ERROR_GOTO(sys_usw_packet_tx_set_property(lchip, SYS_PKT_TX_TYPE_RSV_NH, SYS_NH_RES_OFFSET_TYPE_BRIDGE_NH, ds_nh_offset), ret, roll_back_2);
    CTC_ERROR_GOTO(sys_usw_nh_get_resolved_offset(lchip, SYS_NH_RES_OFFSET_TYPE_BYPASS_NH,  &ds_nh_offset), ret, roll_back_2);
    CTC_ERROR_GOTO(sys_usw_packet_tx_set_property(lchip, SYS_PKT_TX_TYPE_RSV_NH, SYS_NH_RES_OFFSET_TYPE_BYPASS_NH, ds_nh_offset), ret, roll_back_2);
    CTC_ERROR_GOTO(sys_usw_nh_get_resolved_offset(lchip, SYS_NH_RES_OFFSET_TYPE_MIRROR_NH,  &ds_nh_offset), ret, roll_back_2);
    CTC_ERROR_GOTO(sys_usw_packet_tx_set_property(lchip, SYS_PKT_TX_TYPE_RSV_NH, SYS_NH_RES_OFFSET_TYPE_MIRROR_NH, ds_nh_offset), ret, roll_back_2);
    CTC_ERROR_GOTO(sys_usw_nh_get_resolved_offset(lchip, SYS_NH_RES_OFFSET_TYPE_BPE_TRANSPARENT_NH,  &ds_nh_offset), ret, roll_back_2);
    CTC_ERROR_GOTO(sys_usw_packet_tx_set_property(lchip, SYS_PKT_TX_TYPE_RSV_NH, SYS_NH_RES_OFFSET_TYPE_BPE_TRANSPARENT_NH, ds_nh_offset), ret, roll_back_2);

    CTC_ERROR_GOTO(sys_usw_get_sub_queue_id_by_cpu_reason(lchip, CTC_PKT_CPU_REASON_FWD_CPU, &sub_queue_id), ret, roll_back_2);
    CTC_ERROR_GOTO(sys_usw_packet_tx_set_property(lchip, SYS_PKT_TX_TYPE_FWD_CPU_SUB_QUEUE_ID, sub_queue_id, 0), ret, roll_back_2);
    CTC_ERROR_GOTO(sys_usw_get_sub_queue_id_by_cpu_reason(lchip, CTC_PKT_CPU_REASON_C2C_PKT, &sub_queue_id), ret, roll_back_2);
    CTC_ERROR_GOTO(sys_usw_packet_tx_set_property(lchip, SYS_PKT_TX_TYPE_C2C_SUB_QUEUE_ID, sub_queue_id, 0), ret, roll_back_2);

    if (g_dal_op.handle_netif)
    {
        p_usw_pkt_master[lchip]->netif_hash = ctc_hash_create(1, SYS_PKT_NETIF_HASH_SIZE,
                                    (hash_key_fn)_sys_usw_netif_hash_make,
                                    (hash_cmp_fn)_sys_usw_netif_hash_cmp);
        if (NULL == p_usw_pkt_master[lchip]->netif_hash)
        {
            ret = CTC_E_NO_MEMORY;
            goto roll_back_2;
        }

        sys_usw_wb_sync_register_cb(lchip, CTC_FEATURE_PACKET, sys_usw_packet_wb_sync);

        if (CTC_WB_ENABLE && (CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING))
        {
            sys_usw_packet_wb_restore(lchip);
        }
    }

    /*dump db*/
    CTC_ERROR_GOTO(sys_usw_dump_db_register_cb(lchip, CTC_FEATURE_PACKET, sys_usw_packet_dump_db), ret, roll_back_2);

    return CTC_E_NONE;
roll_back_2:
    sal_mutex_destroy(p_usw_pkt_master[lchip]->mutex_rx);

roll_back_1:
#ifndef PACKET_TX_USE_SPINLOCK
    sal_mutex_destroy(p_usw_pkt_master[lchip]->mutex);
#else
    sal_spinlock_destroy((sal_spinlock_t*)p_usw_pkt_master[lchip]->mutex);
#endif

roll_back_0:
    mem_free(p_usw_pkt_master[lchip]);

    return ret;
}

static int32
_sys_usw_packet_free_netif_hash(void* node_data, void* user_data)
{
    uint8 lchip = *(uint8*)user_data;
    dal_netif_t dal_netif = {0};
    ctc_pkt_netif_t* p_del_netif = (ctc_pkt_netif_t *)node_data;

    if (!g_dal_op.handle_netif)
    {
        return CTC_E_NOT_SUPPORT;
    }

    dal_netif.op_type = DAL_OP_DESTORY;
    dal_netif.type = p_del_netif->type;
    dal_netif.gport = p_del_netif->gport;
    dal_netif.vlan = p_del_netif->vlan;

    g_dal_op.handle_netif(lchip, &dal_netif);

    mem_free(node_data);

    return CTC_E_NONE;
}

int32
sys_usw_packet_deinit(uint8 lchip)
{
    LCHIP_CHECK(lchip);
    if (NULL == p_usw_pkt_master[lchip])
    {
        return CTC_E_NONE;
    }

    if (p_usw_pkt_master[lchip]->mutex)
    {
        sal_mutex_destroy(p_usw_pkt_master[lchip]->mutex);
    }

    if (p_usw_pkt_master[lchip]->mutex_rx)
    {
        sal_mutex_destroy(p_usw_pkt_master[lchip]->mutex_rx);
    }

    if (p_usw_pkt_master[lchip]->netif_hash)
    {
        ctc_hash_traverse(p_usw_pkt_master[lchip]->netif_hash, (hash_traversal_fn) _sys_usw_packet_free_netif_hash, (void*)&lchip);
        ctc_hash_free(p_usw_pkt_master[lchip]->netif_hash);
    }

    mem_free(p_usw_pkt_master[lchip]);

    return CTC_E_NONE;
}

