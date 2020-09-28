#include <madCopyright.h>

/********************************************************************************
* madTemp.c
*
* DESCRIPTION:
*       APIs for Temperature measurement.
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

/*******************************************************************************
* mdTempSetEnable
*
* DESCRIPTION:
*       Enable/Disable Temperature sensor.
*
* INPUTS:
*       dev     - MAD driver structure returned from mdLoadDriver
*       port    - The logical port number
*
*       temptype         -  00 = Sample every 1Second (Sensor output read time is around 10.36ms)
*						    01 = Sense Rate decided by 6.27.10:8
*							10 = Use 6.26.5 to enable
*							11 = Disable
* OUTPUTS:
*       None.
*
* RETURNS:
*       MAD_OK - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*       Page 6, Reg 27, Bit 14-15    : Temperature Sensor Enable (88E1780)
*       Page 6, Reg 26, Bit 5		 : One shot Temperature Sample (88E1780)
*
*******************************************************************************/
MAD_STATUS mdTempSetEnable
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT    port,
IN  MAD_TEMP_ENABLE    *tempenable
)
{
	MAD_STATUS  retVal;
	MAD_U8      hwPort;

	MAD_DBG_INFO(("mdTempSetEnable called.\n"));

	if (dev->deviceId != MAD_88E1780)
	{
		MAD_DBG_ERROR(("Temperature Sensor Control not supported.\n"));
		return MAD_API_TEMP_SENSOR_CTRL_NOT_SUPPORT;
	}

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	if (!(MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR)))
	{
		MAD_DBG_ERROR(("The temperature sensor interrup is not supported.\n"));
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;
	}

	/* Set Interrupt Enable Register for Temperature sensor */
	if ((retVal = madHwSetPagedPhyRegField(
		dev, hwPort, MAD_PAGE6, MAD_REG_TEMPERATURE_SENSOR_1, 14, 2, tempenable->tempType)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing from paged phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_GET_PAGED_REG_FIELD);
	}

	/* When tempType == 2, use 6.26.5 to enable temperature sensor */
	if (tempenable->tempType == 2)
	{

		if ((retVal = madHwSetPagedPhyRegField(
			dev, hwPort, MAD_PAGE6, MAD_REG_MISC_TEST, 5, 1, tempenable->tempSense)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing from paged phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_GET_PAGED_REG_FIELD);
		}
	}

	return MAD_OK;
}

