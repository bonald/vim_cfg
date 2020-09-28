#include <madCopyright.h>
/********************************************************************************
* 88E1780_Vod.c
*
* DESCRIPTION:
*       APIs for 88E1780 Vod, different with other products.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include "88E1780.h" 

MAD_STATUS E1780_SetCopperVod(IN MAD_DEV *dev, IN MAD_LPORT port, IN MAD_SPEED_MODE mode, IN MAD_U16 data)
{
	MAD_STATUS  retVal;
	MAD_U8     addr;
	MAD_U16		data1;

	if ((retVal = madHwReadPagedPhyReg(dev, port, 0xfc, 17, &data1)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to paged phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_WRITE_PAGED_REG);
	}
	/* Select speed and Loopback Mode */
	switch (mode)
	{
	case MAD_SPEED_10M:
		addr = 17;
		data |= (data << 4);
		data = (data1 & 0xFF00) | data;
		break;
	case MAD_SPEED_100M:
		addr = 17;
		data |= (data << 4);
		data <<= 8;
		data = (data1 & 0x00FF) | data;
		break;
	case MAD_SPEED_1000M:
		addr = 18;
		data |= (data << 4);
		data |= (data << 8);
		data |= (data << 12);
		break;
	default:
		MAD_DBG_ERROR(("Invalid speed\n"));
		return MAD_API_UNKNOWN_SPEED_MODE;
	}

	if ((retVal = madHwWritePagedPhyReg(dev, port, 0xfc, addr, data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to paged phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_WRITE_PAGED_REG);
	}

	return MAD_OK;
}