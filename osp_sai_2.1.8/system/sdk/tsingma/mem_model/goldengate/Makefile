ifeq ($(targetbase),linux)
include $(MK_DIR)/sys.mk
OBJ_DIR=$(OBJ_BASE_DIR)/mem_model
LIB = memmodel
SRCS =
SRCS += $(shell find src -name "*.c")

ifeq ($(BOARD),ctc-sim)
CPPFLAGS += -DUSE_SIM_MEM
endif

# add by zhouw
ifeq ($(BOARD),linux-sim)
CPPFLAGS += -DUSE_SIM_MEM
endif

ifeq ($(BOARD),ctc-board)
CPPFLAGS += -DUSE_BAY_EMU
endif

CPPFLAGS += -I$(SDK_DIR)/sal/include
CPPFLAGS += -I$(SDK_DIR)/sal/utility/include
CPPFLAGS += -I$(ROOT_DIR)/libctccli/include
CPPFLAGS += -I$(ROOT_DIR)/afx/

ifeq ($(BOARD),ctc-sim)
CPPFLAGS += -DUSE_SIM_MEM
CPPFLAGS += -I$(SDK_DIR)/mem_model/goldengate/include
endif

# add by zhouw
ifeq ($(BOARD),linux-sim)
CPPFLAGS += -DUSE_SIM_MEM
CPPFLAGS += -I$(SDK_DIR)/mem_model/goldengate/include
endif

CPPFLAGS += -I$(SDK_DIR)/driver

include $(MK_DIR)/lib.mk

CFLAGS += -Werror -Wall
endif

ifeq ($(targetbase),vxworks)
OBJ_DIR=$(OBJ_BASE_DIR)/mem_model

all_targets = src libmemmodel

all: $(all_targets) FORCE
clean: $(addprefix clean_,$(all_targets)) FORCE

src: FORCE
	make -C $(SDK_DIR)/mem_model/goldengate/src -f Makefile.vx 

clean_src: FORCE
	make -C $(SDK_DIR)/mem_model/goldengate/src -f Makefile.vx  clean	

libmemmodel: FORCE
	make -f Makefile_libmemmodel.vx 

clean_libmemmodel: FORCE
	make -f Makefile_libmemmodel.vx clean	

.PHONY: FORCE
FORCE:
endif

