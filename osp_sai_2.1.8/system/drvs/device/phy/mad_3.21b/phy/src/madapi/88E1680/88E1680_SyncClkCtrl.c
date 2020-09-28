#include <madCopyright.h>

/********************************************************************************
* 88E1680_SyncClkCtrl.c
*
* DESCRIPTION:
*       APIs for 88E1680 control Syncronous Clocking, different with other products.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include "88E1680.h"

MAD_STATUS E1680_SyncClkSetRClk(IN MAD_DEV *dev, IN MAD_LPORT port, IN MAD_8 rclk, IN MAD_8 freq)
{
	MAD_STATUS  retVal;
	MAD_U16     data;

	if (rclk == MAD_SYNC_CLK_RCLK1)
		data = 1;
	else if (rclk == MAD_SYNC_CLK_RCLK2)
		data = 2;
	else if (rclk == MAD_SYNC_CLK_RCLK1_2)
		data = 3;
	else
		data = 0;

	/* Set the MAC Interface Control Register */
	if ((retVal = madHwSetPagedPhyRegField(
		dev, port, MAD_PAGE18, MAD_REG_GENERAL_CONTROL_2, 8, 2, data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing from paged phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_READ_PAGED_REG);
	}

	if (freq == MAD_SYNC_CLK_FREQ_25M)
		data = 0;
	else if (freq == MAD_SYNC_CLK_FREQ_125M)
		data = 1;
	else
		return MAD_API_SYNCE_BAD_PARAM;
	/* Set the MAC Interface Control Register */
	if ((retVal = madHwSetPagedPhyRegField(
		dev, port, MAD_PAGE18, MAD_REG_GENERAL_CONTROL_2, 12, 1, data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing from paged phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_READ_PAGED_REG);
	}

	return MAD_OK;
}


MAD_STATUS E1680_SyncClkGetRClk(IN MAD_DEV *dev, IN MAD_LPORT port, OUT MAD_8 *rclk, OUT MAD_8 *freq)
{
	MAD_STATUS  retVal;
	MAD_U16     data;
	MAD_U32     data1;

	/* Get the MAC Interface Control Register */
	if ((retVal = madHwGetPagedPhyRegField(
		dev, port, MAD_PAGE18, MAD_REG_GENERAL_CONTROL_2, 8, 2, &data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Reading from paged phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_READ_PAGED_REG);
	}
	if (data == 1)
		*rclk = MAD_SYNC_CLK_RCLK1;
	else if (data == 2)
		*rclk = MAD_SYNC_CLK_RCLK2;
	else if (data == 3)
		*rclk = MAD_SYNC_CLK_RCLK1_2;
	else
		*rclk = MAD_SYNC_CLK_RCLK_NO;

	/* Get the MAC Interface Control Register */
	if ((retVal = madHwGetPagedPhyRegField(
		dev, port, MAD_PAGE18, MAD_REG_GENERAL_CONTROL_2, 12, 1, &data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Reading from paged phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_READ_PAGED_REG);
	}

	data1 = data;
	*freq = madLong2Char(data1);
	if (data == 0)
		*freq = MAD_SYNC_CLK_FREQ_25M;
	else if (data == 1)
		*freq = MAD_SYNC_CLK_FREQ_125M;
	else
		*freq = MAD_SYNC_CLK_FREQ_NO;

	return MAD_OK;
}


MAD_STATUS E1680_SyncClkSetClkSelect(IN MAD_DEV *dev, IN MAD_LPORT port, IN MAD_8 interf, IN MAD_8 refClk)
{
	MAD_U16     data, page;
	MAD_STATUS  retVal;
	MAD_U8      reg, bits;
	MAD_U32     mode;

	if (interf == MAD_SYNC_CLK_MODE_PORT_COPPER)
	{
		/* for 88E1680L:Copper reference clock source select (Page 2, Reg 16, Bit 7) */
		if (dev->deviceId == MAD_88E1680M)
		{
			mode = MAD_RESET_MODE_PORT_COPPER;
			page = MAD_PAGE2;
			reg = madLong2Char(MAD_REG_MAC_CONTROL_1);
			bits = 7;
		}
		/* for 88E1680:Copper reference clock source select (Page 18, Reg 27, Bit 7) */
		else
		{
			mode = MAD_RESET_MODE_PORT_COMMON;
			page = MAD_PAGE18;
			reg = madLong2Char(MAD_REG_GENERAL_CONTROL_2);
			bits = 7;
		}

	}
	else if (interf == MAD_SYNC_CLK_MODE_PORT_125SERDES)
	{
		/* 88E1680, 88E1680L not support Fiber reference clock source select */
		MAD_DBG_ERROR(("Sync Fiber is not supported.\n"));
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;

	 /*	page = MAD_PAGE2;
		reg = madLong2Char(MAD_REG_MAC_CONTROL_1);
		bits = 6;
	 */
	}
	else if ((interf == MAD_SYNC_CLK_MODE_PORT_QSGMII) &&
		(!MAD_DEV_IS_IN_SWITCH(dev)))
	{
		if (!(MAD_DEV_HWREALMODE(dev, port, MAD_PHY_MODE_QSGMII_MEDIA)))
		{
			MAD_DBG_INFO(("The harware mode is not QSGMII.\n"));
			return MAD_API_QSGMII_NOT_SUPPORT;
		}
		mode = MAD_RESET_MODE_PORT_QSGMII;
		page = MAD_PAGE4;
		reg = madLong2Char(MAD_REG_QSGMII_GLOBAL_CTRL_1);
		bits = 14;
	}
	else
		return MAD_API_SYNCE_BAD_PARAM;


	if (refClk == MAD_SYNC_CLK_REF_CLK_XTAL)
		data = 0;
	else if (refClk == MAD_SYNC_CLK_REF_CLK_SCLK)
		data = 1;
	else
		return MAD_API_SYNCE_BAD_PARAM;

	/* Set the MAC Interface Control Register */
	if ((retVal = madHwSetPagedPhyRegField(
		dev, port, page, reg, bits, 1, data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing from paged phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_READ_PAGED_REG);
	}
	/* Soft Reset */
	if ((retVal = mdSysModSoftReset(dev, port, mode)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Soft Reset failed.\n"));
		return (retVal | MAD_API_FAIL_SW_RESET);
	}

	return MAD_OK;
}


MAD_STATUS E1680_SyncClkGetClkSelect(IN MAD_DEV *dev, IN MAD_LPORT port, IN MAD_8 interf, OUT MAD_8 *refClk)
{
	MAD_U16     data, page;
	MAD_STATUS  retVal;
	MAD_U8      reg, bits;

	if (interf == MAD_SYNC_CLK_MODE_PORT_COPPER)
	{
		/* for 88E1680L:Copper reference clock source select (Page 2, Reg 16, Bit 7) */
		if (dev->deviceId == MAD_88E1680M)
		{
			page = MAD_PAGE2;
			reg = madLong2Char(MAD_REG_MAC_CONTROL_1);
			bits = 7;
		}
		/* for 88E1680:Copper reference clock source select (Page 18, Reg 27, Bit 7) */
		else
		{
			page = MAD_PAGE18;
			reg = madLong2Char(MAD_REG_GENERAL_CONTROL_2);
			bits = 7;
		}

	}
	else if (interf == MAD_SYNC_CLK_MODE_PORT_125SERDES)
	{
		/* 88E1680, 88E1680L not support Fiber reference clock source select */
		MAD_DBG_ERROR(("Sync Fiber is not supported.\n"));
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;

		/*	page = MAD_PAGE2;
		reg = madLong2Char(MAD_REG_MAC_CONTROL_1);
		bits = 6;
		*/
	}
	else if ((interf == MAD_SYNC_CLK_MODE_PORT_QSGMII) &&
		(!MAD_DEV_IS_IN_SWITCH(dev)))
	{
		if (!(MAD_DEV_HWREALMODE(dev, port, MAD_PHY_MODE_QSGMII_MEDIA)))
		{
			MAD_DBG_INFO(("The harware mode is not QSGMII.\n"));
			return MAD_API_QSGMII_NOT_SUPPORT;
		}
		page = MAD_PAGE4;
		reg = madLong2Char(MAD_REG_QSGMII_GLOBAL_CTRL_1);
		bits = 14;
	}
	else
		return MAD_API_SYNCE_BAD_PARAM;


	/* Get the MAC Interface Control Register */
	if ((retVal = madHwGetPagedPhyRegField(
		dev, port, page, reg, bits, 1, &data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Reading from paged phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_READ_PAGED_REG);
	}
	if (data == 0)
		*refClk = MAD_SYNC_CLK_REF_CLK_XTAL;
	else if (data == 1)
		*refClk = MAD_SYNC_CLK_REF_CLK_SCLK;
	else
		*refClk = MAD_SYNC_CLK_REF_CLK_NO;

	return MAD_OK;
}