#ifndef __FIBER_API_H__
#define __FIBER_API_H__

#include "fiber_drv.h"
#include "glb_phy_define.h"

#define SFP_P_10GBASE_ER (1<<7)
#define SFP_P_10GBASE_LRM (1<<6)
#define SFP_P_10GBASE_LR (1<<5)
#define SFP_P_10GBASE_SR (1<<4)

#define SFP_P_10GBASE_ACTIVE_CABLE (1<<3)
#define SFP_P_10GBASE_PASSIVE_CABLE (1<<2)
#define SFP_P_25GBASE_PASSIVE_CABLE (1<<2)

#define XFP_10GBASE_SR (1<<7)
#define XFP_10GBASE_LR (1<<6)
#define XFP_10GBASE_ER (1<<5)
#define XFP_10GBASE_LRM (1<<4)

//#define QSFP_10GBASE_LRM (1<<6)
//#define QSFP_10GBASE_LR  (1<<5)
//#define QSFP_10GBASE_SR  (1<<4)
#define QSFP_COMPLIANCE_EXT (1<<7)
#define QSFP_100GBASE_CR4_DEF      0x0 //Just for compatible with LEONI cable
#define QSFP_100G_AOC              0x1
#define QSFP_100GBASE_SR4          0x2
#define QSFP_100GBASE_LR4          0x3
#define QSFP_100GBASE_ER4          0x4
#define QSFP_100GBASE_SR10         0x5
#define QSFP_100G_CWDM4_MSA_FEC    0x6
#define QSFP_100G_PSM4_SMF         0x7
#define QSFP_100G_ACC              0x8
#define QSFP_100G_CWDM4_MSA_NOFEC  0x9
#define QSFP_100GBASE_CR4          0xb
#define QSFP_40GBASE_ER4           0x10
#define QSFP_4X10GBASE_SR          0x11
#define QSFP_40G_PSM4_SMF          0x12
#define QSFP_40GBASE_CR4   (1<<3)
#define QSFP_40GBASE_SR4   (1<<2)
#define QSFP_40GBASE_LR4   (1<<1)
#define QSFP_40GBASE_XLPPI (1<<0)
#define QSFP_100GBASE_DEF  0  //Just for compatible with INNOLIGHT fiber

/* support 25G */
#define SFP_25G_AOC_BER_H       0x1
#define SFP_25GBASE_SR          0x2
#define SFP_25GBASE_LR          0x3
#define SFP_25GBASE_ER          0x4
#define SFP_25G_ACC_BER_H       0x8
#define SFP_25GBASE_CR_L        0xb
#define SFP_25GBASE_CR_S        0xc
#define SFP_25GBASE_CR_N        0xd
#define SFP_25G_AOC_BER_L       0x18
#define SFP_25G_ACC_BER_L       0x19

#define SFP_ETH_ORIGINAL (0)
#define SFP_ETH_BASE_PX (1 << 7)
#define SFP_ETH_BASE_BX10 (1 << 6)
#define SFP_ETH_100BASE_FX  (1 << 5)
#define SFP_ETH_100BASE_LX  (1 << 4)
#define SFP_ETH_1000BASE_T  (1 << 3)
#define SFP_ETH_1000BASE_CX (1 << 2)
#define SFP_ETH_1000BASE_LX (1 << 1)
#define SFP_ETH_1000BASE_SX (1 << 0)

/*fiber need poll 3 times then change state from absent to present */
typedef enum
{
    FIBER_ABSENT,
    FIBER_INIT1,
    FIBER_INIT2,
    FIBER_PRESENT,
}fiber_present_state_t;

typedef enum
{
    FIBER_MEDIA_QSFP_PLUS = 0,
    FIBER_MEDIA_SFP_PLUS,
    FIBER_MEDIA_MAX
}fiber_media_type_t;

/* enable auto negotiation */
typedef enum
{
    FIBER_DISABLE_AUTO_NEGO = 0,
    FIBER_ENABLE_AUTO_NEGO,
}fiber_enable_auto_nego_t;

int32 fiber_init(uint32 sfp_num, uint32 port_num, fiber_port_info_t* fiber_port_info);
int32 fiber_read(uint32 port_id, fiber_para_t *ppara);
int32 fiber_write(uint32 port_id, fiber_para_t *ppara);
int32 fiber_get_chip_info(uint32 port_id, fiber_info_t* p_fiber);
int32 fiber_update_state(uint32 port_id, fiber_info_t* p_fiber);
int32 fiber_get_present(uint32 port_id, uint8* status);
/* Modified by liuht to support QSFP+ for bug25808, 2013-11-20 */
int32 fiber_get_los(uint32 port_id, uint8* status, fiber_info_t* p_fiber);
int32 fiber_set_enable(uint32 port_id, uint8 tx_enable);
int32 sfp_get_internal_phy_status(uint32 port_id, glb_phy_state_t* phy_state, glb_port_cfg_t* port_cfg);
int32 sfp_cfg_internal_phy_mode(uint32 port_id);
int32 sfp_cfg_internal_phy_enable(uint32 port_id, int8 enable);
int32 sfp_cfg_internal_phy_speed_duplex(uint32 port_id, glb_port_speed_t, glb_port_duplex_t);
int32 sfp_cfg_internal_phy_flowctrl(uint32 port_id, uint16 direction, uint16 ability, glb_port_speed_t speed, glb_port_duplex_t duplex);
void fiber_get_type_str(fiber_info_t *fiber_info, char *type_str);
void fiber_dump_diag_info(char* ort_name, fiber_info_t* p_fiber, FILE *fp);
void fiber_dump_basic_info(char* ort_name, fiber_info_t* p_fiber, FILE *fp);
int32 check_fiber_support_ddm(fiber_info_t* p_fiber);
int32 check_fiber_support_tunable(fiber_info_t* p_fiber);
int32 check_fiber_is_XFP(uint32 port_id);
int32 check_fiber_is_QSFP(uint32 port_id);
int32 fiber_is_direct_attach_cable(fiber_info_t* p_fiber);
int32 fiber_get_wavelength_by_freq(uint32 port_id, uint32  *freq,fiber_info_t *fiber_info);
int32 fiber_config_wavelength(uint32 port_id, uint32 value, uint32 mode,fiber_info_t *fiber_info);
int32 fiber_is_mellanox_cable(fiber_info_t* p_fiber);

//int get_sfp_phy_type(sfp_info_t *sfp_info, uint32* phy_type);
#endif /* !__SFP_API_H__ */

