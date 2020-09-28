#include <madCopyright.h>

/********************************************************************************
* madUSGMII.c
*
* DESCRIPTION:
*       API definitions for the USGMII configuration
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
 #include <madApiProducts.h>

/*******************************************************************************
* mdGetPhyUSGMIIRegIdt
*
* DESCRIPTION:
*       This function reads a phy Hidden register of USGMII .
*       This API is provided for debugging or general use.
*
* INPUTS:
*       portNum - Port number to read the register for.
*       regAddr - The register's address.
*
* OUTPUTS:
*       data    - The read register's data.
*
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static MAD_STATUS mdGetPhyUSGMIIRegIdt
(
IN  MAD_DEV    *dev,
IN  MAD_U32    portNum,
IN  MAD_U16    regAddr,
OUT MAD_U16    *data
)
{
	MAD_STATUS  retVal;

	if (MAD_DEV_IS_IN_SWITCH(dev))
	{
		MAD_DBG_ERROR(("The phy is in switch, and do not support the function.\n"));
		return MAD_API_PHY_IN_SWITCH;
	}
	if ((retVal = madHwWritePagedPhyReg(dev, (MAD_U8)portNum, 240,
		7, regAddr)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Paged Phy Register Write failed.\n"));
		return (retVal | MAD_API_FAIL_READ_PAGED_REG);
	}

	if ((retVal = madHwReadPagedPhyReg(dev, (MAD_U8)portNum, 240,
		9, data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Paged Phy Register Write failed.\n"));
		return (retVal | MAD_API_FAIL_READ_PAGED_REG);
	}

	return MAD_OK;
}


/*******************************************************************************
* mdSetPhyUSGMIIRegIdt
*
* DESCRIPTION:
*       This function writes to a phy Hidden register of USGMII .
*       This API is provided for debugging or general use.
*
* INPUTS:
*       portNum - Port number to write the register for.
*       regAddr - The register's address.
*       data    - The data to be written.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static MAD_STATUS mdSetPhyUSGMIIRegIdt
(
IN  MAD_DEV    *dev,
IN  MAD_U32    portNum,
IN  MAD_U16    regAddr,
IN  MAD_U16    data
)
{
	MAD_STATUS  retVal;

	if (MAD_DEV_IS_IN_SWITCH(dev))
	{
		MAD_DBG_ERROR(("The phy is in switch, and do not support the function.\n"));
		return MAD_API_PHY_IN_SWITCH;
	}
	if ((retVal = madHwWritePagedPhyReg(dev, (MAD_U8)portNum, 240,
		8, data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Paged Phy Register Write failed.\n"));
		return (retVal | MAD_API_FAIL_READ_PAGED_REG);
	}
	regAddr = 0x2000 | regAddr;
	if ((retVal = madHwWritePagedPhyReg(dev, (MAD_U8)portNum, 240,
		7, regAddr)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Paged Phy Register Write failed.\n"));
		return (retVal | MAD_API_FAIL_READ_PAGED_REG);
	}

	return MAD_OK;
}
/*******************************************************************************
* mdUSGMIIGetLinkStatus
*
* DESCRIPTION:
*       This routine retrieves the Link status.
*
* INPUTS:
*       dev  - pointer to MAD driver structure returned from mdLoadDriver
*       port - The logical port number
*
* OUTPUTS:
*       linkOn - MAD_TRUE if link is established, MAD_FALSE otherwise
*
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*       Page 30, Reg 17, Bit 10 : USGMII Link
*
*******************************************************************************/
MAD_STATUS mdUSGMIIGetLinkStatus
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT   port,
OUT MAD_BOOL    *linkOn
)
{
	MAD_U16     data;
	MAD_STATUS  retVal;
	MAD_U8      hwPort;

	MAD_DBG_INFO(("mdUSGMIIGetLinkStatus called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}


	if (!MAD_DEV_HWMODE(dev, hwPort, MAD_PHY_MODE_USGMII))
	{
		MAD_DBG_ERROR(("USGMII not supported.\n"));
		return MAD_API_USGMII_NOT_SUPPORT;
	}

	/* Get the required bit in USGMII Status Register 1 */
	if ((retVal = madHwGetPagedPhyRegField(
		dev, hwPort, MAD_PAGE30, MAD_REG_USGMII_STATUS_1,
		10, 1, &data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Reading from paged phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_GET_PAGED_REG_FIELD);
	}

	/* Enabled or disabled */
	MAD_BIT_2_BOOL(data, *linkOn);

	return MAD_OK;
}

/*******************************************************************************
* mdUSGMIIGetLPLinkStatus
*
* DESCRIPTION:
*       This routine retrieves USGMII Link Partner's Link status.
*
* INPUTS:
*       dev  - pointer to MAD driver structure returned from mdLoadDriver
*       port - The logical port number
*
* OUTPUTS:
*       linkOn - MAD_TRUE if link is established, MAD_FALSE otherwise
*
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*       Page 30, Reg 5, Bit 15 : USGMII Link Partner Link
*
*******************************************************************************/
MAD_STATUS mdUSGMIIGetLPLinkStatus
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT   port,
OUT MAD_BOOL    *linkOn
)
{
	MAD_U16     data;
	MAD_STATUS  retVal;
	MAD_U8      hwPort;

	MAD_DBG_INFO(("mdUSGMIIGetLPLinkStatus called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}


	if (!MAD_DEV_HWMODE(dev, hwPort, MAD_PHY_MODE_USGMII))
	{
		MAD_DBG_ERROR(("USGMII not supported.\n"));
		return MAD_API_USGMII_NOT_SUPPORT;
	}

	/* Get the required bit in USGMII Status Register 1 */
	if ((retVal = madHwGetPagedPhyRegField(
		dev, hwPort, MAD_PAGE30, MAD_REG_USGMII_PARTNER_AD,
		15, 1, &data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Reading from paged phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_GET_PAGED_REG_FIELD);
	}

	/* Enabled or disabled */
	MAD_BIT_2_BOOL(data, *linkOn);

	return MAD_OK;
}

/*******************************************************************************
* mdUSGMIIGetSpeedStatus
*
* DESCRIPTION:
*       This routine retrieves the current Speed.
*
* INPUTS:
*       dev  - pointer to MAD driver structure returned from mdLoadDriver
*       port - The logical port number
*
* OUTPUTS:
*       mode - current speed (MAD_SPEED_MODE).
*               MAD_SPEED_10M, for 10 Mbps
*               MAD_SPEED_100M, for 100 Mbps
*               MAD_SPEED_1000M, for 1000 Mbps
*               MAD_SPEED_UNKNOWN, when speed is not resoved.
*
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*       Page 30, Reg 17, Bit 15:14,11 : Speed, Speed resolved
*
*******************************************************************************/
MAD_STATUS mdUSGMIIGetSpeedStatus
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT   port,
OUT MAD_SPEED_MODE *mode
)
{
	MAD_U16     data;
	MAD_STATUS  retVal;
	MAD_U8      hwPort;

	MAD_DBG_INFO(("mdUSGMIIGetSpeedStatus called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}


	if (!MAD_DEV_HWMODE(dev, hwPort, MAD_PHY_MODE_USGMII))
	{
		MAD_DBG_ERROR(("USGMII not supported.\n"));
		return MAD_API_USGMII_NOT_SUPPORT;
	}

	/* Get USGMII Status Register 1 */
	if ((retVal = madHwReadPagedPhyReg(dev, hwPort, 30, MAD_REG_USGMII_STATUS_1, &data))
		!= MAD_OK)
	{
		MAD_DBG_ERROR(("Reading from paged phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_READ_PAGED_REG);
	}

	/* check if Speed and Duplex is resolved */
	if (!(data & MAD_BIT_11))
	{
		MAD_DBG_INFO(("Speed and Duplex is not resolved.\n"));
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
* mdUSGMIIGetLPSpeedStatus
*
* DESCRIPTION:
*       This routine retrieves Link Partner's current Speed.Only after USGMII auto-negotiation,
*		user can read the Link Partner's speed from Page 30 Reg 5.
*
* INPUTS:
*       dev  - pointer to MAD driver structure returned from mdLoadDriver
*       port - The logical port number
*		TxCfgModeSel - Tx_Config mode selection(Page 30 Reg 27 Bit 15) in Link Partner.
*				0 = Tx_Config format as per USGMII specification Table 4.
*				1 = In USGMII mode Speed shifted to 11:10, instead of 11:9.
*
* OUTPUTS:
*       mode - current speed (MAD_SPEED_MODE).
*               MAD_SPEED_10M, for 10 Mbps
*               MAD_SPEED_100M, for 100 Mbps
*               MAD_SPEED_1000M, for 1000 Mbps
*               MAD_SPEED_UNKNOWN, when speed is not resoved.
*
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*       Page 30, Reg 5, Bit 10:9: USGMII Link Partner Speed
*
*******************************************************************************/
MAD_STATUS mdUSGMIIGetLPSpeedStatus
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT   port,
IN  MAD_BOOL    TxCfgModeSel,
OUT MAD_SPEED_MODE *mode
)
{
	MAD_U16     data;
	MAD_STATUS  retVal;
	MAD_U8      hwPort;

	MAD_DBG_INFO(("mdUSGMIIGetLPSpeedStatus called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}


	if (!MAD_DEV_HWMODE(dev, hwPort, MAD_PHY_MODE_USGMII))
	{
		MAD_DBG_ERROR(("USGMII not supported.\n"));
		return MAD_API_USGMII_NOT_SUPPORT;
	}

	/* Get USGMII Status Register 1 */
	if ((retVal = madHwReadPagedPhyReg(dev, hwPort, 30, MAD_REG_USGMII_PARTNER_AD, &data))
		!= MAD_OK)
	{
		MAD_DBG_ERROR(("Reading from paged phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_READ_PAGED_REG);
	}

	/* check if Speed and Duplex is resolved */
	if (!(data & MAD_BIT_15))
	{
		MAD_DBG_INFO(("Speed and Duplex is not resolved.\n"));
		*mode = MAD_SPEED_UNKNOWN;
		return MAD_OK;

	}
	if (TxCfgModeSel == 1)
	{
		data = (data & 0x0C00) >> 10; 
	}
	else
	{
		data = (data & 0x0E00) >> 9;
	}
	

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
* mdUSGMIIGetDuplexStatus
*
* DESCRIPTION:
*       This routine retrieves the current Duplex.
*
* INPUTS:
*       dev  - pointer to MAD driver structure returned from mdLoadDriver
*       port - The logical port number
*
* OUTPUTS:
*       mode - current duplex mode (MAD_DUPLEX_MODE).
*               MAD_HALF_DUPLEX, for half duplex mode
*               MAD_FULL_DUPLEX, for full duplex mode
*               MAD_DUPLEX_UNKNOWN, when duplex is not resolved.
*
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*       Page 30, Reg 17, Bit 13,11 : Duplex, Duplex resolved
*
*******************************************************************************/
MAD_STATUS mdUSGMIIGetDuplexStatus
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT   port,
OUT MAD_DUPLEX_MODE *mode
)
{
	MAD_U16     data;
	MAD_STATUS  retVal;
	MAD_U8      hwPort;

	MAD_DBG_INFO(("mdUSGMIIGetDuplexStatus called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}


	if (!MAD_DEV_HWMODE(dev, hwPort, MAD_PHY_MODE_USGMII))
	{
		MAD_DBG_ERROR(("USGMII not supported.\n"));
		return MAD_API_USGMII_NOT_SUPPORT;
	}

	/* Get USGMII Status Register 1 */
	if ((retVal = madHwReadPagedPhyReg(dev, hwPort, 30, MAD_REG_USGMII_STATUS_1, &data))
		!= MAD_OK)
	{
		MAD_DBG_ERROR(("Reading from paged phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_READ_PAGED_REG);
	}

	/* check if Speed and Duplex is resolved */
	if (!(data & MAD_BIT_11))
	{
		MAD_DBG_INFO(("Speed and Duplex is not resolved.\n"));
		*mode = MAD_DUPLEX_UNKNOWN;
		return MAD_OK;

	}

	data = (data >> 13) & 0x1;

	if (data == 0)
	{
		*mode = MAD_HALF_DUPLEX;
	}
	else
	{
		*mode = MAD_FULL_DUPLEX;
	}

	return MAD_OK;
}

/*******************************************************************************
* mdUSGMIIGetLPDuplexStatus
*
* DESCRIPTION:
*       This routine retrieves Link Partner's current Duplex.
*
* INPUTS:
*       dev  - pointer to MAD driver structure returned from mdLoadDriver
*       port - The logical port number
*
* OUTPUTS:
*       mode - current duplex mode (MAD_DUPLEX_MODE).
*               MAD_HALF_DUPLEX, for half duplex mode
*               MAD_FULL_DUPLEX, for full duplex mode
*               MAD_DUPLEX_UNKNOWN, when duplex is not resolved.
*
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*       Page 30, Reg 5, Bit 12 : USGMII Link Partner's Duplex
*
*******************************************************************************/
MAD_STATUS mdUSGMIIGetLPDuplexStatus
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT   port,
OUT MAD_DUPLEX_MODE *mode
)
{
	MAD_U16     data;
	MAD_STATUS  retVal;
	MAD_U8      hwPort;

	MAD_DBG_INFO(("mdUSGMIIGetLPDuplexStatus called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}


	if (!MAD_DEV_HWMODE(dev, hwPort, MAD_PHY_MODE_USGMII))
	{
		MAD_DBG_ERROR(("USGMII not supported.\n"));
		return MAD_API_USGMII_NOT_SUPPORT;
	}

	/* Get USGMII Status Register 1 */
	if ((retVal = madHwReadPagedPhyReg(dev, hwPort, 30, MAD_REG_USGMII_PARTNER_AD, &data))
		!= MAD_OK)
	{
		MAD_DBG_ERROR(("Reading from paged phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_READ_PAGED_REG);
	}

	/* check if Speed and Duplex is resolved */
	if (!(data & MAD_BIT_15))
	{
		MAD_DBG_INFO(("Speed and Duplex is not resolved.\n"));
		*mode = MAD_DUPLEX_UNKNOWN;
		return MAD_OK;

	}

	data = (data >> 12) & 0x1;

	if (data == 0)
	{
		*mode = MAD_HALF_DUPLEX;
	}
	else if (data == 1)
	{
		*mode = MAD_FULL_DUPLEX;
	}

	return MAD_OK;
}

/*******************************************************************************
* mdUSGMIISetPwDown
*
* DESCRIPTION:
*       This function set USGMII power down.
*
* INPUTS:
*       port - Port number to be configured.
*       en      - MAD_TRUE to power up or MAD_FALSE to power down
*
* OUTPUTS:
*       None
*
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MAD_STATUS mdUSGMIISetPwDown
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT   port,
IN  MAD_BOOL    en
)
{
	MAD_U16     data;
	MAD_STATUS  retVal;
	MAD_U8      hwPort;

	MAD_DBG_INFO(("mdUSGMIISetPwDown called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);

	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}


	if (!(MAD_DEV_HWREALMODE(dev, hwPort, MAD_PHY_MODE_USGMII)))
	{
		MAD_DBG_INFO(("The harware mode is not USGMII.\n"));
		return MAD_API_USGMII_NOT_SUPPORT;
	}

	/* Enable or disable PHY */
	if (en)
		data = 0;
	else
		data = 1;

	if ((MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR)) &&
		(!MAD_DEV_IS_IN_SWITCH(dev)))
	{
		/* Set or reset Power Down bit */
		if ((retVal = madHwSetPagedPhyRegField(
			dev, hwPort, MAD_PAGE30, MAD_REG_USGMII_CONTROL,
			11, 1, data)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
	}
	else
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;

	return MAD_OK;
}

/*******************************************************************************
* mdUSGMIIGetPwDown
*
* DESCRIPTION:
*       This function get USGMII power down status
*
* INPUTS:
*       port - Port number to be configured.
*
* OUTPUTS:
*       en      - MAD_TRUE to power down or MAD_FALSE to power up
*
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MAD_STATUS mdUSGMIIGetPwDown
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT   port,
OUT MAD_BOOL    *en
)
{
	MAD_U16     data;
	MAD_STATUS  retVal;
	MAD_U8      hwPort;

	MAD_DBG_INFO(("mdUSGMIIGetPwDown called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);

	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	if (!(MAD_DEV_HWREALMODE(dev, hwPort, MAD_PHY_MODE_USGMII)))
	{
		MAD_DBG_INFO(("The harware mode is not USGMII.\n"));
		return MAD_API_USGMII_NOT_SUPPORT;
	}

	if ((MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR)) &&
		(!MAD_DEV_IS_IN_SWITCH(dev)))
	{
		/* Set or reset Power Down bit */
		if ((retVal = madHwGetPagedPhyRegField(
			dev, hwPort, MAD_PAGE30, MAD_REG_USGMII_CONTROL,
			11, 1, &data)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Reading to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_GET_PAGED_REG_FIELD);
		}

		if (data)
			*en = MAD_FALSE;
		else
			*en = MAD_TRUE;
	}
	else
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;

	return MAD_OK;
}

/*******************************************************************************
* mdUSGMIISetANBypassEnable
*
* DESCRIPTION:
*       This function set Auto-Negotiation bypass.
*
* INPUTS:
*       port - Port number to be configured.
*       en      - MAD_TRUE to enable or MAD_FALSE to disable
*
* OUTPUTS:
*       None
*
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MAD_STATUS mdUSGMIISetANBypassEnable
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT   port,
IN  MAD_BOOL    en
)
{
	MAD_U16     data;
	MAD_STATUS  retVal;
	MAD_U8      hwPort;

	MAD_DBG_INFO(("mdUSGMIISetANBypass called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);

	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	if (!(MAD_DEV_HWREALMODE(dev, hwPort, MAD_PHY_MODE_USGMII)))
	{
		MAD_DBG_INFO(("The harware mode is not USGMII.\n"));
		return MAD_API_USGMII_NOT_SUPPORT;
	}

	/* Enable or disable PHY */
	if (en)
		data = 1;
	else
		data = 0;

	if ((MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR)) &&
		(!MAD_DEV_IS_IN_SWITCH(dev)))
	{
		/* Set or reset Power Down bit */
		if ((retVal = madHwSetPagedPhyRegField(
			dev, hwPort, MAD_PAGE30, MAD_REG_USGMII_CONTROL_1,
			9, 1, data)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
		mdSysModSoftReset(dev, hwPort, MAD_RESET_MODE_PORT_USGMII);
	}
	else
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;

	return MAD_OK;
}

/*******************************************************************************
* mdUSGMIIGetANBypassEnable
*
* DESCRIPTION:
*       This function get Auto-Negotiation bypass enable status.
*
* INPUTS:
*       port - Port number to be configured.
*
* OUTPUTS:
*       en      - MAD_TRUE to enable or MAD_FALSE to disable
*
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MAD_STATUS mdUSGMIIGetANBypassEnable
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT   port,
OUT MAD_BOOL    *st
)
{
	MAD_U16     data;
	MAD_STATUS  retVal;
	MAD_U8      hwPort;

	MAD_DBG_INFO(("mdUSGMIIGetANBypassEnable called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);

	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	if (!(MAD_DEV_HWREALMODE(dev, hwPort, MAD_PHY_MODE_USGMII)))
	{
		MAD_DBG_INFO(("The harware mode is not USGMII.\n"));
		return MAD_API_USGMII_NOT_SUPPORT;
	}

	if ((MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR)) &&
		(!MAD_DEV_IS_IN_SWITCH(dev)))
	{
		/* Set or reset Power Down bit */
		if ((retVal = madHwGetPagedPhyRegField(
			dev, hwPort, MAD_PAGE30, MAD_REG_USGMII_CONTROL_1,
			9, 1, &data)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Reading to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_GET_PAGED_REG_FIELD);
		}

		if (data)
			*st = MAD_TRUE;
		else
			*st = MAD_FALSE;
	}
	else
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;

	return MAD_OK;
}

/*******************************************************************************
* mdUSGMIIGetANBypassStatus
*
* DESCRIPTION:
*       This function get Auto-Negotiation bypass status.
*
* INPUTS:
*       port - Port number to be configured.
*
* OUTPUTS:
*       st      - MAD_TRUE to AN bypass or MAD_FALSE to regular AN
*
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MAD_STATUS mdUSGMIIGetANBypassStatus
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT   port,
OUT MAD_BOOL    *st
)
{
	MAD_U16     data;
	MAD_STATUS  retVal;
	MAD_U8      hwPort;

	MAD_DBG_INFO(("mdUSGMIIGetANBypassStatus called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);

	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	if (!(MAD_DEV_HWREALMODE(dev, hwPort, MAD_PHY_MODE_USGMII)))
	{
		MAD_DBG_INFO(("The harware mode is not USGMII.\n"));
		return MAD_API_USGMII_NOT_SUPPORT;
	}

	if ((MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR)) &&
		(!MAD_DEV_IS_IN_SWITCH(dev)))
	{
		/* Set or reset Power Down bit */
		if ((retVal = madHwGetPagedPhyRegField(
			dev, hwPort, MAD_PAGE30, MAD_REG_USGMII_STATUS_1,
			9, 1, &data)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Reading to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_GET_PAGED_REG_FIELD);
		}

		if (data)
			*st = MAD_TRUE;
		else
			*st = MAD_FALSE;
	}
	else
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;

	return MAD_OK;
}

/*******************************************************************************
* mdUSGMIIGlobalReset
*
* DESCRIPTION:
*       This function set USGMII Global Reset at all ports.
*
* INPUTS:
*       port - Port number to be configured.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MAD_STATUS mdUSGMIIGlobalReset
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT   port
)
{
	MAD_U16     data = 0;
	MAD_STATUS  retVal;
	MAD_U8      hwPort;

	MAD_DBG_INFO(("mdUSGMIIGlobalReset called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);

	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}


	if (!(MAD_DEV_HWREALMODE(dev, hwPort, MAD_PHY_MODE_USGMII)))
	{
		MAD_DBG_INFO(("The harware mode is not USGMII.\n"));
		return MAD_API_USGMII_NOT_SUPPORT;
	}


	if ((MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR)) &&
		(!MAD_DEV_IS_IN_SWITCH(dev)))
	{
		/* Read USGMII Reg26 */
		if ((retVal = madHwReadPagedPhyReg(dev, hwPort, MAD_PAGE30, MAD_REG_USGMII_GLOBAL_CTRL_1, &data))
			!= MAD_OK)
		{
			MAD_DBG_ERROR(("Reading phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
		data = data | 0x8000;
		/* Set Global Reset bit */
		if ((retVal = madHwWritePagedPhyReg(
			dev, hwPort, MAD_PAGE30, MAD_REG_USGMII_GLOBAL_CTRL_1,
		    data)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
	}
	else
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;

	return MAD_OK;
}

/*******************************************************************************
* mdUSGMIIEnableScrambler
*
* DESCRIPTION:
*       This function enable Scrambler.
*
* INPUTS:
*       port - Port number to be configured.
*       en      - MAD_TRUE to enable scrambler or MAD_FALSE to disable scrambler
*
* OUTPUTS:
*       None
*
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*       Page30, Reg28, Bit3,1: Override bit2 & bit0
*		Page30, Reg28, Bit2,0: Enable scrambler and Descrambler
*
*******************************************************************************/
MAD_STATUS mdUSGMIIEnableScrambler
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT   port,
IN  MAD_BOOL    en
)
{
	MAD_U16     data;
	MAD_STATUS  retVal;
	MAD_U8      hwPort;

	MAD_DBG_INFO(("mdUSGMIIEnableScrambler called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);

	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}


	if (!(MAD_DEV_HWREALMODE(dev, hwPort, MAD_PHY_MODE_USGMII)))
	{
		MAD_DBG_INFO(("The harware mode is not USGMII.\n"));
		return MAD_API_USGMII_NOT_SUPPORT;
	}

	/* Enable or disable Scrambler */
	if (en)
		data = 1;
	else
		data = 0;

	if ((MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR)) &&
		(!MAD_DEV_IS_IN_SWITCH(dev)))
	{
		/*set scrambler override bit*/
		if ((retVal = madHwSetPagedPhyRegField(
			dev, hwPort, MAD_PAGE30, MAD_REG_USGMII_DATAPATH_CTRL,
			1, 1, 1)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}

		if ((retVal = madHwSetPagedPhyRegField(
			dev, hwPort, MAD_PAGE30, MAD_REG_USGMII_DATAPATH_CTRL,
			0, 1, data)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
	}
	else
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;

	return MAD_OK;
}

/*******************************************************************************
* mdUSGMIIEnableDescrambler
*
* DESCRIPTION:
*       This function enable Descrambler.
*
* INPUTS:
*       port - Port number to be configured.
*       en      - MAD_TRUE to enable Descrambler or MAD_FALSE to disable Descrambler
*
* OUTPUTS:
*       None
*
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*       Page30, Reg28, Bit3,1: Override bit2 & bit0
*		Page30, Reg28, Bit2,0: Enable scrambler and Descrambler
*
*******************************************************************************/
MAD_STATUS mdUSGMIIEnableDescrambler
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT   port,
IN  MAD_BOOL    en
)
{
	MAD_U16     data;
	MAD_STATUS  retVal;
	MAD_U8      hwPort;

	MAD_DBG_INFO(("mdUSGMIIEnableDescrambler called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);

	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}


	if (!(MAD_DEV_HWREALMODE(dev, hwPort, MAD_PHY_MODE_USGMII)))
	{
		MAD_DBG_INFO(("The harware mode is not USGMII.\n"));
		return MAD_API_USGMII_NOT_SUPPORT;
	}

	/* Enable or disable Descrambler */
	if (en)
		data = 1;
	else
		data = 0;

	if ((MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR)) &&
		(!MAD_DEV_IS_IN_SWITCH(dev)))
	{
		/*set Descrambler override bit*/
		if ((retVal = madHwSetPagedPhyRegField(
			dev, hwPort, MAD_PAGE30, MAD_REG_USGMII_DATAPATH_CTRL,
			3, 1, 1)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}

		if ((retVal = madHwSetPagedPhyRegField(
			dev, hwPort, MAD_PAGE30, MAD_REG_USGMII_DATAPATH_CTRL,
			2, 1, data)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
	}
	else
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;

	return MAD_OK;
}

/*******************************************************************************
* mdUSGMIISetPCHPacketType
*
* DESCRIPTION:
*       This function Set PCH Packet Type.
*
* INPUTS:
*       port - Port number to be configured.
*       pky_type      - 00: Ethernet Packet with PCH
*						01: Ethernet Packet without PCH
*						10: Idle Packet, contains status data for a port, no packet data
*						11: Reserved
*
* OUTPUTS:
*       None
*
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*       Page 26, Reg 0, Bit 2:1 : Packet Type value for Ingress PCH headers
*
*******************************************************************************/
MAD_STATUS mdUSGMIISetPCHPacketType
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT   port,
IN  MAD_USGMII_PKT_TYPE     pky_type
)
{
	MAD_STATUS  retVal;
	MAD_U8      hwPort;

	MAD_DBG_INFO(("mdUSGMIISetPCHPacketType called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);

	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}


	if (!(MAD_DEV_HWREALMODE(dev, hwPort, MAD_PHY_MODE_USGMII)))
	{
		MAD_DBG_INFO(("The harware mode is not USGMII.\n"));
		return MAD_API_USGMII_NOT_SUPPORT;
	}


	if ((MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR)) &&
		(!MAD_DEV_IS_IN_SWITCH(dev)))
	{
		if ((retVal = madHwSetPagedPhyRegField(
			dev, hwPort, MAD_PAGE26, MAD_REG_PCH_COMMON_CTRL,
			1, 2, pky_type)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
	}
	else
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;

	return MAD_OK;
}

/*******************************************************************************
* mdUSGMIIGetPCHPacketType
*
* DESCRIPTION:
*       This function get PCH Packet Type.
*
* INPUTS:
*       port - Port number to be configured.
*
* OUTPUTS:
*       pky_type      - 00: Ethernet Packet with PCH
*						01: Ethernet Packet without PCH
*						10: Idle Packet, contains status data for a port, no packet data
*						11: Reserved
*
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*       Page 26, Reg 0, Bit 2:1 : Packet Type value for Ingress PCH headers
*
*******************************************************************************/
MAD_STATUS mdUSGMIIGetPCHPacketType
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT    port,
OUT MAD_USGMII_PKT_TYPE     *pky_type 
)
{
	MAD_STATUS  retVal;
	MAD_U8      hwPort;
	MAD_U16		data;

	MAD_DBG_INFO(("mdUSGMIIGetPCHPacketType called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);

	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}


	if (!(MAD_DEV_HWREALMODE(dev, hwPort, MAD_PHY_MODE_USGMII)))
	{
		MAD_DBG_INFO(("The harware mode is not USGMII.\n"));
		return MAD_API_USGMII_NOT_SUPPORT;
	}


	if ((MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR)) &&
		(!MAD_DEV_IS_IN_SWITCH(dev)))
	{
		if ((retVal = madHwGetPagedPhyRegField(
			dev, hwPort, MAD_PAGE26, MAD_REG_PCH_COMMON_CTRL,
			1, 2, &data)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Reading to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_GET_PAGED_REG_FIELD);
		}
		*pky_type = data;
	}
	else
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;

	return MAD_OK;
}

/*******************************************************************************
* mdUSGMIIEnablePortID
*
* DESCRIPTION:
*       This function enable customized port ID.
*
* INPUTS:
*       port - Port number to be configured.
*       en      - MAD_TRUE to enable customized port ID or MAD_FALSE to disable customized port ID
*
* OUTPUTS:
*       None
*
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*       Page 26, Reg 0, Bit 9 : Use customized port ID set in cust_port_id
*
*******************************************************************************/
MAD_STATUS mdUSGMIIEnablePortID
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT   port,
IN  MAD_BOOL    en
)
{
	MAD_U16     data;
	MAD_STATUS  retVal;
	MAD_U8      hwPort;

	MAD_DBG_INFO(("mdUSGMIIEnablePortID called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);

	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}


	if (!(MAD_DEV_HWREALMODE(dev, hwPort, MAD_PHY_MODE_USGMII)))
	{
		MAD_DBG_INFO(("The harware mode is not USGMII.\n"));
		return MAD_API_USGMII_NOT_SUPPORT;
	}

	/* Enable or disable Customized Port ID */
	if (en)
		data = 1;
	else
		data = 0;

	if ((MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR)) &&
		(!MAD_DEV_IS_IN_SWITCH(dev)))
	{
		if ((retVal = madHwSetPagedPhyRegField(
			dev, hwPort, MAD_PAGE26, MAD_REG_PCH_COMMON_CTRL,
			9, 1, data)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
	}
	else
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;

	return MAD_OK;
}

/*******************************************************************************
* mdUSGMIISetCustPortID
*
* DESCRIPTION:
*       This function Set customized port ID in cust_port_id.
*
* INPUTS:
*       port - Port number to be configured.
*       cust_portid   - Customized port ID value for Ingress PCH headers
*
* OUTPUTS:
*       None
*
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*		Page 26, Reg 0, Bit 9 : Use customized port ID set in cust_port_id
*       Page 26, Reg 0, Bit 8:5 : Customized port ID value for Ingress PCH headers
*
*******************************************************************************/
MAD_STATUS mdUSGMIISetCustPortID
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT   port,
IN  MAD_U16     cust_portid
)
{
	MAD_U16     data;
	MAD_STATUS  retVal;
	MAD_U8      hwPort;

	MAD_DBG_INFO(("mdUSGMIISetCustPortID called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);

	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}


	if (!(MAD_DEV_HWREALMODE(dev, hwPort, MAD_PHY_MODE_USGMII)))
	{
		MAD_DBG_INFO(("The harware mode is not USGMII.\n"));
		return MAD_API_USGMII_NOT_SUPPORT;
	}

	if ((MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR)) &&
		(!MAD_DEV_IS_IN_SWITCH(dev)))
	{
		if ((retVal = madHwGetPagedPhyRegField(
			dev, hwPort, MAD_PAGE26, MAD_REG_PCH_COMMON_CTRL,
			9, 1, &data)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}

		if (!data)
		{
			MAD_DBG_INFO(("Set Customized port ID failed,Use default PHY address as port ID.\n"));
			return MAD_FAIL;
		}

		if ((retVal = madHwSetPagedPhyRegField(
			dev, hwPort, MAD_PAGE26, MAD_REG_PCH_COMMON_CTRL,
			5, 4, cust_portid)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
	}
	else
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;

	return MAD_OK;
}

/*******************************************************************************
* mdUSGMIIGetCustPortID
*
* DESCRIPTION:
*       This function Get customized port ID in cust_port_id.
*
* INPUTS:
*       port - Port number to be configured.

*
* OUTPUTS:
*       cust_portid   - Customized port ID value for Ingress PCH headers
*
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*		Page 26, Reg 0, Bit 9 : Use customized port ID set in cust_port_id
*       Page 26, Reg 0, Bit 8:5 : Customized port ID value for Ingress PCH headers
*
*******************************************************************************/
MAD_STATUS mdUSGMIIGetCustPortID
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT   port,
OUT MAD_U16     *cust_portid
)
{
	MAD_U16     data;
	MAD_STATUS  retVal;
	MAD_U8      hwPort;

	MAD_DBG_INFO(("mdUSGMIIGetCustPortID called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);

	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}


	if (!(MAD_DEV_HWREALMODE(dev, hwPort, MAD_PHY_MODE_USGMII)))
	{
		MAD_DBG_INFO(("The harware mode is not USGMII.\n"));
		return MAD_API_USGMII_NOT_SUPPORT;
	}

	if ((MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR)) &&
		(!MAD_DEV_IS_IN_SWITCH(dev)))
	{
		if ((retVal = madHwGetPagedPhyRegField(
			dev, hwPort, MAD_PAGE26, MAD_REG_PCH_COMMON_CTRL,
			9, 1, &data)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Reading to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_GET_PAGED_REG_FIELD);
		}

		if (!data)
		{
			MAD_DBG_INFO(("Get Customized port ID failed,Use default PHY address as port ID.\n"));
			return MAD_FAIL;
		}

		if ((retVal = madHwGetPagedPhyRegField(
			dev, hwPort, MAD_PAGE26, MAD_REG_PCH_COMMON_CTRL,
			5, 4, cust_portid)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
	}
	else
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;

	return MAD_OK;
}

/*******************************************************************************
* mdUSGMIISetExtFieldType
*
* DESCRIPTION:
*       This function Set Extention Field Type value for Ingress PCH headers.
*
* INPUTS:
*       port - Port number to be configured.
*       fieldtype   -   00: Ignore Extension Field
*						01: Extension Field contains 8-bit Reserved + 32-bit Timestamp
*						10, 11: Reserved
*
* OUTPUTS:
*       None
*
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*       Page 26, Reg 0, Bit 4:3 : Extention Field Type value for Ingress PCH headers
*
*******************************************************************************/
MAD_STATUS mdUSGMIISetExtFieldType
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT   port,
IN  MAD_USGMII_EXT_FIELD_TYPE     fieldtype
)
{
	MAD_STATUS  retVal;
	MAD_U8      hwPort;

	MAD_DBG_INFO(("mdUSGMIISetExtFieldType called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);

	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}


	if (!(MAD_DEV_HWREALMODE(dev, hwPort, MAD_PHY_MODE_USGMII)))
	{
		MAD_DBG_INFO(("The harware mode is not USGMII.\n"));
		return MAD_API_USGMII_NOT_SUPPORT;
	}

	if ((MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR)) &&
		(!MAD_DEV_IS_IN_SWITCH(dev)))
	{
		if ((retVal = madHwSetPagedPhyRegField(
			dev, hwPort, MAD_PAGE26, MAD_REG_PCH_COMMON_CTRL,
			3, 2, fieldtype)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
	}
	else
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;

	return MAD_OK;
}

/*******************************************************************************
* mdUSGMIIGetExtFieldType
*
* DESCRIPTION:
*       This function Get Extention Field Type value for Ingress PCH headers.
*
* INPUTS:
*       port - Port number to be configured.
*
* OUTPUTS:
*       fieldtype   -   00: Ignore Extension Field
*						01: Extension Field contains 8-bit Reserved + 32-bit Timestamp
*						10, 11: Reserved
*
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*       Page 26, Reg 0, Bit 4:3 : Extention Field Type value for Ingress PCH headers
*
*******************************************************************************/
MAD_STATUS mdUSGMIIGetExtFieldType
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT   port,
OUT MAD_USGMII_EXT_FIELD_TYPE     *fieldtype
)
{
	MAD_STATUS  retVal;
	MAD_U8      hwPort;
	MAD_U16		data;

	MAD_DBG_INFO(("mdUSGMIIGetExtFieldType called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);

	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}


	if (!(MAD_DEV_HWREALMODE(dev, hwPort, MAD_PHY_MODE_USGMII)))
	{
		MAD_DBG_INFO(("The harware mode is not USGMII.\n"));
		return MAD_API_USGMII_NOT_SUPPORT;
	}

	if ((MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR)) &&
		(!MAD_DEV_IS_IN_SWITCH(dev)))
	{
		if ((retVal = madHwGetPagedPhyRegField(
			dev, hwPort, MAD_PAGE26, MAD_REG_PCH_COMMON_CTRL,
			3, 2, &data)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Reading to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_GET_PAGED_REG_FIELD);
		}
		*fieldtype = data;
	}
	else
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;

	return MAD_OK;
}

/*******************************************************************************
* mdUSGMIIEnableInterrupt
*
* DESCRIPTION:
*       This function Enable USGMII Interrupt.
*
* INPUTS:
*       port - Port number to be configured.
*       intgroup   -   port vector with the bits set for the corresponding. 
*
* OUTPUTS:
*       None
*
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*       Page 30, Reg 18, Bit 14:7 : Enable USGMII Interrupt.
*
*******************************************************************************/
MAD_STATUS mdUSGMIIEnableInterrupt
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT   port,
IN  MAD_U16     intgroup
)
{
	MAD_STATUS  retVal;
	MAD_U8      hwPort;

	MAD_DBG_INFO(("mdUSGMIIEnableInterrupt called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);

	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}


	if (!(MAD_DEV_HWREALMODE(dev, hwPort, MAD_PHY_MODE_USGMII)))
	{
		MAD_DBG_INFO(("The harware mode is not USGMII.\n"));
		return MAD_API_USGMII_NOT_SUPPORT;
	}

	if ((MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR)) &&
		(!MAD_DEV_IS_IN_SWITCH(dev)))
	{
		if ((retVal = madHwWritePagedPhyReg(
			dev, hwPort, MAD_PAGE30, MAD_REG_USGMII_INT_ENABLE,
			intgroup)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
	}
	else
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;

	return MAD_OK;
}

/*******************************************************************************
* mdUSGMIIGetInterruptStatus
*
* DESCRIPTION:
*       This function Get USGMII Interrupt.
*
* INPUTS:
*       port - Port number to be configured.
*
* OUTPUTS:
*       intstatus   -   USGMII Interrupt Status Register
*
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*       Page 30, Reg 19, Bit 14:7 : USGMII Interrupt Status Register
*
*******************************************************************************/
MAD_STATUS mdUSGMIIGetInterruptStatus
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT   port,
OUT MAD_U16     *intstatus
)
{
	MAD_STATUS  retVal;
	MAD_U8      hwPort;

	MAD_DBG_INFO(("mdUSGMIIGetInterrupt called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);

	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}


	if (!(MAD_DEV_HWREALMODE(dev, hwPort, MAD_PHY_MODE_USGMII)))
	{
		MAD_DBG_INFO(("The harware mode is not USGMII.\n"));
		return MAD_API_USGMII_NOT_SUPPORT;
	}
	

	if ((MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR)) &&
		(!MAD_DEV_IS_IN_SWITCH(dev)))
	{
		if ((retVal = madHwReadPagedPhyReg(
			dev, hwPort, MAD_PAGE30, MAD_REG_USGMII_STATUS_2,
			intstatus)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}

	}
	else
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;

	return MAD_OK;
}

/*******************************************************************************
* mdUSGMIIGetSyncStatus
*
* DESCRIPTION:
*       This function get USGMII sync status.
*
* INPUTS:
*       port - Port number to be configured.
*
* OUTPUTS:
*       syncstatus   - The read register's sync data.
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*       Page 30, Reg 29, Bit 0:8 : USGMII sync status
*
*******************************************************************************/
MAD_STATUS mdUSGMIIGetSyncStatus
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT   port,
OUT MAD_U16    *syncstatus
)
{
	MAD_STATUS  retVal;
	MAD_U8      hwPort;

	MAD_DBG_INFO(("mdUSGMIIGetSyncStatus called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);

	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}


	if (!(MAD_DEV_HWREALMODE(dev, hwPort, MAD_PHY_MODE_USGMII)))
	{
		MAD_DBG_INFO(("The harware mode is not USGMII.\n"));
		return MAD_API_USGMII_NOT_SUPPORT;
	}

	if ((MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR)) &&
		(!MAD_DEV_IS_IN_SWITCH(dev)))
	{
		if ((retVal = madHwGetPagedPhyRegField(
			dev, hwPort, MAD_PAGE30, MAD_REG_USGMII_SYNC_STATUS,
			0, 9, syncstatus)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Reading to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_GET_PAGED_REG_FIELD);
		}

	}
	else
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;

	return MAD_OK;
}

/*******************************************************************************
* mdUSGMIISetInvertPolarity
*
* DESCRIPTION:
*       This function Set the SERDES polarity inversion(RX and TX).
*
* INPUTS:
*       port - Port number to be configured.
*		invertpol - polarity inversion(RX and TX).
*
* OUTPUTS:
*       None.
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*       Page 30, Reg 26, Bit 13:12 : SERDES polarity inversion(RX and TX)
*
*******************************************************************************/
MAD_STATUS mdUSGMIISetInvertPolarity
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT   port,
IN  MAD_INVERT_POLARITY    *invertpol
)
{
	MAD_STATUS  retVal;
	MAD_U8      hwPort;
	MAD_U16      data = 0x00;
	
	MAD_DBG_INFO(("mdUSGMIISetInvertPolarity called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);

	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}


	if (!(MAD_DEV_HWREALMODE(dev, hwPort, MAD_PHY_MODE_USGMII)))
	{
		MAD_DBG_INFO(("The harware mode is not USGMII.\n"));
		return MAD_API_USGMII_NOT_SUPPORT;
	}

	data |= (invertpol->invertinp << 13);
	data |= (invertpol->invertoutp << 12);

	if ((MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR)) &&
		(!MAD_DEV_IS_IN_SWITCH(dev)))
	{
		if ((retVal = madHwWritePagedPhyReg(
			dev, hwPort, MAD_PAGE30, MAD_REG_USGMII_GLOBAL_CTRL_1,
			data)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}

	}
	else
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;

	return MAD_OK;
}

/*******************************************************************************
* mdUSGMIIGetInvertPolarity
*
* DESCRIPTION:
*       This function Get the SERDES polarity inversion(RX and TX).
*
* INPUTS:
*       port - Port number to be configured.
*
* OUTPUTS:
*		invertpol - polarity inversion(RX and TX).
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*       Page 30, Reg 26, Bit 13:12 : SERDES polarity inversion(RX and TX)
*
*******************************************************************************/
MAD_STATUS mdUSGMIIGetInvertPolarity
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT   port,
OUT  MAD_INVERT_POLARITY    *invertpol
)
{
	MAD_STATUS  retVal;
	MAD_U8      hwPort;
	MAD_U16      data;

	MAD_DBG_INFO(("mdUSGMIIGetInvertPolarity called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);

	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}


	if (!(MAD_DEV_HWREALMODE(dev, hwPort, MAD_PHY_MODE_USGMII)))
	{
		MAD_DBG_INFO(("The harware mode is not USGMII.\n"));
		return MAD_API_USGMII_NOT_SUPPORT;
	}

	if ((MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR)) &&
		(!MAD_DEV_IS_IN_SWITCH(dev)))
	{
		if ((retVal = madHwReadPagedPhyReg(
			dev, hwPort, MAD_PAGE30, MAD_REG_USGMII_GLOBAL_CTRL_1,
			&data)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Reading to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_GET_PAGED_REG_FIELD);
		}

		invertpol->invertinp = (data & 0x2000) >> 13;
		invertpol->invertoutp = (data & 0x1000) >> 12;

	}
	else
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;

	return MAD_OK;
}

/*******************************************************************************
* mdUSGMIISetSerdesTapChange
*
* DESCRIPTION:
*       This function Set the USGMII SERDES tap change.
*
* INPUTS:
*       port - Port number to be configured.
*		change - USGMII SERDES amplitude/pre/post change.
*
* OUTPUTS:
*       None.
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*       Page 240, Reg 101 : AFE Test Registers: TX settings for S1
*
*******************************************************************************/
MAD_STATUS mdUSGMIISetSerdesTapChange  
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT   port,
IN  MAD_SERDES_TAP_CHANGE     *change
)
{
	MAD_STATUS  retVal;
	MAD_U8      hwPort;
	MAD_U16     data = 0x00;
	MAD_U16     addr = 0x00;

	MAD_DBG_INFO(("mdUSGMIISetSERDESAmpChange called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);

	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}


	if (!(MAD_DEV_HWREALMODE(dev, hwPort, MAD_PHY_MODE_USGMII)))
	{
		MAD_DBG_INFO(("The harware mode is not USGMII.\n"));
		return MAD_API_USGMII_NOT_SUPPORT;
	}

	data |= change->pre << 11;
	data |= change->amplitude << 5;
	data |= change->post;
	addr  = 0x101;
	
	if ((MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR)) &&
		(!MAD_DEV_IS_IN_SWITCH(dev)))
	{
		if ((retVal = mdSetPhyUSGMIIRegIdt(dev, port, addr, data)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
	}
	else
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;

	return MAD_OK;
}

/*******************************************************************************
* mdUSGMIIGetSerdesTapChange
*
* DESCRIPTION:
*       This function Get the USGMII SERDES tap change.
*
* INPUTS:
*       port - Port number to be configured.
*
* OUTPUTS:
*		change - USGMII SERDES amplitude/pre/post change.
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*       Page 240, Reg 101 : AFE Test Registers: TX settings for S1
*
*******************************************************************************/
MAD_STATUS mdUSGMIIGetSerdesTapChange
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT   port,
OUT  MAD_SERDES_TAP_CHANGE     *change
)
{
	MAD_STATUS  retVal;
	MAD_U8      hwPort;
	MAD_U16     data = 0x00;
	MAD_U16     addr = 0x00;

	MAD_DBG_INFO(("mdUSGMIIGetSERDESAmpChange called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);

	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}


	if (!(MAD_DEV_HWREALMODE(dev, hwPort, MAD_PHY_MODE_USGMII)))
	{
		MAD_DBG_INFO(("The harware mode is not USGMII.\n"));
		return MAD_API_USGMII_NOT_SUPPORT;
	}

	addr = 0x101;

	if ((MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR)) &&
		(!MAD_DEV_IS_IN_SWITCH(dev)))
	{
		if ((retVal = mdGetPhyUSGMIIRegIdt(dev, port, addr, &data)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Reading to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
		change->pre = (data & 0xF800) >> 11;
		change->amplitude = (data & 0x07E0) >> 5;
		change->post = data & 0x001F;
	}
	else
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;

	return MAD_OK;
}

/*******************************************************************************
* mdUSGMIIEnablePCH
*
* DESCRIPTION:
*       This function Enable the USGMII PCH.
*
* INPUTS:
*       port - Port number to be configured.
*		en   - Enable the USGMII PCH.
*
* OUTPUTS:
*       None.
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*       Page 26, Reg 0 bit 0 : Enable USGMII PCH
*
*******************************************************************************/
MAD_STATUS mdUSGMIIEnablePCH
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT   port,
IN  MAD_BOOL    en
)
{
	MAD_STATUS  retVal;
	MAD_U8      hwPort;

	MAD_DBG_INFO(("mdUSGMIIEnablePCH called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);

	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}


	if (!(MAD_DEV_HWREALMODE(dev, hwPort, MAD_PHY_MODE_USGMII)))
	{
		MAD_DBG_INFO(("The harware mode is not USGMII.\n"));
		return MAD_API_USGMII_NOT_SUPPORT;
	}


	if ((MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR)) &&
		(!MAD_DEV_IS_IN_SWITCH(dev)))
	{
		if ((retVal = madHwSetPagedPhyRegField(dev, hwPort, MAD_PAGE26, 0, 0, 1, en)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
	}
	else
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;

	return MAD_OK;
}

/*******************************************************************************
* mdUSGMIISetPCHClk
*
* DESCRIPTION:
*       This function Set USGMII PCH clock with external PTP clock.
*
* INPUTS:
*       port - Port number to be configured.
*		en   - Enable the USGMII PCH clock.
*			   MAD_TRUE: Select external PTP clock
*
* OUTPUTS:
*       None.
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*		Page 28, Reg 14 bit 11 : PTP Pin Mode Select 
*
*******************************************************************************/
MAD_STATUS mdUSGMIISetPCHClk
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT   port,
IN  MAD_BOOL    en
)
{
	MAD_STATUS  retVal;
	MAD_U8      hwPort;
	MAD_BOOL    enable;

	MAD_DBG_INFO(("mdUSGMIISetPCHClk called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);

	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}


	if (!(MAD_DEV_HWREALMODE(dev, hwPort, MAD_PHY_MODE_USGMII)))
	{
		MAD_DBG_INFO(("The harware mode is not USGMII.\n"));
		return MAD_API_USGMII_NOT_SUPPORT;
	}

	if (en)
	{
		enable = 0;
	}
	else
	{
		enable = 1;
	}

	if ((MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR)) &&
		(!MAD_DEV_IS_IN_SWITCH(dev)))
	{
		if ((retVal = madHwSetPagedPhyRegField(dev, hwPort, MAD_PAGE28, 14, 11, 1, enable)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
		if ((retVal = madHwSetPagedPhyRegField(dev, hwPort, MAD_PAGE28, 14, 10, 1, 1)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
	}
	else
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;

	return MAD_OK;
}

/*******************************************************************************
* mdUSGMIIWriteDebugETSB
*
* DESCRIPTION:
*       This function write the PCH time stamping with debug mode.
*
* INPUTS:
*       port - Port number to be configured.
*		addr - ETSB Address
*		data - ETSB data
*
* OUTPUTS:
*       None.
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*		Page 26, Reg 23 : ETSB Access Control
*		Page 26, Reg 24,25,26,27 : ETSB Debug Write Data
*
*******************************************************************************/
MAD_STATUS mdUSGMIIWriteDebugETSB
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT   port,
IN  MAD_U16     addr,
IN  MAD_U64     data
)
{
	MAD_STATUS  retVal;
	MAD_U8      hwPort;
	MAD_U16     data0;
	MAD_U16     data1;
	MAD_U16     data2;
	MAD_U16     data3;
	MAD_U16     etsb_addr = 0x00;

	MAD_DBG_INFO(("mdUSGMIIWriteDebugETSB called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);

	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}


	if (!(MAD_DEV_HWREALMODE(dev, hwPort, MAD_PHY_MODE_USGMII)))
	{
		MAD_DBG_INFO(("The harware mode is not USGMII.\n"));
		return MAD_API_USGMII_NOT_SUPPORT;
	}

	data0 = (MAD_U16)(data & 0xFFFFULL);
	data1 = (MAD_U16)((data & 0xFFFF0000ULL) >> 16);
	data2 = (MAD_U16)((data & 0xFFFF00000000ULL) >> 32);
	data3 = (MAD_U16)((data & 0xF000000000000ULL) >> 48);
	etsb_addr |= addr | 0x0400;

	if ((MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR)) &&
		(!MAD_DEV_IS_IN_SWITCH(dev)))
	{
		if ((retVal = madHwSetPagedPhyRegField(dev, hwPort, MAD_PAGE26, 2, 0, 1, 1)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}

		if ((retVal = madHwWritePagedPhyReg(dev, hwPort, MAD_PAGE26, 24, data0)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}

		if ((retVal = madHwWritePagedPhyReg(dev, hwPort, MAD_PAGE26, 25, data1)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}

		if ((retVal = madHwWritePagedPhyReg(dev, hwPort, MAD_PAGE26, 26, data2)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}

		if ((retVal = madHwWritePagedPhyReg(dev, hwPort, MAD_PAGE26, 27, data3)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}

		if ((retVal = madHwWritePagedPhyReg(dev, hwPort, MAD_PAGE26, 23, etsb_addr)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
	}
	else
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;

	return MAD_OK;
}

/*******************************************************************************
* mdUSGMIIReadDebugETSB
*
* DESCRIPTION:
*       This function read the PCH time stamping with debug mode.
*
* INPUTS:
*       port - Port number to be configured.
*		addr - ETSB Address
*
* OUTPUTS:
*		data - ETSB data
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*		Page 26, Reg 23 : ETSB Access Control
*		Page 26, Reg 28,29,30,31 : ETSB Read Data
*
*******************************************************************************/
MAD_STATUS mdUSGMIIReadDebugETSB
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT   port,
IN  MAD_U16     addr,
OUT MAD_U64     *data
)
{
	MAD_STATUS  retVal;
	MAD_U8      hwPort;
	MAD_U16		tmpdata = 0;
	MAD_U64     data0 = 0;
	MAD_U64     data1 = 0;
	MAD_U64     data2 = 0;
	MAD_U64     data3 = 0;
	MAD_U16     etsb_addr = 0x00;

	MAD_DBG_INFO(("mdUSGMIIReadDebugETSB called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);

	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}


	if (!(MAD_DEV_HWREALMODE(dev, hwPort, MAD_PHY_MODE_USGMII)))
	{
		MAD_DBG_INFO(("The harware mode is not USGMII.\n"));
		return MAD_API_USGMII_NOT_SUPPORT;
	}

	etsb_addr = addr | 0x0200;

	if ((MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR)) &&
		(!MAD_DEV_IS_IN_SWITCH(dev)))
	{
		if ((retVal = madHwSetPagedPhyRegField(dev, hwPort, MAD_PAGE26, 2, 0, 1, 1)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}

		if ((retVal = madHwWritePagedPhyReg(dev, hwPort, MAD_PAGE26, 23, etsb_addr)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}

		if ((retVal = madHwReadPagedPhyReg(dev, hwPort, MAD_PAGE26, 28, &tmpdata)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
		data0 = tmpdata;
		if ((retVal = madHwReadPagedPhyReg(dev, hwPort, MAD_PAGE26, 29, &tmpdata)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
		data1 = tmpdata;
		if ((retVal = madHwReadPagedPhyReg(dev, hwPort, MAD_PAGE26, 30, &tmpdata)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
		data2 = tmpdata;
		if ((retVal = madHwReadPagedPhyReg(dev, hwPort, MAD_PAGE26, 31, &tmpdata)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
		data3 = tmpdata;
		*data = data3 << 48 | data2 << 32 | data1 << 16 | data0;

	}
	else
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;

	return MAD_OK;
}

/*******************************************************************************
* mdUSGMIIReadETSB
*
* DESCRIPTION:
*       This function read the PCH time stamping with normal mode.
*
* INPUTS:
*       port - Port number to be configured.
*		addr - ETSB Address
*
* OUTPUTS:
*		data - ETSB data
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*		Page 26, Reg 23 : ETSB Access Control
*		Page 26, Reg 28,29,30,31 : ETSB Read Data
*
*******************************************************************************/
MAD_STATUS mdUSGMIIReadETSB
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT   port,
IN  MAD_U16     addr,
OUT MAD_U64     *data
)
{
	MAD_STATUS  retVal;
	MAD_U8      hwPort;
	MAD_U16		tmpdata = 0;
	MAD_U64     data0 = 0;
	MAD_U64     data1 = 0;
	MAD_U64     data2 = 0;
	MAD_U64     data3 = 0;
	MAD_U16     etsb_addr = 0x00;

	MAD_DBG_INFO(("mdUSGMIIReadETSB called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);

	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}


	if (!(MAD_DEV_HWREALMODE(dev, hwPort, MAD_PHY_MODE_USGMII)))
	{
		MAD_DBG_INFO(("The harware mode is not USGMII.\n"));
		return MAD_API_USGMII_NOT_SUPPORT;
	}

	etsb_addr = addr | 0x0100;

	if ((MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR)) &&
		(!MAD_DEV_IS_IN_SWITCH(dev)))
	{

		if ((retVal = madHwSetPagedPhyRegField(dev, hwPort, MAD_PAGE26, 2, 0, 1, 0)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}

		if ((retVal = madHwWritePagedPhyReg(dev, hwPort, MAD_PAGE26, 23, etsb_addr)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}

		if ((retVal = madHwReadPagedPhyReg(dev, hwPort, MAD_PAGE26, 28, &tmpdata)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Reading to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
		data0 = tmpdata;
		if ((retVal = madHwReadPagedPhyReg(dev, hwPort, MAD_PAGE26, 29, &tmpdata)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Reading to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
		data1 = tmpdata;
		if ((retVal = madHwReadPagedPhyReg(dev, hwPort, MAD_PAGE26, 30, &tmpdata)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Reading to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
		data2 = tmpdata;
		if ((retVal = madHwReadPagedPhyReg(dev, hwPort, MAD_PAGE26, 31, &tmpdata)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Reading to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
		data3 = tmpdata;

		*data = data3 << 48 | data2 << 32 | data1 << 16 | data0;

	}
	else
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;

	return MAD_OK;
}

#define EYETARGETROWS	63
#define EYETARGETCOLS	64 * 2

static MAD_STATUS doMeasHalfEye
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT   port,
IN  MAD_U16     row,
IN  MAD_U16     col,
OUT MAD_U32     *eyedata
)
{
	MAD_STATUS  retVal;
	MAD_U32 amp = (EYETARGETROWS - 1) / 2 + 2;
	MAD_U32 lsb, msb;
	MAD_U16 reg11 = 0, reg14 = 0;
	MAD_U16 tmpAddr = 0x0000;
	MAD_U16 tmpdata = 0;
	MAD_U16 i = 0, j = 0;

	for (i = 0; i < (EYETARGETROWS + 1) / 2; i++)
	{
		/*i) write amp*/
		if (i < (EYETARGETROWS + 1) / 4) 
			amp = amp - 2;
		else if (i == (EYETARGETROWS + 1) / 4) 
			amp = 0;
		else if (i >(EYETARGETROWS + 1) / 4) 
			amp = amp + 2;
		/*write eye amplitude $V.$PL11.10:6 */
		if ((retVal = mdGetPhyUSGMIIRegIdt(dev, port, tmpAddr + 0x11, &reg11)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Reading to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
		if ((retVal = mdSetPhyUSGMIIRegIdt(dev, port, tmpAddr + 0x11, (reg11 & 0xF83F) | (amp << 6))) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
		/*read phase value*/
		if ((retVal = mdGetPhyUSGMIIRegIdt(dev, port, tmpAddr + 0x11, &reg11)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Reading to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
		for ( j = 0; j < 64; j++)  /* through 64 phases (64 columns) */
		{
			/* write phase */
			if ((retVal = mdSetPhyUSGMIIRegIdt(dev, port, tmpAddr + 0x11, (reg11 & 0xFFC0) | j)) != MAD_OK)
			{
				MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
				return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
			}
			/*begin measure*/
			if ((retVal = mdGetPhyUSGMIIRegIdt(dev, port, tmpAddr + 0x11, &reg11)) != MAD_OK)
			{
				MAD_DBG_ERROR(("Reading to phy reg failed.\n"));
				return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
			}
			if ((retVal = mdSetPhyUSGMIIRegIdt(dev, port, tmpAddr + 0x11, (reg11 | 0x8000))) != MAD_OK)
			{
				MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
				return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
			}
			/*wait for measure done*/
			MAD_U32  cnt = 0;
			while (1) 
			{
				if ((retVal = mdGetPhyUSGMIIRegIdt(dev, port, tmpAddr + 0x11, &tmpdata)) != MAD_OK)
				{
					MAD_DBG_ERROR(("Reading to phy reg failed.\n"));
					return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
				}
				if ((tmpdata & 0x8000) == 0)
					break;
				cnt++;
				if (cnt > 20000) /*wait for 1s, assume each reg access is 50 us*/
					return MAD_API_FAIL_EYE_MONITOR;
			}

			/*write $V.$PL14.11:9 = 010; Read P even eye error counter*/
			if ((retVal = mdGetPhyUSGMIIRegIdt(dev, port, tmpAddr + 0x14, &reg14)) != MAD_OK)
			{
				MAD_DBG_ERROR(("Reading to phy reg failed.\n"));
				return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
			}
			if ((retVal = mdSetPhyUSGMIIRegIdt(dev, port, tmpAddr + 0x14, ((reg14 & 0xF0FF) | 0x200))) != MAD_OK)
			{
				MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
				return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
			}
			/*read measure result*/
			if ((retVal = mdGetPhyUSGMIIRegIdt(dev, port, tmpAddr + 0x12, &tmpdata)) != MAD_OK)
			{
				MAD_DBG_ERROR(("Reading to phy reg failed.\n"));
				return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
			}
			lsb = (tmpdata & 0xFFFF);
			if ((retVal = mdGetPhyUSGMIIRegIdt(dev, port, tmpAddr + 0x13, &tmpdata)) != MAD_OK)
			{
				MAD_DBG_ERROR(("Reading to phy reg failed.\n"));
				return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
			}
			msb = (tmpdata & 0xFFFF);

			*(eyedata + col * amp + j) = (msb << 16) + lsb;

			/*write $V.$PL14.11:9 = 011; Read P odd eye error counter*/
			if ((retVal = mdSetPhyUSGMIIRegIdt(dev, port, tmpAddr + 0x14, ((reg14 & 0xF0FF) | 0x300))) != MAD_OK)
			{
				MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
				return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
			}
			/*read measure result*/
			if ((retVal = mdGetPhyUSGMIIRegIdt(dev, port, tmpAddr + 0x12, &tmpdata)) != MAD_OK)
			{
				MAD_DBG_ERROR(("Reading to phy reg failed.\n"));
				return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
			}
			lsb = (tmpdata & 0xFFFF);
			if ((retVal = mdGetPhyUSGMIIRegIdt(dev, port, tmpAddr + 0x13, &tmpdata)) != MAD_OK)
			{
				MAD_DBG_ERROR(("Reading to phy reg failed.\n"));
				return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
			}
			msb = (tmpdata & 0xFFFF);

			*(eyedata + col * amp + j + 64) = (msb << 16) + lsb;

		} /*64 phases*/
	}
	return MAD_OK;
}

static MAD_STATUS measureEye
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT   port,
IN  MAD_U32    sample,
OUT MAD_U32     *eyedata
)
{
	MAD_STATUS  retVal;
	MAD_U32 eyeDataUp1[(EYETARGETROWS + 1) / 2][EYETARGETCOLS] = { { 0 } };
	MAD_U32 eyeDataDn1[(EYETARGETROWS + 1) / 2][EYETARGETCOLS] = { { 0 } };

	MAD_U32 *eyeDataUp = *eyeDataUp1;
	MAD_U32 *eyeDataDn = *eyeDataDn1;

	MAD_U32 raw_topHalf_2eye[(EYETARGETROWS + 1) / 2][EYETARGETCOLS] = { { 0 } };
	MAD_U32 raw_bottomHalf_2eye[(EYETARGETROWS + 1) / 2][EYETARGETCOLS] = { { 0 } };
	MAD_U16 reg_8xBE, DFEdata;
	MAD_U16 tmpAddr = 0x0000;
	MAD_U16 tmpdata = 0;
	MAD_U16 cnt = 0;
	MAD_U16 i = 0, j = 0;

	if ((retVal = mdGetPhyUSGMIIRegIdt(dev, port, tmpAddr + 0xBE, &tmpdata)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Reading to phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}
	reg_8xBE = tmpdata & 0x7f;

	if ((retVal = mdGetPhyUSGMIIRegIdt(dev, port, tmpAddr + 0x84, &tmpdata)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Reading to phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}
	DFEdata = ((tmpdata & 0x7F00) >> 8); 

	/*Set eye monitor speed*/
	/*TBD*/

	/* need set phase to zero */
	if ((retVal = mdGetPhyUSGMIIRegIdt(dev, port, tmpAddr + 0x11, &tmpdata)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Reading to phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}
	if ((retVal = mdSetPhyUSGMIIRegIdt(dev, port, tmpAddr + 0x11, (tmpdata & 0xFFC0))) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}
	/*1. enable eye monitor*/
	if ((retVal = mdGetPhyUSGMIIRegIdt(dev, port, tmpAddr + 0x10, &tmpdata)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Reading to phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}
	if ((retVal = mdSetPhyUSGMIIRegIdt(dev, port, tmpAddr + 0x10, (tmpdata | 0x8000))) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}
	while (1)
	{
		if ((retVal = mdGetPhyUSGMIIRegIdt(dev, port, tmpAddr + 0x10, &tmpdata)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Reading to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
		if ((tmpdata & 0x8000) == 0x8000)
			break;
		cnt++;
		if (cnt > 2000)
		{
			MAD_DBG_ERROR(("EM enable failed. Measurement aborted..\n"));
			return MAD_API_FAIL_EYE_MONITOR;
		}
	}
	/*2. write sample count for calibration*/
	if ((retVal = mdGetPhyUSGMIIRegIdt(dev, port, tmpAddr + 0x10, &tmpdata)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Reading to phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}
	if ((retVal = mdSetPhyUSGMIIRegIdt(dev, port, tmpAddr + 0x10, ((tmpdata & 0xC1FF) | 0x2800))) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}
	/*3. write eye amplitude $V.$PL11.10:6 = 0x19*/
	if ((retVal = mdGetPhyUSGMIIRegIdt(dev, port, tmpAddr + 0x11, &tmpdata)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Reading to phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}
	if ((retVal = mdSetPhyUSGMIIRegIdt(dev, port, tmpAddr + 0x11, ((tmpdata & 0xF83F) | 0x0640))) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}

	/*-------------------- eye monitor measure -----------------------*/
	/*1. set compare sample number*/
	if ((retVal = mdGetPhyUSGMIIRegIdt(dev, port, tmpAddr + 0x10, &tmpdata)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Reading to phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}
	if ((retVal = mdSetPhyUSGMIIRegIdt(dev, port, tmpAddr + 0x10, ((tmpdata & 0xC1FF) | (sample << 9)))) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}
	/*2. first write top half $V.$PL11.11 = 0x1*/
	if ((retVal = mdGetPhyUSGMIIRegIdt(dev, port, tmpAddr + 0x11, &tmpdata)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Reading to phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}
	if ((retVal = mdSetPhyUSGMIIRegIdt(dev, port, tmpAddr + 0x11, (tmpdata | 0x0800))) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}
	/*top half --- loop on amplitude & phase, and measure the eye*/
	doMeasHalfEye(dev, port, EYETARGETROWS, EYETARGETCOLS, eyeDataUp);

	/*3. write bottom half $V.$PL11.11 = 0x0*/
	if ((retVal = mdGetPhyUSGMIIRegIdt(dev, port, tmpAddr + 0x11, &tmpdata)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Reading to phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}
	if ((retVal = mdSetPhyUSGMIIRegIdt(dev, port, tmpAddr + 0x11, (tmpdata & 0xF7FF))) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}
	/*bottom half --- Measure bottom half of the first eye and top half of the 2nd eye*/
	doMeasHalfEye(dev, port, EYETARGETROWS, EYETARGETCOLS, eyeDataDn);

	/*4. write back eye amplitude $V.$PL11.10:6 = 0x19*/
	if ((retVal = mdGetPhyUSGMIIRegIdt(dev, port, tmpAddr + 0x11, &tmpdata)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Reading to phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}
	if ((retVal = mdSetPhyUSGMIIRegIdt(dev, port, tmpAddr + 0x11, ((tmpdata & 0xF83F) | 0x0640))) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}
	/*Disable eye monitor $V.$PL10.15 = 0x0*/
	if ((retVal = mdGetPhyUSGMIIRegIdt(dev, port, tmpAddr + 0x10, &tmpdata)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Reading to phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}
	if ((retVal = mdSetPhyUSGMIIRegIdt(dev, port, tmpAddr + 0x10, (tmpdata & 0x7FFF))) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}
	
	for (i = 0; i < (EYETARGETROWS + 1) / 2; i++) 
	{
		for (j = 0; j < 64; j++)   /*through 64 phases (64 columns)*/ 
		{
			raw_topHalf_2eye[i][j] = eyeDataUp[i*EYETARGETCOLS + j];
			raw_bottomHalf_2eye[i][j] = eyeDataDn[i*EYETARGETCOLS + j];
		}
		for (j = 0; j < 64; j++)  
		{
			raw_topHalf_2eye[i][j + 64] = eyeDataDn[i*EYETARGETCOLS + j + 64];
			raw_bottomHalf_2eye[i][j + 64] = eyeDataUp[i*EYETARGETCOLS + j + 64];
		}
	}

	for (i = 0; i < (EYETARGETROWS + 1) / 2 ; i++)  /*through all amplitudes*/ 
	{
		for (j = 0; j < 128; j++)   /*through 64 phases(64 columns) * 2*/
		{
			eyedata[i * EYETARGETCOLS + j] = (MAD_U32)raw_topHalf_2eye[i][j];
		}
	}
	for (i = 32; i < EYETARGETROWS ; i++) /*through all amplitudes*/
	{
		for (j = 0; j < 128; j++)   /*through 64 phases (64 columns) * 2*/
		{
			eyedata[i * EYETARGETCOLS + j] = (MAD_U32)raw_bottomHalf_2eye[(EYETARGETROWS - 1) / 2 - (i - 31)][j];
		}
	}
	eyedata[8064] = reg_8xBE;
	eyedata[8065] = DFEdata;

	return MAD_OK;
}

static MAD_STATUS selectBigEye
(
IN  MAD_U32      *eyedata,
OUT MAD_BOOL     *en
)
{
	MAD_U32 index = 0;
	MAD_U16 i = 0, j = 0;
	MAD_U32 maxH1 = 0,  maxH2 = 0;
	MAD_U32 countH = 0;
	MAD_U32 rawEye[EYETARGETROWS][EYETARGETCOLS] = { { 0 } };
	for (i = 0; i < EYETARGETROWS; i++)
	{
		for (j = 0; j < EYETARGETCOLS; j++)
		{
			rawEye[i][j] = eyedata[index++];
		}
	}
	/*calculate eye size*/
	for (j = 0; j < 64; j++)  /* through 64 phases (even eye)*/
	{
		countH = 0;
		for (i = 0; i < EYETARGETROWS; i++) /* through all amplitudes */
		{
			if (rawEye[i][j] == 0)
				countH++;
		}
		if (maxH1 < countH)
		{
			maxH1 = countH;
		}
	}
	for (j = 64; j < 128; j++)  /* through 64 phases (odd eye)*/
	{
		countH = 0;
		for (i = 0; i < EYETARGETROWS; i++) /* through all amplitudes */
		{
			if (rawEye[i][j] == 0)
				countH++;
		}
		if (maxH2 < countH)
		{
			maxH2 = countH;
		}
	}
	/*decide which eye (even or odd)*/
	if (maxH1 >= maxH2)
		*en = MAD_FALSE; /* even eye*/
	else
		*en = MAD_TRUE; /*odd eye*/

	return MAD_OK;
}

static MAD_STATUS CalculationCenterInfo_Bathtub
(
IN  MAD_BOOL    en,
IN  MAD_U32     *eyedata,
OUT MAD_U32     *height,
OUT MAD_U32     *width
)
{
	MAD_U16 reg_8xBE, DFEdata;
	MAD_U16 h0;
	int Row = 0, Col = 0;
	MAD_U32 j = 0;
	MAD_U32 RowCol[EYETARGETROWS][EYETARGETCOLS / 2] = { { 0 } };
	MAD_U32 leftZeroPosition = 0, rightZeroPosition = 0;
	MAD_U32 o = 0;
	MAD_U32 cnt = 0, maxN = 0, rowN = 0;
	MAD_BOOL st = MAD_FALSE;
	reg_8xBE = eyedata[8064];
	DFEdata = eyedata[8065];
	if (en)
		o = 64;
	else
		o = 0;
	for (Row = 0; Row < EYETARGETROWS; Row++)
	{
		cnt = 0;
		st = MAD_FALSE;
		for (Col = 0; Col < EYETARGETCOLS / 2; Col++)
		{
			RowCol[Row][Col] = eyedata[o];
			if (eyedata[o] == 0)
			{
				if (!st) { st = MAD_TRUE; cnt = 0; }
				cnt++;
			}
			else
			{
				st = MAD_FALSE;
			}
			o++;
			j++;
		}
		if (cnt > maxN)
		{
			maxN = cnt;
			rowN = Row;
		}
		o += 64;
	}
	/*centerline data*/
	for (Col = 0; Col < EYETARGETCOLS / 2; Col++)
	{
		if (RowCol[rowN][Col] == 0)
		{
			leftZeroPosition = Col;
			break;
		}
	}
	for (Col = EYETARGETCOLS / 2 - 1; Col >= 0; Col--)
	{
		if (RowCol[rowN][Col] == 0)
		{
			rightZeroPosition = Col;
			break;
		}
	}
	/*display center position---look for the top and bottom point, using calibrated value from reg0x80be*/
	MAD_U32 topZeroPosition = 0, bottomZeroPosition = 0;
	for (Row = 0; Row < EYETARGETROWS; Row++)
	{
		if (RowCol[Row][reg_8xBE] == 0)
		{
			topZeroPosition = Row;
			break;
		}
	}
	for (Row = EYETARGETROWS - 1; Row >= 0; Row--)
	{
		if (RowCol[Row][reg_8xBE] == 0)
		{
			bottomZeroPosition = Row;
			break;
		}
	}
	h0 = ((MAD_U16)((DFEdata / 64.0 - 1) * 250) + 1); /* h0 in mV */   
	*height = 2*(h0 * (bottomZeroPosition - topZeroPosition) / EYETARGETROWS);
	*width = ((rightZeroPosition - leftZeroPosition) * 1000 / 64);

	return MAD_OK;
}

/*******************************************************************************
* mdUSGMIIGetEyeMonitor
*
* DESCRIPTION:
*       This function get the height and width of eye monitor.
*
* INPUTS:
*       port - Port number to be configured.
*		sample - Sample Count is 2^(bit 11 to 7), and the valid range is from 10 to 29
*
* OUTPUTS:
*		height - The height of eye diagram. The unit is mV.
*		width  - The width of eye diagram. The unit is mUI.
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*	
*
*******************************************************************************/
MAD_STATUS mdUSGMIIGetEyeMonitor
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT   port,
IN  MAD_U16     sample,
OUT MAD_U32     *height,
OUT MAD_U32     *width
)
{
	MAD_U8      hwPort;
	MAD_BOOL    en;
	MAD_U32		eyedata[9000] = { 0 };

	MAD_DBG_INFO(("mdUSGMIIGetEyeMonitor called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);

	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}


	if (!(MAD_DEV_HWREALMODE(dev, hwPort, MAD_PHY_MODE_USGMII)))
	{
		MAD_DBG_INFO(("The harware mode is not USGMII.\n"));
		return MAD_API_USGMII_NOT_SUPPORT;
	}

	if ((MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR)) &&
		(!MAD_DEV_IS_IN_SWITCH(dev)))
	{
		if ((sample >= 10) && (sample <=29))
		{
			measureEye(dev, port, sample, eyedata);
			selectBigEye(eyedata, &en);
			CalculationCenterInfo_Bathtub(en, eyedata, height, width);
		}
		else{
			MAD_DBG_INFO(("The sample is not valid.\n"));
			return MAD_API_FAIL_EYE_MONITOR;
		}
		
	}
	else
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;

	return MAD_OK;
}

/*******************************************************************************
* mdUSGMIISelectCiscoMode
*
* DESCRIPTION:
*       This function select tx_cfg_mode to follow USGMII spec or SGMII spec.
*
* INPUTS:
*       port - Port number to be configured.
*		mode - MAD_PG_USGMII_SPEC: Tx_Config format as per USGMII specification Table 4
*			   MAD_PG_SGMII_SPEC:  In USGMII mode Speed shifted to 11:10, instead of 11:9.(Valid in USGMII mode only)
*
* OUTPUTS:
*		None.
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*		Page 30, Reg 27, Bit 15    : Tx_cfg_mode_sel
*
*******************************************************************************/
MAD_STATUS mdUSGMIISelectCiscoMode
(
IN  MAD_DEV     *dev,
IN  MAD_LPORT   port,
IN  MAD_USGMII_CISCO_MODE mode
)
{
	MAD_STATUS  retVal;
	MAD_U8      hwPort;
	MAD_U16     data;


	MAD_DBG_INFO(("mdUSGMIISelectCiscoMode called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);

	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}


	if (!(MAD_DEV_HWREALMODE(dev, hwPort, MAD_PHY_MODE_USGMII)))
	{
		MAD_DBG_INFO(("The harware mode is not USGMII.\n"));
		return MAD_API_USGMII_NOT_SUPPORT;
	}
	if (mode)
		data = 1;
	else
		data = 0;


	if ((MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR)) &&
		(!MAD_DEV_IS_IN_SWITCH(dev)))
	{
		if ((retVal = madHwSetPagedPhyRegField(
			dev, hwPort, MAD_PAGE30, MAD_REG_USGMII_GLOBAL_CTRL_2,
			15, 1, data)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
	}
	else
		return MAD_API_NEW_FEATURE_NOT_SUPPORT;

	return MAD_OK;

}
