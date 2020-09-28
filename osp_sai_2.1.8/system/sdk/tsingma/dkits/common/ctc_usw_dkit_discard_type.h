#ifndef _CTC_DKIT_GOLDENGATE_DISCARD_TYPE_H_
#define _CTC_DKIT_GOLDENGATE_DISCARD_TYPE_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "sal.h"

enum ctc_dkit_discard_reason_id_e
{

    CTC_DKIT_DISCARD_INVALIED              ,

   /*-----------------------  IPE   -----------------------*/
    IPE_START                              ,

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
    EPE_START                              ,
    EPE_FEATURE_START = EPE_START,
    EPE_HDR_ADJ_DESTID_DIS = EPE_FEATURE_START,/**EPE_HDR_ADJ_DESTID_DIS, Packet discard by drop channel*/
    EPE_HDR_ADJ_PKT_ERR                    ,/**EPE_HDR_ADJ_PKT_ERR, Packet discard due to header adjust error(no use)*/
    EPE_HDR_ADJ_REMOVE_ERR                 ,/**EPE_HDR_ADJ_REMOVE_ERR, Packet discard due to remove bytes error at header adjust(offset>144B)*/
    EPE_DS_DESTPHYPORT_DSTID_DIS           ,/**EPE_DS_DESTPHYPORT_DSTID_DIS, Egress port discard*/
    EPE_PORT_ISO_DIS                       ,/**EPE_PORT_ISO_DIS, Port isolate or Pvlan discard*/
    EPE_DS_VLAN_TRANSMIT_DIS               ,/**EPE_DS_VLAN_TRANSMIT_DIS, Packet discard due to port/vlan transmit disable*/
    EPE_BRG_TO_SAME_PORT_DIS               ,/**EPE_BRG_TO_SAME_PORT_DIS, Packet discard due to bridge to same port(no use)*/
    EPE_VPLS_HORIZON_SPLIT_DIS             ,/**EPE_VPLS_HORIZON_SPLIT_DIS, VPLS split horizon split or E-tree discard*/
    EPE_VLAN_FILTER_DIS                    ,/**EPE_VLAN_FILTER_DIS, Egress vlan filtering discard*/
    EPE_STP_DIS                            ,/**EPE_STP_DIS, STP discard*/
    EPE_PARSER_LEN_ERR                     ,/**EPE_PARSER_LEN_ERR, Packet discard due to parser length error*/
    EPE_PBB_CHK_DIS                        ,/**EPE_PBB_CHK_DIS, PBB check discard*/
    EPE_UC_MC_FLOODING_DIS                 ,/**EPE_UC_MC_FLOODING_DIS, Unkown-unicast/known-unicast/unkown-mcast/known-mcast/broadcast discard*/
    EPE_OAM_802_3_DIS                      ,/**EPE_OAM_802_3_DIS, Discard non-EFM OAM packet*/
    EPE_TTL_FAIL                           ,/**EPE_TTL_FAIL, TTL check failed*/
    EPE_REMOTE_MIRROR_ESCAP_DIS            ,/**EPE_REMOTE_MIRROR_ESCAP_DIS, Packet discard due to remote mirror filtering*/
    EPE_TUNNEL_MTU_CHK_DIS                 ,/**EPE_TUNNEL_MTU_CHK_DIS, Packet discard due to tunnel MTU check*/
    EPE_INTERF_MTU_CHK_DIS                 ,/**EPE_INTERF_MTU_CHK_DIS, Packet discard due to interface MTU check*/
    EPE_LOGIC_PORT_CHK_DIS                 ,/**EPE_LOGIC_PORT_CHK_DIS, Packet discard due to logic source port is equal to dest port*/
    EPE_DS_ACL_DIS                         ,/**EPE_DS_ACL_DIS, Packet discard due to ACL deny*/
    EPE_DS_VLAN_XLATE_DIS                  ,/**EPE_DS_VLAN_XLATE_DIS, Packet discard due to SCL deny*/
    EPE_DS_PLC_DIS                         ,/**EPE_DS_PLC_DIS, Policing discard*/
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
    EPE_RESERVED3                          ,/**EPE_RESERVED3, EPE reserved3*/
    EPE_SAME_IPDA_IPSA_DIS                 ,/**EPE_SAME_IPDA_IPSA_DIS, Same MAC DA SA or IP DA SA*/
    EPE_SD_CHK_DISCARD                     ,/**EPE_SD_CHK_DISCARD, Signal degrade discard*/
    EPE_TRILL_PLD_OP_DIS                   ,/**EPE_TRILL_PLD_OP_DIS, TRILL payload operation discard(no use)*/
    EPE_PBB_CHK_FAIL_DIS                   ,/**EPE_PBB_CHK_FAIL_DIS, PBB check fail*/
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
    EPE_RESERVED4                          ,/**EPE_RESERVED4, EPE reserved4 discard*/
    EPE_RESERVED                           ,/**EPE_RESERVED, EPE reserved discard*/
    EPE_FEATURE_END = EPE_RESERVED,
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
    CTC_DKIT_DISCARD_MAX,
};
typedef enum ctc_dkit_discard_reason_id_e ctc_dkit_discard_reason_id_t;

