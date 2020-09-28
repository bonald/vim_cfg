ifeq ($(targetbase),linux)

include $(MK_DIR)/sys.mk

LIB = chipagent
SRCS =
SRCS += $(shell find $(CHIPNAME)/src -name "*.c")


CPPFLAGS += -I$(SDK_DIR)/sal/include
CPPFLAGS += -I$(SDK_DIR)/dal/include
CPPFLAGS += -I$(SDK_DIR)/core/switch
CPPFLAGS += -I$(SDK_DIR)/core/common/include
CPPFLAGS += -I$(SDK_DIR)/chip_agent/$(CHIPNAME)/include
CPPFLAGS += -I$(SDK_DIR)/driver/api
CPPFLAGS += -I$(SDK_DIR)/driver
CPPFLAGS += -I$(SDK_DIR)/core/$(CHIPNAME)/include/sys
CPPFLAGS += -I$(SDK_DIR)/app/$(CHIPNAME)
CPPFLAGS += -I$(SDK_DIR)/app/sample
CPPFLAGS += -I$(SDK_DIR)/core/api/include

ifeq ($(USW), yes)
include $(MK_DIR)/duet2.mk
endif

include $(MK_DIR)/lib.mk

endif

ifeq ($(targetbase),vxworks)
BLD_DIR=$(SDK_DIR)/build.$(CPU).$(VER)
OBJ_DIR=$(BLD_DIR)/obj.$(BOARD)/chip_agent
LIB_DIR=$(BLD_DIR)/lib.$(BOARD)
BIN_DIR=$(BLD_DIR)/bin.$(BOARD)

all_targets = src libchipagent

all: $(all_targets) FORCE
clean: $(addprefix clean_,$(all_targets)) FORCE

src: FORCE
	make -C $(SDK_DIR)/chip_agent/$(CHIPNAME)/src -f Makefile.vx

clean_src: FORCE
	make -C $(SDK_DIR)/chip_agent/$(CHIPNAME)/src -f Makefile.vx clean

libchipagent: FORCE
	make -f Makefile_lib_chipagent.vx

clean_libchipagent: FORCE
	make -f Makefile_lib_chipagent.vx clean

.PHONY: FORCE clean
FORCE:
endif
