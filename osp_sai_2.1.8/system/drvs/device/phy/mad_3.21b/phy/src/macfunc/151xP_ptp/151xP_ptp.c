#include <madCopyright.h>

/*******************************************************************************
* 151xP_ptp.c
*
* DESCRIPTION:
*       API definitions for Precise Time Protocol for 151xP device
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*******************************************************************************/
#include <madApi.h>
#include <madApiInternal.h>
#include <madSem.h>
#include <madHwCntl.h>
#include <madDrvPhyRegs.h>
#include <151xP_ptp.h>

#define PTP_READPLUS_SUPPORT(page,reg)       (page==MAD_PAGE15)||(page==MAD_PAGE12&&\
	reg >= 0xE)

/*
* Typedef: enum PTP_OPERATION
*
* Description: Defines the PTP (Precise Time Protocol) Operation type
*
* Fields:
*      PTP_WRITE_DATA             - Write data to the PTP register
*      PTP_READ_DATA            - Read data from PTP register
*      PTP_READ_MULTIPLE_DATA    - Read multiple data from PTP register
*      PTP_READ_TIMESTAMP_DATA    - Read timestamp data from PTP register
*                    valid bit will be reset after read
*/
typedef enum
{
	PTP_WRITE_DATA = 0x3,
	PTP_READ_DATA = 0x4,
	PTP_READ_MULTIPLE_DATA = 0x6,
	PTP_READ_TIMESTAMP_DATA = 0x8,
} PTP_OPERATION;

/*
*  typedef: struct PTP_OP_DATA
*
*  Description: data required by PTP Operation
*
*  Fields:
*      ptpPort        - physical port of the device, Except 0xE for TAI 0xF for global
*      ptpBlock     - block of addresses
*      ptpPhyPort     - MAD Phy port number
*      ptpAddr     - register address
*      ptpData     - data for ptp register.
*      ptpMultiData- used for multiple read operation.
*      nData         - number of data to be read on multiple read operation.
*/
typedef struct
{
	MAD_U32    ptpPort;
	MAD_U32    ptpBlock;
	MAD_U32    ptpPhyPort;
	MAD_U32    ptpAddr;
	MAD_U32    ptpData;
	MAD_U32    ptpMultiData[MAX_PTP_CONSECUTIVE_READ];
	MAD_U32    nData;
} PTP_OP_DATA;

/****************************************************************************/
/* Forward function declaration.                                            */
/****************************************************************************/
static MAD_STATUS ptpReadplus
(
	IN   MAD_DEV    *dev,
	IN   MAD_U8     port,
	IN   MAD_U16    page,
	IN   MAD_U8     reg,
	IN	 MAD_BOOL	multi,
	INOUT PTP_OP_DATA        *opData
);
static MAD_STATUS mapRegs
(
	PTP_OP_DATA		*opData,
	MAD_U8			*port,
	MAD_U16			*page,
	MAD_U8			*reg
);
static MAD_STATUS ptpOperation
(
	IN    MAD_DEV             *dev,
	IN    PTP_OPERATION        ptpOp,
	INOUT PTP_OP_DATA         *opData
);
static MAD_STATUS ptpGetReg
(
	IN  MAD_DEV     *dev,
	IN  MAD_LPORT    port,
	IN  MAD_U8    ptpPort,
	IN  MAD_U32        regOffset,
	OUT MAD_U32        *data
);
static MAD_STATUS ptpSetReg
(
	IN  MAD_DEV     *dev,
	IN  MAD_LPORT    port,
	IN  MAD_U8    ptpPort,
	IN  MAD_U32        regOffset,
	IN  MAD_U32        data
);
static MAD_STATUS ptpGlobalPointDataSet
(
	IN  MAD_DEV         *dev,
	IN  MAD_LPORT		  port,
	IN  MAD_U8             point,
	IN  MAD_U8             data
);
static MAD_STATUS ptpGlobalPointDataGet
(
	IN  MAD_DEV         *dev,
	IN  MAD_LPORT		   port,
	IN  MAD_U8             point,
	OUT  MAD_U8            *data
);
static MAD_STATUS ptpTimeArrayStore
(
	IN    MAD_DEV                         *dev,
	IN    MAD_LPORT                       port,
	IN    MAD_U8                          timeArrayIndex,
	IN    PTP_TOD_STORE_OPERATION		storeOp,
	IN    PTP_TIME_ARRAY				*timeArray
);
static MAD_STATUS ptpTimeArrayCapture
(
	IN    MAD_DEV              *dev,
	IN    MAD_LPORT            port,
	IN    MAD_U8               timeArrayIndex,
	OUT   PTP_TIME_ARRAY	   *timeArray
);