enum ctc_dkit_discard_sub_reason_id_e
{
    CTC_DKIT_SUB_DISCARD_INVALIED = 0x0,/**< no discard*/
    CTC_DKIT_SUB_DISCARD_AMBIGUOUS = 0x1,/**< the discard reason is ambiguous, try to use captured info*/
    CTC_DKIT_SUB_DISCARD_NEED_PARAM = 0x2,/**< please input filter param to get the detail reason*/

    /*IPE*/
    CTC_DKIT_SUB_IPE_PARSER_LEN_ERR_USERID,/**< parser length error at SCL*/
    CTC_DKIT_SUB_IPE_PARSER_LEN_ERR_ACL,/**< parser length error at ACL*/

    CTC_DKIT_SUB_IPE_EXCEP2_DIS_BPDU,/**< BPDU, MAC DA is 0x0180c2000000, redirect to CPU*/
    CTC_DKIT_SUB_IPE_EXCEP2_DIS_SLOW,/**< SLOW protocol, ether type is 0x8809, redirect to CPU*/
    CTC_DKIT_SUB_IPE_EXCEP2_DIS_EAPOL,/**< EAPOL, ether type is 0x888E, redirect to CPU*/
    CTC_DKIT_SUB_IPE_EXCEP2_DIS_LLDP,/**< LLDP, ether type is 0x88CC, redirect to CPU*/
    CTC_DKIT_SUB_IPE_EXCEP2_DIS_L2ISIS,/**< L2ISIS, ether type is 0x22F4, redirect to CPU*/
    CTC_DKIT_SUB_IPE_EXCEP2_DIS_CAM,/**< MAC DA match in cam, redirect to CPU*/

    CTC_DKIT_SUB_IPE_RECEIVE_DIS_PORT,/**< port receive is disable*/
    CTC_DKIT_SUB_IPE_RECEIVE_DIS_VLAN,/**< vlan receive is disable*/

    CTC_DKIT_SUB_IPE_AFT_DIS_ALLOW_ALL,/**< AFT is config as allow all packets*/
    CTC_DKIT_SUB_IPE_AFT_DIS_DROP_UNTAGGED,/**< AFT is config as drop all untagged*/
    CTC_DKIT_SUB_IPE_AFT_DIS_DROP_TAGGED,/**< AFT is config as drop all tagged*/
    CTC_DKIT_SUB_IPE_AFT_DIS_DROP_ALL,/**< AFT is config as drop all (same with receiveEn = 0)*/
    CTC_DKIT_SUB_IPE_AFT_DIS_DROP_WITHOUT_2_TAG,/**< AFT is config as drop packet without 2 vlan tag*/
    CTC_DKIT_SUB_IPE_AFT_DIS_DROP_WITH_2_TAG,/**< AFT is config as drop all packet with 2 vlan tag*/
    CTC_DKIT_SUB_IPE_AFT_DIS_DROP_STAG,/**< AFT is config as drop all S-VLAN tag*/
    CTC_DKIT_SUB_IPE_AFT_DIS_DROP_NO_STAG,/**< AFT is config as drop all non S-VLAN tag*/
    CTC_DKIT_SUB_IPE_AFT_DIS_DROP_ONLY_STAG,/**< AFT is config as drop only S-VLAN tag*/
    CTC_DKIT_SUB_IPE_AFT_DIS_PERMIT_ONLY_STAG,/**< AFT is config as permit only S-VLAN tag*/
    CTC_DKIT_SUB_IPE_AFT_DIS_DROP_CTAG,/**< AFT is config as drop all C-VLAN tag*/
    CTC_DKIT_SUB_IPE_AFT_DIS_DROP_NO_CTAG,/**< AFT is config as drop all non C-VLAN tag*/
    CTC_DKIT_SUB_IPE_AFT_DIS_DROP_ONLY_CTAG,/**< AFT is config as drop only C-VLAN tag*/
    CTC_DKIT_SUB_IPE_AFT_DIS_PERMIT_ONLY_CTAG,/**< AFT is config as permit only C-VLAN tag*/

