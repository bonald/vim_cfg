#include <madCopyright.h>

/*******************************************************************************
* Sample_151xP_Ptp.c
*
* DESCRIPTION:
*       Ptp  Sample for 151xP device
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*******************************************************************************/
#define MSG_PRINT(x) printf x

/*Suppose the ToD time is loaded at loadPoint time = Current Global time + TOD_LOAD_DELAY ns*/
#define TOD_LOAD_DELAY (2 * 1000000000)
/*Suppose the divice PTP module using the internal 125MHz clock, so the clock period is 8 ns*/
#define TAI_CLOCK_PERIOD 8

#include "madApi.h"
#include "151xP_ptp.h"
#include "time.h"

extern MAD_STATUS madHwWritePagedPhyReg
(
IN  MAD_DEV    *dev,
IN  MAD_U8     portNum,
IN  MAD_U16    pageNum,
IN  MAD_U8     regAddr,
IN  MAD_U16    data
);

typedef struct {
	MAD_U64 seconds;
	MAD_U32 nanoseconds;
}TimeInfo;

/********************************************************************************
* setCurrentTime
*
*	This sample API will demonstrate how to load one time array to device:
*	Suppose the divice PTP module using the internal 125MHz clock, so the Global Time is incremented of 8ns
*	The time array load occurs at the instant the PTP Global Time matched the load point time
*
*******************************************************************************/
static MAD_STATUS setCurrentTime(MAD_DEV *dev, MAD_U32 phyAddr, MAD_U8 timeArrayIndex, MAD_U8 domainNumber, TimeInfo *timeinfo)
{
	MAD_STATUS status = MAD_OK;
	PTP_TIME_ARRAY timeArray;
	MAD_BOOL busyStatus;
	TimeInfo temptime;
	

	temptime.nanoseconds = (timeinfo->nanoseconds + TOD_LOAD_DELAY) % 1000000000;
	temptime.seconds = timeinfo->seconds + (timeinfo->nanoseconds + TOD_LOAD_DELAY) / 1000000000;
	
	/*Initialise the time array*/
	timeArray.clkValid = MAD_TRUE;
	timeArray.domainNumber = domainNumber;
	timeArray.todSecondsHigh = (temptime.seconds >> 32) & 0xffff;
	timeArray.todSecondsLow = temptime.seconds & 0xffffffff;
	timeArray.todNanoseconds = temptime.nanoseconds;
	timeArray.Nanoseconds1722High = 0;
	timeArray.Nanoseconds1722Low = 0;
	timeArray.todCompensation = 0;
	/*Get the current Global time and add a delay for loadPoint, the value is incremented onece by 8n, so the loadPoint value = TOD_LOAD_DELAY / TAI_CLOCK_PERIOD*/
	taiGetPTPGlobalTime(dev, phyAddr, &timeArray.todLoadPoint);
	timeArray.todLoadPoint = timeArray.todLoadPoint + TOD_LOAD_DELAY / TAI_CLOCK_PERIOD;

	/*Only one ToD operation can be executing at one time, so the busy bit must be zero before any ToD operation*/
	do
	{
		ptpTODGetBusyBitStatus(dev, phyAddr, &busyStatus);
	} while (busyStatus);
	if ((status = ptpTODStoreAll(dev, phyAddr, timeArrayIndex, &timeArray)) != MAD_OK)
	{
		MSG_PRINT(("Load time array error, error code: %d\n", status));
		return status;
	}
	/*When the requested ToD operation completes, the busy bit will automatically be cleared to a zero, here wait busy bit to make sure the ToD operation have completed*/
	do
	{
		ptpTODGetBusyBitStatus(dev, phyAddr, &busyStatus);
	} while (busyStatus);

	return status;
}