/****************************************************************************/
/* Exported 151xP PTP Functions		 			                            */
/****************************************************************************/
/*******************************************************************************
* ptpSetEnable
*
* DESCRIPTION:
*       This routine Power up/down the PTP module
*
* INPUTS:
*       port - The logical port number
*       en    - 1: Power up PTP. 0: Power down PTP.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MAD_OK      - on success
*       MAD_[ERROR_STATUS]    - on error
*       MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
MAD_STATUS ptpSetEnable
(
	IN  MAD_DEV     *dev,
	IN  MAD_LPORT    port,
	IN  MAD_BOOL     en
)
{
	MAD_STATUS	retVal;
	MAD_U16     data;
	MAD_U16     page;
	MAD_U8      reg;
	MAD_U8      hwPort;         /* the physical port number     */

	MAD_DBG_INFO(("ptpSetEnable Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}
	if (en == MAD_TRUE)
	{
		data = 0;
	}
	else
	{
		data = 1;
	}
	page = MAD_PAGE6;
	reg = MAD_REG_GENERAL_CONTROL;
	if ((retVal = madHwSetPagedPhyRegField(dev, hwPort, page, reg, 9, 1, data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to paged phy reg (%i,%i) failed.\n", page, reg));
		return (retVal | MAD_API_FAIL_WRITE_PAGED_REG);
	}

	return MAD_OK;
}

/*******************************************************************************
* ptpGetEnable
*
* DESCRIPTION:
*       This routine Get port PTP Power up/down status.
*
* INPUTS:
*       port - The logical port number
*
* OUTPUTS:
*       en    - 1: PTP module Power up. 0: PTP module Power down.
*
* RETURNS:
*       MAD_OK      - on success
*       MAD_[ERROR_STATUS]    - on error
*       MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
MAD_STATUS ptpGetEnable
(
	IN  MAD_DEV      *dev,
	IN  MAD_LPORT     port,
	OUT  MAD_BOOL    *en
)
{
	MAD_STATUS	retVal;
	MAD_U16     data;
	MAD_U16     page;
	MAD_U8      reg;
	MAD_U8      hwPort;         /* the physical port number     */

	MAD_DBG_INFO(("ptpGetEnable Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}
	page = MAD_PAGE6;
	reg = MAD_REG_GENERAL_CONTROL;
	if ((retVal = madHwGetPagedPhyRegField(dev, hwPort, page, reg, 9, 1, &data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to paged phy reg (%i,%i) failed.\n", page, reg));
		return (retVal | MAD_API_FAIL_WRITE_PAGED_REG);
	}

	if (data)
	{
		*en = MAD_FALSE;
	}
	else
	{
		*en = MAD_TRUE;
	}
	return MAD_OK;
}

/*******************************************************************************
* ptpSetRefClkSrc
*
* DESCRIPTION:
*       This routine sets port PTP reference clock source.
*
* INPUTS:
*       port - The logical port number
*       rClk   - 1: use 125MHz clock supplied from config pin.
*				 0: use internal clock
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MAD_OK      - on success
*       MAD_[ERROR_STATUS]    - on error
*       MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
MAD_STATUS ptpSetRefClkSrc
(
	IN  MAD_DEV     *dev,
	IN  MAD_LPORT    port,
	IN  MAD_BOOL     rClk
)
{
	MAD_STATUS	retVal;
	MAD_U16     data;
	MAD_U16     page;
	MAD_U8      reg;
	MAD_U8      hwPort;         /* the physical port number     */

	MAD_DBG_INFO(("ptpSetRefClkSrc Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	if (rClk == MAD_TRUE)
	{
		data = 1;
	}
	else
	{
		data = 0;
	}
	page = MAD_PAGE6;
	reg = MAD_REG_GENERAL_CONTROL;
	if ((retVal = madHwSetPagedPhyRegField(dev, hwPort, page, reg, 8, 1, data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to paged phy reg (%i,%i) failed.\n", page, reg));
		return (retVal | MAD_API_FAIL_WRITE_PAGED_REG);
	}
	return MAD_OK;
}

/*******************************************************************************
* ptpGetRefClkSrc
*
* DESCRIPTION:
*       This routine gets port PTP reference clock source.
*
* INPUTS:
*       port - The logical port number
*
* OUTPUTS:
*		rClk   - 1: use 125MHz clock supplied from config pin.
*			   - 0: use internal clock
*
* RETURNS:
*       MAD_OK      - on success
*       MAD_[ERROR_STATUS]    - on error
*       MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
MAD_STATUS ptpGetRefClkSrc
(
	IN  MAD_DEV     *dev,
	IN  MAD_LPORT    port,
	OUT MAD_BOOL    *rClk
)
{
	MAD_STATUS	retVal;
	MAD_U16     data;
	MAD_U16     page;
	MAD_U8      reg;
	MAD_U8      hwPort;         /* the physical port number     */

	MAD_DBG_INFO(("ptpGetRefClkSrc Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	page = MAD_PAGE6;
	reg = MAD_REG_GENERAL_CONTROL;
	if ((retVal = madHwGetPagedPhyRegField(dev, hwPort, page, reg, 8, 1, &data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Reading paged phy reg (%i,%i) failed.\n", page, reg));
		return (retVal | MAD_API_FAIL_WRITE_PAGED_REG);
	}

	if (data)
	{
		*rClk = MAD_TRUE;
	}
	else
	{
		*rClk = MAD_FALSE;
	}
	return MAD_OK;
}

/*******************************************************************************
* ptpSetInputSrc
*
* DESCRIPTION:
*       This routine sets port PTP input source.
*
* INPUTS:
*       port - The logical port number
*       inSrc  - 1: use LED1 for PTP event request input.
*				 0: force input to 0.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MAD_OK      - on success
*       MAD_[ERROR_STATUS]    - on error
*       MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
MAD_STATUS ptpSetInputSrc
(
	IN  MAD_DEV     *dev,
	IN  MAD_LPORT   port,
	IN  MAD_BOOL    inSrc
)
{
	MAD_STATUS	retVal;
	MAD_U16     data;
	MAD_U16     page;
	MAD_U8      reg;
	MAD_U8      hwPort;         /* the physical port number     */

	MAD_DBG_INFO(("ptpSetInputSrc Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	if (inSrc == MAD_TRUE)
	{
		data = 1;
	}
	else
	{
		data = 0;
	}
	page = MAD_PAGE6;
	reg = MAD_REG_GENERAL_CONTROL;
	if ((retVal = madHwSetPagedPhyRegField(dev, hwPort, page, reg, 7, 1, data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to paged phy reg (%i,%i) failed.\n", page, reg));
		return (retVal | MAD_API_FAIL_WRITE_PAGED_REG);
	}
	return MAD_OK;
}

/*******************************************************************************
* ptpGetInputSrc
*
* DESCRIPTION:
*       This routine gets port PTP input source.
*
* INPUTS:
*       port - The logical port number
*
* OUTPUTS:
*       inSrc  - 1: use LED1 for PTP event request input.
*				 0: force input to 0.
*
* RETURNS:
*       MAD_OK      - on success
*       MAD_[ERROR_STATUS]    - on error
*       MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
MAD_STATUS ptpGetInputSrc
(
	IN  MAD_DEV     *dev,
	IN  MAD_LPORT   port,
	OUT MAD_BOOL    *inSrc
)
{
	MAD_STATUS	retVal;
	MAD_U16     data;
	MAD_U16     page;
	MAD_U8      reg;
	MAD_U8      hwPort;         /* the physical port number     */

	MAD_DBG_INFO(("ptpGetInputSrc Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	page = MAD_PAGE6;
	reg = MAD_REG_GENERAL_CONTROL;
	if ((retVal = madHwGetPagedPhyRegField(dev, hwPort, page, reg, 7, 1, &data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Reading paged phy reg (%i,%i) failed.\n", page, reg));
		return (retVal | MAD_API_FAIL_WRITE_PAGED_REG);
	}

	if (data)
	{
		*inSrc = MAD_TRUE;
	}
	else
	{
		*inSrc = MAD_FALSE;
	}
	return MAD_OK;
}


/*******************************************************************************
* ptpSetOutputSrc
*
* DESCRIPTION:
*       This routine sets port PTP output source.
*
* INPUTS:
*       port - The logical port number
*       outSrc  - 1: use LED1 for PTP trigger generate output.
*				  0: force output to 0.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MAD_OK      - on success
*       MAD_[ERROR_STATUS]    - on error
*       MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
MAD_STATUS ptpSetOutputSrc
(
	IN  MAD_DEV     *dev,
	IN  MAD_LPORT   port,
	IN  MAD_BOOL    outSrc
)
{
	MAD_STATUS	retVal;
	MAD_U16     data;
	MAD_U16     page;
	MAD_U8      reg;
	MAD_U8      hwPort;         /* the physical port number     */

	MAD_DBG_INFO(("ptpSetOutputSrc Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	if (outSrc == MAD_TRUE)
	{
		data = 1;
	}
	else
	{
		data = 0;
	}
	page = MAD_PAGE6;
	reg = MAD_REG_GENERAL_CONTROL;
	if ((retVal = madHwSetPagedPhyRegField(dev, hwPort, page, reg, 6, 1, data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to paged phy reg (%i,%i) failed.\n", page, reg));
		return (retVal | MAD_API_FAIL_WRITE_PAGED_REG);
	}
	return MAD_OK;
}

/*******************************************************************************
* ptpGetOutputSrc
*
* DESCRIPTION:
*       This routine gets port PTP output source.
*
* INPUTS:
*       port - The logical port number
*
* OUTPUTS:
*       outSrc  - 1: use LED1 for PTP trigger generate output.
*				  0: force output to 0.
*
* RETURNS:
*       MAD_OK      - on success
*       MAD_[ERROR_STATUS]    - on error
*       MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
MAD_STATUS ptpGetOutputSrc
(
	IN  MAD_DEV     *dev,
	IN  MAD_LPORT   port,
	OUT MAD_BOOL    *outSrc
)
{
	MAD_STATUS	retVal;
	MAD_U16     data;
	MAD_U16     page;
	MAD_U8      reg;
	MAD_U8      hwPort;         /* the physical port number     */

	MAD_DBG_INFO(("ptpGetOutputSrc Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	page = MAD_PAGE6;
	reg = MAD_REG_GENERAL_CONTROL;
	if ((retVal = madHwGetPagedPhyRegField(dev, hwPort, page, reg, 6, 1, &data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Reading paged phy reg (%i,%i) failed.\n", page, reg));
		return (retVal | MAD_API_FAIL_WRITE_PAGED_REG);
	}

	if (data)
	{
		*outSrc = MAD_TRUE;
	}
	else
	{
		*outSrc = MAD_FALSE;
	}
	return MAD_OK;
}

/*******************************************************************************
* ptpSet1PPSOutputSrc
*
* DESCRIPTION:
*       This routine sets port PTP 1PPS output source.
*
* INPUTS:
*       port - The logical port number
*       outSrc  - 1: Use LED0 for PTP One Pulse Per Second(1PPS) output.
*				  0: Use LED0 for other functions.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MAD_OK      - on success
*       MAD_[ERROR_STATUS]    - on error
*       MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
MAD_STATUS ptpSet1PPSOutputSrc
(
	IN  MAD_DEV     *dev,
	IN  MAD_LPORT    port,
	IN  MAD_BOOL     outSrc
)
{
	MAD_STATUS	retVal;
	MAD_U16     data;
	MAD_U16     page;
	MAD_U8      reg;
	MAD_U8      hwPort;         /* the physical port number     */

	MAD_DBG_INFO(("ptpSetOutputSrc Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	if (outSrc == MAD_TRUE)
	{
		data = 1;
	}
	else
	{
		data = 0;
	}
	page = MAD_PAGE6;
	reg = MAD_REG_GENERAL_CONTROL;
	if ((retVal = madHwSetPagedPhyRegField(dev, hwPort, page, reg, 5, 1, data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to paged phy reg (%i,%i) failed.\n", page, reg));
		return (retVal | MAD_API_FAIL_WRITE_PAGED_REG);
	}
	return MAD_OK;
}

/*******************************************************************************
* ptpGet1PPSOutputSrc
*
* DESCRIPTION:
*       This routine gets port PTP 1PPS output source.
*
* INPUTS:
*       port - The logical port number
*
* OUTPUTS:
*       outSrc  - 1: Use LED0 for PTP One Pulse Per Second(1PPS) output.
*				  0: Use LED0 for other functions.
*
* RETURNS:
*       MAD_OK      - on success
*       MAD_[ERROR_STATUS]    - on error
*       MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
MAD_STATUS ptpGet1PPSOutputSrc
(
	IN  MAD_DEV     *dev,
	IN  MAD_LPORT   port,
	OUT MAD_BOOL    *outSrc
)
{
	MAD_STATUS	retVal;
	MAD_U16     data;
	MAD_U16     page;
	MAD_U8      reg;
	MAD_U8      hwPort;         /* the physical port number     */

	MAD_DBG_INFO(("ptpGetOutputSrc Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	page = MAD_PAGE6;
	reg = MAD_REG_GENERAL_CONTROL;
	if ((retVal = madHwGetPagedPhyRegField(dev, hwPort, page, reg, 5, 1, &data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Reading paged phy reg (%i,%i) failed.\n", page, reg));
		return (retVal | MAD_API_FAIL_WRITE_PAGED_REG);
	}

	if (data)
	{
		*outSrc = MAD_TRUE;
	}
	else
	{
		*outSrc = MAD_FALSE;
	}
	return MAD_OK;
}


/*******************************************************************************
* ptpSetIntEnable
*
* DESCRIPTION:
*       This routine set port PTP interrupt Enable/Disable.
*
* INPUTS:
*       port           - The logical port number
*       interruptEn    - Port PTP interrupt Enable/Disable
*                        ptpArrIntEn      - Port Arrival Interrupt enable     MAD_FALSE:Disable MAD_TRUE:Enable
*                        ptpDepIntEn      - Port Departure Interrupt enable   MAD_FALSE:Disable MAD_TRUE:Enable
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MAD_OK      - on success
*       MAD_[ERROR_STATUS]    - on error
*       MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
MAD_STATUS ptpSetIntEnable
(
	IN  MAD_DEV     *dev,
	IN  MAD_LPORT   port,
	IN  PTP_PORT_INTERRUPT_EN    *interruptEn
)
{
	MAD_STATUS	retVal;
	MAD_U16     data;
	MAD_U16     page;
	MAD_U8      reg;
	MAD_U8      hwPort;         /* the physical port number     */

	MAD_DBG_INFO(("ptpSetIntEnable Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}
	data = (interruptEn->ptpArrIntEn ? 1 : 0) | ((interruptEn->ptpDepIntEn ? 1 : 0) << 1);
	page = MAD_PAGE8;
	reg = MAD_REG_PTP_PORT_CONFIG_2;
	if ((retVal = madHwSetPagedPhyRegField(dev, hwPort, page, reg, 0, 2, data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to paged phy reg (%i,%i) failed.\n", page, reg));
		return (retVal | MAD_API_FAIL_WRITE_PAGED_REG);
	}
	return MAD_OK;
}

/*******************************************************************************
* ptpGetIntEnable
*
* DESCRIPTION:
*       This routine get port PTP Interrupt Enable/Disable.
*
* INPUTS:
*       port - The logical port number
*
* OUTPUTS:
*       interruptEn    - Port PTP interrupt Enable/Disable
*                        ptpArrIntEn      - Port Arrival Interrupt enable     MAD_FALSE:Disable MAD_TRUE:Enable
*                        ptpDepIntEn      - Port Departure Interrupt enable   MAD_FALSE:Disable MAD_TRUE:Enable
*
* RETURNS:
*       MAD_OK      - on success
*       MAD_[ERROR_STATUS]    - on error
*       MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
MAD_STATUS ptpGetIntEnable
(
	IN  MAD_DEV     *dev,
	IN  MAD_LPORT    port,
	OUT PTP_PORT_INTERRUPT_EN    *interruptEn
)
{
	MAD_STATUS	retVal;
	MAD_U16     data;
	MAD_U16     page;
	MAD_U8      reg;
	MAD_U8      hwPort;         /* the physical port number     */

	MAD_DBG_INFO(("ptpGetIntEnable Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}
	page = MAD_PAGE8;
	reg = MAD_REG_PTP_PORT_CONFIG_2;
	if ((retVal = madHwGetPagedPhyRegField(dev, hwPort, page, reg, 0, 2, &data)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Reading paged phy reg (%i,%i) failed.\n", page, reg));
		return (retVal | MAD_API_FAIL_WRITE_PAGED_REG);
	}

	interruptEn->ptpDepIntEn = ((data >> 1) & 0x1) ? MAD_TRUE : MAD_FALSE;
	interruptEn->ptpArrIntEn = (data & 0x1) ? MAD_TRUE : MAD_FALSE;

	return MAD_OK;
}

/*******************************************************************************
* ptpSetPortPTPEn
*
* DESCRIPTION:
*       This routine enables or disables PTP on a port.
*
* INPUTS:
*        en - MAD_TRUE to enable PTP, MAD_FALSE to disable PTP
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MAD_OK      - on success
*       MAD_[ERROR_STATUS]    - on error
*       MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
MAD_STATUS ptpSetPortPTPEn
(
	IN  MAD_DEV     *dev,
	IN  MAD_LPORT    port,
	IN  MAD_BOOL        en
)
{
	MAD_STATUS           retVal;
	PTP_OPERATION    op;
	PTP_OP_DATA        opData;
	MAD_U8            hwPort;

	MAD_DBG_INFO(("ptpSetPortPTPEn Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	opData.ptpBlock = 0x0;    /* PTP register space */
	opData.ptpPhyPort = hwPort;    /* port number */
	opData.ptpAddr = 0;

	opData.ptpPort = hwPort;

	op = PTP_READ_DATA;
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading DisPTP.\n"));
		return MAD_FAIL;
	}

	op = PTP_WRITE_DATA;

	if (en)
		opData.ptpData &= ~0x1;
	else
		opData.ptpData |= 0x1;

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing TransSpec,DisTSpecCheck,DisTSOverwrite.\n"));
		return MAD_FAIL;
	}

	MAD_DBG_INFO(("OK.\n"));
	return MAD_OK;
}

/*******************************************************************************
* ptpGetPortPTPEn
*
* DESCRIPTION:
*       This routine checks if PTP is enabled on a port.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*        en - MAD_TRUE if enabled, MAD_FALSE otherwise
*
* RETURNS:
*       MAD_OK      - on success
*       MAD_[ERROR_STATUS]    - on error
*       MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
MAD_STATUS ptpGetPortPTPEn
(
	IN  MAD_DEV     *dev,
	IN  MAD_LPORT    port,
	OUT MAD_BOOL        *en
)
{
	MAD_STATUS           retVal;
	PTP_OPERATION    op;
	PTP_OP_DATA        opData;
	MAD_U8            hwPort;

	MAD_DBG_INFO(("ptpGetPortPTPEn Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	opData.ptpBlock = 0x0;    /* PTP register space */
	opData.ptpPhyPort = hwPort;    /* port number */
	opData.ptpAddr = 0;
	opData.ptpPort = hwPort;

	op = PTP_READ_DATA;

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading DisPTP.\n"));
		return MAD_FAIL;
	}

	*en = (opData.ptpData & 0x1) ? MAD_FALSE : MAD_TRUE;

	MAD_DBG_INFO(("OK.\n"));
	return MAD_OK;
}

/*******************************************************************************
* ptpSetPortConfig
*
* DESCRIPTION:
*       This routine writes PTP port configuration parameters.
*
* INPUTS:
*        ptpData  - PTP port configuration parameters.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MAD_OK      - on success
*       MAD_[ERROR_STATUS]    - on error
*       MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
MAD_STATUS ptpSetPortConfig
(
	IN  MAD_DEV     *dev,
	IN  MAD_LPORT    port,
	IN  PTP_PORT_CONFIG    *ptpData
)
{
	MAD_STATUS           retVal;
	PTP_OPERATION    op;
	PTP_OP_DATA        opData;
	MAD_U8          hwPort;         /* the physical port number     */

	MAD_DBG_INFO(("ptpSetPortConfig Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	if (ptpData->transSpec > 0xF)    /* 4 bits */
		return MAD_API_PTP_BAD_PARAM;

	if (ptpData->etJump > 0x1F)    /* 5 bits */
		return MAD_API_PTP_BAD_PARAM;

	if (ptpData->ipJump > 0x3F)    /* 6 bits */
		return MAD_API_PTP_BAD_PARAM;

	opData.ptpBlock = 0x0;    /* PTP register space */
	opData.ptpPhyPort = hwPort;    /* port number */

	opData.ptpPort = hwPort;

	/* TransSpec, DisTSpecCheck, DisTSOverwrite bit, offset 0 */
	op = PTP_READ_DATA;
	opData.ptpAddr = 0;

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading DisPTP.\n"));
		return MAD_FAIL;
	}

	op = PTP_WRITE_DATA;
	opData.ptpData = (ptpData->transSpec << 12) | (opData.ptpData & 0x1) |
		((ptpData->disTSpec ? 1 : 0) << 11) |
		((ptpData->disTSOverwrite ? 1 : 0) << 1);

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing TransSpec,DisTSpecCheck,DisTSOverwrite.\n"));
		return MAD_FAIL;
	}

	/* setting etJump and ipJump, offset 1 */
	opData.ptpAddr = 1;
	opData.ptpData = (ptpData->ipJump << 8) | ptpData->etJump;

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing MsgIDTSEn.\n"));
		return MAD_FAIL;
	}

	/* setting Int, offset 2 */
	/*Read back first*/
	opData.ptpAddr = 2;
	op = PTP_READ_DATA;
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading TSArrPtr.\n"));
		return MAD_FAIL;
	}
	opData.ptpData &= 0x1c;
	opData.ptpData |= (ptpData->ptpArrIntEn ? 1 : 0) |
		((ptpData->ptpDepIntEn ? 1 : 0) << 1);

	opData.ptpData |= ((ptpData->arrTSMode & 0xff) << 8);
	opData.ptpData |= ((ptpData->filterAct ? 1 : 0) << 7); /* Filter 802.1 Act from LEDs */
	opData.ptpData |= ((ptpData->hwAccPtp ? 1 : 0) << 6);  /* Hardware Accelerated PTP */
	opData.ptpData |= ((ptpData->kpFramwSa ? 1 : 0) << 5); /* KS = Keep Frame SA */


	op = PTP_WRITE_DATA;
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing TSArrPtr.\n"));
		return MAD_FAIL;
	}

	/* setting Led control, offset 3 */
	opData.ptpAddr = 3;
	opData.ptpData = (ptpData->arrLedCtrl << 8) |
		(ptpData->depLedCtrl);

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing LED control.\n"));
		return MAD_FAIL;
	}

	MAD_DBG_INFO(("OK.\n"));
	return MAD_OK;
}


/*******************************************************************************
* ptpGetPortConfig
*
* DESCRIPTION:
*       This routine reads PTP configuration parameters for a port.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*        ptpData  - PTP port configuration parameters.
*
* RETURNS:
*       MAD_OK      - on success
*       MAD_[ERROR_STATUS]    - on error
*       MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
MAD_STATUS ptpGetPortConfig
(
	IN  MAD_DEV     *dev,
	IN  MAD_LPORT    port,
	OUT PTP_PORT_CONFIG    *ptpData
)
{
	MAD_STATUS           retVal;
	PTP_OPERATION    op;
	PTP_OP_DATA        opData;
	MAD_U8          hwPort;         /* the physical port number     */

	MAD_DBG_INFO(("ptpGetPortConfig Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	opData.ptpBlock = 0x0;    /* PTP register space */
	opData.ptpPhyPort = hwPort;    /* port number */

	opData.ptpPort = hwPort;
	op = PTP_READ_DATA;

	/* TransSpec, DisTSpecCheck, DisTSOverwrite bit, offset 0 */
	opData.ptpAddr = 0;
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading PTPEType.\n"));
		return MAD_FAIL;
	}

	ptpData->transSpec = opData.ptpData >> 12;
	ptpData->disTSpec = ((opData.ptpData >> 11) & 0x1) ? MAD_TRUE : MAD_FALSE;
	ptpData->disTSOverwrite = ((opData.ptpData >> 1) & 0x1) ? MAD_TRUE : MAD_FALSE;

	/* getting MsgIDTSEn, offset 1 */
	opData.ptpAddr = 1;
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading MsgIDTSEn.\n"));
		return MAD_FAIL;
	}

	ptpData->ipJump = (opData.ptpData >> 8) & 0x3F;
	ptpData->etJump = opData.ptpData & 0x1F;

	/* getting TSArrPtr, offset 2 */
	opData.ptpAddr = 2;
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading TSArrPtr.\n"));
		return MAD_FAIL;
	}

	ptpData->ptpDepIntEn = ((opData.ptpData >> 1) & 0x1) ? MAD_TRUE : MAD_FALSE;
	ptpData->ptpArrIntEn = (opData.ptpData & 0x1) ? MAD_TRUE : MAD_FALSE;
	ptpData->arrTSMode = (opData.ptpData & 0xff00) >> 8;
	ptpData->filterAct = ((opData.ptpData >> 7) & 0x1) ? MAD_TRUE : MAD_FALSE;
	ptpData->hwAccPtp = ((opData.ptpData >> 6) & 0x1) ? MAD_TRUE : MAD_FALSE;
	ptpData->kpFramwSa = ((opData.ptpData >> 5) & 0x1) ? MAD_TRUE : MAD_FALSE;

	/* getting Led control, offset 3 */
	opData.ptpAddr = 3;
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading LED control.\n"));
	}

	ptpData->arrLedCtrl = madLong2Char((opData.ptpData & 0xff00) >> 8);
	ptpData->depLedCtrl = madLong2Char(opData.ptpData & 0xff);

	MAD_DBG_INFO(("OK.\n"));
	return MAD_OK;
}

/*******************************************************************************
* ptpSetMeanPathDelay
*
* DESCRIPTION:
*		This routine sets ingress mean path delay on a port.
*
* INPUTS:
*       port - The logical port number
*		delay	- cable delay between this port and its link partner in unsiged
*				  nano seconds
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		MAD_OK      - on success
*       MAD_[ERROR_STATUS]    - on error
*       MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
MAD_STATUS ptpSetMeanPathDelay
(
	IN   MAD_DEV	*dev,
	IN   MAD_LPORT	port,
	IN   MAD_U16	delay
)
{
	MAD_STATUS           retVal;
	PTP_OPERATION    op;
	PTP_OP_DATA      opData;
	MAD_U8				 hwPort;

	MAD_DBG_INFO(("ptpSetMeanPathDelay Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	opData.ptpBlock = 0;
	opData.ptpAddr = 0x1C;
	opData.ptpPort = hwPort;
	opData.ptpPhyPort = hwPort;    /* port number */
	opData.ptpData = delay;
	op = PTP_WRITE_DATA;

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing mean path delay.\n"));
		return MAD_FAIL;
	}

	MAD_DBG_INFO(("OK.\n"));
	return MAD_OK;
}

/*******************************************************************************
* ptpGetMeanPathDelay
*
* DESCRIPTION:
*		This routine gets ingress mean path delay on a port.
*
* INPUTS:
*       port - The logical port number
*
* OUTPUTS:
*		delay	- cable delay between this port and its link partner in unsiged
*				  nano seconds
*
* RETURNS:
*		MAD_OK      - on success
*       MAD_[ERROR_STATUS]    - on error
*       MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/

MAD_STATUS ptpGetMeanPathDelay
(
	IN   MAD_DEV	*dev,
	IN   MAD_LPORT	port,
	OUT  MAD_U16	*delay
)
{
	MAD_STATUS           retVal;
	PTP_OPERATION    op;
	PTP_OP_DATA      opData;
	MAD_U8				 hwPort;

	MAD_DBG_INFO(("ptpGetMeanPathDelay Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}


	opData.ptpBlock = 0;
	opData.ptpAddr = 0x1C;
	opData.ptpPort = hwPort;
	opData.ptpPhyPort = hwPort;    /* port number */
	op = PTP_READ_DATA;

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading mean path delay.\n"));
		return MAD_FAIL;
	}

	*delay = madLong2Short(opData.ptpData);

	MAD_DBG_INFO(("OK.\n"));
	return MAD_OK;
}

/*******************************************************************************
* ptpSetIgrPathDelayAsym
*
* DESCRIPTION:
*		This routine sets ingress path delay asymmetry on a port.
*
* INPUTS:
*       port - The logical port number
*		asym	- bit 15 is path delay asymmetry sign
*						0 - the path delay asymmetry number is added
*						1 - the path delay asymmetry number is subtracted
*
*				  bit 14:0 indicates the assymmetry value beyond the mean path delay
*				  that needs to be added or subtracted for more accurate cable
*				  measurements.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		MAD_OK      - on success
*       MAD_[ERROR_STATUS]    - on error
*       MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
MAD_STATUS ptpSetIgrPathDelayAsym
(
	IN   MAD_DEV	*dev,
	IN   MAD_LPORT	port,
	IN   MAD_U16	asym
)
{
	MAD_STATUS           retVal;
	PTP_OPERATION    op;
	PTP_OP_DATA      opData;
	MAD_U8				 hwPort;

	MAD_DBG_INFO(("ptpSetIgrPathDelayAsym Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	opData.ptpBlock = 0;    /* PTP register space */
	opData.ptpAddr = 0x1D;
	opData.ptpPort = hwPort;
	opData.ptpPhyPort = hwPort;    /* port number */
	opData.ptpData = asym;
	op = PTP_WRITE_DATA;

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing ingress path delay asymmetry.\n"));
		return MAD_FAIL;
	}

	MAD_DBG_INFO(("OK.\n"));
	return MAD_OK;
}

/*******************************************************************************
* ptpGetIgrPathDelayAsym
*
* DESCRIPTION:
*		This routine gets ingress path delay asymmetry on a port.
*
* INPUTS:
*       port - The logical port number
*
* OUTPUTS:
*		asym	- bit 15 is path delay asymmetry sign
*						0 - the path delay asymmetry number is added
*						1 - the path delay asymmetry number is subtracted
*
*				  bit 14:0 indicates the assymmetry value beyond the mean path delay
*				  that needs to be added or subtracted for more accurate cable
*				  measurements.
*
* RETURNS:
*		MAD_OK      - on success
*       MAD_[ERROR_STATUS]    - on error
*       MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
MAD_STATUS ptpGetIgrPathDelayAsym
(
	IN   MAD_DEV	*dev,
	IN   MAD_LPORT	port,
	OUT  MAD_U16	*asym
)
{
	MAD_STATUS           retVal;
	PTP_OPERATION    op;
	PTP_OP_DATA      opData;
	MAD_U8				 hwPort;

	MAD_DBG_INFO(("ptpGetIgrPathDelayAsym Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	opData.ptpBlock = 0;    /* PTP register space */
	opData.ptpAddr = 0x1D;
	opData.ptpPort = hwPort;
	opData.ptpPhyPort = hwPort;    /* port number */
	op = PTP_READ_DATA;

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading ingress path delay asymmetry.\n"));
		return MAD_FAIL;
	}

	*asym = madLong2Short(opData.ptpData);

	MAD_DBG_INFO(("OK.\n"));
	return MAD_OK;
}

/*******************************************************************************
* ptpSetEgrPathDelayAsym
*
* DESCRIPTION:
*		This routine sets egress path delay asymmetry on a port.
*
* INPUTS:
*       port - The logical port number
*		asym	- bit 15 is path delay asymmetry sign
*						0 - the path delay asymmetry number is added
*						1 - the path delay asymmetry number is subtracted
*
*				  bit 14:0 indicates the assymmetry value beyond the mean path delay
*				  that needs to be added or subtracted for more accurate cable
*				  measurements.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		MAD_OK      - on success
*       MAD_[ERROR_STATUS]    - on error
*       MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
MAD_STATUS ptpSetEgrPathDelayAsym
(
	IN   MAD_DEV	*dev,
	IN   MAD_LPORT	port,
	IN   MAD_U16	asym
)
{
	MAD_STATUS           retVal;
	PTP_OPERATION    op;
	PTP_OP_DATA      opData;
	MAD_U8				 hwPort;

	MAD_DBG_INFO(("ptpSetEgrPathDelayAsym Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	opData.ptpBlock = 0;    /* PTP register space */
	opData.ptpAddr = 0x1E;
	opData.ptpPort = hwPort;
	opData.ptpPhyPort = hwPort;    /* port number */
	opData.ptpData = asym;
	op = PTP_WRITE_DATA;

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writeg egress path delay asymmetry.\n"));
		return MAD_FAIL;
	}

	MAD_DBG_INFO(("OK.\n"));
	return MAD_OK;
}

/*******************************************************************************
* ptpGetEgrPathDelayAsym
*
* DESCRIPTION:
*		This routine gets egress path delay asymmetry on a port.
*
* INPUTS:
*       port - The logical port number
*
* OUTPUTS:
*		asym	- bit 15 is path delay asymmetry sign
*						0 - the path delay asymmetry number is added
*						1 - the path delay asymmetry number is subtracted
*
*				  bit 14:0 indicates the assymmetry value beyond the mean path delay
*				  that needs to be added or subtracted for more accurate cable
*				  measurements.
*
* RETURNS:
*		MAD_OK      - on success
*       MAD_[ERROR_STATUS]    - on error
*       MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
MAD_STATUS ptpGetEgrPathDelayAsym
(
	IN   MAD_DEV	*dev,
	IN   MAD_LPORT	port,
	OUT  MAD_U16	*asym
)
{
	MAD_STATUS           retVal;
	PTP_OPERATION    op;
	PTP_OP_DATA      opData;
	MAD_U8				 hwPort;

	MAD_DBG_INFO(("ptpGetEgrPathDelayAsym Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	opData.ptpBlock = 0;    /* PTP register space */
	opData.ptpAddr = 0x1E;
	opData.ptpPort = hwPort;
	opData.ptpPhyPort = hwPort;    /* port number */
	op = PTP_READ_DATA;

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed readeg egress path delay asymmetry.\n"));
		return MAD_FAIL;
	}

	*asym = madLong2Short(opData.ptpData);

	MAD_DBG_INFO(("OK.\n"));
	return MAD_OK;
}

/*******************************************************************************
* ptpGetTimeStamped
*
* DESCRIPTION:
*        This routine retrieves the PTP port status that includes time stamp value
*        and sequce Id that are captured by PTP logic for a PTP frame that needs
*        to be time stamped.
*
* INPUTS:
*       port - logical port number.
*       timeToRead    - Arr0, Arr1, or Dep time (PTP_TIME enum type)
*
* OUTPUTS:
*        ptpStatus    - PTP port status
*
* RETURNS:
*       MAD_OK         - on success
*       MAD_[ERROR_STATUS]     - on error
*       MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
MAD_STATUS ptpGetTimeStamped
(
	IN  MAD_DEV     *dev,
	IN  MAD_LPORT    port,
	IN  PTP_TIME    timeToRead,
	OUT PTP_TS_STATUS    *ptpStatus
)
{
	MAD_STATUS           retVal;
	MAD_U8                hwPort;
	PTP_OPERATION    op;
	PTP_OP_DATA        opData;
	MAD_U32                baseReg;

	MAD_DBG_INFO(("ptpGetTimeStamped Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	hwPort = MAD_LPORT_2_PORT(port);
	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	baseReg = 8;

	switch (timeToRead)
	{
		case PTP_ARR0_TIME:
			opData.ptpAddr = baseReg + 0;
			break;
		case PTP_ARR1_TIME:
			opData.ptpAddr = baseReg + 4;
			break;
		case PTP_DEP_TIME:
			opData.ptpAddr = baseReg + 8;
			break;
		default:
			MAD_DBG_INFO(("Invalid time to be read\n"));
			return MAD_API_PTP_BAD_PARAM;
	}

	opData.ptpPort = hwPort;
	opData.ptpPhyPort = hwPort;    /* port number */
	opData.ptpBlock = 0;


	op = PTP_READ_MULTIPLE_DATA;
	opData.nData = 4;

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading DisPTP.\n"));
		return MAD_FAIL;
	}

	ptpStatus->isValid = (opData.ptpMultiData[0] & 0x1) ? MAD_TRUE : MAD_FALSE;
	ptpStatus->status = (PTP_INT_STATUS)((opData.ptpMultiData[0] >> 1) & 0x3);
	ptpStatus->timeStamped = MAD_PTP_BUILD_TIME(opData.ptpMultiData[2], opData.ptpMultiData[1]);
	ptpStatus->ptpSeqId = opData.ptpMultiData[3];

	MAD_DBG_INFO(("OK.\n"));
	return MAD_OK;

}

/*******************************************************************************
* ptpGetTSValidSt
*
* DESCRIPTION:
*        This routine gets PTP Time valid bit.
*
* INPUTS:
*       port - logical port number.
*       timeToRead    - Arr0, Arr1, or Dep time (MAD_PTP_TIME enum type)
*
* OUTPUTS:
*    isValid        - MAD_TRUE: valid TS, MAD_FALSE: invalid TS.
*
* RETURNS:
*       MAD_OK         - on success
*       MAD_[ERROR_STATUS]     - on error
*       MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
MAD_STATUS ptpGetTSValidSt
(
	IN  MAD_DEV     *dev,
	IN  MAD_LPORT    port,
	IN  PTP_TIME    timeToRead,
	OUT MAD_BOOL    *isValid
)
{
	MAD_STATUS           retVal;
	MAD_U8                hwPort;
	PTP_OPERATION    op;
	PTP_OP_DATA        opData;
	MAD_U32                baseReg;

	MAD_DBG_INFO(("ptpGetTSValidSt Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	baseReg = 8;

	switch (timeToRead)
	{
		case PTP_ARR0_TIME:
			opData.ptpAddr = baseReg + 0;
			break;
		case PTP_ARR1_TIME:
			opData.ptpAddr = baseReg + 4;
			break;
		case PTP_DEP_TIME:
			opData.ptpAddr = baseReg + 8;
			break;
		default:
			MAD_DBG_INFO(("Invalid time to get valid\n"));
			return MAD_API_PTP_BAD_PARAM;
	}

	opData.ptpPort = hwPort;
	opData.ptpData = 0;
	opData.ptpBlock = 0;
	opData.ptpPhyPort = hwPort;    /* port number */
	op = PTP_READ_DATA;

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing Port Status.\n"));
		return MAD_FAIL;
	}

	*isValid = (opData.ptpData & 0x1) ? MAD_TRUE : MAD_FALSE;
	MAD_DBG_INFO(("OK.\n"));
	return MAD_OK;

}

/*******************************************************************************
* ptpResetTimeStamp
*
* DESCRIPTION:
*        This routine resets PTP Time valid bit so that PTP logic can time stamp
*        a next PTP frame that needs to be time stamped.
*
* INPUTS:
*       port - logical port number.
*       timeToReset    - Arr0, Arr1, or Dep time (MAD_PTP_TIME enum type)
*
* OUTPUTS:
*        None.
*
* RETURNS:
*       MAD_OK         - on success
*       MAD_[ERROR_STATUS]     - on error
*       MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
MAD_STATUS ptpResetTimeStamp
(
	IN  MAD_DEV     *dev,
	IN  MAD_LPORT    port,
	IN  PTP_TIME    timeToReset
)
{
	MAD_STATUS       retVal;
	MAD_U8           hwPort;
	PTP_OPERATION    op;
	PTP_OP_DATA      opData;
	MAD_U32          baseReg;
	MAD_U32			 index;

	MAD_DBG_INFO(("ptpResetTimeStamp Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	hwPort = MAD_LPORT_2_PORT(port);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	switch (timeToReset)
	{
		case PTP_ARR0_TIME:
			baseReg = 8;
			break;
		case PTP_ARR1_TIME:
			baseReg = 12;
			break;
		case PTP_DEP_TIME:
			baseReg = 16;
			break;
		default:
			MAD_DBG_INFO(("Invalid time to reset\n"));
			return MAD_API_PTP_BAD_PARAM;
	}

	opData.ptpPort = hwPort;
	opData.ptpBlock = 0;
	opData.ptpPhyPort = hwPort;    /* port number */
	opData.ptpData = 0;
	op = PTP_WRITE_DATA;
	for (index = 0; index < 4; index++)
	{
		opData.ptpAddr = baseReg + index;
		if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
		{
			MAD_DBG_INFO(("Failed writing Port Status.\n"));
			return MAD_FAIL;
		}
	}

	MAD_DBG_INFO(("OK.\n"));
	return MAD_OK;
}


/*******************************************************************************
* ptpSetGlobalConfig
*
* DESCRIPTION:
*       This routine writes port PTP global configuration parameters.
*
* INPUTS:
*       phyAddr  - The logical phy port number
*       ptpData  - PTP global configuration parameters.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MAD_OK      - on success
*       MAD_[ERROR_STATUS]    - on error
*       MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
MAD_STATUS ptpSetGlobalConfig
(
	IN  MAD_DEV     *dev,
	IN  MAD_U32     phyAddr,
	IN  PTP_GLOBAL_CONFIG    *ptpData
)
{
	MAD_STATUS           retVal;
	PTP_OPERATION    op;
	PTP_OP_DATA        opData;
	MAD_U8          hwPort;         /* the physical port number     */

	MAD_DBG_INFO(("ptpSetGlobalConfig Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(phyAddr);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)phyAddr));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	opData.ptpBlock = 0x0;    /* PTP register space */
	opData.ptpPhyPort = hwPort;    /* port number */

	opData.ptpPort = 0xF;    /* Global register */
	op = PTP_WRITE_DATA;

	/* setting PTPEType, offset 0 */
	opData.ptpAddr = 0;
	opData.ptpData = ptpData->ptpEType;

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing PTPEType.\n"));
		return MAD_FAIL;
	}

	/* setting MsgIDTSEn, offset 1 */
	opData.ptpAddr = 1;
	opData.ptpData = ptpData->msgIdTSEn;

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing MsgIDTSEn.\n"));
		return MAD_FAIL;
	}

	/* setting TSArrPtr, offset 2 */
	opData.ptpAddr = 2;
	opData.ptpData = ptpData->tsArrPtr;

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing TSArrPtr.\n"));
		return MAD_FAIL;
	}

	MAD_DBG_INFO(("OK.\n"));
	return MAD_OK;
}

/*******************************************************************************
* ptpGlobalGetConfig
*
* DESCRIPTION:
*       This routine reads port PTP global configuration parameters.
*
* INPUTS:
*       phyAddr - The logical phy port number
*
* OUTPUTS:
*        ptpData  - PTP global configuration parameters.
*
* RETURNS:
*       MAD_OK      - on success
*       MAD_[ERROR_STATUS]    - on error
*       MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
MAD_STATUS ptpGetGlobalConfig
(
	IN  MAD_DEV     *dev,
	IN  MAD_U32     phyAddr,
	OUT PTP_GLOBAL_CONFIG    *ptpData
)
{
	MAD_STATUS           retVal;
	PTP_OPERATION    op;
	PTP_OP_DATA        opData;
	MAD_U8          hwPort;         /* the physical port number     */

	/* translate LPORT to hardware port */

	MAD_DBG_INFO(("ptpGetGlobalConfig Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	hwPort = MAD_LPORT_2_PORT(phyAddr);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)phyAddr));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}


	opData.ptpBlock = 0x0;    /* PTP register space */
	opData.ptpPhyPort = hwPort;    /* port number */

	opData.ptpPort = 0xF;    /* Global register */
	op = PTP_READ_DATA;

	/* getting PTPEType, offset 0 */
	opData.ptpAddr = 0;
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading PTPEType.\n"));
		return MAD_FAIL;
	}

	ptpData->ptpEType = opData.ptpData;

	/* getting MsgIDTSEn, offset 1 */
	opData.ptpAddr = 1;
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading MsgIDTSEn.\n"));
		return MAD_FAIL;
	}

	ptpData->msgIdTSEn = opData.ptpData;

	/* getting TSArrPtr, offset 2 */
	opData.ptpAddr = 2;
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading TSArrPtr.\n"));
		return MAD_FAIL;
	}

	ptpData->tsArrPtr = opData.ptpData;

	MAD_DBG_INFO(("OK.\n"));
	return MAD_OK;
}

/*******************************************************************************
* ptpSetMode
*
* DESCRIPTION:
*        Set PTP_GLOBAL_CONFIG_PTP_MODE_REG from PTP point/data registers.
*
* INPUTS:
*        phyAddr - The logical phy port number
*        mode -
*          altScheme    - Alternate Scheme
*          grandMstr    - Grand Master Enable.
*          oneStep    - OneStep Enable.
*          ptpMode    - PTP Mode as follows:
*                   00 = Boundary Clock
*                   01 = Peer to Peer Transparent Clock
*                   10 = End to End Transparent Clock
*
* OUTPUTS:
*        None.
*
* RETURNS:
*        MAD_OK      - on success
*        MAD_[ERROR_STATUS]    - on error
*        MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*        None
*
*******************************************************************************/
MAD_STATUS ptpSetMode
(
	IN  MAD_DEV				*dev,
	IN  MAD_U32				phyAddr,
	IN  PTP_MODE_CONFIG	    *mode
)
{
	MAD_STATUS    retVal;         /* Functions return value.      */
	MAD_U8		data;

	MAD_DBG_INFO(("ptpModeSet Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	if ((retVal = ptpGlobalPointDataGet(dev, phyAddr, PTP_GLOBAL_CONFIG_PTP_MODE_REG, &data)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed get PTP point/data.\n"));
		return MAD_FAIL;
	}
	data &= 0xe0;
	data = (data |
		((mode->altScheme & 1) << 4) |
		((mode->grandMstr & 1) << 3) |
		((mode->oneStep & 1) << 2) |
		(mode->PTPMode & 3));

	if ((retVal = ptpGlobalPointDataSet(dev, phyAddr, PTP_GLOBAL_CONFIG_PTP_MODE_REG, data)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed set PTP point/data.\n"));
		return MAD_FAIL;
	}
	return retVal;
}

/*******************************************************************************
* ptpGetMode
*
* DESCRIPTION:
*        Get PTP_GLOBAL_CONFIG_PTP_MODE_REG from PTP point/data registers.
*
* INPUTS:
*        phyAddr - The logical phy port number
*
* OUTPUTS:
*        mode -
*          altScheme    - Alternate Scheme
*          grandMstr    - Grand Master Enable.
*          oneStep    - OneStep Enable.
*          ptpMode    - PTP Mode as follows:
*                   00 = Boundary Clock
*                   01 = Peer to Peer Transparent Clock
*                   10 = End to End Transparent Clock
*
* RETURNS:
*        MAD_OK      - on success
*        MAD_[ERROR_STATUS]    - on error
*        MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*        None
*
*******************************************************************************/
MAD_STATUS ptpGetMode
(
	IN  MAD_DEV				*dev,
	IN  MAD_U32				phyAddr,
	OUT PTP_MODE_CONFIG	    *mode
)
{
	MAD_STATUS    retVal;         /* Functions return value.      */
	MAD_U8		data;

	MAD_DBG_INFO(("ptpModeGet Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	if ((retVal = ptpGlobalPointDataGet(dev, phyAddr, PTP_GLOBAL_CONFIG_PTP_MODE_REG, &data)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed get PTP point/data.\n"));
		return MAD_FAIL;
	}

	mode->altScheme = (data >> 4) & 1;
	mode->grandMstr = (data >> 3) & 1;
	mode->oneStep = (data >> 2) & 1;
	mode->PTPMode = data & 3;

	return retVal;
}

