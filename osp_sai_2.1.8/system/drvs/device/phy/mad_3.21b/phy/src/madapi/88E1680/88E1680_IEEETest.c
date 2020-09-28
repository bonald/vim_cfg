#include <madCopyright.h>

/********************************************************************************
* 88E1680_IEEETest.c
*
* DESCRIPTION:
*       APIs for 88E1680 IEEE, different with other products.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include "88E1680.h"

MAD_STATUS E1680_Enable_TX_TCLK(IN  MAD_DEV *dev,IN  MAD_LPORT port)
{
	MAD_STATUS  retVal;

	if ((retVal = madHwSetPagedPhyRegField(dev, port, 6, 26, 15, 1, 0x1)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing from paged phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}
	if ((retVal = madHwSetPagedPhyRegField(dev, port, 4, 27, 8, 1, 0x0)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing from paged phy reg failed.\n"));
		return (retVal | MAD_API_FAIL_SET_PAGED_REG_FIELD);
	}

	return MAD_OK;
}