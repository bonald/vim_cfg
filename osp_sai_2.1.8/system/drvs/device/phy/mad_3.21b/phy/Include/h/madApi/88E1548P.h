#include <madCopyright.h>

/************************************************************************
* 88E1548P.h
*
* DESCRIPTION:
*		API definitions for Marvell Phy 88E1548P
*
*
************************************************************************/

#ifndef _88E1548P_H
#define _88E1548P_H


#include <madApi.h>
#include <madApiInternal.h>
#include <madHwCntl.h>
#include <madDrvPhyRegs.h>
#include <madDrvConfig.h>

MAD_STATUS E1548P_SyncClkSetClkSelect(IN MAD_DEV *dev, IN MAD_LPORT port, IN MAD_8 interf, IN MAD_8 refClk);
MAD_STATUS E1548P_SyncClkGetClkSelect(IN MAD_DEV *dev, IN MAD_LPORT port, IN MAD_8 interf, OUT MAD_8 *refClk);
MAD_STATUS E1548P_SysModSoftReset(IN MAD_DEV *dev, IN MAD_LPORT port, IN MAD_U8 mode);

#endif