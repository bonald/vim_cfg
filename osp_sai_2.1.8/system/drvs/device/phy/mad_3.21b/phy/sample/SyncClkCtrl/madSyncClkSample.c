#include <madCopyright.h>
/********************************************************************************
* madPktGenSample.c
*
* DESCRIPTION:
*       Control Syncronous Clocking sample .
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
* Sets the RCLK1 and RCLK2 pins of the chip outputs either a 125MHz or 25 MHz clock
*/
MAD_STATUS SetSyncRClk
(
MAD_DEV         *dev,
MAD_LPORT       port,
MAD_8           rclk,
MAD_8           freq
)
{
	MAD_STATUS status;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Sets the RCLK1 and RCLK2 for port %i\n", (int)port));

	/*
	*    Sets the RCLK1 and RCLK2.
	*/
	if ((status = mdSyncClkSetRClk(dev, port, rclk, freq)) != MAD_OK)
	{
		MSG_PRINT(("mdSyncClkSetRClk return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Read the RCLK1 and RCLK2 pins of the chip outputs either a 125MHz or 25 MHz clock
*/
MAD_STATUS ReadSyncRClk
(
MAD_DEV         *dev,
MAD_LPORT       port
)
{
	MAD_STATUS status;
	MAD_8      rclk;
	MAD_8      freq;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Reads the RCLK1 and RCLK2 for port %i\n", (int)port));

	/*
	*    Read the RCLK1 and RCLK2.
	*/
	if ((status = mdSyncClkGetRClk(dev, port, &rclk, &freq)) != MAD_OK)
	{
		MSG_PRINT(("mdSyncClkGetRClk return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("The RCLK1/RCLK2 selection: %i\n", (int)rclk));
	MSG_PRINT(("The frequency of Sync clock %i\n", (int)freq));

	return MAD_OK;
}

/*
* Sets to select between the local reference clock and a cleaned up recovered clock.
*/
MAD_STATUS SetSyncClkSelect
(
MAD_DEV         *dev,
MAD_LPORT       port,
MAD_8           interf,
MAD_8           refClk
)
{
	MAD_STATUS status;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("sets to select clock for port %i\n", (int)port));

	/*
	*    sets to select between the local reference clock and a cleaned up recovered clock.
	*/
	if ((status = mdSyncClkSetClkSelect(dev, port, interf, refClk)) != MAD_OK)
	{
		MSG_PRINT(("mdSyncClkSetClkSelect return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Read the clock select between the local reference clock and a cleaned up recovered clock.
*/
MAD_STATUS ReadSyncClkSelect
(
MAD_DEV         *dev,
MAD_LPORT       port,
MAD_8           interf
)
{
	MAD_STATUS status;
	MAD_8      refClk;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Read the selectional clock for port %i\n", (int)port));

	/*
	*    Read the clock select between the local reference clock and a cleaned up recovered clock.
	*/
	if ((status = mdSyncClkGetClkSelect(dev, port, interf, &refClk)) != MAD_OK)
	{
		MSG_PRINT(("mdSyncClkGetClkSelect return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("The Copper Reference Clock Source: %i\n", (int)refClk));

	return MAD_OK;
}