    CTC_DKIT_SUB_IPE_ILLEGAL_PKT_DIS_MCAST,/**< Mcast MAC DA is config as discard*/
    CTC_DKIT_SUB_IPE_ILLEGAL_PKT_DIS_SAME_MAC,/**< the packet has the same MAC DA and SA*/

    CTC_DKIT_SUB_IPE_ARP_DHCP_DIS_V4_DSCP,/**< IPv4 DSCP packet discard*/
    CTC_DKIT_SUB_IPE_ARP_DHCP_DIS_V6_DSCP,/**< IPv6 DSCP packet discard*/
    CTC_DKIT_SUB_IPE_ARP_DHCP_DIS_ARP,/**< ARP packet discard*/
    CTC_DKIT_SUB_IPE_ARP_DHCP_DIS_FIP,/**< FIP packet discard*/

    CTC_DKIT_SUB_IPE_ROUTE_MC_IPADD_DIS_V4,/**< IPv4 mcast MAC DA and IP DA check fail*/
    CTC_DKIT_SUB_IPE_ROUTE_MC_IPADD_DIS_V6,/**< IPv6 mcast MAC DA and IP DA check fail*/
    CTC_DKIT_SUB_IPE_ROUTE_MC_IPADD_DIS_PROTOCOL,/**< IP Protocal Translation discard*/

    CTC_DKIT_SUB_IPE_SECURITY_CHK_DIS_PORT_MATCH,/**< port match discard*/
    CTC_DKIT_SUB_IPE_SECURITY_CHK_DIS_MCAST,/**< unknown mcast discard*/
    CTC_DKIT_SUB_IPE_SECURITY_CHK_DIS_UCAST,/**< unknown ucast discard*/
    CTC_DKIT_SUB_IPE_SECURITY_CHK_DIS_BCAST,/**< bcast discard*/

    CTC_DKIT_SUB_IPE_LEARNING_DIS_SRC_MAC,/**< the MAC SA FDB is config as discard*/
    CTC_DKIT_SUB_IPE_LEARNING_DIS_SRC_PORT,/**< the global source port is not config as it in the MAC SA FDB, they are */
    CTC_DKIT_SUB_IPE_LEARNING_LOGIC_DIS_SRC_PORT,/**< the logic source port is not config as it in the MAC SA FDB, they are */
    CTC_DKIT_SUB_IPE_LEARNING_DIS_PORT_SECURITY,/**< */
    CTC_DKIT_SUB_IPE_LEARNING_DIS_VSI_SECURITY,/**< */
    CTC_DKIT_SUB_IPE_LEARNING_DIS_SYSTEM_SECURITY,/**< */
    CTC_DKIT_SUB_IPE_LEARNING_DIS_PROFILE_MAX,/**< */

    CTC_DKIT_SUB_IPE_FATAL_UC_IP_HDR_ERROR_OR_MARTION_ADDR,/**< ucast IP version/header length/checksum error, redirect to CPU*/
    CTC_DKIT_SUB_IPE_FATAL_UC_IP_OPTIONS,/**< ucast IPv4 header length>5/ IPv6 protocol=0/ Trill option!=0, redirect to CPU*/
    CTC_DKIT_SUB_IPE_FATAL_UC_GRE_UNKNOWN_OPT_PROTOCAL,/**< ucast GRE protocol or option is unknown, redirect to CPU*/
    CTC_DKIT_SUB_IPE_FATAL_UC_ISATAP_SRC_ADDR_CHK_FAIL,/**< ucast ISATAP source address check fail, redirect to CPU*/
    CTC_DKIT_SUB_IPE_FATAL_UC_IPTTL_CHK_FAIL,/**< ucast TTL check fail, redirect to CPU*/
    CTC_DKIT_SUB_IPE_FATAL_UC_RPF_CHK_FAIL,/**< ucast RPF check fail, redirect to CPU*/
    CTC_DKIT_SUB_IPE_FATAL_UC_OR_MC_MORERPF,/**< ucast or mcast multi RPF check fail, redirect to CPU*/
    CTC_DKIT_SUB_IPE_FATAL_UC_LINKID_CHK_FAIL,/**< ucast linkid check fail, redirect to CPU(no use)*/
    CTC_DKIT_SUB_IPE_FATAL_MPLS_LABEL_OUTOFRANGE,/**< MPLS label out of range, redirect to CPU*/
    CTC_DKIT_SUB_IPE_FATAL_MPLS_SBIT_ERROR,/**< MPLS sbit check fail, redirect to CPU*/
    CTC_DKIT_SUB_IPE_FATAL_MPLS_TTL_CHK_FAIL,/**< MPLS TTL check fail, redirect to CPU*/
    CTC_DKIT_SUB_IPE_FATAL_FCOE_VIRTUAL_LINK_CHK_FAIL,/**< FCOE RPF or MAC SA check fail, redirect to CPU*/
    CTC_DKIT_SUB_IPE_FATAL_IGMP_SNOOPING_PKT,/**< IGMP snooping, redirect to CPU*/
    CTC_DKIT_SUB_IPE_FATAL_TRILL_OPTION,/**< TRILL header check error, redirect to CPU*/
    CTC_DKIT_SUB_IPE_FATAL_TRILL_TTL_CHK_FAIL,/**< TRILL TTL check fail, redirect to CPU*/
    CTC_DKIT_SUB_IPE_FATAL_VXLAN_OR_NVGRE_CHK_FAIL,/**< VXLAN or NVGRE check fail, redirect to CPU*/

