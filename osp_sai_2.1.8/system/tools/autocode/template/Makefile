include ../../Makefile.env.inc

include $(MK_DIR)/sys.mk

### proto header path
include $(PROTO_DIR)/Makefile.inc

PROG = XXX
SRCS = 
SRCS += $(shell find src -name "*.c")

CFLAGS += -I$(PROTO_DIR)/XXX/include
CFLAGS += -I${TOP_DIR}/lib/include

DEP_LIBS = $(LIBFEI) $(LIBPROTO) $(LIBCDB) $(LIBSDB) $(LIBSOCK) $(LIBTASK) $(LIBCTCLIB) $(LIBGENLOG) $(LIBCJSON) $(LIBSAL)
DEP_LIBS += $(LIBDERHEX)
LD_LIBS = -L$(LIB_DIR) -lproto -lcdb -lsdb -lfei -lsock -ltask -lctclib -lgenlog -lsal -lcjson -lm -lrt -lhiredis -lpthread -lcrypt -lcrypto
LD_LIBS += -lderhex
LD_LIBS += -lssl

include $(MK_DIR)/prog.mk
