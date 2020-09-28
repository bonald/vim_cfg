#ifndef __PHY_VSC8504_H__
#define __PHY_VSC8504_H__

#include "phy_drv.h"

#define PHY_EXTENDED_PAGE       0x1f
#define AUX_PHY_STATUS_REG      0x1c
#define PHY_INT_MASK            0x19
#define PHY_INT_STATUS          0x1a
#define EXTENDED_PHY_CTL1       0x17
#define BYPASS_CTL              0x12

#define PHY_LED_MODE            0x1d
#define PHY_LED_BEHAVIOR        0x1e

/* Register 18G */
#define VTSS_PHY_MICRO_PAGE     18

/* Register 14G + Bit fields */
#define VTSS_PHY_GPIO_CONTROL_2 14
#define VTSS_F_PHY_GPIO_CONTROL_2_COMA_MODE_OUTPUT_ENABLE  (1U << 13)
#define VTSS_F_PHY_GPIO_CONTROL_2_COMA_MODE_OUTPUT_DATA    (1U << 12)

#define VTSS_F_PHY_MODE_CONTROL_AUTO_NEG_ENA               (1U << 12)
#define VTSS_F_PHY_MODE_CONTROL_RESTART_AUTO_NEG           (1U << 9)
#define VTSS_F_PHY_MODE_CONTROL_SW_RESET                   (1U << 15)

#define VSC8504_PHY_MAX_PORT    4

/* The address at where the first byte of the internal 8051 firmware is placed. */
#define FIRMWARE_START_ADDR 0x4000

phy_handle_t* phy_vsc8504_dev_register(phy_info_t* pphy_info);

#endif /* !__PHY_VSC8504_H__ */

