ifeq ($(targetbase),linux)
include $(MK_DIR)/sys.mk
OBJ_DIR=$(OBJ_BASE_DIR)/driver
LIB = drv
SRCS =

SRCS += $(foreach n, $(COMPATIBAL_CHIP), $(shell find $(n)/src/    -name "*.c"))

ifeq ($(USW),yes)
SRCS += $(shell find api -name "*.c")
SRCS += $(shell find usw/src/ -name "*.c")
endif

CPPFLAGS += -I$(SDK_DIR)/core/common/include
CPPFLAGS += -I$(SDK_DIR)/dal/include
CPPFLAGS += -I$(SDK_DIR)/sal/include
CPPFLAGS += -I$(SDK_DIR)/sal/utility/include
CPPFLAGS += -I$(ROOT_DIR)/libctccli/include
CPPFLAGS += -I$(ROOT_DIR)/afx/

ifeq ($(BOARD),ctc-sim)
CPPFLAGS += -I$(SDK_DIR)/../cmodel/ctckal/include
ifeq ($(USW),yes)
CPPFLAGS += -I$(CMODEL_DIR)/$(CHIPNAME)/mem_model/include
CPPFLAGS += -I$(CMODEL_DIR)/$(CHIPNAME)/include
CPPFLAGS += -I$(SDK_DIR)/driver/usw/include
else

CPPFLAGS += -I$(CMODEL_DIR)/$(CHIPNAME)/include
ifeq ($(findstring goldengate, $(CHIPNAME)),goldengate)
CPPFLAGS += -I$(SDK_DIR)/mem_model/goldengate/include
endif
ifeq ($(findstring greatbelt, $(CHIPNAME)),greatbelt)
CPPFLAGS += -I$(SDK_DIR)/mem_model/greatbelt/include
CPPFLAGS += -I$(SDK_DIR)/driver/greatbelt/include
endif

endif
endif




ifeq ($(BOARD),ctc-sim)
CPPFLAGS += -DDRVIER_WORK_PLATFORM=1
else
CPPFLAGS += -DDRVIER_WORK_PLATFORM=0
endif

ifeq ($(USW),yes)
CPPFLAGS += -I$(SDK_DIR)/driver/api
CPPFLAGS += -I$(SDK_DIR)/driver/usw/include
endif
CPPFLAGS += -I$(SDK_DIR)/driver

include $(MK_DIR)/lib.mk

CFLAGS += -Werror -Wall
ifeq ($(USW),yes)
CFLAGS += -Wno-unused


endif
endif

ifeq ($(targetbase),vxworks)
OBJ_DIR=$(OBJ_BASE_DIR)/driver

 
#all_targets = common goldengate libdrv 
ifeq ($(USW),yes)
all_targets = usw api libdrv
else
all_targets = $(CHIPNAME) libdrv
endif

all: $(all_targets) FORCE
clean: $(addprefix clean_,$(all_targets)) FORCE

api: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/driver/api -f Makefile.vx

clean_api: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/driver/api -f Makefile.vx  clean	

usw: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/driver/usw/ -f Makefile.vx

clean_usw: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/driver/usw/ -f Makefile.vx  clean

goldengate: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/driver/goldengate/ -f Makefile.vx

clean_goldengate: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/driver/goldengate/ -f Makefile.vx  clean

greatbelt: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/driver/greatbelt/ -f Makefile.vx

clean_greatbelt: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/driver/greatbelt/ -f Makefile.vx clean

libdrv: FORCE
	$(CTC_MAKE) -f Makefile_libdrv.vx

clean_libdrv: FORCE
	$(CTC_MAKE) -f Makefile_libdrv.vx clean	

.PHONY: FORCE
FORCE:
endif

