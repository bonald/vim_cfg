#include <madCopyright.h>

/********************************************************************************
* madEEEBufferCtrl.c
*
* DESCRIPTION:
*       APIs for EEE Buffer Control configuration.
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

/*******************************************************************************
* madSetEEEBufferConfig
*
* DESCRIPTION:
*       This routine set EEE Buffer configuration.
*
* INPUTS:
*       dev  - pointer to MAD driver structure returned from madLoadDriver
*       port - The logical port number
*       ipg  - Controls how many IPG when sending the packets that were stored in the fifo
*		en   - MAD_TRUE to enable EEE Buffer or MAD_FALSE to disable EEE Buffer
*
* OUTPUTS:
*		None.
*
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*       Rag: 0_18, Bit 15:8,0 : EEE Buffer Control Register 1
*
*******************************************************************************/
MAD_STATUS madSetEEEBufferConfig
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT   port,
IN  MAD_U8      ipg,
IN  MAD_BOOL    en
)
{
	MAD_STATUS  retVal;
	MAD_U8      hwPort;

	MAD_DBG_INFO(("mdEeeBufferConfig called.\n"));

	if (dev->deviceId != MAD_88E1780)
	{
		MAD_DBG_ERROR(("EEE Buffer Control not supported.\n"));
		return MAD_API_EEE_BUFFER_CTRL_NOT_SUPPORT;
	}

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}
	
	/* Enable/Disable EEE Buffer */
	if ((retVal = madHwSetPagedPhyRegField(dev, hwPort, 18, 0, 0, 1, en)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to paged phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}

	/* Set EEE Buffer IPG */
	if ((retVal = madHwSetPagedPhyRegField(dev, hwPort, 18, 0, 8, 8, ipg)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to paged phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}

	return MAD_OK;
}

/*******************************************************************************
* madGetEEEBufferConfig
*
* DESCRIPTION:
*       This routine get EEE Buffer configuration.
*
* INPUTS:
*       dev  - pointer to MAD driver structure returned from madLoadDriver
*       port - The logical port number
*
* OUTPUTS:
*       ipg  - Controls how many IPG when sending the packets that were stored in the fifo
*		en   - MAD_TRUE to enable EEE Buffer or MAD_FALSE to disable EEE Buffer
*
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*       Rag: 0_18, Bit 15:8,0 : EEE Buffer Control Register 1
*
*******************************************************************************/
MAD_STATUS madGetEEEBufferConfig
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT   port,
OUT MAD_U8      *ipg,
OUT MAD_BOOL    *en
)
{
	MAD_U16     data;
	MAD_STATUS  retVal;
	MAD_U8      hwPort;

	MAD_DBG_INFO(("mdGetEeeBufferConfig called.\n"));

	if (dev->deviceId != MAD_88E1780)
	{
		MAD_DBG_ERROR(("EEE Buffer Control not supported.\n"));
		return MAD_API_EEE_BUFFER_CTRL_NOT_SUPPORT;
	}

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	/* Enable/Disable EEE Buffer */
	if ((retVal = madHwGetPagedPhyRegField(dev, hwPort, 18, 0, 0, 1, &data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to paged phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}

	if (data)
		*en = MAD_TRUE;
	else
		*en = MAD_FALSE;

	/* Get EEE Buffer IPG */
	if ((retVal = madHwGetPagedPhyRegField(dev, hwPort, 18, 0, 8, 8, &data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to paged phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}

	*ipg = data & 0xff;

	return MAD_OK;
}