    CTC_DKIT_SUB_IPE_IP_HEADER_ERROR_DIS_IPV4_SAME_IP,/**< IPv4 packet has the same IP DA and SA*/
    CTC_DKIT_SUB_IPE_IP_HEADER_ERROR_DIS_IPV6_SAME_IP,/**< IPv6 packet has the same IP DA and SA*/
    CTC_DKIT_SUB_IPE_IP_HEADER_ERROR_DIS_TCP_L4_ERROR0,/**< TCP SYN=0*/
    CTC_DKIT_SUB_IPE_IP_HEADER_ERROR_DIS_TCP_L4_ERROR1,/**< TCP flags=0 and sequence number=0 */
    CTC_DKIT_SUB_IPE_IP_HEADER_ERROR_DIS_TCP_L4_ERROR2,/**< TCP SYN=0, PSH=0, URG=0, and sequence number=0*/
    CTC_DKIT_SUB_IPE_IP_HEADER_ERROR_DIS_TCP_L4_ERROR3,/**< TCP FIN=0, SYN=0*/
    CTC_DKIT_SUB_IPE_IP_HEADER_ERROR_DIS_TCP_L4_ERROR4,/**< TCP souce port is equal to dest port*/
    CTC_DKIT_SUB_IPE_IP_HEADER_ERROR_DIS_UDP_L4_ERROR4,/**< TCP souce port is equal to dest port*/
    CTC_DKIT_SUB_IPE_IP_HEADER_ERROR_DIS_IGMP_FRAG_ERROR,/**< IGMP fragment check error*/
    CTC_DKIT_SUB_IPE_IP_HEADER_ERROR_DIS_GRE_VERSION_ERROR,/**< GRE version check error*/

    CTC_DKIT_SUB_IPE_MPLSMC_CHECK_DIS_CONTEXT,/**< mcast MPLS with context label MAC DA check fail*/
    CTC_DKIT_SUB_IPE_MPLSMC_CHECK_DIS,/**< mcast MPLS without context label MAC DA check fail*/

    CTC_DKIT_SUB_IPE_TRILL_FILTER_ERR_TRILL_DIS,/**< TRILL packet discard by source port*/
    CTC_DKIT_SUB_IPE_TRILL_FILTER_ERR_NON_TRILL_DIS,/**< non-TRILL packet discard by source port*/
    CTC_DKIT_SUB_IPE_TRILL_FILTER_ERR_TRILL_UCAST,/**< ucast TRILL packet without port MAC DA discard*/

    CTC_DKIT_SUB_IPE_L3_EXCEP_DIS_FRAG,/**< VXLAN or NVGRE packet whose fraginfo should be 0, redirect to CPU*/
    CTC_DKIT_SUB_IPE_L3_EXCEP_DIS_ACH,/**< ACH type is wrong for ACH OAM or control word is not 0, redirect to CPU*/
    CTC_DKIT_SUB_IPE_L3_EXCEP_DIS_ROUTE,/**< the IP is config as redirect to CPU*/

