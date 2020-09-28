#include ../../Makefile.env.inc
include $(MK_DIR)/sys.mk

ifeq ($(ARCH), x86_64)
CFLAGS += -Wno-error=pointer-to-int-cast
CFLAGS += -Wno-error=int-to-pointer-cast
endif

ifeq ($(OS_TYPE),cnos)
UPM_DIR = $(FEA_DIR)
endif
ifeq ($(OS_TYPE),ipi)
UPM_DIR = $(LC_DIR)
endif

ifeq ($(CPU),hi3535)
MY_CPU_DIR=hi3535
endif
ifeq ($(CPU),p1010)
MY_CPU_DIR=powerpc
endif
ifeq ($(CPU),p1014)
MY_CPU_DIR=powerpc
endif
ifeq ($(CPU),ls1023a)
MY_CPU_DIR=ls1023a
endif
ifeq ($(CPU),ctc5236)
MY_CPU_DIR=ctc5236
endif
ifeq ($(CPU),x86_64)
MY_CPU_DIR=x86_64
endif
ifeq ($(CPU),ls2h)
MY_CPU_DIR=ls2h
endif
ifeq ($(CPU),ls2k)
MY_CPU_DIR=ls2k
endif

LIB = lcm
SRCS += $(shell find card chsmtlk lcmgt lcshtlk lcmlai lcapi -name "*.c")

ifeq ($(OS_TYPE),cnos)
CPPFLAGS += -I$(CTC_LIB_DIR)/include
CPPFLAGS += -I$(CTC_LIB_DIR)/sal/include
CPPFLAGS += -I$(CTC_LIB_DIR)/genlog
CPPFLAGS += -I$(CTC_LIB_DIR)/task
CPPFLAGS += -I$(CTC_LIB_DIR)/ipc
CPPFLAGS += -I$(CTC_LIB_DIR)/sock
CPPFLAGS += -I$(CTC_LIB_DIR)/util/include
CPPFLAGS += -I$(CTC_LIB_DIR)/libasn1
CPPFLAGS += -I$(UPM_DIR)/lcm/lcmmsg/gen

CPPFLAGS += -I$(PROTO_DIR)/include
CPPFLAGS += -I$(PROTO_DIR)/lib/include
CPPFLAGS += -I$(PROTO_DIR)/lib/include/ds
CPPFLAGS += -I$(CDB_DIR)/core/include

CPPFLAGS += -I$(SAI_DIR)/inc
CPPFLAGS += -I$(SAI_DIR)/include
CPPFLAGS += -I$(SAI_DIR)/include/inc
CPPFLAGS += -I$(SAI_DIR)/include/common
CPPFLAGS += -I$(SAI_DIR)/src/common

CPPFLAGS += -I$(TOP_DIR)/lai/include
CPPFLAGS += -I$(TOP_DIR)/lai/inc
endif
ifeq ($(OS_TYPE),ipi)
CPPFLAGS += -I$(SAL_TOP_DIR)/include
CPPFLAGS += -I$(TOP_DIR)/include
CPPFLAGS += -I$(UPM_DIR)/lcm/lcmmsg/gen
CPPFLAGS += -I$(TOP_DIR)/lib/util/include
CPPFLAGS += -I$(TOP_DIR)/lib/include
CPPFLAGS += -I$(TOP_DIR)/lib/libasn1
CPPFLAGS += -I$(TOP_DIR)/lib/sal/include
CPPFLAGS += -I$(ZEBOS_TOP_DIR)/lib
endif	

CPPFLAGS += -I$(SDK_DIR)/dal/include
CPPFLAGS += -I$(SDK_DIR)/sal/include
CPPFLAGS += -I$(SDK_DIR)/core
CPPFLAGS += -I$(SDK_DIR)/core/common/include
CPPFLAGS += -I$(SDK_DIR)/core/api/include
CPPFLAGS += -I$(SDK_DIR)/core/init
CPPFLAGS += -I$(CMODEL_DIR)/core/include

CPPFLAGS += -I$(UPM_DIR)/lcm/include
CPPFLAGS += -I$(UPM_DIR)/lcm/diag/log

CPPFLAGS += -I$(UPM_DIR)/hagt/$(CHIPNAME)/include
CPPFLAGS += -I$(UPM_DIR)/hagt/$(CHIPNAME)/include/common

