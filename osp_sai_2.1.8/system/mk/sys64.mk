override TOP_DIR := $(shell cd $(TOP_DIR) && pwd)

#VENDOR = centec

OBJ_DIR = $(BLD_DIR)/obj/$(subst $(TOP_DIR)/,,$(CURDIR))
LIB_DIR = $(BLD_DIR)/lib
BIN_DIR = $(BLD_DIR)/bin

LD = $(CROSS_COMPILE)ld
CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
RANLIB = $(CROSS_COMPILE)ranlib
OBJCOPY = $(CROSS_COMPILE)objcopy 

ifeq ($(ARCH),um)
CPPFLAGS = -D_GLB_HOST_IS_LE=1
else
CPPFLAGS = -D_GLB_HOST_IS_LE=0
endif

ifeq ($(CHIPNAME),goldengate)
CPPFLAGS += -DGOLDENGATE
else
CPPFLAGS += -DGREATBELT
endif

ifeq ($(PRODUCT), tap)
CPPFLAGS += -DTAPPRODUCT
endif

ifeq ($(PRODUCT), sai)
CPPFLAGS += -DSAIPRODUCT
endif
CPPFLAGS += -DKERNEL_ARP
ifeq ($(PRODUCT), of)
CPPFLAGS += -DOFPRODUCT
endif

ifeq ($(USE_LICENSE), yes)
CPPFLAGS += -DLICENSE
endif

CPPFLAGS += -DHAVE_DMA
CPPFLAGS += -DHAVE_CONFIG_H
CPPFLAGS += -D_CENTEC_
CPPFLAGS += -D_GLB_ENABLE_DBGSHOW_
CPPFLAGS += -DMEMMGR
CPPFLAGS += -D_NEW_VERSION_LINUX_

ifeq ($(ARCH),mips)
CPPFLAGS += -D_CTC_OCTEON_CN50XX_
endif

ifeq ($(ARCH), powerpc)
ifeq ($(CPU), p1010)
CPPFLAGS += -D_CTC_BOARD_TYPE_EEPROM_
CPPFLAGS += -D_CTC_FSCL_P1010_
CPPFLAGS += -D_GLB_HOST_IS_LE=0
endif
ifeq ($(CPU), p1014)
CPPFLAGS += -D_CTC_BOARD_TYPE_EEPROM_
CPPFLAGS += -D_CTC_FSCL_P1014_
CPPFLAGS += -D_GLB_HOST_IS_LE=0
endif
ifeq ($(CPU), 8t4s12x)
CPPFLAGS += -D_CTC_BOARD_TYPE_EEPROM_
CPPFLAGS += -D_CTC_FSCL_P1014_
CPPFLAGS += -D_CTC_NO_PARTITION2
endif
CPPFLAGS += -Wno-error=unused-but-set-variable 
CPPFLAGS += -Wno-error=address
CPPFLAGS += -Wno-error=deprecated-declarations
CPPFLAGS += -Wno-error=implicit-function-declaration
CPPFLAGS += -Wno-error=implicit-int
CPPFLAGS += -Wno-error=maybe-uninitialized
CPPFLAGS += -Wno-error=array-bounds
endif
ifeq ($(IPC), y)
CPPFLAGS += -DHAVE_IPC
endif

ifeq ($(ETH_OAM), y)
CPPFLAGS += -DHAVE_ETH_OAM
endif


# SDK flags which is used by sal.
# added by zhangdy for merging v3.0.2.17, we should add the following def here,
# since our sal may be used by sdk in a first be included first wins way.
# the macro SDK_BOARD in defined in env.inc and be passed to sdk mk system as
# BOARD, so here we should use SDK_BOARD.
ifeq ($(SDK_BOARD),ctc-sim)
CPPFLAGS += -DSDK_IN_USERMODE
CPPFLAGS += -DUSE_SIM_MEM
CPPFLAGS += -DSDK_WORK_ENV=1
CPPFLAGS += -DSDK_WORK_PLATFORM=1
endif

ifeq ($(SDK_BOARD),linux-sim)
CPPFLAGS += -DSDK_IN_USERMODE
CPPFLAGS += -DUSE_SIM_MEM
CPPFLAGS += -DSDK_WORK_ENV=0
CPPFLAGS += -DSDK_WORK_PLATFORM=1
endif

ifeq ($(SDK_BOARD),ctc-board)
CPPFLAGS += -DSDK_IN_USERMODE
CPPFLAGS += -DSDK_WORK_ENV=1
CPPFLAGS += -DSDK_WORK_PLATFORM=0
CPPFLAGS += -DEMULATION_PLATFORM
endif

ifeq ($(SDK_BOARD),linux-board)
CPPFLAGS += -DSDK_IN_USERMODE
CPPFLAGS += -DSDK_WORK_ENV=0
CPPFLAGS += -DSDK_WORK_PLATFORM=0
endif

ifeq ($(BTYPE), chassis_svc)
CPPFLAGS += -D_GLB_DISTRIBUTE_SYSTEM_
endif

ifeq ($(IS_UML), y)
CPPFLAGS += -DUSE_SIM_MEM
CPPFLAGS += -D_GLB_UML_SYSTEM_
endif

CFLAGS = -Wall -g -fno-strict-aliasing -D_GNU_SOURCE
#comment off by cuixl for too many warnnings, deal it later
CFLAGS += -Werror

ifeq ($(ARCH), mips)
CFLAGS += -mabi=n32
CPPFLAGS += -D_GNU_SOURCE
endif

#add by jiangz for valgrind
IS_VALGRIND = no
ifeq ($(IS_VALGRIND),yes)
CFLAGS += -DISVALGRIND
endif

#add by zhuj for gprof
IS_GPROF = no
ifeq ($(IS_GPROF),yes)
CFLAGS += -DISGPROF
CFLAGS += -pg
endif

#add by zzhu for gcov
IS_GCOV = no
ifeq ($(IS_GCOV),yes)
CFLAGS += -DISGCOV
endif

ifeq ($(VER),r)
CFLAGS += -O2
endif

# common lib
ifeq ($(ARCH),um)
LIBASN1=libasn1.a
LIBMSG=libmsg.a
LIBMEMIO=libmemio.a
else
LIBASN1=libasn1.so
LIBMSG=libmsg.so
LIBMEMIO=libmemio.so
endif
