/**
 @file ctc_diag_cli.c

 @date 2019-04-01

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
#include "ctc_diag_cli.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_diag.h"
#include "ctc_error.h"
#include "ctc_debug.h"


#define CTC_CLI_DIAG_TRACE_NET_KEY\
    "{l2-type L2-TYPE |l3-type L3_TYPE|l4-type L4_TYPE|l4-user-type L4_USER_TYPE|eth-type ETH_TYPE|mac-sa MAC_SA|mac-da MAC_DA|svlan SVLAN|cvlan CVLAN| \
    vlan-num VLAN_NUM|ip-sa IP_SA |ip-da IP_DA |ipv6-sa IP6_SA |ipv6-da IP6_DA |ipv6-flow-label IP6_LABEL|ip-protocol IP_PROTO|ip-ttl TTL|arp-op-code ARP_CODE| \
    arp-protocol-type ARP_TYPE|arp-sender-ip ARP_SIP|arp-target-ip ARP_TIP|arp-sender-mac SENDER_MAC|arp-target-mac TARGET_MAC|eth-oam-level OAM_LVL|eth-oam-op-code OAM_OP_CODE| \
    eth-oam-version OAM_VER|fcoe-dst-fcid FCOE_DST_FCID|fcoe-src-fcid FCOE_SRC_FCID|mpls-label0 LABEL0|mpls-label1 LABEL1|mpls-label2 LABEL2|slow-protocol-code SLOW_CODE| \
    slow-protocol-flags SLOW_FLAGS|slow-protocol-sub-type SLOW_SUB_TYPE|trill-igs-nickname IGS_NICKNAME|trill-egs-nickname EGS_NICKNAME|trill-inner-vlan TRILL_VLAN| \
    trill-version TRILL_VERSION|l4-dst-port L4_DPORT|l4-src-port L4_SPORT|tcp-ecn TCP_ECN|tcp-flags TCP_FLAG|vxlan-vn-id VN_ID|vxlan-flags VXLAN_FLAG|vxlan-rsv1 VXLAN_RSV1| \
    vxlan-rsv2 VXLAN_RSV2|gre-key GRE_KEY|gre-flags GRE_FLAG|gre-protocol GRE_PRO} "

#define CTC_CLI_DIAG_TRACE_NET_KEY_DESC\
    "L2 Type", "L2 Type Value","L3 Type", "L3 Type Value", "L4 Type", "L4 Type Value", "L4 User Type", "L4 User Type Value", "Ether Type", "Ether Type Value","Surce MAC Address", CTC_CLI_MAC_FORMAT, \
    "Destination MAC Address", CTC_CLI_MAC_FORMAT, "Svlan Id", "Svlan Id Value", "Cvlan Id", "Cvlan Id Value", "Vlan Number", "Vlan Number Value", "Source IP Address", CTC_CLI_IPV4_FORMAT, \
     "Destination IP Address", CTC_CLI_IPV4_FORMAT,"Source IPv6 Address", CTC_CLI_IPV6_FORMAT, \
     "Destination IPv6 Address", CTC_CLI_IPV6_FORMAT, "IPv6 Flow Label", "IPv6 Flow Label Value", "IP Protocol", "IP Protocol Value",\
     "IP TTL", "IP TTL Value", "ARP Opcode","ARP Opcode Value", "ARP Protocol Type", "ARP Protocol Type Value", "ARP Sender IP", CTC_CLI_IPV4_FORMAT, "ARP Target IP", CTC_CLI_IPV4_FORMAT, \
     "ARP Sender MAC", CTC_CLI_MAC_FORMAT, "ARP Target MAC", CTC_CLI_MAC_FORMAT, "Ether OAM Level", "Ether OAM Level Value", "Ether OAM Opcode", "Ether OAM Opcode Value",\
     "Ether OAM Version", "Ether OAM Version Value","Fcoe Dest Fcid", "Fcoe Dest Fcid Value", "Fcoe Source Fcid", "Fcoe Source Fcid Value", "MPLS Label 0", "MPLS Label 0 Value", \
     "MPLS Label 1", "MPLS Label 1 Value","MPLS Label 2", "MPLS Label 2 Value", "Slow Protocol Code", "Slow Protocol Code Value", "Slow Protocol Flags", "Slow Protocol Flags Value", \
     "Slow Protocol Sub Type", "Slow Protocol Sub Type Value", "Trill Ingress Nickname", "Trill Ingress Nickname Value", "Trill Egress Nickname", "Trill Egress Nickname Value",\
     "Trill Inner Vlan", "Trill Inner Vlan Id", "Trill Version", "Trill Version Value", "L4 Destination Port","L4 Destination Port Value", "L4 Source Port", "L4 Source Port Value", \
     "TCP Ecn", "TCP Ecn Value", "TCP Flags", "TCP Flags Value", "Vxlan Network Id", "Vxlan Network Id Value", "Vxlan Flags", "Vxlan Flags Value", "Vxlan Rsv1", "Vxlan Rsv1 Value",\
     "Vxlan Rsv2", "Vxlan Rsv2 Value","Gre Key", "Gre Key Value", "Gre Flags", "Gre Flags Value", "Gre Protocol", "Gre Protocol Value"


#define CTC_CLI_DIAG_TRACE_WATCH_POINT\
    "(watch-point (ipe-scl|tm-bs|tm-met|epe-adj|oam-adj|oam-tx)|)"

#define CTC_CLI_DIAG_TRACE_WATCH_POINT_DESC\
    "Watch Point", "Ipe Scl", "Tm bufferstore", "Tm Metfifo", "Epe Header Adjust", "OAM Header Adjust","OAM Tx"

#define CTC_CLI_DIAG_TRACE_WATCH_KEY\
    "(watch-key {channel CHAN | src-lport SRC_LPORT|src-gport SRC_GPORT|dst-gport DST_GPORT|mcast-group MC_GROUP| mep-index MEP_INDEX} |)"

#define CTC_CLI_DIAG_TRACE_WATCH_KEY_DESC\
    "Watch Key", "Network Channel", "Network Channel Id", "Source Local Port", "Source Local Port Id", \
    "Source Global Port", "Source Global Port Id", "Destination Global Port", "Destination Global Port Id",\
    "Mcast Group", "Mcast Group Id", "Oam Mep Index", "Oam Mep Index Value"


int32
_ctc_diag_cli_get_packet_from_file(char* file, uint8* buf, uint32* pkt_len)
{
    sal_file_t fp = NULL;
    char  line[128];
    uint8 loop = 0;
    char  str[2] = {0};
    int32 lidx = 0, cidx = 0, tmp_val = 0;

    /* read packet from file */
    fp = sal_fopen(file, "r");
    if (fp == NULL)
    {
        ctc_cli_out("file:%s open failed!!!\n", file);
        return CLI_ERROR;
    }

    sal_memset(line, 0, 128);
    while (sal_fgets(line, 128, fp))
    {
        for (cidx = 0; cidx < sal_strlen(line); cidx++)
        {
            if ((line[cidx] == ' ') || (line[cidx] == '\n'))
            {
                continue;
            }
            str[loop] = line[cidx];
            loop++;
            if (loop == 2)
            {
                if (1 == sal_sscanf(str, "%02x", &tmp_val))
                {
                    buf[*pkt_len] = tmp_val;
                    (*pkt_len)++;
                }
                else
                {
                    break;
                }
                loop = 0;
            }
        }
        lidx++;
    }
    sal_fclose(fp);
    fp = NULL;
    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_diag_module_init,
        ctc_cli_diag_module_init_cmd,
        "diag module (init | deinit)",
        CTC_CLI_DIAG_M_STR,
        "Module",
        "Init",
        "Deinit")
{
    int32 ret = CLI_SUCCESS;
    uint8 init = 0;
    if (CLI_CLI_STR_EQUAL("init", 0))
    {
        init = 1;
    }
    if (g_ctcs_api_en)
    {
        ret = init ? ctcs_diag_init(g_api_lchip, NULL) : ctcs_diag_deinit(g_api_lchip);
    }
    else
    {
        ret = init ? ctc_diag_init(NULL) : ctc_diag_deinit();
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    return ret;
}

STATIC void
_ctc_cli_diag_print_packet(uint8* buf, uint32 len, sal_file_t p_file)
{
    uint32 index  = 0;

    if (NULL == buf)
    {
        return;
    }
    for (index = 0; index < len; index++)
    {
        if (!p_file && (index % 16 == 0))
        {
            ctc_cli_out("\n0x%.4x:  ", index / 16 * 16);
        }
        if (p_file)
        {
            sal_fprintf(p_file, "%.2x", buf[index]);
        }
        else
        {
            ctc_cli_out("%.2x", buf[index]);
        }
        if (index % 2)
        {
            if (p_file)
            {
                sal_fprintf(p_file, " ");
            }
            else
            {
                ctc_cli_out(" ");
            }
        }
        if (p_file && ((index+1) % 16 == 0))
        {
            sal_fprintf(p_file, "\n");
        }
    }
    return;
}

STATIC void
_ctc_cli_diag_print_drop_info(uint8 lchip, ctc_diag_drop_t* diag_drop, char* file)
{
    uint32 count = 0;
    uint8  output_file = 0;
    uint16 port = 0;
    uint16 reason = 0;
    uint32 index = 0;
    char* p_tm = NULL;
    char* str_pos[] = {"IPE", "TM", "EPE", "OAM"};
    char  file_name[128] = {0};
    sal_file_t p_file = NULL;

    if (diag_drop->oper_type == CTC_DIAG_DROP_OPER_TYPE_GET_PORT_BMP)
    {
        ctc_cli_out("Lchip %d Drop Port List:\n", lchip);
        ctc_cli_out("-------------------------------------------\n");
        for (port = 0; port < CTC_PORT_BITMAP_IN_WORD * CTC_UINT32_BITS; port++)
        {
            if (!CTC_BMP_ISSET(diag_drop->u.port_bmp, port))
            {
                continue;
            }
            index++;
            ctc_cli_out("%-4d  ", port);
            if (0 == index % 4)
            {
                ctc_cli_out("\n");
            }
        }
        ctc_cli_out("\n");
    }
    else if (diag_drop->oper_type == CTC_DIAG_DROP_OPER_TYPE_GET_REASON_BMP)
    {
        ctc_cli_out("Lchip %d Port %d Reason List:\n", lchip, diag_drop->lport);
        ctc_cli_out("-------------------------------------------\n");
        for (reason = 0; reason < CTC_DIAG_DROP_REASON_BMP_LEN * CTC_UINT32_BITS; reason++)
        {
            if (!CTC_BMP_ISSET(diag_drop->u.reason_bmp, reason))
            {
                continue;
            }
            index++;
            ctc_cli_out("%-4d  ", reason);
            if (0 == index % 4)
            {
                ctc_cli_out("\n");
            }
        }
        ctc_cli_out("\n");
    }
    else
    {
        uint32 min_len = 0;
        uint32 min_count = (diag_drop->u.info.buffer_count > diag_drop->u.info.real_buffer_count) ?
                    diag_drop->u.info.real_buffer_count : diag_drop->u.info.buffer_count;

        ctc_cli_out("Lchip %d Port %d Reason %d Detail Info:\n", lchip, diag_drop->lport, diag_drop->reason);
        ctc_cli_out("-------------------------------------------\n");
        ctc_cli_out("%-20s:%s\n", "Drop Position", str_pos[diag_drop->u.info.position]);
        ctc_cli_out("%-20s:%d\n", "Drop Count", diag_drop->u.info.count);
        ctc_cli_out("%-20s:%s\n", "Drop Description", diag_drop->u.info.reason_desc);
        ctc_cli_out("-------------------------------------------\n");

        if (file && sal_strlen(file))
        {
            output_file = 1;
        }

        while (count < min_count)
        {
            if (output_file)
            {
                if (count == 0)
                {
                    sal_sprintf(file_name, "%s", file);
                }
                else
                {
                    sal_sprintf(file_name, "%s%d", file, count);
                }
                p_file = sal_fopen(file_name, "wb+");
                if (p_file == NULL)
                {
                    ctc_cli_out("file:%s create or open failed!!!\n", file_name);
                }
            }
            min_len = (diag_drop->u.info.buffer[count].real_len > diag_drop->u.info.buffer[count].len) ?
                  diag_drop->u.info.buffer[count].len : diag_drop->u.info.buffer[count].real_len;
            p_tm = sal_ctime(&diag_drop->u.info.buffer[count].timestamp);

            ctc_cli_out("%-20s:(Index: %d  Length: %d)\n", "Drop Packet", count, diag_drop->u.info.buffer[count].real_len);
            ctc_cli_out("%-20s:%s", "Timestamp", p_tm);
            ctc_cli_out("------------------------------------------------");
            if (p_file)
            {
                ctc_cli_out("\noutput file name:%s\n\n", file_name);
            }
            _ctc_cli_diag_print_packet(diag_drop->u.info.buffer[count].pkt_buf, min_len, p_file);
            if (p_file)
            {
                sal_fclose(p_file);
                p_file = NULL;
            }
            if (!output_file && min_len)
            {
                ctc_cli_out("\n------------------------------------------------\n");
            }

            count++;
        }
        ctc_cli_out("\n");
    }
    return;
}

CTC_CLI(ctc_cli_diag_get_drop_info,
        ctc_cli_diag_get_drop_info_cmd,
        "show diag drop-info (port (PORT|) (reason REASON (output-file FILENAME |) (buffer-clear|) |) | all (buffer-clear|)) (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_DIAG_M_STR,
        "Drop Information",
        "Local Port",
        "Local Port Value",
        "Drop Reason",
        "Drop Reason Value",
        "Output File",
        "Output File Name",
        "Buffer Clear",
        "All Drop Information",
        "Buffer Clear",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint16 port = 0;
    uint16 reason = 0;
    uint8 is_all = 0;
    uint32 index = 0;
    ctc_diag_drop_info_buffer_t buffer[5];
    uint32 buffer_count = 5;
    uint8 pkt_buf[5][128] = {{0}};
    uint32 pkt_buf_len = 128;
    ctc_diag_drop_t diag_drop;
    uint32 phy_port_num = 0;
    uint32 *capability = NULL;
    char file_name[64] = {0};

    capability = mem_malloc(MEM_CLI_MODULE, sizeof(uint32)*CTC_GLOBAL_CAPABILITY_MAX);
    if (NULL == capability)
    {
        return  CTC_E_NO_MEMORY;
    }
    sal_memset(capability, 0, sizeof(uint32)*CTC_GLOBAL_CAPABILITY_MAX);
    sal_memset(buffer, 0, buffer_count * sizeof(ctc_diag_drop_info_buffer_t));
    sal_memset(&diag_drop, 0, sizeof(ctc_diag_drop_t));

    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (index != 0xFF)
    {
        diag_drop.oper_type = CTC_DIAG_DROP_OPER_TYPE_GET_PORT_BMP;
        if (argc > 1)
        {
            diag_drop.oper_type = CTC_DIAG_DROP_OPER_TYPE_GET_REASON_BMP;
            CTC_CLI_GET_UINT16_RANGE("port id", diag_drop.lport, argv[index+1], 0, CTC_MAX_UINT16_VALUE);
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX("reason");
    if (index != 0xFF)
    {
        diag_drop.oper_type = CTC_DIAG_DROP_OPER_TYPE_GET_DETAIL_INFO;
        CTC_CLI_GET_UINT16_RANGE("reason id", diag_drop.reason, argv[index+1], 0, CTC_MAX_UINT16_VALUE);
        diag_drop.u.info.buffer = buffer;
        diag_drop.u.info.buffer_count = buffer_count;
        for (index = 0; index < buffer_count; index++)
        {
            buffer[index].pkt_buf = pkt_buf[index];
            buffer[index].len = pkt_buf_len;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("all");
    if (index != 0xFF)
    {
        is_all = 1;
        diag_drop.oper_type = CTC_DIAG_DROP_OPER_TYPE_GET_DETAIL_INFO;
        diag_drop.u.info.buffer = buffer;
        diag_drop.u.info.buffer_count = buffer_count;
        for (index = 0; index < buffer_count; index++)
        {
            buffer[index].pkt_buf = pkt_buf[index];
            buffer[index].len = pkt_buf_len;
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX("output-file");
    if (index != 0xFF)
    {
        if (sal_strlen(argv[1]) >= 64)
        {
            mem_free(capability);
            ctc_cli_out("File Length Exceed 64 Chars!\n");
            return CLI_ERROR;
        }
        sal_memcpy(file_name, argv[index+1], sal_strlen(argv[index+1]));
    }

    index = CTC_CLI_GET_ARGC_INDEX("buffer-clear");
    if (index != 0xFF)
    {
        diag_drop.with_clear = 1;
    }
    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    if (is_all)
    {
        ret = g_ctcs_api_en ? ctcs_global_ctl_get(g_api_lchip, CTC_GLOBAL_CHIP_CAPABILITY, capability)
                            : ctc_global_ctl_get(CTC_GLOBAL_CHIP_CAPABILITY, capability);
        if (ret < 0)
        {
            mem_free(capability);
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
        phy_port_num = capability[CTC_GLOBAL_CAPABILITY_MAX_PHY_PORT_NUM];
        for (port = 0; port < phy_port_num; port++)
        {
            diag_drop.lport = port;
            for (reason = 0; reason < CTC_DIAG_DROP_REASON_MAX; reason++)
            {
                diag_drop.u.info.count = 0;
                diag_drop.reason = reason;
                ret = g_ctcs_api_en ? ctcs_diag_get_drop_info(g_api_lchip, &diag_drop)
                                    : ctc_diag_get_drop_info(lchip, &diag_drop);
                if (ret < 0)
                {
                    continue;
                }
                _ctc_cli_diag_print_drop_info(lchip, &diag_drop, NULL);
            }
        }
    }
    else
    {
        ret = g_ctcs_api_en ? ctcs_diag_get_drop_info(g_api_lchip, &diag_drop)
                            : ctc_diag_get_drop_info(lchip, &diag_drop);
        if (ret < 0)
        {
            mem_free(capability);
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }

        _ctc_cli_diag_print_drop_info(lchip, &diag_drop, file_name);
    }
    if(capability)
    {
        mem_free(capability);
    }
    return ret;
}

#define CTC_CLI_DIAG_NETWORK_KEY_U32(F, STR, TYPE)\
    index = CTC_CLI_GET_ARGC_INDEX(STR);\
    if (index != 0xFF)\
    {\
        F[_cnt].type = TYPE;\
        F[_cnt].mask = 0xFFFFFFFF;\
        CTC_CLI_GET_UINT32(STR, F[_cnt].data, argv[index+1]);\
        _cnt++;\
    }\

#define CTC_CLI_DIAG_NETWORK_KEY_IP4(F, STR, TYPE)\
    index = CTC_CLI_GET_ARGC_INDEX(STR);\
    if (index != 0xFF)\
    {\
        F[_cnt].type = TYPE;\
        F[_cnt].mask = 0xFFFFFFFF;\
        CTC_CLI_GET_IPV4_ADDRESS(STR, F[_cnt].data, argv[index+1]);\
        _cnt++;\
    }\

#define CTC_CLI_DIAG_NETWORK_KEY(field) \
{\
    uint32 _cnt = 0;\
    CTC_CLI_DIAG_NETWORK_KEY_U32(field, "l2-type", CTC_FIELD_KEY_L2_TYPE)\
    CTC_CLI_DIAG_NETWORK_KEY_U32(field, "l3-type", CTC_FIELD_KEY_L3_TYPE)\
    CTC_CLI_DIAG_NETWORK_KEY_U32(field, "l4-type", CTC_FIELD_KEY_L4_TYPE)\
    CTC_CLI_DIAG_NETWORK_KEY_U32(field, "l4-user-type", CTC_FIELD_KEY_L4_USER_TYPE)\
    CTC_CLI_DIAG_NETWORK_KEY_U32(field, "eth-type", CTC_FIELD_KEY_ETHER_TYPE)\
    CTC_CLI_DIAG_NETWORK_KEY_U32(field, "svlan", CTC_FIELD_KEY_SVLAN_ID)\
    CTC_CLI_DIAG_NETWORK_KEY_U32(field, "cvlan", CTC_FIELD_KEY_CVLAN_ID)\
    CTC_CLI_DIAG_NETWORK_KEY_U32(field, "vlan-num", CTC_FIELD_KEY_VLAN_NUM)\
    CTC_CLI_DIAG_NETWORK_KEY_IP4(field, "ip-sa", CTC_FIELD_KEY_IP_SA)\
    CTC_CLI_DIAG_NETWORK_KEY_IP4(field, "ip-da", CTC_FIELD_KEY_IP_DA)\
    CTC_CLI_DIAG_NETWORK_KEY_U32(field, "ipv6-flow-label", CTC_FIELD_KEY_IPV6_FLOW_LABEL)\
    CTC_CLI_DIAG_NETWORK_KEY_U32(field, "ip-protocol", CTC_FIELD_KEY_IP_PROTOCOL)\
    CTC_CLI_DIAG_NETWORK_KEY_U32(field, "ip-ttl", CTC_FIELD_KEY_IP_TTL)\
    CTC_CLI_DIAG_NETWORK_KEY_U32(field, "arp-op-code", CTC_FIELD_KEY_ARP_OP_CODE)\
    CTC_CLI_DIAG_NETWORK_KEY_U32(field, "arp-protocol-type", CTC_FIELD_KEY_ARP_PROTOCOL_TYPE)\
    CTC_CLI_DIAG_NETWORK_KEY_IP4(field, "arp-sender-ip", CTC_FIELD_KEY_ARP_SENDER_IP)\
    CTC_CLI_DIAG_NETWORK_KEY_IP4(field, "arp-target-ip", CTC_FIELD_KEY_ARP_TARGET_IP)\
    CTC_CLI_DIAG_NETWORK_KEY_U32(field, "eth-oam-level", CTC_FIELD_KEY_ETHER_OAM_LEVEL)\
    CTC_CLI_DIAG_NETWORK_KEY_U32(field, "eth-oam-op-code", CTC_FIELD_KEY_ETHER_OAM_OP_CODE)\
    CTC_CLI_DIAG_NETWORK_KEY_U32(field, "eth-oam-version", CTC_FIELD_KEY_ETHER_OAM_VERSION)\
    CTC_CLI_DIAG_NETWORK_KEY_U32(field, "fcoe-dst-fcid", CTC_FIELD_KEY_FCOE_DST_FCID)\
    CTC_CLI_DIAG_NETWORK_KEY_U32(field, "fcoe-src-fcid", CTC_FIELD_KEY_FCOE_SRC_FCID)\
    CTC_CLI_DIAG_NETWORK_KEY_U32(field, "mpls-label0", CTC_FIELD_KEY_MPLS_LABEL0)\
    CTC_CLI_DIAG_NETWORK_KEY_U32(field, "mpls-label1", CTC_FIELD_KEY_MPLS_LABEL1)\
    CTC_CLI_DIAG_NETWORK_KEY_U32(field, "mpls-label2", CTC_FIELD_KEY_MPLS_LABEL2)\
    CTC_CLI_DIAG_NETWORK_KEY_U32(field, "slow-protocol-code", CTC_FIELD_KEY_SLOW_PROTOCOL_CODE)\
    CTC_CLI_DIAG_NETWORK_KEY_U32(field, "slow-protocol-flags", CTC_FIELD_KEY_SLOW_PROTOCOL_FLAGS)\
    CTC_CLI_DIAG_NETWORK_KEY_U32(field, "slow-protocol-sub-type", CTC_FIELD_KEY_SLOW_PROTOCOL_SUB_TYPE)\
    CTC_CLI_DIAG_NETWORK_KEY_U32(field, "trill-igs-nickname", CTC_FIELD_KEY_INGRESS_NICKNAME)\
    CTC_CLI_DIAG_NETWORK_KEY_U32(field, "trill-egs-nickname", CTC_FIELD_KEY_EGRESS_NICKNAME)\
    CTC_CLI_DIAG_NETWORK_KEY_U32(field, "trill-inner-vlan", CTC_FIELD_KEY_TRILL_INNER_VLANID)\
    CTC_CLI_DIAG_NETWORK_KEY_U32(field, "trill-version", CTC_FIELD_KEY_TRILL_VERSION)\
    CTC_CLI_DIAG_NETWORK_KEY_U32(field, "l4-dst-port", CTC_FIELD_KEY_L4_DST_PORT)\
    CTC_CLI_DIAG_NETWORK_KEY_U32(field, "l4-src-port", CTC_FIELD_KEY_L4_SRC_PORT)\
    CTC_CLI_DIAG_NETWORK_KEY_U32(field, "tcp-ecn", CTC_FIELD_KEY_TCP_ECN)\
    CTC_CLI_DIAG_NETWORK_KEY_U32(field, "tcp-flags", CTC_FIELD_KEY_TCP_FLAGS)\
    CTC_CLI_DIAG_NETWORK_KEY_U32(field, "vxlan-vn-id", CTC_FIELD_KEY_VN_ID)\
    CTC_CLI_DIAG_NETWORK_KEY_U32(field, "vxlan-flags", CTC_FIELD_KEY_VXLAN_FLAGS)\
    CTC_CLI_DIAG_NETWORK_KEY_U32(field, "vxlan-rsv1", CTC_FIELD_KEY_VXLAN_RSV1)\
    CTC_CLI_DIAG_NETWORK_KEY_U32(field, "vxlan-rsv2", CTC_FIELD_KEY_VXLAN_RSV2)\
    CTC_CLI_DIAG_NETWORK_KEY_U32(field, "gre-key", CTC_FIELD_KEY_GRE_KEY)\
    CTC_CLI_DIAG_NETWORK_KEY_U32(field, "gre-flags", CTC_FIELD_KEY_GRE_FLAGS)\
    CTC_CLI_DIAG_NETWORK_KEY_U32(field, "gre-protocol", CTC_FIELD_KEY_GRE_PROTOCOL_TYPE)\
    index = CTC_CLI_GET_ARGC_INDEX("mac-sa");\
    if (index != 0xFF)\
    {\
        field[_cnt].type = CTC_FIELD_KEY_MAC_SA;\
        field[_cnt].ext_mask = mac_mask;\
        CTC_CLI_GET_MAC_ADDRESS("mac sa", macsa, argv[index+1]);\
        field[_cnt].ext_data = macsa;\
        _cnt++;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mac-da");\
    if (index != 0xFF)\
    {\
        field[_cnt].type = CTC_FIELD_KEY_MAC_DA;\
        field[_cnt].ext_mask = mac_mask;\
        CTC_CLI_GET_MAC_ADDRESS("mac da", macda, argv[index+1]);\
        field[_cnt].ext_data = macda;\
        _cnt++;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ipv6-sa");\
    if (index != 0xFF)\
    {\
        field[_cnt].type = CTC_FIELD_KEY_IPV6_SA;\
        field[_cnt].ext_mask = ipv6_mask;\
        CTC_CLI_GET_IPV6_ADDRESS("ipv6 sa", ipv6_sa, argv[index+1]);\
        field[_cnt].ext_data = ipv6_sa;\
        _cnt++;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ipv6-da");\
    if (index != 0xFF)\
    {\
        field[_cnt].type = CTC_FIELD_KEY_IPV6_DA;\
        field[_cnt].ext_mask = ipv6_mask;\
        CTC_CLI_GET_IPV6_ADDRESS("ipv6 da", ipv6_da, argv[index+1]);\
        field[_cnt].ext_data = ipv6_da;\
        _cnt++;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("arp-sender-mac");\
    if (index != 0xFF)\
    {\
        field[_cnt].type = CTC_FIELD_KEY_SENDER_MAC;\
        field[_cnt].ext_mask = mac_mask;\
        CTC_CLI_GET_MAC_ADDRESS("arp-sender-mac", mac_sender, argv[index+1]);\
        field[_cnt].ext_data = mac_sender;\
        _cnt++;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("arp-target-mac");\
    if (index != 0xFF)\
    {\
        field[_cnt].type = CTC_FIELD_KEY_TARGET_MAC;\
        field[_cnt].ext_mask = mac_mask;\
        CTC_CLI_GET_MAC_ADDRESS("arp-target-mac", mac_target, argv[index+1]);\
        field[_cnt].ext_data = mac_target;\
        _cnt++;\
    }\
}


CTC_CLI(ctc_cli_diag_trigger_packet_trace,
        ctc_cli_diag_trigger_packet_trace_cmd,
        "diag trigger pkt-trace mode \
        (user-packet FILE_NAME src-gport SRC_GPORT | \
        network-packet "CTC_CLI_DIAG_TRACE_NET_KEY")"
            CTC_CLI_DIAG_TRACE_WATCH_POINT
            CTC_CLI_DIAG_TRACE_WATCH_KEY
            "(lchip LCHIP|)",
        CTC_CLI_DIAG_M_STR,
        "Trigger",
        "Packet Trace",
        "Mode",
        "User Packet",
        "User Packet File Name",
        "Source Global Port",
        "Source Global Port Id",
        "Network Packet",
        CTC_CLI_DIAG_TRACE_NET_KEY_DESC,
        CTC_CLI_DIAG_TRACE_WATCH_POINT_DESC,
        CTC_CLI_DIAG_TRACE_WATCH_KEY_DESC,
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint32 index  = 0;
    char  file_name[64] = {0};
    ctc_diag_pkt_trace_t pkt_trace;
    uint32 field_count = 0;
    mac_addr_t macsa = {0};
    mac_addr_t macda = {0};
    mac_addr_t mac_sender = {0};
    mac_addr_t mac_target = {0};
    mac_addr_t mac_mask = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    ipv6_addr_t ipv6_sa = {0};
    ipv6_addr_t ipv6_da = {0};
    ipv6_addr_t ipv6_mask = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
    ctc_field_key_t* field = NULL;
    uint8* buf = NULL;

    sal_memset(&pkt_trace, 0, sizeof(ctc_diag_pkt_trace_t));
    buf = mem_malloc(MEM_CLI_MODULE, sizeof(uint8)*1024);
    if (NULL == buf)
    {
        return  CTC_E_NO_MEMORY;
    }
    sal_memset(buf, 0, sizeof(uint8)*1024);
    if (CLI_CLI_STR_EQUAL("user-packet", 0))
    {
        pkt_trace.mode = CTC_DIAG_TRACE_MODE_USER;
        if ((sal_strlen(argv[1]) >= 64) || (sal_strlen(argv[1]) == 0))
        {
            mem_free(buf);
            ctc_cli_out("File Length Exceed 64 Chars Or Zero!\n");
            return CLI_ERROR;
        }
        sal_memcpy(file_name, argv[1], sal_strlen(argv[1]));
        CTC_CLI_GET_UINT32("source port", pkt_trace.pkt.user.src_port, argv[3]);
        ret = _ctc_diag_cli_get_packet_from_file(file_name, buf, &pkt_trace.pkt.user.len);
        if (ret != CLI_SUCCESS)
        {
            mem_free(buf);
            return ret;
        }
        pkt_trace.pkt.user.pkt_buf = buf;
    }
    else
    {
        pkt_trace.mode = CTC_DIAG_TRACE_MODE_NETWORK;
    }

    index = CTC_CLI_GET_ARGC_INDEX("watch-point");
    if (index != 0xFF)
    {
        field_count = field_count ? field_count : (index/2);
        if (CLI_CLI_STR_EQUAL("ipe-scl", index+1))
        {
            pkt_trace.watch_point = CTC_DIAG_TRACE_POINT_IPE_SCL;
        }
        else if (CLI_CLI_STR_EQUAL("tm-bs", index+1))
        {
            pkt_trace.watch_point = CTC_DIAG_TRACE_POINT_TM_BS;
        }
        else if (CLI_CLI_STR_EQUAL("tm-met", index+1))
        {
            pkt_trace.watch_point = CTC_DIAG_TRACE_POINT_TM_MET;
        }
        else if (CLI_CLI_STR_EQUAL("epe-adj", index+1))
        {
            pkt_trace.watch_point = CTC_DIAG_TRACE_POINT_EPE_ADJ;
        }
        else if (CLI_CLI_STR_EQUAL("oam-adj", index+1))
        {
            pkt_trace.watch_point = CTC_DIAG_TRACE_POINT_OAM_ADJ;
        }
        else if (CLI_CLI_STR_EQUAL("oam-tx", index+1))
        {
            pkt_trace.watch_point = CTC_DIAG_TRACE_POINT_OAM_TX;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("watch-key");
    if (index != 0xFF)
    {
        field_count = field_count ? field_count : (index/2);
        index = CTC_CLI_GET_ARGC_INDEX("channel");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("channel id", pkt_trace.watch_key.channel, argv[index + 1]);
            CTC_BMP_SET(pkt_trace.watch_key.key_mask_bmp, CTC_DIAG_WATCH_KEY_MASK_CHANNEL);
        }
        index = CTC_CLI_GET_ARGC_INDEX("src-lport");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("source lport", pkt_trace.watch_key.src_lport, argv[index + 1]);
            CTC_BMP_SET(pkt_trace.watch_key.key_mask_bmp, CTC_DIAG_WATCH_KEY_MASK_SRC_LPORT);
        }
        index = CTC_CLI_GET_ARGC_INDEX("src-gport");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("source gport", pkt_trace.watch_key.src_gport, argv[index + 1]);
            CTC_BMP_SET(pkt_trace.watch_key.key_mask_bmp, CTC_DIAG_WATCH_KEY_MASK_SRC_GPORT);
        }
        index = CTC_CLI_GET_ARGC_INDEX("dst-gport");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("dest gport", pkt_trace.watch_key.dst_gport, argv[index + 1]);
            CTC_BMP_SET(pkt_trace.watch_key.key_mask_bmp, CTC_DIAG_WATCH_KEY_MASK_DST_GPORT);
        }
        index = CTC_CLI_GET_ARGC_INDEX("mcast-group");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("mcast group", pkt_trace.watch_key.mc_group, argv[index + 1]);
            CTC_BMP_SET(pkt_trace.watch_key.key_mask_bmp, CTC_DIAG_WATCH_KEY_MASK_MC_GROUP);
        }
        index = CTC_CLI_GET_ARGC_INDEX("mep-index");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("mep index", pkt_trace.watch_key.mep_index, argv[index + 1]);
            CTC_BMP_SET(pkt_trace.watch_key.key_mask_bmp, CTC_DIAG_WATCH_KEY_MASK_MEP_INDEX);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        field_count = field_count ? field_count : (index/2);
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    if (pkt_trace.mode == CTC_DIAG_TRACE_MODE_NETWORK)
    {
        field_count = field_count ? field_count : (argc/2);
        field = mem_malloc(MEM_CLI_MODULE, field_count * sizeof(ctc_field_key_t));
        if (NULL == field)
        {
            mem_free(buf);
            ctc_cli_out("No Memory!\n");
            return CLI_ERROR;
        }
        sal_memset(field, 0, field_count * sizeof(ctc_field_key_t));
        CTC_CLI_DIAG_NETWORK_KEY(field);
        pkt_trace.pkt.network.count = field_count;
        pkt_trace.pkt.network.field = field;
    }

    ret = g_ctcs_api_en ? ctcs_diag_trigger_pkt_trace(g_api_lchip, &pkt_trace)
                        : ctc_diag_trigger_pkt_trace(lchip, &pkt_trace);

    if (field)
    {
        mem_free(field);
    }
    if (buf)
    {
        mem_free(buf);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}


CTC_CLI(ctc_cli_diag_get_packet_trace,
        ctc_cli_diag_get_packet_trace_cmd,
        "show diag pkt-trace (detail "CTC_CLI_DIAG_TRACE_WATCH_POINT"|)(lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_DIAG_M_STR,
        "Packet Trace",
        "Display Detail Information",
        CTC_CLI_DIAG_TRACE_WATCH_POINT_DESC,
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    uint32 index = 0;
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 get_detail = 0;
    uint32 real_count = 0;
    char* str_dest_type[] = {"None", "Network","CPU", "Drop", "OAM"};
    char* str_pos[] = {"IPE", "TM", "EPE", "OAM"};
    char buf[CTC_IPV6_ADDR_STR_LEN] = {0};
    uint32 temp_ip = 0;
    uint32 loop = 0;
    uint32 watch_point = 0;
    uint32 temp_ip6[4] = {0, 0, 0, 0};
    ctc_diag_pkt_trace_result_t trace_rslt;
    ctc_diag_pkt_trace_result_info_t* info = NULL;
    ctc_diag_pkt_trace_result_info_t* info_temp = NULL;

    sal_memset(&trace_rslt, 0, sizeof(ctc_diag_pkt_trace_result_t));

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("detail");
    if (0xFF != index)
    {
        get_detail = 1;
    }
    index = CTC_CLI_GET_ARGC_INDEX("watch-point");
    if (index != 0xFF)
    {
        if (CLI_CLI_STR_EQUAL("ipe-scl", index+1))
        {
            watch_point = CTC_DIAG_TRACE_POINT_IPE_SCL;
        }
        else if (CLI_CLI_STR_EQUAL("tm-bs", index+1))
        {
            watch_point = CTC_DIAG_TRACE_POINT_TM_BS;
        }
        else if (CLI_CLI_STR_EQUAL("tm-met", index+1))
        {
            watch_point = CTC_DIAG_TRACE_POINT_TM_MET;
        }
        else if (CLI_CLI_STR_EQUAL("epe-adj", index+1))
        {
            watch_point = CTC_DIAG_TRACE_POINT_EPE_ADJ;
        }
        else if (CLI_CLI_STR_EQUAL("oam-adj", index+1))
        {
            watch_point = CTC_DIAG_TRACE_POINT_OAM_ADJ;
        }
        else if (CLI_CLI_STR_EQUAL("oam-tx", index+1))
        {
            watch_point = CTC_DIAG_TRACE_POINT_OAM_TX;
        }
    }

    trace_rslt.watch_point = watch_point;
    ctc_cli_out("Show Diag Packet Trace:\n");
    ctc_cli_out("--------------------------------\n");

    ret = g_ctcs_api_en ? ctcs_diag_get_pkt_trace(g_api_lchip, &trace_rslt)
                        : ctc_diag_get_pkt_trace(lchip, &trace_rslt);

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    real_count = trace_rslt.real_count;
    if (real_count == 0)
    {
        return CLI_SUCCESS;
    }

    ctc_cli_out("%-20s:%s\n","Dest Type", str_dest_type[trace_rslt.dest_type]);
    if (trace_rslt.flags & CTC_DIAG_TRACE_RSTL_FLAG_PORT)
    {
        ctc_cli_out("%-20s:0x%.4x\n","Dest Port", trace_rslt.port);
    }
    if (trace_rslt.flags & CTC_DIAG_TRACE_RSTL_FLAG_CHANNLE)
    {
        ctc_cli_out("%-20s:%d\n","Dest Channel", trace_rslt.channel);
    }
    if (trace_rslt.flags & CTC_DIAG_TRACE_RSTL_FLAG_MC_GROUP)
    {
        ctc_cli_out("%-20s:%d\n","Mcast Group", trace_rslt.mc_group);
    }
    if (trace_rslt.flags & CTC_DIAG_TRACE_RSTL_FLAG_TID)
    {
        ctc_cli_out("%-20s:%d\n","Linkagg Group", trace_rslt.tid);
    }
    if (trace_rslt.flags & CTC_DIAG_TRACE_RSTL_FLAG_ECMP_GROUP)
    {
        ctc_cli_out("%-20s:%d\n","Ecmp Group", trace_rslt.ecmp_group);
    }
    if (trace_rslt.flags & CTC_DIAG_TRACE_RSTL_FLAG_L3IF_ID)
    {
        ctc_cli_out("%-20s:%d\n","Interface Id", trace_rslt.l3if_id);
    }
    if (trace_rslt.flags & CTC_DIAG_TRACE_RSTL_FLAG_DROP_REASON)
    {
        ctc_cli_out("%-20s:%d\n","Drop Reason", trace_rslt.drop_reason);
    }
    if (trace_rslt.flags & CTC_DIAG_TRACE_RSTL_FLAG_DROP_DESC)
    {
        ctc_cli_out("%-20s:%s\n","Drop Description", trace_rslt.drop_desc);
    }
    if (trace_rslt.flags & CTC_DIAG_TRACE_RSTL_FLAG_EXCP_VECTOR)
    {
        ctc_cli_out("%-20s:0x%x\n","Exception Vector", trace_rslt.excp_vector);
    }
    if (trace_rslt.loop_flags & CTC_DIAG_LOOP_FLAG_ILOOP)
    {
        ctc_cli_out("%-20s:","Loop Flags");
        if (trace_rslt.loop_flags & CTC_DIAG_LOOP_FLAG_ILOOP)
        {
            ctc_cli_out("%s","iLoop");
        }
        if (trace_rslt.loop_flags & CTC_DIAG_LOOP_FLAG_ELOOP)
        {
            ctc_cli_out("%s"," eLoop");
        }
        ctc_cli_out("\n");
    }
    ctc_cli_out("%-20s:%s\n","Current Position", str_pos[trace_rslt.position]);
    ctc_cli_out("--------------------------------\n");

    if (!get_detail)
    {
        return CLI_SUCCESS;
    }

    info_temp = mem_malloc(MEM_CLI_MODULE,real_count*sizeof(ctc_diag_pkt_trace_result_info_t));
    if (NULL == info_temp)
    {
        ctc_cli_out("No Memory!\n");
        return CLI_ERROR;
    }
    sal_memset(&trace_rslt, 0, sizeof(ctc_diag_pkt_trace_result_t));
    trace_rslt.watch_point = watch_point;
    trace_rslt.count = real_count;
    trace_rslt.info = info_temp;
    ret = g_ctcs_api_en ? ctcs_diag_get_pkt_trace(g_api_lchip, &trace_rslt)
                        : ctc_diag_get_pkt_trace(lchip, &trace_rslt);
    if (ret < 0)
    {
        mem_free(info_temp);
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    if (trace_rslt.real_count == 0)
    {
        mem_free(info_temp);
        return CLI_SUCCESS;
    }

    ctc_cli_out("Detail Path Info:\n");
    ctc_cli_out("------------------------------------------------\n");
    for (index = 0; index < real_count; index++)
    {
        info = &trace_rslt.info[index];
        switch (info->val_type)
        {
            case CTC_DIAG_VAL_NONE:
                ctc_cli_out("\n%s\n", info->str);
                break;
            case CTC_DIAG_VAL_UINT32:
                ctc_cli_out("    %-25s:%d\n", info->str, info->val.u32);
                break;
            case CTC_DIAG_VAL_U32_HEX:
                ctc_cli_out("    %-25s:0x%x\n", info->str, info->val.u32);
                break;
            case CTC_DIAG_VAL_UINT64:
                ctc_cli_out("    %-25s:%"PRId64"\n", info->str, info->val.u64);
                break;
            case CTC_DIAG_VAL_U64_HEX:
                ctc_cli_out("    %-25s:0x%"PRIx64"\n", info->str, info->val.u64);
                break;
            case CTC_DIAG_VAL_CHAR:
                ctc_cli_out("    %-25s:%s\n", info->str, info->val.str);
                break;
            case CTC_DIAG_VAL_MAC:
                ctc_cli_out("    %-25s:%.4x.%.4x.%.4x\n", info->str,
                                    sal_ntohs(*(unsigned short*)&info->val.mac[0]),
                                    sal_ntohs(*(unsigned short*)&info->val.mac[2]),
                                    sal_ntohs(*(unsigned short*)&info->val.mac[4]));
                break;
            case CTC_DIAG_VAL_IPv4:
                temp_ip = sal_ntohl(info->val.ipv4);
                sal_inet_ntop(AF_INET, &temp_ip, buf, CTC_IPV6_ADDR_STR_LEN);
                ctc_cli_out("    %-25s:%s\n", info->str, buf);
                break;
            case CTC_DIAG_VAL_IPv6:
                temp_ip6[0] = sal_ntohl(info->val.ipv6[0]);
                temp_ip6[1] = sal_ntohl(info->val.ipv6[1]);
                temp_ip6[2] = sal_ntohl(info->val.ipv6[2]);
                temp_ip6[3] = sal_ntohl(info->val.ipv6[3]);
                sal_inet_ntop(AF_INET6, temp_ip6, buf, CTC_IPV6_ADDR_STR_LEN);
                ctc_cli_out("    %-25s:%s\n", info->str, buf);
                break;
            case CTC_DIAG_VAL_PKT_HDR:
                ctc_cli_out("    %-25s:0x", info->str);
                loop = CTC_DIAG_PACKET_HEAR_LEN;
                while (loop)
                {
                    ctc_cli_out("%.2X", info->val.pkt_hdr[--loop]);
                }
                ctc_cli_out("\n");
                break;
            default:
                break;
        }
    }
    ctc_cli_out("\n");
    mem_free(info_temp);
    return CLI_SUCCESS;
}

int32
_ctc_cli_diag_drop_pkt_report_cb_func(uint32 gport, uint16 reason, ctc_diag_drop_info_buffer_t* pkt_buf)
{
    if (NULL == pkt_buf)
    {
        return CTC_E_INVALID_PARAM;
    }
    ctc_cli_out("Drop Packet Report:gport:0x%.4x  reason:%d\n", gport, reason);
    ctc_cli_out("-------------------------------------------");
    _ctc_cli_diag_print_packet(pkt_buf->pkt_buf, pkt_buf->len, NULL);
    ctc_cli_out("\n-------------------------------------------\n");
    return CTC_E_NONE;
}

CTC_CLI(ctc_cli_diag_set_property,
        ctc_cli_diag_set_property_cmd,
        "diag property drop-pkt {store-en (enable|disable)|hash-calc-len VALUE|store-len VALUE|store-count VALUE|overwrite (enable|disable)|clear|report-en (enable|disable)}  (lchip LCHIP|)",
        CTC_CLI_DIAG_M_STR,
        "Property",
        "Drop packet",
        "Strore packet enable",
        "Enable",
        "Disable",
        "Hash calculate length",
        "Value",
        "Store packet length",
        "Value",
        "Store packet count",
        "Value",
        "Overwrite the oldest packet buffer when buffer exceed",
        "Enable",
        "Disable",
        "Clear store packet",
        "Drop packet report enable",
        "Enable",
        "Disable",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    uint32 index = 0;
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    ctc_diag_property_t prop = CTC_DIAG_PROP_DROP_PKT_CONFIG;
    ctc_diag_drop_pkt_config_t cfg;

    sal_memset(&cfg, 0, sizeof(ctc_diag_drop_pkt_config_t));
    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("store-en");
    if (0xFF != index)
    {
        cfg.flags |= CTC_DIAG_DROP_PKT_STORE_EN;
        if (CLI_CLI_STR_EQUAL("enable", index + 1))
        {
            cfg.storage_en = TRUE;
        }
        else
        {
            cfg.storage_en = FALSE;
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX("overwrite");
    if (0xFF != index)
    {
        cfg.flags |= CTC_DIAG_DROP_PKT_OVERWRITE;
        if (CLI_CLI_STR_EQUAL("enable", index + 1))
        {
            cfg.overwrite = TRUE;
        }
        else
        {
            cfg.overwrite = FALSE;
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX("hash-calc-len");
    if (0xFF != index)
    {
        cfg.flags |= CTC_DIAG_DROP_PKT_HASH_CALC_LEN;
        CTC_CLI_GET_UINT32("hash-calc-len", cfg.hash_calc_len, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("store-len");
    if (0xFF != index)
    {
        cfg.flags |= CTC_DIAG_DROP_PKT_STORE_LEN;
        CTC_CLI_GET_UINT32("store-len", cfg.storage_len, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("store-count");
    if (0xFF != index)
    {
        cfg.flags |= CTC_DIAG_DROP_PKT_STORE_CNT;
        CTC_CLI_GET_UINT32("store-count", cfg.storage_cnt, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("clear");
    if (0xFF != index)
    {
        cfg.flags |= CTC_DIAG_DROP_PKT_CLEAR;
    }
    index = CTC_CLI_GET_ARGC_INDEX("report-en");
    if (0xFF != index)
    {
        cfg.flags |= CTC_DIAG_DROP_PKT_REPORT_CALLBACK;

        if (CLI_CLI_STR_EQUAL("enable", index + 1))
        {
            cfg.report_cb = _ctc_cli_diag_drop_pkt_report_cb_func;
        }
        else
        {
            cfg.report_cb = NULL;
        }
    }

    ret = g_ctcs_api_en ? ctcs_diag_set_property(g_api_lchip, prop, &cfg)
                        : ctc_diag_set_property(lchip, prop, &cfg);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    return ret;
}

CTC_CLI(ctc_cli_diag_get_property,
        ctc_cli_diag_get_property_cmd,
        "show diag property drop-pkt (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_DIAG_M_STR,
        "Property",
        "Drop packet",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    uint32 index = 0;
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    ctc_diag_property_t prop = CTC_DIAG_PROP_DROP_PKT_CONFIG;
    ctc_diag_drop_pkt_config_t cfg;

    sal_memset(&cfg, 0, sizeof(ctc_diag_drop_pkt_config_t));

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    ret = g_ctcs_api_en ? ctcs_diag_get_property(g_api_lchip, prop, &cfg)
                        : ctc_diag_get_property(lchip, prop, &cfg);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    if (cfg.flags & CTC_DIAG_DROP_PKT_STORE_EN)
    {
        ctc_cli_out("%-25s:%d\n", "Storage enable", cfg.storage_en);
    }
    if (cfg.flags & CTC_DIAG_DROP_PKT_OVERWRITE)
    {
        ctc_cli_out("%-25s:%d\n", "Storage overwrite", cfg.overwrite);
    }
    if (cfg.flags & CTC_DIAG_DROP_PKT_STORE_LEN)
    {
        ctc_cli_out("%-25s:%d\n", "Storage length", cfg.storage_len);
    }
    if (cfg.flags & CTC_DIAG_DROP_PKT_STORE_CNT)
    {
        ctc_cli_out("%-25s:%d\n", "Storage count", cfg.storage_cnt);
    }
    if (cfg.flags & CTC_DIAG_DROP_PKT_HASH_CALC_LEN)
    {
        ctc_cli_out("%-25s:%d\n", "Hash calculate length", cfg.hash_calc_len);
    }
    return ret;
}

CTC_CLI(ctc_cli_diag_debug_on,
        ctc_cli_diag_debug_on_cmd,
        "debug diag (ctc|sys) (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        CTC_CLI_DIAG_M_STR,
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
        typeenum = DIAG_CTC;
    }
    else
    {
        typeenum = DIAG_SYS;
    }

    ctc_debug_set_flag("diag", "diag", typeenum, level, TRUE);
    return CLI_SUCCESS;
}
CTC_CLI(ctc_cli_diag_debug_off,
        ctc_cli_diag_debug_off_cmd,
        "no debug diag (ctc|sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_DIAG_M_STR,
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = DIAG_CTC;
    }
    else
    {
        typeenum = DIAG_SYS;
    }

    ctc_debug_set_flag("diag", "diag", typeenum, level, FALSE);
    return CLI_SUCCESS;
}


int32
ctc_diag_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_diag_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_diag_debug_off_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_diag_module_init_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_diag_get_drop_info_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_diag_trigger_packet_trace_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_diag_get_packet_trace_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_diag_set_property_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_diag_get_property_cmd);

    return CLI_SUCCESS;
}