    CTC_DKIT_SUB_IPE_TRILL_VERSION_CHK_DIS_VERSION,/**< TRILL packet version check fail, the version should be */
    CTC_DKIT_SUB_IPE_TRILL_VERSION_CHK_DIS_BFD_MCAST,/**< mcast TRILL packet discard*/
    CTC_DKIT_SUB_IPE_TRILL_VERSION_CHK_DIS_BFD_MULTIHOP,/**< multihop TRILL BFD packet discard, the TTL should >*/
    CTC_DKIT_SUB_IPE_TRILL_VERSION_CHK_DIS_BFD_1HOP,/**< 1hop TRILL BFD packet discard, the TTL should be 0x3F*/
    CTC_DKIT_SUB_IPE_TRILL_VERSION_CHK_DIS_INNER_VLAN_INVALID,/**< TRILL packet discard, the inner vlan id should not be 0xFFF or 0*/
    CTC_DKIT_SUB_IPE_TRILL_VERSION_CHK_DIS_INNER_VLAN_ABSENT,/**< TRILL packet without inner vlan discard*/

    CTC_DKIT_SUB_IPE_PTP_EXCEP_DIS_MESSAGE_DIS,/**< PTP message config as discard, message type is */
    CTC_DKIT_SUB_IPE_PTP_EXCEP_DIS_V1,/**< PTPv1 message discard, redirect to CPU*/
    CTC_DKIT_SUB_IPE_PTP_EXCEP_DIS_HIGH_VERSION,/**< PTP high version message discard, redirect to CPU*/
    CTC_DKIT_SUB_IPE_PTP_EXCEP_DIS_UNKNOWN,/**< unkonwn PTP message in MPLS tunnel discard*/

    CTC_DKIT_SUB_IPE_OAM_NOT_FIND_DIS_CONFLICT_ETHER,/**< ether OAM hash lookup conflict, key: globalSrcPort, vlanId, isFid are */
    CTC_DKIT_SUB_IPE_OAM_NOT_FIND_DIS_EDGE_PORT,/**< ether OAM packet discard on edge port, key: globalSrcPort, vlanId, isFid are */

    CTC_DKIT_SUB_IPE_OAM_FILTER_DIS_VLAN,/**< ether OAM packet discard by vlan filtering, dsvlan index=*/
    CTC_DKIT_SUB_IPE_OAM_FILTER_DIS_STP,/**< ether OAM packet discard by STP filtering*/

    CTC_DKIT_SUB_IPE_NO_MEP_MIP_DIS_ETHER,/**< ether OAM not find MEP or MIP, key: globalSrcPort, vlanId, isFid are */
    CTC_DKIT_SUB_IPE_NO_MEP_MIP_DIS_ACH_SECTION_PORT,/**< ACH port based section OAM not find MEP or MIP, key: localPhyPort is */
    CTC_DKIT_SUB_IPE_NO_MEP_MIP_DIS_ACH_SECTION_INTF,/**< ACH interface based section OAM not find MEP or MIP, key: interfaceId is */
    CTC_DKIT_SUB_IPE_NO_MEP_MIP_DIS_ACH,/**< ACH OAM not find MEP or MIP, key: mplsOamIndex is */
    CTC_DKIT_SUB_IPE_NO_MEP_MIP_DIS_MPLS_BFD,/**< MPLS BFD not find MEP, key: bfdMyDiscriminator is */
    CTC_DKIT_SUB_IPE_NO_MEP_MIP_DIS_IP_BFD,/**< IP BFD not find MEP, key: bfdMyDiscriminator is */
    CTC_DKIT_SUB_IPE_NO_MEP_MIP_DIS_TRILL_BFD,/**< TRILL BFD not find MEP*/

    CTC_DKIT_SUB_IPE_ICMP_ERR_MSG_DIS_IPV4,/**< IPv4 ICMP, l4 source port is 0x03XX or 0x04XX or 0x11XX or 0x12XX, redirect to CPU*/
    CTC_DKIT_SUB_IPE_ICMP_ERR_MSG_DIS_IPV6,/**< IPv6 ICMP, l4 source port is 0, redirect to CPU*/
    CTC_DKIT_SUB_IPE_ICMP_ERR_MSG_DIS_PT,/**< ICMP packet discard in PT scenario, redirect to CPU*/

    CTC_DKIT_SUB_IPE_VXLAN_NVGRE_CHK_FAIL_VXLAN,/**< the VxLAN tunnel is not exist on the Gateway device*/
    CTC_DKIT_SUB_IPE_VXLAN_NVGRE_CHK_FAIL_NVGRE,/**< the NvGRE tunnel is not exist on the Gateway device*/

    CTC_DKIT_SUB_IPE_VXLAN_NVGRE_INNER_VTAG_CHK_DIS_VXLAN,/**< VXLAN packet with inner VTAG discard*/
    CTC_DKIT_SUB_IPE_VXLAN_NVGRE_INNER_VTAG_CHK_DIS_NVGRE,/**< VXLAN packet with inner VTAG discard*/