/*******************************************************************************
* ptpSetMasterSlave
*
* DESCRIPTION:
*        Set PTP as Master or Slave.
*
* INPUTS:
*        phyAddr - The logical phy port number
*        role - PTP_ROLE_SLAVE:Set device as slave   PTP_ROLE_MASTER:Set device as master
*
* RETURNS:
*        MAD_OK      - on success
*        MAD_[ERROR_STATUS]    - on error
*        MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*        None
*
*******************************************************************************/
MAD_STATUS ptpSetMasterSlave
(
	IN MAD_DEV		*dev,
	IN MAD_U32		phyAddr,
	IN PTP_MASTER_SLAVE role
)
{
	MAD_STATUS    retVal;         /* Functions return value.      */
	MAD_U8		data;
	MAD_U8		tmp;

	MAD_DBG_INFO(("ptpSetMasterSlave Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	if (role == PTP_ROLE_SLAVE)
		tmp = 0;
	else if (role == PTP_ROLE_MASTER)
		tmp = 1;
	else
	{
		MAD_DBG_ERROR(("Bad parametar input"));
		return MAD_API_PTP_BAD_PARAM;
	}

	if ((retVal = ptpGlobalPointDataGet(dev, phyAddr, PTP_GLOBAL_CONFIG_PTP_MODE_REG, &data)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed get PTP point/data.\n"));
		return MAD_FAIL;
	}
	data &= 0xf7;
	data = data | (tmp << 3);

	if ((retVal = ptpGlobalPointDataSet(dev, phyAddr, PTP_GLOBAL_CONFIG_PTP_MODE_REG, data)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed set PTP point/data.\n"));
		return MAD_FAIL;
	}
	return retVal;
}

/*******************************************************************************
* ptpGetMasterSlave
*
* DESCRIPTION:
*        Get PTP as Master or Slave.
*
* INPUTS:
*        phyAddr - The logical phy port number

* OUTPUT:
*        role - PTP_ROLE_SLAVE:Set device as slave   PTP_ROLE_MASTER:Set device as master
*
* RETURNS:
*        MAD_OK      - on success
*        MAD_[ERROR_STATUS]    - on error
*        MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*        None
*
*******************************************************************************/
MAD_STATUS ptpGetMasterSlave
(
	IN MAD_DEV		*dev,
	IN MAD_U32		phyAddr,
	OUT PTP_MASTER_SLAVE *role
)
{
	MAD_STATUS    retVal;         /* Functions return value.      */
	MAD_U8		data;

	MAD_DBG_INFO(("ptpModeGet Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	if ((retVal = ptpGlobalPointDataGet(dev, phyAddr, PTP_GLOBAL_CONFIG_PTP_MODE_REG, &data)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed get PTP point/data.\n"));
		return MAD_FAIL;
	}

	*role = (data >> 3) & 1;

	return retVal;
}

/*******************************************************************************
* ptpGetPTPIntStatus
*
* DESCRIPTION:
*       This routine gets PTP interrupt status for a port.
*
* INPUTS:
*       phyAddr - The logical phy port number
*
* OUTPUTS:
*       ptpIntSt   - interrupt status for
*                       ptpTrigGenInt;      Trigger generate mode Interrupt
*                       ptpEventInt;        Event Capture Interrupt
*                       ptpUpperPortInt;    Upper Ports Interrupt
*                       ptpIntStPortVect
*                         interrupt status for each port (bit 0 for port 0, bit 1 for port 1, etc.)
*
* RETURNS:
*       MAD_OK      - on success
*       MAD_FAIL    - on error
*       MAD_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
MAD_STATUS ptpGetPTPIntStatus
(
	IN  MAD_DEV						*dev,
	IN  MAD_U32						phyAddr,
	OUT PTP_INTERRUPT_STATUS		*ptpIntSt
)
{
	MAD_STATUS           retVal;
	PTP_OPERATION    op;
	PTP_OP_DATA      opData;
	MAD_U8				 hwPort;         /* the physical port number     */


	MAD_DBG_INFO(("ptpGetPTPIntStatus Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(phyAddr);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)phyAddr));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	opData.ptpBlock = 0;
	opData.ptpPort = 0xF;    /* Global register */
	opData.ptpPhyPort = hwPort;    /* port number */
	op = PTP_READ_DATA;
	opData.ptpAddr = 8;

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading PTP status.\n"));
		return MAD_FAIL;
	}

	ptpIntSt->ptpIntStPortVect = (opData.ptpData & ((1 << 11) - 1));
	ptpIntSt->ptpTrigGenInt = madLong2Char((opData.ptpData >> 15) & 1);
	ptpIntSt->ptpEventInt = madLong2Char((opData.ptpData >> 14) & 1);

	MAD_DBG_INFO(("OK.\n"));
	return MAD_OK;
}


/*******************************************************************************
* ptpTODBusyBit
*
* DESCRIPTION:
*        Check the busy bit to see if the hardware is busy. Make sure the busy status is False befor any other ToD operation
*
* INPUTS:
*        phyAddr - The logical phy port number
*
* OUTPUTS:
*        busyStatus - MAD_TRUE: Hardware is busy, should wait. MAD_FALSE: busy bit has been self clear, can do other operation
*
* RETURNS:
*        MAD_OK      - on success
*        MAD_[ERROR_STATUS]    - on error
*        MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*        None
*
*******************************************************************************/
MAD_STATUS ptpTODGetBusyBitStatus
(
	IN MAD_DEV		*dev,
	IN MAD_U32		phyAddr,
	OUT MAD_BOOL	*busyStatus
)
{
	MAD_U32 tmpData;
	MAD_STATUS retVal;

	retVal = ptpGetReg(dev, phyAddr, 0xf, 0x12, &tmpData);
	if (retVal != MAD_OK)
	{
		MAD_DBG_INFO(("Get the PTP Tod Busy bit Failed.\n"));
		return retVal;
	}

	*busyStatus = tmpData & 0x8000 ? MAD_TRUE : MAD_FALSE;
	return retVal;
}

/*******************************************************************************
* ptpTODStoreCompensation
*
* DESCRIPTION:
*        Store Compensation register only to selected TimeArray
*
* INPUTS:
*        phyAddr - The logical phy port number
*        timeArrayIndex - Time array index
*        timeArray -    TOD load pointer
*                       10 byte ToD time,
*                       8 byte 1722 time,
*                       4 byte Compensation,
*                       1 byte Domain Number,
*                       And a 1 bit Clock Valid.
*
* OUTPUTS:
*        None.
*
* RETURNS:
*        MAD_OK      - on success
*        MAD_[ERROR_STATUS]    - on error
*        MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*        None
*
*******************************************************************************/
MAD_STATUS ptpTODStoreCompensation
(
	IN  MAD_DEV              *dev,
	IN  MAD_U32				 phyAddr,
	IN  MAD_U8               timeArrayIndex,
	IN  PTP_TIME_ARRAY		*timeArray
)
{
	MAD_STATUS			retVal;         /* Functions return value.      */
	MAD_U8				hwPort;

	MAD_DBG_INFO(("ptpTODStoreCompensation Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	hwPort = MAD_LPORT_2_PORT(phyAddr);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)phyAddr));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}
	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	/*check the timeArrayIndex*/
	if (!(timeArrayIndex < 4))
	{
		MAD_DBG_ERROR(("Time array index is over range.\n"));
		return MAD_API_PTP_BAD_PARAM;
	}

	madSemTake(dev, dev->ptpTodSem, OS_WAIT_FOREVER);

	retVal = ptpTimeArrayStore(dev, phyAddr, timeArrayIndex, PTP_TOD_STORE_COMP, timeArray);

	if (retVal != MAD_OK)
	{
		madSemGive(dev, dev->ptpTodSem);
		return MAD_FAIL;
	}

	madSemGive(dev, dev->ptpTodSem);
	MAD_DBG_INFO(("OK.\n"));
	return retVal;
}

/*******************************************************************************
* ptpTODStoreAll
*
* DESCRIPTION:
*        Store all register to selected TimeArray
*
* INPUTS:
*        phyAddr - The logical phy port number
*        timeArrayIndex - Time array index
*        timeArray -    TOD load pointer
*                       10 byte ToD time,
*                       8 byte 1722 time,
*                       4 byte Compensation,
*                       1 byte Domain Number,
*                       And a 1 bit Clock Valid.
*
* OUTPUTS:
*        None.
*
* RETURNS:
*        MAD_OK      - on success
*        MAD_[ERROR_STATUS]    - on error
*        MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*        None
*
*******************************************************************************/
MAD_STATUS ptpTODStoreAll
(
	IN  MAD_DEV              *dev,
	IN  MAD_U32				 phyAddr,
	IN  MAD_U8               timeArrayIndex,
	IN  PTP_TIME_ARRAY		 *timeArray
)
{
	MAD_STATUS			retVal;         /* Functions return value.      */
	MAD_U8				hwPort;

	MAD_DBG_INFO(("ptpTODStoreAll Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	hwPort = MAD_LPORT_2_PORT(phyAddr);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)phyAddr));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}
	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}
	/*check the timeArrayIndex*/
	if (!(timeArrayIndex < 4))
	{
		MAD_DBG_ERROR(("Time array index is over range.\n"));
		return MAD_API_PTP_BAD_PARAM;
	}

	madSemTake(dev, dev->ptpTodSem, OS_WAIT_FOREVER);

	retVal = ptpTimeArrayStore(dev, phyAddr, timeArrayIndex, PTP_TOD_STORE_ALL, timeArray);

	if (retVal != MAD_OK)
	{
		madSemGive(dev, dev->ptpTodSem);
		return MAD_FAIL;
	}

	madSemGive(dev, dev->ptpTodSem);
	MAD_DBG_INFO(("OK.\n"));
	return retVal;
}

/*******************************************************************************
* ptpTODCaptureAll
*
* DESCRIPTION:
*        Capture selected TimeArray.
*
* INPUTS:
*        phyAddr - The logical phy port number
*        timeArrayIndex - Time array index
*
* OUTPUTS:
*        timeArray -    TOD load pointer
*                       10 byte ToD time,
*                       8 byte 1722 time,
*                       4 byte Compensation,
*                       1 byte Domain Number,
*                       And a 1 bit Clock Valid.
*
* RETURNS:
*        MAD_OK      - on success
*        MAD_[ERROR_STATUS]    - on error
*        MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*        None
*
*******************************************************************************/
MAD_STATUS ptpTODCaptureAll
(
	IN  MAD_DEV              *dev,
	IN  MAD_U32				 phyAddr,
	IN  MAD_U8               timeArrayIndex,
	OUT PTP_TIME_ARRAY		*timeArray
)
{
	MAD_STATUS			retVal;         /* Functions return value.      */
	MAD_U8				hwPort;

	MAD_DBG_INFO(("ptpTODCaptureAll Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	hwPort = MAD_LPORT_2_PORT(phyAddr);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)phyAddr));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}
	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}
	/*check the timeArrayIndex*/
	if (!(timeArrayIndex < 4))
	{
		MAD_DBG_ERROR(("Time array index is over range.\n"));
		return MAD_API_PTP_BAD_PARAM;
	}

	madSemTake(dev, dev->ptpTodSem, OS_WAIT_FOREVER);

	retVal = ptpTimeArrayCapture(dev, phyAddr, timeArrayIndex, timeArray);

	if (retVal != MAD_OK)
	{
		madSemGive(dev, dev->ptpTodSem);
		return MAD_FAIL;
	}

	madSemGive(dev, dev->ptpTodSem);
	MAD_DBG_INFO(("OK.\n"));
	return retVal;
}

/*******************************************************************************
* ptpPulseSet
*
* DESCRIPTION:
*        Set PTP Pulse parameters.
*
* INPUTS:
*        phyAddr - The logical phy port number
*        pulsePara -  following parameters:
*             ptpPulseWidth;
*             ptp1ppsWidthRange;
*             ptp1ppsPhase;
*             ptp1ppsSelect;
*
* OUTPUTS:
*        None.
*
* RETURNS:
*        MAD_OK      - on success
*        MAD_[ERROR_STATUS]    - on error
*        MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*        None
*
*******************************************************************************/
MAD_STATUS ptpPulseSet
(
	IN  MAD_DEV					*dev,
	IN  MAD_U32					phyAddr,
	IN  PTP_PULSE_STRUCT		*pulsePara
)
{
	MAD_STATUS			retVal;         /* Functions return value.      */
	PTP_OPERATION	op;
	PTP_OP_DATA		opData;
	MAD_U8				hwPort;

	MAD_DBG_INFO(("ptpPulseSet Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	hwPort = MAD_LPORT_2_PORT(phyAddr);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)phyAddr));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}
	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	opData.ptpBlock = 0;
	opData.ptpPort = 0xF;    /* Global register */
	opData.ptpPhyPort = hwPort;    /* port number */
	op = PTP_WRITE_DATA;
	opData.ptpAddr = 0x1E;
	opData.ptpData = ((((pulsePara->ptpPulseWidth) & 0xf) << 12) |
		(((pulsePara->ptp1ppsWidthRange) & 7) << 8) |
		(((pulsePara->ptp1ppsPhase) & 1) << 3) |
		(pulsePara->ptp1ppsSelect & 0x3));
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing Pulse parameters.\n"));
		return MAD_FAIL;
	}

	return retVal;
}

/*******************************************************************************
* ptpPulseGet
*
* DESCRIPTION:
*        Get PTP Pulse parameters.
*
* INPUTS:
*        phyAddr - The logical phy port number
*
* OUTPUTS:
*        pulsePara -  following parameters:
*             ptpPulseWidth;
*             ptp1ppsWidthRange;
*             ptp1ppsPhase;
*             ptp1ppsSelect;
*
* RETURNS:
*        MAD_OK      - on success
*        MAD_[ERROR_STATUS]    - on error
*        MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*        None
*
*******************************************************************************/
MAD_STATUS ptpPulseGet
(
	IN   MAD_DEV				*dev,
	IN	 MAD_U32				 phyAddr,
	OUT  PTP_PULSE_STRUCT		*pulsePara
)
{
	MAD_STATUS			retVal;         /* Functions return value.      */
	PTP_OPERATION	op;
	PTP_OP_DATA		opData;
	MAD_U8				hwPort;

	MAD_DBG_INFO(("ptpPulseGet Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	hwPort = MAD_LPORT_2_PORT(phyAddr);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)phyAddr));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}
	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	opData.ptpBlock = 0;
	opData.ptpPort = 0xF;    /* Global register */
	opData.ptpPhyPort = hwPort;    /* port number */
	op = PTP_READ_DATA;
	opData.ptpAddr = 0x1E;
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading Pulse parameters.\n"));
		return MAD_FAIL;
	}
	pulsePara->ptpPulseWidth = madLong2Char((opData.ptpData >> 12) & 0xF);
	pulsePara->ptp1ppsWidthRange = madLong2Char((opData.ptpData >> 8) & 7);
	pulsePara->ptp1ppsPhase = madLong2Char((opData.ptpData >> 3) & 1);
	pulsePara->ptp1ppsSelect = madLong2Char(opData.ptpData & 0x3);

	return retVal;
}


/*******************************************************************************
* taiSetEventConfig
*
* DESCRIPTION:
*       This routine sets port TAI Event Capture configuration parameters.
*
* INPUTS:
*       phyAddr - The logical phy port number
*       eventData  - TAI Event Capture configuration parameters.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*       MAD_OK      - on success
*       MAD_[ERROR_STATUS]    - on error
*       MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
MAD_STATUS taiSetEventConfig
(
	IN  MAD_DEV     *dev,
	IN	MAD_U32		phyAddr,
	IN  TAI_EVENT_CONFIG    *eventData
)
{
	MAD_STATUS           retVal;
	PTP_OPERATION    op;
	PTP_OP_DATA        opData;
	MAD_U8          hwPort;         /* the physical port number     */

	/* translate LPORT to hardware port */
	MAD_DBG_INFO(("taiSetEventConfig Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	hwPort = MAD_LPORT_2_PORT(phyAddr);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)phyAddr));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	opData.ptpBlock = 0x0;    /* PTP register space */
	opData.ptpPhyPort = hwPort;    /* port number */

	opData.ptpPort = 0xE;    /* TAI register */
	op = PTP_READ_DATA;

	/* getting PTPEType, offset 0 */
	opData.ptpAddr = 0;
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading TAI global config.\n"));
		return MAD_FAIL;
	}

	opData.ptpData &= ~((3 << 14) | (1 << 8));
	if (eventData->eventCapIntEn)
		opData.ptpData |= (1 << 8);
	if (eventData->eventOverwrite)
		opData.ptpData |= (1 << 15);
	if (eventData->eventCtrStart)
		opData.ptpData |= (1 << 14);

	opData.ptpData &= ~(1 << 13);
	if (eventData->eventPhase)
		opData.ptpData |= (1 << 13);

	op = PTP_WRITE_DATA;

	opData.ptpAddr = 0;

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing TAI global config.\n"));
		return MAD_FAIL;
	}
	
	/* getting Capture trigger, offset 9 */
	op = PTP_READ_DATA;
	opData.ptpAddr = 9;
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading TAI global config.\n"));
		return MAD_FAIL;
	}
	opData.ptpData &= ~(1 << 14);
	opData.ptpData |= (eventData->capTrigEvent == MAD_TRUE) ? 0x4000 : 0x0;

	op = PTP_WRITE_DATA;
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading TAI global config.\n"));
		return MAD_FAIL;
	}


	MAD_DBG_INFO(("OK.\n"));
	return MAD_OK;

}

/*******************************************************************************
* taiGetEventConfig
*
* DESCRIPTION:
*       This routine gets port TAI Event Capture configuration parameters.
*
* INPUTS:
*       phyAddr - The logical phy port number
*
* OUTPUTS:
*        eventData  - TAI Event Capture configuration parameters.
*
* RETURNS:
*       MAD_OK      - on success
*       MAD_[ERROR_STATUS]    - on error
*       MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
MAD_STATUS taiGetEventConfig
(
	IN  MAD_DEV     *dev,
	IN	MAD_U32	     phyAddr,
	OUT TAI_EVENT_CONFIG    *eventData
)
{
	MAD_STATUS           retVal;
	PTP_OPERATION    op;
	PTP_OP_DATA        opData;
	MAD_U8          hwPort;         /* the physical port number     */

	/* translate LPORT to hardware port */
	MAD_DBG_INFO(("taiGetEventConfig Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	hwPort = MAD_LPORT_2_PORT(phyAddr);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)phyAddr));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	opData.ptpBlock = 0x0;    /* PTP register space */
	opData.ptpPhyPort = hwPort;    /* port number */

	opData.ptpPort = 0xE;    /* TAI register */
	op = PTP_READ_DATA;

	/* getting PTPEType, offset 0 */
	opData.ptpAddr = 0;
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading TAI global config.\n"));
		return MAD_FAIL;
	}

	eventData->eventCapIntEn = (opData.ptpData & (1 << 8)) ? MAD_TRUE : MAD_FALSE;
	eventData->eventOverwrite = (opData.ptpData & (1 << 15)) ? MAD_TRUE : MAD_FALSE;
	eventData->eventCtrStart = (opData.ptpData & (1 << 14)) ? MAD_TRUE : MAD_FALSE;
	eventData->eventPhase = (opData.ptpData & (1 << 13)) ? MAD_TRUE : MAD_FALSE;

	op = PTP_READ_DATA;
	opData.ptpAddr = 9;
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading TAI global config.\n"));
		return MAD_FAIL;
	}
	eventData->capTrigEvent = (opData.ptpData & (1 << 14)) ? MAD_TRUE : MAD_FALSE;

	MAD_DBG_INFO(("OK.\n"));
	return MAD_OK;

}


/*******************************************************************************
* taiSetTrigConfig
*
* DESCRIPTION:
*       This routine sets port TAI Trigger configuration parameters.
*
* INPUTS:
*        phyAddr - The logical phy port number
*        trigEn    - enable/disable TAI Trigger.
*        trigData  - TAI Trigger configuration parameters (valid only if trigEn is MAD_TRUE).
*
* OUTPUTS:
*		None.
*
* RETURNS:
*       MAD_OK      - on success
*       MAD_[ERROR_STATUS]    - on error
*       MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
MAD_STATUS taiSetTrigConfig
(
	IN  MAD_DEV     *dev,
	IN	MAD_U32	     phyAddr,
	IN  MAD_BOOL     trigEn,
	IN  TAI_TRIGGER_CONFIG    *trigData
)
{
	MAD_STATUS           retVal;
	PTP_OPERATION    op;
	PTP_OP_DATA        opData;
	MAD_U8          hwPort;         /* the physical port number     */

	/* translate LPORT to hardware port */
	MAD_DBG_INFO(("taiSetTrigConfig Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	hwPort = MAD_LPORT_2_PORT(phyAddr);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)phyAddr));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	opData.ptpBlock = 0x0;    /* PTP register space */
	opData.ptpPhyPort = hwPort;    /* port number */

	opData.ptpPort = 0xE;    /* TAI register */
	op = PTP_WRITE_DATA;

	opData.ptpAddr = 2;
	opData.ptpData = MAD_PTP_L16_TIME(trigData->trigGenAmt);

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing Trig generation amount.\n"));
		return MAD_FAIL;
	}

	opData.ptpAddr = 3;
	opData.ptpData = MAD_PTP_H16_TIME(trigData->trigGenAmt);

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing Trig generation amount.\n"));
		return MAD_FAIL;
	}

	op = PTP_WRITE_DATA;
	opData.ptpAddr = 4;        /* TrigClkComp */

	opData.ptpData = (trigData->trigClkCompDir) ? 0x8000 : 0;
	opData.ptpData |= (trigData->trigClkComp &= 0x7fff);

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing TAI trig compare clock.\n"));
		return MAD_FAIL;
	}

	op = PTP_READ_DATA;
	opData.ptpAddr = 5;        /* PulseWidth */

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing TAI global config.\n"));
		return MAD_FAIL;
	}

	op = PTP_WRITE_DATA;
	opData.ptpAddr = 5;        /* PulseWidth */
	opData.ptpData &= (~0xF000);
	opData.ptpData |= (MAD_U32)(trigData->pulseWidth << 12);
	opData.ptpData &= ~(0x7ff);
	opData.ptpData |= ((trigData->pulseWidthRange & 7) << 8);
	opData.ptpData |= ((trigData->trigClkCompSub & 0xff));


	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing TAI global config.\n"));
		return MAD_FAIL;
	}

	op = PTP_WRITE_DATA;
	opData.ptpAddr = 0x10;
	opData.ptpData = MAD_PTP_L16_TIME(trigData->trigGenTime);

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing trigger generation time.\n"));
		return MAD_FAIL;
	}

	opData.ptpAddr = 0x11;
	opData.ptpData = MAD_PTP_H16_TIME(trigData->trigGenTime);

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing trigger generation time.\n"));
		return MAD_FAIL;
	}

	/* Set Trig control, offset 0 */
	op = PTP_READ_DATA;
	opData.ptpAddr = 0;

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading TAI global config.\n"));
		return MAD_FAIL;
	}

	opData.ptpData &= 0xed00;
	if (trigData->trigGenIntEn == MAD_TRUE)
		opData.ptpData |= (1 << 9);
	else
		opData.ptpData &= ~(1 << 9);

	if (trigData->mode == MAD_TAI_TRIG_ON_GIVEN_TIME)
		opData.ptpData |= (1 << 1);
	else
		opData.ptpData &= ~(1 << 1);

	if (trigEn == MAD_TRUE)
		opData.ptpData |= 1;
	else
		opData.ptpData &= ~1;

	if (trigData->trigPhase == MAD_TRUE)
		opData.ptpData |= (1 << 12);
	else
		opData.ptpData &= ~(1 << 12);

	if (trigData->trigLock == MAD_TRUE)
		opData.ptpData |= (1 << 7);
	else
		opData.ptpData &= ~(1 << 7);

	opData.ptpData &= ~(7 << 4);
	opData.ptpData |= ((trigData->trigLockRange & 7) << 4);

	if (trigData->blkUpdate == MAD_TRUE)
		opData.ptpData |= (1 << 3);
	else
		opData.ptpData &= ~(1 << 3);

	if (trigData->multiSyncMode == TAI_MULTI_PTP_SYNC_ENABLE)
		opData.ptpData |= (1 << 2);
	else
		opData.ptpData &= ~(1 << 2);


	op = PTP_WRITE_DATA;
	opData.ptpAddr = 0;

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing TAI global config.\n"));
		return MAD_FAIL;
	}

	MAD_DBG_INFO(("OK.\n"));
	return MAD_OK;

}

