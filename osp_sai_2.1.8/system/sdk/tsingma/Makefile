# CROSS_COMPILE should set appropriate compiler path to PATH.
# Suggestion: add these two lines to your ~/.bash_profile
#    PATH=$PATH:/sw/pub/xtools.ppc-linux/bin/:/opt/gcc-4.3/bin/
#    export PATH
# In this case, /sw/pub/xtools.ppc-linux/bin/    /opt/gcc-4.3/bin/  is our appropriate compiler path.

####################### Set default param ##############3###
include ../Makefile.env.inc
include $(MK_DIR)/sys.mk
ifndef CHIPNAME
export CHIPNAME = goldengate
endif

ifndef SUBCHIPNAME
export SUBCHIPNAME = goldengate
endif

ifndef VER
export VER = r
endif

ifndef targetbase
export targetbase = linux
endif


ifndef BOARD
export BOARD = linux-board
endif

ifndef ARCH
export ARCH = mips
endif

ifndef auto
export auto = yes
endif

ifndef cpp
export cpp = no
endif

ifndef DRV_DS_LITE
export DRV_DS_LITE = no
endif

# featureMode 0: full, 1: l2, 2: l2l3
ifndef featureMode
export featureMode = full
endif
######################################################################################################################
#ONE_LIB = yes   only one lib   --> libctcsdk, include ctccli core libctccli drvier dal sal and so on, not include app
ifndef ONE_LIB
export ONE_LIB = no
endif

ifndef SO_LIB
export SO_LIB = no
endif

ifneq ($(VER),r)
override VER = d
endif

ifeq ($(CHIPNAME),greatbelt)
CHIPNAME = greatbelt
endif


ifeq ($(CHIPNAME),humber)
CHIPNAME = humber
endif

ifndef LINUX_LK	
export LINUX_LK = FALSE
endif

ifndef M64 
export M64 = FALSE
endif

ifneq ($(CHIPNAME), $(filter $(CHIPNAME), humber greatbelt goldengate))
export USW = yes
else
export USW = no
endif

############################# linux #####################################
ifeq ($(targetbase),linux)

ifeq ($(LINUX_LK),TRUE)
export SDK_DIR = $(M)
else
export SDK_DIR = $(CURDIR)
endif
export MK_DIR = $(CURDIR)/mk
# ctcsdk: cli+sdk+cmodel (software simulation)
ifeq ($(BOARD),ctc-sim)
export CMODEL_DIR = $(CURDIR)/../cmodel
ifeq ($(M64),TRUE)
CPU = i686_x64
else
CPU = i686
endif
ARCH = um
CROSS_COMPILE =
KDIR =
ifeq ($(ARCH),powerpc)
CPU = ppc_82xx
CROSS_COMPILE =ppc-linux-
endif
ifeq ($(ARCH),mips)
CPU = octeon
CROSS_COMPILE =mips64-octeon-linux-gnu-
endif

ifeq ($(ARCH),freescale)
CPU = fscle
CROSS_COMPILE =powerpc-fsl_networking-linux-gnuspe-
endif

all_targets = libsal libdal libdrv liblibctccli
ifeq ($(CHIPNAME),humber)
all_targets += libmemmodel libdkit
endif
ifeq ($(CHIPNAME),greatbelt)
all_targets += libmemmodel libdkit
endif
ifeq ($(CHIPNAME),goldengate)
all_targets += libmemmodel libdkit
endif
ifeq ($(USW),yes)
all_targets += libdkit
endif
#all_targets = libsal libdal libdrv libmemmodel liblibctccli
all_targets += libcmodel libcmodelcli libsdkcore libctccli

ifeq ($(CHIPNAME),goldengate)
all_targets += libmodelcpp
endif
ifeq ($(USW),yes)
all_targets += libmodelcpp
endif

ifeq ($(ChipAgent),TRUE)
all_targets += libchipagent
endif
#/*SYSTEM MODIFY, Added by weij for update SDK, 2017-09-11*/
#all_targets += app

ifeq ($(ONE_LIB),yes)
all_targets = libctcsdk libcmodel libcmodelcli
ifneq ($(CHIPNAME), $(filter $(CHIPNAME), humber greatbelt))
all_targets += libmodelcpp
endif
ifneq ($(USW),yes)
all_targets += libmemmodel
endif
#/*SYSTEM MODIFY, Added by weij for update SDK, 2017-09-11*/
#all_targets += app
endif

dkcli_targets = libsal libdal libdrv libmemmodel liblibctccli libdkit
dkcli_targets += libdkcli
endif