/********************************************************************************
* getCurrentTime
*
*	This sample API will demonstrate how to get time array form device:
*	This is sample code and using the time info struct which accuracy is second based, this should be modified based on real system to support Higher accuracy
*
*******************************************************************************/
static MAD_STATUS getCurrentTime(MAD_DEV *dev, MAD_U32 phyAddr, MAD_U8 timeArrayIndex, TimeInfo *timeinfo)
{
	MAD_STATUS status = MAD_OK;
	PTP_TIME_ARRAY timeArray;
	MAD_BOOL busyStatus;

	/*Only one ToD operation can be executing at one time, so the busy bit must be zero before any ToD operation*/
	do
	{
		ptpTODGetBusyBitStatus(dev, phyAddr, &busyStatus);
	} while (busyStatus);
	if ((status = ptpTODCaptureAll(dev, phyAddr, timeArrayIndex, &timeArray)) != MAD_OK)
	{
		MSG_PRINT(("Load time array error, error code: %d\n", status));
		return status;
	}
	/*When the requested ToD operation completes, the busy bit will automatically be cleared to a zero, here wait busy bit to make sure the ToD operation have completed*/
	do
	{
		ptpTODGetBusyBitStatus(dev, phyAddr, &busyStatus);
	} while (busyStatus);

	timeinfo->nanoseconds = timeArray.todNanoseconds;
	timeinfo->seconds = timeArray.todSecondsHigh;
	timeinfo->seconds = (timeinfo->seconds << 32) | timeArray.todSecondsLow;

	return status;
}

/********************************************************************************
* adjustTime
*
*	This sample API will demonstrate how to adjust device time of day based on master's time:
*	This is sample code and using the time info struct which accuracy is second based, this should be modified based on real system to support Higher accuracy
*	timeoffset - nanoseconds based
*	direction - 0 = Local time is slow than master time
*				1 = Local time is fast than master time
*
*******************************************************************************/
static MAD_STATUS adjustTime(MAD_DEV *dev, MAD_U32 phyAddr, MAD_U8 timeArrayIndex, MAD_BOOL direction, MAD_U64 timeoffset)
{
	MAD_STATUS status = MAD_OK;
	PTP_TIME_ARRAY timeArray;
	MAD_BOOL busyStatus;
	MAD_U64 temp;

	/*	///////*Adjust the ToD based on the delta*////// */
	/*Get current ToD firstly*/
	/*Only one ToD operation can be executing at one time, so the busy bit must be zero before any ToD operation*/
	do
	{
		ptpTODGetBusyBitStatus(dev, phyAddr, &busyStatus);
	} while (busyStatus);
	if ((status = ptpTODCaptureAll(dev, phyAddr, timeArrayIndex, &timeArray)) != MAD_OK)
	{
		MSG_PRINT(("Load time array error, error code: %d\n", status));
		return status;
	}
	/*When the requested ToD operation completes, the busy bit will automatically be cleared to a zero, here wait busy bit to make sure the ToD operation have completed*/
	do
	{
		ptpTODGetBusyBitStatus(dev, phyAddr, &busyStatus);
	} while (busyStatus);

	timeArray.todLoadPoint = timeArray.todLoadPoint + TOD_LOAD_DELAY / TAI_CLOCK_PERIOD;
	temp = timeArray.todSecondsHigh;
	if (direction == 0)
		temp = (temp << 32 | timeArray.todSecondsLow) * 1000000000 + timeArray.todNanoseconds + timeoffset + TOD_LOAD_DELAY;
	else
		temp = (temp << 32 | timeArray.todSecondsLow) * 1000000000 + timeArray.todNanoseconds - timeoffset + TOD_LOAD_DELAY;

	timeArray.todNanoseconds = temp % 1000000000;
	timeArray.todSecondsHigh = (MAD_U16)(((temp / 1000000000) >> 32) & 0xffffffff);
	timeArray.todSecondsLow = (temp / 1000000000) & 0xffffffff;

	/*Only one ToD operation can be executing at one time, so the busy bit must be zero before any ToD operation*/
	do
	{
		ptpTODGetBusyBitStatus(dev, phyAddr, &busyStatus);
	} while (busyStatus);
	if ((status = ptpTODStoreAll(dev, phyAddr, timeArrayIndex, &timeArray)) != MAD_OK)
	{
		MSG_PRINT(("Load time array error, error code: %d\n", status));
		return status;
	}
	/*When the requested ToD operation completes, the busy bit will automatically be cleared to a zero, here wait busy bit to make sure the ToD operation have completed*/
	do
	{
		ptpTODGetBusyBitStatus(dev, phyAddr, &busyStatus);
	} while (busyStatus);

	return status;
}