/*******************************************************************************
* mdTempSetSampleRate
*
* DESCRIPTION:
*       Set Temperature sensor sample rate.
*
* INPUTS:
*       dev     - MAD driver structure returned from mdLoadDriver
*       port    - The logical port number
*       tempsample    - Temperature sample rate.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*       MAD_OK - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*       Page 6, Reg 27, Bit 12:11    : Temperature Sensor Number of Samples to Average (88E1780)
*       Page 6, Reg 27, Bit 10:8     : Temperature Sensor Sampling Rate (88E1780)
*
*******************************************************************************/
MAD_STATUS mdTempSetSampleRate
(
IN  MAD_DEV    *dev,
IN  MAD_LPORT   port,
IN  MAD_TEMP_SAMPLE     *tempsample
)
{
	MAD_STATUS  retVal;
	MAD_U8      hwPort;

	MAD_DBG_INFO(("mdTempSetThresholdData called.\n"));

	if (dev->deviceId != MAD_88E1780)
	{
		MAD_DBG_ERROR(("Temperature Sensor Control not supported.\n"));
		return MAD_API_TEMP_SENSOR_CTRL_NOT_SUPPORT;
	}

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	if (MAD_DEV_IS_IN_SWITCH(dev))
	{
		MAD_DBG_ERROR(("The phy is in switch, and do not support the function.\n"));
		return MAD_API_PHY_IN_SWITCH;
	}

	if ((retVal = madHwSetPagedPhyRegField(
		dev, hwPort, MAD_PAGE6, MAD_REG_TEMPERATURE_SENSOR_1, 8, 3, tempsample->insSample)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Paged Phy Register Write failed.\n"));
		return (retVal | MAD_API_FAIL_WRITE_PAGED_REG);
	}

	if ((retVal = madHwSetPagedPhyRegField(
		dev, hwPort, MAD_PAGE6, MAD_REG_TEMPERATURE_SENSOR_1, 11, 2, tempsample->avgSample)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Paged Phy Register Write failed.\n"));
		return (retVal | MAD_API_FAIL_WRITE_PAGED_REG);
	}

	return MAD_OK;
}

/*******************************************************************************
* mdTempGetAverageData
*
* DESCRIPTION:
*       Get Temperature sensor Alternative data.
*
* INPUTS:
*       dev     - MAD driver structure returned from mdLoadDriver
*       port    - The logical port number
*
* OUTPUTS:
*        data    - The average data of temperature.
*
* RETURNS:
*       MAD_OK - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*       Page 6, Reg 27, Bit 7:0    : Temperature Sensor Alternative Reading (8-bit) (88E1780, 88E1680, 88E1680M)
*
*******************************************************************************/
MAD_STATUS mdTempGetAverageData
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT    port,
OUT MAD_U8    *data
)
{
	MAD_STATUS  retVal;
	MAD_U16     temp_data;
	MAD_U8      hwPort;

	MAD_DBG_INFO(("mdTempGetAverageData called.\n"));

	if (dev->deviceId != MAD_88E1780)
	{
		MAD_DBG_ERROR(("Temperature Sensor Control not supported.\n"));
		return MAD_API_TEMP_SENSOR_CTRL_NOT_SUPPORT;
	}

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	if (MAD_DEV_IS_IN_SWITCH(dev))
	{
		MAD_DBG_ERROR(("The phy is in switch, and do not support the function.\n"));
		return MAD_API_PHY_IN_SWITCH;
	}
	if ((retVal = madHwGetPagedPhyRegField(
		dev, hwPort, MAD_PAGE6, MAD_REG_TEMPERATURE_SENSOR_1, 0, 8, &temp_data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Paged Phy Register read failed.\n"));
		return (retVal | MAD_API_FAIL_READ_PAGED_REG);
	}
	if (dev->deviceId == MAD_88E1780)
	{
		*data = 1 * temp_data - 75;
	}
	else if (dev->deviceId == MAD_88E1680 || dev->deviceId == MAD_88E1680M)
	{
		*data = 1 * temp_data - 25;
	}

	return MAD_OK;
}

/*******************************************************************************
* mdTempGetInstantData
*
* DESCRIPTION:
*       Get Temperature sensor Instantaneous data.
*
* INPUTS:
*       dev     - MAD driver structure returned from mdLoadDriver
*       port    - The logical port number
*
* OUTPUTS:
*        data    - The Instantaneous data of temperature.
*
* RETURNS:
*       MAD_OK - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*       Page 6, Reg 28, Bit 7:0    : Temperature Sensor Instantaneous reading (8-bit) (88E1780)
*
*******************************************************************************/
MAD_STATUS mdTempGetInstantData
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT    port,
OUT MAD_U8    *data
)
{
	MAD_STATUS  retVal;
	MAD_U16     temp_data;
	MAD_U8      hwPort;

	MAD_DBG_INFO(("mdTempGetInstantData called.\n"));

	if (dev->deviceId != MAD_88E1780)
	{
		MAD_DBG_ERROR(("Temperature Sensor Control not supported.\n"));
		return MAD_API_TEMP_SENSOR_CTRL_NOT_SUPPORT;
	}

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);

	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	if (MAD_DEV_IS_IN_SWITCH(dev))
	{
		MAD_DBG_ERROR(("The phy is in switch, and do not support the function.\n"));
		return MAD_API_PHY_IN_SWITCH;
	}

	if ((retVal = madHwGetPagedPhyRegField(
		dev, hwPort, MAD_PAGE6, MAD_REG_TEMPERATURE_SENSOR_2, 0, 8, &temp_data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Paged Phy Register read failed.\n"));
		return (retVal | MAD_API_FAIL_READ_PAGED_REG);
	}

	*data = 1 * temp_data - 75;

	return MAD_OK;
}

/*******************************************************************************
* mdTempSetThresholdData
*
* DESCRIPTION:
*       Set Temperature sensor Threshold data.
*
* INPUTS:
*       dev     - MAD driver structure returned from mdLoadDriver
*       port    - The logical port number
*       data    - Temperature Interrupt Threshold data.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*       MAD_OK - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*       Page 6, Reg 28, Bit 15:8    : Temperature Interrupt Threshold (88E1780)
*
*******************************************************************************/
MAD_STATUS mdTempSetThresholdData
(
IN  MAD_DEV    *dev,
IN  MAD_LPORT    port,
IN  MAD_U8     data
)
{
	MAD_STATUS  retVal;
	MAD_U8      hwPort;

	MAD_DBG_INFO(("mdTempSetThresholdData called.\n"));

	if (dev->deviceId != MAD_88E1780)
	{
		MAD_DBG_ERROR(("Temperature Sensor Control not supported.\n"));
		return MAD_API_TEMP_SENSOR_CTRL_NOT_SUPPORT;
	}

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	if (MAD_DEV_IS_IN_SWITCH(dev))
	{
		MAD_DBG_ERROR(("The phy is in switch, and do not support the function.\n"));
		return MAD_API_PHY_IN_SWITCH;
	}

	data = data + 75;

	if ((retVal = madHwSetPagedPhyRegField(
		dev, hwPort, MAD_PAGE6, MAD_REG_TEMPERATURE_SENSOR_2, 8, 8, data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Paged Phy Register Write failed.\n"));
		return (retVal | MAD_API_FAIL_WRITE_PAGED_REG);
	}

	return MAD_OK;
}
