#include <madCopyright.h>

/************************************************************************
* 88E1780.h
*
* DESCRIPTION:
*		API definitions for Marvell Phy 88E1780
*
*
************************************************************************/

#ifndef _88E1780_H
#define _88E1780_H


#include <madApi.h>
#include <madApiInternal.h>
#include <madHwCntl.h>
#include <madDrvPhyRegs.h>
#include <madDrvConfig.h>


MAD_STATUS E1780_Enable_TX_TCLK(IN  MAD_DEV *dev, IN  MAD_LPORT port);

MAD_STATUS E1780_MacSetFifoDepth(IN MAD_DEV *dev, IN MAD_LPORT port, IN MAD_U8 trFifoDepth, IN MAD_U8 reFifoDepth);

MAD_STATUS E1780_MacGetFifoDepth(IN MAD_DEV *dev, IN MAD_LPORT port, OUT MAD_U8 *trFifoDepth, OUT MAD_U8 *reFifoDepth);

MAD_STATUS E1780_SetCRCCheck(IN  MAD_DEV *dev, IN  MAD_LPORT port, IN  MAD_U8 mode);

MAD_STATUS E1780_GetCRCCheck(IN  MAD_DEV *dev, IN  MAD_LPORT port, OUT  MAD_U16 *data);

MAD_STATUS E1780_GetCRCCounter(IN  MAD_DEV *dev, IN  MAD_LPORT port, IN MAD_U16 offset, OUT  MAD_U16 *data);

MAD_STATUS E1780_GetRxErCapture(IN  MAD_DEV *dev, IN  MAD_LPORT port, IN  MAD_U8 path, OUT MAD_BOOL *capture);

MAD_STATUS E1780_GetRxErByteNum(IN  MAD_DEV *dev, IN  MAD_LPORT port, IN  MAD_U8 path, OUT MAD_U16 *byteNum);

MAD_STATUS E1780_GetRxEr(IN  MAD_DEV *dev, IN  MAD_LPORT port, IN  MAD_U8 path, OUT MAD_U8 *rxEr);

MAD_STATUS E1780_GetRxDv(IN  MAD_DEV *dev, IN  MAD_LPORT port, IN  MAD_U8 path, OUT MAD_BOOL *rxDv);

MAD_STATUS E1780_GetRxData(IN  MAD_DEV *dev, IN  MAD_LPORT port, IN  MAD_U8 path, OUT MAD_U8 *rxData);

MAD_STATUS E1780_GetLinkDisconnectCounter(IN  MAD_DEV *dev, IN  MAD_LPORT port, OUT  MAD_U16 *count);

MAD_STATUS E1780_SyncClkSetRClk(IN MAD_DEV *dev, IN MAD_LPORT port, IN MAD_8 rclk, IN MAD_8 freq);

MAD_STATUS E1780_SyncClkGetRClk(IN MAD_DEV *dev, IN MAD_LPORT port, OUT MAD_8 *rclk, OUT MAD_8 *freq);

MAD_STATUS E1780_SyncClkSetClkSelect(IN MAD_DEV *dev, IN MAD_LPORT port, IN MAD_8 interf, IN MAD_8 refClk);

MAD_STATUS E1780_SyncClkGetClkSelect(IN MAD_DEV *dev, IN MAD_LPORT port, IN MAD_8 interf, OUT MAD_8 *refClk);

MAD_STATUS E1780_SysModSoftReset(IN  MAD_DEV *dev, IN  MAD_LPORT port, IN  MAD_U8 mode);

MAD_STATUS E1780_SysSetQSGMIIPwDown(IN MAD_DEV *dev, IN MAD_LPORT port, IN  MAD_BOOL en);

MAD_STATUS E1780_SysGetQSGMIIPwDown(IN MAD_DEV *dev, IN MAD_LPORT port, OUT  MAD_BOOL *en);

MAD_STATUS E1780_SysSetMacPowerDown(IN  MAD_DEV *dev, IN  MAD_LPORT port, IN MAD_U16 data);

MAD_STATUS E1780_SysGetMacPowerDown(IN MAD_DEV *dev, IN MAD_LPORT port, OUT MAD_BOOL *en);

MAD_STATUS E1780_PatCtrlSetPRBS(IN  MAD_DEV   *dev, IN  MAD_LPORT  port, IN  MAD_PRBS_GEN_CONFIG *genCfg);

MAD_STATUS E1780_GetRxErCapture(IN  MAD_DEV *dev, IN  MAD_LPORT port, IN  MAD_U8 path, OUT MAD_BOOL *capture);

MAD_STATUS E1780_GetRxErByteNun(IN  MAD_DEV *dev, IN  MAD_LPORT port, IN  MAD_U8 path, OUT MAD_BOOL *byteNum);

MAD_STATUS E1780_GetRxEr(IN  MAD_DEV *dev, IN  MAD_LPORT port, IN  MAD_U8 path, OUT MAD_U8 *rxEr);

MAD_STATUS E1780_GetRxDv(IN  MAD_DEV *dev, IN  MAD_LPORT port, IN  MAD_U8 path, OUT MAD_BOOL *rxDv);

MAD_STATUS E1780_GetRxData(IN  MAD_DEV *dev, IN  MAD_LPORT port, IN  MAD_U8 path, OUT MAD_U8 *rxData);

MAD_STATUS E1780_SysSetEnergyDetect(IN MAD_DEV *dev, IN MAD_LPORT port, IN MAD_U16 mode);

MAD_STATUS E1780_SysGetEnergyDetect(IN MAD_DEV *dev, IN MAD_LPORT port, OUT MAD_U16 *mode);

MAD_STATUS E1780_PatCtrlSetPRBS(IN MAD_DEV *dev, IN MAD_LPORT port, IN MAD_PRBS_GEN_CONFIG *genCfg);

MAD_STATUS E1780_PatCtrlGetPRBS(IN MAD_DEV *dev, IN MAD_LPORT port, OUT MAD_PRBS_GEN_CONFIG *genCfg);

MAD_STATUS E1780_ResetPRBSCounters(IN MAD_DEV *dev, IN MAD_LPORT  port);

MAD_STATUS E1780_SetCopperVod(IN MAD_DEV *dev, IN MAD_LPORT port, IN MAD_SPEED_MODE mode, IN MAD_U16 data);


#endif