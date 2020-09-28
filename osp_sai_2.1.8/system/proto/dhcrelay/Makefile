include ../../Makefile.env.inc

export WARN_NOERROR = y
include $(MK_DIR)/sys.mk

### proto header path
include $(PROTO_DIR)/Makefile.inc

PROG = dhcrelay
SRCS = 
SRCS += $(shell find src -name "*.c")
SRCS += $(shell find ../../apps/dhcp/relay -name "*.c")

CPPFLAGS += -I$(APP_TOP_DIR)/dhcp -I$(APP_TOP_DIR)/dhcp/includes
CPPFLAGS += -DLINUX_MAJOR=2 -DLINUX_MINOR=6 -DLOCALSTATEDIR='"/var"'
CPPFLAGS += -I$(APP_TOP_DIR)/dhcp/relay
CPPFLAGS += -I${TOP_DIR}/lib/task/ivykis/src/include

CPPFLAGS += -I$(PROTO_DIR)/dhcrelay/include
CFLAGS += -I${CTC_LIB_DIR}/derhex

CFLAGS += -I${TOP_DIR}/third_party/precompiled/include

DEP_LIBS = $(LIBFEI) $(LIBPROTO) $(LIBCDB) $(LIBSDB) $(LIBSOCK) $(LIBTASK) $(LIBCTCLIB) $(LIBGENLOG) $(LIBCJSON) $(LIBSAL)
DEP_LIBS += $(LIBDERHEX)
LD_LIBS = -L$(LIB_DIR) -lproto -lcdb -lsdb -lfei -lsock -ltask -lctclib -lgenlog -lsal -lcjson -lm -lrt -lhiredis -lpthread -lcrypt -lcrypto
LD_LIBS += -lderhex
LD_LIBS += -lssl

DEP_LIBS += $(LIB_DIR)/libdhcp6.a $(LIB_DIR)/libomapi6.a $(LIB_DIR)/libdst6.a 
DEP_LIBS += $(LIB_DIR)/libsock.so
LD_LIBS  += -L$(LIB_DIR) -ldhcp6 -lomapi6 -ldst6 -lrt -lsock


include $(MK_DIR)/prog.mk
