#include <madCopyright.h>
/********************************************************************************
* madPatGenSample.c
*
* DESCRIPTION:
*       PRBS Generator setup sample (SetPatGenerator¡¢ReadPatGenerator 
*		and ResetPatGenerator).
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
* Start/Stop PRBS Generator.
*/
MAD_STATUS SetPatGenerator
(
MAD_DEV         *dev,
MAD_LPORT       port,
MAD_BOOL		genEn,
MAD_BOOL		countEn,
MAD_U8          polarCheckInv,
MAD_U8			polarGenInv,
MAD_U8			counterLock,
MAD_U8			patternSel
)
{
	MAD_STATUS status;
	MAD_PRBS_GEN_CONFIG     genCfg;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Start/Stop Packet Generator for port %i\n", (int)port));

	genCfg.genEn = genEn;
	genCfg.countEn = countEn;
	genCfg.polarCheckInv = polarCheckInv;
	genCfg.polarGenInv = polarGenInv;
	genCfg.counterLock = counterLock;
	genCfg.patternSel = patternSel;
	/*
	*    Start/Stop PRBS Generator
	*/
	if ((status = madPatCtrlSetPRBS(dev, port, &genCfg)) != MAD_OK)
	{
		MSG_PRINT(("madPatCtrlSetPRBS return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Read PRBS Generator Configuration.
*/
MAD_STATUS ReadPatGenerator
(
MAD_DEV         *dev,
MAD_LPORT       port
)
{
	MAD_STATUS status;
	MAD_PRBS_GEN_CONFIG     genCfg;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Read PRBS Generator Configuration for port %i\n", (int)port));

	/*
	*    Read PRBS Generator Configuration.
	*/
	if ((status = madPatCtrlGetPRBS(dev, port, &genCfg)) != MAD_OK)
	{
		MSG_PRINT(("madPatCtrlGetPRBS return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}
	MSG_PRINT(("PRBS Generator Enable: %i\n", genCfg.genEn));
	MSG_PRINT(("PRBS Checker Enable: %i\n", genCfg.countEn));
	MSG_PRINT(("Invert Checker Polarity: %i\n", genCfg.polarCheckInv));
	MSG_PRINT(("Invert Generator Polarity: %i\n", genCfg.polarGenInv));
	MSG_PRINT(("PRBS Lock Usage: %i\n", genCfg.counterLock));
	MSG_PRINT(("PRBS Pattern Select: %i\n", genCfg.patternSel));

	return MAD_OK;
}

/*
* Reset PRBS Generator.
*/
MAD_STATUS ResetPatGenerator
(
MAD_DEV         *dev,
MAD_LPORT       port
)
{
	MAD_STATUS status;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Reset PRBS Generator for port %i\n", (int)port));

	/*
	*    Reset PRBS Generator.
	*/
	if ((status = madResetPRBSCounters(dev, port)) != MAD_OK)
	{
		MSG_PRINT(("madResetPRBSCounters return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}