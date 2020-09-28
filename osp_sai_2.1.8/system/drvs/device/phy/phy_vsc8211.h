#ifndef __VTSS_8211_DRV__
#define __VTSS_8211_DRV__

#include "phy_drv.h"

#define PHY_EXTENDED_PAGE       0x1f
#define AUX_PHY_STATUS_REG      0x1c
#define PHY_INT_MASK            0x19
#define PHY_INT_STATUS          0x1a
#define EXTENDED_PHY_CTL1       0x17

phy_handle_t* phy_vtss8211_dev_register(phy_info_t* pphy_info);


#endif /* !__VTSS_8211_DRV__ */
