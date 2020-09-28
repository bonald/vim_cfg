#ifndef __PHY_VSC8488_DRV__
#define __PHY_VSC8488_DRV__

#include "phy_drv.h"

phy_handle_t* phy_vsc8488_dev_register(phy_info_t* pphy_info);
int32 phy_update_vsc8488_handle(phy_handle_t* phdl, phy_info_t* p_phyinfo);


#endif /* __PHY_QT2X25_DRV__ */
