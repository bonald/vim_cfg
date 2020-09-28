

#include "ctc_error.h"
#include "ctc_diag.h"
#include "sys_usw_diag.h"
#include "sys_usw_common.h"
#include "sys_usw_mchip.h"

#include "drv_api.h"
#include "usw/include/drv_enum.h"
#include "usw/include/drv_common.h"
extern int32
sys_usw_diag_drop_hash_update_count(uint8 lchip, uint32 port, uint16 reason, uint32 count);

#define DIAG_LOOKUP_CONFLICT "CONFLICT"
#define DIAG_LOOKUP_DEFAULT "HIT-DEFAULT-ENTRY"
#define DIAG_LOOKUP_HIT  "HIT"
#define DIAG_LOOKUP_HIT_NONE "HIT-NONE"

typedef struct sys_diag_pkt_trace_table_s
{
    uint32 table;
    uint32 field;
}sys_diag_pkt_trace_table_t;

enum sys_tsingma_drop_reason_id_e
{
   /*-----------------------  IPE   -----------------------*/
    IPE_START        = SYS_DIAG_DROP_REASON_BASE_IPE,

    IPE_FEATURE_START = IPE_START,
    IPE_RESERVED0     = IPE_FEATURE_START  ,/**IPE_RESERVED0, Ipe reserved0 discard*/
    IPE_LPBK_HDR_ADJ_RM_ERR                ,/**IPE_LPBK_HDR_ADJ_RM_ERR, Packet discard due to length error when do loopback header adjust*/
    IPE_PARSER_LEN_ERR                     ,/**IPE_PARSER_LEN_ERR, Packet discard due to length error when do parser*/
    IPE_UC_TO_LAG_GROUP_NO_MEMBER          ,/**IPE_UC_TO_LAG_GROUP_NO_MEMBER, Packet discard due to ucast no linkagg member*/
    IPE_EXCEP2_DIS                         ,/**IPE_EXCEP2_DIS, L2 PDU discard*/
    IPE_DS_USERID_DIS                      ,/**IPE_DS_USERID_DIS, Next hop index from SCL is invalid*/
    IPE_RECEIVE_DIS                        ,/**IPE_RECEIVE_DIS, Port or vlan receive function is disabled*/
    IPE_MICROFLOW_POLICING_FAIL_DROP       ,/**IPE_MICROFLOW_POLICING_FAIL_DROP, Microflow policing discard*/
    IPE_PROTOCOL_VLAN_DIS                  ,/**IPE_PROTOCOL_VLAN_DIS, Protocol vlan is configed as discard*/
    IPE_AFT_DIS                            ,/**IPE_AFT_DIS, AFT discard*/
    IPE_L2_ILLEGAL_PKT_DIS                 ,/**IPE_L2_ILLEGAL_PKT_DIS, Packet discard due to invalid mcast MACSA or invalid IPDA/IPSA*/
    IPE_STP_DIS                            ,/**IPE_STP_DIS, STP discard*/
    IPE_DEST_MAP_PROFILE_DISCARD           ,/**IPE_DEST_MAP_PROFILE_DISCARD, Destmap of destmap profile is invalid(destmap=0xFFFF)*/
    IPE_STACK_REFLECT_DISCARD              ,/**IPE_STACK_REFLECT_DISCARD, Stack reflect discard*/
    IPE_ARP_DHCP_DIS                       ,/**IPE_ARP_DHCP_DIS, ARP or DHCP discard*/
    IPE_DS_PHYPORT_SRCDIS                  ,/**IPE_DS_PHYPORT_SRCDIS, Ingress phyport discard*/
    IPE_VLAN_FILTER_DIS                    ,/**IPE_VLAN_FILTER_DIS, Ingress vlan filtering discard*/
    IPE_DS_SCL_DIS                         ,/**IPE_DS_SCL_DIS, Packet discard due to SCL deny or GRE flag mismatch*/
    IPE_ROUTE_ERROR_PKT_DIS                ,/**IPE_ROUTE_ERROR_PKT_DIS, Mcast MAC DA discard or same IP DA SA discard*/
    IPE_SECURITY_CHK_DIS                   ,/**IPE_SECURITY_CHK_DIS, Bridge dest port is equal to source port discard or unknown Mcast/Ucast or Bcast discard*/
    IPE_STORM_CTL_DIS                      ,/**IPE_STORM_CTL_DIS, Port or vlan based storm Control discard*/
    IPE_LEARNING_DIS                       ,/**IPE_LEARNING_DIS, MACSA is configed as discard or source port mismatch or port security discard*/
    IPE_NO_FORWARDING_PTR                  ,/**IPE_NO_FORWARDING_PTR, Packet discard due to not find next hop(no dsFwdPtr)*/
    IPE_IS_DIS_FORWARDING_PTR              ,/**IPE_IS_DIS_FORWARDING_PTR, Packet discard due to next hop index is invalid(dsFwdPtr=0xFFFF)*/
    IPE_FATAL_EXCEP_DIS                    ,/**IPE_FATAL_EXCEP_DIS, Packet discard due to some fatal event occured*/
    IPE_APS_DIS                            ,/**IPE_APS_DIS, APS selector discard*/
    IPE_DS_FWD_DESTID_DIS                  ,/**IPE_DS_FWD_DESTID_DIS, Packet discard due to next hop is invalid(nextHopPtr=0x3FFFF)*/
    IPE_LOOPBACK_DIS                       ,/**IPE_LOOPBACK_DIS, Loopback discard when from Fabric is disabled*/
    IPE_DISCARD_PACKET_LOG_ALL_TYPE        ,/**IPE_DISCARD_PACKET_LOG_ALL_TYPE, Packet log all type discard*/
    IPE_PORT_MAC_CHECK_DIS                 ,/**IPE_PORT_MAC_CHECK_DIS, Port mac check discard*/
    IPE_L3_EXCEP_DIS                       ,/**IPE_L3_EXCEP_DIS, Packet discard due to VXLAN/NVGRE fraginfo error or ACH type error*/
    IPE_STACKING_HDR_CHK_ERR               ,/**IPE_STACKING_HDR_CHK_ERR, Stacking network header length or MAC DA check error*/
    IPE_TUNNEL_DECAP_MARTIAN_ADD           ,/**IPE_TUNNEL_DECAP_MARTIAN_ADD, IPv4/IPv6 tunnel outer IPSA is Martian address*/
    IPE_TUNNELID_FWD_PTR_DIS               ,/**IPE_TUNNELID_FWD_PTR_DIS, Packet discard due to tunnel dsfwdptr is invalid(dsfwdptr=0xFFFF)*/
    IPE_VXLAN_FLAG_CHK_ERROR_DISCARD       ,/**IPE_VXLAN_FLAG_CHK_ERROR_DISCARD, VXLAN flag error*/
    IPE_VXLAN_NVGRE_INNER_VTAG_CHK_DIS     ,/**IPE_VXLAN_NVGRE_INNER_VTAG_CHK_DIS, VXLAN/NVGRE inner Vtag check error*/
    IPE_VXLAN_NVGRE_CHK_FAIL               ,/**IPE_VXLAN_NVGRE_CHK_FAIL, VXLAN/NVGRE check error*/
    IPE_GENEVE_PAKCET_DISCARD              ,/**IPE_GENEVE_PAKCET_DISCARD, VXLAN packet is OAM frame or has option or redirect to CPU*/
    IPE_ICMP_REDIRECT_DIS                  ,/**IPE_ICMP_REDIRECT_DIS, ICMP discard or redirect to CPU*/
    IPE_ICMP_ERR_MSG_DIS                   ,/**IPE_ICMP_ERR_MSG_DIS, IPv4/IPv6 ICMP error MSG check discard*/
    IPE_PTP_PKT_DIS                        ,/**IPE_PTP_PKT_DIS, PTP packet discard*/
    IPE_MUX_PORT_ERR                       ,/**IPE_MUX_PORT_ERR, Mux port error*/
    IPE_HW_ERROR_DISCARD                   ,/**IPE_HW_ERROR_DISCARD, Packet discard due to hardware error*/
    IPE_USERID_BINDING_DIS                 ,/**IPE_USERID_BINDING_DIS, Packet discard due to mac/port/vlan/ip/ binding mismatch */
    IPE_DS_PLC_DIS                         ,/**IPE_DS_PLC_DIS, Policing discard*/
    IPE_DS_ACL_DIS                         ,/**IPE_DS_ACL_DIS, ACL discard*/
    IPE_DOT1AE_CHK                         ,/**IPE_DOT1AE_CHK, Dot1ae check discard*/
    IPE_OAM_DISABLE                        ,/**IPE_OAM_DISABLE, OAM discard*/
    IPE_OAM_NOT_FOUND                      ,/**IPE_OAM_NOT_FOUND, OAM lookup hash conflict or no MEP/MIP at ether OAM edge port*/
    IPE_CFLAX_SRC_ISOLATE_DIS              ,/**IPE_CFLAX_SRC_ISOLATE_DIS, Cflex source isolate discard*/
    IPE_OAM_ETH_VLAN_CHK                   ,/**IPE_OAM_ETH_VLAN_CHK, Linkoam with vlan tag or ethoam without vlan tag*/
    IPE_OAM_BFD_TTL_CHK                    ,/**IPE_OAM_BFD_TTL_CHK, BFD ttl check discard*/
    IPE_OAM_FILTER_DIS                     ,/**IPE_OAM_FILTER_DIS, OAM packet STP/VLAN filter when no MEP/MIP*/
    IPE_TRILL_CHK                          ,/**IPE_TRILL_CHK, Trill check discard*/
    IPE_WLAN_CHK                           ,/**IPE_WLAN_CHK, Wlan check diacard*/
    IPE_TUNNEL_ECN_DIS                     ,/**IPE_TUNNEL_ECN_DIS, Tunnel ECN discard*/
    IPE_EFM_DIS                            ,/**IPE_EFM_DIS, EFM OAM packet discard or redirect to CPU*/
    IPE_ILOOP_DIS                          ,/**IPE_ILOOP_DIS, Iloop discard*/
    IPE_MPLS_ENTROPY_LABEL_CHK             ,/**IPE_MPLS_ENTROPY_LABEL_CHK, Entropy label check discard*/
    IPE_MPLS_TP_MCC_SCC_DIS                ,/**IPE_MPLS_TP_MCC_SCC_DIS, TP MCC or SCC packet redirect to CPU*/
    IPE_MPLS_MC_PKT_ERROR                  ,/**IPE_MPLS_MC_PKT_ERROR, MPLS mcast discard*/
    IPE_L2_EXCPTION_DIS                    ,/**IPE_L2_EXCPTION_DIS, L2 PDU discard*/
    IPE_NAT_PT_CHK                         ,/**IPE_NAT_PT_CHK, IPv4/IPv6 Ucast NAT discard*/
    IPE_SD_CHECK_DIS                       ,/**IPE_SD_CHECK_DIS, Signal degrade discard*/
    IPE_FEATURE_END = IPE_SD_CHECK_DIS,
    IPE_END = IPE_FEATURE_END,
   /*-----------------------  EPE   ------------------------*/
    EPE_START          = SYS_DIAG_DROP_REASON_BASE_EPE,
    EPE_FEATURE_START = EPE_START,
    EPE_HDR_ADJ_DESTID_DIS = EPE_FEATURE_START,/**EPE_HDR_ADJ_DESTID_DIS, Packet discard by drop channel*/
    EPE_RESERVED                           ,/**EPE_RESERVED, Epe reserved discard*/
    EPE_HDR_ADJ_REMOVE_ERR                 ,/**EPE_HDR_ADJ_REMOVE_ERR, Packet discard due to remove bytes error at header adjust(offset>144B)*/
    EPE_DS_DESTPHYPORT_DSTID_DIS           ,/**EPE_DS_DESTPHYPORT_DSTID_DIS, Egress port discard*/
    EPE_PORT_ISO_DIS                       ,/**EPE_PORT_ISO_DIS, Port isolate or Pvlan discard*/
    EPE_DS_VLAN_TRANSMIT_DIS               ,/**EPE_DS_VLAN_TRANSMIT_DIS, Packet discard due to port/vlan transmit disable*/
    EPE_BRG_TO_SAME_PORT_DIS               ,/**EPE_BRG_TO_SAME_PORT_DIS, Packet discard due to bridge to same port(no use)*/
    EPE_VPLS_HORIZON_SPLIT_DIS             ,/**EPE_VPLS_HORIZON_SPLIT_DIS, VPLS split horizon split or E-tree discard*/
    EPE_VLAN_FILTER_DIS                    ,/**EPE_VLAN_FILTER_DIS, Egress vlan filtering discard*/
    EPE_STP_DIS                            ,/**EPE_STP_DIS, STP discard*/
    EPE_PARSER_LEN_ERR                     ,/**EPE_PARSER_LEN_ERR, Packet discard due to parser length error*/
    EPE_RESERVED0                        ,/**EPE_RESERVED0, Epe reserved0 discard*/
    EPE_UC_MC_FLOODING_DIS                 ,/**EPE_UC_MC_FLOODING_DIS, Unkown-unicast/known-unicast/unkown-mcast/known-mcast/broadcast discard*/
    EPE_OAM_802_3_DIS                      ,/**EPE_OAM_802_3_DIS, Discard non-EFM OAM packet*/
    EPE_TTL_FAIL                           ,/**EPE_TTL_FAIL, TTL check failed*/
    EPE_REMOTE_MIRROR_ESCAP_DIS            ,/**EPE_REMOTE_MIRROR_ESCAP_DIS, Packet discard due to remote mirror filtering*/
    EPE_TUNNEL_MTU_CHK_DIS                 ,/**EPE_TUNNEL_MTU_CHK_DIS, Packet discard due to tunnel MTU check*/
    EPE_INTERF_MTU_CHK_DIS                 ,/**EPE_INTERF_MTU_CHK_DIS, Packet discard due to interface MTU check*/
    EPE_LOGIC_PORT_CHK_DIS                 ,/**EPE_LOGIC_PORT_CHK_DIS, Packet discard due to logic source port is equal to dest port*/
    EPE_DS_ACL_DIS                         ,/**EPE_DS_ACL_DIS, Packet discard due to ACL deny*/
    EPE_DS_VLAN_XLATE_DIS                  ,/**EPE_DS_VLAN_XLATE_DIS, Packet discard due to SCL deny*/
    EPE_CRC_ERR_DIS                        ,/**EPE_CRC_ERR_DIS, Packet discard due to CRC check error(no use)*/
    EPE_ROUTE_PLD_OP_DIS                   ,/**EPE_ROUTE_PLD_OP_DIS, Packet discard due to route payload operation no IP*/
    EPE_BRIDGE_PLD_OP_DIS                  ,/**EPE_BRIDGE_PLD_OP_DIS, Bridge payload operation bridge is disabled*/
    EPE_STRIP_OFFSET_LARGE                 ,/**EPE_STRIP_OFFSET_LARGE, Packet strip offset is larger then efficientFirstBufferLength*/
    EPE_BFD_DIS                            ,/**EPE_BFD_DIS, BFD discard(no use)*/
    EPE_PORT_REFLECTIVE_CHK_DIS            ,/**EPE_PORT_REFLECTIVE_CHK_DIS, Packet discard due to dest port is equal to source port*/
    EPE_IP_MPLS_TTL_CHK_ERR                ,/**EPE_IP_MPLS_TTL_CHK_ERR, Packet discard due to IP/MPLS TTL check error when do L3 edit*/
    EPE_OAM_EDGE_PORT_DIS                  ,/**EPE_OAM_EDGE_PORT_DIS, No MEP/MIP at edge port for OAM packet*/
    EPE_NAT_PT_ICMP_ERR                    ,/**EPE_NAT_PT_ICMP_ERR, NAT/PT ICMP error*/
    EPE_LATENCY_DISCARD                    ,/**EPE_LATENCY_DISCARD, Packet discard due to latency is too long*/
    EPE_LOCAL_OAM_DIS                      ,/**EPE_LOCAL_OAM_DIS, Local OAM discard(no use)*/
    EPE_OAM_FILTERING_DIS                  ,/**EPE_OAM_FILTERING_DIS, OAM packet Port/STP/VLAN filter discard or forward link OAM packet*/
    EPE_SAME_IPDA_IPSA_DIS                 ,/**EPE_SAME_IPDA_IPSA_DIS, Same MAC DA SA or IP DA SA*/
    EPE_SD_CHK_DISCARD                     ,/**EPE_SD_CHK_DISCARD, Signal degrade discard*/
    EPE_TRILL_PLD_OP_DIS                   ,/**EPE_TRILL_PLD_OP_DIS, TRILL payload operation discard(no use)*/
    EPE_RESERVED2                   ,/**EPE_RESERVED2, Epe reserved2 discard*/
    EPE_DS_NEXTHOP_DATA_VIOLATE            ,/**EPE_DS_NEXTHOP_DATA_VIOLATE, Packet discard due to DsNextHop data violate*/
    EPE_DEST_VLAN_PTR_DIS                  ,/**EPE_DEST_VLAN_PTR_DIS, Packet discard due to dest vlan index is invalid*/
    EPE_DS_L3EDIT_DATA_VIOLATE1            ,/**EPE_DS_L3EDIT_DATA_VIOLATE1, Discard by L3 edit or inner L2 edit violate*/
    EPE_DS_L3EDIT_DATA_VIOLATE2            ,/**EPE_DS_L3EDIT_DATA_VIOLATE2, Discard by L3 edit or inner L2 edit violate*/
    EPE_DS_L3EDITNAT_DATA_VIOLATE          ,/**EPE_DS_L3EDITNAT_DATA_VIOLATE, Discard by L3 edit or inner L2 edit violate*/
    EPE_DS_L2EDIT_DATA_VIOLATE1            ,/**EPE_DS_L2EDIT_DATA_VIOLATE1, Discard by L2 edit config*/
    EPE_DS_L2EDIT_DATA_VIOLATE2            ,/**EPE_DS_L2EDIT_DATA_VIOLATE2, Discard by L2 edit violate*/
    EPE_PKT_HDR_C2C_TTL_ZERO               ,/**EPE_PKT_HDR_C2C_TTL_ZERO, Packet discard due to packetHeader C2C TTL zero(no use)*/
    EPE_PT_UDP_CHKSUM_ZERO                 ,/**EPE_PT_UDP_CHKSUM_ZERO, Packet discard due to PT/UDP checksum is zero for NAT*/
    EPE_OAM_TO_LOCAL_DIS                   ,/**EPE_OAM_TO_LOCAL_DIS, Packet is sent to local up MEP*/
    EPE_HARD_ERROR_DIS                     ,/**EPE_HARD_ERROR_DIS, Packet discard due to hardware error*/
    EPE_MICROFLOW_POLICING_FAIL_DROP       ,/**EPE_MICROFLOW_POLICING_FAIL_DROP, Microflow policing discard*/
    EPE_ARP_MISS_DISCARD                   ,/**EPE_ARP_MISS_DISCARD, ARP miss discard*/
    EPE_ILLEGAL_PACKET_TO_E2I_LOOP_CHANNEL ,/**EPE_ILLEGAL_PACKET_TO_E2I_LOOP_CHANNEL, Packet discard due to illegal packet to e2iloop channel*/
    EPE_UNKNOWN_DOT11_PACKET_DISCARD       ,/**EPE_UNKNOWN_DOT11_PACKET_DISCARD, Unknow dot1ae packet discard*/
    EPE_DOT1AE_PN_OVERFLOW                 ,/**EPE_DOT1AE_PN_OVERFLOW, dot1ae pn overflow*/
    EPE_PON_DOWNSTREAM_CHECK_FAIL          ,/**EPE_PON_DOWNSTREAM_CHECK_FAIL, pon downstream check fail*/
    EPE_FEATURE_END = EPE_PON_DOWNSTREAM_CHECK_FAIL,
    EPE_MIN_PKT_LEN_ERR                    ,/**EPE_MIN_PKT_LEN_ERR, Packet discard due to min length check error*/
    EPE_ELOG_ABORTED_PKT                   ,/**EPE_ELOG_ABORTED_PKT, Packet discard due to elog aborted*/
    EPE_ELOG_DROPPED_PKT                   ,/**EPE_ELOG_DROPPED_PKT, Packet discard due to elog dropped*/
    EPE_HW_START                           ,
    EPE_HW_HAR_ADJ = EPE_HW_START,/**EPE_HW_HAR_ADJ, Some hardware error at header adjust*/
    EPE_HW_NEXT_HOP                        ,/**EPE_HW_NEXT_HOP, Some hardware error occurred at nexthop mapper*/
    EPE_HW_L2_EDIT                         ,/**EPE_HW_L2_EDIT, Some hardware error occurred at l2 edit*/
    EPE_HW_L3_EDIT                         ,/**EPE_HW_L3_EDIT, Some hardware error occurred at l3 edit*/
    EPE_HW_INNER_L2                        ,/**EPE_HW_INNER_L2, Some hardware error occurred at inner l2 edit*/
    EPE_HW_PAYLOAD                         ,/**EPE_HW_PAYLOAD, Some hardware error occurred when edit payload*/
    EPE_HW_ACL_OAM                         ,/**EPE_HW_ACL_OAM, Some hardware error occurred when do OAM lookup*/
    EPE_HW_CLASS                           ,/**EPE_HW_CLASS, Some hardware error occurred at classification*/
    EPE_HW_OAM                             ,/**EPE_HW_OAM, Some hardware error occurred when do OAM process*/
    EPE_HW_EDIT                            ,/**EPE_HW_EDIT, Some hardware error occurred when do edit*/
    EPE_HW_END = EPE_HW_EDIT,
    EPE_END = EPE_HW_END,
    /*-----------------------  BSR   ------------------------*/
    BSR_START                              ,
    /*----------per channel discard stats start--------------*/
    BUFSTORE_ABORT_TOTAL = BSR_START,       /**BUFSTORE_ABORT_TOTAL, Packet discard in BSR and is due to crc/ecc/packet len error or miss EOP or pkt buffer is full */
    BUFSTORE_LEN_ERROR              ,/**BUFSTORE_LEN_ERROR, Packet discard in BSR and is due to maximum or minimum packet length check*/
    BUFSTORE_IRM_RESRC              ,/**BUFSTORE_IRM_RESRC, Packet discard in BSR and is due to IRM no resource*/
    BUFSTORE_DATA_ERR               ,/**BUFSTORE_DATA_ERR, Packet discard in BSR and is due to crc error or ecc error*/
    BUFSTORE_CHIP_MISMATCH          ,/**BUFSTORE_CHIP_MISMATCH, Packet discard in BSR and is due to chip id mismatch*/
    BUFSTORE_NO_BUFF                ,/**BUFSTORE_NO_BUFF, Packet discard in BSR and is due to no pkt buffer*/
    BUFSTORE_NO_EOP                 ,/**BUFSTORE_NO_EOP, Packet discard in BSR and is due to missing EOP */
    METFIFO_STALL_TO_BS_DROP        ,/**METFIFO_STALL_TO_BS_DROP, Packet discard in BSR and is due to the copy ability of MET*/
    BUFSTORE_SLIENT_TOTAL           ,/**BUFSTORE_SLIENT_TOTAL, Packet discard in BSR and is due to resource check fail*/
    /*----------per channel discard stats end----------------*/
    TO_BUFSTORE_FROM_DMA            ,/**TO_BUFSTORE_FROM_DMA, Packet from DMA discard in BSR and is due to Bufstore error*/
    TO_BUFSTORE_FROM_NETRX          ,/**TO_BUFSTORE_FROM_NETRX, Packet from NetRx discard in BSR and is due to Bufstore error*/
    TO_BUFSTORE_FROM_OAM            ,/**TO_BUFSTORE_FROM_OAM, Packet from OAM discard in BSR and is due to Bufstore error*/
    BUFSTORE_OUT_SOP_PKT_LEN_ERR    ,/**BUFSTORE_OUT_SOP_PKT_LEN_ERR, Packet from BUFSTORE discard in BSR and is due to SOP length error */
    METFIFO_MC_FROM_DMA             ,/**METFIFO_MC_FROM_DMA, Mcast packet from DMA discard in BSR and is due to the copy ability of MET*/
    METFIFO_UC_FROM_DMA             ,/**METFIFO_UC_FROM_DMA, Ucast packet from DMA discard in BSR and is due to the copy ability of MET*/
    METFIFO_MC_FROM_ELOOP_MCAST     ,/**METFIFO_MC_FROM_ELOOP_MCAST, Eloop mcast packet discard in BSR and is due to the copy ability of MET*/
    METFIFO_UC_FROM_ELOOP_UCAST     ,/**METFIFO_UC_FROM_ELOOP_UCAST, Eloop ucast packet discard in BSR and is due to the copy ability of MET*/
    METFIFO_MC_FROM_IPE_CUT_THR     ,/**METFIFO_MC_FROM_IPE_CUT_THR, Mcast packet discard in BSR and is due to the copy ability of MET in cutthrough model*/
    METFIFO_UC_FROM_IPE_CUT_THR     ,/**METFIFO_UC_FROM_IPE_CUT_THR, Ucast packet discard in BSR and is due to the copy ability of MET in cutthrough model*/
    METFIFO_MC_FROM_IPE             ,/**METFIFO_MC_FROM_IPE, IPE mcast packet discard in BSR and is due to the copy ability of MET*/
    METFIFO_UC_FROM_IPE             ,/**METFIFO_UC_FROM_IPE, IPE ucast packet discard in BSR and is due to the copy ability of MET*/
    METFIFO_MC_FROM_OAM             ,/**METFIFO_MC_FROM_OAM, OAM mcast packet discard in BSR and is due to the copy ability of MET*/
    METFIFO_UC_FROM_OAM             ,/**METFIFO_UC_FROM_OAM, OAM ucast packet discard in BSR and is due to the copy ability of MET*/
    BUFRETRV_FROM_DEQ_MSG_ERR       ,/**BUFRETRV_FROM_DEQ_MSG_ERR, Packet from DEQ discard in BSR and is due to MSG error */
    BUFRETRV_FROM_QMGR_LEN_ERR      ,/**BUFRETRV_FROM_QMGR_LEN_ERR, Packet from QMGR discard in BSR and is due to length error*/
    BUFRETRV_OUT_DROP               ,/**BUFRETRV_OUT_DROP, Packet from bufRetrv discard in BSR*/
    /*----------per queue discard stats start----------------*/
    ENQ_WRED_DROP                   ,/**ENQ_WRED_DROP, ENQ packet discard in BSR and is due to WRED error*/
    ENQ_TOTAL_DROP                  ,/**ENQ_TOTAL_DROP, ENQ packe total discard*/
    ENQ_NO_QUEUE_ENTRY              ,/**ENQ_NO_QUEUE_ENTRY, ENQ packet discard in BSR and is due to no ptr*/
    ENQ_PORT_NO_BUFF                ,/**ENQ_PORT_NO_BUFF, ENQ packet discard in BSR and is due to port no buffer*/
    ENQ_QUEUE_NO_BUFF               ,/**ENQ_QUEUE_NO_BUFF, ENQ packet discard in BSR and is due to queue no buffer*/
    ENQ_SC_NO_BUFF                  ,/**ENQ_SC_NO_BUFF, ENQ packet discard in BSR and is due to service class no buffer*/
    ENQ_SPAN_NO_BUFF                ,/**ENQ_SPAN_NO_BUFF, ENQ packet discard in BSR and is due to span no buffer*/
    ENQ_TOTAL_NO_BUFF               ,/**ENQ_TOTAL_NO_BUFF, ENQ packet discard in BSR and is due to no total buffer*/
    ENQ_FWD_DROP                    ,/**ENQ_FWD_DROP, ENQ packet discard in BSR and is due to foward drop*/
    ENQ_FWD_DROP_CFLEX_ISOLATE_BLOCK,/**ENQ_FWD_DROP_CFLEX_ISOLATE_BLOCK, ENQ packet discard in BSR and is due to isolated port when stacking*/
    ENQ_FWD_DROP_CHAN_INVALID       ,/**ENQ_FWD_DROP_CHAN_INVALID, ENQ packet discard in BSR and is due to invalid channel id*/
    ENQ_FWD_DROP_TOTAL              ,/**ENQ_FWD_DROP_TOTAL, ENQ packet discard in BSR and is due to fowarding drop*/
    ENQ_FWD_DROP_FROM_LAG           ,/**ENQ_FWD_DROP_FROM_LAG, ENQ packet discard in BSR and is due to linkagg error*/
    ENQ_FWD_DROP_FROM_LAG_ERR       ,/**ENQ_FWD_DROP_FROM_LAG_ERR, ENQ packet discard in BSR and is due to crc error or memory error when do linkagg*/
    ENQ_FWD_DROP_FROM_LAG_MC        ,/**ENQ_FWD_DROP_FROM_LAG_MC, ENQ packet discard in BSR and is due to invalid dest chip id when do mcast linkagg*/
    ENQ_FWD_DROP_FROM_LAG_NO_MEM    ,/**ENQ_FWD_DROP_FROM_LAG_NO_MEM, ENQ packet discard in BSR and is due to no linkagg member*/
    ENQ_FWD_DROP_PORT_ISOLATE_BLOCK ,/**ENQ_FWD_DROP_PORT_ISOLATE_BLOCK, ENQ packet discard in BSR and is due to isolated port*/
    ENQ_FWD_DROP_RSV_CHAN_DROP      ,/**ENQ_FWD_DROP_RSV_CHAN_DROP, ENQ packet discard in BSR and is due to drop channel*/
    ENQ_FWD_DROP_FROM_STACKING_LAG  ,/**ENQ_FWD_DROP_FROM_STACKING_LAG, ENQ packet discard in BSR and is due to stacking linkagg error*/
    /*-----------per queue discard stats end-----------------*/
    BSR_END = ENQ_FWD_DROP_FROM_STACKING_LAG,
   /*-----------------------  NETRX   ----------------------*/
    NETRX_START                            ,
    NETRX_NO_BUFFER = NETRX_START,/**NETRX_NO_BUFFER, Packet discard due to no buffer in port oversub model*/
    NETRX_LEN_ERROR                        ,/**NETRX_LEN_ERROR, Packet discard due to the failure of max or min pkt len check*/
    NETRX_PKT_ERROR                        ,/**NETRX_PKT_ERROR, Packet error discard*/
    NETRX_BPDU_ERROR                       ,/**NETRX_BPDU_ERROR, BPDU discard*/
    NETRX_FRAME_ERROR                      ,/**NETRX_FRAME_ERROR, Packet discard due to misssing sop or eop*/
    NETRX_END = NETRX_FRAME_ERROR,
   /*-----------------------  NETTX   ----------------------*/
    NETTX_START                            ,
    NETTX_MIN_LEN = NETTX_START,/**NETTX_MIN_LEN, Packet discard due to smaller than the cfgMinPktLen*/
    NETTX_NO_BUFFER                        ,/**NETTX_NO_BUFFER, Packet discard due to the use up of PktBuffer in NETTX*/
    NETTX_SOP_EOP                          ,/**NETTX_SOP_EOP, Packet discard due to missing EOP or SOP*/
    NETTX_TX_ERROR                         ,/**NETTX_TX_ERROR, The number of error packets sending to MAC*/
    NETTX_END = NETTX_TX_ERROR,
   /*-----------------------   OAM   ------------------------*/
    OAM_START                              ,
    OAM_HW_ERROR = OAM_START,/**OAM_HW_ERROR, Some hardware error occured*/
    OAM_EXCEPTION                            ,/**OAM_EXCEPTION, Some exception occured when process OAM packet*/
    OAM_END = OAM_EXCEPTION ,
    SYS_TM_DROP_REASON_MAX,
};
typedef enum sys_tsingma_drop_reason_id_e sys_tsingma_drop_reason_id_t;


enum sys_tsingma_diag_l3da_key_type_e
{
    SYS_TM_DIAG_L3DA_TYPE_IPV4UC,
    SYS_TM_DIAG_L3DA_TYPE_IPV6UC,
    SYS_TM_DIAG_L3DA_TYPE_IPV4MC,
    SYS_TM_DIAG_L3DA_TYPE_IPV6MC,
    SYS_TM_DIAG_L3DA_TYPE_FCOE,
    SYS_TM_DIAG_L3DA_TYPE_TRILLUC,
    SYS_TM_DIAG_L3DA_TYPE_TRILLMC,

    SYS_TM_DIAG_L3DA_TYPE_MAX
};
typedef  enum sys_tsingma_diag_l3da_key_type_e sys_tsingma_diag_l3da_key_type_t;

enum sys_tsingma_diag_l3sa_key_type_e
{
    SYS_TM_DIAG_L3SA_TYPE_IPV4RPF,
    SYS_TM_DIAG_L3SA_TYPE_IPV6RPF,
    SYS_TM_DIAG_L3SA_TYPE_IPV4PBR,
    SYS_TM_DIAG_L3SA_TYPE_IPV6PBR,
    SYS_TM_DIAG_L3SA_TYPE_IPV4NATSA,
    SYS_TM_DIAG_L3SA_TYPE_IPV6NATSA,
    SYS_TM_DIAG_L3SA_TYPE_FCOERPF,

    SYS_TM_DIAG_L3SA_TYPE_MAX
};
typedef  enum sys_tsingma_diag_l3sa_key_type_e sys_tsingma_diag_l3sa_key_type_t;

#define DIAG_VLAN_ACTION_NONE              0
#define DIAG_VLAN_ACTION_MODIFY            1
#define DIAG_VLAN_ACTION_ADD               2
#define DIAG_VLAN_ACTION_DELETE            3

#define DIAG_INFO_NONE(POS, STR)\
    if (p_rslt->real_count < p_rslt->count)\
    {\
        sal_strcpy(p_rslt->info[p_rslt->real_count].str, STR);\
        p_rslt->info[p_rslt->real_count].val_type = CTC_DIAG_VAL_NONE;\
        p_rslt->info[p_rslt->real_count].position = POS;\
    }\
    p_rslt->real_count++;

#define DIAG_INFO_CHAR(POS, STR, STR_OUT)\
    if (p_rslt->real_count < p_rslt->count)\
    {\
        sal_strcpy(p_rslt->info[p_rslt->real_count].str, STR);\
        p_rslt->info[p_rslt->real_count].val_type = CTC_DIAG_VAL_CHAR;\
        sal_strcpy(p_rslt->info[p_rslt->real_count].val.str, (STR_OUT));\
        p_rslt->info[p_rslt->real_count].position = POS;\
    }\
    p_rslt->real_count++;

#define DIAG_INFO_UINT32(POS, STR, U32)\
    if (p_rslt->real_count < p_rslt->count)\
    {\
        sal_strcpy(p_rslt->info[p_rslt->real_count].str, STR);\
        p_rslt->info[p_rslt->real_count].val_type = CTC_DIAG_VAL_UINT32;\
        p_rslt->info[p_rslt->real_count].val.u32 = U32;\
        p_rslt->info[p_rslt->real_count].position = POS;\
    }\
    p_rslt->real_count++;

#define DIAG_INFO_U32_HEX(POS, STR, U32)\
    if (p_rslt->real_count < p_rslt->count)\
    {\
        sal_strcpy(p_rslt->info[p_rslt->real_count].str, STR);\
        p_rslt->info[p_rslt->real_count].val_type = CTC_DIAG_VAL_U32_HEX;\
        p_rslt->info[p_rslt->real_count].val.u32 = U32;\
        p_rslt->info[p_rslt->real_count].position = POS;\
    }\
    p_rslt->real_count++;

#define DIAG_INFO_UINT64(POS, STR, U64)\
    if (p_rslt->real_count < p_rslt->count)\
    {\
        sal_strcpy(p_rslt->info[p_rslt->real_count].str, STR);\
        p_rslt->info[p_rslt->real_count].val_type = CTC_DIAG_VAL_UINT64;\
        p_rslt->info[p_rslt->real_count].val.u64 = U64;\
        p_rslt->info[p_rslt->real_count].position = POS;\
    }\
    p_rslt->real_count++;

#define DIAG_INFO_U64_HEX(POS, STR, U64)\
    if (p_rslt->real_count < p_rslt->count)\
    {\
        sal_strcpy(p_rslt->info[p_rslt->real_count].str, STR);\
        p_rslt->info[p_rslt->real_count].val_type = CTC_DIAG_VAL_U64_HEX;\
        p_rslt->info[p_rslt->real_count].val.u64 = U64;\
        p_rslt->info[p_rslt->real_count].position = POS;\
    }\
    p_rslt->real_count++;

#define DIAG_INFO_MAC(POS, STR, HW_MAC)\
    if (p_rslt->real_count < p_rslt->count)\
    {\
        sal_strcpy(p_rslt->info[p_rslt->real_count].str, STR);\
        p_rslt->info[p_rslt->real_count].val_type = CTC_DIAG_VAL_MAC;\
        SYS_USW_SET_USER_MAC(p_rslt->info[p_rslt->real_count].val.mac,HW_MAC);\
        p_rslt->info[p_rslt->real_count].position = POS;\
    }\
    p_rslt->real_count++;

#define DIAG_INFO_IP4(POS, STR, V4)\
    if (p_rslt->real_count < p_rslt->count)\
    {\
        sal_strcpy(p_rslt->info[p_rslt->real_count].str, STR);\
        p_rslt->info[p_rslt->real_count].val_type = CTC_DIAG_VAL_IPv4;\
        p_rslt->info[p_rslt->real_count].val.ipv4 = V4;\
        p_rslt->info[p_rslt->real_count].position = POS;\
    }\
    p_rslt->real_count++;

#define DIAG_INFO_IP6(POS, STR, V6)\
    if (p_rslt->real_count < p_rslt->count)\
    {\
        sal_strcpy(p_rslt->info[p_rslt->real_count].str, STR);\
        p_rslt->info[p_rslt->real_count].val_type = CTC_DIAG_VAL_IPv6;\
        p_rslt->info[p_rslt->real_count].val.ipv6[0] = V6[3];\
        p_rslt->info[p_rslt->real_count].val.ipv6[1] = V6[2];\
        p_rslt->info[p_rslt->real_count].val.ipv6[2] = V6[1];\
        p_rslt->info[p_rslt->real_count].val.ipv6[3] = V6[0];\
        p_rslt->info[p_rslt->real_count].position = POS;\
    }\
    p_rslt->real_count++;

#define DIAG_INFO_PKT_HDR(POS, STR, HDR)\
    if (p_rslt->real_count < p_rslt->count)\
    {\
        sal_strcpy(p_rslt->info[p_rslt->real_count].str, STR);\
        p_rslt->info[p_rslt->real_count].val_type = CTC_DIAG_VAL_PKT_HDR;\
        sal_memcpy(p_rslt->info[p_rslt->real_count].val.pkt_hdr, HDR, CTC_DIAG_PACKET_HEAR_LEN * sizeof(uint8));\
        p_rslt->info[p_rslt->real_count].position = POS;\
    }\
    p_rslt->real_count++;


const char *_sys_tsingma_diag_get_reason_desc(uint16 reason_id)
{
    static char  reason_desc[CTC_DIAG_REASON_DESC_LEN] = {0};

    switch (reason_id)
    {
        case IPE_RESERVED0:
            return "IPE_RESERVED0, Ipe reserved0 discard";
        case IPE_LPBK_HDR_ADJ_RM_ERR:
            return "IPE_LPBK_HDR_ADJ_RM_ERR, Packet discard due to length error when do loopback header adjust";
        case IPE_PARSER_LEN_ERR:
            return "IPE_PARSER_LEN_ERR, Packet discard due to length error when do parser";
        case IPE_UC_TO_LAG_GROUP_NO_MEMBER:
            return "IPE_UC_TO_LAG_GROUP_NO_MEMBER, Packet discard due to ucast no linkagg member";
        case IPE_EXCEP2_DIS:
            return "IPE_EXCEP2_DIS, L2 PDU discard";
        case IPE_DS_USERID_DIS:
            return "IPE_DS_USERID_DIS, Next hop index from SCL is invalid";
        case IPE_RECEIVE_DIS:
            return "IPE_RECEIVE_DIS, Port or vlan receive function is disabled";
        case IPE_MICROFLOW_POLICING_FAIL_DROP:
            return "IPE_MICROFLOW_POLICING_FAIL_DROP, Microflow policing discard";
        case IPE_PROTOCOL_VLAN_DIS:
            return "IPE_PROTOCOL_VLAN_DIS, Protocol vlan is configed as discard";
        case IPE_AFT_DIS:
            return "IPE_AFT_DIS, AFT discard";
        case IPE_L2_ILLEGAL_PKT_DIS:
            return "IPE_L2_ILLEGAL_PKT_DIS, Packet discard due to invalid mcast MACSA or invalid IPDA/IPSA";
        case IPE_STP_DIS:
            return "IPE_STP_DIS, STP discard";
        case IPE_DEST_MAP_PROFILE_DISCARD:
            return "IPE_DEST_MAP_PROFILE_DISCARD, Destmap of destmap profile is invalid(destmap=0xFFFF)";
        case IPE_STACK_REFLECT_DISCARD:
            return "IPE_STACK_REFLECT_DISCARD, Stack reflect discard";
        case IPE_ARP_DHCP_DIS:
            return "IPE_ARP_DHCP_DIS, ARP or DHCP discard";
        case IPE_DS_PHYPORT_SRCDIS:
            return "IPE_DS_PHYPORT_SRCDIS, Ingress phyport discard";
        case IPE_VLAN_FILTER_DIS:
            return "IPE_VLAN_FILTER_DIS, Ingress vlan filtering discard";
        case IPE_DS_SCL_DIS:
            return "IPE_DS_SCL_DIS, Packet discard due to ACL deny or GRE flag mismatch";
        case IPE_ROUTE_ERROR_PKT_DIS:
            return "IPE_ROUTE_ERROR_PKT_DIS, Mcast MAC DA discard or same IP DA SA discard";
        case IPE_SECURITY_CHK_DIS:
            return "IPE_SECURITY_CHK_DIS, Bridge dest port is equal to source port discard or unknown Mcast/Ucast or Bcast discard";
        case IPE_STORM_CTL_DIS:
            return "IPE_STORM_CTL_DIS, Port or vlan based storm Control discard";
        case IPE_LEARNING_DIS:
            return "IPE_LEARNING_DIS, MACSA is configed as discard or source port mismatch or port security discard";
        case IPE_NO_FORWARDING_PTR:
            return "IPE_NO_FORWARDING_PTR, Packet discard due to not find next hop(no dsFwdPtr)";
        case IPE_IS_DIS_FORWARDING_PTR:
            return "IPE_IS_DIS_FORWARDING_PTR, Packet discard due to next hop index is invalid(dsFwdPtr=0xFFFF)";
        case IPE_FATAL_EXCEP_DIS:
            return "IPE_FATAL_EXCEP_DIS, Packet discard due to some fatal event occured";
        case IPE_APS_DIS:
            return "IPE_APS_DIS, APS selector discard";
        case IPE_DS_FWD_DESTID_DIS:
            return "IPE_DS_FWD_DESTID_DIS, Packet discard due to next hop is invalid(nextHopPtr=0x3FFFF)";
        case IPE_LOOPBACK_DIS:
            return "IPE_LOOPBACK_DIS, Loopback discard when from Fabric is disabled";
        case IPE_DISCARD_PACKET_LOG_ALL_TYPE:
            return "IPE_DISCARD_PACKET_LOG_ALL_TYPE, Packet log all type discard";
        case IPE_PORT_MAC_CHECK_DIS:
            return "IPE_PORT_MAC_CHECK_DIS, Port mac check discard";
        case IPE_L3_EXCEP_DIS:
            return "IPE_L3_EXCEP_DIS, Packet discard due to VXLAN/NVGRE fraginfo error or ACH type error";
        case IPE_STACKING_HDR_CHK_ERR:
            return "IPE_STACKING_HDR_CHK_ERR, Stacking network header length or MAC DA check error";
        case IPE_TUNNEL_DECAP_MARTIAN_ADD:
            return "IPE_TUNNEL_DECAP_MARTIAN_ADD, IPv4/IPv6 tunnel outer IPSA is Martiana address";
        case IPE_TUNNELID_FWD_PTR_DIS:
            return "IPE_TUNNELID_FWD_PTR_DIS, Packet discard due to tunnel dsfwdptr is invalid(dsfwdptr=0xFFFF)";
        case IPE_VXLAN_FLAG_CHK_ERROR_DISCARD:
            return "IPE_VXLAN_FLAG_CHK_ERROR_DISCARD, VXLAN flag error";
        case IPE_VXLAN_NVGRE_INNER_VTAG_CHK_DIS:
            return "IPE_VXLAN_NVGRE_INNER_VTAG_CHK_DIS, VXLAN/NVGRE inner Vtag check error";
        case IPE_VXLAN_NVGRE_CHK_FAIL:
            return "IPE_VXLAN_NVGRE_CHK_FAIL, VXLAN/NVGRE check error";
        case IPE_GENEVE_PAKCET_DISCARD:
            return "IPE_GENEVE_PAKCET_DISCARD, VXLAN packet is OAM frame or has option or redirect to CPU";
        case IPE_ICMP_REDIRECT_DIS:
            return "IPE_ICMP_REDIRECT_DIS, ICMP discard or redirect to CPU";
        case IPE_ICMP_ERR_MSG_DIS:
            return "IPE_ICMP_ERR_MSG_DIS, IPv4/IPv6 ICMP error MSG check discard";
        case IPE_PTP_PKT_DIS:
            return "IPE_PTP_PKT_DIS, PTP packet discard";
        case IPE_MUX_PORT_ERR:
            return "IPE_MUX_PORT_ERR, Mux port error";
        case IPE_HW_ERROR_DISCARD:
            return "IPE_HW_ERROR_DISCARD, Packet discard due to hardware error";
        case IPE_USERID_BINDING_DIS:
            return "IPE_USERID_BINDING_DIS, Packet discard due to mac/port/vlan/ip/ binding mismatch ";
        case IPE_DS_PLC_DIS:
            return "IPE_DS_PLC_DIS, Policing discard";
        case IPE_DS_ACL_DIS:
            return "IPE_DS_ACL_DIS, Pcl discard";
        case IPE_DOT1AE_CHK:
            return "IPE_DOT1AE_CHK, Dot1ae check discard";
        case IPE_OAM_DISABLE:
            return "IPE_OAM_DISABLE, OAM discard";
        case IPE_OAM_NOT_FOUND:
            return "IPE_OAM_NOT_FOUND, OAM lookup hash conflict or no MEP/MIP at ether OAM edge port";
        case IPE_CFLAX_SRC_ISOLATE_DIS:
            return "IPE_CFLAX_SRC_ISOLATE_DIS, Cflex source isolate discard";
        case IPE_OAM_ETH_VLAN_CHK:
            return "IPE_OAM_ETH_VLAN_CHK, Linkoam with vlan tag or ethoam without vlan tag";
        case IPE_OAM_BFD_TTL_CHK:
            return "IPE_OAM_BFD_TTL_CHK, BFD ttl check discard";
        case IPE_OAM_FILTER_DIS:
            return "IPE_OAM_FILTER_DIS, OAM packet STP/VLAN filter when no MEP/MIP";
        case IPE_TRILL_CHK:
            return "IPE_TRILL_CHK, Trill check discard";
        case IPE_WLAN_CHK:
            return "IPE_WLAN_CHK, Wlan check diacard";
        case IPE_TUNNEL_ECN_DIS:
            return "IPE_TUNNEL_ECN_DIS, Tunnel ECN discard";
        case IPE_EFM_DIS:
            return "IPE_EFM_DIS, EFM OAM packet discard or redirect to CPU";
        case IPE_ILOOP_DIS:
            return "IPE_ILOOP_DIS, Iloop discard";
        case IPE_MPLS_ENTROPY_LABEL_CHK:
            return "IPE_MPLS_ENTROPY_LABEL_CHK, Entropy label check discard";
        case IPE_MPLS_TP_MCC_SCC_DIS:
            return "IPE_MPLS_TP_MCC_SCC_DIS, TP MCC or SCC packet redirect to CPU";
        case IPE_MPLS_MC_PKT_ERROR:
            return "IPE_MPLS_MC_PKT_ERROR, MPLS mcast discard";
        case IPE_L2_EXCPTION_DIS:
            return "IPE_L2_EXCPTION_DIS, L2 PDU discard";
        case IPE_NAT_PT_CHK:
            return "IPE_NAT_PT_CHK, IPv4/IPv6 Ucast NAT discard";
        case IPE_SD_CHECK_DIS:
            return "IPE_SD_CHECK_DIS, Signal degrade discard";
        case EPE_HDR_ADJ_DESTID_DIS:
            return "EPE_HDR_ADJ_DESTID_DIS, Packet discard by drop channel";
        case EPE_RESERVED:
            return "EPE_RESERVED, Epe reserved discard";
        case EPE_HDR_ADJ_REMOVE_ERR:
            return "EPE_HDR_ADJ_REMOVE_ERR, Packet discard due to remove bytes error at header adjust(offset>144B)";
        case EPE_DS_DESTPHYPORT_DSTID_DIS:
            return "EPE_DS_DESTPHYPORT_DSTID_DIS, Egress port discard";
        case EPE_PORT_ISO_DIS:
            return "EPE_PORT_ISO_DIS, Port isolate or Pvlan discard";
        case EPE_DS_VLAN_TRANSMIT_DIS:
            return "EPE_DS_VLAN_TRANSMIT_DIS, Packet discard due to port/vlan transmit disable";
        case EPE_BRG_TO_SAME_PORT_DIS:
            return "EPE_BRG_TO_SAME_PORT_DIS, Packet discard due to bridge to same port(no use)";
        case EPE_VPLS_HORIZON_SPLIT_DIS:
            return "EPE_VPLS_HORIZON_SPLIT_DIS, VPLS split horizon split or E-tree discard";
        case EPE_VLAN_FILTER_DIS:
            return "EPE_VLAN_FILTER_DIS, Egress vlan filtering discard";
        case EPE_STP_DIS:
            return "EPE_STP_DIS, STP discard";
        case EPE_PARSER_LEN_ERR:
            return "EPE_PARSER_LEN_ERR, Packet discard due to parser length error";
        case EPE_RESERVED0:
            return "EPE_RESERVED0, Epe reserved0 discard";
        case EPE_UC_MC_FLOODING_DIS:
            return "EPE_UC_MC_FLOODING_DIS, Unkown-unicast/known-unicast/unkown-mcast/known-mcast/broadcast discard";
        case EPE_OAM_802_3_DIS:
            return "EPE_OAM_802_3_DIS, Discard non-EFM OAM packet";
        case EPE_TTL_FAIL:
            return "EPE_TTL_FAIL, TTL check failed";
        case EPE_REMOTE_MIRROR_ESCAP_DIS:
            return "EPE_REMOTE_MIRROR_ESCAP_DIS, Packet discard due to remote mirror filtering";
        case EPE_TUNNEL_MTU_CHK_DIS:
            return "EPE_TUNNEL_MTU_CHK_DIS, Packet discard due to tunnel MTU check";
        case EPE_INTERF_MTU_CHK_DIS:
            return "EPE_INTERF_MTU_CHK_DIS, Packet discard due to interface MTU check";
        case EPE_LOGIC_PORT_CHK_DIS:
            return "EPE_LOGIC_PORT_CHK_DIS, Packet discard due to logic source port is equal to dest port";
        case EPE_DS_ACL_DIS:
            return "EPE_DS_ACL_DIS, Packet discard due to ACL deny";
        case EPE_DS_VLAN_XLATE_DIS:
            return "EPE_DS_VLAN_XLATE_DIS, Packet discard due to SCL deny";
        case EPE_CRC_ERR_DIS:
            return "EPE_CRC_ERR_DIS, Packet discard due to CRC check error(no use)";
        case EPE_ROUTE_PLD_OP_DIS:
            return "EPE_ROUTE_PLD_OP_DIS, Packet discard due to route payload operation no IP";
        case EPE_BRIDGE_PLD_OP_DIS:
            return "EPE_BRIDGE_PLD_OP_DIS, Bridge payload operation bridge is disabled";
        case EPE_STRIP_OFFSET_LARGE:
            return "EPE_STRIP_OFFSET_LARGE, Packet strip offset is larger then efficientFirstBufferLength";
        case EPE_BFD_DIS:
            return "EPE_BFD_DIS, BFD discard(no use)";
        case EPE_PORT_REFLECTIVE_CHK_DIS:
            return "EPE_PORT_REFLECTIVE_CHK_DIS, Packet discard due to dest port is equal to source port";
        case EPE_IP_MPLS_TTL_CHK_ERR:
            return "EPE_IP_MPLS_TTL_CHK_ERR, Packet discard due to IP/MPLS TTL check error when do L3 edit";
        case EPE_OAM_EDGE_PORT_DIS:
            return "EPE_OAM_EDGE_PORT_DIS, No MEP/MIP at edge port for OAM packet";
        case EPE_NAT_PT_ICMP_ERR:
            return "EPE_NAT_PT_ICMP_ERR, NAT/PT ICMP error";
        case EPE_LATENCY_DISCARD:
            return "EPE_LATENCY_DISCARD, Packet discard due to latency is too long";
        case EPE_LOCAL_OAM_DIS:
            return "EPE_LOCAL_OAM_DIS, Local OAM discard(no use)";
        case EPE_OAM_FILTERING_DIS:
            return "EPE_OAM_FILTERING_DIS, OAM packet Port/STP/VLAN filter discard or forward link OAM packet";
        case EPE_SAME_IPDA_IPSA_DIS:
            return "EPE_SAME_IPDA_IPSA_DIS, Same MAC DA SA or IP DA SA";
        case EPE_SD_CHK_DISCARD:
            return "EPE_SD_CHK_DISCARD, Signal degrade discard";
        case EPE_TRILL_PLD_OP_DIS:
            return "EPE_TRILL_PLD_OP_DIS, TRILL payload operation discard(no use)";
        case EPE_RESERVED2:
            return "EPE_RESERVED2, Epe reserved2 discard";
        case EPE_DS_NEXTHOP_DATA_VIOLATE:
            return "EPE_DS_NEXTHOP_DATA_VIOLATE, Packet discard due to DsNextHop data violate";
        case EPE_DEST_VLAN_PTR_DIS:
            return "EPE_DEST_VLAN_PTR_DIS, Packet discard due to dest vlan index is invalid";
        case EPE_DS_L3EDIT_DATA_VIOLATE1:
            return "EPE_DS_L3EDIT_DATA_VIOLATE1, Discard by L3 edit or inner L2 edit violate";
        case EPE_DS_L3EDIT_DATA_VIOLATE2:
            return "EPE_DS_L3EDIT_DATA_VIOLATE2, Discard by L3 edit or inner L2 edit violate";
        case EPE_DS_L3EDITNAT_DATA_VIOLATE:
            return "EPE_DS_L3EDITNAT_DATA_VIOLATE, Discard by L3 edit or inner L2 edit violate";
        case EPE_DS_L2EDIT_DATA_VIOLATE1:
            return "EPE_DS_L2EDIT_DATA_VIOLATE1, Discard by L2 edit config";
        case EPE_DS_L2EDIT_DATA_VIOLATE2:
            return "EPE_DS_L2EDIT_DATA_VIOLATE2, Discard by L2 edit violate";
        case EPE_PKT_HDR_C2C_TTL_ZERO:
            return "EPE_PKT_HDR_C2C_TTL_ZERO, Packet discard due to packetHeader C2C TTL zero(no use)";
        case EPE_PT_UDP_CHKSUM_ZERO:
            return "EPE_PT_UDP_CHKSUM_ZERO, Packet discard due to PT/UDP checksum is zero for NAT";
        case EPE_OAM_TO_LOCAL_DIS:
            return "EPE_OAM_TO_LOCAL_DIS, Packet is sent to local up MEP";
        case EPE_HARD_ERROR_DIS:
            return "EPE_HARD_ERROR_DIS, Packet discard due to hardware error";
        case EPE_MICROFLOW_POLICING_FAIL_DROP:
            return "EPE_MICROFLOW_POLICING_FAIL_DROP, Microflow policing discard";
        case EPE_ARP_MISS_DISCARD:
            return "EPE_ARP_MISS_DISCARD, ARP miss discard";
        case EPE_ILLEGAL_PACKET_TO_E2I_LOOP_CHANNEL:
            return "EPE_ILLEGAL_PACKET_TO_E2I_LOOP_CHANNEL, Packet discard due to illegal packet to e2iloop channel";
        case EPE_UNKNOWN_DOT11_PACKET_DISCARD:
            return "EPE_UNKNOWN_DOT11_PACKET_DISCARD, Unknow dot1ae packet discard";
        case EPE_DOT1AE_PN_OVERFLOW:
            return "EPE_DOT1AE_PN_OVERFLOW, dot1ae pn overflow";
        case EPE_PON_DOWNSTREAM_CHECK_FAIL:
            return "EPE_PON_DOWNSTREAM_CHECK_FAIL, pon downstream check fail";
        case EPE_MIN_PKT_LEN_ERR:
            return "EPE_MIN_PKT_LEN_ERR, Packet discard due to min length check error";
        case EPE_ELOG_ABORTED_PKT:
            return "EPE_ELOG_ABORTED_PKT, Packet discard due to elog aborted";
        case EPE_ELOG_DROPPED_PKT:
            return "EPE_ELOG_DROPPED_PKT, Packet discard due to elog dropped";
       case EPE_HW_HAR_ADJ:
            return "EPE_HW_HAR_ADJ, Some hardware error at header adjust";
        case EPE_HW_NEXT_HOP:
            return "EPE_HW_NEXT_HOP, Some hardware error occurred at nexthop mapper";
        case EPE_HW_L2_EDIT:
            return "EPE_HW_L2_EDIT, Some hardware error occurred at l2 edit";
        case EPE_HW_L3_EDIT:
            return "EPE_HW_L3_EDIT, Some hardware error occurred at l3 edit";
        case EPE_HW_INNER_L2:
            return "EPE_HW_INNER_L2, Some hardware error occurred at inner l2 edit";
        case EPE_HW_PAYLOAD:
            return "EPE_HW_PAYLOAD, Some hardware error occurred when edit payload";
        case EPE_HW_ACL_OAM:
            return "EPE_HW_ACL_OAM, Some hardware error occurred when do OAM lookup";
        case EPE_HW_CLASS:
            return "EPE_HW_CLASS, Some hardware error occurred at classification";
        case EPE_HW_OAM:
            return "EPE_HW_OAM, Some hardware error occurred when do OAM process";
        case EPE_HW_EDIT:
            return "EPE_HW_EDIT, Some hardware error occurred when do edit";
        case BUFSTORE_ABORT_TOTAL:
            return "BUFSTORE_ABORT_TOTAL, Packet discard in BSR and is due to crc/ecc/packet len error or miss EOP or pkt buffer is full ";
        case BUFSTORE_LEN_ERROR:
            return "BUFSTORE_LEN_ERROR, Packet discard in BSR and is due to maximum or minimum packet length check";
        case BUFSTORE_IRM_RESRC:
            return "BUFSTORE_IRM_RESRC, Packet discard in BSR and is due to IRM no resource";
        case BUFSTORE_DATA_ERR:
            return "BUFSTORE_DATA_ERR, Packet discard in BSR and is due to crc error or ecc error";
        case BUFSTORE_CHIP_MISMATCH:
            return "BUFSTORE_CHIP_MISMATCH, Packet discard in BSR and is due to chip id mismatch";
        case BUFSTORE_NO_BUFF:
            return "BUFSTORE_NO_BUFF, Packet discard in BSR and is due to no pkt buffer";
        case BUFSTORE_NO_EOP:
            return "BUFSTORE_NO_EOP, Packet discard in BSR and is due to missing EOP ";
        case METFIFO_STALL_TO_BS_DROP:
            return "METFIFO_STALL_TO_BS_DROP, Packet discard in BSR and is due to the copy ability of MET";
        case BUFSTORE_SLIENT_TOTAL:
            return "BUFSTORE_SLIENT_TOTAL, Packet discard in BSR and is due to resource check fail";
        case TO_BUFSTORE_FROM_DMA:
            return "TO_BUFSTORE_FROM_DMA, Packet from DMA discard in BSR and is due to Bufstore error";
        case TO_BUFSTORE_FROM_NETRX:
            return "TO_BUFSTORE_FROM_NETRX, Packet from NetRx discard in BSR and is due to Bufstore error";
        case TO_BUFSTORE_FROM_OAM:
            return "TO_BUFSTORE_FROM_OAM, Packet from OAM discard in BSR and is due to Bufstore error";
        case BUFSTORE_OUT_SOP_PKT_LEN_ERR:
            return "BUFSTORE_OUT_SOP_PKT_LEN_ERR, Packet from BUFSTORE discard in BSR and is due to SOP length error ";
        case METFIFO_MC_FROM_DMA:
            return "METFIFO_MC_FROM_DMA, Mcast packet from DMA discard in BSR and is due to the copy ability of MET";
        case METFIFO_UC_FROM_DMA:
            return "METFIFO_UC_FROM_DMA, Ucast packet from DMA discard in BSR and is due to the copy ability of MET";
        case METFIFO_MC_FROM_ELOOP_MCAST:
            return "METFIFO_MC_FROM_ELOOP_MCAST, Eloop mcast packet discard in BSR and is due to the copy ability of MET";
        case METFIFO_UC_FROM_ELOOP_UCAST:
            return "METFIFO_UC_FROM_ELOOP_UCAST, Eloop ucast packet discard in BSR and is due to the copy ability of MET";
        case METFIFO_MC_FROM_IPE_CUT_THR:
            return "METFIFO_MC_FROM_IPE_CUT_THR, Mcast packet discard in BSR and is due to the copy ability of MET in cutthrough model";
        case METFIFO_UC_FROM_IPE_CUT_THR:
            return "METFIFO_UC_FROM_IPE_CUT_THR, Ucast packet discard in BSR and is due to the copy ability of MET in cutthrough model";
        case METFIFO_MC_FROM_IPE:
            return "METFIFO_MC_FROM_IPE, IPE mcast packet discard in BSR and is due to the copy ability of MET";
        case METFIFO_UC_FROM_IPE:
            return "METFIFO_UC_FROM_IPE, IPE ucast packet discard in BSR and is due to the copy ability of MET";
        case METFIFO_MC_FROM_OAM:
            return "METFIFO_MC_FROM_OAM, OAM mcast packet discard in BSR and is due to the copy ability of MET";
        case METFIFO_UC_FROM_OAM:
            return "METFIFO_UC_FROM_OAM, OAM ucast packet discard in BSR and is due to the copy ability of MET";
        case BUFRETRV_FROM_DEQ_MSG_ERR:
            return "BUFRETRV_FROM_DEQ_MSG_ERR, Packet from DEQ discard in BSR and is due to MSG error ";
        case BUFRETRV_FROM_QMGR_LEN_ERR:
            return "BUFRETRV_FROM_QMGR_LEN_ERR, Packet from QMGR discard in BSR and is due to length error";
        case BUFRETRV_OUT_DROP:
            return "BUFRETRV_OUT_DROP, Packet from bufRetrv discard in BSR";
        case ENQ_WRED_DROP:
            return "ENQ_WRED_DROP, ENQ packet discard in BSR and is due to WRED error";
        case ENQ_TOTAL_DROP:
            return "ENQ_TOTAL_DROP, ENQ packe total discard";
        case ENQ_NO_QUEUE_ENTRY:
            return "ENQ_NO_QUEUE_ENTRY, ENQ packet discard in BSR and is due to no ptr";
        case ENQ_PORT_NO_BUFF:
            return "ENQ_PORT_NO_BUFF, ENQ packet discard in BSR and is due to port no buffer";
        case ENQ_QUEUE_NO_BUFF:
            return "ENQ_QUEUE_NO_BUFF, ENQ packet discard in BSR and is due to queue no buffer";
        case ENQ_SC_NO_BUFF:
            return "ENQ_SC_NO_BUFF, ENQ packet discard in BSR and is due to service class no buffer";
        case ENQ_SPAN_NO_BUFF:
            return "ENQ_SPAN_NO_BUFF, ENQ packet discard in BSR and is due to span no buffer";
        case ENQ_TOTAL_NO_BUFF:
            return "ENQ_TOTAL_NO_BUFF, ENQ packet discard in BSR and is due to no total buffer";
        case ENQ_FWD_DROP:
            return "ENQ_FWD_DROP, ENQ packet discard in BSR and is due to foward drop";
        case ENQ_FWD_DROP_CFLEX_ISOLATE_BLOCK:
            return "ENQ_FWD_DROP_CFLEX_ISOLATE_BLOCK, ENQ packet discard in BSR and is due to isolated port when stacking";
        case ENQ_FWD_DROP_CHAN_INVALID:
            return "ENQ_FWD_DROP_CHAN_INVALID, ENQ packet discard in BSR and is due to invalid channel id";
        case ENQ_FWD_DROP_TOTAL:
            return "ENQ_FWD_DROP_TOTAL, ENQ packet discard in BSR and is due to fowarding drop";
        case ENQ_FWD_DROP_FROM_LAG:
            return "ENQ_FWD_DROP_FROM_LAG, ENQ packet discard in BSR and is due to linkagg error";
        case ENQ_FWD_DROP_FROM_LAG_ERR:
            return "ENQ_FWD_DROP_FROM_LAG_ERR, ENQ packet discard in BSR and is due to crc error or memory error when do linkagg";
        case ENQ_FWD_DROP_FROM_LAG_MC:
            return "ENQ_FWD_DROP_FROM_LAG_MC, ENQ packet discard in BSR and is due to invalid dest chip id when do mcast linkagg";
        case ENQ_FWD_DROP_FROM_LAG_NO_MEM:
            return "ENQ_FWD_DROP_FROM_LAG_NO_MEM, ENQ packet discard in BSR and is due to no linkagg member";
        case ENQ_FWD_DROP_PORT_ISOLATE_BLOCK:
            return "ENQ_FWD_DROP_PORT_ISOLATE_BLOCK, ENQ packet discard in BSR and is due to isolated port";
        case ENQ_FWD_DROP_RSV_CHAN_DROP:
            return "ENQ_FWD_DROP_RSV_CHAN_DROP, ENQ packet discard in BSR and is due to drop channel";
        case ENQ_FWD_DROP_FROM_STACKING_LAG:
            return "ENQ_FWD_DROP_FROM_STACKING_LAG, ENQ packet discard in BSR and is due to stacking linkagg error";
        case NETRX_NO_BUFFER:
            return "NETRX_NO_BUFFER, Packet discard due to no buffer in port oversub model";
        case NETRX_LEN_ERROR:
            return "NETRX_LEN_ERROR, Packet discard due to the failure of max or min pkt len check";
        case NETRX_PKT_ERROR:
            return "NETRX_PKT_ERROR, Packet error discard";
        case NETRX_BPDU_ERROR:
            return "NETRX_BPDU_ERROR, BPDU discard";
        case NETRX_FRAME_ERROR:
            return "NETRX_FRAME_ERROR, Packet discard due to misssing sop or eop";
        case NETTX_MIN_LEN:
            return "NETTX_MIN_LEN, Packet discard due to smaller than the cfgMinPktLen";
        case NETTX_NO_BUFFER:
            return "NETTX_NO_BUFFER, Packet discard due to the use up of PktBuffer in NETTX";
        case NETTX_SOP_EOP:
            return "NETTX_SOP_EOP, Packet discard due to missing EOP or SOP";
        case NETTX_TX_ERROR:
            return "NETTX_TX_ERROR, The number of error packets sending to MAC";
        case OAM_HW_ERROR:
            return "OAM_HW_ERROR, Some hardware error occured";
        case OAM_EXCEPTION:
            return "OAM_EXCEPTION, Some exception occured when process OAM packet";
        default:
            sal_sprintf(reason_desc, "Reason id:%d", reason_id);
            return reason_desc;
    }
}


STATIC int32
_sys_tsingma_diag_pkt_tracce_drop_map(ctc_diag_pkt_trace_result_t* p_rslt, uint32 discard, uint32 discard_type)
{
    uint8 pos = p_rslt->position;
    const char* drop_desc = NULL;
    char  str_print[64] = {0};
    uint16 index = 0;
    uint16 str_desc_len = 0;

    if (0 == discard)
    {
        return CTC_E_NONE;
    }
    p_rslt->dest_type = CTC_DIAG_DEST_DROP;

    if (CTC_DIAG_TRACE_POS_IPE == pos)
    {
        drop_desc = _sys_tsingma_diag_get_reason_desc(discard_type + IPE_FEATURE_START);
        p_rslt->drop_reason = discard_type + IPE_FEATURE_START;
    }
    else if (CTC_DIAG_TRACE_POS_EPE == pos)
    {
        drop_desc = _sys_tsingma_diag_get_reason_desc(discard_type + EPE_FEATURE_START);
        p_rslt->drop_reason = discard_type + EPE_FEATURE_START;
    }
    p_rslt->flags |= CTC_DIAG_TRACE_RSTL_FLAG_DROP_REASON;
    p_rslt->flags |= CTC_DIAG_TRACE_RSTL_FLAG_DROP_DESC;
    str_desc_len = sal_strlen(drop_desc);
    if (str_desc_len)
    {
        sal_memcpy(p_rslt->drop_desc, drop_desc, (str_desc_len < CTC_DIAG_REASON_DESC_LEN) ? str_desc_len : CTC_DIAG_REASON_DESC_LEN);
    }
    while (index < str_desc_len)
    {
        if ((drop_desc[index] == ',')
            || (index >= 64))
        {
            break;
        }
        str_print[index] = drop_desc[index];
        index++;
    }
    DIAG_INFO_CHAR(pos, (CTC_DIAG_TRACE_POS_IPE == pos) ? "IPE-DROP" : "EPE-DROP", str_print)
    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_diag_pkt_tracce_exception_map(ctc_diag_pkt_trace_result_t* p_rslt,
                                          uint32 excp, uint32 excp_index, uint32 excp_sub_index)
{
    uint8 pos = p_rslt->position;
    char* exception_desc[4] =
    {
        "Reserved",         /* IPEEXCEPTIONINDEX_RESERVED               CBit(3,'h',"0", 1)*/
        "Bridge exception", /* IPEEXCEPTIONINDEX_BRIDGE_EXCEPTION       CBit(3,'h',"1", 1)*/
        "Route exception",  /* IPEEXCEPTIONINDEX_ROUTE_EXCEPTION        CBit(3,'h',"2", 1)*/
        "Other exception"   /* IPEEXCEPTIONINDEX_OTHER_EXCEPTION        CBit(3,'h',"3", 1)*/
    };

    if (0 == excp)
    {
        return CTC_E_NONE;
    }

    p_rslt->dest_type = CTC_DIAG_DEST_CPU;
    DIAG_INFO_CHAR(pos, "exception-type",exception_desc[excp_index])
    DIAG_INFO_UINT32(pos, "exception-index", excp_index)
    DIAG_INFO_UINT32(pos, "exception-sub-index", excp_sub_index)

    return CTC_E_NONE;
}


STATIC int32
_sys_tsingma_diag_pkt_trace_get_parser_info(uint8 lchip, uint8 type, ctc_diag_pkt_trace_result_t* p_rslt)
{
    uint32 cmd = 0;
    uint32 value = 0;
    uint32 value1 = 0;
    char str[64] = {0};
    hw_mac_addr_t hw_mac = {0};
    ip_addr_t hw_ipv4 = 0;
    ipv6_addr_t hw_ipv6 = {0};
    uint8 pos = CTC_DIAG_TRACE_POS_IPE;
    char* str_parser[] = {"1st Parsing", "2nd Parsing", "Egress Parsing"};
    char* str_l3_type[] = {"NONE", "IP", "IPV4", "IPV6", "MPLS", "MPLSUP", "ARP", "FCOE", "TRILL",
                           "ETHEROAM", "SLOWPROTO", "CMAC", "PTP", "DOT1AE", "SATPDU", "FLEXIBLE"};

    char* str_l4_type[] = {"NONE", "TCP", "UDP", "GRE", "IPINIP", "V6INIP", "ICMP", "IGMP", "IPINV6",
                           "RESERVED", "ACHOAM", "V6INV6", "RDP", "SCTP", "DCCP", "FLEXIBLE"};
    DbgParserFromIpeHdrAdjInfo_m parser_result_tmp;
    ParserResult_m parser_result;

    sal_memset(&parser_result_tmp, 0, sizeof(DbgParserFromIpeHdrAdjInfo_m));
    sal_memset(&parser_result, 0, sizeof(ParserResult_m));
    if (0 == type)       /*ipe 1st parser*/
    {
        cmd = DRV_IOR(DbgParserFromIpeHdrAdjInfo_t, DRV_ENTRY_FLAG);
    }
    else if (1 == type)  /*ipe 2nd parser*/
    {
        cmd = DRV_IOR(DbgIpeMplsDecapInfo_t, DbgIpeMplsDecapInfo_secondParserEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        if(!value)
        {
            return CTC_E_NONE;
        }
        cmd = DRV_IOR(DbgParserFromIpeIntfInfo_t, DRV_ENTRY_FLAG);
    }
    else if (2 == type)  /*epe parser*/
    {
        cmd = DRV_IOR(DbgParserFromEpeHdrAdjInfo_t, DRV_ENTRY_FLAG);
    }
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &parser_result_tmp));

    if(!GetDbgParserFromIpeHdrAdjInfo(V, valid_f, &parser_result_tmp))
    {
        return CTC_E_NONE;
    }
    p_rslt->position = (2 == type) ? CTC_DIAG_TRACE_POS_EPE : CTC_DIAG_TRACE_POS_IPE;
    DIAG_INFO_NONE(pos, str_parser[type])
    GetDbgParserFromIpeHdrAdjInfo(A, data_f, &parser_result_tmp, &parser_result);

    GetParserResult(A, macDa_f,        &parser_result, hw_mac);
    DIAG_INFO_MAC(pos, "mac-da", hw_mac)
    GetParserResult(A, macSa_f,        &parser_result, hw_mac);
    DIAG_INFO_MAC(pos, "mac-sa", hw_mac)

    if (GetParserResult(V, svlanIdValid_f, &parser_result))
    {
        DIAG_INFO_UINT32(pos, "svlan-id", GetParserResult(V, svlanId_f, &parser_result))
        DIAG_INFO_UINT32(pos, "stag-cos", GetParserResult(V, stagCos_f, &parser_result))
    }
    if (GetParserResult(V, cvlanIdValid_f, &parser_result))
    {
        DIAG_INFO_UINT32(pos, "cvlan-id", GetParserResult(V, cvlanId_f, &parser_result))
        DIAG_INFO_UINT32(pos, "cvlan-id", GetParserResult(V, ctagCos_f, &parser_result))
    }

    DIAG_INFO_U32_HEX(pos, "ether-type", GetParserResult(V, etherType_f, &parser_result))
    value = GetParserResult(V, layer3Type_f, &parser_result);
    if (value < MAX_CTC_PARSER_L3_TYPE)
    {
        sal_sprintf(str, "L3TYPE_%s", str_l3_type[value]);
        DIAG_INFO_CHAR(pos, "layer3-type", str)
    }
    switch (value)
    {
        case CTC_PARSER_L3_TYPE_IPV4:
            DIAG_INFO_U32_HEX(pos, "ip-protocol", GetParserResult(V, layer3HeaderProtocol_f, &parser_result))
            GetParserResult(A, uL3Dest_gIpv4_ipDa_f,   &parser_result, &hw_ipv4);
            DIAG_INFO_IP4(pos, "ip-da", hw_ipv4)
            GetParserResult(A, uL3Source_gIpv4_ipSa_f, &parser_result, &hw_ipv4);
            DIAG_INFO_IP4(pos, "ip-sa", hw_ipv4)
            break;
        case CTC_PARSER_L3_TYPE_IPV6:
            DIAG_INFO_U32_HEX(pos, "ip-protocol", GetParserResult(V, layer3HeaderProtocol_f, &parser_result))
            GetParserResult(A, uL3Dest_gIpv6_ipDa_f,   &parser_result, hw_ipv6);
            DIAG_INFO_IP6(pos, "ip6-da", hw_ipv6)
            GetParserResult(A, uL3Source_gIpv6_ipSa_f, &parser_result, hw_ipv6);
            DIAG_INFO_IP6(pos, "ip6-sa", hw_ipv6)
            break;
        case CTC_PARSER_L3_TYPE_MPLS:
            value1 = GetParserResult(V, uL3Tos_gMpls_labelNum_f, &parser_result);
            switch (value1)
            {
                case 8:
                    GetParserResult(A, uL3Source_gMpls_mplsLabel7_f, &parser_result, &value);
                    DIAG_INFO_UINT32(pos, "mpls-label-7", value);
                case 7:
                    GetParserResult(A, uL3Source_gMpls_mplsLabel6_f, &parser_result, &value);
                    DIAG_INFO_UINT32(pos, "mpls-label-6", value);
                case 6:
                    GetParserResult(A, uL3Source_gMpls_mplsLabel5_f, &parser_result, &value);
                    DIAG_INFO_UINT32(pos, "mpls-label-5", value);
                case 5:
                    GetParserResult(A, uL3Source_gMpls_mplsLabel4_f, &parser_result, &value);
                    DIAG_INFO_UINT32(pos, "mpls-label-4", value);
                case 4:
                    GetParserResult(A, uL3Dest_gMpls_mplsLabel3_f,   &parser_result, &value);
                    DIAG_INFO_UINT32(pos, "mpls-label-3", value);
                case 3:
                    GetParserResult(A, uL3Dest_gMpls_mplsLabel2_f,   &parser_result, &value);
                    DIAG_INFO_UINT32(pos, "mpls-label-2", value);
                case 2:
                    GetParserResult(A, uL3Dest_gMpls_mplsLabel1_f,   &parser_result, &value);
                    DIAG_INFO_UINT32(pos, "mpls-label-1", value);
                case 1:
                    GetParserResult(A, uL3Dest_gMpls_mplsLabel0_f,   &parser_result, &value);
                    DIAG_INFO_UINT32(pos, "mpls-label-0", value);
                case 0:
                    break;
            }
            break;
        case CTC_PARSER_L3_TYPE_ARP:
            DIAG_INFO_UINT32(pos, "arp-op-code", GetParserResult(V, uL3Dest_gArp_arpOpCode_f, &parser_result))
            DIAG_INFO_UINT32(pos, "arp-protocol-type", GetParserResult(V, uL3Source_gArp_protocolType_f, &parser_result))
            GetParserResult(A, uL3Dest_gArp_targetIp_f, &parser_result, &hw_ipv4);
            DIAG_INFO_IP4(pos, "arp-target-ip", hw_ipv4)
            GetParserResult(A, uL3Source_gArp_senderIp_f, &parser_result, &hw_ipv4);
            DIAG_INFO_IP4(pos, "arp-sender-ip", hw_ipv4)
            GetParserResult(A, uL3Dest_gArp_targetMac_f, &parser_result, hw_mac);
            DIAG_INFO_MAC(pos, "arp-target-mac", hw_mac)
            GetParserResult(A, uL3Source_gArp_senderMac_f, &parser_result, hw_mac);
            DIAG_INFO_MAC(pos, "arp-sender-mac", hw_mac)
            break;
        case CTC_PARSER_L3_TYPE_FCOE:
            DIAG_INFO_UINT32(pos, "fcoe-did", GetParserResult(V, uL3Dest_gFcoe_fcoeDid_f, &parser_result))
            DIAG_INFO_UINT32(pos, "fcoe-sid", GetParserResult(V, uL3Source_gFcoe_fcoeSid_f, &parser_result))
            break;
        case CTC_PARSER_L3_TYPE_TRILL:
            DIAG_INFO_U32_HEX(pos, "ingress-nickname", GetParserResult(V, uL3Source_gTrill_ingressNickname_f, &parser_result))
            DIAG_INFO_U32_HEX(pos, "egress-nickname", GetParserResult(V, uL3Dest_gTrill_egressNickname_f, &parser_result))
            DIAG_INFO_UINT32(pos, "trill-is-esadi", GetParserResult(V, uL3Dest_gTrill_isEsadi_f, &parser_result))
            DIAG_INFO_UINT32(pos, "trill-is-bfd", GetParserResult(V, uL3Dest_gTrill_isTrillBfd_f, &parser_result))
            DIAG_INFO_UINT32(pos, "trill-is-bfd-echo", GetParserResult(V, uL3Dest_gTrill_isTrillBfdEcho_f, &parser_result))
            DIAG_INFO_UINT32(pos, "is-channel", GetParserResult(V, uL3Dest_gTrill_isTrillChannel_f, &parser_result))
            DIAG_INFO_UINT32(pos, "bfd-my-discrim", GetParserResult(V, uL3Dest_gTrill_trillBfdMyDiscriminator_f, &parser_result))
            if (GetParserResult(V, uL3Dest_gTrill_trillInnerVlanValid_f, &parser_result))
            {
                DIAG_INFO_UINT32(pos, "inner-vlan-id", GetParserResult(V, uL3Dest_gTrill_trillInnerVlanId_f, &parser_result))
            }
            DIAG_INFO_UINT32(pos, "trill-length", GetParserResult(V, uL3Dest_gTrill_trillLength_f, &parser_result))
            DIAG_INFO_UINT32(pos, "multi-hop", GetParserResult(V, uL3Dest_gTrill_trillMultiHop_f, &parser_result))
            DIAG_INFO_UINT32(pos, "trill-multicast", GetParserResult(V, uL3Dest_gTrill_trillMulticast_f, &parser_result))
            DIAG_INFO_UINT32(pos, "trill-version", GetParserResult(V, uL3Dest_gTrill_trillVersion_f, &parser_result))
            break;
        case CTC_PARSER_L3_TYPE_ETHER_OAM:
            DIAG_INFO_UINT32(pos, "ether-oam-level", GetParserResult(V, uL3Dest_gEtherOam_etherOamLevel_f, &parser_result))
            DIAG_INFO_UINT32(pos, "ether-oam-op-code", GetParserResult(V, uL3Dest_gEtherOam_etherOamOpCode_f, &parser_result))
            DIAG_INFO_UINT32(pos, "ether-oam-version", GetParserResult(V, uL3Dest_gEtherOam_etherOamVersion_f, &parser_result))
            break;
        case CTC_PARSER_L3_TYPE_SLOW_PROTO:
            DIAG_INFO_UINT32(pos, "slow-proto-code", GetParserResult(V, uL3Dest_gSlowProto_slowProtocolCode_f, &parser_result))
            DIAG_INFO_U32_HEX(pos, "slow-proto-flags", GetParserResult(V, uL3Dest_gSlowProto_slowProtocolFlags_f, &parser_result))
            DIAG_INFO_UINT32(pos, "slow-proto-subtype", GetParserResult(V, uL3Dest_gSlowProto_slowProtocolSubType_f, &parser_result))
            break;
        case CTC_PARSER_L3_TYPE_PTP:
            DIAG_INFO_UINT32(pos, "ptp-message", GetParserResult(V, uL3Dest_gPtp_ptpMessageType_f, &parser_result))
            DIAG_INFO_UINT32(pos, "ptp-version", GetParserResult(V, uL3Dest_gPtp_ptpVersion_f, &parser_result))
            break;
        case CTC_PARSER_L3_TYPE_DOT1AE:
            DIAG_INFO_UINT32(pos, "dot1ae-an", GetParserResult(V, uL3Dest_gDot1Ae_secTagAn_f, &parser_result))
            DIAG_INFO_UINT32(pos, "dot1ae-es", GetParserResult(V, uL3Dest_gDot1Ae_secTagEs_f, &parser_result))
            DIAG_INFO_UINT32(pos, "dot1ae-pn", GetParserResult(V, uL3Dest_gDot1Ae_secTagPn_f, &parser_result))
            DIAG_INFO_UINT32(pos, "dot1ae-sc", GetParserResult(V, uL3Dest_gDot1Ae_secTagSc_f, &parser_result))
            DIAG_INFO_UINT32(pos, "dot1ae-sci", GetParserResult(V, uL3Dest_gDot1Ae_secTagSci_f, &parser_result))
            DIAG_INFO_UINT32(pos, "dot1ae-sl", GetParserResult(V, uL3Dest_gDot1Ae_secTagSl_f, &parser_result))
            DIAG_INFO_UINT32(pos, "dot1ae-unknow", GetParserResult(V, uL3Dest_gDot1Ae_unknownDot1AePacket_f, &parser_result))
            break;
        case CTC_PARSER_L3_TYPE_SATPDU:
            DIAG_INFO_UINT32(pos, "satpdu-mef-oui", GetParserResult(V, uL3Dest_gSatPdu_mefOui_f, &parser_result))
            DIAG_INFO_UINT32(pos, "satpdu-oui-subtype", GetParserResult(V, uL3Dest_gSatPdu_ouiSubType_f, &parser_result))
            DIAG_INFO_UINT32(pos, "satpdu-pdu-byte", GetParserResult(V, uL3Dest_gSatPdu_pduByte_f, &parser_result))
            break;
        default:
            break;
    }

    value = GetParserResult(V, layer4Type_f, &parser_result);
    if (value < MAX_CTC_PARSER_L4_TYPE)
    {
        sal_sprintf(str, "L4TYPE_%s", str_l4_type[value]);
        DIAG_INFO_CHAR(pos, "layer4-type", str)
    }
    switch (value)
    {
        case CTC_PARSER_L4_TYPE_GRE:
            DIAG_INFO_UINT32(pos, "gre-flags", GetParserResult(V, uL4Source_gGre_greFlags_f, &parser_result))
            DIAG_INFO_U32_HEX(pos, "gre-proto-type", GetParserResult(V, uL4Dest_gGre_greProtocolType_f, &parser_result))
            break;
        case CTC_PARSER_L4_TYPE_TCP:
        case CTC_PARSER_L4_TYPE_UDP:
        case CTC_PARSER_L4_TYPE_RDP:
        case CTC_PARSER_L4_TYPE_SCTP:
        case CTC_PARSER_L4_TYPE_DCCP:
            DIAG_INFO_UINT32(pos, "l4-src-port", GetParserResult(V, uL4Dest_gPort_l4DestPort_f, &parser_result))
            DIAG_INFO_UINT32(pos, "l4-dst-port", GetParserResult(V, uL4Source_gPort_l4SourcePort_f, &parser_result))
            break;
        default:
            break;
    }
    if (GetParserResult(V, udfValid_f, &parser_result))
    {
        DIAG_INFO_UINT32(pos, "udf-hit-index", GetParserResult(V, udfHitIndex_f, &parser_result))
    }
    return CTC_E_NONE;
}


STATIC char*
_sys_tsingma_diag_get_igr_scl_key_desc(uint8 lchip, uint32 key_type, uint8 lk_level)
{
    lk_level = !lk_level;
    switch (key_type)
    {
        case USERIDHASHTYPE_DISABLE:
            return "";
        case USERIDHASHTYPE_DOUBLEVLANPORT:
            return lk_level ? "DsUserIdDoubleVlanPortHashKey" :"DsUserId1DoubleVlanPortHashKey";
        case USERIDHASHTYPE_SVLANPORT:
            return lk_level ? "DsUserIdSvlanPortHashKey" : "DsUserId1SvlanPortHashKey";
        case USERIDHASHTYPE_CVLANPORT:
            return lk_level ? "DsUserIdCvlanPortHashKey" : "DsUserId1CvlanPortHashKey";
        case USERIDHASHTYPE_SVLANCOSPORT:
            return lk_level ? "DsUserIdSvlanCosPortHashKey" : "DsUserId1SvlanCosPortHashKey";
        case USERIDHASHTYPE_CVLANCOSPORT:
            return lk_level ? "DsUserIdCvlanCosPortHashKey" : "DsUserId1CvlanCosPortHashKey";
        case USERIDHASHTYPE_MACPORT:
            return lk_level ? "DsUserIdMacPortHashKey" : "DsUserId1MacPortHashKey";
        case USERIDHASHTYPE_IPV4PORT:
            return lk_level ? "DsUserIdIpv4PortHashKey" : "DsUserId1Ipv4PortHashKey";
        case USERIDHASHTYPE_MAC:
            return lk_level ? "DsUserIdMacHashKey" : "DsUserId1MacHashKey";
        case USERIDHASHTYPE_IPV4SA:
            return lk_level ? "DsUserIdIpv4SaHashKey" : "DsUserId1Ipv4SaHashKey";
        case USERIDHASHTYPE_PORT:
            return lk_level ? "DsUserIdPortHashKey" : "DsUserId1PortHashKey";
        case USERIDHASHTYPE_SVLANMACSA:
            return lk_level ? "DsUserIdSvlanMacSaHashKey" : "DsUserId1SvlanMacSaHashKey";
        case USERIDHASHTYPE_SVLAN:
            return lk_level ? "DsUserIdSvlanHashKey" : "DsUserId1SvlanHashKey";
        case USERIDHASHTYPE_ECIDNAMESPACE:
            return lk_level ? "DsUserIdEcidNameSpaceHashKey" : "DsUserId1EcidNameSpaceHashKey";
        case USERIDHASHTYPE_INGECIDNAMESPACE:
            return lk_level ? "DsUserIdIngEcidNameSpaceHashKey" : "DsUserId1IngEcidNameSpaceHashKey";
        case USERIDHASHTYPE_IPV6SA:
            return lk_level ? "DsUserIdIpv6SaHashKey" : "DsUserId1Ipv6SaHashKey";
        case USERIDHASHTYPE_IPV6PORT:
            return lk_level ? "DsUserIdIpv6PortHashKey" : "DsUserId1Ipv6PortHashKey";
        case USERIDHASHTYPE_CAPWAPSTASTATUS:
            return lk_level ? "DsUserIdCapwapStaStatusHashKey" : "DsUserId1CapwapStaStatusHashKey";
        case USERIDHASHTYPE_CAPWAPSTASTATUSMC:
            return lk_level ? "DsUserIdCapwapStaStatusMcHashKey" : "DsUserId1CapwapStaStatusMcHashKey";
        case USERIDHASHTYPE_CAPWAPMACDAFORWARD:
            return lk_level ? "DsUserIdCapwapMacDaForwardHashKey" : "DsUserId1CapwapMacDaForwardHashKey";
        case USERIDHASHTYPE_CAPWAPVLANFORWARD:
            return lk_level ? "DsUserIdCapwapVlanForwardHashKey" : "DsUserId1CapwapVlanForwardHashKey";
        case USERIDHASHTYPE_TUNNELIPV4:
            return lk_level ? "DsUserIdTunnelIpv4HashKey" : "DsUserId1TunnelIpv4HashKey";
        case USERIDHASHTYPE_TUNNELIPV4GREKEY:
            return lk_level ? "DsUserIdTunnelIpv4GreKeyHashKey" : "DsUserId1TunnelIpv4GreKeyHashKey";
        case USERIDHASHTYPE_TUNNELIPV4UDP:
            return lk_level ? "DsUserIdTunnelIpv4UdpHashKey" : "DsUserId1TunnelIpv4UdpHashKey";
        case USERIDHASHTYPE_TUNNELPBB:
            return "";
        case USERIDHASHTYPE_TUNNELTRILLUCRPF:
            return lk_level ? "DsUserIdTunnelTrillUcRpfHashKey" : "DsUserId1TunnelTrillUcRpfHashKey";
        case USERIDHASHTYPE_TUNNELTRILLUCDECAP:
            return lk_level ? "DsUserIdTunnelTrillUcDecapHashKey" : "DsUserId1TunnelTrillUcDecapHashKey";
        case USERIDHASHTYPE_TUNNELTRILLMCRPF:
            return lk_level ? "DsUserIdTunnelTrillMcRpfHashKey" : "DsUserId1TunnelTrillMcRpfHashKey";
        case USERIDHASHTYPE_TUNNELTRILLMCDECAP:
            return lk_level ? "DsUserIdTunnelTrillMcDecapHashKey" : "DsUserId1TunnelTrillMcDecapHashKey";
        case USERIDHASHTYPE_TUNNELTRILLMCADJ:
            return lk_level ? "DsUserIdTunnelTrillMcAdjHashKey" : "DsUserId1TunnelTrillMcAdjHashKey";
        case USERIDHASHTYPE_TUNNELIPV4RPF:
            return lk_level ? "DsUserIdTunnelIpv4RpfHashKey" : "DsUserId1TunnelIpv4RpfHashKey";
        case USERIDHASHTYPE_TUNNELIPV4UCVXLANMODE0:
            return lk_level ? "DsUserIdTunnelIpv4UcVxlanMode0HashKey" : "DsUserId1TunnelIpv4UcVxlanMode0HashKey";
        case USERIDHASHTYPE_TUNNELIPV4UCVXLANMODE1:
            return lk_level ? "DsUserIdTunnelIpv4UcVxlanMode1HashKey" : "DsUserId1TunnelIpv4UcVxlanMode1HashKey";
        case USERIDHASHTYPE_TUNNELIPV6UCVXLANMODE0:
            return lk_level ? "DsUserIdTunnelIpv6UcVxlanMode0HashKey" : "DsUserId1TunnelIpv6UcVxlanMode0HashKey";
        case USERIDHASHTYPE_TUNNELIPV6UCVXLANMODE1:
            return lk_level ? "DsUserIdTunnelIpv6UcVxlanMode1HashKey" : "DsUserId1TunnelIpv6UcVxlanMode1HashKey";
        case USERIDHASHTYPE_TUNNELIPV4UCNVGREMODE0:
            return lk_level ? "DsUserIdTunnelIpv4UcNvgreMode0HashKey" : "DsUserId1TunnelIpv4UcNvgreMode0HashKey";
        case USERIDHASHTYPE_TUNNELIPV4UCNVGREMODE1:
            return lk_level ? "DsUserIdTunnelIpv4UcNvgreMode1HashKey" : "DsUserId1TunnelIpv4UcNvgreMode1HashKey";
        case USERIDHASHTYPE_TUNNELIPV6UCNVGREMODE0:
            return lk_level ? "DsUserIdTunnelIpv6UcNvgreMode0HashKey" : "DsUserId1TunnelIpv6UcNvgreMode0HashKey";
        case USERIDHASHTYPE_TUNNELIPV6UCNVGREMODE1:
            return lk_level ? "DsUserIdTunnelIpv6UcNvgreMode1HashKey" : "DsUserId1TunnelIpv6UcNvgreMode1HashKey";
        case USERIDHASHTYPE_TUNNELIPV4MCVXLANMODE0:
            return lk_level ? "DsUserIdTunnelIpv4McVxlanMode0HashKey" : "DsUserId1TunnelIpv4McVxlanMode0HashKey";
        case USERIDHASHTYPE_TUNNELIPV4VXLANMODE1:
            return lk_level ? "DsUserIdTunnelIpv4VxlanMode1HashKey" : "DsUserId1TunnelIpv4VxlanMode1HashKey";
        case USERIDHASHTYPE_TUNNELIPV6MCVXLANMODE0:
            return lk_level ? "DsUserIdTunnelIpv6McVxlanMode0HashKey" : "DsUserId1TunnelIpv6McVxlanMode0HashKey";
        case USERIDHASHTYPE_TUNNELIPV6MCVXLANMODE1:
            return lk_level ? "DsUserIdTunnelIpv6McVxlanMode1HashKey" : "DsUserId1TunnelIpv6McVxlanMode1HashKey";
        case USERIDHASHTYPE_TUNNELIPV4MCNVGREMODE0:
            return lk_level ? "DsUserIdTunnelIpv4McNvgreMode0HashKey" : "DsUserId1TunnelIpv4McNvgreMode0HashKey";
        case USERIDHASHTYPE_TUNNELIPV4NVGREMODE1:
            return lk_level ? "DsUserIdTunnelIpv4NvgreMode1HashKey" : "DsUserId1TunnelIpv4NvgreMode1HashKey";
        case USERIDHASHTYPE_TUNNELIPV6MCNVGREMODE0:
            return lk_level ? "DsUserIdTunnelIpv6McNvgreMode0HashKey" : "DsUserId1TunnelIpv6McNvgreMode0HashKey";
        case USERIDHASHTYPE_TUNNELIPV6MCNVGREMODE1:
            return lk_level ? "DsUserIdTunnelIpv6McNvgreMode1HashKey" : "DsUserId1TunnelIpv6McNvgreMode1HashKey";
        case USERIDHASHTYPE_TUNNELIPV4CAPWAP:
            return lk_level ? "DsUserIdTunnelIpv4CapwapHashKey" : "DsUserId1TunnelIpv4CapwapHashKey";
        case USERIDHASHTYPE_TUNNELIPV6CAPWAP:
            return lk_level ? "DsUserIdTunnelIpv6CapwapHashKey" : "DsUserId1TunnelIpv6CapwapHashKey";
        case USERIDHASHTYPE_TUNNELCAPWAPRMAC:
            return lk_level ? "DsUserIdTunnelCapwapRmacHashKey" : "DsUserId1TunnelCapwapRmacHashKey";
        case USERIDHASHTYPE_TUNNELCAPWAPRMACRID:
            return lk_level ? "DsUserIdTunnelCapwapRmacRidHashKey" : "DsUserId1TunnelCapwapRmacRidHashKey";
        case USERIDHASHTYPE_TUNNELMPLS:
            return lk_level ? "DsUserIdTunnelMplsHashKey" : "";
        default:
            if(key_type == DRV_ENUM(DRV_USERID_HASHTYPE_TUNNELIPV4DA))
            {
                return lk_level ? "DsUserIdTunnelIpv4DaHashKey" : "DsUserId1TunnelIpv4DaHashKey";
            }
            else if(key_type == DRV_ENUM(DRV_USERID_HASHTYPE_SCLFLOWL2))
            {
                return lk_level ? "DsUserIdSclFlowL2HashKey" :"DsUserId1SclFlowL2HashKey";
            }
            return "";
    }
}


STATIC int32
_sys_tsingma_diag_map_tcam_key_index(uint8 lchip, uint32 dbg_index, uint8 is_acl, uint8 lkup_level, uint32* o_key_index)
{
    uint8  tcam_blk_id = 0;
    uint16 local_index = 0;
    uint32 key_index = 0xFFFFFFFF;
    uint32 entry_num = 0;
    uint8  hit_blk_max_default = 0;
    uint32 acl_tbl_id[] = {DsAclQosMacKey160Ing0_t, DsAclQosMacKey160Ing1_t, DsAclQosMacKey160Ing2_t, DsAclQosMacKey160Ing3_t,
                           DsAclQosMacKey160Ing4_t, DsAclQosMacKey160Ing5_t, DsAclQosMacKey160Ing6_t, DsAclQosMacKey160Ing7_t};
    uint32 scl_tbl_id[] = {DsScl0L3Key160_t, DsScl1L3Key160_t,DsScl2L3Key160_t, DsScl3L3Key160_t};

    tcam_blk_id = dbg_index >> 11;
    local_index = dbg_index & 0x7FF;

    /*TM:USERID 4,IPE_ACL 8,EPE_ACL 3*/
    hit_blk_max_default = 15;
    if(tcam_blk_id < hit_blk_max_default)
    {
        key_index = local_index;
    }
    else  /*used share tcam*/
    {
        drv_usw_ftm_get_entry_num(lchip, is_acl ?acl_tbl_id[lkup_level]:scl_tbl_id[lkup_level], &entry_num);
        key_index = 2*entry_num - 2048 + local_index;/*Uints_80bits_Per_Block=2048*/
    }

    if(o_key_index)
    {
        *o_key_index = key_index;
    }

    return CTC_E_NONE;
}


STATIC int32
_sys_tsingma_diag_pkt_trace_get_ipe_scl_info(uint8 lchip, ctc_diag_pkt_trace_result_t* p_rslt)
{
    uint32 cmd = 0;
    uint32 value = 0;
    uint32 value1 = 0;
    char str[64] = {0};
    uint32 hash0_keyType = 0;
    uint32 hash0_valid = 0;
    uint32 hash1_keyType = 0;
    uint32 hash1_valid = 0;
    uint8  buf_cnt[4] = {0};
    uint32 ad_ds_type[4] = {0};
    uint32 ad_proc_valid[4] = {0};
    uint32 scl_userIdTcamEn[4] = {0};
    uint32 scl_userIdTcamType[4] = {0};
    uint8 pos = CTC_DIAG_TRACE_POS_IPE;
    char* str_scl_key_type[] = {"MACKEY160", "L3KEY160", "IPV6KEY320", "MACL3KEY320", "UDFKEY320", "USERIDKEY", "UDFKEY160", "MACIPV6KEY640"};
    char* str_ad_ds_type[] = {"NONE","USERID","SCL","TUNNELID"};
    uint32 scl_key_size = 0;
    uint32 tcam_userIdTcamResultValid[4] = {0};
    uint32 tcam_userIdTcamIndex[4] = {0};
    uint32 tcam_userIdValid[4] = {0};
    DbgFlowTcamEngineUserIdKeyInfo0_m dbg_scl_key_info;
    DbgIpeUserIdInfo_m user_id;
    DbgUserIdHashEngineForUserId0Info_m user_id0;
    DbgUserIdHashEngineForUserId1Info_m user_id1;
    DbgFlowTcamEngineUserIdInfo0_m tcam_user_id[4];
    DbgFlowTcamEngineUserIdKeyInfo0_m scl_key;
    uint32 scl_key_info_key_id[4] = {DbgFlowTcamEngineUserIdKeyInfo0_t, DbgFlowTcamEngineUserIdKeyInfo1_t,
                                     DbgFlowTcamEngineUserIdKeyInfo2_t,DbgFlowTcamEngineUserIdKeyInfo3_t};
    uint32 scl_key_info_keysizefld[4] = {DbgFlowTcamEngineUserIdKeyInfo0_keySize_f, DbgFlowTcamEngineUserIdKeyInfo1_keySize_f,
                                         DbgFlowTcamEngineUserIdKeyInfo2_keySize_f,DbgFlowTcamEngineUserIdKeyInfo3_keySize_f};

    sal_memset(&scl_key, 0, sizeof(scl_key));
    sal_memset(&user_id, 0, sizeof(user_id));
    cmd = DRV_IOR(DbgIpeUserIdInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &user_id));

    if (!GetDbgIpeUserIdInfo(V, valid_f, &user_id))
    {
        return CTC_E_NONE;
    }

    /*DbgUserIdHashEngineForUserId0Info*/
    sal_memset(&user_id0, 0, sizeof(user_id0));
    cmd = DRV_IOR(DbgUserIdHashEngineForUserId0Info_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &user_id0));

    /*DbgUserIdHashEngineForUserId1Info*/
    sal_memset(&user_id1, 0, sizeof(user_id1));
    cmd = DRV_IOR(DbgUserIdHashEngineForUserId1Info_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &user_id1));

    /*DbgFlowTcamEngineUserIdInfo0*/
    sal_memset(tcam_user_id, 0, 4 * sizeof(DbgFlowTcamEngineUserIdInfo0_m));
    cmd = DRV_IOR(DbgFlowTcamEngineUserIdInfo0_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tcam_user_id[0]));
    cmd = DRV_IOR(DbgFlowTcamEngineUserIdInfo1_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tcam_user_id[1]));
    cmd = DRV_IOR(DbgFlowTcamEngineUserIdInfo2_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tcam_user_id[2]));
    cmd = DRV_IOR(DbgFlowTcamEngineUserIdInfo3_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tcam_user_id[3]));

    hash0_valid = GetDbgUserIdHashEngineForUserId0Info(V, valid_f, &user_id0);
    hash0_keyType = GetDbgUserIdHashEngineForUserId0Info(V, userIdKeyType_f, &user_id0);
    hash1_valid = GetDbgUserIdHashEngineForUserId1Info(V, userIdKeyType_f, &user_id1);
    hash1_keyType =GetDbgUserIdHashEngineForUserId1Info(V, valid_f, &user_id1);
    scl_userIdTcamEn[0] = GetDbgIpeUserIdInfo(V, userIdTcamLookupEn0_f, &user_id);
    scl_userIdTcamEn[1] = GetDbgIpeUserIdInfo(V, userIdTcamLookupEn1_f, &user_id);
    scl_userIdTcamEn[2] = GetDbgIpeUserIdInfo(V, userIdTcamLookupEn2_f, &user_id);
    scl_userIdTcamEn[3] = GetDbgIpeUserIdInfo(V, userIdTcamLookupEn3_f, &user_id);

    if (!((hash0_keyType && hash0_valid)|| (hash1_keyType && hash1_valid) || scl_userIdTcamEn[0] ||
    scl_userIdTcamEn[1] ||scl_userIdTcamEn[2] || scl_userIdTcamEn[3]))
    {
        return CTC_E_NONE;
    }
    p_rslt->position = CTC_DIAG_TRACE_POS_IPE;
    DIAG_INFO_NONE(pos, "SCL")

    ad_ds_type[0] = GetDbgIpeUserIdInfo(V, process0AdDsType_f, &user_id);
    ad_ds_type[1] = GetDbgIpeUserIdInfo(V, process1AdDsType_f, &user_id);
    ad_ds_type[2] = GetDbgIpeUserIdInfo(V, process2AdDsType_f, &user_id);
    ad_ds_type[3] = GetDbgIpeUserIdInfo(V, process3AdDsType_f, &user_id);
    ad_proc_valid[0] = GetDbgIpeUserIdInfo(V, adProcessValid0_f, &user_id);
    ad_proc_valid[1] = GetDbgIpeUserIdInfo(V, adProcessValid1_f, &user_id);
    ad_proc_valid[2] = GetDbgIpeUserIdInfo(V, adProcessValid2_f, &user_id);
    ad_proc_valid[3] = GetDbgIpeUserIdInfo(V, adProcessValid3_f, &user_id);

    cmd = DRV_IOR(IpeUserIdCtl_t, IpeUserIdCtl_userIdResultPriorityMode_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    switch (value)
    {
        case 0:
            buf_cnt[0] = 0;
            buf_cnt[1] = 1;
            buf_cnt[2] = 2;
            buf_cnt[3] = 3;
            break;
        case 1:
            buf_cnt[2] = 0;
            buf_cnt[0] = 1;
            buf_cnt[1] = 2;
            buf_cnt[3] = 3;
            break;
        case 2:
            buf_cnt[2] = 0;
            buf_cnt[3] = 1;
            buf_cnt[0] = 2;
            buf_cnt[1] = 3;
            break;
        case 3:
            buf_cnt[0] = 0;
            buf_cnt[2] = 1;
            buf_cnt[1] = 2;
            buf_cnt[3] = 3;
            break;
        case 4:
            buf_cnt[2] = 0;
            buf_cnt[0] = 1;
            buf_cnt[3] = 2;
            buf_cnt[1] = 3;
            break;
        default:
            break;
    }

    for (value = 0; value < 4; value++)
    {
        if (ad_proc_valid[buf_cnt[value]])
        {
            sal_sprintf(str, "ad-type-%d", value);
            DIAG_INFO_CHAR(pos, str, str_ad_ds_type[ad_ds_type[buf_cnt[value]]])
        }
    }

    /*hash0*/
    if (hash0_keyType && hash0_valid)
    {
        value = GetDbgUserIdHashEngineForUserId0Info(V, keyIndex_f, &user_id0);
        value1 = GetDbgUserIdHashEngineForUserId0Info(V, adIndex_f, &user_id0);
        if (GetDbgUserIdHashEngineForUserId0Info(V, hashConflict_f, &user_id0))
        {
            DIAG_INFO_CHAR(pos, "hash-0-lookup-result", DIAG_LOOKUP_CONFLICT)
            DIAG_INFO_UINT32(pos, "key-index", value)
            DIAG_INFO_CHAR(pos, "key-name", _sys_tsingma_diag_get_igr_scl_key_desc(lchip, hash0_keyType, 0))
        }
        else if (GetDbgUserIdHashEngineForUserId0Info(V, defaultEntryValid_f, &user_id0))
        {
            DIAG_INFO_CHAR(pos, "hash-0-lookup-result", DIAG_LOOKUP_DEFAULT)
        }
        else if (GetDbgUserIdHashEngineForUserId0Info(V, lookupResultValid_f, &user_id0))
        {
            DIAG_INFO_CHAR(pos, "hash-0-lookup-result", DIAG_LOOKUP_HIT)
        }
        else
        {
            DIAG_INFO_CHAR(pos, "hash-0-lookup-result", DIAG_LOOKUP_HIT_NONE)
        }
        if (GetDbgUserIdHashEngineForUserId0Info(V, defaultEntryValid_f, &user_id0)
            || GetDbgUserIdHashEngineForUserId0Info(V, lookupResultValid_f, &user_id0))
        {
            DIAG_INFO_UINT32(pos, "key-index", value)
            DIAG_INFO_CHAR(pos, "key-name", _sys_tsingma_diag_get_igr_scl_key_desc(lchip, hash0_keyType, 0))
            DIAG_INFO_UINT32(pos, "ad-index", value1)

            cmd = DRV_IOR(DsPhyPortExt_t, DsPhyPortExt_sclFlowHashEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, GetDbgIpeUserIdInfo(V, localPhyPort_f, &user_id), cmd, &value));
            if (value)
            {
                DIAG_INFO_CHAR(pos, "ad-name", "DsSclFlow")
            }
            else if (hash0_keyType && (hash0_keyType < USERIDHASHTYPE_TUNNELIPV4))
            {
                cmd = DRV_IOR(DsUserIdHalf_t, DsUserIdHalf_isHalf_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, value1, cmd, &value));
                if (value)
                {
                    DIAG_INFO_CHAR(pos, "ad-name", "DsUserIdHalf")
                }
                else
                {
                    DIAG_INFO_CHAR(pos, "ad-name", "DsUserId")
                }
            }
            else if (hash0_keyType >= USERIDHASHTYPE_TUNNELIPV4)
            {
                cmd = DRV_IOR(DsTunnelIdHalf_t, DsTunnelIdHalf_isHalf_f);
                DRV_IOCTL(lchip, value1, cmd, &value);
                if (value)
                {
                    DIAG_INFO_CHAR(pos, "ad-name", "DsTunnelIdHalf")
                }
                else
                {
                    DIAG_INFO_CHAR(pos, "ad-name", "DsTunnelId")
                }
            }
        }
    }

    /*hash1*/
    if (hash1_keyType && hash1_valid)
    {
        value = GetDbgUserIdHashEngineForUserId1Info(V, keyIndex_f, &user_id0);
        value1 = GetDbgUserIdHashEngineForUserId1Info(V, adIndex_f, &user_id0);
        if (GetDbgUserIdHashEngineForUserId1Info(V, hashConflict_f, &user_id1))
        {
            DIAG_INFO_CHAR(pos, "hash-1-lookup-result", DIAG_LOOKUP_CONFLICT)
            DIAG_INFO_UINT32(pos, "key-index", value)
            DIAG_INFO_CHAR(pos, "key-name", _sys_tsingma_diag_get_igr_scl_key_desc(lchip, hash0_keyType, 1))
        }
        else if (GetDbgUserIdHashEngineForUserId1Info(V, defaultEntryValid_f, &user_id1))
        {
            DIAG_INFO_CHAR(pos, "hash-0-lookup-result", DIAG_LOOKUP_DEFAULT)
        }
        else if (GetDbgUserIdHashEngineForUserId1Info(V, lookupResultValid_f, &user_id1))
        {
            DIAG_INFO_CHAR(pos, "hash-0-lookup-result", DIAG_LOOKUP_HIT)
        }
        else
        {
            DIAG_INFO_CHAR(pos, "hash-0-lookup-result", DIAG_LOOKUP_HIT_NONE)
        }
        if (GetDbgUserIdHashEngineForUserId1Info(V, defaultEntryValid_f, &user_id1)
            || GetDbgUserIdHashEngineForUserId1Info(V, lookupResultValid_f, &user_id1))
        {
            DIAG_INFO_UINT32(pos, "key-index", value)
            DIAG_INFO_CHAR(pos, "key-name", _sys_tsingma_diag_get_igr_scl_key_desc(lchip, hash0_keyType, 1))
            DIAG_INFO_UINT32(pos, "ad-index", value1)
            if (hash1_keyType && (hash1_keyType < USERIDHASHTYPE_TUNNELIPV4))
            {
                cmd = DRV_IOR(DsUserIdHalf1_t, DsUserIdHalf1_isHalf_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, value1, cmd, &value));
                if (value)
                {
                    DIAG_INFO_CHAR(pos, "ad-name", "DsUserIdHalf1")
                }
                else
                {
                    DIAG_INFO_CHAR(pos, "ad-name", "DsUserId1")
                }
            }
            else if (hash1_keyType >= USERIDHASHTYPE_TUNNELIPV4)
            {
                cmd = DRV_IOR(DsTunnelIdHalf1_t, DsTunnelIdHalf1_isHalf_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, value1, cmd, &value));
                if (value)
                {
                    DIAG_INFO_CHAR(pos, "ad-name", "DsTunnelIdHalf1")
                }
                else
                {
                    DIAG_INFO_CHAR(pos, "ad-name", "DsTunnelId1")
                }
            }
        }
    }

    scl_userIdTcamType[0] = GetDbgIpeUserIdInfo(V, userIdTcamKeyType0_f, &user_id);
    scl_userIdTcamType[1] = GetDbgIpeUserIdInfo(V, userIdTcamKeyType1_f, &user_id);
    scl_userIdTcamType[2] = GetDbgIpeUserIdInfo(V, userIdTcamKeyType2_f, &user_id);
    scl_userIdTcamType[3] = GetDbgIpeUserIdInfo(V, userIdTcamKeyType3_f, &user_id);

    /*DbgFlowTcamEngineUserIdInfo0*/
    tcam_userIdTcamResultValid[0] = GetDbgFlowTcamEngineUserIdInfo0(V, tcamResultValid0_f, &tcam_user_id[0]);
    tcam_userIdTcamIndex[0] = GetDbgFlowTcamEngineUserIdInfo0(V, tcamIndex0_f, &tcam_user_id[0]);
    tcam_userIdValid[0] = GetDbgFlowTcamEngineUserIdInfo0(V, valid_f, &tcam_user_id[0]);
    /*DbgFlowTcamEngineUserIdInfo1*/
    tcam_userIdTcamResultValid[1] = GetDbgFlowTcamEngineUserIdInfo1(V, tcamResultValid1_f, &tcam_user_id[1]);
    tcam_userIdTcamIndex[1] = GetDbgFlowTcamEngineUserIdInfo1(V, tcamIndex1_f, &tcam_user_id[1]);
    tcam_userIdValid[1] = GetDbgFlowTcamEngineUserIdInfo1(V, valid_f, &tcam_user_id[1]);
    /*DbgFlowTcamEngineUserIdInfo2*/
    tcam_userIdTcamResultValid[2] = GetDbgFlowTcamEngineUserIdInfo2(V, tcamResultValid2_f, &tcam_user_id[2]);
    tcam_userIdTcamIndex[2] = GetDbgFlowTcamEngineUserIdInfo2(V, tcamIndex2_f, &tcam_user_id[2]);
    tcam_userIdValid[2] = GetDbgFlowTcamEngineUserIdInfo2(V, valid_f, &tcam_user_id[2]);
    /*DbgFlowTcamEngineUserIdInfo3*/
    tcam_userIdTcamResultValid[3] = GetDbgFlowTcamEngineUserIdInfo3(V, tcamResultValid3_f, &tcam_user_id[3]);
    tcam_userIdTcamIndex[3] = GetDbgFlowTcamEngineUserIdInfo3(V, tcamIndex3_f, &tcam_user_id[3]);
    tcam_userIdValid[3] = GetDbgFlowTcamEngineUserIdInfo3(V, valid_f, &tcam_user_id[3]);

    sal_memset(&dbg_scl_key_info, 0, sizeof(DbgFlowTcamEngineUserIdKeyInfo0_m));
    /*tcam0~3*/
    for (value1 = 0; value1 < 4; value1++)
    {
        if (!(scl_userIdTcamEn[value1] && tcam_userIdValid[value1]))
        {
            continue;
        }
        sal_sprintf(str, "tcam-%d-lookup-result",value1);
        if (tcam_userIdTcamResultValid[value1])
        {
            DIAG_INFO_CHAR(pos, str, DIAG_LOOKUP_HIT)
            DIAG_INFO_CHAR(pos, "key-type", str_scl_key_type[scl_userIdTcamType[value1]])
            _sys_tsingma_diag_map_tcam_key_index(lchip, tcam_userIdTcamIndex[value1], 0, value1, &value);
            cmd = DRV_IOR(scl_key_info_key_id[value1], DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &dbg_scl_key_info));
            DRV_GET_FIELD_A(lchip, scl_key_info_key_id[value1], scl_key_info_keysizefld[value1], &dbg_scl_key_info, &scl_key_size);
            DIAG_INFO_UINT32(pos, "key-index", value / (1 << scl_key_size))
            DIAG_INFO_UINT32(pos, "key-size", 80*(1<<scl_key_size))
        }
        else
        {
            DIAG_INFO_CHAR(pos, str, DIAG_LOOKUP_HIT_NONE)
        }
    }


    /* Drop Process */
    _sys_tsingma_diag_pkt_tracce_drop_map(p_rslt,
                                          GetDbgIpeUserIdInfo(V, discard_f, &user_id),
                                          GetDbgIpeUserIdInfo(V, discardType_f, &user_id));



    /* Exception Process */
    _sys_tsingma_diag_pkt_tracce_exception_map(p_rslt,
                                               GetDbgIpeUserIdInfo(V, exceptionEn_f, &user_id),
                                               GetDbgIpeUserIdInfo(V, exceptionIndex_f, &user_id),
                                               GetDbgIpeUserIdInfo(V, exceptionSubIndex_f, &user_id));
    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_diag_pkt_trace_get_ipe_intf_info(uint8 lchip, ctc_diag_pkt_trace_result_t* p_rslt)
{
    uint8 gchip = 0;
    uint32 value = 0;
    uint32 value1 = 0;
    uint32 vlanPtr = 0;
    uint32 cmd = 0;
    uint32 localPhyPort = 0;
    uint8 pos = CTC_DIAG_TRACE_POS_IPE;
    char* vlanId_action[3] = {"NONE", "SWAP", "USER"};
    DbgIpeUserIdInfo_m user_id_info;
    DbgIpeIntfMapperInfo_m intf_mapper_info;


    sal_memset(&user_id_info, 0, sizeof(user_id_info));
    cmd = DRV_IOR(DbgIpeUserIdInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &user_id_info));
    sal_memset(&intf_mapper_info, 0, sizeof(intf_mapper_info));
    cmd = DRV_IOR(DbgIpeIntfMapperInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &intf_mapper_info));

    if (!GetDbgIpeIntfMapperInfo(V, valid_f, &intf_mapper_info))
    {
        return CTC_E_NONE;
    }
    p_rslt->position = CTC_DIAG_TRACE_POS_IPE;
    DIAG_INFO_NONE(pos, "Interface Mapper")

    localPhyPort = GetDbgIpeUserIdInfo(V, localPhyPort_f, &user_id_info);
    if (GetDbgIpeUserIdInfo(V, valid_f, &user_id_info))
    {
        CTC_ERROR_RETURN(drv_get_gchip_id(lchip, &gchip));
        DIAG_INFO_U32_HEX(pos, "source-port", CTC_MAP_LPORT_TO_GPORT(gchip, localPhyPort))
    }

    vlanPtr = GetDbgIpeIntfMapperInfo(V, vlanPtr_f, &intf_mapper_info);
    if (vlanPtr < 4096)
    {
        DIAG_INFO_UINT32(pos, "vlan-ptr", vlanPtr)
    }
    if (GetDbgIpeIntfMapperInfo(V, svlanTagOperationValid_f, &intf_mapper_info))
    {
        value = GetDbgIpeIntfMapperInfo(V, sVlanIdAction_f, &intf_mapper_info);
        DIAG_INFO_CHAR(pos, "ingress-svlan-action", vlanId_action[value])
        if (value)
        {
            DIAG_INFO_UINT32(pos, "new-svlan-id", GetDbgIpeIntfMapperInfo(V, svlanId_f, &intf_mapper_info))
        }
    }
    if (GetDbgIpeIntfMapperInfo(V, cvlanTagOperationValid_f, &intf_mapper_info))
    {
        value = GetDbgIpeIntfMapperInfo(V, cVlanIdAction_f, &intf_mapper_info);
        DIAG_INFO_CHAR(pos, "ingress-cvlan-action", vlanId_action[value])
        if (value)
        {
            DIAG_INFO_UINT32(pos, "new-cvlan-id", GetDbgIpeIntfMapperInfo(V, cvlanId_f, &intf_mapper_info))
        }
    }
    value = GetDbgIpeIntfMapperInfo(V, interfaceId_f, &intf_mapper_info);
    if (value)
    {
        DIAG_INFO_UINT32(pos, "l3-intf-id", value)
        cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_vrfId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, value, cmd, &value1));
        DIAG_INFO_UINT32(pos, "intf-vrfid", value1)
        DIAG_INFO_CHAR(pos, "route-mac", GetDbgIpeIntfMapperInfo(V, isRouterMac_f, &intf_mapper_info) ? "YES" : "NO")
    }
    cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_portCrossConnect_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, localPhyPort, cmd, &value));
    if (value)
    {
        DIAG_INFO_CHAR(pos, "cross-connect", "YES")
    }
    else
    {
        /*bridge en*/
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_bridgeEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, localPhyPort, cmd, &value));
        if (vlanPtr < 4096)
        {
            cmd = DRV_IOR(DsSrcVlan_t, DsSrcVlan_bridgeDisable_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, vlanPtr, cmd, &value1));
        }
        value = value && (!value1);
        DIAG_INFO_CHAR(pos, "bridge-port", value ?"YES":"NO")

        /*route en*/
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_routedPort_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, localPhyPort, cmd, &value));
        DIAG_INFO_CHAR(pos, "route-port", value ?"YES":"NO")
    }
    /*mpls en*/
    value = GetDbgIpeIntfMapperInfo(V, interfaceId_f, &intf_mapper_info);
    if (value)
    {
        cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_mplsEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, value, cmd, &value1));
        DIAG_INFO_CHAR(pos, "mpls-port", value1 ?"YES":"NO")
    }

    /* Drop Process */
    _sys_tsingma_diag_pkt_tracce_drop_map(p_rslt,
                                          GetDbgIpeIntfMapperInfo(V, discard_f, &intf_mapper_info),
                                          GetDbgIpeIntfMapperInfo(V, discardType_f, &intf_mapper_info));
    /* Exception Process */
    _sys_tsingma_diag_pkt_tracce_exception_map(p_rslt,
                                               GetDbgIpeIntfMapperInfo(V, exceptionEn_f, &intf_mapper_info),
                                               GetDbgIpeIntfMapperInfo(V, exceptionIndex_f, &intf_mapper_info),
                                               GetDbgIpeIntfMapperInfo(V, exceptionSubIndex_f, &intf_mapper_info));
    return CTC_E_NONE;
}


STATIC int32
_sys_tsingma_diag_pkt_trace_get_ipe_decap_info(uint8 lchip, ctc_diag_pkt_trace_result_t* p_rslt)
{
    uint32 cmd = 0;
    uint32 value = 0;
    char* str[] = {"1st-label-lookup-result", "2nd-label-lookup-result", "3th-label-lookup-result"};
    uint8 pos = CTC_DIAG_TRACE_POS_IPE;
    DbgMplsHashEngineForLabel0Info_m  label0_info;
    DbgMplsHashEngineForLabel1Info_m  label1_info;
    DbgMplsHashEngineForLabel2Info_m  label2_info;
    DbgIpeMplsDecapInfo_m mpls_decap_info;

    sal_memset(&label0_info, 0, sizeof(label0_info));
    cmd = DRV_IOR(DbgMplsHashEngineForLabel0Info_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &label0_info));
    sal_memset(&label1_info, 0, sizeof(label1_info));
    cmd = DRV_IOR(DbgMplsHashEngineForLabel1Info_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &label1_info));
    sal_memset(&label2_info, 0, sizeof(label2_info));
    cmd = DRV_IOR(DbgMplsHashEngineForLabel2Info_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &label2_info));
    sal_memset(&mpls_decap_info, 0, sizeof(mpls_decap_info));
    cmd = DRV_IOR(DbgIpeMplsDecapInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mpls_decap_info));

    if (!GetDbgIpeMplsDecapInfo(V, valid_f, &mpls_decap_info))
    {
        return CTC_E_NONE;
    }
    p_rslt->position = CTC_DIAG_TRACE_POS_IPE;
    DIAG_INFO_NONE(pos, "Decapsulation")

    if (GetDbgIpeMplsDecapInfo(V, tunnelDecap0_f, &mpls_decap_info)
        || GetDbgIpeMplsDecapInfo(V, tunnelDecap1_f, &mpls_decap_info))
    {
        if (GetDbgIpeMplsDecapInfo(V, innerPacketLookup_f, &mpls_decap_info))
        {
            DIAG_INFO_CHAR(pos, "tunnel-process", "inner-packet-forward")
        }
        else
        {
            DIAG_INFO_CHAR(pos, "tunnel-process", "tunnel-forward")
        }
    }
    else if (GetDbgIpeMplsDecapInfo(V, isMplsSwitched_f, &mpls_decap_info))
    {
        uint8 i = 0;
        uint32 mpls_valid[3] = {
                GetDbgMplsHashEngineForLabel0Info(V, valid_f, &label0_info),
                GetDbgMplsHashEngineForLabel1Info(V, valid_f, &label1_info),
                GetDbgMplsHashEngineForLabel2Info(V, valid_f, &label2_info)};
        uint32 mpls_key_index[3] = {
                GetDbgMplsHashEngineForLabel0Info(V, keyIndex_f, &label0_info),
                GetDbgMplsHashEngineForLabel1Info(V, keyIndex_f, &label1_info),
                GetDbgMplsHashEngineForLabel2Info(V, keyIndex_f, &label2_info)};
        uint32 mpls_ad_index[3] = {0};
        uint32 mpls_conflict[3] = {
                GetDbgMplsHashEngineForLabel0Info(V, mplsHashConflict_f, &label1_info),
                GetDbgMplsHashEngineForLabel1Info(V, mplsHashConflict_f, &label1_info),
                GetDbgMplsHashEngineForLabel2Info(V, mplsHashConflict_f, &label2_info)};
        uint32 mpls_result_valid[3] = {
                GetDbgMplsHashEngineForLabel0Info(V, lookupResultValid_f, &label0_info),
                GetDbgMplsHashEngineForLabel1Info(V, lookupResultValid_f, &label1_info),
                GetDbgMplsHashEngineForLabel2Info(V, lookupResultValid_f, &label2_info)};

        for (i = 0; i < 3; i++)
        {
            if (mpls_valid[i])
            {
                if (GetDbgIpeMplsDecapInfo(V, innerPacketLookup_f, &mpls_decap_info))
                {
                    DIAG_INFO_CHAR(pos, "mpls-process", "inner-packet-forward")
                }
                else
                {
                    DIAG_INFO_CHAR(pos, "mpls-process", "mpls-label-forward")
                }

                value = TABLE_MAX_INDEX(lchip, DsMplsHashCam_t);
                mpls_ad_index[i] = (mpls_key_index[i] >= value) ? (mpls_key_index[i] - value):(mpls_key_index[i]);
                if (mpls_conflict[i])
                {
                    DIAG_INFO_CHAR(pos, str[i], DIAG_LOOKUP_CONFLICT)
                    DIAG_INFO_CHAR(pos, "key-name", "DsMplsLabelHashKey")
                    DIAG_INFO_UINT32(pos, "key-index", mpls_key_index[i])
                }
                else if (mpls_result_valid[i])
                {
                    DIAG_INFO_CHAR(pos, str[i], DIAG_LOOKUP_HIT)
                    DIAG_INFO_CHAR(pos, "key-name", "DsMplsLabelHashKey")
                    DIAG_INFO_UINT32(pos, "key-index", mpls_key_index[i])
                    DIAG_INFO_CHAR(pos, "ad-name", ((mpls_key_index[i] < value) ? "DsMplsHashCamAd" : "DsMpls"))
                    DIAG_INFO_UINT32(pos, "ad-index", mpls_ad_index[i])
                    DIAG_INFO_CHAR(pos, "pop-lable", "YES")
                }
                else
                {
                    DIAG_INFO_CHAR(pos, str[i], DIAG_LOOKUP_HIT_NONE)
                }
            }
        }
    }

    if (GetDbgIpeMplsDecapInfo(V, policerValid0_f, &mpls_decap_info))
    {
        DIAG_INFO_UINT32(pos, "policer-ptr-0", GetDbgIpeMplsDecapInfo(V, policerPtr0_f, &mpls_decap_info))
    }
    if (GetDbgIpeMplsDecapInfo(V, policerValid1_f, &mpls_decap_info))
    {
        DIAG_INFO_UINT32(pos, "policer-ptr-1", GetDbgIpeMplsDecapInfo(V, policerPtr1_f, &mpls_decap_info))
    }
    /* Drop Process */
    _sys_tsingma_diag_pkt_tracce_drop_map(p_rslt,
                                          GetDbgIpeMplsDecapInfo(V, discard_f, &mpls_decap_info),
                                          GetDbgIpeMplsDecapInfo(V, discardType_f, &mpls_decap_info));
    /* Exception Process */
    _sys_tsingma_diag_pkt_tracce_exception_map(p_rslt,
                                               GetDbgIpeMplsDecapInfo(V, exceptionEn_f, &mpls_decap_info),
                                               GetDbgIpeMplsDecapInfo(V, exceptionIndex_f, &mpls_decap_info),
                                               GetDbgIpeMplsDecapInfo(V, exceptionSubIndex_f, &mpls_decap_info));

    return CTC_E_NONE;
}
STATIC int32
_sys_tsingma_diag_pkt_trace_get_lkup_mgr_info(uint8 lchip, ctc_diag_pkt_trace_result_t* p_rslt)
{
    uint32 cmd = 0;
    uint32 value = 0;
    uint8 pos = CTC_DIAG_TRACE_POS_IPE;
    hw_mac_addr_t hw_mac = {0};
    DbgIpeLkpMgrInfo_m lkp_mgr_info;
    DbgFibLkpEngineMacSaHashInfo_m macsa_info;
/*    DbgFibLkpEngineHostUrpfHashInfo_m  urpf_info;*/
    DbgFibLkpEngineMacDaHashInfo_m  macda_info;
    DbgFibLkpEnginel3DaHashInfo_m   l3da_info;
    DbgFibLkpEnginel3SaHashInfo_m   l3sa_info;



    sal_memset(&lkp_mgr_info, 0, sizeof(lkp_mgr_info));
    cmd = DRV_IOR(DbgIpeLkpMgrInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &lkp_mgr_info));

    if (!GetDbgIpeLkpMgrInfo(V, valid_f, &lkp_mgr_info))
    {
        return CTC_E_NONE;
    }
    p_rslt->position = CTC_DIAG_TRACE_POS_IPE;
    DIAG_INFO_NONE(pos, "Lookup Manager")
    if (GetDbgIpeLkpMgrInfo(V, isInnerMacSaLookup_f, &lkp_mgr_info))
    {
        DIAG_INFO_CHAR(pos, "inner-macsa-lookup", "TRUE")
    }
    if (GetDbgIpeLkpMgrInfo(V, efdEnable_f, &lkp_mgr_info))
    {
        DIAG_INFO_CHAR(pos, "elephant-detect", "ENABLE")
    }
    if (GetDbgIpeLkpMgrInfo(V, isElephant_f, &lkp_mgr_info))
    {
        DIAG_INFO_CHAR(pos, "elephant-flow", "TURE")
    }
    if (GetDbgIpeLkpMgrInfo(V, isNewElephant_f, &lkp_mgr_info))
    {
        DIAG_INFO_CHAR(pos, "new-elephant-flow", "TURE")
    }
    if (GetDbgIpeLkpMgrInfo(V, isIpv4Ucast_f, &lkp_mgr_info))
    {
        DIAG_INFO_CHAR(pos, "packet-type", "IPv4-UC")
    }
    if (GetDbgIpeLkpMgrInfo(V, isIpv4Mcast_f, &lkp_mgr_info))
    {
        DIAG_INFO_CHAR(pos, "packet-type", "IPv4-MC")
    }
    if (GetDbgIpeLkpMgrInfo(V, isIpv4UcastNat_f, &lkp_mgr_info))
    {
        DIAG_INFO_CHAR(pos, "packet-type", "IPv4-UC-NAT")
    }
    if (GetDbgIpeLkpMgrInfo(V, isIpv4UcastRpf_f, &lkp_mgr_info))
    {
        DIAG_INFO_CHAR(pos, "packet-type", "IPv4-UC-RPF")
    }
    if (GetDbgIpeLkpMgrInfo(V, isIpv6Ucast_f, &lkp_mgr_info))
    {
        DIAG_INFO_CHAR(pos, "packet-type", "IPv6-UC")
    }
    if (GetDbgIpeLkpMgrInfo(V, isIpv6Mcast_f, &lkp_mgr_info))
    {
        DIAG_INFO_CHAR(pos, "packet-type", "IPv6-MC")
    }
    if (GetDbgIpeLkpMgrInfo(V, isIpv6UcastNat_f, &lkp_mgr_info))
    {
        DIAG_INFO_CHAR(pos, "packet-type", "IPv6-UC-NAT")
    }
    if (GetDbgIpeLkpMgrInfo(V, isIpv6UcastRpf_f, &lkp_mgr_info))
    {
        DIAG_INFO_CHAR(pos, "packet-type", "IPv6-UC-RPF")
    }
    if (GetDbgIpeLkpMgrInfo(V, isIpv4IcmpErrMsg_f, &lkp_mgr_info))
    {
        DIAG_INFO_CHAR(pos, "packet-type", "IPv4-ICMP-ERROR-MSG")
    }
    if (GetDbgIpeLkpMgrInfo(V, isIpv6IcmpErrMsg_f, &lkp_mgr_info))
    {
        DIAG_INFO_CHAR(pos, "packet-type", "IPv6-ICMP-ERROR-MSG")
    }
    if (GetDbgIpeLkpMgrInfo(V, isTrillUcast_f, &lkp_mgr_info))
    {
        DIAG_INFO_CHAR(pos, "packet-type", "TRILL-UC")
    }
    if (GetDbgIpeLkpMgrInfo(V, isTrillMcast_f, &lkp_mgr_info))
    {
        DIAG_INFO_CHAR(pos, "packet-type", "TRILL-MC")
    }
    if (GetDbgIpeLkpMgrInfo(V, isFcoe_f, &lkp_mgr_info))
    {
        DIAG_INFO_CHAR(pos, "packet-type", "FCoE")
    }
    if (GetDbgIpeLkpMgrInfo(V, isFcoeRpf_f, &lkp_mgr_info))
    {
        DIAG_INFO_CHAR(pos, "packet-type", "FCoE-RPF")
    }
    if (GetDbgIpeLkpMgrInfo(V, ipv4McastAddressCheckFailure_f, &lkp_mgr_info))
    {
        DIAG_INFO_CHAR(pos, "ipv4mc-addr-check", "FAILURE")
    }
    if (GetDbgIpeLkpMgrInfo(V, ipv6McastAddressCheckFailure_f, &lkp_mgr_info))
    {
        DIAG_INFO_CHAR(pos, "ipv6mc-addr-check", "FAILURE")
    }

    sal_memset(&macda_info, 0, sizeof(macda_info));
    cmd = DRV_IOR(DbgFibLkpEngineMacDaHashInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &macda_info));
    if (GetDbgFibLkpEngineMacDaHashInfo(V, valid_f, &macda_info)
        && GetDbgFibLkpEngineMacDaHashInfo(V, gMacDaLkp_macDaLookupEn_f, &macda_info))
    {
        GetDbgFibLkpEngineMacDaHashInfo(A, gMacDaLkp_mappedMac_f, &macda_info, hw_mac);
        /*mac da lookup key*/
        DIAG_INFO_CHAR(pos, "macda-lookup", "ENABLE")
        DIAG_INFO_MAC(pos, "key-macda", hw_mac)
        DIAG_INFO_UINT32(pos, "key-fid", GetDbgFibLkpEngineMacDaHashInfo(V, gMacDaLkp_vsiId_f, &macda_info))
    }

    sal_memset(&macsa_info, 0, sizeof(macsa_info));
    cmd = DRV_IOR(DbgFibLkpEngineMacSaHashInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &macsa_info));
    if (GetDbgFibLkpEngineMacSaHashInfo(V, valid_f, &macsa_info)
        && GetDbgFibLkpEngineMacSaHashInfo(V, gMacSaLkp_macSaLookupEn_f, &macsa_info))
    {
        /*mac sa lookup key*/
        GetDbgFibLkpEngineMacSaHashInfo(A, gMacSaLkp_mappedMac_f, &macsa_info, hw_mac);
        DIAG_INFO_CHAR(pos, "macsa-lookup", "ENABLE")
        DIAG_INFO_MAC(pos, "key-macsa", hw_mac)
        DIAG_INFO_UINT32(pos, "key-fid", GetDbgFibLkpEngineMacSaHashInfo(V, gMacSaLkp_vsiId_f, &macsa_info))
    }

    sal_memset(&l3da_info, 0, sizeof(l3da_info));
    cmd = DRV_IOR(DbgFibLkpEnginel3DaHashInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &l3da_info));
    if (GetDbgFibLkpEnginel3DaHashInfo(V, valid_f, &l3da_info)
        && GetDbgFibLkpEnginel3DaHashInfo(V, l3DaLookupEn_f, &l3da_info))
    {
        /*l3 da lookup key*/
        int32 ret = 0;
        drv_acc_in_t acc_in;
        drv_acc_out_t acc_out;
        char* str_l3da[] = {"IPv4-UCAST", "IPv6-UCAST", "IPv4-MCAST",
                            "IPv6-MCAST", "FCoE", "TRILL-UCAST", "TRILL-MCAST", "RESERVED"};
        DIAG_INFO_CHAR(pos, "l3da-lookup", "ENABLE")
        value = GetDbgFibLkpEnginel3DaHashInfo(V, l3DaKeyType_f, &l3da_info);
        DIAG_INFO_CHAR(pos, "l3da-lkup-key-type", str_l3da[value])

        sal_memset(&acc_in, 0, sizeof(acc_in));
        sal_memset(&acc_out, 0, sizeof(acc_out));
        switch (value)
        {
            case SYS_TM_DIAG_L3DA_TYPE_IPV4UC:
                /*DsFibHost0Ipv4HashKey_t*/
                break;
            case SYS_TM_DIAG_L3DA_TYPE_IPV6UC:
                /*DsFibHost0Ipv6UcastHashKey_t*/
                break;
            case SYS_TM_DIAG_L3DA_TYPE_IPV4MC:
                /*DsFibHost0Ipv4HashKey_t*/
                break;
            case SYS_TM_DIAG_L3DA_TYPE_IPV6MC:
                /*DsFibHost0MacIpv6McastHashKey_t*/
                break;
            case SYS_TM_DIAG_L3DA_TYPE_TRILLUC:
                acc_in.index = GetDbgFibLkpEnginel3DaHashInfo(V, hostHashFirstL3DaKeyIndex_f, &l3da_info);
                acc_in.type = DRV_ACC_TYPE_LOOKUP;
                acc_in.tbl_id = DsFibHost0TrillHashKey_t;
                acc_in.op_type = DRV_ACC_OP_BY_INDEX;
                ret = drv_acc_api(lchip, &acc_in, &acc_out);
                if (ret == 0)
                {
                    DIAG_INFO_U32_HEX(pos, "key-egs-nickname", GetDsFibHost0TrillHashKey(V, egressNickname_f, acc_out.data))
                }
                break;
            case SYS_TM_DIAG_L3DA_TYPE_TRILLMC:
                acc_in.index = GetDbgFibLkpEnginel3DaHashInfo(V, hostHashFirstL3DaKeyIndex_f, &l3da_info);
                acc_in.type = DRV_ACC_TYPE_LOOKUP;
                acc_in.tbl_id = DsFibHost1TrillMcastVlanHashKey_t;
                acc_in.op_type = DRV_ACC_OP_BY_INDEX;
                ret = drv_acc_api(lchip, &acc_in, &acc_out);
                if (ret == 0)
                {
                    DIAG_INFO_U32_HEX(pos, "key-egs-nickname", GetDsFibHost1TrillMcastVlanHashKey(V, egressNickname_f, acc_out.data))
                    DIAG_INFO_U32_HEX(pos, "key-vlan-id", GetDsFibHost1TrillMcastVlanHashKey(V, vlanId_f, acc_out.data))
                }
                break;
            case SYS_TM_DIAG_L3DA_TYPE_FCOE:
                acc_in.index = GetDbgFibLkpEnginel3DaHashInfo(V, hostHashFirstL3DaKeyIndex_f, &l3da_info);
                acc_in.type = DRV_ACC_TYPE_LOOKUP;
                acc_in.tbl_id = DsFibHost0FcoeHashKey_t;
                acc_in.op_type = DRV_ACC_OP_BY_INDEX;
                ret = drv_acc_api(lchip, &acc_in, &acc_out);
                if (ret == 0)
                {
                    DIAG_INFO_U32_HEX(pos, "key-fcid", GetDsFibHost0FcoeHashKey(V, fcoeDid_f, acc_out.data))
                    DIAG_INFO_U32_HEX(pos, "key-fid", GetDsFibHost0FcoeHashKey(V, vsiId_f, acc_out.data))
                }
                break;
            default:
                break;
        }
    }

    sal_memset(&l3sa_info, 0, sizeof(l3sa_info));
    cmd = DRV_IOR(DbgFibLkpEnginel3SaHashInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &l3sa_info));
    if (GetDbgFibLkpEnginel3SaHashInfo(V, valid_f, &l3sa_info)
        && GetDbgFibLkpEnginel3SaHashInfo(V, l3SaLookupEn_f, &l3sa_info))
    {

        char* str_l3sa[] = {"IPv4-RPF", "IPv6-RPF", "IPv4-PBR",
                            "IPv6-PBR", "IPv4-NATSA", "IPv6-NATSA", "FCoE-RPF"};
        /*l3 da lookup key*/
        DIAG_INFO_CHAR(pos, "l3sa-lookup", "ENABLE")
        value = GetDbgFibLkpEnginel3SaHashInfo(V, l3SaKeyType_f, &l3da_info);
        DIAG_INFO_CHAR(pos, "l3sa-lkup-key-type", str_l3sa[value])
    }

    /* Drop Process */
    _sys_tsingma_diag_pkt_tracce_drop_map(p_rslt,
                                          GetDbgIpeLkpMgrInfo(V, discard_f, &lkp_mgr_info),
                                          GetDbgIpeLkpMgrInfo(V, discardType_f, &lkp_mgr_info));
    /* Exception Process */
    _sys_tsingma_diag_pkt_tracce_exception_map(p_rslt,
                                               GetDbgIpeLkpMgrInfo(V, exceptionEn_f, &lkp_mgr_info),
                                               GetDbgIpeLkpMgrInfo(V, exceptionIndex_f, &lkp_mgr_info),
                                               GetDbgIpeLkpMgrInfo(V, exceptionSubIndex_f, &lkp_mgr_info));
    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_diag_pkt_trace_get_flow_hash_info(uint8 lchip, ctc_diag_pkt_trace_result_t* p_rslt)
{
    uint32 cmd = 0;
    uint32 value = 0;
    uint8 pos = CTC_DIAG_TRACE_POS_IPE;
    char*  str_hash_key_name[] = {"Invalid", "DsFlowL2HashKey", "DsFlowL2L3HashKey", "DsFlowL3Ipv4HashKey",
                                    "DsFlowL3Ipv6HashKey", "DsFlowL3MplsHashKey"};
    char*  str_field_sel_name[] = {"Invalid", "FlowL2HashFieldSelect", "FlowL2L3HashFieldSelect", "FlowL3Ipv4HashFieldSelect",
                                    "FlowL3Ipv6HashFieldSelect", "FlowL3MplsHashFieldSelect"};
    DbgFibLkpEngineFlowHashInfo_m flow_hash_info;
    DbgIpeFlowProcessInfo_m flow_pro_info;
    sal_memset(&flow_hash_info, 0, sizeof(flow_hash_info));
    cmd = DRV_IOR(DbgFibLkpEngineFlowHashInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &flow_hash_info));
    sal_memset(&flow_pro_info, 0, sizeof(flow_pro_info));
    cmd = DRV_IOR(DbgIpeFlowProcessInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &flow_pro_info));

    /*hash*/
    if (GetDbgFibLkpEngineFlowHashInfo(V, valid_f, &flow_hash_info)
        && GetDbgFibLkpEngineFlowHashInfo(V, gFlowHashLkp_flowHashLookupEn_f, &flow_hash_info))
    {
        DIAG_INFO_NONE(pos, "Flow Hash")
        p_rslt->position = CTC_DIAG_TRACE_POS_IPE;
        value = GetDbgFibLkpEngineFlowHashInfo(V, gFlowHashLkp_hashKeyType_f, &flow_hash_info);
        if (GetDbgFibLkpEngineFlowHashInfo(V, gFlowHashResult_hashConflict_f, &flow_hash_info))
        {
            DIAG_INFO_CHAR(pos, "flow-hash-lkup-result", DIAG_LOOKUP_CONFLICT)
            DIAG_INFO_CHAR(pos, "key-name", str_hash_key_name[value])
            DIAG_INFO_UINT32(pos, "key-index", GetDbgFibLkpEngineFlowHashInfo(V, gFlowHashResult_keyIndex_f, &flow_hash_info))

        }
        else if (GetDbgFibLkpEngineFlowHashInfo(V, gFlowHashResult_lookupResultValid_f, &flow_hash_info))
        {
            DIAG_INFO_CHAR(pos, "flow-hash-lkup-result", DIAG_LOOKUP_HIT)
            DIAG_INFO_CHAR(pos, "key-name", str_hash_key_name[value])
            DIAG_INFO_UINT32(pos, "key-index", GetDbgFibLkpEngineFlowHashInfo(V, gFlowHashResult_keyIndex_f, &flow_hash_info))
            DIAG_INFO_CHAR(pos, "ad-name", "DsFlow")
            DIAG_INFO_UINT32(pos, "ad-index", GetDbgFibLkpEngineFlowHashInfo(V, gFlowHashResult_dsAdIndex_f, &flow_hash_info))
            DIAG_INFO_CHAR(pos, "field-sel-name", str_field_sel_name[value])
            DIAG_INFO_UINT32(pos, "field-sel", GetDbgFibLkpEngineFlowHashInfo(V, gFlowHashLkp_flowFieldSel_f, &flow_hash_info))
        }
        else
        {
            DIAG_INFO_CHAR(pos, "flow-hash-lkup-result", DIAG_LOOKUP_HIT_NONE)
        }
    }

    if (!GetDbgIpeFlowProcessInfo(V, valid_f, &flow_pro_info))
    {
        return CTC_E_NONE;
    }
    /* Exception Process */
    _sys_tsingma_diag_pkt_tracce_exception_map(p_rslt,
                                               GetDbgIpeFlowProcessInfo(V, exceptionEn_f, &flow_pro_info),
                                               GetDbgIpeFlowProcessInfo(V, exceptionIndex_f, &flow_pro_info),
                                               GetDbgIpeFlowProcessInfo(V, exceptionSubIndex_f, &flow_pro_info));
    return CTC_E_NONE;
}


STATIC int32
_sys_tsingma_diag_pkt_trace_get_ipe_bridge_info(uint8 lchip, ctc_diag_pkt_trace_result_t* p_rslt)
{
    uint32 value = 0;
    uint32 value1 = 0;
    uint32 fid = 0;
    uint32 cmd = 0;
    uint8 pos = CTC_DIAG_TRACE_POS_IPE;
    drv_acc_in_t  in;
    drv_acc_out_t out;
    DbgFibLkpEngineMacDaHashInfo_m mac_da_info;
    DbgIpeMacBridgingInfo_m mac_bridge_info;

    sal_memset(&in, 0, sizeof(drv_acc_in_t));
    sal_memset(&out, 0, sizeof(drv_acc_out_t));

    sal_memset(&mac_da_info, 0, sizeof(mac_da_info));
    cmd = DRV_IOR(DbgFibLkpEngineMacDaHashInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_da_info));
    sal_memset(&mac_bridge_info, 0, sizeof(mac_bridge_info));
    cmd = DRV_IOR(DbgIpeMacBridgingInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_bridge_info));

    /*bridge process*/
    if (GetDbgIpeMacBridgingInfo(V, valid_f, &mac_bridge_info))
    {
        if (GetDbgFibLkpEngineMacDaHashInfo(V, gMacDaLkp_macDaLookupEn_f, &mac_da_info)
            && GetDbgIpeMacBridgingInfo(V, bridgePacket_f, &mac_bridge_info))
        {
            DIAG_INFO_NONE(pos, "Layer2 Bridging")
            p_rslt->position = CTC_DIAG_TRACE_POS_IPE;
            if (GetDbgIpeMacBridgingInfo(V, macDaResultValid_f, &mac_bridge_info))
            {
                fid = GetDbgFibLkpEngineMacDaHashInfo(V, gMacDaLkp_vsiId_f, &mac_da_info);
                if (GetDbgFibLkpEngineMacDaHashInfo(V, blackHoleMacDaResultValid_f, &mac_da_info))
                {
                    value = GetDbgFibLkpEngineMacDaHashInfo(V, blackHoleHitMacDaKeyIndex_f, &mac_da_info);
                    cmd = DRV_IOR(DsFibMacBlackHoleHashKey_t, DsFibMacBlackHoleHashKey_dsAdIndex_f);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, value, cmd, &value1));

                    DIAG_INFO_CHAR(pos, "macda-lookup-result", "HIT-BLACKHOLE-ENTRY")
                    DIAG_INFO_CHAR(pos, "key-name", "DsFibMacBlackHoleHashKey")
                    DIAG_INFO_UINT32(pos, "key-index", value)
                    DIAG_INFO_CHAR(pos, "ad-name", "DsMac")
                    DIAG_INFO_UINT32(pos, "ad-index", value1)
                }
                else if (GetDbgIpeMacBridgingInfo(V, macDaDefaultEntryValid_f, &mac_bridge_info))
                {
                    cmd = DRV_IOR(FibEngineLookupResultCtl_t, FibEngineLookupResultCtl_gMacDaLookupResultCtl_defaultEntryBase_f);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
                    DIAG_INFO_CHAR(pos, "macda-lookup-result", DIAG_LOOKUP_DEFAULT)
                    DIAG_INFO_UINT32(pos, "bridge-fid", fid)
                    DIAG_INFO_CHAR(pos, "ad-name", "DsMac")
                    DIAG_INFO_UINT32(pos, "ad-index", (fid + (value << 8)))
                }
                else if (GetDbgFibLkpEngineMacDaHashInfo(V, valid_f, &mac_da_info)
                    && GetDbgFibLkpEngineMacDaHashInfo(V, gMacDaLkpResult_macDaResultValid_f, &mac_da_info))
                {
                    value = GetDbgFibLkpEngineMacDaHashInfo(V, gMacDaLkpResult_macDaHashKeyHitIndex_f, &mac_da_info);
                    in.type = DRV_ACC_TYPE_LOOKUP;
                    in.tbl_id = DsFibHost0MacHashKey_t;
                    in.index =  value;
                    in.op_type = DRV_ACC_OP_BY_INDEX;
                    drv_acc_api(lchip, &in, &out);
                    drv_get_field(lchip, DsFibHost0MacHashKey_t, DsFibHost0MacHashKey_dsAdIndex_f, out.data, &value1);

                    DIAG_INFO_CHAR(pos, "macda-lookup-result", DIAG_LOOKUP_HIT)
                    DIAG_INFO_CHAR(pos, "key-name", "DsFibHost0MacHashKey")
                    DIAG_INFO_UINT32(pos, "key-index", value)
                    DIAG_INFO_CHAR(pos, "ad-name", "DsMac")
                    DIAG_INFO_UINT32(pos, "ad-index", value1)

                }
                else
                {
                    DIAG_INFO_CHAR(pos, "macda-lookup-result", DIAG_LOOKUP_HIT_NONE)
                }
            }
            if (GetDbgIpeMacBridgingInfo(V, bridgeEscape_f, &mac_bridge_info))
            {
                DIAG_INFO_CHAR(pos, "bridge-escape", "ENABLE")
            }
            if (GetDbgIpeMacBridgingInfo(V, stormCtlEn_f, &mac_bridge_info))
            {
                DIAG_INFO_CHAR(pos, "bridge-stromctl", "ENABLE")
            }
        }
    }

    /* Drop Process */
    _sys_tsingma_diag_pkt_tracce_drop_map(p_rslt,
                                          GetDbgIpeMacBridgingInfo(V, discard_f, &mac_bridge_info),
                                          GetDbgIpeMacBridgingInfo(V, discardType_f, &mac_bridge_info));
    /* Exception Process */
    _sys_tsingma_diag_pkt_tracce_exception_map(p_rslt,
                                               GetDbgIpeMacBridgingInfo(V, exceptionEn_f, &mac_bridge_info),
                                               GetDbgIpeMacBridgingInfo(V, exceptionIndex_f, &mac_bridge_info),
                                               GetDbgIpeMacBridgingInfo(V, exceptionSubIndex_f, &mac_bridge_info));

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_diag_pkt_trace_get_ipe_learn_info(uint8 lchip, ctc_diag_pkt_trace_result_t* p_rslt)
{
    uint32 value = 0;
    uint32 ad_index = 0;
    uint32 fid = 0;
    uint32 cmd = 0;
    uint8 pos = CTC_DIAG_TRACE_POS_IPE;
    DbgFibLkpEngineMacSaHashInfo_m mac_sa_info;
    DbgIpeMacLearningInfo_m mac_learn_info;
    drv_acc_in_t  in;
    drv_acc_out_t out;

    sal_memset(&in, 0, sizeof(drv_acc_in_t));
    sal_memset(&out, 0, sizeof(drv_acc_out_t));
    sal_memset(&mac_learn_info, 0, sizeof(mac_learn_info));
    cmd = DRV_IOR(DbgIpeMacLearningInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_learn_info));
    sal_memset(&mac_sa_info, 0, sizeof(mac_sa_info));
    cmd = DRV_IOR(DbgFibLkpEngineMacSaHashInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_sa_info));

    if (GetDbgIpeMacLearningInfo(V, valid_f, &mac_learn_info)
        && GetDbgFibLkpEngineMacSaHashInfo(V, gMacSaLkp_macSaLookupEn_f, &mac_sa_info))
    {
        DIAG_INFO_NONE(pos, "Learning")
        p_rslt->position = CTC_DIAG_TRACE_POS_IPE;
        fid = GetDbgIpeMacLearningInfo(V, learningFid_f, &mac_learn_info);

        if (GetDbgIpeMacLearningInfo(V, macSaDefaultEntryValid_f, &mac_learn_info))
        {
            DIAG_INFO_CHAR(pos, "macsa-lookup-result", DIAG_LOOKUP_DEFAULT)
        }
        else if (GetDbgIpeMacLearningInfo(V, macSaHashConflict_f, &mac_learn_info))
        {
            DIAG_INFO_CHAR(pos, "macsa-lookup-result", DIAG_LOOKUP_CONFLICT)
            value = GetDbgFibLkpEngineMacSaHashInfo(V, gMacSaLkpResult_macSaHashKeyHitIndex_f, &mac_sa_info);
            in.type = DRV_ACC_TYPE_LOOKUP;
            in.tbl_id = DsFibHost0MacHashKey_t;
            in.index =  value;
            in.op_type = DRV_ACC_OP_BY_INDEX;
            drv_acc_api(lchip, &in, &out);
            drv_get_field(lchip, DsFibHost0MacHashKey_t, DsFibHost0MacHashKey_dsAdIndex_f, out.data, &ad_index);
            DIAG_INFO_CHAR(pos, "key-name", "DsFibHost0HashMacCam")
            DIAG_INFO_UINT32(pos, "key-index", value)
            DIAG_INFO_CHAR(pos, "ad-name", "DsMac")
            DIAG_INFO_UINT32(pos, "ad-index", ad_index)
        }
        else if (GetDbgIpeMacLearningInfo(V, macSaResultValid_f, &mac_learn_info))
        {
            DIAG_INFO_CHAR(pos, "macsa-lookup-result", DIAG_LOOKUP_HIT)
            value = GetDbgFibLkpEngineMacSaHashInfo(V, gMacSaLkpResult_macSaHashKeyHitIndex_f, &mac_sa_info);
            in.type = DRV_ACC_TYPE_LOOKUP;
            in.tbl_id = DsFibHost0MacHashKey_t;
            in.index =  value;
            in.op_type = DRV_ACC_OP_BY_INDEX;
            drv_acc_api(lchip, &in, &out);
            drv_get_field(lchip, DsFibHost0MacHashKey_t, DsFibHost0MacHashKey_dsAdIndex_f, out.data, &ad_index);
            DIAG_INFO_CHAR(pos, "key-name", "DsFibHost0MacHashKey")
            DIAG_INFO_UINT32(pos, "key-index", value)
            DIAG_INFO_CHAR(pos, "ad-name", "DsMac")
            DIAG_INFO_UINT32(pos, "ad-index", ad_index)
        }
        else
        {
            DIAG_INFO_CHAR(pos, "macsa-lookup-result", DIAG_LOOKUP_HIT_NONE)
        }

        if (GetDbgIpeMacLearningInfo(V, learningEn_f, &mac_learn_info))
        {
            DIAG_INFO_CHAR(pos, "learning-mode", (GetDbgIpeMacLearningInfo(V, learningType_f, &mac_learn_info) ? "HW" : "SW"))
            DIAG_INFO_UINT32(pos, "learning-fid", fid)
            value = GetDbgIpeMacLearningInfo(V, learningSrcPort_f, &mac_learn_info);
            if (!GetDbgIpeMacLearningInfo(V, learningUseGlobalSrcPort_f, &mac_learn_info))
            {
                DIAG_INFO_UINT32(pos, "learning-logic-port", value)
            }
            else
            {
                DIAG_INFO_UINT32(pos, "learning-global-port", SYS_MAP_DRV_GPORT_TO_CTC_GPORT(value))
            }
            DIAG_INFO_UINT32(pos, "aging-ptr", GetDbgIpeMacLearningInfo(V, agingIndex_f, &mac_learn_info))
        }
    }

    /* Drop Process */
    _sys_tsingma_diag_pkt_tracce_drop_map(p_rslt,
                                          GetDbgIpeMacLearningInfo(V, discard_f, &mac_learn_info),
                                          GetDbgIpeMacLearningInfo(V, discardType_f, &mac_learn_info));
    /* Exception Process */
    _sys_tsingma_diag_pkt_tracce_exception_map(p_rslt,
                                               GetDbgIpeMacLearningInfo(V, exceptionEn_f, &mac_learn_info),
                                               GetDbgIpeMacLearningInfo(V, exceptionIndex_f, &mac_learn_info),
                                               GetDbgIpeMacLearningInfo(V, exceptionSubIndex_f, &mac_learn_info));
    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_diag_pkt_trace_get_route_tbl_id(uint8 lchip, uint32 pri_valid, uint32 tcam_group_index, uint32 key_type, uint32* key_id, uint32* ad_id)
{
    uint32 cmd = 0;
    LpmTcamCtl_m lpm_tcam_ctl;

    sal_memset( &lpm_tcam_ctl, 0, sizeof(LpmTcamCtl_m));
    cmd = DRV_IOR(LpmTcamCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &lpm_tcam_ctl));

    if (SYS_TM_DIAG_L3DA_TYPE_IPV4UC == key_type)
    {
        *key_id = pri_valid ? DsLpmTcamIpv4HalfKey_t : DsLpmTcamIpv4DaPubHalfKey_t;
        *ad_id = pri_valid ? DsLpmTcamIpv4HalfKeyAd_t : DsLpmTcamIpv4DaPubHalfKeyAd_t;
    }
    else if (SYS_TM_DIAG_L3DA_TYPE_IPV6UC == key_type && !GetLpmTcamCtl(V, tcamGroupConfig_0_ipv6Lookup64Prefix_f + tcam_group_index, &lpm_tcam_ctl))
    {
        *key_id = pri_valid ? DsLpmTcamIpv6DoubleKey0_t : DsLpmTcamIpv6DaPubDoubleKey0_t;
        *ad_id = pri_valid ? DsLpmTcamIpv6DoubleKey0Ad_t : DsLpmTcamIpv6DaPubDoubleKey0Ad_t;
    }
    else if (SYS_TM_DIAG_L3DA_TYPE_IPV6UC == key_type && GetLpmTcamCtl(V, tcamGroupConfig_0_ipv6Lookup64Prefix_f + tcam_group_index, &lpm_tcam_ctl))
    {
        *key_id = pri_valid ? DsLpmTcamIpv6SingleKey_t : DsLpmTcamIpv6DaPubSingleKey_t;
        *ad_id = pri_valid ? DsLpmTcamIpv6SingleKeyAd_t : DsLpmTcamIpv6DaPubSingleKeyAd_t;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_diag_pkt_trace_map_ipuc_key_index(uint8 lchip, uint32 tbl_id, uint32 key_index, uint32* mapped_key_idx, uint32* mapped_ad_idx)
{
    uint8 block_num = ((key_index >> 12) & 0x3);
    uint8 blk_id = 0;
    uint16 blk_offset = 0;
    uint32 local_index = key_index & 0xFFF;/* use 50 bit as unit */
    uint32 block_size = (block_num <= 1) ? 512 : 1024;

    blk_id = local_index / block_size + block_num * 4;
    blk_offset = local_index - local_index / block_size * block_size;
    *mapped_key_idx = (TCAM_START_INDEX(lchip, tbl_id, blk_id) +  blk_offset) / (TCAM_KEY_SIZE(lchip, tbl_id) / DRV_LPM_KEY_BYTES_PER_ENTRY);
    *mapped_ad_idx = TCAM_START_INDEX(lchip, tbl_id, blk_id) +  blk_offset;

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_diag_pkt_trace_get_ipe_route_info(uint8 lchip, ctc_diag_pkt_trace_result_t* p_rslt)
{
    uint8 pos = CTC_DIAG_TRACE_POS_IPE;
    uint32 cmd = 0;
    uint32 value = 0;
    uint8 index = 0;
    uint32 key_index = 0;
    uint32 ad_index = 0;
    uint32 grp0_valid[6] = {0};
    uint32 grp0_index[6] = {0};
    uint32 grp1_valid[6] = {0};
    uint32 grp1_index[6] = {0};
    char* key_name = NULL;
    char* ad_name = NULL;
    DbgFibLkpEnginel3DaHashInfo_m l3_da_info;
    DbgFibLkpEnginel3SaHashInfo_m l3_sa_info;
    DbgFibLkpEngineHostUrpfHashInfo_m urpf_info;
    DbgLpmTcamEngineResult0Info_m lpm_res0;
    DbgLpmTcamEngineResult1Info_m lpm_res1;
    DbgIpeIpRoutingInfo_m routing_info;

    sal_memset(&l3_da_info, 0, sizeof(l3_da_info));
    cmd = DRV_IOR(DbgFibLkpEnginel3DaHashInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &l3_da_info));
    sal_memset(&l3_sa_info, 0, sizeof(l3_sa_info));
    cmd = DRV_IOR(DbgFibLkpEnginel3SaHashInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &l3_sa_info));
    sal_memset(&urpf_info, 0, sizeof(urpf_info));
    cmd = DRV_IOR(DbgFibLkpEngineHostUrpfHashInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &urpf_info));
    sal_memset(&routing_info, 0, sizeof(routing_info));
    cmd = DRV_IOR(DbgIpeIpRoutingInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &routing_info));
    sal_memset(&lpm_res0, 0, sizeof(lpm_res0));
    cmd = DRV_IOR(DbgLpmTcamEngineResult0Info_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &lpm_res0));
    for (index = 0; index < 6; index++)
    {
        DRV_IOR_FIELD(lchip, DbgLpmTcamEngineResult0Info_t, DbgLpmTcamEngineResult0Info_gTcamGroup_0_lookupResultValid_f+index*2,
            &grp0_valid[index], &lpm_res0);

        DRV_IOR_FIELD(lchip, DbgLpmTcamEngineResult0Info_t, DbgLpmTcamEngineResult0Info_gTcamGroup_0_tcamHitIndex_f+index*2,
            &grp0_index[index], &lpm_res0);
    }
    sal_memset(&lpm_res1, 0, sizeof(lpm_res1));
    cmd = DRV_IOR(DbgLpmTcamEngineResult1Info_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &lpm_res1));
    for (index = 0; index < 6; index++)
    {
        DRV_IOR_FIELD(lchip, DbgLpmTcamEngineResult1Info_t, DbgLpmTcamEngineResult1Info_gTcamGroup_1_lookupResultValid_f+index*2,
            &grp1_valid[index], &lpm_res1);

        DRV_IOR_FIELD(lchip, DbgLpmTcamEngineResult1Info_t, DbgLpmTcamEngineResult1Info_gTcamGroup_1_tcamHitIndex_f+index*2,
            &grp1_index[index], &lpm_res1);
    }

    if (!GetDbgIpeIpRoutingInfo(V, valid_f, &routing_info))
    {
        return CTC_E_NONE;
    }

    if (!(GetDbgFibLkpEnginel3DaHashInfo(V, l3DaLookupEn_f, &l3_da_info)
        || GetDbgFibLkpEnginel3SaHashInfo(V, l3SaLookupEn_f, &l3_sa_info)))
    {
        return CTC_E_NONE;
    }
    p_rslt->position = CTC_DIAG_TRACE_POS_IPE;
    DIAG_INFO_NONE(pos, "Layer3 Routing")
    if (GetDbgFibLkpEnginel3DaHashInfo(V, l3DaLookupEn_f, &l3_da_info))
    {
        if (GetDbgIpeIpRoutingInfo(V, ipHeaderError_f, &routing_info))
        {
            DIAG_INFO_CHAR(pos, "ipda-lookup-result", "IP-HEADER-ERROR")
        }
        else if (GetDbgIpeIpRoutingInfo(V, ipDaResultValid_f, &routing_info))
        {
            if (GetDbgFibLkpEnginel3DaHashInfo(V, hostHashFirstL3DaResultValid_f, &l3_da_info)
                || GetDbgFibLkpEnginel3DaHashInfo(V, hostHashSecondDaResultValid_f, &l3_da_info))
            {
                char* key_name = NULL;
                DIAG_INFO_CHAR(pos, "ipda-lookup-result", "HIT-HASH")
                DIAG_INFO_UINT32(pos, "vrf-id", GetDbgFibLkpEnginel3DaHashInfo(V, vrfId_f, &l3_da_info))

                value = GetDbgFibLkpEnginel3DaHashInfo(V, l3DaKeyType_f, &l3_da_info);
                if ((SYS_TM_DIAG_L3DA_TYPE_IPV4UC == value) ||
                    (SYS_TM_DIAG_L3DA_TYPE_IPV4MC == value))/* FIBDAKEYTYPE_IPV4UCAST or FIBDAKEYTYPE_IPV4MCAST */
                {
                    key_name = "DsFibHost0Ipv4HashKey";
                    ad_name = "DsIpDa";
                }
                else if (SYS_TM_DIAG_L3DA_TYPE_IPV6UC == value)/* FIBDAKEYTYPE_IPV6UCAST */
                {
                    key_name = "DsFibHost0Ipv6UcastHashKey";
                    ad_name = "DsIpDa";
                }
                else if (SYS_TM_DIAG_L3DA_TYPE_IPV6MC == value)/* FIBDAKEYTYPE_IPV6MCAST */
                {
                    key_name = "DsFibHost0MacIpv6McastHashKey";
                    ad_name = "DsIpDa";
                }
                else if (SYS_TM_DIAG_L3DA_TYPE_TRILLUC == value)
                {
                    key_name = "DsFibHost0TrillHashKey";
                    ad_name = "DsTrillDa";
                }
                else if (SYS_TM_DIAG_L3DA_TYPE_TRILLMC == value)
                {
                    key_name = "DsFibHost1TrillMcastVlanHashKey";
                    ad_name = "DsTrillDa";
                }
                else if (SYS_TM_DIAG_L3DA_TYPE_FCOE== value)
                {
                    key_name = "DsFibHost0FcoeHashKey";
                    ad_name = "DsFcoeDa";
                }
                else
                {
                    key_name = "Unknown..";
                    ad_name = "Unknown..";
                }

                DIAG_INFO_CHAR(pos, "key-name", key_name)
                value = GetDbgFibLkpEnginel3DaHashInfo(V, hostHashFirstL3DaResultValid_f, &l3_da_info)
                        ? GetDbgFibLkpEnginel3DaHashInfo(V, hostHashFirstL3DaKeyIndex_f, &l3_da_info)
                        : GetDbgFibLkpEnginel3DaHashInfo(V, hostHashSecondDaKeyIndex_f, &l3_da_info);
                DIAG_INFO_UINT32(pos, "key-index", value)
                DIAG_INFO_CHAR(pos, "ad-name", ad_name)
                DIAG_INFO_UINT32(pos, "ad-index", GetDbgFibLkpEnginel3DaHashInfo(V, l3DaAdindexForDebug_f, &l3_da_info))
            }
            else if (GetDbgFibLkpEnginel3DaHashInfo(V, l3DaTcamLookup1PrivateResultValid_f, &l3_da_info)
                || GetDbgFibLkpEnginel3DaHashInfo(V, l3DaTcamLookup1PublicResultValid_f, &l3_da_info)
                || GetDbgFibLkpEnginel3DaHashInfo(V, l3DaTcamLookup2PrivateResultValid_f, &l3_da_info)
                || GetDbgFibLkpEnginel3DaHashInfo(V, l3DaTcamLookup2PublicResultValid_f, &l3_da_info))
            {
                char str[64] = {0};
                uint32 cmd1 = 0;
                uint32 cmd2 = 0;
                uint32 key_table_id = 0;
                uint32 ad_table_id = 0;
                uint32 mapped_key_index = 0;
                uint32 mapped_ad_index = 0;
                DIAG_INFO_CHAR(pos, "ipda-lookup-result", "HIT-LPM-TCAM")

                if (GetDbgFibLkpEnginel3DaHashInfo(V, l3DaTcamLookup1PrivateResultValid_f, &l3_da_info))
                {
                    for (index = 0; index < 6; index++)
                    {
                        if (!grp0_valid[index])
                        {
                            continue;
                        }
                        key_index = grp0_index[index];
                        sal_printf(str, "lookup1-private-block-%d", index);
                        DIAG_INFO_CHAR(pos, "tcam-key", str)
                        break;
                    }
                }

                if (GetDbgFibLkpEnginel3DaHashInfo(V, l3DaTcamLookup1PublicResultValid_f, &l3_da_info))
                {
                    for (index = 0; index < 6; index++)
                    {
                        if (!grp0_valid[index])
                        {
                            continue;
                        }
                        key_index = grp0_index[index];
                        sal_printf(str, "lookup1-public-block-%d", index);
                        DIAG_INFO_CHAR(pos, "tcam-key", str)
                        break;
                    }
                }
                if (GetDbgFibLkpEnginel3DaHashInfo(V, l3DaTcamLookup2PrivateResultValid_f, &l3_da_info))
                {
                    for (index = 0; index < 6; index++)
                    {
                        if (!grp1_valid[index])
                        {
                            continue;
                        }
                        key_index = grp1_index[index];
                        sal_printf(str, "lookup2-private-block-%d", index);
                        DIAG_INFO_CHAR(pos, "tcam-key", str)
                        break;
                    }
                }
                if (GetDbgFibLkpEnginel3DaHashInfo(V, l3DaTcamLookup2PublicResultValid_f, &l3_da_info))
                {
                    for (index = 0; index < 6; index++)
                    {
                        if (!grp1_valid[index])
                        {
                            continue;
                        }
                        key_index = grp1_index[index];
                        sal_printf(str, "lookup2-public-block-%d", index);
                        DIAG_INFO_CHAR(pos, "tcam-key", str)
                        break;
                    }
                }

                value = GetDbgFibLkpEnginel3DaHashInfo(V, l3DaKeyType_f, &l3_da_info);
                CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_get_route_tbl_id(lchip,
                                        GetDbgFibLkpEnginel3DaHashInfo(V, l3DaTcamLookup1PrivateResultValid_f, &l3_da_info),
                                        index, value, &key_table_id, &ad_table_id));

                if (SYS_TM_DIAG_L3DA_TYPE_IPV4UC == value)
                {
                    key_name = TABLE_NAME(lchip, key_table_id);
                    ad_name = TABLE_NAME(lchip, ad_table_id);
                    cmd = DRV_IOR(ad_table_id, DsLpmTcamIpv4HalfKeyAd_lpmPipelineValid_f);
                    cmd1 = DRV_IOR(ad_table_id, DsLpmTcamIpv4HalfKeyAd_nexthop_f);
                    cmd2 = DRV_IOR(ad_table_id, DsLpmTcamIpv4HalfKeyAd_pointer_f);

                    _sys_tsingma_diag_pkt_trace_map_ipuc_key_index(lchip, DsLpmTcamIpv4HalfKey_t, key_index, &mapped_key_index, &mapped_ad_index);
                }
                else if (SYS_TM_DIAG_L3DA_TYPE_IPV6UC == value)
                {
                    key_name = TABLE_NAME(lchip, key_table_id);
                    ad_name = TABLE_NAME(lchip, ad_table_id);
                    cmd = DRV_IOR(ad_table_id, DsLpmTcamIpv6DoubleKey0Ad_lpmPipelineValid_f);
                    cmd1 = DRV_IOR(ad_table_id, DsLpmTcamIpv6DoubleKey0Ad_nexthop_f);
                    cmd2 = DRV_IOR(ad_table_id, DsLpmTcamIpv6DoubleKey0Ad_pointer_f);

                    _sys_tsingma_diag_pkt_trace_map_ipuc_key_index(lchip, DsLpmTcamIpv6DoubleKey0_t, key_index, &mapped_key_index, &mapped_ad_index);
                }
                DIAG_INFO_UINT32(pos, "spec-tcam-index", key_index)
                DIAG_INFO_CHAR(pos, "key-name", key_name)
                DIAG_INFO_UINT32(pos, "key-index", mapped_key_index)
                DIAG_INFO_CHAR(pos, "tcam-ad-name", ad_name)
                DIAG_INFO_UINT32(pos, "tcam-ad-index", mapped_ad_index)

                CTC_ERROR_RETURN(DRV_IOCTL(lchip, mapped_ad_index, cmd, &value));

                if (!value)
                {
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, mapped_ad_index, cmd1, &ad_index));
                    DIAG_INFO_CHAR(pos, "ad-name", "DsIpDa")
                    DIAG_INFO_UINT32(pos, "ad-index", ad_index)
                }
                else
                {
                    uint32 lpm_type0 = 0;
                    uint32 lpm_type1 = 0;
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, mapped_ad_index, cmd2, &key_index));

                    DIAG_INFO_CHAR(pos, "LPM-pipeline-0-lookup", DIAG_LOOKUP_HIT)

                    /*Later should optimise, dbg info should record lpm pipeline index*/

                    cmd = DRV_IOR(DsLpmLookupKey_t, DsLpmLookupKey_type0_f);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, key_index, cmd, &lpm_type0));
                    cmd = DRV_IOR(DsLpmLookupKey_t, DsLpmLookupKey_type1_f);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, key_index, cmd, &lpm_type1));
                    if ((lpm_type0 == 1) || (lpm_type0 == 2))
                    {
                        cmd = DRV_IOR(DsLpmLookupKey_t, DsLpmLookupKey_nexthop0_f);
                        CTC_ERROR_RETURN(DRV_IOCTL(lchip, key_index, cmd, &ad_index));
                    }
                    else if ((lpm_type1 == 1) || (lpm_type0 == 2))
                    {
                        cmd = DRV_IOR(DsLpmLookupKey_t, DsLpmLookupKey_nexthop1_f);
                        CTC_ERROR_RETURN(DRV_IOCTL(lchip, key_index, cmd, &ad_index));
                    }

                    if ((lpm_type0 == 2) || (lpm_type1 == 2))
                    {
                        value = 1;
                    }
                    else
                    {
                        value = 0;
                    }
                    DIAG_INFO_CHAR(pos, "key-name", "DsLpmLookupKey")
                    DIAG_INFO_UINT32(pos, "key-index", key_index)

                    if (!value)
                    {
                        DIAG_INFO_CHAR(pos, "ad-name", "DsIpDa")
                        DIAG_INFO_UINT32(pos, "ad-index", ad_index)
                    }
                    else
                    {
                        key_index = ad_index;
                        DIAG_INFO_CHAR(pos, "LPM-pipeline-1-lookup", DIAG_LOOKUP_HIT)

                        cmd = DRV_IOR(DsLpmLookupKey_t, DsLpmLookupKey_type0_f);
                        CTC_ERROR_RETURN(DRV_IOCTL(lchip, key_index, cmd, &lpm_type0));
                        cmd = DRV_IOR(DsLpmLookupKey_t, DsLpmLookupKey_type1_f);
                        CTC_ERROR_RETURN(DRV_IOCTL(lchip, key_index, cmd, &lpm_type1));
                        if (lpm_type0 == 1)
                        {
                            cmd = DRV_IOR(DsLpmLookupKey_t, DsLpmLookupKey_nexthop0_f);
                            CTC_ERROR_RETURN(DRV_IOCTL(lchip, key_index, cmd, &ad_index));
                        }
                        else if (lpm_type1 == 1)
                        {
                            cmd = DRV_IOR(DsLpmLookupKey_t, DsLpmLookupKey_nexthop1_f);
                            CTC_ERROR_RETURN(DRV_IOCTL(lchip, key_index, cmd, &ad_index));
                        }
                        DIAG_INFO_CHAR(pos, "key-name", "DsLpmLookupKey")
                        DIAG_INFO_UINT32(pos, "key-index", key_index)
                        DIAG_INFO_CHAR(pos, "ad-name", "DsIpDa")
                        DIAG_INFO_UINT32(pos, "ad-index", ad_index)
                    }
                }
            }
        }
        else
        {
            DIAG_INFO_CHAR(pos, "ipda-lookup-result", DIAG_LOOKUP_HIT_NONE)
        }
    }

    if (GetDbgFibLkpEnginel3SaHashInfo(V, l3SaLookupEn_f, &l3_sa_info))
    {
        if (GetDbgIpeIpRoutingInfo(V, ipSaResultValid_f, &routing_info))
        {
            char* result = NULL;


            value = GetDbgFibLkpEnginel3SaHashInfo(V, l3SaKeyType_f, &l3_sa_info);
            if (value == SYS_TM_DIAG_L3SA_TYPE_IPV4NATSA)
            {
                if (GetDbgFibLkpEnginel3SaHashInfo(V, hostHashFirstSaNatTcamResultValid_f, &l3_sa_info))
                {
                    result = "HIT-LPM-TCAM-IPv4-NAT";
                    key_name = "DsLpmTcamIpv4NatDoubleKey";
                    key_index = GetDbgFibLkpEnginel3SaHashInfo(V, hostHashFirstSaTcamHitIndex_f, &l3_sa_info);
                }
                else if (GetDbgFibLkpEnginel3SaHashInfo(V, l3SaResultValid_f, &l3_sa_info))
                {
                    result = "HIT-HASH-IPv4-NAT";
                    key_name = "DsFibHost1Ipv4NatSaPortHashKey";
                    key_index = GetDbgFibLkpEnginel3SaHashInfo(V, hostHashSecondSaKeyIndex_f, &l3_sa_info);
                }
            }
            else if (value == SYS_TM_DIAG_L3SA_TYPE_IPV6NATSA)
            {
                if (GetDbgFibLkpEnginel3SaHashInfo(V, hostHashFirstSaNatTcamResultValid_f, &l3_sa_info))
                {
                    result = "HIT-LPM-TCAM-IPv6-NAT";
                    key_name = "DsLpmTcamIpv6DoubleKey0";
                    key_index = GetDbgFibLkpEnginel3SaHashInfo(V, hostHashFirstSaTcamHitIndex_f, &l3_sa_info);
                }
                else if (GetDbgFibLkpEnginel3SaHashInfo(V, l3SaResultValid_f, &l3_sa_info))
                {
                    result = "HIT-HASH-IPv6-NAT";
                    key_name = "DsFibHost1Ipv6NatSaPortHashKey";
                    key_index = GetDbgFibLkpEnginel3SaHashInfo(V, hostHashSecondSaKeyIndex_f, &l3_sa_info);
                }
            }
            else if (value == SYS_TM_DIAG_L3SA_TYPE_IPV4RPF)
            {
                if (GetDbgFibLkpEnginel3SaHashInfo(V, hostHashFirstSaRpfTcamResultValid_f, &l3_sa_info))
                {
                    result = "HIT-LPM-TCAM-IPv4-RPF";
                    key_name = "DsLpmTcamIpv4HalfKey";
                    key_index = GetDbgFibLkpEnginel3SaHashInfo(V, hostHashFirstSaTcamHitIndex_f, &l3_sa_info);
                }
                else if (GetDbgFibLkpEnginel3SaHashInfo(V, l3SaResultValid_f, &l3_sa_info))
                {
                    result = "HIT-HASH-IPv4-RPF";
                    key_name = "DsFibHost0Ipv4HashKey";
                    key_index = GetDbgFibLkpEngineHostUrpfHashInfo(V, hostUrpfHashHitIndex_f, &urpf_info);
                }
            }
            else if (value == SYS_TM_DIAG_L3SA_TYPE_IPV6RPF)
            {
                if (GetDbgFibLkpEnginel3SaHashInfo(V, hostHashFirstSaRpfTcamResultValid_f, &l3_sa_info))
                {
                    result = "HIT-LPM-TCAM-IPv6-RPF";
                    key_name = "DsLpmTcamIpv6DoubleKey0";
                    key_index = GetDbgFibLkpEnginel3SaHashInfo(V, hostHashFirstSaTcamHitIndex_f, &l3_sa_info);
                }
                else if (GetDbgFibLkpEnginel3SaHashInfo(V, l3SaResultValid_f, &l3_sa_info))
                {
                    result = "HIT-HASH-IPv6-RPF";
                    key_name = "DsFibHost0Ipv6UcastHashKey";
                    key_index = GetDbgFibLkpEnginel3SaHashInfo(V, hostHashSecondSaKeyIndex_f, &l3_sa_info);
                }
            }
            else if (value == SYS_TM_DIAG_L3SA_TYPE_FCOERPF)
            {
                result = "HIT-HASH-FCoE-RPF";
                key_name = "DsFibHost1FcoeRpfHashKey";
                key_index = GetDbgFibLkpEnginel3SaHashInfo(V, hostHashSecondSaKeyIndex_f, &l3_sa_info);
            }

            DIAG_INFO_CHAR(pos, "ipda-lookup-result", result)
            DIAG_INFO_CHAR(pos, "key-name", key_name)
            DIAG_INFO_UINT32(pos, "key-index", key_index)
        }
        else
        {
            DIAG_INFO_CHAR(pos, "ipsa-lookup-result", DIAG_LOOKUP_HIT_NONE)
        }
    }

    if (GetDbgIpeIpRoutingInfo(V, routingEscape_f, &routing_info))
    {
        DIAG_INFO_CHAR(pos, "routing-escape", "ENABLE")
    }

    /* Drop Process */
    _sys_tsingma_diag_pkt_tracce_drop_map(p_rslt,
                                          GetDbgIpeIpRoutingInfo(V, discard_f, &routing_info),
                                          GetDbgIpeIpRoutingInfo(V, discardType_f, &routing_info));
    /* Exception Process */
    _sys_tsingma_diag_pkt_tracce_exception_map(p_rslt,
                                               GetDbgIpeIpRoutingInfo(V, exceptionEn_f, &routing_info),
                                               GetDbgIpeIpRoutingInfo(V, exceptionIndex_f, &routing_info),
                                               GetDbgIpeIpRoutingInfo(V, exceptionSubIndex_f, &routing_info));
    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_diag_pkt_trace_get_lag_info(uint8 lchip, ctc_diag_pkt_trace_result_t* p_rslt)
{
    uint32 cmd = 0;
    uint8 pos = CTC_DIAG_TRACE_POS_IPE;
    IpeToLagEngineInput_m ipe_lag_input;
    LagEngineOutput_m  lag_ouput;
    DbgLagEngineInfoFromIpeFwd_m lag_info_fwd;

    sal_memset(&lag_info_fwd, 0, sizeof(DbgLagEngineInfoFromIpeFwd_m));
    cmd = DRV_IOR(DbgLagEngineInfoFromIpeFwd_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &lag_info_fwd));
    GetDbgLagEngineInfoFromIpeFwd(A, lagEngineInput_f, &lag_info_fwd, &ipe_lag_input);
    GetDbgLagEngineInfoFromIpeFwd(A, lagEngineOutput_f, &lag_info_fwd, &lag_ouput);

    if (!GetDbgLagEngineInfoFromIpeFwd(V, valid_f, &lag_info_fwd))
    {
        return CTC_E_NONE;
    }

    if (GetLagEngineOutput(V, sgmacLagEnable_f, &lag_ouput)
        || GetLagEngineOutput(V, portLagEn_f, &lag_ouput))
    {
        if (GetLagEngineOutput(V, sgmacLagEnable_f, &lag_ouput))
        {
            DIAG_INFO_CHAR(pos, "Linkagg Process", "sgmac-linkagg")
            DIAG_INFO_UINT32(pos, "cFlexSgGroupSel", GetIpeToLagEngineInput(V, cFlexFwdSgGroupSel_f, &ipe_lag_input))
            DIAG_INFO_UINT32(pos, "cFlexLagHash", GetIpeToLagEngineInput(V, cFlexLagHash_f, &ipe_lag_input))
            DIAG_INFO_UINT32(pos, "stackingDiscard", GetLagEngineOutput(V, stackingDiscard_f, &lag_ouput))
            DIAG_INFO_UINT32(pos, "dest-sgmac-group", GetLagEngineOutput(V, destSgmacGroupId_f, &lag_ouput))
            p_rslt->tid = GetLagEngineOutput(V, destSgmacGroupId_f, &lag_ouput);
            p_rslt->flags |= CTC_DIAG_TRACE_RSTL_FLAG_TID;
        }
        if (GetLagEngineOutput(V, portLagEn_f, &lag_ouput))
        {
            DIAG_INFO_CHAR(pos, "Linkagg Process", "port-linkagg")
            DIAG_INFO_UINT32(pos, "linkagg-id", GetIpeToLagEngineInput(V, destMap_f, &ipe_lag_input) & CTC_LINKAGGID_MASK)
            DIAG_INFO_UINT32(pos, "hash-value", GetLagEngineOutput(V, portLagSlbHash_f, &lag_ouput))
            p_rslt->tid = GetIpeToLagEngineInput(V, destMap_f, &ipe_lag_input) & CTC_LINKAGGID_MASK;
            p_rslt->flags |= CTC_DIAG_TRACE_RSTL_FLAG_TID;
        }
        if (GetLagEngineOutput(V, destChannelIdValid_f, &lag_ouput))
        {
            DIAG_INFO_UINT32(pos, "dest-channel", GetLagEngineOutput(V, destChannelId_f, &lag_ouput))
        }
        DIAG_INFO_U32_HEX(pos, "dest-map", GetLagEngineOutput(V, updateDestMap_f, &lag_ouput))
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_diag_pkt_trace_get_ipe_phb_info(uint8 lchip, ctc_diag_pkt_trace_result_t* p_rslt)
{
    uint32 cmd = 0;
    uint8 pos = CTC_DIAG_TRACE_POS_IPE;
    DbgIpePerHopBehaviorInfo_m dbg_phb_info;
    char*  str_color[] = {"None", "Red", "Yellow", "Green"};

    /*phb process*/
    sal_memset(&dbg_phb_info, 0, sizeof(DbgIpePerHopBehaviorInfo_m));
    cmd = DRV_IOR(DbgIpePerHopBehaviorInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &dbg_phb_info));

    if(GetDbgIpePerHopBehaviorInfo(V, valid_f, &dbg_phb_info))
    {
        p_rslt->position = CTC_DIAG_TRACE_POS_IPE;
        DIAG_INFO_NONE(pos, "Per-hop Behavior")
        DIAG_INFO_CHAR(pos, "phb-color", str_color[GetDbgIpePerHopBehaviorInfo(V, color_f, &dbg_phb_info)])
        DIAG_INFO_UINT32(pos, "phb-prio", GetDbgIpePerHopBehaviorInfo(V, prio_f, &dbg_phb_info))
        DIAG_INFO_UINT32(pos, "phb-classified-cfi", GetDbgIpePerHopBehaviorInfo(V, classifiedCfi_f, &dbg_phb_info))
        DIAG_INFO_UINT32(pos, "phb-classified-cos", GetDbgIpePerHopBehaviorInfo(V, classifiedCos_f, &dbg_phb_info))
        DIAG_INFO_UINT32(pos, "phb-classified-dscp", GetDbgIpePerHopBehaviorInfo(V, classifiedDscp_f, &dbg_phb_info))
        DIAG_INFO_UINT32(pos, "phb-classified-tc", GetDbgIpePerHopBehaviorInfo(V, classifiedTc_f, &dbg_phb_info))
        if (GetDbgIpePerHopBehaviorInfo(V, srcDscpValid_f, &dbg_phb_info))
        {
            DIAG_INFO_UINT32(pos, "phb-src-dscp",  GetDbgIpePerHopBehaviorInfo(V, srcDscp_f, &dbg_phb_info))
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_diag_pkt_trace_get_ipe_oam_info(uint8 lchip, ctc_diag_pkt_trace_result_t* p_rslt)
{
    uint32 cmd = 0;
    uint8  idx = 0;
    uint32 value = 0;
    uint8 pos = CTC_DIAG_TRACE_POS_IPE;
    char* lookup_result[4] = {"ETHER", "BFD", "TP", "Section"};
    char* key_name[4] = {"DsEgressXcOamEthHashKey", "DsEgressXcOamBfdHashKey",
                                             "DsEgressXcOamMplsLabelHashKey", "DsEgressXcOamMplsSectionHashKey"};
    DbgEgrXcOamHashEngineFromIpeOam0Info_m oam0_info;
    DbgEgrXcOamHashEngineFromIpeOam1Info_m oam1_info;
    DbgEgrXcOamHashEngineFromIpeOam2Info_m oam2_info;
    DbgIpeLkpMgrInfo_m lkp_mgr_info;
    DbgIpeOamInfo_m ipe_oam_info;
    uint8 key_type[3] = {0};
    uint8 hash_valid[3] = {0};
    uint8 confict[3] = {0};
    uint32 key_index[3] = {0};
    uint8 hash_resultValid[3] = {0};

    sal_memset(&oam0_info, 0, sizeof(oam0_info));
    cmd = DRV_IOR(DbgEgrXcOamHashEngineFromIpeOam0Info_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam0_info));
    sal_memset(&oam1_info, 0, sizeof(oam1_info));
    cmd = DRV_IOR(DbgEgrXcOamHashEngineFromIpeOam1Info_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam1_info));
    sal_memset(&oam2_info, 0, sizeof(oam2_info));
    cmd = DRV_IOR(DbgEgrXcOamHashEngineFromIpeOam2Info_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam2_info));
    sal_memset(&lkp_mgr_info, 0, sizeof(lkp_mgr_info));
    cmd = DRV_IOR(DbgIpeLkpMgrInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &lkp_mgr_info));
    sal_memset(&ipe_oam_info, 0, sizeof(ipe_oam_info));
    cmd = DRV_IOR(DbgIpeOamInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_oam_info));

    key_type[0] = GetDbgIpeLkpMgrInfo(V, ingressOamKeyType0_f, &lkp_mgr_info);
    key_type[1] = GetDbgIpeLkpMgrInfo(V, ingressOamKeyType1_f, &lkp_mgr_info);
    key_type[2] = GetDbgIpeLkpMgrInfo(V, ingressOamKeyType2_f, &lkp_mgr_info);

    hash_resultValid[0] = GetDbgEgrXcOamHashEngineFromIpeOam0Info(V, resultValid_f, &oam0_info);
    confict[0] = GetDbgEgrXcOamHashEngineFromIpeOam0Info(V, hashConflict_f, &oam0_info);
    key_index[0] = GetDbgEgrXcOamHashEngineFromIpeOam0Info(V, keyIndex_f, &oam0_info);
    hash_valid[0] = GetDbgEgrXcOamHashEngineFromIpeOam0Info(V, valid_f, &oam0_info);

    hash_resultValid[1] = GetDbgEgrXcOamHashEngineFromIpeOam1Info(V, resultValid_f, &oam1_info);
    confict[1] = GetDbgEgrXcOamHashEngineFromIpeOam1Info(V, hashConflict_f, &oam1_info);
    key_index[1] = GetDbgEgrXcOamHashEngineFromIpeOam1Info(V, keyIndex_f, &oam1_info);
    hash_valid[1] = GetDbgEgrXcOamHashEngineFromIpeOam1Info(V, valid_f, &oam1_info);

    hash_resultValid[2] = GetDbgEgrXcOamHashEngineFromIpeOam2Info(V, resultValid_f, &oam2_info);
    confict[2] = GetDbgEgrXcOamHashEngineFromIpeOam2Info(V, hashConflict_f, &oam2_info);
    key_index[2] = GetDbgEgrXcOamHashEngineFromIpeOam2Info(V, keyIndex_f, &oam2_info);
    hash_valid[2] = GetDbgEgrXcOamHashEngineFromIpeOam2Info(V, valid_f, &oam2_info);

    if (!GetDbgIpeOamInfo(V, valid_f, &ipe_oam_info))
    {
        return CTC_E_NONE;
    }
    p_rslt->position = CTC_DIAG_TRACE_POS_IPE;
    DIAG_INFO_NONE(pos, "IPE OAM")
    if (key_type[0] || key_type[1] || key_type[2])
    {
        char str[64] = {0};
        for (idx = 0; idx < 3; idx++)
        {
            if (!key_type[idx] || !hash_valid[idx])
            {
                continue;
            }
            /*EGRESSXCOAMHASHTYPE_ETH = 0x10*/
            value = key_type[idx] - 0x10;
            sal_sprintf(str, "hash-%d", idx);
            DIAG_INFO_CHAR(pos, str, lookup_result[value])
            if (confict[idx])
            {
                DIAG_INFO_CHAR(pos, "lookup-result", DIAG_LOOKUP_CONFLICT)
                DIAG_INFO_CHAR(pos, "key-name", key_name[value])
                DIAG_INFO_UINT32(pos, "key-index", key_index[idx])
            }
            else if (hash_resultValid[idx])
            {
                DIAG_INFO_CHAR(pos, "lookup-result", DIAG_LOOKUP_HIT)
                DIAG_INFO_CHAR(pos, "key-name", key_name[value])
                DIAG_INFO_UINT32(pos, "key-index", key_index[idx])
            }
            else
            {
                DIAG_INFO_CHAR(pos, "lookup-result", DIAG_LOOKUP_HIT_NONE)
            }
        }
    }

    if (hash_resultValid[0] || hash_resultValid[1] || hash_resultValid[2])
    {
        value = GetDbgIpeOamInfo(V, tempRxOamType_f, &ipe_oam_info);
        if (value)
        {
            /*RXOAMTYPE_TRILLBFD = 0xa*/
            char* oam_type[10] =
            {
                "ETHER OAM", "IP BFD", "PBT OAM", "PBBBSI", "PBBBV",
                "MPLS OAM", "MPLS BFD", "ACH OAM", "RSV", "TRILL BFD"
            };
            DIAG_INFO_CHAR(pos, "oam-type", oam_type[value - 1])
        }

        value = GetDbgIpeOamInfo(V, oamDestChipId_f, &ipe_oam_info);
        if (GetDbgIpeOamInfo(V, mepEn_f, &ipe_oam_info))
        {
            DIAG_INFO_CHAR(pos, "MEP", "HIT")
            DIAG_INFO_UINT32(pos, "master-chip-id", value)
            DIAG_INFO_UINT32(pos, "mep-index", GetDbgIpeOamInfo(V, mepIndex_f, &ipe_oam_info))
        }
        if (GetDbgIpeOamInfo(V, mipEn_f, &ipe_oam_info))
        {
            DIAG_INFO_CHAR(pos, "MIP", "HIT")
            DIAG_INFO_UINT32(pos, "master-chip-id", value)
        }

        if (GetDbgIpeOamInfo(V, mplsLmValid_f, &ipe_oam_info) || GetDbgIpeOamInfo(V, etherLmValid_f, &ipe_oam_info))
        {
            DIAG_INFO_CHAR(pos, "action", "read-LM")
            DIAG_INFO_CHAR(pos, "table-name", "DsOamLmStats")
            DIAG_INFO_UINT32(pos, "table-index", GetDbgIpeOamInfo(V, lmStatsIndex_f, &ipe_oam_info))

        }
        if (GetDbgIpeOamInfo(V, lmStatsEn0_f, &ipe_oam_info))
        {
            DIAG_INFO_CHAR(pos, "action", "write-LM")
            DIAG_INFO_CHAR(pos, "table-name", "DsOamLmStats")
            DIAG_INFO_UINT32(pos, "table-index", GetDbgIpeOamInfo(V, lmStatsPtr0_f, &ipe_oam_info))
        }
        if (GetDbgIpeOamInfo(V, lmStatsEn1_f, &ipe_oam_info))
        {
            DIAG_INFO_CHAR(pos, "action", "write-LM")
            DIAG_INFO_CHAR(pos, "table-name", "DsOamLmStats")
            DIAG_INFO_UINT32(pos, "table-index", GetDbgIpeOamInfo(V, lmStatsPtr1_f, &ipe_oam_info))
        }
        if (GetDbgIpeOamInfo(V, lmStatsEn2_f, &ipe_oam_info))
        {
            DIAG_INFO_CHAR(pos, "action", "write-LM")
            DIAG_INFO_CHAR(pos, "table-name", "DsOamLmStats")
            DIAG_INFO_UINT32(pos, "table-index", GetDbgIpeOamInfo(V, lmStatsPtr2_f, &ipe_oam_info))
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_diag_pkt_trace_get_oam_rx_info(uint8 lchip, ctc_diag_pkt_trace_result_t* p_rslt)
{
    uint32 cmd = 0;
    uint8 pos = CTC_DIAG_TRACE_POS_OAM;
    char* oam_pdu_type[40] = {"OAM-TYPE-NONE", "ETHER-CCM", "ETHER-APS", "ETHER-1DM", "ETHER-DMM",
                          "ETHER-DMR", "ETHER-RAPS", "ETHER-LBM", "ETHER-LBR", "ETHER-LMR",
                          "ETHER-LMM", "MPLSTP-LBM", "ETHER-TST", "ETHER-LTR", "ETHER-LTM",
                          "ETHER-OTHER", "MPLSTP-DM", "MPLSTP-DLM", "MPLSTP-DLMDM", "MPLSTP-CV",
                          "MPLSTP-CSF", "ETHER-CSF", "ETHER-MCC", "MPLS-OTHER", "MPLSTP-MCC",
                          "MPLSTP-FM", "ETHER-SLM", "ETHER-SLR", "MPLS-OAM", "BFD-OAM",
                          "ETHER-SCC", "MPLSTP-SCC", "FLEX-TEST", "TWAMP", "ETHER-1SL",
                          "ETHER-LLM", "ETHER-LLR", "ETHER-SCM", "ETHER-SCR"};

    DbgOamHdrAdj_m  oam_hdr_adj;
    DbgOamParser_m   oam_parser;
    DbgOamRxProc_m  oam_rx_proc;
    DbgOamDefectProc_m  oam_defect_proc;
    DbgOamHdrEdit_m     oam_hdr_edit;
    DbgOamApsSwitch_m oam_aps_switch;
    DbgOamApsProcess_m oam_aps_proc;

    sal_memset(&oam_hdr_adj, 0, sizeof(oam_hdr_adj));
    cmd = DRV_IOR(DbgOamHdrAdj_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_hdr_adj));

    sal_memset(&oam_parser, 0, sizeof(oam_parser));
    cmd = DRV_IOR(DbgOamParser_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_parser));

    sal_memset(&oam_rx_proc, 0, sizeof(oam_rx_proc));
    cmd = DRV_IOR(DbgOamRxProc_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_rx_proc));

    sal_memset(&oam_defect_proc, 0, sizeof(oam_defect_proc));
    cmd = DRV_IOR(DbgOamDefectProc_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_defect_proc));

    sal_memset(&oam_hdr_edit, 0, sizeof(oam_hdr_edit));
    cmd = DRV_IOR(DbgOamHdrEdit_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_hdr_edit));

    sal_memset(&oam_aps_switch, 0, sizeof(oam_aps_switch));
    cmd = DRV_IOR(DbgOamApsSwitch_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_aps_switch));

    sal_memset(&oam_aps_proc, 0, sizeof(oam_aps_proc));
    cmd = DRV_IOR(DbgOamApsProcess_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_aps_switch));

    if (!(GetDbgOamApsSwitch(V, oamApsSwitchValid_f, &oam_aps_switch)
           || GetDbgOamHdrEdit(V, oamHdrEditValid_f, &oam_hdr_edit)
           || GetDbgOamDefectProc(V, oamDefectProcValid_f, &oam_defect_proc)
           || GetDbgOamRxProc(V, valid_f, &oam_rx_proc)
           || GetDbgOamParser(V, oamPduInvalid_f, &oam_parser)
           || GetDbgOamHdrAdj(V, oamHdrAdjValid_f, &oam_hdr_adj)))
    {
        return CTC_E_NONE;
    }
    p_rslt->position = CTC_DIAG_TRACE_POS_OAM;
    DIAG_INFO_NONE(pos, "OAM RX Process")

    if (GetDbgOamHdrEdit(V, oamHdrEditBypassAll_f, &oam_hdr_edit)
        || GetDbgOamRxProc(V, bypassAll_f, &oam_rx_proc))
    {
        DIAG_INFO_CHAR(pos, "oam-edit", "bypass-all")
    }
    if (GetDbgOamHdrEdit(V, oamHdrEditDiscard_f, &oam_hdr_edit))
    {
        p_rslt->dest_type = CTC_DIAG_DEST_DROP;
        DIAG_INFO_CHAR(pos, "discard", "TURE")
    }

    if (GetDbgOamHdrEdit(V, oamHdrEditException_f, &oam_hdr_edit))
    {
        p_rslt->dest_type = CTC_DIAG_DEST_CPU;
        DIAG_INFO_UINT32(pos, "exception-id", GetDbgOamHdrEdit(V, oamHdrEditException_f, &oam_hdr_edit))
    }

    /*oam parser*/
    if (GetDbgOamParser(V, oamPduInvalid_f, &oam_parser) || GetDbgOamParser(V, bfdOamPduInvalid_f, &oam_parser))
    {
        DIAG_INFO_CHAR(pos, "oam-pdu-type", oam_pdu_type[GetDbgOamParser(V, oamType_f, &oam_parser)])

        if (GetDbgOamParser(V, bfdOamPduInvalid_f, &oam_parser))
        {
            DIAG_INFO_UINT32(pos, "dbit", GetDbgOamParser(V, u_bfd_dbit_f, &oam_parser))
            DIAG_INFO_UINT32(pos, "my-disc", GetDbgOamParser(V, u_bfd_myDisc_f, &oam_parser))
            DIAG_INFO_UINT32(pos, "your-disc", GetDbgOamParser(V, u_bfd_yourDisc_f, &oam_parser))
            DIAG_INFO_UINT32(pos, "desired-min-tx-interval", GetDbgOamParser(V, u_bfd_desiredMinTxInterval_f, &oam_parser))
            DIAG_INFO_UINT32(pos, "required-min-rx-interval", GetDbgOamParser(V, u_bfd_requiredMinRxInterval_f, &oam_parser))
        }
        else if(GetDbgOamParser(V, oamPduInvalid_f, &oam_parser))
        {
            DIAG_INFO_UINT32(pos, "md-level", GetDbgOamParser(V, u_ethoam_mdlevel_f, &oam_parser))
        }
    }

    /*oam rx proc*/
    if (GetDbgOamRxProc(V, valid_f, &oam_rx_proc))
    {
        DIAG_INFO_UINT32(pos, "mep-index", GetDbgOamRxProc(V, mepIndex_f, &oam_rx_proc))
        DIAG_INFO_UINT32(pos, "ma-index", GetDbgOamRxProc(V, maIndex_f, &oam_rx_proc))
        DIAG_INFO_UINT32(pos, "rmep-index", GetDbgOamRxProc(V, rmepIndex_f, &oam_rx_proc))

        DIAG_INFO_UINT32(pos, "Rx-exception", GetDbgOamRxProc(V, exception_f, &oam_rx_proc))
        if (GetDbgOamRxProc(V, linkOam_f, &oam_rx_proc))
        {
            DIAG_INFO_CHAR(pos, "LinkOAM", "YES")
        }
        if (GetDbgOamRxProc(V, lowCcm_f, &oam_rx_proc))
        {
            DIAG_INFO_CHAR(pos, "LowCCM", "YES")
        }
        if (GetDbgOamRxProc(V, lowSlowOam_f, &oam_rx_proc))
        {
            DIAG_INFO_CHAR(pos, "LowSlowCCM", "YES")
        }
        if (GetDbgOamRxProc(V, mipEn_f, &oam_rx_proc))
        {
            DIAG_INFO_CHAR(pos, "MipEn", "YES")
        }

        if (GetDbgOamRxProc(V, equalBfd_f, &oam_rx_proc))
        {
            DIAG_INFO_CHAR(pos, "BFD-Proc", "YES")
        }
        if (GetDbgOamRxProc(V, equalCcm_f, &oam_rx_proc))
        {
            DIAG_INFO_CHAR(pos, "CCM-Proc", "YES")
        }
        if (GetDbgOamRxProc(V, equalDm_f, &oam_rx_proc))
        {
            DIAG_INFO_CHAR(pos, "DM-Proc", "YES")
        }
        if (GetDbgOamRxProc(V, equalEthTst_f, &oam_rx_proc))
        {
            DIAG_INFO_CHAR(pos, "Tst-Proc", "YES")
        }
        if (GetDbgOamRxProc(V, equalFlexTest_f, &oam_rx_proc))
        {
            DIAG_INFO_CHAR(pos, "FlexTest-Proc", "YES")
        }
        if (GetDbgOamRxProc(V, equalLb_f, &oam_rx_proc))
        {
            DIAG_INFO_CHAR(pos, "LB-Proc", "YES")
        }
        if (GetDbgOamRxProc(V, equalLm_f, &oam_rx_proc))
        {
            DIAG_INFO_CHAR(pos, "LM-Proc", "YES")
        }
        if (GetDbgOamRxProc(V, equalSm_f, &oam_rx_proc))
        {
            DIAG_INFO_CHAR(pos, "SM-Proc", "YES")
        }
        if (GetDbgOamRxProc(V, equalTwamp_f, &oam_rx_proc))
        {
            DIAG_INFO_CHAR(pos, "TWAMP-Proc", "YES")
        }
        if (GetDbgOamRxProc(V, equalSBfdReflector_f, &oam_rx_proc))
        {
            DIAG_INFO_CHAR(pos, "SBFD-Ref-Proc", "YES")
        }
        if (GetDbgOamRxProc(V, isMdLvlMatch_f, &oam_rx_proc))
        {
            DIAG_INFO_CHAR(pos, "Md-Lvl-Match", "YES")
        }
        if (GetDbgOamRxProc(V, isUp_f, &oam_rx_proc))
        {
            DIAG_INFO_CHAR(pos, "is-up", "YES")
        }
    }

    if ( GetDbgOamDefectProc(V, oamDefectProcValid_f, &oam_defect_proc)
        && (GetDbgOamDefectProc(V, oamDefectProcDefectSubType_f, &oam_defect_proc)
            || GetDbgOamDefectProc(V, oamDefectProcDefectType_f, &oam_defect_proc)))
    {
        DIAG_INFO_UINT32(pos, "oam-defect-type", GetDbgOamDefectProc(V, oamDefectProcDefectType_f, &oam_defect_proc))
        DIAG_INFO_UINT32(pos, "oam-defect-sub-type", GetDbgOamDefectProc(V, oamDefectProcDefectSubType_f, &oam_defect_proc))
    }

    /*oam head edit*/
    if (GetDbgOamHdrEdit(V, oamHdrEditBypassAll_f, &oam_hdr_edit))
    {
        DIAG_INFO_CHAR(pos, "oam-header-edit-process", "bypass-all")
    }
    if (GetDbgOamHdrEdit(V, oamHdrEditIsDefectFree_f, &oam_hdr_edit))
    {
        DIAG_INFO_CHAR(pos, "oam-header-edit-process", "no-defect")
    }
    if (GetDbgOamHdrEdit(V, oamHdrEditDiscard_f, &oam_hdr_edit))
    {
        DIAG_INFO_CHAR(pos, "oam-header-edit-process", "discard")
    }
    if (GetDbgOamHdrEdit(V, oamHdrEditEqualDM_f, &oam_hdr_edit))
    {
        DIAG_INFO_CHAR(pos, "oam-header-edit-process", "DM-Proc")
    }
    if (GetDbgOamHdrEdit(V, oamHdrEditEqualLB_f, &oam_hdr_edit))
    {
        DIAG_INFO_CHAR(pos, "oam-header-edit-process", "LB-Proc")
    }
    if (GetDbgOamHdrEdit(V, oamHdrEditEqualLM_f, &oam_hdr_edit))
    {
        DIAG_INFO_CHAR(pos, "oam-header-edit-process", "LM-Proc")
    }
    if (GetDbgOamHdrEdit(V, oamHdrEditEqualSm_f, &oam_hdr_edit))
    {
        DIAG_INFO_CHAR(pos, "oam-header-edit-process", "SM-Proc")
    }
    if (GetDbgOamHdrEdit(V, oamHdrEditEqualTwamp_f, &oam_hdr_edit))
    {
        DIAG_INFO_CHAR(pos, "oam-header-edit-process", "TWAMP-Proc")
    }
    if (GetDbgOamHdrEdit(V, oamHdrEditTwampAck_f, &oam_hdr_edit))
    {
        DIAG_INFO_CHAR(pos, "oam-header-edit-process", "TWAMP-ACK-Proc")
    }
    if (GetDbgOamHdrEdit(V, oamHdrEditEqualSBfdReflector_f, &oam_hdr_edit))
    {
        DIAG_INFO_CHAR(pos, "oam-header-edit-process", "SBFD-Proc")
    }

    if (GetDbgOamApsSwitch(V, oamApsSwitchValid_f, &oam_aps_switch))
    {
        DIAG_INFO_UINT32(pos, "aps-group", GetDbgOamApsSwitch(V, oamApsSwitchApsGroupId_f, &oam_aps_switch))
        if (GetDbgOamApsSwitch(V, oamApsSwitchSignalOk_f, &oam_aps_switch))
        {
            DIAG_INFO_CHAR(pos, "aps-switch-signal", "OK")
        }
        if (GetDbgOamApsSwitch(V, oamApsSwitchSignalFail_f, &oam_aps_switch))
        {
            DIAG_INFO_CHAR(pos, "aps-switch-signal", "FAIL")
        }
        if (GetDbgOamApsSwitch(V, oamApsSwitchProtectionFail_f, &oam_aps_switch))
        {
            DIAG_INFO_CHAR(pos, "aps-switch-protection", "FAIL")
        }
        if (GetDbgOamApsSwitch(V, oamApsSwitchWorkingFail_f, &oam_aps_switch))
        {
            DIAG_INFO_CHAR(pos, "aps-switch-working", "FAIL")
        }
    }
    else if (!GetDbgOamApsSwitch(V, oamApsSwitchValid_f, &oam_aps_switch)
            && GetDbgOamApsProcess(V, oamApsProcessValid_f, &oam_aps_proc))
    {
        if (GetDbgOamApsProcess(V, oamApsProcessSignalOk_f, &oam_aps_proc))
        {
            DIAG_INFO_CHAR(pos, "aps-process-signal", "OK")
        }
        if (GetDbgOamApsProcess(V, oamApsProcessSignalFail_f, &oam_aps_proc))
        {
            DIAG_INFO_CHAR(pos, "aps-process-signal", "FAIL")
        }
        if (GetDbgOamApsProcess(V, oamApsProcessUpdateApsEn_f, &oam_aps_proc))
        {
            DIAG_INFO_CHAR(pos, "aps-process-update", "ENABLE")
        }
    }

    p_rslt->dest_type = CTC_DIAG_DEST_OAM;
    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_diag_pkt_trace_get_oam_tx_info(uint8 lchip, ctc_diag_pkt_trace_result_t* p_rslt)
{
    uint32 cmd = 0;
    uint8 pos = CTC_DIAG_TRACE_POS_OAM;
    char* rx_oam_type[15] = {"NONE", "ETHEROAM", "IPBFD", "PBTOAM", "PBBBSI",
                          "PBBBV", "MPLSOAM", "MPLSBFD", "ACHOAM", "TRILLBFD",
                          "TWAMP", "FLEX_PM_TEST"};
    char* packet_type[10] = {"ETHERNET", "IPV4", "MPLS", "IPV6", "MPLSUP",
                          "IP", "FLEXIBLE", "RESERVED"};
    DbgOamTxProc_m   oam_tx_proc;

    sal_memset(&oam_tx_proc, 0, sizeof(oam_tx_proc));
    cmd = DRV_IOR(DbgOamTxProc_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_tx_proc));

    if (!GetDbgOamTxProc(V, oamTxProcValid_f, &oam_tx_proc))
    {
        return CTC_E_NONE;
    }
    p_rslt->position = CTC_DIAG_TRACE_POS_OAM;
    DIAG_INFO_NONE(pos, "OAM TX Process")

    DIAG_INFO_CHAR(pos, "rx-oam-type", rx_oam_type[GetDbgOamTxProc(V, oamTxProcRxOamType_f, &oam_tx_proc)])
    DIAG_INFO_CHAR(pos, "packet-type", packet_type[GetDbgOamTxProc(V, oamTxProcPacketType_f, &oam_tx_proc)])
    DIAG_INFO_UINT32(pos, "nexthop-ptr", GetDbgOamTxProc(V, oamTxProcNextHopPtr_f, &oam_tx_proc))
    DIAG_INFO_U32_HEX(pos, "dest-map", GetDbgOamTxProc(V, oamTxProcDestmap_f, &oam_tx_proc))

    if (GetDbgOamTxProc(V, oamTxProcIsEth_f, &oam_tx_proc))
    {
        DIAG_INFO_CHAR(pos, "is-Eth", "YES")
    }
    if (GetDbgOamTxProc(V, oamTxProcIsTxCv_f, &oam_tx_proc))
    {
        DIAG_INFO_CHAR(pos, "is-TxCv", "YES")
    }
    if (GetDbgOamTxProc(V, oamTxProcIstxCsf_f, &oam_tx_proc))
    {
        DIAG_INFO_CHAR(pos, "is-TxCsf", "YES")
    }
    if (GetDbgOamTxProc(V, oamTxProcIsUp_f, &oam_tx_proc))
    {
        DIAG_INFO_CHAR(pos, "is-UpMep", "YES")
    }

    p_rslt->dest_type = CTC_DIAG_DEST_OAM;
    return CTC_E_NONE;
}


STATIC int32
_sys_tsingma_diag_pkt_trace_get_ipe_cid_pair_info(uint8 lchip, ctc_diag_pkt_trace_result_t* p_rslt)
{
    uint32 value = 0;
    uint32 cmd = 0;
    uint8 pos = CTC_DIAG_TRACE_POS_IPE;
    char*  str_hash_key_name[] = {"DsCategoryIdPairHashRightKey", "DsCategoryIdPairHashLeftKey"};
    char*  str_hash_ad_name[] = {"DsCategoryIdPairHashRightAd", "DsCategoryIdPairHashLeftAd"};
    DbgIpeAclProcInfo0_m   acl_info;
    DsCategoryIdPair_m     ds_cid_pair;

    sal_memset(&ds_cid_pair, 0, sizeof(DsCategoryIdPair_m));
    sal_memset(&acl_info, 0, sizeof(DbgIpeAclProcInfo0_m));

    cmd = DRV_IOR(IpeFwdCategoryCtl_t, IpeFwdCategoryCtl_categoryIdPairLookupEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    if (!value)
    {
        return CTC_E_NONE;
    }

    cmd = DRV_IOR(DbgIpeAclProcInfo0_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &acl_info));

    if(GetDbgIpeAclProcInfo0(V, gCidLkp_srcCategoryIdClassfied_f , &acl_info))
    {
        DIAG_INFO_UINT32(pos, "source-category-id", GetDbgIpeAclProcInfo0(V, gCidLkp_srcCategoryId_f, &acl_info))
    }

    if(GetDbgIpeAclProcInfo0(V, gCidLkp_destCategoryIdClassfied_f, &acl_info))
    {
        DIAG_INFO_UINT32(pos, "dest-category-id", GetDbgIpeAclProcInfo0(V, gCidLkp_destCategoryId_f, &acl_info))
    }

    if (GetDbgIpeAclProcInfo0(V, gCidLkp_cidPairHashHit_f, &acl_info))
    {
        value = GetDbgIpeAclProcInfo0(V, gCidLkp_cidPairHashIsLeft_f, &acl_info);
        DIAG_INFO_CHAR(pos, "cid-lookup-result", "HIT-HASH")

        DIAG_INFO_CHAR(pos, "key-name", str_hash_key_name[value])
        DIAG_INFO_UINT32(pos, "key-index", GetDbgIpeAclProcInfo0(V, gCidLkp_cidPairHashKeyIndex_f, &acl_info))
        DIAG_INFO_CHAR(pos, "ad-name", str_hash_ad_name[value])
        DIAG_INFO_UINT32(pos, "ad-index", GetDbgIpeAclProcInfo0(V, gCidLkp_cidPairHashAdIndex_f, &acl_info))
        DIAG_INFO_UINT32(pos, "array-offset", GetDbgIpeAclProcInfo0(V, gCidLkp_cidPairHashEntryOffset_f, &acl_info))
        if(value)
        {
            cmd = DRV_IOR(DsCategoryIdPairHashRightAd_t, DRV_ENTRY_FLAG);
        }
        else
        {
            cmd = DRV_IOR(DsCategoryIdPairHashLeftAd_t, DRV_ENTRY_FLAG);
        }
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, GetDbgIpeAclProcInfo0(V, gCidLkp_cidPairHashAdIndex_f, &acl_info), cmd, &ds_cid_pair));
    }
    else if (GetDbgIpeAclProcInfo0(V, gCidLkp_cidPairTcamResultValid_f , &acl_info))
    {
        value = GetDbgIpeAclProcInfo0(V, gCidLkp_cidPairTcamHitIndex_f, &acl_info);
        DIAG_INFO_CHAR(pos, "cid-lookup-result", "HIT-TCAM")
        DIAG_INFO_CHAR(pos, "key-name", "DsCategoryIdPairTcamKey")
        DIAG_INFO_UINT32(pos, "key-index", value)
        DIAG_INFO_CHAR(pos, "ad-name", "DsCategoryIdPairTcamAd")
        DIAG_INFO_UINT32(pos, "ad-index", value)
        cmd = DRV_IOR(DsCategoryIdPairTcamAd_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, value, cmd, &ds_cid_pair));
    }
    else
    {
        DIAG_INFO_CHAR(pos, "cid-lookup-result", DIAG_LOOKUP_HIT_NONE)
    }

    if(GetDbgIpeAclProcInfo0(V, gCidLkp_cidPairHashHit_f, &acl_info)
       || GetDbgIpeAclProcInfo0(V, gCidLkp_cidPairTcamResultValid_f , &acl_info))
    {
        if (GetDsCategoryIdPair(V, operationMode_f, &ds_cid_pair))
        {
            if (GetDsCategoryIdPair(V, u1_g2_permit_f, &ds_cid_pair))
            {
                DIAG_INFO_CHAR(pos, "cid-pair-action", "PERMIT")
            }
            else if (GetDsCategoryIdPair(V, u1_g2_deny_f, &ds_cid_pair))
            {
                DIAG_INFO_CHAR(pos, "cid-pair-action", "DENY")
            }
        }
        else
        {
            char str[64] = {0};
            sal_sprintf(str, "OverWrite-ACL-Level-%d", GetDsCategoryIdPair(V, aclLookupLevel_f, &ds_cid_pair));
            DIAG_INFO_CHAR(pos, "cid-pair-action", str)
            DIAG_INFO_UINT32(pos, "acl-lookup-type", GetDsCategoryIdPair(V, u1_g1_lookupType_f, &ds_cid_pair))

            if(!GetDsCategoryIdPair(V, u1_g1_keepAclLabel_f, &ds_cid_pair))
            {
                DIAG_INFO_UINT32(pos, "acl-label", GetDsCategoryIdPair(V, u1_g1_aclLabel_f, &ds_cid_pair))
            }
            if(!GetDsCategoryIdPair(V, u1_g1_keepAclUsePIVlan_f, &ds_cid_pair))
            {
                DIAG_INFO_UINT32(pos, "use-mapped-vlan", GetDsCategoryIdPair(V, u1_g1_aclUsePIVlan_f, &ds_cid_pair))
            }
            if (!GetDsCategoryIdPair(V, u1_g1_keepAclUseGlobalPortType_f, &ds_cid_pair))
            {
                DIAG_INFO_UINT32(pos, "use-mapped-gport", GetDsCategoryIdPair(V, u1_g1_aclUseGlobalPortType_f, &ds_cid_pair))
            }
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_diag_pkt_trace_map_tcam_key_index(uint8 lchip, uint32 dbg_index, uint8 is_acl, uint8 lkup_level, uint32* o_key_index)
{
    uint8  tcam_blk_id = 0;
    uint16 local_index = 0;
    uint32 key_index = 0xFFFFFFFF;
    uint32 entry_num = 0;
    uint8  hit_blk_max_default = 0;
    uint32 acl_tbl_id[] = {DsAclQosMacKey160Ing0_t, DsAclQosMacKey160Ing1_t, DsAclQosMacKey160Ing2_t, DsAclQosMacKey160Ing3_t,
                           DsAclQosMacKey160Ing4_t, DsAclQosMacKey160Ing5_t, DsAclQosMacKey160Ing6_t, DsAclQosMacKey160Ing7_t};
    uint32 scl_tbl_id[] = {DsScl0L3Key160_t, DsScl1L3Key160_t,DsScl2L3Key160_t, DsScl3L3Key160_t};

    tcam_blk_id = dbg_index >> 11;
    local_index = dbg_index & 0x7FF;

    /*TM:USERID 4,IPE_ACL 8,EPE_ACL 3*/
    hit_blk_max_default = 15;
    if(tcam_blk_id < hit_blk_max_default)
    {
        key_index = local_index;
    }
    else  /*used share tcam*/
    {
        drv_usw_ftm_get_entry_num(lchip, is_acl ?acl_tbl_id[lkup_level]:scl_tbl_id[lkup_level], &entry_num);
        key_index = 2*entry_num - 2048 + local_index; /*Uints_80bits_Per_Block = 2048*/
    }

    if(o_key_index)
    {
        *o_key_index = key_index;
    }

    return CTC_E_NONE;
}


STATIC int32
_sys_tsingma_diag_pkt_trace_get_ipe_acl_info(uint8 lchip, ctc_diag_pkt_trace_result_t* p_rslt)
{
    uint32 cmd = 0;
    uint8 idx = 0;
    uint8 pos = CTC_DIAG_TRACE_POS_IPE;
    IpePreLookupAclCtl_m ipe_pre_acl_ctl;
    char* acl_key_type_desc[] = {"TCAML2KEY", "TCAMl3KEY","TCAML3EXTKEY","TCAMIPV6KEY","TCAMIPV6EXTKEY","TCAML2L3KEY","TCAML2L3KEYEXT",
                                 "TCAML2IPV6KEY","TCAMCIDKEY", "TCAMINTFKEY", "TCAMFWDKEY", "TCAMFWDEXT","TCAMCOPPKEY","TCAMCOPPEXTKEY", "TCAMUDFKEY"};
    DbgFlowTcamEngineIpeAclInfo0_m tcam_info[8];
    DbgFlowTcamEngineIpeAclKeyInfo0_m acl_key_info;
    uint8  step = DbgFlowTcamEngineIpeAclKeyInfo1_key_f - DbgFlowTcamEngineIpeAclKeyInfo0_key_f;
    uint32 acl_key_info_key_id[8] = {DbgFlowTcamEngineIpeAclKeyInfo0_t, DbgFlowTcamEngineIpeAclKeyInfo1_t,
                                     DbgFlowTcamEngineIpeAclKeyInfo2_t, DbgFlowTcamEngineIpeAclKeyInfo3_t,
                                     DbgFlowTcamEngineIpeAclKeyInfo4_t, DbgFlowTcamEngineIpeAclKeyInfo5_t,
                                     DbgFlowTcamEngineIpeAclKeyInfo6_t, DbgFlowTcamEngineIpeAclKeyInfo7_t};

    uint8 step2 = IpePreLookupAclCtl_gGlbAcl_1_aclLookupType_f - IpePreLookupAclCtl_gGlbAcl_0_aclLookupType_f ;
    DbgIpeAclProcInfo0_m acl_proc_info;

    sal_memset(&ipe_pre_acl_ctl, 0, sizeof(ipe_pre_acl_ctl));
    /*DbgFlowTcamEngineIpeAclInfo0*/
    sal_memset(tcam_info, 0, 8 * sizeof(DbgFlowTcamEngineIpeAclInfo0_m));
    cmd = DRV_IOR(DbgFlowTcamEngineIpeAclInfo0_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tcam_info[0]));
    cmd = DRV_IOR(DbgFlowTcamEngineIpeAclInfo1_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tcam_info[1]));
    cmd = DRV_IOR(DbgFlowTcamEngineIpeAclInfo2_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tcam_info[2]));
    cmd = DRV_IOR(DbgFlowTcamEngineIpeAclInfo3_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tcam_info[3]));
    cmd = DRV_IOR(DbgFlowTcamEngineIpeAclInfo4_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tcam_info[4]));
    cmd = DRV_IOR(DbgFlowTcamEngineIpeAclInfo5_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tcam_info[5]));
    cmd = DRV_IOR(DbgFlowTcamEngineIpeAclInfo6_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tcam_info[6]));
    cmd = DRV_IOR(DbgFlowTcamEngineIpeAclInfo7_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tcam_info[7]));
    cmd = DRV_IOR(IpePreLookupAclCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_pre_acl_ctl));

    sal_memset(&acl_proc_info, 0, sizeof(acl_proc_info));
    cmd = DRV_IOR(DbgIpeAclProcInfo0_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &acl_proc_info));
    if (!GetDbgIpeAclProcInfo0(V, aclEnBitMap_f, &acl_proc_info))
    {
        return CTC_E_NONE;
    }
    p_rslt->position = CTC_DIAG_TRACE_POS_IPE;
    DIAG_INFO_NONE(pos, "ACL/TCAM Processing")

    /*tcam*/
    if (GetDbgFlowTcamEngineIpeAclInfo0(V, valid_f, &tcam_info[0])
        || GetDbgFlowTcamEngineIpeAclInfo1(V, valid_f, &tcam_info[1])
        || GetDbgFlowTcamEngineIpeAclInfo2(V, valid_f, &tcam_info[2])
        || GetDbgFlowTcamEngineIpeAclInfo3(V, valid_f, &tcam_info[3])
        || GetDbgFlowTcamEngineIpeAclInfo4(V, valid_f, &tcam_info[4])
        || GetDbgFlowTcamEngineIpeAclInfo5(V, valid_f, &tcam_info[5])
        || GetDbgFlowTcamEngineIpeAclInfo6(V, valid_f, &tcam_info[6])
        || GetDbgFlowTcamEngineIpeAclInfo7(V, valid_f, &tcam_info[7]))
    {
        uint32 acl_key_valid = 0;
        uint32 acl_key_size = 0;
        uint32 acl_key_type = 0;
        uint32 key_index = 0;
        char* lookup_result[8] = {"TCAM0-lookup-result", "TCAM1-lookup-result", "TCAM2-lookup-result", "TCAM3-lookup-result",
                                  "TCAM4-lookup-result", "TCAM5-lookup-result", "TCAM6-lookup-result", "TCAM7-lookup-result"};
        uint32 result_valid[8] = {  GetDbgFlowTcamEngineIpeAclInfo0(V, ingrTcamResultValid0_f, &tcam_info[0]),
                                    GetDbgFlowTcamEngineIpeAclInfo1(V, ingrTcamResultValid1_f, &tcam_info[1]),
                                    GetDbgFlowTcamEngineIpeAclInfo2(V, ingrTcamResultValid2_f, &tcam_info[2]),
                                    GetDbgFlowTcamEngineIpeAclInfo3(V, ingrTcamResultValid3_f, &tcam_info[3]),
                                    GetDbgFlowTcamEngineIpeAclInfo4(V, ingrTcamResultValid4_f, &tcam_info[4]),
                                    GetDbgFlowTcamEngineIpeAclInfo5(V, ingrTcamResultValid5_f, &tcam_info[5]),
                                    GetDbgFlowTcamEngineIpeAclInfo6(V, ingrTcamResultValid6_f, &tcam_info[6]),
                                    GetDbgFlowTcamEngineIpeAclInfo7(V, ingrTcamResultValid7_f, &tcam_info[7])};

        uint32 tcam_index[8] = {    GetDbgFlowTcamEngineIpeAclInfo0(V, ingrTcamIndex0_f, &tcam_info[0]),
                                    GetDbgFlowTcamEngineIpeAclInfo1(V, ingrTcamIndex1_f, &tcam_info[1]),
                                    GetDbgFlowTcamEngineIpeAclInfo2(V, ingrTcamIndex2_f, &tcam_info[2]),
                                    GetDbgFlowTcamEngineIpeAclInfo3(V, ingrTcamIndex3_f, &tcam_info[3]),
                                    GetDbgFlowTcamEngineIpeAclInfo4(V, ingrTcamIndex4_f, &tcam_info[4]),
                                    GetDbgFlowTcamEngineIpeAclInfo5(V, ingrTcamIndex5_f, &tcam_info[5]),
                                    GetDbgFlowTcamEngineIpeAclInfo6(V, ingrTcamIndex6_f, &tcam_info[6]),
                                    GetDbgFlowTcamEngineIpeAclInfo7(V, ingrTcamIndex7_f, &tcam_info[7])};

        for (idx = 0; idx < 8 ; idx++)
        {
            if (!((GetDbgIpeAclProcInfo0(V, aclEnBitMap_f, &acl_proc_info) >> idx) & 0x1))
            {
                continue;
            }
            cmd = DRV_IOR(acl_key_info_key_id[idx], DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &acl_key_info));
            DRV_GET_FIELD_A(lchip, acl_key_info_key_id[idx], DbgFlowTcamEngineIpeAclKeyInfo0_key_f + step * idx,
                            &acl_key_info, &acl_key_info);
            DRV_GET_FIELD_A(lchip, acl_key_info_key_id[idx], DbgFlowTcamEngineIpeAclKeyInfo0_valid_f + step * idx,
                            &acl_key_info, &acl_key_valid);
            DRV_GET_FIELD_A(lchip, acl_key_info_key_id[idx], DbgFlowTcamEngineIpeAclKeyInfo0_keySize_f + step * idx,
                            &acl_key_info, &acl_key_size);

            if (result_valid[idx])
            {
                cmd = DRV_IOR(acl_key_info_key_id[idx], DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &acl_key_info));
                DRV_GET_FIELD_A(lchip, acl_key_info_key_id[idx], DbgFlowTcamEngineIpeAclKeyInfo0_key_f + step * idx,
                &acl_key_info, &acl_key_info);

                _sys_tsingma_diag_pkt_trace_map_tcam_key_index(lchip, tcam_index[idx], 1, idx, &key_index);
                DIAG_INFO_CHAR(pos, lookup_result[idx], DIAG_LOOKUP_HIT)
                DIAG_INFO_UINT32(pos, "key-index", key_index/(1<<acl_key_size))
                DIAG_INFO_UINT32(pos, "key-size", 80*(1<<acl_key_size))
            }
            else
            {
                DIAG_INFO_CHAR(pos, lookup_result[idx], DIAG_LOOKUP_HIT_NONE)
            }
            /*get key type*/
            if(!acl_key_valid)
            {
                continue;
            }
            if (11 == GetIpePreLookupAclCtl(V, gGlbAcl_0_aclLookupType_f + step2 * idx, &ipe_pre_acl_ctl))   /*copp*/
            {
                acl_key_type = GetDsAclQosCoppKey320Ing0(V, keyType_f, &acl_key_info);
                acl_key_type = (0 == acl_key_type) ? 12 : 13;
            }
            else
            {
                acl_key_type = GetDsAclQosL3Key320Ing0(V, aclQosKeyType0_f, &acl_key_info);
            }
            if (acl_key_type<=14)
            {
                DIAG_INFO_CHAR(pos, "key-type", acl_key_type_desc[acl_key_type])
            }
        }
    }

    CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_get_ipe_cid_pair_info(lchip, p_rslt));
    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_diag_pkt_trace_get_ipfix_info(uint8 lchip, ctc_diag_pkt_trace_result_t* p_rslt, uint8 dir)
{
    uint32 cmd = 0;
    uint32 value = 0;
    uint8 pos = CTC_DIAG_TRACE_POS_IPE;
    DbgIpfixAccInfo0_m  ipfixAccInfo;

    if (dir == 0)
    {
        cmd = DRV_IOR(DbgIpfixAccInfo0_t, DRV_ENTRY_FLAG);
    }
    else
    {
        cmd = DRV_IOR(DbgIpfixAccInfo1_t, DRV_ENTRY_FLAG);
    }
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipfixAccInfo));
    if (!GetDbgIpfixAccInfo0(V, valid_f, &ipfixAccInfo))
    {
        return CTC_E_NONE;
    }
    if(dir == 0)
    {
        DIAG_INFO_NONE(pos, "Ingress Ipfix Process")

    }
    else
    {
        DIAG_INFO_NONE(pos, "Egress Ipfix Process")
    }

    DIAG_INFO_CHAR(pos, "ipfix-enable", GetDbgIpfixAccInfo0(V, ipfixEn_f, &ipfixAccInfo) ? "ENABLE" : "DISABLE")
    DIAG_INFO_UINT32(pos, "ipfix-configure-profile-id", GetDbgIpfixAccInfo0(V, ipfixCfgProfileId_f, &ipfixAccInfo))
    DIAG_INFO_UINT32(pos, "hash-key-type", GetDbgIpfixAccInfo0(V, hashKeyType_f, &ipfixAccInfo))
    DIAG_INFO_UINT32(pos, "hash-field-sele ct-id", GetDbgIpfixAccInfo0(V, flowFieldSel_f, &ipfixAccInfo))

    if(GetDbgIpfixAccInfo0(V, resultValid_f, &ipfixAccInfo))
    {
        value = GetDbgIpfixAccInfo0(V, hashConflict_f, &ipfixAccInfo);
        DIAG_INFO_CHAR(pos, "hash-lookup-result", value ? DIAG_LOOKUP_CONFLICT : DIAG_LOOKUP_HIT)
        DIAG_INFO_UINT32(pos, "key-index", GetDbgIpfixAccInfo0(V, keyIndex_f, &ipfixAccInfo))
    }

    if(GetDbgIpfixAccInfo0(V, samplingEn_f, &ipfixAccInfo))
    {
        value = GetDbgIpfixAccInfo0(V, isSamplingPkt_f, &ipfixAccInfo);
        DIAG_INFO_CHAR(pos, "sample-packet", value ? "TRUE" : "FALSE")
    }
    value = GetDbgIpfixAccInfo0(V, isAddOperation_f, &ipfixAccInfo);
    DIAG_INFO_CHAR(pos, "opration-add", value ? "TRUE" : "FALSE")
    value = GetDbgIpfixAccInfo0(V, isUpdateOperation_f, &ipfixAccInfo);
    DIAG_INFO_CHAR(pos, "opration-update", value ? "TRUE" : "FALSE")
    value = GetDbgIpfixAccInfo0(V, byPassIpfixProcess_f, &ipfixAccInfo);
    DIAG_INFO_CHAR(pos, "opration-bypass", value ? "TRUE" : "FALSE")
    value = GetDbgIpfixAccInfo0(V, denyIpfixInsertOperation_f, &ipfixAccInfo);
    DIAG_INFO_CHAR(pos, "opration-deny-learning", value ? "TRUE" : "FALSE")

    if(GetDbgIpfixAccInfo0(V, mfpEn_f, &ipfixAccInfo))
    {
        DIAG_INFO_UINT32(pos, "mfp-policer-id", GetDbgIpfixAccInfo0(V, microflowPolicingProfId_f, &ipfixAccInfo))
    }
    DIAG_INFO_UINT32(pos, "destination-info", GetDbgIpfixAccInfo0(V, destinationInfo_f, &ipfixAccInfo))
    DIAG_INFO_UINT32(pos, "destination-type", GetDbgIpfixAccInfo0(V, destinationType_f, &ipfixAccInfo))

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_diag_pkt_trace_get_ipe_fcoe_info(uint8 lchip, ctc_diag_pkt_trace_result_t* p_rslt)
{
    uint32 cmd = 0;
    uint8 pos = CTC_DIAG_TRACE_POS_IPE;
    DbgIpeFcoeInfo_m fcoe_info;

    sal_memset(&fcoe_info, 0, sizeof(fcoe_info));
    cmd = DRV_IOR(DbgIpeFcoeInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fcoe_info));
    if (!GetDbgIpeFcoeInfo(V, valid_f, &fcoe_info))
    {
        return CTC_E_NONE;
    }
    p_rslt->position = CTC_DIAG_TRACE_POS_IPE;
    DIAG_INFO_NONE(pos, "FCoE")
    if (GetDbgIpeFcoeInfo(V, fcoeDaResultValid_f, &fcoe_info))
    {
        DIAG_INFO_CHAR(pos, "fcoe-da-result", "HIT")
    }
    if (GetDbgIpeFcoeInfo(V, fcoeSaResultValid_f, &fcoe_info))
    {
        DIAG_INFO_CHAR(pos, "fcoe-sa-result", "HIT")
    }


    /* Exception Process */
    _sys_tsingma_diag_pkt_tracce_exception_map(p_rslt,
                                               GetDbgIpeFcoeInfo(V, exceptionEn_f, &fcoe_info),
                                               GetDbgIpeFcoeInfo(V, exceptionIndex_f, &fcoe_info),
                                               GetDbgIpeFcoeInfo(V, exceptionSubIndex_f, &fcoe_info));
    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_diag_pkt_trace_get_ipe_trill_info(uint8 lchip, ctc_diag_pkt_trace_result_t* p_rslt)
{
    uint32 cmd = 0;
    uint8 pos = CTC_DIAG_TRACE_POS_IPE;
    DbgIpeTrillInfo_m trill_info;

    sal_memset(&trill_info, 0, sizeof(trill_info));
    cmd = DRV_IOR(DbgIpeTrillInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &trill_info));
    if (!GetDbgIpeTrillInfo(V, valid_f, &trill_info))
    {
        return CTC_E_NONE;
    }
    p_rslt->position = CTC_DIAG_TRACE_POS_IPE;
    DIAG_INFO_NONE(pos, "TRILL")
    if (GetDbgIpeTrillInfo(V, trillResultValid_f, &trill_info))
    {
        DIAG_INFO_CHAR(pos, "trill-result", "HIT")
    }

    /* Drop Process */
    _sys_tsingma_diag_pkt_tracce_drop_map(p_rslt,
                                          GetDbgIpeTrillInfo(V, discard_f, &trill_info),
                                          GetDbgIpeTrillInfo(V, discardType_f, &trill_info));
    /* Exception Process */
    _sys_tsingma_diag_pkt_tracce_exception_map(p_rslt,
                                               GetDbgIpeTrillInfo(V, exceptionEn_f, &trill_info),
                                               GetDbgIpeTrillInfo(V, exceptionIndex_f, &trill_info),
                                               GetDbgIpeTrillInfo(V, exceptionSubIndex_f, &trill_info));
    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_diag_pkt_trace_get_ipe_stmctl_info(uint8 lchip, ctc_diag_pkt_trace_result_t* p_rslt)
{
    uint32 cmd = 0;
    uint8 pos = CTC_DIAG_TRACE_POS_IPE;
    char*  str_color[] = {"None", "Red", "Yellow", "Green"};
    char*  str_stmctl_type[] = {"Unknow-Ucast", "Broadcast", "Unknow-Mcast", " ",
                                    "Know-Ucast", "Broadcast", "Know-Mcast", " "};
    DbgIpeFwdStormCtlInfo_m storm_ctl_info;

    sal_memset(&storm_ctl_info, 0, sizeof(DbgIpeFwdStormCtlInfo_m));
    cmd = DRV_IOR(DbgIpeFwdStormCtlInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &storm_ctl_info));

        /*stormctl*/
    if(!(GetDbgIpeFwdStormCtlInfo(V, stormCtlEn_f, &storm_ctl_info)
       && GetDbgIpeFwdStormCtlInfo(V, valid_f, &storm_ctl_info)))
    {
        return CTC_E_NONE;
    }
    p_rslt->position = CTC_DIAG_TRACE_POS_IPE;
    DIAG_INFO_NONE(pos, "Storm Control")
    DIAG_INFO_CHAR(pos, "stormctl-color", str_color[GetDbgIpeFwdStormCtlInfo(V, color_f, &storm_ctl_info)])
    DIAG_INFO_CHAR(pos, "stormctl-type", str_stmctl_type[GetDbgIpeFwdStormCtlInfo(V, offsetSel_f, &storm_ctl_info)])
    DIAG_INFO_UINT32(pos, "stormctl-length", GetDbgIpeFwdStormCtlInfo(V, stormCtlLen_f, &storm_ctl_info))

    if (GetDbgIpeFwdStormCtlInfo(V, stormCtlDrop_f, &storm_ctl_info))
    {
        DIAG_INFO_CHAR(pos, "stormctl-result", "DROP")
    }
    if (GetDbgIpeFwdStormCtlInfo(V, stormCtlExceptionEn_f, &storm_ctl_info))
    {
        DIAG_INFO_CHAR(pos, "stormctl-result", "EXCEPTION")
    }

    if (GetDbgIpeFwdStormCtlInfo(V, stormCtlPtrValid0_f, &storm_ctl_info))
    {
        DIAG_INFO_UINT32(pos, "stormctl-ptr-0", GetDbgIpeFwdStormCtlInfo(V, stormCtlPtr0_f, &storm_ctl_info))
    }
    if (GetDbgIpeFwdStormCtlInfo(V, stormCtlPtrValid1_f, &storm_ctl_info))
    {
        DIAG_INFO_UINT32(pos, "stormctl-ptr-1", GetDbgIpeFwdStormCtlInfo(V, stormCtlPtr1_f, &storm_ctl_info))
    }

    return CTC_E_NONE;
}
STATIC int32
_sys_tsingma_diag_pkt_trace_get_ipe_policer_info(uint8 lchip, ctc_diag_pkt_trace_result_t* p_rslt)
{
    uint32 cmd = 0;
    uint32 value = 0;
    uint8 port_policerEn[2] = {0};
    uint8 vlan_policerEn[2] = {0};
    uint8 policerValid[2] = {0};
    uint8 pos = CTC_DIAG_TRACE_POS_IPE;
    char*  str_color[] = {"None", "Red", "Yellow", "Green"};
    DbgIpeFwdPolicingInfo_m policer_info;

    sal_memset(&policer_info, 0, sizeof(DbgIpeFwdPolicingInfo_m));
    cmd = DRV_IOR(DbgIpeFwdPolicingInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &policer_info));

    /*policer*/
    if(!GetDbgIpeFwdPolicingInfo(V, valid_f, &policer_info))
    {
        return CTC_E_NONE;
    }
    port_policerEn[0] = GetDbgIpeFwdPolicingInfo(V, portPolicer0En_f, &policer_info);
    port_policerEn[1] = GetDbgIpeFwdPolicingInfo(V, portPolicer1En_f, &policer_info);
    vlan_policerEn[0] = GetDbgIpeFwdPolicingInfo(V, vlanPolicer0En_f, &policer_info);
    vlan_policerEn[1] = GetDbgIpeFwdPolicingInfo(V, vlanPolicer1En_f, &policer_info);
    policerValid[0] = GetDbgIpeFwdPolicingInfo(V, policerValid0_f, &policer_info);
    policerValid[1] = GetDbgIpeFwdPolicingInfo(V, policerValid1_f, &policer_info);

    if (!(policerValid[0] || policerValid[1]))
    {
        return CTC_E_NONE;
    }
    p_rslt->position = CTC_DIAG_TRACE_POS_IPE;
    DIAG_INFO_NONE(pos, "Policing")
    DIAG_INFO_CHAR(pos, "policer-color", str_color[GetDbgIpeFwdPolicingInfo(V, color_f, &policer_info)])
    DIAG_INFO_UINT32(pos, "policer-length", GetDbgIpeFwdPolicingInfo(V, policerLength_f, &policer_info))
    value = GetDbgIpeFwdPolicingInfo(V, policyProfId_f, &policer_info);
    if (value)
    {
        DIAG_INFO_UINT32(pos, "policer-profile-id", value)
    }

    if (policerValid[0])
    {
        value = GetDbgIpeFwdPolicingInfo(V, policer0SplitModeValid_f, &policer_info);
        DIAG_INFO_CHAR(pos, "policer-0-split", value ? "ENABLE" : "DISABLE")
        value = GetDbgIpeFwdPolicingInfo(V, policerPhbEn0_f, &policer_info);
        DIAG_INFO_CHAR(pos, "policer-0-phb", value ? "ENABLE" : "DISABLE")
        if (port_policerEn[0])
        {
            DIAG_INFO_CHAR(pos, "policer-0-port", "ENABLE")
        }
        if (vlan_policerEn[0])
        {
            DIAG_INFO_CHAR(pos, "policer-0-vlan",  "ENABLE")
        }
        DIAG_INFO_UINT32(pos, "policer-0-ptr", GetDbgIpeFwdPolicingInfo(V, policerPtr0_f, &policer_info))
        DIAG_INFO_UINT32(pos, "policer-0-stats-ptr", GetDbgIpeFwdPolicingInfo(V, policerStatsPtr0_f, &policer_info))
    }
    if (policerValid[1])
    {
        value = GetDbgIpeFwdPolicingInfo(V, policer1SplitModeValid_f, &policer_info);
        DIAG_INFO_CHAR(pos, "policer-1-split", value ? "ENABLE" : "DISABLE")
        value = GetDbgIpeFwdPolicingInfo(V, policerPhbEn1_f, &policer_info);
        DIAG_INFO_CHAR(pos, "policer-1-phb", value ? "ENABLE" : "DISABLE")
        if (port_policerEn[1])
        {
            DIAG_INFO_CHAR(pos, "policer-1-port", "ENABLE")
        }
        if (vlan_policerEn[1])
        {
            DIAG_INFO_CHAR(pos, "policer-1-vlan", "ENABLE")
        }
        DIAG_INFO_UINT32(pos, "policer-1-ptr", GetDbgIpeFwdPolicingInfo(V, policerPtr1_f, &policer_info))
        DIAG_INFO_UINT32(pos, "policer-1-stats-ptr", GetDbgIpeFwdPolicingInfo(V, policerStatsPtr1_f, &policer_info))
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_diag_pkt_trace_get_ipe_copp_info(uint8 lchip, ctc_diag_pkt_trace_result_t* p_rslt)
{
    uint32 cmd = 0;
    uint8 pos = CTC_DIAG_TRACE_POS_IPE;
    char*  str_color[] = {"None", "Red", "Yellow", "Green"};
    DbgIpeFwdCoppInfo_m copp_info;

    sal_memset(&copp_info, 0, sizeof(DbgIpeFwdCoppInfo_m));
    cmd = DRV_IOR(DbgIpeFwdCoppInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &copp_info));

    /*copp*/
    if (!(GetDbgIpeFwdCoppInfo(V, valid_f,&copp_info)
        && GetDbgIpeFwdCoppInfo(V, coppValid_f, &copp_info)))
    {
        return CTC_E_NONE;
    }
    p_rslt->position = CTC_DIAG_TRACE_POS_IPE;
    DIAG_INFO_NONE(pos, "CoPP")
    DIAG_INFO_CHAR(pos, "copp-color", str_color[GetDbgIpeFwdCoppInfo(V, color_f, &copp_info)])
    DIAG_INFO_UINT32(pos, "copp-length", GetDbgIpeFwdCoppInfo(V, coppLen_f, &copp_info))
    DIAG_INFO_UINT32(pos, "copp-ptr", GetDbgIpeFwdCoppInfo(V, coppPtr_f, &copp_info))
    if(GetDbgIpeFwdCoppInfo(V, coppDrop_f, &copp_info))
    {
        DIAG_INFO_CHAR(pos, "copp-result", "DROP")
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_diag_pkt_trace_get_ipe_dest_info(uint8 lchip, ctc_diag_pkt_trace_result_t* p_rslt)
 {
    uint32 cmd = 0;
    uint32 value = 0;
    uint8 pos = CTC_DIAG_TRACE_POS_IPE;
    uint32 rx_oam_type = 0;
    uint32 hdr_data[12] = {0};
    uint32 hdr_data_temp[12] = {0};
    DbgIpeEcmpProcInfo_m ecmp_info;
    DbgIpeFwdProcessInfo_m fwd_info;
    DbgIpeAclProcInfo0_m   acl_info;
    DbgIpeFwdInputHeaderInfo_m ipe_hdr;

    sal_memset(&ipe_hdr, 0, sizeof(ipe_hdr));
    cmd = DRV_IOR(DbgIpeFwdInputHeaderInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_hdr));
    sal_memset(&ecmp_info, 0, sizeof(ecmp_info));
    cmd = DRV_IOR(DbgIpeEcmpProcInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ecmp_info));
    sal_memset(&fwd_info, 0, sizeof(fwd_info));
    cmd = DRV_IOR(DbgIpeFwdProcessInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fwd_info));
    sal_memset(&acl_info, 0, sizeof(acl_info));
    cmd = DRV_IOR(DbgIpeAclProcInfo0_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &acl_info));
    cmd = DRV_IOR(DbgIpeOamInfo_t, DbgIpeOamInfo_tempRxOamType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rx_oam_type));

    if (!GetDbgIpeFwdProcessInfo(V, valid_f,  &fwd_info))
    {
        return CTC_E_NONE;
    }
    p_rslt->position = CTC_DIAG_TRACE_POS_IPE;
    DIAG_INFO_NONE(pos, "Destination Processing")

    GetDbgIpeFwdInputHeaderInfo(A, data_f, &ipe_hdr, hdr_data);
    if (!sal_memcmp(hdr_data, hdr_data_temp, 12 * sizeof(uint32)))
    {
        DIAG_INFO_PKT_HDR(pos, "packet-header", hdr_data)
    }

    value = GetDbgIpeEcmpProcInfo(V, ecmpGroupId_f, &ecmp_info);
    if (GetDbgIpeEcmpProcInfo(V, ecmpEn_f, &ecmp_info)
        && GetDbgIpeEcmpProcInfo(V, valid_f, &ecmp_info)
        && value)
    {
        DIAG_INFO_CHAR(pos, "dest-type", "ECMP")
        DIAG_INFO_UINT32(pos, "ecmp-group", value)
        p_rslt->ecmp_group = value;
        p_rslt->flags |= CTC_DIAG_TRACE_RSTL_FLAG_ECMP_GROUP;
    }

    if (GetDbgIpeAclProcInfo0(V, apsBridgeValid0_f, &acl_info))
    {
        DIAG_INFO_CHAR(pos, "dest-type", "APS")
        DIAG_INFO_UINT32(pos, "aps-group-1st-level", GetDbgIpeAclProcInfo0(V, apsBridgeIndex0_f, &acl_info))
        if (GetDbgIpeAclProcInfo0(V, apsBridgeValid1_f,  &acl_info))
        {
            DIAG_INFO_UINT32(pos, "aps-group-2nd-level", GetDbgIpeAclProcInfo0(V, apsBridgeIndex1_f, &acl_info))
        }
    }
    else
    {
        if (GetDbgIpeAclProcInfo0(V, apsSelectValid0_f,  &acl_info))
        {
            DIAG_INFO_UINT32(pos, "aps-group-select-0", GetDbgIpeAclProcInfo0(V, apsSelectGroupId0_f, &acl_info))
        }
        if (GetDbgIpeAclProcInfo0(V, apsSelectValid1_f,  &acl_info))
        {
            DIAG_INFO_UINT32(pos, "aps-group-select-1", GetDbgIpeAclProcInfo0(V, apsSelectGroupId1_f, &acl_info))
        }
    }

    if (rx_oam_type)
    {
        DIAG_INFO_CHAR(pos, "dest-type", "OAM")
        p_rslt->dest_type = CTC_DIAG_DEST_OAM;
        CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_get_oam_rx_info(lchip, p_rslt));
    }
    else
    {
        uint32 fwd_destMap = GetDbgIpeFwdProcessInfo(V, destMap_f,  &fwd_info);
        if (0x3FFFF == GetDbgIpeFwdProcessInfo(V, nextHopPtr_f,  &fwd_info))
        {
            DIAG_INFO_CHAR(pos, "dest-type", "DROP")
            p_rslt->dest_type = CTC_DIAG_DEST_DROP;
        }
        else if (IS_BIT_SET(fwd_destMap, 18))/* Mcast */
        {
            DIAG_INFO_CHAR(pos, "dest-type", "Mcast Group")
            DIAG_INFO_UINT32(pos, "mcast-group", fwd_destMap&0xFFFF)
            p_rslt->mc_group = fwd_destMap&0xFFFF;
            p_rslt->flags |= CTC_DIAG_TRACE_RSTL_FLAG_MC_GROUP;
        }
        else
        {
            uint32 dest_chip = (fwd_destMap >> 9)&0x7F;
            if (IS_BIT_SET(fwd_destMap, 16))/* fwd to cpu */
            {
                DIAG_INFO_CHAR(pos, "dest-type", "CPU")
                p_rslt->port =  CTC_GPORT_RCPU(dest_chip);
                p_rslt->flags |= CTC_DIAG_TRACE_RSTL_FLAG_PORT;
                p_rslt->dest_type = CTC_DIAG_DEST_CPU;
            }
            else
            {
                if (dest_chip == 0x1F) /* Linkagg */
                {
                    p_rslt->dest_type = CTC_DIAG_DEST_NETWORK;
                    p_rslt->tid =  fwd_destMap&CTC_LINKAGGID_MASK;
                    p_rslt->flags |= CTC_DIAG_TRACE_RSTL_FLAG_TID;
                    DIAG_INFO_CHAR(pos, "dest-type", "Linkagg Port")
                    DIAG_INFO_UINT32(pos, "linkagg-group", p_rslt->tid)
                }
                else
                {
                    value = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(dest_chip, fwd_destMap&0x1FF);
                    p_rslt->port =  value;
                    p_rslt->flags |= CTC_DIAG_TRACE_RSTL_FLAG_PORT;
                    p_rslt->dest_type = CTC_DIAG_DEST_NETWORK;
                    DIAG_INFO_CHAR(pos, "dest-type", "Network Port")
                    DIAG_INFO_U32_HEX(pos, "network-port", value)
                }
/*
                cmd = DRV_IOR(EpeHdrAdjustCtl_t, EpeHdrAdjustCtl_dsNextHopInternalBase_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
*/
                DIAG_INFO_CHAR(pos, "nexthop-lookup-result", DIAG_LOOKUP_HIT)
                DIAG_INFO_U32_HEX(pos,"nexthop-ptr", GetDbgIpeFwdProcessInfo(V, nextHopPtr_f,  &fwd_info))
                /*
                if (((GetDbgIpeFwdProcessInfo(V, nextHopPtr_f,  &fwd_info) >> 4)&0x3FFF) != value)
                {
                    DIAG_INFO_CHAR(pos, "table-name", GetDbgIpeFwdProcessInfo(V, nextHopExt_f,  &fwd_info) ? "DsNextHop8W" : "DsNextHop4W")

                }
                else
                {
                    DIAG_INFO_CHAR(pos, "table-name", "EpeNextHopInternal")
                    DIAG_INFO_UINT32(pos,"table-index", (GetDbgIpeFwdProcessInfo(V, nextHopPtr_f,  &fwd_info)&0xF) >> 1)
                }
                */
            }
        }
    }

    CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_get_ipfix_info(lchip, p_rslt, 0));

    /* Drop Process */
    _sys_tsingma_diag_pkt_tracce_drop_map(p_rslt,
                                          GetDbgIpeFwdProcessInfo(V, discard_f, &fwd_info),
                                          GetDbgIpeFwdProcessInfo(V, discardType_f,  &fwd_info));
    /* Exception Process */
    _sys_tsingma_diag_pkt_tracce_exception_map(p_rslt,
                                               GetDbgIpeFwdProcessInfo(V, exceptionEn_f, &fwd_info),
                                               GetDbgIpeFwdProcessInfo(V, exceptionIndex_f, &fwd_info),
                                               GetDbgIpeFwdProcessInfo(V, exceptionSubIndex_f, &fwd_info));

     return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_diag_pkt_trace_get_epe_scl_info(uint8 lchip, ctc_diag_pkt_trace_result_t* p_rslt)
{
    uint32 cmd = 0;
    uint8 pos = CTC_DIAG_TRACE_POS_EPE;
    uint32 value = 0;
    uint32 key_index = 0;
    char* key_name[11] ={
        "None", "DsEgressXcOamDoubleVlanPortHashKey", "DsEgressXcOamSvlanPortHashKey", "DsEgressXcOamCvlanPortHashKey",
        "DsEgressXcOamSvlanCosPortHashKey", "DsEgressXcOamCvlanCosPortHashKey", "DsEgressXcOamPortVlanCrossHashKey",
        "DsEgressXcOamPortCrossHashKey", "DsEgressXcOamPortHashKey", "DsEgressXcOamSvlanPortMacHashKey",
        "DsEgressXcOamTunnelPbbHashKey"
    };
    DbgEgrXcOamHash0EngineFromEpeNhpInfo_m oam_hash0_info;
    DbgEgrXcOamHash1EngineFromEpeNhpInfo_m oam_hash1_info;
    DbgEpeNextHopInfo_m next_hop_info;

    sal_memset(&oam_hash0_info, 0, sizeof(oam_hash0_info));
    cmd = DRV_IOR(DbgEgrXcOamHash0EngineFromEpeNhpInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_hash0_info));
    sal_memset(&oam_hash1_info, 0, sizeof(oam_hash1_info));
    cmd = DRV_IOR(DbgEgrXcOamHash1EngineFromEpeNhpInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_hash1_info));
    sal_memset(&next_hop_info, 0, sizeof(next_hop_info));
    cmd = DRV_IOR(DbgEpeNextHopInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &next_hop_info));

    if (!GetDbgEpeNextHopInfo(V, valid_f, &next_hop_info))
    {
        return CTC_E_NONE;
    }
    if (!(GetDbgEgrXcOamHash0EngineFromEpeNhpInfo(V, valid_f, &oam_hash0_info)
          || GetDbgEgrXcOamHash1EngineFromEpeNhpInfo(V, valid_f, &oam_hash1_info)))
    {
        return CTC_E_NONE;
    }
    p_rslt->position = CTC_DIAG_TRACE_POS_EPE;
    DIAG_INFO_NONE(pos, "SCL Process")
    if (GetDbgEpeNextHopInfo(V, lookup0Valid_f, &next_hop_info))
    {
        value = GetDbgEpeNextHopInfo(V, vlanHash0Type_f, &next_hop_info);
        key_index = GetDbgEgrXcOamHash0EngineFromEpeNhpInfo(V, keyIndex_f, &oam_hash0_info);
        if (GetDbgEgrXcOamHash0EngineFromEpeNhpInfo(V, defaultEntryValid_f, &oam_hash0_info))
        {
            DIAG_INFO_CHAR(pos, "hash-0-lookup-result", DIAG_LOOKUP_DEFAULT)
            DIAG_INFO_CHAR(pos, "key-name", key_name[value])
            DIAG_INFO_UINT32(pos, "key-index", key_index)
        }
        else if (GetDbgEgrXcOamHash0EngineFromEpeNhpInfo(V, resultValid_f, &oam_hash0_info))
        {
            DIAG_INFO_CHAR(pos, "hash-0-lookup-result", DIAG_LOOKUP_HIT)
            DIAG_INFO_CHAR(pos, "key-name", key_name[value])
            DIAG_INFO_UINT32(pos, "key-index", key_index)
        }
        else
        {
            DIAG_INFO_CHAR(pos, "hash-0-lookup-result", DIAG_LOOKUP_HIT_NONE)
        }
    }
    if (GetDbgEpeNextHopInfo(V, lookup1Valid_f, &next_hop_info))
    {
        value = GetDbgEpeNextHopInfo(V, vlanHash1Type_f, &next_hop_info);
        key_index = GetDbgEgrXcOamHash1EngineFromEpeNhpInfo(V, keyIndex_f, &oam_hash1_info);
        if (GetDbgEgrXcOamHash1EngineFromEpeNhpInfo(V, defaultEntryValid_f, &oam_hash1_info))
        {
            DIAG_INFO_CHAR(pos, "hash-1-lookup-result", DIAG_LOOKUP_DEFAULT)
            DIAG_INFO_CHAR(pos, "key-name", key_name[value])
            DIAG_INFO_UINT32(pos, "key-index", key_index)
        }
        else if (GetDbgEgrXcOamHash1EngineFromEpeNhpInfo(V, resultValid_f, &oam_hash1_info))
        {
            DIAG_INFO_CHAR(pos, "hash-1-lookup-result", DIAG_LOOKUP_HIT)
            DIAG_INFO_CHAR(pos, "key-name", key_name[value])
            DIAG_INFO_UINT32(pos, "key-index", key_index)
        }
        else
        {
            DIAG_INFO_CHAR(pos, "hash-1-lookup-result", DIAG_LOOKUP_HIT_NONE)
        }
    }

    return CTC_E_NONE;
}


STATIC int32
_sys_tsingma_diag_pkt_trace_get_epe_hdj_info(uint8 lchip, ctc_diag_pkt_trace_result_t* p_rslt)
{
    uint32 cmd = 0;
    uint32 value = 0;
    uint32 nhp_nhpIs8w = 0;
    uint32 dsNextHopInternalBase = 0;
    uint8 pos = CTC_DIAG_TRACE_POS_EPE;
    char* vlanTagAction[] = {"None", "Replace", "Add", "Remove"};
    char* str_opType[] = {"NORMAL", "LTMX", "E2ILOOP", "DMTX", "C2C", "PTP", "NAT", "OAM"};
    DbgEpeHdrAdjInfo_m hdr_info;

    sal_memset(&hdr_info, 0, sizeof(DbgEpeHdrAdjInfo_m));

    cmd = DRV_IOR(DbgEpeHdrAdjInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hdr_info));
    cmd = DRV_IOR(EpeHdrAdjustCtl_t, EpeHdrAdjustCtl_dsNextHopInternalBase_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &dsNextHopInternalBase));
    cmd = DRV_IOR(DbgEpeNextHopInfo_t, DbgEpeNextHopInfo_nhpIs8w_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &nhp_nhpIs8w));

    if (!GetDbgEpeHdrAdjInfo(V, valid_f, &hdr_info))
    {
        return CTC_E_NONE;
    }
    p_rslt->position = CTC_DIAG_TRACE_POS_EPE;
    DIAG_INFO_NONE(pos, "EPE Header Adjust")

    value = GetDbgEpeHdrAdjInfo(V, channelId_f, &hdr_info);
    DIAG_INFO_UINT32(pos, "channel-id", value)
    if (!CTC_FLAG_ISSET(p_rslt->flags, CTC_DIAG_TRACE_RSTL_FLAG_MC_GROUP))
    {
        p_rslt->channel = value;
        p_rslt->flags |= CTC_DIAG_TRACE_RSTL_FLAG_CHANNLE;
    }
    if (value == MCHIP_CAP(SYS_CAP_CHANID_ILOOP))
    {
        p_rslt->loop_flags |= CTC_DIAG_LOOP_FLAG_ILOOP;
    }
    if (value == MCHIP_CAP(SYS_CAP_CHANID_ELOOP))
    {
        p_rslt->loop_flags |= CTC_DIAG_LOOP_FLAG_ELOOP;
    }

    DIAG_INFO_UINT32(pos, "local-phy-port", GetDbgEpeHdrAdjInfo(V, localPhyPort_f, &hdr_info))

    value = GetDbgEpeHdrAdjInfo(V, nextHopPtr_f, &hdr_info);
    DIAG_INFO_U32_HEX(pos, "nexthop-ptr", value)
    if ((value >> 4) != dsNextHopInternalBase)
    {
        DIAG_INFO_CHAR(pos, "nexthop-table-name", nhp_nhpIs8w? "DsNextHop8W" : "DsNextHop4W")
        DIAG_INFO_UINT32(pos, "nexthop-table-index", value >> 4)
    }
    else
    {
        DIAG_INFO_CHAR(pos, "nexthop-table-name",  "EpeNextHopInternal")
        DIAG_INFO_UINT32(pos, "nexthop-table-index", (value & 0xF) >> 1)
    }

    DIAG_INFO_UINT32(pos, "mux-length-type", GetDbgEpeHdrAdjInfo(V, muxLengthType_f, &hdr_info))
    value = GetDbgEpeHdrAdjInfo(V, packetOffset_f, &hdr_info);
    if (value)
    {
        DIAG_INFO_UINT32(pos, "strip-packet(bytes)", value)
    }
    DIAG_INFO_UINT32(pos, "packet-length-adjust", GetDbgEpeHdrAdjInfo(V, packetLengthAdjust_f, &hdr_info))
    DIAG_INFO_UINT32(pos, "packet-length-adjust-type", GetDbgEpeHdrAdjInfo(V, packetLengthAdjustType_f, &hdr_info))
    DIAG_INFO_CHAR(pos, "operation-type", str_opType[GetDbgEpeHdrAdjInfo(V, operationType_f, &hdr_info)])
    if (GetDbgEpeHdrAdjInfo(V, svlanTagOperationValid_f, &hdr_info))
    {
        DIAG_INFO_CHAR(pos, "svlan-action", vlanTagAction[GetDbgEpeHdrAdjInfo(V, sTagAction_f, &hdr_info)])
    }
    if (GetDbgEpeHdrAdjInfo(V, cvlanTagOperationValid_f, &hdr_info))
    {
        DIAG_INFO_CHAR(pos, "cvlan-action", vlanTagAction[GetDbgEpeHdrAdjInfo(V, cTagAction_f, &hdr_info)])
    }
    if (GetDbgEpeHdrAdjInfo(V, isSendtoCpuEn_f, &hdr_info))
    {
        DIAG_INFO_CHAR(pos, "send-to-cpu", "TRUE")
    }
    if (GetDbgEpeHdrAdjInfo(V, fromFabric_f, &hdr_info))
    {
        DIAG_INFO_CHAR(pos, "packet-from-fabric", "TRUE")
    }
    if (GetDbgEpeHdrAdjInfo(V, pktWithCidHeader_f, &hdr_info))
    {
        DIAG_INFO_CHAR(pos, "packet-with-cid-header", "TRUE")
    }
    if (GetDbgEpeHdrAdjInfo(V, bypassAll_f, &hdr_info))
    {
        DIAG_INFO_CHAR(pos, "egress-edit-bypass-all", "TRUE")
    }
    if (GetDbgEpeHdrAdjInfo(V, ingressEditNexthopBypassAll_f, &hdr_info))
    {
        DIAG_INFO_CHAR(pos, "ingress-edit-bypass-all", "TRUE")
    }

    /* Drop Process */
    _sys_tsingma_diag_pkt_tracce_drop_map(p_rslt,
                                          GetDbgEpeHdrAdjInfo(V, discard_f, &hdr_info),
                                          GetDbgEpeHdrAdjInfo(V, discardType_f,  &hdr_info));
    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_diag_pkt_trace_get_epe_acl_info(uint8 lchip, ctc_diag_pkt_trace_result_t* p_rslt)
{
    uint32 cmd = 0;
    uint8 idx = 0;
    uint32 value = 0;
    uint8 pos = CTC_DIAG_TRACE_POS_EPE;
    char* acl_key_type_desc[] = {"TCAML2KEY", "TCAMl3KEY","TCAML3EXTKEY","TCAMIPV6KEY","TCAMIPV6EXTKEY","TCAML2L3KEY","TCAML2L3KEYEXT",
                                 "TCAML2IPV6KEY","TCAMCIDKEY", "TCAMINTFKEY", "TCAMFWDKEY", "TCAMFWDEXT","TCAMCOPPKEY","TCAMCOPPEXTKEY","TCAMUDFKEY"};
    EpeAclQosCtl_m       epe_acl_ctl;
    DbgFlowTcamEngineEpeAclInfo0_m epe_acl_info[3];
    DbgFlowTcamEngineEpeAclKeyInfo0_m dbg_acl_key_info;
    uint8  step = DbgFlowTcamEngineEpeAclKeyInfo1_key_f - DbgFlowTcamEngineEpeAclKeyInfo0_key_f;
    uint32 acl_key_info_key_id[3] = {DbgFlowTcamEngineEpeAclKeyInfo0_t, DbgFlowTcamEngineEpeAclKeyInfo1_t,
                                 DbgFlowTcamEngineEpeAclKeyInfo2_t};
    uint8 step2 = EpeAclQosCtl_gGlbAcl_1_aclLookupType_f - EpeAclQosCtl_gGlbAcl_0_aclLookupType_f ;
    DbgEpeAclInfo_m dbg_epe_acl_info;

    sal_memset(&epe_acl_ctl, 0, sizeof(epe_acl_ctl));
    sal_memset(epe_acl_info, 0, 3 * sizeof(DbgFlowTcamEngineEpeAclInfo0_m));
    cmd = DRV_IOR(DbgFlowTcamEngineEpeAclInfo0_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_acl_info[0]));
    cmd = DRV_IOR(DbgFlowTcamEngineEpeAclInfo1_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_acl_info[1]));
    cmd = DRV_IOR(DbgFlowTcamEngineEpeAclInfo2_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_acl_info[2]));
    cmd = DRV_IOR(EpeAclQosCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_acl_ctl));
    sal_memset(&dbg_epe_acl_info, 0, sizeof(dbg_epe_acl_info));
    cmd = DRV_IOR(DbgEpeAclInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &dbg_epe_acl_info));

    if (!GetDbgEpeAclInfo(V, aclLookupLevelEnBitMap_f, &dbg_epe_acl_info)
        || !GetDbgEpeAclInfo(V, valid_f, &dbg_epe_acl_info))
    {
        return CTC_E_NONE;
    }
    p_rslt->position = CTC_DIAG_TRACE_POS_EPE;
    DIAG_INFO_NONE(pos, "Egress ACL/TCAM Processing")

    /*tcam*/
    if (GetDbgFlowTcamEngineEpeAclInfo0(V, valid_f, &epe_acl_info[0])
        || GetDbgFlowTcamEngineEpeAclInfo1(V, valid_f, &epe_acl_info[1])
        || GetDbgFlowTcamEngineEpeAclInfo2(V, valid_f, &epe_acl_info[2]))
    {

        char* lookup_result[3] = {"TCAM0-lookup-result", "TCAM1-lookup-result", "TCAM2-lookup-result"};
        uint32 result_valid[3] = {GetDbgFlowTcamEngineEpeAclInfo0(V, egrTcamResultValid0_f, &epe_acl_info[0]),
                                  GetDbgFlowTcamEngineEpeAclInfo1(V, egrTcamResultValid1_f, &epe_acl_info[1]),
                                  GetDbgFlowTcamEngineEpeAclInfo2(V, egrTcamResultValid2_f, &epe_acl_info[2])};
        uint32 tcam_index[3] = {GetDbgFlowTcamEngineEpeAclInfo0(V, egrTcamIndex0_f, &epe_acl_info[0]),
                                GetDbgFlowTcamEngineEpeAclInfo1(V, egrTcamIndex1_f, &epe_acl_info[1]),
                                GetDbgFlowTcamEngineEpeAclInfo2(V, egrTcamIndex2_f, &epe_acl_info[2])};
        uint32 acl_key_info[10] = {0};
        uint32 acl_key_valid = 0;
        uint32 acl_key_size = 0;  /* 0:80bit; 1:160bit; 2:320bit; 3:640bit */
        uint32 key_index = 0;
        uint32 acl_key_type = 0;

        value = GetDbgEpeAclInfo(V, aclLookupLevelEnBitMap_f, &dbg_epe_acl_info);
        for (idx = 0; idx < 3 ; idx++)
        {
            if (!((value >> idx) & 0x1))
            {
                continue;
            }
            cmd = DRV_IOR(acl_key_info_key_id[idx], DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &dbg_acl_key_info));
            DRV_GET_FIELD_A(lchip, acl_key_info_key_id[idx], DbgFlowTcamEngineEpeAclKeyInfo0_key_f + step * idx,
                            &dbg_acl_key_info, &acl_key_info);
            DRV_GET_FIELD_A(lchip, acl_key_info_key_id[idx], DbgFlowTcamEngineEpeAclKeyInfo0_valid_f + step * idx,
                            &dbg_acl_key_info, &acl_key_valid);
            DRV_GET_FIELD_A(lchip, acl_key_info_key_id[idx], DbgFlowTcamEngineEpeAclKeyInfo0_keySize_f + step * idx,
                            &dbg_acl_key_info, &acl_key_size);

            if (result_valid[idx])
            {
                DIAG_INFO_CHAR(pos, lookup_result[idx], DIAG_LOOKUP_HIT)
                _sys_tsingma_diag_map_tcam_key_index(lchip, tcam_index[idx], 1, idx, &key_index);
                DIAG_INFO_UINT32(pos, "key-index", key_index / (1<<acl_key_size))
                DIAG_INFO_UINT32(pos, "key-size", 80*(1<<acl_key_size))
            }
            else
            {
                DIAG_INFO_CHAR(pos, lookup_result[idx], DIAG_LOOKUP_HIT_NONE)
            }
            /*get key type*/
            if(!acl_key_valid)
            {
                continue;
            }
            if (11 == GetEpeAclQosCtl(V, gGlbAcl_0_aclLookupType_f + step2 * idx, &epe_acl_ctl))
            {
                acl_key_type = GetDsAclQosCoppKey320Ing0(V, keyType_f, &acl_key_info);
                acl_key_type = (0 == acl_key_type) ? 12 : 13;
            }
            else
            {
                acl_key_type = GetDsAclQosL3Key320Ing0(V, aclQosKeyType0_f, &acl_key_info);
            }
            if(acl_key_type <= 14)
            {
                DIAG_INFO_CHAR(pos, "key-type", acl_key_type_desc[acl_key_type])
            }
        }
    }

    /* Exception Process */
    _sys_tsingma_diag_pkt_tracce_exception_map(p_rslt,
                                               GetDbgEpeAclInfo(V, exceptionEn_f, &dbg_epe_acl_info),
                                               GetDbgEpeAclInfo(V, exceptionIndex_f, &dbg_epe_acl_info),
                                               GetDbgEpeAclInfo(V, exceptionSubIndex_f, &dbg_epe_acl_info));
    return CTC_E_NONE;
}
STATIC int32
_sys_tsingma_diag_pkt_trace_get_epe_nh_map_info(uint8 lchip, ctc_diag_pkt_trace_result_t* p_rslt)
{
    uint32 cmd = 0;
    uint8 pos = CTC_DIAG_TRACE_POS_EPE;
    uint32 value = 0;
    uint32 gport = 0;
    char* str_pl_op[] = {"NONE", "ROUTE", "BRIDGE", "BRIDGE-VPLS", "BRIDGE-INNER", "MIRROR", "ROUTE-NOTTL", "ROUTE-COMPACT"};
    DbgEpeNextHopInfo_m nh_info;

    sal_memset(&nh_info, 0, sizeof(DbgEpeNextHopInfo_m));
    cmd = DRV_IOR(DbgEpeNextHopInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &nh_info));
    if (!GetDbgEpeNextHopInfo(V, valid_f, &nh_info))
    {
        return CTC_E_NONE;
    }
    p_rslt->position = CTC_DIAG_TRACE_POS_EPE;
    DIAG_INFO_NONE(pos, "NextHop Mapper");

    cmd = DRV_IOR(DbgEpeHdrAdjInfo_t, DbgEpeHdrAdjInfo_valid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    if (value)
    {
        cmd = DRV_IOR(DbgEpeHdrAdjInfo_t, DbgEpeHdrAdjInfo_localPhyPort_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        cmd = DRV_IOR(DsDestPort_t, DsDestPort_globalDestPort_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, value, cmd, &gport));
        if (SYS_DRV_IS_LINKAGG_PORT(gport))
        {
            DIAG_INFO_UINT32(pos, "linkagg-group", gport&0xFF)
        }
        DIAG_INFO_U32_HEX(pos, "dest-gport", SYS_MAP_DRV_GPORT_TO_CTC_GPORT(value))
        p_rslt->dest_type = CTC_DIAG_DEST_NETWORK;
        p_rslt->port = SYS_MAP_DRV_GPORT_TO_CTC_GPORT(value);
        p_rslt->flags |= CTC_DIAG_TRACE_RSTL_FLAG_PORT;
    }
    value = GetDbgEpeNextHopInfo(V, destVlanPtr_f, &nh_info);
    if (value < 4096)
    {
        DIAG_INFO_UINT32(pos, "vlan-ptr", value)
    }
    if (GetDbgEpeNextHopInfo(V, outputSvlanIdValid_f, &nh_info))
    {
        DIAG_INFO_UINT32(pos, "output-svlan", GetDbgEpeNextHopInfo(V, outputSvlanId_f, &nh_info))
    }
    if (GetDbgEpeNextHopInfo(V, outputCvlanIdValid_f, &nh_info))
    {
        DIAG_INFO_UINT32(pos, "output-cvlan", GetDbgEpeNextHopInfo(V, outputCvlanId_f, &nh_info))
    }
    value = GetDbgEpeNextHopInfo(V, interfaceId_f, &nh_info);
    if (value)
    {
        p_rslt->l3if_id = value;
        p_rslt->flags |= CTC_DIAG_TRACE_RSTL_FLAG_L3IF_ID;
        DIAG_INFO_UINT32(pos, "interface-id", value)
    }
    value = GetDbgEpeNextHopInfo(V, logicDestPort_f, &nh_info);
    if (value)
    {
        DIAG_INFO_UINT32(pos, "logic-port", value)
    }
    if (GetDbgEpeNextHopInfo(V, nextApsBridgeEn_f, &nh_info))
    {
        DIAG_INFO_UINT32(pos, "next-aps-group", GetDbgEpeNextHopInfo(V, nextApsGroupId_f, &nh_info))
    }
    DIAG_INFO_CHAR(pos, "payload-opration", str_pl_op[GetDbgEpeNextHopInfo(V, payloadOperation_f, &nh_info)])
    if (GetDbgEpeNextHopInfo(V, portLogEn_f, &nh_info))
    {
        DIAG_INFO_CHAR(pos, "port-log", "ENABLE")
    }
    if (GetDbgEpeNextHopInfo(V, portIsolateValid_f, &nh_info))
    {
        DIAG_INFO_CHAR(pos, "port-isolate", "ENABLE")
    }

    /* Drop Process */
    _sys_tsingma_diag_pkt_tracce_drop_map(p_rslt,
                                          GetDbgEpeNextHopInfo(V, discard_f, &nh_info),
                                          GetDbgEpeNextHopInfo(V, discardType_f,  &nh_info));
    /* Exception Process */
    _sys_tsingma_diag_pkt_tracce_exception_map(p_rslt,
                                               GetDbgEpeNextHopInfo(V, exceptionEn_f, &nh_info),
                                               GetDbgEpeNextHopInfo(V, exceptionIndex_f, &nh_info),
                                               GetDbgEpeNextHopInfo(V, exceptionSubIndex_f, &nh_info));
    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_diag_pkt_trace_get_epe_oam_info(uint8 lchip, ctc_diag_pkt_trace_result_t* p_rslt)
{
    uint32 cmd = 0;
    uint8 idx = 0;
    uint8 pos = CTC_DIAG_TRACE_POS_EPE;
    uint32 value = 0;
    uint32 hdr_shareType = 0;
    char* lookup_result[4] = {"ETHER", "BFD", "TP", "Section"};
    char* key_name[4] = {"DsEgressXcOamEthHashKey", "DsEgressXcOamBfdHashKey",
                         "DsEgressXcOamMplsLabelHashKey", "DsEgressXcOamMplsSectionHashKey"};

    DbgEgrXcOamHashEngineFromEpeOam0Info_m epe_oam0_info;
    DbgEgrXcOamHashEngineFromEpeOam1Info_m epe_oam1_info;
    DbgEgrXcOamHashEngineFromEpeOam2Info_m epe_oam2_info;
    DbgEpeOamInfo_m dbg_epe_oam_info;
    uint8 key_type[3] = {0};
    uint8 hash_valid[3] = {0};
    uint8 confict[3] = {0};
    uint32 key_index[3] = {0};
    uint8 hash_resultValid[3] = {0};

    sal_memset(&epe_oam0_info, 0, sizeof(epe_oam0_info));
    cmd = DRV_IOR(DbgEgrXcOamHashEngineFromEpeOam0Info_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_oam0_info));
    sal_memset(&epe_oam1_info, 0, sizeof(epe_oam1_info));
    cmd = DRV_IOR(DbgEgrXcOamHashEngineFromEpeOam1Info_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_oam1_info));
    sal_memset(&epe_oam2_info, 0, sizeof(epe_oam2_info));
    cmd = DRV_IOR(DbgEgrXcOamHashEngineFromEpeOam2Info_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_oam2_info));
    sal_memset(&dbg_epe_oam_info, 0, sizeof(dbg_epe_oam_info));
    cmd = DRV_IOR(DbgEpeOamInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &dbg_epe_oam_info));

    if (!GetDbgEpeOamInfo(V, valid_f, &dbg_epe_oam_info))
    {
        return CTC_E_NONE;
    }
    p_rslt->position = CTC_DIAG_TRACE_POS_EPE;
    hash_resultValid[0] = GetDbgEgrXcOamHashEngineFromEpeOam0Info(V, resultValid_f, &epe_oam0_info);
    confict[0] = GetDbgEgrXcOamHashEngineFromEpeOam0Info(V, hashConflict_f, &epe_oam0_info);
    key_index[0] = GetDbgEgrXcOamHashEngineFromEpeOam0Info(V, keyIndex_f, &epe_oam0_info);
    hash_valid[0] = GetDbgEgrXcOamHashEngineFromEpeOam0Info(V, valid_f, &epe_oam0_info);

    hash_resultValid[1] = GetDbgEgrXcOamHashEngineFromEpeOam1Info(V, resultValid_f, &epe_oam1_info);
    confict[1] = GetDbgEgrXcOamHashEngineFromEpeOam1Info(V, hashConflict_f, &epe_oam1_info);
    key_index[1] = GetDbgEgrXcOamHashEngineFromEpeOam1Info(V, keyIndex_f, &epe_oam1_info);
    hash_valid[1] = GetDbgEgrXcOamHashEngineFromEpeOam1Info(V, valid_f, &epe_oam1_info);

    hash_resultValid[2] = GetDbgEgrXcOamHashEngineFromEpeOam2Info(V, resultValid_f, &epe_oam2_info);
    confict[2] = GetDbgEgrXcOamHashEngineFromEpeOam2Info(V, hashConflict_f, &epe_oam2_info);
    key_index[2] = GetDbgEgrXcOamHashEngineFromEpeOam2Info(V, keyIndex_f, &epe_oam2_info);
    hash_valid[2] = GetDbgEgrXcOamHashEngineFromEpeOam2Info(V, valid_f, &epe_oam2_info);

    key_type[0] = GetDbgEgrXcOamHashEngineFromEpeOam0Info(V, oamKeyType_f, &epe_oam0_info);
    key_type[1] = GetDbgEgrXcOamHashEngineFromEpeOam1Info(V, oamKeyType_f, &epe_oam1_info);
    key_type[2] = GetDbgEgrXcOamHashEngineFromEpeOam2Info(V, oamKeyType_f, &epe_oam2_info);

    if (key_type[0] || key_type[1] || key_type[2])
    {
        char str[64] = {0};
        for (idx = 0; idx < 3; idx++)
        {
            if (!key_type[idx] || !hash_valid[idx])
            {
                continue;
            }
            /*EGRESSXCOAMHASHTYPE_ETH = 0x10*/
            value = key_type[idx] - 0x10;
            sal_sprintf(str, "hash-%d", idx);
            DIAG_INFO_CHAR(pos, str, lookup_result[value])
            if (confict[idx])
            {
                DIAG_INFO_CHAR(pos, "lookup-result", DIAG_LOOKUP_CONFLICT)
                DIAG_INFO_CHAR(pos, "key-name", key_name[value])
                DIAG_INFO_UINT32(pos, "key-index", key_index[idx])
            }
            else if (hash_resultValid[idx])
            {
                DIAG_INFO_CHAR(pos, "lookup-result", DIAG_LOOKUP_HIT)
                DIAG_INFO_CHAR(pos, "key-name", key_name[value])
                DIAG_INFO_UINT32(pos, "key-index", key_index[idx])
            }
            else
            {
                DIAG_INFO_CHAR(pos, "lookup-result", DIAG_LOOKUP_HIT_NONE)
            }
        }
    }

    if (hash_resultValid[0] || hash_resultValid[1] || hash_resultValid[2])
    {
        value = GetDbgEpeOamInfo(V, oamDestChipId_f, &dbg_epe_oam_info);
        if (GetDbgEpeOamInfo(V, mepEn_f, &dbg_epe_oam_info))
        {
            DIAG_INFO_CHAR(pos, "MEP", "HIT")
            DIAG_INFO_UINT32(pos, "master-chip-id", value)
            DIAG_INFO_UINT32(pos, "mep-index", GetDbgEpeOamInfo(V, mepIndex_f, &dbg_epe_oam_info))
        }
        if (GetDbgEpeOamInfo(V, mipEn_f, &dbg_epe_oam_info))
        {
            DIAG_INFO_CHAR(pos, "MIP", "HIT")
            DIAG_INFO_UINT32(pos, "master-chip-id", value)
        }
        value = GetDbgEpeOamInfo(V, tempRxOamType_f, &dbg_epe_oam_info);
        if (value)
        {
            /*RXOAMTYPE_TRILLBFD = 0xa*/
            char* oam_type[10] =
            {
                "ETHER OAM", "IP BFD", "PBT OAM", "PBBBSI", "PBBBV",
                "MPLS OAM", "MPLS BFD", "ACH OAM", "RSV", "TRILL BFD"
            };
            DIAG_INFO_CHAR(pos, "oam-type", oam_type[value - 1])
        }
        if (GetDbgEpeOamInfo(V, mplsLmValid_f, &dbg_epe_oam_info) || GetDbgEpeOamInfo(V, etherLmValid_f, &dbg_epe_oam_info))
        {
            DIAG_INFO_CHAR(pos, "action", "read-LM")
            DIAG_INFO_CHAR(pos, "table-name", "DsOamLmStats")
            DIAG_INFO_UINT32(pos, "table-index", GetDbgEpeOamInfo(V, lmStatsIndex_f, &dbg_epe_oam_info))
        }
        if (GetDbgEpeOamInfo(V, lmStatsEn0_f, &dbg_epe_oam_info))
        {
            DIAG_INFO_CHAR(pos, "action", "write-LM")
            DIAG_INFO_CHAR(pos, "table-name", "DsOamLmStats")
            DIAG_INFO_UINT32(pos, "table-index", GetDbgEpeOamInfo(V, lmStatsPtr0_f, &dbg_epe_oam_info))
        }
        if (GetDbgEpeOamInfo(V, lmStatsEn1_f, &dbg_epe_oam_info))
        {
            DIAG_INFO_CHAR(pos, "action", "write-LM")
            DIAG_INFO_CHAR(pos, "table-name", "DsOamLmStats")
            DIAG_INFO_UINT32(pos, "table-index", GetDbgEpeOamInfo(V, lmStatsPtr1_f, &dbg_epe_oam_info))
        }
        if (GetDbgEpeOamInfo(V, lmStatsEn2_f, &dbg_epe_oam_info))
        {
            DIAG_INFO_CHAR(pos, "action", "write-LM")
            DIAG_INFO_CHAR(pos, "table-name", "DsOamLmStats")
            DIAG_INFO_UINT32(pos, "table-index", GetDbgEpeOamInfo(V, lmStatsPtr2_f, &dbg_epe_oam_info))
        }
    }

    cmd = DRV_IOR(DbgEpeHdrEditInfo_t, DbgEpeHdrEditInfo_valid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    cmd = DRV_IOR(DbgEpeHdrEditInfo_t, DbgEpeHdrEditInfo_shareType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hdr_shareType));
    if ((value) && (4 == hdr_shareType))/*SHARETYPE_OAM = 4*/
    {
        DIAG_INFO_CHAR(pos, "destination", "OAM")
        CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_get_oam_rx_info(lchip, p_rslt));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_diag_pkt_trace_get_epe_edit_info(uint8 lchip, ctc_diag_pkt_trace_result_t* p_rslt)
{
    uint32 cmd = 0;
    uint32 value = 0;
    uint32 value1 = 0;
    uint32 label = 0;
    uint32 step = 0;
    uint32 edit_valid = 0;
    uint32 nhp_nhpIs8w = 0;
    uint32 nhpShareType = 0;
    uint8 pos = CTC_DIAG_TRACE_POS_EPE;
    uint32 dsNextHopInternalBase = 0;
    uint32 nhpOuterEditPtrType = 0;
    hw_mac_addr_t hw_mac = {0};
    char* l3_edit[12] =
    {
        "NONE-edit",
        "MPLS-edit",
        "RESERVED-edit",
        "NAT-edit",
        "NAT-edit",
        "TUNNELV4-edit",
        "TUNNELV6-edit",
        "L3FLEX-edit",
        "OPEN-FLOW-edit",
        "OPEN-FLOW-edit",
        "LOOPBACK-edit",
        "TRILL-edit"
    };
    char* l3_edit_name[12] =
    {
        " ",
        "DsL3EditMpls3W",
        " ",
        "DsL3EditNat3W",
        "DsL3EditNat6W",
        "DsL3EditTunnelV4",
        "DsL3EditTunnelV6",
        "DsL3EditFlex",
        "DsL3EditOf6W",
        "DsL3EditOf12W",
        "DsL3EditLoopback",
        "DsL3EditTrill"
    };
    char* vlan_action[4] = {"None", "Replace", "Add","Delete"};

    DsNextHop8W_m nexthop_8w;
    DsNextHop4W_m nexthop_4w;
    DsL2EditEth3W_m l2edit_eth3w;
    DsL3EditMpls3W_m l3edit_mpls3w;
    DsL2Edit6WOuter_m l2edit_6w_outer;
    DsL2EditSwap_m l2edit_swap;
    DsL2EditInnerSwap_m l2edit_inner_swap;
    DsL3Edit6W3rd_m l3edit_6w3rd;
    DbgEpeHdrAdjInfo_m hdr_adj_info;
    DbgEpeNextHopInfo_m nexthop_info;
    DbgEpePayLoadInfo_m payload_info;
    DbgEpeEgressEditInfo_m egress_edit_info;
    DbgEpeInnerL2EditInfo_m inner_l2edit_info;
    DbgEpeOuterL2EditInfo_m outer_l2edit_info;
    DbgEpeL3EditInfo_m l3edit_info;

    uint32 next_edit_ptr = 0;
    uint32 next_edit_type = 0;
    uint32 do_l2_edit = 0;
    uint32 l2edit_valid = 0;
    uint32 l3edit_valid = 0;
    uint32 l2_edit_ptr = 0;
    uint32 nhp_outerEditPtr = 0;
    uint32 l2_individual_memory = 0;
    uint32 l3edit_l3RewriteType = 0;
    uint32 l2edit_l2RewriteType = 0;
    uint32 edit_outerEditPipe1Exist = 0;
    uint32 edit_nhpOuterEditLocation = 0;

    sal_memset(&nexthop_8w, 0, sizeof(nexthop_8w));
    sal_memset(&nexthop_4w, 0, sizeof(nexthop_4w));
    sal_memset(&l2edit_eth3w, 0, sizeof(l2edit_eth3w));
    sal_memset(&l3edit_mpls3w, 0, sizeof(l3edit_mpls3w));
    sal_memset(&l2edit_6w_outer, 0, sizeof(l2edit_6w_outer));
    sal_memset(&l2edit_swap, 0, sizeof(l2edit_swap));
    sal_memset(&l2edit_inner_swap, 0, sizeof(l2edit_inner_swap));
    sal_memset(&l3edit_6w3rd, 0, sizeof(l3edit_6w3rd));

    /*DbgEpeHdrAdjInfo*/
    sal_memset(&hdr_adj_info, 0, sizeof(hdr_adj_info));
    cmd = DRV_IOR(DbgEpeHdrAdjInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hdr_adj_info));
    sal_memset(&nexthop_info, 0, sizeof(nexthop_info));
    cmd = DRV_IOR(DbgEpeNextHopInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &nexthop_info));
    sal_memset(&payload_info, 0, sizeof(payload_info));
    cmd = DRV_IOR(DbgEpePayLoadInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &payload_info));
    sal_memset(&egress_edit_info, 0, sizeof(egress_edit_info));
    cmd = DRV_IOR(DbgEpeEgressEditInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &egress_edit_info));
    sal_memset(&inner_l2edit_info, 0, sizeof(inner_l2edit_info));
    cmd = DRV_IOR(DbgEpeInnerL2EditInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &inner_l2edit_info));
    sal_memset(&outer_l2edit_info, 0, sizeof(outer_l2edit_info));
    cmd = DRV_IOR(DbgEpeOuterL2EditInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &outer_l2edit_info));
    sal_memset(&l3edit_info, 0, sizeof(l3edit_info));
    cmd = DRV_IOR(DbgEpeL3EditInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &l3edit_info));


    edit_valid = GetDbgEpeEgressEditInfo(V, valid_f, &egress_edit_info);
    l2edit_valid = GetDbgEpeOuterL2EditInfo(V, valid_f, &outer_l2edit_info);
    l3edit_valid = GetDbgEpeL3EditInfo(V, valid_f, &l3edit_info);
    l3edit_l3RewriteType = GetDbgEpeL3EditInfo(V, l3RewriteType_f, &l3edit_info);
    edit_outerEditPipe1Exist = GetDbgEpeEgressEditInfo(V, outerEditPipe1Exist_f, &egress_edit_info);
    edit_nhpOuterEditLocation = GetDbgEpeNextHopInfo(V, nhpOuterEditLocation_f, &nexthop_info);
    nhp_outerEditPtr = GetDbgEpeNextHopInfo(V, nhpOuterEditPtr_f, &nexthop_info);
    l2edit_l2RewriteType = GetDbgEpeOuterL2EditInfo(V, l2RewriteType_f, &outer_l2edit_info);

    if (!(edit_valid || l2edit_valid || l3edit_valid))
    {
        return CTC_E_NONE;
    }
    p_rslt->position = CTC_DIAG_TRACE_POS_EPE;
    DIAG_INFO_NONE(pos, "Layer3 Packet Editing")

    if (GetDbgEpePayLoadInfo(V, valid_f, &payload_info))
    {

        if (GetDbgEpePayLoadInfo(V, newTtlValid_f, &payload_info))
        {
            DIAG_INFO_UINT32(pos, "new-ttl", GetDbgEpePayLoadInfo(V, newTtl_f, &payload_info))
        }
        if (GetDbgEpePayLoadInfo(V, newDscpValid_f, &payload_info))
        {
            DIAG_INFO_UINT32(pos, "new-dscp", GetDbgEpePayLoadInfo(V, newDscp_f, &payload_info))
        }
    }

    cmd = DRV_IOR(EpeHdrAdjustCtl_t, EpeHdrAdjustCtl_dsNextHopInternalBase_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &dsNextHopInternalBase));

    nhp_nhpIs8w = GetDbgEpeNextHopInfo(V, nhpIs8w_f, &nexthop_info);
    value = GetDbgEpeHdrAdjInfo(V, nextHopPtr_f, &hdr_adj_info);
    if ((value >> 4) != dsNextHopInternalBase)
    {
        cmd = DRV_IOR(DsNextHop8W_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, (value >> 1) << 1, cmd, &nexthop_8w));

        if (!nhp_nhpIs8w)
        {
            if (IS_BIT_SET(value, 0))
            {
                sal_memcpy((uint8*)&nexthop_4w, (uint8*)&nexthop_8w + sizeof(nexthop_4w), sizeof(nexthop_4w));
            }
            else
            {
                sal_memcpy((uint8*)&nexthop_4w, (uint8*)&nexthop_8w , sizeof(nexthop_4w));
            }
        }
    }
    else
    {
        cmd = DRV_IOR(EpeNextHopInternal_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, (value & 0xF) >> 1, cmd, &nexthop_8w));
        if (!nhp_nhpIs8w)
        {
            if (IS_BIT_SET(value & 0xF , 0))
            {
                sal_memcpy((uint8*)&nexthop_4w, (uint8*)&nexthop_8w + sizeof(nexthop_4w), sizeof(nexthop_4w));
            }
            else
            {
                sal_memcpy((uint8*)&nexthop_4w, (uint8*)&nexthop_8w , sizeof(nexthop_4w));
            }
        }
    }

    if (GetDbgEpeL3EditInfo(V, needDot1AeEncrypt_f, &l3edit_info))
    {
        uint32 dot1ae_current_an = 0;
        uint32 dot1ae_next_an = 0;
        uint32 dot1ae_pn = 0;
        uint64 dot1ae_pn_thrd = 0;
        uint32 dot1ae_sci_en = 0;
        uint32 dot1ae_ebit_cbit[4] = {0};
        value = GetDbgEpeL3EditInfo(V, dot1AeSaIndexBase_f, &l3edit_info);
        value1 = GetDbgEpeL3EditInfo(V, dot1AeSaIndex_f, &l3edit_info);
        step = EpeDot1AeAnCtl0_array_1_currentAn_f - EpeDot1AeAnCtl0_array_0_currentAn_f;
        cmd = DRV_IOR(EpeDot1AeAnCtl0_t, EpeDot1AeAnCtl0_array_0_currentAn_f + step * value);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &dot1ae_current_an));

        step = EpeDot1AeAnCtl1_array_1_nextAn_f - EpeDot1AeAnCtl1_array_0_nextAn_f;
        cmd = DRV_IOR(EpeDot1AeAnCtl1_t, EpeDot1AeAnCtl1_array_0_nextAn_f + step * value);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &dot1ae_next_an));
        cmd = DRV_IOR(DsDestPort_t, DsDestPort_dot1AeSciEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, GetDbgEpeHdrAdjInfo(V, localPhyPort_f, &hdr_adj_info), cmd, &dot1ae_sci_en));
        cmd = DRV_IOR(DsEgressDot1AePn_t, DsEgressDot1AePn_pn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, value1, cmd, &dot1ae_pn));
        cmd = DRV_IOR(EpePktProcCtl_t, EpePktProcCtl_dot1AePnThreshold_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &dot1ae_pn_thrd));
        cmd = DRV_IOR(EpePktProcCtl_t, EpePktProcCtl_dot1AeTciEbitCbit_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, dot1ae_ebit_cbit));

        DIAG_INFO_CHAR(pos, "dot1ae-edit", "ENABLE")
        DIAG_INFO_UINT32(pos, "dot1ae-current-an", dot1ae_current_an)
        DIAG_INFO_UINT32(pos, "dot1ae-next-an", dot1ae_next_an)
        DIAG_INFO_UINT32(pos, "dot1ae-current-pn", dot1ae_pn - 1)
        DIAG_INFO_UINT64(pos, "dot1ae-pn-threshold", dot1ae_pn_thrd)
        DIAG_INFO_UINT32(pos, "dot1ae-tx-sci-en", dot1ae_sci_en)
        DIAG_INFO_UINT32(pos, "dot1ae-sa-index-base", value)
        DIAG_INFO_UINT32(pos, "dot1ae-sa-index", value1)
        DIAG_INFO_UINT32(pos, "dot1ae-ebit-cbit", CTC_BMP_ISSET(dot1ae_ebit_cbit, value1))
    }

    if (nhp_nhpIs8w)
    {
        nhpShareType = GetDsNextHop8W(V, shareType_f, &nexthop_8w);
    }
    else
    {
        nhpShareType = GetDsNextHop4W(V, shareType_f, &nexthop_4w);
    }

    /*NHPSHARETYPE_L2EDIT_PLUS_L3EDIT_OP = 0*/
    if ((0 == nhpShareType) || (nhp_nhpIs8w))
    {
        if (nhp_nhpIs8w)
        {
            nhpOuterEditPtrType = GetDsNextHop8W(V, outerEditPtrType_f, &nexthop_8w);
        }
        else
        {
            nhpOuterEditPtrType = GetDsNextHop4W(V, outerEditPtrType_f, &nexthop_4w);
        }
        /*inner edit*/
        value = GetDbgEpeNextHopInfo(V, nhpInnerEditPtr_f, &nexthop_info);
        if (edit_valid
            && GetDbgEpeEgressEditInfo(V, innerEditExist_f, &egress_edit_info)
            && GetDbgEpeEgressEditInfo(V, innerEditPtrType_f, &egress_edit_info))/* inner for l3edit */
        {
            if (l3edit_valid)
            {
                /*L3REWRITETYPE_MPLS4W =1*/
                if (1 == l3edit_l3RewriteType)
                {
                    cmd = DRV_IOR(DsL3EditMpls3W_t, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, value, cmd, &l3edit_mpls3w));
                    label = GetDsL3EditMpls3W(V, label_f, &l3edit_mpls3w);

                    DIAG_INFO_CHAR(pos, "inner-mpls-edit", "ENABLE")
                    DIAG_INFO_UINT32(pos, "new-label", label)
                    DIAG_INFO_UINT32(pos, "edit-table-index", value)
                    DIAG_INFO_CHAR(pos, "edit-table-name", "DsL3EditMpls3W")
                }
                /*L3REWRITETYPE_NONE = 0*/
                else if (0 != l3edit_l3RewriteType)
                {
                    DIAG_INFO_CHAR(pos, l3_edit[l3edit_l3RewriteType], "ENABLE")
                    DIAG_INFO_UINT32(pos, "edit-table-index", value)
                    DIAG_INFO_CHAR(pos, "edit-table-name", l3_edit_name[l3edit_l3RewriteType])
                }
            }
        }

        /*outer pipeline1*/
        if (edit_valid && edit_outerEditPipe1Exist)
        {
            if (l3edit_valid && nhpOuterEditPtrType)/* l3edit */
            {
                /* share memory */
                if (!edit_nhpOuterEditLocation)
                {
                    cmd = DRV_IOR(DsL3EditMpls3W_t, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, nhp_outerEditPtr, cmd, &l3edit_mpls3w));
                    /* outerEditPtr and outerEditPtrType are in the same position in all l3edit, refer to DsL3Edit3WTemplate  */
                    next_edit_ptr = GetDsL3EditMpls3W(V, outerEditPtr_f, &l3edit_mpls3w);
                    next_edit_type = GetDsL3EditMpls3W(V, outerEditPtrType_f, &l3edit_mpls3w);
                    if (1 == l3edit_l3RewriteType)/*L3REWRITETYPE_MPLS4W=1*/
                    {

                        label = GetDsL3EditMpls3W(V, label_f, &l3edit_mpls3w);
                        DIAG_INFO_CHAR(pos, "outer-mpls-edit", "ENABLE")
                        DIAG_INFO_UINT32(pos, "new-label", label)
                        DIAG_INFO_UINT32(pos, "edit-table-index", nhp_outerEditPtr)
                        DIAG_INFO_CHAR(pos, "edit-table-name", "DsL3EditMpls3W")
                    }
                    else if (0 != l3edit_l3RewriteType)/*L3REWRITETYPE_NONE = 0*/
                    {
                        DIAG_INFO_CHAR(pos, l3_edit[l3edit_l3RewriteType], "ENABLE")
                        DIAG_INFO_UINT32(pos, "edit-table-index", nhp_outerEditPtr)
                        DIAG_INFO_CHAR(pos, "edit-table-name", l3_edit_name[l3edit_l3RewriteType])
                    }
                }
            }
            else if (l2edit_valid)/* l2edit */
            {
                do_l2_edit = 1;
                l2_edit_ptr = nhp_outerEditPtr;
                l2_individual_memory = edit_nhpOuterEditLocation ? 1 : 0;
            }
        }
        /*outer pipeline2*/
        if (edit_valid && GetDbgEpeEgressEditInfo(V, outerEditPipe2Exist_f, &egress_edit_info))/* outer pipeline2, individual memory for SPME */
        {
            if (l3edit_valid && next_edit_type)/* l3edit */
            {
                cmd = DRV_IOR(DsL3Edit6W3rd_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, next_edit_ptr/2, cmd, &l3edit_6w3rd));
                if (IS_BIT_SET((next_edit_ptr), 0))
                {
                    sal_memcpy((uint8*)&l3edit_mpls3w, (uint8*)&l3edit_6w3rd + sizeof(l3edit_mpls3w), sizeof(l3edit_mpls3w));
                }
                else
                {
                    sal_memcpy((uint8*)&l3edit_mpls3w, (uint8*)&l3edit_6w3rd , sizeof(l3edit_mpls3w));
                }
                label = GetDsL3EditMpls3W(V, label_f, &l3edit_mpls3w);
                DIAG_INFO_CHAR(pos, "outer-mpls-edit", "ENABLE")
                DIAG_INFO_UINT32(pos, "new-label", label)
                DIAG_INFO_UINT32(pos, "edit-table-index", next_edit_ptr/2)
                DIAG_INFO_CHAR(pos, "edit-table-name", "DsL3Edit6W3rd")
                next_edit_ptr = GetDsL3EditMpls3W(V, outerEditPtr_f, &l3edit_mpls3w);
                next_edit_type = GetDsL3EditMpls3W(V, outerEditPtrType_f, &l3edit_mpls3w);
            }
            else if (l2edit_valid)/* l2edit */
            {
                do_l2_edit = 1;
                if (!edit_outerEditPipe1Exist)
                {
                    l2_edit_ptr = nhp_outerEditPtr;
                }
                else
                {
                    l2_edit_ptr = next_edit_ptr;
                }
                l2_individual_memory = 1;
            }
        }
        /*outer pipeline3*/
        if (l2edit_valid && edit_valid && GetDbgEpeEgressEditInfo(V, outerEditPipe3Exist_f, &egress_edit_info))/* outer pipeline3, individual memory for l2edit */
        {
            do_l2_edit = 1;
            if (!edit_outerEditPipe1Exist)
            {
                l2_edit_ptr = nhp_outerEditPtr;
            }
            else
            {
                l2_edit_ptr = next_edit_ptr;
            }
            l2_individual_memory = 1;
        }
    }
    else if (2 == nhpShareType)/*NHPSHARETYPE_L2EDIT_PLUS_STAG_OP = 2*/
    {
        if (l2edit_valid)/* l2edit */
        {
            do_l2_edit = 1;
            l2_edit_ptr = nhp_outerEditPtr;
            l2_individual_memory = edit_nhpOuterEditLocation? 1 : 0;
        }
    }

    DIAG_INFO_NONE(pos, "Layer2 Packet Editing")
    if (GetDbgEpePayLoadInfo(V, valid_f, &payload_info))
    {
        value = GetDbgEpePayLoadInfo(V, svlanTagOperation_f, &payload_info);
        value1 = GetDbgEpePayLoadInfo(V, cvlanTagOperation_f, &payload_info);
        if (value || value1)
        {
            if (value)
            {
                DIAG_INFO_CHAR(pos, "svlan-edit", vlan_action[value])
                if ((DIAG_VLAN_ACTION_MODIFY == value) || (DIAG_VLAN_ACTION_ADD== value))
                {
                    value = GetDbgEpePayLoadInfo(V, l2NewSvlanTag_f, &payload_info);
                    DIAG_INFO_UINT32(pos, "new-cos", (value >> 13) & 0x7)
                    DIAG_INFO_UINT32(pos, "new-cfi", (value >> 12) & 0x1)
                    DIAG_INFO_UINT32(pos, "new-vlan-id", value & 0xFFF)
                }
            }
            if (value1)
            {
                DIAG_INFO_CHAR(pos, "cvlan-edit", vlan_action[value1])
                if ((DIAG_VLAN_ACTION_MODIFY == value1) || (DIAG_VLAN_ACTION_ADD== value1))
                {
                    value1 = GetDbgEpePayLoadInfo(V, l2NewCvlanTag_f, &payload_info);
                    DIAG_INFO_UINT32(pos, "new-cos", (value1 >> 13) & 0x7)
                    DIAG_INFO_UINT32(pos, "new-cfi", (value1 >> 12) & 0x1)
                    DIAG_INFO_UINT32(pos, "new-vlan-id", value1 & 0xFFF)
                }
            }
        }
    }
    if ((7 == GetDbgEpeNextHopInfo(V, payloadOperation_f, &nexthop_info))/*PAYLOADOPERATION_ROUTE_COMPACT = 7*/
        ||((0 == GetDbgEpeNextHopInfo(V, payloadOperation_f, &nexthop_info)) /*PAYLOADOPERATION_NONE = 0*/
            && GetDbgEpeNextHopInfo(V, routeNoL2Edit_f, &nexthop_info)))
    {
        uint32 dest_map = 0;
        cmd = DRV_IOR( DbgFwdMetFifoInfo1_t,  DbgFwdMetFifoInfo1_destMap_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &dest_map));
        if (!IS_BIT_SET(dest_map,18))
        {
            GetDsNextHop4W(A, macDa_f, &nexthop_4w, hw_mac);
            DIAG_INFO_CHAR(pos, "mac-edit", "ENABLE")
            DIAG_INFO_MAC(pos, "new-macda", hw_mac)
        }
    }
    /*NHPSHARETYPE_L2EDIT_PLUS_L3EDIT_OP=0*/
    if ((0 == nhpShareType) || (nhp_nhpIs8w))
    {
        /*inner edit*/
        value = GetDbgEpeNextHopInfo(V, nhpInnerEditPtr_f, &nexthop_info);
        if (edit_valid
            && GetDbgEpeEgressEditInfo(V, innerEditExist_f, &egress_edit_info)
            && !GetDbgEpeEgressEditInfo(V, innerEditPtrType_f, &egress_edit_info))/* inner for l2edit */
        {
            if (GetDbgEpeInnerL2EditInfo(V, valid_f, &inner_l2edit_info))
            {
                DIAG_INFO_CHAR(pos, "inner-mac-edit", "ENABLE")
                DIAG_INFO_UINT32(pos, "edit-table-index", value)
                if (GetDbgEpeInnerL2EditInfo(V, isInnerL2EthSwapOp_f, &inner_l2edit_info))
                {
                    DIAG_INFO_CHAR(pos, "edit-table-name", "DsL2EditInnerSwap")
                    cmd = DRV_IOR(DsL2EditInnerSwap_t, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, value, cmd, &l2edit_inner_swap));
                    if (GetDsL2EditInnerSwap(V, replaceInnerMacDa_f, &l2edit_inner_swap))
                    {
                        GetDsL2EditInnerSwap(A, macDa_f, &l2edit_inner_swap, hw_mac);
                        DIAG_INFO_MAC(pos, "new-macda", hw_mac)
                    }
                }
                else if (GetDbgEpeInnerL2EditInfo(V, isInnerL2EthAddOp_f, &inner_l2edit_info))
                {
                    /*L2REWRITETYPE_ETH8W=3*/
                    if (3 == GetDbgEpeEgressEditInfo(V, innerL2RewriteType_f, &egress_edit_info))
                    {
                        DIAG_INFO_CHAR(pos, "edit-table-name", "DsL2EditEth6W")
                    }
                    /*L2REWRITETYPE_ETH4W = 2*/
                    else if (2 == GetDbgEpeEgressEditInfo(V, innerL2RewriteType_f, &egress_edit_info))
                    {
                        DIAG_INFO_CHAR(pos, "edit-table-name", "DsL2EditEth3W")
                    }
                }
                else if (GetDbgEpeInnerL2EditInfo(V, isInnerDsLiteOp_f, &inner_l2edit_info))
                {
                    DIAG_INFO_CHAR(pos, "edit-table-name", "DsL2EditDsLite")
                }
            }
            else if (l2edit_valid)
            {
                do_l2_edit = 1;
                l2_edit_ptr = value;
                l2_individual_memory = 0;
            }
        }
    }
    if (do_l2_edit)
    {
        if (l2_individual_memory)/* individual memory */
        {
            cmd = DRV_IOR(DsL2Edit6WOuter_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, (l2_edit_ptr >> 1), cmd, &l2edit_6w_outer));
            /*L2REWRITETYPE_ETH4W =2*/
            if (2 == l2edit_l2RewriteType)
            {
                if (l2_edit_ptr % 2 == 0)
                {
                    sal_memcpy((uint8*)&l2edit_eth3w, (uint8*)&l2edit_6w_outer, sizeof(l2edit_eth3w));
                }
                else
                {
                    sal_memcpy((uint8*)&l2edit_eth3w, (uint8*)&l2edit_6w_outer + sizeof(l2edit_eth3w), sizeof(l2edit_eth3w));
                }
            }
            /*L2REWRITETYPE_MAC_SWAP=4*/
            else if (4 == l2edit_l2RewriteType)
            {
                sal_memcpy((uint8*)&l2edit_swap, (uint8*)&l2edit_6w_outer, sizeof(l2edit_swap));
            }
        }
        else/* share memory */
        {
            /*L2REWRITETYPE_ETH4W=2*/
            if (2 == l2edit_l2RewriteType)
            {
                cmd = DRV_IOR(DsL2EditEth3W_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, l2_edit_ptr, cmd, &l2edit_eth3w));

            }
            /*L2REWRITETYPE_MAC_SWAP=4*/
            else if (4 == l2edit_l2RewriteType)
            {
                cmd = DRV_IOR(DsL2EditSwap_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, l2_edit_ptr, cmd, &l2edit_swap));
            }
        }
        /*L2REWRITETYPE_ETH4W=2*/
        if (2 == l2edit_l2RewriteType)
        {
            GetDsL2EditEth3W(A, macDa_f, &l2edit_eth3w, hw_mac);
            DIAG_INFO_CHAR(pos, "mac-edit", "ENABLE")
            DIAG_INFO_UINT32(pos, "edit-table-index", l2_edit_ptr)
            DIAG_INFO_CHAR(pos, "edit-table-name", l2_individual_memory ? "DsL2Edit3WOuter" : "DsL2EditEth3W")
            DIAG_INFO_MAC(pos, "new-macda", hw_mac)
            //no_l2_edit = 0;
        }
        /*L2REWRITETYPE_MAC_SWAP=4*/
        else if (4 == l2edit_l2RewriteType)
        {
            uint32 deriveMcastMac = 0;
            uint32 outputVlanIdValid = 0;
            deriveMcastMac = GetDsL2EditSwap(V, deriveMcastMac_f, &l2edit_swap);
            outputVlanIdValid = GetDsL2EditSwap(V, outputVlanIdValid_f, &l2edit_swap);

            DIAG_INFO_CHAR(pos, "mac-swap", "ENABLE")
            DIAG_INFO_UINT32(pos, "edit-table-index", l2_edit_ptr)
            DIAG_INFO_CHAR(pos, "edit-table-name", l2_individual_memory ? "DsL2Edit3WOuter" : "DsL2EditSwap")
            if (deriveMcastMac)
            {
                GetDsL2EditSwap(A, macDa_f, &l2edit_eth3w, hw_mac);
                DIAG_INFO_MAC(pos, "new-macda", hw_mac)
            }
            if (outputVlanIdValid)
            {
                GetDsL2EditSwap(A, macSa_f, &l2edit_eth3w, hw_mac);
                DIAG_INFO_MAC(pos, "new-macsa", hw_mac)
            }
        }
        /*L2REWRITETYPE_NONE=0*/
        else if (0 != l2edit_l2RewriteType)
        {
            DIAG_INFO_CHAR(pos, "l2edit", "ENABLE")
            DIAG_INFO_UINT32(pos, "l2edit-type", l2edit_l2RewriteType)
            DIAG_INFO_UINT32(pos, "l2edit-index", l2_edit_ptr)
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_diag_pkt_trace_get_epe_hdr_edit_info(uint8 lchip, ctc_diag_pkt_trace_result_t* p_rslt)
{
    uint32 cmd = 0;
    uint32 value = 0;
    uint8 pos = CTC_DIAG_TRACE_POS_EPE;
    DbgEpeHdrEditInfo_m edit_info;
    char* str_type[] = {"IPv4", "IPv6", "MPLS", "TRILL"};

    sal_memset(&edit_info, 0, sizeof(DbgEpeHdrEditInfo_m));
    cmd = DRV_IOR(DbgEpeHdrEditInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &edit_info));
    if (!GetDbgEpeHdrEditInfo(V, valid_f, &edit_info))
    {
        return CTC_E_NONE;
    }
    p_rslt->position = CTC_DIAG_TRACE_POS_EPE;
    DIAG_INFO_NONE(pos, "EPE Header Editing")

    value = GetDbgEpeHdrEditInfo(V, cfHeaderLen_f, &edit_info);
    if (value)
    {
        DIAG_INFO_UINT32(pos, "stacking-header-length", value)
    }
    value = GetDbgEpeHdrEditInfo(V, l2NewHeaderLen_f, &edit_info);
    if (value)
    {
        DIAG_INFO_UINT32(pos, "new-l2-header-length", value)
    }
    value = GetDbgEpeHdrEditInfo(V, l2NewHeaderOuterLen_f, &edit_info);
    if (value)
    {
        DIAG_INFO_UINT32(pos, "new-l2-outer-header-length", value)
    }
    value = GetDbgEpeHdrEditInfo(V, l3NewHeaderLen_f, &edit_info);
    if (value)
    {
        DIAG_INFO_UINT32(pos, "new-l3-header-length", value)
    }
    value = GetDbgEpeHdrEditInfo(V, l3NewHeaderOuterLen_f, &edit_info);
    if (value)
    {
        DIAG_INFO_UINT32(pos, "new-l3-outer-header-length", value)
    }
    if (GetDbgEpeHdrEditInfo(V, e2iLoopEn_f, &edit_info))
    {
        DIAG_INFO_CHAR(pos, "e2iloop", "ENABLE")
    }
    if (GetDbgEpeHdrEditInfo(V, flexEditEn_f, &edit_info))
    {
        DIAG_INFO_CHAR(pos, "flex-edit", "ENABLE")
    }
    if (GetDbgEpeHdrEditInfo(V, loopbackEn_f, &edit_info))
    {
        DIAG_INFO_CHAR(pos, "loopback", "ENABLE")
    }
    if (GetDbgEpeHdrEditInfo(V, isSpanPkt_f, &edit_info))
    {
        DIAG_INFO_CHAR(pos, "is-span-packet", "TRUE")
    }
    if (GetDbgEpeHdrEditInfo(V, newTtlValid_f, &edit_info))
    {
        DIAG_INFO_CHAR(pos, "new-ttl-packet-type", str_type[GetDbgEpeHdrEditInfo(V, newTtlPacketType_f, &edit_info)])
    }

    /* Drop Process */
    _sys_tsingma_diag_pkt_tracce_drop_map(p_rslt,
                                          GetDbgEpeHdrEditInfo(V, discard_f, &edit_info),
                                          GetDbgEpeHdrEditInfo(V, discardType_f,  &edit_info));
    /* Exception Process */
    _sys_tsingma_diag_pkt_tracce_exception_map(p_rslt,
                                               GetDbgEpeHdrEditInfo(V, exceptionEn_f, &edit_info),
                                               GetDbgEpeHdrEditInfo(V, exceptionIndex_f, &edit_info),
                                               GetDbgEpeHdrEditInfo(V, exceptionSubIndex_f, &edit_info));
    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_diag_pkt_trace_get_tm_bs_info(uint8 lchip, ctc_diag_pkt_trace_result_t* p_rslt)
{
    uint32 cmd = 0;
    uint32 value = 0;
    uint8 pos = CTC_DIAG_TRACE_POS_TM;
    char* str[] = {"UCAST-H", "UCAST-L", "MCAST-H", "MCAST-L"};
    DbgFwdBufStoreInfo_m buf_store_info;

    sal_memset(&buf_store_info, 0, sizeof(buf_store_info));
    cmd = DRV_IOR(DbgFwdBufStoreInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &buf_store_info));

    if (!GetDbgFwdBufStoreInfo(V, valid_f, &buf_store_info))
    {
        return CTC_E_NONE;
    }
    p_rslt->position = CTC_DIAG_TRACE_POS_TM;
    DIAG_INFO_NONE(pos, "BufferStore")
    value = GetDbgFwdBufStoreInfo(V, metFifoSelect_f, &buf_store_info);
    DIAG_INFO_CHAR(pos, "met-fifo-priority", str[value])
    value = GetDbgFwdBufStoreInfo(V, exceptionVector_f, &buf_store_info);
    if (value & (1 << CTC_DIAG_EXCP_VEC_EXCEPTION))
    {
        DIAG_INFO_UINT32(pos, "exception-vector", value)
        p_rslt->dest_type = CTC_DIAG_DEST_CPU;
        p_rslt->excp_vector = value;
        p_rslt->flags |= CTC_DIAG_TRACE_RSTL_FLAG_EXCP_VECTOR;
    }
    value = GetDbgFwdBufStoreInfo(V, destChannelIdValid_f, &buf_store_info);
    if (value)
    {
        DIAG_INFO_UINT32(pos, "dest-channel", GetDbgFwdBufStoreInfo(V, destChannelId_f, &buf_store_info))
    }
    if (GetDbgFwdBufStoreInfo(V, backPressureDiscard_f, &buf_store_info))
    {
        DIAG_INFO_CHAR(pos, "drop-desc", "back-pressure-discard")
    }
    if (GetDbgFwdBufStoreInfo(V, destIdDiscard_f, &buf_store_info))
    {
        DIAG_INFO_CHAR(pos, "drop-desc", "dest-id-discard")
    }
    if (GetDbgFwdBufStoreInfo(V, fwdCheckDiscard_f, &buf_store_info))
    {
        DIAG_INFO_CHAR(pos, "drop-desc", "forward-check-fail")
    }
    if (GetDbgFwdBufStoreInfo(V, resourceCheckPass_f, &buf_store_info))
    {
        DIAG_INFO_CHAR(pos, "drop-desc", "resource-check-fail")
    }
    if (GetDbgFwdBufStoreInfo(V, noFifoDiscard_f, &buf_store_info))
    {
        DIAG_INFO_CHAR(pos, "drop-desc", "no-fifo")
    }

    return CTC_E_NONE;
}
STATIC int32
_sys_tsingma_diag_pkt_trace_get_tm_met_info(uint8 lchip, ctc_diag_pkt_trace_result_t* p_rslt)
{
    uint32 cmd = 0;
    uint32 count = 0;
    uint32 value = 0;
    uint8 pos = CTC_DIAG_TRACE_POS_TM;
    char str[64] = {0};
    uint32 ucastId = 0;
    uint32 local_chip = 0;
    uint32 dsMetEntryBase = 0;
    uint32 groupIdShiftBit = 0;
    uint32 nextMetEntryPtr = 0;
    uint32 metEntryPtr = 0;
    uint32 nexthopPtr = 0;
    uint32 currentMetEntryType = 0;
    uint32 port_bitmap_high[2] = {0};
    uint32 port_bitmap_low[2] = {0};
    uint32 dest_map = 0;
    uint64 value_u64 = 0;
    DsMetEntry6W_m ds_met_entry6_w;
    DsMetEntry6W_m ds_met_entry;
    DbgFwdMetFifoInfo1_m met_fifo_info1;
    DbgFwdMetFifoInfo2_m met_fifo_info2;

    sal_memset(&ds_met_entry6_w, 0, sizeof(ds_met_entry6_w));
    sal_memset(&ds_met_entry, 0, sizeof(ds_met_entry));

    sal_memset(&met_fifo_info1, 0, sizeof(met_fifo_info1));
    cmd = DRV_IOR(DbgFwdMetFifoInfo1_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &met_fifo_info1));
    sal_memset(&met_fifo_info2, 0, sizeof(met_fifo_info2));
    cmd = DRV_IOR(DbgFwdMetFifoInfo2_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &met_fifo_info2));

    if (!GetDbgFwdMetFifoInfo1(V, valid_f, &met_fifo_info1))
    {
        return CTC_E_NONE;
    }
    p_rslt->position = CTC_DIAG_TRACE_POS_TM;
    DIAG_INFO_NONE(pos, "MetFifo")

    dest_map = GetDbgFwdMetFifoInfo1(V, destMap_f, &met_fifo_info1);
    DIAG_INFO_U32_HEX(pos, "dest-map", dest_map)
    if (!GetDbgFwdMetFifoInfo1(V, execMet_f, &met_fifo_info1))
    {
        DIAG_INFO_CHAR(pos, "forward-type", "UCAST")
        return CTC_E_NONE;
    }
    DIAG_INFO_CHAR(pos, "forward-type", "MCAST")
    DIAG_INFO_UINT32(pos, "mcast-group", dest_map & 0xFFFF)

    cmd = DRV_IOR(MetFifoCtl_t, MetFifoCtl_chipId_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &local_chip));
    cmd = DRV_IOR(MetFifoCtl_t, MetFifoCtl_dsMetEntryBase_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &dsMetEntryBase));
    cmd = DRV_IOR(MetFifoCtl_t, MetFifoCtl_groupIdShiftBit_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &groupIdShiftBit));

    nextMetEntryPtr = (dest_map&0xFFFF) << groupIdShiftBit;
    for (;;)
    {
        metEntryPtr =  nextMetEntryPtr + dsMetEntryBase;
        cmd = DRV_IOR(DsMetEntry6W_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, (metEntryPtr&0x1FFFE), cmd, &ds_met_entry6_w));

        currentMetEntryType = GetDsMetEntry6W(V, currentMetEntryType_f, &ds_met_entry6_w);
        sal_sprintf(str, "met-entry-%d", count++);
        DIAG_INFO_CHAR(pos, str, "HIT")
        DIAG_INFO_CHAR(pos, "table-name", currentMetEntryType ? "DsMetEntry6W" : "DsMetEntry3W")
        DIAG_INFO_UINT32(pos, "table-index", metEntryPtr)
        if (currentMetEntryType)
        {
            sal_memcpy((uint8*)&ds_met_entry, (uint8*)&ds_met_entry6_w, sizeof(ds_met_entry6_w));
        }
        else
        {
            if (IS_BIT_SET(metEntryPtr, 0))
            {
                sal_memcpy((uint8*)&ds_met_entry, ((uint8 *)&ds_met_entry6_w) + sizeof(DsMetEntry3W_m), sizeof(DsMetEntry3W_m));
            }
            else
            {
                sal_memcpy((uint8*)&ds_met_entry, ((uint8 *)&ds_met_entry6_w), sizeof(DsMetEntry3W_m));
            }
            sal_memset(((uint8*)&ds_met_entry) + sizeof(DsMetEntry3W_m), 0, sizeof(DsMetEntry3W_m));
        }

        nextMetEntryPtr = GetDsMetEntry(V, nextMetEntryPtr_f, &ds_met_entry);
        ucastId = GetDsMetEntry(V, ucastId_f, &ds_met_entry);

        if (currentMetEntryType)
        {
            nexthopPtr = GetDsMetEntry(V, nextHopPtr_f, &ds_met_entry);
        }
        else
        {
            cmd = DRV_IOR(MetFifoCtl_t, MetFifoCtl_portBitmapNextHopPtr_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &nexthopPtr));
        }

        if(GetDsMetEntry(V, remoteChip_f, &ds_met_entry))
        {
            DIAG_INFO_CHAR(pos, "to-remote-chip", "YES")
        }
        else
        {
            if (!GetDsMetEntry(V, mcastMode_f, &ds_met_entry) && GetDsMetEntry(V, apsBridgeEn_f, &ds_met_entry))
            {
                DIAG_INFO_CHAR(pos, "aps-bridge", "YES")
                DIAG_INFO_UINT32(pos, "aps-group", ucastId)
            }
            if (!GetDsMetEntry(V, mcastMode_f, &ds_met_entry))
            {
                DIAG_INFO_CHAR(pos, "met-mode", "link-list")
                if (GetDsMetEntry(V, isLinkAggregation_f, &ds_met_entry))
                {
                    DIAG_INFO_UINT32(pos, "linkagg-group", ucastId&CTC_LINKAGGID_MASK)
                }
                else
                {
                    if (!GetDsMetEntry(V, apsBridgeEn_f, &ds_met_entry))
                    {
                        DIAG_INFO_U32_HEX(pos, "dest-gport", CTC_MAP_LPORT_TO_GPORT(local_chip, ucastId&0x1FF))
                    }
                }
                value = GetDsMetEntry(V, replicationCtl_f, &ds_met_entry);
                DIAG_INFO_UINT32(pos, "replication-count", value&0x1F)
                DIAG_INFO_U32_HEX(pos, "nexthop-ptr", nexthopPtr)
            }
            else
            {
                GetDsMetEntry(A, portBitmapHigh_f, &ds_met_entry, port_bitmap_high);
                GetDsMetEntry(A, portBitmap_f, &ds_met_entry, port_bitmap_low);
                DIAG_INFO_CHAR(pos, "met-mode", "bitmap")
                if (GetDsMetEntry(V, isLinkAggregation_f, &ds_met_entry))
                {
                    DIAG_INFO_CHAR(pos, "bitmap-type", "linkagg-port")
                }
                else
                {
                    DIAG_INFO_CHAR(pos, "bitmap-type", "normal-gport")
                }
                DIAG_INFO_UINT32(pos, "port-bitmap-base", GetDsMetEntry(V, portBitmapBase_f, &ds_met_entry) << 6)
                value_u64 = port_bitmap_low[1];
                value_u64 = value_u64 << 32 | port_bitmap_low[0];
                DIAG_INFO_U64_HEX(pos, "dest-port-bitmap63~0", value_u64)
                if (currentMetEntryType)
                {
                    value_u64 = port_bitmap_high[1];
                    value_u64 = value_u64 << 32 | port_bitmap_high[0];
                    DIAG_INFO_U64_HEX(pos, "dest-port-bitmap127~64", value_u64)
                }
                DIAG_INFO_U32_HEX(pos, "nexthop-ptr", nexthopPtr)
            }
        }
        if (0xFFFF == nextMetEntryPtr
            || GetDsMetEntry(V, endLocalRep_f, &ds_met_entry))
        {
            break;
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_diag_pkt_trace_get_tm_queue_info(uint8 lchip, ctc_diag_pkt_trace_result_t* p_rslt)
{
    uint32 cmd = 0;
    uint32 channel = 0;
    uint8 pos = CTC_DIAG_TRACE_POS_TM;
    DbgFwdQWriteInfo_m qwrite_info;
    DbgErmResrcAllocInfo_m  rsc_alloc_info;

    sal_memset(&qwrite_info, 0, sizeof(DbgFwdQWriteInfo_m));
    cmd = DRV_IOR(DbgFwdQWriteInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &qwrite_info));
    sal_memset(&rsc_alloc_info, 0, sizeof(DbgErmResrcAllocInfo_m));
    cmd = DRV_IOR(DbgErmResrcAllocInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rsc_alloc_info));


    if (!GetDbgFwdQWriteInfo(V, valid_f, &qwrite_info))
    {
        return CTC_E_NONE;
    }
    DIAG_INFO_NONE(pos, "Queue")
    p_rslt->position = CTC_DIAG_TRACE_POS_TM;
    if (GetDbgFwdQWriteInfo(V, channelIdValid_f, &qwrite_info))
    {
        channel = GetDbgFwdQWriteInfo(V, channelId_f, &qwrite_info);
        DIAG_INFO_UINT32(pos, "channel-id", channel)
        if (!CTC_FLAG_ISSET(p_rslt->flags, CTC_DIAG_TRACE_RSTL_FLAG_MC_GROUP))
        {
            p_rslt->channel = channel;
            p_rslt->flags |= CTC_DIAG_TRACE_RSTL_FLAG_CHANNLE;
        }
        if (channel == MCHIP_CAP(SYS_CAP_CHANID_ILOOP))
        {
            p_rslt->loop_flags |= CTC_DIAG_LOOP_FLAG_ILOOP;
        }
        if (channel == MCHIP_CAP(SYS_CAP_CHANID_ELOOP))
        {
            p_rslt->loop_flags |= CTC_DIAG_LOOP_FLAG_ELOOP;
        }
    }
    if (GetDbgFwdQWriteInfo(V, noLinkAggregationMemberDiscard_f, &qwrite_info))
    {
        DIAG_INFO_CHAR(pos, "drop-desc", "port-linkagg-no-member")
    }
    else if (GetDbgFwdQWriteInfo(V, mcastLinkAggregationDiscard_f, &qwrite_info))
    {
        DIAG_INFO_CHAR(pos, "drop-desc", "mcast-linkagg-no-member")
    }
    else if (GetDbgFwdQWriteInfo(V, stackingDiscard_f, &qwrite_info))
    {
        DIAG_INFO_CHAR(pos, "drop-desc", "stacking-discard")
    }
    else
    {
        DIAG_INFO_UINT32(pos, "queue-id", GetDbgFwdQWriteInfo(V, queueId_f, &qwrite_info))
    }
    if (GetDbgFwdQWriteInfo(V, tcamResultValid_f, &qwrite_info))
    {
        DIAG_INFO_UINT32(pos, "tcam-hit-index", GetDbgFwdQWriteInfo(V, hitIndex_f, &qwrite_info))
    }
    if (GetDbgFwdQWriteInfo(V, cutThroughEn_f, &qwrite_info))
    {
        DIAG_INFO_CHAR(pos, "cut-through", "ENABLE")
    }

    /*lag process*/
    CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_get_lag_info(lchip, p_rslt));

    if (!GetDbgErmResrcAllocInfo(V, valid_f, &rsc_alloc_info))
    {
        return CTC_E_NONE;
    }
    if (!GetDbgErmResrcAllocInfo(V, resourceCheckPass_f, &rsc_alloc_info))
    {
        DIAG_INFO_CHAR(pos, "drop-desc", "enqueue-discard")
    }
    DIAG_INFO_UINT32(pos, "mapped-sc", GetDbgErmResrcAllocInfo(V, mappedSc_f, &rsc_alloc_info))
    DIAG_INFO_UINT32(pos, "mapped-tc", GetDbgErmResrcAllocInfo(V, mappedTc_f, &rsc_alloc_info))
    if (GetDbgErmResrcAllocInfo(V, isC2cPacket_f, &rsc_alloc_info))
    {
        DIAG_INFO_CHAR(pos, "is-c2c-packet", "TRUE")
    }
    if (GetDbgErmResrcAllocInfo(V, isCriticalPacket_f, &rsc_alloc_info))
    {
        DIAG_INFO_CHAR(pos, "is-critical-packet", "TRUE")
    }


    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_diag_pkt_trace_get_tm_bufrtv_info(uint8 lchip, ctc_diag_pkt_trace_result_t* p_rslt)
{
    uint32 cmd = 0;
    uint32 value = 0;
    uint8 pos = CTC_DIAG_TRACE_POS_TM;
    uint32 hdr_data[40] = {0};
    char* str_color[] = {"None", "RED", "YELLOW", "GREEN"};
    DbgFwdBufRetrvInfo_m buf_rtv_info;
    DbgFwdBufRetrvHeaderInfo_m hdr_info;

    sal_memset(&hdr_info, 0, sizeof(hdr_info));
    cmd = DRV_IOR(DbgFwdBufRetrvHeaderInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hdr_info));
    sal_memset(&buf_rtv_info, 0, sizeof(DbgFwdBufRetrvInfo_m));
    cmd = DRV_IOR(DbgFwdBufRetrvInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &buf_rtv_info));

    if (!GetDbgFwdBufRetrvInfo(V, valid_f, &buf_rtv_info))
    {
        return CTC_E_NONE;
    }
    p_rslt->position = CTC_DIAG_TRACE_POS_TM;
    DIAG_INFO_NONE(pos, "BufRetrv")

    DIAG_INFO_UINT32(pos, "queue-id", GetDbgFwdBufRetrvInfo(V, queId_f, &buf_rtv_info))
    if (GetDbgFwdBufRetrvInfo(V, destIdDiscard_f, &buf_rtv_info))
    {
        DIAG_INFO_CHAR(pos, "drop-desc", "dest-id-discard")
    }
    value = GetDbgFwdBufRetrvInfo(V, reColor_f, &buf_rtv_info);
    if (value)
    {
        DIAG_INFO_CHAR(pos, "mapped-color", str_color[value])
    }
    if (GetDbgFwdBufRetrvInfo(V, cutThroughEn_f, &buf_rtv_info))
    {
        DIAG_INFO_CHAR(pos, "cut-through", "ENABLE")
    }
    GetDbgFwdBufRetrvHeaderInfo(A, packetHeader_f, &hdr_info, hdr_data);
    DIAG_INFO_PKT_HDR(pos, "packet-header", hdr_data)

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_diag_get_pkt_trace_pos_ipe(uint8 lchip, ctc_diag_pkt_trace_result_t* p_rslt)
{
    if (p_rslt->watch_point != CTC_DIAG_TRACE_POINT_IPE_SCL)
    {
        return CTC_E_NONE;
    }
    CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_get_parser_info(lchip, 0, p_rslt));

    if (p_rslt->dest_type != CTC_DIAG_DEST_DROP)
    {
        CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_get_ipe_scl_info(lchip, p_rslt));
    }
    if (p_rslt->dest_type != CTC_DIAG_DEST_DROP)
    {
        CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_get_ipe_intf_info(lchip, p_rslt));
    }
    if (p_rslt->dest_type != CTC_DIAG_DEST_DROP)
    {
        CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_get_ipe_decap_info(lchip, p_rslt));
    }
    if (p_rslt->dest_type != CTC_DIAG_DEST_DROP)
    {
        CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_get_parser_info(lchip, 1, p_rslt));
    }
    if (p_rslt->dest_type != CTC_DIAG_DEST_DROP)
    {
        CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_get_lkup_mgr_info(lchip, p_rslt));
    }
    if (p_rslt->dest_type != CTC_DIAG_DEST_DROP)
    {
        CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_get_flow_hash_info(lchip, p_rslt));
    }
    if (p_rslt->dest_type != CTC_DIAG_DEST_DROP)
    {
        CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_get_ipe_bridge_info(lchip, p_rslt));
    }
    if (p_rslt->dest_type != CTC_DIAG_DEST_DROP)
    {
        CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_get_ipe_route_info(lchip, p_rslt));
    }
    if (p_rslt->dest_type != CTC_DIAG_DEST_DROP)
    {
        CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_get_ipe_fcoe_info(lchip, p_rslt));
    }
    if (p_rslt->dest_type != CTC_DIAG_DEST_DROP)
    {
        CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_get_ipe_trill_info(lchip, p_rslt));
    }
    if (p_rslt->dest_type != CTC_DIAG_DEST_DROP)
    {
        CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_get_ipe_phb_info(lchip, p_rslt));
    }
        if (p_rslt->dest_type != CTC_DIAG_DEST_DROP)
    {
        CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_get_ipe_oam_info(lchip, p_rslt));
    }
    if (p_rslt->dest_type != CTC_DIAG_DEST_DROP)
    {
        CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_get_ipe_acl_info(lchip, p_rslt));
    }
    if (p_rslt->dest_type != CTC_DIAG_DEST_DROP)
    {
        CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_get_ipe_stmctl_info(lchip, p_rslt));
    }
    if (p_rslt->dest_type != CTC_DIAG_DEST_DROP)
    {
        CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_get_ipe_policer_info(lchip, p_rslt));
    }
    if (p_rslt->dest_type != CTC_DIAG_DEST_DROP)
    {
        CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_get_ipe_copp_info(lchip, p_rslt));
    }
    if (p_rslt->dest_type != CTC_DIAG_DEST_DROP)
    {
        CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_get_ipe_learn_info(lchip, p_rslt));
    }
    if (p_rslt->dest_type != CTC_DIAG_DEST_DROP)
    {
        CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_get_ipe_dest_info(lchip, p_rslt));
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_diag_get_pkt_trace_pos_tm(uint8 lchip, ctc_diag_pkt_trace_result_t* p_rslt)
{
    if (p_rslt->dest_type == CTC_DIAG_DEST_DROP)
    {
        return CTC_E_NONE;
    }
    if ((p_rslt->watch_point == CTC_DIAG_TRACE_POINT_IPE_SCL)
        || (p_rslt->watch_point == CTC_DIAG_TRACE_POINT_TM_BS)
        || (p_rslt->watch_point == CTC_DIAG_TRACE_POINT_OAM_TX))
    {
        CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_get_tm_bs_info(lchip, p_rslt));
    }
    if ((p_rslt->watch_point == CTC_DIAG_TRACE_POINT_IPE_SCL)
        || (p_rslt->watch_point == CTC_DIAG_TRACE_POINT_TM_BS)
        || (p_rslt->watch_point == CTC_DIAG_TRACE_POINT_TM_MET)
        || (p_rslt->watch_point == CTC_DIAG_TRACE_POINT_OAM_TX))
    {
        CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_get_tm_met_info(lchip, p_rslt));
        if (0 == (p_rslt->flags & CTC_DIAG_TRACE_RSTL_FLAG_MC_GROUP))
        {
            CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_get_tm_queue_info(lchip, p_rslt));
            CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_get_tm_bufrtv_info(lchip, p_rslt));
        }
    }
    return CTC_E_NONE;
}


STATIC int32
_sys_tsingma_diag_get_pkt_trace_pos_epe(uint8 lchip, ctc_diag_pkt_trace_result_t* p_rslt)
{
    if (p_rslt->watch_point == CTC_DIAG_TRACE_POINT_OAM_ADJ)
    {
        return CTC_E_NONE;
    }
    CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_get_epe_hdj_info(lchip, p_rslt));

    if (p_rslt->dest_type != CTC_DIAG_DEST_DROP)
    {
        CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_get_parser_info(lchip, 2, p_rslt));
    }
    if (p_rslt->dest_type != CTC_DIAG_DEST_DROP)
    {
        CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_get_epe_nh_map_info(lchip, p_rslt));
    }
    if (p_rslt->dest_type != CTC_DIAG_DEST_DROP)
    {
        CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_get_epe_scl_info(lchip, p_rslt));
    }
    if (p_rslt->dest_type != CTC_DIAG_DEST_DROP)
    {
        CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_get_epe_edit_info(lchip, p_rslt));
    }
    if (p_rslt->dest_type != CTC_DIAG_DEST_DROP)
    {
        CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_get_epe_acl_info(lchip, p_rslt));
    }
    if (p_rslt->dest_type != CTC_DIAG_DEST_DROP)
    {
        CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_get_epe_oam_info(lchip, p_rslt));
    }
    if (p_rslt->dest_type != CTC_DIAG_DEST_DROP)
    {
        CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_get_epe_hdr_edit_info(lchip, p_rslt));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_diag_get_pkt_trace_pos_oam(uint8 lchip, ctc_diag_pkt_trace_result_t* p_rslt)
{
    if ((p_rslt->watch_point == CTC_DIAG_TRACE_POINT_IPE_SCL)
        || (p_rslt->watch_point == CTC_DIAG_TRACE_POINT_TM_BS)
        || (p_rslt->watch_point == CTC_DIAG_TRACE_POINT_TM_MET)
        || (p_rslt->watch_point == CTC_DIAG_TRACE_POINT_OAM_ADJ))
    {
        CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_get_oam_rx_info(lchip, p_rslt));
    }
    if (p_rslt->watch_point == CTC_DIAG_TRACE_POINT_OAM_TX)
    {
        CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_get_oam_tx_info(lchip, p_rslt));
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_diag_pkt_trace_clear_rslt(uint8 lchip, uint32 watch_point)
{
    uint32 loop = 0;
    uint32 loop_size = 0;
    uint32 cmd = 0;
    uint32 temp_v = 0;
    IpeUserIdFlowLoopCam_m ds_flow_loop_cam;
    BufStoreDebugCam_m ds_buff_store_cam;
    MetFifoDebugCam_m ds_met_fifo_cam;
    EpeHdrAdjustDebugCam_m ds_epe_dbg_cam;

    sys_diag_pkt_trace_table_t ipe_tbl[] =
    {
        {DbgFibLkpEngineFlowHashInfo_t, DbgFibLkpEngineFlowHashInfo_valid_f},
        {DbgFibLkpEngineHostUrpfHashInfo_t, DbgFibLkpEngineHostUrpfHashInfo_valid_f},
        {DbgLpmPipelineLookupResultInfo_t, DbgLpmPipelineLookupResultInfo_valid_f},
        {DbgFlowTcamEngineUserIdKeyInfo2_t, DbgFlowTcamEngineUserIdKeyInfo2_valid_f},
        {DbgIpeFwdInputHeaderInfo_t, DbgIpeFwdInputHeaderInfo_valid_f},
        {DbgFibLkpEngineMacDaHashInfo_t, DbgFibLkpEngineMacDaHashInfo_valid_f},
        {DbgFibLkpEngineMacSaHashInfo_t, DbgFibLkpEngineMacSaHashInfo_valid_f},
        {DbgFibLkpEnginel3DaHashInfo_t, DbgFibLkpEnginel3DaHashInfo_valid_f},
        {DbgFibLkpEnginel3SaHashInfo_t, DbgFibLkpEnginel3SaHashInfo_valid_f},
        {DbgIpfixAccEgrInfo_t, DbgIpfixAccEgrInfo_gEgr_valid_f},
        {DbgIpfixAccIngInfo_t, DbgIpfixAccIngInfo_gIngr_valid_f},
        {DbgIpfixAccInfo0_t, DbgIpfixAccInfo0_valid_f},
        {DbgIpfixAccInfo1_t, DbgIpfixAccInfo1_valid_f},
        {DbgFlowTcamEngineEpeAclInfo0_t, DbgFlowTcamEngineEpeAclInfo0_valid_f},
        {DbgFlowTcamEngineEpeAclInfo1_t, DbgFlowTcamEngineEpeAclInfo1_valid_f},
        {DbgFlowTcamEngineEpeAclInfo2_t, DbgFlowTcamEngineEpeAclInfo2_valid_f},
        {DbgFlowTcamEngineEpeAclKeyInfo0_t, DbgFlowTcamEngineEpeAclKeyInfo0_valid_f},
        {DbgFlowTcamEngineEpeAclKeyInfo1_t, DbgFlowTcamEngineEpeAclKeyInfo1_valid_f},
        {DbgFlowTcamEngineEpeAclKeyInfo2_t, DbgFlowTcamEngineEpeAclKeyInfo2_valid_f},
        {DbgFlowTcamEngineIpeAclInfo0_t, DbgFlowTcamEngineIpeAclInfo0_valid_f},
        {DbgFlowTcamEngineIpeAclInfo1_t, DbgFlowTcamEngineIpeAclInfo1_valid_f},
        {DbgFlowTcamEngineIpeAclInfo2_t, DbgFlowTcamEngineIpeAclInfo2_valid_f},
        {DbgFlowTcamEngineIpeAclInfo3_t, DbgFlowTcamEngineIpeAclInfo3_valid_f},
        {DbgFlowTcamEngineIpeAclInfo4_t, DbgFlowTcamEngineIpeAclInfo4_valid_f},
        {DbgFlowTcamEngineIpeAclInfo5_t, DbgFlowTcamEngineIpeAclInfo5_valid_f},
        {DbgFlowTcamEngineIpeAclInfo6_t, DbgFlowTcamEngineIpeAclInfo6_valid_f},
        {DbgFlowTcamEngineIpeAclInfo7_t, DbgFlowTcamEngineIpeAclInfo7_valid_f},
        {DbgFlowTcamEngineIpeAclKeyInfo0_t, DbgFlowTcamEngineIpeAclKeyInfo0_valid_f},
        {DbgFlowTcamEngineIpeAclKeyInfo1_t, DbgFlowTcamEngineIpeAclKeyInfo1_valid_f},
        {DbgFlowTcamEngineIpeAclKeyInfo2_t, DbgFlowTcamEngineIpeAclKeyInfo2_valid_f},
        {DbgFlowTcamEngineIpeAclKeyInfo3_t, DbgFlowTcamEngineIpeAclKeyInfo3_valid_f},
        {DbgFlowTcamEngineIpeAclKeyInfo4_t, DbgFlowTcamEngineIpeAclKeyInfo4_valid_f},
        {DbgFlowTcamEngineIpeAclKeyInfo5_t, DbgFlowTcamEngineIpeAclKeyInfo5_valid_f},
        {DbgFlowTcamEngineIpeAclKeyInfo6_t, DbgFlowTcamEngineIpeAclKeyInfo6_valid_f},
        {DbgFlowTcamEngineIpeAclKeyInfo7_t, DbgFlowTcamEngineIpeAclKeyInfo7_valid_f},
        {DbgFlowTcamEngineUserIdInfo0_t, DbgFlowTcamEngineUserIdInfo0_valid_f},
        {DbgFlowTcamEngineUserIdInfo1_t, DbgFlowTcamEngineUserIdInfo1_valid_f},
        {DbgFlowTcamEngineUserIdInfo2_t, DbgFlowTcamEngineUserIdInfo2_valid_f},
        {DbgFlowTcamEngineUserIdKeyInfo0_t, DbgFlowTcamEngineUserIdKeyInfo0_valid_f},
        {DbgFlowTcamEngineUserIdKeyInfo1_t, DbgFlowTcamEngineUserIdKeyInfo1_valid_f},
        {DbgIpeAclProcInfo_t, DbgIpeAclProcInfo_valid_f},
        {DbgIpeEcmpProcInfo_t, DbgIpeEcmpProcInfo_valid_f},
        {DbgIpeFwdCoppInfo_t, DbgIpeFwdCoppInfo_valid_f},
        {DbgIpeFwdPolicingInfo_t, DbgIpeFwdPolicingInfo_valid_f},
        {DbgIpeFwdProcessInfo_t, DbgIpeFwdProcessInfo_valid_f},
        {DbgIpeAclProcInfo0_t, DbgIpeAclProcInfo0_valid_f},
        {DbgIpeAclProcInfo1_t, DbgIpeAclProcInfo1_valid_f},
        {DbgIpeFwdStormCtlInfo_t, DbgIpeFwdStormCtlInfo_valid_f},
        {DbgIpeIntfMapperInfo_t, DbgIpeIntfMapperInfo_valid_f},
        {DbgIpeMplsDecapInfo_t, DbgIpeMplsDecapInfo_valid_f},
        {DbgIpeUserIdInfo_t, DbgIpeUserIdInfo_valid_f},
        {DbgIpeLkpMgrInfo_t, DbgIpeLkpMgrInfo_valid_f},
        {DbgParserFromIpeIntfInfo_t, DbgParserFromIpeIntfInfo_valid_f},
        {DbgIpeFcoeInfo_t, DbgIpeFcoeInfo_valid_f},
        {DbgIpeFlowProcessInfo_t, DbgIpeFlowProcessInfo_valid_f},
        {DbgIpeIpRoutingInfo_t, DbgIpeIpRoutingInfo_valid_f},
        {DbgIpeMacBridgingInfo_t, DbgIpeMacBridgingInfo_valid_f},
        {DbgIpeMacLearningInfo_t, DbgIpeMacLearningInfo_valid_f},
        {DbgIpeOamInfo_t, DbgIpeOamInfo_valid_f},
        {DbgIpePacketProcessInfo_t, DbgIpePacketProcessInfo_valid_f},
        {DbgIpePerHopBehaviorInfo_t, DbgIpePerHopBehaviorInfo_valid_f},
        {DbgIpeTrillInfo_t, DbgIpeTrillInfo_valid_f},
        {DbgLagEngineInfoFromBsrEnqueue_t, DbgLagEngineInfoFromBsrEnqueue_valid_f},
        {DbgLagEngineInfoFromIpeFwd_t, DbgLagEngineInfoFromIpeFwd_valid_f},
        {DbgLpmTcamEngineResult0Info_t, DbgLpmTcamEngineResult0Info_valid_f},
        {DbgLpmTcamEngineResult1Info_t, DbgLpmTcamEngineResult1Info_valid_f},
        {DbgParserFromIpeHdrAdjInfo_t, DbgParserFromIpeHdrAdjInfo_valid_f},
        {DbgUserIdHashEngineForMpls0Info_t, DbgUserIdHashEngineForMpls0Info_valid_f},
        {DbgUserIdHashEngineForMpls1Info_t, DbgUserIdHashEngineForMpls1Info_valid_f},
        {DbgUserIdHashEngineForMpls2Info_t, DbgUserIdHashEngineForMpls2Info_valid_f},
        {DbgUserIdHashEngineForUserId0Info_t, DbgUserIdHashEngineForUserId0Info_valid_f},
        {DbgUserIdHashEngineForUserId1Info_t, DbgUserIdHashEngineForUserId1Info_valid_f},
        {DbgDlbEngineInfo_t, DbgDlbEngineInfo_valid_f},
        {DbgEfdEngineInfo_t, DbgEfdEngineInfo_valid_f},
    };

    sys_diag_pkt_trace_table_t bsr_tbl[] =
    {
        {DbgFwdBufStoreInfo_t, DbgFwdBufRetrvInfo_valid_f},
        {DbgFwdBufStoreInfo2_t, DbgFwdBufStoreInfo2_valid_f},
        {DbgIrmResrcAllocInfo_t, DbgIrmResrcAllocInfo_valid_f},
        {DbgFwdBufStoreInfo_t, DbgFwdBufStoreInfo_valid_f},
        {DbgFwdBufStoreInfo2_t, DbgFwdBufStoreInfo2_valid_f},
    };
    sys_diag_pkt_trace_table_t epe_tbl[] =
    {
        {DbgWlanDecryptEngineInfo_t, DbgWlanDecryptEngineInfo_valid_f},
        {DbgWlanEncryptAndFragmentEngineInfo_t, DbgWlanEncryptAndFragmentEngineInfo_valid_f},
        {DbgWlanReassembleEngineInfo_t, DbgWlanReassembleEngineInfo_valid_f},
        {DbgEgrXcOamHash0EngineFromEpeNhpInfo_t, DbgEgrXcOamHash0EngineFromEpeNhpInfo_valid_f},
        {DbgEgrXcOamHash1EngineFromEpeNhpInfo_t, DbgEgrXcOamHash1EngineFromEpeNhpInfo_valid_f},
        {DbgEgrXcOamHashEngineFromEpeOam0Info_t, DbgEgrXcOamHashEngineFromEpeOam0Info_valid_f},
        {DbgEgrXcOamHashEngineFromEpeOam1Info_t, DbgEgrXcOamHashEngineFromEpeOam0Info_valid_f},
        {DbgEgrXcOamHashEngineFromEpeOam2Info_t, DbgEgrXcOamHashEngineFromEpeOam2Info_valid_f},
        {DbgEgrXcOamHashEngineFromIpeOam0Info_t, DbgEgrXcOamHashEngineFromIpeOam0Info_valid_f},
        {DbgEgrXcOamHashEngineFromIpeOam1Info_t, DbgEgrXcOamHashEngineFromIpeOam1Info_valid_f},
        {DbgEgrXcOamHashEngineFromIpeOam2Info_t, DbgEgrXcOamHashEngineFromIpeOam2Info_valid_f},
        {DbgEpeAclInfo_t, DbgEpeAclInfo_valid_f},
        {DbgEpeClassificationInfo_t, DbgEpeClassificationInfo_valid_f},
        {DbgEpeOamInfo_t, DbgEpeOamInfo_valid_f},
        {DbgEpeHdrAdjInfo_t, DbgEpeHdrAdjInfo_valid_f},
        {DbgEpeHdrEditCflexHdrInfo_t, DbgEpeHdrEditCflexHdrInfo_valid_f},
        {DbgEpeHdrEditInfo_t, DbgEpeHdrEditInfo_valid_f},
        {DbgEpeEgressEditInfo_t, DbgEpeEgressEditInfo_valid_f},
        {DbgEpeInnerL2EditInfo_t, DbgEpeInnerL2EditInfo_valid_f},
        {DbgEpeL3EditInfo_t, DbgEpeL3EditInfo_valid_f},
        {DbgEpeOuterL2EditInfo_t, DbgEpeOuterL2EditInfo_valid_f},
        {DbgEpePayLoadInfo_t, DbgEpePayLoadInfo_valid_f},
        {DbgParserFromEpeHdrAdjInfo_t, DbgParserFromEpeHdrAdjInfo_valid_f},
        {DbgEpeNextHopInfo_t, DbgEpeNextHopInfo_valid_f},
        {DbgMACSecDecryptEngineInfo_t, DbgMACSecDecryptEngineInfo_valid_f},
        {DbgMACSecEncryptEngineInfo_t, DbgMACSecEncryptEngineInfo_valid_f},
    };
    sys_diag_pkt_trace_table_t met_tbl[] =
    {
        {DbgFwdMetFifoInfo1_t, DbgFwdMetFifoInfo1_valid_f},
        {DbgFwdMetFifoInfo2_t, DbgFwdMetFifoInfo2_valid_f},
        {DbgFwdBufRetrvHeaderInfo_t, DbgFwdBufRetrvHeaderInfo_valid_f},
        {DbgFwdBufRetrvInfo_t, DbgFwdBufRetrvInfo_valid_f},
        {DbgErmResrcAllocInfo_t, DbgErmResrcAllocInfo_valid_f},
        {DbgFwdQWriteInfo_t, DbgFwdQWriteInfo_valid_f},
        {MetFifoCtl_t, MetFifoCtl_debugLookupEn_f},
    };
    sys_diag_pkt_trace_table_t oam_rx_tbl[] =
    {
        {DbgOamHdrEdit_t, DbgOamHdrEdit_oamHdrEditValid_f},
        {DbgOamHdrAdj_t, DbgOamHdrAdj_oamHdrAdjValid_f},
        {DbgOamParser_t, DbgOamParser_valid_f},
        {DbgOamDefectProc_t, DbgOamDefectProc_oamDefectProcValid_f},
        {DbgOamRxProc_t, DbgOamRxProc_valid_f},
        {DbgOamApsSwitch_t, DbgOamApsSwitch_oamApsSwitchValid_f},
        {DbgOamApsProcess_t, DbgOamApsProcess_oamApsProcessValid_f},
    };

    sys_diag_pkt_trace_table_t oam_tx_tbl[] =
    {
        {DbgOamTxProc_t, DbgOamTxProc_oamTxProcValid_f},
    };

    switch (watch_point)
    {
        case CTC_DIAG_TRACE_POINT_IPE_SCL:
            loop_size = sizeof(oam_rx_tbl)/sizeof(sys_diag_pkt_trace_table_t);
            for (loop = 0; loop < loop_size; loop++)
            {
                cmd = DRV_IOW(oam_rx_tbl[loop].table, oam_rx_tbl[loop].field);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &temp_v));
            }
            goto clear_ipe;
        case CTC_DIAG_TRACE_POINT_TM_BS:
            goto clear_bsr;
        case CTC_DIAG_TRACE_POINT_TM_MET:
            goto clear_met;
        case CTC_DIAG_TRACE_POINT_EPE_ADJ:
            goto clear_epe;
        case CTC_DIAG_TRACE_POINT_OAM_ADJ:
            loop_size = sizeof(oam_rx_tbl)/sizeof(sys_diag_pkt_trace_table_t);
            for (loop = 0; loop < loop_size; loop++)
            {
                cmd = DRV_IOW(oam_rx_tbl[loop].table, oam_rx_tbl[loop].field);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &temp_v));
            }
            break;
        case CTC_DIAG_TRACE_POINT_OAM_TX:
            loop_size = sizeof(oam_tx_tbl)/sizeof(sys_diag_pkt_trace_table_t);
            for (loop = 0; loop < loop_size; loop++)
            {
                cmd = DRV_IOW(oam_tx_tbl[loop].table, oam_tx_tbl[loop].field);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &temp_v));
            }
            goto clear_bsr;
            break;
        default:
            break;
    }
    return CTC_E_NONE;

clear_ipe:
    sal_memset(&ds_flow_loop_cam, 0, sizeof(IpeUserIdFlowLoopCam_m));
    cmd = DRV_IOW(IpeUserIdFlowLoopCam_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds_flow_loop_cam));

    loop_size = sizeof(ipe_tbl)/sizeof(sys_diag_pkt_trace_table_t);
    for (loop = 0; loop < loop_size; loop++)
    {
        cmd = DRV_IOW(ipe_tbl[loop].table, ipe_tbl[loop].field);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &temp_v));
    }

    loop_size = sizeof(oam_rx_tbl)/sizeof(sys_diag_pkt_trace_table_t);
    for (loop = 0; loop < loop_size; loop++)
    {
        cmd = DRV_IOW(oam_rx_tbl[loop].table, oam_rx_tbl[loop].field);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &temp_v));
    }
    loop_size = sizeof(oam_tx_tbl)/sizeof(sys_diag_pkt_trace_table_t);
    for (loop = 0; loop < loop_size; loop++)
    {
        cmd = DRV_IOW(oam_tx_tbl[loop].table, oam_tx_tbl[loop].field);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &temp_v));
    }

clear_bsr:
    sal_memset(&ds_buff_store_cam, 0, sizeof(BufStoreDebugCam_m));
    cmd = DRV_IOW(BufStoreDebugCam_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds_buff_store_cam));

    loop_size = sizeof(bsr_tbl)/sizeof(sys_diag_pkt_trace_table_t);
    for (loop = 0; loop < loop_size; loop++)
    {
        cmd = DRV_IOW(bsr_tbl[loop].table, bsr_tbl[loop].field);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &temp_v));
    }

clear_met:
    sal_memset(&ds_met_fifo_cam, 0, sizeof(MetFifoDebugCam_m));
    cmd = DRV_IOW(MetFifoDebugCam_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds_met_fifo_cam));
    loop_size = sizeof(met_tbl)/sizeof(sys_diag_pkt_trace_table_t);
    for (loop = 0; loop < loop_size; loop++)
    {
        cmd = DRV_IOW(met_tbl[loop].table, met_tbl[loop].field);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &temp_v));
    }
clear_epe:
    sal_memset(&ds_epe_dbg_cam, 0, sizeof(EpeHdrAdjustDebugCam_m));
    cmd = DRV_IOW(EpeHdrAdjustDebugCam_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds_epe_dbg_cam));

    loop_size = sizeof(epe_tbl)/sizeof(sys_diag_pkt_trace_table_t);
    for (loop = 0; loop < loop_size; loop++)
    {
        cmd = DRV_IOW(epe_tbl[loop].table, epe_tbl[loop].field);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &temp_v));
    }

    return CTC_E_NONE;
}


STATIC int32
_sys_tsingma_diag_get_drop_info(uint8 lchip, ctc_diag_drop_t* p_drop)
{
    uint32 cmd = 0;
    uint32 cmd_lport = 0;
    uint16 lport = 0;
    uint8 gchip = 0;
    uint32 value = 0;
    uint32 loop_reason =0;
    DsIngressDiscardStats_m  ds_igs_stats;
    DsEgressDiscardStats_m  ds_egs_stats;
    uint32 channel_id = 0;
    uint32 start_chan = 0;
    uint32 end_chan = MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM) - 1;
    uint32 index = 0;
    uint8 get_detail_en = 0;
    uint16 user_reason = 0;
    drv_work_platform_type_t platform_type = 0;
    NetRxDebugStatsTable_m net_rx_debug_stats_table;
    PktErrStatsMem_m       pkt_err_stats_mem;
    BufStoreInputDebugStats_m buf_store_in_stats;
    BufStoreOutputDebugStats_m buf_store_out_stats;
    BufStoreStallDropCntSaturation_m buf_store_stall_drop;
    BufRetrvInputDebugStats_m buf_retrv_in_dbg_stats;
    BufRetrvOutputPktDebugStats_m buf_retrv_out_dbg_stats;
    NetTxDebugStats_m net_tx_debug_stats;

    drv_get_gchip_id(lchip, &gchip);
    CTC_ERROR_RETURN(drv_get_platform_type(lchip, &platform_type));

    if (p_drop->oper_type != CTC_DIAG_DROP_OPER_TYPE_GET_PORT_BMP)
    {
        start_chan = SYS_GET_CHANNEL_ID(lchip, CTC_MAP_LPORT_TO_GPORT(gchip, p_drop->lport));
        if (start_chan >= MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM))
        {
            return CTC_E_INVALID_GLOBAL_PORT;
        }
        end_chan = start_chan;
    }
    if (p_drop->oper_type == CTC_DIAG_DROP_OPER_TYPE_GET_DETAIL_INFO)
    {
        user_reason = p_drop->reason;
        get_detail_en = 1;
        sal_strcpy(p_drop->u.info.reason_desc, _sys_tsingma_diag_get_reason_desc(p_drop->reason));
        if (p_drop->reason <= IPE_END)
        {
            p_drop->u.info.position = CTC_DIAG_TRACE_POS_IPE;
        }
        else if (p_drop->reason <= EPE_END)
        {
            p_drop->u.info.position = CTC_DIAG_TRACE_POS_EPE;
        }
        else if (p_drop->reason <= BSR_END)
        {
            p_drop->u.info.position = CTC_DIAG_TRACE_POS_TM;
        }
        else if (p_drop->reason <= NETRX_END)
        {
            p_drop->u.info.position = CTC_DIAG_TRACE_POS_IPE;
        }
        else if (p_drop->reason <= NETTX_END)
        {
            p_drop->u.info.position = CTC_DIAG_TRACE_POS_EPE;
        }
        else if (p_drop->reason <= OAM_END)
        {
            p_drop->u.info.position = CTC_DIAG_TRACE_POS_OAM;
        }
    }

    sal_memset(&ds_igs_stats, 0, sizeof(ds_igs_stats));
    sal_memset(&ds_egs_stats, 0, sizeof(ds_egs_stats));
    cmd_lport = DRV_IOR(IpeHeaderAdjustPhyPortMap_t, IpeHeaderAdjustPhyPortMap_localPhyPort_f);
    for (channel_id = start_chan; channel_id <= end_chan; channel_id++)
    {
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd_lport, &value));
        lport = value;
        /* IPE feature discard*/
        for (loop_reason = IPE_FEATURE_START; loop_reason <= IPE_FEATURE_END; loop_reason++)
        {
            if (get_detail_en && (user_reason != loop_reason))
            {
                continue;
            }
            index = channel_id * MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM) + loop_reason - IPE_FEATURE_START;
            cmd = DRV_IOR(DsIngressDiscardStats_t, DsIngressDiscardStats_packetCount_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));
            CTC_ERROR_RETURN(sys_usw_diag_drop_hash_update_count(lchip, lport, loop_reason, value));
            if (SW_SIM_PLATFORM == platform_type)
            {
                cmd = DRV_IOW(DsIngressDiscardStats_t, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, index, cmd, &ds_igs_stats);
            }
        }

        /* EPE feature discard*/
        for (loop_reason = EPE_FEATURE_START; loop_reason <= EPE_FEATURE_END; loop_reason++)
        {
            if (get_detail_en && (user_reason != loop_reason))
            {
                continue;
            }
            index = (channel_id << 5) + (channel_id << 4) + (channel_id << 3) + loop_reason - EPE_FEATURE_START;
            cmd = DRV_IOR(DsEgressDiscardStats_t, DsEgressDiscardStats_packetCount_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));
            CTC_ERROR_RETURN(sys_usw_diag_drop_hash_update_count(lchip, lport, loop_reason, value));
            if (SW_SIM_PLATFORM == platform_type)
            {
                cmd = DRV_IOW(DsEgressDiscardStats_t, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, index, cmd, &ds_egs_stats);
            }
        }

        /*bufferstore */
        sal_memset(&pkt_err_stats_mem, 0, sizeof(pkt_err_stats_mem));
        cmd = DRV_IOR(BufStorePktErrStatsMem_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &pkt_err_stats_mem));
        GetPktErrStatsMem(A, pktAbortCnt_f, &pkt_err_stats_mem, &value);
        CTC_ERROR_RETURN(sys_usw_diag_drop_hash_update_count(lchip, lport, BUFSTORE_ABORT_TOTAL, value));
        value = GetPktErrStatsMem(V, pktAbortOverLenErrorCnt_f, &pkt_err_stats_mem) +
                GetPktErrStatsMem(V, pktSilentDropUnderLenError_f, &pkt_err_stats_mem);
        CTC_ERROR_RETURN(sys_usw_diag_drop_hash_update_count(lchip, lport, BUFSTORE_LEN_ERROR, value));
        GetPktErrStatsMem(A, pktSilentDropResrcFailCnt_f, &pkt_err_stats_mem, &value);
        CTC_ERROR_RETURN(sys_usw_diag_drop_hash_update_count(lchip, lport, BUFSTORE_IRM_RESRC, value));
        value = GetPktErrStatsMem(V, pktAbortDataErrorCnt_f, &pkt_err_stats_mem) +
                GetPktErrStatsMem(V, pktSilentDropDataErrorCnt_f, &pkt_err_stats_mem);
        CTC_ERROR_RETURN(sys_usw_diag_drop_hash_update_count(lchip, lport, BUFSTORE_DATA_ERR, value));
        GetPktErrStatsMem(A, pktSilentDropChipIdMismatchCnt_f, &pkt_err_stats_mem, &value);
        CTC_ERROR_RETURN(sys_usw_diag_drop_hash_update_count(lchip, lport, BUFSTORE_CHIP_MISMATCH, value));
        value = GetPktErrStatsMem(V, pktAbortNoBufCnt_f, &pkt_err_stats_mem) +
                GetPktErrStatsMem(V, pktSilentDropNoBufCnt_f, &pkt_err_stats_mem);
        CTC_ERROR_RETURN(sys_usw_diag_drop_hash_update_count(lchip, lport, BUFSTORE_NO_BUFF, value));
        GetPktErrStatsMem(A, pktAbortFramingErrorCnt_f, &pkt_err_stats_mem, &value);
        CTC_ERROR_RETURN(sys_usw_diag_drop_hash_update_count(lchip, lport, BUFSTORE_NO_EOP, value));
        GetPktErrStatsMem(A, pktAbortMetFifoDropCnt_f, &pkt_err_stats_mem, &value);
        CTC_ERROR_RETURN(sys_usw_diag_drop_hash_update_count(lchip, lport, METFIFO_STALL_TO_BS_DROP, value));
        GetPktErrStatsMem(A, pktSilentDropCnt_f, &pkt_err_stats_mem, &value);
        CTC_ERROR_RETURN(sys_usw_diag_drop_hash_update_count(lchip, lport, BUFSTORE_SLIENT_TOTAL, value));

        /*read net rx discard stats*/
        sal_memset(&net_rx_debug_stats_table, 0, sizeof(net_rx_debug_stats_table));
        cmd = DRV_IOR(NetRxDebugStatsTable_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &net_rx_debug_stats_table));
        GetNetRxDebugStatsTable(A, fullDropCntSaturation_f, &net_rx_debug_stats_table, &value);
        CTC_ERROR_RETURN(sys_usw_diag_drop_hash_update_count(lchip, lport, NETRX_NO_BUFFER, value));
        value = GetNetRxDebugStatsTable(V, overLenErrDropCntSaturation_f, &net_rx_debug_stats_table) +
                GetNetRxDebugStatsTable(V, underLenErrDropCntSaturation_f, &net_rx_debug_stats_table);
        CTC_ERROR_RETURN(sys_usw_diag_drop_hash_update_count(lchip, lport, NETRX_LEN_ERROR, value));
        value = GetNetRxDebugStatsTable(V, pktErrDropCntSaturation_f, &net_rx_debug_stats_table) +
                GetNetRxDebugStatsTable(V, pktWithErrorCntSaturation_f, &net_rx_debug_stats_table);
        CTC_ERROR_RETURN(sys_usw_diag_drop_hash_update_count(lchip, lport, NETRX_PKT_ERROR, value));
        GetNetRxDebugStatsTable(A, pauseDiscardCntSaturation_f, &net_rx_debug_stats_table, &value);
        CTC_ERROR_RETURN(sys_usw_diag_drop_hash_update_count(lchip, lport, NETRX_BPDU_ERROR, value));
        GetNetRxDebugStatsTable(A, frameErrDropCntSaturation_f, &net_rx_debug_stats_table, &value);
        CTC_ERROR_RETURN(sys_usw_diag_drop_hash_update_count(lchip, lport, NETRX_FRAME_ERROR, value));
        if (SW_SIM_PLATFORM == platform_type)
        {
            sal_memset(&pkt_err_stats_mem, 0, sizeof(pkt_err_stats_mem));
            cmd = DRV_IOW(BufStorePktErrStatsMem_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, channel_id, cmd, &pkt_err_stats_mem);
            sal_memset(&net_rx_debug_stats_table, 0, sizeof(net_rx_debug_stats_table));
            cmd = DRV_IOW(NetRxDebugStatsTable_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, channel_id, cmd, &net_rx_debug_stats_table);
        }
    }



    sal_memset(&buf_store_in_stats, 0, sizeof(buf_store_in_stats));
    cmd = DRV_IOR(BufStoreInputDebugStats_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &buf_store_in_stats));
    GetBufStoreInputDebugStats(A, frDmaDataErrCnt_f, &buf_store_in_stats, &value);
    CTC_ERROR_RETURN(sys_usw_diag_drop_hash_update_count(lchip, SYS_DIAG_DROP_COUNT_SYSTEM, TO_BUFSTORE_FROM_DMA, value));
    GetBufStoreInputDebugStats(A, frNetRxDataErrCnt_f, &buf_store_in_stats, &value);
    CTC_ERROR_RETURN(sys_usw_diag_drop_hash_update_count(lchip, SYS_DIAG_DROP_COUNT_SYSTEM, TO_BUFSTORE_FROM_NETRX, value));
    GetBufStoreInputDebugStats(A, frOamDataErrCnt_f, &buf_store_in_stats, &value);
    CTC_ERROR_RETURN(sys_usw_diag_drop_hash_update_count(lchip, SYS_DIAG_DROP_COUNT_SYSTEM, TO_BUFSTORE_FROM_OAM, value));
    if (SW_SIM_PLATFORM == platform_type)
    {
        sal_memset(&buf_store_in_stats, 0, sizeof(buf_store_in_stats));
        cmd = DRV_IOW(BufStoreInputDebugStats_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &buf_store_in_stats);
    }

    sal_memset(&buf_store_out_stats, 0, sizeof(buf_store_out_stats));
    cmd = DRV_IOR(BufStoreOutputDebugStats_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &buf_store_out_stats));
    GetBufStoreOutputDebugStats(A, sopPktLenErrorCnt_f, &buf_store_out_stats, &value);
    CTC_ERROR_RETURN(sys_usw_diag_drop_hash_update_count(lchip, SYS_DIAG_DROP_COUNT_SYSTEM, BUFSTORE_OUT_SOP_PKT_LEN_ERR, value));
    if (SW_SIM_PLATFORM == platform_type)
    {
        sal_memset(&buf_store_out_stats, 0, sizeof(buf_store_out_stats));
        cmd = DRV_IOW(BufStoreOutputDebugStats_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &buf_store_out_stats);
    }


    sal_memset(&buf_store_stall_drop, 0, sizeof(buf_store_stall_drop));
    cmd = DRV_IOR(BufStoreStallDropCntSaturation_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &buf_store_stall_drop));
    GetBufStoreStallDropCntSaturation(A, dmaMcastDropCnt_f, &buf_store_stall_drop, &value);
    CTC_ERROR_RETURN(sys_usw_diag_drop_hash_update_count(lchip, SYS_DIAG_DROP_COUNT_SYSTEM, METFIFO_MC_FROM_DMA, value));

    GetBufStoreStallDropCntSaturation(A, dmaUcastDropCnt_f, &buf_store_stall_drop, &value);
    CTC_ERROR_RETURN(sys_usw_diag_drop_hash_update_count(lchip, SYS_DIAG_DROP_COUNT_SYSTEM, METFIFO_UC_FROM_DMA, value));
    GetBufStoreStallDropCntSaturation(A, eLoopMcastDropCnt_f, &buf_store_stall_drop, &value);
    CTC_ERROR_RETURN(sys_usw_diag_drop_hash_update_count(lchip, SYS_DIAG_DROP_COUNT_SYSTEM, METFIFO_MC_FROM_ELOOP_MCAST, value));
    GetBufStoreStallDropCntSaturation(A, eLoopUcastDropCnt_f, &buf_store_stall_drop, &value);
    CTC_ERROR_RETURN(sys_usw_diag_drop_hash_update_count(lchip, SYS_DIAG_DROP_COUNT_SYSTEM, METFIFO_UC_FROM_ELOOP_UCAST, value));
    GetBufStoreStallDropCntSaturation(A, ipeHdrMcastDropCnt_f, &buf_store_stall_drop, &value);
    CTC_ERROR_RETURN(sys_usw_diag_drop_hash_update_count(lchip, SYS_DIAG_DROP_COUNT_SYSTEM, METFIFO_MC_FROM_IPE_CUT_THR, value));
    GetBufStoreStallDropCntSaturation(A, ipeHdrUcastDropCnt_f, &buf_store_stall_drop, &value);
    CTC_ERROR_RETURN(sys_usw_diag_drop_hash_update_count(lchip, SYS_DIAG_DROP_COUNT_SYSTEM, METFIFO_UC_FROM_IPE_CUT_THR, value));
    GetBufStoreStallDropCntSaturation(A, ipeMcastDropCnt_f, &buf_store_stall_drop, &value);
    CTC_ERROR_RETURN(sys_usw_diag_drop_hash_update_count(lchip, SYS_DIAG_DROP_COUNT_SYSTEM, METFIFO_MC_FROM_IPE, value));
    GetBufStoreStallDropCntSaturation(A, ipeUcastDropCnt_f, &buf_store_stall_drop, &value);
    CTC_ERROR_RETURN(sys_usw_diag_drop_hash_update_count(lchip, SYS_DIAG_DROP_COUNT_SYSTEM, METFIFO_UC_FROM_IPE, value));
    GetBufStoreStallDropCntSaturation(A, oamMcastDropCnt_f, &buf_store_stall_drop, &value);
    CTC_ERROR_RETURN(sys_usw_diag_drop_hash_update_count(lchip, SYS_DIAG_DROP_COUNT_SYSTEM, METFIFO_MC_FROM_OAM, value));
    GetBufStoreStallDropCntSaturation(A, oamUcastDropCnt_f, &buf_store_stall_drop, &value);
    CTC_ERROR_RETURN(sys_usw_diag_drop_hash_update_count(lchip, SYS_DIAG_DROP_COUNT_SYSTEM, METFIFO_UC_FROM_OAM, value));
    if (SW_SIM_PLATFORM == platform_type)
    {
        sal_memset(&buf_store_stall_drop, 0, sizeof(buf_store_stall_drop));
        cmd = DRV_IOW(BufStoreStallDropCntSaturation_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &buf_store_stall_drop);
    }

    sal_memset(&buf_retrv_in_dbg_stats, 0, sizeof(buf_retrv_in_dbg_stats));
    cmd = DRV_IOR(BufRetrvInputDebugStats_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &buf_retrv_in_dbg_stats));
    GetBufRetrvInputDebugStats(A, frDeqMsgDiscardCnt_f, &buf_retrv_in_dbg_stats, &value);
    CTC_ERROR_RETURN(sys_usw_diag_drop_hash_update_count(lchip, SYS_DIAG_DROP_COUNT_SYSTEM, BUFRETRV_FROM_DEQ_MSG_ERR, value));
    GetBufRetrvInputDebugStats(A, frQMgrLenErrorCnt_f, &buf_retrv_in_dbg_stats, &value);
    CTC_ERROR_RETURN(sys_usw_diag_drop_hash_update_count(lchip, SYS_DIAG_DROP_COUNT_SYSTEM, BUFRETRV_FROM_QMGR_LEN_ERR, value));
    if (SW_SIM_PLATFORM == platform_type)
    {
        sal_memset(&buf_retrv_in_dbg_stats, 0, sizeof(buf_retrv_in_dbg_stats));
        cmd = DRV_IOW(BufRetrvInputDebugStats_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &buf_retrv_in_dbg_stats);
    }

    sal_memset(&buf_retrv_out_dbg_stats, 0, sizeof(buf_retrv_out_dbg_stats));
    cmd = DRV_IOR(BufRetrvOutputPktDebugStats_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &buf_retrv_out_dbg_stats));
    GetBufRetrvOutputPktDebugStats(A, toIntfFifoErrorCnt_f, &buf_retrv_out_dbg_stats, &value);
    CTC_ERROR_RETURN(sys_usw_diag_drop_hash_update_count(lchip, SYS_DIAG_DROP_COUNT_SYSTEM, BUFRETRV_OUT_DROP, value));
    if (SW_SIM_PLATFORM == platform_type)
    {
        sal_memset(&buf_retrv_out_dbg_stats, 0, sizeof(buf_retrv_out_dbg_stats));
        cmd = DRV_IOW(BufRetrvOutputPktDebugStats_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &buf_retrv_out_dbg_stats);
    }

    sal_memset(&net_tx_debug_stats, 0, sizeof(net_tx_debug_stats));
    cmd = DRV_IOR(NetTxDebugStats_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &net_tx_debug_stats));
    GetNetTxDebugStats(A, minLenDropCnt_f, &net_tx_debug_stats, &value);
    CTC_ERROR_RETURN(sys_usw_diag_drop_hash_update_count(lchip, SYS_DIAG_DROP_COUNT_SYSTEM, NETTX_MIN_LEN, value));
    GetNetTxDebugStats(A, rxNoBufDropCnt_f, &net_tx_debug_stats, &value);
    CTC_ERROR_RETURN(sys_usw_diag_drop_hash_update_count(lchip, SYS_DIAG_DROP_COUNT_SYSTEM, NETTX_NO_BUFFER, value));
    value = GetNetTxDebugStats(V, rxNoEopDropCnt_f, &net_tx_debug_stats) +
            GetNetTxDebugStats(V, rxNoSopDropCnt_f, &net_tx_debug_stats);
    CTC_ERROR_RETURN(sys_usw_diag_drop_hash_update_count(lchip, SYS_DIAG_DROP_COUNT_SYSTEM, NETTX_SOP_EOP, value));
    GetNetTxDebugStats(A, txErrorCnt_f, &net_tx_debug_stats, &value);
    CTC_ERROR_RETURN(sys_usw_diag_drop_hash_update_count(lchip, SYS_DIAG_DROP_COUNT_SYSTEM, NETTX_TX_ERROR, value));
    if (SW_SIM_PLATFORM == platform_type)
    {
        sal_memset(&net_tx_debug_stats, 0, sizeof(net_tx_debug_stats));
        cmd = DRV_IOW(NetTxDebugStats_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &net_tx_debug_stats);
    }

    cmd = DRV_IOR(OamFwdDebugStats_t, OamFwdDebugStats_asicHardDiscardCnt_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    CTC_ERROR_RETURN(sys_usw_diag_drop_hash_update_count(lchip, SYS_DIAG_DROP_COUNT_SYSTEM, OAM_HW_ERROR, value));
    if (SW_SIM_PLATFORM == platform_type)
    {
        value = 0;
        cmd = DRV_IOW(OamFwdDebugStats_t, OamFwdDebugStats_asicHardDiscardCnt_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    }

    cmd = DRV_IOR(OamFwdDebugStats_t, OamFwdDebugStats_exceptionDiscardCnt_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    CTC_ERROR_RETURN(sys_usw_diag_drop_hash_update_count(lchip, SYS_DIAG_DROP_COUNT_SYSTEM, OAM_EXCEPTION, value));
    if (SW_SIM_PLATFORM == platform_type)
    {
        value = 0;
        cmd = DRV_IOW(OamFwdDebugStats_t, OamFwdDebugStats_exceptionDiscardCnt_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    }

    return CTC_E_NONE;
}


#define ______DIAG_MCHIP_API______

int32
sys_tsingma_diag_get_pkt_trace(uint8 lchip, void* p_value)
{

    ctc_diag_pkt_trace_result_t* p_rslt = (ctc_diag_pkt_trace_result_t*)p_value;

    CTC_PTR_VALID_CHECK(p_rslt);

    CTC_ERROR_RETURN(_sys_tsingma_diag_get_pkt_trace_pos_ipe(lchip, p_rslt));
    CTC_ERROR_RETURN(_sys_tsingma_diag_get_pkt_trace_pos_tm(lchip, p_rslt));
    CTC_ERROR_RETURN(_sys_tsingma_diag_get_pkt_trace_pos_epe(lchip, p_rslt));
    CTC_ERROR_RETURN(_sys_tsingma_diag_get_pkt_trace_pos_oam(lchip, p_rslt));

    return CTC_E_NONE;
}

int32
sys_tsingma_diag_get_drop_info(uint8 lchip, void* p_value)
{
    ctc_diag_drop_t* p_drop = (ctc_diag_drop_t*)p_value;
    CTC_PTR_VALID_CHECK(p_drop);
    CTC_ERROR_RETURN(_sys_tsingma_diag_get_drop_info(lchip, p_drop));
    return CTC_E_NONE;
}

int32
sys_tsingma_diag_set_dbg_pkt(uint8 lchip, void* p_value)
{
    uint32 cmd = 0;
    uint32 loop = 0;
    hw_mac_addr_t   hw_mac      = {0};
    ipv6_addr_t     hw_ip6 = {0};
    ctc_field_key_t*  field = NULL;
    ctc_diag_pkt_trace_t* p_trace = (ctc_diag_pkt_trace_t*)p_value;
    IpeUserIdFlowCam_m ds_flow_cam;
    ParserResult_m parser_result;
    ParserResult_m parser_result_mask;

    CTC_PTR_VALID_CHECK(p_trace);
    sal_memset(&ds_flow_cam, 0, sizeof(IpeUserIdFlowCam_m));
    sal_memset(&parser_result, 0, sizeof(ParserResult_m));
    sal_memset(&parser_result_mask, 0, sizeof(ParserResult_m));

    while (loop < p_trace->pkt.network.count)
    {
        field = &(p_trace->pkt.network.field[loop]);
        loop++;
        switch (field->type)
        {
            case CTC_FIELD_KEY_L2_TYPE:
                SetParserResult(V, layer2Type_f, &parser_result, field->data);
                SetParserResult(V, layer2Type_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_L3_TYPE:
                SetParserResult(V, layer3Type_f, &parser_result, field->data);
                SetParserResult(V, layer3Type_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_L4_TYPE:
                SetParserResult(V, layer4Type_f, &parser_result, field->data);
                SetParserResult(V, layer4Type_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_L4_USER_TYPE:
                SetParserResult(V, layer4UserType_f, &parser_result, field->data);
                SetParserResult(V, layer4UserType_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_ETHER_TYPE:
                SetParserResult(V, etherType_f, &parser_result, field->data);
                SetParserResult(V, etherType_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_MAC_DA:
                CTC_PTR_VALID_CHECK(field->ext_data);
                CTC_PTR_VALID_CHECK(field->ext_mask);
                SYS_USW_SET_HW_MAC(hw_mac, (uint8*)field->ext_data);
                SetParserResult(A, macDa_f, &parser_result, hw_mac);
                SYS_USW_SET_HW_MAC(hw_mac, (uint8*)field->ext_mask);
                SetParserResult(A, macDa_f, &parser_result_mask, hw_mac);
                break;
            case CTC_FIELD_KEY_MAC_SA:
                CTC_PTR_VALID_CHECK(field->ext_data);
                CTC_PTR_VALID_CHECK(field->ext_mask);
                SYS_USW_SET_HW_MAC(hw_mac, (uint8*)field->ext_data);
                SetParserResult(A, macSa_f, &parser_result, hw_mac);
                SYS_USW_SET_HW_MAC(hw_mac, (uint8*)field->ext_mask);
                SetParserResult(A, macSa_f, &parser_result_mask, hw_mac);
                break;
            case CTC_FIELD_KEY_SVLAN_ID:
                SetParserResult(V, svlanId_f, &parser_result, field->data);
                SetParserResult(V, svlanId_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_CVLAN_ID:
                SetParserResult(V, cvlanId_f, &parser_result, field->data);
                SetParserResult(V, cvlanId_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_VLAN_NUM:
                SetParserResult(V, vlanNum_f, &parser_result, field->data);
                SetParserResult(V, vlanNum_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_IP_SA:
                SetParserResult(V, uL3Source_gIpv4_ipSa_f, &parser_result, field->data);
                SetParserResult(V, uL3Source_gIpv4_ipSa_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_IP_DA:
                SetParserResult(V, uL3Dest_gIpv4_ipDa_f, &parser_result, field->data);
                SetParserResult(V, uL3Dest_gIpv4_ipDa_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_IPV6_SA:
                CTC_PTR_VALID_CHECK(field->ext_data);
                CTC_PTR_VALID_CHECK(field->ext_mask);
                SYS_USW_REVERT_IP6(hw_ip6, (uint32*)field->ext_data);
                SetParserResult(A, uL3Source_gIpv6_ipSa_f, &parser_result, hw_ip6);
                SYS_USW_REVERT_IP6(hw_ip6, (uint32*)field->ext_mask);
                SetParserResult(A, uL3Source_gIpv6_ipSa_f, &parser_result_mask, hw_ip6);
                break;
            case CTC_FIELD_KEY_IPV6_DA:
                CTC_PTR_VALID_CHECK(field->ext_data);
                CTC_PTR_VALID_CHECK(field->ext_mask);
                SYS_USW_REVERT_IP6(hw_ip6, (uint32*)field->ext_data);
                SetParserResult(A, uL3Dest_gIpv6_ipDa_f, &parser_result, hw_ip6);
                SYS_USW_REVERT_IP6(hw_ip6, (uint32*)field->ext_mask);
                SetParserResult(A, uL3Dest_gIpv6_ipDa_f, &parser_result_mask, hw_ip6);
                break;
            case CTC_FIELD_KEY_IPV6_FLOW_LABEL:
                SetParserResult(V, ipv6FlowLabel_f, &parser_result, field->data);
                SetParserResult(V, ipv6FlowLabel_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_IP_PROTOCOL:
                SetParserResult(V, layer3HeaderProtocol_f, &parser_result, field->data);
                SetParserResult(V, layer3HeaderProtocol_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_IP_TTL:
                SetParserResult(V, ttl_f, &parser_result, field->data);
                SetParserResult(V, ttl_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_ARP_OP_CODE:
                SetParserResult(V, uL3Dest_gArp_arpOpCode_f, &parser_result, field->data);
                SetParserResult(V, uL3Dest_gArp_arpOpCode_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_ARP_PROTOCOL_TYPE:
                SetParserResult(V, uL3Source_gArp_protocolType_f, &parser_result, field->data);
                SetParserResult(V, uL3Source_gArp_protocolType_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_ARP_SENDER_IP:
                SetParserResult(V, uL3Source_gArp_senderIp_f, &parser_result, field->data);
                SetParserResult(V, uL3Source_gArp_senderIp_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_ARP_TARGET_IP:
                SetParserResult(V, uL3Dest_gArp_targetIp_f, &parser_result, field->data);
                SetParserResult(V, uL3Dest_gArp_targetIp_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_SENDER_MAC:
                CTC_PTR_VALID_CHECK(field->ext_data);
                CTC_PTR_VALID_CHECK(field->ext_mask);
                SYS_USW_SET_HW_MAC(hw_mac, (uint8*)field->ext_data);
                SetParserResult(A, uL3Source_gArp_senderMac_f, &parser_result, hw_mac);
                SYS_USW_SET_HW_MAC(hw_mac, (uint8*)field->ext_mask);
                SetParserResult(A, uL3Source_gArp_senderMac_f, &parser_result_mask, hw_mac);
                break;
            case CTC_FIELD_KEY_TARGET_MAC:
                CTC_PTR_VALID_CHECK(field->ext_data);
                CTC_PTR_VALID_CHECK(field->ext_mask);
                SYS_USW_SET_HW_MAC(hw_mac, (uint8*)field->ext_data);
                SetParserResult(A, uL3Dest_gArp_targetMac_f, &parser_result, hw_mac);
                SYS_USW_SET_HW_MAC(hw_mac, (uint8*)field->ext_mask);
                SetParserResult(A, uL3Dest_gArp_targetMac_f, &parser_result_mask, hw_mac);
                break;
            case CTC_FIELD_KEY_ETHER_OAM_LEVEL:
                SetParserResult(V, uL3Dest_gEtherOam_etherOamLevel_f, &parser_result, field->data);
                SetParserResult(V, uL3Dest_gEtherOam_etherOamLevel_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_ETHER_OAM_OP_CODE:
                SetParserResult(V, uL3Dest_gEtherOam_etherOamOpCode_f, &parser_result, field->data);
                SetParserResult(V, uL3Dest_gEtherOam_etherOamOpCode_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_ETHER_OAM_VERSION:
                SetParserResult(V, uL3Dest_gEtherOam_etherOamVersion_f, &parser_result, field->data);
                SetParserResult(V, uL3Dest_gEtherOam_etherOamVersion_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_FCOE_DST_FCID:
                SetParserResult(V, uL3Dest_gFcoe_fcoeDid_f, &parser_result, field->data);
                SetParserResult(V, uL3Dest_gFcoe_fcoeDid_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_FCOE_SRC_FCID:
                SetParserResult(V, uL3Source_gFcoe_fcoeSid_f, &parser_result, field->data);
                SetParserResult(V, uL3Source_gFcoe_fcoeSid_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_LABEL_NUM:
                SetParserResult(V, uL3Tos_gMpls_labelNum_f, &parser_result, field->data);
                SetParserResult(V, uL3Tos_gMpls_labelNum_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_MPLS_LABEL0:
                SetParserResult(V, uL3Dest_gMpls_mplsLabel0_f, &parser_result, field->data);
                SetParserResult(V, uL3Dest_gMpls_mplsLabel0_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_MPLS_LABEL1:
                SetParserResult(V, uL3Dest_gMpls_mplsLabel1_f, &parser_result, field->data);
                SetParserResult(V, uL3Dest_gMpls_mplsLabel1_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_MPLS_LABEL2:
                SetParserResult(V, uL3Dest_gMpls_mplsLabel2_f, &parser_result, field->data);
                SetParserResult(V, uL3Dest_gMpls_mplsLabel2_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_SLOW_PROTOCOL_CODE:
                SetParserResult(V, uL3Dest_gSlowProto_slowProtocolCode_f, &parser_result, field->data);
                SetParserResult(V, uL3Dest_gSlowProto_slowProtocolCode_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_SLOW_PROTOCOL_FLAGS:
                SetParserResult(V, uL3Dest_gSlowProto_slowProtocolFlags_f, &parser_result, field->data);
                SetParserResult(V, uL3Dest_gSlowProto_slowProtocolFlags_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_SLOW_PROTOCOL_SUB_TYPE:
                SetParserResult(V, uL3Dest_gSlowProto_slowProtocolSubType_f, &parser_result, field->data);
                SetParserResult(V, uL3Dest_gSlowProto_slowProtocolSubType_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_INGRESS_NICKNAME:
                SetParserResult(V, uL3Source_gTrill_ingressNickname_f, &parser_result, field->data);
                SetParserResult(V, uL3Source_gTrill_ingressNickname_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_EGRESS_NICKNAME:
                SetParserResult(V, uL3Dest_gTrill_egressNickname_f, &parser_result, field->data);
                SetParserResult(V, uL3Dest_gTrill_egressNickname_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_TRILL_INNER_VLANID:
                SetParserResult(V, uL3Dest_gTrill_trillInnerVlanId_f, &parser_result, field->data);
                SetParserResult(V, uL3Dest_gTrill_trillInnerVlanId_f, &parser_result_mask, field->mask);
                SetParserResult(V, uL3Dest_gTrill_trillInnerVlanValid_f, &parser_result, 1);
                SetParserResult(V, uL3Dest_gTrill_trillInnerVlanValid_f, &parser_result_mask, 1);
                break;
            case CTC_FIELD_KEY_TRILL_VERSION:
                SetParserResult(V, uL3Dest_gTrill_trillVersion_f, &parser_result, field->data);
                SetParserResult(V, uL3Dest_gTrill_trillVersion_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_L4_DST_PORT:
                SetParserResult(V, uL4Dest_gPort_l4DestPort_f, &parser_result, field->data);
                SetParserResult(V, uL4Dest_gPort_l4DestPort_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_L4_SRC_PORT:
                SetParserResult(V, uL4Source_gPort_l4SourcePort_f, &parser_result, field->data);
                SetParserResult(V, uL4Source_gPort_l4SourcePort_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_TCP_ECN:
                SetParserResult(V, uL4UserData_gTcp_tcpEcn_f, &parser_result, field->data);
                SetParserResult(V, uL4UserData_gTcp_tcpEcn_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_TCP_FLAGS:
                SetParserResult(V, uL4UserData_gTcp_tcpFlags_f, &parser_result, field->data);
                SetParserResult(V, uL4UserData_gTcp_tcpFlags_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_VN_ID:
                SetParserResult(V, uL4UserData_gVxlan_vxlanVni_f, &parser_result, field->data);
                SetParserResult(V, uL4UserData_gVxlan_vxlanVni_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_VXLAN_FLAGS:
                SetParserResult(V, uL4UserData_gVxlan_vxlanFlags_f, &parser_result, field->data);
                SetParserResult(V, uL4UserData_gVxlan_vxlanFlags_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_VXLAN_RSV1:
                SetParserResult(V, uL4UserData_gVxlan_vxlanReserved1_f, &parser_result, field->data);
                SetParserResult(V, uL4UserData_gVxlan_vxlanReserved1_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_VXLAN_RSV2:
                SetParserResult(V, uL4UserData_gVxlan_vxlanReserved2_f, &parser_result, field->data);
                SetParserResult(V, uL4UserData_gVxlan_vxlanReserved2_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_GRE_KEY:
                SetParserResult(V, uL4UserData_gGre_greKey_f, &parser_result, field->data);
                SetParserResult(V, uL4UserData_gGre_greKey_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_GRE_FLAGS:
                SetParserResult(V, uL4Source_gGre_greFlags_f, &parser_result, field->data);
                SetParserResult(V, uL4Source_gGre_greFlags_f, &parser_result_mask, field->mask);
                break;
            case CTC_FIELD_KEY_GRE_PROTOCOL_TYPE:
                SetParserResult(V, uL4Dest_gGre_greProtocolType_f, &parser_result, field->data);
                SetParserResult(V, uL4Dest_gGre_greProtocolType_f, &parser_result_mask, field->mask);
                break;
            default:
                return CTC_E_NOT_SUPPORT;
        }
    }
    cmd = DRV_IOW(IpeUserIdFlowCam_t, IpeUserIdFlowCam_g_0_data_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &parser_result));
    cmd = DRV_IOW(IpeUserIdFlowCam_t, IpeUserIdFlowCam_g_1_data_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &parser_result_mask));

    return CTC_E_NONE;
}

int32
sys_tsingma_diag_set_dbg_session(uint8 lchip, void* p_value)
{
    uint32 value = 0;
    uint32 cmd = 0;
    uint32 dest_map = 0;
    uint32 watch_point = 0;
    ctc_diag_pkt_trace_t* p_trace = (ctc_diag_pkt_trace_t*)p_value;
    ctc_diag_trace_watch_key_t* watch_key = NULL;
    IpeUserIdFlowCam_m ds_flow_cam;
    BufStoreDebugCam_m ds_buff_store_cam;
    MetFifoDebugCam_m ds_met_fifo_cam;
    EpeHdrAdjustDebugCam_m ds_epe_dbg_cam;

    CTC_PTR_VALID_CHECK(p_trace);
    sal_memset(&ds_flow_cam, 0, sizeof(IpeUserIdFlowCam_m));
    sal_memset(&ds_buff_store_cam, 0, sizeof(BufStoreDebugCam_m));
    sal_memset(&ds_met_fifo_cam, 0, sizeof(MetFifoDebugCam_m));
    sal_memset(&ds_epe_dbg_cam, 0, sizeof(EpeHdrAdjustDebugCam_m));
    watch_point = p_trace->watch_point;
    watch_key = &p_trace->watch_key;
    CTC_ERROR_RETURN(_sys_tsingma_diag_pkt_trace_clear_rslt(lchip, watch_point));
    /* MetFif select isDbgPkt */
    value = 1;
    cmd = DRV_IOW(MetFifoDebugCtl_t, MetFifoDebugCtl_gSrcChanType_0_debugPathSel_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    cmd =DRV_IOW(MetFifoDebugCtl_t, MetFifoDebugCtl_gSrcChanType_1_debugPathSel_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    cmd = DRV_IOW(MetFifoDebugCtl_t, MetFifoDebugCtl_gSrcChanType_2_debugPathSel_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    cmd = DRV_IOW(MetFifoDebugCtl_t, MetFifoDebugCtl_gSrcChanType_3_debugPathSel_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    cmd = DRV_IOW(MetFifoDebugCtl_t, MetFifoDebugCtl_gSrcChanType_4_debugPathSel_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    cmd = DRV_IOW(MetFifoDebugCtl_t, MetFifoDebugCtl_gSrcChanType_5_debugPathSel_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    cmd = DRV_IOW(MetFifoDebugCtl_t, MetFifoDebugCtl_gSrcChanType_6_debugPathSel_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    cmd = DRV_IOW(MetFifoDebugCtl_t, MetFifoDebugCtl_gSrcChanType_7_debugPathSel_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    cmd = DRV_IOW(DebugSessionStatusCtl_t, DebugSessionStatusCtl_isFree_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    cmd = DRV_IOW(IpeUserIdFlowCtl_t, IpeUserIdFlowCtl_flowEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    value = (watch_point == CTC_DIAG_TRACE_POINT_IPE_SCL) ? 0 : 1;
    cmd = DRV_IOW(IpeUserIdFlowLoopCam_t, IpeUserIdFlowLoopCam_isDebuggedPktMask_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    cmd = DRV_IOW(IpeUserIdFlowCtl_t, IpeUserIdFlowCtl_debugMode_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    switch (watch_point)
    {
        case CTC_DIAG_TRACE_POINT_IPE_SCL:
            if (CTC_BMP_ISSET(watch_key->key_mask_bmp, CTC_DIAG_WATCH_KEY_MASK_SRC_LPORT))
            {
                SetIpeUserIdFlowCam(V, g_0_localPhyPort_f, &ds_flow_cam, watch_key->src_lport);
                SetIpeUserIdFlowCam(V, g_1_localPhyPort_f, &ds_flow_cam, 0xFF);
            }
            if (CTC_BMP_ISSET(watch_key->key_mask_bmp, CTC_DIAG_WATCH_KEY_MASK_CHANNEL))
            {
                SetIpeUserIdFlowCam(V, g_0_channelId0_f, &ds_flow_cam, watch_key->channel);
                SetIpeUserIdFlowCam(V, g_0_channelId1_f, &ds_flow_cam, 0xFF);
                SetIpeUserIdFlowCam(V, g_0_channelId2_f, &ds_flow_cam, 0xFF);
                SetIpeUserIdFlowCam(V, g_0_channelId3_f, &ds_flow_cam, 0xFF);
                SetIpeUserIdFlowCam(V, g_1_channelId0_f, &ds_flow_cam, 0xFF);
                SetIpeUserIdFlowCam(V, g_1_channelId1_f, &ds_flow_cam, 0xFF);
                SetIpeUserIdFlowCam(V, g_1_channelId2_f, &ds_flow_cam, 0xFF);
                SetIpeUserIdFlowCam(V, g_1_channelId3_f, &ds_flow_cam, 0xFF);
            }
            if (p_trace->mode == CTC_DIAG_TRACE_MODE_USER)
            {
                SetBufStoreDebugCam(V, g_0_sourcePort_f, &ds_buff_store_cam, p_trace->pkt.user.src_port);
                SetBufStoreDebugCam(V, g_1_sourcePort_f, &ds_buff_store_cam, 0xFFFF);
                SetBufStoreDebugCam(V, g_0_sourcePort_f, &ds_buff_store_cam, p_trace->pkt.user.src_port);
                SetBufStoreDebugCam(V, g_1_sourcePort_f, &ds_buff_store_cam, 0xFFFF);
            }
            value = 1;
            cmd = DRV_IOW(DbgOamHdrAdj_t, DbgOamHdrAdj_oamHdrAdjDebug1stPkt_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            cmd = DRV_IOW(DbgOamHdrAdj_t, DbgOamHdrAdj_oamHdrAdjDebugSessionEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            goto install_ipe;
        case CTC_DIAG_TRACE_POINT_TM_BS:
            if (CTC_BMP_ISSET(watch_key->key_mask_bmp, CTC_DIAG_WATCH_KEY_MASK_CHANNEL))
            {
                SetBufStoreDebugCam(V, g_0_channelId_f, &ds_buff_store_cam, watch_key->channel);
                SetBufStoreDebugCam(V, g_1_channelId_f, &ds_buff_store_cam, 0xFF);
            }
            if (CTC_BMP_ISSET(watch_key->key_mask_bmp, CTC_DIAG_WATCH_KEY_MASK_MC_GROUP))
            {
                dest_map = SYS_ENCODE_MCAST_IPE_DESTMAP(watch_key->mc_group);
                SetBufStoreDebugCam(V, g_0_destMap_f, &ds_buff_store_cam, dest_map & 0x7FFFF);
                SetBufStoreDebugCam(V, g_1_destMap_f, &ds_buff_store_cam, 0x7FFFF);
            }
            if (CTC_BMP_ISSET(watch_key->key_mask_bmp, CTC_DIAG_WATCH_KEY_MASK_DST_GPORT))
            {
                dest_map = SYS_ENCODE_DESTMAP(CTC_MAP_GPORT_TO_GCHIP(watch_key->dst_gport),
                                              CTC_MAP_GPORT_TO_LPORT(watch_key->dst_gport));
                SetBufStoreDebugCam(V, g_0_destMap_f, &ds_buff_store_cam, dest_map & 0x7FFFF);
                SetBufStoreDebugCam(V, g_1_destMap_f, &ds_buff_store_cam, 0x7FFFF);
            }
            if (CTC_BMP_ISSET(watch_key->key_mask_bmp, CTC_DIAG_WATCH_KEY_MASK_SRC_GPORT))
            {
                SetBufStoreDebugCam(V, g_0_sourcePort_f, &ds_buff_store_cam, watch_key->src_gport);
                SetBufStoreDebugCam(V, g_1_sourcePort_f, &ds_buff_store_cam, 0xFFFF);
            }
            else if (p_trace->mode == CTC_DIAG_TRACE_MODE_USER)
            {
                SetBufStoreDebugCam(V, g_0_sourcePort_f, &ds_buff_store_cam, p_trace->pkt.user.src_port);
                SetBufStoreDebugCam(V, g_1_sourcePort_f, &ds_buff_store_cam, 0xFFFF);
            }
            if (CTC_BMP_ISSET(watch_key->key_mask_bmp, CTC_DIAG_WATCH_KEY_MASK_SRC_LPORT))
            {
                SetBufStoreDebugCam(V, g_0_localPhyPort_f, &ds_buff_store_cam, watch_key->src_lport);
                SetBufStoreDebugCam(V, g_1_localPhyPort_f, &ds_buff_store_cam, 0xFF);
            }
            SetBufStoreDebugCam(V, g_0_isDebuggedPkt_f, &ds_buff_store_cam, 1);
            SetBufStoreDebugCam(V, g_1_isDebuggedPkt_f, &ds_buff_store_cam, 0x1);
            goto install_bsr;
        case CTC_DIAG_TRACE_POINT_TM_MET:
            value = 1;
            cmd = DRV_IOW(MetFifoCtl_t, MetFifoCtl_debugLookupEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            if (CTC_BMP_ISSET(watch_key->key_mask_bmp, CTC_DIAG_WATCH_KEY_MASK_DST_GPORT))
            {
                dest_map = SYS_ENCODE_DESTMAP(CTC_MAP_GPORT_TO_GCHIP(watch_key->dst_gport),
                                              CTC_MAP_GPORT_TO_LPORT(watch_key->dst_gport));
                dest_map |= (1 << 18);
                SetMetFifoDebugCam(V, g_0_destMap_f, &ds_met_fifo_cam, dest_map & 0x7FFFF);
                SetMetFifoDebugCam(V, g_1_destMap_f, &ds_met_fifo_cam, 0x7FFFF);
            }
            goto install_met;
        case CTC_DIAG_TRACE_POINT_EPE_ADJ:
            if (CTC_BMP_ISSET(watch_key->key_mask_bmp, CTC_DIAG_WATCH_KEY_MASK_CHANNEL))
            {
                SetEpeHdrAdjustDebugCam(V, g_0_channelId_f, &ds_epe_dbg_cam, watch_key->channel);
                SetEpeHdrAdjustDebugCam(V, g_1_channelId_f, &ds_epe_dbg_cam, 0xFF);
            }
            if (CTC_BMP_ISSET(watch_key->key_mask_bmp, CTC_DIAG_WATCH_KEY_MASK_SRC_LPORT))
            {
                SetEpeHdrAdjustDebugCam(V, g_0_localPhyPort_f, &ds_epe_dbg_cam, watch_key->src_lport);
                SetEpeHdrAdjustDebugCam(V, g_1_localPhyPort_f, &ds_epe_dbg_cam, 0xFF);
            }
            if (CTC_BMP_ISSET(watch_key->key_mask_bmp, CTC_DIAG_WATCH_KEY_MASK_SRC_GPORT))
            {
                SetEpeHdrAdjustDebugCam(V, g_0_sourcePort_f, &ds_epe_dbg_cam, watch_key->src_gport);
                SetEpeHdrAdjustDebugCam(V, g_1_sourcePort_f, &ds_epe_dbg_cam, 0xFFFF);
            }
            else if (p_trace->mode == CTC_DIAG_TRACE_MODE_USER)
            {
                SetBufStoreDebugCam(V, g_0_sourcePort_f, &ds_buff_store_cam, p_trace->pkt.user.src_port);
                SetBufStoreDebugCam(V, g_1_sourcePort_f, &ds_buff_store_cam, 0xFFFF);
            }
            if (CTC_BMP_ISSET(watch_key->key_mask_bmp, CTC_DIAG_WATCH_KEY_MASK_DST_GPORT))
            {
                dest_map = SYS_ENCODE_DESTMAP(CTC_MAP_GPORT_TO_GCHIP(watch_key->dst_gport),
                                              CTC_MAP_GPORT_TO_LPORT(watch_key->dst_gport));
                SetEpeHdrAdjustDebugCam(V, g_0_destMap_f, &ds_epe_dbg_cam, dest_map & 0x7FFFF);
                SetEpeHdrAdjustDebugCam(V, g_1_destMap_f, &ds_epe_dbg_cam, 0x7FFFF);
            }
            if (CTC_BMP_ISSET(watch_key->key_mask_bmp, CTC_DIAG_WATCH_KEY_MASK_MC_GROUP))
            {
                dest_map = SYS_ENCODE_MCAST_IPE_DESTMAP(watch_key->mc_group);
                SetEpeHdrAdjustDebugCam(V, g_0_destMap_f, &ds_epe_dbg_cam, dest_map & 0x7FFFF);
                SetEpeHdrAdjustDebugCam(V, g_1_destMap_f, &ds_epe_dbg_cam, 0x7FFFF);
            }
            goto install_epe;
        case CTC_DIAG_TRACE_POINT_OAM_ADJ:
            if (CTC_BMP_ISSET(watch_key->key_mask_bmp, CTC_DIAG_WATCH_KEY_MASK_MEP_INDEX))
            {
                uint32 ma_index = 0;
                value = watch_key->mep_index;
                cmd = DRV_IOW(DbgOamHdrAdj_t, DbgOamHdrAdj_oamHdrAdjMepIndex_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
                cmd = DRV_IOW(DbgOamApsSwitch_t, DbgOamApsSwitch_oamApsSwitchMepIndex_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
                cmd = DRV_IOW(DbgOamApsProcess_t, DbgOamApsProcess_oamApsProcessMepIndex_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
                value = 0;
                cmd = DRV_IOW(DbgOamHdrAdj_t, DbgOamHdrAdj_oamHdrAdjDebugSessionEn_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
                value = 1;
                cmd = DRV_IOW(DbgOamApsSwitch_t, DbgOamApsSwitch_oamApsSwitchDebugUseMepIdx_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
                cmd = DRV_IOW(DbgOamApsProcess_t, DbgOamApsProcess_oamApsProcessDebugUseMepIdx_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
                cmd = DRV_IOW(DbgOamHdrAdj_t, DbgOamHdrAdj_oamHdrAdjDebugUseMepIdx_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
                cmd = DRV_IOR(DsEthMep_t, DsEthMep_isBfd_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, watch_key->mep_index, cmd, &value));
                if (value)
                {
                    cmd = DRV_IOR(DsBfdMep_t, DsBfdMep_maIndex_f);
                }
                else
                {
                    cmd = DRV_IOR(DsEthMep_t, DsEthMep_maIndex_f);
                }
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, watch_key->mep_index, cmd, &ma_index));
                value = 1;
                cmd = DRV_IOW(DsMa_t, DsMa_debugSessionEn_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, ma_index, cmd, &value));
            }
            else
            {
                value = 1;
                cmd = DRV_IOW(DbgOamHdrAdj_t, DbgOamHdrAdj_oamHdrAdjDebugSessionEn_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
                value = 0;
                cmd = DRV_IOW(DbgOamHdrAdj_t, DbgOamHdrAdj_oamHdrAdjDebugUseMepIdx_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            }
            break;
        case CTC_DIAG_TRACE_POINT_OAM_TX:
            if (CTC_BMP_ISSET(watch_key->key_mask_bmp, CTC_DIAG_WATCH_KEY_MASK_MEP_INDEX))
            {
                uint32 ma_index = 0;
                value = watch_key->mep_index;
                cmd = DRV_IOW(DbgOamTxProc_t, DbgOamTxProc_oamTxProcMepIndex_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
                value = 1;
                cmd = DRV_IOW(DbgOamTxProc_t, DbgOamTxProc_oamTxProcDebugUseMepIdx_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
                cmd = DRV_IOR(DsEthMep_t, DsEthMep_isBfd_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, watch_key->mep_index, cmd, &value));
                if (value)
                {
                    cmd = DRV_IOR(DsBfdMep_t, DsBfdMep_maIndex_f);
                }
                else
                {
                    cmd = DRV_IOR(DsEthMep_t, DsEthMep_maIndex_f);
                }
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, watch_key->mep_index, cmd, &ma_index));
                value = 1;
                cmd = DRV_IOW(DsMa_t, DsMa_debugSessionEn_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, ma_index, cmd, &value));
            }
            else
            {
                value = 1;
                cmd = DRV_IOW(DbgOamTxProc_t, DbgOamTxProc_oamTxProcDebug1stPkt_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            }
            goto install_bsr;
            break;
        default:
            break;
    }
    return CTC_E_NONE;
install_ipe:
    if (p_trace->mode == CTC_DIAG_TRACE_MODE_USER)
    {
        SetIpeUserIdFlowCam(V, g_0_isDebuggedPkt_f, &ds_flow_cam, 1);
        SetIpeUserIdFlowCam(V, g_1_isDebuggedPkt_f, &ds_flow_cam, 0x1);
    }
install_bsr:
    value = 1;
    cmd = DRV_IOW(BufStoreDebugCtl_t, BufStoreDebugCtl_debugLookupEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    SetBufStoreDebugCam(V, g_0_isDebuggedPkt_f, &ds_buff_store_cam, 1);
    SetBufStoreDebugCam(V, g_1_isDebuggedPkt_f, &ds_buff_store_cam, 0x1);
install_met:
    value = 1;
    cmd = DRV_IOW(MetFifoCtl_t, MetFifoCtl_debugLookupEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    SetMetFifoDebugCam(V, g_0_isDebuggedPkt_f, &ds_met_fifo_cam, 1);
    SetMetFifoDebugCam(V, g_1_isDebuggedPkt_f, &ds_met_fifo_cam, 0x1);
install_epe:
    value = 1;
    cmd = DRV_IOW(EpeHdrAdjustChanCtl_t, EpeHdrAdjustChanCtl_debugIsFree_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    cmd = DRV_IOW(EpeHdrAdjustCtl_t, EpeHdrAdjustCtl_debugLookupEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    SetEpeHdrAdjustDebugCam(V, g_0_isDebuggedPkt_f, &ds_epe_dbg_cam, 1);
    SetEpeHdrAdjustDebugCam(V, g_1_isDebuggedPkt_f, &ds_epe_dbg_cam, 0x1);

    cmd = DRV_IOW(IpeUserIdFlowCam_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds_flow_cam));
    cmd = DRV_IOW(BufStoreDebugCam_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds_buff_store_cam));
    cmd = DRV_IOW(MetFifoDebugCam_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds_met_fifo_cam));
    cmd = DRV_IOW(EpeHdrAdjustDebugCam_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds_epe_dbg_cam));
    return CTC_E_NONE;
}