    CTC_DKIT_SUB_IPE_PORT_MAC_CHECK_DIS_DA,/**< MAC DA is in port MAC range*/
    CTC_DKIT_SUB_IPE_PORT_MAC_CHECK_DIS_SA,/**< MAC SA is in port MAC range*/

    CTC_DKIT_SUB_IPE_FWD_PORT,/**< */
    CTC_DKIT_SUB_IPE_FWD_USERID,/**< */
    CTC_DKIT_SUB_IPE_FWD_TUNNEL,/**< the packet forward by tunnel*/
    CTC_DKIT_SUB_IPE_FWD_TUNNEL_ECMP,/**< the packet forward by tunnel, and then do ECMP, ECMP group id is */
    CTC_DKIT_SUB_IPE_FWD_TUNNEL_IP,/**< the packet forward by IP after tunnel process*/
    CTC_DKIT_SUB_IPE_FWD_TUNNEL_IP_ECMP,/**< the packet forward by IP after tunnel process, and then do ECMP, ECMP group id is */
    CTC_DKIT_SUB_IPE_FWD_TUNNEL_BRIDGE,/**< the packet forward by MAC after tunnel process*/
    CTC_DKIT_SUB_IPE_FWD_TUNNEL_BRIDGE_ECMP,/**< the packet forward by MAC after tunnel process, and then do ECMP, ECMP group id is */
    CTC_DKIT_SUB_IPE_FWD_MPLS,/**< the packet forward by mpls*/
    CTC_DKIT_SUB_IPE_FWD_MPLS_ECMP,/**< the packet forward by mpls, and the do ECMP, ECMP group id is*/
    CTC_DKIT_SUB_IPE_FWD_MPLS_IP,/**< the packet forward by IP after mpls process*/
    CTC_DKIT_SUB_IPE_FWD_MPLS_IP_ECMP,/**< the packet forward by IP after mpls process, and then do ECMP, ECMP group id is */
    CTC_DKIT_SUB_IPE_FWD_MPLS_BRIDGE,/**< the packet forward by MAC after mpls process*/
    CTC_DKIT_SUB_IPE_FWD_MPLS_BRIDGE_ECMP,/**< the packet forward by MAC after mpls process, and then do ECMP, ECMP group id is */
    CTC_DKIT_SUB_IPE_FWD_ACL,/**< */
    CTC_DKIT_SUB_IPE_FWD_ROUTE,/**< the packet forward by IP*/
    CTC_DKIT_SUB_IPE_FWD_ROUTE_ECMP,/**< the packet forward by IP, and then do ECMP, ECMP group id is */
    CTC_DKIT_SUB_IPE_FWD_ROUTE_BRIDGE,/**< the packet forward by MAC after IP process*/
    CTC_DKIT_SUB_IPE_FWD_ROUTE_BRIDGE_ECMP,/**< the packet forward by MAC after IP process, and then do ECMP, ECMP group id is */
    CTC_DKIT_SUB_IPE_FWD_FCOE,/**< the packet forward by FCOE*/
    CTC_DKIT_SUB_IPE_FWD_TRILL,/**< the packet forward by TRILL*/
    CTC_DKIT_SUB_IPE_FWD_BRIDGE,/**< the packet forward by MAC and hit a FDB entry*/
    CTC_DKIT_SUB_IPE_FWD_BRIDGE_ECMP,/**< the packet forward by MAC and hit a FDB entry, and then do ECMP, ECMP group id is */
    CTC_DKIT_SUB_IPE_FWD_BRIDGE_NOT_HIT,/**< the packet forward by MAC but not hit FDB entry*/
    CTC_DKIT_SUB_IPE_FWD_BRIDGE_NOT_HIT_ECMP,/**< the packet forward by MAC but use default FDB entry, and then do ECMP, ECMP group id is */

    /*EPE*/
    CTC_DKIT_SUB_EPE_PORT_ISO_DIS_PVLAN_P2X,/**< Pvlan isolation discard, from promiscuous port*/
    CTC_DKIT_SUB_EPE_PORT_ISO_DIS_PVLAN_C2C,/**< Pvlan isolation discard, from community port to community port, source and dest community id are */
    CTC_DKIT_SUB_EPE_PORT_ISO_DIS_PVLAN_X2P,/**< Pvlan isolation discard, to promiscuous port*/
    CTC_DKIT_SUB_EPE_PORT_ISO_DIS_UNKNOWN_UC_ISOLATION,/**< port isolation discard, unknown ucast isolation, source and dest isolation id are */
    CTC_DKIT_SUB_EPE_PORT_ISO_DIS_KNOWN_UC_ISOLATION,/**< port isolation discard, known ucast isolation, source and dest isolation id are */
    CTC_DKIT_SUB_EPE_PORT_ISO_DIS_UNKNOWN_MC_ISOLATION,/**< port isolation discard, unknown mcast isolation, source and dest isolation id are */
    CTC_DKIT_SUB_EPE_PORT_ISO_DIS_KNOWN_MC_ISOLATION,/**< port isolation discard, known mcast isolation, source and dest isolation id are */
    CTC_DKIT_SUB_EPE_PORT_ISO_DIS_BC_ISOLATION,/**< port isolation discard, bcast isolation, source and dest isolation id are */

