include $(MK_DIR)/sys.mk

### proto header path
include $(PROTO_DIR)/Makefile.inc

PROG = chsm
SRCS += $(wildcard chsmgt/*.c)
SRCS += $(wildcard stm/*.c)
SRCS += $(wildcard chsmtlk/*.c)
#SRCS += $(wildcard diag/bootup/*.c)
SRCS += $(wildcard diag/cold/*.c)
#SRCS += $(wildcard diag/log/*.c)

CPPFLAGS += -I$(PROTO_DIR)/chsm/include
CPPFLAGS += -I$(PROTO_DIR)/chsm/chsmtlk
CPPFLAGS += -I$(PROTO_DIR)/hal/include/common
CPPFLAGS += -I$(PROTO_DIR)/hal/include/misc
CPPFLAGS += -I$(DRV_TOP_DIR)/device/fiber
CPPFLAGS += -I$(CTC_LIB_DIR)/libasn1
CPPFLAGS += -I$(CTC_LIB_DIR)/msg/lcmmsg/gen

DEP_LIBS = $(LIBFEI) $(LIBPROTO) $(LIBCDB) $(LIBSDB) $(LIBLCMMSG) $(LIBASN1) $(LIBSOCK)
LD_LIBS = -L$(LIB_DIR) -lproto -lcjson -lcdb -lsdb -llcmmsg -lasn1 -lfei -lsock -lhiredis -lcrypt -lssl -lcrypto

DEP_LIBS +=  $(LIBTASK) $(LIBCTCLIB) $(LIBGENLOG) $(LIBSAL)
LD_LIBS += -ltask -lctclib -lgenlog -lsal -lm -lrt -lpthread


include $(MK_DIR)/prog.mk
