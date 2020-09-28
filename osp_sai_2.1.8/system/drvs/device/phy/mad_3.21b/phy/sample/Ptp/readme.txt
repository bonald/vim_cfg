
            Sample for 151xP PTP
====================================================
This sample shows how to run the 151xP device PTP feature.
For simple reference, following functions are listed.
    1)151xP PTP initialization:
	    <ptpDevInit> - Initialize PTP functions in 151xP. It should be called first.
	2)151xP PTP workaround:
	    <ptpWorkAround> - Workaround for 151xP PTP feature, Prefer to call this API after the ptpDevInit()
	3)151xP PTP configuration:
	    <becomeMaster> - Set the device to Master
		<becomeSlave> - Set the device to Slave
    4)151xP PTP Clock adjustment:
	    <ptpSetCurTime> - Sample for set current ToD (timeofday) in domain 0
		<ptpGetCurTime> - Sample for get current ToD (timeofday) in domain 0
		<ptpAdjTime> - Sample for how to adjust the ToD
		<ptpAdjFreq> - Sample for how to adjust the frequency
		<ptpGen1PPS> - Sample for generate one pulse per second based on time array 0
	5)151xP PTP TAI trigger
        <taiTriggerGen> - Sample for how to generate 50% duty cycle signal
		<taiPulseGen> - Sample for how to generate pulse when PTP global time matches with given time
		<taiGetEventCaptureTime> - Sample for how to get the event capture time