    CTC_DKIT_SUB_EPE_VPLS_HORIZON_SPLIT_DIS_HORIZON,/**< VPLS horrizon split discard*/
    CTC_DKIT_SUB_EPE_VPLS_HORIZON_SPLIT_DIS_E_TREE,/**< E-tree discard*/

    CTC_DKIT_SUB_EPE_UC_MC_FLOODING_DIS_UNKNOWN_UC,/**< unknown ucast discard*/
    CTC_DKIT_SUB_EPE_UC_MC_FLOODING_DIS_KNOWN_UC,/**< known ucast discard*/
    CTC_DKIT_SUB_EPE_UC_MC_FLOODING_DIS_UNKNOWN_MC,/**< unknown mcast discard*/
    CTC_DKIT_SUB_EPE_UC_MC_FLOODING_DIS_KNOWN_MC,/**< known mcast discard*/
    CTC_DKIT_SUB_EPE_UC_MC_FLOODING_DIS_BC,/**< bcast discard*/

    CTC_DKIT_SUB_EPE_TTL_FAIL_ROUTE_MCAST,/**< new TTL is too little after decrease 1 when do route operation, new TTL and threshold are */
    CTC_DKIT_SUB_EPE_TTL_FAIL_ROUTE_TTL_0,/**< new TTL is 0 after decrease 1 when do route operation*/
    CTC_DKIT_SUB_EPE_TTL_FAIL_MPLS,/**< new TTL is 0 after decrease 1 when do MPLS operation*/
    CTC_DKIT_SUB_EPE_TTL_FAIL_TRILL,/**< new TTL is 0 after decrease 1 when do TRILL operation*/

    CTC_DKIT_SUB_EPE_NAT_PT_ICMP_ERR_IPV4,/**< IPv4 ICMP, l4 source port is 0x03XX or 0x04XX or 0x11XX or 0x12XX, redirect to CPU*/
    CTC_DKIT_SUB_EPE_NAT_PT_ICMP_ERR_IPV6,/**< IPv6 ICMP, l4 source port is 0, redirect to CPU*/

    CTC_DKIT_SUB_EPE_OAM_FILTERING_DIS_EDGE,/**< up MEP send ether OAM packet to egde port discard*/
    CTC_DKIT_SUB_EPE_OAM_FILTERING_DIS_LINK,/**< link OAM can not forward*/
    CTC_DKIT_SUB_EPE_OAM_FILTERING_DIS_VLAN,/**< ether OAM packet discard by vlan filtering, dsvlan index=*/
    CTC_DKIT_SUB_EPE_OAM_FILTERING_DIS_STP,/**< ether OAM packet discard by STP filtering*/
    CTC_DKIT_SUB_EPE_OAM_FILTERING_DIS_PORT,/**< ether OAM packet discard by port transmit disable*/

    CTC_DKIT_SUB_EPE_SAME_IPDA_IPSA_DIS_MAC,/**< same MAC discard*/
    CTC_DKIT_SUB_EPE_SAME_IPDA_IPSA_DIS_IP,/**< same IP discard*/

    CTC_DKIT_SUB_EPE_DEST_VLAN_PTR_DIS_1FFE,/**< invalid dest vlan index 0x1FFE*/
    CTC_DKIT_SUB_EPE_DEST_VLAN_PTR_DIS_1FFD,/**< invalid dest vlan index 0x1FFD*/

