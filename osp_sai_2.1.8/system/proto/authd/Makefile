include ../../Makefile.env.inc

include $(MK_DIR)/sys.mk

### proto header path
include $(PROTO_DIR)/Makefile.inc

PROG = authd
SRCS = 
SRCS += $(shell find src -name "*.c")

CPPFLAGS += -I$(PROTO_DIR)/fei/include
CPPFLAGS += -I$(PROTO_DIR)/authd/include
CPPFLAGS += -I${TOP_DIR}/lib/task/ivykis/src/include

DEP_LIBS = $(LIBFEI) $(LIBPROTO) $(LIBCDB) $(LIBSDB) $(LIBSOCK) $(LIBTASK) $(LIBCTCLIB) $(LIBGENLOG) $(LIBCJSON) $(LIBSAL)
DEP_LIBS += $(LIBDERHEX)
LD_LIBS = -L$(LIB_DIR) -lproto -lcdb -lsdb -lfei -lsock -ltask -lctclib -lgenlog -lsal -lcjson -lm -lrt -lhiredis -lpthread -lcrypt -lcrypto
LD_LIBS += -lderhex
LD_LIBS += -lssl

include $(MK_DIR)/prog.mk
