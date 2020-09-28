#include <madCopyright.h>
/********************************************************************************
* madCpCtrlSample.c
*
* DESCRIPTION:
*       Copper setup sample (EnableCpAutoNeg, DisableCpAutoNeg,
*       ReadCpAutoNeg, and so on).

* DEPENDENCIES:
*       MAD Driver should be loaded and Phys should be enabled
*       before running sample test functions as specified in
*       sample\Initialization\firefox\madApiInit.c.
*
* FILE REVISION NUMBER:
*
*******************************************************************************/
#include "madSample.h"

/********************************************************************************
*       AN_mode - if en is MAD_TRUE, combination of the followings:
*                   MAD_AUTO_AD_ASYM_PAUSE,
*                   MAD_AUTO_AD_PAUSE,
*                   MAD_AUTO_AD_100FDX,
*                   MAD_AUTO_AD_100HDX,
*                   MAD_AUTO_AD_10FDX,
*                   MAD_AUTO_AD_10HDX,
*                   MAD_AUTO_AD_1000FDX,
*                   MAD_AUTO_AD_1000HDX,
*                   MAD_AUTO_AD_MANUAL_CONF_MS,
*                   MAD_AUTO_AD_FORCE_MASTER, and
*                   MAD_AUTO_AD_PREFER_MULTI_PORT
*              if en is MAD_FALSE,
*                   MAD_PHY_1000FDX,
*                   MAD_PHY_1000HDX,
*                   MAD_PHY_100FDX,
*                   MAD_PHY_100HDX,
*                   MAD_PHY_10FDX, or
*                   MAD_PHY_10HDX
*******************************************************************************/

MAD_U32 AN_mode = MAD_AUTO_AD_MASK_REG4 | MAD_AUTO_AD_MASK_REG9;