# ctcsdk: cli+sdk++asic (hardware platform board)
ifeq ($(BOARD),linux-board)
ifeq ($(ARCH),powerpc)
CPU = ppc_82xx
CROSS_COMPILE =ppc-linux-
ARCH = powerpc
endif
ifeq ($(ARCH),mips)
CPU = octeon
CROSS_COMPILE =mips64-octeon-linux-gnu-
ARCH = mips
endif
ifeq ($(ARCH),loongson)
CPU = ls2f
CROSS_COMPILE =mipsel-linux-
ARCH = mips
endif
ifeq ($(ARCH),freescale)
CPU = fscl
CROSS_COMPILE =powerpc-fsl_networking-linux-gnuspe-
ARCH = powerpc
endif
ifeq ($(ARCH),arm)
CPU = arm
ifeq ($(M64), TRUE)
ARCH = arm64
CROSS_COMPILE=aarch64-linux-gnu-
else
ARCH = arm
CROSS_COMPILE=arm-hisiv200-linux-
endif
endif
ifeq ($(ARCH),x86)
CPU = x86
CROSS_COMPILE =
ARCH = x86
endif
 
#/*SYSTEM MODIFY, Added by weij for update SDK, 2017-09-11*/
KDIR = $(K_DIR)
all_targets = libsal libdal libdrv liblibctccli

all_targets += libdkit

ifeq ($(ChipAgent),TRUE)
all_targets += libchipagent
endif
all_targets += libsdkcore libctccli
#/*SYSTEM MODIFY, Added by weij for update SDK, 2017-09-11*/
#all_targets += app

ifeq ($(ONE_LIB), yes)
all_targets = libctcsdk app
endif

dkcli_targets = libsal libdal libdrv liblibctccli libdkit
dkcli_targets += libdkcli
endif

export BLD_DIR = $(SDK_DIR)/build.$(CPU).$(VER)
export OBJ_BASE_DIR = $(BLD_DIR)/obj.$(BOARD)
export LIB_DIR = $(BLD_DIR)/lib.$(BOARD)
export BIN_DIR = $(BLD_DIR)/bin.$(BOARD)
export BOARD
export VER
export CPU
export ARCH
export SUBARCH
export CROSS_COMPILE
export ONE_LIB
export SO_LIB

ifeq ($(LINUX_LK),TRUE)
include $(SDK_DIR)/mk/sys_km.mk
include $(SDK_DIR)/Makefile.kernel
else
include $(SDK_DIR)/Makefile.user
endif 

endif

############################# vxworks #####################################3

ifeq ($(targetbase),vxworks)

ifeq ($(BUILD_SPEC),SIMNTgnu)
CPU              = SIMNT
TOOL             = gnu
TOOL_FAMILY      = gnu
DEFAULT_RULE     = sdk.out
endif


ifeq ($(BUILD_SPEC),PPC603gnu)
CPU              = PPC603
TOOL             = gnu
TOOL_FAMILY      = gnu
DEFAULT_RULE     = sdk.out
endif

#include $(TGT_DIR)/h/make/defs.project

## build-configuration info

ifeq ($(BUILD_SPEC),PPC603gnu)
AR               = arppc
AS               = ccppc
CC               = ccppc
CC_ARCH_SPEC     = -mcpu=603 -mstrict-align
CFLAGS           = -O0 -mcpu=603 -Wall -mstrict-align -mlongcall -fno-builtin -I. -I$(WIND_BASE)/target/h/ -DCPU=PPC603 -DTOOL_FAMILY=gnu -DTOOL=gnu -D_WRS_KERNEL -DVXWORKS -DBE_HOST=1
CFLAGS_AS        = -g -mcpu=603 -mstrict-align -ansi -fno-builtin -P -xassembler-with-cpp -I. -I$(WIND_BASE)/target/h/ -I$(WIND_BASE)/target/h/wrn/coreip -DCPU=PPC603 -DTOOL_FAMILY=gnu -DTOOL=gnu -D_WRS_KERNEL
CPP              = ccppc -E -P
LD               = ldppc
LDDEPS           =
LDFLAGS          = -X -N
LD_PARTIAL       = ccppc -r -nostdlib -Wl,-X
LD_PARTIAL_FLAGS = -X -r
NM               = nmppc
OPTION_DEFINE_MACRO = -D
OPTION_DEPEND    = -M -w
OPTION_GENERATE_DEPENDENCY_FILE = -MD
OPTION_INCLUDE_DIR = -I
OPTION_LANG_C    = -xc
OPTION_UNDEFINE_MACRO = -U
#POST_BUILD_RULE  = $(PRJ_DIR)\PPC603gnu\XelApp.bat
PRJ_LIBS         =
SIZE             = sizeppc
TOOL_FAMILY      = gnu
endif

