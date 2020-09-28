/**
 @file sys_usw_chip.h

 @date 2009-10-19

 @version v2.0

 The file contains all chip related APIs of sys layer
*/

#ifndef _SYS_USW_PERI_H
#define _SYS_USW_PERI_H
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "ctc_const.h"
#include "ctc_chip.h"
#include "ctc_debug.h"
#include "sys_usw_common.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/

#define SYS_USW_DATAPATH_SERDES_NUM              40
#define SYS_USW_MDIO_CTLR_NUM                    4
#define SYS_USW_MACLED_BLINK_INTERVAL_MAX        0x7FFFFFFF

#define SYS_PERI_DBG_OUT(level, FMT, ...) \
    do { \
        CTC_DEBUG_OUT(chip, peri, PERI_SYS, level, FMT, ##__VA_ARGS__); \
    } while (0)

#define PERI_LOCK(lchip) \
    if (p_usw_peri_master[lchip]->p_peri_mutex) sal_mutex_lock(p_usw_peri_master[lchip]->p_peri_mutex)

#define PERI_UNLOCK(lchip) \
    if (p_usw_peri_master[lchip]->p_peri_mutex) sal_mutex_unlock(p_usw_peri_master[lchip]->p_peri_mutex)

#define SWITCH_LOCK(lchip) \
    if (p_usw_peri_master[lchip]->p_switch_mutex) sal_mutex_lock(p_usw_peri_master[lchip]->p_switch_mutex)

#define SWITCH_UNLOCK(lchip) \
    if (p_usw_peri_master[lchip]->p_switch_mutex) sal_mutex_unlock(p_usw_peri_master[lchip]->p_switch_mutex)

#define SYS_PERI_I2C_LOCK(lchip) \
    if(p_usw_peri_master[lchip]->p_i2c_mutex) sal_mutex_lock(p_usw_peri_master[lchip]->p_i2c_mutex)
#define SYS_PERI_I2C_UNLOCK(lchip) \
    if(p_usw_peri_master[lchip]->p_i2c_mutex) sal_mutex_unlock(p_usw_peri_master[lchip]->p_i2c_mutex)
#define SYS_PERI_SMI_LOCK(lchip) \
    if(p_usw_peri_master[lchip]->p_smi_mutex) sal_mutex_lock(p_usw_peri_master[lchip]->p_smi_mutex)
#define SYS_PERI_SMI_UNLOCK(lchip) \
    if(p_usw_peri_master[lchip]->p_smi_mutex) sal_mutex_unlock(p_usw_peri_master[lchip]->p_smi_mutex)
#define SYS_CHIP_MAC_LED_CLK    2.5
#define SYS_CHIP_MAC_LED_CLK_TWOFOLD 5

#define SYS_CHIP_I2C_MAX_BITAMP     48
#define SYS_CHIP_I2C_32BIT_DEV_ID   32   /*max dev id of first 32 bits*/
#define SYS_CHIP_I2C_READ_MAX_LENGTH    384

#define SYS_CHIP_LED_FORCE_OFF      0
#define SYS_CHIP_LED_ON_RX_LINK     1
#define SYS_CHIP_LED_ON_TX_LINK     2
#define SYS_CHIP_LED_FORCE_ON       3

#define SYS_CHIP_LED_BLINK_OFF      0
#define SYS_CHIP_LED_BLINK_RX_ACT   1
#define SYS_CHIP_LED_BLINK_TX_ACT   2
#define SYS_CHIP_LED_BLINK_ON       3

#define SYS_CHIP_1G_MDIO_READ       0x2
#define SYS_CHIP_XG_MDIO_READ       0x3

#define SYS_CHIP_MDIO_WRITE         0x1
#define SYS_CHIP_MAX_GPIO_NUM       16
#define SYS_CHIP_MAX_PHY_PORT       68

#define SYS_CHIP_INVALID_SWITCH_ID  0
#define SYS_CHIP_MIN_SWITCH_ID      1
#define SYS_CHIP_MAX_SWITCH_ID      16


#define SYS_CHIP_CONTROL0_ID         0
#define SYS_CHIP_CONTROL1_ID         1
#define SYS_CHIP_CONTROL_NUM         2

#define SYS_PERI_MDIO_LANE0_ID        0
#define SYS_PERI_MDIO_LANE1_ID        1
#define SYS_PERI_MDIO_LANE2_ID        2
#define SYS_PERI_MDIO_LANE3_ID        3
#define SYS_PERI_LANE_NUM             4

#define SYS_PERI_MDIO_INTF0_ID     0
#define SYS_PERI_MDIO_INTF1_ID     1

#define SYS_CHIP_MAX_GPIO           4

#define SYS_CHIP_MAX_TIME_OUT       256

#define SYS_CHIP_MAC_GPIO_ID        4

#define SYS_CHIPMDIOXG1_ADDR 0x40001268
#define SYS_CHIPMDIOXG0_ADDR 0x20001268

#define SYS_REG_CLKSLOW_FREQ   125

#define SYS_CHIP_FLAG_ISSET(VAL, FLAG)        (((VAL)&(FLAG)) == (FLAG))
#define SYS_CHIP_FLAG_ISZERO(VAL)        ((VAL) == 0)

#define SYS_PERI_GPIO_IS_HS(ID) ((ID>=SYS_PERI_MAX_GPIO_ID)?TRUE:FALSE)
#define SYS_PERI_MAX_GPIO_ID  16
#define SYS_PERI_HS_MAX_GPIO_ID (SYS_PERI_MAX_GPIO_ID+18)

#define SetMdioScanCtl(ctl_id, field, p_scan_para, value)\
    if (ctl_id)\
    {\
        SetMdioScanCtl01(V, field, p_scan_para, value);\
    }\
    else\
    {\
        SetMdioScanCtl00(V, field, p_scan_para, value);\
    }
#define SetI2CMasterBmpCfg(ctl_id, p_bitmap_ctl, p_slave_bitmap)\
    if (ctl_id)\
    {\
        SetI2CMasterBmpCfg1(A, slaveBitmap_f, p_bitmap_ctl, p_slave_bitmap);\
    }\
    else\
    {\
        SetI2CMasterBmpCfg0(A, slaveBitmap_f, p_bitmap_ctl, p_slave_bitmap);\
    }

#define SetI2CMasterReadCfg(ctl_id, field, p_master_rd, value)\
    if(ctl_id)\
    {\
        SetI2CMasterReadCfg1(V, field, p_master_rd, value);\
    }\
    else\
    {\
        SetI2CMasterReadCfg0(V, field, p_master_rd, value);\
    }

#define SetI2CMasterReadCtl(ctl_id, field, p_read_ctl, value)\
    if (ctl_id)\
    {\
        SetI2CMasterReadCtl1(V, field, p_read_ctl, value);\
    }\
    else\
    {\
        SetI2CMasterReadCtl0(V, field, p_read_ctl, value);\
    }

#define GetI2CMasterStatus(ctl_id, field, p_master_status, value)\
    if (ctl_id)\
    {\
        value = GetI2CMasterStatus1(V, field, p_master_status);\
    }\
    else\
    {\
        value = GetI2CMasterStatus0(V, field, p_master_status);\
    }

#define SetI2CMasterWrCfg(ctl_id, field, p_master_wr, value)\
    if (ctl_id)\
    {\
        SetI2CMasterWrCfg1(V, field, p_master_wr, value);\
    }\
    else\
    {\
        SetI2CMasterWrCfg0(V, field, p_master_wr, value);\
    }

#define GetI2CMasterWrCfg(ctl_id, field, p_master_wr, value)\
    if (ctl_id)\
    {\
        value = GetI2CMasterWrCfg1(V, field, p_master_wr);\
    }\
    else\
    {\
        value = GetI2CMasterWrCfg0(V, field, p_master_wr);\
    }

#define GetI2CMasterDebugState(ctl_id, field, p_master_dbg, value)\
    if (ctl_id)\
    {\
        value = GetI2CMasterDebugState1(V, field, p_master_dbg);\
    }\
    else\
    {\
        value = GetI2CMasterDebugState0(V, field, p_master_dbg);\
    }
#define GetI2CMasterReadStatus(ctl_id, p_read_status, arr_read_status)\
        if (ctl_id)\
        {\
            GetI2CMasterReadStatus1(A, readStatusBmp_f, p_read_status, arr_read_status);\
        }\
        else\
        {\
            GetI2CMasterReadStatus0(A, readStatusBmp_f, p_read_status, arr_read_status);\
        }
#define SetI2CMasterPollingCfg(ctl_id, field, p_master_wr, value)\
        if (ctl_id)\
        {\
            SetI2CMasterPollingCfg1(V, field, p_master_wr, value);\
        }\
        else\
        {\
            SetI2CMasterPollingCfg0(V, field, p_master_wr, value);\
        }
#define SetMdioUserPhy(ctl_id, field, p_use_phy, arr_value)\
    if (ctl_id)\
    {\
        SetMdioUserPhy1(A, field, p_use_phy, arr_value);\
    }\
    else\
    {\
        SetMdioUserPhy0(A, field, p_use_phy, arr_value);\
    }

#define SYS_PERI_INIT_CHECK(lchip) \
    do { \
        LCHIP_CHECK(lchip); \
        if (NULL == p_usw_peri_master[lchip]) \
        { \
            return CTC_E_NOT_INIT; \
        } \
    } while (0)


struct sys_chip_phy_shim_s
{
    ctc_slistnode_t head;
    ctc_chip_phy_shim_t phy_shim;
    uint32 is_init;
};
typedef struct sys_chip_phy_shim_s sys_chip_phy_shim_t;

struct sys_peri_master_s
{
    sal_mutex_t* p_peri_mutex;
    sal_mutex_t* p_switch_mutex;
    uint8   resv[2];
    char    datapath_mode[20];
    uint8   first_ge_opt_reg_used;
    uint8   first_ge_opt_reg;
    uint8   second_ge_opt_reg_used;
    uint8   second_ge_opt_reg;
    uint8   port_mdio_mapping_tbl[SYS_USW_MAX_PHY_PORT];
    uint8   port_phy_mapping_tbl[SYS_USW_MAX_PHY_PORT];
    uint8   port_dev_no_mapping_tbl[SYS_USW_MAX_PHY_PORT];
    ctc_chip_phy_shim_t**   phy_register;
    ctc_slist_t* phy_list;
    sal_task_t*  p_polling_scan;

    sal_mutex_t* p_i2c_mutex;
    sal_mutex_t* p_smi_mutex;
};
typedef struct sys_peri_master_s sys_peri_master_t;

enum sys_peri_mdio_bus_e
{
    SYS_CHIP_MDIO_BUS0,
    SYS_CHIP_MDIO_BUS1,
    SYS_CHIP_MDIO_BUS2,
    SYS_CHIP_MDIO_BUS3,

    SYS_CHIP_MAX_MDIO_BUS
};
typedef enum sys_peri_mdio_bus_e sys_peri_mdio_bus_t;

#define SYS_CHIP_MAC_LED_CLK    2.5
#define SYS_CHIP_MAC_LED_CLK_TWOFOLD 5

struct sys_chip_serdes_mac_info_s
{
    char* p_datapath_mode;
    uint8 serdes_id[60];
};
typedef struct sys_chip_serdes_mac_info_s sys_chip_serdes_mac_info_t;

struct sys_chip_i2c_slave_bitmap_s
{
    uint32 slave_bitmap1;
    uint32 slave_bitmap2;
};
typedef struct sys_chip_i2c_slave_bitmap_s sys_chip_i2c_slave_bitmap_t;

enum sys_chip_sfp_scan_op_e
{
    SYS_CHIP_SFP_INTERVAL_OP = 0x01,
    SYS_CHIP_SFP_BITMAP_OP = 0x02,
    SYS_CHIP_SFP_SCAN_REG_OP = 0x04
};
typedef enum sys_chip_sfp_scan_op_e sys_chip_sfp_scan_op_t;

enum sys_chip_mac_led_op_e
{
    SYS_CHIP_LED_MODE_SET_OP = 0x01,
    SYS_CHIP_LED_POLARITY_SET_OP = 0x02
};
typedef enum sys_chip_mac_led_op_e sys_chip_mac_led_op_t;
/****************************************************************************
 *
* Function
*
*****************************************************************************/
extern int32
sys_usw_peri_init(uint8 lchip, uint8 lchip_num);

extern int32
sys_usw_peri_set_phy_mapping(uint8 lchip, ctc_chip_phy_mapping_para_t* phy_mapping_para);

extern int32
sys_usw_peri_get_phy_mapping(uint8 lchip, ctc_chip_phy_mapping_para_t* phy_mapping_para);

extern int32
sys_usw_peri_set_phy_scan_para(uint8 lchip, ctc_chip_phy_scan_ctrl_t* p_scan_para);

extern int32
sys_usw_peri_get_phy_scan_para(uint8 lchip, ctc_chip_phy_scan_ctrl_t* p_scan_para);

extern int32
sys_usw_peri_set_phy_scan_en(uint8 lchip, bool enable);

extern int32
sys_usw_peri_get_phy_scan_en(uint8 lchip, bool* enable);

extern int32
sys_usw_peri_set_i2c_scan_para(uint8 lchip, ctc_chip_i2c_scan_t* p_i2c_para);

extern int32
sys_usw_peri_get_i2c_scan_para(uint8 lchip, ctc_chip_i2c_scan_t* p_i2c_para);

extern int32
sys_usw_peri_set_i2c_scan_en(uint8 lchip, bool enable);

extern int32
sys_usw_peri_get_i2c_scan_en(uint8 lchip, bool* enable);

extern int32
sys_usw_peri_read_i2c_buf(uint8 lchip, ctc_chip_i2c_scan_read_t *p_i2c_scan_read);

extern int32
sys_usw_peri_i2c_read(uint8 lchip, ctc_chip_i2c_read_t* p_i2c_para);

extern int32
sys_usw_peri_i2c_write(uint8 lchip, ctc_chip_i2c_write_t* p_i2c_para);

extern int32
sys_usw_peri_set_mac_led_mode(uint8 lchip, ctc_chip_led_para_t* p_led_para, ctc_chip_mac_led_type_t led_type, uint8 inner);

extern int32
sys_usw_peri_set_mac_led_mapping(uint8 lchip, ctc_chip_mac_led_mapping_t* p_led_map);

extern int32
sys_usw_peri_set_mac_led_en(uint8 lchip, bool enable);

extern int32
sys_usw_peri_get_mac_led_en(uint8 lchip, bool* enable);

extern int32
sys_usw_peri_set_gpio_mode(uint8 lchip, uint8 gpio_id, ctc_chip_gpio_mode_t mode);

extern int32
sys_usw_peri_set_gpio_output(uint8 lchip, uint8 gpio_id, uint8 out_para);

extern int32
sys_usw_peri_get_gpio_input(uint8 lchip, uint8 gpio_id, uint8* in_value);

extern int32
sys_usw_peri_get_serdes_info(uint8 lchip, uint8 lport, uint8* macro_idx, uint8* link_idx);

extern int32
sys_usw_peri_set_serdes_mode(uint8 lchip, ctc_chip_serdes_info_t* p_serdes_info);

extern int32
sys_usw_peri_mdio_read(uint8 lchip, ctc_chip_mdio_type_t type, ctc_chip_mdio_para_t* p_para);

extern int32
sys_usw_peri_mdio_write(uint8 lchip, ctc_chip_mdio_type_t type, ctc_chip_mdio_para_t* p_para);

extern int32
sys_usw_peri_set_mdio_clock(uint8 lchip, ctc_chip_mdio_type_t type, uint16 freq);

extern int32
sys_usw_peri_set_property(uint8 lchip, ctc_chip_property_t chip_prop, void* p_value);

extern int32
sys_usw_get_chip_sensor(uint8 lchip, ctc_chip_sensor_type_t type, uint32* p_value);

extern int32
sys_usw_peri_get_property(uint8 lchip, ctc_chip_property_t chip_prop, void* p_value);

extern int32
sys_usw_peri_set_dlb_chan_type(uint8 lchip, uint8 chan_id);

extern int32
sys_usw_peri_get_mdio_clock(uint8 lchip, ctc_chip_mdio_type_t type, uint16* freq);

extern int32
sys_usw_peri_deinit(uint8 lchip);

extern int32
sys_usw_peri_get_gport_by_mdio_para(uint8 lchip, uint8 bus, uint8 phy_addr, uint32* gport);

extern int32
sys_usw_peri_phy_link_change_isr(uint8 lchip, uint32 intr, void * p_data);

extern int32
sys_usw_peri_set_mac_led_clock(uint8 lchip, uint16 freq);

extern int32
sys_usw_peri_get_mac_led_clock(uint8 lchip, uint16* freq);

extern int32
sys_usw_peri_set_i2c_clock(uint8 lchip, uint8 ctl_id, uint16 freq);

extern int32
sys_usw_peri_get_i2c_clock(uint8 lchip, uint8 ctl_id, uint16* freq);

extern int32
sys_usw_peri_get_phy_register_exist(uint8 lchip, uint16 lport);

extern int32
sys_usw_peri_get_phy_id(uint8 lchip, uint16 lport, uint32* phy_id);

extern int32
sys_usw_peri_get_phy_prop(uint8 lchip, uint16 lport, uint16 type, uint32* p_value);

extern int32
sys_usw_peri_set_phy_prop(uint8 lchip, uint16 lport, uint16 type, uint32 value);

#ifdef __cplusplus
}
#endif

#endif

