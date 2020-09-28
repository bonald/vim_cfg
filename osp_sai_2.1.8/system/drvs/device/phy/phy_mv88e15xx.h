#ifndef __PHY_MV88E15XX_H__
#define __PHY_MV88E15XX_H__

#include "phy_drv.h"

#define MV_EXTENDED_PHY_CTL1       0x21
#define MV_PHY_INT_STATUS          0x13
#define MV_PHY_STATUS_SPEED        0xc000
#define MV_PHY_STATUS_FDX          0x2000

phy_handle_t* phy_mv88e15xx_dev_register(phy_info_t* pphy_info);

#endif /* !__PHY_LSI1082_H__ */

