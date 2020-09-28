
#include "proto.h"

#include "gen/tbl_ptp_port_define.h"
#include "gen/tbl_ptp_port.h"

#include "ptp_datatype.h"
#include "ptp_desc.h"
#include "ptp_error.h"
#include "gen/tbl_ptp_global_define.h"
#include "gen/tbl_ptp_global.h"

#define PTP_EDESC_LEN 128

static char g_errstr[PTP_EDESC_LEN];

char *
ptp_error_str(ptp_error_t error)
{
    switch (error)
    {
    case PTP_E_SUCCESS:
        return "OK";

    case PTP_E_ERROR:
        return "General error";

    case PTP_E_NO_MEMORY:
        return "PTP_E_NO_MEMORY";

    case PTP_E_NOT_INIT:
        return "PTP_E_NOT_INIT";

    case PTP_E_NOT_FOUND:
        return "PTP_E_NOT_FOUND";

    case PTP_E_NOT_EXIST:
        return "PTP_E_NOT_EXIST";

    case PTP_E_HAVE_EXIST:
        return "PTP_E_HAVE_EXIST";

    case PTP_E_INIT_FAILED:
        return "PTP_E_INIT_FAILED";

    case PTP_E_FILE:
        return "PTP_E_FILE";

    case PTP_E_FILE_OPEN:
        return "PTP_E_FILE_OPEN";

    case PTP_E_TIMER:
        return "PTP_E_TIMER";

    case PTP_E_TIMEOUT:
        return "PTP_E_TIMEOUT";

    case PTP_E_IOCTL:
        return "PTP_E_IOCTL";

    case PTP_E_NOT_IMPLEMENT:
        return "PTP_E_NOT_IMPLEMENT";

    case PTP_E_INVALID_PARAM:
        return "PTP_E_INVALID_PARAM";

    case PTP_E_INVALID_MAC_ADDR:
        return "Unable to translate MAC address";

    case PTP_E_INVALID_IP_ADDR:
        return "PTP_E_INVALID_IP_ADDR";

    case PTP_E_INVALID_PTR:
        return "PTP_E_INVALID_PTR";

    case PTP_E_INVALID_INDEX:
        return "PTP_E_INVALID_INDEX";

    case PTP_E_INVALID_IFINDEX:
        return "PTP_E_INVALID_IFINDEX";

    case PTP_E_INVALID_IFNAME:
        return "PTP_E_INVALID_IFNAME";

    case PTP_E_INVALID_LENGTH:
        return "PTP_E_INVALID_LENGTH";

    case PTP_E_INVALID_PORT:
        return "PTP_E_INVALID_PORT";

    case PTP_E_INVALID_CHIP:
        return "PTP_E_INVALID_CHIP";

    case PTP_E_INVALID_MSG:
        return "PTP_E_INVALID_MSG";

    case PTP_E_SOCK_GEN:
        return "PTP_E_SOCK_GEN";

    case PTP_E_SOCK_SEND:
        return "PTP_E_SOCK_SEND";

    case PTP_E_SOCK_RECV:
        return "PTP_E_SOCK_RECV";

    case PTP_E_SOCK_CREATE:
        return "PTP_E_SOCK_CREATE";

    case PTP_E_SOCK_BIND:
        return "PTP_E_SOCK_BIND";

    case PTP_E_SOCK_LISTEN:
        return "PTP_E_SOCK_LISTEN";

    case PTP_E_SOCK_ACCEPT:
        return "PTP_E_SOCK_ACCEPT";

    case PTP_E_NSM_PORT_INIT_ERROR:
        return "PTP_E_NSM_PORT_INIT_ERROR";

    case PTP_E_INTERNAL_TO_STAMP_NEG:
        return "Negative value cannot be converted into time stamp";

    case PTP_E_STAMP_TO_INTERNAL_OVERFLOW:
        return "Clock servo cannot be executed : seconds field is higher than signed integer (32bits)";

    case PTP_E_NON_PHY_AGG_INTF:
        return "PTP port must be physical or aggregation interface";

    case PTP_E_CONF_NEED_UNICAST_MAC:
        return "The MAC address must be unicast address";

    case PTP_E_CONF_INVALID_PORT:
        return "Cannot configure PTP on this interface";

    case PTP_E_CONF_ONLY_OC:
        return "Only can be configured when the device works as ordinary clock";

    case PTP_E_CONF_ONLY_TC:
        return "Only can be configured when the device works as transparent clock";

    case PTP_E_CONF_ONLY_E2E_TC:
        return "Only can be configured when the device works as end-to-end transparent clock";

    case PTP_E_CONF_ONLY_P2P_TC:
        return "Only can be configured when the device works as peer-to-peer transparent clock";

    case PTP_E_CONF_ONLY_MAC_ENCAP:
        return "Encapsulation mode must be IEEE 802.3";

    case PTP_E_CONF_ONLY_IPV4_ENCAP:
        return "Encapsulation mode must be UDP over IPv4";

    case PTP_E_CONF_NOT_SUPPORTED_ENCAP:
        return "Encapsulation mode must be IEEE 802.3 or UDP over IPv4";

    case PTP_E_CONF_UNIQUE_PORT_ON_OC:
        return "Only one interface can enable PTP on OC";

    case PTP_E_CONF_UNIQUE_SLAVE_ON_BC:
        return "Only one interface can be configured to slave";

    case PTP_E_CONF_ONLY_DYNAMIC_MODE:
        return "PTP_E_CONF_ONLY_DYNAMIC_MODE";

    case PTP_E_CONF_ONLY_PORT_DISABLED:
        return "Only can be configured when the interface disabled PTP";

    case PTP_E_CONF_ONLY_FAULTY_STATE:
        return "This PTP port is not in faulty state";

    case PTP_E_OP_ONLY_OC_BC:
        return "PTP_E_OP_ONLY_OC_BC";

    case PTP_E_OP_ONLY_STATIC_MODE:
        return "PTP_E_OP_ONLY_STATIC_MODE";

    case PTP_E_OP_TC_PORT_FAULTY_DISCARD:
        return "PTP_E_OP_TC_PORT_FAULTY_DISCARD";

    case PTP_E_MSG_NOT_CORRESPONDING_SYNC:
        return "PTP_E_MSG_NOT_CORRESPONDING_SYNC";

    case PTP_E_MSG_NOT_CORRESPONDING_VERSION:
        return "PTP_E_MSG_NOT_CORRESPONDING_VERSION";

    case PTP_E_MSG_NOT_CORRESPONDING_DOMAIN:
        return "PTP_E_MSG_NOT_CORRESPONDING_DOMAIN";

    case PTP_E_MSG_ERROR_ENCAP:
        return "PTP_E_MSG_ERROR_ENCAP";

    case PTP_E_CHIP_FRC_OFFSET_GET_FAIL:
        return "PTP_E_CHIP_FRC_OFFSET_GET_FAIL";

    case PTP_E_CHIP_FRC_OFFSET_SET_FAIL:
        return "PTP_E_CHIP_FRC_OFFSET_SET_FAIL";

    case PTP_E_CHIP_FRC_OFFSET_ADJUST_FAIL:
        return "PTP_E_CHIP_FRC_OFFSET_ADJUST_FAIL";

    case PTP_E_CHIP_DRIFT_ADJUST_FAIL:
        return "PTP_E_CHIP_DRIFT_ADJUST_FAIL";

    case PTP_E_CHIP_TX_TIME_GET_FAIL:
        return "PTP_E_CHIP_TX_TIME_GET_FAIL";

    case PTP_E_CHIP_ENABLE_SET_FAIL:
        return "PTP_E_CHIP_ENABLE_SET_FAIL";

    case PTP_E_PORT_ENABLE_SET_FAIL:
        return "PTP_E_PORT_ENABLE_SET_FAIL";

    case PTP_E_TOO_MANY_FOREIGN_MASTER:
        return "PTP_E_TOO_MANY_FOREIGN_MASTER";

    case PTP_E_IF_BLOCKED:
        return "PTP_E_IF_BLOCKED";

    case PTP_E_SWITCH_DEVICE_TYPE:
        return "PTP_E_SWITCH_DEVICE_TYPE";

    case PTP_E_SET_ASYMMETRY_CORRECTION:
        return "PTP_E_SET_ASYMMETRY_CORRECTION";

    case PTP_E_SET_LATENCY_CORRECTION:
        return "PTP_E_SET_LATENCY_CORRECTION";

    case PTP_E_SET_INTF_ENABLE:
        return "PTP_E_SET_INTF_ENABLE";

    case PTP_E_SET_INTF_VLAN_INFO:
	  return "PTP_E_SET_INTF_VLAN_INFO";

    case PTP_E_SET_TOD_PULSE_DELAY_CORRECTION:
        return "PTP_E_SET_TOD_PULSE_DELAY_CORRECTION";

    default:
        return "General error";
    }
}

