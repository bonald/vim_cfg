#ifndef __CHSM_ERROR_H__
#define __CHSM_ERROR_H__

typedef enum {
    /* chsm function succeed */
    CHSM_E_SUCCESS          = 0,

    /* chsm common error number */
    CHSM_E_NO_MEMORY        = -999,     
    CHSM_E_NOT_INIT,
    CHSM_E_NOT_FOUND,
    CHSM_E_INIT_FAILED,
    CHSM_E_FILE,
    CHSM_E_FILE_OPEN,
    CHSM_E_TIMER,
    CHSM_E_START_NSM,
    CHSM_E_IOCTL,
    
    /* chsm parameter check error number */
    CHSM_E_INVALID_PARAM,
    CHSM_E_INVALID_MAC_ADDR,
    CHSM_E_INVALID_IP_ADDR,    
    CHSM_E_INVALID_PTR,
    CHSM_E_INVALID_INDEX,
    CHSM_E_INVALID_IFINDEX,
    CHSM_E_INVALID_IFNAME,
    CHSM_E_INVALID_LENGTH,    
    CHSM_E_INVALID_SLOT,
    CHSM_E_INVALID_PORT,
    CHSM_E_INVALID_CHIP,
    CHSM_E_INVALID_BOARD,    
    CHSM_E_INVALID_MODE,
    CHSM_E_INVALID_SPEED,
    CHSM_E_INVALID_DUPLEX,
    CHSM_E_INVALID_MEDIA,
    CHSM_E_INVALID_RESETTIME,

    /* chsm socket error number */    
    CHSM_E_SOCK_GEN,
    CHSM_E_SOCK_SEND,
    CHSM_E_SOCK_RECV,
    CHSM_E_SOCK_CREATE,
    CHSM_E_SOCK_BIND,
    CHSM_E_SOCK_LISTEN,
    CHSM_E_SOCK_ACCEPT,
    
    /* chsm hardware error number */    
    CHSM_E_HW_GEN,
    CHSM_E_HW_CHIP_GEN,
    CHSM_E_HW_POWER_GEN,
    CHSM_E_HW_TMPR_GEN, 
    CHSM_E_HW_I2C_GEN,
    CHSM_E_HW_I2C_OPEN,
    CHSM_E_HW_I2C_READ,
    CHSM_E_HW_I2C_WRITE,    
    CHSM_E_HW_EPLD_GEN,
    CHSM_E_HW_FPGA_GEN,
    CHSM_E_HW_EEPROM_GEN,
    CHSM_E_HW_PCI_GEN,
    CHSM_E_HW_LED_GEN,
    CHSM_E_HW_LCD_GEN,    
    CHSM_E_HW_PHY_GEN, 
    CHSM_E_HW_MDIO_GEN,     
    CHSM_E_HW_SFP_GEN,
    CHSM_E_HW_XFP_GEN,
    CHSM_E_HW_SERIAL_GEN,
    CHSM_E_HW_TIMEOUT,
    CHSM_E_HW_BUSY,
    CHSM_E_HW_PRESENT,
    CHSM_E_HW_ABSENT,

    /* chsm communication error number */    
    CHSM_E_COMM_CONNECT,
    CHSM_E_COMM_DISCONNECT,
    CHSM_E_COMM_KEEPALIVE,
    CHSM_E_COMM_ACTIVE,
    CHSM_E_COMM_STANDBY,
    CHSM_E_LC_NOT_INIT,

    /* chsm linecard error number */    
    CHSM_E_LCM_PRESENT,
    CHSM_E_LCM_ABSENT,
    CHSM_E_LCM_MODE,
    CHSM_E_LCM_NOT_READY,
    CHSM_E_LCM_MSG_CTRL_ID,
    CHSM_E_LCM_MSG_KEEPALIVE,
    CHSM_E_LCM_MSG_SEND,
    CHSM_E_LCM_MSG_RECV,
    CHSM_E_LCM_MSG_ENCODE,
    CHSM_E_LCM_MSG_DECODE,
    CHSM_E_LCM_DELETE,

    /* chsm configure error number */ 
    CHSM_E_CFG_GEN,
    CHSM_E_CFG_MANAGE_IP,
    CHSM_E_CFG_GATEWAY,
    CHSM_E_CFG_SYNC_GEN,
    CHSM_E_CFG_SWITCH_GEN,
    CHSM_E_CFG_STANDBY,

    /* chsm server error number */
    CHSM_E_SRV_SOCK_INIT,
    CHSM_E_SRV_MSG_SEND,
    CHSM_E_SRV_MSG_RECV,
    CHSM_E_SRV_ACCEPT,
    CHSM_E_SRV_MSG_ENCODE,
    CHSM_E_SRV_MSG_DECODE,

    /* chsm stm error number*/
    CHSM_E_STM_MODE_IN_USE,   
    CHSM_E_STM_LACK_HASH_RESOUCE,
    CHSM_E_STM_LACK_ETHOAM_RESOUCE,
    CHSM_E_STM_LACK_TPOAM_RESOUCE,
    CHSM_E_STM_LACK_TCAM_RESOUCE,
    CHSM_E_STM_LACK_SRAM_RESOUCE,
    CHSM_E_STM_PARSE,
} chsm_error_t;


