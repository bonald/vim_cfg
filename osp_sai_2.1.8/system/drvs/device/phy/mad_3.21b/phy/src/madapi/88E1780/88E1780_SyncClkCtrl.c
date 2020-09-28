#include <madCopyright.h>

/********************************************************************************
* 88E1780_SyncClkCtrl.c
*
* DESCRIPTION:
*       APIs for 88E1780 control Syncronous Clocking, different with other products.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include "88E1780.h"

MAD_STATUS E1780_SyncClkSetRClk(IN MAD_DEV *dev, IN MAD_LPORT port, IN MAD_8 rclk, IN MAD_8 freq)
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


MAD_STATUS E1780_SyncClkGetRClk(IN MAD_DEV *dev, IN MAD_LPORT port, OUT MAD_8 *rclk, OUT MAD_8 *freq)
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

MAD_STATUS E1780_SyncClkSetClkSelect(IN MAD_DEV *dev, IN MAD_LPORT port, IN MAD_8 interf, IN MAD_8 refClk)
{
	MAD_U16     data, page;
	MAD_STATUS  retVal;
	MAD_U8      reg, bits;
	MAD_U32     mode;

	if (interf == MAD_SYNC_CLK_MODE_PORT_COPPER)
	{
		mode = MAD_RESET_MODE_PORT_COPPER;
		page = MAD_PAGE2;
		reg = madLong2Char(MAD_REG_MAC_CONTROL_1);
		bits = 7;
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


MAD_STATUS E1780_SyncClkGetClkSelect(IN MAD_DEV *dev, IN MAD_LPORT port, IN MAD_8 interf, OUT MAD_8 *refClk)
{
	MAD_U16     data, page;
	MAD_STATUS  retVal;
	MAD_U8      reg, bits;

	if (interf == MAD_SYNC_CLK_MODE_PORT_COPPER)
	{
		page = MAD_PAGE2;
		reg = madLong2Char(MAD_REG_MAC_CONTROL_1);
		bits = 7;
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
