/**
 @file ctc_packet_cli.c

 @date 2012-07-09

 @version v2.0

---file comments----
*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_const.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_debug.h"
#include "ctc_error.h"
#include "ctc_packet.h"

#define CTC_CLI_PKT_ALERT_LABEL_13  13
#define CTC_CLI_PKT_PUSH_LABEL(pkt, label, exp, bos, ttl) \
do {                                        \
    uint32 shim = 0;                        \
    shim |= (((label) & 0xFFFFF) << 12);    \
    shim |= ((exp) & 0x7) << 9;             \
    shim |= ((bos) & 0x1) << 8;             \
    shim |= ((ttl) & 0xFF);                 \
    shim = htonl(shim);                     \
    sal_memcpy(pkt, &shim, 4);              \
    pkt += 4;                               \
} while(0)


uint32 _ctc_cli_pkt_file_2_str(char *packet_buf, const char *pfile_path)
{
    sal_file_t  pfile_r = NULL;
    char     tmp_buff[128] = "";
    int32    tmp_buff_idx  = 0;
    int32    tmp_buff_tok_idx  = 0;
    int32    packet_buf_len = CTC_PKT_MTU;
    pfile_r = sal_fopen(pfile_path, "r");

    if(NULL == pfile_r)
    {
        sal_printf("file not exist\n");
        return -1;
    }

    while(sal_fgets(tmp_buff,sizeof(tmp_buff),pfile_r))
    {
        tmp_buff_idx = 0;
        while(tmp_buff[tmp_buff_idx] != '\0')
        {
            if((tmp_buff[tmp_buff_idx] == ' ')
                ||(tmp_buff[tmp_buff_idx] == '\n')
                ||(tmp_buff[tmp_buff_idx] == '\r')
                ||(tmp_buff[tmp_buff_idx] == '\t'))
            {
                tmp_buff_idx++;
                continue;
            }

            if(tmp_buff_tok_idx >= packet_buf_len)
            {
                goto out;
            }

            packet_buf[tmp_buff_tok_idx] = tmp_buff[tmp_buff_idx];
            tmp_buff_tok_idx++;
            tmp_buff_idx++;
        }
        sal_memset(tmp_buff,0,sizeof(tmp_buff));
    }

    out:
    sal_fclose(pfile_r);
    return tmp_buff_tok_idx;
}

int32 _ctc_cli_pkt_file_str_2_hex(char *dest_buf_hex, uint32 src_buf_len ,char *src_buf_str)
{
    int      tmp_buff_tok_idx = 0;
    uint8    er_tmp  = 0;
    uint8    er_tmp1 = 0;

    
    while(src_buf_str[tmp_buff_tok_idx] != '\0') 
    {
        if (tmp_buff_tok_idx  >= src_buf_len)
        {
            return  CTC_E_NONE;  
        }
        if (src_buf_str[tmp_buff_tok_idx] >= '0' && src_buf_str[tmp_buff_tok_idx] <= '9')
        {
            er_tmp = src_buf_str[tmp_buff_tok_idx] - '0';
        }
        else if(src_buf_str[tmp_buff_tok_idx] >= 'A' && src_buf_str[tmp_buff_tok_idx] <= 'F')
        {
            er_tmp = src_buf_str[tmp_buff_tok_idx] - 'A' + 10;
        }
        else if(src_buf_str[tmp_buff_tok_idx] >= 'a' && src_buf_str[tmp_buff_tok_idx] <= 'f')
        {
            er_tmp = src_buf_str[tmp_buff_tok_idx] - 'a' + 10;
        }


        if (tmp_buff_tok_idx + 1 >= src_buf_len)
        {
            return  CTC_E_NONE;  
        }
        if(src_buf_str[tmp_buff_tok_idx+1] >= '0' && src_buf_str[tmp_buff_tok_idx+1] <= '9')
        {
            er_tmp1 = src_buf_str[tmp_buff_tok_idx+1] - '0';
        }
        else if(src_buf_str[tmp_buff_tok_idx+1] >= 'A' && src_buf_str[tmp_buff_tok_idx+1] <= 'F')
        {
            er_tmp1 = src_buf_str[tmp_buff_tok_idx+1] - 'A' + 10;
        }
        else if(src_buf_str[tmp_buff_tok_idx+1] >= 'a' && src_buf_str[tmp_buff_tok_idx+1] <= 'f')
        {
            er_tmp1 = src_buf_str[tmp_buff_tok_idx+1] - 'a' + 10;
        }

        dest_buf_hex[tmp_buff_tok_idx/2] = er_tmp << 4 | er_tmp1;

        tmp_buff_tok_idx += 2;
    }

    return CTC_E_NONE;
}

int32
_ctc_cli_packet_get_from_file(char* file, uint8* dest_buf_hex, uint32* pkt_len)
{
    uint8* pkt_buf_str = NULL;

    pkt_buf_str = (uint8*)mem_malloc(MEM_CLI_MODULE, CTC_PKT_MTU);
    if (NULL == pkt_buf_str)
    {
        return CTC_E_NO_MEMORY;
    }
    sal_memset(pkt_buf_str, 0 , CTC_PKT_MTU);
    
    *pkt_len = _ctc_cli_pkt_file_2_str((char*)pkt_buf_str, file);

    _ctc_cli_pkt_file_str_2_hex((char*)dest_buf_hex, *pkt_len, (char*)pkt_buf_str);
    
    *pkt_len = *pkt_len/2 + *pkt_len % 2;
    
    if (pkt_buf_str)
    {
        mem_free(pkt_buf_str);
    }
    return CTC_E_NONE;
}

STATIC void ctc_pkt_tx_cli_callback_sys(ctc_pkt_tx_t* pkt, void* user_data)
{
    /*Do nothing*/
    return;
}

