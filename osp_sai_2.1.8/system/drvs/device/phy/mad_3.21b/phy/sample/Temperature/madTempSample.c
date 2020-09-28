#include <madCopyright.h>
/********************************************************************************
* madTempSample.c
*
* DESCRIPTION:
*       Temperature measurement.
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

MAD_TEMP_TYPE tempType = 1;   /*Page6,Reg27,bit15:14 */
MAD_BOOL      tempSense = 0;  /*This bit is valid only when tempType == 2'b10 */
MAD_TEMP_SAMPLE_AVERAGE	avgSample = 1;   /*Page6,Reg27,bit12:11 */
MAD_TEMP_SAMPLE_INSTANT insSample = 5;    /*Page6,Reg27,bit10:8 */

/*
* Enable Temperature sensor.
*/
MAD_STATUS SetTempEnable
(
MAD_DEV         *dev,
MAD_LPORT       port
)
{
	MAD_STATUS status;
	MAD_TEMP_ENABLE   tempenable;
	tempenable.tempType = tempType;
	tempenable.tempSense = tempSense;
	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Enable Temperature sensor at port:%i\n", (int)port));

	/*
	*    Enable/Disable Temperature sensor
	*/
	if ((status = mdTempSetEnable(dev, port, &tempenable)) != MAD_OK)
	{
		MSG_PRINT(("mdTempSetEnable return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}
/*
* Disable Temperature sensor.
*/
MAD_STATUS SetTempDisable
(
MAD_DEV         *dev,
MAD_LPORT       port
)
{
	MAD_STATUS status;
	MAD_TEMP_ENABLE   tempenable;
	tempenable.tempType = 3;
	tempenable.tempSense = tempSense;
	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Disable Temperature sensor at port:%i\n", (int)port));

	/*
	*    Disable Temperature sensor
	*/
	if ((status = mdTempSetEnable(dev, port, &tempenable)) != MAD_OK)
	{
		MSG_PRINT(("mdTempSetEnable return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}
/*
* Set Temperature sensor sample rate.
*/
MAD_STATUS SetTempSampleRate
(
MAD_DEV         *dev,
MAD_LPORT       port
)
{
	MAD_STATUS status;
	MAD_TEMP_SAMPLE	tempsample;
	tempsample.avgSample = avgSample;
	tempsample.insSample = insSample;
	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Set Temperature sensor sample rate at port:%i\n", (int)port));

	/*
	*    Set Temperature sensor sample rate
	*/
	if ((status = mdTempSetSampleRate(dev, port, &tempsample)) != MAD_OK)
	{
		MSG_PRINT(("mdTempSetSampleRate return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Read Temperature sensor Alternative data.
*/
MAD_STATUS ReadTempAverageData
(
MAD_DEV         *dev,
MAD_LPORT       port
)
{
	MAD_STATUS status;
	MAD_U8     data = 0;
	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Read Temperature sensor Alternative data at port:%i\n", (int)port));

	/*
	*    Read Temperature sensor Alternative data
	*/
	if ((status = mdTempGetAverageData(dev, port, &data)) != MAD_OK)
	{
		MSG_PRINT(("mdTempGetAverageData return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}
	if (data == 0xb5)
		MSG_PRINT(("The data of average temperature is not ready.\n"));
	else
		MSG_PRINT(("The data of average temperature: %i\n", (int)data));

	return MAD_OK;
}

/*
* Read Temperature sensor Instantaneous data.
*/
MAD_STATUS ReadTempInstantData
(
MAD_DEV         *dev,
MAD_LPORT       port
)
{
	MAD_STATUS status;
	MAD_U8     data = 0;
	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Read Temperature sensor Instantaneous data at port:%i\n", (int)port));

	/*
	*    Temperature sensor Instantaneous data
	*/
	if ((status = mdTempGetInstantData(dev, port, &data)) != MAD_OK)
	{
		MSG_PRINT(("mdTempGetInstantData return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}
	MSG_PRINT(("The data of Instantaneous temperature: %i\n", (int)data));

	return MAD_OK;
}

/*
* Set Temperature sensor Threshold data.
*/
MAD_STATUS SetTempThresholdData
(
MAD_DEV         *dev,
MAD_LPORT       port,
MAD_U8			data
)
{
	MAD_STATUS status;
	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Set Temperature sensor Threshold data at port:%i\n", (int)port));

	/*
	*    Set Temperature sensor Threshold data
	*/
	if ((status = mdTempSetThresholdData(dev, port, data)) != MAD_OK)
	{
		MSG_PRINT(("mdTempSetThresholdData return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}