#define CHSM_E_SUCCESS_DESC                 "success"

/* chsm common error number */
#define CHSM_E_NO_MEMORY_DESC               "no memory"
#define CHSM_E_NOT_INIT_DESC                "not init"
#define CHSM_E_NOT_FOUND_DESC               "not found"
#define CHSM_E_INIT_FAILED_DESC             "init fail"
#define CHSM_E_FILE_DESC                    "file fail"
#define CHSM_E_TIMER_DESC                   "timer fail"
#define CHSM_E_START_NSM_DESC               "start nsm fail"
#define CHSM_E_IOCTL_DESC                   "ioctl fail"
#define CHSM_E_FILE_OPEN_DESC               "file open fail"

/* chsm parameter check error number */
#define CHSM_E_INVALID_PARAM_DESC           "invalid parameter"
#define CHSM_E_INVALID_MAC_ADDR_DESC        "invalid MAC address"
#define CHSM_E_INVALID_IP_ADDR_DESC         "invalid IP address"
#define CHSM_E_INVALID_PTR_DESC             "invalid pointer"
#define CHSM_E_INVALID_INDEX_DESC           "invalid index"
#define CHSM_E_INVALID_IFINDEX_DESC         "invalid ifindex"
#define CHSM_E_INVALID_IFNAME_DESC          "invalid ifname"
#define CHSM_E_INVALID_LENGTH_DESC          "invalid length"
#define CHSM_E_INVALID_SLOT_DESC            "invalid slot"
#define CHSM_E_INVALID_PORT_DESC            "invalid port"
#define CHSM_E_INVALID_CHIP_DESC            "invalid chip"
#define CHSM_E_INVALID_BOARD_DESC           "invalid board"
#define CHSM_E_INVALID_MODE_DESC            "invalid mode"
#define CHSM_E_INVALID_SPEED_DESC           "invalid speed"
#define CHSM_E_INVALID_DUPLEX_DESC          "invalid duplex"
#define CHSM_E_INVALID_MEDIA_DESC           "invalid phy media"
#define CHSM_E_INVALID_RESETTIME_DESC       "invalid reset time"

/* chsm socket error number */
#define CHSM_E_SOCK_GEN_DESC                "socket fail"
#define CHSM_E_SOCK_SEND_DESC               "socket send fail"
#define CHSM_E_SOCK_RECV_DESC               "socket recv fail"
#define CHSM_E_SOCK_CREATE_DESC             "socket create fail"
#define CHSM_E_SOCK_BIND_DESC               "socket bind fail"
#define CHSM_E_SOCK_LISTEN_DESC             "socket listen fail"
#define CHSM_E_SOCK_ACCEPT_DESC             "socket accept fail"

