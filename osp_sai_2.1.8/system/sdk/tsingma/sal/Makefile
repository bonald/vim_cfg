ifeq ($(targetbase),linux)
include $(MK_DIR)/sys.mk

OBJ_DIR=$(OBJ_BASE_DIR)/sal
#/*SYSTEM MODIFY, Added by weij for update SDK, 2017-09-11*/
LIB = sdksal
SRCS = $(shell find src/linux_um -name "*.c")
SRCS += $(shell find src/common -name "*.c")

CPPFLAGS += -I$(SDK_DIR)/sal/include

include $(MK_DIR)/lib.mk

CFLAGS += -Werror -Wall
endif

ifeq ($(targetbase),vxworks)

OBJ_DIR=$(OBJ_BASE_DIR)/sal

#/*SYSTEM MODIFY, Added by weij for update SDK, 2017-09-11*/
all_targets = src libsdksal

all: $(all_targets) FORCE
clean: $(addprefix clean_,$(all_targets)) FORCE

src: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/sal/src -f Makefile.vx

clean_src: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/sal/src -f Makefile.vx clean

#/*SYSTEM MODIFY, Added by weij for update SDK, 2017-09-11*/
libsdksal: FORCE
	$(CTC_MAKE) -f Makefile_libsal.vx

clean_libsal: FORCE
	$(CTC_MAKE) -f Makefile_libsal.vx clean

.PHONY: FORCE
FORCE:
endif
