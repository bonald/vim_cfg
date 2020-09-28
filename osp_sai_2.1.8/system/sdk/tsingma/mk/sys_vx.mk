
ifeq ($(BOARD),vxworks)
CPPFLAGS = -DSDK_IN_VXWORKS
CPPFLAGS += -DSDK_WORK_ENV=0
CPPFLAGS += -DSDK_WORK_PLATFORM=0
endif

ifeq ($(BOARD),vxworks-sim)
CPPFLAGS = -DSDK_IN_VXWORKS
CPPFLAGS += -DUSE_SIM_MEM
CPPFLAGS += -DSDK_WORK_ENV=0
CPPFLAGS += -DSDK_WORK_PLATFORM=1
endif

ifneq ($(ARCH),powerpc)
CPPFLAGS += -DHOST_IS_LE=1
else
CPPFLAGS += -DHOST_IS_LE=0
endif

ifneq ($(VER),r)
CPPFLAGS += -DSDK_IN_DEBUG_VER
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

ifeq ($(findstring duet2, $(CHIPNAME)),duet2)
CPPFLAGS += -DDUET2
CPPFLAGS += -DUSW
endif

ifeq ($(findstring tsingma, $(CHIPNAME)),tsingma)
CPPFLAGS += -DTSINGMA
CPPFLAGS += -DUSW
endif

ifeq ($(findstring goldengate, $(CHIPNAME)),goldengate)
CPPFLAGS += -DGOLDENGATE
endif

ifeq ($(findstring humber, $(CHIPNAME)),humber)
CPPFLAGS += -DHUMBER
endif

ifeq ($(findstring greatbelt, $(CHIPNAME)),greatbelt)
CPPFLAGS += -DGREATBELT
endif


CPPFLAGS += -I$(VX_HEADER)/
CPPFLAGS += -I$(VX_HEADER)/wrn/coreip

ifeq ($(CPU),SIMNT)
CFLAGS = -Wall -fno-strict-aliasing
else
CFLAGS = -Wall -fno-strict-aliasing -mlongcall
endif

ifeq ($(VER),r)
CFLAGS += -O2
else
CFLAGS += -g
endif

CFLAGS += -O3

#add by zzhu for gcov
IS_GCOV = no
