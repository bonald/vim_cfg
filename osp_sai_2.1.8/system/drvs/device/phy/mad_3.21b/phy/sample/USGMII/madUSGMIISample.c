#include <madCopyright.h>
/********************************************************************************
* madUSGMIISample.c
*
* DESCRIPTION:
*      USGMII sample (startPktGenerator and stopPktGenerator).
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
* Read USGMII Link status.
*/
MAD_STATUS ReadUSGMIILinkStatus
(
MAD_DEV         *dev,
MAD_LPORT       port
)
{
	MAD_STATUS status;
	MAD_BOOL   linkOn;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Read USGMII Link status at port:%i\n", (int)port));

	/*
	*    Read USGMII Link status.
	*/
	if ((status = mdUSGMIIGetLinkStatus(dev, port, &linkOn)) != MAD_OK)
	{
		MSG_PRINT(("mdUSGMIIGetLinkStatus return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("The USGMII Link status: %i\n", (int)linkOn));

	return MAD_OK;
}

/*
* Read USGMII Link Partner's link status.
*/
MAD_STATUS ReadUSGMIILPLinkStatus
(
MAD_DEV         *dev,
MAD_LPORT       port
)
{
	MAD_STATUS status;
	MAD_BOOL   linkOn;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Read USGMII Link Partner's link status at port:%i\n", (int)port));

	/*
	*    Read USGMII Link Partner's link status.
	*/
	if ((status = mdUSGMIIGetLPLinkStatus(dev, port, &linkOn)) != MAD_OK)
	{
		MSG_PRINT(("mdUSGMIIGetLPLinkStatus return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("The USGMII Link Partner's Link status: %i\n", (int)linkOn));

	return MAD_OK;
}

/*
* Read USGMII's current speed.
*/
MAD_STATUS ReadUSGMIISpeed
(
MAD_DEV         *dev,
MAD_LPORT       port
)
{
	MAD_STATUS status;
	MAD_SPEED_MODE   speed;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Read USGMII's current speed at port:%i\n", (int)port));

	/*
	*    Read USGMII's current speed.
	*/
	if ((status = mdUSGMIIGetSpeedStatus(dev, port, &speed)) != MAD_OK)
	{
		MSG_PRINT(("mdUSGMIIGetSpeedStatus return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("The USGMII Speed status: %i\n", (int)speed));

	return MAD_OK;
}

/*
* Read the USGMII Link Partner's current speed.
*/
MAD_STATUS ReadUSGMIILPSpeed
(
MAD_DEV         *dev,
MAD_LPORT       port,
MAD_BOOL    TxCfgModeSel
)
{
	MAD_STATUS status;
	MAD_SPEED_MODE   speed;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Read the USGMII Link Partner'current speed at port:%i\n", (int)port));

	/*
	*   Read the USGMII Link Partner'scurrent speed.
	*/
	if ((status = mdUSGMIIGetLPSpeedStatus(dev, port, TxCfgModeSel, &speed)) != MAD_OK)
	{
		MSG_PRINT(("mdUSGMIIGetLPSpeedStatus return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("The USGMII Link Partner's Speed status: %i\n", (int)speed));

	return MAD_OK;
}

/*
* Read the USGMII current duplex.
*/
MAD_STATUS ReadUSGMIIDuplex
(
MAD_DEV         *dev,
MAD_LPORT       port
)
{
	MAD_STATUS status;
	MAD_DUPLEX_MODE   duplex;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Read the USGMII current duplex at port:%i\n", (int)port));

	/*
	*   Read the USGMII current duplex.
	*/
	if ((status = mdUSGMIIGetDuplexStatus(dev, port, &duplex)) != MAD_OK)
	{
		MSG_PRINT(("mdUSGMIIGetDuplexStatus return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("The USGMII Duplex status: %i\n", (int)duplex));

	return MAD_OK;
}

/*
* Read the USGMII Link Partner's current duplex.
*/
MAD_STATUS ReadUSGMIILPDuplex
(
MAD_DEV         *dev,
MAD_LPORT       port
)
{
	MAD_STATUS status;
	MAD_DUPLEX_MODE   duplex;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Read the USGMII Link Partner current duplex at port:%i\n", (int)port));

	/*
	*   Read the USGMII Link Partner current duplex.
	*/
	if ((status = mdUSGMIIGetLPDuplexStatus(dev, port, &duplex)) != MAD_OK)
	{
		MSG_PRINT(("mdUSGMIIGetLPDuplexStatus return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("The USGMII Link Partner's Duplex status: %i\n", (int)duplex));

	return MAD_OK;
}

/*
* Set USGMII power down.
*/
MAD_STATUS SetUSGMIIPwDown
(
MAD_DEV         *dev,
MAD_LPORT       port,
MAD_BOOL        en
)
{
	MAD_STATUS status;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Set USGMII power down at port:%i\n", (int)port));

	/*
	*   Set USGMII power down.
	*/
	if ((status = mdUSGMIISetPwDown(dev, port, en)) != MAD_OK)
	{
		MSG_PRINT(("mdUSGMIISetPwDown return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Read USGMII power down.
*/
MAD_STATUS ReadUSGMIIPwDown
(
MAD_DEV         *dev,
MAD_LPORT       port
)
{
	MAD_STATUS status;
	MAD_BOOL   en;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Read USGMII power down at port:%i\n", (int)port));

	/*
	*   Read USGMII power down.
	*/
	if ((status = mdUSGMIIGetPwDown(dev, port, &en)) != MAD_OK)
	{
		MSG_PRINT(("mdUSGMIIGetPwDown return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("The USGMII Power Down status: %i\n", (int)en));

	return MAD_OK;
}

/*
* Set USGMII Auto-Negotiation bypass.
*/
MAD_STATUS SetUSGMIIANBypassEnable
(
MAD_DEV         *dev,
MAD_LPORT       port,
MAD_BOOL        en
)
{
	MAD_STATUS status;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Set USGMII Auto-Negotiation bypass at port:%i\n", (int)port));

	/*
	*   Set USGMII Auto-Negotiation bypass.
	*/
	if ((status = mdUSGMIISetANBypassEnable(dev, port, en)) != MAD_OK)
	{
		MSG_PRINT(("mdUSGMIISetANBypass return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Read Auto-Negotiation bypass enable status.
*/
MAD_STATUS ReadUSGMIIANBypassEnable
(
MAD_DEV         *dev,
MAD_LPORT       port
)
{
	MAD_STATUS status;
	MAD_BOOL   en;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Read Auto-Negotiation bypass enable status at port:%i\n", (int)port));

	/*
	*   Read Auto-Negotiation bypass enable status.
	*/
	if ((status = mdUSGMIIGetANBypassEnable(dev, port, &en)) != MAD_OK)
	{
		MSG_PRINT(("mdUSGMIIGetANBypassEnable return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("The Auto-Negotiation bypass enable status: %i\n", (int)en));

	return MAD_OK;
}

/*
* Read Auto-Negotiation bypass status.
*/
MAD_STATUS ReadUSGMIIANBypassStatus
(
MAD_DEV         *dev,
MAD_LPORT       port
)
{
	MAD_STATUS status;
	MAD_BOOL   st;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Read Auto-Negotiation bypass status at port:%i\n", (int)port));

	/*
	*   Read Auto-Negotiation bypass status.
	*/
	if ((status = mdUSGMIIGetANBypassStatus(dev, port, &st)) != MAD_OK)
	{
		MSG_PRINT(("mdUSGMIIGetANBypassStatus return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("The Auto-Negotiation bypass status: %i\n", (int)st));

	return MAD_OK;
}

/*
* Set USGMII Global Reset at all ports.
*/
MAD_STATUS SetUSGMIIGlobalReset
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

	MSG_PRINT(("Set USGMII Global Reset at all ports at port:%i\n", (int)port));

	/*
	*   Set USGMII Global Reset at all ports.
	*/
	if ((status = mdUSGMIIGlobalReset(dev, port)) != MAD_OK)
	{
		MSG_PRINT(("mdUSGMIIGlobalReset return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Enable Scrambler.
*/
MAD_STATUS SetUSGMIIScrambler
(
MAD_DEV         *dev,
MAD_LPORT       port,
MAD_BOOL        en
)
{
	MAD_STATUS status;
	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Enable Scrambler at port:%i\n", (int)port));

	/*
	*   Enable Scrambler.
	*/
	if ((status = mdUSGMIIEnableScrambler(dev, port, en)) != MAD_OK)
	{
		MSG_PRINT(("mdUSGMIIEnableScrambler return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Enable Descrambler.
*/
MAD_STATUS SetUSGMIIDescrambler
(
MAD_DEV         *dev,
MAD_LPORT       port,
MAD_BOOL        en
)
{
	MAD_STATUS status;
	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Enable Descrambler at port:%i\n", (int)port));

	/*
	*   Enable Descrambler.
	*/
	if ((status = mdUSGMIIEnableDescrambler(dev, port, en)) != MAD_OK)
	{
		MSG_PRINT(("mdUSGMIIEnableScrambler return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Set PCH Packet Type.
*/
MAD_STATUS SetUSGMIIPCHPacketType
(
MAD_DEV         *dev,
MAD_LPORT       port,
MAD_USGMII_PKT_TYPE         pky_type
)
{
	MAD_STATUS status;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Set PCH Packet Type at port:%i\n", (int)port));

	/*
	*   Set PCH Packet Type.
	*/
	if ((status = mdUSGMIISetPCHPacketType(dev, port, pky_type)) != MAD_OK)
	{
		MSG_PRINT(("mdUSGMIISetPCHPacketType return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Read PCH Packet Type.
*/
MAD_STATUS ReadUSGMIIPCHPacketType
(
MAD_DEV         *dev,
MAD_LPORT       port
)
{
	MAD_STATUS status;
	MAD_USGMII_PKT_TYPE    pky_type;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Read PCH Packet Type at port:%i\n", (int)port));

	/*
	*   Read PCH Packet Type.
	*/
	if ((status = mdUSGMIIGetPCHPacketType(dev, port, &pky_type)) != MAD_OK)
	{
		MSG_PRINT(("mdUSGMIIGetPCHPacketType return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("The PCH Packet Type: %i\n", (int)pky_type));

	return MAD_OK;
}

/*
* Enable customized port ID.
*/
MAD_STATUS EnableUSGMIIPortID
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

	MSG_PRINT(("Enable customized port ID at port:%i\n", (int)port));

	/*
	*   Enable customized port ID.
	*/
	if ((status = mdUSGMIIEnablePortID(dev, port, 1)) != MAD_OK)
	{
		MSG_PRINT(("mdUSGMIIEnablePortID return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Disable customized port ID.
*/
MAD_STATUS DisableUSGMIIPortID
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

	MSG_PRINT(("Disable customized port ID at port:%i\n", (int)port));

	/*
	*   Enable customized port ID.
	*/
	if ((status = mdUSGMIIEnablePortID(dev, port, 0)) != MAD_OK)
	{
		MSG_PRINT(("mdUSGMIIEnablePortID return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Set customized port ID.
*/
MAD_STATUS SetUSGMIIPortID
(
MAD_DEV         *dev,
MAD_LPORT       port,
MAD_U16         cust_portid
)
{
	MAD_STATUS status;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Set customized port ID at port:%i\n", (int)port));

	/*
	*   Set customized port ID.
	*/
	if ((status = mdUSGMIISetCustPortID(dev, port, cust_portid)) != MAD_OK)
	{
		MSG_PRINT(("mdUSGMIISetCustPortID return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Read customized port ID.
*/
MAD_STATUS ReadUSGMIIPortID
(
MAD_DEV         *dev,
MAD_LPORT       port
)
{
	MAD_STATUS status;
	MAD_U16    cust_portid;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Read customized port ID at port:%i\n", (int)port));

	/*
	*   Read customized port ID.
	*/
	if ((status = mdUSGMIIGetCustPortID(dev, port, &cust_portid)) != MAD_OK)
	{
		MSG_PRINT(("mdUSGMIISetCustPortID return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("The customized port ID: %i\n", (int)cust_portid));

	return MAD_OK;
}

/*
* Set Extention Field Type value for Ingress PCH headers.
*/
MAD_STATUS SetUSGMIIExtFieldType
(
MAD_DEV         *dev,
MAD_LPORT       port,
MAD_USGMII_EXT_FIELD_TYPE         fieldtype
)
{
	MAD_STATUS status;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Set Extention Field Type value for Ingress PCH headers at port:%i\n", (int)port));

	/*
	*   Set Extention Field Type value for Ingress PCH headers.
	*/
	if ((status = mdUSGMIISetExtFieldType(dev, port, fieldtype)) != MAD_OK)
	{
		MSG_PRINT(("mdUSGMIISetExtFieldType return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Read Extention Field Type value for Ingress PCH headers.
*/
MAD_STATUS ReadUSGMIIExtFieldType
(
MAD_DEV         *dev,
MAD_LPORT       port
)
{
	MAD_STATUS status;
	MAD_USGMII_EXT_FIELD_TYPE    fieldtype;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Read Extention Field Type value for Ingress PCH headers at port:%i\n", (int)port));

	/*
	*   Read Extention Field Type value for Ingress PCH headers.
	*/
	if ((status = mdUSGMIIGetExtFieldType(dev, port, &fieldtype)) != MAD_OK)
	{
		MSG_PRINT(("mdUSGMIIGetExtFieldType return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("The Extention Field Type value: %i\n", (int)fieldtype));

	return MAD_OK;
}

/*
* Enable USGMII all Interrupts.
*/
MAD_STATUS SetUSGMIIInterrupt
(
MAD_DEV         *dev,
MAD_LPORT       port,
MAD_U16         intgroup
)
{
	MAD_STATUS status;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Enable USGMII all Interrupts at port:%i\n", (int)port));

	/*
	*   Enable USGMII all Interrupts.
	*/
	if ((status = mdUSGMIIEnableInterrupt(dev, port, intgroup)) != MAD_OK)
	{
		MSG_PRINT(("mdUSGMIIEnableInterrupt return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Read USGMII Interrupt status.
*/
MAD_STATUS ReadUSGMIIInterruptStatus
(
MAD_DEV         *dev,
MAD_LPORT       port
)
{
	MAD_STATUS status;
	MAD_U16    intstatus;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Read USGMII Interrupt status at port:%i\n", (int)port));

	/*
	*   Read USGMII Interrupt status.
	*/
	if ((status = mdUSGMIIGetInterruptStatus(dev, port, &intstatus)) != MAD_OK)
	{
		MSG_PRINT(("mdUSGMIIGetExtFieldType return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("The USGMII Interrupt status: %i\n", (int)intstatus));

	return MAD_OK;
}

/*
* Read sync status.
*/
MAD_STATUS ReadUSGMIISycStatus
(
MAD_DEV         *dev,
MAD_LPORT       port
)
{
	MAD_STATUS status;
	MAD_U16   syncstatus;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Read sync status at port:%i\n", (int)port));

	/*
	*   Read sync status.
	*/
	if ((status = mdUSGMIIGetSyncStatus(dev, port, &syncstatus)) != MAD_OK)
	{
		MSG_PRINT(("mdUSGMIIGetExtFieldType return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("The USGMII sync status: %i\n", (int)((syncstatus & 0x100)>>8)));
	MSG_PRINT(("The Gphy_Port7 sync status: %i\n", (int)((syncstatus & 0x080) >> 7)));
	MSG_PRINT(("The Gphy_Port6 sync status: %i\n", (int)((syncstatus & 0x040) >> 6)));
	MSG_PRINT(("The Gphy_Port5 sync status: %i\n", (int)((syncstatus & 0x020) >> 5)));
	MSG_PRINT(("The Gphy_Port4 sync status: %i\n", (int)((syncstatus & 0x010) >> 4)));
	MSG_PRINT(("The Gphy_Port3 sync status: %i\n", (int)((syncstatus & 0x008) >> 3)));
	MSG_PRINT(("The Gphy_Port2 sync status: %i\n", (int)((syncstatus & 0x004) >> 2)));
	MSG_PRINT(("The Gphy_Port1 sync status: %i\n", (int)((syncstatus & 0x002) >> 1)));
	MSG_PRINT(("The Gphy_Port0 sync status: %i\n", (int)((syncstatus & 0x001))));

	return MAD_OK;
}

/*
* Set the SERDES polarity inversion(RX and TX).
*/
MAD_STATUS SetUSGMIIInvertPolarity
(
MAD_DEV         *dev,
MAD_LPORT       port,
MAD_INVERT_POLARITY    *invertpol
)
{
	MAD_STATUS status;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Set the SERDES polarity inversion(RX and TX) at port:%i\n", (int)port));

	/*
	*   Set the SERDES polarity inversion(RX and TX).
	*/
	if ((status = mdUSGMIISetInvertPolarity(dev, port, invertpol)) != MAD_OK)
	{
		MSG_PRINT(("mdUSGMIISetInvertPolarity return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Read the SERDES polarity inversion(RX and TX).
*/
MAD_STATUS ReadUSGMIIInvertPolarity
(
MAD_DEV         *dev,
MAD_LPORT       port
)
{
	MAD_STATUS status;
	MAD_INVERT_POLARITY    invertpol;
	/* memset(invertpol, 0, sizeof(MAD_INVERT_POLARITY)); */
	MAD_BOOL   invertinp;     
	MAD_BOOL   invertoutp;    

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Read the SERDES polarity inversion(RX and TX) at port:%i\n", (int)port));

	/*
	*   Read the SERDES polarity inversion(RX and TX).
	*/
	if ((status = mdUSGMIIGetInvertPolarity(dev, port, &invertpol)) != MAD_OK)
	{
		MSG_PRINT(("mdUSGMIIGetInvertPolarity return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}
	invertinp = invertpol.invertinp;
	invertoutp = invertpol.invertoutp;

	MSG_PRINT(("The Rx polarity: %i\n", (int)invertinp));
	MSG_PRINT(("The Tx polarity: %i\n", (int)invertoutp));

	return MAD_OK;
}

/*
* Set the USGMII SERDES tap change.
*/
MAD_STATUS SetUSGMIISerdesTapChange
(
MAD_DEV         *dev,
MAD_LPORT       port,
MAD_SERDES_TAP_CHANGE     *change
)
{
	MAD_STATUS status;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Set the USGMII SERDES amplitude change at port:%i\n", (int)port));

	/*
	*   Set the USGMII SERDES amplitude change.
	*/
	if ((status = mdUSGMIISetSerdesTapChange(dev, port, change)) != MAD_OK)
	{
		MSG_PRINT(("mdUSGMIISetSERDESAmpChange return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Read the USGMII SERDES tap change.
*/
MAD_STATUS ReadUSGMIISerdesTapChange
(
MAD_DEV         *dev,
MAD_LPORT       port
)
{
	MAD_STATUS status;
	MAD_SERDES_TAP_CHANGE     change;
	MAD_U16       amplitude;     /*Page240, Reg101, bit10:5 */
	MAD_U16       pre;			 /*Page240, Reg101, bit15:11 */
	MAD_U16       post;		     /*Page240, Reg101, bit4:0 */

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Read the USGMII SERDES amplitude change at port:%i\n", (int)port));

	/*
	*   Read the USGMII SERDES amplitude change.
	*/
	if ((status = mdUSGMIIGetSerdesTapChange(dev, port, &change)) != MAD_OK)
	{
		MSG_PRINT(("mdUSGMIIGetSERDESAmpChange return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}
	amplitude = change.amplitude;
	pre = change.pre;
	post = change.post;

	MSG_PRINT(("The SERDES amplitude change: %i\n", (int)amplitude));
	MSG_PRINT(("The SERDES pre change: %i\n", (int)pre));
	MSG_PRINT(("The SERDES post change: %i\n", (int)post));

	return MAD_OK;
}

/*
* Enable the USGMII PCH.
*/
MAD_STATUS SetUSGMIIPCHEnable
(
MAD_DEV         *dev,
MAD_LPORT       port,
MAD_BOOL		en
)
{
	MAD_STATUS status;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Enable the USGMII PCH at port:%i\n", (int)port));

	/*
	*   Enable the USGMII PCH.
	*/
	if ((status = mdUSGMIIEnablePCH(dev, port, en)) != MAD_OK)
	{
		MSG_PRINT(("mdUSGMIIEnablePCH return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}


/*
* Set USGMII PCH clock with external PTP clock.
*/
MAD_STATUS SetUSGMIIPCHClock
(
MAD_DEV         *dev,
MAD_LPORT       port,
MAD_BOOL		en
)
{
	MAD_STATUS status;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Set USGMII PCH clock with external PTP clock at port:%i\n", (int)port));

	/*
	*   Set USGMII PCH clock with external PTP clock.
	*/
	if ((status = mdUSGMIISetPCHClk(dev, port, en)) != MAD_OK)
	{
		MSG_PRINT(("mdUSGMIISetPCHClk return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Write the PCH time stamping with debug mode.
*/
MAD_STATUS SetUSGMIIDebugETSB
(
MAD_DEV         *dev,
MAD_LPORT       port,
MAD_U16			addr,
MAD_U64			data
)
{
	MAD_STATUS status;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Write the PCH time stamping with debug mode at port:%i\n", (int)port));

	/*
	*   Enable PCH.
	*/
	if ((status = mdUSGMIIEnablePCH(dev, port, 1)) != MAD_OK)
	{
		MSG_PRINT(("mdUSGMIIEnablePCH return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	/*
	*   Write the PCH time stamping with debug mode.
	*/
	if ((status = mdUSGMIIWriteDebugETSB(dev, port, addr, data)) != MAD_OK)
	{
		MSG_PRINT(("mdUSGMIIWriteDebugETSB return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	return MAD_OK;
}

/*
* Read the PCH time stamping with debug mode.
*/
MAD_STATUS ReadUSGMIIDebugETSB
(
MAD_DEV         *dev,
MAD_LPORT       port,
MAD_U16			addr
)
{
	MAD_STATUS status;
	MAD_U64	   data;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Read the PCH time stamping with debug mode at port:%i\n", (int)port));

	/*
	*   Enable PCH.
	*/
	if ((status = mdUSGMIIEnablePCH(dev, port, 1)) != MAD_OK)
	{
		MSG_PRINT(("mdUSGMIIEnablePCH return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	/*
	*   Read the PCH time stamping with debug mode.
	*/
	if ((status = mdUSGMIIReadDebugETSB(dev, port, addr, &data)) != MAD_OK)
	{
		MSG_PRINT(("mdUSGMIIReadDebugETSB return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("The PCH Time Stamping with debug reading: 0X%llx \n", (MAD_U64)data));

	return MAD_OK;
}

/*
* Read the PCH time stamping with normal mode.
*/
MAD_STATUS ReadUSGMIINormalETSB
(
MAD_DEV         *dev,
MAD_LPORT       port,
MAD_U16			addr
)
{
	MAD_STATUS status;
	MAD_U64	   data;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Read the PCH time stamping with normal mode at port:%i\n", (int)port));

	/*
	*   Enable PCH.
	*/
	if ((status = mdUSGMIIEnablePCH(dev, port, 1)) != MAD_OK)
	{
		MSG_PRINT(("mdUSGMIIEnablePCH return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	/*
	*   Read the PCH time stamping with normal mode.
	*/
	if ((status = mdUSGMIIReadETSB(dev, port, addr, &data)) != MAD_OK)
	{
		MSG_PRINT(("mdUSGMIIReadETSB return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("The PCH Time Stamping with normal reading: 0X%llx\n", (MAD_U64)data));

	return MAD_OK;
}

/*
* Read the height and width of eye monitor.
*/
MAD_STATUS ReadUSGMIIEyeMonitor
(
MAD_DEV         *dev,
MAD_LPORT       port,
MAD_U16			sample
)
{
	MAD_STATUS status;
	MAD_U32    height;
	MAD_U32    width;

	if (dev == 0)
	{
		MSG_PRINT(("MAD driver is not initialized\n"));
		return MAD_FAIL;
	}

	MSG_PRINT(("Read the height and width of eye monitor at port:%i\n", (int)port));

	/*
	*  Read the height and width of eye monitor.
	*/
	if ((status = mdUSGMIIGetEyeMonitor(dev, port, sample, &height, &width)) != MAD_OK)
	{
		MSG_PRINT(("mdUSGMIIGetEyeMonitor return Failed\n"));
		testMADDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("The height of eye monitor: %imV\n", (int)height));
	MSG_PRINT(("The width of eye monitor: %imUI\n", (int)width));

	return MAD_OK;
}