    /*OAM*/
    CTC_DKIT_SUB_OAM_EXCE_NON,/**< None*/
    CTC_DKIT_SUB_OAM_EXCE_ETH_INVALID,    /**<ETH OAM (Y1731)PDU invalid /CSF interval mismatch /MD level lower than passive MEP MD level to CPU/MIP received non-process oam PDU/ Configure error*/
    CTC_DKIT_SUB_OAM_EXCE_ETH_LB,    /**< Send equal LBM/LBR to CPU */
    CTC_DKIT_SUB_OAM_EXCE_ETH_LT,    /**< Send equal LTM/LTR to CPU */
    CTC_DKIT_SUB_OAM_EXCE_ETH_LM,    /**< Send equal LMM/LmR to CPU */
    CTC_DKIT_SUB_OAM_EXCE_ETH_DM,    /**< ETH_1DM/ETH_DMM/ETH_DMR to CPU */
    CTC_DKIT_SUB_OAM_EXCE_ETH_TST,    /**< ETH TST to CPU */
    CTC_DKIT_SUB_OAM_EXCE_ETH_APS,    /**< APS PDU to CPU(EthAPS & RAPS) */
    CTC_DKIT_SUB_OAM_EXCE_ETH_SCC,    /**< ETH_SCC to CPU */
    CTC_DKIT_SUB_OAM_EXCE_ETH_MCC,    /**< ETH_MCC to CPU */
    CTC_DKIT_SUB_OAM_EXCE_ETH_CSF,    /**< ETH_CSF to CPU */
    CTC_DKIT_SUB_OAM_EXCE_ETH_BIG_CCM,    /**< Big ccm to CPU/mep cfg in sw */
    CTC_DKIT_SUB_OAM_EXCE_ETH_LEARN_CCM,    /**< Learning CCM to CPU */
    CTC_DKIT_SUB_OAM_EXCE_ETH_DEFECT,    /**< someRDIdefect/someMACstatusDefect/lowCcm/Maid mismatch/ UnExcepect MEP-Rmep MisMatch (P2P mode)/CCM sequence number error /Source MAC mismatch/Unexp Interval-CCM interval mismatch to CPU/PBT OAM PresentTraffic Mismatch*/
    CTC_DKIT_SUB_OAM_EXCE_PBX_OAM,    /**< PBB/PBT OAM */
    CTC_DKIT_SUB_OAM_EXCE_ETH_HIGH_VER,    /**< High version oam send to cpu */
    CTC_DKIT_SUB_OAM_EXCE_ETH_TLV,    /**< Ether CCM TLV Option */
    CTC_DKIT_SUB_OAM_EXCE_ETH_OTHER,    /**< Other ETH OAM PDU to CPU */
    CTC_DKIT_SUB_OAM_EXCE_BFD_INVALID,    /**< BFD PDU invalid */
    CTC_DKIT_SUB_OAM_EXCE_BFD_LEARN,    /**< Learning BFD send to CPU (ParserResult.YourDiscr == 0 ) */
    CTC_DKIT_SUB_OAM_EXCE_BIG_BFD,    /**< Big bfd to CPU/mep cfg in sw */
    CTC_DKIT_SUB_OAM_EXCE_BFD_TIMER_NEG,    /**< BFD timer Negotiation packet to CPU */
    CTC_DKIT_SUB_OAM_EXCE_TP_SCC,    /**< MPLS TP SCC to CPU */
    CTC_DKIT_SUB_OAM_EXCE_TP_MCC,    /**< MPLS TP MCC to CPU */
    CTC_DKIT_SUB_OAM_EXCE_TP_CSF,    /**< MPLS TP CSF to CPU */
    CTC_DKIT_SUB_OAM_EXCE_TP_LB,    /**< MPLS TP LB to CPU */
    CTC_DKIT_SUB_OAM_EXCE_TP_DLM,    /**< MPLS TP DLM to cpu */
    CTC_DKIT_SUB_OAM_EXCE_TP_DM,    /**< DM/DLMDM to cpu */
    CTC_DKIT_SUB_OAM_EXCE_TP_FM,    /**< MPLS TP FM to CPU */
    CTC_DKIT_SUB_OAM_EXCE_BFD_OTHER,    /**< Other BFD OAM PDU to CPU*/
    CTC_DKIT_SUB_OAM_EXCE_BFD_DISC_MISMATCH,/**< BFD discriminator mismatch*/
    CTC_DKIT_SUB_OAM_EXCE_ETH_SM, /**< Send equal SLM/SLR to CPU*/
    CTC_DKIT_SUB_OAM_EXCE_TP_CV, /**< MPLS TP CV to CPU*/

};
typedef enum ctc_dkit_discard_sub_reason_id_e ctc_dkit_discard_sub_reason_id_t;


extern const char*
ctc_usw_dkit_get_reason_desc(ctc_dkit_discard_reason_id_t reason_id);
extern const char*
ctc_usw_dkit_get_sub_reason_desc(ctc_dkit_discard_sub_reason_id_t reason_id);



#ifdef __cplusplus
}
#endif

#endif




