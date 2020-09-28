#ifndef __LCM_ERROR_H__
#define __LCM_ERROR_H__

typedef enum {
    /* LCM function succeed */
    LCM_E_SUCCESS          = 0,

    /* LCM common error number */
    LCM_E_NO_MEMORY        = -999,     
    LCM_E_NOT_INIT,
    LCM_E_NOT_FOUND,
    LCM_E_INIT_FAILED,
    LCM_E_FILE,
    LCM_E_FILE_OPEN,
    LCM_E_TIMER,
    LCM_E_START_NSM,
    LCM_E_IOCTL,
    LCM_E_THREAD_FAILED,
    
    /* LCM parameter check error number */
    LCM_E_INVALID_PARAM,
    LCM_E_INVALID_MAC_ADDR,
    LCM_E_INVALID_IP_ADDR,    
    LCM_E_INVALID_PTR,
    LCM_E_INVALID_INDEX,
    LCM_E_INVALID_IFINDEX,
    LCM_E_INVALID_IFNAME,
    LCM_E_INVALID_LENGTH,    
    LCM_E_INVALID_SLOT,
    LCM_E_INVALID_PORT,
    LCM_E_INVALID_CHIP,
    LCM_E_INVALID_BOARD,    
    LCM_E_INVALID_MODE,
    LCM_E_INVALID_SPEED,
    LCM_E_INVALID_DUPLEX,
    LCM_E_INVALID_MEDIA,
    LCM_E_INVALID_RESETTIME,

    /* LCM socket error number */    
    LCM_E_SOCK_GEN,
    LCM_E_SOCK_SEND,
    LCM_E_SOCK_RECV,
    LCM_E_SOCK_CREATE,
    LCM_E_SOCK_BIND,
    LCM_E_SOCK_LISTEN,
    LCM_E_SOCK_ACCEPT,
    
    /* LCM hardware error number */    
    LCM_E_HW_GEN,
    LCM_E_HW_CHIP_GEN,
    LCM_E_HW_CLKGEN_GEN,
    LCM_E_HW_POWER_GEN,
    LCM_E_HW_TMPR_GEN, 
    LCM_E_HW_I2C_GEN,
    LCM_E_HW_I2C_OPEN,
    LCM_E_HW_I2C_READ,
    LCM_E_HW_I2C_WRITE,    
    LCM_E_HW_EPLD_GEN,
    LCM_E_HW_FPGA_GEN,
    LCM_E_HW_EEPROM_GEN,
    LCM_E_HW_PCI_GEN,
    LCM_E_HW_LED_GEN,
    LCM_E_HW_LCD_GEN,    
    LCM_E_HW_PHY_GEN, 
    LCM_E_HW_UPDATEBTROM_GEN,
    LCM_E_HW_UPDATEEPLD_GEN,
    LCM_E_HW_MDIO_GEN,     
    LCM_E_HW_SFP_GEN,
    LCM_E_HW_XFP_GEN,
    LCM_E_HW_SERIAL_GEN,
    LCM_E_HW_TIMEOUT,
    LCM_E_HW_BUSY,
    LCM_E_HW_PRESENT,
    LCM_E_HW_ABSENT,
    LCM_E_HW_CLOCK,

    /* LCM communication error number */    
    LCM_E_COMM_CONNECT,
    LCM_E_COMM_DISCONNECT,
    LCM_E_COMM_KEEPALIVE,
    LCM_E_COMM_ACTIVE,
    LCM_E_COMM_STANDBY,
    
    /* LCM linecard error number */    
    LCM_E_LCM_PRESENT,
    LCM_E_LCM_ABSENT,
    LCM_E_LCM_MODE,
    LCM_E_LCM_NOT_READY,
    LCM_E_LCM_MSG_CTRL_ID,
    LCM_E_LCM_MSG_KEEPALIVE,
    LCM_E_LCM_MSG_SEND,
    LCM_E_LCM_MSG_RECV,
    LCM_E_LCM_MSG_ENCODE,
    LCM_E_LCM_MSG_DECODE,
    LCM_E_LCM_DELETE,

    /* LCM configure error number */ 
    LCM_E_CFG_GEN,
    LCM_E_CFG_MANAGE_IP,
    LCM_E_CFG_GATEWAY,
    LCM_E_CFG_SYNC_GEN,
    LCM_E_CFG_SWITCH_GEN,
    LCM_E_CFG_STANDBY,

    /* LCM server error number */
    LCM_E_SRV_SOCK_INIT,
    LCM_E_SRV_MSG_SEND,
    LCM_E_SRV_MSG_RECV,
    LCM_E_SRV_ACCEPT,
    LCM_E_SRV_MSG_SIZE,
    LCM_E_MAX,
} LCM_error_t;


#define LCM_E_SUCCESS_DESC                 "success"

/* LCM common error number */
#define LCM_E_NO_MEMORY_DESC               "no memory"
#define LCM_E_NOT_INIT_DESC                "not init"
#define LCM_E_NOT_FOUND_DESC               "not found"
#define LCM_E_INIT_FAILED_DESC             "init fail"
#define LCM_E_FILE_DESC                    "file fail"
#define LCM_E_TIMER_DESC                   "timer fail"
#define LCM_E_START_NSM_DESC               "start nsm fail"
#define LCM_E_IOCTL_DESC                   "ioctl fail"
#define LCM_E_FILE_OPEN_DESC               "file open fail"

