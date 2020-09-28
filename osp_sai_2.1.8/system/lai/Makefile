include $(MK_DIR)/sys.mk
ifeq ($(ARCH), x86_64)
CFLAGS += -Wno-error=pointer-to-int-cast
CFLAGS += -Wno-error=int-to-pointer-cast
endif

LIB = lai
SRCS += $(shell find src -name "*.c")

CPPFLAGS += -I$(CTC_LIB_DIR)/include
CPPFLAGS += -I$(CTC_LIB_DIR)/sal/include
CPPFLAGS += -I$(CTC_LIB_DIR)/genlog
CPPFLAGS += -I$(CTC_LIB_DIR)/task
CPPFLAGS += -I$(CTC_LIB_DIR)/ipc
CPPFLAGS += -I$(CTC_LIB_DIR)/sock
CPPFLAGS += -I$(CTC_LIB_DIR)/util/include
CPPFLAGS += -I$(CTC_LIB_DIR)/libasn1
CPPFLAGS += -I$(FEA_DIR)/lcm/lcmmsg/gen
CPPFLAGS += -I$(FEA_DIR)/lcm/diag/log


CPPFLAGS += -I$(FEA_DIR)/lcm/lcapi/include
CPPFLAGS += -I$(FEA_DIR)/lcm/include
CPPFLAGS += -I$(TOP_DIR)/lai/include
CPPFLAGS += -I$(TOP_DIR)/lai/inc

CPPFLAGS += -I$(DRV_TOP_DIR)/bus/asic/mdio
CPPFLAGS += -I$(DRV_TOP_DIR)/bus/cpu/pci
CPPFLAGS += -I$(DRV_TOP_DIR)/bus/cpu/spi/include
CPPFLAGS += -I$(DRV_TOP_DIR)/bus/cpu/i2c/include
CPPFLAGS += -I$(DRV_TOP_DIR)/device/phy
CPPFLAGS += -I$(DRV_TOP_DIR)/device/flowctrl
CPPFLAGS += -I$(DRV_TOP_DIR)/device/poe
CPPFLAGS += -I$(DRV_TOP_DIR)/device/fiber
CPPFLAGS += -I$(DRV_TOP_DIR)/device/fpga
CPPFLAGS += -I$(DRV_TOP_DIR)/device/epld
CPPFLAGS += -I$(DRV_TOP_DIR)/device/ad9517
CPPFLAGS += -I$(DRV_TOP_DIR)/device/ltc2991
CPPFLAGS += -I$(DRV_TOP_DIR)/device/asic
CPPFLAGS += -I$(DRV_TOP_DIR)/device/bootrom
CPPFLAGS += -I$(DRV_TOP_DIR)/device/gpio
CPPFLAGS += -I$(DRV_TOP_DIR)/device/mux
CPPFLAGS += -I$(DRV_TOP_DIR)/device/sensor
CPPFLAGS += -I$(DRV_TOP_DIR)/device/fan
CPPFLAGS += -I$(DRV_TOP_DIR)/device/vsc3308
CPPFLAGS += -I$(DRV_TOP_DIR)/device/power
CPPFLAGS += -I$(DRV_TOP_DIR)/device/eeprom
CPPFLAGS += -I$(DRV_TOP_DIR)/device/led
CPPFLAGS += -I$(DRV_TOP_DIR)/drvio/ctc_cpm
CPPFLAGS += -I$(DRV_TOP_DIR)/drvio/kernel_monitor

include $(MK_DIR)/lib.mk
