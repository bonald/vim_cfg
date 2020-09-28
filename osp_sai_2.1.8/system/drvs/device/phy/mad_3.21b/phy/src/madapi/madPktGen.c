#include <madCopyright.h>

/********************************************************************************
* madPktGen.c
* 
* DESCRIPTION:
*       APIs for Packet Generator.
*
* DEPENDENCIES:
*       madCpStatus.c.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include <madApi.h>
#include <madApiInternal.h>
#include <madHwCntl.h>
#include <madDrvPhyRegs.h>

/*
    Packet Generator for the device using register 16 page 6 (1340)
*/
static
MAD_STATUS madPktGen_1340
(
    IN  MAD_DEV   *dev,
    IN  MAD_U8    hwPort,
    IN  MAD_U32  en,
    IN  MAD_PG    *pktInfo
)
{
    MAD_STATUS  retVal;
    MAD_U8      offset,len;
    MAD_BOOL    link;
    MAD_U16     data, en_type;

    en_type = 0;
    if (en)
    {
        data = MAD_BIT_3;

        /* Get the current link */
        if((retVal = mdGetLinkStatus(dev,MAD_PORT_2_LPORT(hwPort),&link)) != MAD_OK)
        {
            MAD_DBG_ERROR(("mdGetLinkStatus failed.\n"));
            return (retVal | MAD_API_FAIL_GET_LINK_ST);
        }

        if(!link)
        {
            MAD_DBG_INFO(("Link should be established before this call\n"));
            return MAD_API_LINK_DOWN;
        }

        if (pktInfo->payload == MAD_PG_PAYLOAD_5AA5)
            data |= MAD_BIT_2;

        if (pktInfo->length == MAD_PG_LENGTH_1518)
            data |= MAD_BIT_1;

        if (pktInfo->tx == MAD_PG_TX_ERROR)
            data |= MAD_BIT_0;

        if(MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR))
           en_type = pktInfo->en_type;

        offset = 0;
        len = 4;
    }
    else
    {
        data = 0;
        offset = 3;
        len = 1;
        en_type = 0;
    }

    /* Set Packet Generation config */
    if((retVal = madHwSetPagedPhyRegField(
                    dev,hwPort,6,MAD_REG_PACKET_GENERATION,
                    offset,len,data)) != MAD_OK)
    {
        MAD_DBG_ERROR(("Writing to paged phy reg failed.\n"));
        return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
    }

    if(MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR))
    {
      /* Set Packet Generation enble type */
      if((retVal = madHwSetPagedPhyRegField(
                    dev,hwPort,6,MAD_REG_PACKET_GENERATION,
                    5,3,en_type)) != MAD_OK)
      {
        MAD_DBG_ERROR(("Writing to paged phy reg failed.\n"));
        return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
      }
    }

    return MAD_OK;
}

/*
    Packet Generator for the device using register 16 page 6 (1181)
*/
static
MAD_STATUS madPktGen_16_6
(
    IN  MAD_DEV   *dev,
    IN  MAD_U8    hwPort,
    IN  MAD_U32  en,
    IN  MAD_PG    *pktInfo
)
{
    MAD_STATUS  retVal;
    MAD_U8      offset,len;
    MAD_BOOL    link;
    MAD_U16     data, en_type;

    en_type = 0;
    if (en)
    {
        data = MAD_BIT_3;

        /* Get the current link */
        if((retVal = mdGetLinkStatus(dev,MAD_PORT_2_LPORT(hwPort),&link)) != MAD_OK)
        {
            MAD_DBG_ERROR(("mdGetLinkStatus failed.\n"));
            return (retVal | MAD_API_FAIL_GET_LINK_ST);
        }

        if(!link)
        {
            MAD_DBG_INFO(("Link should be established before this call\n"));
            return MAD_API_LINK_DOWN;
        }

        if (pktInfo->payload == MAD_PG_PAYLOAD_5AA5)
            data |= MAD_BIT_2;

        if (pktInfo->length == MAD_PG_LENGTH_1518)
            data |= MAD_BIT_1;

        if (pktInfo->tx == MAD_PG_TX_ERROR)
            data |= MAD_BIT_0;

        if(MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR))
           en_type = pktInfo->en_type;

        offset = 0;
        len = 4;
    }
    else
    {
        data = 0;
        offset = 3;
        len = 1;
        en_type = 0;
    }

    /* Set Packet Generation config */
    if((retVal = madHwSetPagedPhyRegField(
                    dev,hwPort,6,MAD_REG_PACKET_GENERATION,
                    offset,len,data)) != MAD_OK)
    {
        MAD_DBG_ERROR(("Writing to paged phy reg failed.\n"));
        return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
    }

    if(MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR))
    {
      /* Set Packet Generation enble type */
      if((retVal = madHwSetPagedPhyRegField(
                    dev,hwPort,6,MAD_REG_PACKET_GENERATION,
                    5,3,en_type)) != MAD_OK)
      {
        MAD_DBG_ERROR(("Writing to paged phy reg failed.\n"));
        return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
      }
    }

    return MAD_OK;
}


