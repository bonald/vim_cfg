#include <madCopyright.h>
/********************************************************************************
* madCpStatusSample.c
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

/*
* Read Copper Downshift status.
*/
MAD_STATUS ReadCpDownshiftStatus(MAD_DEV *dev, MAD_LPORT port)
{
	MAD_STATUS status;
	MAD_BOOL st;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Read Copper Downshift status for port %i\n", (int)port));

	/*
	*    Read Copper Downshift status.
	*/
	if ((status = mdCopperGetDownshiftStatus(dev, port, &st)) != MAD_OK)
	{
		MSG_PRINT(("mdCopperGetDownshiftStatus return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("Copper Downshift status : %i\n", st));

	return MAD_OK;
}

/*
* Read Copper Link status.
*/
MAD_STATUS ReadCpLinkStatus(MAD_DEV *dev, MAD_LPORT port)
{
	MAD_STATUS status;
	MAD_BOOL linkOn;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Read Copper Link status for port %i\n", (int)port));

	/*
	*    Read Copper Link status.
	*/
	if ((status = mdCopperGetLinkStatus(dev, port, &linkOn)) != MAD_OK)
	{
		MSG_PRINT(("mdCopperGetLinkStatus return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("Copper Link status : %i\n", linkOn));

	return MAD_OK;
}

/*
* Read Copper Speed status.
*/
MAD_STATUS ReadCpSpeedStatus(MAD_DEV *dev, MAD_LPORT port)
{
	MAD_STATUS status;
	MAD_SPEED_MODE mode = 0;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Read Copper Speed status for port %i\n", (int)port));

	/*
	*   Read Copper Speed status.
	*/
	if ((status = mdCopperGetSpeedStatus(dev, port, &mode)) != MAD_OK)
	{
		MSG_PRINT(("mdCopperGetSpeedStatus return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("Read Copper Speed status : %i\n", mode));

	return MAD_OK;
}

/*
* Read Copper Duplex status.
*/
MAD_STATUS ReadCpDuplexStatus(MAD_DEV *dev, MAD_LPORT port)
{
	MAD_STATUS status;
	MAD_DUPLEX_MODE mode;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Read Copper Duplex status for port %i\n", (int)port));

	/*
	*   Read Copper Duplex status.
	*/
	if ((status = mdCopperGetDuplexStatus(dev, port, &mode)) != MAD_OK)
	{
		MSG_PRINT(("mdCopperGetDuplexStatus return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("Read Copper Duplex status : %i\n", mode));

	return MAD_OK;
}

/*
* Read Copper DTE Detect status.
*/
MAD_STATUS ReadCpDTEDetectStatus(MAD_DEV *dev, MAD_LPORT port)
{
	MAD_STATUS status;
	MAD_BOOL   st;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Read Copper DTE Detect status for port %i\n", (int)port));

	/*
	*   Read Copper DTE Detect status.
	*/
	if ((status = mdCopperGetDTEDetectStatus(dev, port, &st)) != MAD_OK)
	{
		MSG_PRINT(("mdCopperGetDTEDetectStatus return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("Read Copper DTE Detect status : %i\n", st));

	return MAD_OK;
}