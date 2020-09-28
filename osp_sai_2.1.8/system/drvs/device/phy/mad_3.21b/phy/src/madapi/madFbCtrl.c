#include <madCopyright.h>

/********************************************************************************
* madFbCtrl.c
* 
* DESCRIPTION:
*       APIs to control Fiber.
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
* mdFiberSetAutoNeg
*
* DESCRIPTION:
*       This routine enables or disables Fiber Auto-Neg with the specified mode.
*       When enables Auto-Neg, advertisement register value is updated with the  
*       combination of the followings:
*           Asym Pause, Pause, 1000FDX, 1000HDX,
*           Master/Slave Manual, Force Master, Port Type, 1000FDX, and 1000HDX
*       When disables Auto-Neg, only one of the following modes is supported:
*           1000FDX, or 1000HDX.
*
* INPUTS:
*       dev  - pointer to MAD driver structure returned from mdLoadDriver
*       port - The logical port number
*       en   - MAD_TRUE to enable, MAD_FALSE to disable
*       mode - if en is MAD_TRUE, combination of the followings:
*                   MAD_FIBER_AUTO_AD_ASYM_PAUSE,
*                   MAD_FIBER_AUTO_AD_PAUSE,
*                   MAD_FIBER_AUTO_AD_1000FDX,
*                   MAD_FIBER_AUTO_AD_1000HDX,
*              if en is MAD_FALSE,
*                   MAD_FIBER_AUTO_AD_ASYM_PAUSE,
*                   MAD_FIBER_AUTO_AD_PAUSE,
*                   MAD_PHY_1000FDX,
*                   MAD_PHY_1000HDX,
*   
* OUTPUTS:
*       None.
*
* RETURNS:
*       MAD_OK               - on success
*       MAD_[ERROR_STATUS]  - on error
*
* COMMENTS:
*       Page 1, Reg 0, Bit 15,13,12,8 : Soft reset,Speed,Auto-neg enable,Duplex
*       Page 1, Reg 4, Bit 11,10,8:5  : Advertisement Register
*
*******************************************************************************/
MAD_STATUS mdFiberSetAutoNeg
(
    IN  MAD_DEV   *dev,
    IN  MAD_LPORT port,
    IN  MAD_BOOL  en,
    IN  MAD_U32   mode
)
{
    MAD_U16     data;
    MAD_STATUS  retVal;
    MAD_U8      hwPort;

    MAD_DBG_INFO(("mdFiberSetAutoNeg called.\n"));

    /* translate LPORT to hardware port */
    hwPort = MAD_LPORT_2_PORT(port);
    if(!(MAD_DEV_PORT_CHECK(dev, hwPort)))
    {
      MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
      return MAD_API_ERROR_OVER_PORT_RANGE;
    }


    if(!MAD_DEV_HWMODE(dev, hwPort, MAD_PHY_MODE_FIBER))
    {
        MAD_DBG_ERROR(("Fiber not supported.\n"));
        return MAD_API_FIBER_NOT_SUPPORT;
    }
    if(MAD_DEV_IS_IN_SWITCH(dev))
    {
        MAD_DBG_ERROR(("The phy is in switch, and do not support the function.\n"));
        return MAD_API_PHY_IN_SWITCH;
    }


    /* Enable Auto-Nego */
    if (en)
    {

        /* Check if the given mode is valid */
        if (mode & ~MAD_AUTO_AD_MASK)
        {
            /* unknown mode is set */
            MAD_DBG_ERROR(("Unknown AutoNeg mode(%#x) is set.\n", (unsigned int)(mode & ~MAD_AUTO_AD_MASK)));
            return MAD_API_UNKNOWN_AUTONEG_MODE;
        }

        /* modify Auto-Neg Ad Register */
        if((retVal = madHwReadPagedPhyReg(dev,hwPort,MAD_PAGE1,MAD_REG_AUTO_AD,&data))
            != MAD_OK)
        {
            MAD_DBG_ERROR(("Reading from paged phy reg (%i,%i) failed.\n",0,MAD_REG_AUTO_AD));
            return (retVal | MAD_API_FAIL_READ_PAGED_REG);
        }
        
        data = (data & ~MAD_FIBER_AUTO_AD_MASK_REG4) | ((MAD_U16)(mode & 0xFFFF));

        if((retVal = madHwWritePagedPhyReg(dev,hwPort,MAD_PAGE1,MAD_REG_AUTO_AD,data))
            != MAD_OK)
        {
            MAD_DBG_ERROR(("Writing to paged phy reg (%i,%i) failed.\n",0,MAD_REG_AUTO_AD));
            return (retVal | MAD_API_FAIL_WRITE_PAGED_REG);
        }
        /* bits to set to Control Register */
        /* data = MAD_REG_CTRL_RESET|MAD_REG_CTRL_AUTONEGO; */
        data = MAD_REG_CTRL_AUTONEGO|MAD_REG_CTRL_DUPLEX|MAD_REG_CTRL_SPEED_MSB;
    }
    else
    {
        /* Force Speed and Duplex */
        switch (mode)
        {
            case MAD_PHY_1000FDX:
                    data = MAD_REG_CTRL_SPEED_MSB|MAD_REG_CTRL_DUPLEX;
                    break;
            case MAD_PHY_1000HDX:
                    data = MAD_REG_CTRL_SPEED_MSB;
                    break;
            case MAD_PHY_100FDX:
                    data = MAD_REG_CTRL_SPEED|MAD_REG_CTRL_DUPLEX;
                    break;
            case MAD_PHY_100HDX:
                    data = MAD_REG_CTRL_SPEED;
                    break;
            case MAD_PHY_10FDX:
                    data = MAD_REG_CTRL_DUPLEX;
                    break;
            case MAD_PHY_10HDX:
                    data = 0;
                    break;
            default:
                    /* unknown mode is set */
                    MAD_DBG_ERROR(("Unknown mode %i.\n",(int)mode));
                    return MAD_API_UNKNOWN_SPEED_MODE;
        }

    }


  /* Set Control Register */
  if((retVal = madHwWritePagedPhyReg(dev,hwPort,MAD_PAGE1,MAD_REG_CONTROL,data))
      != MAD_OK)
  {
    MAD_DBG_ERROR(("Writing to paged phy reg failed.\n"));
    return (retVal | MAD_API_FAIL_WRITE_PAGED_REG);
  }

  /* Soft Reset */
  if((retVal = mdSysModSoftReset(dev, hwPort, MAD_RESET_MODE_PORT_FIBER)) != MAD_OK)
  {
    MAD_DBG_ERROR(("Soft Reset failed.\n"));
    return (retVal | MAD_API_FAIL_SW_RESET);
  }


    return MAD_OK;
}