/*
    Packet Generator for the device using register 30 page 18  (114X)
*/
static
MAD_STATUS madPktGen_30_18
(
    IN  MAD_DEV   *dev,
    IN  MAD_U8    hwPort,
    IN  MAD_U32  en,
    IN  MAD_PG    *pktInfo
)
{
    MAD_STATUS  retVal;
    MAD_U8      offset,len;
    MAD_BOOL    link;
    MAD_U16     data;

    if (en)
    {
        data = MAD_BIT_3;

        /* Get the current link */
        if((retVal = mdGetLinkStatus(dev,MAD_PORT_2_LPORT(hwPort),&link)) != MAD_OK)
        {
            MAD_DBG_ERROR(("mdGetLinkStatus failed.\n"));
            return (retVal | MAD_API_FAIL_GET_LINK_ST);
        }

        if(!link)
        {
            MAD_DBG_INFO(("Link should be established before this call\n"));
            return MAD_API_LINK_DOWN;
        }

        if (pktInfo->payload == MAD_PG_PAYLOAD_5AA5)
            data |= MAD_BIT_2;

        if (pktInfo->length == MAD_PG_LENGTH_1518)
            data |= MAD_BIT_1;

        if (pktInfo->tx == MAD_PG_TX_ERROR)
            data |= MAD_BIT_0;
        offset = 2;
        len = 4;
    }
    else
    {
        data = 0;
        offset = 5;
        len = 1;
    }
    /* Set Packet Generation config */
    if((retVal = madHwSetPagedPhyRegField(
                    dev,hwPort,18,30,
                    offset,len,data)) != MAD_OK)
    {
        MAD_DBG_ERROR(("Writing to paged phy reg failed.\n"));
        return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
    }

    return MAD_OK;
}