/********************************************************************************
* adjustFrequency
*
*	This sample API will demonstrate how to adjust PPM difference between the local crystal clocking this PTP block and the PTP Grand Master device that this Time Array is tracking
*	Suppose the divice PTP module using the internal 125MHz clock
*	The compensation register is incremented of 465.661 zeptoseconds(4.65661E-19 seconds). A difference of 1 PPM for a 125 MHz local PTP clock is 8 femtoseconds(8.0E-15 seconds)
*	And the value of compensation register should be 0x431E
*	direction - 0 = Local clock is slow than master clock
*				1 = Local clock is fast than master clock
*
*******************************************************************************/
static MAD_STATUS adjustFrequency(MAD_DEV *dev, MAD_U32 phyAddr, MAD_U8 timeArrayIndex, MAD_BOOL direction, MAD_U32 ppm)
{
	MAD_STATUS status = MAD_OK;
	MAD_BOOL busyStatus;
	PTP_TIME_ARRAY timeArray;
	MAD_U32 temp;

	temp = ppm * 0x431E >= 0x80000000 ? 0x7fffffff : ppm * 0x431E;
	timeArray.clkValid = MAD_TRUE;
	if (direction == 0)
	{
		timeArray.todCompensation = temp;
	}
	else
	{
		timeArray.todCompensation = 0x80000000 | temp;
	}

	/*Only one ToD operation can be executing at one time, so the busy bit must be zero before any ToD operation*/
	do
	{
		ptpTODGetBusyBitStatus(dev, phyAddr, &busyStatus);
	} while (busyStatus);
	if ((status = ptpTODStoreCompensation(dev, phyAddr, timeArrayIndex, &timeArray)) != MAD_OK)
	{
		MSG_PRINT(("Load time array error, error code: %d\n", status));
		return status;
	}
	/*When the requested ToD operation completes, the busy bit will automatically be cleared to a zero, here wait busy bit to make sure the ToD operation have completed*/
	do
	{
		ptpTODGetBusyBitStatus(dev, phyAddr, &busyStatus);
	} while (busyStatus);

	return status;
}


