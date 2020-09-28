#include <madCopyright.h>
/********************************************************************************
* madSNRSample.c
*
* DESCRIPTION:
*       1G SNR Monitor Logic feature is based on S/N indicator result plus a constant
*		offset to indicate the system behavior.
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
#include<math.h>

/*
* Reads SNR Result for 1000BASE-T.
*/
MAD_STATUS ReadSNRData
(
MAD_DEV         *dev,
MAD_LPORT       port
)
{
	MAD_STATUS status;
	double fdataA, fdataB, fdataC, fdataD;
	MAD_SNR_CHANNEL_DATA  data;
	MAD_U16  gain;
	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Reads SNR Result at port:%i\n", (int)port));

	/*
	*    Reads SNR Result.
	*/
	if ((status = mdSNRGetData(dev, port, &gain,&data)) != MAD_OK)
	{
		MSG_PRINT(("mdSNRGetData return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}
	
	fdataA = -20 * log10((double)data.AData) + 55 - gain;
	fdataB = -20 * log10((double)data.BData) + 55 - gain;
	fdataC = -20 * log10((double)data.CData) + 55 - gain;
	fdataD = -20 * log10((double)data.DData) + 55 - gain;
	MSG_PRINT(("The PGA Gain: %d\n", gain));
	MSG_PRINT(("The Combined SNR Channel A: %x\n", data.AData));
	MSG_PRINT(("The SNR A channel Result: %f\n", fdataA));
	MSG_PRINT(("The Combined SNR Channel B: %x\n", data.BData));
	MSG_PRINT(("The SNR B channel Result: %f\n", fdataB));
	MSG_PRINT(("The Combined SNR Channel C: %x\n", data.CData));
	MSG_PRINT(("The SNR C channel Result: %f\n", fdataC));
	MSG_PRINT(("The Combined SNR Channel D: %x\n", data.DData));
	MSG_PRINT(("The SNR D channel Result: %f\n", fdataD));
	
	return MAD_OK;
}

/*
* Reads SNR Result for 100BASE-T.
*/
MAD_STATUS ReadSNRChannelData
(
MAD_DEV         *dev,
MAD_LPORT       port
)
{
	MAD_STATUS status;
	double fdata;
	MAD_U16  tdata[10];
	MAD_U16  data;
	MAD_U16  gain;
	MAD_U16 i = 0;
	MAD_U32 sum = 0;
	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Reads SNR Result at port:%i\n", (int)port));

	/*
	*    Reads SNR Result.
	*/
	for (i = 0; i < 10; i++)
	{
		if ((status = mdSNRGetData100T(dev, port, &gain, &tdata[i])) != MAD_OK)
		{
			MSG_PRINT(("mdSNRGetData100T return Failed\n"));
			testMADDisplayStatus(status);
			return status;
		}
		sum += tdata[i];
	}
	data = sum / 10;

	fdata = -20 * log10((double)data) + 58 - gain;
	MSG_PRINT(("The PGA Gain: %d\n", gain));
	MSG_PRINT(("The Combined SNR : %x\n", data));
	MSG_PRINT(("The SNR Result: %f\n", fdata));

	return MAD_OK;
}