/*******************************************************************************
* taiGetTrigConfig
*
* DESCRIPTION:
*       This routine gets port TAI Trigger configuration parameters.
*
* INPUTS:
*       phyAddr - The logical phy port number
*
* OUTPUTS:
*        trigEn    - enable/disable TAI Trigger.
*        trigData  - TAI Trigger configuration parameters (valid only if trigEn is MAD_TRUE).
*
* RETURNS:
*       MAD_OK      - on success
*       MAD_[ERROR_STATUS]    - on error
*       MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
MAD_STATUS taiGetTrigConfig
(
	IN  MAD_DEV     *dev,
	IN	MAD_U32	     phyAddr,
	OUT MAD_BOOL     *trigEn,
	OUT TAI_TRIGGER_CONFIG    *trigData
)
{
	MAD_STATUS           retVal;
	PTP_OPERATION    op;
	PTP_OP_DATA        opData;
	MAD_U8          hwPort;         /* the physical port number     */

	MAD_DBG_INFO(("taiGetTrigConfig Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(phyAddr);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)phyAddr));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	opData.ptpBlock = 0x0;    /* PTP register space */
	opData.ptpPhyPort = hwPort;    /* port number */

	opData.ptpPort = 0xE;    /* TAI register */
	op = PTP_READ_DATA;

	opData.ptpAddr = 3;
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading TAI trigGenAmt.\n"));
		return MAD_FAIL;
	}
	trigData->trigGenAmt = opData.ptpData;

	opData.ptpAddr = 2;
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading TAI trigGenAmt.\n"));
		return MAD_FAIL;
	}
	trigData->trigGenAmt = (trigData->trigGenAmt) << 16 | opData.ptpData;

	/* getting TrigClkComp, offset 4 */
	opData.ptpAddr = 4;
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading TAI TrigClkComp.\n"));
		return MAD_FAIL;
	}

	trigData->trigClkCompDir = (opData.ptpData & 0x8000) ? 1 : 0;
	trigData->trigClkComp = opData.ptpData & 0x7fff;

	opData.ptpAddr = 5;        /* PulseWidth */
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing TAI PulseWidth.\n"));
		return MAD_FAIL;
	}
	trigData->pulseWidth = (MAD_U32)((opData.ptpData >> 12) & 0xF);
	trigData->pulseWidthRange = madLong2Char((opData.ptpData >> 8) & 0x7);
	trigData->trigClkCompSub = opData.ptpData & 0xff;

	op = PTP_READ_DATA;
	opData.ptpAddr = 0x11;
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading TAI trigGenTime.\n"));
		return MAD_FAIL;
	}
	trigData->trigGenTime = opData.ptpData;

	opData.ptpAddr = 0x10;
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading TAI trigGenTime.\n"));
		return MAD_FAIL;
	}

	trigData->trigGenTime = (trigData->trigGenTime) << 16 | opData.ptpData;

	/* lockCorrect read only */
	opData.ptpAddr = 0x12;
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading TAI lockCorrect.\n"));
		return MAD_FAIL;
	}
	trigData->lockCorrect = madLong2Char(opData.ptpData & 0xF);

	/* Get Trig control, offset 0 */
	op = PTP_READ_DATA;
	opData.ptpAddr = 0;
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading TAI trig config 0.\n"));
		return MAD_FAIL;
	}

	trigData->trigGenIntEn = ((opData.ptpData >> 9) & 1) ? MAD_TRUE : MAD_FALSE;
	trigData->mode = (opData.ptpData & 2) ? TAI_TRIG_ON_GIVEN_TIME : TAI_TRIG_PERIODIC_PULSE;
	*trigEn = (opData.ptpData & 1) ? MAD_TRUE : MAD_FALSE;
	trigData->trigPhase = ((opData.ptpData >> 12) & 1) ? MAD_TRUE : MAD_FALSE;
	trigData->trigLock = ((opData.ptpData >> 7) & 1) ? MAD_TRUE : MAD_FALSE;
	trigData->trigLockRange = madLong2Char((opData.ptpData >> 4) & 7);
	trigData->blkUpdate = ((opData.ptpData >> 3) & 1) ? MAD_TRUE : MAD_FALSE;
	trigData->multiSyncMode = (opData.ptpData & 4) ? TAI_MULTI_PTP_SYNC_ENABLE : TAI_MULTI_PTP_SYNC_DISABLE;

	MAD_DBG_INFO(("OK.\n"));
	return MAD_OK;
}

