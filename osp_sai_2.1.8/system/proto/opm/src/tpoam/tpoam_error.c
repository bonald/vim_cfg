#include "proto.h"

#include "tpoam_error.h"


log_message_t tpoam_log_message[] =
{
    { 0, "Unrecognized Error Code" },
    { TPOAM_4_CHANGE_SF_REASONS, "Change SF reasons, former contained %s, current contains %s"},
    { TPOAM_4_CHANGE_AIS_REASONS, "Change AIS reasons, former contained %s, current contains %s"},
    { TPOAM_4_CHANGE_RDI_REASONS, "Change RDI reasons, former contained %s, current contains %s"},
    { TPOAM_4_LOG_EVENT, "MEGID %s %s event %s"},
    { TPOAM_4_TX_CSF_VPWS_EVENT, "VPWS %s local CSF state changes from %s to %s"},
    { TPOAM_4_RX_CSF_VPWS_EVENT, "VPWS %s remote CSF state changes from %s to %s"},
    { TPOAM_4_RX_CSF_TO_VPWS_EVENT, "VPWS %s remote CSF state changes from %s to %s for timeout"},
    { TPOAM_6_RX_PDU_UNSUPPORT_CHANNEL_TYPE, "Received PDU with unsupported channel type 0x%04X"},
    { TPOAM_6_RX_PDU_INVALID_MEL, "Received PDU with invalid level %d, expected level is %d"},
    { TPOAM_6_RX_PDU_INVALID_VERSION, "Received PDU with invalid version %d, expected version is %d"},
    { TPOAM_6_RX_PDU_INVALID_FIRST_TLV_OFFSET, "Received PDU with invalid first TLV offset %d, expected offset is %d"},
    { TPOAM_6_RX_PDU_INVALID_FLAGS, "Received PDU with invalid flags 0x%02X, expected flags is 0x%02X"},
    { TPOAM_6_RX_PDU_LSP_NOT_EXIST, "Received PDU with LSP label %u process fail, LSP does not exist"},
    { TPOAM_6_RX_PDU_PW_NOT_EXIST, "Received PDU with PW label %u process fail, PW does not exist"},
    { TPOAM_6_RX_PDU_SECTION_OAM_NOT_EXIST, "Received PDU with section interface %s process fail, section has no OAM"},
    { TPOAM_6_RX_PDU_LSP_OAM_NOT_EXIST, "Received PDU with LSP label %d process fail, LSP has no OAM"},
    { TPOAM_6_RX_PDU_PW_OAM_NOT_EXIST, "Received PDU with PW label %d process fail, PW has no OAM"},
    { TPOAM_6_RX_PDU_LCKED_BY_SRV_SERVICE, "MEG %s received %s process fail, service is locked by server service"},
    { TPOAM_6_TX_PDU_LSP_P_EAST_NOT_EXIST, "LSP-P %s transmit PDU fail, east direction does not exist"},
    { TPOAM_6_TX_PDU_LSP_P_WEST_NOT_EXIST, "LSP-P %s transmit PDU fail, west direction does not exist"},
    { TPOAM_6_TX_PDU_LSP_P_SPME_TUNNEL_EAST_NOT_EXIST, "LSP-P %s transmit PDU fail, east tunnel %s does not exist"},
    { TPOAM_6_TX_PDU_LSP_P_SPME_TUNNEL_WEST_NOT_EXIST, "LSP-P %s transmit PDU fail, west tunnel %s does not exist"},
    { TPOAM_6_TX_PDU_AIS_GET_DIRECTION_FAIL, "MEG %s cannot get client MEG %s AIS direction"},
    { TPOAM_6_RX_LBM_MEP_ID_MISMATCH, "MEG %s received LBM with mismatched MEPID %d, local MEPID is %d"},
    { TPOAM_6_RX_LBM_MEP_ID_MISMATCH_IN_REQUEST_TLV, "MEG %s received LBM with mismatched MEPID %d in requesting TLV, local RMEPID is %d"},
    { TPOAM_6_RX_LBM_MEG_ID_MISMATCH_IN_REQUEST_TLV, "MEG %s received LBM with mismatched MEGID %s in requesting TLV"},
    { TPOAM_6_RX_LBM_LMEP_NOT_EXIST, "MEG %s received LBM process fail, local MEP does not exist"},
    { TPOAM_6_RX_LBM_MIP_NOT_EXIST, "MEG %s received LBM process fail, MIP does not exist"},
    { TPOAM_6_RX_LBM_INVALID_TARGET_TLV_TYPE, "MEG %s received LBM process fail, PDU has invalid target TLV type %d, expected type is %d"},
    { TPOAM_6_RX_LBM_INVALID_TARGET_TLV_LEN, "MEG %s received LBM process fail, PDU has invalid target TLV length %d, expected length is %d"},
    { TPOAM_6_RX_LBM_ICC_MISMATCH, "MEG %s received LBM process fail, PDU has mismatched ICC %s, expected ICC is %s"},
    { TPOAM_6_RX_LBM_DISCOVERY_TTL_MISMATCH, "MEG %s received LBM process fail, discovery PDU has mismatched TTL %d, expected TTL is 1"},    
    { TPOAM_6_RX_LBM_ZERO_CHECK, "MEG %s received LBM process fail, PDU has mismatched nonzero bytes"},
    { TPOAM_6_RX_LBM_NODE_ID_MISMATCH, "MEG %s received LBM process fail, PDU has mismatched Node ID %s, expected Node ID is %s"},
    { TPOAM_6_RX_LBR_INVALID_REPLY_TLV_TYPE, "MEG %s received LBR process fail, PDU has invalid reply TLV type %d, expected type is %d"},
    { TPOAM_6_RX_LBR_INVALID_REPLY_TLV_LEN, "MEG %s received LBR process fail, PDU has invalid reply TLV length %d, expected length is %d"},
    { TPOAM_6_RX_LBR_TRANS_ID_MISMATCH, "MEG %s received LBR process fail, PDU has mismatched transaction ID %d, expected ID is %d"},
    { TPOAM_6_RX_LBR_ICC_MISMATCH, "MEG %s received LBR process fail, PDU has mismatched ICC %s, expected ICC is %s"},
    { TPOAM_6_RX_LBR_MEPID_MISMATCH, "MEG %s received LBR process fail, PDU has mismatched MEPID %d, expected MEPID is %d"},
    { TPOAM_6_RX_LBR_MEPID_MISMATCH_IN_REQUEST_TLV, "MEG %s received LBR with mismatched MEPID %d in requesting TLV, local MEPID is %d"},
    { TPOAM_6_RX_LBR_MEGID_MISMATCH_IN_REQUEST_TLV, "MEG %s received LBR with mismatched MEGID %s in requesting TLV"},
    { TPOAM_6_RX_LBR_NODE_ID_MISMATCH, "MEG %s received LBR process fail, PDU has mismatched Node ID %s, expected Node ID is %s"},
    { TPOAM_6_RX_LBR_ZERO_CHECK, "MEG %s received LBR process fail, PDU has mismatched nonzero bytes"},
    { TPOAM_6_RX_LBR_NO_END_TLV, "MEG %s RX LBR without end TLV"},
    { TPOAM_6_RX_AIS_INVALID_INTERVAL, "Received PDU with invalid AIS interval 0x%02X, expected interval is 0x04 or 0x06"},
    { TPOAM_6_AIS_SRV_CLIENT_MODE_MISMATCH, "AIS server MEG %s mode %s and client MEG %s mode %s is mismatch"},
    { TPOAM_6_LCK_SRV_CLIENT_MODE_MISMATCH, "LCK server MEG %s mode %s and client MEG %s mode %s is mismatch"},
};

