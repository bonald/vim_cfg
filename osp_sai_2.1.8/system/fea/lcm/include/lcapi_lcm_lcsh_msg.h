/****************************************************************************
 * lcapi_lcm_lcsh_msg.h :
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision :   R0.01
 * Author   :   weij
 * Date     :   2010-8-20
 ****************************************************************************/
#ifndef __LCAPI_LCM_LCSH_MSG_H__
#define __LCAPI_LCM_LCSH_MSG_H__

#include "sal_common.h"
#include "glb_phy_define.h"

#define LCAPI_LCM_LCSH_MSG_SIZE  1024
#define LCAPI_LCM_LCSH_HEAD_SIZE    8
#define LCAPI_LCM_LCSH_REG_NUM    64
#define LCSH_I2C_BUF_LEN     32

enum lcapi_lcm_lcsh_msg_type_e
{
    LCAPI_LCM_LCSH_NONE,
    LCAPI_LCM_LCSH_TEST1,
    LCAPI_LCM_LCSH_READ_REGS,
    LCAPI_LCM_LCSH_WRITE_REGS,
    LCAPI_LCM_LCSH_GET_PORT_INFO,
    LCAPI_LCM_LCSH_CONFIG_PORT,
    LCAPI_LCM_LCSH_DUMP_TRANSCEIVER_INFO,
    LCAPI_LCM_LCSH_CONFIG_BRINGUP_LOG,
    LCAPI_LCM_LCSH_CONFIG_LCM_DEBUG_LOG,
    LCAPI_LCM_LCSH_CONFIG_DRV_DEBUG_LOG,
    LCAPI_LCM_LCSH_READ_SOC_REG,
    LCAPI_LCM_LCSH_WRITE_SOC_REG,
    LCAPI_LCM_LCSH_CHANGE_WAVELENGTH,
    LCAPI_LCM_LCSH_CONFIG_PRBS,
    LCAPI_LCM_LCSH_CONFIG_TIMER,
    LCAPI_LCM_LCSH_MAX
};
typedef enum lcapi_lcm_lcsh_msg_type_e lcapi_lcm_lcsh_msg_type_t;

typedef enum
{ 
    REG_EEPROM_R,
    REG_EEPROM_W,  
    REG_GEPHY_R,
    REG_GEPHY_W,    
    REG_XGPHY_R,
    REG_XGPHY_W,
    REG_FIBER_R,   /* added by tongzb for read fiber 2016-07-08*/
    REG_FIBER_W,  /* added by tongzb for write fiber 2016-07-08*/
    REG_SENSOR_R,
    REG_SENSOR_W,
    REG_SENSOR_TMP_W,
    REG_SENSOR_TMP_R,
    REG_EPLD_R,
    REG_EPLD_W,
    REG_XGPHY_LOAD,
    REG_FAN_R,
    REG_FAN_W,
    REG_FAN_SPEED_R,
    REG_FAN_SPEED_W,
    REG_PSU_R,
    REG_PSU_W,
    REG_VSC3308_R,
    REG_VSC3308_W,
    REG_VSC3308_CONF,
    REG_I2C_R,
    REG_I2C_W,
    REG_I2C_RAW,
    REG_I2C_BRIDGE_R,
    REG_MUX_CHIP_R,
    REG_MUX_CHIP_W,
    REG_GPIO_CHIP_R,
    REG_GPIO_CHIP_W,
    REG_CLOCK_R,
    REG_CLOCK_W,
    REG_PHY_CLK_CONF,
    REG_SPI_R,
    REG_SPI_W,
    REG_SYSENV_R,
    REG_SYSENV_W,  
    REG_MAX
} reg_opcode_t;

typedef enum
{   
    CONFIG_SPEED,
    CONFIG_DUPLEX,
    CONFIG_MEDIA,
    CONFIG_LB,
    CONFIG_ENABLE,
    CONFIG_MAX
} conport_opcode_t;

