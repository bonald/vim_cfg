ifeq ($(targetbase),linux)

include $(MK_DIR)/sys.mk

LIB = libctccli
SRCS =
SRCS += $(shell find src -name "*.c")

CFLAGS += -Werror -Wall
CFLAGS += -I$(SDK_DIR)/ctc_shell/

CPPFLAGS += -I$(SDK_DIR)/sal/include
CPPFLAGS += -I$(SDK_DIR)/core
CPPFLAGS += -I$(SDK_DIR)/libctccli/include
CPPFLAGS += -I$(SDK_DIR)/libctccli/src
CPPFLAGS += -I$(SDK_DIR)/libpktxr/include
CPPFLAGS += -I$(SDK_DIR)/libpktxr/src

include $(MK_DIR)/lib.mk

endif

ifeq ($(targetbase),vxworks)
OBJ_DIR=$(OBJ_BASE_DIR)/libctccli

all_targets = src libctccli

all: $(all_targets) FORCE
clean: $(addprefix clean_,$(all_targets)) FORCE

src: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/libctccli/src -f Makefile.vx

clean_src: FORCE
	$(CTC_MAKE) -C $(SDK_DIR)/libctccli/src -f Makefile.vx clean

libctccli: FORCE
	$(CTC_MAKE) -f Makefile_lib_ctccli.vx

clean_libctccli: FORCE
	$(CTC_MAKE) -f Makefile_lib_ctccli.vx clean

.PHONY: FORCE clean
FORCE:
endif