uint32 tpoam_log_msg_len = sizeof(tpoam_log_message);

char*
tpoam_err_desc(tpoam_error_t error)
{   
    switch (error)
    {
    case TPOAM_E_NONE:
        return "OK";
    case TPOAM_E_FAIL:
        return "General error";
    case TPOAM_E_INVALID_TYPE:
        return "Invalid OAM type";
    case TPOAM_E_INVALID_CHAN_TYPE:
        return "Invalid MPLS-TP OAM Y.1731 ACH channel type";
    case TPOAM_E_MEMORY_ALLOC_FAIL:
        return "Alloc memory fail";
    case TPOAM_E_CONTROL_WORD_DISABLED:
        return "Can not config pw oam when control-word is disabled";
    case TPOAM_E_SET_WITH_NO_MEG:
        return "Should delete all OAM MEGs at first";
    case TPOAM_E_MEGID_INVALID_ICC_LEN:
        return "MEG ID is mismatched with ICC length";
    case TPOAM_E_MEGID_EXIST:
        return "MEG ID has already existed";
    case TPOAM_E_MEGID_NOT_EXIST:
        return "MEG ID does not exist";
    case TPOAM_E_MEGID_SEV_EXIST:
        return "Service has MEG ID";
    case TPOAM_E_MEGID_SEV_NOT_EXIST:
        return "Service has no MEG ID";
    case TPOAM_E_LMEP_EXIST:
        return "Local MEP has already existed";
    case TPOAM_E_LMEP_NOT_EXIST:
        return "Local MEP does not exist";
    case TPOAM_E_RMEP_EXIST:
        return "Remote MEP has already existed";
    case TPOAM_E_RMEP_NOT_EXIST:
        return "Remote MEP does not exist";
    case TPOAM_E_MIP_EXIST:
        return "MIP has already existed";
    case TPOAM_E_MIP_NOT_EXIST:
        return "MIP does not exist";        
    case TPOAM_E_STRING_1ST_CHAR:
        return "The first character should be 'a-z' or 'A-Z'";
    case TPOAM_E_STRING_LAST_CHAR:
        return "The last character should be '0-9' or 'a-z' or 'A-Z'";
    case TPOAM_E_STRING_INVALID_CHAR:
        return "The character should be '0-9' or 'a-z' or 'A-Z' or '.-_'";
    case TPOAM_E_MEGID_LENGTH:
        return "MEG ID length should be no more than 13";
    case TPOAM_E_SECTION_FULL:
        return "Section resource is full";
    case TPOAM_E_MEGID_FULL:
        return "MEG ID resource is full";
    case TPOAM_E_LMEP_FULL:
        return "Local MEP resource is full";
    case TPOAM_E_MIP_FULL:
        return "MIP resource is full";
    case TPOAM_E_CSF_FULL:
        return "CSF resource is full";
    case TPOAM_E_INVALID_MEG_LEVEL:
        return "Invalid MEG Level";
    case TPOAM_E_INVALID_CCM_INTERVAL:
        return "Invalid CCM Interval";
    case TPOAM_E_INVALID_MEP_ID:
        return "Invalid MEP ID";
    case TPOAM_E_SAME_TO_LMEP_ID:
        return "Same to Local MEP ID";
    case TPOAM_E_HAL_ERROR:
        return "Set to Hardware fail";
    case TPOAM_E_PW_VPWS_NOT_EXIST:
        return "PW VPWS does not exist";
    case TPOAM_E_PW_VPWS_NOT_STATIC:
        return "PW VPWS is not manual";
    case TPOAM_E_PW_VPLS_NOT_STATIC:
        return "PW VPLS peer is not manual";
    case TPOAM_E_PW_VPLS_NOT_EXIST:
        return "PW VPLS does not exist";
    case TPOAM_E_PW_VPLS_PEER_NOT_EXIST:
        return "PW VPLS peer does not exist";
    case TPOAM_E_PW_SPE_NOT_EXIST:
        return "PW SPE does not exist";
    case TPOAM_E_PW_NOT_EXIST:
        return "PW does not exist";
    case TPOAM_E_PW_VPWS_FIB_NOT_EXIST:
        return "PW VPWS FIB does not exist";
    case TPOAM_E_PW_VPLS_FIB_NOT_EXIST:
        return "PW VPLS FIB does not exist";        
    case TPOAM_E_PW_TPE_INLABEL_NOT_EXIST:
        return "TPE is not configure inlabel";        
    case TPOAM_E_LSP_NOT_EXIST:
        return "LSP does not exist";
    case TPOAM_E_LSP_P_NOT_EXIST:
        return "LSP-P does not exist";
    case TPOAM_E_LSP_PE_NOT_EXIST:
        return "LSP-PE does not exist";
    case TPOAM_E_LSP_PE_INLABEL_NOT_EXIST:
        return "LSP-PE is not configure inlabel";  
    case TPOAM_E_LSP_PE_OUTLABEL_NOT_EXIST:
        return "LSP-PE is not configure outlabel";  
    case TPOAM_E_SERVICE_MODE_INVALID:
        return "Service is not configure IP/non-IP mode";
    case TPOAM_E_SECTION_STR_LENGTH:
        return "Section name length should no more than 12";
    case TPOAM_E_SECTION_NOT_EXIST:
        return "Section does not exist";
    case TPOAM_E_SECTION_MODE_NOT_EXIST:
        return "Section is not configured with peer IP or out interface";
    case TPOAM_E_SECTION_MODE_IP:
        return "Section has been configured with IP mode";
    case TPOAM_E_SECTION_MODE_NON_IP:
        return "Section has been configured with non-IP mode";
    case TPOAM_E_SECTION_OUT_IF_EXIST:
        return "Out interface is in existence";
    case TPOAM_E_SECTION_OUT_IF_NOT_EXIST:
        return "No out interface has been added";
    case TPOAM_E_SECTION_OUT_IF_IS_USED:
        return "Out interface has been binded to other Section OAM";
    case TPOAM_E_SECTION_OUT_IF_INVALID_OIF:
        return "Out interface is not routed-port";
    case TPOAM_E_SECTION_OUT_IF_MEMBER_INVALID_OIF:
        return "Out member interface is invalid";
    case TPOAM_E_SECTION_OUT_IF_MEMBER_NOT_L2_OIF:
        return "Out member interface is not switch port";
    case TPOAM_E_SECTION_OUT_IF_MEMBER_NOT_IN_VLAN:
        return "Out member interface is not belong to vlan";
    case TPOAM_E_SECTION_OUT_IF_MEMBER_IS_LAG_MEMBER:
        return "Out member interface is LAG member port";
    case TPOAM_E_SECTION_OUT_IF_IS_LAG_MEMBER:
        return "Out interface is LAG member port";
    case TPOAM_E_SECTION_PEER_IP_EXIST:
        return "Peer IP is in existence";
    case TPOAM_E_SECTION_PEER_IP_NOT_EXIST:
        return "No Peer IP has been added";
    case TPOAM_E_SECTION_PEER_NO_OIF:
        return "Peer IP should be a connected IP address";
    case TPOAM_E_SECTION_PEER_OIF_IS_USED:
        return "Peer IP's out interface has been binded to other Section OAM";
    case TPOAM_E_SECTION_PEER_INVALID_OIF:
        return "Peer IP out interface is not routed-port";
    case TPOAM_E_SECTION_PEER_INVALID_ADDR:
        return "Peer IP address is invalid";
    case TPOAM_E_SECTION_PEER_IP_IS_BIND:
        return "Outgoing interface has been occupied by other peer";
    case TPOAM_E_SECTION_PEER_IP_IS_LOCAL:
        return "Peer IP address should not be local";
    case TPOAM_E_SECTION_HAS_OAM:
        return "Section is bind with OAM";
    case TPOAM_E_SECTION_INVALID_IFINDEX:
        return "Ifindex is invalid";
    case TPOAM_E_SECTION_NOT_FOUND_BY_IFINDEX:
        return "Not found section by ifindex";
    case TPOAM_E_SECTION_PEERIP_NOT_EXIST:
        return "Section is not configure Peer IP";
    case TPOAM_E_SECTION_INVALID_AIS_CLIENT_PW:
        return "Section should not be the AIS server of PW";
    case TPOAM_E_SECTION_MODE_IS_IP:
        return "Section is IP mode";
    case TPOAM_E_SECTION_MODE_IS_NON_IP:
        return "Section is non-IP mode";
    case TPOAM_E_SERVICE_DOWN:
        return "Service is down";
    case TPOAM_E_SERVICE_NON_MEP:
        return "Service has no MEP";
    case TPOAM_E_SERVICE_NON_MIP:
        return "Service has no MIP";
    case TPOAM_E_SERVICE_NON_OAM:
        return "Service has no OAM";
    case TPOAM_E_SOCK_INIT_FAIL:
        return "Socket init fail";
    case TPOAM_E_SOCK_INVALID_SOCK:
        return "Invalid socket ID";
    case TPOAM_E_SOCK_RX_FAIL:
        return "Socket RX fail";
    case TPOAM_E_SOCK_TX_SECTION_CHECK_FAIL:
        return "Socket Section TX PDU is invalid";
    case TPOAM_E_SOCK_TX_SECTION_FAIL:
        return "Socket Section TX fail";
    case TPOAM_E_SOCK_TX_LSP_CHECK_FAIL:
        return "Socket LSP TX PDU is invalid";
    case TPOAM_E_SOCK_TX_LSP_FAIL:
        return "Socket LSP TX fail";
    case TPOAM_E_SOCK_TX_LSP_P_EAST_NOT_EXIST:
        return "LSP-P inlabel-east does not exist";
    case TPOAM_E_SOCK_TX_LSP_P_WEST_NOT_EXIST:
        return "LSP-P inlabel-west does not exist";
    case TPOAM_E_SOCK_TX_LSP_P_SPME_TUNNEL_EAST_NOT_EXIST:
        return "SPME tunnel inlabel-east does not exist";
    case TPOAM_E_SOCK_TX_LSP_P_SPME_TUNNEL_WEST_NOT_EXIST:
        return "SPME tunnel inlabel-west does not exist";
    case TPOAM_E_SOCK_TX_LSP_PE_NHLFE_NOT_EXIST:
        return "LSP-PE out-label does not exist";
    case TPOAM_E_SOCK_TX_PW_FAIL:
        return "Socket PW TX fail";
    case TPOAM_E_SOCK_TX_VPWS_CHECK_FAIL:
        return "PW VPWS TX fail";
    case TPOAM_E_SOCK_TX_VPLS_CHECK_FAIL:
        return "PW VPLS TX fail";
    case TPOAM_E_SOCK_TX_TPE_CHECK_FAIL:
        return "PW TPE TX fail";
    case TPOAM_E_SOCK_TX_SPE_CHECK_FAIL:
        return "PW SPE TX fail";
    case TPOAM_E_SOCK_TX_SPE_NO_TUNNEL:
        return "PW SPE tunnel does not exist";
    case TPOAM_E_SOCK_TX_SPE_NO_SEL_LSP:
        return "PW SPE tunnel has no selected LSP";
    case TPOAM_E_SOCK_TX_SPE_INVALID_SEL_LSP:
        return "PW SPE tunnel has invalided selected LSP";
    case TPOAM_E_PDU_UNSUPPORT_CHANNEL_TYPE:
        return "PDU has mismatched channel type";
    case TPOAM_E_PDU_INVALID_OPCODE:
        return "PDU has invalid opcode";
    case TPOAM_E_PDU_INVALID_FLAGS:
        return "PDU has invalid flags";
    case TPOAM_E_PDU_INVALID_FIRST_TLV_OFFSET:
        return "PDU has invalid first TLV offset";
    case TPOAM_E_PDU_INVALID_MEL:
        return "PDU has invalid MEL";
    case TPOAM_E_PDU_INVALID_VERSION:
        return "PDU has invalid version";
    case TPOAM_E_PDU_INVALID_TARGET_TLV_TYPE:
        return "PDU has invalid target TLV type";
    case TPOAM_E_PDU_INVALID_REPLY_TLV_TYPE:
        return "PDU has invalid reply TLV type";
    case TPOAM_E_PDU_INVALID_TARGET_TLV_LEN:
        return "PDU has invalid target TLV length";
    case TPOAM_E_PDU_INVALID_REPLY_TLV_LEN:
        return "PDU has invalid reply TLV length";
    case TPOAM_E_PDU_INVALID_APS_PKT:
        return "Invalid APS pkt";
    case TPOAM_E_PDU_LBR_WITH_DISCOVERY:
        return "LBR has discovery TLV";
    case TPOAM_E_PDU_LBR_MEPID_MISMATCH:
        return "LBR has mismatched MEP ID";
    case TPOAM_E_PDU_AIS_INVALID_INTERVAL:
        return "AIS has invalid interval";
    case TPOAM_E_PDU_UNSUPPORT_IF_NUM:
        return "PDU not support non-zero IF_NUM";
    case TPOAM_E_PDU_DECODE_TLV_FAIL:
        return "PDU decode TLV fail";
    case TPOAM_E_LB_MEP_ID_MISMATCH:
        return "PDU has mismatched MEP ID";
    case TPOAM_E_LB_MEP_ID_MISMATCH_IN_REQUEST_TLV:
        return "PDU has mismatched MEP ID in requesting TLV";
    case TPOAM_E_LB_MEG_ID_MISMATCH_IN_REQUEST_TLV:
        return "PDU has mismatched MEG ID in requesting TLV";
    case TPOAM_E_LB_TRANS_ID_MISMATCH:
        return "PDU has mismatched transaction ID";
    case TPOAM_E_LB_ICC_MISMATCH:
        return "PDU has mismatched ICC";
    case TPOAM_E_LB_NODE_ID_MISMATCH:
        return "PDU has mismatched Node ID";
    case TPOAM_E_LB_IS_RUNNING:
        return "LB has running";
    case TPOAM_E_LB_UNSUPPORT_LB_TYPE:
        return "Unsupported LB type";
    case TPOAM_E_LB_DISCOVERY_TTL_MISMATCH:
        return "LB discovery PDU TTL is not 1";
    case TPOAM_E_LB_ZERO_CHECK:
        return "LB PDU has mismatched nonzero bytes";
    case TPOAM_E_LB_TEST_TLV_CRC_CHECK:
        return "LB PDU test TLV CRC check fail";
    case TPOAM_E_LB_TEST_TLV_NON_ZERO_CHECK:
        return "LB PDU test TLV has mismatched nonzero bytes";
    case TPOAM_E_LB_TEST_TLV_LENGTH_CHECK:
        return "LB PDU test TLV length is not enough";
    case TPOAM_E_LB_TLV_LEN_MORE_THAN_LEFT:
        return "LB PDU TLV length is large than left bytes";
    case TPOAM_E_LB_NO_END_TLV:
        return "LB PDU has no end TLV";
    case TPOAM_E_BIND_SRV_CANNOT_CHANGE_INTERVAL:
        return "Can't change interval";
    case TPOAM_E_BIND_CLIENT_IS_BIND:
        return "Client service is binding with the server service";
    case TPOAM_E_BIND_CLIENT_IS_NOT_BIND:
        return "Client service is not binding with the server service";
    case TPOAM_E_BIND_LSP_PE_TO_LSP_PE:
        return "LSP-PE cannot bind LSP-PE client";
    case TPOAM_E_BIND_PW_TO_SECTION:
        return "Section cannot bind PW client";
    case TPOAM_E_DM_INVALID_TYPE:
        return "Invalid DM action type";
    case TPOAM_E_DM_1DM_IS_RUNNING:
        return "1DM is running";
    case TPOAM_E_DM_1DM_IS_NOT_RUNNING:
        return "1DM is not running";
    case TPOAM_E_DM_DMM_IS_RUNNING:
        return "DMM is running";
    case TPOAM_E_LM_LMM_IS_RUNNING:
        return "LMM is running";
    case TPOAM_E_LM_PROACTIVE_LMM_IS_RUNNING:
        return "Proactive LMM is running";
    case TPOAM_E_LM_SINGLE_NOT_ENABLED:
        return "Single-ended LM is not enabled";
    case TPOAM_E_LM_SINGLE_NOT_PROACTIVE:
        return "Proactive single-ended LM is not enabled";
    case TPOAM_E_DM_DMM_IS_NOT_RUNNING:
        return "DMM is not running";        
    case TPOAM_E_LM_LMM_IS_NOT_RUNNING:
        return "LMM is not running";        
    case TPOAM_E_DM_INVALID_INTERVAL:
        return "Invalid Interval";
    case TPOAM_E_DM_1DM_INVALID_CACHE_SIZE:
        return "Invalid 1DM cache size";
    case TPOAM_E_EVT_INVALID_CACHE_SIZE:
        return "Invalid Event cache size";
    case TPOAM_E_CACHE_CURR_LARGE_MAX:
        return "The current count > cache size, please clear cache at first";
    case TPOAM_E_CLI_EXEC_IS_RUNNING:
        return "CLI is running another on-demand session";
    case TPOAM_E_INVALID_PEER_FORMAT:
        return "Invalid peer format";
    case TPOAM_E_INVALID_IFINDEX:
        return "Ifindex does not exist";
    case TPOAM_E_INVALID_VLAN_MEMBER_IFINDEX:
        return "VLAN member ifindex does not exist";
    case TPOAM_E_INTF_NO_P2P_MAC:
        return "Interface is not configured with P2P MAC";
    case TPOAM_E_INTF_DOWN:
        return "Interface is down";
    case TPOAM_E_SERVICE_MODE_MISMATCH:
        return "Server and client modes (Unconfigured/IP/Non-IP) are mismatched";
    case TPOAM_E_LCKED_BY_SRV_SERVICE:
        return "Service is locked by server service";
    case TPOAM_E_CSF_ENABLED:
        return "CSF has enabled";
    case TPOAM_E_CSF_NOT_ENABLED:
        return "CSF has not enabled";  
    /*added by zhoulei for csf-relay,2014-9-15*/
    case TPOAM_E_AC_DOWN_CONFLICT:
        return "Conflict:CSF rx-action down was enabled ";
    case TPOAM_E_AC_RELAY_CONFLICT:
        return "Conflict:CSF rx-action relay was enabled "; 
    case TPOAM_E_VPWS_VC_INFO_INVALID:
        return "VPWS vc  info is invalid";
    case TPOAM_E_VPWS_VLAN_NOT_FOUND:
        return "CSF associate vlan not found";
    case TPOAM_E_CSF_MD_CONFIG_FAIL:
        return "CSF matainence domain name is configured illogically";
    case TPOAM_E_CSF_MEP_NOT_FOUND:
        return "CSF mep is no found";
    case TPOAM_E_CSF_MEP_UP:
        return "CSF mep is up ";    
    /* modified by liwh for bug 21382, 2012-11-21 */
    case TPOAM_E_CSF_G8031_CONFLICT:
        return "CSF can't be enabled because VPWS AC binds G8031 instance"; 
    case TPOAM_E_LM_ALREADY_CONFIG:
        return "LM already configured"; 
    case TPOAM_E_SD_REASON_SET:
        return "WARNING: The change of SD reasons does not refresh previous occured SD status!"; 
    case TPOAM_E_SF_REASON_SET:
        return "WARNING: The change of SF reasons does not refresh previous occured SF status!"; 
    case TPOAM_E_RDI_REASON_SET:
        return "WARNING: The change of RDI reasons does not refresh previous occured RDI status!"; 
    case TPOAM_E_DESCRIPTION_LEN_MAX_EXCEED:
        return "String length should not exceed 32"; 
    case TPOAM_E_OAM_SESSION_IS_RUNNING:
        return "Oam session is running, please delete oam flow first";
    default:
        return "Unknown error";
    }
}