/* chsm hardware error number */
#define CHSM_E_HW_GEN_DESC                  "HW fail"
#define CHSM_E_HW_CHIP_GEN_DESC             "HW chip fail"
#define CHSM_E_HW_POWER_GEN_DESC            "HW power fail"
#define CHSM_E_HW_TMPR_GEN_DESC             "HW temperature fail"
#define CHSM_E_HW_I2C_GEN_DESC              "HW I2C fail"
#define CHSM_E_HW_I2C_OPEN_DESC             "HW I2C open fail"
#define CHSM_E_HW_I2C_READ_DESC             "HW I2C read fail"
#define CHSM_E_HW_I2C_WRITE_DESC            "HW I2C write fail"
#define CHSM_E_HW_EPLD_GEN_DESC             "HW EPLD fail"
#define CHSM_E_HW_FPGA_GEN_DESC             "HW FPGA fail"
#define CHSM_E_HW_EEPROM_GEN_DESC           "HW EEPROM fail"
#define CHSM_E_HW_PCI_GEN_DESC              "HW PCI fail"
#define CHSM_E_HW_LED_GEN_DESC              "HW LED fail"
#define CHSM_E_HW_LCD_GEN_DESC              "HW LCD fail"
#define CHSM_E_HW_PHY_GEN_DESC              "HW PHY fail"
#define CHSM_E_HW_MDIO_GEN_DESC             "HW MDIO fail"
#define CHSM_E_HW_RJ45_GEN_DESC             "HW RJ45 fail"
#define CHSM_E_HW_SFP_GEN_DESC              "HW SFP fail"
#define CHSM_E_HW_XFP_GEN_DESC              "HW XFP fail"
#define CHSM_E_HW_SERIAL_GEN_DESC           "HW serial fail"
#define CHSM_E_HW_TIMEOUT_DESC              "HW access timeout"
#define CHSM_E_HW_BUSY_DESC                 "HW busy"
#define CHSM_E_HW_PRESENT_DESC              "HW present error"
#define CHSM_E_HW_ABSENT_DESC               "HW absent error"

/* chsm communication error number */
#define CHSM_E_COMM_CONNECT_DESC            "Comm connect fail"
#define CHSM_E_COMM_DISCONNECT_DESC         "Comm disconnect fail"
#define CHSM_E_COMM_KEEPALIVE_DESC          "Comm keepalive fail"
#define CHSM_E_COMM_ACTIVE_DESC             "active board fail"
#define CHSM_E_COMM_STANDBY_DESC            "standby board fail"

/* chsm linecard error number */
#define CHSM_E_LCM_PRESENT_DESC             "linecard present error"
#define CHSM_E_LCM_ABSENT_DESC              "linecard absent error"
#define CHSM_E_LCM_MODE_DESC                "linecard mode error"
#define CHSM_E_LCM_NOT_READY_DESC           "linecard not ready"
#define CHSM_E_LCM_MSG_CTRL_ID_DESC         "linecard MSG ID error"
#define CHSM_E_LCM_MSG_KEEPALIVE_DESC       "linecard MSG keepalive fail"
#define CHSM_E_LCM_MSG_SEND_DESC            "linecard MSG send fail"
#define CHSM_E_LCM_MSG_RECV_DESC            "linecard MSG recv fail"
#define CHSM_E_LCM_MSG_ENCODE_DESC          "linecard MSG encode fail"
#define CHSM_E_LCM_MSG_DECODE_DESC          "linecard MSG decode fail"

/* chsm configure error number */ 
#define CHSM_E_CFG_GEN_DESC                 "config fail"
#define CHSM_E_CFG_MANAGE_IP_DESC           "config manage ip fail"
#define CHSM_E_CFG_GATEWAY_DESC             "config gateway fail"
#define CHSM_E_CFG_SYNC_GEN_DESC            "config sync fail"
#define CHSM_E_CFG_SWITCH_GEN_DESC          "config switchover fail"
#define CHSM_E_CFG_STANDBY_DESC             "config standby card fail"
/* get error information according to error number */
char* chsm_get_errinfo (chsm_error_t error_no);
#endif /* !__CHSM_ERROR_H__ */



