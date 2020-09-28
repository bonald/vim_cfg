ifeq ($(targetbase),linux)
include $(MK_DIR)/sys.mk

LIB = ctccli
SRCS = $(wildcard *.c)
SRCS += $(shell find sdkcli -maxdepth 1 -name "*.c")
SRCS += $(foreach n, $(COMPATIBAL_CHIP), $(shell find sdkcli/$(n)/ -name "*.c"))

ifeq ($(UNIT_TEST),TRUE)
CPPFLAGS += -DSDK_UNIT_TEST
endif

CPPFLAGS += -I$(SDK_DIR)/dal/include
CPPFLAGS += -I$(SDK_DIR)/sal/include
CPPFLAGS += -I$(SDK_DIR)/core/api/include
CPPFLAGS += -I$(SDK_DIR)/core/common/include
CPPFLAGS += -I$(SDK_DIR)/core/app
CPPFLAGS += -I$(SDK_DIR)/core
CPPFLAGS += $(foreach n, $(COMPATIBAL_CHIP), -I$(SDK_DIR)/core/$(n)/include)
CPPFLAGS += $(foreach n, $(COMPATIBAL_CHIP), -I$(SDK_DIR)/core/$(n)/include/ctc)
CPPFLAGS += $(foreach n, $(COMPATIBAL_CHIP), -I$(SDK_DIR)/core/$(n)/include/sys)
CPPFLAGS += -I$(SDK_DIR)/libctccli/include
CPPFLAGS += -I$(SDK_DIR)/driver
CPPFLAGS += -I$(SDK_DIR)/sample
CPPFLAGS += $(foreach n, $(COMPATIBAL_CHIP), -I$(SDK_DIR)/ctccli/sdkcli/$(n))
CPPFLAGS += -I$(SDK_DIR)/ctccli/sdkcli/sample
CPPFLAGS += -I$(SDK_DIR)/ctccli/sdkcli/
CPPFLAGS += -I$(SDK_DIR)/dkits
CPPFLAGS += -I$(SDK_DIR)/libctccli/ctc_shell/include
CPPFLAGS += -I$(SDK_DIR)/libpktxr/include
CFLAGS += -Wall

ifeq ($(USW),yes)
CPPFLAGS += -I$(SDK_DIR)/driver/api
CPPFLAGS += -I$(SDK_DIR)/core/usw/include
CPPFLAGS += -I$(SDK_DIR)/core/usw/include/ctc
CPPFLAGS += -I$(SDK_DIR)/core/usw/include/sys
SRCS += $(shell find sdkcli/usw/ -name "*.c")
endif

include $(MK_DIR)/lib.mk
endif


################################## vxworks #####################################
ifeq ($(targetbase),vxworks)
OBJ_DIR=$(OBJ_BASE_DIR)/ctccli

all_targets = master sdkcli libctccli

all: $(all_targets) FORCE
clean: $(addprefix clean_,$(all_targets)) FORCE

master: FORCE
	$(CTC_MAKE) -f Makefile.vx

clean_master: FORCE
	$(CTC_MAKE) -f Makefile.vx clean

sdkcli: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/ctccli/sdkcli -f Makefile_sdkcli.vx

clean_sdkcli: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/ctccli/sdkcli -f Makefile_sdkcli.vx clean
diagcli: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/ctccli/diagcli/$(CHIPNAME) -f Makefile.vx

clean_diagcli: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/ctccli/diagcli/$(CHIPNAME) -f Makefile.vx clean
libctccli: FORCE
	$(CTC_MAKE) -f Makefile_libctccli.vx

clean_libctccli: FORCE
	$(CTC_MAKE) -f Makefile_libctccli.vx clean
	
.PHONY: FORCE
FORCE:
endif