/*******************************************************************************
* taiGetTSClkPer
*
* DESCRIPTION:
*         Time Stamping Clock Period in pico seconds.
*        This routine specifies the clock period for the time stamping clock supplied
*        to the port PTP hardware logic.
*        This is the clock that is used by the hardware logic to update the PTP
*        Global Time Counter.
*
* INPUTS:
*       phyAddr - The logical phy port number
*
* OUTPUTS:
*        clk        - time stamping clock period
*
* RETURNS:
*         MAD_OK      - on success
*         MAD_[ERROR_STATUS]    - on error
*         MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*         None
*
*******************************************************************************/
MAD_STATUS taiGetTSClkPer
(
	IN  MAD_DEV     *dev,
	IN	MAD_U32	     phyAddr,
	OUT MAD_U32        *clk
)
{
	MAD_STATUS           retVal;
	PTP_OPERATION    op;
	PTP_OP_DATA        opData;
	MAD_U8          hwPort;         /* the physical port number     */

	/* translate LPORT to hardware port */
	MAD_DBG_INFO(("taiGetTSClkPer Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	hwPort = MAD_LPORT_2_PORT(phyAddr);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)phyAddr));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	opData.ptpBlock = 0x0;    /* PTP register space */
	opData.ptpPhyPort = hwPort;    /* port number */

	opData.ptpPort = 0xE;    /* TAI register */
	op = PTP_READ_DATA;

	opData.ptpAddr = 1;

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading TS clock period.\n"));
		return MAD_FAIL;
	}

	*clk = (MAD_U32)opData.ptpData;

	MAD_DBG_INFO(("OK.\n"));
	return MAD_OK;
}


/*******************************************************************************
* taiSetTSClkPer
*
* DESCRIPTION:
*         Time Stamping Clock Period in pico seconds.
*        This routine specifies the clock period for the time stamping clock supplied
*        to the PTP hardware logic.
*        This is the clock that is used by the hardware logic to update the port PTP
*        Global Time Counter.
*
* INPUTS:
*       phyAddr - The logical phy port number
*       clk        - time stamping clock period
*
* OUTPUTS:
*		None.
*
* RETURNS:
*         MAD_OK      - on success
*         MAD_[ERROR_STATUS]    - on error
*         MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*         None
*
*******************************************************************************/
MAD_STATUS taiSetTSClkPer
(
	IN  MAD_DEV     *dev,
	IN	MAD_U32	     phyAddr,
	IN  MAD_U32        clk
)
{
	MAD_STATUS           retVal;
	PTP_OPERATION    op;
	PTP_OP_DATA        opData;
	MAD_U8          hwPort;         /* the physical port number     */

	/* translate LPORT to hardware port */

	MAD_DBG_INFO(("taiSetTSClkPer Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	hwPort = MAD_LPORT_2_PORT(phyAddr);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)phyAddr));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	opData.ptpBlock = 0x0;    /* PTP register space */
	opData.ptpPhyPort = hwPort;    /* port number */

	opData.ptpPort = 0xE;    /* TAI register */
	op = PTP_WRITE_DATA;

	opData.ptpAddr = 1;

	opData.ptpData = (MAD_U16)clk;

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing TS clock period.\n"));
		return MAD_FAIL;
	}

	MAD_DBG_INFO(("OK.\n"));
	return MAD_OK;
}

/*******************************************************************************
* taiSetMultiPTPSync
*
* DESCRIPTION:
*         This routine sets port Multiple PTP device sync mode and sync time (TrigGenAmt).
*        When enabled, the hardware logic detects a low to high transition on the
*        EventRequest(GPIO) and transfers the sync time into the PTP Global Time
*        register. The EventCapTime is also updated at that instant.
*
* INPUTS:
*        phyAddr - The logical phy port number
*        multiEn        - enable/disable Multiple PTP device sync mode
*        syncTime    - sync time (valid only if multiEn is MAD_TRUE)
*
* OUTPUTS:
*        None.
*
* RETURNS:
*        MAD_OK      - on success
*        MAD_[ERROR_STATUS]    - on error
*        MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*         When enabled, taiSetTrigConfig, taiSetEventConfig APIs are not operational.
*
*******************************************************************************/
MAD_STATUS taiSetMultiPTPSync
(
	IN  MAD_DEV     *dev,
	IN  MAD_U32	     phyAddr,
	IN  MAD_BOOL     multiEn,
	IN  MAD_U32        syncTime
)
{
	MAD_STATUS           retVal;
	PTP_OPERATION    op;
	PTP_OP_DATA        opData;
	MAD_U8          hwPort;         /* the physical port number     */

	/* translate LPORT to hardware port */

	MAD_DBG_INFO(("taiSetMultiPTPSync Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	hwPort = MAD_LPORT_2_PORT(phyAddr);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)phyAddr));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	opData.ptpBlock = 0x0;    /* PTP register space */
	opData.ptpPhyPort = hwPort;    /* port number */

	opData.ptpPort = 0xE;    /* TAI register */
	op = PTP_READ_DATA;

	/* getting PTPEType, offset 0 */
	opData.ptpAddr = 0;
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading TAI register.\n"));
		return MAD_FAIL;
	}

	opData.ptpData &= ~(1 << 2);

	if (multiEn == MAD_FALSE)
	{
		op = PTP_WRITE_DATA;

		if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
		{
			MAD_DBG_INFO(("Failed writing TAI register.\n"));
			return MAD_FAIL;
		}

		return MAD_OK;
	}

	opData.ptpData |= (1 << 2);


	op = PTP_WRITE_DATA;

	opData.ptpAddr = 0;

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing TAI register.\n"));
		return MAD_FAIL;
	}

	opData.ptpAddr = 2;
	opData.ptpData = MAD_PTP_L16_TIME(syncTime);

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing TAI register.\n"));
		return MAD_FAIL;
	}

	opData.ptpAddr = 3;
	opData.ptpData = MAD_PTP_H16_TIME(syncTime);

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing TAI register.\n"));
		return MAD_FAIL;
	}

	MAD_DBG_INFO(("OK.\n"));
	return MAD_OK;
}