/********************************************************************************
* ptpDevInit
*
*	This sample API will demonstrate how to init the PTP engine:
*	1. Power up device¡¯s PTP engine
*	2. Configure using the internal clock or external clock for the PTP core
*	3. Load ToD with inital system time for domain X (0~3)
*	4. Configure device to be Ordinary Clock, Peer to Peer Transparent Clock, or End to End Transparent Clock 
*	   in the mode of Master or slave, one-step or two-step
*	5. Configure Per-Port setting, e.g., support 802.1as frame only, support HW accelerated time-stamp mode
*	6. Configure PTP global setting, e.g., program PTPEType as 0x88F7 (support IEEE 1588v2/802.1as layer 2 PTP frame),
*	   enable time stamp for PTP frame, using port Arr0 Time
*	7. Configure PTP Ingress and Egress path delay asymmetry, the value should be collected from real system, here set 0 for sample 
*
*******************************************************************************/
MAD_STATUS ptpDevInit(MAD_DEV *dev)
{
	MAD_STATUS status = MAD_OK;
	MAD_LPORT port = 0;
	MAD_BOOL busyStatus;
	MAD_U8 timeArrayIndex;	
	PTP_TIME_ARRAY timeArray;
	PTP_MODE_CONFIG ptpMode;
	PTP_PORT_CONFIG ptpPortCfg;
	PTP_GLOBAL_CONFIG ptpGlobalCfg;
	

	/*Power up device's PTP engine*/
	if ((status = ptpSetEnable(dev, port, MAD_FALSE)) != MAD_OK)
	{
		MSG_PRINT(("Power down the PTP engine error, error code: %d\n", status));
		return status;
	}
	if ((status = ptpSetEnable(dev, port, MAD_TRUE)) != MAD_OK)
	{
		MSG_PRINT(("Power up the PTP engine error, error code: %d\n", status));
		return status;
	}

	/*Configure using internal clock*/
	if ((status = ptpSetRefClkSrc(dev, port, MAD_FALSE)) != MAD_OK)
	{
		MSG_PRINT(("Set to use internal clock error, error code: %d\n", status));
		return status;
	}

	/*Load one time array for domain 0*/
	timeArrayIndex = 0;
	timeArray.clkValid = MAD_TRUE;
	timeArray.domainNumber = 0;
	timeArray.Nanoseconds1722High = 0;
	timeArray.Nanoseconds1722Low = 0;
	timeArray.todCompensation = 0;	
	timeArray.todNanoseconds = 0;
	timeArray.todSecondsHigh = 0;
	timeArray.todSecondsLow = 0;
	/*Get the current Global time and add a delay for loadPoint*/
	taiGetPTPGlobalTime(dev, port, &timeArray.todLoadPoint);
	timeArray.todLoadPoint = timeArray.todLoadPoint + TOD_LOAD_DELAY / TAI_CLOCK_PERIOD;

	/*Only one ToD operation can be executing at one time, so the busy bit must be zero before any ToD operation*/
	do
	{
		ptpTODGetBusyBitStatus(dev, port, &busyStatus);
	} while (busyStatus);
	if ((status = ptpTODStoreAll(dev, port, timeArrayIndex, &timeArray)) != MAD_OK)
	{
		MSG_PRINT(("Load time array error, error code: %d\n", status));
		return status;
	}
	/*When the requested ToD operation completes, the busy bit will automatically be cleared to a zero, here wait busy bit to make sure the ToD operation have completed*/
	do
	{
		ptpTODGetBusyBitStatus(dev, port, &busyStatus);
	} while (busyStatus);

	/*Configure device to be Ordinary Clock, slave mode and two-step*/
	ptpMode.altScheme = 0;
	ptpMode.grandMstr = 0;
	ptpMode.oneStep = 0;
	ptpMode.PTPMode = 0;
	if ((status = ptpSetMode(dev, port, &ptpMode)) != MAD_OK)
	{
		MSG_PRINT(("Set PTP mode error, error code: %d\n", status));
		return status;
	}

	/*Configure device to support 802.1as abd HW accelerated*/
	if ((status = ptpGetPortConfig(dev, port, &ptpPortCfg)) != MAD_OK)
	{
		MSG_PRINT(("Get PTP port config error, error code: %d\n", status));
		return status;
	}
	ptpPortCfg.transSpec = 1;
	ptpPortCfg.hwAccPtp = 1;
	if ((status = ptpSetPortConfig(dev, port, &ptpPortCfg)) != MAD_OK)
	{
		MSG_PRINT(("Set PTP port config error, error code: %d\n", status));
		return status;
	}

	/*Program PTPEType as 0x88F7, enable time stamp and using port Arr0 time*/
	ptpGlobalCfg.ptpEType = 0x88F7;
	ptpGlobalCfg.msgIdTSEn = 0xFFFF;
	ptpGlobalCfg.tsArrPtr = 0;
	if ((status = ptpSetGlobalConfig(dev, port, &ptpGlobalCfg)) != MAD_OK)
	{
		MSG_PRINT(("Set PTP Global config error, error code: %d\n", status));
		return status;
	}

	/*Configure the Ingress and Egress path delay asymmetry*/
	if ((status = ptpSetIgrPathDelayAsym(dev, port, 0)) != MAD_OK)
	{
		MSG_PRINT(("Set Ingress path delay asymmetry error, error code: %d\n", status));
		return status;
	}
	if ((status = ptpSetEgrPathDelayAsym(dev, port, 0)) != MAD_OK)
	{
		MSG_PRINT(("Set Egress path delay asymmetry error, error code: %d\n", status));
		return status;
	}

	return status;
}

