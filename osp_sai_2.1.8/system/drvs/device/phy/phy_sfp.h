#ifndef __PHY_SFP_DRV__
#define __PHY_SFP_DRV__

#include "phy_drv.h"

phy_handle_t* phy_sfp_dev_register(phy_info_t* pphy_info);
int32 phy_sfp_dev_unregister(phy_handle_t* phy_handle);

#endif /* !__PHY_SFP_DRV__ */
