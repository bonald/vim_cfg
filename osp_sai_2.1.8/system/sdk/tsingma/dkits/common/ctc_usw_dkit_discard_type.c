#include "sal.h"
#include "ctc_usw_dkit_discard_type.h"

const char *ctc_usw_dkit_get_reason_desc(ctc_dkit_discard_reason_id_t reason_id)
{
    static char  reason_desc[256+1] = {0};
    if (reason_id == CTC_DKIT_DISCARD_INVALIED)
    {
        return " ";
    }

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
        case EPE_HDR_ADJ_PKT_ERR:
            return "EPE_HDR_ADJ_PKT_ERR, Packet discard due to header adjust error(no use)";
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
        case EPE_PBB_CHK_DIS:
            return "EPE_PBB_CHK_DIS, PBB check discard";
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
        case EPE_DS_PLC_DIS:
            return "EPE_DS_PLC_DIS, Policing discard";
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
        case EPE_RESERVED3:
            return "EPE_RESERVED3, EPE reserved3";
        case EPE_SAME_IPDA_IPSA_DIS:
            return "EPE_SAME_IPDA_IPSA_DIS, Same MAC DA SA or IP DA SA";
        case EPE_SD_CHK_DISCARD:
            return "EPE_SD_CHK_DISCARD, Signal degrade discard";
        case EPE_TRILL_PLD_OP_DIS:
            return "EPE_TRILL_PLD_OP_DIS, TRILL payload operation discard(no use)";
        case EPE_PBB_CHK_FAIL_DIS:
            return "EPE_PBB_CHK_FAIL_DIS, PBB check fail";
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
        case EPE_RESERVED4:
            return "EPE_RESERVED4, EPE reserved4 discard";
        case EPE_RESERVED:
            return "EPE_RESERVED, EPE reserved discard";
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


const char *ctc_usw_dkit_get_sub_reason_desc(ctc_dkit_discard_sub_reason_id_t reason_id)
{
    static char  reason_desc[256+1] = {0};
    if (reason_id == CTC_DKIT_SUB_DISCARD_INVALIED)
    {
        return " ";
    }

    switch(reason_id)
    {
     case CTC_DKIT_SUB_DISCARD_AMBIGUOUS:
       return "the discard reason is ambiguous, try to use captured info";
     case CTC_DKIT_SUB_DISCARD_NEED_PARAM:
       return "please input filter param to get the detail reason";
     case CTC_DKIT_SUB_IPE_PARSER_LEN_ERR_USERID:
       return "parser length error at SCL";
     case CTC_DKIT_SUB_IPE_PARSER_LEN_ERR_ACL:
       return "parser length error at ACL";
     case CTC_DKIT_SUB_IPE_EXCEP2_DIS_BPDU:
       return "BPDU, MAC DA is 0x0180c2000000, redirect to CPU";
     case CTC_DKIT_SUB_IPE_EXCEP2_DIS_SLOW:
       return "SLOW protocol, ether type is 0x8809, redirect to CPU";
     case CTC_DKIT_SUB_IPE_EXCEP2_DIS_EAPOL:
       return "EAPOL, ether type is 0x888E, redirect to CPU";
     case CTC_DKIT_SUB_IPE_EXCEP2_DIS_LLDP:
       return "LLDP, ether type is 0x88CC, redirect to CPU";
     case CTC_DKIT_SUB_IPE_EXCEP2_DIS_L2ISIS:
       return "L2ISIS, ether type is 0x22F4, redirect to CPU";
     case CTC_DKIT_SUB_IPE_EXCEP2_DIS_CAM:
       return "MAC DA match in cam, redirect to CPU";
     case CTC_DKIT_SUB_IPE_RECEIVE_DIS_PORT:
       return "port receive is disable";
     case CTC_DKIT_SUB_IPE_RECEIVE_DIS_VLAN:
       return "vlan receive is disable";
     case CTC_DKIT_SUB_IPE_AFT_DIS_ALLOW_ALL:
       return "AFT is config as allow all packets";
     case CTC_DKIT_SUB_IPE_AFT_DIS_DROP_UNTAGGED:
       return "AFT is config as drop all untagged";
     case CTC_DKIT_SUB_IPE_AFT_DIS_DROP_TAGGED:
       return "AFT is config as drop all tagged";
     case CTC_DKIT_SUB_IPE_AFT_DIS_DROP_ALL:
       return "AFT is config as drop all (same with receiveEn = 0)";
     case CTC_DKIT_SUB_IPE_AFT_DIS_DROP_WITHOUT_2_TAG:
       return "AFT is config as drop packet without 2 vlan tag";
     case CTC_DKIT_SUB_IPE_AFT_DIS_DROP_WITH_2_TAG:
       return "AFT is config as drop all packet with 2 vlan tag";
     case CTC_DKIT_SUB_IPE_AFT_DIS_DROP_STAG:
       return "AFT is config as drop all S-VLAN tag";
     case CTC_DKIT_SUB_IPE_AFT_DIS_DROP_NO_STAG:
       return "AFT is config as drop all non S-VLAN tag";
     case CTC_DKIT_SUB_IPE_AFT_DIS_DROP_ONLY_STAG:
       return "AFT is config as drop only S-VLAN tag";
     case CTC_DKIT_SUB_IPE_AFT_DIS_PERMIT_ONLY_STAG:
       return "AFT is config as permit only S-VLAN tag";
     case CTC_DKIT_SUB_IPE_AFT_DIS_DROP_CTAG:
       return "AFT is config as drop all C-VLAN tag";
     case CTC_DKIT_SUB_IPE_AFT_DIS_DROP_NO_CTAG:
       return "AFT is config as drop all non C-VLAN tag";
     case CTC_DKIT_SUB_IPE_AFT_DIS_DROP_ONLY_CTAG:
       return "AFT is config as drop only C-VLAN tag";
     case CTC_DKIT_SUB_IPE_AFT_DIS_PERMIT_ONLY_CTAG:
       return "AFT is config as permit only C-VLAN tag";
     case CTC_DKIT_SUB_IPE_ILLEGAL_PKT_DIS_MCAST:
       return "Mcast MAC DA is config as discard";
     case CTC_DKIT_SUB_IPE_ILLEGAL_PKT_DIS_SAME_MAC:
       return "the packet has the same MAC DA and SA";
     case CTC_DKIT_SUB_IPE_ARP_DHCP_DIS_V4_DSCP:
       return "IPv4 DSCP packet discard";
     case CTC_DKIT_SUB_IPE_ARP_DHCP_DIS_V6_DSCP:
       return "IPv6 DSCP packet discard";
     case CTC_DKIT_SUB_IPE_ARP_DHCP_DIS_ARP:
       return "ARP packet discard";
     case CTC_DKIT_SUB_IPE_ARP_DHCP_DIS_FIP:
       return "FIP packet discard";
     case CTC_DKIT_SUB_IPE_ROUTE_MC_IPADD_DIS_V4:
       return "IPv4 mcast MAC DA and IP DA check fail";
     case CTC_DKIT_SUB_IPE_ROUTE_MC_IPADD_DIS_V6:
       return "IPv6 mcast MAC DA and IP DA check fail";
     case CTC_DKIT_SUB_IPE_ROUTE_MC_IPADD_DIS_PROTOCOL:
       return "IP Protocal Translation discard";
     case CTC_DKIT_SUB_IPE_SECURITY_CHK_DIS_PORT_MATCH:
       return "port match discard";
     case CTC_DKIT_SUB_IPE_SECURITY_CHK_DIS_MCAST:
       return "unknown mcast discard";
     case CTC_DKIT_SUB_IPE_SECURITY_CHK_DIS_UCAST:
       return "unknown ucast discard";
     case CTC_DKIT_SUB_IPE_SECURITY_CHK_DIS_BCAST:
       return "bcast discard";
     case CTC_DKIT_SUB_IPE_LEARNING_DIS_SRC_MAC:
       return "the MAC SA FDB is config as discard";
     case CTC_DKIT_SUB_IPE_LEARNING_DIS_SRC_PORT:
       return "the global source port is not config as it in the MAC SA FDB, they are ";
     case CTC_DKIT_SUB_IPE_LEARNING_LOGIC_DIS_SRC_PORT:
       return "the logic source port is not config as it in the MAC SA FDB, they are ";
     case CTC_DKIT_SUB_IPE_LEARNING_DIS_PORT_SECURITY:
       return "";
     case CTC_DKIT_SUB_IPE_LEARNING_DIS_VSI_SECURITY:
       return "";
     case CTC_DKIT_SUB_IPE_LEARNING_DIS_SYSTEM_SECURITY:
       return "";
     case CTC_DKIT_SUB_IPE_LEARNING_DIS_PROFILE_MAX:
       return "";
     case CTC_DKIT_SUB_IPE_FATAL_UC_IP_HDR_ERROR_OR_MARTION_ADDR:
       return "ucast IP version/header length/checksum error, redirect to CPU";
     case CTC_DKIT_SUB_IPE_FATAL_UC_IP_OPTIONS:
       return "ucast IPv4 header length>5/ IPv6 protocol=0/ Trill option!=0, redirect to CPU";
     case CTC_DKIT_SUB_IPE_FATAL_UC_GRE_UNKNOWN_OPT_PROTOCAL:
       return "ucast GRE protocol or option is unknown, redirect to CPU";
     case CTC_DKIT_SUB_IPE_FATAL_UC_ISATAP_SRC_ADDR_CHK_FAIL:
       return "ucast ISATAP source address check fail, redirect to CPU";
     case CTC_DKIT_SUB_IPE_FATAL_UC_IPTTL_CHK_FAIL:
       return "ucast TTL check fail, redirect to CPU";
     case CTC_DKIT_SUB_IPE_FATAL_UC_RPF_CHK_FAIL:
       return "ucast RPF check fail, redirect to CPU";
     case CTC_DKIT_SUB_IPE_FATAL_UC_OR_MC_MORERPF:
       return "ucast or mcast multi RPF check fail, redirect to CPU";
     case CTC_DKIT_SUB_IPE_FATAL_UC_LINKID_CHK_FAIL:
       return "ucast linkid check fail, redirect to CPU(no use)";
     case CTC_DKIT_SUB_IPE_FATAL_MPLS_LABEL_OUTOFRANGE:
       return "MPLS label out of range, redirect to CPU";
     case CTC_DKIT_SUB_IPE_FATAL_MPLS_SBIT_ERROR:
       return "MPLS sbit check fail, redirect to CPU";
     case CTC_DKIT_SUB_IPE_FATAL_MPLS_TTL_CHK_FAIL:
       return "MPLS TTL check fail, redirect to CPU";
     case CTC_DKIT_SUB_IPE_FATAL_FCOE_VIRTUAL_LINK_CHK_FAIL:
       return "FCOE RPF or MAC SA check fail, redirect to CPU";
     case CTC_DKIT_SUB_IPE_FATAL_IGMP_SNOOPING_PKT:
       return "IGMP snooping, redirect to CPU";
     case CTC_DKIT_SUB_IPE_FATAL_TRILL_OPTION:
       return "TRILL header check error, redirect to CPU";
     case CTC_DKIT_SUB_IPE_FATAL_TRILL_TTL_CHK_FAIL:
       return "TRILL TTL check fail, redirect to CPU";
     case CTC_DKIT_SUB_IPE_FATAL_VXLAN_OR_NVGRE_CHK_FAIL:
       return "VXLAN or NVGRE check fail, redirect to CPU";
     case CTC_DKIT_SUB_IPE_IP_HEADER_ERROR_DIS_IPV4_SAME_IP:
       return "IPv4 packet has the same IP DA and SA";
     case CTC_DKIT_SUB_IPE_IP_HEADER_ERROR_DIS_IPV6_SAME_IP:
       return "IPv6 packet has the same IP DA and SA";
     case CTC_DKIT_SUB_IPE_IP_HEADER_ERROR_DIS_TCP_L4_ERROR0:
       return "TCP SYN=0";
     case CTC_DKIT_SUB_IPE_IP_HEADER_ERROR_DIS_TCP_L4_ERROR1:
       return "TCP flags=0 and sequence number=0 ";
     case CTC_DKIT_SUB_IPE_IP_HEADER_ERROR_DIS_TCP_L4_ERROR2:
       return "TCP SYN=0, PSH=0, URG=0, and sequence number=0";
     case CTC_DKIT_SUB_IPE_IP_HEADER_ERROR_DIS_TCP_L4_ERROR3:
       return "TCP FIN=0, SYN=0";
     case CTC_DKIT_SUB_IPE_IP_HEADER_ERROR_DIS_TCP_L4_ERROR4:
       return "TCP souce port is equal to dest port";
     case CTC_DKIT_SUB_IPE_IP_HEADER_ERROR_DIS_UDP_L4_ERROR4:
       return "TCP souce port is equal to dest port";
     case CTC_DKIT_SUB_IPE_IP_HEADER_ERROR_DIS_IGMP_FRAG_ERROR:
       return "IGMP fragment check error";
     case CTC_DKIT_SUB_IPE_IP_HEADER_ERROR_DIS_GRE_VERSION_ERROR:
       return "GRE version check error";
     case CTC_DKIT_SUB_IPE_MPLSMC_CHECK_DIS_CONTEXT:
       return "mcast MPLS with context label MAC DA check fail";
     case CTC_DKIT_SUB_IPE_MPLSMC_CHECK_DIS:
       return "mcast MPLS without context label MAC DA check fail";
     case CTC_DKIT_SUB_IPE_TRILL_FILTER_ERR_TRILL_DIS:
       return "TRILL packet discard by source port";
     case CTC_DKIT_SUB_IPE_TRILL_FILTER_ERR_NON_TRILL_DIS:
       return "non-TRILL packet discard by source port";
     case CTC_DKIT_SUB_IPE_TRILL_FILTER_ERR_TRILL_UCAST:
       return "ucast TRILL packet without port MAC DA discard";
     case CTC_DKIT_SUB_IPE_L3_EXCEP_DIS_FRAG:
       return "VXLAN or NVGRE packet whose fraginfo should be 0, redirect to CPU";
     case CTC_DKIT_SUB_IPE_L3_EXCEP_DIS_ACH:
       return "ACH type is wrong for ACH OAM or control word is not 0, redirect to CPU";
     case CTC_DKIT_SUB_IPE_L3_EXCEP_DIS_ROUTE:
       return "the IP is config as redirect to CPU";
     case CTC_DKIT_SUB_IPE_TRILL_VERSION_CHK_DIS_VERSION:
       return "TRILL packet version check fail, the version should be ";
     case CTC_DKIT_SUB_IPE_TRILL_VERSION_CHK_DIS_BFD_MCAST:
       return "mcast TRILL packet discard";
     case CTC_DKIT_SUB_IPE_TRILL_VERSION_CHK_DIS_BFD_MULTIHOP:
       return "multihop TRILL BFD packet discard, the TTL should >";
     case CTC_DKIT_SUB_IPE_TRILL_VERSION_CHK_DIS_BFD_1HOP:
       return "1hop TRILL BFD packet discard, the TTL should be 0x3F";
     case CTC_DKIT_SUB_IPE_TRILL_VERSION_CHK_DIS_INNER_VLAN_INVALID:
       return "TRILL packet discard, the inner vlan id should not be 0xFFF or 0";
     case CTC_DKIT_SUB_IPE_TRILL_VERSION_CHK_DIS_INNER_VLAN_ABSENT:
       return "TRILL packet without inner vlan discard";
     case CTC_DKIT_SUB_IPE_PTP_EXCEP_DIS_MESSAGE_DIS:
       return "PTP message config as discard, message type is ";
     case CTC_DKIT_SUB_IPE_PTP_EXCEP_DIS_V1:
       return "PTPv1 message discard, redirect to CPU";
     case CTC_DKIT_SUB_IPE_PTP_EXCEP_DIS_HIGH_VERSION:
       return "PTP high version message discard, redirect to CPU";
     case CTC_DKIT_SUB_IPE_PTP_EXCEP_DIS_UNKNOWN:
       return "unkonwn PTP message in MPLS tunnel discard";
     case CTC_DKIT_SUB_IPE_OAM_NOT_FIND_DIS_CONFLICT_ETHER:
       return "ether OAM hash lookup conflict, key: globalSrcPort, vlanId, isFid are ";
     case CTC_DKIT_SUB_IPE_OAM_NOT_FIND_DIS_EDGE_PORT:
       return "ether OAM packet discard on edge port, key: globalSrcPort, vlanId, isFid are ";
     case CTC_DKIT_SUB_IPE_OAM_FILTER_DIS_VLAN:
       return "ether OAM packet discard by vlan filtering, dsvlan index=";
     case CTC_DKIT_SUB_IPE_OAM_FILTER_DIS_STP:
       return "ether OAM packet discard by STP filtering";
     case CTC_DKIT_SUB_IPE_NO_MEP_MIP_DIS_ETHER:
       return "ether OAM not find MEP or MIP, key: globalSrcPort, vlanId, isFid are ";
     case CTC_DKIT_SUB_IPE_NO_MEP_MIP_DIS_ACH_SECTION_PORT:
       return "ACH port based section OAM not find MEP or MIP, key: localPhyPort is ";
     case CTC_DKIT_SUB_IPE_NO_MEP_MIP_DIS_ACH_SECTION_INTF:
       return "ACH interface based section OAM not find MEP or MIP, key: interfaceId is ";
     case CTC_DKIT_SUB_IPE_NO_MEP_MIP_DIS_ACH:
       return "ACH OAM not find MEP or MIP, key: mplsOamIndex is ";
     case CTC_DKIT_SUB_IPE_NO_MEP_MIP_DIS_MPLS_BFD:
       return "MPLS BFD not find MEP, key: bfdMyDiscriminator is ";
     case CTC_DKIT_SUB_IPE_NO_MEP_MIP_DIS_IP_BFD:
       return "IP BFD not find MEP, key: bfdMyDiscriminator is ";
     case CTC_DKIT_SUB_IPE_NO_MEP_MIP_DIS_TRILL_BFD:
       return "TRILL BFD not find MEP";
     case CTC_DKIT_SUB_IPE_ICMP_ERR_MSG_DIS_IPV4:
       return "IPv4 ICMP, l4 source port is 0x03XX or 0x04XX or 0x11XX or 0x12XX, redirect to CPU";
     case CTC_DKIT_SUB_IPE_ICMP_ERR_MSG_DIS_IPV6:
       return "IPv6 ICMP, l4 source port is 0, redirect to CPU";
     case CTC_DKIT_SUB_IPE_ICMP_ERR_MSG_DIS_PT:
       return "ICMP packet discard in PT scenario, redirect to CPU";
     case CTC_DKIT_SUB_IPE_VXLAN_NVGRE_CHK_FAIL_VXLAN:
       return "the VxLAN tunnel is not exist on the Gateway device";
     case CTC_DKIT_SUB_IPE_VXLAN_NVGRE_CHK_FAIL_NVGRE:
       return "the NvGRE tunnel is not exist on the Gateway device";
     case CTC_DKIT_SUB_IPE_VXLAN_NVGRE_INNER_VTAG_CHK_DIS_VXLAN:
       return "VXLAN packet with inner VTAG discard";
     case CTC_DKIT_SUB_IPE_VXLAN_NVGRE_INNER_VTAG_CHK_DIS_NVGRE:
       return "VXLAN packet with inner VTAG discard";
     case CTC_DKIT_SUB_IPE_PORT_MAC_CHECK_DIS_DA:
       return "MAC DA is in port MAC range";
     case CTC_DKIT_SUB_IPE_PORT_MAC_CHECK_DIS_SA:
       return "MAC SA is in port MAC range";
     case CTC_DKIT_SUB_IPE_FWD_PORT:
       return "";
     case CTC_DKIT_SUB_IPE_FWD_USERID:
       return "";
     case CTC_DKIT_SUB_IPE_FWD_TUNNEL:
       return "the packet forward by tunnel";
     case CTC_DKIT_SUB_IPE_FWD_TUNNEL_ECMP:
       return "the packet forward by tunnel, and then do ECMP, ECMP group id is ";
     case CTC_DKIT_SUB_IPE_FWD_TUNNEL_IP:
       return "the packet forward by IP after tunnel process";
     case CTC_DKIT_SUB_IPE_FWD_TUNNEL_IP_ECMP:
       return "the packet forward by IP after tunnel process, and then do ECMP, ECMP group id is ";
     case CTC_DKIT_SUB_IPE_FWD_TUNNEL_BRIDGE:
       return "the packet forward by MAC after tunnel process";
     case CTC_DKIT_SUB_IPE_FWD_TUNNEL_BRIDGE_ECMP:
       return "the packet forward by MAC after tunnel process, and then do ECMP, ECMP group id is ";
     case CTC_DKIT_SUB_IPE_FWD_MPLS:
       return "the packet forward by mpls";
     case CTC_DKIT_SUB_IPE_FWD_MPLS_ECMP:
       return "the packet forward by mpls, and the do ECMP, ECMP group id is";
     case CTC_DKIT_SUB_IPE_FWD_MPLS_IP:
       return "the packet forward by IP after mpls process";
     case CTC_DKIT_SUB_IPE_FWD_MPLS_IP_ECMP:
       return "the packet forward by IP after mpls process, and then do ECMP, ECMP group id is ";
     case CTC_DKIT_SUB_IPE_FWD_MPLS_BRIDGE:
       return "the packet forward by MAC after mpls process";
     case CTC_DKIT_SUB_IPE_FWD_MPLS_BRIDGE_ECMP:
       return "the packet forward by MAC after mpls process, and then do ECMP, ECMP group id is ";
     case CTC_DKIT_SUB_IPE_FWD_ACL:
       return "";
     case CTC_DKIT_SUB_IPE_FWD_ROUTE:
       return "the packet forward by IP";
     case CTC_DKIT_SUB_IPE_FWD_ROUTE_ECMP:
       return "the packet forward by IP, and then do ECMP, ECMP group id is ";
     case CTC_DKIT_SUB_IPE_FWD_ROUTE_BRIDGE:
       return "the packet forward by MAC after IP process";
     case CTC_DKIT_SUB_IPE_FWD_ROUTE_BRIDGE_ECMP:
       return "the packet forward by MAC after IP process, and then do ECMP, ECMP group id is ";
     case CTC_DKIT_SUB_IPE_FWD_FCOE:
       return "the packet forward by FCOE";
     case CTC_DKIT_SUB_IPE_FWD_TRILL:
       return "the packet forward by TRILL";
     case CTC_DKIT_SUB_IPE_FWD_BRIDGE:
       return "the packet forward by MAC and hit a FDB entry";
     case CTC_DKIT_SUB_IPE_FWD_BRIDGE_ECMP:
       return "the packet forward by MAC and hit a FDB entry, and then do ECMP, ECMP group id is ";
     case CTC_DKIT_SUB_IPE_FWD_BRIDGE_NOT_HIT:
       return "the packet forward by MAC but not hit FDB entry";
     case CTC_DKIT_SUB_IPE_FWD_BRIDGE_NOT_HIT_ECMP:
       return "the packet forward by MAC but use default FDB entry, and then do ECMP, ECMP group id is ";
     case CTC_DKIT_SUB_EPE_PORT_ISO_DIS_PVLAN_P2X:
       return "Pvlan isolation discard, from promiscuous port";
     case CTC_DKIT_SUB_EPE_PORT_ISO_DIS_PVLAN_C2C:
       return "Pvlan isolation discard, from community port to community port, source and dest community id are ";
     case CTC_DKIT_SUB_EPE_PORT_ISO_DIS_PVLAN_X2P:
       return "Pvlan isolation discard, to promiscuous port";
     case CTC_DKIT_SUB_EPE_PORT_ISO_DIS_UNKNOWN_UC_ISOLATION:
       return "port isolation discard, unknown ucast isolation, source and dest isolation id are ";
     case CTC_DKIT_SUB_EPE_PORT_ISO_DIS_KNOWN_UC_ISOLATION:
       return "port isolation discard, known ucast isolation, source and dest isolation id are ";
     case CTC_DKIT_SUB_EPE_PORT_ISO_DIS_UNKNOWN_MC_ISOLATION:
       return "port isolation discard, unknown mcast isolation, source and dest isolation id are ";
     case CTC_DKIT_SUB_EPE_PORT_ISO_DIS_KNOWN_MC_ISOLATION:
       return "port isolation discard, known mcast isolation, source and dest isolation id are ";
     case CTC_DKIT_SUB_EPE_PORT_ISO_DIS_BC_ISOLATION:
       return "port isolation discard, bcast isolation, source and dest isolation id are ";
     case CTC_DKIT_SUB_EPE_VPLS_HORIZON_SPLIT_DIS_HORIZON:
       return "VPLS horrizon split discard";
     case CTC_DKIT_SUB_EPE_VPLS_HORIZON_SPLIT_DIS_E_TREE:
       return "E-tree discard";
     case CTC_DKIT_SUB_EPE_UC_MC_FLOODING_DIS_UNKNOWN_UC:
       return "unknown ucast discard";
     case CTC_DKIT_SUB_EPE_UC_MC_FLOODING_DIS_KNOWN_UC:
       return "known ucast discard";
     case CTC_DKIT_SUB_EPE_UC_MC_FLOODING_DIS_UNKNOWN_MC:
       return "unknown mcast discard";
     case CTC_DKIT_SUB_EPE_UC_MC_FLOODING_DIS_KNOWN_MC:
       return "known mcast discard";
     case CTC_DKIT_SUB_EPE_UC_MC_FLOODING_DIS_BC:
       return "bcast discard";
     case CTC_DKIT_SUB_EPE_TTL_FAIL_ROUTE_MCAST:
       return "new TTL is too little after decrease 1 when do route operation, new TTL and threshold are ";
     case CTC_DKIT_SUB_EPE_TTL_FAIL_ROUTE_TTL_0:
       return "new TTL is 0 after decrease 1 when do route operation";
     case CTC_DKIT_SUB_EPE_TTL_FAIL_MPLS:
       return "new TTL is 0 after decrease 1 when do MPLS operation";
     case CTC_DKIT_SUB_EPE_TTL_FAIL_TRILL:
       return "new TTL is 0 after decrease 1 when do TRILL operation";
     case CTC_DKIT_SUB_EPE_NAT_PT_ICMP_ERR_IPV4:
       return "IPv4 ICMP, l4 source port is 0x03XX or 0x04XX or 0x11XX or 0x12XX, redirect to CPU";
     case CTC_DKIT_SUB_EPE_NAT_PT_ICMP_ERR_IPV6:
       return "IPv6 ICMP, l4 source port is 0, redirect to CPU";
     case CTC_DKIT_SUB_EPE_OAM_FILTERING_DIS_EDGE:
       return "up MEP send ether OAM packet to egde port discard";
     case CTC_DKIT_SUB_EPE_OAM_FILTERING_DIS_LINK:
       return "link OAM can not forward";
     case CTC_DKIT_SUB_EPE_OAM_FILTERING_DIS_VLAN:
       return "ether OAM packet discard by vlan filtering, dsvlan index=";
     case CTC_DKIT_SUB_EPE_OAM_FILTERING_DIS_STP:
       return "ether OAM packet discard by STP filtering";
     case CTC_DKIT_SUB_EPE_OAM_FILTERING_DIS_PORT:
       return "ether OAM packet discard by port transmit disable";
     case CTC_DKIT_SUB_EPE_SAME_IPDA_IPSA_DIS_MAC:
       return "same MAC discard";
     case CTC_DKIT_SUB_EPE_SAME_IPDA_IPSA_DIS_IP:
       return "same IP discard";
     case CTC_DKIT_SUB_EPE_DEST_VLAN_PTR_DIS_1FFE:
       return "invalid dest vlan index 0x1FFE";
     case CTC_DKIT_SUB_EPE_DEST_VLAN_PTR_DIS_1FFD:
       return "invalid dest vlan index 0x1FFD";
     case CTC_DKIT_SUB_OAM_EXCE_NON:
       return "None";
     case CTC_DKIT_SUB_OAM_EXCE_ETH_INVALID:
       return "ETH OAM (Y1731)PDU invalid /CSF interval mismatch /MD level lower than passive MEP MD level to CPU/MIP received non-process oam PDU/ Configure error";
     case CTC_DKIT_SUB_OAM_EXCE_ETH_LB:
       return "Send equal LBM/LBR to CPU ";
     case CTC_DKIT_SUB_OAM_EXCE_ETH_LT:
       return "Send equal LTM/LTR to CPU ";
     case CTC_DKIT_SUB_OAM_EXCE_ETH_LM:
       return "Send equal LMM/LmR to CPU ";
     case CTC_DKIT_SUB_OAM_EXCE_ETH_DM:
       return "ETH_1DM/ETH_DMM/ETH_DMR to CPU ";
     case CTC_DKIT_SUB_OAM_EXCE_ETH_TST:
       return "ETH TST to CPU ";
     case CTC_DKIT_SUB_OAM_EXCE_ETH_APS:
       return "APS PDU to CPU(EthAPS & RAPS) ";
     case CTC_DKIT_SUB_OAM_EXCE_ETH_SCC:
       return "ETH_SCC to CPU ";
     case CTC_DKIT_SUB_OAM_EXCE_ETH_MCC:
       return "ETH_MCC to CPU ";
     case CTC_DKIT_SUB_OAM_EXCE_ETH_CSF:
       return "ETH_CSF to CPU ";
     case CTC_DKIT_SUB_OAM_EXCE_ETH_BIG_CCM:
       return "Big ccm to CPU/mep cfg in sw ";
     case CTC_DKIT_SUB_OAM_EXCE_ETH_LEARN_CCM:
       return "Learning CCM to CPU ";
     case CTC_DKIT_SUB_OAM_EXCE_ETH_DEFECT:
       return "someRDIdefect/someMACstatusDefect/lowCcm/Maid mismatch/ UnExcepect MEP-Rmep MisMatch (P2P mode)/CCM sequence number error /Source MAC mismatch/Unexp Interval-CCM interval mismatch to CPU/PBT OAM PresentTraffic Mismatch";
     case CTC_DKIT_SUB_OAM_EXCE_PBX_OAM:
       return "PBB/PBT OAM ";
     case CTC_DKIT_SUB_OAM_EXCE_ETH_HIGH_VER:
       return "High version oam send to cpu ";
     case CTC_DKIT_SUB_OAM_EXCE_ETH_TLV:
       return "Ether CCM TLV Option ";
     case CTC_DKIT_SUB_OAM_EXCE_ETH_OTHER:
       return "Other ETH OAM PDU to CPU ";
     case CTC_DKIT_SUB_OAM_EXCE_BFD_INVALID:
       return "BFD PDU invalid ";
     case CTC_DKIT_SUB_OAM_EXCE_BFD_LEARN:
       return "Learning BFD send to CPU (ParserResult.YourDiscr == 0 ) ";
     case CTC_DKIT_SUB_OAM_EXCE_BIG_BFD:
       return "Big bfd to CPU/mep cfg in sw ";
     case CTC_DKIT_SUB_OAM_EXCE_BFD_TIMER_NEG:
       return "BFD timer Negotiation packet to CPU ";
     case CTC_DKIT_SUB_OAM_EXCE_TP_SCC:
       return "MPLS TP SCC to CPU ";
     case CTC_DKIT_SUB_OAM_EXCE_TP_MCC:
       return "MPLS TP MCC to CPU ";
     case CTC_DKIT_SUB_OAM_EXCE_TP_CSF:
       return "MPLS TP CSF to CPU ";
     case CTC_DKIT_SUB_OAM_EXCE_TP_LB:
       return "MPLS TP LB to CPU ";
     case CTC_DKIT_SUB_OAM_EXCE_TP_DLM:
       return "MPLS TP DLM to cpu ";
     case CTC_DKIT_SUB_OAM_EXCE_TP_DM:
       return "DM/DLMDM to cpu ";
     case CTC_DKIT_SUB_OAM_EXCE_TP_FM:
       return "MPLS TP FM to CPU ";
     case CTC_DKIT_SUB_OAM_EXCE_BFD_OTHER:
       return "Other BFD OAM PDU to CPU";
     case CTC_DKIT_SUB_OAM_EXCE_BFD_DISC_MISMATCH:
       return "BFD discriminator mismatch";
     case CTC_DKIT_SUB_OAM_EXCE_ETH_SM:
       return "Send equal SLM/SLR to CPU";
     case CTC_DKIT_SUB_OAM_EXCE_TP_CV:
       return "MPLS TP CV to CPU";
      default:
      sal_sprintf(reason_desc,"Sub reason id:%d",reason_id);
      return reason_desc;
    }

}

