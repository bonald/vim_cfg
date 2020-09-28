#include <madCopyright.h>

/********************************************************************************
* 88E1548P_SyncClkCtrl.c
*
* DESCRIPTION:
*       APIs for 88E1548P control Syncronous Clocking, different with other products.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include "88E1548P.h"

MAD_STATUS E1548P_SyncClkSetClkSelect(IN MAD_DEV *dev, IN MAD_LPORT port, IN MAD_8 interf, IN MAD_8 refClk)
{
	MAD_U16     data, page;
	MAD_STATUS  retVal;
	MAD_U8      reg, bits;
	MAD_U32     mode;

	if (interf == MAD_SYNC_CLK_MODE_PORT_COPPER)
	{
		mode = MAD_RESET_MODE_PORT_COMMON;
		page = MAD_PAGE18;
		reg = madLong2Char(MAD_REG_GENERAL_CONTROL_2);
		bits = 7;
	}
	else if (interf == MAD_SYNC_CLK_MODE_PORT_125SERDES)
	{
		mode = MAD_RESET_MODE_PORT_COMMON;
		page = MAD_PAGE18;
		reg = madLong2Char(MAD_REG_GENERAL_CONTROL_2);
		bits = 6;
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


MAD_STATUS E1548P_SyncClkGetClkSelect(IN MAD_DEV *dev, IN MAD_LPORT port, IN MAD_8 interf, OUT MAD_8 *refClk)
{
	MAD_U16     data, page;
	MAD_STATUS  retVal;
	MAD_U8      reg, bits;

	if (interf == MAD_SYNC_CLK_MODE_PORT_COPPER)
	{
		page = MAD_PAGE18;
		reg = madLong2Char(MAD_REG_GENERAL_CONTROL_2);
		bits = 7;
	}
	else if (interf == MAD_SYNC_CLK_MODE_PORT_125SERDES)
	{
		page = MAD_PAGE18;
		reg = madLong2Char(MAD_REG_GENERAL_CONTROL_2);
		bits = 6;
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