#include <madCopyright.h>
#include <string.h>
#include <stdlib.h>
/********************************************************************************
* madSysCtrlSample.c
*
* DESCRIPTION:
*       the system control configuration sample .
*
* DEPENDENCIES:
*       MAD Driver should be loaded and Phys should be enabled
*       before running sample test functions as specified in
*       sample\Initialization\madApiInit.c.
*
* FILE REVISION NUMBER:
*
*******************************************************************************/
#include "madSample.h"   

/*
* Read Link status.
*/
MAD_STATUS ReadLinkStatus(MAD_DEV *dev, MAD_LPORT port)
{
	MAD_STATUS status;
	MAD_BOOL linkOn;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Read Link status for port %i\n", (int)port));

	/*
	*    Read Link status.
	*/
	if ((status = mdGetLinkStatus(dev, port, &linkOn)) != MAD_OK)
	{
		MSG_PRINT(("mdGetLinkStatus return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("Link status: %i\n", (int)linkOn));

	return MAD_OK;
}

/*
* Read Speed status.
*/
MAD_STATUS ReadSpeedStatus(MAD_DEV *dev, MAD_LPORT port)
{
	MAD_STATUS status;
	MAD_SPEED_MODE speed = MAD_SPEED_10M;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Read Speed status for port %i\n", (int)port));

	/*
	*    Read Speed status.
	*/
	if ((status = mdGetSpeedStatus(dev, port, &speed)) != MAD_OK)
	{
		MSG_PRINT(("mdGetSpeedStatus return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("Speed status: %i\n", (int)speed));

	return MAD_OK;
}

/*
* Read Mac Speed status.
*/
MAD_STATUS ReadMacSpeedStatus(MAD_DEV *dev, MAD_LPORT port)
{
	MAD_STATUS status;
	MAD_SPEED_MODE speed;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Read Mac Speed status for port %i\n", (int)port));

	/*
	*    Read Mac Speed status.
	*/
	if ((status = mdGetMacSpeedStatus(dev, port, &speed)) != MAD_OK)
	{
		MSG_PRINT(("mdGetMacSpeedStatus return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("MAC Speed status: %i\n", (int)speed));

	return MAD_OK;
}

/*
* Read Duplex status.
*/
MAD_STATUS ReadDuplexStatus(MAD_DEV *dev, MAD_LPORT port)
{
	MAD_STATUS status;
	MAD_DUPLEX_MODE duplex;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Read Duplex status for port %i\n", (int)port));

	/*
	*    Read Duplex status.
	*/
	if ((status = mdGetDuplexStatus(dev, port, &duplex)) != MAD_OK)
	{
		MSG_PRINT(("mdGetDuplexStatus return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("Duplex status: %i\n", (int)duplex));

	return MAD_OK;
}

/*
* Enables a copper medium of the phy by powering up.
*/
MAD_STATUS SetPhyEnable(MAD_DEV *dev, MAD_LPORT port, MAD_BOOL en)
{
	MAD_STATUS status;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Set PHY enable for port %i\n", (int)port));

	/*
	*    Enables a copper medium of the phy by powering up.
	*/
	if ((status = mdSysSetPhyEnable(dev, port, en)) != MAD_OK)
	{
		MSG_PRINT(("mdSysSetPhyEnable return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Checks if copper of the phy is powered up.
*/
MAD_STATUS ReadPhyEnable(MAD_DEV *dev, MAD_LPORT port)
{
	MAD_STATUS status;
	MAD_BOOL en;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Check if copper of the phy is powered up for port %i\n", (int)port));

	/*
	*    Checks if copper of the phy is powered up by powering up.
	*/
	if ((status = mdSysGetPhyEnable(dev, port, &en)) != MAD_OK)
	{
		MSG_PRINT(("mdSysGetPhyEnable return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("The status of th PHY: %i\n", (int)en));

	return MAD_OK;
}

/*
* Sets soft reset.
*/
MAD_STATUS SetSoftReset(MAD_DEV *dev, MAD_LPORT port)
{
	MAD_STATUS status;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Sets different type soft reset for port %i\n", (int)port));

	/*
	*    Sets different type soft reset.
	*/
	if ((status = mdSysSoftReset(dev, port)) != MAD_OK)
	{
		MSG_PRINT(("mdSysSoftReset return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Sets different type soft reset.
*/
MAD_STATUS SetModSoftReset(MAD_DEV *dev, MAD_LPORT port, MAD_U32 mode)
{
	MAD_STATUS status;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Sets different type soft reset for port %i\n", (int)port));

	/*
	*    Sets different type soft reset.
	*/
	if ((status = mdSysModSoftReset(dev, port, mode)) != MAD_OK)
	{
		MSG_PRINT(("mdSysModSoftReset return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Enables Mac Interface of the phy by powering up.
*/
MAD_STATUS SetMacPowerDown(MAD_DEV *dev, MAD_LPORT port, MAD_BOOL en)
{
	MAD_STATUS status;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Enables Mac Interface of the phy by powering up for port %i\n", (int)port));

	/*
	*    Enables Mac Interface of the phy by powering up.
	*/
	if ((status = mdSysSetMacPowerDown(dev, port, en)) != MAD_OK)
	{
		MSG_PRINT(("mdSysSetMacPowerDown return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Check if Mac interface is powered up.
*/
MAD_STATUS ReadMacPowerDown(MAD_DEV *dev, MAD_LPORT port)
{
	MAD_STATUS status;
	MAD_BOOL en = 0;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Check if Mac interface is powered up for port %i\n", (int)port));

	/*
	*    Check if Mac interface is powered up.
	*/
	if ((status = mdSysGetMacPowerDown(dev, port, &en)) != MAD_OK)
	{
		MSG_PRINT(("mdSysGetMacPowerDown return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("The status of MAC Interface: %i\n", (int)en));

	return MAD_OK;
}

/*
* Reads the status of Energe Detect Power down Modes.
*/
MAD_STATUS ReadDetectPowerDownModeSt(MAD_DEV *dev, MAD_LPORT port)
{
	MAD_STATUS status;
	MAD_U16    st = 0;
	MAD_U16    change = 0;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Reads the status of Energe Detect Power down Modes for port %i\n", (int)port));

	/*
	*    Reads the status of Energe Detect Power down Modes.
	*/
	if ((status = mdSysGetDetectPowerDownModeSt(dev, port, &st, &change)) != MAD_OK)
	{
		MSG_PRINT(("mdSysGetDetectPowerDownModeSt return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("The status of Copper Energy Detect : %i\n", (int)st));
	MSG_PRINT(("The change of Copper Energy Detect : %i\n", (int)change));

	return MAD_OK;
}

/*
* Sets modes of Energe Detect Power down.
*/
MAD_STATUS SetDetectPowerDownMode(MAD_DEV *dev, MAD_LPORT port, MAD_U16 mode)
{
	MAD_STATUS status;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Sets modes of Energe Detect Power down for port %i\n", (int)port));

	/*
	*    Sets modes of Energe Detect Power down.
	*/
	if ((status = mdSysSetDetectPowerDownMode(dev, port, mode)) != MAD_OK)
	{
		MSG_PRINT(("mdSysSetDetectPowerDownMode return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Reads the modes of Energe Detect Power down.
*/
MAD_STATUS ReadDetectPowerDownMode(MAD_DEV *dev, MAD_LPORT port)
{
	MAD_STATUS status;
	MAD_U16    mode = 0;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Reads the modes of Energe Detect Power down for port %i\n", (int)port));

	/*
	*    Reads the modes of Energe Detect Power down.
	*/
	if ((status = mdSysGetDetectPowerDownMode(dev, port, &mode)) != MAD_OK)
	{
		MSG_PRINT(("mdSysGetDetectPowerDownMode return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("The mode of Energe Detect Power down : %i\n", (int)mode));

	return MAD_OK;
}

/*
* Sets LED Control Data.
*/
MAD_STATUS SetLEDCtrl(MAD_DEV *dev, MAD_LPORT port, MAD_LED_CTRL_DATA *ledData)
{
	MAD_STATUS status;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Sets LED Control Data for port %i\n", (int)port));

	/*
	*    Sets LED Control Data.
	*/
	if ((status = mdSysSetLEDCtrl(dev, port, ledData)) != MAD_OK)
	{
		MSG_PRINT(("mdSysSetLEDCtrl return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Reads LED Control Data.
*/
MAD_STATUS ReadLEDCtrl(MAD_DEV *dev, MAD_LPORT port)
{
	MAD_STATUS status;
	MAD_LED_CTRL_DATA ledData;
	memset(&ledData, 0, sizeof(MAD_LED_CTRL_DATA));
	ledData.ledCtrl[0] = 0xb;   /* set led to blink in 670ms */
	ledData.ledCtrl[0] = 0xb;
	ledData.ledBlinkRate = 0x4;
	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Sets LED Control Data for port %i\n", (int)port));

	/*
	*    Sets LED Control Data.
	*/
	if ((status = mdSysSetLEDCtrl(dev, port, &ledData)) != MAD_OK)
	{
		MSG_PRINT(("mdSysSetLEDCtrl return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("ledCtrl[0]:%i,ledCtrl[1]:%i,ledCtrl[2]:%i,ledCtrl[3]:%i\n", ledData.ledCtrl[0], ledData.ledCtrl[1], ledData.ledCtrl[2], ledData.ledCtrl[3]));
	MSG_PRINT(("ledPolarity[0]:%i,ledPolarity[1]:%i,ledPolarity[2]:%i,ledPolarity[3]:%i\n", ledData.ledPolarity[0], ledData.ledPolarity[1], ledData.ledPolarity[2], ledData.ledPolarity[3]));
	MSG_PRINT(("ledMixPercen0:%i,ledMixPercen0:%i\n", ledData.ledMixPercen0, ledData.ledMixPercen1));
	MSG_PRINT(("ledForceInt:%i\n", ledData.ledForceInt));
	MSG_PRINT(("ledPulseDuration:%i\n", ledData.ledPulseDuration));
	MSG_PRINT(("ledBlinkRate:%i\n", ledData.ledBlinkRate));
	MSG_PRINT(("ledSppedOffPulse_perio:%i\n", ledData.ledSppedOffPulse_perio));
	MSG_PRINT(("ledSppedOnPulse_perio:%i\n", ledData.ledSppedOnPulse_perio));
	MSG_PRINT(("ledCtrl[4]:%i,ledCtrl[5]:%i\n", ledData.ledCtrl[4], ledData.ledCtrl[5]));
	MSG_PRINT(("ledPolarity[4]:%i,ledPolarity[5]:%i\n", ledData.ledPolarity[4], ledData.ledPolarity[5]));
	MSG_PRINT(("ledFuncMap3:%i\n", ledData.ledFuncMap3));
	MSG_PRINT(("ledFuncMap2:%i\n", ledData.ledFuncMap2));

	return MAD_OK;
}

/*
* Sets 802.3ah unidirectioanal transmit.
*/
MAD_STATUS SetUniDirTrans(MAD_DEV *dev, MAD_LPORT port, MAD_BOOL en)
{
	MAD_STATUS status;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Sets 802.3ah unidirectioanal transmit for port %i\n", (int)port));

	/*
	*    Sets 802.3ah unidirectioanal transmit.
	*/
	if ((status = mdSysSetUniDirTrans(dev, port, en)) != MAD_OK)
	{
		MSG_PRINT(("mdSysSetUniDirTrans return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Reads 802.3ah unidirectioanal transmit.
*/
MAD_STATUS ReadUniDirTrans(MAD_DEV *dev, MAD_LPORT port)
{
	MAD_STATUS status;
	MAD_BOOL   en = 0;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Reads 802.3ah unidirectioanal transmit for port %i\n", (int)port));

	/*
	*    Reads 802.3ah unidirectioanal transmit.
	*/
	if ((status = mdSysGetUniDirTrans(dev, port, &en)) != MAD_OK)
	{
		MSG_PRINT(("mdSysGetUniDirTrans return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("The 802.3ah unidirectioanal transmit status: %i\n", (int)en));

	return MAD_OK;
}