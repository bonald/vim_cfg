   #include <madCopyright.h>
/********************************************************************************
* 88E1780_SysCtrl.c
*
* DESCRIPTION:
*       APIs for 88E1780 SysCtrl, different with other products.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include "88E1780.h"

MAD_STATUS E1780_SysModSoftReset(IN  MAD_DEV *dev, IN  MAD_LPORT port, IN  MAD_U8 mode)
{
	switch (mode)
	{
	case MAD_RESET_MODE_PORT_COPPER:
		return madHwPagedReset(dev, MAD_LPORT_2_PORT(port), MAD_PAGE0);
	case MAD_RESET_MODE_PORT_USGMII:
		if (MAD_DEV_IS_IN_SWITCH(dev))
		{
			MAD_DBG_ERROR(("The phy is in switch, and do not support the function.\n"));
			return MAD_API_PHY_IN_SWITCH;
		}
		else
			return madHwPagedReset(dev, MAD_LPORT_2_PORT(port), MAD_PAGE30);
	case MAD_RESET_MODE_ALL_USGMII:
		return madHwSetPagedPhyRegField(dev, MAD_LPORT_2_PORT(port),
			MAD_PAGE30, (MAD_U8)26, 15, 1, 1);
	case MAD_RESET_MODE_PORT_COMMON:
		return madHwSetPagedPhyRegField(dev, MAD_LPORT_2_PORT(port),
			MAD_PAGE18, (MAD_U8)20, 15, 1, 1);
	default:
		MAD_DBG_ERROR(("Invalid reset mode is set %x.\n", (unsigned int)mode));
		return MAD_API_UNKNOWN_RESET_MODE;
	}

	return MAD_OK;
}

MAD_STATUS E1780_SysSetMacPowerDown(IN MAD_DEV *dev, IN MAD_LPORT port, IN MAD_U16 data)
{
	MAD_STATUS  retVal;

	/* Set or reset Power Down bit */
	if ((retVal = madHwSetPagedPhyRegField(
		dev, port, MAD_PAGE2, MAD_REG_MAC_CONTROL_1,
		3, 1, data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}

	/* Soft Reset */
	if ((retVal = mdSysSoftReset(dev, port)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Soft Reset failed.\n"));
		return (retVal | MAD_API_FAIL_SW_RESET);
	}

	return MAD_OK;
}

MAD_STATUS E1780_SysGetMacPowerDown(IN MAD_DEV *dev, IN MAD_LPORT port, OUT MAD_BOOL *en)
{
	MAD_STATUS  retVal;
	MAD_U16    data;

	/* Get Power Down bit */
	if ((retVal = madHwGetPagedPhyRegField(
		dev, port, MAD_PAGE2, MAD_REG_MAC_CONTROL_1,
		3, 1, &data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Reading to phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}

	if (data)
		*en = MAD_FALSE;
	else
		*en = MAD_TRUE;

	return MAD_OK;
}

MAD_STATUS E1780_SysSetEnergyDetect(IN MAD_DEV *dev, IN MAD_LPORT port, IN MAD_U16 mode)
{
	MAD_STATUS  retVal;
	/* Set the Energe Detect Power down mode */
	if ((retVal = madHwSetPagedPhyRegField(dev, port, MAD_PAGE0, MAD_REG_COPPER_CONTROL_1, 7, 3, mode))
		!= MAD_OK)
	{
		MAD_DBG_ERROR(("Writing phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}
	return MAD_OK;
}

MAD_STATUS E1780_SysGetEnergyDetect(IN MAD_DEV *dev, IN MAD_LPORT port, OUT MAD_U16 *mode)
{
	MAD_STATUS  retVal;
	MAD_U16    data;
	/* Get the Energe Detect Power down mode */
	if ((retVal = madHwGetPagedPhyRegField(dev, port, MAD_PAGE0, MAD_REG_COPPER_CONTROL_1, 7, 3, &data))
		!= MAD_OK)
	{
		MAD_DBG_ERROR(("Reading phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_GET_PAGED_REG_FIELD);
	}
	*mode = data;
	return MAD_OK;
}