typedef enum
{   
    CONFIG_PRBS_TX_ENABLE,
    CONFIG_PRBS_TX_DISABLE,
    CONFIG_PRBS_RX_CHECK,
    CONFIG_PRBS_MAX
} conprbs_opcode_t;

typedef enum 
 {
    CONFIG_WAVELENGTH,
    CONFIG_CHANNEL
 }fiber_opmode_t;
 
typedef enum
{   
    GET_PORT_NUM,
    GET_PORT_INFO,
    GET_PORT_MAX
} showport_opcode_t;

typedef enum
{   
    LCAPI_LCM_LCSH_OK,
    LCAPI_LCM_LCSH_ERR,
    LCAPI_LCM_LCSH_ERR_MAX
} lcapi_lcm_lcsh_errno_t;

struct lcapi_lcm_lcsh_msg_s
{
    uint16 msg_len;
    uint16 msg_type;
    int32 errcode;
    char msg[LCAPI_LCM_LCSH_MSG_SIZE];
};
typedef struct lcapi_lcm_lcsh_msg_s lcapi_lcm_lcsh_msg_t;

struct lcapi_lcm_lcsh_test1_msg_s
{
    uint32 int_value;
    char str_value[100];
};
typedef struct lcapi_lcm_lcsh_test1_msg_s lcapi_lcm_lcsh_test1_msg_t;

typedef struct reg_values_s {
    uint32    addr;
    uint32    value;
} reg_values_t;

typedef struct read_regs_reg_s {
    reg_opcode_t opcode;
    uint32      chip;
    uint32      addr;
    uint32      addr_1;
    uint32      count;
    uint8       indirect;
} read_regs_reg_t;

typedef struct write_regs_reg_s {
    reg_opcode_t opcode;
    uint32      chip;
    uint32      addr;
    uint32      addr_1;
    uint32      value[LCSH_I2C_BUF_LEN];
    uint8       indirect;
} write_regs_reg_t;

typedef struct portconfig_s {
    int32 up;
    glb_port_speed_t speed;
    glb_port_duplex_t duplex;
    glb_lb_phy_t loopback;
} portconfig_t;

typedef struct conport_s {
    conport_opcode_t opcode;
    uint32 port_id;
    glb_port_speed_t speed;
    glb_port_duplex_t duplex;
    glb_port_interface_t media_type;
    glb_lb_phy_t loopback;
    uint32 enable;
} conport_t;

typedef struct conprbs_s {
    conprbs_opcode_t opcode;
    uint32 port_id;
    uint8 polynome_type;
} conprbs_t;

typedef struct contimer_s {
    uint8 opcode;
    uint8 timer_type;
} contimer_t;

typedef struct fiber_conf_s {
    fiber_opmode_t opmode;
    uint32 port_id;
    uint32 value;
} fiber_conf_t;

typedef struct portstatus_s {
    int32 up;
    glb_port_speed_t speed;
    glb_port_duplex_t duplex;
} portstatus_t;

typedef struct showport_s {
    showport_opcode_t opcode;
    uint32 port_num;
    uint32 offset;
    uint8* msg;
} showport_t;

typedef struct portdesc_s {
    int32 portno;
    int32  present;
    int32  mac_id;
    glb_phy_type_t phy_type;
    int32 is_combo;
    portconfig_t admin;
    portstatus_t curstatus;
} portdesc_t;

typedef struct bringuplog_s {
    uint8 enable;
} bringuplog_t;

typedef struct lcmdebuglog_s {
    int32 debug_type;
    uint8 enable;
} lcmdebuglog_t;

typedef struct read_soc_reg_s {
    uint64_t baseaddr;
    uint64_t offset;
    uint64_t width;
    uint64_t count;
} read_soc_reg_t;

typedef struct write_soc_reg_s {
    uint64_t baseaddr;
    uint64_t offset;
    uint64_t width;
    uint64_t value;
} write_soc_reg_t;

#endif /*__LCAPI_LCM_LCSH_MSG_H__*/
