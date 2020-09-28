/**
 @file ctc_usw_packet_cli.c

 @author  Copyright (C) 2012 Centec Networks Inc.  All rights reserved.

 @date 2012-11-25

 @version v2.0

 This file define packet CLI functions

*/

#include "sal.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_error.h"
#include "ctc_debug.h"
#include "ctc_packet.h"
#include "ctc_usw_packet.h"
#include "ctc_usw_nexthop.h"

extern int32
sys_usw_packet_tx_dump_enable(uint8 lchip, uint8 enable);

extern int32
sys_usw_packet_pkt_buf_en(uint8 lchip, uint8 dir,uint16 reason_id,uint8 enable);

extern int32
sys_usw_packet_stats_dump(uint8 lchip);
extern int32
sys_usw_packet_stats_clear(uint8 lchip);

extern int32
sys_usw_packet_rx_buffer_dump(uint8 lchip, uint8 start , uint8 end, uint8 flag, uint8 is_brief);
extern int32
sys_usw_packet_tx_buffer_dump(uint8 lchip, uint8 start , uint8 end, uint8 flag, uint8 is_brief);

extern int32
sys_usw_packet_buffer_clear(uint8 lchip, uint8 bitmap);
extern int32
sys_usw_packet_set_rx_reason_log(uint8 lchip, uint16 reason_id, uint8 enable, uint8 is_all, uint8 is_detail);
extern int32
sys_usw_packet_tx_discard_pkt(uint8 lchip, uint8 buf_id, ctc_pkt_tx_t* p_pkt_tx);

extern int32
_ctc_cli_packet_get_from_file(char* file, uint8* buf, uint32* pkt_len);

