#include <madCopyright.h>
/********************************************************************************
* madMacCtrlSample.c
*
* DESCRIPTION:
*       Control MAC Interface sample (SetMacFifoDepth and ReadMacFifoDepth).
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
* Set MAC Transmit/Receive FIFO depth.
*/
MAD_STATUS SetMacFifoDepth
(
MAD_DEV         *dev,
MAD_LPORT       port,
MAD_U8          transFifoDepth,
MAD_U8          recvFifoDepth
)
{
	MAD_STATUS status;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Set MAC Transmit/Receive FIFO depth for port %i\n", (int)port));

	/*
	*    Set MAC Transmit/Receive FIFO depth.
	*/
	if ((status = mdMacSetFifoDepth(dev, port, transFifoDepth, recvFifoDepth)) != MAD_OK)
	{
		MSG_PRINT(("mdMacSetFifoDepth return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Read MAC Transmit/Receive FIFO depth.
*/
MAD_STATUS ReadMacFifoDepth
(
MAD_DEV         *dev,
MAD_LPORT       port
)
{
	MAD_STATUS status;
	MAD_U8          transFifoDepth;
	MAD_U8          recvFifoDepth;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Read MAC Transmit/Receive FIFO depth for port %i\n", (int)port));

	/*
	*    Read MAC Transmit/Receive FIFO depth.
	*/
	if ((status = mdMacGetFifoDepth(dev, port, &transFifoDepth, &recvFifoDepth)) != MAD_OK)
	{
		MSG_PRINT(("mdMacGetFifoDepth return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("Transmit FIFO Depth: %i\n", transFifoDepth));
	MSG_PRINT(("Receive FIFO Depth: %i\n", recvFifoDepth));

	return MAD_OK;
}
