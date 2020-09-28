
ifndef MK_DIR
include ../Makefile.env.inc
endif

ifeq ($(targetbase),linux)

include $(MK_DIR)/sys.mk

LIB = libctccli
SRCS =
SRCS += $(shell find src -name "*.c")

include $(MK_DIR)/lib.mk

CFLAGS += -Werror -Wall 
CFLAGS += -DVTYSH_DEBUG -DVTYSH

#CPPFLAGS += -I$(TOP_DIR)/tools/vtysh/lib/sal/include
CPPFLAGS += -I$(TOP_DIR)/tools/vtysh/libctccli/include
CPPFLAGS += -I$(TOP_DIR)/tools/vtysh/libctccli/src
CPPFLAGS += -I$(TOP_DIR)/tools/vtysh/lib/afx

endif

ifeq ($(targetbase),vxworks)
BLD_DIR=$(SDK_DIR)/build.$(CPU).$(VER)
OBJ_DIR=$(BLD_DIR)/obj.$(BOARD)/libctccli
LIB_DIR=$(BLD_DIR)/lib.$(BOARD)
BIN_DIR=$(BLD_DIR)/bin.$(BOARD)

all_targets = src libctccli

all: $(all_targets) FORCE
clean: $(addprefix clean_,$(all_targets)) FORCE

src: FORCE
	make -C $(SDK_DIR)/libctccli/src -f Makefile.vx

clean_src: FORCE
	make -C $(SDK_DIR)/libctccli/src -f Makefile.vx clean

libctccli: FORCE
	make -f Makefile_libctccli.vx 

clean_libctccli: FORCE
	make -f Makefile_libctccli.vx clean	

.PHONY: FORCE clean
FORCE:
endif