/*******************************************************************************
* mdFiberGetAutoNeg
*
* DESCRIPTION:
*       This routine get the specified mode of Fiber Auto-Neg.
*       When enables Auto-Neg, advertisement register value is updated with the  
*       combination of the followings:
*           Asym Pause, Pause, 1000FDX, 1000HDX,
*           Master/Slave Manual, Force Master, Port Type, 1000FDX, and 1000HDX
*       When disables Auto-Neg, only one of the following modes is supported:
*           1000FDX, or 1000HDX.
*
* INPUTS:
*       dev  - pointer to MAD driver structure returned from mdLoadDriver
*       port - The logical port number
*   
* OUTPUTS:
*       en   - MAD_TRUE if enabled, MAD_FALSE otherwise
*       mode - if en is MAD_TRUE, combination of the followings:
*                   MAD_FIBER_AUTO_AD_ASYM_PAUSE,
*                   MAD_FIBER_AUTO_AD_PAUSE,
*                   MAD_FIBER_AUTO_AD_1000FDX,
*                   MAD_FIBER_AUTO_AD_1000HDX,
*              if en is MAD_FALSE,
*                   MAD_FIBER_AUTO_AD_ASYM_PAUSE,
*                   MAD_FIBER_AUTO_AD_PAUSE,
*                   MAD_PHY_1000FDX,
*                   MAD_PHY_1000HDX,
*
* RETURNS:
*       MAD_OK   - on success
*       MAD_[ERROR_STATUS] - on error
*
* COMMENTS:
*       Page 1, Reg 0, Bit 13,12,8   : Speed, Auto-neg enable, and Duplex
*       Page 1, Reg 4, Bit 11,10,8:5 : Advertisement Register
*
*******************************************************************************/
MAD_STATUS mdFiberGetAutoNeg
(
    IN  MAD_DEV   *dev,
    IN  MAD_LPORT port,
    OUT MAD_BOOL  *en,
    OUT MAD_U32   *mode
)
{
    MAD_U16     data;
    MAD_STATUS  retVal;
    MAD_U8      hwPort;

    MAD_DBG_INFO(("mdFiberGetAutoNeg called.\n"));

    /* translate LPORT to hardware port */
    hwPort = MAD_LPORT_2_PORT(port);
    if(!(MAD_DEV_PORT_CHECK(dev, hwPort)))
    {
		MAD_DBG_INFO(("The port %d is over range.\n", (int)port));
      return MAD_API_ERROR_OVER_PORT_RANGE;
    }

    if(!MAD_DEV_HWMODE(dev, hwPort, MAD_PHY_MODE_FIBER))
    {
        MAD_DBG_ERROR(("Fiber not supported.\n"));
        return MAD_API_FIBER_NOT_SUPPORT;
    }
    if(MAD_DEV_IS_IN_SWITCH(dev))
    {
        MAD_DBG_ERROR(("The phy is in switch, and do not support the function.\n"));
        return MAD_API_PHY_IN_SWITCH;
    }

    /* read Control Register */
    if((retVal = madHwReadPagedPhyReg(dev,hwPort,MAD_PAGE1,MAD_REG_CONTROL,&data))
        != MAD_OK)
    {
        MAD_DBG_ERROR(("Reading from paged phy reg (%i,%i) failed.\n",0,MAD_REG_AUTO_AD));
        return (retVal | MAD_API_FAIL_READ_PAGED_REG);
    }

    /* check if Auto-Nego is enabled */
    if (data & MAD_REG_CTRL_AUTONEGO)
    {
        /* read Auto-Neg Ad Register */
        if((retVal = madHwReadPagedPhyReg(dev,hwPort,MAD_PAGE1,MAD_REG_AUTO_AD,&data))
            != MAD_OK)
        {
            MAD_DBG_ERROR(("Reading from paged phy reg (%i,%i) failed.\n",0,MAD_REG_AUTO_AD));
            return (retVal | MAD_API_FAIL_READ_PAGED_REG);
        }



        *mode = (MAD_U32)data & MAD_FIBER_AUTO_AD_MASK_REG4;

        *en = MAD_TRUE;
    }
    else
    {
      *mode = 0;

      /* Auto-Nego is disabled. */
      if (data & (MAD_REG_CTRL_SPEED_MSB))
      {
        switch (data & (MAD_REG_CTRL_DUPLEX))
        {
            case MAD_REG_CTRL_DUPLEX:
                *mode = MAD_PHY_1000FDX;
                break;
            default:
                *mode = MAD_PHY_1000HDX;
                break;
        }
      }
      *en = MAD_FALSE;
    }

    return MAD_OK;
}

