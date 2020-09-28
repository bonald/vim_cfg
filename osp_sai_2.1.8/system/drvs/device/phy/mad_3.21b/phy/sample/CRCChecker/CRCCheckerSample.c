#include <madCopyright.h>
/********************************************************************************
* CRCCheckerSample.c
*
* DESCRIPTION:
*       CRCChecker setup sample (enableCRCChecker, disableCRCChecker, 
*       readCRCCounter, and clearCRCCounter).
*       CRC Error Counter and Frame Counter is valid after CRC Checker is enabled, and
*       in order to clear those counters, CRC Checker needs to be disabled and then 
*       enabled again.
*       Both counters will peg to 0xFF.
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
 * Enable CRC Checker.
 */
MAD_STATUS enableCRCChecker(MAD_DEV *dev,MAD_LPORT port, MAD_U32 mode)
{
    MAD_STATUS status;

    if (dev == 0)
    {
        MSG_PRINT(("MAD driver is not initialized\n"));
        return MAD_API_ERR_DEV;
    }

    MSG_PRINT(("Enable CRC Checker for port %i\n",(int)port));

    /*
     *    Enable CRC Checker
    */
    if((status = mdStatsSetCRCCheckerEnable(dev,port,mode,MAD_TRUE)) != MAD_OK)
    {
        MSG_PRINT(("mdStatsSetCRCCheckerEnable return Failed\n"));
        testMADDisplayStatus(status);
        return status;
    }

    return MAD_OK;
}


/*
 * Disable CRC Checker.
 */
MAD_STATUS disableCRCChecker(MAD_DEV *dev,MAD_LPORT port)
{
    MAD_STATUS status;

    if (dev == 0)
    {
        MSG_PRINT(("MAD driver is not initialized\n"));
        return MAD_API_ERR_DEV;
    }

    MSG_PRINT(("Disable CRC Checker for port %i\n",(int)port));

    /*
     *    Disable CRC Checker
    */
    if((status = mdStatsSetCRCCheckerEnable(dev,port,0, MAD_FALSE)) != MAD_OK)
    {
        MSG_PRINT(("mdStatsSetCRCCheckerEnable return Failed\n"));
        testMADDisplayStatus(status);
        return status;
    }

    return MAD_OK;
}

