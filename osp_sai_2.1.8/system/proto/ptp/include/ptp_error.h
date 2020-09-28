
#ifndef __PTP_ERROR_H__
#define __PTP_ERROR_H__

#include "glb_macro.h"
#include "ptp_debug.h"

enum ptp_error_e
{
    PTP_E_SUCCESS = 0,
    PTP_E_ERROR = -999,
    PTP_E_NO_MEMORY,
    PTP_E_NOT_INIT,
    PTP_E_NOT_FOUND,
    PTP_E_NOT_EXIST,
    PTP_E_HAVE_EXIST,
    PTP_E_INIT_FAILED,
    PTP_E_FILE,
    PTP_E_FILE_OPEN,
    PTP_E_TIMER,
    PTP_E_TIMEOUT,
    PTP_E_IOCTL,
    PTP_E_NOT_IMPLEMENT,

    /* parameter check error */
    PTP_E_INVALID_PARAM,
    PTP_E_INVALID_MAC_ADDR,
    PTP_E_INVALID_IP_ADDR,
    PTP_E_INVALID_PTR,
    PTP_E_INVALID_INDEX,
    PTP_E_INVALID_IFINDEX,
    PTP_E_INVALID_IFNAME,
    PTP_E_INVALID_LENGTH,
    PTP_E_INVALID_PORT,
    PTP_E_INVALID_CHIP,
    PTP_E_INVALID_MSG,

    /* socket error number */
    PTP_E_SOCK_GEN,
    PTP_E_SOCK_SEND,
    PTP_E_SOCK_RECV,
    PTP_E_SOCK_CREATE,
    PTP_E_SOCK_BIND,
    PTP_E_SOCK_LISTEN,
    PTP_E_SOCK_ACCEPT,

    PTP_E_NSM_PORT_INIT_ERROR,

    /*"Negative value cannot be converted into time stamp \n"*/
    PTP_E_INTERNAL_TO_STAMP_NEG,

    /*"Clock servo cannot be executed : seconds field is higher than signed integer (32bits) \n"*/
    PTP_E_STAMP_TO_INTERNAL_OVERFLOW,

    PTP_E_NON_PHY_AGG_INTF,

    PTP_E_CONF_NEED_UNICAST_MAC,
    PTP_E_CONF_INVALID_PORT,

    PTP_E_CONF_ONLY_OC,
    PTP_E_CONF_ONLY_OC_BC,
    PTP_E_CONF_ONLY_TC,
    PTP_E_CONF_ONLY_E2E_TC,
    PTP_E_CONF_ONLY_P2P_TC,

    PTP_E_CONF_ONLY_MAC_ENCAP,
    PTP_E_CONF_ONLY_IPV4_ENCAP,
    PTP_E_CONF_NOT_SUPPORTED_ENCAP,
    PTP_E_CONF_UNIQUE_PORT_ON_OC,
    PTP_E_CONF_UNIQUE_SLAVE_ON_BC,
    PTP_E_CONF_ONLY_DYNAMIC_MODE,
    PTP_E_CONF_ONLY_PORT_DISABLED,

    PTP_E_CONF_ONLY_FAULTY_STATE,

    PTP_E_OP_ONLY_OC_BC,
    PTP_E_OP_ONLY_STATIC_MODE,

    PTP_E_OP_TC_PORT_FAULTY_DISCARD,

    PTP_E_MSG_NOT_CORRESPONDING_SYNC,
    PTP_E_MSG_NOT_CORRESPONDING_VERSION,
    PTP_E_MSG_NOT_CORRESPONDING_DOMAIN,
    PTP_E_MSG_ERROR_ENCAP,

    PTP_E_CHIP_FRC_OFFSET_GET_FAIL,
    PTP_E_CHIP_FRC_OFFSET_SET_FAIL,
    PTP_E_CHIP_FRC_OFFSET_ADJUST_FAIL,
    PTP_E_CHIP_DRIFT_ADJUST_FAIL,
    PTP_E_CHIP_TX_TIME_GET_FAIL,
    PTP_E_CHIP_ENABLE_SET_FAIL,
    PTP_E_PORT_ENABLE_SET_FAIL,
    PTP_E_TOO_MANY_FOREIGN_MASTER,
    PTP_E_IF_BLOCKED,
    PTP_E_SWITCH_DEVICE_TYPE,
    PTP_E_SET_ASYMMETRY_CORRECTION,
    PTP_E_SET_LATENCY_CORRECTION,
    PTP_E_SET_INTF_ENABLE,
    PTP_E_SET_INTF_VLAN_INFO,
    PTP_E_SET_TOD_PULSE_DELAY_CORRECTION,
};
typedef enum ptp_error_e ptp_error_t;

enum ptp_api_error_e
{
    PTP_E_API_NONE = 0,

    PTP_E_API_GENERAL_ERROR = -50000,
    PTP_E_API_INVALID_PARAM,
    PTP_E_API_INVALID_CMD_INPUT,
    PTP_E_API_INVALID_INTERFACE,
    PTP_E_API_INVALID_DEVICE_TYPE,
    PTP_E_API_INVALID_DELAY_MECHANISM,
    PTP_E_API_INVALID_IP_ADDRESS,
    PTP_E_API_INVALID_IP_ADDRESS_SAME_SOURCE_DEST,
    PTP_E_API_INVALID_IP_ADDRESS_SOURCE_DEST_CONFLICT_WITH_OTHER,
    PTP_E_API_OC_TOO_MANY_PORT,
    PTP_E_API_CANNOT_OP_ON_RUNNING,
    PTP_E_API_CANNOT_OP_ON_SLAVE_ONLY,
    PTP_E_API_TOO_MANY_VID,
    PTP_E_API_NO_TOCPU_VLAN,
    PTP_E_API_TOCPU_VLAN_EXIST,
    PTP_E_API_NO_UNICAST_ON_INVALID_PORT,
    PTP_E_API_CLEAR_CONFIGURATION,
};
typedef enum ptp_api_error_e ptp_api_error_t;

#define PTP_OP_ERROR_RETURN(_op) \
do \
{ \
    int32 rv; \
    if ((rv = (_op)) < 0) \
    { \
        PTP_LOG_ERR("PTPD OP failed: rv = %d, %s:%d. %s", rv, __FUNCTION__, __LINE__, ptp_error_str(rv)); \
        return (rv); \
    } \
} \
while (0)

#define PTP_OP_ERROR_CONTINUE(_op) \
{ \
    int32 rv; \
    if ((rv = (_op)) < 0) \
    { \
        PTP_LOG_ERR("PTPD OP failed: rv = %d, %s:%d. %s", rv, __FUNCTION__, __LINE__, ptp_error_str(rv)); \
        continue; \
    } \
}

#define PTP_MEM_INVALID_RETURN(_v) GLB_MEM_PTR_CHECK(_v, PTP_E_NO_MEMORY)
#define PTP_PTR_INVALID_RETURN(_v) GLB_PTR_VALID_CHECK(_v, PTP_E_INVALID_PTR)
#define PTP_PTR_INVALID_RETURN_NULL(_v) GLB_PTR_VALID_CHECK_RETURN_NULL(_v)
#define PTP_PORT_INFO_CHECK(_v) GLB_PTR_VALID_CHECK(_v, PTP_E_CONF_INVALID_PORT)
#define PTP_PTR_NULL_CONTINUE(_v) \
{ \
    if (NULL == (_v)) \
    { \
        continue; \
    } \
}

char*
ptp_error_str(ptp_error_t error);

char *
ptp_error_api_desc(ptp_api_error_t error);

#endif /*!__PTP_ERROR_H__*/

