override SDK_DIR := $(shell cd $(SDK_DIR) && pwd)
#/*SYSTEM MODIFY, Added by weij for update SDK, 2017-09-11*/
#OBJ_DIR = $(BLD_DIR)/obj.$(BOARD)/$(subst $(SDK_DIR)/,,$(CURDIR))
#LIB_DIR = $(BLD_DIR)/lib.$(BOARD)
#BIN_DIR = $(BLD_DIR)/bin.$(BOARD)
OBJ_DIR = $(BLD_DIR)/obj/$(subst $(TOP_DIR)/,,$(CURDIR))
LIB_DIR = $(BLD_DIR)/lib
CMODEL_LIB_DIR = $(BLD_DIR)/lib
BIN_DIR = $(BLD_DIR)/bin

CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar cru
RANLIB = $(CROSS_COMPILE)ranlib

export COMPATIBAL_CHIP

ifeq ($(findstring goldengate, $(CHIPNAME)),goldengate)
COMPATIBAL_CHIP += goldengate
endif

ifeq ($(findstring greatbelt, $(CHIPNAME)),greatbelt)
COMPATIBAL_CHIP += greatbelt
endif

ifeq ($(PRODUCT), of)
CPPFLAGS += -DOFPRODUCT
endif

ifeq ($(ARCH),powerpc)
CPPFLAGS = -DHOST_IS_LE=0
endif

ifeq ($(ARCH),x86)
CPPFLAGS = -DHOST_IS_LE=1
endif

ifeq ($(ARCH),mips)
CPPFLAGS = -DHOST_IS_LE=0
CPPFLAGS += -mabi=n32
endif

ifeq ($(ARCH),loongson)
CPPFLAGS = -DHOST_IS_LE=1
endif
ifeq ($(ARCH),freescale)
CPPFLAGS = -DHOST_IS_LE=0
endif

ifeq ($(ARCH),arm)
CPPFLAGS = -DHOST_IS_LE=1
endif

#/*SYSTEM MODIFY, Added by weij for bug 104341, 2017-09-11*/
ifeq ($(ARCH),arm64)
CPPFLAGS = -DHOST_IS_LE=1
endif

ifeq ($(ARCH),um)
CPPFLAGS = -DHOST_IS_LE=1
endif

ifeq ($(CPU),ls2f)
ifneq ($(M64),TRUE)
CPPFLAGS += -DDAL_USE_MMAP2
endif
endif

ifeq ($(PRODUCT), of)
CPPFLAGS += -DOFPRODUCT
endif

ifeq ($(PRODUCT), tap)
CPPFLAGS += -DTAPPRODUCT
endif

#################################
#SDK_WORK_ENV
#0 - with no cmodel;
#1 - with  cmodel;
#################################

#################################
#SDK_WORK_PLATFORM
#0 - hardware platform ;
#1 - software simulation platform;
#################################

ifeq ($(BOARD),ctc-sim)
CPPFLAGS += -DSDK_IN_USERMODE
CPPFLAGS += -DUSE_SIM_MEM
CPPFLAGS += -DSDK_WORK_ENV=1
CPPFLAGS += -DSDK_WORK_PLATFORM=1
ifeq ($(VER),d)
RPATH += $(LIB_DIR)
else
ifeq ($(M64),TRUE)
RPATH += /data01/users/sdk/humber_release/share/dll_x64/$(CHIPNAME)
else
RPATH += /data01/users/sdk/humber_release/share/dll/$(CHIPNAME)
endif
endif
endif

CPPFLAGS += -fgnu89-inline
CPPFLAGS += -Wno-error=unused-but-set-variable
ifeq ($(ARCH),arm64)
CPPFLAGS += -Wno-error=misleading-indentation
endif

ifeq ($(BOARD),linux-board)
CPPFLAGS += -DSDK_IN_USERMODE
CPPFLAGS += -DSDK_WORK_PLATFORM=0
CPPFLAGS += -DMCU

ifeq ($(ChipAgent),TRUE)
CPPFLAGS += -DSDK_WORK_ENV=1
else
CPPFLAGS += -DSDK_WORK_ENV=0
endif

endif	

ifeq ($(M64),TRUE)
ifeq ($(ARCH),freescale)
CPPFLAGS += -fPIC -Wno-long-long -Wformat
else

#/*SYSTEM MODIFY, Added by weij for bug 104341, 2017-09-11*/
ifeq ($(ARCH),arm64)
CPPFLAGS += -fPIC -Wno-long-long -Wformat
else
LD_FLAGS += -m64
CPPFLAGS += -fPIC -Wno-long-long -Wformat -m64
endif
endif
else
ifeq ($(CPU),ls2f)
LD_FLAGS += -mabi=n32
else
ifneq ($(ARCH),arm)
#/*SYSTEM MODIFY, Added by weij for bug 104341, 2017-09-11*/
ifeq ($(ARCH),arm64)
else
CPPFLAGS += -m32
LD_FLAGS += -m32
endif
endif
endif
endif

ifeq ($(CROSS_COMPILE),ghs)
CC = $(GHS)/ccppc
AR = $(GHS)/ccppc -archive -o 
# Green Hills doesn't do ranlib
RANLIB = echo
CFLAGS += --diag_error 223,821 -w
else 
CFLAGS += -Wall -Werror
endif

ifneq ($(VER),r)
CPPFLAGS += -DSDK_IN_DEBUG_VER
endif

CFLAGS += -g -fno-strict-aliasing

IS_GCOV = no

ifeq ($(IS_GCOV),yes)
CFLAGS += -DISGCOV
endif

ifeq ($(DRV_DS_LITE),yes)
CFLAGS += -DDRV_DS_LITE
endif

ifeq ($(findstring l2, $(featureMode)),l2)
CPPFLAGS += -DFEATURE_MODE=1
else
	ifeq ($(findstring l2l3, $(featureMode)),l2l3)
	CPPFLAGS += -DFEATURE_MODE=2
	else
	CPPFLAGS += -DFEATURE_MODE=0
	endif
endif

ifeq ($(findstring humber, $(CHIPNAME)),humber)
CPPFLAGS += -DHUMBER
CPPFLAGS += -DCHIP_NAME=humber
endif


ifeq ($(findstring greatbelt, $(CHIPNAME)),greatbelt)
CPPFLAGS += -DGREATBELT
ifeq ($(ChipAgent),TRUE)
CPPFLAGS += -DCHIP_AGENT
endif
endif


ifeq ($(findstring goldengate, $(CHIPNAME)),goldengate)
CPPFLAGS += -DGOLDENGATE
ifeq ($(ChipAgent),TRUE)
CPPFLAGS += -DCHIP_AGENT
endif
endif

ifeq ($(findstring duet2, $(CHIPNAME)),duet2)
CPPFLAGS += -DDUET2
CPPFLAGS += -DUSW
endif

ifeq ($(findstring tsingma, $(CHIPNAME)),tsingma)
CPPFLAGS += -DTSINGMA
CPPFLAGS += -DUSW
endif

ifeq ($(ChipAgent),TRUE)
CPPFLAGS += -DCHIP_AGENT
endif

ifeq ($(VER),r)
ifeq ($(ChipAgent),FALSE)
CFLAGS += -O2
endif
endif

#CFLAGS += -Wframe-larger-than=1024