extern int32
sys_usw_packet_netif_show(uint8 lchip);
extern int32
sys_usw_packet_tx_performance_test(uint8 lchip, ctc_pkt_tx_t* p_pkt_tx);
CTC_CLI(ctc_cli_usw_packet_show_packet_stats,
        ctc_cli_usw_packet_show_packet_stats_cmd,
        "show packet stats (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PACKET_M_STR,
        "Stats",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CTC_E_NONE;
    uint8 lchip = 0;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_packet_stats_dump(lchip);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_packet_clear_packet_stats,
        ctc_cli_usw_packet_clear_packet_stats_cmd,
        "clear packet stats (lchip LCHIP|)",
        CTC_CLI_CLEAR_STR,
        CTC_CLI_PACKET_M_STR,
        "Stats",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CTC_E_NONE;
    uint8 lchip = 0;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_packet_stats_clear(lchip);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_usw_packet_show_packet_buffer,
        ctc_cli_usw_packet_show_packet_buffer_cmd,
        "show packet (rx|tx) {(buffer-count CNT | buffer-range START END |brief )| packet-header|}  (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PACKET_M_STR,
        "Rx",
        "Tx",
        "The buffer count",
        "Buffer count",
        "Buffer range",
        "Range start",
        "Range end",
        "Brief",
        "Packet header info",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CTC_E_NONE;
    uint8 index = 0;
    uint8 buf_cnt = 0;
    uint8 flag = 0;
    uint8 lchip = 0;
    uint8 packet_mode = 2;
    uint8 start = 0;
    uint8 end   = 0;
    uint8 is_brief =0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("rx");
    if (INDEX_VALID(index))
    {
        packet_mode = 0;
    }
    else
    {
        packet_mode = 1;

    }
    CTC_BIT_SET(flag, 0);
    start = 0;
    end  = 63;

    index = CTC_CLI_GET_ARGC_INDEX("packet-header");
    if (INDEX_VALID(index))
    {
        CTC_BIT_SET(flag, 1);
    }

    index = CTC_CLI_GET_ARGC_INDEX("buffer-count");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT8("buffer-count", buf_cnt, argv[index + 1]);
       if (buf_cnt == 0)
       {
           return CLI_ERROR;
       }
       start = 0;
       end  = buf_cnt -1;
    }


    index = CTC_CLI_GET_ARGC_INDEX("buffer-range");
    if (INDEX_VALID(index))
    {

        CTC_CLI_GET_UINT8("buffer-range", start, argv[index + 1]);
        CTC_CLI_GET_UINT8("buffer-range", end, argv[index + 2]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("brief");
    if (INDEX_VALID(index))
    {
        is_brief = 1;
    }
    else
    {
        is_brief = 0;

    }
    lchip = g_ctcs_api_en?g_api_lchip:lchip;

    if (packet_mode == 0)
    {
        ret = sys_usw_packet_rx_buffer_dump(lchip, start,end, flag, is_brief);
    }
    else if (packet_mode == 1)
    {
        ret = sys_usw_packet_tx_buffer_dump(lchip, start,end, flag, is_brief);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_packet_clear_packet_buffer,
        ctc_cli_usw_packet_clear_packet_buffer_cmd,
        "clear packet {rx|tx} (lchip LCHIP|)",
        CTC_CLI_CLEAR_STR,
        CTC_CLI_PACKET_M_STR,
        "Rx",
        "Tx",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CTC_E_NONE;
    uint8 lchip = 0;
    uint8 index = 0;
    uint8 bitmap = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;

    index = CTC_CLI_GET_ARGC_INDEX("rx");
    if (INDEX_VALID(index))
    {
        CTC_BIT_SET(bitmap, 0);
    }
    index = CTC_CLI_GET_ARGC_INDEX("tx");
    if (INDEX_VALID(index))
    {
        CTC_BIT_SET(bitmap, 1);
    }

    ret = sys_usw_packet_buffer_clear(lchip ,bitmap);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_packet_dump_packet_rx,
        ctc_cli_usw_packet_dump_packet_rx_cmd,
        "debug packet rx (on|off) (cpu-reason REASON |) (detail |) (lchip LCHIP|)",
        "Debug",
        CTC_CLI_PACKET_M_STR,
        "Rx",
        "Dump packet due to cpu-reason",
        "Debug on",
        "Debug off",
        "Cpu reason",
        "<0-max> refer to cli show qos cpu-reason",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CTC_E_NONE;
    uint16 reason_id = 0;
    uint8 index = 0;
    uint8 enable = 0;
    uint8 is_all = 1;
    uint8 is_detail = 0;
    uint8 lchip = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("cpu-reason");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT16("reason-id", reason_id, argv[index + 1]);
        is_all = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("on");
    if (INDEX_VALID(index))
    {
        enable = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("off");
    if (INDEX_VALID(index))
    {
        enable = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("detail");
    if (INDEX_VALID(index))
    {
        is_detail = 1;
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_packet_set_rx_reason_log(lchip, reason_id, enable, is_all, is_detail);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_packet_dump_packet_tx,
        ctc_cli_usw_packet_dump_packet_tx_cmd,
        "debug packet tx (on|off) (lchip LCHIP|)",
        "Debug",
        CTC_CLI_PACKET_M_STR,
        "Tx",
        "Debug on",
        "Debug off",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CTC_E_NONE;
    uint8 index = 0;
    uint8 enable = 0;
    uint8 lchip = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("on");
    if (INDEX_VALID(index))
    {
        enable = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("off");
    if (INDEX_VALID(index))
    {
        enable = 0;
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_packet_tx_dump_enable(lchip, enable);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_packet_rx_buf_enable,
        ctc_cli_usw_packet_rx_buf_enable_cmd,
        "packet rx buffer (cpu-reason REASON|enable|disable)  (lchip LCHIP|)",
        CTC_CLI_PACKET_M_STR,
        "Rx",
        "Buffer",
        "CPU reason",
        "Reason id",
        "Enable",
        "Disable",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CTC_E_NONE;
    uint8 index = 0;
    uint8 enable = 2;
    uint8 lchip = 0;
    uint16 reason_id = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("cpu-reason");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("cpu-reason", reason_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (INDEX_VALID(index))
    {
        enable = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("disable");
    if (INDEX_VALID(index))
    {
        enable = 0;
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_packet_pkt_buf_en(lchip, CTC_INGRESS,reason_id,enable);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_packet_tx_buf_enable,
        ctc_cli_usw_packet_tx_buf_enable_cmd,
        "packet tx buffer (enable|disable)  (lchip LCHIP|)",
        CTC_CLI_PACKET_M_STR,
        "Tx",
        "Buffer",
        "Enable",
        "Disable",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CTC_E_NONE;
    uint8 index = 0;
    uint8 enable = 0;
    uint8 lchip = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (INDEX_VALID(index))
    {
        enable = 1;
    }
    else
    {
        enable = 0;
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_packet_pkt_buf_en(lchip, CTC_EGRESS,0,enable);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_packet_tx_diag_discard_packet,
        ctc_cli_usw_packet_tx_diag_discard_packet_cmd,
        "packet tx diag-discard-pkt (dest-gport GPHYPORT_ID) {ingress |buffer-id ID| bypass|dma} (lchip LCHIP|)",
        CTC_CLI_PACKET_M_STR,
        "Send packet",
        "Discard packet for diagnostic",
        "Destination gport",
        CTC_CLI_GPHYPORT_ID_DESC,
        "Send to ingress port by iloop",
        "Buffer ID",
        "Buffer ID",
        "Bypass nexthop for send to egress port",
        "DMA mode")
{
    int32 ret = CTC_E_NONE;
    uint8 index = 0;
    uint8 lchip = 0;
    uint8 buf_id = 0;
    uint8 mode = 0;
    ctc_pkt_tx_t* p_pkt_tx = NULL;
    uint32 dest_gport = 0;
    uint32 flags = 0;

    mode = CTC_PKT_MODE_ETH;
    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("buffer-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("buffer-id", buf_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("dest-gport");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("dest-gport", dest_gport, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("bypass");
    if (0xFF != index)
    {
        flags |= CTC_PKT_FLAG_NH_OFFSET_BYPASS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dma");
    if (0xFF != index)
    {
        mode = CTC_PKT_MODE_DMA;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ingress");
    if (0xFF != index)
    {
        flags |= CTC_PKT_FLAG_INGRESS_MODE;
    }

    p_pkt_tx = (ctc_pkt_tx_t*)mem_malloc(MEM_CLI_MODULE, sizeof(ctc_pkt_tx_t));
    if (NULL == p_pkt_tx)
    {
        return CLI_ERROR;
    }
    sal_memset(p_pkt_tx, 0, sizeof(ctc_pkt_tx_t));
    p_pkt_tx->tx_info.oper_type = CTC_PKT_OPER_NORMAL;
    p_pkt_tx->tx_info.ttl = 64;
    p_pkt_tx->tx_info.dest_gport = dest_gport;
    p_pkt_tx->tx_info.flags = flags;
    p_pkt_tx->mode = mode;

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_packet_tx_discard_pkt(lchip, buf_id, p_pkt_tx);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        mem_free(p_pkt_tx);
        return CLI_ERROR;
    }
    mem_free(p_pkt_tx);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_packet_create_netif,
        ctc_cli_usw_packet_create_netif_cmd,
        "packet netif create NAME (port GPORT|vlan VLAN) MAC (lchip LCHIP|)",
        CTC_CLI_PACKET_M_STR,
        "Netif",
        "Create",
        "Netif name",
        "Port netif type",
        "Gport",
        "Vlan netif type",
        "Vlan",
        "Mac",
        "lchip",
        "LCHIP")
{
    int32 ret = CTC_E_NONE;
    uint8 index = 0;
    uint8 lchip = 0;
    ctc_pkt_netif_t netif = {0};

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    sal_memcpy(netif.name, argv[0], CTC_PKT_MAX_NETIF_NAME_LEN);

    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != index)
    {
        netif.type = 0;
        CTC_CLI_GET_UINT32("netif-gport", netif.gport, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("vlan");
    if (0xFF != index)
    {
        netif.type = 1;
        CTC_CLI_GET_UINT16("netif-vlan", netif.vlan, argv[index+1]);
    }

    CTC_CLI_GET_MAC_ADDRESS("mac address", netif.mac, argv[3]);

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = ctc_usw_packet_create_netif(lchip, &netif);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_packet_get_and_destory_netif,
        ctc_cli_usw_packet_get_and_destory_netif_cmd,
        "packet netif (get|destory) (port GPORT|vlan VLAN) (lchip LCHIP|)",
        CTC_CLI_PACKET_M_STR,
        "Netif",
        "Get",
        "Destory",
        "Port netif type",
        "Gport",
        "Vlan netif type",
        "Vlan",
        "lchip",
        "LCHIP")
{
    int32 ret = CTC_E_NONE;
    uint8 index = 0;
    uint8 lchip = 0;
    uint8 is_get = 0;
    ctc_pkt_netif_t netif = {0};

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("get");
    if (INDEX_VALID(index))
    {
        is_get = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != index)
    {
        netif.type = 0;
        CTC_CLI_GET_UINT32("netif-gport", netif.gport, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("vlan");
    if (0xFF != index)
    {
        netif.type = 1;
        CTC_CLI_GET_UINT16("netif-vlan", netif.vlan, argv[index+1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    if (is_get)
    {
        ret = ctc_usw_packet_get_netif(lchip, &netif);
        if (!ret)
        {
            ctc_cli_out("   Name: %s\tType: %s 0x%x\tMac: %.2x%.2x.%.2x%.2x.%.2x%.2x\n",
                netif.name, (netif.type ? "vlan   Vlan:" : "port   Gport:"), (netif.type ? netif.vlan : netif.gport), netif.mac[0], netif.mac[1], netif.mac[2], netif.mac[3], netif.mac[4], netif.mac[5]);
        }
    }
    else
    {
        ret = ctc_usw_packet_destory_netif(lchip, &netif);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_packet_show_netif,
        ctc_cli_usw_packet_show_netif_cmd,
        "show packet netif (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PACKET_M_STR,
        "Netif",
        "lchip",
        "LCHIP")
{
    int32 ret = CTC_E_NONE;
    uint8 index = 0;
    uint8 lchip = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en?g_api_lchip:lchip;

    ret = sys_usw_packet_netif_show(lchip);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

#ifndef _SAL_LINUX_KM
CTC_CLI(ctc_cli_packe_performance_test,
        ctc_cli_packe_performance_test_cmd,
        "packet performance-test dest-gport DEST_GPORT pkt-len LEN (lchip LCHIP|)",
        CTC_CLI_PACKET_M_STR,
        "Performance test",
        "Destination global port ID",
        CTC_CLI_GPORT_ID_DESC,
        "pkt length",
        "length value",
        "lchip",
        "LCHIP")
{
    uint8 pkt_buf_uc[CTC_PKT_MTU] = {
        0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x81, 0x00, 0x00, 0x0a,
        0x08, 0x00, 0x45, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x22, 0xf5, 0xc7, 0xc7,
        0xc7, 0x01, 0xc8, 0x01, 0x01, 0x01, 0xaa, 0xaa, 0xaa, 0xdd, 0xaa, 0xaa, 0xaa, 0xdd, 0xaa, 0xaa,
        0xaa, 0xdd, 0xaa, 0xaa, 0xaa, 0xdd, 0xaa, 0xaa, 0xaa, 0xdd, 0xaa, 0xaa, 0xaa, 0xdd, 0xaa, 0xaa,
        0xaa, 0xdd, 0xaa, 0xaa, 0xaa, 0xdd, 0xaa, 0xaa, 0xaa, 0xdd, 0xaa, 0xaa, 0xaa, 0xdd, 0xaa, 0xaa
    };

    ctc_pkt_tx_t pkt_tx;
    ctc_pkt_tx_t* p_pkt_tx = &pkt_tx;
    ctc_pkt_info_t* p_tx_info = &(p_pkt_tx->tx_info);
    ctc_pkt_skb_t* p_skb = &(p_pkt_tx->skb);
    uint8* p_data = NULL;
    uint32 pkt_len = 0;
    int32  ret = 0;
    uint8  lchip = 0;
    uint8  index = 0;

    sal_memset(&pkt_tx, 0, sizeof(pkt_tx));

    p_tx_info->ttl = 1;

    CTC_CLI_GET_UINT32("pkt len", pkt_len, argv[1]);
    if(pkt_len > CTC_PKT_MTU || pkt_len < 1)
    {
        ctc_cli_out("%% Invalid packet length\n");
        return CLI_ERROR;
    }
    CTC_CLI_GET_UINT32("dest-gport", p_tx_info->dest_gport, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en?g_api_lchip:lchip;

    ctc_packet_skb_init(p_skb);
    p_data = ctc_packet_skb_put(p_skb, pkt_len);
    sal_memcpy(p_data, pkt_buf_uc, pkt_len);

    ret = sys_usw_packet_tx_performance_test(lchip, p_pkt_tx);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}
#endif

int32
ctc_usw_packet_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_usw_packet_show_packet_stats_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_packet_clear_packet_stats_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_packet_show_packet_buffer_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_packet_clear_packet_buffer_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_packet_dump_packet_rx_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_packet_dump_packet_tx_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_packet_rx_buf_enable_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_packet_tx_buf_enable_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_packet_tx_diag_discard_packet_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_packet_create_netif_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_packet_get_and_destory_netif_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_packet_show_netif_cmd);
#ifndef _SAL_LINUX_KM
    install_element(CTC_SDK_MODE, &ctc_cli_packe_performance_test_cmd);
#endif
    /* debug commands */
    return CLI_SUCCESS;
}

int32
ctc_usw_packet_cli_deinit(void)
{
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_packet_show_packet_stats_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_packet_clear_packet_stats_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_packet_show_packet_buffer_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_packet_clear_packet_buffer_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_packet_dump_packet_rx_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_packet_dump_packet_tx_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_packet_rx_buf_enable_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_packet_tx_buf_enable_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_packet_tx_diag_discard_packet_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_packet_create_netif_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_packet_get_and_destory_netif_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_packet_show_netif_cmd);
#ifndef _SAL_LINUX_KM
    uninstall_element(CTC_SDK_MODE, &ctc_cli_packe_performance_test_cmd);
#endif
    /* debug commands */
    return CLI_SUCCESS;
}