ifeq ($(IS_SHOW_FORWARD_ENABLE),yes)
SRCS += diag/lcm_show_forward.c
SRCS += $(shell find $(CMODEL_DIR)/core/src/{epe,ipe,oam,qmgt,util,foam} -name "*.c")
SRCS += $(shell find $(CMODEL_DIR)/ctckal/src -name "*.c")

ifeq ($(IS_UML),y)
CPPFLAGS += -DDRVIVER_WORK_ENV=2
CPPFLAGS += -DDRVIER_WORK_PLATFORM=1
CPPFLAGS += -DCMODEL_SHOW_FORWARD_OPTIMIZE=1
CPPFLAGS += -DSDK_WORK_ENV=1
else
CPPFLAGS += -DDRVIVER_WORK_ENV=1
CPPFLAGS += -DDRVIER_WORK_PLATFORM=0
CPPFLAGS += -DCMODEL_SHOW_FORWARD_OPTIMIZE=0
CPPFLAGS += -DSDK_WORK_ENV=0
endif

CPPFLAGS += -I$(SDK_DIR)/libctccli/include

CPPFLAGS += -I$(SDK_DIR)/driver
CPPFLAGS += -I$(SDK_DIR)/driver/common/include
CPPFLAGS += -I$(SDK_DIR)/driver/greatbelt/include
CPPFLAGS += -I$(SDK_DIR)/core/common/include
CPPFLAGS += -I$(CMODEL_DIR)/mem_model/include
CPPFLAGS += -I$(CMODEL_DIR)/ctckal/include
CPPFLAGS += -I$(CMODEL_DIR)/core/include
endif

CPPFLAGS += -I$(SDK_DIR)/driver/greatbelt/include
CPPFLAGS += -I$(DRV_TOP_DIR)
CPPFLAGS += -I$(DRV_TOP_DIR)/bus/asic/mdio
CPPFLAGS += -I$(DRV_TOP_DIR)/bus/cpu/pci
CPPFLAGS += -I$(DRV_TOP_DIR)/bus/cpu/spi/include
CPPFLAGS += -I$(DRV_TOP_DIR)/bus/cpu/i2c/include
CPPFLAGS += -I$(DRV_TOP_DIR)/device/phy
CPPFLAGS += -I$(DRV_TOP_DIR)/device/phy/mad_3.21b/phy/Include
CPPFLAGS += -I$(DRV_TOP_DIR)/device/flowctrl
CPPFLAGS += -I$(DRV_TOP_DIR)/device/poe
CPPFLAGS += -I$(DRV_TOP_DIR)/device/fiber
CPPFLAGS += -I$(DRV_TOP_DIR)/device/fpga
CPPFLAGS += -I$(DRV_TOP_DIR)/device/epld
CPPFLAGS += -I$(DRV_TOP_DIR)/device/clock
CPPFLAGS += -I$(DRV_TOP_DIR)/device/asic
CPPFLAGS += -I$(DRV_TOP_DIR)/device/bootrom
CPPFLAGS += -I$(DRV_TOP_DIR)/device/sensor
CPPFLAGS += -I$(DRV_TOP_DIR)/device/bmr
CPPFLAGS += -I$(DRV_TOP_DIR)/device/gpio
CPPFLAGS += -I$(DRV_TOP_DIR)/device/mux
CPPFLAGS += -I$(DRV_TOP_DIR)/device/fan
CPPFLAGS += -I$(DRV_TOP_DIR)/device/vsc3308
CPPFLAGS += -I$(DRV_TOP_DIR)/device/power
CPPFLAGS += -I$(DRV_TOP_DIR)/device/eeprom
CPPFLAGS += -I$(DRV_TOP_DIR)/device/led
CPPFLAGS += -I$(DRV_TOP_DIR)/device/vcm
CPPFLAGS += -I$(DRV_TOP_DIR)/device/sysenv

CPPFLAGS += -I$(DRV_TOP_DIR)/device/ltc2991
CPPFLAGS += -I$(DRV_TOP_DIR)/device/ltc4151
CPPFLAGS += -I$(DRV_TOP_DIR)/drvio/${MY_CPU_DIR}/ctc_cpm
CPPFLAGS += -I$(DRV_TOP_DIR)/drvio/${MY_CPU_DIR}/kernel_monitor
CPPFLAGS += -I$(DRV_TOP_DIR)/drvio/${MY_CPU_DIR}/ctc_gpio

include $(MK_DIR)/lib.mk
