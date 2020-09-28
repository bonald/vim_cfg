# 
ifeq ($(ARCH),powerpc)
EXTRA_CFLAGS += -DHOST_IS_LE=0
EXTRA_CFLAGS += -DDRVIER_WORK_PLATFORM=0
endif

ifeq ($(ARCH),mips)
EXTRA_CFLAGS += -DHOST_IS_LE=1
EXTRA_CFLAGS += -DDRVIER_WORK_PLATFORM=0
EXTRA_CFLAGS += -DDMA_MEM_MODE_PLATFORM
endif

ifeq ($(ARCH),loongson)
EXTRA_CFLAGS += -DHOST_IS_LE=1
EXTRA_CFLAGS += -DDRVIER_WORK_PLATFORM=0
EXTRA_CFLAGS += -DDMA_MEM_MODE_PLATFORM
endif
ifeq ($(ARCH),freescale)
EXTRA_CFLAGS += -DHOST_IS_LE=0
EXTRA_CFLAGS += -DDRVIER_WORK_PLATFORM=0
endif
ifeq ($(ARCH),arm)
EXTRA_CFLAGS = -DHOST_IS_LE=1
EXTRA_CFLAGS += -DDRVIER_WORK_PLATFORM=0
endif
ifeq ($(ARCH),um)
EXTRA_CFLAGS += -DHOST_IS_LE=1
EXTRA_CFLAGS += -DDRVIER_WORK_PLATFORM=1
endif

ifeq ($(DRV_DS_LITE),yes)
EXTRA_CFLAGS += -DDRV_DS_LITE
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
EXTRA_CFLAGS += -DUSE_SIM_MEM
EXTRA_CFLAGS += -DSDK_WORK_ENV=1
EXTRA_CFLAGS += -DSDK_WORK_PLATFORM=1
ifeq ($(ChipAgent),TRUE)
EXTRA_CFLAGS += -DCHIP_AGENT
endif

endif

ifeq ($(BOARD),linux-sim)
EXTRA_CFLAGS += -DUSE_SIM_MEM
EXTRA_CFLAGS += -DSDK_WORK_ENV=0
EXTRA_CFLAGS += -DSDK_WORK_PLATFORM=1
endif

ifeq ($(BOARD),ctc-board)
EXTRA_CFLAGS += -DSDK_WORK_ENV=1
EXTRA_CFLAGS += -DSDK_WORK_PLATFORM=0
endif

ifeq ($(BOARD),linux-board)
EXTRA_CFLAGS += -DSDK_WORK_ENV=0
EXTRA_CFLAGS += -DSDK_WORK_PLATFORM=0
endif

ifeq ($(findstring l2, $(featureMode)),l2)
EXTRA_CFLAGS += -DFEATURE_MODE=1
else
    ifeq ($(findstring l2l3, $(featureMode)),l2l3)
    EXTRA_CFLAGS += -DFEATURE_MODE=2
    else
    EXTRA_CFLAGS += -DFEATURE_MODE=0
    endif
endif

ifeq ($(findstring humber, $(CHIPNAME)),humber)
EXTRA_CFLAGS += -DHUMBER
EXTRA_CFLAGS += -DCHIP_NAME=humber
endif

ifeq ($(findstring greatbelt, $(CHIPNAME)),greatbelt)
EXTRA_CFLAGS += -DGREATBELT
EXTRA_CFLAGS += -DCHIP_NAME=greatbelt
endif

ifeq ($(findstring goldengate, $(CHIPNAME)),goldengate)
EXTRA_CFLAGS += -DGOLDENGATE
endif

ifeq ($(findstring duet2, $(CHIPNAME)),duet2)
EXTRA_CFLAGS += -DDUET2
EXTRA_CFLAGS += -DUSW
endif

ifeq ($(findstring tsingma, $(CHIPNAME)),tsingma)
EXTRA_CFLAGS += -DTSINGMA
EXTRA_CFLAGS += -DUSW
endif

ifneq ($(VER),r)
EXTRA_CFLAGS += -DSDK_IN_DEBUG_VER
endif
# Only used for building sdk in kernel mode
#

EXTRA_CFLAGS += -DCOMP_CRC=1 -Wno-unused -Wno-strict-prototypes -DSDK_IN_DEBUG_VER=1
EXTRA_CFLAGS += -DSDK_IN_KERNEL
EXTRA_CFLAGS += -I$(TOP_DIR)/include/
EXTRA_CFLAGS += -I$(M)/sal/include
EXTRA_CFLAGS += -I$(M)/app/usr
EXTRA_CFLAGS += -I$(M)/app
EXTRA_CFLAGS += -I$(M)/app/sample
EXTRA_CFLAGS += -I$(M)/app/config
EXTRA_CFLAGS += -I$(M)/dal/include
EXTRA_CFLAGS += -I$(M)/dal/src/linux_kernel
EXTRA_CFLAGS += -I$(M)/driver
EXTRA_CFLAGS += -I$(M)/core/api/include
EXTRA_CFLAGS += -I$(M)/core/common/include
EXTRA_CFLAGS += -I$(M)/core/init
EXTRA_CFLAGS += -I$(M)/core/switch
EXTRA_CFLAGS += -I$(M)/core/app
EXTRA_CFLAGS += -I$(M)/core
ifeq ($(USW),yes)
EXTRA_CFLAGS += -I$(M)/core/usw/include
EXTRA_CFLAGS += -I$(M)/core/usw/include/ctc
EXTRA_CFLAGS += -I$(M)/core/usw/include/sys
EXTRA_CFLAGS += -I$(M)/ctccli/sdkcli/usw
EXTRA_CFLAGS += -I$(M)/dkits/usw
EXTRA_CFLAGS += -I$(M)/driver/api
else
EXTRA_CFLAGS += $(foreach n, $(CHIPNAME), -I$(M)/core/$(n)/include)
EXTRA_CFLAGS += $(foreach n, $(CHIPNAME), -I$(M)/core/$(n)/include/ctc)
EXTRA_CFLAGS += $(foreach n, $(CHIPNAME), -I$(M)/core/$(n)/include/sys)
EXTRA_CFLAGS += $(foreach n, $(CHIPNAME), -I$(M)/ctccli/sdkcli/$(n))
EXTRA_CFLAGS += $(foreach n, $(CHIPNAME), -I$(M)/dkits/$(n))
endif
EXTRA_CFLAGS += -I$(M)/libctccli/include
EXTRA_CFLAGS += -I$(M)/ctccli/sdkcli
EXTRA_CFLAGS += -I$(M)/ctc_shell
EXTRA_CFLAGS += -I$(M)/dkits
EXTRA_CFLAGS += -I$(M)/dkits/common
#EXTRA_CFLAGS += -I$(M)/chip_agent/include


