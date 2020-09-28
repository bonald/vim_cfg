#include <madCopyright.h>

/********************************************************************************
* 151xP_ptp.h
*
* DESCRIPTION:
*       API definitions for Marvell 151xP PTP feature
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*
*******************************************************************************/

#ifndef __151xP_PTP_H_
#define __151xP_PTP_H_

#ifdef __cplusplus
extern "C" {
#endif
#define IN 
#define OUT  
#include "madApiTypes.h"
#include "madApiDefs.h"

/****************************************************************************/
/* Exported 151xP PTP Types  			                                    */
/****************************************************************************/

/*
*  typedef: struct PTP_PORT_INTERRUPT_EN
*
*  Description: Port PTP interrupt Enable/Disable
*
*  Fields:
*      ptpArrIntEn      - Port Arrival Interrupt enable     MAD_FALSE:Disable MAD_TRUE:Enable
*      ptpDepIntEn      - Port Departure Interrupt enable   MAD_FALSE:Disable MAD_TRUE:Enable
*/
typedef struct
{
	MAD_BOOL		ptpArrIntEn;
	MAD_BOOL		ptpDepIntEn;
}PTP_PORT_INTERRUPT_EN;

/*
*  typedef: struct PTP_PORT_CONFIG
*
*  Description: PTP configuration parameters for each port
*
*  Fields:
*      transSpec    - This is to differentiate between various timing protocols.
*      disTSpec     - Disable Transport specific check
*      etJump         - offset to Ether type start address in bytes
*      ipJump         - offset to IP header start address counting from Ether type offset
*      ptpArrIntEn    - PTP port arrival interrupt enable
*      ptpDepIntEn    - PTP port departure interrupt enable
*      disTSOverwrite     - disable time stamp counter overwriting until the corresponding
*                          timer valid bit is cleared.
*/
typedef struct
{
	MAD_PTP_SPEC	transSpec;
	MAD_BOOL		disTSpec;
	MAD_U32			etJump;
	MAD_U32			ipJump;
	MAD_BOOL		ptpArrIntEn;
	MAD_BOOL		ptpDepIntEn;
	MAD_BOOL		disTSOverwrite;
	MAD_BOOL        filterAct;    /* Filter 802.1 Act from LEDs */
	MAD_BOOL        hwAccPtp;     /* Hardware Accelerated PTP */
	MAD_BOOL        kpFramwSa;    /* KS = Keep Frame SA */
	MAD_U16         arrTSMode;
	MAD_U8			arrLedCtrl; /*LED control for packets entering the device. */
	MAD_U8			depLedCtrl; /*LED control for packets departing the device. */
}PTP_PORT_CONFIG;

/*
* Typedef: enum PTP_TIME
*
* Description: Defines the PTP Time to be read
*
* Fields:
*      PTP_ARR0_TIME			- PTP Arrival Time 0
*      PTP_ARR1_TIME			- PTP Arrival Time 1
*      PTP_DEP_TIME			- PTP Departure Time
*/
typedef enum
{
	PTP_ARR0_TIME = 0x0,
	PTP_ARR1_TIME = 0x1,
	PTP_DEP_TIME = 0x2
} PTP_TIME;

/*
* Typedef: enum PTP_INT_STATUS
*
* Description: Defines the PTP Port interrupt status for time stamp
*
* Fields:
*      PTP_INT_NORMAL        - No error condition occurred
*      PTP_INT_OVERWRITE     - PTP logic has to process more than one PTP frame
*                                  that needs time stamping before the current read.
*                                Only the latest one is saved.
*      PTP_INT_DROP          - PTP logic has to process more than one PTP frame
*                                  that needs time stamping before the current read.
*                                Only the oldest one is saved.
*
*/
typedef enum
{
	PTP_INT_NORMAL = 0x0,
	PTP_INT_OVERWRITE = 0x1,
	PTP_INT_DROP = 0x2
} PTP_INT_STATUS;

/*
*  typedef: struct PTP_TS_STATUS
*
*  Description: PTP port status of time stamp
*
*  Fields:
*      isValid        - time stamp is valid
*      status        - time stamp error status
*      timeStamped    - time stamp value of a PTP frame that needs to be time stamped
*      ptpSeqId    - sequence ID of the frame whose time stamp information has been captured
*/
typedef struct
{
	MAD_BOOL    isValid;
	MAD_U32    timeStamped;
	MAD_U32    ptpSeqId;
	PTP_INT_STATUS    status;
} PTP_TS_STATUS;

/*
*  typedef: struct PTP_GLOBAL_CONFIG
*
*  Description: PTP global configuration parameters
*
*  Fields:
*      ptpEType    - PTP Ether Type
*      msgIdTSEn     - Message IDs that needs time stamp
*      tsArrPtr     - Time stamp arrival time counter pointer (either Arr0Time or Arr1Time)
*/
typedef struct
{
	MAD_U32    ptpEType;
	MAD_U32    msgIdTSEn;
	MAD_U32    tsArrPtr;
} PTP_GLOBAL_CONFIG;

/*
*  typedef: struct PTP_MODE_CONFIG
*
*  Description: PTP mode configuration parameters
*
*  Fields:
*      altScheme    - Alternate Scheme
*      grandMstr    - Grand Master Enable.
*      oneStep    - OneStep Enable.
*      PTPMode    - PTP Mode as follows:
*                   00 = Boundary Clock
*                   01 = Peer to Peer Transparent Clock
*                   10 = End to End Transparent Clock
*/
typedef struct
{
	MAD_U8    altScheme;
	MAD_U8    grandMstr;
	MAD_U8    oneStep;
	MAD_U8    PTPMode;
} PTP_MODE_CONFIG;

/* PTP global configuration by pointer/data set. */
/* The Pointer bits are used to access the Index registers as follows:
0x00: PTP Mode Register
0x01: PTP External Time Format
*/

typedef enum
{
	PTP_GLOBAL_CONFIG_PTP_MODE_REG = 0x0,
	PTP_GLOBAL_CONFIG_PTP_EXTERNAL_TIME_FORM = 0x1,
} PTP_GLOBAL_CONFIG_POINTER;

/*
*  typedef: struct PTP_EXTERNAL_TIME_FORM
*
*  Description: PTP External Time Format parameters
*
*  Fields:
*      extMask    - External Mask on the lower bits of the selected 1722 time.
*      ext1722Time    - IEEE 1722 Time Format to External switch blocks.
*      extTASel    - Time Array Selection for External switch blocks.
*/
typedef struct
{
	MAD_U8    extMask;
	MAD_U8    ext1722Time;
	MAD_U8    extTASel;
} PTP_EXTERNAL_TIME_FORM;

/* PTP role set. */
/*
0: Set device as slave
1: Set device as master
*/
typedef enum
{
	PTP_ROLE_SLAVE = 0,
	PTP_ROLE_MASTER = 1,
}PTP_MASTER_SLAVE;

/*
*  typedef: struct PTP_INTERRUPT_STATUS
*
*  Description: PTP global interrupt status structure after Opus
*
*  Fields:
*     ptpTrigGenInt;                Trigger generate mode Interrupt
*     ptpEventInt;                  Event Capture Interrupt
*     ptpIntStPortVect;             Precise Time Protocol Interrupt for Ports.
*/
typedef struct
{
	MAD_U8     ptpTrigGenInt;
	MAD_U8     ptpEventInt;
	MAD_U32    ptpIntStPortVect;
} PTP_INTERRUPT_STATUS;

/*
*  typedef: struct PTP_TIME_ARRAY
*
*  Description: PTP Time Array consists of:
*                       10 byte ToD time,
*                       8 byte 1722 time,
*                       4 byte Compensation,
*                       1 byte Domain Number,
*                       And a 1 bit Clock Valid.
*
*  Fields:
*     todLoadPoint:        Time of Day Load Point
*     clkValid:            Clock Valid bit
*     domainNumber:        Domain Number, to be associated with the selected Time Array
*     todNanoseconds:      Time Array Time of Day, Nano second 32 bits
*     todSecondsLow:       Time Array Time of Day, second portion 0-31 bits
*     todSecondsHigh:      Time Array Time of Day, second portion 32-47 bits
*     Nanoseconds1722Low:  Time Array 1722 Time of Day in Nano seconds 0-31 bits
*     Nanoseconds1722High: Time Array 1722 Time of Day in Nano seconds 32-63 bits
*     todCompensation:     Time Array Time of Day Compensation 32 bits
*/
typedef struct
{
	MAD_U32    todLoadPoint;
	MAD_BOOL   clkValid;
	MAD_U8     domainNumber;
	MAD_U32    todNanoseconds;
	MAD_U32    todSecondsLow;
	MAD_U16    todSecondsHigh;
	MAD_U32    Nanoseconds1722Low;
	MAD_U32    Nanoseconds1722High;
	MAD_U32    todCompensation;
}PTP_TIME_ARRAY;

/*
*  typedef: struct PTP_PULSE_STRUCT
*
*  Description: PTP 1pps Pulse parameters
*
*  Fields:
*     ptpPulseWidth; Pulse Width for the 1 Pulse Per Second on the Second signal
*     ptp1ppsWidthRange; Pulse Width Range for the 1 Pulse Per Second on the Second signal
*     ptp1ppsPhase; Phase of the 1 Pulse Per Second on the Second signal
*     ptp1ppsSelect; Select for the 1 Pulse Per Second on the Second signal.
*/
typedef struct
{
	MAD_U8     ptpPulseWidth;
	MAD_U8     ptp1ppsWidthRange;
	MAD_U8     ptp1ppsPhase;
	MAD_U8     ptp1ppsSelect;
} PTP_PULSE_STRUCT;

/*
* Typedef: enum PTP_TOD_STORE_OPERATION
*
* Description: Defines the PTP time array Store Operation type
*
* Fields:
*      PTP_TOD_STORE_COMP           - Store Comp register only to selected TimeArray
*      PTP_TOD_STORE_ALL            - Store All Registers to selected TimeArray
*/
typedef enum
{
	PTP_TOD_STORE_COMP = 0x2,
	PTP_TOD_STORE_ALL = 0x3,
} PTP_TOD_STORE_OPERATION;

/*
*  typedef: struct TAI_EVENT_CONFIG
*
*  Description: TAI event capture configuration parameters
*
*  Fields:
*      eventOverwrite    - event capture overwrite
*      eventCtrStart         - event counter start
Page12 Reg0 bit13
*      eventPhase        - event phase, When 0x1 the active phase of the PTP_EVREQ input
*        is inverted to be active low. When 0x0 the active phase of the PTP_EVREQ input
*        is normal activehigh
*      eventCapIntEn             - event capture interrupt enable
*      capTrigEvent     - Catpture Trig. 1: from waveform generated by PTP_TRIG.
*                                         0: from PTP_EVREQ pin.
*/
typedef struct
{
	MAD_BOOL    eventOverwrite;
	MAD_BOOL    eventCtrStart;
	MAD_BOOL    eventPhase;
	MAD_BOOL    eventCapIntEn;
	MAD_BOOL    capTrigEvent;
} TAI_EVENT_CONFIG;


typedef enum
{
	TAI_TRIG_PERIODIC_PULSE = 0,    /* generate periodic pulse */
	TAI_TRIG_ON_GIVEN_TIME         /* generate purse when
									PTP global time matches with given time */
} TAI_TRIG_MODE;
typedef enum
{
	TAI_MULTI_PTP_SYNC_DISABLE = 0,  /* the EventRequest and TriggerGen interfaces operate normally. */
	TAI_MULTI_PTP_SYNC_ENABLE        /* the logic detects a low to high transition on
										the EventRequest (GPIO) and transfers the value
										in TrigGenAmt[31:0] (TAI Global Config 0x2, 0x3) into
										the PTP Global Time register[31:0]. The EventCapTime[31:0]
										(TAI global Status 0xA, 0xB) is also updated at that
										instant. */
} TAI_MULTI_PTP_SYNC_MODE;
/*
*  typedef: struct TAI_TRIGGER_CONFIG
*
*  Description: TAI trigger generator configuration parameters
*
*  Fields:
*      trigGenIntEn         - Trigger Generation interrupt enable
*      trigPhase     - trigger phase, When 0x1 the active phase of the PTP_TRIG output
*        is inverted to be active low. When 0x0 the active phase of the PTP_TRIG output
*        is normal active high.
*      trigLock      - trigger Lock, When 0x1 the leading edge of PTP_TRIG will be adjudterd in following range.
*      blkUpdate     - Block Update..
*      trigLockRange - trigger Locking range.
*      lockCorrect   - Trig Lock Correction amount
*      trigClkCompDir  - Trigger Clock Comp Direction.
*      clkCompDir      - Clock Comp Direction.
*      pulseWidth      - pulse width in units of TSClkPer.
*                      this value should be 1 ~ 0xF. If it's 0, no changes made.
*                      this value is valid only in MAD_TAI_TRIG_ON_GIVEN_TIME mode.
*      pulseWidthRange - Pulse Width Range for the PTP_TRIG signal.
*      multiSyncMode   - Multi PTP Sync Mode
*      mode        - trigger mode, either MAD_TAI_TRIG_PERIODIC_PULSE or
*                        MAD_TAI_TRIG_ON_GIVEN_TIME
*      trigGenAmt      - if mode is MAD_TAI_TRIG_PERIODIC_PULSE,
*                      this value is used as a clock period in TSClkPer increments
*                      If mode is MAD_TAI_TRIG_ON_GIVEN_TIME,
*                      this value is used to compare with PTP global time.
*      trigClkComp    - trigger mode clock compensation amount in pico sec.
*                      this value is valid only in MAD_TAI_TRIG_PERIODIC_PULSE mode.
*      trigClkCompSub - Trigger mode Clock Compensation Amount in Sub Pico seconds as an
*                       unsigned number.
*      trigGenTime    - Trigger Generation Time.
*/
typedef struct
{
	MAD_BOOL   trigGenIntEn;
	MAD_BOOL   trigPhase;
	MAD_BOOL   trigLock;
	MAD_BOOL   blkUpdate;
	MAD_U8     trigLockRange;
	MAD_U8     lockCorrect;
	MAD_BOOL     trigClkCompDir;
	MAD_U8     pulseWidth;
	MAD_U8     pulseWidthRange;
	TAI_MULTI_PTP_SYNC_MODE  multiSyncMode;
	MAD_TAI_TRIG_MODE            mode;
	MAD_U32    trigGenAmt;
	MAD_U32    trigClkComp;
	MAD_U32    trigClkCompSub;
	MAD_U32    trigGenTime;
} TAI_TRIGGER_CONFIG;

/*
*  typedef: struct TAI_EVENT_STATUS
*
*  Description: TAI event capture status
*
*  Fields:
*      isValid        - eventTime is valid
*      eventTime     - PTP global time when event is registered.
*      eventCtr    - event capture counter. increamented only if eventCtrStart is set.
*      eventErr    - isValid is already set when a new event is observed.
*/
typedef struct
{
	MAD_BOOL    isValid;
	MAD_U32    eventTime;
	MAD_U32    eventCtr;
	MAD_BOOL    eventErr;
} TAI_EVENT_STATUS;

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
);
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
);

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
);

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
);

