include ../../Makefile.env.inc

include $(MK_DIR)/sys.mk

### proto header path
include $(PROTO_DIR)/Makefile.inc

PROG = dhcsnooping
SRCS = src/dhcsnooping_main.c 
SRCS += src/dhcsnooping_cmd.c
SRCS += src/dhcp_common_funcs.c
SRCS += src/dhcsnooping_func.c
SRCS += src/dhcsnoopingv6_func.c

CPPFLAGS += -I$(APP_TOP_DIR)/dhcp -I$(APP_TOP_DIR)/dhcp/includes
CPPFLAGS += -DLINUX_MAJOR=2 -DLINUX_MINOR=6 -DLOCALSTATEDIR='"/var"'
CPPFLAGS += -I${TOP_DIR}/lib/task/ivykis/src/include

CPPFLAGS += -I$(PROTO_DIR)/fei/include
CPPFLAGS += -I$(PROTO_DIR)/dhcsnooping/include
CFLAGS += -I${CTC_LIB_DIR}/derhex

CFLAGS += -I${TOP_DIR}/third_party/precompiled/include

DEP_LIBS = $(LIBHAL) $(LIBPROTO) $(LIBCDB) $(LIBSDB) $(LIBSOCK) $(LIBTASK) $(LIBCTCLIB) $(LIBGENLOG) $(LIBCJSON) $(LIBSAL)
DEP_LIBS += $(LIBDERHEX)
LD_LIBS = -L$(LIB_DIR) -lproto -lcdb -lsdb -lfei -lsock -ltask -lctclib -lgenlog -lsal -lcjson -lm -lrt -lhiredis -lpthread -lcrypt -lcrypto
LD_LIBS += -lderhex
LD_LIBS += -lssl

DEP_LIBS += $(LIB_DIR)/libdhcp6.a $(LIB_DIR)/libomapi6.a $(LIB_DIR)/libdst6.a 
DEP_LIBS += $(LIB_DIR)/libsock.so
LD_LIBS  += -L$(LIB_DIR) -ldhcp6 -lomapi6 -ldst6 -lrt -lsock -ldhcp6 -lcrypt

include $(MK_DIR)/prog.mk