/*
* Read CRC Checker enable.
*/
MAD_STATUS ReadCRCCheckerEnable(MAD_DEV *dev, MAD_LPORT port)
{
	MAD_STATUS status;
	MAD_BOOL en;
	MAD_U8 mode;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Read CRC Checker enable for port %i\n", (int)port));

	/*
	*    Read CRC Checker enable
	*/
	if ((status = mdStatsGetCRCCheckerEnable(dev, port, &mode, &en)) != MAD_OK)
	{
		MSG_PRINT(("mdStatsGetCRCCheckerEnable return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("CRC Checker Enable: %i\n", en));
	MSG_PRINT(("CRC Checker Mode: %i\n", mode));

	return MAD_OK;
}


/*
 * Clear CRC Counter (CRC Error Counter and Frame Counter).
 */
MAD_STATUS clearCRCCounter(MAD_DEV *dev, MAD_LPORT port, MAD_U32 mode)
{
    MAD_STATUS status;

    if (dev == 0)
    {
        MSG_PRINT(("MAD driver is not initialized\n"));
        return MAD_API_ERR_DEV;
    }

    MSG_PRINT(("Clear CRC Counter for port %i\n",(int)port));

    /*
     *    Disable CRC Checker
    */
    if((status = mdStatsSetCRCCheckerEnable(dev,port,0, MAD_FALSE)) != MAD_OK)
    {
        MSG_PRINT(("mdStatsSetCRCCheckerEnable return Failed\n"));
        testMADDisplayStatus(status);
        return status;
    }

    /*
     *    Enable CRC Checker
    */
	if ((status = mdStatsSetCRCCheckerEnable(dev, port, mode, MAD_TRUE)) != MAD_OK)
    {
        MSG_PRINT(("mdStatsSetCRCCheckerEnable return Failed\n"));
        testMADDisplayStatus(status);
        return status;
    }

    return MAD_OK;
}


/*
 * Read CRC Counter (CRC Error Counter and Frame Counter).
 */
MAD_STATUS readCRCCounter(MAD_DEV *dev,MAD_LPORT port)
{
    MAD_STATUS status;
    MAD_U32    counter;

    if (dev == 0)
    {
        MSG_PRINT(("MAD driver is not initialized\n"));
        return MAD_API_ERR_DEV;
    }

    MSG_PRINT(("Reading CRC Counter for port %i\n",(int)port));

    /*
     *    Read CRC Error Counter
    */
    if((status = mdStatsGetCRCErrorCounter(dev,port,&counter)) != MAD_OK)
    {
        MSG_PRINT(("mdStatsGetCRCErrorCounter return Failed\n"));
        testMADDisplayStatus(status);
        return status;
    }

    MSG_PRINT(("CRC Error Counter: %i\n",(int)counter));

    /*
     *    Read Frame Counter
    */
    if((status = mdStatsGetFrameCounter(dev,port,&counter)) != MAD_OK)
    {
        MSG_PRINT(("mdStatsGetFrameCounter return Failed\n"));
        testMADDisplayStatus(status);
        return status;
    }

    MSG_PRINT(("Frame Counter:     %i\n",(int)counter));

    return MAD_OK;
}

/*
* Read Link Disconnect counter.
*/
MAD_STATUS ReadLinkDisconnectCounter(MAD_DEV *dev, MAD_LPORT port)
{
	MAD_STATUS status;
	MAD_U16    count;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Read Link Disconnect counter for port %i\n", (int)port));

	/*
	*    Read Link Disconnect counter.
	*/
	if ((status = mdSysGetLinkDisconnectCounter(dev, port, &count)) != MAD_OK)
	{
		MSG_PRINT(("mdSysGetLinkDisconnectCounter return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("Link Disconnect counter: %i\n", (int)count));

	return MAD_OK;
}

/*
* Read Duplex Mismatch indicator: Late collision 97-128 bytes.
*/
MAD_STATUS ReadLateCollision(MAD_DEV *dev, MAD_LPORT port)
{
	MAD_STATUS status;
	MAD_LATE_COLLISION_COUNTER    count;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Read Duplex Mismatch indicator for port %i\n", (int)port));

	/*
	*    Read Duplex Mismatch indicator.
	*/
	if ((status = mdSysGetLateCollision(dev, port, &count)) != MAD_OK)
	{
		MSG_PRINT(("mdSysGetLateCollision return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("count_65: %i\n", (int)count.count_65));
	MSG_PRINT(("count_97: %i\n", (int)count.count_97));
	MSG_PRINT(("count_129: %i\n", (int)count.count_129));
	MSG_PRINT(("count_192: %i\n", (int)count.count_192));

	return MAD_OK;

}

/*
* Set Late collision Window adjust.
*/
MAD_STATUS SetLateCollision_winAdj(MAD_DEV *dev, MAD_LPORT port, MAD_U8 adj)
{
	MAD_STATUS status;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Set Late collision Window adjust for port %i\n", (int)port));

	/*
	*    Set Late collision Window adjust.
	*/
	if ((status = mdSysSetLateCollision_winAdj(dev, port, adj)) != MAD_OK)
	{
		MSG_PRINT(("mdSysSetLateCollision_winAdj return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;

}

/*
* Read Late collision Window adjust.
*/
MAD_STATUS ReadLateCollision_winAdj(MAD_DEV *dev, MAD_LPORT port)
{
	MAD_STATUS status;
	MAD_U8     adj;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_API_ERR_DEV;
	}

	MSG_PRINT(("Read Late collision Window adjust for port %i\n", (int)port));

	/*
	*    Read Late collision Window adjust.
	*/
	if ((status = mdSysGetLateCollision_winAdj(dev, port, &adj)) != MAD_OK)
	{
		MSG_PRINT(("mdSysGetLateCollision_winAdj return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("Late collision Window adjust: %i\n", (int)adj));

	return MAD_OK;

}