/********************************************************************************
* ptpWorkAround
*
*	In E1512 validation, after register programming, sometimes the first PTP frame isn¡¯t being processed 
*	correctly i.e. not being recognized as a PTP frame.
*	If one frame (it can be any frame) can be sent before beginning the PTP protocol, there would be no issue. 
*	This one extra frame is required under 2 situations:
*		1.	Whenever PTP is enabled (since in PHY PTP isn¡¯t enabled at powerup) and before sending PTP protocol frames.
*		2.	After any write to ¡®PTPEtherType¡¯, ¡®MsgTypeTSEn¡¯, ¡®TransSpec¡¯, and ¡®DisableTransSpecChk¡¯ registers occurs and before sending PTP protocol frames
*
*	To run this workaround, must connect one loopback stub to the test port and run the procedure as follows:
*		1.  Enable external loopback
*		2.  Chenck the link status, then send one packet using internal packet generator 
*		3.  Disable the packet generator
*		4.  Disable the external loopback
*	Note:
*		Prefer to call this API after the ptpDevInit()
*******************************************************************************/
MAD_STATUS ptpWorkAround(MAD_DEV *dev)
{
	MAD_STATUS status;
	MAD_LPORT port = 0;
	MAD_SPEED_MODE mode = MAD_SPEED_1000M;
	MAD_BOOL linkOn = MAD_FALSE;
	char temp;

	printf("Finish the ptpDevInit and connect the external loopback stub to the port?(y/n)\n");
	scanf(" %c", &temp);

	if ((temp == 'y') || (temp == 'Y'))
	{
		if ((status = mdDiagSetExternalLoopback(dev, port, MAD_TRUE, mode)) != MAD_OK)
		{
			MSG_PRINT(("Enable the External Loopback error, error code: %d\n", status));
			return status;
		}

		do
		{
			mdGetLinkStatus(dev, port, &linkOn);
		} while (!linkOn);

		if ((status = madHwWritePagedPhyReg(dev, port, 18, 16, 0x142)) != MAD_OK)
		{
			MSG_PRINT(("Enable the PacketGen error, error code: %d\n", status));
			return status;
		}

		if ((status = madHwWritePagedPhyReg(dev, port, 18, 16, 0)) != MAD_OK)
		{
			MSG_PRINT(("Disable the PacketGen error, error code: %d\n", status));
			return status;
		}
		if ((status = mdDiagSetExternalLoopback(dev, port, MAD_FALSE, mode)) != MAD_OK)
		{
			MSG_PRINT(("Disable the External Loopback error, error code: %d\n", status));
			return status;
		}
	}
	
	return status;
}

/********************************************************************************
* becomeMaster
*
*	This sample API will set the device to Master	
*
*******************************************************************************/
MAD_STATUS becomeMaster(MAD_DEV *dev)
{
	MAD_STATUS status = MAD_OK;
	MAD_U32 port = 0;
	PTP_MASTER_SLAVE role = PTP_ROLE_MASTER;

	if ((status = ptpSetMasterSlave(dev, port, role)) != MAD_OK)
	{
		MSG_PRINT(("Set device to Master error, error code: %d\n", status));
		return status;
	}

	return status;
}

/********************************************************************************
* becomeSlave
*
*	This sample API will set the device to Slave
*
*******************************************************************************/
MAD_STATUS becomeSlave(MAD_DEV *dev)
{
	MAD_STATUS status = MAD_OK;
	MAD_U32 port = 0;
	PTP_MASTER_SLAVE role = PTP_ROLE_SLAVE;

	if ((status = ptpSetMasterSlave(dev, port, role)) != MAD_OK)
	{
		MSG_PRINT(("Set device to Slave error, error code: %d\n", status));
		return status;
	}

	return status;
}