/*
* Enable Auto Negotiation.
*/
MAD_STATUS EnableCpAutoNeg(MAD_DEV *dev, MAD_LPORT port)
{
	MAD_STATUS status;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Enable Copper AN for port %i\n", (int)port));

	/*
	*    Enable Auto Negotiation.
	*/
	if ((status = mdCopperSetAutoNeg(dev, port, MAD_TRUE, AN_mode)) != MAD_OK)
	{
		MSG_PRINT(("mdCopperSetAutoNeg return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Disable Auto Negotiation.
*/
MAD_STATUS DisableCpAutoNeg(MAD_DEV *dev, MAD_LPORT port)
{
	MAD_STATUS status;
	MAD_U32   mode = 3;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Disable Copper AN for port %i\n", (int)port));

	/*
	*    Enable Auto Negotiation.
	*/
	if ((status = mdCopperSetAutoNeg(dev, port, MAD_FALSE, mode)) != MAD_OK)
	{
		MSG_PRINT(("DisableCpAutoNeg return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Read Auto Negotiation Status.
*/
MAD_STATUS ReadCpAutoNeg(MAD_DEV *dev, MAD_LPORT port)
{
	MAD_STATUS status;
	MAD_U32 mode;
	MAD_BOOL en;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Read Copper AN Status for port %i\n", (int)port));

	/*
	*    Enable Auto Negotiation.
	*/
	if ((status = mdCopperGetAutoNeg(dev, port, &en, &mode)) != MAD_OK)
	{
		MSG_PRINT(("mdCopperGetAutoNeg return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}
	if (en)
		MSG_PRINT(("Copper Augo-Negotiation is enable and its mode : %i\n", (int)mode));
	else
		MSG_PRINT(("Copper Augo-Negotiation is disable\n"));

	return MAD_OK;
}

/*
* Restart Copper Auto Negotiation.
*/
MAD_STATUS RestartCpAutoNeg(MAD_DEV *dev, MAD_LPORT port)
{
	MAD_STATUS status;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Restart Copper AN Status for port %i\n", (int)port));

	/*
	*    Restart Copper Auto Negotiation.
	*/
	if ((status = mdCopperSetAutoRestart(dev, port)) != MAD_OK)
	{
		MSG_PRINT(("mdCopperSetAutoRestart return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Transltes Copper Auto Negotiation mode.
*/
MAD_STATUS GetCpAutoNegMode(MAD_BOOL autoNegoEn, MAD_SPEED_MODE speedMode, MAD_DUPLEX_MODE duplexMode)
{
	MAD_STATUS status;
	MAD_U32 automode;

	/*
	*    Transltes Copper Auto Negotiation mode.
	*/
	if ((status = mdGetAutoNegoMode(autoNegoEn, speedMode, duplexMode, &automode)) != MAD_OK)
	{
		MSG_PRINT(("mdGetAutoNegoMode return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("Auto Negotiation mode:     %i\n", (int)automode));

	return MAD_OK;
}

/*
* Set Copper MDI Mode.
*/
MAD_STATUS SetCpMDIMode(MAD_DEV *dev, MAD_LPORT port, MAD_MDI_MODE mode)
{
	MAD_STATUS status;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Set Copper MDI Mode for port %i\n", (int)port));

	/*
	*    Set Copper MDI Mode.
	*/
	if ((status = mdCopperSetMDIMode(dev, port, mode)) != MAD_OK)
	{
		MSG_PRINT(("mdCopperSetMDIMode return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Read Copper MDI Mode.
*/
MAD_STATUS ReadCpMDIMode(MAD_DEV *dev, MAD_LPORT port)
{
	MAD_STATUS status;
	MAD_MDI_MODE  mode;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Read Copper MDI Mode for port %i\n", (int)port));

	/*
	*    Read Copper MDI Mode.
	*/
	if ((status = mdCopperGetMDIMode(dev, port, &mode)) != MAD_OK)
	{
		MSG_PRINT(("mdCopperGetMDIMode return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("MDI Mode:    %i\n", mode));

	return MAD_OK;
}

/*
* Enable Copper Downshift and set Downshift counter.
*/
MAD_STATUS EnableDownshift(MAD_DEV *dev, MAD_LPORT port, MAD_U16 count)
{
	MAD_STATUS status;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Enable Copper Downshift for port %i\n", (int)port));

	/*
	*    Enable Copper Downshift.
	*/
	if ((status = mdCopperSetDownshiftEnable(dev, port, MAD_TRUE, count)) != MAD_OK)
	{
		MSG_PRINT(("mdCopperSetDownshiftEnable return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Disable Copper Downshift.
*/
MAD_STATUS DisableDownshift(MAD_DEV *dev, MAD_LPORT port)
{
	MAD_STATUS status;
	MAD_U16    count = 0;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Disable Copper Downshift for port %i\n", (int)port));

	/*
	*    Disable Copper Downshift.
	*/
	if ((status = mdCopperSetDownshiftEnable(dev, port, MAD_FALSE, count)) != MAD_OK)
	{
		MSG_PRINT(("mdCopperSetDownshiftEnable return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Read Copper Downshift status.
*/
MAD_STATUS ReadDownshiftStatus(MAD_DEV *dev, MAD_LPORT port)
{
	MAD_STATUS status;
	MAD_U16 count;
	MAD_BOOL en;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Get Copper Downshift status for port %i\n", (int)port));

	/*
	*    Get Copper Downshift status.
	*/
	if ((status = mdCopperGetDownshiftEnable(dev, port, &en, &count)) != MAD_OK)
	{
		MSG_PRINT(("mdCopperGetDownshiftEnable return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	if (en)
		MSG_PRINT(("Copper Downshift is enable and Downshift count : %i\n", (int)count));
	else
		MSG_PRINT(("Copper Downshift is disable\n"));

	return MAD_OK;
}

/*
* Enable DTE (such as IP Phone) detect.
*/
MAD_STATUS EnableDTEDetect(MAD_DEV *dev, MAD_LPORT port, MAD_U16 dropHys)
{
	MAD_STATUS status;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Enable DTE (such as IP Phone) detect for port %i\n", (int)port));

	/*
	*    Enable Copper Downshift.
	*/
	if ((status = mdCopperSetDTEDetectEnable(dev, port, MAD_TRUE, dropHys)) != MAD_OK)
	{
		MSG_PRINT(("mdCopperSetDTEDetectEnable return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Disable DTE (such as IP Phone) detect.
*/
MAD_STATUS DisableDTEDetect(MAD_DEV *dev, MAD_LPORT port)
{
	MAD_STATUS status;
	MAD_U16 dropHys = 0;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Disable DTE (such as IP Phone) detect for port %i\n", (int)port));

	/*
	*    Enable Copper Downshift.
	*/
	if ((status = mdCopperSetDTEDetectEnable(dev, port, MAD_FALSE, dropHys)) != MAD_OK)
	{
		MSG_PRINT(("mdCopperSetDTEDetectEnable return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Read DTE (such as IP Phone) detect status.
*/
MAD_STATUS ReadDTEDetectStatus(MAD_DEV *dev, MAD_LPORT port)
{
	MAD_STATUS status;
	MAD_U16 dropHys;
	MAD_BOOL en;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Get DTE (such as IP Phone) detect status for port %i\n", (int)port));

	/*
	*    Get Copper Downshift status.
	*/
	if ((status = mdCopperGetDTEDetectEnable(dev, port, &en, &dropHys)) != MAD_OK)
	{
		MSG_PRINT(("mdCopperGetDTEDetectEnable return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	if (en)
		MSG_PRINT(("Copper DTE detect is enable and drop hysteresis : %i\n", (int)dropHys));
	else
		MSG_PRINT(("Copper DTE detect is disable\n"));

	return MAD_OK;
}

/*
* Set Link down delay.
*/
MAD_STATUS SetLinkDownDelay(MAD_DEV *dev, MAD_LPORT port, MAD_U16 delay)
{
	MAD_STATUS status;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Set Link down delay for port %i\n", (int)port));

	/*
	*    Set Link down delay.
	*/
	if ((status = mdCopperSetLinkDownDelay(dev, port, delay)) != MAD_OK)
	{
		MSG_PRINT(("mdCopperSetLinkDownDelay return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Read Link down delay.
*/
MAD_STATUS ReadLinkDownDelay(MAD_DEV *dev, MAD_LPORT port)
{
	MAD_STATUS status;
	MAD_U16 delay;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Read Link down delay for port %i\n", (int)port));

	/*
	*    Read Link down delay.
	*/
	if ((status = mdCopperGetLinkDownDelay(dev, port, &delay)) != MAD_OK)
	{
		MSG_PRINT(("mdCopperGetLinkDownDelay return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("Link Down Delay:   %i\n", delay));

	return MAD_OK;
}

/*
* Enable Link Down Delay.
*/
MAD_STATUS EnableLinkDownDelay(MAD_DEV *dev, MAD_LPORT port)
{
	MAD_STATUS status;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Enable Link Down Delay for port %i\n", (int)port));

	/*
	*    Enable Link Down Delay.
	*/
	if ((status = mdCopperSetLinkDownDelayEnable(dev, port, MAD_TRUE)) != MAD_OK)
	{
		MSG_PRINT(("mdCopperSetLinkDownDelayEnable return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Disable Link Down Delay.
*/
MAD_STATUS DisableLinkDownDelay(MAD_DEV *dev, MAD_LPORT port)
{
	MAD_STATUS status;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Disable Link Down Delay for port %i\n", (int)port));

	/*
	*    Disable Link Down Delay.
	*/
	if ((status = mdCopperSetLinkDownDelayEnable(dev, port, MAD_FALSE)) != MAD_OK)
	{
		MSG_PRINT(("mdCopperSetLinkDownDelayEnable return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Read Gigabit Link Down Delay enable status.
*/
MAD_STATUS ReadLinkDownDelayEnable(MAD_DEV *dev, MAD_LPORT port)
{
	MAD_STATUS status;
	MAD_BOOL en;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Read Gigabit Link Down Delay enable status for port %i\n", (int)port));

	/*
	*    Read Gigabit Link Down Delay enable status.
	*/
	if ((status = mdCopperGetLinkDownDelayEnable(dev, port, &en)) != MAD_OK)
	{
		MSG_PRINT(("mdCopperGetLinkDownDelayEnable return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("Gigabit Link Down Delay enable status:   %i\n", en));

	return MAD_OK;
}

/*
* Set Phy speed and deplex.
*/
MAD_STATUS SetCpSpeedDuplex (MAD_DEV *dev, MAD_LPORT port, MAD_U32 speed, MAD_BOOL duplex)
{
	MAD_STATUS status;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Set Phy speed and deplex for port %i\n", (int)port));

	/*
	*    Set Phy speed and deplex.
	*/
	if ((status = mdCopperSetSpeedDuplex(dev, port, speed, duplex)) != MAD_OK)
	{
		MSG_PRINT(("mdCopperSetSpeedDuplex return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Sets The ports 1000Base-T Master mode.
*/
MAD_STATUS SetCp1000TMasterMode(MAD_DEV *dev, MAD_LPORT port, MAD_BOOL autoConfig, MAD_BOOL masterPrefer)
{
	MAD_STATUS status;
	MAD_1000T_MASTER_SLAVE mode;
	mode.autoConfig = autoConfig;
	mode.masterPrefer = masterPrefer;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("sets The ports 1000Base-T Master modefor port %i\n", (int)port));

	/*
	*    sets The ports 1000Base-T Master mode.
	*/
	if ((status = mdCopperSet1000TMasterMode(dev, port, &mode)) != MAD_OK)
	{
		MSG_PRINT(("mdCopperSet1000TMasterMode return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Reads The ports 1000Base-T Master mode.
*/
MAD_STATUS ReadCp1000TMasterMode(MAD_DEV *dev, MAD_LPORT port)
{
	MAD_STATUS status;
	MAD_1000T_MASTER_SLAVE mode;
	mode.autoConfig = 0;
	mode.masterPrefer = 0;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("gets The ports 1000Base-T Master modefor port %i\n", (int)port));

	/*
	*    sets The ports 1000Base-T Master mode.
	*/
	if ((status = mdCopperGet1000TMasterMode(dev, port, &mode)) != MAD_OK)
	{
		MSG_PRINT(("mdCopperGet1000TMasterMode return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("mode->autoConfig: %i , mode->masterPrefer: %i\n", mode.autoConfig, mode.masterPrefer));
	
	return MAD_OK;
}