/* LCM parameter check error number */
#define LCM_E_INVALID_PARAM_DESC           "invalid parameter"
#define LCM_E_INVALID_MAC_ADDR_DESC        "invalid MAC address"
#define LCM_E_INVALID_IP_ADDR_DESC         "invalid IP address"
#define LCM_E_INVALID_PTR_DESC             "invalid pointer"
#define LCM_E_INVALID_INDEX_DESC           "invalid index"
#define LCM_E_INVALID_IFINDEX_DESC         "invalid ifindex"
#define LCM_E_INVALID_IFNAME_DESC          "invalid ifname"
#define LCM_E_INVALID_LENGTH_DESC          "invalid length"
#define LCM_E_INVALID_SLOT_DESC            "invalid slot"
#define LCM_E_INVALID_PORT_DESC            "invalid port"
#define LCM_E_INVALID_CHIP_DESC            "invalid chip"
#define LCM_E_INVALID_BOARD_DESC           "invalid board"
#define LCM_E_INVALID_MODE_DESC            "invalid mode"
#define LCM_E_INVALID_SPEED_DESC           "invalid speed"
#define LCM_E_INVALID_DUPLEX_DESC          "invalid duplex"
#define LCM_E_INVALID_MEDIA_DESC           "invalid phy media"
#define LCM_E_INVALID_RESETTIME_DESC       "invalid reset time"

/* LCM socket error number */
#define LCM_E_SOCK_GEN_DESC                "socket fail"
#define LCM_E_SOCK_SEND_DESC               "socket send fail"
#define LCM_E_SOCK_RECV_DESC               "socket recv fail"
#define LCM_E_SOCK_CREATE_DESC             "socket create fail"
#define LCM_E_SOCK_BIND_DESC               "socket bind fail"
#define LCM_E_SOCK_LISTEN_DESC             "socket listen fail"
#define LCM_E_SOCK_ACCEPT_DESC             "socket accept fail"

/* LCM hardware error number */
#define LCM_E_HW_GEN_DESC                  "HW fail"
#define LCM_E_HW_CHIP_GEN_DESC             "HW chip fail"
#define LCM_E_HW_POWER_GEN_DESC            "HW power fail"
#define LCM_E_HW_TMPR_GEN_DESC             "HW temperature fail"
#define LCM_E_HW_I2C_GEN_DESC              "HW I2C fail"
#define LCM_E_HW_I2C_OPEN_DESC             "HW I2C open fail"
#define LCM_E_HW_I2C_READ_DESC             "HW I2C read fail"
#define LCM_E_HW_I2C_WRITE_DESC            "HW I2C write fail"
#define LCM_E_HW_EPLD_GEN_DESC             "HW EPLD fail"
#define LCM_E_HW_FPGA_GEN_DESC             "HW FPGA fail"
#define LCM_E_HW_EEPROM_GEN_DESC           "HW EEPROM fail"
#define LCM_E_HW_PCI_GEN_DESC              "HW PCI fail"
#define LCM_E_HW_LED_GEN_DESC              "HW LED fail"
#define LCM_E_HW_LCD_GEN_DESC              "HW LCD fail"
#define LCM_E_HW_PHY_GEN_DESC              "HW PHY fail"
#define LCM_E_HW_MDIO_GEN_DESC             "HW MDIO fail"
#define LCM_E_HW_RJ45_GEN_DESC             "HW RJ45 fail"
#define LCM_E_HW_SFP_GEN_DESC              "HW SFP fail"
#define LCM_E_HW_XFP_GEN_DESC              "HW XFP fail"
#define LCM_E_HW_SERIAL_GEN_DESC           "HW serial fail"
#define LCM_E_HW_TIMEOUT_DESC              "HW access timeout"
#define LCM_E_HW_BUSY_DESC                 "HW busy"
#define LCM_E_HW_PRESENT_DESC              "HW present error"
#define LCM_E_HW_ABSENT_DESC               "HW absent error"

/* LCM communication error number */
#define LCM_E_COMM_CONNECT_DESC            "Comm connect fail"
#define LCM_E_COMM_DISCONNECT_DESC         "Comm disconnect fail"
#define LCM_E_COMM_KEEPALIVE_DESC          "Comm keepalive fail"
#define LCM_E_COMM_ACTIVE_DESC             "active board fail"
#define LCM_E_COMM_STANDBY_DESC            "standby board fail"

/* LCM linecard error number */
#define LCM_E_LCM_PRESENT_DESC             "linecard present error"
#define LCM_E_LCM_ABSENT_DESC              "linecard absent error"
#define LCM_E_LCM_MODE_DESC                "linecard mode error"
#define LCM_E_LCM_NOT_READY_DESC           "linecard not ready"
#define LCM_E_LCM_MSG_CTRL_ID_DESC         "linecard MSG ID error"
#define LCM_E_LCM_MSG_KEEPALIVE_DESC       "linecard MSG keepalive fail"
#define LCM_E_LCM_MSG_SEND_DESC            "linecard MSG send fail"
#define LCM_E_LCM_MSG_RECV_DESC            "linecard MSG recv fail"
#define LCM_E_LCM_MSG_ENCODE_DESC          "linecard MSG encode fail"
#define LCM_E_LCM_MSG_DECODE_DESC          "linecard MSG decode fail"

/* LCM configure error number */ 
#define LCM_E_CFG_GEN_DESC                 "config fail"
#define LCM_E_CFG_MANAGE_IP_DESC           "config manage ip fail"
#define LCM_E_CFG_GATEWAY_DESC             "config gateway fail"
#define LCM_E_CFG_SYNC_GEN_DESC            "config sync fail"
#define LCM_E_CFG_SWITCH_GEN_DESC          "config switchover fail"
#define LCM_E_CFG_STANDBY_DESC             "config standby card fail"
/* get error information according to error number */
char* LCM_get_errinfo (LCM_error_t error_no);
#endif /* !__LCM_ERROR_H__ */




