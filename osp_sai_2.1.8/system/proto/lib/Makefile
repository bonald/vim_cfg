include ../../Makefile.env.inc
include $(MK_DIR)/sys.mk

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
MY_CPU_DIR=ls1023a
endif

ifeq ($(CPU),ctc5236)
MY_CPU_DIR=ctc5236
endif

### proto header path
include $(PROTO_DIR)/Makefile.inc

CPPFLAGS += -I$(PROTO_DIR)/chsm/include
CPPFLAGS += -I$(DRV_TOP_DIR)/drvio/${MY_CPU_DIR}/ctc_watchdog

LIB = proto
SRCS = $(shell find src -name "*.c")

include $(MK_DIR)/solib.mk
