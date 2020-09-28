#ifndef __VTSS_8X58_DRV__
#define __VTSS_8X58_DRV__

#include "phy_drv.h"

#define VTSS_8558_PART_NUMBER 0x38
#define VTSS_8658_PART_NUMBER 0x35

#define AUX_PHY_STATUS_REG      0x1c
#define PHY_CTL_SPEED_MASK      0x2040
#define PHY_CTL_DUPLEX_MASK     0x100
#define PHY_STATUS_LINK         0x4
#define PHY_STATUS_SPEED        0x18
#define PHY_STATUS_FDX          0x20
#define PHY_STATUS_AUTO         0x20
#define AUTO_NEGO_LP_CAPABILITY_REG  0x05  /* bit 11,10 asymmetric pause, symmetric pause */
#define DIAG_AUTO_EXPAN         0x6
#define EXTENDED_PHY_CTL1       0x17
#define EXTENDED_PHY_CTL2       0x18
#define PHY_INT_MASK            0x19
#define BYPASS_CTL              0x12
#define PHY_INT_STATUS          0x1a
#define PHY_LED_MODE            0x1d
#define PHY_LED_BEHAVIOR        0x1e
#define PHY_EXTENDED_PAGE       0x1f
#define PHY_E_ACTIPHY_CTL       0x14

#define PHY_10M_FULL            6
#define PHY_10M_HALF            5
#define PHY_100M_FULL           8
#define PHY_100M_HALF           7 
#define PHY_1G_FULL             9
#define PHY_1G_HALF             8

#define PHY_SPEED_CHANGE        14
#define PHY_LINK_CHANGE         13
#define PHY_FDX_CHANGE          12

phy_handle_t* phy_vtss8x58_dev_register(phy_info_t *pphy_info);

#endif /* !__VTSS_8X58_DRV__ */