/*
Packet Generator for the device using register 16 page 18 (1780)
*/
static
MAD_STATUS madPktGen_1780
(
IN  MAD_DEV   *dev,
IN  MAD_U8    hwPort,
IN  MAD_U16  crc_check,
IN  MAD_PG_1780    *pktInfo
)
{
	MAD_STATUS  retVal;
	MAD_BOOL    link;
	MAD_U16     data;

	data = 0;
	/* Get the current link */
	if ((retVal = mdGetLinkStatus(dev, MAD_PORT_2_LPORT(hwPort), &link)) != MAD_OK)
	{
		MAD_DBG_ERROR(("mdGetLinkStatus failed.\n"));
		return (retVal | MAD_API_FAIL_GET_LINK_ST);
	}

	if (!link)
	{
		MAD_DBG_INFO(("Link should be established before this call\n"));
		return MAD_API_LINK_DOWN;
	}
	
	if (pktInfo->burst != 0)
		data |= (pktInfo->burst & 0xff) << 8;

	if (pktInfo->clear == MAD_PG_CLEAR_STAY)
		data |= MAD_BIT_3;

	if (pktInfo->payload == MAD_PG_PAYLOAD_5AA5)
		data |= MAD_BIT_2;

	if (pktInfo->length == MAD_PG_LENGTH_1518)
		data |= MAD_BIT_1;

	if (pktInfo->tx == MAD_PG_TX_ERROR)
		data |= MAD_BIT_0;

	if (MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR))
		data |= pktInfo->en_type << 5;

	
	/* Disable Packet Generation type */
	if ((retVal = madHwSetPagedPhyRegField(
		dev, hwPort, MAD_PAGE18, MAD_REG_PACKET_GENERATION_1,
		5, 3, 0)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}

	/* Set Packet Generation config */
	if ((retVal = madHwWritePagedPhyReg(
		dev, hwPort, MAD_PAGE18, MAD_REG_PACKET_GENERATION_1, data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to paged phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}

	if (MAD_DEV_CAPABILITY(dev, MAD_PHY_FULL_PAGE_ADDR))
	{
		/* Set Packet Generation CRC Checker type */
		if ((retVal = madHwSetPagedPhyRegField(
			dev, hwPort, MAD_PAGE18, MAD_REG_CHECK_CONTROL_1,
			0, 3, crc_check)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to phy reg failed.\n"));
			return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
		}
	}

	return MAD_OK;
}

/*
Packet Check for the device using reg 16¡¢reg 17¡¢reg 18 at page 18 (1780)
*/
static
MAD_STATUS madPktCount_1780
(
IN  MAD_DEV   *dev,
IN  MAD_U8    hwPort,
IN  MAD_PG_1780    *pktInfo,
OUT MAD_PG_COUNT_1780   *pktcount
)
{
	MAD_STATUS  retVal;
	MAD_BOOL    link;
	MAD_U16     crc_check, data;

	/* Get the current link */
	if ((retVal = mdGetLinkStatus(dev, MAD_PORT_2_LPORT(hwPort), &link)) != MAD_OK)
	{
		MAD_DBG_ERROR(("mdGetLinkStatus failed.\n"));
		return (retVal | MAD_API_FAIL_GET_LINK_ST);
	}

	if (!link)
	{
		MAD_DBG_INFO(("Link should be established before this call\n"));
		return MAD_API_LINK_DOWN;
	}

	/* Get Packet Check config */
	if ((retVal = madHwGetPagedPhyRegField(
		dev, hwPort, MAD_PAGE18, MAD_REG_CHECK_CONTROL_1,
		0, 3, &crc_check)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to paged phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}

	if (!crc_check)
	{
		MAD_DBG_INFO(("Packet Check was disabled\n"));
		return MAD_FAIL;
	}

	if ((retVal = madHwReadPagedPhyReg(dev, hwPort, MAD_PAGE18,
		MAD_REG_PACKET_GENERATION_1, &data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Paged Phy Register read failed.\n"));
		return (retVal | MAD_API_FAIL_READ_PAGED_REG);
	}

	pktcount->crc_count = data & 0xf;
	pktcount->packet_count = (data & 0xf0) >> 8;

	/* Set Packet Check config */
	if ((retVal = madHwSetPagedPhyRegField(
		dev, hwPort, MAD_PAGE18, MAD_REG_CHECK_CONTROL_1,
		4, 1, pktInfo->trigger)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to paged phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}

	return MAD_OK;
}

/*
Packet Generator for the device using register 16 page 6 (1780)
*/
static
MAD_STATUS madCopperPktGen_1780
(
IN  MAD_DEV   *dev,
IN  MAD_U8    hwPort,
IN  MAD_U32  en,
IN  MAD_COPPER_PG_1780    *pktInfo
)
{
	MAD_STATUS  retVal;
	MAD_U8      offset, len;
	MAD_BOOL    link;
	MAD_U16     data;

	data = 0;
	if (en)
	{
		data |= MAD_BIT_3;
		/* Get the current link */
		if ((retVal = mdGetLinkStatus(dev, MAD_PORT_2_LPORT(hwPort), &link)) != MAD_OK)
		{
			MAD_DBG_ERROR(("mdGetLinkStatus failed.\n"));
			return (retVal | MAD_API_FAIL_GET_LINK_ST);
		}

		if (!link)
		{
			MAD_DBG_INFO(("Link should be established before this call\n"));
			return MAD_API_LINK_DOWN;
		}
		if (pktInfo->burst != 0)
			data |= (pktInfo->burst & 0xff) << 8;

		if (pktInfo->check == MAD_PG_CHECKER_ENABLE)
			data |= MAD_BIT_4;

		if (pktInfo->payload == MAD_PG_PAYLOAD_5AA5)
			data |= MAD_BIT_2;

		if (pktInfo->length == MAD_PG_LENGTH_1518)
			data |= MAD_BIT_1;

		if (pktInfo->tx == MAD_PG_TX_ERROR)
			data |= MAD_BIT_0;

		offset = 0;
		len = 16;
	}
	else
	{
		data = 0;
		offset = 3;
		len = 1;
	}

	/* Set Packet Generation config */
	if ((retVal = madHwSetPagedPhyRegField(
		dev, hwPort, 6, MAD_REG_PACKET_GENERATION,
		offset, len, data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to paged phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}

	return MAD_OK;
}

/*******************************************************************************
* mdDiagSetPktGenEnable
*
* DESCRIPTION:
*       This routine enables or disables Packet Generator.
*       Link should be established first prior to enabling the packet generator,
*       and generator will generate packets at the speed of the established link.
*        When enables packet generator, the following information should be 
*       provided:
*           Payload Type:  either Random or 5AA55AA5
*           Packet Length: either 64 or 1514 bytes
*           Error Packet:  either Error packet or normal packet
*
* INPUTS:
*       dev     - MAD driver structure returned from mdLoadDriver
*       port    - The logical port number
*       en      - 1 to enable, 0 to disable
*       pktInfo - packet information(MAD_PG structure pointer), if en is MAD_TRUE
*                 ignored, if en is MAD_FALSE
*   
* OUTPUTS:
*       None.
*
* RETURNS:
*       MAD_OK   - on success
*       MAD_FAIL - on error
*
* COMMENTS:
*       Page 0, Reg 17, Bit 10  : Copper Link
*       Page 6, Reg 16, Bit 3:0 : Packet Generation Register
*        Various hidden registers are accessed. (114X)
*
*******************************************************************************/
MAD_STATUS mdDiagSetPktGenEnable
(
    IN  MAD_DEV   *dev,
    IN  MAD_LPORT port,
    IN  MAD_U32   en,
    IN  MAD_PG    *pktInfo
)
{
    MAD_U8      hwPort;

    MAD_DBG_INFO(("mdDiagSetPktGenEnable called.\n"));

    /* translate LPORT to hardware port */
    hwPort = MAD_LPORT_2_PORT(port);
    if(!(MAD_DEV_PORT_CHECK(dev, hwPort)))
    {
      MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
      return MAD_API_ERROR_OVER_PORT_RANGE;
    }

    if(!MAD_DEV_CAPABILITY(dev, MAD_PHY_PKT_GENERATOR))
    {
        MAD_DBG_ERROR(("Package generator not supported.\n"));
        return MAD_API_PKG_GEN_NOT_SUPPORT;
    }

    switch(dev->phyInfo.pktGenType)
    {
        case MAD_PHY_PKTGEN_TYPE2:
            return madPktGen_16_6(dev,hwPort,en,pktInfo);

        case MAD_PHY_PKTGEN_TYPE1:
            return madPktGen_30_18(dev,hwPort,en,pktInfo);

        case MAD_PHY_PKTGEN_TYPE4:   /* need for 1340 */
            return madPktGen_1340(dev,hwPort,en,pktInfo);

        case MAD_PHY_PKTGEN_TYPE3:
        default:
            return MAD_API_UNKNOWN_PKG_GEN_TYPE;
    }

    return MAD_OK;
}

/*******************************************************************************
* mdDiagSetAltPktGenEnable
*
* DESCRIPTION:
*       This routine enables or disables Packet Generator.
*       Link should be established first prior to enabling the packet generator,
*       and generator will generate packets at the speed of the established link.
*        When enables packet generator, the following information should be
*       provided:
*			Transmit trigger:  either transmitting or done
*			Self Clear:        either self clear or stay
*           Payload Type:      either Random or 5AA55AA5
*           Packet Length:     either 64 or 1514 bytes
*           Error Packet:      either Error packet or normal packet
*
* INPUTS:
*       dev     - MAD driver structure returned from mdLoadDriver
*       port    - The logical port number
*       en      - 1 to enable, 0 to disable
*		MAD_U16 - crc_check:
*				  0 = Disable Packet Check
*				  2 = Enable Packet Check from copper
*				  6 = Enable Packet Check from usgmii
*       pktInfo - packet information(MAD_PG structure pointer), if en is MAD_TRUE
*                 ignored, if en is MAD_FALSE
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MAD_OK   - on success
*       MAD_FAIL - on error
*
* COMMENTS:
*       Page 0, Reg 17, Bit 10  : Copper Link
*       Page 18, Reg 16, Bit 3:0 : Packet Generation Register
*        Various hidden registers are accessed. (1780)
*
*******************************************************************************/
MAD_STATUS mdDiagSetAltPktGenEnable
(
IN  MAD_DEV   *dev,
IN  MAD_LPORT port,
IN  MAD_U16   crc_check,
IN  MAD_PG_1780    *pktInfo
)
{
	MAD_U8      hwPort;

	MAD_DBG_INFO(("mdDiagSetAltPktGenEnable called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	if (!MAD_DEV_CAPABILITY(dev, MAD_PHY_PKT_GENERATOR))
	{
		MAD_DBG_ERROR(("Package generator not supported.\n"));
		return MAD_API_PKG_GEN_NOT_SUPPORT;
	}

	switch (dev->phyInfo.pktGenType)
	{
		case MAD_PHY_PKTGEN_TYPE5:
			return madPktGen_1780(dev, hwPort, crc_check, pktInfo);

		default:
			return MAD_API_UNKNOWN_PKG_GEN_TYPE;
	}

	return MAD_OK;
}

/*******************************************************************************
* mdDiagGetAltPktGenCount
*
* DESCRIPTION:
*       This routine enables or disables Packet Generator.
*       Link should be established first prior to enabling the packet generator,
*       and generator will generate packets at the speed of the established link.
*        When enables packet generator, the following information should be
*       provided:
*			Transmit trigger:       either transmitting or done
*			Packet Generator type:  000 = normal Operation
*									010 = Generate Packets on Copper Interface
*									110 = Generate Packets on USGMII Interface
*			Packet Count:           0x00 = no packets received
*									0xFF = 256 packets received (max count)
*			CRC Error Count:		0x00 = no CRC errors detected in the packets received
*									0xFF = 256 CRC errors detected in the packets received (max count)
*
*
* INPUTS:
*       dev     - MAD driver structure returned from mdLoadDriver
*       port    - The logical port number
*       pktInfo - packet information(MAD_PG structure pointer), if en is MAD_TRUE
*                 ignored, if en is MAD_FALSE
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MAD_OK   - on success
*       MAD_FAIL - on error
*
* COMMENTS:
*       Page 0, Reg 17, Bit 10  : Copper Link
*       Page 18, Reg 16, Bit 3:0 : Packet Generation Register
*        Various hidden registers are accessed. (1780)
*
*******************************************************************************/
MAD_STATUS mdDiagGetAltPktGenCount
(
IN  MAD_DEV   *dev,
IN  MAD_LPORT port,
IN  MAD_PG_1780    *pktInfo,
OUT MAD_PG_COUNT_1780   *pktcount
)
{
	MAD_U8      hwPort;

	MAD_DBG_INFO(("mdDiagGetAltPktGenCount called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	if (!MAD_DEV_CAPABILITY(dev, MAD_PHY_PKT_GENERATOR))
	{
		MAD_DBG_ERROR(("Package generator not supported.\n"));
		return MAD_API_PKG_GEN_NOT_SUPPORT;
	}

	switch (dev->phyInfo.pktGenType)
	{
	case MAD_PHY_PKTGEN_TYPE5:
		return madPktCount_1780(dev, hwPort, pktInfo, pktcount);

	default:
		return MAD_API_UNKNOWN_PKG_GEN_TYPE;
	}
	
	return MAD_OK;
}

/*******************************************************************************
* mdDiagSetCopperPktGenEnable
*
* DESCRIPTION:
*       This routine enables or disables Packet Generator.
*       Link should be established first prior to enabling the packet generator,
*       and generator will generate packets at the speed of the established link.
*        When enables packet generator, the following information should be
*       provided:
*           Payload Type:  either Random or 5AA55AA5
*           Packet Length: either 64 or 1514 bytes
*           Error Packet:  either Error packet or normal packet
*
* INPUTS:
*       dev     - MAD driver structure returned from mdLoadDriver
*       port    - The logical port number
*       en      - 1 to enable, 0 to disable
*       pktInfo - packet information(MAD_PG structure pointer), if en is MAD_TRUE
*                 ignored, if en is MAD_FALSE
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MAD_OK   - on success
*       MAD_FAIL - on error
*
* COMMENTS:
*       Page 0, Reg 17, Bit 10  : Copper Link
*       Page 6, Reg 16, Bit 3:0 : Packet Generation Register
*
*******************************************************************************/
MAD_STATUS mdDiagSetCopperPktGenEnable
(
IN  MAD_DEV   *dev,
IN  MAD_LPORT port,
IN  MAD_U32   en,
IN  MAD_COPPER_PG_1780    *pktInfo
)
{
	MAD_U8      hwPort;

	MAD_DBG_INFO(("mdDiagSetCopperPktGenEnable called.\n"));

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	if (!MAD_DEV_CAPABILITY(dev, MAD_PHY_PKT_GENERATOR))
	{
		MAD_DBG_ERROR(("Package generator not supported.\n"));
		return MAD_API_PKG_GEN_NOT_SUPPORT;
	}

	switch (dev->phyInfo.pktGenType)
	{
	case MAD_PHY_PKTGEN_TYPE5:
		return madCopperPktGen_1780(dev, hwPort, en, pktInfo);

	default:
		return MAD_API_UNKNOWN_PKG_GEN_TYPE;
	}

	return MAD_OK;
}