/********************************************************************************
* ptpSetCurTime
*
*	This sample API will demonstrate how to set current ToD (timeofday) in domain 0
*	This is sample code and using the time info struct which accuracy is second based, this should be modified based on real system to support Higher accuracy
*	1. Get the current time, this sample using the c function time() to get the system time
*	2. Initialise timeArrayIndex and domainNumber as 0 for sample
*	3. Store the time array to device
*
*******************************************************************************/
MAD_STATUS ptpSetCurTime(MAD_DEV *dev)
{
	MAD_STATUS status = MAD_OK;
	MAD_U32 phyAddr;
	MAD_U8 timeArrayIndex;
	MAD_U8 domainNumber;
	time_t rawtime;
	TimeInfo timeinfo;

	phyAddr = 0;
	timeArrayIndex = 0;
	domainNumber = 0;
	time(&rawtime);
	timeinfo.seconds = rawtime;
	timeinfo.nanoseconds = 0;
	
	status = setCurrentTime(dev, phyAddr, timeArrayIndex, domainNumber, &timeinfo);

	return status;
}

/********************************************************************************
* ptpGetCurTime
*
*	This sample API will demonstrate how to get current ToD (timeofday) in domain 0
*
*******************************************************************************/
MAD_STATUS ptpGetCurTime(MAD_DEV *dev)
{
	MAD_STATUS status = MAD_OK;
	MAD_U32 phyAddr;
	MAD_U8 timeArrayIndex;
	TimeInfo timeinfo;
	struct tm *ptime;

	phyAddr = 0;
	timeArrayIndex = 0;
	
	status = getCurrentTime(dev, phyAddr, timeArrayIndex, &timeinfo);

	ptime = localtime(&timeinfo.seconds);
	MSG_PRINT(("ToD time: %d-%d-%d %d:%d:%d  %d ns\n", ptime->tm_year + 1900, ptime->tm_mon + 1, ptime->tm_mday, 
		ptime->tm_hour, ptime->tm_min, ptime->tm_sec, timeinfo.nanoseconds));
	return status;
}

/********************************************************************************
* ptpAdjTime
*
*	This sample API will demonstrate how to adjust the ToD
*	This sample using the c function time() to get the system time, real system should get the time form grand master
*	Suppose our device time is slower than master time with offset = 1 second
*
*******************************************************************************/
MAD_STATUS ptpAdjTime(MAD_DEV *dev)
{
	MAD_STATUS status = MAD_OK;
	MAD_U32 phyAddr;
	MAD_U8 timeArrayIndex;
	MAD_BOOL direction;
	MAD_U64 offset;

	timeArrayIndex = 0;
	phyAddr = 0;
	direction = 0;
	offset = 1000000000;

	status = adjustTime(dev, phyAddr, timeArrayIndex, direction, offset);

	return status;
}

/********************************************************************************
* ptpAdjFreq
*
*	This sample API will demonstrate how to adjust the PPM difference between the local crystal clocking this PTP block and the PTP Grand Master device that this Time Array is tracking
*	Suppose our device clock is faster than master clock with 20 PPM difference
*******************************************************************************/
MAD_STATUS ptpAdjFreq(MAD_DEV *dev)
{
	MAD_STATUS status = MAD_OK;
	MAD_U32 phyAddr;
	MAD_U8 timeArrayIndex;
	int ppm, direction;

	timeArrayIndex = 0;
	phyAddr = 0;
	direction = 1;
	ppm = 20;

	status = adjustFrequency(dev, phyAddr, timeArrayIndex, direction, ppm);

	return status;
}

