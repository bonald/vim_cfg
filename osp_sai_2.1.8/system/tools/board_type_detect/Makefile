include $(MK_DIR)/sys.mk

PROG = board_type_detect

ifeq ($(CPU),hi3535)
MY_CPU_DIR=hi3535
endif

ifeq ($(CPU),p1010)
MY_CPU_DIR=powerpc
endif

ifeq ($(CPU),p1014)
MY_CPU_DIR=powerpc
endif

ifeq ($(CPU),x86_64)
MY_CPU_DIR=x86_64
endif

ifeq ($(CPU),ls1023a)
CPPFLAGS +=-DCTC_ENV_IN_SPI_FLASH
endif

ifeq ($(CPU),ctc5236)
CPPFLAGS +=-DCTC_ENV_IN_SPI_FLASH
endif

ifeq ($(CPU),hi3535)
SRCS = e550_detect/detect.c 
SRCS += e550_detect/i2c_drv.c 
SRCS += e550_detect/epld_info.c 
SRCS += e550_detect/mgmt_cfg.c 
SRCS += e550_detect/cpu_info.c 
SRCS += e550_detect/motherboard_info.c
SRCS += e550_detect/bootrom_ver.c
CPPFLAGS += -I$(DRV_TOP_DIR)/drvio/ctc_hw
else 
ifeq ($(CPU),ls2f)
MY_ARCH_DIR=ls2f
SRCS = $(MY_ARCH_DIR)/detect.c
CPPFLAGS += -I$(DRV_TOP_DIR)/drvio/ctc_hw/$(MY_ARCH_DIR)
else
SRCS = detect.c
SRCS += common_detect.c
SRCS += normal_detect.c
SRCS += onie_detect.c
endif
endif
CPPFLAGS += -I.

ifeq ($(OS_TYPE),cnos)
CPPFLAGS += -I$(CTC_LIB_DIR)/sal/include
CPPFLAGS += -I$(CTC_LIB_DIR)/include
CPPFLAGS += -I$(TOP_DIR)/proto/include
endif
ifeq ($(OS_TYPE),ipi)
CPPFLAGS += -I$(ZEBOS_TOP_DIR)/lib
CPPFLAGS += -I$(TOP_DIR)/include
CPPFLAGS += -I$(TOP_DIR)/sal/include
endif


include $(MK_DIR)/prog.mk
