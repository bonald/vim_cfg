
            151xP PTP API
===============================================
		   
Table of Content:
-----------------
1) Release History 
2) Source Code Organization
3) General Introduction
4) 151xP PTP APIs		   

1) Release History 
-----------------------------------------------
MAD_3.5/src/macfunc/151xP_ptp - Nov.30.2015

2) Source Code Organization
-----------------------------------------------
    2.1) MAD_3.5/src/macfunc/151xP_ptp
	     151xP PTP header file and functions code
	2.2) MAD_3.5/sample/Ptp/Sample_151xP_Ptp.c
	     151xP PTP sample code
		 
3) General Introduction
-----------------------------------------------
The 151xP PTP is in Marvell Phy 151xP device. The APIs are included in Marvell Phy Driver (MAD).
Please first read <README_PHY.txt> to know how to use general PHY APIs, and how to access Phy registers.
In MAD_3.5/src/macfunc/151xP_ptp
<151xP_ptp.c> - Source code of API level.
<151xP_ptp.h> - Header file of API level.

4) 151xP PTP APIs
-----------------------------------------------
See <151xP_ptp.h> in detail.

For simple reference, following functions are listed. The sample functions are in MAD_3.5/sample/Ptp/Sample_151xP_Ptp.c

    4.1)151xP PTP initialization:
	    <ptpDevInit> - Initialize PTP functions in 151xP. It should be called first.
	4.2)151xP PTP workaround:
	    <ptpWorkAround> - Workaround for 151xP PTP feature, Prefer to call this API after the ptpDevInit()
	4.3)151xP PTP configuration:
	    <becomeMaster> - Set the device to Master
		<becomeSlave> - Set the device to Slave
    4.4)151xP PTP Clock adjustment:
	    <ptpSetCurTime> - Sample for set current ToD (timeofday) in domain 0
		<ptpGetCurTime> - Sample for get current ToD (timeofday) in domain 0
		<ptpAdjTime> - Sample for how to adjust the ToD
		<ptpAdjFreq> - Sample for how to adjust the frequency
		<ptpGen1PPS> - Sample for generate one pulse per second based on time array 0
	4.5)151xP PTP TAI trigger
        <taiTriggerGen> - Sample for how to generate 50% duty cycle signal
		<taiPulseGen> - Sample for how to generate pulse when PTP global time matches with given time
		<taiGetEventCaptureTime> - Sample for how to get the event capture time




