#include <madCopyright.h>

/********************************************************************************
* madSNR.c
*
* DESCRIPTION:
*       APIs for 1G SNR Monitor Logic Feature.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include <madApi.h>
#include <madApiInternal.h>
#include <madHwCntl.h>
#include <madDrvPhyRegs.h>

typedef enum  {
	MAD_SNR_CHANNELA = 0,
	MAD_SNR_CHANNELB = 1,
	MAD_SNR_CHANNELC = 2,
	MAD_SNR_CHANNELD = 3,
}MAD_SNR_CHANNEL;

static void madWait(int waitTime)
{
	volatile int count = waitTime * 100000;
	while (count--);
}

static void madSleep(int time)
{
	madWait(time * 10);
}

static MAD_STATUS enableSNRChannel
(
IN  MAD_DEV         *dev,
IN  MAD_LPORT       port,
IN  MAD_SNR_CHANNEL channel
)
{
	MAD_U16     data = 0;
	MAD_U16     address;
	MAD_STATUS  retVal;
	MAD_U8      hwPort;

	MAD_DBG_INFO(("mdSNREnable called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}
	/* SNR feature only support 88E151x, 88E1540, 88E1680 and 88E1780*/
	if (dev->deviceId != MAD_88E151x && dev->deviceId != MAD_88E1540 && dev->deviceId != MAD_88E1680 && dev->deviceId != MAD_88E1780)
	{
		MAD_DBG_INFO(("The hardware does not support SNR.\n"));
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;
	}

	/* Enable or disable Descrambler */
	switch (channel)
	{
	case MAD_SNR_CHANNELA:
		data = 0x8220;
		break;
	case MAD_SNR_CHANNELB:
		data = 0x9220;
		break;
	case MAD_SNR_CHANNELC:
		data = 0xA220;
		break;
	case MAD_SNR_CHANNELD:
		data = 0xB220;
		break;
	default:
		MAD_DBG_ERROR(("Invalid HW mode is set %x.\n", (unsigned int)data));
		return MAD_API_UNKNOWN_HW_MODE;
	}

	address = 0x2101;
	if ((MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR)) &&
		(!MAD_DEV_IS_IN_SWITCH(dev)))
	{
		/*set Descrambler override bit*/
		if ((retVal = madHwWritePagedPhyReg(dev, hwPort, 0xFF, 17, data)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}

		if ((retVal = madHwWritePagedPhyReg(dev, hwPort, 0xFF, 16, address)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
	}
	else
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;


	return MAD_OK;
}


static MAD_STATUS enableReadPulse
(
IN  MAD_DEV         *dev,
IN  MAD_LPORT       port,
IN  MAD_BOOL        en
)
{
	MAD_U16     data;
	MAD_U16     address;
	MAD_STATUS  retVal;
	MAD_U8      hwPort;

	MAD_DBG_INFO(("mdSNREnableReadPulse called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}
	/* SNR feature only support 88E151x, 88E1540, 88E1680 and 88E1780*/
	if (dev->deviceId != MAD_88E151x && dev->deviceId != MAD_88E1540 && dev->deviceId != MAD_88E1680 && dev->deviceId != MAD_88E1780)
	{
		MAD_DBG_INFO(("The hardware does not support SNR.\n"));
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;
	}

	address = 0x1101;
	if ((retVal = madHwWritePagedPhyReg(dev, hwPort, 0xFF, 16, address)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}
	while (((address & 0x8000) >> 15) != 1)
	{
		if ((retVal = madHwReadPagedPhyReg(dev, hwPort, 0xFF, 16, &address)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
	}
	if ((retVal = madHwReadPagedPhyReg(dev, hwPort, 0xFF, 17, &data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}
	/* Enable or disable Descrambler */
	if (en)
		data |= (1 << 14);
	else
		data &= 0xbfff;

	address = 0x2101;
	if ((MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR)) &&
		(!MAD_DEV_IS_IN_SWITCH(dev)))
	{
		/*set Descrambler override bit*/
		if ((retVal = madHwWritePagedPhyReg(dev, hwPort, 0xFF, 17, data)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}

		if ((retVal = madHwWritePagedPhyReg(dev, hwPort, 0xFF, 16, address)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
	}
	else
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;


	return MAD_OK;
}

static MAD_STATUS getSNRData
(
IN  MAD_DEV         *dev,
IN  MAD_LPORT       port,
OUT MAD_U16        *data
)
{
	MAD_U16     Ldata;
	MAD_U16     Mdata;
	MAD_U16     address;
	MAD_STATUS  retVal;
	MAD_U8      hwPort;

	MAD_DBG_INFO(("mdSNRReadPulseEnable called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}
	/* SNR feature only support 88E151x, 88E1540, 88E1680 and 88E1780*/
	if (dev->deviceId != MAD_88E151x && dev->deviceId != MAD_88E1540 && dev->deviceId != MAD_88E1680 && dev->deviceId != MAD_88E1780)
	{
		MAD_DBG_INFO(("The hardware does not support SNR.\n"));
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;
	}

	address = 0x1007;
	if ((retVal = madHwWritePagedPhyReg(dev, hwPort, 0xFF, 16, address)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}
	while (((address & 0x8000) >> 15) != 1)
	{
		if ((retVal = madHwReadPagedPhyReg(dev, hwPort, 0xFF, 16, &address)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
	}
	if ((retVal = madHwReadPagedPhyReg(dev, hwPort, 0xFF, 18, &Mdata)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}

	address = 0x1008;
	if ((retVal = madHwWritePagedPhyReg(dev, hwPort, 0xFF, 16, address)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}
	while (((address & 0x8000) >> 15) != 1)
	{
		if ((retVal = madHwReadPagedPhyReg(dev, hwPort, 0xFF, 16, &address)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
	}
	if ((retVal = madHwReadPagedPhyReg(dev, hwPort, 0xFF, 18, &Ldata)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}

	*data = (((Mdata & 0x003f) << 6) | (Ldata & 0x003f));

	return MAD_OK;
}

static MAD_STATUS mdCopperGetMDIStatus
(
IN  MAD_DEV         *dev,
IN  MAD_LPORT       port,
OUT MAD_BOOL		*status
)
{
	MAD_U16     data;
	MAD_STATUS  retVal;
	MAD_U8      hwPort;

	MAD_DBG_INFO(("mdCopperGetMDIStatus called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}


	if (!MAD_DEV_HWMODE(dev, hwPort, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("Copper not supported.\n"));
		return MAD_API_COPPER_NOT_SUPPORT;
	}


	/* Get the required bits in Copper Control Register 1 */
	if ((retVal = madHwReadPagedPhyReg(
		dev, hwPort, 0, MAD_REG_COPPER_STATUS_1, &data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Reading to paged phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_GET_PAGED_REG_FIELD);
	}

	*status = (data & 0x40) >> 6;

	return MAD_OK;
}

static MAD_STATUS mdCopperGetSpeed
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT   port,
OUT MAD_SPEED_MODE *mode
)
{
	MAD_U16     data;
	MAD_STATUS  retVal;
	MAD_U8      hwPort;

	MAD_DBG_INFO(("mdCopperGetSpeed called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}


	if (!MAD_DEV_HWMODE(dev, hwPort, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("Copper not supported.\n"));
		return MAD_API_COPPER_NOT_SUPPORT;
	}

	/* Get Copper Status Register 1 */
	if ((retVal = madHwReadPagedPhyReg(dev, hwPort, 0, MAD_REG_COPPER_STATUS_1, &data))
		!= MAD_OK)
	{
		MAD_DBG_ERROR(("Reading from paged phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_READ_PAGED_REG);
	}

	/* check if the link status */
	if (!(data & MAD_BIT_10))
	{
		MAD_DBG_INFO(("The copper link is down.\n"));
		*mode = MAD_SPEED_UNKNOWN;
		return MAD_OK;

	}

	data = (data & 0xC000) >> 14;

	switch (data)
	{
	case 0:
		*mode = MAD_SPEED_10M;
		break;
	case 1:
		*mode = MAD_SPEED_100M;
		break;
	case 2:
		*mode = MAD_SPEED_1000M;
		break;
	default:
		MAD_DBG_ERROR(("Invalid speed is set %i.\n", data));
		return MAD_API_UNKNOWN_SPEED_MODE;
	}

	return MAD_OK;
}

/*******************************************************************************
* mdSNRGetData
*
* DESCRIPTION:
*       This routine gets SNR Result for 1000BASE-T.
*
* INPUTS:
*       dev  - pointer to MAD driver structure returned from mdLoadDriver
*       port - The logical port number
*
* OUTPUTS:
*		gain - Analog PGA Gain
*       data - The four channels of SNR Data
*			AData: The SNR data of Channel A
*			BData: The SNR data of Channel B
*			CData: The SNR data of Channel C
*			DData: The SNR data of Channel D
*
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*       Page 255, Reg 16 : Echo/Next Register Access
*       Page 255, Reg 17 : Echo/Next Write Data Register
*       Page 255, Reg 18 : Echo/Next Read Data Register
*
*******************************************************************************/
MAD_STATUS mdSNRGetData
(
IN  MAD_DEV         *dev,
IN  MAD_LPORT       port,
OUT MAD_U16         *gain,
OUT MAD_SNR_CHANNEL_DATA        *data
)
{
	MAD_STATUS  retVal;
	MAD_U8      hwPort;
	MAD_U16		tdata;
	MAD_U16     pdata;
	MAD_SPEED_MODE cSpeed;

	MAD_DBG_INFO(("mdSNRGetData called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}
	/* SNR feature only support 88E151x, 88E1540, 88E1680 and 88E1780*/
	if (dev->deviceId != MAD_88E151x && dev->deviceId != MAD_88E1540 && dev->deviceId != MAD_88E1680 && dev->deviceId != MAD_88E1780)
	{
		MAD_DBG_INFO(("The hardware does not support SNR.\n"));
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;
	}

	/* Get the current speed */
	if ((retVal = mdCopperGetSpeed(dev, MAD_PORT_2_LPORT(hwPort), &cSpeed)) != MAD_OK)
	{
		MAD_DBG_ERROR(("mdCopperGetSpeed failed.\n"));
		return (retVal | MAD_API_FAIL_GET_SPEED_ST);
	}
	if (cSpeed != MAD_SPEED_1000M)
	{
		MAD_DBG_INFO(("This API only support 1000BASE-T.\n"));
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;
	}

	if ((MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR)) &&
		(!MAD_DEV_IS_IN_SWITCH(dev)))
	{
		/*Get PAG Gain*/
		if ((retVal = madHwReadPagedPhyReg(dev, hwPort, 0xFC, 23, &pdata)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Reading to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
		if (dev->deviceId == MAD_88E151x || dev->deviceId == MAD_88E1540)
		{
			*gain = pdata & 0x03;
		}
		else if (dev->deviceId == MAD_88E1680 || dev->deviceId == MAD_88E1780)
		{
			*gain = pdata & 0x07;
		}

		/*Get the channel A data*/
		if ((retVal = enableSNRChannel(dev, hwPort, MAD_SNR_CHANNELA)) != MAD_OK)
		{
			MAD_DBG_INFO(("The hardware does not support SNR.\n"));
			return MAD_API_NEW_FEATURE_NOT_SUPPORT;
		}
		if ((retVal = enableReadPulse(dev, hwPort, MAD_TRUE)) != MAD_OK)
		{
			MAD_DBG_INFO(("The hardware does not support SNR.\n"));
			return MAD_API_NEW_FEATURE_NOT_SUPPORT;
		}
		madSleep(5000);
		if ((retVal = enableReadPulse(dev, hwPort, MAD_FALSE)) != MAD_OK)
		{
			MAD_DBG_INFO(("The hardware does not support SNR.\n"));
			return MAD_API_NEW_FEATURE_NOT_SUPPORT;
		}
		madSleep(100);
		if ((retVal = getSNRData(dev, hwPort, &tdata)) != MAD_OK)
		{
			MAD_DBG_INFO(("The hardware does not support SNR.\n"));
			return MAD_API_NEW_FEATURE_NOT_SUPPORT;
		}
		data->AData = tdata;

		/*Get the channel B data*/
		if ((retVal = enableSNRChannel(dev, hwPort, MAD_SNR_CHANNELB)) != MAD_OK)
		{
			MAD_DBG_INFO(("The hardware does not support SNR.\n"));
			return MAD_API_NEW_FEATURE_NOT_SUPPORT;
		}
		if ((retVal = enableReadPulse(dev, hwPort, MAD_TRUE)) != MAD_OK)
		{
			MAD_DBG_INFO(("The hardware does not support SNR.\n"));
			return MAD_API_NEW_FEATURE_NOT_SUPPORT;
		}
		madSleep(5000);
		if ((retVal = enableReadPulse(dev, hwPort, MAD_FALSE)) != MAD_OK)
		{
			MAD_DBG_INFO(("The hardware does not support SNR.\n"));
			return MAD_API_NEW_FEATURE_NOT_SUPPORT;
		}
		madSleep(100);
		if ((retVal = getSNRData(dev, hwPort, &tdata)) != MAD_OK)
		{
			MAD_DBG_INFO(("The hardware does not support SNR.\n"));
			return MAD_API_NEW_FEATURE_NOT_SUPPORT;
		}
		data->BData = tdata;

		/*Get the channel C data*/
		if ((retVal = enableSNRChannel(dev, hwPort, MAD_SNR_CHANNELC)) != MAD_OK)
		{
			MAD_DBG_INFO(("The hardware does not support SNR.\n"));
			return MAD_API_NEW_FEATURE_NOT_SUPPORT;
		}
		if ((retVal = enableReadPulse(dev, hwPort, MAD_TRUE)) != MAD_OK)
		{
			MAD_DBG_INFO(("The hardware does not support SNR.\n"));
			return MAD_API_NEW_FEATURE_NOT_SUPPORT;
		}
		madSleep(5000);
		if ((retVal = enableReadPulse(dev, hwPort, MAD_FALSE)) != MAD_OK)
		{
			MAD_DBG_INFO(("The hardware does not support SNR.\n"));
			return MAD_API_NEW_FEATURE_NOT_SUPPORT;
		}
		madSleep(100);
		if ((retVal = getSNRData(dev, hwPort, &tdata)) != MAD_OK)
		{
			MAD_DBG_INFO(("The hardware does not support SNR.\n"));
			return MAD_API_NEW_FEATURE_NOT_SUPPORT;
		}
		data->CData = tdata;

		/*Get the channel D data*/
		if ((retVal = enableSNRChannel(dev, hwPort, MAD_SNR_CHANNELD)) != MAD_OK)
		{
			MAD_DBG_INFO(("The hardware does not support SNR.\n"));
			return MAD_API_NEW_FEATURE_NOT_SUPPORT;
		}
		if ((retVal = enableReadPulse(dev, hwPort, MAD_TRUE)) != MAD_OK)
		{
			MAD_DBG_INFO(("The hardware does not support SNR.\n"));
			return MAD_API_NEW_FEATURE_NOT_SUPPORT;
		}
		madSleep(5000);
		if ((retVal = enableReadPulse(dev, hwPort, MAD_FALSE)) != MAD_OK)
		{
			MAD_DBG_INFO(("The hardware does not support SNR.\n"));
			return MAD_API_NEW_FEATURE_NOT_SUPPORT;
		}
		madSleep(100);
		if ((retVal = getSNRData(dev, hwPort, &tdata)) != MAD_OK)
		{
			MAD_DBG_INFO(("The hardware does not support SNR.\n"));
			return MAD_API_NEW_FEATURE_NOT_SUPPORT;
		}
		data->DData = tdata;

	}
	else
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;

	return MAD_OK;
}

/*******************************************************************************
* mdSNRGetData100T
*
* DESCRIPTION:
*       This routine gets SNR Result for 100BASE-T, according to MDI/MDIX status.
*
* INPUTS:
*       dev  - pointer to MAD driver structure returned from mdLoadDriver
*       port - The logical port number
*
* OUTPUTS:
*		gain - Analog PGA Gain
*       data - The single channels of SNR Data
*
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*       Page 255, Reg 16 : Echo/Next Register Access
*       Page 255, Reg 17 : Echo/Next Write Data Register
*       Page 255, Reg 18 : Echo/Next Read Data Register
*
*******************************************************************************/
MAD_STATUS mdSNRGetData100T
(
IN  MAD_DEV         *dev,
IN  MAD_LPORT       port,
OUT MAD_U16         *gain,
OUT MAD_U16         *data
)
{
	MAD_STATUS  retVal;
	MAD_U8      hwPort;
	MAD_U16     pdata;
	MAD_U16     tdata;
	MAD_BOOL    st;
	MAD_SPEED_MODE cSpeed;

	MAD_DBG_INFO(("mdSNRGetData100T called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}
	/* SNR feature only support 88E151x, 88E1540, 88E1680 and 88E1780*/
	if (dev->deviceId != MAD_88E151x && dev->deviceId != MAD_88E1540 && dev->deviceId != MAD_88E1680 && dev->deviceId != MAD_88E1780)
	{
		MAD_DBG_INFO(("The hardware does not support SNR.\n"));
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;
	}

	/* Get the current speed */
	if ((retVal = mdCopperGetSpeed(dev, MAD_PORT_2_LPORT(hwPort), &cSpeed)) != MAD_OK)
	{
		MAD_DBG_ERROR(("mdCopperGetSpeed failed.\n"));
		return (retVal | MAD_API_FAIL_GET_SPEED_ST);
	}
	if (cSpeed == MAD_SPEED_100M)
	{
		if ((retVal = mdCopperGetMDIStatus(dev, port, &st)) != MAD_OK)
		{
			MAD_DBG_ERROR(("mdCopperGetMDIMode return Failed.\n"));
			return retVal;
		}
	}
	else
	{
		MAD_DBG_INFO(("This API only support 100BASE-T.\n"));
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;
	}

	if ((MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR)) &&
		(!MAD_DEV_IS_IN_SWITCH(dev)))
	{
		/*Get PAG Gain*/
		if ((retVal = madHwReadPagedPhyReg(dev, hwPort, 0xFC, 23, &pdata)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Reading to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
		if (dev->deviceId == MAD_88E151x || dev->deviceId == MAD_88E1540)
		{
			*gain = pdata & 0x03;
		}
		else if (dev->deviceId == MAD_88E1680 || dev->deviceId == MAD_88E1780)
		{
			*gain = pdata & 0x07;
		}

		if (st == 0)
		{
			/*Get the channel B data*/
			if ((retVal = enableSNRChannel(dev, hwPort, MAD_SNR_CHANNELB)) != MAD_OK)
			{
				MAD_DBG_INFO(("The hardware does not support SNR.\n"));
				return MAD_API_NEW_FEATURE_NOT_SUPPORT;
			}
			if ((retVal = enableReadPulse(dev, hwPort, MAD_TRUE)) != MAD_OK)
			{
				MAD_DBG_INFO(("The hardware does not support SNR.\n"));
				return MAD_API_NEW_FEATURE_NOT_SUPPORT;
			}
			madSleep(5000);
			if ((retVal = enableReadPulse(dev, hwPort, MAD_FALSE)) != MAD_OK)
			{
				MAD_DBG_INFO(("The hardware does not support SNR.\n"));
				return MAD_API_NEW_FEATURE_NOT_SUPPORT;
			}
			madSleep(100);
			if ((retVal = getSNRData(dev, hwPort, &tdata)) != MAD_OK)
			{
				MAD_DBG_INFO(("The hardware does not support SNR.\n"));
				return MAD_API_NEW_FEATURE_NOT_SUPPORT;
			}
			*data = tdata;
		}
		else
		{
			/*Get the channel A data*/
			if ((retVal = enableSNRChannel(dev, hwPort, MAD_SNR_CHANNELA)) != MAD_OK)
			{
				MAD_DBG_INFO(("The hardware does not support SNR.\n"));
				return MAD_API_NEW_FEATURE_NOT_SUPPORT;
			}
			if ((retVal = enableReadPulse(dev, hwPort, MAD_TRUE)) != MAD_OK)
			{
				MAD_DBG_INFO(("The hardware does not support SNR.\n"));
				return MAD_API_NEW_FEATURE_NOT_SUPPORT;
			}
			madSleep(5000);
			if ((retVal = enableReadPulse(dev, hwPort, MAD_FALSE)) != MAD_OK)
			{
				MAD_DBG_INFO(("The hardware does not support SNR.\n"));
				return MAD_API_NEW_FEATURE_NOT_SUPPORT;
			}
			madSleep(100);
			if ((retVal = getSNRData(dev, hwPort, &tdata)) != MAD_OK)
			{
				MAD_DBG_INFO(("The hardware does not support SNR.\n"));
				return MAD_API_NEW_FEATURE_NOT_SUPPORT;
			}
			*data = tdata;
		}
	}
	else
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;

	return MAD_OK;
}