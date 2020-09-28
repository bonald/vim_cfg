#include <madCopyright.h>
/********************************************************************************
* madPktGenSample.c
*
* DESCRIPTION:
*       Packet Generator setup sample (startPktGenerator and stopPktGenerator).
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
 * Start Copper Packet Generator.
 * Input:
 *      pktload - enum MAD_PG_PAYLOAD (MAD_PG_PAYLOAD_RANDOM or MAD_PG_PAYLOAD_5AA5)
 *      length  - enum MAD_PG_LENGTH  (MAD_PG_LENGTH_64 or MAD_PG_LENGTH_1518)
 *      tx      - enum MAD_PG_TX      (MAD_PG_TX_NORMAL or MAD_PG_TX_ERROR)
*/
MAD_STATUS startCopperPktGenerator
(
    MAD_DEV         *dev,
    MAD_LPORT       port,
    MAD_PG_PAYLOAD  payload,
    MAD_PG_LENGTH   length,
    MAD_PG_TX       tx
)
{
    MAD_STATUS status;
	MAD_COPPER_PG_1780     pktInfo;

    if (dev == 0)
    {
        MSG_PRINT(("MAD driver is not initialized\n"));
        return MAD_FAIL;
    }

    MSG_PRINT(("Start Packet Generator for port %i\n",(int)port));

	pktInfo.trigger = 0;
	pktInfo.check = 0;
	pktInfo.burst = 0;
    pktInfo.payload = payload; /* Pseudo-random, 5AA55AA5... */
    pktInfo.length = length;   /* 64 bytes, 1514 bytes */
    pktInfo.tx = tx;           /* normal packet, error packet */

    /*
     *    Start Packet Generator
    */
	if ((status = mdDiagSetCopperPktGenEnable(dev, port, MAD_TRUE, &pktInfo)) != MAD_OK)
    {
        MSG_PRINT(("mdDiagSetCopperPktGenEnable return Failed\n"));
        testMADDisplayStatus(status);
        return status;
    }

    return MAD_OK;
}

/*
 * Stop Copper Packet Generator.
 */
MAD_STATUS stopCopperPktGenerator(MAD_DEV *dev, MAD_LPORT port)
{
    MAD_STATUS status;

    if (dev == 0)
    {
        MSG_PRINT(("MAD driver is not initialized\n"));
        return MAD_FAIL;
    }

    MSG_PRINT(("Stopping Packet Generator for port %i\n",(int)port));

    /*
     *    Stop Packet Generator
    */
	if ((status = mdDiagSetCopperPktGenEnable(dev, port, MAD_FALSE, NULL)) != MAD_OK)
    {
        MSG_PRINT(("mdDiagSetCopperPktGenEnable return Failed\n"));
        testMADDisplayStatus(status);
        return status;
    }

    return MAD_OK;
}

/*
* Start Alternative Packet Generator.
* Input:
*      pktload - enum MAD_PG_PAYLOAD (MAD_PG_PAYLOAD_RANDOM or MAD_PG_PAYLOAD_5AA5)
*      length  - enum MAD_PG_LENGTH  (MAD_PG_LENGTH_64 or MAD_PG_LENGTH_1518)
*      tx      - enum MAD_PG_TX      (MAD_PG_TX_NORMAL or MAD_PG_TX_ERROR)
*      en_type - enum MAD_PG_EN_TYPE (MAD_PG_DISABLE or MAD_PG_EN_COPPER...)
*/
MAD_STATUS startAlterPktGenerator
(
MAD_DEV         *dev,
MAD_LPORT       port,
MAD_U16			crc_check,
MAD_PG_PAYLOAD  payload,
MAD_PG_LENGTH   length,
MAD_PG_TX       tx,
MAD_PG_EN_TYPE  en_type
)
{
	MAD_STATUS status;
	MAD_PG_1780     pktInfo;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Start Alternative Packet Generator for port %i\n", (int)port));

	pktInfo.trigger = 0;
	pktInfo.burst = 0;
	pktInfo.payload = payload; /* Pseudo-random, 5AA55AA5... */
	pktInfo.length = length;   /* 64 bytes, 1514 bytes */
	pktInfo.tx = tx;           /* normal packet, error packet */
	pktInfo.en_type = en_type; /* Enable_type */

	/*
	*    Start Alternative Packet Generator
	*/
	if ((status = mdDiagSetAltPktGenEnable(dev, port, crc_check, &pktInfo)) != MAD_OK)
	{
		MSG_PRINT(("mdDiagSetCopperPktGenEnable return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Stop Alternative Packet Generator.
*/
MAD_STATUS stopAlterPktGenerator(MAD_DEV *dev, MAD_LPORT port)
{
	MAD_STATUS status;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Stopping Alternative Packet Generator for port %i\n", (int)port));

	/*
	*    Stop Alternative Packet Generator
	*/
	if ((status = mdDiagSetAltPktGenEnable(dev, port, MAD_FALSE, NULL)) != MAD_OK)
	{
		MSG_PRINT(("mdDiagSetCopperPktGenEnable return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}