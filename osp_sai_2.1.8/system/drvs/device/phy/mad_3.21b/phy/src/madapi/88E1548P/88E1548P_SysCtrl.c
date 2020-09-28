#include <madCopyright.h>

/********************************************************************************
* 88E1548P_SysCtrl.c
*
* DESCRIPTION:
*       APIs for 88E1548P SysCtrl, different with other products.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include "88E1548P.h"

MAD_STATUS E1548P_SysModSoftReset(IN  MAD_DEV *dev, IN  MAD_LPORT port, IN  MAD_U8 mode)
{
	switch (mode)
	{
	case MAD_RESET_MODE_PORT_COPPER:
		return madHwPagedReset(dev, MAD_LPORT_2_PORT(port), MAD_PAGE0);
	case MAD_RESET_MODE_PORT_FIBER:
		return madHwPagedReset(dev, MAD_LPORT_2_PORT(port), MAD_PAGE1);
	case MAD_RESET_MODE_PORT_QSGMII:
		if (MAD_DEV_IS_IN_SWITCH(dev))
		{
			MAD_DBG_ERROR(("The phy is in switch, and do not support the function.\n"));
			return MAD_API_PHY_IN_SWITCH;
		}
		else
			return madHwPagedReset(dev, MAD_LPORT_2_PORT(port), MAD_PAGE4);

	case MAD_RESET_MODE_ALL_QSGNII:
		if (MAD_DEV_IS_IN_SWITCH(dev))
		{
			MAD_DBG_ERROR(("The phy is in switch, and do not support the function.\n"));
			return MAD_API_PHY_IN_SWITCH;
		}
		else
			return madHwSetPagedPhyRegField(dev, MAD_LPORT_2_PORT(port),
			MAD_PAGE4, (MAD_U8)26, 15, 1, 1);
	/*case MAD_RESET_MODE_PORT_COUNTER:
		return madHwSetPagedPhyRegField(dev, MAD_LPORT_2_PORT(port),
			MAD_PAGE6, (MAD_U8)20, 15, 1, 1);*/
	case MAD_RESET_MODE_ALL_CHIP:
		if (MAD_DEV_IS_IN_SWITCH(dev))
		{
			MAD_DBG_ERROR(("The phy is in switch, and do not support the function.\n"));
			return MAD_API_PHY_IN_SWITCH;
		}
		else
			return madHwSetPagedPhyRegField(dev, MAD_LPORT_2_PORT(port),
			MAD_PAGE4, (MAD_U8)27, 15, 1, 1);
	case MAD_RESET_MODE_PORT_COMMON:
		return madHwSetPagedPhyRegField(dev, MAD_LPORT_2_PORT(port),
			MAD_PAGE18, (MAD_U8)20, 15, 1, 1);
	default:
		MAD_DBG_ERROR(("Invalid reset mode is set %x.\n", (unsigned int)mode));
		return MAD_API_UNKNOWN_RESET_MODE;
	}

	return MAD_OK;
}