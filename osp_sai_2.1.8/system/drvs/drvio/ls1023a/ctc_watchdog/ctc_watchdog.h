#ifndef __CTC_WD_H__
#define __CTC_WD_H__

#define CTC_WD_DEV_NAME         "/dev/ctc_watchdog"
#define CTC_WD_DEV_MAJOR        114

#define CTC_EN_WATCHDOG         _IO(CTC_WD_DEV_MAJOR, 6)
#define CTC_DIS_WATCHDOG        _IO(CTC_WD_DEV_MAJOR, 7)
#define CTC_DIS_FEED_WATCHDOG   _IO(CTC_WD_DEV_MAJOR, 8)                                                             \

#endif