/********************************************************************************
* ptpGen1PPS
*
*	This sample API will demonstrate how to generate one pulse per second based on time array 0
*	1. Enable the 1PPS output source (LED0)
*	2. Configure device to generate one pulse per second
*	   Configure the 1PPS width as 0xf and width Range as 0x7, so the pulse width is 15*16.777ms = 251.655ms
*	   Select the time array 0, make sure the time array 0 is actived
*	   The selected 1 PPS signal is available on LED[0] pin
*
*******************************************************************************/
MAD_STATUS ptpGen1PPS(MAD_DEV *dev)
{
	MAD_STATUS status = MAD_OK;
	MAD_LPORT port = 0;
	PTP_PULSE_STRUCT pulsePara;

	/*Enable the 1PPS output source*/
	if ((status = ptpSet1PPSOutputSrc(dev, port, MAD_TRUE)) != MAD_OK)
	{
		MSG_PRINT(("Enable 1PPS Output source error, error code: %d\n", status));
		return status;
	}

	/*Configure device to generate one pulse per second*/
	pulsePara.ptpPulseWidth = 0xf;
	pulsePara.ptp1ppsWidthRange = 0x7;
	pulsePara.ptp1ppsPhase = 0; /*the leading edge of the 1 PPS signal is the rising edge of the signal*/
	pulsePara.ptp1ppsSelect = 0;
	if ((status = ptpPulseSet(dev, port, &pulsePara)) != MAD_OK)
	{
		MSG_PRINT(("Configure 1PPS error, error code: %d\n", status));
		return status;
	}

	return status;
}

/********************************************************************************
* taiTriggerGen
*
*	This sample API will demonstrate how to generate 50% duty cycle signal
*	1. Enable the LED[1] for PTP output trigger signal
*	2. Configure the device to generate the 50% duty cycle signal
*	   Set the trigger amount as 125000000, so the signal cycle is 125000000 * 8ns = 1s
*	   Configure the startTime as 100, when the PTP global time match, the signal will be generated
*	   The signal is avalilable on LED[1] pin
*
*******************************************************************************/
MAD_STATUS taiTriggerGen(MAD_DEV *dev)
{
	MAD_STATUS status = MAD_OK;
	MAD_LPORT port = 0;
	TAI_TRIGGER_CONFIG taiTriggerCfg;
	MAD_BOOL trigEn;

	/*Enable the PTP output source*/
	if ((status = ptpSetOutputSrc(dev, port, MAD_TRUE)) != MAD_OK)
	{
		MSG_PRINT(("Set PTP output source error, error code: %d\n", status));
		return status;
	}
	/*Disable the PTP input source*/
	if ((status = ptpSetInputSrc(dev, port, MAD_FALSE)) != MAD_OK)
	{
		MSG_PRINT(("Set PTP input source error, error code: %d\n", status));
		return status;
	}

	/*Configure the device to generate the 50% duty cycle signal*/
	if ((status = taiGetTrigConfig(dev, port, &trigEn, &taiTriggerCfg)) != MAD_OK)
	{
		MSG_PRINT(("Get TAI Trigger configure error, error code: %d\n", status));
		return status;
	}
	taiTriggerCfg.mode = MAD_TAI_TRIG_PERIODIC_PURSE;
	taiTriggerCfg.trigGenAmt = 125000000;/*decide clock period = 125000000 * PTP_CLOCK_PERIOD*/
	taiTriggerCfg.trigGenTime = 100; /*decide when the 1st leading edge of PTP_TRIG occurs*/
	trigEn = MAD_TRUE;
	if ((status = taiSetTrigConfig(dev, port, trigEn, &taiTriggerCfg)) != MAD_OK)
	{
		MSG_PRINT(("Get TAI Trigger configure error, error code: %d\n", status));
		return status;
	}

	return status;
}

