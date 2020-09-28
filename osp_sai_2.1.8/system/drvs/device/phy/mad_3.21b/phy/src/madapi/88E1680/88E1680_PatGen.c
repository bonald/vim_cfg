#include <madCopyright.h>

/********************************************************************************
* 88E1680_MACCtrl.c
*
* DESCRIPTION:
*       APIs for 88E1680 PatGen, different with other products.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include "88E1680.h"

MAD_STATUS E1680_PatCtrlSetPRBS
(
	IN  MAD_DEV   *dev, 
	IN  MAD_LPORT  port, 
	IN  MAD_PRBS_GEN_CONFIG *genCfg
)
{
	MAD_STATUS  retVal;
	MAD_U16        data;

	if (!(MAD_DEV_CAPABILITY1(dev, MAD_PHY_QSGMII_PRBS_GEN)))
	{
		MAD_DBG_ERROR(("Pattern generator not supported.\n"));
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;
	}
	if (MAD_DEV_IS_IN_SWITCH(dev))
	{
		MAD_DBG_ERROR(("The phy is in switch, and do not support the function.\n"));
		return MAD_API_PHY_IN_SWITCH;
	}
	if (!(MAD_DEV_HWREALMODE(dev, port, MAD_PHY_MODE_QSGMII)))
	{
		MAD_DBG_INFO(("The harware mode is not QSGMII.\n"));
		return MAD_API_QSGMII_NOT_SUPPORT;
	}

	data = (genCfg->polarCheckInv & 1) << 7 |
		(genCfg->polarGenInv & 1) << 6 |
		(genCfg->counterLock & 1) << 5 |
		(genCfg->patternSel & 0x3) << 2;
	if (genCfg->genEn)
		data |= 1; /* enable PRBS generator */
	if (genCfg->countEn)
		data |= (1 << 1); /* enable PRBS generator */

	if ((retVal = madHwWritePagedPhyReg(dev, port, MAD_PAGE4, MAD_REG_QSGMII_PRBS_CTRL, data))
		!= MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to paged phy reg (%i,%i) failed.\n", MAD_PAGE4, MAD_REG_QSGMII_PRBS_CTRL));
		return (retVal | MAD_API_FAIL_WRITE_PAGED_REG);
	}

	return MAD_OK;
}