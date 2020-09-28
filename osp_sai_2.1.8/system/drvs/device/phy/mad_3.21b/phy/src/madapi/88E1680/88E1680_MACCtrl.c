#include <madCopyright.h>

/********************************************************************************
* 88E1680_MACCtrl.c
*
* DESCRIPTION:
*       APIs for 88E1680 MAC, different with other products.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include "88E1680.h"

MAD_STATUS E1680_MacSetFifoDepth
(
	IN  MAD_DEV   *dev, 
	IN  MAD_LPORT  port, 
	IN  MAD_U8     trFifoDepth,
	IN  MAD_U8     reFifoDepth
)
{
	MAD_STATUS  retVal;

	/* Set Transmit FIFO */
	if ((retVal = madHwSetPagedPhyRegField(dev, port, 2, MAD_REG_MAC_CONTROL_1, 14, 2, trFifoDepth)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to paged phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}

	/* Set Receive FIFO, QSGMII Transmit FIFO page4_reg16_bit15:bit14 as MAC Receive FIFO */
	if ((retVal = madHwSetPagedPhyRegField(dev, port, 4, MAD_REG_QSGMII_CONTROL_1, 14, 2, reFifoDepth)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to paged phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}

	return MAD_OK;
}


MAD_STATUS E1680_MacGetFifoDepth
(
	IN  MAD_DEV   *dev,
	IN  MAD_LPORT  port,
	OUT  MAD_U8     *trFifoDepth,
	OUT  MAD_U8     *reFifoDepth
)
{
	MAD_U16     data;
	MAD_STATUS  retVal;

	/* Get Transmit FIFO */
	if ((retVal = madHwGetPagedPhyRegField(
		dev, port, 2, MAD_REG_MAC_CONTROL_1,
		14, 2, &data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to paged phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}
	*trFifoDepth = data & 0x3;

	/* Get Receive FIFO, QSGMII Transmit FIFO page4_reg16_bit15:bit14 as MAC Receive FIFO */
	if ((retVal = madHwGetPagedPhyRegField(
		dev, port, 4, MAD_REG_QSGMII_CONTROL_1,
		14, 2, &data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to paged phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}
	*reFifoDepth = data & 0x3;

	return MAD_OK;
}