/*******************************************************************************
* taiGetMultiPTPSync
*
* DESCRIPTION:
*         This routine gets port Multiple PTP device sync mode and sync time (TrigGenAmt).
*        When enabled, the hardware logic detects a low to high transition on the
*        EventRequest(GPIO) and transfers the sync time into the PTP Global Time
*        register. The EventCapTime is also updated at that instant.
*
* INPUTS:
*        phyAddr - The logical phy port number
*
* OUTPUTS:
*        multiEn        - enable/disable Multiple PTP device sync mode
*        syncTime    - sync time (valid only if multiEn is MAD_TRUE)
*
* RETURNS:
*        MAD_OK      - on success
*        MAD_[ERROR_STATUS]    - on error
*        MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*         When enabled, taiSetTrigConfig, taiSetEventConfig APIs are not operational.
*
*******************************************************************************/
MAD_STATUS taiGetMultiPTPSync
(
	IN  MAD_DEV     *dev,
	IN  MAD_U32	     phyAddr,
	OUT MAD_BOOL     *multiEn,
	OUT MAD_U32        *syncTime
)
{
	MAD_STATUS           retVal;
	PTP_OPERATION    op;
	PTP_OP_DATA        opData;
	MAD_U32                 data[2];
	MAD_U8          hwPort;         /* the physical port number     */

	/* translate LPORT to hardware port */
	MAD_DBG_INFO(("taiGetMultiPTPSync Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	hwPort = MAD_LPORT_2_PORT(phyAddr);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)phyAddr));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	opData.ptpBlock = 0x0;    /* PTP register space */
	opData.ptpPhyPort = hwPort;    /* port number */

	opData.ptpPort = 0xE;    /* TAI register */
	op = PTP_READ_DATA;

	/* getting PTPEType, offset 0 */
	opData.ptpAddr = 0;
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading TAI register.\n"));
		return MAD_FAIL;
	}

	if (!(opData.ptpData & (1 << 2)))
	{
		*multiEn = MAD_FALSE;
		*syncTime = 0;
		return MAD_OK;
	}
	*multiEn = MAD_TRUE;

	opData.ptpAddr = 2;
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading TAI register.\n"));
		return MAD_FAIL;
	}
	data[0] = opData.ptpData;

	opData.ptpAddr = 3;
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading TAI register.\n"));
		return MAD_FAIL;
	}
	data[1] = opData.ptpData;

	*syncTime = MAD_PTP_BUILD_TIME(data[1], data[0]);

	MAD_DBG_INFO(("OK.\n"));
	return MAD_OK;
}


/*******************************************************************************
* taiGetPTPGlobalTime
*
* DESCRIPTION:
*       This routine gets the port global timer value that is running off of the free
*        running clock.
*
* INPUTS:
*       phyAddr - The logical phy port number
*
* OUTPUTS:
*        ptpTime    - PTP global time
*
* RETURNS:
*       MAD_OK      - on success
*       MAD_[ERROR_STATUS]    - on error
*       MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
MAD_STATUS taiGetPTPGlobalTime
(
	IN  MAD_DEV      *dev,
	IN	MAD_U32	     phyAddr,
	OUT MAD_U32      *ptpTime
)
{
	MAD_STATUS           retVal;
	PTP_OPERATION    op;
	PTP_OP_DATA        opData;
	MAD_U8          hwPort;         /* the physical port number     */

	/* translate LPORT to hardware port */
	MAD_DBG_INFO(("taiGetPTPGlobalTime Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	hwPort = MAD_LPORT_2_PORT(phyAddr);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)phyAddr));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}


	opData.ptpBlock = 0x0;    /* PTP register space */
	opData.ptpPhyPort = hwPort;    /* port number */
	op = PTP_READ_MULTIPLE_DATA;
	opData.ptpPort = 0xE; /* TAI Global */
	opData.ptpAddr = 14;
	opData.nData = 2;

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading PTP Global Time.\n"));
		return MAD_FAIL;
	}

	*ptpTime = MAD_PTP_BUILD_TIME(opData.ptpMultiData[1], opData.ptpMultiData[0]);

	MAD_DBG_INFO(("OK.\n"));
	return MAD_OK;
}

/*******************************************************************************
* taiGetEventStatus
*
* DESCRIPTION:
*       This routine gets port TAI Event Capture status.
*
* INPUTS:
*       phyAddr - The logical phy port number
*
* OUTPUTS:
*        status  - TAI Event Status parameters.
*
* RETURNS:
*       MAD_OK      - on success
*       MAD_[ERROR_STATUS]    - on error
*       MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
MAD_STATUS taiGetEventStatus
(
	IN  MAD_DEV     *dev,
	IN	MAD_U32	     phyAddr,
	OUT TAI_EVENT_STATUS    *status
)
{
	MAD_STATUS           retVal;
	PTP_OPERATION    op;
	PTP_OP_DATA        opData;
	MAD_U32                 data[2];
	MAD_U8          hwPort;         /* the physical port number     */

	/* translate LPORT to hardware port */
	MAD_DBG_INFO(("taiGetEventStatus Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	hwPort = MAD_LPORT_2_PORT(phyAddr);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)phyAddr));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	opData.ptpBlock = 0x0;    /* PTP register space */
	opData.ptpPhyPort = hwPort;    /* port number */

	opData.ptpPort = 0xE;    /* TAI register */
	op = PTP_READ_DATA;

	opData.ptpAddr = 9;
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading PTPEType.\n"));
		return MAD_FAIL;
	}

	status->isValid = (opData.ptpData & (1 << 8)) ? MAD_TRUE : MAD_FALSE;
	status->eventCtr = opData.ptpData & 0xFF;
	status->eventErr = (opData.ptpData & (1 << 9)) ? MAD_TRUE : MAD_FALSE;

	if (status->isValid == MAD_FALSE)
	{
		return MAD_OK;
	}

	opData.ptpAddr = 0x0A;
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading PTPEType.\n"));
		return MAD_FAIL;
	}
	data[0] = opData.ptpData;

	opData.ptpAddr = 0x0B;
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading PTPEType.\n"));
		return MAD_FAIL;
	}
	data[1] = opData.ptpData;

	status->eventTime = MAD_PTP_BUILD_TIME(data[1], data[0]);

	MAD_DBG_INFO(("OK.\n"));
	return MAD_OK;
}


