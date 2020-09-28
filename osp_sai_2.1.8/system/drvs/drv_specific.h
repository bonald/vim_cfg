#ifndef __DRV_SPECIFIC__
#define __DRV_SPECIFIC__

/* include files difference */
#ifdef CNOS_OS
#include "ctclib_memory.h"
#include "ctc_task.h"
#else
#include "memory.h"
#include "memmgr.h"
#include "ctclib_thread.h"
#endif 

/* MEM define difference */
#ifdef CNOS_OS
#define CTCLIB_MEM_DRIVER_SPI_INFO          MEM_DRV_SPI
#define CTCLIB_MEM_DRIVER_AD9517_INFO       MEM_DRV_CLOCK
#define CTCLIB_MEM_DRIVER_PHY_INFO          MEM_DRV_PHY
#define CTCLIB_MEM_DRIVER_POE_INFO          MEM_DRV_POE
#define CTCLIB_MEM_DRIVER_FIBER_INFO        MEM_DRV_FIBER
#define CTCLIB_MEM_DRIVER_EPLD_INFO         MEM_DRV_EPLD
#define CTCLIB_MEM_DRIVER_FPGA_INFO         MEM_DRV_FPGA
#define CTCLIB_MEM_DRIVER_I2C_INFO          MEM_DRV_I2C
#define CTCLIB_MEM_DRIVER_EEPROM_INFO       MEM_DRV_EEPROM
#define CTCLIB_MEM_DRIVER_SENSOR_INFO       MEM_DRV_SENSOR
#define CTCLIB_MEM_DRIVER_BMR_INFO       MEM_DRV_BMR
#define CTCLIB_MEM_DRIVER_POWER_INFO        MEM_DRV_POWER
#define CTCLIB_MEM_DRIVER_FAN_INFO          MEM_DRV_FAN
#define CTCLIB_MEM_DRIVER_VSC3308_INFO      MEM_DRV_SWITCH
#define CTCLIB_MEM_DRIVER_LED_INFO          MEM_DRV_LED
#define CTCLIB_MEM_DRIVER_MDIO_INFO         MEM_DRV_MDIO
#define CTCLIB_MEM_DRIVER_MUX_INFO          MEM_DRV_MUX
#define CTCLIB_MEM_DRIVER_GPIO_INFO         MEM_DRV_GPIO
#define CTCLIB_MEM_DRIVER_LTC2991_INFO      MEM_DRV_POWER
#define CTCLIB_MEM_DRIVER_LTC4151_INFO      MEM_DRV_POWER
#define CTCLIB_MEM_DRIVER_VCM_INFO          MEM_DRV_VCM
#define CTCLIB_MEM_DRIVER_CLOCK_INFO          MEM_DRV_CLOCK
#define CTCLIB_MEM_DRIVER_SYSENV_INFO          MEM_DRV_SYSENV
#define CTCLIB_MEM_DRIVER_BOOTROM_INFO      MEM_DRV_BOOTROM
#else
#endif 

/* MEM define difference */
#ifdef CNOS_OS
#ifdef GOLDENGATE
#define _GOLDENGATE_
#endif
#else
#define _GOLDENGATE_
#endif

/* Timer difference : in ctc_task.h and ctclib_thread.h e.g. CTC_TASK_ADD_TIME */
#ifdef CNOS_OS
#define DRV_DEFINE_TASK_ENCAP(_FUNC_)
#else
#define DRV_DEFINE_TASK_ENCAP(_FUNC_) \
int32 _FUNC_##_encap(thread_t* p_thread) \
{ \
    _FUNC_(p_thread->arg); \
    return 0; \
}
#endif

#endif /* !__DRV_SPECIFIC__ */
