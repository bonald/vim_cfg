#include <madCopyright.h>

/************************************************************************
* 88E1680.h
*
* DESCRIPTION:
*		API definitions for Marvell Phy 88E1680 88E1680L
*
*
************************************************************************/

#ifndef _88E1680_H
#define _88E1680_H


#include <madApi.h>
#include <madApiInternal.h>
#include <madHwCntl.h>
#include <madDrvPhyRegs.h>
#include <madDrvConfig.h>


MAD_STATUS E1680_Enable_TX_TCLK(IN  MAD_DEV *dev, IN  MAD_LPORT port);

MAD_STATUS E1680_MacSetFifoDepth(IN MAD_DEV *dev, IN MAD_LPORT port, IN MAD_U8 trFifoDepth, IN MAD_U8 reFifoDepth);

MAD_STATUS E1680_MacGetFifoDepth(IN MAD_DEV *dev, IN MAD_LPORT port, OUT MAD_U8 *trFifoDepth, OUT MAD_U8 *reFifoDepth);

MAD_STATUS E1680_SetCRCCheck(IN  MAD_DEV *dev, IN  MAD_LPORT port, IN  MAD_U8 mode);

MAD_STATUS E1680_GetCRCCheck(IN  MAD_DEV *dev, IN  MAD_LPORT port, OUT  MAD_U16 *data);

MAD_STATUS E1680_GetCRCCounter(IN  MAD_DEV *dev, IN  MAD_LPORT port, IN MAD_U16 offset, OUT  MAD_U16 *data);

MAD_STATUS E1680_GetLinkDisconnectCounter(IN  MAD_DEV *dev, IN  MAD_LPORT port, OUT  MAD_U16 *count);

MAD_STATUS E1680_SyncClkSetRClk(IN MAD_DEV *dev, IN MAD_LPORT port, IN MAD_8 rclk, IN MAD_8 freq);

MAD_STATUS E1680_SyncClkGetRClk(IN MAD_DEV *dev, IN MAD_LPORT port, OUT MAD_8 *rclk, OUT MAD_8 *freq);

MAD_STATUS E1680_SyncClkSetClkSelect(IN MAD_DEV *dev, IN MAD_LPORT port, IN MAD_8 interf, IN MAD_8 refClk);

MAD_STATUS E1680_SyncClkGetClkSelect(IN MAD_DEV *dev, IN MAD_LPORT port, IN MAD_8 interf, OUT MAD_8 *refClk);

MAD_STATUS E1680_SysModSoftReset(IN  MAD_DEV *dev, IN  MAD_LPORT port, IN  MAD_U8 mode);

MAD_STATUS E1680_SysSetQSGMIIPwDown(IN MAD_DEV *dev, IN MAD_LPORT port, IN  MAD_BOOL en);

MAD_STATUS E1680_SysGetQSGMIIPwDown(IN MAD_DEV *dev, IN MAD_LPORT port, OUT  MAD_BOOL *en);

MAD_STATUS E1680_SysSetMacPowerDown(IN  MAD_DEV *dev, IN  MAD_LPORT port, IN MAD_U16 data);

MAD_STATUS E1680_SysGetMacPowerDown(IN MAD_DEV *dev, IN MAD_LPORT port, OUT MAD_BOOL *en);

MAD_STATUS E1680_SysSetLEDCtrl(IN MAD_DEV *dev, IN MAD_LPORT port, IN MAD_LED_CTRL_DATA *ledData);

MAD_STATUS E1680_SysGetLEDCtrl(IN MAD_DEV *dev, IN MAD_LPORT port, OUT MAD_LED_CTRL_DATA *ledData);

MAD_STATUS E1680_PatCtrlSetPRBS(IN  MAD_DEV   *dev, IN  MAD_LPORT  port, IN  MAD_PRBS_GEN_CONFIG *genCfg);

MAD_STATUS E1680_GetRxErCapture(IN  MAD_DEV *dev, IN  MAD_LPORT port, IN  MAD_U8 path, OUT MAD_BOOL *capture);

MAD_STATUS E1680_GetRxErByteNun(IN  MAD_DEV *dev, IN  MAD_LPORT port, IN  MAD_U8 path, OUT MAD_U16 *byteNum);

MAD_STATUS E1680_GetRxEr(IN  MAD_DEV *dev, IN  MAD_LPORT port, IN  MAD_U8 path, OUT MAD_U8 *rxEr);

MAD_STATUS E1680_GetRxDv(IN  MAD_DEV *dev, IN  MAD_LPORT port, IN  MAD_U8 path, OUT MAD_BOOL *rxDv);

MAD_STATUS E1680_GetRxData(IN  MAD_DEV *dev, IN  MAD_LPORT port, IN  MAD_U8 path, OUT MAD_U8 *rxData);

#endif