#ifndef __CTC_KERNEL_MONITOR_H__
#define __CTC_KERNEL_MONITOR_H__

#define CTC_SYS_LED_DEV_MAJOR    107
#define CTC_LED_RDWR        _IO(CTC_SYS_LED_DEV_MAJOR, 6)
#define CTC_SYS_LED_DEV_NAME     "/dev/ctc_sys_led"
#endif