/*******************************************************************************
* ptpSetInputSrc
*
* DESCRIPTION:
*       This routine sets port PTP input source.
*
* INPUTS:
*       port - The logical port number
*       inSrc  - 1: use LED1 for PTP input trigger pulse after configuration is completed.
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
	IN  MAD_LPORT    port,
	IN  MAD_BOOL     inSrc
);

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
*       inSrc  - 1: use LED1 for PTP input trigger pulse after configuration is completed.
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
	IN  MAD_LPORT    port,
	OUT MAD_BOOL    *inSrc
);

/*******************************************************************************
* ptpSetOutputSrc
*
* DESCRIPTION:
*       This routine sets port PTP output source.
*
* INPUTS:
*       port - The logical port number
*       outSrc  - 1: use LED1 for PTP trigger pulse generate output.
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
	IN  MAD_LPORT    port,
	IN  MAD_BOOL     outSrc
);

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
*       outSrc  - 1: use LED1 for PTP trigger pulse generate output.
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
);

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
);

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
);


/*******************************************************************************
* ptpSetIntEnable
*
* DESCRIPTION:
*       This routine set port PTP interrupt Enable/Disable.
*
* INPUTS:
*       port - The logical port number
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
	IN  MAD_LPORT    port,
	IN  PTP_PORT_INTERRUPT_EN    *interruptEn
);

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
);

/*******************************************************************************
* ptpSetPortPTPEn
*
* DESCRIPTION:
*       This routine enables or disables PTP on a port.
*
* INPUTS:
*       port    - The logical port number
*       en - MAD_TRUE to enable PTP, MAD_FALSE to disable PTP
*
* OUTPUTS:
*       None
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
	IN  MAD_BOOL     en
);
/*******************************************************************************
* ptpGetPortPTPEn
*
* DESCRIPTION:
*       This routine checks if PTP is enabled on a port.
*
* INPUTS:
*       port    - The logical port number
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
);

/*******************************************************************************
* ptpSetPortConfig
*
* DESCRIPTION:
*       This routine writes PTP port configuration parameters.
*
* INPUTS:
*       port    - The logical port number
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
);

/*******************************************************************************
* ptpGetPortConfig
*
* DESCRIPTION:
*       This routine reads PTP configuration parameters for a port.
*
* INPUTS:
*       port    - The logical port number
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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
*       timeToRead    - Arr0, Arr1, or Dep time (MAD_PTP_TIME enum type);
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
);

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
);

/*******************************************************************************
* ptpResetTimeStamp
*
* DESCRIPTION:
*        This routine resets PTP Time valid bit so that PTP logic can time stamp
*        a next PTP frame that needs to be time stamped.
*
* INPUTS:
*       port - logical port number.
*       timeToReset    - Arr0, Arr1, or Dep time (MAD_PTP_TIME enum type);
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
);


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
);

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
);
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
);

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
);

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
);

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
);

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
);


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
);
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
);

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
);

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
);


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
);

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
);


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
);

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
);


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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

/*******************************************************************************
* taiClearEventInt
*
* DESCRIPTION:
*       This routine clear TAI Event Capture Interrupt status.
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
);






#ifdef __cplusplus
}
#endif

#endif
