#include <madCopyright.h>

/********************************************************************************
* 88E1680_SysCtrl.c
*
* DESCRIPTION:
*       APIs for 88E1680 SysCtrl, different with other products.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include "88E1680.h"

MAD_STATUS E1680_SysModSoftReset(IN  MAD_DEV *dev, IN  MAD_LPORT port, IN  MAD_U8 mode)
{
	switch (mode)
	{
	case MAD_RESET_MODE_PORT_COPPER:
		return madHwPagedReset(dev, MAD_LPORT_2_PORT(port), MAD_PAGE0);
	case MAD_RESET_MODE_PORT_FIBER:
		MAD_DBG_ERROR(("Fiber Reset is not supported.\n"));
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;
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
	case MAD_RESET_MODE_PORT_COUNTER:
		MAD_DBG_ERROR(("Mode Software Reset is not supported.\n"));
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;
	case MAD_RESET_MODE_ALL_CHIP:
		if (MAD_DEV_IS_IN_SWITCH(dev))
		{
			MAD_DBG_ERROR(("The phy is in switch, and do not support the function.\n"));
			return MAD_API_PHY_IN_SWITCH;
		}
		else
			MAD_DBG_ERROR(("Chip Hardware Reset is not supported.\n"));
		    return MAD_API_NEW_FEATURE_NOT_SUPPORT;
	case MAD_RESET_MODE_PORT_COMMON:
		return madHwSetPagedPhyRegField(dev, MAD_LPORT_2_PORT(port),
			MAD_PAGE18, (MAD_U8)20, 15, 1, 1);
	default:
		MAD_DBG_ERROR(("Invalid reset mode is set %x.\n", (unsigned int)mode));
		return MAD_API_UNKNOWN_RESET_MODE;
	}

	return MAD_OK;
}

MAD_STATUS E1680_SysSetQSGMIIPwDown(IN MAD_DEV *dev, IN MAD_LPORT port, IN  MAD_BOOL en)
{
	MAD_STATUS  retVal;
	MAD_U16     data;

	/* Enable or disable PHY */
	if (en)
		data = 0;
	else
		data = 1;

	if ((MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR)) &&
		(!MAD_DEV_IS_IN_SWITCH(dev)))
	{
		/* Set or reset Power Down bit */
		if ((retVal = madHwSetPagedPhyRegField(
			dev, port, MAD_PAGE4, MAD_REG_QSGMII_GLOBAL_CTRL_1,
			11, 1, data)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
	}
	else
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;

	return MAD_OK;
}

MAD_STATUS E1680_SysGetQSGMIIPwDown(IN MAD_DEV *dev, IN MAD_LPORT port, OUT  MAD_BOOL *en)
{
	MAD_STATUS  retVal;
	MAD_U16     data;

	if ((MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR)) &&
		(!MAD_DEV_IS_IN_SWITCH(dev)))
	{
		/* Set or reset Power Down bit */
		if ((retVal = madHwGetPagedPhyRegField(
			dev, port, MAD_PAGE4, MAD_REG_QSGMII_GLOBAL_CTRL_1,
			11, 1, &data)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}

		if (data)
			*en = MAD_FALSE;
		else
			*en = MAD_TRUE;
	}
	else
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;

	return MAD_OK;
}

MAD_STATUS E1680_SysSetMacPowerDown(IN MAD_DEV *dev, IN MAD_LPORT port, IN MAD_U16 data)
{
	MAD_STATUS  retVal;

	/*  88E1680L, 88E1680 --> Mode[2:0] --> Reserved (000) */

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

MAD_STATUS E1680_SysGetMacPowerDown(IN MAD_DEV *dev, IN MAD_LPORT port, OUT MAD_BOOL *en)
{
	MAD_STATUS  retVal;
	MAD_U16    data;

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


MAD_STATUS E1680_SysSetLEDCtrl(IN MAD_DEV *dev, IN MAD_LPORT port, IN MAD_LED_CTRL_DATA *ledData)
{
	MAD_STATUS  retVal;
	MAD_U8      page, reg;
	MAD_U16     data;

	page = MAD_PAGE3;
	reg = MAD_REG_LED_FUNC_CTRL;
	
	/* Read Page 3, Reg 16 Bit 7:4,3:0 (LED1, LED0 Control) */
	if ((retVal = madHwReadPagedPhyReg(dev, (MAD_U8)port, (MAD_U16)page,
		reg, &data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Paged Phy Register read failed.\n"));
		return (retVal | MAD_API_FAIL_WRITE_PAGED_REG);
	}
	data &= 0xFF00;
	data |= ((ledData->ledCtrl[0] & 0xf)) | ((ledData->ledCtrl[1] << 4) & 0xf0);

	/* Set LED1, LED0 Control */
	if ((retVal = madHwWritePagedPhyReg(dev, (MAD_U8)port, (MAD_U16)page,
		reg, (MAD_U16)data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Paged Phy Register write failed.\n"));
		return (retVal | MAD_API_FAIL_WRITE_PAGED_REG);
	}

	reg = MAD_REG_LED_POLAR_CTRL;

	/* Read Page 3, Reg 17 Bit 15:12,11:8, 3:2, 1:0 (LED1, LED0 Mix-percentage, Polarity) */
	if ((retVal = madHwReadPagedPhyReg(dev, (MAD_U8)port, (MAD_U16)page,
		reg, &data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Paged Phy Register read failed.\n"));
		return (retVal | MAD_API_FAIL_WRITE_PAGED_REG);
	}
	data &= 0x00F0;
	data |= (ledData->ledPolarity[0] & 0x3) | ((ledData->ledPolarity[1] << 2) & 0xc)
		| ((ledData->ledMixPercen0 << 8) & 0xf00) | ((ledData->ledMixPercen1 << 12) & 0xf000);
	
	/* Set LED1, LED0 Mix-percentage, Polarity */
	if ((retVal = madHwWritePagedPhyReg(dev, (MAD_U8)port, (MAD_U16)page,
		reg, (MAD_U16)data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Paged Phy Register write failed.\n"));
		return (retVal | MAD_API_FAIL_WRITE_PAGED_REG);
	}

	reg = MAD_REG_LED_TIMER_CTRL;

	/* Read Page 3, Reg 18 Bit 15, 14:12, 10:8, 3:2, 1:0 (Force INT, Pulse stretch duration, Blink rate, Speed Off Pulse Period, Speed On Pulse Period) */
	if ((retVal = madHwReadPagedPhyReg(dev, (MAD_U8)port, (MAD_U16)page,
		reg, &data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Paged Phy Register read failed.\n"));
		return (retVal | MAD_API_FAIL_WRITE_PAGED_REG);
	}
	data &= 0x08F0;
	data |= ((ledData->ledForceInt << 15) & 0x8000) | ((ledData->ledPulseDuration << 12) & 0x7000)
		| ((ledData->ledBlinkRate << 8) & 0x700) | ((ledData->ledSppedOffPulse_perio << 2) & 0xc)
		| ((ledData->ledSppedOnPulse_perio) & 0x3);
	if ((retVal = madHwWritePagedPhyReg(dev, (MAD_U8)port, (MAD_U16)page,
		reg, (MAD_U16)data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Paged Phy Register write failed.\n"));
		return (retVal | MAD_API_FAIL_WRITE_PAGED_REG);
	}

	return MAD_OK;
}


MAD_STATUS E1680_SysGetLEDCtrl(IN MAD_DEV *dev, IN MAD_LPORT port, OUT MAD_LED_CTRL_DATA *ledData)
{
	MAD_STATUS  retVal;
	MAD_U8      page, reg;
	MAD_U16     data;

	page = MAD_PAGE3;
	reg = MAD_REG_LED_FUNC_CTRL;
	if ((retVal = madHwReadPagedPhyReg(dev, (MAD_U8)port, (MAD_U16)page,
		reg, &data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Paged Phy Register read failed.\n"));
		return (retVal | MAD_API_FAIL_WRITE_PAGED_REG);
	}
	ledData->ledCtrl[0] = data & 0xf;
	ledData->ledCtrl[1] = (data >> 4) & 0xf;
	ledData->ledCtrl[2] = 0;
	ledData->ledCtrl[3] = 0;

	reg = MAD_REG_LED_POLAR_CTRL;
	if ((retVal = madHwReadPagedPhyReg(dev, (MAD_U8)port, (MAD_U16)page,
		reg, &data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Paged Phy Register read failed.\n"));
		return (retVal | MAD_API_FAIL_WRITE_PAGED_REG);
	}
	ledData->ledPolarity[0] = data & 0x3;
	ledData->ledPolarity[1] = (data >> 2) & 0x3;
	ledData->ledPolarity[2] = 0;
	ledData->ledPolarity[3] = 0;
	ledData->ledMixPercen0 = (data >> 8) & 0xf;
	ledData->ledMixPercen1 = (data >> 12) & 0xf;

	reg = MAD_REG_LED_TIMER_CTRL;
	if ((retVal = madHwReadPagedPhyReg(dev, (MAD_U8)port, (MAD_U16)page,
		reg, &data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Paged Phy Register read failed.\n"));
		return (retVal | MAD_API_FAIL_WRITE_PAGED_REG);
	}
	ledData->ledForceInt = (data >> 15) & 0x1;
	ledData->ledPulseDuration = (data >> 12) & 0x7;
	ledData->ledBlinkRate = (data >> 8) & 0x7;
	ledData->ledSppedOffPulse_perio = (data >> 2) & 0x3;
	ledData->ledSppedOnPulse_perio = data & 0x3;

	/* For 88E1680L, 88E1680 not support LED2,3 function map,LED4,5 control and polarity  */
	reg = MAD_REG_LED_FUNC_POLAR_CTRL;
	if ((retVal = madHwReadPagedPhyReg(dev, (MAD_U8)port, (MAD_U16)page,
		reg, &data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Paged Phy Register read failed.\n"));
		return (retVal | MAD_API_FAIL_WRITE_PAGED_REG);
	}
	ledData->ledCtrl[4] = 0;
	ledData->ledCtrl[5] = 0;
	ledData->ledPolarity[4] = 0;
	ledData->ledPolarity[5] = 0;
	ledData->ledFuncMap3 = 0;
	ledData->ledFuncMap2 = 0;

	return MAD_OK;
}