STATIC void ctc_pkt_tx_cli_callback_dma(ctc_pkt_tx_t* pkt, void* user_data)
{
    if(g_ctcs_api_en)
    {
        ctcs_packet_tx_free(g_api_lchip,user_data);
    }
    else
    {
        ctc_packet_tx_free(user_data);
    }
}

CTC_CLI(ctc_cli_packet_tx,
        ctc_cli_packet_tx_cmd,
        "packet tx (mode MODE) {dest-gport DEST_GPORT (assign-dest-port LPORT|)| is-mcast dest-group-id DEST_GID | src-svid SRC_SVID| src-cvid SRC_CVID | src-port SRC_GPORT | oper-type OPER_TYPE | priority PRIORITY | color COLOR | src-cos SRC_COS |\
        ttl TTL | is-critical |(nhid NHID | nh-offset NH_OFFSET (nh-offset-is-8w |)|bypass) |working-path | protection-path | linkagg-hash HASH | vlan-domain VLAN-DOMAIN | fid FID | svlan-tpid SVLAN_TPID | isolated-id ISOLATED_ID |\
        oam type TYPE {use-fid |link-oam | is-dm | tx-mip-tunnel| is-up-mep| has-mpls-gal| has-mpls-cw| dm-ts-offset DM_TS_OFFSET | vid VID |} |\
        ptp oper OPER seconds SECONDS nano-seconds NANO_SECONDS {seq-id SEQ_ID| ptp-ts-offset PTP-TS-OFFSET} | ingress-mode |cancel-dot1ae| async |(zero-copy| use-skb-buf|) | logic-port-type}\
        (pkt-sample (ucast| mcast| eth-lbm|eth-dmm|eth-lmm) |pkt-file FILENAME){pkt-len LEN|session SESSION (pending|)|}(is-tx-array|)(count COUNT |)",
        CTC_CLI_PACKET_M_STR,
        "Send packet",
        "Packet Transport mode",
        "0 DMA, 1 Ethernet",
        "Destination global port ID",
        CTC_CLI_GPORT_ID_DESC,
        "Cpu assign the out port for stacking",
        "Assign local dest port",
        "Is multicast packet",
        CTC_CLI_GLOBAL_MCASTGRP_ID_DESC,
        "Group ID value",
        "Source S-VLAN",
        CTC_CLI_VLAN_RANGE_DESC,
        "Source C-VLAN",
        CTC_CLI_VLAN_RANGE_DESC,
        "Source port",
        CTC_CLI_GPORT_ID_WITHOUT_LINKAGG_DESC,
        "Operation type",
        "Refer to ctc_pkt_oper_type_t",
        "Priority of the packet",
        "Priority value",
        "Color of the packet",
        "Refer to ctc_qos_color_t",
        "COS of the packet",
        "Range is [0, 7]",
        "TTL of the packet",
        "Valur of ttl",
        "Packet should not be droped in queue",
        "Nexthop id",
        CTC_CLI_NH_ID_STR,
        CTC_CLI_NH_DSNH_OFFSET_STR,
        CTC_CLI_NH_DSNH_OFFSET_VALUE_STR,
        "Nexthop is 8w",
        "Nexthop is bypass",
        "Use aps working path",
        "Use aps protection path",
        "Hash of LAG for load balance",
        "Hash Value",
        "Vlan domain",
        "Refer to ctc_port_vlan_domain_type_t",
        "Forwarding Id",
        CTC_CLI_FID_ID_DESC,
        "Svlan tpid",
        "Tpid value",
        "Port isolated id",
        CTC_CLI_PORT_ISOLATION_ID_DESC,
        "OAM packet",
        "OAM type",
        "refer to ctc_oam_type_t",
        "Oam Using fid lookup",
        "Indicate is link-level",
        "Indicate is DM",
        "Indicate bypass MIP configured port when TX",
        "Indicate is UP MEP",
        "Indicate has GAL",
        "Indicate has CW",
        "Offset in bytes of timestamp",
        "Offset value",
        "VLAN ID, valid for UP MEP",
        CTC_CLI_VLAN_RANGE_DESC,
        "PTP packet",
        "PTP operation type",
        "refer to ctc_ptp_oper_type_t",
        "Time stamp second",
        "Second value",
        "Time stamp nano second",
        "Nano second value",
        "Capture sequence Id",
        "Sequence Id value",
        "Timestamp offset",
        "Offset value",
        "Ingress mode",
        "Pakcet won't be encrypted",
        "Transmit pkt asynchronously",
        "zero copy mode to tx",
        "pkt data is in skb.buf",
        "Logic-port-type",
        "Pkt sample",
        "unicast sample",
        "multicast sample",
        "ethernet oam lbm sample",
        "dmm",
        "lmm",
        "File store packet",
        "File name",
        "pkt length",
        "length value",
        "Session",
        "Session ID",
        "Session pending",
        "Pkt count",
        "Is tx array",
        "count value")
{
    uint8* pkt_buf = NULL;
    static uint8 pkt_buf_tmp[CTC_PKT_MTU] = {0};
    static uint8 pkt_buf_uc[CTC_PKT_MTU] = {
        0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x81, 0x00, 0x00, 0x0a,
        0x08, 0x00, 0x45, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x22, 0xf5, 0xc7, 0xc7,
        0xc7, 0x01, 0xc8, 0x01, 0x01, 0x01, 0xaa, 0xaa, 0xaa, 0xdd, 0xaa, 0xaa, 0xaa, 0xdd, 0xaa, 0xaa,
        0xaa, 0xdd, 0xaa, 0xaa, 0xaa, 0xdd, 0xaa, 0xaa, 0xaa, 0xdd, 0xaa, 0xaa, 0xaa, 0xdd, 0xaa, 0xaa,
        0xaa, 0xdd, 0xaa, 0xaa, 0xaa, 0xdd, 0xaa, 0xaa, 0xaa, 0xdd, 0xaa, 0xaa, 0xaa, 0xdd, 0xaa, 0xaa
    };
    static uint8 pkt_buf_mc[CTC_PKT_MTU] = {
        0x01, 0x00, 0x5E, 0x7F, 0x00, 0x01, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x81, 0x00, 0x00, 0x0a,
        0x08, 0x00, 0x45, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x22, 0xF5, 0xC7, 0xC7,
        0xC7, 0x01, 0xC8, 0x01, 0x01, 0x01, 0xAA, 0xAA, 0xAA, 0xDD, 0xAA, 0xAA, 0xAA, 0xDD, 0xAA, 0xAA,
        0xAA, 0xDD, 0xAA, 0xAA, 0xAA, 0xDD, 0xAA, 0xAA, 0xAA, 0xDD, 0xAA, 0xAA, 0xAA, 0xDD, 0xAA, 0xAA,
        0xAA, 0xDD, 0xAA, 0xAA, 0xAA, 0xDD, 0xAA, 0xAA, 0xAA, 0xDD, 0xAA, 0xAA, 0xAA, 0xDD, 0xAA, 0xAA
    };
    static uint8 pkt_buf_cfm_lbm[CTC_PKT_MTU] = {
        0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x81, 0x00, 0x00, 0x0a,
        0x89, 0x02, 0xa0, 0x03, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    static uint8 dmm_pkthd[64] =
    {
        0x01, 0x80, 0xC2, 0x00, 0x00, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x81, 0x00, 0x00, 0x02,
        0x89, 0x02, 0x61, 0x2f, 0x00, 0x20, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
    static uint8 lmm_pkthd[64] =
    {
        0x01, 0x80, 0xC2, 0x00, 0x00, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x81, 0x00, 0x00, 0x02,
        0x89, 0x02, 0x60, 0x2b, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };

    static char file[256] = {0};
    static ctc_pkt_tx_t pkt_tx;
    ctc_pkt_tx_t* p_pkt_tx = &pkt_tx;
    ctc_pkt_info_t* p_tx_info = &(p_pkt_tx->tx_info);
    ctc_pkt_skb_t* p_skb = &(p_pkt_tx->skb);
    uint8* p_data = NULL;
    uint32 pkt_len = 0;
    int32 idx = 0;
    int32 ret = 0;
    uint8* p_user_data = NULL;
    uint8 is_async = 0;
    uint8 is_zero_copy = 0;
    uint8 is_buf_valid = 0;
    uint32 count = 1;
    uint32 cnt = 0;
    uint32 ptp_oper = 0;
    int32 mem_type = -1;
    uint8 is_tx_array = 0;
    ctc_pkt_tx_t** p_pkt_tx_array = NULL;
    ctc_pkt_tx_t* pkt_tx_array = NULL;
    uint32 i = 0;

    sal_memset(&pkt_tx, 0, sizeof(pkt_tx));

    p_tx_info->ttl = 1;

    idx = CTC_CLI_GET_ARGC_INDEX("mode");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT8("mode", pkt_tx.mode, argv[idx + 1])
         /*pkt_tx.mode = (mode == 1 ? CTC_PKT_MODE_ETH : CTC_PKT_MODE_DMA)*/
    }
    idx = CTC_CLI_GET_ARGC_INDEX("dest-gport");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT32("dest-gport", p_tx_info->dest_gport, argv[idx + 1]);
    }

    idx = CTC_CLI_GET_ARGC_INDEX("assign-dest-port");
    if (0xFF != idx)
    {
        p_tx_info->flags |= CTC_PKT_FLAG_ASSIGN_DEST_PORT;
        CTC_CLI_GET_UINT16("assign-port", p_tx_info->lport, argv[idx + 1]);
    }

    idx = CTC_CLI_GET_ARGC_INDEX("is-mcast");
    if (0xFF != idx)
    {
        p_tx_info->flags |= CTC_PKT_FLAG_MCAST;

        idx = CTC_CLI_GET_ARGC_INDEX("dest-group-id");
        if (0xFF != idx)
        {
             CTC_CLI_GET_UINT16("dest-group-id", p_tx_info->dest_group_id, argv[idx + 1]);
        }
    }

    idx = CTC_CLI_GET_ARGC_INDEX("src-svid");
    if (0xFF != idx)
    {
        p_tx_info->flags |= CTC_PKT_FLAG_SRC_SVID_VALID;
        CTC_CLI_GET_UINT16("src-svid", p_tx_info->src_svid, argv[idx + 1]);
    }
    idx = CTC_CLI_GET_ARGC_INDEX("src-cvid");
    if (0xFF != idx)
    {
        p_tx_info->flags |= CTC_PKT_FLAG_SRC_CVID_VALID;
        CTC_CLI_GET_UINT16("src-cvid", p_tx_info->src_cvid, argv[idx + 1]);
    }
    idx = CTC_CLI_GET_ARGC_INDEX("src-port");
    if (0xFF != idx)
    {
        p_tx_info->flags |= CTC_PKT_FLAG_SRC_PORT_VALID;
        CTC_CLI_GET_UINT32("src-port", p_tx_info->src_port, argv[idx + 1]);
    }
    idx = CTC_CLI_GET_ARGC_INDEX("oper-type");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT8("oper-type", p_tx_info->oper_type, argv[idx + 1]);
    }
    idx = CTC_CLI_GET_ARGC_INDEX("priority");
    if (0xFF != idx)
    {
        p_tx_info->flags |= CTC_PKT_FLAG_PRIORITY;
        CTC_CLI_GET_UINT8("priority", p_tx_info->priority, argv[idx + 1]);
    }
    idx = CTC_CLI_GET_ARGC_INDEX("color");
    if (0xFF != idx)
    {
        p_tx_info->flags |= CTC_PKT_FLAG_COLOR;
        CTC_CLI_GET_UINT8("color", p_tx_info->color, argv[idx + 1]);
    }
    idx = CTC_CLI_GET_ARGC_INDEX("src-cos");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT8("src-cos", p_tx_info->src_cos, argv[idx + 1]);
    }
    idx = CTC_CLI_GET_ARGC_INDEX("ttl");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT8("ttl", p_tx_info->ttl, argv[idx + 1]);
    }
    idx = CTC_CLI_GET_ARGC_INDEX("is-critical");
    if (0xFF != idx)
    {
        p_tx_info->is_critical = 1;
    }
    idx = CTC_CLI_GET_ARGC_INDEX("nhid");
    if (0xFF != idx)
    {   p_tx_info->flags |= CTC_PKT_FLAG_NHID_VALID;
        CTC_CLI_GET_UINT32("nhid", p_tx_info->nhid, argv[idx + 1]);
    }
    idx = CTC_CLI_GET_ARGC_INDEX("nh-offset");
    if (0xFF != idx)
    {
        p_tx_info->flags |= CTC_PKT_FLAG_NH_OFFSET_VALID;
        CTC_CLI_GET_UINT32("nh-offset", p_tx_info->nh_offset, argv[idx + 1]);
    }
    idx = CTC_CLI_GET_ARGC_INDEX("nh-offset-is-8w");
    if (0xFF != idx)
    {
        p_tx_info->flags |= CTC_PKT_FLAG_NH_OFFSET_IS_8W;
    }
    idx = CTC_CLI_GET_ARGC_INDEX("bypass");
    if (0xFF != idx)
    {
        p_tx_info->flags |= CTC_PKT_FLAG_NH_OFFSET_BYPASS;
    }
    idx = CTC_CLI_GET_ARGC_INDEX("working-path");
    if (0xFF != idx)
    {
        p_tx_info->flags |= CTC_PKT_FLAG_APS_PATH_EN;
        p_tx_info->aps_p_path = 0;
    }
    idx = CTC_CLI_GET_ARGC_INDEX("protection-path");
    if (0xFF != idx)
    {
        p_tx_info->flags |= CTC_PKT_FLAG_APS_PATH_EN;
        p_tx_info->aps_p_path = 1;
    }
    idx = CTC_CLI_GET_ARGC_INDEX("linkagg-hash");
    if (0xFF != idx)
    {
        p_tx_info->flags |= CTC_PKT_FLAG_HASH_VALID;
        CTC_CLI_GET_UINT8("linkagg-hash", p_tx_info->hash, argv[idx + 1]);
    }
    idx = CTC_CLI_GET_ARGC_INDEX("vlan-domain");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT8("vlan-domain", p_tx_info->vlan_domain, argv[idx + 1]);
    }
    idx = CTC_CLI_GET_ARGC_INDEX("fid");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT16("fid", p_tx_info->fid, argv[idx + 1]);
    }
    idx = CTC_CLI_GET_ARGC_INDEX("svlan-tpid");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT16("svlan-tpid", p_tx_info->svlan_tpid, argv[idx + 1]);
    }
    idx = CTC_CLI_GET_ARGC_INDEX("isolated-id");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT16("isolated-id", p_tx_info->isolated_id, argv[idx + 1]);
    }
    idx = CTC_CLI_GET_ARGC_INDEX("oam");
    if (0xFF != idx)
    {
        idx = CTC_CLI_GET_ARGC_INDEX("type");
        if (0xFF != idx)
        {
            CTC_CLI_GET_UINT32("type", p_tx_info->oam.type, argv[idx + 1]);
        }
        idx = CTC_CLI_GET_ARGC_INDEX("use-fid");
        if (0xFF != idx)
        {
            p_tx_info->flags |= CTC_PKT_FLAG_OAM_USE_FID;
        }
        idx = CTC_CLI_GET_ARGC_INDEX("link-oam");
        if (0xFF != idx)
        {
            p_tx_info->oam.flags |= CTC_PKT_OAM_FLAG_LINK_OAM;
        }
        idx = CTC_CLI_GET_ARGC_INDEX("is-dm");
        if (0xFF != idx)
        {
            p_tx_info->oam.flags |= CTC_PKT_OAM_FLAG_IS_DM;
        }
        idx = CTC_CLI_GET_ARGC_INDEX("tx-mip-tunnel");
        if (0xFF != idx)
        {
            p_tx_info->oam.flags |= CTC_PKT_OAM_FLAG_TX_MIP_TUNNEL;
        }
        idx = CTC_CLI_GET_ARGC_INDEX("is-up-mep");
        if (0xFF != idx)
        {
            p_tx_info->oam.flags |= CTC_PKT_OAM_FLAG_IS_UP_MEP;
        }
        idx = CTC_CLI_GET_ARGC_INDEX("has-mpls-gal");
        if (0xFF != idx)
        {
            p_tx_info->oam.flags |= CTC_PKT_OAM_FLAG_HAS_MPLS_GAL;
        }
        idx = CTC_CLI_GET_ARGC_INDEX("has-mpls-cw");
        if (0xFF != idx)
        {
            p_tx_info->oam.flags |= CTC_PKT_OAM_FLAG_HAS_MPLS_CW;
        }
        idx = CTC_CLI_GET_ARGC_INDEX("dm-ts-offset");
        if (0xFF != idx)
        {
            CTC_CLI_GET_UINT32("dm-ts-offset", p_tx_info->oam.dm_ts_offset, argv[idx + 1]);
        }
        idx = CTC_CLI_GET_ARGC_INDEX("vid");
        if (0xFF != idx)
        {
            CTC_CLI_GET_UINT16("vid", p_tx_info->oam.vid, argv[idx + 1]);
        }
    }
    idx = CTC_CLI_GET_ARGC_INDEX("ptp");
    if (0xFF != idx)
    {
        idx = CTC_CLI_GET_ARGC_INDEX("oper");
        if (0xFF != idx)
        {
            CTC_CLI_GET_UINT16("oper", ptp_oper, argv[idx + 1]);
            p_tx_info->ptp.oper = (ctc_ptp_oper_type_t)ptp_oper;
        }
        idx = CTC_CLI_GET_ARGC_INDEX("seconds");
        if (0xFF != idx)
        {
            CTC_CLI_GET_UINT32("seconds", p_tx_info->ptp.ts.seconds, argv[idx + 1]);
        }
        idx = CTC_CLI_GET_ARGC_INDEX("nano-seconds");
        if (0xFF != idx)
        {
            CTC_CLI_GET_UINT32("nano-seconds", p_tx_info->ptp.ts.nanoseconds, argv[idx + 1]);
        }
        idx = CTC_CLI_GET_ARGC_INDEX("seq-id");
        if (0xFF != idx)
        {
            CTC_CLI_GET_UINT8("seq-id", p_tx_info->ptp.seq_id, argv[idx + 1]);
        }
        idx = CTC_CLI_GET_ARGC_INDEX("ptp-ts-offset");
        if (0xFF != idx)
        {
            CTC_CLI_GET_UINT8("ptp-ts-offset", p_tx_info->ptp.ts_offset, argv[idx + 1]);
        }
    }
    idx = CTC_CLI_GET_ARGC_INDEX("ingress-mode");
    if (0xFF != idx)
    {
        p_tx_info->flags |= CTC_PKT_FLAG_INGRESS_MODE;
    }
    idx = CTC_CLI_GET_ARGC_INDEX("cancel-dot1ae");
    if (0xFF != idx)
    {
        p_tx_info->flags |= CTC_PKT_FLAG_CANCEL_DOT1AE;
    }
    idx = CTC_CLI_GET_ARGC_INDEX("async");
    if (0xFF != idx)
    {
        is_async = 1;
    }

    idx = CTC_CLI_GET_ARGC_INDEX("zero-copy");
    if (0xFF != idx)
    {
        is_zero_copy= 1;
        if (pkt_tx.mode == CTC_PKT_MODE_ETH)
        {
            return CLI_ERROR;
        }
    }

    idx = CTC_CLI_GET_ARGC_INDEX("use-skb-buf");
    if (0xFF != idx)
    {
        is_buf_valid= 1;
    }

    idx = CTC_CLI_GET_ARGC_INDEX("logic-port-type");
    if (0xFF != idx)
    {
        p_tx_info->logic_port_type = 1;
    }

    idx = CTC_CLI_GET_ARGC_INDEX("session");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT16("session", p_tx_info->session_id, argv[idx + 1]);
        CTC_SET_FLAG(p_tx_info->flags, CTC_PKT_FLAG_SESSION_ID_EN);
        dmm_pkthd[15] = (p_tx_info->session_id + 2)&0xFF;
        dmm_pkthd[14] = ((p_tx_info->session_id + 2) >> 8)&0xFF;
        lmm_pkthd[15] = (p_tx_info->session_id + 2)&0xFF;
        lmm_pkthd[14] = ((p_tx_info->session_id + 2) >> 8)&0xFF;

        idx = CTC_CLI_GET_ARGC_INDEX("pending");
        if (0xFF != idx)
        {
            p_tx_info->flags |= CTC_PKT_FLAG_SESSION_PENDING_EN;
        }
    }

    idx = CTC_CLI_GET_ARGC_INDEX("pkt-file");
    if (0xFF != idx)
    {
        sal_strcpy((char*)file, argv[idx + 1]);
        /* get packet from file */
        ret = _ctc_cli_packet_get_from_file(file, pkt_buf_tmp, &pkt_len);
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
        pkt_buf = pkt_buf_tmp;
    }

    idx = CTC_CLI_GET_ARGC_INDEX("pkt-sample");
    if (0xFF != idx)
    {
        /* packet type */
        if (sal_strncmp(argv[idx + 1], "ucast", sal_strlen("ucast"))== 0)
        {
            pkt_buf = pkt_buf_uc;
            pkt_len = 80;
        }
        else if (sal_strncmp(argv[idx + 1], "mcast", sal_strlen("mcast")) == 0)
        {
            pkt_buf = pkt_buf_mc;
            pkt_len = 80;
        }
        else if (sal_strncmp(argv[idx + 1], "eth-lbm", sal_strlen("eth-lbm")) == 0)
        {
            pkt_buf = pkt_buf_cfm_lbm;
            pkt_len = 60;
        }
        else if (sal_strncmp(argv[idx + 1], "eth-dmm", sal_strlen("eth-dmm")) == 0)
        {
            pkt_buf = dmm_pkthd;
            pkt_len = 60;
        }
        else if (sal_strncmp(argv[idx + 1], "eth-lmm", sal_strlen("eth-lmm")) == 0)
        {
            pkt_buf = lmm_pkthd;
            pkt_len = 60;
        }
    }

    idx = CTC_CLI_GET_ARGC_INDEX("pkt-len");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT32("pkt len", pkt_len, argv[idx + 1]);
        if(pkt_len > CTC_PKT_MTU || pkt_len < 1)
        {
            ctc_cli_out("%% Invalid packet length\n");
            return CLI_ERROR;
        }
    }
    idx = CTC_CLI_GET_ARGC_INDEX("is-tx-array");
    if (0xFF != idx)
    {
        is_tx_array = 1;
    }
    idx = CTC_CLI_GET_ARGC_INDEX("count");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT32("count", count, argv[idx + 1]);
    }
    /* send packet by encap info */
    while(cnt < count)
    {
        if(is_zero_copy)
        {
            if(g_ctcs_api_en)
            {
                ret = ctcs_packet_tx_alloc(g_api_lchip,pkt_len, (void**)&p_user_data);
            }
            else
            {
                ret = ctc_packet_tx_alloc(pkt_len, (void**)&p_user_data);
            }
            if (ret < 0)
            {
                if(CTC_E_NO_MEMORY == ret)
                {
                    ctc_cli_out("%% ctc_packet_tx_alloc,ret = %d, %s\n", ret, ctc_get_error_desc(ret));
                    cnt++;
                    continue;
                }
                else
                {
                    ctc_cli_out("%% ctc_packet_tx_alloc,ret = %d, %s\n", ret, ctc_get_error_desc(ret));
                    return CLI_ERROR;
                }
            }
            sal_memcpy(p_user_data, pkt_buf, pkt_len);
            p_skb->data = p_user_data;
            p_skb->len= pkt_len;
            mem_type = 0;
        }
        else if(is_buf_valid)
        {
            ctc_packet_skb_init(p_skb);
            p_data = ctc_packet_skb_put(p_skb, pkt_len);
            sal_memcpy(p_data, pkt_buf, pkt_len);
            mem_type = 2;
        }
        else
        {
             p_skb->data = pkt_buf;
             p_skb->len= pkt_len;
             mem_type = 1;
        }

        if (is_async)
        {
            if(0 == mem_type)
            {
                p_pkt_tx->callback = ctc_pkt_tx_cli_callback_dma;
                p_pkt_tx->user_data = (void*)p_user_data;
            }
            else if (1 == mem_type)
            {
                p_pkt_tx->callback = ctc_pkt_tx_cli_callback_sys;
                p_pkt_tx->user_data = (void*)NULL;
            }
            else if (2 == mem_type)
            {
                p_pkt_tx->callback = ctc_pkt_tx_cli_callback_sys;
                p_pkt_tx->user_data = (void*)NULL;
            }
        }

        if (is_tx_array)
        {
            p_pkt_tx_array =  (ctc_pkt_tx_t**)mem_malloc(MEM_CLI_MODULE, count*sizeof(ctc_pkt_tx_t**));
            if (p_pkt_tx_array == NULL)
            {
                return CLI_ERROR;
            }
            pkt_tx_array =  (ctc_pkt_tx_t*)mem_malloc(MEM_CLI_MODULE, count*sizeof(ctc_pkt_tx_t));
            if (pkt_tx_array == NULL)
            {
                mem_free(p_pkt_tx_array);
                return CLI_ERROR;
            }
            for (i = 0; i < count ; i ++)
            {
                sal_memcpy(pkt_tx_array + i, p_pkt_tx, sizeof(ctc_pkt_tx_t));
                p_pkt_tx_array[i] = &pkt_tx_array[i];
            }

        }

        if(g_ctcs_api_en)
        {
             if (!is_tx_array)
             {
                ret = ctcs_packet_tx(g_api_lchip, p_pkt_tx);
             }
             else
             {
                ret = ctcs_packet_tx_array(g_api_lchip, p_pkt_tx_array, count, ctc_pkt_tx_cli_callback_sys, NULL);
             }
        }
        else
        {
             if (!is_tx_array)
             {
                ret = ctc_packet_tx(p_pkt_tx);
             }
             else
             {
                ret = ctc_packet_tx_array(p_pkt_tx_array, count, ctc_pkt_tx_cli_callback_sys, NULL);
             }
        }
        if (ret < 0)
        {
            if(CTC_E_NO_MEMORY == ret)
            {
                ctc_cli_out("%% ctc_packet_tx,ret = %d, %s\n", ret, ctc_get_error_desc(ret));
                if (mem_type == 0)
                {
                    if(g_ctcs_api_en)
                    {
                        ctcs_packet_tx_free(g_api_lchip,(void*)p_user_data);
                    }
                    else
                    {
                        ctc_packet_tx_free((void*)p_user_data);
                    }
                }

                if (!is_tx_array)
                {
                    cnt++;
                    continue;
                }
                else
                {
                    if (p_pkt_tx_array)
                    {
                        mem_free(p_pkt_tx_array);
                    }
                    if (pkt_tx_array)
                    {
                        mem_free(pkt_tx_array);
                    }
                    return CLI_SUCCESS;
                }
            }
            else
            {
                ctc_cli_out("%% ctc_packet_tx,ret = %d, %s, count: %5d\n", ret, ctc_get_error_desc(ret),cnt);
                return CLI_ERROR;
            }
        }
        if (!is_tx_array)
        {
            cnt++;
        }
        else
        {
            if (p_pkt_tx_array)
            {
                mem_free(p_pkt_tx_array);
            }
            if (pkt_tx_array)
            {
                mem_free(pkt_tx_array);
            }
            return CLI_SUCCESS;
        }
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_packet_timer_cfg,
        ctc_cli_packet_timer_cfg_cmd,
        "packet tx timer session-num NUM interval INTV pkt-len LEN (pending|start|stop|destroy|) (lchip LCHIP|)",
        CTC_CLI_PACKET_M_STR,
        "Tx",
        "Timer",
        "Session number",
        "Number",
        "Tx interval",
        "Interval",
        "Packet length",
        "Length",
        "Timer pending",
        "Timer start",
        "Timer stop",
        "Timer destroy",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CTC_E_NONE;
    uint8 index = 0;
    ctc_pkt_tx_timer_t tx_timer;
    uint8 lchip = 0;

    sal_memset(&tx_timer, 0, sizeof(ctc_pkt_tx_timer_t));

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    CTC_CLI_GET_UINT16_RANGE("session num", tx_timer.session_num, argv[0], 0, CTC_MAX_UINT16_VALUE);
    CTC_CLI_GET_UINT16_RANGE("interval", tx_timer.interval, argv[1], 0, CTC_MAX_UINT16_VALUE);
    CTC_CLI_GET_UINT16_RANGE("length", tx_timer.pkt_len, argv[2], 0, CTC_MAX_UINT16_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("stop");
    if (INDEX_VALID(index))
    {
        tx_timer.timer_state = 2;
    }

    index = CTC_CLI_GET_ARGC_INDEX("stop");
    if (INDEX_VALID(index))
    {
        tx_timer.timer_state = 2;
    }

    index = CTC_CLI_GET_ARGC_INDEX("pending");
    if (INDEX_VALID(index))
    {
        tx_timer.timer_state = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("start");
    if (INDEX_VALID(index))
    {
        tx_timer.timer_state = 1;
    }
    index = CTC_CLI_GET_ARGC_INDEX("destroy");
    if (INDEX_VALID(index))
    {
        tx_timer.timer_state = 3;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_packet_set_tx_timer(g_api_lchip,&tx_timer);
    }
    else
    {
        ret = ctc_packet_set_tx_timer(lchip, &tx_timer);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_pkt_debug_on,
        ctc_cli_pkt_debug_on_cmd,
        "debug packet (ctc|sys) (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        CTC_CLI_PACKET_M_STR,
        "Ctc layer",
        "Sys layer",
        CTC_CLI_DEBUG_LEVEL_STR,
        CTC_CLI_DEBUG_LEVEL_FUNC,
        CTC_CLI_DEBUG_LEVEL_PARAM,
        CTC_CLI_DEBUG_LEVEL_INFO,
        CTC_CLI_DEBUG_LEVEL_ERROR)
{

    uint32 typeenum = 0;
    uint8 level = CTC_DEBUG_LEVEL_INFO | CTC_DEBUG_LEVEL_FUNC | CTC_DEBUG_LEVEL_PARAM | CTC_DEBUG_LEVEL_ERROR;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("debug-level");
    if (index != 0xFF)
    {
        level = CTC_DEBUG_LEVEL_NONE;
        index = CTC_CLI_GET_ARGC_INDEX("func");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_FUNC;
        }

        index = CTC_CLI_GET_ARGC_INDEX("param");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_PARAM;
        }

        index = CTC_CLI_GET_ARGC_INDEX("info");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_INFO;
        }

        index = CTC_CLI_GET_ARGC_INDEX("error");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_ERROR;
        }
    }

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = PACKET_CTC;

    }
    else
    {
        typeenum = PACKET_SYS;
    }

    ctc_debug_set_flag("packet", "packet", typeenum, level, TRUE);
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_pkt_debug_off,
        ctc_cli_pkt_debug_off_cmd,
        "no debug packet (ctc | sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_PACKET_M_STR,
        "Ctc layer",
        "Sys layer")
{
    int32 ret = CLI_SUCCESS;
    uint32 typeenum = 0;
    uint8 level = 0;

    if (CLI_CLI_STR_EQUAL("ctc", 0))
    {
        typeenum = PACKET_CTC;
    }
    else if (CLI_CLI_STR_EQUAL("sys", 0))
    {
        typeenum = PACKET_SYS;
    }

    ret = ctc_debug_set_flag("packet", "packet", typeenum, level, FALSE);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

int32
ctc_packet_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_packet_tx_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_packet_timer_cfg_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_pkt_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_pkt_debug_off_cmd);
    return 0;
}