/*******************************************************************************
* taiGetEventInt
*
* DESCRIPTION:
*       This routine gets port TAI Event Capture Interrupt status.
*
* INPUTS:
*       phyAddr - The logical phy port number
*
* OUTPUTS:
*        intStatus     - interrupt status for TAI Event capture
*
* RETURNS:
*       MAD_OK      - on success
*       MAD_[ERROR_STATUS]    - on error
*       MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
MAD_STATUS taiGetEventInt
(
	IN  MAD_DEV     *dev,
	IN	MAD_U32	     phyAddr,
	OUT MAD_BOOL        *intStatus
)
{
	MAD_STATUS           retVal;
	PTP_OPERATION    op;
	PTP_OP_DATA        opData;
	MAD_U8          hwPort;         /* the physical port number     */

	MAD_DBG_INFO(("taiGetEventInt Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(phyAddr);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)phyAddr));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	opData.ptpPort = 0xE;    /* TAI Global register */
	op = PTP_READ_DATA;
	opData.ptpAddr = 9;
	opData.ptpBlock = 0;
	opData.ptpPhyPort = hwPort;    /* port number */

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading DisPTP.\n"));
		return MAD_FAIL;
	}

	*intStatus = (opData.ptpData & 0x0100) ? MAD_TRUE : MAD_FALSE;

	MAD_DBG_INFO(("OK.\n"));
	return MAD_OK;
}

/*******************************************************************************
* taiClearEventInt
*
* DESCRIPTION:
*       This routine clear TAI Event Capture Interrupt status. Software needs to clear the EventCap Valid bit to a zero to enable the EventCap Register to be able to acquire a subsequent event if the 
*		EventCapOv (Event Capture Override ¨C TAI Page 12 Register 0) is not enabled
*
* INPUTS:
*       phyAddr - The logical phy port number
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MAD_OK      - on success
*       MAD_[ERROR_STATUS]    - on error
*       MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
MAD_STATUS taiClearEventInt
(
	IN  MAD_DEV			*dev,
	IN	MAD_U32			phyAddr
)
{
	MAD_STATUS           retVal;
	PTP_OPERATION    op;
	PTP_OP_DATA      opData;
	MAD_U8				 hwPort;         /* the physical port number     */

	MAD_DBG_INFO(("taiClearEventInt Called.\n"));

	if (!(dev->deviceId == MAD_88E151x))
	{
		MAD_DBG_INFO(("MAD_API_PTP_NOT_SUPPORT\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	/* translate LPORT to hardware port */
	hwPort = MAD_LPORT_2_PORT(phyAddr);
	if (!(MAD_DEV_PORT_CHECK(dev, hwPort)))
	{
		MAD_DBG_INFO(("The port %d is over range.\n", (int)phyAddr));
		return MAD_API_ERROR_OVER_PORT_RANGE;
	}

	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	opData.ptpPort = 0xE;    /* TAI register */
	op = PTP_READ_DATA;
	opData.ptpAddr = 9;
	opData.ptpBlock = 0;
	opData.ptpPhyPort = hwPort;    /* port number */

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading eventInt.\n"));
		return MAD_FAIL;
	}

	opData.ptpData &= 0x7EFF;
	op = PTP_WRITE_DATA;

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing eventInt.\n"));
		return MAD_FAIL;
	}

	MAD_DBG_INFO(("OK.\n"));
	return MAD_OK;
}

/****************************************************************************/
/* Internal use functions.                                                  */
/****************************************************************************/
/*******************************************************************************
* ptpReadplus
*
* DESCRIPTION:
*       This function accesses ToD Registers and some TAI Registers.
*
* INPUTS:
*       port - Port number to write the register for.
*       page - Page number to be accessed.
*       reg  - The register's address.
*		multi - Multi read(MAD_TRUE) or single read(MAD_FALSE).
*
* OUTPUTS:
*		ptpData    - points to the data storage that the operation requires.
*
* RETURNS:
*       MAD_OK on success,
*       MAD_[ERROR_STATUS] otherwise.
*
* COMMENTS:
*
*******************************************************************************/
static MAD_STATUS ptpReadplus
(
	IN   MAD_DEV    *dev,
	IN   MAD_U8     port,
	IN   MAD_U16    page,
	IN   MAD_U8     reg,
	IN	 MAD_BOOL	multi,
	INOUT PTP_OP_DATA        *opData
)
{
	MAD_STATUS	retVal;    /* Functions return value */
	MAD_U16		tmpData;
	unsigned int	i;

	if (page == MAD_PAGE12)
	{
		if (reg >= 0xE)
		{
			tmpData = (reg & 0xF) | 0x8E00;
		}
		else
		{
			MAD_DBG_ERROR(("Readplus should not be called.\n"));
			return MAD_FAIL;
		}
	}
	else if (page == MAD_PAGE15)
	{
		tmpData = (reg & 0xF) | 0x8F10;
	}
	else
	{
		MAD_DBG_ERROR(("Readplus should not be called.\n"));
		return MAD_FAIL;
	}

	madSemTake(dev, dev->ptpRegsSem, OS_WAIT_FOREVER);

	if ((retVal = madHwWritePagedPhyReg(dev, port, 14, 14, tmpData)) != MAD_OK)
	{
		MAD_DBG_ERROR(("Writing to paged phy reg (%i,%i) failed.\n", 14, 14));
		madSemGive(dev, dev->ptpRegsSem);
		return (retVal | MAD_API_FAIL_WRITE_PAGED_REG);
	}

	if (multi)
	{
		for (i = 0; i < opData->nData; i++)
		{
			if ((retVal = madHwReadPagedPhyReg(dev, port, 14, 15, &tmpData)) != MAD_OK)
			{
				MAD_DBG_ERROR(("Reading paged phy reg (%i,%i) failed.\n", 14, 15));
				madSemGive(dev, dev->ptpRegsSem);
				return (retVal | MAD_API_FAIL_READ_PAGED_REG);
			}
			opData->ptpMultiData[i] = (MAD_U32)tmpData;
		}
	}
	else
	{
		if ((retVal = madHwReadPagedPhyReg(dev, port, 14, 15, &tmpData)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Reading paged phy reg (%i,%i) failed.\n", 14, 15));
			madSemGive(dev, dev->ptpRegsSem);
			return (retVal | MAD_API_FAIL_READ_PAGED_REG);
		}
		opData->ptpData = tmpData;
	}

	madSemGive(dev, dev->ptpRegsSem);
	return MAD_OK;
}

/*******************************************************************************
* mapRegs
*
* DESCRIPTION:
*       This function maps parameters in PTP_OP_DATA to page and register number of PHY.
*
* INPUTS:
*       opData    - points to the data storage that the operation requires.
*
* OUTPUTS:
*       port    - port number.
*       page    - page number.
*       reg     - register number.
*
* RETURNS:
*       MAD_OK on success,
*       MAD_[ERROR_STATUS] otherwise.
*
* COMMENTS:
*
*******************************************************************************/
static MAD_STATUS mapRegs
(
	PTP_OP_DATA		*opData, 
	MAD_U8			*port, 
	MAD_U16			*page, 
	MAD_U8			*reg
)
{
	*port = madLong2Char(opData->ptpPhyPort);
	switch (opData->ptpPort)
	{
	case 0: /* PTP Port configuration/Status */
	case 1: /* PTP Port configuration/Status */
	case 2: /* PTP Port configuration/Status */
	case 3: /* PTP Port configuration/Status */
		/* Port 0, 1, 2, 3 for 1340*/
		*page = MAD_PAGE8;
		*reg = madLong2Char(opData->ptpAddr);
		if (opData->ptpAddr>15)
		{
			*page = MAD_PAGE9;
			*reg = madLong2Char(opData->ptpAddr - 16);
		}
		break;
	case 0xe: /* TAI configuration/Status */
		*page = MAD_PAGE12;
		*reg = madLong2Char(opData->ptpAddr);
		if (opData->ptpAddr>15)
		{
			*page = MAD_PAGE13;
			*reg = madLong2Char(opData->ptpAddr - 16);
		}
		break;
	case 0xf:/* PTP Global configuration/Status */
		*page = MAD_PAGE14;
		*reg = madLong2Char(opData->ptpAddr);
		if (opData->ptpAddr>15)
		{
			*page = MAD_PAGE15;
			*reg = madLong2Char(opData->ptpAddr - 16);
		}
		break;
	default:
		return MAD_FAIL;
	}
	return MAD_OK;
}

/*******************************************************************************
* ptpOperation
*
* DESCRIPTION:
*       This function accesses PTP Command Register and Data Register.
*
* INPUTS:
*       ptpOp      - The stats operation bits to be written into the stats
*                    operation register.
*
* OUTPUTS:
*       ptpData    - points to the data storage that the operation requires.
*
* RETURNS:
*       MAD_OK on success,
*       MAD_[ERROR_STATUS] otherwise.
*
* COMMENTS:
*
*******************************************************************************/
static MAD_STATUS ptpOperation
(
	IN    MAD_DEV             *dev,
	IN    PTP_OPERATION        ptpOp,
	INOUT PTP_OP_DATA         *opData
)
{
	MAD_STATUS       retVal;    /* Functions return value */
	MAD_U16          data;     /* temporary Data storage */
	MAD_U32     i;
	MAD_U8     port;
	MAD_U16    page;
	MAD_U8     reg, kpReg;

	/* Set the PTP Operation register */
	switch (ptpOp)
	{
	case PTP_WRITE_DATA:
		data = (MAD_U16)opData->ptpData;

		retVal = mapRegs(opData, &port, &page, &reg);
		if (retVal != MAD_OK)
		{
			return retVal;
		}

		if ((retVal = madHwWritePagedPhyReg(dev, port, page, reg, data)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to paged phy reg (%i,%i) failed.\n", page, reg));
			return (retVal | MAD_API_FAIL_WRITE_PAGED_REG);
		}

		break;

	case PTP_READ_DATA:
		retVal = mapRegs(opData, &port, &page, &reg);
		if (retVal != MAD_OK)
		{
			return retVal;
		}
		/* PTP global time registers and ToD registers should use readplus */
		if (PTP_READPLUS_SUPPORT(page, reg))
		{
			retVal = ptpReadplus(dev, port, page, reg, MAD_FALSE, opData);
		}
		else
		{
			retVal = madHwReadPagedPhyReg(dev, port, page, reg, &data);
			opData->ptpData = data;
		}
		if (retVal != MAD_OK)
		{
			MAD_DBG_ERROR(("Reading paged phy reg (%i,%i) failed.\n", page, reg));
			return (retVal | MAD_API_FAIL_READ_PAGED_REG);
		}

		break;

	case PTP_READ_MULTIPLE_DATA:
		retVal = mapRegs(opData, &port, &page, &reg);
		if (retVal != MAD_OK)
		{
			return retVal;
		}

		if (PTP_READPLUS_SUPPORT(page, reg))
		{
			retVal = ptpReadplus(dev, port, page, reg, MAD_TRUE, opData);
			if (retVal != MAD_OK)
			{
				return retVal;
			}
		}
		else
		{
			for (i = 0; i<opData->nData; i++)
			{
				retVal = mapRegs(opData, &port, &page, &reg);
				if (retVal != MAD_OK)
				{
					return retVal;
				}
				retVal = madHwReadPagedPhyReg(dev, port, page, reg, &data);
				opData->ptpMultiData[i] = (MAD_U32)data;
				if (retVal != MAD_OK)
				{
					return retVal;
				}
				if (!PTP_READPLUS_SUPPORT(page, reg))
					opData->ptpAddr++;
			}
		}
		break;

	case PTP_READ_TIMESTAMP_DATA:
		kpReg = madLong2Char(opData->ptpAddr);
		retVal = mapRegs(opData, &port, &page, &reg);
		if (retVal != MAD_OK)
		{
			return retVal;
		}

		retVal = madHwReadPagedPhyReg(dev, port, page, reg, &data);
		opData->ptpMultiData[0] = (MAD_U32)data;
		if (!(data & 0x1))
		{
			/* valid bit is not set */
			return retVal;
		}

		for (i = 1; i<opData->nData; i++)
		{
			opData->ptpAddr = kpReg + i;
			retVal = mapRegs(opData, &port, &page, &reg);
			if (retVal != MAD_OK)
			{
				return retVal;
			}
			retVal = madHwReadPagedPhyReg(dev, port, page, reg, &data);
			opData->ptpMultiData[i] = (MAD_U32)data;
			if (retVal != MAD_OK)
			{
				return retVal;
			}
			opData->ptpAddr++;
		}

		data = 0;
		opData->ptpAddr = kpReg;
		retVal = mapRegs(opData, &port, &page, &reg);
		if (retVal != MAD_OK)
		{
			return retVal;
		}

		if ((retVal = madHwWritePagedPhyReg(dev, port, page, reg, data)) != MAD_OK)
		{
			MAD_DBG_ERROR(("Writing to paged phy reg (%i,%i) failed.\n", page, reg));
			return (retVal | MAD_API_FAIL_WRITE_PAGED_REG);
		}
		break;

	default:
		return MAD_FAIL;
	}

	return MAD_OK;
}

/*******************************************************************************
* ptpGetReg
*
* DESCRIPTION:
*       This routine reads PTP register.
*
* INPUTS:
*       port - logical port number.
*       ptpPort     - original PTP port number. (global port is 0xf)
*       regOffset    - register to read
*
* OUTPUTS:
*        data        - register data
*
* RETURNS:
*       MAD_OK      - on success
*       MAD_[ERROR_STATUS]    - on error
*       MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
static MAD_STATUS ptpGetReg
(
	IN  MAD_DEV     *dev,
	IN  MAD_LPORT    port,
	IN  MAD_U8    ptpPort,
	IN  MAD_U32        regOffset,
	OUT MAD_U32        *data
)
{
	MAD_STATUS           retVal;
	MAD_U8                hwPort;
	PTP_OPERATION    op;
	PTP_OP_DATA        opData;

	MAD_DBG_INFO(("ptpGetReg Called.\n"));

	hwPort = MAD_LPORT_2_PORT(port);

	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	if (regOffset > 0x1F)
	{
		MAD_DBG_INFO(("Invalid reg offset\n"));
		return MAD_API_PTP_BAD_PARAM;
	}

	op = PTP_READ_DATA;
	opData.ptpPort = ptpPort;
	opData.ptpAddr = regOffset;
	opData.ptpBlock = 0;
	opData.ptpPhyPort = hwPort;    /* port number */

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading DisPTP.\n"));
		return MAD_FAIL;
	}

	*data = opData.ptpData;

	MAD_DBG_INFO(("OK.\n"));
	return MAD_OK;
}

/*******************************************************************************
* ptpSetReg
*
* DESCRIPTION:
*       This routine writes data to PTP register.
*
* INPUTS:
*       port         - logical port number
*       ptpPort     - original PTP port number. (global port is 0xf)
*       regOffset    - register to be written
*        data        - data to be written
*
* OUTPUTS:
*        None.
*
* RETURNS:
*       MAD_OK      - on success
*       MAD_[ERROR_STATUS]    - on error
*       MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
static MAD_STATUS ptpSetReg
(
	IN  MAD_DEV     *dev,
	IN  MAD_LPORT    port,
	IN  MAD_U8    ptpPort,
	IN  MAD_U32        regOffset,
	IN  MAD_U32        data
)
{
	MAD_STATUS           retVal;
	MAD_U8                hwPort;
	PTP_OPERATION    op;
	PTP_OP_DATA        opData;

	MAD_DBG_INFO(("ptpSetReg Called.\n"));

	hwPort = MAD_LPORT_2_PORT(port);
	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}


	if ((regOffset > 0x1F) || (data > 0xFFFF))
	{
		MAD_DBG_INFO(("Invalid reg offset/data\n"));
		return MAD_API_PTP_BAD_PARAM;
	}

	op = PTP_WRITE_DATA;
	opData.ptpPort = ptpPort;
	opData.ptpAddr = regOffset;
	opData.ptpData = data;
	opData.ptpBlock = 0;
	opData.ptpPhyPort = hwPort;    /* port number */

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading DisPTP.\n"));
		return MAD_FAIL;
	}


	MAD_DBG_INFO(("OK.\n"));
	return MAD_OK;

}