/*******************************************************************************
* mdGetPhySGMIIReg
*
* DESCRIPTION:
*       This function reads a phy register of SGMII interface.
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
MAD_STATUS mdGetPhySGMIIReg
(
    IN  MAD_DEV    *dev,
    IN  MAD_U32    portNum,
    IN  MAD_U32    regAddr,
    OUT MAD_U32    *data
)
{
    MAD_STATUS  retVal;

    if(MAD_DEV_IS_IN_SWITCH(dev))
    {
        MAD_DBG_ERROR(("The phy is in switch, and do not support the function.\n"));
        return MAD_API_PHY_IN_SWITCH;
    }
    if((retVal=madHwReadPagedPhyReg(dev,(MAD_U8)portNum,MAD_PAGE1,
                                (MAD_U8)regAddr,(MAD_U16*)data)) != MAD_OK)
    {
        MAD_DBG_ERROR(("Paged Phy Register read failed.\n"));
        return (retVal | MAD_API_FAIL_READ_PAGED_REG);
    }

    return MAD_OK;
}


/*******************************************************************************
* mdSetPhySGMIIReg
*
* DESCRIPTION:
*       This function writes to a phy register of SGMII interface.
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
MAD_STATUS mdSetPhySGMIIReg
(
    IN  MAD_DEV    *dev,
    IN  MAD_U32    portNum,
    IN  MAD_U32    regAddr,
    IN  MAD_U32    data
)
{
    MAD_STATUS  retVal;

    if(MAD_DEV_IS_IN_SWITCH(dev))
    {
        MAD_DBG_ERROR(("The phy is in switch, and do not support the function.\n"));
        return MAD_API_PHY_IN_SWITCH;
    }
    if((retVal=madHwWritePagedPhyReg(dev,(MAD_U8)portNum, MAD_PAGE1,
                                (MAD_U8)regAddr,(MAD_U16)data)) != MAD_OK)
    {
        MAD_DBG_ERROR(("Paged Phy Register write failed.\n"));
        return (retVal | MAD_API_FAIL_WRITE_PAGED_REG);
    }

    return MAD_OK;
}


/*******************************************************************************
* mdGetPhyQSGMIIReg
*
* DESCRIPTION:
*       This function reads a phy register of QSGMII interface.
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
MAD_STATUS mdGetPhyQSGMIIReg
(
    IN  MAD_DEV    *dev,
    IN  MAD_U32    portNum,
    IN  MAD_U32    regAddr,
    OUT MAD_U32    *data
)
{
    MAD_STATUS  retVal;

    if(MAD_DEV_IS_IN_SWITCH(dev))
    {
        MAD_DBG_ERROR(("The phy is in switch, and do not support the function.\n"));
        return MAD_API_PHY_IN_SWITCH;
    }
    if((retVal=madHwReadPagedPhyReg(dev,(MAD_U8)portNum,MAD_PAGE4,
                                (MAD_U8)regAddr,(MAD_U16*)data)) != MAD_OK)
    {
        MAD_DBG_ERROR(("Paged Phy Register read failed.\n"));
        return (retVal | MAD_API_FAIL_READ_PAGED_REG);
    }

    return MAD_OK;
}


/*******************************************************************************
* mdSetPhyQSGMIIReg
*
* DESCRIPTION:
*       This function writes to a phy register of QSGMII interface.
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
MAD_STATUS mdSetPhyQSGMIIReg
(
    IN  MAD_DEV    *dev,
    IN  MAD_U32    portNum,
    IN  MAD_U32    regAddr,
    IN  MAD_U32    data
)
{
    MAD_STATUS  retVal;

    if(MAD_DEV_IS_IN_SWITCH(dev))
    {
        MAD_DBG_ERROR(("The phy is in switch, and do not support the function.\n"));
        return MAD_API_PHY_IN_SWITCH;
    }
    if((retVal=madHwWritePagedPhyReg(dev,(MAD_U8)portNum, MAD_PAGE4,
                                (MAD_U8)regAddr,(MAD_U16)data)) != MAD_OK)
    {
        MAD_DBG_ERROR(("Paged Phy Register write failed.\n"));
        return (retVal | MAD_API_FAIL_WRITE_PAGED_REG);
    }

    return MAD_OK;
}