ifeq ($(BUILD_SPEC),SIMNTgnu)
AR               = arsimpc
AS               = ccsimpc
RANLIB           = ranlibsimpc
CC               = ccsimpc
CC_ARCH_SPEC     = -mpentium
CFLAGS           = -g -mpentium -ansi -fno-builtin -fno-defer-pop -I. -I$(WIND_BASE)/target/h/  -DCPU=SIMNT -DTOOL_FAMILY=gnu  -DTOOL=gnu
CFLAGS_AS        = -g -mpentium -ansi -fno-builtin -fno-defer-pop -P -xassembler-with-cpp -I. -I$(WIND_BASE)/target/h/  -DCPU=SIMNT -DTOOL_FAMILY=gnu  -DTOOL=gnu
CPP              = ccsimpc -E -P
LD               = ldsimpc
LDFLAGS          = --subsystem=windows
LD_PARTIAL       = ccsimpc -r -nostdlib
LD_PARTIAL_FLAGS = -r
NM               = nmsimpc -g
OPTION_DEFINE_MACRO = -D
OPTION_DEPEND    = -M -w
OPTION_GENERATE_DEPENDENCY_FILE = -MD
OPTION_INCLUDE_DIR = -I
OPTION_LANG_C    = -xc
OPTION_UNDEFINE_MACRO = -U
SIZE             = sizesimpc
TOOL_FAMILY      = gnu
POST_BUILD_RULE  =
PRJ_LIBS         =
endif


# override make definitions only above this line
include $(TGT_DIR)/h/make/rules.project

# should change these two variable according to your compile environment
# LINK: YES - build ctcsdk vxWorks image , NO - build libctcsdk.a
export LINK = YES
# SPLIT_CORE: NO - libsdkcore.a , YES - libsdkcoreapi.a libsdkcoresys.a
export SPLIT_CORE = YES

export BLD_DIR = $(SDK_DIR)/build.$(CPU).$(VER)
export OBJ_BASE_DIR=$(BLD_DIR)/obj.$(BOARD)
export LIB_DIR=$(BLD_DIR)/lib.$(BOARD)
export BIN_DIR=$(BLD_DIR)/bin.$(BOARD)

all_targets = libsal libdal libdrv libsdkcore liblibctccli libctccli libdkit

ifeq ($(ChipAgent),TRUE)
all_targets += libchipagent
endif

# ctcsdk: sdk + asic
ifeq ($(BOARD),vxworks)
all_targets += libapp
dkcli_targets = libsal libdal libdrv liblibctccli libdkit
dkcli_targets += libdkcli
endif

# ctcsdk: sdk + memmodel
ifeq ($(BOARD),vxworks-sim)
ifneq ($(USW),yes)
all_targets += libmemmodel
endif
all_targets += libapp
dkcli_targets = libsal libdal libdrv libmemmodel liblibctccli libdkit
dkcli_targets += libdkcli
endif

all: $(all_targets) FORCE
clean: $(addprefix clean_,$(all_targets)) FORCE

dkcli: $(dkcli_targets) FORCE
clean_dkcli: $(addprefix clean_,$(dkcli_targets)) FORCE

libsal: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/sal

clean_libsal: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/sal clean

libdal: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/dal -f Makefile

clean_libdal: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/dal clean

libdrv: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/driver

clean_libdrv: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/driver clean

libsdkcore: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/core

clean_libsdkcore: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/core clean

libdkit: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/dkits -f Makefile.lib

clean_libdkit: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/dkits -f Makefile.lib clean

libdkcli: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/dkits -f Makefile

clean_libdkcli: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/dkits -f Makefile clean
	
libmemmodel: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/mem_model/$(CHIPNAME)

clean_libmemmodel: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/mem_model/$(CHIPNAME) clean

libchipagent: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/chip_agent

clean_libchipagent: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/chip_agent clean
liblibctccli: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/libctccli

clean_liblibctccli: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/libctccli clean

libctccli: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/ctccli

clean_libctccli: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/ctccli clean

libappusr:	FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/app/usr

clean_libappusr:	FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/app/usr clean

libapp:	FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/app

clean_libapp:	FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/app clean

distclean: FORCE
	rm -rf $(BLD_DIR)

.PHONY: FORCE
FORCE:

endif
