#include <madCopyright.h>

/********************************************************************************
* 88E1780_MACCtrl.c
*
* DESCRIPTION:
*       APIs for 88E1780 PatGen, different with other products.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include "88E1780.h"

MAD_STATUS E1780_PatCtrlSetPRBS
(
IN  MAD_DEV   *dev,
IN  MAD_LPORT  port,
IN  MAD_PRBS_GEN_CONFIG *genCfg
)
{
	MAD_STATUS  retVal;
	MAD_U16        data;

	if (!(MAD_DEV_CAPABILITY1(dev, MAD_PHY_USGMII_PRBS_GEN)))
	{
		MAD_DBG_ERROR(("Pattern generator not supported.\n"));
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;
	}
	if (MAD_DEV_IS_IN_SWITCH(dev))
	{
		MAD_DBG_ERROR(("The phy is in switch, and do not support the function.\n"));
		return MAD_API_PHY_IN_SWITCH;
	}
	if (!(MAD_DEV_HWREALMODE(dev, port, MAD_PHY_MODE_USGMII)))
	{
		MAD_DBG_INFO(("The harware mode is not USGMII.\n"));
		return MAD_API_USGMII_NOT_SUPPORT;
	}

	data = (genCfg->polarCheckInv & 1) << 7 |
		(genCfg->polarGenInv & 1) << 6 |
		(genCfg->counterLock & 1) << 5 |
		(genCfg->patternSel & 0x3) << 2;
	if (genCfg->genEn)
		data |= 1; /* enable PRBS generator */
	if (genCfg->countEn)
	{
		if ((retVal = madHwSetPagedPhyRegField(
			dev, port, 240, 24,
			14, 1, 0)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to paged phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
		data |= (1 << 1); /* enable PRBS Checker */
	}
		

	if ((retVal = madHwWritePagedPhyReg(dev, port, MAD_PAGE30, MAD_REG_USGMII_PRBS_CTRL, data))
		!= MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to paged phy reg (%i,%i) failed.\n", MAD_PAGE30, MAD_REG_USGMII_PRBS_CTRL));
		return (retVal | MAD_API_FAIL_WRITE_PAGED_REG);
	}

	return MAD_OK;
}

MAD_STATUS E1780_PatCtrlGetPRBS
(
IN  MAD_DEV   *dev,
IN  MAD_LPORT  port,
OUT  MAD_PRBS_GEN_CONFIG *genCfg
)
{
	MAD_STATUS  retVal;
	MAD_U16        data;

	if (!(MAD_DEV_CAPABILITY1(dev, MAD_PHY_USGMII_PRBS_GEN)))
	{
		MAD_DBG_ERROR(("Pattern generator not supported.\n"));
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;
	}
	if (MAD_DEV_IS_IN_SWITCH(dev))
	{
		MAD_DBG_ERROR(("The phy is in switch, and do not support the function.\n"));
		return MAD_API_PHY_IN_SWITCH;
	}

	if ((retVal = madHwReadPagedPhyReg(dev, port, MAD_PAGE30, MAD_REG_USGMII_PRBS_CTRL, &data))
		!= MAD_OK)
	{
		MAD_DBG_ERROR(("Reading to paged phy reg (%i,%i) failed.\n", MAD_PAGE30, MAD_REG_USGMII_PRBS_CTRL));
		return (retVal | MAD_API_FAIL_WRITE_PAGED_REG);
	}
	genCfg->lockstatus = (data >> 8) & 0x1;
	genCfg->polarCheckInv = (data >> 7) & 0x1;
	genCfg->polarGenInv = (data >> 6) & 0x1;
	genCfg->counterLock = (data >> 5) & 0x1;
	genCfg->patternSel = (data >> 2) & 0x3;

	genCfg->genEn = data & 1;
	genCfg->countEn = (data >> 1) & 1;

	if ((retVal = madHwReadPagedPhyReg(dev, port, MAD_PAGE30, MAD_REG_USGMII_PRBS_ERR_COUNT_LSB, &data))
		!= MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to paged phy reg (%i,%i) failed.\n", MAD_PAGE30, MAD_REG_USGMII_PRBS_ERR_COUNT_LSB));
		return (retVal | MAD_API_FAIL_WRITE_PAGED_REG);
	}
	genCfg->errCounter = data;
	if ((retVal = madHwReadPagedPhyReg(dev, port, MAD_PAGE30, MAD_REG_USGMII_PRBS_ERR_COUNT_MSB, &data))
		!= MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to paged phy reg (%i,%i) failed.\n", MAD_PAGE30, MAD_REG_USGMII_PRBS_ERR_COUNT_MSB));
		return (retVal | MAD_API_FAIL_WRITE_PAGED_REG);
	}
	genCfg->errCounter &= 0x0000ffff;
	genCfg->errCounter |= (data << 16);

	return MAD_OK;
}

MAD_STATUS E1780_ResetPRBSCounters
(
IN  MAD_DEV   *dev,
IN  MAD_LPORT  port
)
{
	MAD_STATUS  retVal;

	if (!(MAD_DEV_CAPABILITY1(dev, MAD_PHY_USGMII_PRBS_GEN)))
	{
		MAD_DBG_ERROR(("Pattern generator not supported.\n"));
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;
	}
	if (MAD_DEV_IS_IN_SWITCH(dev))
	{
		MAD_DBG_ERROR(("The phy is in switch, and do not support the function.\n"));
		return MAD_API_PHY_IN_SWITCH;
	}

	/* Reset counter */
	if ((retVal = madHwSetPagedPhyRegField(dev, port, MAD_PAGE30, MAD_REG_USGMII_PRBS_CTRL, 4, 1, 1))
		!= MAD_OK)
	{
		MAD_DBG_ERROR(("Writing phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}
	if ((retVal = madHwSetPagedPhyRegField(dev, port, MAD_PAGE30, MAD_REG_USGMII_PRBS_CTRL, 4, 1, 0))
		!= MAD_OK)
	{
		MAD_DBG_ERROR(("Writing phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}
	return MAD_OK;
}