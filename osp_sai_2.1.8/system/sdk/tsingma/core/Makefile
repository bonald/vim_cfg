ifeq ($(targetbase),linux)
include $(MK_DIR)/sys.mk


LIB = sdkcore
SRCS = $(shell find api/src -name "*.c")
SRCS += $(shell find common/src -name "*.c")
SRCS += $(shell find init/ -name "*.c")
SRCS += $(shell find app/ -name "*.c")

ifeq ($(findstring humber, $(CHIPNAME)),humber)
CPPFLAGS += -DHUMBER
SRCS += $(shell find humber/src/    -name "*.c")
CPPFLAGS += -I$(SDK_DIR)/core/humber/include
CPPFLAGS += -I$(SDK_DIR)/core/humber/include/ctc
CPPFLAGS += -I$(SDK_DIR)/core/humber/include/sys
CPPFLAGS += -I$(SDK_DIR)/driver/humber/include
endif

ifeq ($(findstring greatbelt, $(CHIPNAME)),greatbelt)
CPPFLAGS += -DGREATBELT
SRCS += $(shell find greatbelt/src/    -name "*.c")
CPPFLAGS += -I$(SDK_DIR)/core/greatbelt/include
CPPFLAGS += -I$(SDK_DIR)/core/greatbelt/include/ctc
CPPFLAGS += -I$(SDK_DIR)/core/greatbelt/include/sys
endif

ifeq ($(findstring goldengate, $(CHIPNAME)),goldengate)
CPPFLAGS += -DGOLDENGATE
SRCS += $(shell find goldengate/src/    -name "*.c")
CPPFLAGS += -I$(SDK_DIR)/core/goldengate/include
CPPFLAGS += -I$(SDK_DIR)/core/goldengate/include/ctc
CPPFLAGS += -I$(SDK_DIR)/core/goldengate/include/sys
endif

CPPFLAGS += -I$(SDK_DIR)/driver
CPPFLAGS += -I$(SDK_DIR)/libctccli/include
CPPFLAGS += -I$(SDK_DIR)/dal/include
CPPFLAGS += -I$(SDK_DIR)/sal/include
CPPFLAGS += -I$(SDK_DIR)/core/api/include
CPPFLAGS += -I$(SDK_DIR)/core/common/include
CPPFLAGS += -I$(SDK_DIR)/core/init
CPPFLAGS += -I$(SDK_DIR)/core

ifeq ($(USW),yes)
SRCS += $(shell find usw/src/  -maxdepth 2  -name "*.c")
SRCS += $(foreach n, $(CHIPNAME), $(shell find usw/src/sys/$(n) -name "*.c"))
CPPFLAGS += -I$(SDK_DIR)/core/usw/include
CPPFLAGS += -I$(SDK_DIR)/core/usw/include/ctc
CPPFLAGS += -I$(SDK_DIR)/core/usw/include/sys
CPPFLAGS += -I$(SDK_DIR)/driver/api
endif

include $(MK_DIR)/lib.mk

endif




ifeq ($(targetbase),vxworks)

OBJ_DIR=$(OBJ_BASE_DIR)/core

ifeq ($(USW),yes)
all_targets = api init common app usw libsdkcore
else
all_targets = api init common app $(CHIPNAME) libsdkcore
endif

all: $(all_targets) FORCE
clean: $(addprefix clean_,$(all_targets)) FORCE

api: FORCE
	make -C $(SDK_DIR)/core/api -f Makefile.vx

clean_api: FORCE
	make -C $(SDK_DIR)/core/api -f Makefile.vx clean

init: FORCE
	make -C $(SDK_DIR)/core/init -f Makefile.vx

clean_init: FORCE
	make -C $(SDK_DIR)/core/init -f Makefile.vx clean

common: FORCE
	make -C $(SDK_DIR)/core/common -f Makefile.vx

clean_common: FORCE
	make -C $(SDK_DIR)/core/common -f Makefile.vx clean

app: FORCE
	make -C $(SDK_DIR)/core/app -f Makefile.vx

clean_app: FORCE
	make -C $(SDK_DIR)/core/app -f Makefile.vx clean

usw: FORCE
	make -C $(SDK_DIR)/core/usw -f Makefile.vx

clean_usw: FORCE
	make -C $(SDK_DIR)/core/usw -f Makefile.vx clean

goldengate: FORCE
	make -C $(SDK_DIR)/core/goldengate -f Makefile.vx

clean_goldengate: FORCE
	make -C $(SDK_DIR)/core/goldengate -f Makefile.vx clean

greatbelt: FORCE
	make -C $(SDK_DIR)/core/greatbelt -f Makefile.vx

clean_greatbelt: FORCE
	make -C $(SDK_DIR)/core/greatbelt -f Makefile.vx clean

libsdkcore: FORCE
	make -f Makefile_libsdkcore.vx

clean_libsdkcore: FORCE
	make -f Makefile_libsdkcore.vx clean

.PHONY: FORCE
FORCE:
endif