/********************************************************************************
* taiPulseGen
*
*	This sample API will demonstrate how to generate pulse when PTP global time matches with given time
*	1. Enable the LED[1] for PTP output trigger signal 
*	2. Configure the device to generate pulse
*	   Configure the width as 0xf and width Range as 0x7, so the pulse width is 15*16.777ms = 251.655ms
*	   Configure the startTime as 100, when the PTP global time match, the pulse will be generated
*	   The signal is avalilable on LED[1] pin
*
*******************************************************************************/
MAD_STATUS taiPulseGen(MAD_DEV *dev)
{
	MAD_STATUS status = MAD_OK;
	MAD_LPORT port = 0;
	TAI_TRIGGER_CONFIG taiTriggerCfg;
	MAD_BOOL trigEn;

	/*Enable the PTP output source*/
	if ((status = ptpSetOutputSrc(dev, port, MAD_TRUE)) != MAD_OK)
	{
		MSG_PRINT(("Set PTP output source error, error code: %d\n", status));
		return status;
	}
	/*Disable the PTP input source*/
	if ((status = ptpSetInputSrc(dev, port, MAD_FALSE)) != MAD_OK)
	{
		MSG_PRINT(("Set PTP input source error, error code: %d\n", status));
		return status;
	}

	/*Configure the device to generate the 50% duty cycle signal*/
	if ((status = taiGetTrigConfig(dev, port, &trigEn, &taiTriggerCfg)) != MAD_OK)
	{
		MSG_PRINT(("Get TAI Trigger configure error, error code: %d\n", status));
		return status;
	}
	taiTriggerCfg.mode = MAD_TAI_TRIG_ON_GIVEN_TIME;
	taiTriggerCfg.trigGenAmt = 125000000; /*decide when the pulse occurs*/
	taiTriggerCfg.trigGenTime = 0;
	taiTriggerCfg.pulseWidth = 0xf;
	taiTriggerCfg.pulseWidthRange = 0x7;
	trigEn = MAD_TRUE;
	if ((status = taiSetTrigConfig(dev, port, trigEn, &taiTriggerCfg)) != MAD_OK)
	{
		MSG_PRINT(("Get TAI Trigger configure error, error code: %d\n", status));
		return status;
	}

	return status;
}

/********************************************************************************
* taiGetEventCaptureTime
*
*	This sample API will demonstrate how to get the event capture time
*	1. Disable the PTP output source and enable the PTP input source
*	2. Set the Event Capture Overwrite as 1 (Capture all PTP Events & Retain the last PTP Event)
*	3. Enable the Event Counter Start
*	4. Set the event phase as 0 ( Event Requests occur on the rising edge of LED[1] pin)
*	5. Send a trigger from GPIO Access (LED[1] write 0 then write 1)
*	6. Read back the event capture time
*
*******************************************************************************/
MAD_STATUS taiGetEventCaptureTime(MAD_DEV *dev)
{
	MAD_STATUS status = MAD_OK;
	MAD_LPORT port = 0;
	TAI_EVENT_STATUS taiStatus;
	TAI_EVENT_CONFIG    eventData;
	char temp;

	/*Disable the PTP output source*/
	if ((status = ptpSetOutputSrc(dev, port, MAD_FALSE)) != MAD_OK)
	{
		MSG_PRINT(("Set PTP output source error, error code: %d\n", status));
		return status;
	}

	/*Enable the PTP input source*/
	if ((status = ptpSetInputSrc(dev, port, MAD_TRUE)) != MAD_OK)
	{
		MSG_PRINT(("Set PTP input source error, error code: %d\n", status));
		return status;
	}

	eventData.eventOverwrite = MAD_TRUE;/*Capture all PTP Events & Retain the last PTP Event*/
	eventData.eventCtrStart = MAD_TRUE;/*Increment the Event Capture Counter on EventReq¡¯s*/
	eventData.eventPhase = MAD_FALSE; /*Capture on the rising edge*/

	eventData.capTrigEvent = MAD_FALSE; /*Capture PTP_EVREQ pin events*/
	eventData.eventCapIntEn = MAD_FALSE;

	if ((status = taiSetEventConfig(dev, port, &eventData)) != MAD_OK)
	{
		MSG_PRINT(("Set event configure error, error code: %d\n", status));
		return status;
	}


	printf("Ready to capture global time, Send a trigger from GPIO Access (write 0 then write 1), finished?(y/n)\n");
	scanf(" %c", &temp);
	if ((temp == 'y') || (temp == 'Y'))
	{
		if ((status = taiGetEventStatus(dev, port, &taiStatus)) != MAD_OK)
		{
			MSG_PRINT(("Get TAI Event Status error, error code: %d\n", status));
			return status;
		}		

		printf("Event Capture time is: 0x%x\n", taiStatus.eventTime);
	}

	return status;
}