char *
ptp_error_api_desc(ptp_api_error_t error)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    
    switch (error)
    {
    case PTP_E_API_NONE:
        return "OK";

    case PTP_E_API_INVALID_PARAM:
        return "Cannot recognize your input";

    case PTP_E_API_INVALID_CMD_INPUT:
        return "Invalid input parameter";
        
    case PTP_E_API_INVALID_INTERFACE:
        return "Cannot configure on this interface";

    case PTP_E_API_INVALID_DEVICE_TYPE:
        sal_memset(g_errstr, 0x00, PTP_EDESC_LEN);
        sal_snprintf(g_errstr, PTP_EDESC_LEN, "Cannot configure on %s", ptp_desc_dev_type(g_p_master->device_type));
        return g_errstr;

    case PTP_E_API_INVALID_DELAY_MECHANISM:
        return "Cannot configure on current delay mechanism";

    case PTP_E_API_INVALID_IP_ADDRESS:
        return "Cannot configure invalid IP address";
        
    case PTP_E_API_INVALID_IP_ADDRESS_SAME_SOURCE_DEST:
        return "Source IP cannot be configured the same with dest IP";

    case PTP_E_API_INVALID_IP_ADDRESS_SOURCE_DEST_CONFLICT_WITH_OTHER:
        return "Source IP and dest IP conflict with other interface";

    case PTP_E_API_OC_TOO_MANY_PORT:
        return "Cannot configure more than one PTP port on ordinary clock";

    case PTP_E_API_CANNOT_OP_ON_RUNNING:
        return "Cannot configure on running PTP port";

    case PTP_E_API_CANNOT_OP_ON_SLAVE_ONLY:
        return "Cannot configure on slave-only clock";

    case PTP_E_API_TOO_MANY_VID:
	 return "Cannot configure more than ten vid on clock";

    case PTP_E_API_NO_TOCPU_VLAN:
        return "The vlan id is not be configured";

    case PTP_E_API_TOCPU_VLAN_EXIST:
        return "The vlan id has already existed";
        
    case PTP_E_API_NO_UNICAST_ON_INVALID_PORT:
        return "Unicast ptp is not used in this port";

    case PTP_E_API_CLEAR_CONFIGURATION:
        return "PTP configurations have been cleared, except clock-id, global-enable, device-type and clock-step";
        
    case PTP_E_API_GENERAL_ERROR:
    default:
        return "General error";
    }
}

