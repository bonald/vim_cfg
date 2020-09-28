#include <madCopyright.h>

/********************************************************************************
* 88E1680_Stats.c
*
* DESCRIPTION:
*       APIs for 88E1680 Statistics, different with other products.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include "88E1680.h"

/********************************************************************************
* mode - 0  Disable/reset CRC checker
*		 2  Check data from copper interface
*		 6  Check data from QSGMII interface
*
*******************************************************************************/

MAD_STATUS E1680_SetCRCCheck(IN  MAD_DEV *dev, IN  MAD_LPORT port, IN  MAD_U8 mode)
{
	MAD_STATUS  retVal;

	if ((retVal = madHwSetPagedPhyRegField(
		dev, port, 18, MAD_REG_CHECK_CONTROL_1,
		0, 3, mode)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to paged phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}

	return MAD_OK;
}

MAD_STATUS E1680_GetCRCCheck(IN  MAD_DEV *dev, IN  MAD_LPORT port, OUT  MAD_U16 *data)
{
	MAD_STATUS  retVal;

	if ((retVal = madHwGetPagedPhyRegField(
		dev, port, 18, MAD_REG_CHECK_CONTROL_1,
		0, 3, data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Reading to paged phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}

	return MAD_OK;
}

MAD_STATUS E1680_GetCRCCounter(IN  MAD_DEV *dev, IN  MAD_LPORT port, IN MAD_U16 offset, OUT  MAD_U16 *data)
{
	MAD_STATUS  retVal;

	if ((retVal = madHwGetPagedPhyRegField(
		dev, port, 18, MAD_REG_CRC_COUNTERS,
		(MAD_U8)(offset & 0xff), 8, data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to paged phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}

	return MAD_OK;
}

MAD_STATUS E1680_GetRxErCapture(IN  MAD_DEV *dev, IN  MAD_LPORT port, IN  MAD_U8 path, OUT MAD_BOOL *capture)
{
	MAD_STATUS  retVal;
	MAD_U8      page,reg;
	MAD_U16     data;

	if (path)
	{
		page = MAD_PAGE4;
		reg = MAD_REG_NAC_RX_ER_BYTE_CAPT;
	}
	else
	{
		page = MAD_PAGE18;
		reg = MAD_REG_Copper_RX_ER_BYTE_CAPT;
	}
	/* Get the required bits in CRC Counter Register */
	if ((retVal = madHwGetPagedPhyRegField(dev, port, page, reg,15, 1, &data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Reading from paged phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_GET_PAGED_REG_FIELD);
	}

	/* Enable or disable */
	MAD_BIT_2_BOOL(data, *capture);

	return MAD_OK;
}

MAD_STATUS E1680_GetRxErByteNun(IN  MAD_DEV *dev, IN  MAD_LPORT port, IN  MAD_U8 path, OUT MAD_U16 *byteNum)
{
	MAD_STATUS  retVal;
	MAD_U8      page, reg;
	MAD_U16     data;

	if (path)
	{
		page = MAD_PAGE4;
		reg = MAD_REG_NAC_RX_ER_BYTE_CAPT;
	}
	else
	{
		page = MAD_PAGE18;
		reg = MAD_REG_Copper_RX_ER_BYTE_CAPT;
	}
	/* Get the required bits in CRC Counter Register */
	if ((retVal = madHwGetPagedPhyRegField(dev, port, page, reg, 12, 2, &data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Reading from paged phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_GET_PAGED_REG_FIELD);
	}

	*byteNum = data;

	return MAD_OK;
}

MAD_STATUS E1680_GetRxEr(IN  MAD_DEV *dev, IN  MAD_LPORT port, IN  MAD_U8 path, OUT MAD_U8 *rxEr)
{
	MAD_STATUS  retVal;
	MAD_U8      page, reg;
	MAD_U16     data;

	if (path)
	{
		page = MAD_PAGE4;
		reg = MAD_REG_NAC_RX_ER_BYTE_CAPT;
	}
	else
	{
		page = MAD_PAGE18;
		reg = MAD_REG_Copper_RX_ER_BYTE_CAPT;
	}
	/* Get the required bits in CRC Counter Register */
	if ((retVal = madHwGetPagedPhyRegField(dev, port, page, reg,9, 1, &data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Reading from paged phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_GET_PAGED_REG_FIELD);
	}

	*rxEr = data & 0xff;

	return MAD_OK;
}

MAD_STATUS E1680_GetRxDv(IN  MAD_DEV *dev, IN  MAD_LPORT port, IN  MAD_U8 path, OUT MAD_BOOL *rxDv)
{
	MAD_STATUS  retVal;
	MAD_U8      page, reg;
	MAD_U16     data;

	if (path)
	{
		page = MAD_PAGE4;
		reg = MAD_REG_NAC_RX_ER_BYTE_CAPT;
	}
	else
	{
		page = MAD_PAGE18;
		reg = MAD_REG_Copper_RX_ER_BYTE_CAPT;
	}
	/* Get the required bits in CRC Counter Register */
	if ((retVal = madHwGetPagedPhyRegField(dev, port, page, reg, 8, 1, &data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Reading from paged phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_GET_PAGED_REG_FIELD);
	}

	/* Enable or disable */
	MAD_BIT_2_BOOL(data, *rxDv);

	return MAD_OK;
}

MAD_STATUS E1680_GetRxData(IN  MAD_DEV *dev, IN  MAD_LPORT port, IN  MAD_U8 path, OUT MAD_U8 *rxData)
{
	MAD_STATUS  retVal;
	MAD_U8      page, reg;
	MAD_U16     data;

	if (path)
	{
		page = MAD_PAGE4;
		reg = MAD_REG_NAC_RX_ER_BYTE_CAPT;
	}
	else
	{
		page = MAD_PAGE18;
		reg = MAD_REG_Copper_RX_ER_BYTE_CAPT;
	}
	/* Get the required bits in CRC Counter Register */
	if ((retVal = madHwGetPagedPhyRegField(dev, port, page, reg, 0, 8, &data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Reading from paged phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_GET_PAGED_REG_FIELD);
	}

	*rxData = data & 0xff;

	return MAD_OK;
}

MAD_STATUS E1680_GetLinkDisconnectCounter(IN  MAD_DEV *dev, IN  MAD_LPORT port, OUT  MAD_U16 *count)
{
	MAD_STATUS  retVal;

	if ((retVal = madHwGetPagedPhyRegField(dev, port, MAD_PAGE18, MAD_REG_LATE_COLIS_WIN_ADJ, 0, 8, count))
		!= MAD_OK)
	{
		MAD_DBG_ERROR(("Reading phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}

	return MAD_OK;
}