/*******************************************************************************
* ptpGlobalPointDataSet
*
* DESCRIPTION:
*        Set indirect register pointer control data from PTP Global registers.
*
* INPUTS:
*        port - logical port number.
*        point - Pointer to following index registers:
*            PTP_GLOBAL_CONFIG_PTP_MODE_REG
*            PTP_GLOBAL_CONFIG_PTP_EXTERNAL_TIME_FORM
*        data -  PTP Global configuration data write to the indirect register
*                pointed to by the point above.
*
* OUTPUTS:
*        None.
*
* RETURNS:
*        MAD_OK      - on success
*        MAD_[ERROR_STATUS]    - on error
*        MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*        None
*
*******************************************************************************/
static MAD_STATUS ptpGlobalPointDataSet
(
	IN  MAD_DEV         *dev,
	IN  MAD_LPORT		  port,
	IN  MAD_U8             point,
	IN  MAD_U8             data
)
{
	MAD_STATUS    retVal;         /* Functions return value.      */
	MAD_U32       count, tmpData;
	MAD_DBG_INFO(("ptpGlobalPointDataSet Called.\n"));

	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	madSemTake(dev, dev->ptpModeSem, OS_WAIT_FOREVER);

	/* Wait until the Scratch and Misc control is ready. */
	count = 5;
	tmpData = 0x8000;
	while (tmpData == 0x8000)
	{
		retVal = ptpGetReg(dev, port, 0xf, 7, &tmpData);
		if (retVal != MAD_OK)
		{
			madSemGive(dev, dev->ptpModeSem);
			return retVal;
		}
		if (--count == 0) break;
	}

	tmpData = (MAD_U16)((1 << 15) | (point << 8) | data);

	retVal = ptpSetReg(dev, port, 0xf, 7, tmpData);
	if (retVal != MAD_OK)
	{
		MAD_DBG_INFO(("Failed.\n"));
		madSemGive(dev, dev->ptpModeSem);
		return retVal;
	}

	madSemGive(dev, dev->ptpModeSem);

	return retVal;

}

/*******************************************************************************
* ptpGlobalPointDataGet
*
* DESCRIPTION:
*        Get indirect register pointer control data from PTP Global registers.
*
* INPUTS:
*        point - Pointer to following index registers:
*            PTP_GLOBAL_CONFIG_PTP_MODE_REG
*            PTP_GLOBAL_CONFIG_PTP_EXTERNAL_TIME_FORM
* OUTPUTS:
*        data -  PTP Global configuration data read from the indirect register
*                pointed to by the point above.
*
* RETURNS:
*        MAD_OK      - on success
*        MAD_[ERROR_STATUS]    - on error
*        MAD_API_PTP_NOT_SUPPORT - if current device does not support this feature.
*
* COMMENTS:
*        None
*
*******************************************************************************/
static MAD_STATUS ptpGlobalPointDataGet
(
	IN  MAD_DEV         *dev,
	IN  MAD_LPORT		   port,
	IN  MAD_U8             point,
	OUT  MAD_U8            *data
)
{
	MAD_STATUS    retVal;         /* Functions return value.      */
	MAD_U32       tmpData;
	int count = 0x10;
	MAD_DBG_INFO(("ptpGlobalPointDataGet Called.\n"));

	/* check if device supports this feature */
	if (!MAD_DEV_CAPABILITY1(dev, MAD_PHY_PTP_TAI_CAPABLE) ||
		!MAD_DEV_HWMODE(dev, 0, MAD_PHY_MODE_COPPER))
	{
		MAD_DBG_ERROR(("PTP not supported.\n"));
		return MAD_API_PTP_NOT_SUPPORT;
	}

	madSemTake(dev, dev->ptpModeSem, OS_WAIT_FOREVER);

	tmpData = (MAD_U16)((point << 8) | 0);
	retVal = ptpSetReg(dev, port, 0xf, 7, tmpData);
	if (retVal != MAD_OK)
	{
		MAD_DBG_INFO(("Failed.\n"));
		madSemGive(dev, dev->ptpModeSem);
		return retVal;
	}

	do {
		retVal = ptpGetReg(dev, port, 0xf, 7, &tmpData);
		if (retVal != MAD_OK)
		{
			MAD_DBG_INFO(("Failed.\n"));
			madSemGive(dev, dev->ptpModeSem);
			return retVal;
		}
		if ((count--) == 0) return MAD_FAIL;
	} while (tmpData & 0x8000);

	*data = madLong2Char(tmpData & 0xff);

	madSemGive(dev, dev->ptpModeSem);

	return retVal;
}

/*******************************************************************************
* ptpTimeArraySet
*
* DESCRIPTION:
*       This function Store PTP time array Data.
*
* INPUTS:
*       port      - logical port number.
*       storeOp   - Store compensation and store all
*       timeArrayIndex - Time array index
*       timeArray -    TOD load pointer
*                       10 byte ToD time,
*                       8 byte 1722 time,
*                       4 byte Compensation,
*                       1 byte Domain Number,
*                       And a 1 bit Clock Valid.
*
* RETURNS:
*       MAD_OK on success,
*       MAD_[ERROR_STATUS] otherwise.
*
* COMMENTS:
*
*******************************************************************************/
static MAD_STATUS ptpTimeArrayStore
(
	IN    MAD_DEV                         *dev,
	IN    MAD_LPORT                       port,
	IN    MAD_U8                          timeArrayIndex,
	IN    PTP_TOD_STORE_OPERATION		storeOp,
	IN    PTP_TIME_ARRAY				*timeArray
)
{
	MAD_STATUS			retVal;         /* Functions return value.      */
	PTP_OP_DATA     opData;
	PTP_OPERATION   op;
	MAD_U8				hwPort;

	hwPort = MAD_LPORT_2_PORT(port);

	/*Load point Set*/
	opData.ptpBlock = 0;
	opData.ptpPort = 0xF;    /* Global register */
	op = PTP_WRITE_DATA;
	opData.ptpPhyPort = hwPort;    /* port number */
	opData.ptpAddr = 0x10;
	opData.ptpData = madLong2Short(timeArray->todLoadPoint & 0xffff);
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing TOD Load Point.\n"));
		return MAD_FAIL;
	}
	opData.ptpAddr = 0x11;
	opData.ptpData = madLong2Short((timeArray->todLoadPoint >> 16) & 0xffff);
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading TOD Load Point.\n"));
		return MAD_FAIL;
	}

	/*Time array index, Clock Valid bit and domain number will be set when do the operation*/

	/*ToD Set*/
	opData.ptpBlock = 0;
	opData.ptpPort = 0xF;    /* Global register */
	opData.ptpPhyPort = hwPort;    /* port number */
	op = PTP_WRITE_DATA;

	opData.ptpAddr = 0x13;
	opData.ptpData = timeArray->todNanoseconds & 0xffff;
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing ToD.\n"));
		return MAD_FAIL;
	}
	opData.ptpAddr = 0x14;
	opData.ptpData = (timeArray->todNanoseconds >> 16) & 0xffff;
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing ToD.\n"));
		return MAD_FAIL;
	}
	opData.ptpAddr = 0x15;
	opData.ptpData = timeArray->todSecondsLow & 0xffff;
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing ToD.\n"));
		return MAD_FAIL;
	}
	opData.ptpAddr = 0x16;
	opData.ptpData = (timeArray->todSecondsLow >> 16) & 0xffff;
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing ToD.\n"));
		return MAD_FAIL;
	}
	opData.ptpAddr = 0x17;
	opData.ptpData = (timeArray->todSecondsHigh) & 0xffff;
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing ToD.\n"));
		return MAD_FAIL;
	}

	/*1722 time set*/
	opData.ptpBlock = 0;
	opData.ptpPort = 0xF;    /* Global register */
	opData.ptpPhyPort = hwPort;    /* port number */
	op = PTP_WRITE_DATA;

	opData.ptpAddr = 0x18;
	opData.ptpData = madLonglong2Long(timeArray->Nanoseconds1722Low & 0xffff);
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing 1722 ToD.\n"));
		return MAD_FAIL;
	}
	opData.ptpAddr = 0x19;
	opData.ptpData = madLonglong2Long((timeArray->Nanoseconds1722Low >> 16) & 0xffff);
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing 1722 ToD.\n"));
		return MAD_FAIL;
	}
	opData.ptpAddr = 0x1a;
	opData.ptpData = madLonglong2Long((timeArray->Nanoseconds1722High) & 0xffff);
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing 1722 ToD.\n"));
		return MAD_FAIL;
	}
	opData.ptpAddr = 0x1b;
	opData.ptpData = madLonglong2Long((timeArray->Nanoseconds1722High >> 16) & 0xffff);
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing 1722 ToD.\n"));
		return MAD_FAIL;
	}

	/*Tod compensation set*/
	opData.ptpBlock = 0;
	opData.ptpPort = 0xF;    /* Global register */
	opData.ptpPhyPort = hwPort;    /* port number */
	op = PTP_WRITE_DATA;
	opData.ptpAddr = 0x1C;
	opData.ptpData = (timeArray->todCompensation & 0xffff);
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing ToD compensation.\n"));
		return MAD_FAIL;
	}

	opData.ptpAddr = 0x1D;
	opData.ptpData = ((timeArray->todCompensation >> 16) & 0xffff);
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing ToD compensation.\n"));
		return MAD_FAIL;
	}

	/*Store operation*/
	opData.ptpBlock = 0;
	opData.ptpPort = 0xF;    /* Global register */
	opData.ptpPhyPort = hwPort;    /* port number */
	op = PTP_WRITE_DATA;
	opData.ptpAddr = 0x12;
	opData.ptpData = ((1 << 15) |
		(storeOp << 12) |
		((timeArrayIndex & 3) << 9) |
		(((timeArray->clkValid) & 1) << 8) |
		(timeArray->domainNumber & 0xff));

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing ToD store operation control.\n"));
	}

	return retVal;
}

/*******************************************************************************
* ptpTimeArrayCapture
*
* DESCRIPTION:
*       This function Capture PTP time array Data.
*
* INPUTS:
*       port      - logical port number.
*       timeArrayIndex - Time array index
*       timeArray -    TOD load pointer
*                       10 byte ToD time,
*                       8 byte 1722 time,
*                       4 byte Compensation,
*                       1 byte Domain Number,
*                       And a 1 bit Clock Valid.
*
* RETURNS:
*       MAD_OK on success,
*       MAD_[ERROR_STATUS] otherwise.
*
* COMMENTS:
*
*******************************************************************************/
static MAD_STATUS ptpTimeArrayCapture
(
	IN    MAD_DEV              *dev,
	IN    MAD_LPORT            port,
	IN    MAD_U8               timeArrayIndex,
	OUT   PTP_TIME_ARRAY	   *timeArray
)
{
	MAD_STATUS			retVal;         /* Functions return value.      */
	PTP_OP_DATA     opData;
	PTP_OPERATION   op;
	MAD_U32				tmpData;
	MAD_U8				hwPort;

	hwPort = MAD_LPORT_2_PORT(port);

    /*Get the origin operation register data*/
    opData.ptpBlock = 0;
    opData.ptpPort = 0xF;    /* Global register */
    opData.ptpPhyPort = hwPort;    /* port number */
    op = PTP_READ_DATA;
    opData.ptpAddr = 0x12;
    if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
    {
        MAD_DBG_INFO(("Failed reading ToD operation control.\n"));
        return MAD_FAIL;
    }
    tmpData = opData.ptpData;

	/*Capture operation*/
	opData.ptpBlock = 0;
	opData.ptpPort = 0xF;    /* Global register */
	opData.ptpPhyPort = hwPort;    /* port number */
	op = PTP_WRITE_DATA;
	opData.ptpAddr = 0x12;
	tmpData &= 0x89ff;
	opData.ptpData = ((1 << 15) |
		(4 << 12) |
		((timeArrayIndex & 3) << 9)) |
		tmpData;
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed writing ToD capture operation control.\n"));
		return MAD_FAIL;
	}

	/*Get the Load Point*/
	opData.ptpBlock = 0;
	opData.ptpPort = 0xF;    /* Global register */
	opData.ptpPhyPort = hwPort;    /* port number */
	opData.ptpAddr = 0x10;
	op = PTP_READ_MULTIPLE_DATA;
	opData.nData = 2;

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading TOD Load Point.\n"));
		return MAD_FAIL;
	}

	timeArray->todLoadPoint = opData.ptpMultiData[1];
	timeArray->todLoadPoint <<= 16;
	timeArray->todLoadPoint |= (opData.ptpMultiData[0]);

	/*Get clock valid bit and domain number*/
	opData.ptpBlock = 0;
	opData.ptpPort = 0xF;    /* Global register */
	opData.ptpPhyPort = hwPort;    /* port number */
	op = PTP_READ_DATA;
	opData.ptpAddr = 0x12;
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading ToD operation control.\n"));
		return MAD_FAIL;
	}
	timeArray->clkValid = madLong2Char((opData.ptpData >> 8) & 1);
	timeArray->domainNumber = madLong2Char(opData.ptpData & 0xff);

	/*Get the Tod time*/
	opData.ptpBlock = 0;
	opData.ptpPort = 0xF;    /* Global register */
	opData.ptpPhyPort = hwPort;    /* port number */
	op = PTP_READ_MULTIPLE_DATA;
	opData.nData = 4;
	opData.ptpAddr = 0x13;

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading ToD.\n"));
		return MAD_FAIL;
	}

	timeArray->todNanoseconds = opData.ptpMultiData[1] & 0xffff;
	timeArray->todNanoseconds <<= 16;
	timeArray->todNanoseconds |= (opData.ptpMultiData[0] & 0xffff);
	timeArray->todSecondsLow = opData.ptpMultiData[3] & 0xffff;
	timeArray->todSecondsLow <<= 16;
	timeArray->todSecondsLow |= (opData.ptpMultiData[2] & 0xffff);

	op = PTP_READ_DATA;
	opData.ptpAddr = 0x17;
	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading ToD.\n"));
		return MAD_FAIL;
	}

	timeArray->todSecondsHigh = madLong2Short((opData.ptpData & 0xffff));

	/*Get 1722 time*/
	opData.ptpBlock = 0;
	opData.ptpPort = 0xF;    /* Global register */
	opData.ptpPhyPort = hwPort;    /* port number */
	op = PTP_READ_MULTIPLE_DATA;
	opData.nData = 4;
	opData.ptpAddr = 0x18;

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading 1722 ToD.\n"));
		return MAD_FAIL;
	}

	timeArray->Nanoseconds1722High = (opData.ptpMultiData[3] << 16) | opData.ptpMultiData[2];
	timeArray->Nanoseconds1722Low = (opData.ptpMultiData[1] << 16) | opData.ptpMultiData[0];

	/*Get compensation*/
	opData.ptpBlock = 0;
	opData.ptpPort = 0xF;    /* Global register */
	opData.ptpPhyPort = hwPort;    /* port number */
	op = PTP_READ_MULTIPLE_DATA;
	opData.nData = 2;
	opData.ptpAddr = 0x1C;

	if ((retVal = ptpOperation(dev, op, &opData)) != MAD_OK)
	{
		MAD_DBG_INFO(("Failed reading ToD compensation.\n"));
		return MAD_FAIL;
	}

	timeArray->todCompensation = (opData.ptpMultiData[1] & 0xffff) << 16;
	timeArray->todCompensation |= opData.ptpMultiData[0];

	return retVal;
}