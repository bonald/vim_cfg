######################################set linux compile env #############################
ifeq ($(targetbase),linux)
include $(MK_DIR)/sys.mk
OBJ_DIR=$(OBJ_BASE_DIR)/app
__LIB_DIR := $(LIB_DIR)

PROG = ctcsdk
SRCS = $(wildcard *.c)
SRCS += $(wildcard config/*.c)
SRCS += $(wildcard usr/*.c)
SRCS += $(wildcard sample/*.c)

ifeq ($(findstring humber, $(CHIPNAME)),humber)
CPPFLAGS += -DHUMBER
endif

ifeq ($(findstring greatbelt, $(CHIPNAME)),greatbelt)
CPPFLAGS += -DGREATBELT
endif

ifeq ($(findstring goldengate, $(CHIPNAME)),goldengate)
CPPFLAGS += -DGOLDENGATE
endif


CPPFLAGS += -I$(SDK_DIR)/dal/include
CPPFLAGS += -I$(SDK_DIR)/sal/include
CPPFLAGS += -I$(SDK_DIR)/core/api/include
CPPFLAGS += $(foreach n, $(CHIPNAME), -I$(SDK_DIR)/core/$(n)/include)
CPPFLAGS += $(foreach n, $(CHIPNAME), -I$(SDK_DIR)/core/$(n)/include/ctc)
CPPFLAGS += $(foreach n, $(CHIPNAME), -I$(SDK_DIR)/core/$(n)/include/sys)
CPPFLAGS += -I$(SDK_DIR)/core/init
CPPFLAGS += -I$(SDK_DIR)/driver
CPPFLAGS += -I$(SDK_DIR)/driver/api
CPPFLAGS += -I$(SDK_DIR)/libctccli/include
CPPFLAGS += $(foreach n, $(CHIPNAME), -I$(SDK_DIR)/mem_model/$(n)/include)
CPPFLAGS += -I$(SDK_DIR)/ctccli/sdkcli
CPPFLAGS += -I$(SDK_DIR)/app
CPPFLAGS += -I$(SDK_DIR)/app/config
CPPFLAGS += -I$(SDK_DIR)/app/usr
CPPFLAGS += -I$(SDK_DIR)/app/sample
CPPFLAGS += $(foreach n, $(CHIPNAME), -I$(SDK_DIR)/ctccli/sdkcli/$(n))
CPPFLAGS += -I$(SDK_DIR)/core/common/include
CPPFLAGS += -I$(SDK_DIR)/libptkxr/include
CPPFLAGS += $(foreach n, $(CHIPNAME), -I$(SDK_DIR)/chip_agent/$(n)/include)

LIBSAL    = $(LIB_DIR)/libsal.a
LIBDAL    = $(LIB_DIR)/libdal.a
LIBSDK    = $(LIB_DIR)/libsdkcore.a
LIBDRV    = $(LIB_DIR)/libdrv.a

LIBLIBCTCCLI  = $(LIB_DIR)/liblibctccli.a
LIBCTCCLI  = $(LIB_DIR)/libctccli.a

ifeq ($(BOARD),ctc-sim)
LIBMEMMODEL = $(LIB_DIR)/libmemmodel.a
LIBCTCLI  = $(LIB_DIR)/libcmodelcli.a
LIBCMODEL = $(LIB_DIR)/libcmodel.a
LIBMODELCPP = $(LIB_DIR)/libmodelcpp.so
endif

LIBDKIT = $(LIB_DIR)/libdkit.a

ifeq ($(SO_LIB),yes)
LIBCTCSDK = $(LIB_DIR)/libctcsdk.so
else
LIBCTCSDK = $(LIB_DIR)/libctcsdk.a
endif

LD_FLAGS += -Wl,--rpath=$(RPATH)

ifeq ($(ONE_LIB),yes)
LD_LIBS = -L$(__LIB_DIR) -L$(LIB_DIR) -lctcsdk
else
DEP_LIBS = $(LIBLIBCTCCLI)
LD_LIBS = -L$(__LIB_DIR) -L$(LIB_DIR) -lctccli
endif

ifeq ($(BOARD),ctc-sim)
ifneq ($(CHIPNAME), $(filter $(CHIPNAME),humber greatbelt))
DEP_LIBS += $(LIBMODELCPP) $(LIBCMODEL)
LD_LIBS += -Wl,--start-group,-lmodelcpp,-lcmodel,-lcmodelcli,--end-group -ldl
else
DEP_LIBS += $(LIBCMODEL)
LD_LIBS += -Wl,--start-group,-lcmodel,-lcmodelcli,--end-group -ldl
endif
ifneq ($(USW),yes)
DEP_LIBS += $(LIBMEMMODEL)
LD_LIBS += -lmemmodel
endif
endif

ifeq ($(ONE_LIB), yes)
DEP_LIBS += $(LIBCTCSDK)
LD_LIBS += -Wl,--start-group,-lctcsdk,--end-group -lpthread -ldl
else
DEP_LIBS += $(LIBCTCCLI) $(LIBSDK) $(LIBDRV) $(LIBDAL) $(LIBSAL) $(LIBDKIT)
LD_LIBS += -Wl,--start-group,-llibctccli,-lsdkcore,-ldrv,-lsal,-ldal,-ldkit,--end-group -lpthread -ldl
ifeq ($(ChipAgent),TRUE)
DEP_LIBS += $(LIB_DIR)/libchipagent.a
LD_LIBS += -lchipagent
endif
endif

ifeq ($(USW),yes)
ifeq ($(BOARD),ctc-sim)
CPPFLAGS += -I/opt/openssl-1.0.2c/usr/local/include/openssl
ifeq ($(M64),TRUE)
LIB_LCRYPTO_DIR := /opt/openssl-1.0.2c/usr/local/lib64
LD_LIBS += -L$(LIB_LCRYPTO_DIR) -lcrypto -ldl -lstdc++
else
LIB_LCRYPTO_DIR := /opt/openssl-1.0.2c/usr/local/lib
LD_LIBS += -L$(LIB_LCRYPTO_DIR) -lcrypto -ldl -lstdc++
endif
endif
endif

include $(MK_DIR)/prog.mk
endif

######################################set vxworks compile env #############################
ifeq ($(targetbase),vxworks)

OBJ_DIR=$(OBJ_BASE_DIR)/app
ifeq ($(LINK),yes)
all_targets = sample config sdkinit ctcsdk vxworks
else
all_targets = sample config sdkinit ctcsdk
endif

all: $(all_targets) FORCE
clean: $(addprefix clean_,$(all_targets)) FORCE


usr: FORCE
	make -C $(SDK_DIR)/app/usr -f Makefile.vx

clean_usr: FORCE
	make -C $(SDK_DIR)/app/usr -f Makefile.vx clean
	
sample: FORCE
	make -C $(SDK_DIR)/app/sample -f Makefile.vx

clean_sample: FORCE
	make -C $(SDK_DIR)/app/sample -f Makefile.vx clean

config: FORCE
	make -C $(SDK_DIR)/app/config -f Makefile.vx

config_sample: FORCE
	make -C $(SDK_DIR)/app/config -f Makefile.vx clean

sdkinit: FORCE
	make -f Makefile.vx

clean_sdkinit: FORCE
	make -f Makefile.vx clean
	
ctcsdk: FORCE
	make -f Makefile_sdk.vx

clean_ctcsdk: FORCE
	make -f Makefile_sdk.vx clean

vxworks: FORCE
	make -f Makefile_vx

clean_vxworks: FORCE
	make -f Makefile_vx clean
	
.PHONY: FORCE
FORCE:
endif
